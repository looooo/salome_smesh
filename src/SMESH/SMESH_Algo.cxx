// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

//  SMESH SMESH : implementaion of SMESH idl descriptions
//  File   : SMESH_Algo.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH

#include "SMESH_Algo.hxx"

#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMESH_Comment.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypoFilter.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_TypeDefs.hxx"
#include "SMESH_subMesh.hxx"

#include <Basics_OCCTVersion.hxx>

#include <BRepAdaptor_Curve.hxx>
#include <BRepLProp.hxx>
#include <BRep_Tool.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Geom_Surface.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>

#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>

#include "utilities.h"

#include <algorithm>
#include <limits>

using namespace std;

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Algo::SMESH_Algo (int hypId, int studyId, SMESH_Gen * gen)
  : SMESH_Hypothesis(hypId, studyId, gen)
{
  gen->_mapAlgo[hypId] = this;

  _onlyUnaryInput = _requireDiscreteBoundary = _requireShape = true;
  _quadraticMesh = _supportSubmeshes = false;
  _error = COMPERR_OK;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Algo::~SMESH_Algo()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_0D_Algo::SMESH_0D_Algo(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Algo(hypId, studyId, gen)
{
  _shapeType = (1 << TopAbs_VERTEX);
  _type = ALGO_0D;
  gen->_map0D_Algo[hypId] = this;
}
SMESH_1D_Algo::SMESH_1D_Algo(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Algo(hypId, studyId, gen)
{
  _shapeType = (1 << TopAbs_EDGE);
  _type = ALGO_1D;
  gen->_map1D_Algo[hypId] = this;
}
SMESH_2D_Algo::SMESH_2D_Algo(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Algo(hypId, studyId, gen)
{
  _shapeType = (1 << TopAbs_FACE);
  _type = ALGO_2D;
  gen->_map2D_Algo[hypId] = this;
}
SMESH_3D_Algo::SMESH_3D_Algo(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Algo(hypId, studyId, gen)
{
  _shapeType = (1 << TopAbs_SOLID);
  _type = ALGO_3D;
  gen->_map3D_Algo[hypId] = this;
}

//=============================================================================
/*!
 * Usually an algoritm has nothing to save
 */
//=============================================================================

ostream & SMESH_Algo::SaveTo(ostream & save) { return save; }
istream & SMESH_Algo::LoadFrom(istream & load) { return load; }

//=============================================================================
/*!
 *  
 */
//=============================================================================

const vector < string > &SMESH_Algo::GetCompatibleHypothesis()
{
  return _compatibleHypothesis;
}

//=============================================================================
/*!
 *  List the hypothesis used by the algorithm associated to the shape.
 *  Hypothesis associated to father shape -are- taken into account (see
 *  GetAppliedHypothesis). Relevant hypothesis have a name (type) listed in
 *  the algorithm. This method could be surcharged by specific algorithms, in 
 *  case of several hypothesis simultaneously applicable.
 */
//=============================================================================

const list <const SMESHDS_Hypothesis *> &
SMESH_Algo::GetUsedHypothesis(SMESH_Mesh &         aMesh,
                              const TopoDS_Shape & aShape,
                              const bool           ignoreAuxiliary)
{
  _usedHypList.clear();
  SMESH_HypoFilter filter;
  if ( InitCompatibleHypoFilter( filter, ignoreAuxiliary ))
  {
    aMesh.GetHypotheses( aShape, filter, _usedHypList, true );
    if ( ignoreAuxiliary && _usedHypList.size() > 1 )
      _usedHypList.clear(); //only one compatible hypothesis allowed
  }
  return _usedHypList;
}

//=============================================================================
/*!
 *  List the relevant hypothesis associated to the shape. Relevant hypothesis
 *  have a name (type) listed in the algorithm. Hypothesis associated to
 *  father shape -are not- taken into account (see GetUsedHypothesis)
 */
//=============================================================================

const list<const SMESHDS_Hypothesis *> &
SMESH_Algo::GetAppliedHypothesis(SMESH_Mesh &         aMesh,
                                 const TopoDS_Shape & aShape,
                                 const bool           ignoreAuxiliary)
{
  _appliedHypList.clear();
  SMESH_HypoFilter filter;
  if ( InitCompatibleHypoFilter( filter, ignoreAuxiliary ))
    aMesh.GetHypotheses( aShape, filter, _appliedHypList, false );

  return _appliedHypList;
}

//=============================================================================
/*!
 *  Compute length of an edge
 */
//=============================================================================

double SMESH_Algo::EdgeLength(const TopoDS_Edge & E)
{
  double UMin = 0, UMax = 0;
  if (BRep_Tool::Degenerated(E))
    return 0;
  TopLoc_Location L;
  Handle(Geom_Curve) C = BRep_Tool::Curve(E, L, UMin, UMax);
  GeomAdaptor_Curve AdaptCurve(C, UMin, UMax); //range is important for periodic curves
  double length = GCPnts_AbscissaPoint::Length(AdaptCurve, UMin, UMax);
  return length;
}

//================================================================================
/*!
 * \brief Calculate normal of a mesh face
 */
//================================================================================

bool SMESH_Algo::FaceNormal(const SMDS_MeshElement* F, gp_XYZ& normal, bool normalized)
{
  if ( !F || F->GetType() != SMDSAbs_Face )
    return false;

  normal.SetCoord(0,0,0);
  int nbNodes = F->IsQuadratic() ? F->NbNodes()/2 : F->NbNodes();
  for ( int i = 0; i < nbNodes-2; ++i )
  {
    gp_XYZ p[3];
    for ( int n = 0; n < 3; ++n )
    {
      const SMDS_MeshNode* node = F->GetNode( i + n );
      p[n].SetCoord( node->X(), node->Y(), node->Z() );
    }
    normal += ( p[2] - p[1] ) ^ ( p[0] - p[1] );
  }
  double size2 = normal.SquareModulus();
  bool ok = ( size2 > numeric_limits<double>::min() * numeric_limits<double>::min());
  if ( normalized && ok )
    normal /= sqrt( size2 );

  return ok;
}

//================================================================================
/*!
 * \brief Find out elements orientation on a geometrical face
 * \param theFace - The face correctly oriented in the shape being meshed
 * \param theMeshDS - The mesh data structure
 * \retval bool - true if the face normal and the normal of first element
 *                in the correspoding submesh point in different directions
 */
//================================================================================

bool SMESH_Algo::IsReversedSubMesh (const TopoDS_Face&  theFace,
                                    SMESHDS_Mesh*       theMeshDS)
{
  if ( theFace.IsNull() || !theMeshDS )
    return false;

  // find out orientation of a meshed face
  int faceID = theMeshDS->ShapeToIndex( theFace );
  TopoDS_Shape aMeshedFace = theMeshDS->IndexToShape( faceID );
  bool isReversed = ( theFace.Orientation() != aMeshedFace.Orientation() );

  const SMESHDS_SubMesh * aSubMeshDSFace = theMeshDS->MeshElements( faceID );
  if ( !aSubMeshDSFace )
    return isReversed;

  // find element with node located on face and get its normal
  const SMDS_FacePosition* facePos = 0;
  int vertexID = 0;
  gp_Pnt nPnt[3];
  gp_Vec Ne;
  bool normalOK = false;
  SMDS_ElemIteratorPtr iteratorElem = aSubMeshDSFace->GetElements();
  while ( iteratorElem->more() ) // loop on elements on theFace
  {
    const SMDS_MeshElement* elem = iteratorElem->next();
    if ( elem && elem->NbNodes() > 2 ) {
      SMDS_ElemIteratorPtr nodesIt = elem->nodesIterator();
      const SMDS_FacePosition* fPos = 0;
      int i = 0, vID = 0;
      while ( nodesIt->more() ) { // loop on nodes
        const SMDS_MeshNode* node
          = static_cast<const SMDS_MeshNode *>(nodesIt->next());
        if ( i == 3 ) i = 2;
        nPnt[ i++ ].SetCoord( node->X(), node->Y(), node->Z() );
        // check position
        const SMDS_PositionPtr& pos = node->GetPosition();
        if ( !pos ) continue;
        if ( pos->GetTypeOfPosition() == SMDS_TOP_FACE ) {
          fPos = dynamic_cast< const SMDS_FacePosition* >( pos );
        }
        else if ( pos->GetTypeOfPosition() == SMDS_TOP_VERTEX ) {
          vID = node->getshapeId();
        }
      }
      if ( fPos || ( !normalOK && vID )) {
        // compute normal
        gp_Vec v01( nPnt[0], nPnt[1] ), v02( nPnt[0], nPnt[2] );
        if ( v01.SquareMagnitude() > RealSmall() &&
             v02.SquareMagnitude() > RealSmall() )
        {
          Ne = v01 ^ v02;
          normalOK = ( Ne.SquareMagnitude() > RealSmall() );
        }
        // we need position on theFace or at least on vertex
        if ( normalOK ) {
          vertexID = vID;
          if ((facePos = fPos))
            break;
        }
      }
    }
  }
  if ( !normalOK )
    return isReversed;

  // node position on face
  double u,v;
  if ( facePos ) {
    u = facePos->GetUParameter();
    v = facePos->GetVParameter();
  }
  else if ( vertexID ) {
    TopoDS_Shape V = theMeshDS->IndexToShape( vertexID );
    if ( V.IsNull() || V.ShapeType() != TopAbs_VERTEX )
      return isReversed;
    gp_Pnt2d uv = BRep_Tool::Parameters( TopoDS::Vertex( V ), theFace );
    u = uv.X();
    v = uv.Y();
  }
  else
  {
    return isReversed;
  }

  // face normal at node position
  TopLoc_Location loc;
  Handle(Geom_Surface) surf = BRep_Tool::Surface( theFace, loc );
  if ( surf.IsNull() || surf->Continuity() < GeomAbs_C1 ) return isReversed;
  gp_Vec d1u, d1v;
  surf->D1( u, v, nPnt[0], d1u, d1v );
  gp_Vec Nf = (d1u ^ d1v).Transformed( loc );

  if ( theFace.Orientation() == TopAbs_REVERSED )
    Nf.Reverse();

  return Ne * Nf < 0.;
}

//================================================================================
/*!
 * \brief Just return false as the algorithm does not hold parameters values
 */
//================================================================================

bool SMESH_Algo::SetParametersByMesh(const SMESH_Mesh* /*theMesh*/,
                                     const TopoDS_Shape& /*theShape*/)
{
  return false;
}
bool SMESH_Algo::SetParametersByDefaults(const TDefaults& , const SMESH_Mesh*)
{
  return false;
}
//================================================================================
/*!
 * \brief Fill vector of node parameters on geometrical edge, including vertex nodes
 * \param theMesh - The mesh containing nodes
 * \param theEdge - The geometrical edge of interest
 * \param theParams - The resulting vector of sorted node parameters
 * \retval bool - false if not all parameters are OK
 */
//================================================================================

bool SMESH_Algo::GetNodeParamOnEdge(const SMESHDS_Mesh* theMesh,
                                    const TopoDS_Edge&  theEdge,
                                    vector< double > &  theParams)
{
  theParams.clear();

  if ( !theMesh || theEdge.IsNull() )
    return false;

  SMESHDS_SubMesh * eSubMesh = theMesh->MeshElements( theEdge );
  if ( !eSubMesh || !eSubMesh->GetElements()->more() )
    return false; // edge is not meshed

  //int nbEdgeNodes = 0;
  set < double > paramSet;
  if ( eSubMesh )
  {
    // loop on nodes of an edge: sort them by param on edge
    SMDS_NodeIteratorPtr nIt = eSubMesh->GetNodes();
    while ( nIt->more() )
    {
      const SMDS_MeshNode* node = nIt->next();
      const SMDS_PositionPtr& pos = node->GetPosition();
      if ( pos->GetTypeOfPosition() != SMDS_TOP_EDGE )
        return false;
      const SMDS_EdgePosition* epos =
        static_cast<const SMDS_EdgePosition*>(node->GetPosition());
      if ( !paramSet.insert( epos->GetUParameter() ).second )
        return false; // equal parameters
    }
  }
  // add vertex nodes params
  TopoDS_Vertex V1,V2;
  TopExp::Vertices( theEdge, V1, V2);
  if ( VertexNode( V1, theMesh ) &&
       !paramSet.insert( BRep_Tool::Parameter(V1,theEdge) ).second )
    return false; // there are equal parameters
  if ( VertexNode( V2, theMesh ) &&
       !paramSet.insert( BRep_Tool::Parameter(V2,theEdge) ).second )
    return false; // there are equal parameters

  // fill the vector
  theParams.resize( paramSet.size() );
  set < double >::iterator   par    = paramSet.begin();
  vector< double >::iterator vecPar = theParams.begin();
  for ( ; par != paramSet.end(); ++par, ++vecPar )
    *vecPar = *par;

  return theParams.size() > 1;
}

//================================================================================
/*!
 * \brief Fill vector of node parameters on geometrical edge, including vertex nodes
 * \param theMesh - The mesh containing nodes
 * \param theEdge - The geometrical edge of interest
 * \param theParams - The resulting vector of sorted node parameters
 * \retval bool - false if not all parameters are OK
 */
//================================================================================

bool SMESH_Algo::GetSortedNodesOnEdge(const SMESHDS_Mesh*                   theMesh,
                                      const TopoDS_Edge&                    theEdge,
                                      const bool                            ignoreMediumNodes,
                                      map< double, const SMDS_MeshNode* > & theNodes)
{
  theNodes.clear();

  if ( !theMesh || theEdge.IsNull() )
    return false;

  SMESHDS_SubMesh * eSubMesh = theMesh->MeshElements( theEdge );
  if ( !eSubMesh || !eSubMesh->GetElements()->more() )
    return false; // edge is not meshed

  int nbNodes = 0;
  set < double > paramSet;
  if ( eSubMesh )
  {
    // loop on nodes of an edge: sort them by param on edge
    SMDS_NodeIteratorPtr nIt = eSubMesh->GetNodes();
    while ( nIt->more() )
    {
      const SMDS_MeshNode* node = nIt->next();
      if ( ignoreMediumNodes ) {
        SMDS_ElemIteratorPtr elemIt = node->GetInverseElementIterator();
        if ( elemIt->more() && elemIt->next()->IsMediumNode( node ))
          continue;
      }
      const SMDS_PositionPtr& pos = node->GetPosition();
      if ( pos->GetTypeOfPosition() != SMDS_TOP_EDGE )
        return false;
      const SMDS_EdgePosition* epos =
        static_cast<const SMDS_EdgePosition*>(node->GetPosition());
      theNodes.insert( theNodes.end(), make_pair( epos->GetUParameter(), node ));
      //MESSAGE("U " << epos->GetUParameter() << " ID " << node->GetID());
      ++nbNodes;
    }
  }
  // add vertex nodes
  TopoDS_Vertex v1, v2;
  TopExp::Vertices(theEdge, v1, v2);
  const SMDS_MeshNode* n1 = VertexNode( v1, (SMESHDS_Mesh*) theMesh );
  const SMDS_MeshNode* n2 = VertexNode( v2, (SMESHDS_Mesh*) theMesh );
  //MESSAGE("Vertices ID " << n1->GetID() << " " << n2->GetID());
  Standard_Real f, l;
  BRep_Tool::Range(theEdge, f, l);
  if ( v1.Orientation() != TopAbs_FORWARD )
    std::swap( f, l );
  if ( n1 && ++nbNodes )
    theNodes.insert( make_pair( f, n1 ));
  if ( n2 && ++nbNodes )
    theNodes.insert( make_pair( l, n2 ));

  return theNodes.size() == nbNodes;
}

//================================================================================
/*!
 * \brief Make filter recognize only compatible hypotheses
 * \param theFilter - the filter to initialize
 * \param ignoreAuxiliary - make filter ignore compatible auxiliary hypotheses
 */
//================================================================================

bool SMESH_Algo::InitCompatibleHypoFilter( SMESH_HypoFilter & theFilter,
                                           const bool         ignoreAuxiliary) const
{
  if ( !_compatibleHypothesis.empty() )
  {
    theFilter.Init( theFilter.HasName( _compatibleHypothesis[0] ));
    for ( int i = 1; i < _compatibleHypothesis.size(); ++i )
      theFilter.Or( theFilter.HasName( _compatibleHypothesis[ i ] ));

    if ( ignoreAuxiliary )
      theFilter.AndNot( theFilter.IsAuxiliary() );

    return true;
  }
  return false;
}

//================================================================================
/*!
 * \brief Return continuity of two edges
 * \param E1 - the 1st edge
 * \param E2 - the 2nd edge
 * \retval GeomAbs_Shape - regularity at the junction between E1 and E2
 */
//================================================================================

GeomAbs_Shape SMESH_Algo::Continuity(TopoDS_Edge E1,
                                     TopoDS_Edge E2)
{
  //E1.Orientation(TopAbs_FORWARD), E2.Orientation(TopAbs_FORWARD); // avoid pb with internal edges
  if (E1.Orientation() > TopAbs_REVERSED) // INTERNAL
    E1.Orientation( TopAbs_FORWARD );
  if (E2.Orientation() > TopAbs_REVERSED) // INTERNAL
    E2.Orientation( TopAbs_FORWARD );

  TopoDS_Vertex V, VV1[2], VV2[2];
  TopExp::Vertices( E1, VV1[0], VV1[1], true );
  TopExp::Vertices( E2, VV2[0], VV2[1], true );
  if      ( VV1[1].IsSame( VV2[0] ))  { V = VV1[1]; }
  else if ( VV1[0].IsSame( VV2[1] ))  { V = VV1[0]; }
  else if ( VV1[1].IsSame( VV2[1] ))  { V = VV1[1]; E1.Reverse(); }
  else if ( VV1[0].IsSame( VV2[0] ))  { V = VV1[0]; E1.Reverse(); }
  else { return GeomAbs_C0; }

  Standard_Real u1 = BRep_Tool::Parameter( V, E1 );
  Standard_Real u2 = BRep_Tool::Parameter( V, E2 );
  BRepAdaptor_Curve C1( E1 ), C2( E2 );
  Standard_Real tol = BRep_Tool::Tolerance( V );
  Standard_Real angTol = 2e-3;
  try {
#if OCC_VERSION_LARGE > 0x06010000
    OCC_CATCH_SIGNALS;
#endif
    return BRepLProp::Continuity(C1, C2, u1, u2, tol, angTol);
  }
  catch (Standard_Failure) {
  }
  return GeomAbs_C0;
}

//================================================================================
/*!
 * \brief Return the node built on a vertex
 * \param V - the vertex
 * \param meshDS - mesh
 * \retval const SMDS_MeshNode* - found node or NULL
 */
//================================================================================

const SMDS_MeshNode* SMESH_Algo::VertexNode(const TopoDS_Vertex& V,
                                            const SMESHDS_Mesh*  meshDS)
{
  if ( SMESHDS_SubMesh* sm = meshDS->MeshElements(V) ) {
    SMDS_NodeIteratorPtr nIt= sm->GetNodes();
    if (nIt->more())
      return nIt->next();
  }
  return 0;
}

//=======================================================================
//function : GetCommonNodes
//purpose  : Return nodes common to two elements
//=======================================================================

vector< const SMDS_MeshNode*> SMESH_Algo::GetCommonNodes(const SMDS_MeshElement* e1,
                                                         const SMDS_MeshElement* e2)
{
  vector< const SMDS_MeshNode*> common;
  for ( int i = 0 ; i < e1->NbNodes(); ++i )
    if ( e2->GetNodeIndex( e1->GetNode( i )) >= 0 )
      common.push_back( e1->GetNode( i ));
  return common;
}

//=======================================================================
//function : GetMeshError
//purpose  : Finds topological errors of a sub-mesh
//WARNING  : 1D check is NOT implemented so far
//=======================================================================

SMESH_Algo::EMeshError SMESH_Algo::GetMeshError(SMESH_subMesh* subMesh)
{
  EMeshError err = MEr_OK;

  SMESHDS_SubMesh* smDS = subMesh->GetSubMeshDS();
  if ( !smDS )
    return MEr_EMPTY;

  switch ( subMesh->GetSubShape().ShapeType() )
  {
  case TopAbs_FACE: { // ====================== 2D =====================

    SMDS_ElemIteratorPtr fIt = smDS->GetElements();
    if ( !fIt->more() )
      return MEr_EMPTY;

    // We check that olny links on EDGEs encouter once, the rest links, twice
    set< SMESH_TLink > links;
    while ( fIt->more() )
    {
      const SMDS_MeshElement* f = fIt->next();
      int nbNodes = f->NbCornerNodes(); // ignore medium nodes
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* n1 = f->GetNode( i );
        const SMDS_MeshNode* n2 = f->GetNode(( i+1 ) % nbNodes);
        std::pair< set< SMESH_TLink >::iterator, bool > it_added =
          links.insert( SMESH_TLink( n1, n2 ));
        if ( !it_added.second )
          // As we do NOT(!) check if mesh is manifold, we believe that a link can
          // encounter once or twice only (not three times), we erase a link as soon
          // as it encounters twice to speed up search in the <links> map.
          links.erase( it_added.first );
      }
    }
    // the links remaining in the <links> should all be on EDGE
    set< SMESH_TLink >::iterator linkIt = links.begin();
    for ( ; linkIt != links.end(); ++linkIt )
    {
      const SMESH_TLink& link = *linkIt;
      if ( link.node1()->GetPosition()->GetTypeOfPosition() > SMDS_TOP_EDGE ||
           link.node2()->GetPosition()->GetTypeOfPosition() > SMDS_TOP_EDGE )
        return MEr_HOLES;
    }
    // TODO: to check orientation
    break;
  }
  case TopAbs_SOLID: { // ====================== 3D =====================

    SMDS_ElemIteratorPtr vIt = smDS->GetElements();
    if ( !vIt->more() )
      return MEr_EMPTY;

    SMDS_VolumeTool vTool;
    while ( !vIt->more() )
    {
      if (!vTool.Set( vIt->next() ))
        continue; // strange

      for ( int iF = 0; iF < vTool.NbFaces(); ++iF )
        if ( vTool.IsFreeFace( iF ))
        {
          int nbN = vTool.NbFaceNodes( iF );
          const SMDS_MeshNode** nodes =  vTool.GetFaceNodes( iF );
          for ( int i = 0; i < nbN; ++i )
            if ( nodes[i]->GetPosition()->GetTypeOfPosition() > SMDS_TOP_FACE )
              return MEr_HOLES;
        }
    }
    break;
  }
  default:;
  }
  return err;
}

//================================================================================
/*!
 * \brief Sets event listener to submeshes if necessary
 * \param subMesh - submesh where algo is set
 * 
 * After being set, event listener is notified on each event of a submesh.
 * By default non listener is set
 */
//================================================================================

void SMESH_Algo::SetEventListener(SMESH_subMesh* /*subMesh*/)
{
}

//================================================================================
/*!
 * \brief Allow algo to do something after persistent restoration
 * \param subMesh - restored submesh
 *
 * This method is called only if a submesh has HYP_OK algo_state.
 */
//================================================================================

void SMESH_Algo::SubmeshRestored(SMESH_subMesh* /*subMesh*/)
{
}

//================================================================================
/*!
 * \brief Computes mesh without geometry
 * \param aMesh - the mesh
 * \param aHelper - helper that must be used for adding elements to \aaMesh
 * \retval bool - is a success
 */
//================================================================================

bool SMESH_Algo::Compute(SMESH_Mesh & /*aMesh*/, SMESH_MesherHelper* /*aHelper*/)
{
  return error( COMPERR_BAD_INPUT_MESH, "Mesh built on shape expected");
}

//=======================================================================
//function : CancelCompute
//purpose  : Sets _computeCanceled to true. It's usage depends on
//  *        implementation of a particular mesher.
//=======================================================================

void SMESH_Algo::CancelCompute()
{
  _computeCanceled = true;
  _error = COMPERR_CANCELED;
}

//================================================================================
/*!
 * \brief store error and comment and then return ( error == COMPERR_OK )
 */
//================================================================================

bool SMESH_Algo::error(int error, const SMESH_Comment& comment)
{
  _error   = error;
  _comment = comment;
  return ( error == COMPERR_OK );
}

//================================================================================
/*!
 * \brief store error and return ( error == COMPERR_OK )
 */
//================================================================================

bool SMESH_Algo::error(SMESH_ComputeErrorPtr error)
{
  if ( error ) {
    _error   = error->myName;
    _comment = error->myComment;
    _badInputElements = error->myBadElements;
    return error->IsOK();
  }
  return true;
}

//================================================================================
/*!
 * \brief return compute error
 */
//================================================================================

SMESH_ComputeErrorPtr SMESH_Algo::GetComputeError() const
{
  SMESH_ComputeErrorPtr err = SMESH_ComputeError::New( _error, _comment, this );
  // hope this method is called by only SMESH_subMesh after this->Compute()
  err->myBadElements.splice( err->myBadElements.end(),
                             (list<const SMDS_MeshElement*>&) _badInputElements );
  return err;
}

//================================================================================
/*!
 * \brief initialize compute error
 */
//================================================================================

void SMESH_Algo::InitComputeError()
{
  _error = COMPERR_OK;
  _comment.clear();
  list<const SMDS_MeshElement*>::iterator elem = _badInputElements.begin();
  for ( ; elem != _badInputElements.end(); ++elem )
    if ( (*elem)->GetID() < 1 )
      delete *elem;
  _badInputElements.clear();

  _computeCanceled = false;
}

//================================================================================
/*!
 * \brief store a bad input element preventing computation,
 *        which may be a temporary one i.e. not residing the mesh,
 *        then it will be deleted by InitComputeError()
 */
//================================================================================

void SMESH_Algo::addBadInputElement(const SMDS_MeshElement* elem)
{
  if ( elem )
    _badInputElements.push_back( elem );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESH_Algo::NumberOfWires(const TopoDS_Shape& S)
{
  int i = 0;
  for (TopExp_Explorer exp(S,TopAbs_WIRE); exp.More(); exp.Next())
    i++;
  return i;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESH_Algo::NumberOfPoints(SMESH_Mesh& aMesh, const TopoDS_Wire& W)
{
  int nbPoints = 0;
  for (TopExp_Explorer exp(W,TopAbs_EDGE); exp.More(); exp.Next()) {
    const TopoDS_Edge& E = TopoDS::Edge(exp.Current());
    int nb = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
    if(_quadraticMesh)
      nb = nb/2;
    nbPoints += nb + 1; // internal points plus 1 vertex of 2 (last point ?)
  }
  return nbPoints;
}


