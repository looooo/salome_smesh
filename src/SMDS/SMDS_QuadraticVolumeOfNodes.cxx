// File:      SMDS_QuadraticVolumeOfNodes.cxx
// Created:   17.01.06 09:46:11
// Author:    Sergey KUUL
// Copyright: Open CASCADE 2006

#include "SMDS_QuadraticVolumeOfNodes.hxx"

#include "SMDS_IteratorOfElements.hxx"
#include "SMDS_MeshNode.hxx"

#include "utilities.h"

using namespace std;


//=======================================================================
//function : SMDS_QuadraticVolumeOfNodes()
//purpose  : Constructor tetrahedron of 10 nodes
//=======================================================================

SMDS_QuadraticVolumeOfNodes::SMDS_QuadraticVolumeOfNodes
                                               (const SMDS_MeshNode * n1,
                                                const SMDS_MeshNode * n2,
                                                const SMDS_MeshNode * n3,
                                                const SMDS_MeshNode * n4,
                                                const SMDS_MeshNode * n12,
                                                const SMDS_MeshNode * n23,
                                                const SMDS_MeshNode * n31,
                                                const SMDS_MeshNode * n14, 
                                                const SMDS_MeshNode * n24,
                                                const SMDS_MeshNode * n34)
{
  myNodes.push_back(n1);
  myNodes.push_back(n2);
  myNodes.push_back(n3);
  myNodes.push_back(n4);
  myNodes.push_back(n12);
  myNodes.push_back(n23);
  myNodes.push_back(n31);
  myNodes.push_back(n14);
  myNodes.push_back(n24);
  myNodes.push_back(n34);
}


//=======================================================================
//function : SMDS_QuadraticVolumeOfNodes()
//purpose  : Constructor pyramid of 13 nodes
//=======================================================================

SMDS_QuadraticVolumeOfNodes::SMDS_QuadraticVolumeOfNodes
                                               (const SMDS_MeshNode * n1,
                                                const SMDS_MeshNode * n2,
                                                const SMDS_MeshNode * n3,
                                                const SMDS_MeshNode * n4,
                                                const SMDS_MeshNode * n5,
                                                const SMDS_MeshNode * n12,
                                                const SMDS_MeshNode * n23,
                                                const SMDS_MeshNode * n34,
                                                const SMDS_MeshNode * n41, 
                                                const SMDS_MeshNode * n15,
                                                const SMDS_MeshNode * n25,
                                                const SMDS_MeshNode * n35,
                                                const SMDS_MeshNode * n45)
{
  myNodes.push_back(n1);
  myNodes.push_back(n2);
  myNodes.push_back(n3);
  myNodes.push_back(n4);
  myNodes.push_back(n5);
  myNodes.push_back(n12);
  myNodes.push_back(n23);
  myNodes.push_back(n34);
  myNodes.push_back(n41);
  myNodes.push_back(n15);
  myNodes.push_back(n25);
  myNodes.push_back(n35);
  myNodes.push_back(n45);
}


//=======================================================================
//function : SMDS_QuadraticVolumeOfNodes()
//purpose  : Constructor Pentahedron with 15 nodes
//=======================================================================

SMDS_QuadraticVolumeOfNodes::SMDS_QuadraticVolumeOfNodes
                                               (const SMDS_MeshNode * n1,
                                                const SMDS_MeshNode * n2,
                                                const SMDS_MeshNode * n3,
                                                const SMDS_MeshNode * n4,
                                                const SMDS_MeshNode * n5,
                                                const SMDS_MeshNode * n6, 
                                                const SMDS_MeshNode * n12,
                                                const SMDS_MeshNode * n23,
                                                const SMDS_MeshNode * n31, 
                                                const SMDS_MeshNode * n45,
                                                const SMDS_MeshNode * n56,
                                                const SMDS_MeshNode * n64, 
                                                const SMDS_MeshNode * n14,
                                                const SMDS_MeshNode * n25,
                                                const SMDS_MeshNode * n36)
{
  myNodes.push_back(n1);
  myNodes.push_back(n2);
  myNodes.push_back(n3);
  myNodes.push_back(n4);
  myNodes.push_back(n5);
  myNodes.push_back(n6);
  myNodes.push_back(n12);
  myNodes.push_back(n23);
  myNodes.push_back(n31);
  myNodes.push_back(n45);
  myNodes.push_back(n56);
  myNodes.push_back(n64);
  myNodes.push_back(n14);
  myNodes.push_back(n25);
  myNodes.push_back(n36);
}


//=======================================================================
//function : SMDS_QuadraticVolumeOfNodes()
//purpose  : Constructor Hexahedrons with 20 nodes
//=======================================================================

SMDS_QuadraticVolumeOfNodes::SMDS_QuadraticVolumeOfNodes
                                               (const SMDS_MeshNode * n1,
                                                const SMDS_MeshNode * n2,
                                                const SMDS_MeshNode * n3,
                                                const SMDS_MeshNode * n4,
                                                const SMDS_MeshNode * n5,
                                                const SMDS_MeshNode * n6,
                                                const SMDS_MeshNode * n7,
                                                const SMDS_MeshNode * n8, 
                                                const SMDS_MeshNode * n12,
                                                const SMDS_MeshNode * n23,
                                                const SMDS_MeshNode * n34,
                                                const SMDS_MeshNode * n41, 
                                                const SMDS_MeshNode * n56,
                                                const SMDS_MeshNode * n67,
                                                const SMDS_MeshNode * n78,
                                                const SMDS_MeshNode * n85, 
                                                const SMDS_MeshNode * n15,
                                                const SMDS_MeshNode * n26,
                                                const SMDS_MeshNode * n37,
                                                const SMDS_MeshNode * n48)
{
  myNodes.push_back(n1);
  myNodes.push_back(n2);
  myNodes.push_back(n3);
  myNodes.push_back(n4);
  myNodes.push_back(n5);
  myNodes.push_back(n6);
  myNodes.push_back(n7);
  myNodes.push_back(n8);
  myNodes.push_back(n12);
  myNodes.push_back(n23);
  myNodes.push_back(n34);
  myNodes.push_back(n41);
  myNodes.push_back(n56);
  myNodes.push_back(n67);
  myNodes.push_back(n78);
  myNodes.push_back(n85);
  myNodes.push_back(n15);
  myNodes.push_back(n26);
  myNodes.push_back(n37);
  myNodes.push_back(n48);
}


//=======================================================================
//function : IsMediumNode
//purpose  : 
//=======================================================================

bool SMDS_QuadraticVolumeOfNodes::IsMediumNode(const SMDS_MeshNode* node) const
{
  if(NbNodes()==10) {
    int i=4;
    for(; i<10; i++) {
      if(myNodes[i]==node) return true;
    }
  }
  else if(NbNodes()==13) {
    int i=5;
    for(; i<13; i++) {
      if(myNodes[i]==node) return true;
    }
  }
  else if(NbNodes()==15) {
    int i=6;
    for(; i<15; i++) {
      if(myNodes[i]==node) return true;
    }
  }
  else {
    int i=8;
    for(; i<20; i++) {
      if(myNodes[i]==node) return true;
    }
  }
  return false;
}


//=======================================================================
//function : ChangeNodes
//purpose  : 
//=======================================================================

bool SMDS_QuadraticVolumeOfNodes::ChangeNodes(const SMDS_MeshNode* nodes[],
                                              const int            nbNodes)
{
  if( nbNodes==10 || nbNodes==13 || nbNodes==15 || nbNodes==20 ) {
    myNodes.resize(nbNodes);
    int i=0;
    for(; i<nbNodes; i++) {
      myNodes[i] = nodes[i];
    }
    return true;
  }
  return false;
}


//=======================================================================
//function : NbNodes
//purpose  : 
//=======================================================================
int SMDS_QuadraticVolumeOfNodes::NbNodes() const
{
  return myNodes.size();
}


//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================
int SMDS_QuadraticVolumeOfNodes::NbEdges() const
{
  if(NbNodes()==10)
    return 6;
  else if(NbNodes()==13)
    return 8;
  else if(NbNodes()==15)
    return 9;
  else
    return 12;
}


//=======================================================================
//function : NbFaces
//purpose  : 
//=======================================================================
int SMDS_QuadraticVolumeOfNodes::NbFaces() const
{
  if(NbNodes()==10)
    return 4;
  else if(NbNodes()==20)
    return 6;
  else
    return 5;
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================
void SMDS_QuadraticVolumeOfNodes::Print(ostream & OS) const
{
  OS << "quadratic volume <" << GetID() << " > : ";
  int i, nbNodes = myNodes.size();
  for (i = 0; i < nbNodes - 1; i++)
    OS << myNodes[i] << ",";
  OS << myNodes[i] << ") " << endl;
}


//=======================================================================
//function : elementsIterator
//purpose  : 
//=======================================================================
class SMDS_QuadraticVolumeOfNodes_MyIterator:public SMDS_ElemIterator
{
  const std::vector<const SMDS_MeshNode *> mySet;
  int index;
 public:
  SMDS_QuadraticVolumeOfNodes_MyIterator(const std::vector<const SMDS_MeshNode *> s):
    mySet(s),index(0) {}

  bool more()
  {
    return index < mySet.size();
  }

  const SMDS_MeshElement* next()
  {
    index++;
    return mySet[index-1];
  }
};

SMDS_ElemIteratorPtr SMDS_QuadraticVolumeOfNodes::elementsIterator
                                         (SMDSAbs_ElementType type) const
{
  switch(type)
  {
  case SMDSAbs_Volume:
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Volume);
  case SMDSAbs_Node:
    return SMDS_ElemIteratorPtr(new SMDS_QuadraticVolumeOfNodes_MyIterator(myNodes));
  case SMDSAbs_Edge:
    MESSAGE("Error : edge iterator for SMDS_QuadraticVolumeOfNodes not implemented");
    break;
  case SMDSAbs_Face:
    MESSAGE("Error : face iterator for SMDS_QuadraticVolumeOfNodes not implemented");
    break;
  default:
    return SMDS_ElemIteratorPtr
      (new SMDS_IteratorOfElements
       (this,type,SMDS_ElemIteratorPtr
        (new SMDS_QuadraticVolumeOfNodes_MyIterator(myNodes))));
  }
  return SMDS_ElemIteratorPtr();
}

