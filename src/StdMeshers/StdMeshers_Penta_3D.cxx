//  SMESH StdMeshers_Penta_3D implementaion of SMESH idl descriptions
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : StdMeshers_Penta_3D.cxx
//  Module : SMESH

#include "StdMeshers_Penta_3D.hxx"

#include "utilities.h"
#include "Utils_ExceptHandlers.hxx"

#include "SMDS_EdgePosition.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_VolumeOfNodes.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_MeshEditor.hxx"

#include <BRep_Tool.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Pnt.hxx>

#include <stdio.h>
#include <algorithm>

using namespace std;

typedef map < int, int, less<int> >::iterator   \
  StdMeshers_IteratorOfDataMapOfIntegerInteger;

//=======================================================================
//function : StdMeshers_Penta_3D
//purpose  : 
//=======================================================================
StdMeshers_Penta_3D::StdMeshers_Penta_3D()
: myErrorStatus(1)
{
  myTol3D=0.1;
  myWallNodesMaps.resize( SMESH_Block::NbFaces() );
  myShapeXYZ.resize( SMESH_Block::NbSubShapes() );
}
//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================
bool StdMeshers_Penta_3D::Compute(SMESH_Mesh& aMesh, 
				  const TopoDS_Shape& aShape)
{
  MESSAGE("StdMeshers_Penta_3D::Compute()");
  //
  myErrorStatus=0;
  //
  bool bOK=false;
  //
  myShape=aShape;
  SetMesh(aMesh);
  //
  CheckData();
  if (myErrorStatus){
    return bOK;
  }
  //
  MakeBlock();
    if (myErrorStatus){
    return bOK;
  }
  //
  MakeNodes();
  if (myErrorStatus){
    return bOK;
  }
  //
  MakeConnectingMap();
  //
  ClearMeshOnFxy1();
  if (myErrorStatus) {
    return bOK;
  }
  //
  MakeMeshOnFxy1();
  if (myErrorStatus) {
    return bOK;
  }
  //
  MakeVolumeMesh();
  //
  return !bOK;
}
//=======================================================================
//function : MakeNodes
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeNodes()
{
  myErrorStatus=0;
  //
  const int aNbSIDs=9;
  int i, j, k, ij, iNbN, aNodeID, aSize, iErr;
  double aX, aY, aZ;
  SMESH_Block::TShapeID aSID, aSIDs[aNbSIDs]={
    SMESH_Block::ID_V000, SMESH_Block::ID_V100, 
    SMESH_Block::ID_V110, SMESH_Block::ID_V010,
    SMESH_Block::ID_Ex00, SMESH_Block::ID_E1y0, 
    SMESH_Block::ID_Ex10, SMESH_Block::ID_E0y0,
    SMESH_Block::ID_Fxy0
  }; 
  //
  SMESH_Mesh* pMesh=GetMesh();
  //
  // 1. Define the sizes of mesh
  //
  // 1.1 Horizontal size
  myJSize=0;
  for (i=0; i<aNbSIDs; ++i) {
    const TopoDS_Shape& aS=myBlock.Shape(aSIDs[i]);
    SMESH_subMesh *aSubMesh = pMesh->GetSubMeshContaining(aS);
    ASSERT(aSubMesh);
    SMESHDS_SubMesh *aSM=aSubMesh->GetSubMeshDS();
    iNbN=aSM->NbNodes();
    myJSize+=iNbN;
  }
  //printf("***  Horizontal: number of nodes summary=%d\n", myJSize);
  //
  // 1.2 Vertical size
  myISize=2;
  {
    const TopoDS_Shape& aS=myBlock.Shape(SMESH_Block::ID_E00z);
    SMESH_subMesh *aSubMesh = pMesh->GetSubMeshContaining(aS);
    ASSERT(aSubMesh);
    SMESHDS_SubMesh *aSM=aSubMesh->GetSubMeshDS();
    iNbN=aSM->NbNodes();
    myISize+=iNbN;
  }
  //printf("***  Vertical: number of nodes on edges and vertices=%d\n", myISize);
  //
  aSize=myISize*myJSize;
  myTNodes.resize(aSize);
  //
  StdMeshers_TNode aTNode;
  gp_XYZ aCoords;
  gp_Pnt aP3D;
  //
  // 2. Fill the repers on base face (Z=0)
  i=0; j=0;
  // vertices
  for (k=0; k<aNbSIDs; ++k) {
    aSID=aSIDs[k];
    const TopoDS_Shape& aS=myBlock.Shape(aSID);
    SMDS_NodeIteratorPtr ite =pMesh->GetSubMeshContaining(aS)->GetSubMeshDS()->GetNodes();
    while(ite->more()) {
      const SMDS_MeshNode* aNode = ite->next();
      aNodeID=aNode->GetID();
      //
      aTNode.SetNode(aNode);
      aTNode.SetShapeSupportID(aSID);
      aTNode.SetBaseNodeID(aNodeID);
      //
      if ( SMESH_Block::IsEdgeID (aSID))
      {
        const SMDS_EdgePosition* epos =
          static_cast<const SMDS_EdgePosition*>(aNode->GetPosition().get());
        myBlock.ComputeParameters( epos->GetUParameter(), aS, aCoords );
      }
      else {
        aX=aNode->X();
        aY=aNode->Y();
        aZ=aNode->Z();
        aP3D.SetCoord(aX, aY, aZ);
        myBlock.ComputeParameters(aP3D, aS, aCoords);
      }
      iErr=myBlock.ErrorStatus();
      if (iErr) {
        MESSAGE("StdMeshers_Penta_3D::MakeNodes()," <<
                "SMESHBlock: ComputeParameters operation failed");
        myErrorStatus=101; // SMESHBlock: ComputeParameters operation failed
        return;
      }
      aTNode.SetNormCoord(aCoords);
      ij=i*myJSize+j;
      myTNodes[ij]=aTNode;
      ++j;
    }
  }
  /*
  //DEB
  {
    int iShapeSupportID, iBaseNodeID;
    //
    //printf("\n\n*** Base Face\n");
    i=0;
    for (j=0; j<myJSize; ++j) {
      ij=i*myJSize+j;
      const StdMeshers_TNode& aTNode=myTNodes[ij];
      iShapeSupportID=aTNode.ShapeSupportID();
      iBaseNodeID=aTNode.BaseNodeID();
      const gp_XYZ& aXYZ=aTNode.NormCoord();
      printf("*** j:%d bID#%d iSS:%d { %lf %lf %lf }\n",
	     j,  iBaseNodeID, iShapeSupportID, aXYZ.X(),  aXYZ.Y(), aXYZ.Z());
    }
  }
  */
  //DEB
  //return; //zz
  //
  // 3. Finding of Z-layers
//   vector<double> aZL(myISize);
//   vector<double>::iterator aItZL1, aItZL2 ;
//   //
//   const TopoDS_Shape& aE00z=myBlock.Shape(SMESH_Block::ID_E00z);
//   SMDS_NodeIteratorPtr aItaE00z =
//     pMesh->GetSubMeshContaining(aE00z)->GetSubMeshDS()->GetNodes();
//   //
//   aZL[0]=0.;
//   i=1;
//   while (aItaE00z->more()) {
//     const SMDS_MeshNode* aNode=aItaE00z->next();
//     const SMDS_EdgePosition* epos =
//       static_cast<const SMDS_EdgePosition*>(aNode->GetPosition().get());
//     myBlock.ComputeParameters( epos->GetUParameter(), aE00z, aCoords );
//     iErr=myBlock.ErrorStatus();
//     if (iErr) {
//       MESSAGE("StdMeshers_Penta_3D::MakeNodes()," <<
//               "SMESHBlock: ComputeParameters operation failed");
//       myErrorStatus=101; // SMESHBlock: ComputeParameters operation failed
//       return;
//     }
//     aZL[i]=aCoords.Z();
//     ++i;
//   }
//   aZL[i]=1.;
//   //
//   aItZL1=aZL.begin();
//   aItZL2=aZL.end();
//   //
//   // Sorting the layers
//   sort(aItZL1, aItZL2);
  //DEB
  /*
  printf("** \n\n Layers begin\n");
  for(i=0, aItZL=aItZL1; aItZL!=aItZL2; ++aItZL, ++i) {
    printf(" #%d : %lf\n", i, *aItZL);
  } 
  printf("** Layers end\n");
  */
  //DEB
  //
  //

  // 3.1 Fill maps of wall nodes
  SMESH_Block::TShapeID wallFaceID[4] = {
    SMESH_Block::ID_Fx0z, SMESH_Block::ID_Fx1z,
    SMESH_Block::ID_F0yz, SMESH_Block::ID_F1yz
    };
  SMESH_Block::TShapeID baseEdgeID[4] = {
    SMESH_Block::ID_Ex00, SMESH_Block::ID_Ex10,
    SMESH_Block::ID_E0y0, SMESH_Block::ID_E1y0
    };
  for ( i = 0; i < 4; ++i ) {
    int fIndex = SMESH_Block::ShapeIndex( wallFaceID[ i ]);
    bool ok = LoadIJNodes (myWallNodesMaps[ fIndex ],
                           TopoDS::Face( myBlock.Shape( wallFaceID[ i ] )),
                           TopoDS::Edge( myBlock.Shape( baseEdgeID[ i ] )),
                           pMesh->GetMeshDS());
    if ( !ok ) {
      myErrorStatus = i + 1;
      MESSAGE(" Cant LoadIJNodes() from a wall face " << myErrorStatus );
      return;
    }
  }

  // 3.2 find node columns for vertical edges and edge IDs
  vector<const SMDS_MeshNode*> * verticEdgeNodes[ 4 ];
  SMESH_Block::TShapeID          verticEdgeID   [ 4 ];
  for ( i = 0; i < 4; ++i ) { // 4 first base nodes are nodes on vertices
    // edge ID
    SMESH_Block::TShapeID eID, vID = aSIDs[ i ];
    ShapeSupportID(false, vID, eID);
    verticEdgeID[ i ] = eID;
    // column nodes
    StdMeshers_TNode& aTNode = myTNodes[ i ];
    verticEdgeNodes[ i ] = 0;
    for ( j = 0; j < 4; ++j ) { // loop on 4 wall faces
      int fIndex = SMESH_Block::ShapeIndex( wallFaceID[ j ]);
      StdMeshers_IJNodeMap & ijNodes= myWallNodesMaps[ fIndex ];
      if ( ijNodes.begin()->second[0] == aTNode.Node() )
        verticEdgeNodes[ i ] = & ijNodes.begin()->second;
      else if ( ijNodes.rbegin()->second[0] == aTNode.Node() )
        verticEdgeNodes[ i ] = & ijNodes.rbegin()->second;
      if ( verticEdgeNodes[ i ] )
        break;
    }
  }

  // 3.3 set XYZ of vertices, and initialize of the rest
  SMESHDS_Mesh* aMesh = GetMesh()->GetMeshDS();
  for ( int id = SMESH_Block::ID_V000; id < SMESH_Block::ID_Shell; ++id )
  {
    if ( SMESH_Block::IsVertexID( id )) {
      TopoDS_Shape V = myBlock.Shape( id );
      SMESHDS_SubMesh* sm = aMesh->MeshElements( V );
      const SMDS_MeshNode* n = sm->GetNodes()->next();
      myShapeXYZ[ id ].SetCoord( n->X(), n->Y(), n->Z() );
    }
    else
      myShapeXYZ[ id ].SetCoord( 0., 0., 0. );
  }


  // 4. Fill the rest repers
  bool bIsUpperLayer;
  int iBNID;
  SMESH_Block::TShapeID aSSID, aBNSSID;
  StdMeshers_TNode aTN;
  //
  for (j=0; j<myJSize; ++j)
  {
    // base node info
    const StdMeshers_TNode& aBN=myTNodes[j];
    aBNSSID=(SMESH_Block::TShapeID)aBN.ShapeSupportID();
    iBNID=aBN.BaseNodeID();
    const gp_XYZ& aBNXYZ=aBN.NormCoord();
    bool createNode = ( aBNSSID == SMESH_Block::ID_Fxy0 );
    //
    // set XYZ on horizontal edges and get node columns of faces:
    // 2 columns for each face, between which a base node is located
    vector<const SMDS_MeshNode*>* nColumns[8];
    double ratio[4]; // base node position between columns [0.-1.]
    if ( createNode )
      for ( k = 0; k < 4; ++k )
        ratio[ k ] = SetHorizEdgeXYZ (aBNXYZ, wallFaceID[ k ],
                                      nColumns[k*2], nColumns[k*2+1]);
    //
    // XYZ on the bottom and top faces
    const SMDS_MeshNode* n = aBN.Node();
    myShapeXYZ[ SMESH_Block::ID_Fxy0 ].SetCoord( n->X(), n->Y(), n->Z() );
    myShapeXYZ[ SMESH_Block::ID_Fxy1 ].SetCoord( 0., 0., 0. );
    //
    // first create or find a top node, then the rest ones in a column
    for (i=myISize-1; i>0; --i)
    {
      if ( createNode ) {
        // set XYZ on vertical edges and faces
        for ( k = 0; k < 4; ++k ) {
          const SMDS_MeshNode* n = (*verticEdgeNodes[ k ]) [ i ];
          myShapeXYZ[ verticEdgeID[ k ] ].SetCoord( n->X(), n->Y(), n->Z() );
          //
          n = (*nColumns[k*2]) [ i ];
          gp_XYZ xyz( n->X(), n->Y(), n->Z() );
          myShapeXYZ[ wallFaceID[ k ]] = ( 1. - ratio[ k ]) * xyz;
          n = (*nColumns[k*2+1]) [ i ];
          xyz.SetCoord( n->X(), n->Y(), n->Z() );
          myShapeXYZ[ wallFaceID[ k ]] += ratio[ k ] * xyz;
        }
      }
      // fill current node info
      //   -index in aTNodes
      ij=i*myJSize+j; 
      //   -normalized coordinates  
      aX=aBNXYZ.X();  
      aY=aBNXYZ.Y();
      //aZ=aZL[i];
      aZ=(double)i/(double)(myISize-1);
      aCoords.SetCoord(aX, aY, aZ);
      //
      //   suporting shape ID
      bIsUpperLayer=(i==(myISize-1));
      ShapeSupportID(bIsUpperLayer, aBNSSID, aSSID);
      if (myErrorStatus) {
        MESSAGE("StdMeshers_Penta_3D::MakeNodes() ");
	return;
      }
      //
      aTN.SetShapeSupportID(aSSID);
      aTN.SetNormCoord(aCoords);
      aTN.SetBaseNodeID(iBNID);
      //
      if (aSSID!=SMESH_Block::ID_NONE){
	// try to find the node
	const TopoDS_Shape& aS=myBlock.Shape((int)aSSID);
	FindNodeOnShape(aS, aCoords, i, aTN);
      }
      else{
	// create node and get it id
	CreateNode (bIsUpperLayer, aCoords, aTN);
        //
        if ( bIsUpperLayer ) {
          const SMDS_MeshNode* n = aTN.Node();
          myShapeXYZ[ SMESH_Block::ID_Fxy1 ].SetCoord( n->X(), n->Y(), n->Z() );
        }
      }
      if (myErrorStatus) {
        MESSAGE("StdMeshers_Penta_3D::MakeNodes() ");
	return;
      }
      //
      myTNodes[ij]=aTN;
    }
  }
  //DEB
  /*
  {
    int iSSID, iBNID, aID;
    //
    for (i=0; i<myISize; ++i) {
      printf(" Layer# %d\n", i);
      for (j=0; j<myJSize; ++j) {
	ij=i*myJSize+j; 
	const StdMeshers_TNode& aTN=myTNodes[ij];
	//const StdMeshers_TNode& aTN=aTNodes[ij];
	const gp_XYZ& aXYZ=aTN.NormCoord();
	iSSID=aTN.ShapeSupportID();
	iBNID=aTN.BaseNodeID();
	//
	const SMDS_MeshNode* aNode=aTN.Node();
	aID=aNode->GetID(); 
	aX=aNode->X();
	aY=aNode->Y();
	aZ=aNode->Z();
	printf("*** j:%d BNID#%d iSSID:%d ID:%d { %lf %lf %lf },  { %lf %lf %lf }\n",
	       j,  iBNID, iSSID, aID, aXYZ.X(),  aXYZ.Y(), aXYZ.Z(), aX, aY, aZ);
      }
    }
  }
  */
  //DEB t
}
//=======================================================================
//function : FindNodeOnShape
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::FindNodeOnShape(const TopoDS_Shape& aS,
					  const gp_XYZ&       aParams,
                                          const int           z,
					  StdMeshers_TNode&   aTN)
{
  myErrorStatus=0;
  //
  double aX, aY, aZ, aD, aTol2, minD;
  gp_Pnt aP1, aP2;
  //
  SMESH_Mesh* pMesh=GetMesh();
  aTol2=myTol3D*myTol3D;
  minD = 1.e100;
  SMDS_MeshNode* pNode=NULL;
  //
  if ( aS.ShapeType() == TopAbs_FACE ||
       aS.ShapeType() == TopAbs_EDGE )
  {
    // find a face ID to which aTN belongs to
    int faceID;
    if ( aS.ShapeType() == TopAbs_FACE )
      faceID = myBlock.ShapeID( aS );
    else { // edge maybe vertical or top horizontal
      gp_XYZ aCoord = aParams;
      if ( aCoord.Z() == 1. )
        aCoord.SetZ( 0.5 ); // move from top down
      else
        aCoord.SetX( 0.5 ); // move along X
      faceID = SMESH_Block::GetShapeIDByParams( aCoord );
    }
    ASSERT( SMESH_Block::IsFaceID( faceID ));
    int fIndex = SMESH_Block::ShapeIndex( faceID );
    StdMeshers_IJNodeMap & ijNodes= myWallNodesMaps[ fIndex ];
    // look for a base node in ijNodes
    const SMDS_MeshNode* baseNode = pMesh->GetMeshDS()->FindNode( aTN.BaseNodeID() );
    StdMeshers_IJNodeMap::const_iterator par_nVec = ijNodes.begin();
    for ( ; par_nVec != ijNodes.end(); par_nVec++ )
      if ( par_nVec->second[ 0 ] == baseNode ) {
        pNode=(SMDS_MeshNode*)par_nVec->second.at( z );
        aTN.SetNode(pNode);
        return;
      }
  }
  //
  myBlock.Point(aParams, aS, aP1);
  //
  SMDS_NodeIteratorPtr ite=
    pMesh->GetSubMeshContaining(aS)->GetSubMeshDS()->GetNodes();
  while(ite->more()) {
    const SMDS_MeshNode* aNode = ite->next();
    aX=aNode->X();
    aY=aNode->Y();
    aZ=aNode->Z();
    aP2.SetCoord(aX, aY, aZ);
    aD=(double)aP1.SquareDistance(aP2);
    //printf("** D=%lf ", aD, aTol2);
    if (aD < minD) {
      pNode=(SMDS_MeshNode*)aNode;
      aTN.SetNode(pNode);
      minD = aD;
      //printf(" Ok\n");
      if (aD<aTol2)
        return; 
    }
  }
  //
  //printf(" KO\n");
  //aTN.SetNode(pNode);
  //MESSAGE("StdMeshers_Penta_3D::FindNodeOnShape(), can not find the node");
  //myErrorStatus=11; // can not find the node;
}

//=======================================================================
//function : SetHorizEdgeXYZ
//purpose  : 
//=======================================================================

double StdMeshers_Penta_3D::SetHorizEdgeXYZ(const gp_XYZ&                  aBaseNodeParams,
                                            const int                      aFaceID,
                                            vector<const SMDS_MeshNode*>*& aCol1,
                                            vector<const SMDS_MeshNode*>*& aCol2)
{
  // find base and top edges of the face
  vector< int > edgeVec; // 0-base, 1-top
  SMESH_Block::GetFaceEdgesIDs( aFaceID, edgeVec );
  //
  int coord = SMESH_Block::GetCoordIndOnEdge( edgeVec[ 0 ] );
  double param = aBaseNodeParams.Coord( coord );
  if ( !myBlock.IsForwadEdge( edgeVec[ 0 ] ))
    param = 1. - param;
  //
  // look for columns around param
  StdMeshers_IJNodeMap & ijNodes =
    myWallNodesMaps[ SMESH_Block::ShapeIndex( aFaceID )];
  StdMeshers_IJNodeMap::iterator par_nVec_1 = ijNodes.begin();
  while ( par_nVec_1->first < param )
    par_nVec_1++;
  StdMeshers_IJNodeMap::iterator par_nVec_2 = par_nVec_1;
  //
  double r = 0;
  if ( par_nVec_1 != ijNodes.begin() ) {
    par_nVec_1--;
    r = ( param - par_nVec_1->first ) / ( par_nVec_2->first - par_nVec_1->first );
  }
  aCol1 = & par_nVec_1->second;
  aCol2 = & par_nVec_2->second;

  // base edge
  const SMDS_MeshNode* n1 = aCol1->front();
  const SMDS_MeshNode* n2 = aCol2->front();
  gp_XYZ xyz1( n1->X(), n1->Y(), n1->Z() ), xyz2( n2->X(), n2->Y(), n2->Z() );
  myShapeXYZ[ edgeVec[ 0 ] ] = ( 1. - r ) * xyz1 + r * xyz2;

  // top edge
  n1 = aCol1->back();
  n2 = aCol2->back();
  xyz1.SetCoord( n1->X(), n1->Y(), n1->Z() );
  xyz2.SetCoord( n2->X(), n2->Y(), n2->Z() );
  myShapeXYZ[ edgeVec[ 1 ] ] = ( 1. - r ) * xyz1 + r * xyz2;

  return r;
}

//=======================================================================
//function : MakeVolumeMesh
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeVolumeMesh()
{
  myErrorStatus=0;
  //
  int i, j, ij, ik, i1, i2, aSSID; 
  //
  SMESH_Mesh*   pMesh =GetMesh();
  SMESHDS_Mesh* meshDS=pMesh->GetMeshDS();
  //
  int shapeID = meshDS->ShapeToIndex( myShape );
  //
  // 1. Set Node In Volume
  ik=myISize-1;
  for (i=1; i<ik; ++i){
    for (j=0; j<myJSize; ++j){
      ij=i*myJSize+j;
      const StdMeshers_TNode& aTN=myTNodes[ij];
      aSSID=aTN.ShapeSupportID();
      if (aSSID==SMESH_Block::ID_NONE) {
	SMDS_MeshNode* aNode=(SMDS_MeshNode*)aTN.Node();
	meshDS->SetNodeInVolume(aNode, shapeID);
      }
    }
  }
  //
  // 2. Make pentahedrons
  int aID0, k , aJ[3];
  vector<const SMDS_MeshNode*> aN;
  //
  SMDS_ElemIteratorPtr itf, aItNodes;
  //
  const TopoDS_Face& aFxy0=
    TopoDS::Face(myBlock.Shape(SMESH_Block::ID_Fxy0));
  SMESH_subMesh *aSubMesh0 = pMesh->GetSubMeshContaining(aFxy0);
  SMESHDS_SubMesh *aSM0=aSubMesh0->GetSubMeshDS();
  //
  itf=aSM0->GetElements();
  while(itf->more()) {
    const SMDS_MeshElement* pE0=itf->next();
    //
    int nbFaceNodes = pE0->NbNodes();
    if ( aN.size() < nbFaceNodes * 2 )
      aN.resize( nbFaceNodes * 2 );
    //
    k=0;
    aItNodes=pE0->nodesIterator();
    while (aItNodes->more()) {
      const SMDS_MeshElement* pNode=aItNodes->next();
      aID0=pNode->GetID();
      aJ[k]=GetIndexOnLayer(aID0);
      if (myErrorStatus) {
        MESSAGE("StdMeshers_Penta_3D::MakeVolumeMesh");
	return;
      }
      //
      ++k;
    }
    //
    bool forward = true;
    for (i=0; i<ik; ++i){
      i1=i;
      i2=i+1;
      for(j=0; j<nbFaceNodes; ++j) {
	ij=i1*myJSize+aJ[j];
	const StdMeshers_TNode& aTN1=myTNodes[ij];
	const SMDS_MeshNode* aN1=aTN1.Node();
	aN[j]=aN1;
	//
	ij=i2*myJSize+aJ[j];
	const StdMeshers_TNode& aTN2=myTNodes[ij];
	const SMDS_MeshNode* aN2=aTN2.Node();
	aN[j+nbFaceNodes]=aN2;
      }
      // check if volume orientation will be ok
      if ( i == 0 ) {
        SMDS_VolumeTool vTool;
        switch ( nbFaceNodes ) {
        case 3: {
          SMDS_VolumeOfNodes tmpVol (aN[0], aN[1], aN[2],
                                     aN[3], aN[4], aN[5]);
          vTool.Set( &tmpVol );
          break;
        }
        case 4: {
          SMDS_VolumeOfNodes tmpVol(aN[0], aN[1], aN[2], aN[3],
                                    aN[4], aN[5], aN[6], aN[7]);
          vTool.Set( &tmpVol );
          break;
        }
        default:
          continue;
        }
        forward = vTool.IsForward();
      }
      // add volume
      SMDS_MeshVolume* aV = 0;
      switch ( nbFaceNodes ) {
      case 3:
        if ( forward )
          aV = meshDS->AddVolume(aN[0], aN[1], aN[2],
                                 aN[3], aN[4], aN[5]);
        else
          aV = meshDS->AddVolume(aN[0], aN[2], aN[1],
                                 aN[3], aN[5], aN[4]);
        break;
      case 4:
        if ( forward )
          aV = meshDS->AddVolume(aN[0], aN[1], aN[2], aN[3],
                                 aN[4], aN[5], aN[6], aN[7]);
        else
          aV = meshDS->AddVolume(aN[0], aN[3], aN[2], aN[1],
                                 aN[4], aN[7], aN[6], aN[5]);
        break;
      default:
        continue;
      }
      meshDS->SetMeshElementOnShape(aV, shapeID);
    }
  }
}

//=======================================================================
//function : MakeMeshOnFxy1
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeMeshOnFxy1()
{
  myErrorStatus=0;
  //
  int aID0, aJ, aLevel, ij, aNbNodes, k;
  //
  SMDS_NodeIteratorPtr itn;
  SMDS_ElemIteratorPtr itf, aItNodes;
  SMDSAbs_ElementType aElementType;
  //
  const TopoDS_Face& aFxy0=
    TopoDS::Face(myBlock.Shape(SMESH_Block::ID_Fxy0));
  const TopoDS_Face& aFxy1=
    TopoDS::Face(myBlock.Shape(SMESH_Block::ID_Fxy1));
  //
  SMESH_Mesh* pMesh=GetMesh();
  SMESHDS_Mesh * meshDS = pMesh->GetMeshDS();
  //
  SMESH_subMesh *aSubMesh0 = pMesh->GetSubMeshContaining(aFxy0);
  SMESHDS_SubMesh *aSM0=aSubMesh0->GetSubMeshDS();
  //
  // set nodes on aFxy1
  aLevel=myISize-1;
  itn=aSM0->GetNodes();
  aNbNodes=aSM0->NbNodes();
  //printf("** aNbNodes=%d\n", aNbNodes);
  while(itn->more()) {
    const SMDS_MeshNode* aN0=itn->next();
    aID0=aN0->GetID();
    aJ=GetIndexOnLayer(aID0);
    if (myErrorStatus) {
      MESSAGE("StdMeshers_Penta_3D::MakeMeshOnFxy1() ");
      return;
    }
    //
    ij=aLevel*myJSize+aJ;
    const StdMeshers_TNode& aTN1=myTNodes[ij];
    SMDS_MeshNode* aN1=(SMDS_MeshNode*)aTN1.Node();
    //
    meshDS->SetNodeOnFace(aN1, aFxy1);
  }
  //
  // set elements on aFxy1
  vector<const SMDS_MeshNode*> aNodes1;
  //
  itf=aSM0->GetElements();
  while(itf->more()) {
    const SMDS_MeshElement * pE0=itf->next();
    aElementType=pE0->GetType();
    if (!aElementType==SMDSAbs_Face) {
      continue;
    }
    aNbNodes=pE0->NbNodes();
//     if (aNbNodes!=3) {
//       continue;
//     }
    if ( aNodes1.size() < aNbNodes )
      aNodes1.resize( aNbNodes );
    //
    k=aNbNodes-1; // reverse a face
    aItNodes=pE0->nodesIterator();
    while (aItNodes->more()) {
      const SMDS_MeshElement* pNode=aItNodes->next();
      aID0=pNode->GetID();
      aJ=GetIndexOnLayer(aID0);
      if (myErrorStatus) {
        MESSAGE("StdMeshers_Penta_3D::MakeMeshOnFxy1() ");
	return;
      }
      //
      ij=aLevel*myJSize+aJ;
      const StdMeshers_TNode& aTN1=myTNodes[ij];
      const SMDS_MeshNode* aN1=aTN1.Node();
      aNodes1[k]=aN1;
      --k;
    }
    SMDS_MeshFace * face = 0;
    switch ( aNbNodes ) {
    case 3:
      face = meshDS->AddFace(aNodes1[0], aNodes1[1], aNodes1[2]);
      break;
    case 4:
      face = meshDS->AddFace(aNodes1[0], aNodes1[1], aNodes1[2], aNodes1[3]);
      break;
    default:
      continue;
    }
    meshDS->SetMeshElementOnShape(face, aFxy1);
  }
}
//=======================================================================
//function : ClearMeshOnFxy1
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::ClearMeshOnFxy1()
{
  myErrorStatus=0;
  //
  SMESH_subMesh* aSubMesh;
  SMESH_Mesh* pMesh=GetMesh();
  //
  const TopoDS_Shape& aFxy1=myBlock.Shape(SMESH_Block::ID_Fxy1);
  aSubMesh = pMesh->GetSubMeshContaining(aFxy1);
  if (aSubMesh)
    aSubMesh->ComputeStateEngine( SMESH_subMesh::CLEAN );
}

//=======================================================================
//function : GetIndexOnLayer
//purpose  : 
//=======================================================================
int StdMeshers_Penta_3D::GetIndexOnLayer(const int aID)
{
  myErrorStatus=0;
  //
  int j=-1;
  StdMeshers_IteratorOfDataMapOfIntegerInteger aMapIt;
  //
  aMapIt=myConnectingMap.find(aID);
  if (aMapIt==myConnectingMap.end()) {
    myErrorStatus=200;
    return j;
  }
  j=(*aMapIt).second;
  return j;
}
//=======================================================================
//function : MakeConnectingMap
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeConnectingMap()
{
  int j, aBNID;
  //
  for (j=0; j<myJSize; ++j) {
    const StdMeshers_TNode& aBN=myTNodes[j];
    aBNID=aBN.BaseNodeID();
    myConnectingMap[aBNID]=j;
  }
}
//=======================================================================
//function : CreateNode
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::CreateNode(const bool bIsUpperLayer,
				     const gp_XYZ& aParams,
				     StdMeshers_TNode& aTN)
{
  myErrorStatus=0;
  //
  // int iErr;
  double aX, aY, aZ;
  //
  gp_Pnt aP;
  //
  SMDS_MeshNode* pNode=NULL; 
  aTN.SetNode(pNode);  
  //
//   if (bIsUpperLayer) {
//     // point on face Fxy1
//     const TopoDS_Shape& aS=myBlock.Shape(SMESH_Block::ID_Fxy1);
//     myBlock.Point(aParams, aS, aP);
//   }
//   else {
//     // point inside solid
//     myBlock.Point(aParams, aP);
//   }
  if (bIsUpperLayer)
  {
    double u = aParams.X(), v = aParams.Y();
    double u1 = ( 1. - u ), v1 = ( 1. - v );
    aP.ChangeCoord()  = myShapeXYZ[ SMESH_Block::ID_Ex01 ] * v1;
    aP.ChangeCoord() += myShapeXYZ[ SMESH_Block::ID_Ex11 ] * v;
    aP.ChangeCoord() += myShapeXYZ[ SMESH_Block::ID_E0y1 ] * u1;
    aP.ChangeCoord() += myShapeXYZ[ SMESH_Block::ID_E1y1 ] * u;

    aP.ChangeCoord() -= myShapeXYZ[ SMESH_Block::ID_V001 ] * u1 * v1;
    aP.ChangeCoord() -= myShapeXYZ[ SMESH_Block::ID_V101 ] * u  * v1;
    aP.ChangeCoord() -= myShapeXYZ[ SMESH_Block::ID_V011 ] * u1 * v;
    aP.ChangeCoord() -= myShapeXYZ[ SMESH_Block::ID_V111 ] * u  * v;
  }
  else
  {
    SMESH_Block::ShellPoint( aParams, myShapeXYZ, aP.ChangeCoord() );
  }
  //
//   iErr=myBlock.ErrorStatus();
//   if (iErr) {
//     myErrorStatus=12; // can not find the node point;
//     return;
//   }
  //
  aX=aP.X(); aY=aP.Y(); aZ=aP.Z(); 
  //
  SMESH_Mesh* pMesh=GetMesh();
  SMESHDS_Mesh* pMeshDS=pMesh->GetMeshDS();
  //
  pNode = pMeshDS->AddNode(aX, aY, aZ);
  aTN.SetNode(pNode);
}
//=======================================================================
//function : ShapeSupportID
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::ShapeSupportID(const bool bIsUpperLayer,
					 const SMESH_Block::TShapeID aBNSSID,
					 SMESH_Block::TShapeID& aSSID)
{
  myErrorStatus=0;
  //
  switch (aBNSSID) {
    case SMESH_Block::ID_V000:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_V001 : SMESH_Block::ID_E00z;
      break;
    case SMESH_Block::ID_V100:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_V101 : SMESH_Block::ID_E10z;
      break; 
    case SMESH_Block::ID_V110:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_V111 : SMESH_Block::ID_E11z;
      break;
    case SMESH_Block::ID_V010:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_V011 : SMESH_Block::ID_E01z;
      break;
    case SMESH_Block::ID_Ex00:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_Ex01 : SMESH_Block::ID_Fx0z;
      break;
    case SMESH_Block::ID_Ex10:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_Ex11 : SMESH_Block::ID_Fx1z;
      break; 
    case SMESH_Block::ID_E0y0:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_E0y1 : SMESH_Block::ID_F0yz;
      break; 
    case SMESH_Block::ID_E1y0:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_E1y1 : SMESH_Block::ID_F1yz;
      break; 
    case SMESH_Block::ID_Fxy0:
      aSSID=SMESH_Block::ID_NONE;//(bIsUpperLayer) ?  Shape_ID_Fxy1 : Shape_ID_NONE;
      break;   
    default:
      aSSID=SMESH_Block::ID_NONE;
      myErrorStatus=10; // Can not find supporting shape ID
      break;
  }
  return;
}
//=======================================================================
//function : MakeBlock
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeBlock()
{
  myErrorStatus=0;
  //
  bool bFound;
  int i, j, iNbEV, iNbE, iErr, iCnt, iNbNodes, iNbF;
  //
  TopoDS_Vertex aV000, aV001;
  TopoDS_Shape aFTr;
  TopTools_IndexedDataMapOfShapeListOfShape aMVES;
  TopTools_IndexedMapOfShape aME ,aMEV, aM;
  TopTools_ListIteratorOfListOfShape aIt;
  //
  TopExp::MapShapes(myShape, TopAbs_FACE, aM);
  //
  // 0. Find triangulated face aFTr
  SMDSAbs_ElementType aElementType;
  SMESH_Mesh* pMesh=GetMesh();
  //
  iCnt=0;
  iNbF=aM.Extent();
  for (i=1; i<=iNbF; ++i) {
    const TopoDS_Shape& aF=aM(i);
    SMESH_subMesh *aSubMesh = pMesh->GetSubMeshContaining(aF);
    ASSERT(aSubMesh);
    SMESHDS_SubMesh *aSM=aSubMesh->GetSubMeshDS();
    SMDS_ElemIteratorPtr itf=aSM->GetElements();
    while(itf->more()) {
      const SMDS_MeshElement * pElement=itf->next();
      aElementType=pElement->GetType();
      if (aElementType==SMDSAbs_Face) {
	iNbNodes=pElement->NbNodes();
	if (iNbNodes==3) {
	  aFTr=aF;
	  ++iCnt;
	  if (iCnt>1) {
            MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
	    myErrorStatus=5; // more than one face has triangulation
	    return;
	  }
	  break; // next face
	}
      }
    }
  }
  // 
  // 1. Vetrices V00, V001;
  //
  TopExp::MapShapes(aFTr, TopAbs_EDGE, aME);
  TopExp::MapShapesAndAncestors(myShape, TopAbs_VERTEX, TopAbs_EDGE, aMVES);
  //
  // 1.1 Base vertex V000
  iNbE=aME.Extent();
  if (iNbE!=4){
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=7; // too few edges are in base face aFTr 
    return;
  }
  const TopoDS_Edge& aE1=TopoDS::Edge(aME(1));
  aV000=TopExp::FirstVertex(aE1);
  //
  const TopTools_ListOfShape& aLE=aMVES.FindFromKey(aV000);
  aIt.Initialize(aLE);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aEx=aIt.Value();
    aMEV.Add(aEx);
  }
  iNbEV=aMEV.Extent();
  if (iNbEV!=3){
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=7; // too few edges meet in base vertex 
    return;
  }
  //
  // 1.2 Vertex V001
  bFound=false;
  for (j=1; j<=iNbEV; ++j) {
    const TopoDS_Shape& aEx=aMEV(j);
    if (!aME.Contains(aEx)) {
      TopoDS_Vertex aV[2];
      //
      const TopoDS_Edge& aE=TopoDS::Edge(aEx);
      TopExp::Vertices(aE, aV[0], aV[1]);
      for (i=0; i<2; ++i) {
	if (!aV[i].IsSame(aV000)) {
	  aV001=aV[i];
	  bFound=!bFound;
	  break;
	}
      }
    }
  }
  //
  if (!bFound) {
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=8; // can not find reper V001 
    return;
  }
  //DEB
  //gp_Pnt aP000, aP001;
  //
  //aP000=BRep_Tool::Pnt(TopoDS::Vertex(aV000));
  //printf("*** aP000 { %lf, %lf, %lf }\n", aP000.X(), aP000.Y(), aP000.Z());
  //aP001=BRep_Tool::Pnt(TopoDS::Vertex(aV001));
  //printf("*** aP001 { %lf, %lf, %lf }\n", aP001.X(), aP001.Y(), aP001.Z());
  //DEB
  //
  aME.Clear();
  TopExp::MapShapes(myShape, TopAbs_SHELL, aME);
  iNbE=aME.Extent();
  if (iNbE!=1) {
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=9; // number of shells in source shape !=1 
    return;
  }
  //
  // 2. Load Block
  const TopoDS_Shell& aShell=TopoDS::Shell(aME(1));
  myBlock.Load(aShell, aV000, aV001);
  iErr=myBlock.ErrorStatus();
  if (iErr) {
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=100; // SMESHBlock: Load operation failed
    return;
  }
}
//=======================================================================
//function : CheckData
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::CheckData()
{
  myErrorStatus=0;
  //
  int i, iNb;
  int iNbEx[]={8, 12, 6};
  //
  TopAbs_ShapeEnum aST;
  TopAbs_ShapeEnum aSTEx[]={
    TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE
  }; 
  TopTools_IndexedMapOfShape aM;
  //
  if (myShape.IsNull()){
    MESSAGE("StdMeshers_Penta_3D::CheckData() ");
    myErrorStatus=2; // null shape
    return;
  }
  //
  aST=myShape.ShapeType();
  if (!(aST==TopAbs_SOLID || aST==TopAbs_SHELL)) {
    MESSAGE("StdMeshers_Penta_3D::CheckData() ");
    myErrorStatus=3; // not compatible type of shape
    return;
  }
  //
  for (i=0; i<3; ++i) {
    aM.Clear();
    TopExp::MapShapes(myShape, aSTEx[i], aM);
    iNb=aM.Extent();
    if (iNb!=iNbEx[i]){
      MESSAGE("StdMeshers_Penta_3D::CheckData() ");
      myErrorStatus=4; // number of subshape is not compatible
      return;
    }
  }
}

//=======================================================================
//function : LoadIJNodes
//purpose  : Load nodes bound to theFace into column (vectors) and rows
//           of theIJNodes.
//           The value of theIJNodes map is a vector of ordered nodes so
//           that the 0-the one lies on theBaseEdge.
//           The key of theIJNodes map is a normalized parameter of each
//           0-the node on theBaseEdge.
//=======================================================================

bool StdMeshers_Penta_3D::LoadIJNodes(StdMeshers_IJNodeMap & theIJNodes,
                                      const TopoDS_Face&     theFace,
                                      const TopoDS_Edge&     theBaseEdge,
                                      SMESHDS_Mesh*          theMesh)
{
  // get vertices of theBaseEdge
  TopoDS_Vertex vfb, vlb, vft; // first and last, bottom and top vertices
  TopoDS_Edge eFrw = TopoDS::Edge( theBaseEdge.Oriented( TopAbs_FORWARD ));
  TopExp::Vertices( eFrw, vfb, vlb );

  // find the other edges of theFace and orientation of e1
  TopoDS_Edge e1, e2, eTop;
  bool rev1, CumOri = false;
  TopExp_Explorer exp( theFace, TopAbs_EDGE );
  int nbEdges = 0;
  for ( ; exp.More(); exp.Next() )
  {
    if ( ++nbEdges > 4 )
      return false; // more than 4 edges in theFace
    TopoDS_Edge e = TopoDS::Edge( exp.Current() );
    if ( theBaseEdge.IsSame( e ))
      continue;
    TopoDS_Vertex vCommon;
    if ( !TopExp::CommonVertex( theBaseEdge, e, vCommon ))
      eTop = e;
    else if ( vCommon.IsSame( vfb )) {
      e1 = e;
      vft = TopExp::LastVertex( e1, CumOri );
      rev1 = vfb.IsSame( vft );
      if ( rev1 )
        vft = TopExp::FirstVertex( e1, CumOri );
    }
    else
      e2 = e;
  }
  if ( nbEdges < 4 )
    return false; // lass than 4 edges in theFace

  // submeshes corresponding to shapes
  SMESHDS_SubMesh* smFace = theMesh->MeshElements( theFace );
  SMESHDS_SubMesh* smb = theMesh->MeshElements( theBaseEdge );
  SMESHDS_SubMesh* smt = theMesh->MeshElements( eTop );
  SMESHDS_SubMesh* sm1 = theMesh->MeshElements( e1 );
  SMESHDS_SubMesh* sm2 = theMesh->MeshElements( e2 );
  SMESHDS_SubMesh* smVfb = theMesh->MeshElements( vfb );
  SMESHDS_SubMesh* smVlb = theMesh->MeshElements( vlb );
  SMESHDS_SubMesh* smVft = theMesh->MeshElements( vft );
  if (!smFace || !smb || !smt || !sm1 || !sm2 || !smVfb || !smVlb || !smVft ) {
    MESSAGE( "NULL submesh " <<smFace<<" "<<smb<<" "<<smt<<" "<<
            sm1<<" "<<sm2<<" "<<smVfb<<" "<<smVlb<<" "<<smVft);
    return false;
  }
  if ( smb->NbNodes() != smt->NbNodes() || sm1->NbNodes() != sm2->NbNodes() ) {
    MESSAGE(" Diff nb of nodes on opposite edges" );
    return false;
  }
  if (smVfb->NbNodes() != 1 || smVlb->NbNodes() != 1 || smVft->NbNodes() != 1) {
    MESSAGE("Empty submesh of vertex");
    return false;
  }
  if ( sm1->NbNodes() * smb->NbNodes() != smFace->NbNodes() ) {
    MESSAGE( "Wrong nb face nodes: " <<
            sm1->NbNodes()<<" "<<smb->NbNodes()<<" "<<smFace->NbNodes());
    return false;
  }
  // IJ size
  int vsize = sm1->NbNodes() + 2;
  int hsize = smb->NbNodes() + 2;

  // load nodes from theBaseEdge

  set<const SMDS_MeshNode*> loadedNodes;
  const SMDS_MeshNode* nullNode = 0;

  vector<const SMDS_MeshNode*> & nVecf = theIJNodes[ 0.];
  nVecf.resize( vsize, nullNode );
  loadedNodes.insert( nVecf[ 0 ] = smVfb->GetNodes()->next() );

  vector<const SMDS_MeshNode*> & nVecl = theIJNodes[ 1.];
  nVecl.resize( vsize, nullNode );
  loadedNodes.insert( nVecl[ 0 ] = smVlb->GetNodes()->next() );

  double f, l;
  BRep_Tool::Range( eFrw, f, l );
  double range = l - f;
  SMDS_NodeIteratorPtr nIt = smb->GetNodes();
  const SMDS_MeshNode* node;
  while ( nIt->more() )
  {
    node = nIt->next();
    const SMDS_EdgePosition* pos =
      dynamic_cast<const SMDS_EdgePosition*>( node->GetPosition().get() );
    if ( !pos ) return false;
    double u = ( pos->GetUParameter() - f ) / range;
    vector<const SMDS_MeshNode*> & nVec = theIJNodes[ u ];
    nVec.resize( vsize, nullNode );
    loadedNodes.insert( nVec[ 0 ] = node );
  }
  if ( theIJNodes.size() != hsize ) {
    MESSAGE( "Wrong node positions on theBaseEdge" );
    return false;
  }

  // load nodes from e1

  map< double, const SMDS_MeshNode*> sortedNodes; // sort by param on edge
  nIt = sm1->GetNodes();
  while ( nIt->more() )
  {
    node = nIt->next();
    const SMDS_EdgePosition* pos =
      dynamic_cast<const SMDS_EdgePosition*>( node->GetPosition().get() );
    if ( !pos ) return false;
    sortedNodes.insert( make_pair( pos->GetUParameter(), node ));
  }
  loadedNodes.insert( nVecf[ vsize - 1 ] = smVft->GetNodes()->next() );
  map< double, const SMDS_MeshNode*>::iterator u_n = sortedNodes.begin();
  int row = rev1 ? vsize - 1 : 0;
  for ( ; u_n != sortedNodes.end(); u_n++ )
  {
    if ( rev1 ) row--;
    else        row++;
    loadedNodes.insert( nVecf[ row ] = u_n->second );
  }

  // try to load the rest nodes

  // get all faces from theFace
  set<const SMDS_MeshElement*> allFaces, foundFaces;
  SMDS_ElemIteratorPtr eIt = smFace->GetElements();
  while ( eIt->more() ) {
    const SMDS_MeshElement* e = eIt->next();
    if ( e->GetType() == SMDSAbs_Face )
      allFaces.insert( e );
  }
  // Starting from 2 neighbour nodes on theBaseEdge, look for a face
  // the nodes belong to, and between the nodes of the found face,
  // look for a not loaded node considering this node to be the next
  // in a column of the starting second node. Repeat, starting
  // from nodes next to the previous starting nodes in their columns,
  // and so on while a face can be found. Then go the the next pair
  // of nodes on theBaseEdge.
  StdMeshers_IJNodeMap::iterator par_nVec_1 = theIJNodes.begin();
  StdMeshers_IJNodeMap::iterator par_nVec_2 = par_nVec_1;
  // loop on columns
  int col = 0;
  for ( par_nVec_2++; par_nVec_2 != theIJNodes.end(); par_nVec_1++, par_nVec_2++ )
  {
    col++;
    row = 0;
    const SMDS_MeshNode* n1 = par_nVec_1->second[ row ];
    const SMDS_MeshNode* n2 = par_nVec_2->second[ row ];
    const SMDS_MeshElement* face = 0;
    do {
      // look for a face by 2 nodes
      face = SMESH_MeshEditor::FindFaceInSet( n1, n2, allFaces, foundFaces );
      if ( face )
      {
        int nbFaceNodes = face->NbNodes();
        if ( nbFaceNodes > 4 ) {
          MESSAGE(" Too many nodes in a face: " << nbFaceNodes );
          return false;
        }
        // look for a not loaded node of the <face>
        bool found = false;
        const SMDS_MeshNode* n3 = 0; // a node defferent from n1 and n2
        eIt = face->nodesIterator() ;
        while ( !found && eIt->more() ) {
          node = static_cast<const SMDS_MeshNode*>( eIt->next() );
          found = loadedNodes.insert( node ).second;
          if ( !found && node != n1 && node != n2 )
            n3 = node;
        }
        if ( found ) {
          if ( ++row > vsize - 1 ) {
            MESSAGE( "Too many nodes in column "<< col <<": "<< row+1);
            return false;
          }
          par_nVec_2->second[ row ] = node;
          foundFaces.insert( face );
          n2 = node;
          if ( nbFaceNodes == 4 )
            n1 = par_nVec_1->second[ row ];
        }
        else if (nbFaceNodes == 3 &&
                 n3 == par_nVec_1->second[ row ] )
          n1 = n3;
        else {
          MESSAGE( "Not quad mesh, column "<< col );
          return false;
        }
      }
    } while ( face && n1 && n2 );

    if ( row < vsize - 1 ) {
      MESSAGE( "Too few nodes in column "<< col <<": "<< row+1);
      MESSAGE( "Base node 1: "<< par_nVec_1->second[0]);
      MESSAGE( "Base node 2: "<< par_nVec_2->second[0]);
      MESSAGE( "Current node 1: "<< n1);
      MESSAGE( "Current node 2: "<< n2);
      MESSAGE( "first base node: "<< theIJNodes.begin()->second[0]);
      MESSAGE( "last base node: "<< theIJNodes.rbegin()->second[0]);
      return false;
    }
  } // loop on columns

  return true;
}

//////////////////////////////////////////////////////////////////////////
//
//   StdMeshers_SMESHBlock
//
//////////////////////////////////////////////////////////////////////////

//=======================================================================
//function : StdMeshers_SMESHBlock
//purpose  : 
//=======================================================================
StdMeshers_SMESHBlock::StdMeshers_SMESHBlock()
{
  myErrorStatus=1;
  myIsEdgeForward.resize( SMESH_Block::NbEdges(), -1 );
}

//=======================================================================
//function : IsForwadEdge
//purpose  : 
//=======================================================================

bool StdMeshers_SMESHBlock::IsForwadEdge(const int theEdgeID)
{
  int index = myTBlock.ShapeIndex( theEdgeID );
  if ( !myTBlock.IsEdgeID( theEdgeID ))
    return false;

  if ( myIsEdgeForward[ index ] < 0 )
    myIsEdgeForward[ index ] =
      myTBlock.IsForwardEdge( TopoDS::Edge( Shape( theEdgeID )), myShapeIDMap );

  return myIsEdgeForward[ index ];
}

//=======================================================================
//function : ErrorStatus
//purpose  : 
//=======================================================================
int StdMeshers_SMESHBlock::ErrorStatus() const
{
  return myErrorStatus;
}
//=======================================================================
//function : Load
//purpose  : 
//=======================================================================
void StdMeshers_SMESHBlock::Load(const TopoDS_Shell& theShell)
{
  
  TopoDS_Vertex aV000, aV001;
  //
  Load(theShell, aV000, aV001);
}
//=======================================================================
//function : Load
//purpose  : 
//=======================================================================
void StdMeshers_SMESHBlock::Load(const TopoDS_Shell& theShell,
				 const TopoDS_Vertex& theV000,
				 const TopoDS_Vertex& theV001)
{
  myErrorStatus=0;
  //
  myShell=theShell;
  //
  bool bOk;
  //
  myShapeIDMap.Clear();  
  bOk=myTBlock.LoadBlockShapes(myShell, theV000, theV001, myShapeIDMap);
  if (!bOk) {
    myErrorStatus=2;
    return;
  }
}
//=======================================================================
//function : ComputeParameters
//purpose  : 
//=======================================================================
void StdMeshers_SMESHBlock::ComputeParameters(const gp_Pnt& thePnt, 
					      gp_XYZ& theXYZ)
{
  ComputeParameters(thePnt, myShell, theXYZ);
}
//=======================================================================
//function : ComputeParameters
//purpose  : 
//=======================================================================
void StdMeshers_SMESHBlock::ComputeParameters(const gp_Pnt& thePnt,
					      const TopoDS_Shape& theShape,
					      gp_XYZ& theXYZ)
{
  myErrorStatus=0;
  //
  int aID;
  bool bOk;
  //
  aID=ShapeID(theShape);
  if (myErrorStatus) {
    return;
  }
  bOk=myTBlock.ComputeParameters(thePnt, theXYZ, aID);
  if (!bOk) {
    myErrorStatus=4; // problems with computation Parameters 
    return;
  }
}

//=======================================================================
//function : ComputeParameters
//purpose  : 
//=======================================================================

void StdMeshers_SMESHBlock::ComputeParameters(const double& theU,
                                              const TopoDS_Shape& theShape,
                                              gp_XYZ& theXYZ)
{
  myErrorStatus=0;
  //
  int aID;
  bool bOk=false;
  //
  aID=ShapeID(theShape);
  if (myErrorStatus) {
    return;
  }
  if ( SMESH_Block::IsEdgeID( aID ))
      bOk=myTBlock.EdgeParameters( aID, theU, theXYZ );
  if (!bOk) {
    myErrorStatus=4; // problems with computation Parameters 
    return;
  }
}

//=======================================================================
//function : Point
//purpose  : 
//=======================================================================
 void StdMeshers_SMESHBlock::Point(const gp_XYZ& theParams,
				   gp_Pnt& aP3D)
{
  TopoDS_Shape aS;
  //
  Point(theParams, aS, aP3D);
}
//=======================================================================
//function : Point
//purpose  : 
//=======================================================================
 void StdMeshers_SMESHBlock::Point(const gp_XYZ& theParams,
				   const TopoDS_Shape& theShape,
				   gp_Pnt& aP3D)
{
  myErrorStatus=0;
  //
  int aID;
  bool bOk=false;
  gp_XYZ aXYZ(99.,99.,99.);
  aP3D.SetXYZ(aXYZ);
  //
  if (theShape.IsNull()) {
    bOk=myTBlock.ShellPoint(theParams, aXYZ);
  }
  //
  else {
    aID=ShapeID(theShape);
    if (myErrorStatus) {
      return;
    }
    //
    if (SMESH_Block::IsVertexID(aID)) {
      bOk=myTBlock.VertexPoint(aID, aXYZ);
    }
    else if (SMESH_Block::IsEdgeID(aID)) {
      bOk=myTBlock.EdgePoint(aID, theParams, aXYZ);
    }
    //
    else if (SMESH_Block::IsFaceID(aID)) {
      bOk=myTBlock.FacePoint(aID, theParams, aXYZ);
    }
  }
  if (!bOk) {
    myErrorStatus=4; // problems with point computation 
    return;
  }
  aP3D.SetXYZ(aXYZ);
}
//=======================================================================
//function : ShapeID
//purpose  : 
//=======================================================================
int StdMeshers_SMESHBlock::ShapeID(const TopoDS_Shape& theShape)
{
  myErrorStatus=0;
  //
  int aID=-1;
  TopoDS_Shape aSF, aSR;
  //
  aSF=theShape;
  aSF.Orientation(TopAbs_FORWARD);
  aSR=theShape;
  aSR.Orientation(TopAbs_REVERSED);
  //
  if (myShapeIDMap.Contains(aSF)) {
    aID=myShapeIDMap.FindIndex(aSF);
    return aID;
  }
  if (myShapeIDMap.Contains(aSR)) {
    aID=myShapeIDMap.FindIndex(aSR);
    return aID;
  }
  myErrorStatus=2; // unknown shape;
  return aID;
}
//=======================================================================
//function : Shape
//purpose  : 
//=======================================================================
const TopoDS_Shape& StdMeshers_SMESHBlock::Shape(const int theID)
{
  myErrorStatus=0;
  //
  int aNb;
  //
  aNb=myShapeIDMap.Extent();
  if (theID<1 || theID>aNb) {
    myErrorStatus=3; // ID is out of range
    return myEmptyShape;
  }
  //
  const TopoDS_Shape& aS=myShapeIDMap.FindKey(theID);
  return aS;
}
