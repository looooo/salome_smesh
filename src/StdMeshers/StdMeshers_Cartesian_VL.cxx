// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File      : StdMeshers_Cartesian_VL.cxx
// Created   : Tue May 24 13:03:09 2022
// Author    : Edward AGAPOV (eap)

#include "StdMeshers_Cartesian_VL.hxx"
#include "StdMeshers_ViscousLayers.hxx"

#include <SMDS_MeshGroup.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMESHDS_SubMesh.hxx>
#include <SMESH_Algo.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESH_MeshEditor.hxx>
#include <SMESH_MesherHelper.hxx>
#include <SMESH_TypeDefs.hxx>
#include <SMESH_subMesh.hxx>

#include <BRepAdaptor_Curve.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

using namespace StdMeshers_Cartesian_VL;

namespace
{
  typedef int                     TGeomID; // IDs of sub-shapes

  /*!
   * \brief Temporary mesh
   */
  struct TmpMesh: public SMESH_Mesh
  {
    TmpMesh() {
      _isShapeToMesh = (_id = 0);
      _meshDS  = new SMESHDS_Mesh( _id, true );
    }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Edge of viscous layers; goes from a grid node by normal to boundary
   */
  struct VLEdge
  {
    std::vector< SMESH_NodeXYZ > _nodes;
    double                       _uv[2]; // position of TgtNode() on geometry
    double                       _length; 

    const SMDS_MeshNode* TgtNode() const { return _nodes.back().Node(); }
  };
  typedef NCollection_DataMap< const SMDS_MeshNode*, VLEdge* > TNode2VLEdge;
  // --------------------------------------------------------------------------
  /*!
   * \brief VLEdge's of one shape
   */
  struct VLEdgesOnShape
  {
    std::vector< VLEdge > _edges;
    TopoDS_Shape          _initShape;
    TopoDS_Shape          _offsetShape;
    int                   _initShapeID;
    bool                  _toCheckCoinc; // to check if nodes on shape boundary coincide
  };

  //================================================================================
  /*!
   * \brief Project a point on offset FACE to EDGEs of an initial FACE
   *  \param [in] offP - point to project
   *  \param [in] initFace - FACE to project on
   *  \return gp_Pnt - projection point
   */
  //================================================================================

  gp_Pnt projectToWire( const gp_Pnt&      offP,
                        const TopoDS_Face& initFace,
                        gp_Pnt2d &         uv )
  {
    double   minDist = Precision::Infinite();
    const double tol = Precision::Confusion();
    gp_Pnt              proj, projction;
    Standard_Real       param;
    ShapeAnalysis_Curve projector;

    for ( TopExp_Explorer eExp( initFace, TopAbs_EDGE ); eExp.More(); eExp.Next() )
    {
      BRepAdaptor_Curve initCurve( TopoDS::Edge( eExp.Current() ));
      //const double f = initCurve.FirstParameter(), l = initCurve.LastParameter();
      double dist = projector.Project( initCurve, offP, tol, proj, param, /*adjustToEnds=*/false );
      if ( dist < minDist )
      {
        projction = proj;
        minDist = dist;
      }
    }
    uv.SetCoord(0.,0.); // !!!!!!!
    return projction;
  }

  //================================================================================
  /*!
   * \brief Project nodes from offset FACE to initial FACE
   *  \param [inout] theEOS - VL edges on a geom FACE
   *  \param [inout] theOffsetMDS - offset mesh to fill in
   */
  //================================================================================

  void projectToFace( VLEdgesOnShape & theEOS,
                      SMESHDS_Mesh*    theOffsetMDS,
                      TNode2VLEdge &   theN2E )
  {
    SMESHDS_SubMesh* sm = theOffsetMDS->MeshElements( theEOS._offsetShape );
    if ( !sm || sm->NbElements() == 0 || sm->NbNodes() == 0 )
      return;
    theEOS._edges.resize( sm->NbNodes() );

    const TopoDS_Face& initFace = TopoDS::Face( theEOS._initShape );
    ShapeAnalysis_Surface projector( BRep_Tool::Surface( initFace ));
    const double clsfTol = 1e2 * BRep_Tool::MaxTolerance( initFace, TopAbs_VERTEX );
    BRepTopAdaptor_FClass2d classifier( initFace, clsfTol );

    const double        tol = Precision::Confusion();
    //const double f = initCurve.FirstParameter(), l = initCurve.LastParameter();
    gp_Pnt              proj;

    int iN = 0;
    for ( SMDS_NodeIteratorPtr nIt = sm->GetNodes(); nIt->more(); ++iN )
    {
      SMESH_NodeXYZ offP = nIt->next();
      gp_Pnt2d uv = projector.ValueOfUV( offP, tol );
      TopAbs_State st = classifier.Perform( uv );
      if ( st == TopAbs_IN || st == TopAbs_ON )
      {
        proj = projector.Value( uv );
      }
      else
      {
        proj = projectToWire( offP, initFace, uv );
      }
      if ( st == TopAbs_ON || st == TopAbs_OUT )
        theEOS._toCheckCoinc = true;

      VLEdge & vlEdge = theEOS._edges[ iN ];
      vlEdge._nodes.push_back( offP.Node() );
      vlEdge._nodes.push_back( theOffsetMDS->AddNode( proj.X(), proj.Y(), proj.Z() ));
      vlEdge._uv[0] = uv.X();
      vlEdge._uv[1] = uv.Y();
      //vlEdge._length = proj.Distance( offP );

      theN2E.Bind( offP.Node(), &vlEdge );
    }
    return;
    
  }

  //================================================================================
  /*!
   * \brief Project nodes from offset EDGE to initial EDGE
   *  \param [inout] theEOS - VL edges on a geom EDGE
   *  \param [inout] theOffsetMDS - offset mesh to add new nodes to
   */
  //================================================================================

  void projectToEdge( VLEdgesOnShape & theEOS,
                      SMESHDS_Mesh*    theOffsetMDS,
                      TNode2VLEdge &   theN2E )
  {
    SMESHDS_SubMesh* sm = theOffsetMDS->MeshElements( theEOS._offsetShape );
    if ( !sm || sm->NbElements() == 0 )
      return;
    theEOS._edges.resize( sm->NbNodes() );

    ShapeAnalysis_Curve projector;
    BRepAdaptor_Curve   initCurve( TopoDS::Edge( theEOS._initShape ));
    const double        tol = Precision::Confusion();
    const double f = initCurve.FirstParameter(), l = initCurve.LastParameter();
    gp_Pnt              proj;
    Standard_Real       param;

    int iN = 0;
    for ( SMDS_NodeIteratorPtr nIt = sm->GetNodes(); nIt->more(); ++iN )
    {
      SMESH_NodeXYZ offP = nIt->next();
      double dist = projector.Project( initCurve, offP, tol, proj, param, /*adjustToEnds=*/false );
      bool paramOK = ( f < param && param < l );
      if ( !paramOK )
      {
        if ( param < f )
          param = f;
        else if ( param > l )
          param = l;
        theEOS._toCheckCoinc = true;
      }
      proj = initCurve.Value( param );

      VLEdge & vlEdge = theEOS._edges[ iN ];
      vlEdge._nodes.push_back( offP.Node() );
      vlEdge._nodes.push_back( theOffsetMDS->AddNode( proj.X(), proj.Y(), proj.Z() ));
      vlEdge._uv[0] = param;
      vlEdge._length = paramOK ? dist : proj.Distance( offP );

      theN2E.Bind( offP.Node(), &vlEdge );
    }
    return;
  }

  //================================================================================
  /*!
   * \brief Compute heights of viscous layers and finds FACEs with VL
   *  \param [in] hyp - viscous layers hypothesis
   *  \param [in] mesh - the main mesh
   *  \param [in] shape - the main shape
   *  \param [out] vlH - heights of viscous layers to compute
   *  \param [out] shapesWVL - IDs of shapes with VL
   *  \return bool - isOK
   */
  //================================================================================

  bool computeVLHeight( const StdMeshers_ViscousLayers* hyp,
                        SMESHDS_Mesh *                  mesh,
                        const TopoDS_Shape &            shape,
                        std::vector< double > &         vlH,
                        std::set< TGeomID > &           shapesWVL )
  {
    const double  T = hyp->GetTotalThickness();
    const double  f = hyp->GetStretchFactor();
    const int     N = hyp->GetNumberLayers();
    const double h0 = hyp->Get1stLayerThickness( T, f, N );

    vlH.reserve( hyp->GetNumberLayers() );
    vlH.push_back( h0 );
    for ( double h = h0 * f; (int)vlH.size() < N-1; h *= f )
      vlH.push_back( h + vlH.back() );
    vlH.push_back( T );


    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes( shape, TopAbs_FACE, faces );

    if ( hyp->IsToIgnoreShapes() )
    {
      for ( int i = 1; i <= faces.Size(); ++i )
        shapesWVL.insert( mesh->ShapeToIndex( faces( i )));

      for ( TGeomID& sID : hyp->GetBndShapes() )
        shapesWVL.erase( sID );
    }
    else
    {
      for ( TGeomID& sID : hyp->GetBndShapes() )
        shapesWVL.insert( sID );
    }

    for ( TGeomID fID : shapesWVL )
    {
      const TopoDS_Shape & face = mesh->IndexToShape( fID );
      for ( TopExp_Explorer exp( face, TopAbs_EDGE ); exp.More(); exp.Next() )
        shapesWVL.insert( mesh->ShapeToIndex( exp.Current() ));
      for ( TopExp_Explorer exp( face, TopAbs_VERTEX ); exp.More(); exp.Next() )
        shapesWVL.insert( mesh->ShapeToIndex( exp.Current() ));
    }

    return !shapesWVL.empty();
  }

  //================================================================================
  /*!
   * \brief Create intermediate nodes on VLEdge
   *  \param [inout] vlEdge - VLEdge to divide
   *  \param [in] vlH - thicknesses of layers
   *  \param [inout] mesh - the mesh no fill in
   */
  //================================================================================

  void divideVLEdge( VLEdge*                      vlEdge,
                     const std::vector< double >& vlH,
                     SMESHDS_Mesh*                mesh )
  {
    SMESH_NodeXYZ lastNode = vlEdge->_nodes.back();
    SMESH_NodeXYZ frstNode = vlEdge->_nodes[0];
    gp_XYZ dir = frstNode - lastNode;

    vlEdge->_nodes.resize( vlH.size() + 1 );

    for ( size_t i = 1; i < vlH.size(); ++i )
    {
      double r = vlH[ i-1 ] / vlH.back();
      gp_XYZ p = lastNode + dir * r;
      vlEdge->_nodes[ vlH.size() - i ] = mesh->AddNode( p.X(), p.Y(), p.Z() );
    }
    vlEdge->_nodes.back() = lastNode;
  }

  //================================================================================
  /*!
   * \brief Create a polyhedron from nodes of VLEdge's
   *  \param [in] edgesVec - the VLEdge's
   *  \param [in] vNodes - node buffer
   *  \param [in] editor - editor of offset mesh
   */
  //================================================================================

  bool makePolyhedron( const std::vector< VLEdge*> &         edgesVec,
                       std::vector< const SMDS_MeshNode* > & vNodes,
                       SMESH_MeshEditor&                     editor,
                       SMESH_MeshEditor::ElemFeatures        elemType)
  {
    elemType.SetPoly( true );
    elemType.myPolyhedQuantities.clear();
    elemType.myNodes.clear();

    // add facets of walls
    size_t nbBaseNodes = edgesVec.size();
    for ( size_t iN = 0; iN < nbBaseNodes; ++iN )
    {
      VLEdge* e0 = edgesVec[ iN ];
      VLEdge* e1 = edgesVec[( iN + 1 ) % nbBaseNodes ];
      size_t nbN0 = e0->_nodes.size();
      size_t nbN1 = e1->_nodes.size();
      if ( nbN0 == nbN1 )
      {
        for ( size_t i = 1; i < nbN0; ++i )
        {
          elemType.myNodes.push_back( e1->_nodes[ i - 1 ].Node());
          elemType.myNodes.push_back( e1->_nodes[ i     ].Node());
          elemType.myNodes.push_back( e0->_nodes[ i     ].Node());
          elemType.myNodes.push_back( e0->_nodes[ i - 1 ].Node());
          elemType.myPolyhedQuantities.push_back( 4 );
        }
      }
      else
      {
        for ( size_t i = 0; i < nbN1; ++i )
          elemType.myNodes.push_back( e1->_nodes[ i ].Node() );
        for ( size_t i = 0; i < nbN0; ++i )
          elemType.myNodes.push_back( e0->_nodes[ nbN0 - 1 - i ].Node() );
        elemType.myPolyhedQuantities.push_back( nbN0 + nbN1 );
      }
    }

    // add facets of top
    vNodes.clear();
    for ( size_t iN = 0; iN < nbBaseNodes; ++iN )
    {
      VLEdge* e0 = edgesVec[ iN ];
      elemType.myNodes.push_back( e0->_nodes.back().Node() );
      vNodes.push_back( e0->_nodes[ 0 ].Node());
    }
    elemType.myPolyhedQuantities.push_back( nbBaseNodes );

    // add facets of bottom
    elemType.myNodes.insert( elemType.myNodes.end(), vNodes.rbegin(), vNodes.rend() );
    elemType.myPolyhedQuantities.push_back( nbBaseNodes );

    const SMDS_MeshElement* vol = editor.AddElement( elemType.myNodes, elemType );
    vol->setIsMarked( true ); // to add to group

    return vol;
  }

  //================================================================================
  /*!
   * \brief Transform prism nodal connectivity to that of polyhedron
   *  \param [inout] nodes - nodes of prism, return nodes of polyhedron
   *  \param [inout] elemType - return quantities of polyhedron,
   */
  //================================================================================

  void prism2polyhedron( std::vector< const SMDS_MeshNode* > & nodes,
                         SMESH_MeshEditor::ElemFeatures &      elemType )
  {
    elemType.myPolyhedQuantities.clear();
    elemType.myNodes.clear();

    // walls
    size_t nbBaseNodes = nodes.size() / 2;
    for ( size_t i = 0; i < nbBaseNodes; ++i )
    {
      int iNext = ( i + 1 ) % nbBaseNodes;
      elemType.myNodes.push_back( nodes[ iNext ]);
      elemType.myNodes.push_back( nodes[ i ]);
      elemType.myNodes.push_back( nodes[ i + nbBaseNodes ]);
      elemType.myNodes.push_back( nodes[ iNext + nbBaseNodes ]);
      elemType.myPolyhedQuantities.push_back( 4 );
    }

    // base
    elemType.myNodes.insert( elemType.myNodes.end(), nodes.begin(), nodes.begin() + nbBaseNodes );
    elemType.myPolyhedQuantities.push_back( nbBaseNodes );

    // top
    elemType.myNodes.insert( elemType.myNodes.end(), nodes.rbegin(), nodes.rbegin() + nbBaseNodes );
    elemType.myPolyhedQuantities.push_back( nbBaseNodes );

    nodes.swap( elemType.myNodes );
  }

  //================================================================================
  /*!
   * \brief Create prisms from faces
   *  \param [in] theEOS - shape to treat
   *  \param [inout] theMesh - offset mesh to fill in
   */
  //================================================================================

  bool makePrisms( VLEdgesOnShape & theEOS,
                   SMESH_Mesh*      theMesh,
                   TNode2VLEdge   & theN2E )
  {
    SMESHDS_SubMesh* sm = theMesh->GetMeshDS()->MeshElements( theEOS._offsetShape );
    if ( !sm || sm->NbElements() == 0 )
      return true;

    SMESH_MeshEditor editor( theMesh );
    SMESH_MeshEditor::ElemFeatures volumElem( SMDSAbs_Volume );

    std::vector< const SMDS_MeshNode* > vNodes;
    std::vector< VLEdge*> edgesVec;
    for ( SMDS_ElemIteratorPtr eIt = sm->GetElements(); eIt->more(); )
    {
      const SMDS_MeshElement* face = eIt->next();
      if ( face->GetType() != SMDSAbs_Face )
        continue;

      const int nbNodes = face->NbCornerNodes();
      edgesVec.resize( nbNodes );
      vNodes.resize( nbNodes * 2 );
      int maxNbLayer = 0, minNbLayer = IntegerLast();
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* n = face->GetNode( i );
        if ( !theN2E.IsBound( n ))
          return false;
        edgesVec[ i ] = theN2E( n );
        maxNbLayer = Max( maxNbLayer, edgesVec[i]->_nodes.size() - 1 );
        minNbLayer = Min( minNbLayer, edgesVec[i]->_nodes.size() - 1 );
      }
      if ( maxNbLayer == minNbLayer )
      {
        size_t nbPrism = edgesVec[0]->_nodes.size() - 1;
        for ( size_t iP = 0; iP < nbPrism; ++iP )
        {
          vNodes.resize( nbNodes * 2 );
          for ( int i = 0; i < nbNodes; ++i )
          {
            vNodes[ i           ] = edgesVec[ i ]->_nodes[ iP + 1 ].Node();
            vNodes[ i + nbNodes ] = edgesVec[ i ]->_nodes[ iP ].Node();
          }
          volumElem.SetPoly( nbNodes > 4 );
          if ( volumElem.myIsPoly )
            prism2polyhedron( vNodes, volumElem );

          if ( const SMDS_MeshElement* vol = editor.AddElement( vNodes, volumElem ))
            vol->setIsMarked( true ); // to add to group
        }
      }
      else // at inlet/outlet
      {
        makePolyhedron( edgesVec, vNodes, editor, volumElem );
      }
      editor.ClearLastCreated();

      // move the face to the top of prisms, on mesh boundary
      //theMesh->GetMeshDS()->ChangeElementNodes( face, fNodes.data(), nbNodes );
    }
    return true;
  }

  //================================================================================
  /*!
   * \brief Append the offset mesh to the initial one
   */
  //================================================================================

  void copyMesh( SMESH_Mesh* theFromMesh,
                 SMESH_Mesh* theToMesh,
                 int         theSolidID)
  {
    SMESHDS_Mesh* offsetMDS = theFromMesh->GetMeshDS();
    SMESHDS_Mesh*   initMDS = theToMesh->GetMeshDS();

    const smIdType nShift = initMDS->NbNodes();

    for ( SMDS_NodeIteratorPtr nIt = offsetMDS->nodesIterator(); nIt->more(); )
    {
      SMESH_NodeXYZ pOff = nIt->next();
      const SMDS_MeshNode* n = initMDS->AddNodeWithID( pOff.X(), pOff.Y(), pOff.Z(),
                                                       nShift + pOff.Node()->GetID() );
      initMDS->SetNodeInVolume( n, theSolidID );
    }

    SMESH_MeshEditor editor( theToMesh );
    SMESH_MeshEditor::ElemFeatures elemType;
    std::vector<smIdType> nIniIDs;

    for ( SMDS_ElemIteratorPtr eIt = offsetMDS->elementsIterator(); eIt->more(); )
    {
      const SMDS_MeshElement* offElem = eIt->next();
      const int nbNodes = offElem->NbNodes();
      nIniIDs.resize( nbNodes );
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* offNode = offElem->GetNode( i );
        nIniIDs[ i ] = offNode->GetID() + nShift;
      }
      elemType.Init( offElem, /*basicOnly=*/false );
      const SMDS_MeshElement* iniElem = editor.AddElement( nIniIDs, elemType );
      initMDS->SetMeshElementOnShape( iniElem, theSolidID );
      iniElem->setIsMarked( offElem->isMarked() );
      editor.ClearLastCreated();
    }
    return;
  }

  //================================================================================
  /*!
   * \brief set elements of layers boundary to sub-meshes
   *  \param [in] theEOS - vlEdges of a sub-shape
   *  \param [inout] theMesh - the mesh
   *  \param [in] theN2E - map of node to vlEdge
   *  \param [in] theNShift - nb of nodes in the mesh before adding the offset mesh
   */
  //================================================================================

  void setBnd2Sub( VLEdgesOnShape &     theEOS,
                   SMESH_Mesh*          theInitMesh,
                   SMESH_Mesh*          theOffsMesh,
                   const TNode2VLEdge & theN2E,
                   const smIdType       theNShift,
                   TIDSortedNodeSet&    theNodesToCheckCoinc )
  {
    SMESHDS_Mesh* offsetMDS = theOffsMesh->GetMeshDS();
    SMESHDS_Mesh*   initMDS = theInitMesh->GetMeshDS();

    SMESHDS_SubMesh* sm = offsetMDS->MeshElements( theEOS._offsetShape );
    if ( !sm || ( sm->NbElements() + sm->NbNodes() == 0 ))
      return;

    for ( SMDS_NodeIteratorPtr nIt = sm->GetNodes(); nIt->more(); )
    {
      const SMDS_MeshNode* offNode = nIt->next();
      VLEdge* vlEd = theN2E( offNode );
      const SMDS_MeshNode* nOffBnd = vlEd->_nodes.back().Node();
      const SMDS_MeshNode* nIniBnd = initMDS->FindNode( nOffBnd->GetID() + theNShift );
      initMDS->UnSetNodeOnShape( nIniBnd );

      switch( theEOS._initShape.ShapeType() ) {
      case TopAbs_FACE:   initMDS->SetNodeOnFace( nIniBnd, TopoDS::Face( theEOS._initShape ),
                                                  vlEd->_uv[0], vlEd->_uv[1] );
        break;
      case TopAbs_EDGE:   initMDS->SetNodeOnEdge( nIniBnd, TopoDS::Edge( theEOS._initShape ),
                                                  vlEd->_uv[0]);
        break;
      case TopAbs_VERTEX: initMDS->SetNodeOnVertex( nIniBnd, TopoDS::Vertex( theEOS._initShape ));
        break;
      default:;
      }
    }

    std::vector< const SMDS_MeshNode* > iniNodes, iniNodesBnd;
    std::vector< VLEdge*> edgesVec;
    for ( SMDS_ElemIteratorPtr eIt = sm->GetElements(); eIt->more(); )
    {
      const SMDS_MeshElement* offElem = eIt->next();
      if ( offElem->GetType() != SMDSAbs_Face &&
           offElem->GetType() != SMDSAbs_Edge )
        continue;

      const int nbNodes = offElem->NbCornerNodes();
      iniNodes.resize( nbNodes );
      iniNodesBnd.resize( nbNodes );
      //edgesVec.resize( nbNodes );
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* nOff = offElem->GetNode( i );
        const SMDS_MeshNode* nIni = initMDS->FindNode( nOff->GetID() + theNShift );
        iniNodes[ i ] = nIni;
        VLEdge* vlEd = theN2E( nOff );
        const SMDS_MeshNode* nOffBnd = vlEd->_nodes.back().Node();
        const SMDS_MeshNode* nIniBnd = initMDS->FindNode( nOffBnd->GetID() + theNShift );
        iniNodesBnd[ i ] = nIniBnd;
      }
      if ( const SMDS_MeshElement* iniElem = initMDS->FindElement( iniNodes ))
      {
        initMDS->UnSetElementOnShape( iniElem );
        initMDS->SetMeshElementOnShape( iniElem, theEOS._initShape );

        // move the face to the top of prisms, on init mesh boundary
        initMDS->ChangeElementNodes( iniElem, iniNodesBnd.data(), nbNodes );

        if ( theEOS._toCheckCoinc )
          theNodesToCheckCoinc.insert( iniNodesBnd.begin(), iniNodesBnd.end() );
      }
    }
    return;
  }
} // namespace


//================================================================================
/*!
 * \brief Create a temporary mesh used to hold elements of the offset shape
 */
//================================================================================

SMESH_Mesh* StdMeshers_Cartesian_VL::ViscousBuilder::MakeOffsetMesh()
{
  return _offsetMesh;
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

StdMeshers_Cartesian_VL::ViscousBuilder::ViscousBuilder( const StdMeshers_ViscousLayers* hyp )
  : _hyp( hyp ), _offsetMesh( new TmpMesh )
{
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshers_Cartesian_VL::ViscousBuilder::~ViscousBuilder()
{
  delete _offsetMesh; _offsetMesh = 0;
}

//================================================================================
/*!
 * \brief Create an offset shape from a given one
 *  \param [in] theShape - input shape
 *  \param [out] theError - error description
 *  \return TopoDS_Shape - result offset shape
 */
//================================================================================

TopoDS_Shape
StdMeshers_Cartesian_VL::ViscousBuilder::MakeOffsetShape(const TopoDS_Shape & theShape,
                                                         std::string &        theError )
{
  // TopoDS_Shape S = theShape;
  //     BRep_Builder aBuilder;
  //     TopoDS_Compound comp;
  //     aBuilder.MakeCompound( comp );
  // for ( TopExp_Explorer e( theShape, TopAbs_FACE); e.More(); e.Next() )
  // {
  //   aBuilder.Add( comp, e.Current());
  //   e.Next();
  //   if ( !e.More() )
  //     break;
  // }
  // S = comp;

  double offset = -_hyp->GetTotalThickness();
  double    tol = Precision::Confusion();
  _makeOffset.Initialize( theShape, offset, tol, BRepOffset_Skin, /*Intersection=*/false,
                          /*selfInter=*/false, GeomAbs_Intersection );
  _makeOffset.MakeOffsetShape();
  if ( _makeOffset.IsDone() )
  {
    _offsetShape = _makeOffset.Shape();
    _offsetMesh->ShapeToMesh( _offsetShape );
    _offsetMesh->GetSubMesh( _offsetShape )->DependsOn();
    //SMESH_MesherHelper::WriteShape( _offsetShape );
    return _offsetShape;
  }

  switch ( _makeOffset.Error() )
  {
  case BRepOffset_NoError:
    theError = "OK. Offset performed successfully.";break;
  case BRepOffset_BadNormalsOnGeometry:
    theError = "Degenerated normal on input data.";break;
  case BRepOffset_C0Geometry:
    theError = "C0 continuity of input data.";break;
  case BRepOffset_NullOffset:
    theError = "Null offset of all faces.";break;
  case BRepOffset_NotConnectedShell:
    theError = "Incorrect set of faces to remove, the remaining shell is not connected.";break;
  case BRepOffset_CannotTrimEdges:
    theError = "Can not trim edges.";break;
  case BRepOffset_CannotFuseVertices:
    theError = "Can not fuse vertices.";break;
  case BRepOffset_CannotExtentEdge:
    theError = "Can not extent edge.";break;
  default:
    theError = "operation not done.";
  }
  theError = "BRepOffset_MakeOffset error: " + theError;

  return TopoDS_Shape();
}

//================================================================================
/*!
 * \brief Return a sub-shape of the offset shape generated from a given initial sub-shape
 */
//================================================================================

TopoDS_Shape StdMeshers_Cartesian_VL::ViscousBuilder::getOffsetSubShape( const TopoDS_Shape& S )
{
  const TopTools_ListOfShape& newShapes = _makeOffset.Generated( S );
  for ( const TopoDS_Shape& ns : newShapes )
    if ( ns.ShapeType() == S.ShapeType() )
      return ns;
  return TopoDS_Shape();
}

//================================================================================
/*!
 * \brief Create prismatic mesh between _offsetShape and theShape
 *  \param [out] theMesh - mesh to fill in
 *  \param [in] theShape - initial shape
 *  \return bool - is Ok
 */
//================================================================================

bool StdMeshers_Cartesian_VL::ViscousBuilder::MakeViscousLayers( SMESH_Mesh &         theMesh,
                                                                 const TopoDS_Shape & theShape )
{
  SMESHDS_Mesh* offsetMDS = _offsetMesh->GetMeshDS();
  SMESHDS_Mesh*   initMDS = theMesh.GetMeshDS();
  offsetMDS->SetAllCellsNotMarked();

  // Compute heights of viscous layers and finds FACEs with VL
  std::vector< double > vlH;
  std::set< int > shapesWVL;
  if ( !computeVLHeight( _hyp, initMDS, theShape, vlH, shapesWVL ))
    return false;

  std::vector< VLEdgesOnShape > edgesOnShape;
  edgesOnShape.reserve( offsetMDS->MaxShapeIndex() + 1 );
  TNode2VLEdge n2e;

  // loop on sub-shapes to project nodes from offset boundary to initial boundary
  TopAbs_ShapeEnum types[3] = { TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE };
  for ( TopAbs_ShapeEnum shType : types )
  {
    TopTools_IndexedMapOfShape shapes;
    TopExp::MapShapes( theShape, shType, shapes );
    for ( int i = 1; i <= shapes.Size(); ++i )
    {
      edgesOnShape.resize( edgesOnShape.size() + 1 );
      VLEdgesOnShape& EOS = edgesOnShape.back();

      EOS._initShape    = shapes( i );
      EOS._offsetShape  = getOffsetSubShape( EOS._initShape );
      EOS._initShapeID  = initMDS->ShapeToIndex( EOS._initShape );
      EOS._toCheckCoinc = false;

      // project boundary nodes of offset mesh to boundary of init mesh
      // (new nodes are created in the offset mesh)
      switch( EOS._offsetShape.ShapeType() ) {
      case TopAbs_VERTEX:
      {
        EOS._edges.resize( 1 );
        EOS._edges[0]._nodes.resize( 2 );
        EOS._edges[0]._nodes[0] = SMESH_Algo::VertexNode( TopoDS::Vertex( EOS._offsetShape ),
                                                          offsetMDS );
        gp_Pnt offP = BRep_Tool::Pnt( TopoDS::Vertex( EOS._initShape ));
        EOS._edges[0]._nodes[1] = offsetMDS->AddNode( offP.X(), offP.Y(), offP.Z() );
        //EOS._edges[0]._length   = offP.Distance( EOS._edges[0]._nodes[0] );
        n2e.Bind( EOS._edges[0]._nodes[0].Node(), & EOS._edges[0] );
        break;
      }
      case TopAbs_EDGE:
      {
        projectToEdge( EOS, offsetMDS, n2e );
        break;
      }
      case TopAbs_FACE:
      {
        projectToFace( EOS, offsetMDS, n2e );
        break;
      }
      default:;
      }

      // create nodes of layers
      if ( _hyp->GetNumberLayers() > 1 )
      {
        if ( shapesWVL.count( EOS._initShapeID ))
          for ( size_t i = 0; i < EOS._edges.size(); ++i )
          {
            divideVLEdge( &EOS._edges[ i ], vlH, offsetMDS );
          }
      }
    } // loop on shapes
  } // loop on shape types

  // create prisms
  bool prismsOk = true;
  for ( size_t i = 0; i < edgesOnShape.size(); ++i )
  {
    VLEdgesOnShape& EOS = edgesOnShape[ i ];
    if ( EOS._initShape.ShapeType() == TopAbs_FACE )
    {
      if ( !makePrisms( EOS, _offsetMesh, n2e ))
        prismsOk = false;
    }
  }

  // copy offset mesh to the main one
  initMDS->Modified();
  initMDS->CompactMesh();
  smIdType nShift = initMDS->NbNodes();
  TGeomID solidID = initMDS->ShapeToIndex( theShape );
  copyMesh( _offsetMesh, & theMesh, solidID );


  if ( !prismsOk )
  {
    if ( SMESH_subMesh * sm = theMesh.GetSubMesh( theShape ))
    {
      sm->GetComputeError() =
        SMESH_ComputeError::New( COMPERR_ALGO_FAILED,
                                 "Viscous layers construction error: bad mesh on offset geometry" );
    }
    return prismsOk;
  }

  // set elements of layers boundary to sub-meshes
  TIDSortedNodeSet nodesToCheckCoinc;
  for ( size_t i = 0; i < edgesOnShape.size(); ++i )
  {
    VLEdgesOnShape& EOS = edgesOnShape[ i ];
    setBnd2Sub( EOS, &theMesh, _offsetMesh, n2e, nShift, nodesToCheckCoinc );
  }

  // merge coincident nodes
  SMESH_MeshEditor editor( &theMesh );
  SMESH_MeshEditor::TListOfListOfNodes nodesToMerge;
  editor.FindCoincidentNodes( nodesToCheckCoinc, vlH[0]/20., nodesToMerge, false );
  editor.MergeNodes( nodesToMerge );

  // create a group
  if ( !_hyp->GetGroupName().empty() )
  {
    SMDS_MeshGroup* group = _hyp->CreateGroup( _hyp->GetGroupName(), theMesh, SMDSAbs_Volume );

    for ( SMDS_ElemIteratorPtr it = initMDS->elementsIterator(); it->more(); )
    {
      const SMDS_MeshElement* el = it->next();
      if ( el->isMarked() )
        group->Add( el );
    }
  }

  return prismsOk;
}
