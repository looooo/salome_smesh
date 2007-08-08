//  SMESH SMESH : implementaion of SMESH idl descriptions
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
// File      : StdMeshers_Projection_3D.cxx
// Module    : SMESH
// Created   : Fri Oct 20 11:37:07 2006
// Author    : Edward AGAPOV (eap)


#include "StdMeshers_Projection_3D.hxx"
#include "StdMeshers_ProjectionSource3D.hxx"

#include "StdMeshers_ProjectionUtils.hxx"

#include "SMESHDS_Hypothesis.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMESH_Block.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_Pattern.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_Comment.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMDS_PolyhedralVolumeOfNodes.hxx"

#include "utilities.h"

#define RETURN_BAD_RESULT(msg) { MESSAGE(msg); return false; }
#define gpXYZ(n) gp_XYZ(n->X(),n->Y(),n->Z())
#define SHOWYXZ(msg, xyz) // {\
// gp_Pnt p (xyz); \
// cout << msg << " ("<< p.X() << "; " <<p.Y() << "; " <<p.Z() << ") " <<endl;\
// }

typedef StdMeshers_ProjectionUtils TAssocTool;


//=======================================================================
//function : StdMeshers_Projection_3D
//purpose  : 
//=======================================================================

StdMeshers_Projection_3D::StdMeshers_Projection_3D(int hypId, int studyId, SMESH_Gen* gen)
  :SMESH_3D_Algo(hypId, studyId, gen)
{
  _name = "Projection_3D";
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);	// 1 bit per shape type

  _compatibleHypothesis.push_back("ProjectionSource3D");
  _sourceHypo = 0;
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshers_Projection_3D::~StdMeshers_Projection_3D()
{}

//=======================================================================
//function : CheckHypothesis
//purpose  : 
//=======================================================================

bool StdMeshers_Projection_3D::CheckHypothesis(SMESH_Mesh&                          aMesh,
                                               const TopoDS_Shape&                  aShape,
                                               SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  // check aShape that must be a 6 faces block
/*  PAL16229
  if ( TAssocTool::Count( aShape, TopAbs_SHELL, 1 ) != 1 ||
       TAssocTool::Count( aShape, TopAbs_FACE , 1 ) != 6 ||
       TAssocTool::Count( aShape, TopAbs_EDGE , 1 ) != 12 ||
       TAssocTool::Count( aShape, TopAbs_WIRE , 1 ) != 6 )
  {
    aStatus = HYP_BAD_GEOMETRY;
    return false;
  }
*/
  list <const SMESHDS_Hypothesis * >::const_iterator itl;

  const list <const SMESHDS_Hypothesis * >&hyps = GetUsedHypothesis(aMesh, aShape);
  if ( hyps.size() == 0 )
  {
    aStatus = SMESH_Hypothesis::HYP_MISSING;
    return false;  // can't work with no hypothesis
  }

  if ( hyps.size() > 1 )
  {
    aStatus = SMESH_Hypothesis::HYP_ALREADY_EXIST;
    return false;
  }

  const SMESHDS_Hypothesis *theHyp = hyps.front();

  string hypName = theHyp->GetName();

  aStatus = SMESH_Hypothesis::HYP_OK;

  if (hypName == "ProjectionSource3D")
  {
    _sourceHypo = static_cast<const StdMeshers_ProjectionSource3D *>(theHyp);
    // Check hypo parameters

    SMESH_Mesh* srcMesh = _sourceHypo->GetSourceMesh();
    SMESH_Mesh* tgtMesh = & aMesh;
    if ( !srcMesh )
      srcMesh = tgtMesh;

    // check vertices
    if ( _sourceHypo->HasVertexAssociation() )
    {
      // source vertices
      TopoDS_Shape edge = TAssocTool::GetEdgeByVertices
        ( srcMesh, _sourceHypo->GetSourceVertex(1), _sourceHypo->GetSourceVertex(2) );
      if ( edge.IsNull() ||
           !TAssocTool::IsSubShape( edge, srcMesh ) ||
           !TAssocTool::IsSubShape( edge, _sourceHypo->GetSource3DShape() ))
      {
        SCRUTE((edge.IsNull()));
        SCRUTE((TAssocTool::IsSubShape( edge, srcMesh )));
        SCRUTE((TAssocTool::IsSubShape( edge, _sourceHypo->GetSource3DShape() )));
        aStatus = SMESH_Hypothesis::HYP_BAD_PARAMETER;
      }
      else
      {
        // target vertices
        edge = TAssocTool::GetEdgeByVertices
          ( tgtMesh, _sourceHypo->GetTargetVertex(1), _sourceHypo->GetTargetVertex(2) );
        if ( edge.IsNull() ||
             !TAssocTool::IsSubShape( edge, tgtMesh ) ||
             !TAssocTool::IsSubShape( edge, aShape ))
        {
          SCRUTE((edge.IsNull()));
          SCRUTE((TAssocTool::IsSubShape( edge, tgtMesh )));
          SCRUTE((TAssocTool::IsSubShape( edge, aShape )));
          aStatus = SMESH_Hypothesis::HYP_BAD_PARAMETER;
        }
      }
    }
    // check a source shape
    if ( !TAssocTool::IsSubShape( _sourceHypo->GetSource3DShape(), srcMesh ) ||
         ( srcMesh == tgtMesh && aShape == _sourceHypo->GetSource3DShape()))
    {
      SCRUTE((TAssocTool::IsSubShape( _sourceHypo->GetSource3DShape(), srcMesh)));
      SCRUTE((srcMesh == tgtMesh));
      SCRUTE((aShape == _sourceHypo->GetSource3DShape()));
      aStatus = SMESH_Hypothesis::HYP_BAD_PARAMETER;
    }
  }
  else
  {
    aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;
  }
  return ( aStatus == HYP_OK );
}

//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================

bool StdMeshers_Projection_3D::Compute(SMESH_Mesh& aMesh, const TopoDS_Shape& aShape)
{
  if ( !_sourceHypo )
    return false;

  SMESH_Mesh * srcMesh = _sourceHypo->GetSourceMesh();
  SMESH_Mesh * tgtMesh = & aMesh;
  if ( !srcMesh )
    srcMesh = tgtMesh;

  SMESHDS_Mesh * srcMeshDS = srcMesh->GetMeshDS();
  SMESHDS_Mesh * tgtMeshDS = tgtMesh->GetMeshDS();

  // get shell from shape3D
  TopoDS_Shell srcShell, tgtShell;
  TopExp_Explorer exp( _sourceHypo->GetSource3DShape(), TopAbs_SHELL );
  int nbShell;
  for ( nbShell = 0; exp.More(); exp.Next(), ++nbShell )
    srcShell = TopoDS::Shell( exp.Current() );
  if ( nbShell != 1 )
    return error(COMPERR_BAD_SHAPE,
                 SMESH_Comment("Source shape must have 1 shell but not ") << nbShell);

  exp.Init( aShape, TopAbs_SHELL );
  for ( nbShell = 0; exp.More(); exp.Next(), ++nbShell )
    tgtShell = TopoDS::Shell( exp.Current() );
  if ( nbShell != 1 )
    return error(COMPERR_BAD_SHAPE,
                 SMESH_Comment("Target shape must have 1 shell but not ") << nbShell);

  // Check that shapes are blocks
  if ( TAssocTool::Count( tgtShell, TopAbs_FACE , 1 ) != 6 ||
       TAssocTool::Count( tgtShell, TopAbs_EDGE , 1 ) != 12 ||
       TAssocTool::Count( tgtShell, TopAbs_WIRE , 1 ) != 6 )
    return error(COMPERR_BAD_SHAPE, "Target shape is not a block");
  if ( TAssocTool::Count( srcShell, TopAbs_FACE , 1 ) != 6 ||
       TAssocTool::Count( srcShell, TopAbs_EDGE , 1 ) != 12 ||
       TAssocTool::Count( srcShell, TopAbs_WIRE , 1 ) != 6 )
    return error(COMPERR_BAD_SHAPE, "Source shape is not a block");

  // Assure that mesh on a source shape is computed

  SMESH_subMesh* srcSubMesh = srcMesh->GetSubMesh( _sourceHypo->GetSource3DShape() );
  //SMESH_subMesh* tgtSubMesh = tgtMesh->GetSubMesh( aShape );

  if ( tgtMesh == srcMesh && !aShape.IsSame( _sourceHypo->GetSource3DShape() )) {
    if ( !TAssocTool::MakeComputed( srcSubMesh ))
      return error(COMPERR_BAD_INPUT_MESH,"Source mesh not computed");
  }
  else {
    if ( !srcSubMesh->IsMeshComputed() )
      return error(COMPERR_BAD_INPUT_MESH,"Source mesh not computed");
  }

  // Find 2 pairs of corresponding vertices

  TopoDS_Vertex tgtV000, tgtV100, srcV000, srcV100;
  TAssocTool::TShapeShapeMap shape2ShapeMap;

  if ( _sourceHypo->HasVertexAssociation() )
  {
    tgtV000 = _sourceHypo->GetTargetVertex(1);
    tgtV100 = _sourceHypo->GetTargetVertex(2);
    srcV000 = _sourceHypo->GetSourceVertex(1);
    srcV100 = _sourceHypo->GetSourceVertex(2);
  }
  else
  {
    if ( !TAssocTool::FindSubShapeAssociation( tgtShell, tgtMesh, srcShell, srcMesh,
                                               shape2ShapeMap) )
      return error(COMPERR_BAD_SHAPE,"Topology of source and target shapes seems different" );

    exp.Init( tgtShell, TopAbs_EDGE );
    TopExp::Vertices( TopoDS::Edge( exp.Current() ), tgtV000, tgtV100 );

    if ( !shape2ShapeMap.IsBound( tgtV000 ) || !shape2ShapeMap.IsBound( tgtV100 ))
      return error("Association of subshapes failed" );
    srcV000 = TopoDS::Vertex( shape2ShapeMap( tgtV000 ));
    srcV100 = TopoDS::Vertex( shape2ShapeMap( tgtV100 ));
    if ( !TAssocTool::IsSubShape( srcV000, srcShell ) ||
         !TAssocTool::IsSubShape( srcV100, srcShell ))
      return error("Incorrect association of subshapes" );
  }

  // Load 2 SMESH_Block's with src and tgt shells

  SMESH_Block srcBlock, tgtBlock;
  TopTools_IndexedMapOfOrientedShape scrShapes, tgtShapes;
  if ( !tgtBlock.LoadBlockShapes( tgtShell, tgtV000, tgtV100, tgtShapes ))
    return error(COMPERR_BAD_SHAPE, "Can't detect block subshapes. Not a block?");

  if ( !srcBlock.LoadBlockShapes( srcShell, srcV000, srcV100, scrShapes ))
    return error(COMPERR_BAD_SHAPE, "Can't detect block subshapes. Not a block?");

  // Find matching nodes of src and tgt shells

  TNodeNodeMap src2tgtNodeMap;
  for ( int fId = SMESH_Block::ID_FirstF; fId < SMESH_Block::ID_Shell; ++fId )
  {
    // Corresponding subshapes
    TopoDS_Face srcFace = TopoDS::Face( scrShapes( fId ));
    TopoDS_Face tgtFace = TopoDS::Face( tgtShapes( fId ));
    if ( _sourceHypo->HasVertexAssociation() ) { // associate face subshapes
      shape2ShapeMap.Clear();
      vector< int > edgeIdVec;
      SMESH_Block::GetFaceEdgesIDs( fId, edgeIdVec );
      for ( int i = 0; i < edgeIdVec.size(); ++i ) {
        int eID = edgeIdVec[ i ];
        shape2ShapeMap.Bind( tgtShapes( eID ), scrShapes( eID ));
        if ( i < 2 ) {
          vector< int > vertexIdVec;
          SMESH_Block::GetEdgeVertexIDs( eID, vertexIdVec );
          shape2ShapeMap.Bind( tgtShapes( vertexIdVec[0] ), scrShapes( vertexIdVec[0] ));
          shape2ShapeMap.Bind( tgtShapes( vertexIdVec[1] ), scrShapes( vertexIdVec[1] ));
        }
      }
    }
    // Find matching nodes of tgt and src faces
    TNodeNodeMap faceMatchingNodes;
    if ( ! TAssocTool::FindMatchingNodesOnFaces( srcFace, srcMesh, tgtFace, tgtMesh, 
                                                 shape2ShapeMap, faceMatchingNodes ))
    return error(COMPERR_BAD_INPUT_MESH,SMESH_Comment("Mesh on faces #")
                 << srcMeshDS->ShapeToIndex( srcFace ) << " and "
                 << tgtMeshDS->ShapeToIndex( tgtFace ) << " seems different" );

    // put found matching nodes of 2 faces to the global map
    src2tgtNodeMap.insert( faceMatchingNodes.begin(), faceMatchingNodes.end() );
  }

  // ------------------
  // Make mesh
  // ------------------

  SMDS_VolumeTool volTool;
  SMESH_MesherHelper helper( *tgtMesh );
  helper.IsQuadraticSubMesh( aShape );

  SMESHDS_SubMesh* srcSMDS = srcSubMesh->GetSubMeshDS();
  SMDS_ElemIteratorPtr volIt = srcSMDS->GetElements();
  while ( volIt->more() ) // loop on source volumes
  {
    const SMDS_MeshElement* srcVol = volIt->next();
    if ( !srcVol || srcVol->GetType() != SMDSAbs_Volume )
        continue;
    int nbNodes = srcVol->NbNodes();
    SMDS_VolumeTool::VolumeType  volType = volTool.GetType( nbNodes );
    if ( srcVol->IsQuadratic() )
      nbNodes = volTool.NbCornerNodes( volType );

    // Find or create a new tgt node for each node of a src volume

    vector< const SMDS_MeshNode* > nodes( nbNodes );
    for ( int i = 0; i < nbNodes; ++i )
    {
      const SMDS_MeshNode* srcNode = srcVol->GetNode( i );
      const SMDS_MeshNode* tgtNode = 0;
      TNodeNodeMap::iterator sN_tN = src2tgtNodeMap.find( srcNode );
      if ( sN_tN != src2tgtNodeMap.end() ) // found
      {
        tgtNode = sN_tN->second;
      }
      else // Create a new tgt node
      {
        // compute normalized parameters of source node in srcBlock
        gp_Pnt srcCoord = gpXYZ( srcNode );
        gp_XYZ srcParam;
        if ( !srcBlock.ComputeParameters( srcCoord, srcParam ))
          return error(SMESH_Comment("Can't compute normalized parameters ")
                       << "for source node " << srcNode->GetID());
        // compute coordinates of target node by srcParam
        gp_XYZ tgtXYZ;
        if ( !tgtBlock.ShellPoint( srcParam, tgtXYZ ))
          return error("Can't compute coordinates by normalized parameters");
        // add node
        SMDS_MeshNode* newNode = tgtMeshDS->AddNode( tgtXYZ.X(), tgtXYZ.Y(), tgtXYZ.Z() );
        tgtMeshDS->SetNodeInVolume( newNode, helper.GetSubShapeID() );
        tgtNode = newNode;
        src2tgtNodeMap.insert( make_pair( srcNode, tgtNode ));
      }
      nodes[ i ] = tgtNode;
    }

    // Create a new volume

    SMDS_MeshVolume * tgtVol = 0;
    int id = 0, force3d = false;
    switch ( volType ) {
    case SMDS_VolumeTool::TETRA     :
    case SMDS_VolumeTool::QUAD_TETRA:
      tgtVol = helper.AddVolume( nodes[0],
                                 nodes[1],
                                 nodes[2],
                                 nodes[3], id, force3d); break;
    case SMDS_VolumeTool::PYRAM     :
    case SMDS_VolumeTool::QUAD_PYRAM:
      tgtVol = helper.AddVolume( nodes[0],
                                 nodes[1],
                                 nodes[2],
                                 nodes[3],
                                 nodes[4], id, force3d); break;
    case SMDS_VolumeTool::PENTA     :
    case SMDS_VolumeTool::QUAD_PENTA:
      tgtVol = helper.AddVolume( nodes[0],
                                 nodes[1],
                                 nodes[2],
                                 nodes[3],
                                 nodes[4],
                                 nodes[5], id, force3d); break;
    case SMDS_VolumeTool::HEXA      :
    case SMDS_VolumeTool::QUAD_HEXA :
      tgtVol = helper.AddVolume( nodes[0],
                                 nodes[1],
                                 nodes[2],
                                 nodes[3],
                                 nodes[4],
                                 nodes[5],
                                 nodes[6],
                                 nodes[7], id, force3d); break;
    default: // polyhedron
      const SMDS_PolyhedralVolumeOfNodes * poly =
        dynamic_cast<const SMDS_PolyhedralVolumeOfNodes*>( srcVol );
      if ( !poly )
        RETURN_BAD_RESULT("Unexpected volume type");
      tgtVol = tgtMeshDS->AddPolyhedralVolume( nodes, poly->GetQuanities() );
    }
    if ( tgtVol ) {
      tgtMeshDS->SetMeshElementOnShape( tgtVol, helper.GetSubShapeID() );
    }
  } // loop on volumes of src shell

  return true;
}

//=============================================================================
/*!
 * \brief Sets a default event listener to submesh of the source shape
  * \param subMesh - submesh where algo is set
 *
 * This method is called when a submesh gets HYP_OK algo_state.
 * After being set, event listener is notified on each event of a submesh.
 * Arranges that CLEAN event is translated from source submesh to
 * the submesh
 */
//=============================================================================

void StdMeshers_Projection_3D::SetEventListener(SMESH_subMesh* subMesh)
{
  TAssocTool::SetEventListener( subMesh,
                                _sourceHypo->GetSource3DShape(),
                                _sourceHypo->GetSourceMesh() );
}
  
