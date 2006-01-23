// File:      SMDS_QuadraticFaceOfNodes.cxx
// Created:   16.01.06 17:12:58
// Author:    Sergey KUUL
// Copyright: Open CASCADE 2006

#include "SMDS_QuadraticFaceOfNodes.hxx"

#include "SMDS_IteratorOfElements.hxx"
#include "SMDS_MeshNode.hxx"

#include "utilities.h"

using namespace std;


//=======================================================================
//function : SMDS_QuadraticFaceOfNodes()
//purpose  : Constructor
//=======================================================================

SMDS_QuadraticFaceOfNodes::SMDS_QuadraticFaceOfNodes(const SMDS_MeshNode * n1,
                                                     const SMDS_MeshNode * n2,
                                                     const SMDS_MeshNode * n3,
                                                     const SMDS_MeshNode * n12,
                                                     const SMDS_MeshNode * n23,
                                                     const SMDS_MeshNode * n31)
{
  myNodes.push_back(n1);
  myNodes.push_back(n2);
  myNodes.push_back(n3);
  myNodes.push_back(n12);
  myNodes.push_back(n23);
  myNodes.push_back(n31);
}


//=======================================================================
//function : SMDS_QuadraticFaceOfNodes()
//purpose  : Constructor
//=======================================================================

SMDS_QuadraticFaceOfNodes::SMDS_QuadraticFaceOfNodes(const SMDS_MeshNode * n1,
                                                     const SMDS_MeshNode * n2,
                                                     const SMDS_MeshNode * n3,
                                                     const SMDS_MeshNode * n4,
                                                     const SMDS_MeshNode * n12,
                                                     const SMDS_MeshNode * n23,
                                                     const SMDS_MeshNode * n34,
                                                     const SMDS_MeshNode * n41)
{
  myNodes.push_back(n1);
  myNodes.push_back(n2);
  myNodes.push_back(n3);
  myNodes.push_back(n4);
  myNodes.push_back(n12);
  myNodes.push_back(n23);
  myNodes.push_back(n34);
  myNodes.push_back(n41);
}


//=======================================================================
//function : IsMediumNode
//purpose  : 
//=======================================================================

bool SMDS_QuadraticFaceOfNodes::IsMediumNode(const SMDS_MeshNode * node) const
{
  int i=NbNodes()/2;
  for(; i<NbNodes(); i++) {
    if(myNodes[i]==node) return true;
  }
  return false;
}


//=======================================================================
//function : ChangeNodes
//purpose  : 
//=======================================================================

bool SMDS_QuadraticFaceOfNodes::ChangeNodes(const SMDS_MeshNode* nodes[],
                                            const int            nbNodes)
{
  if( nbNodes==6 || nbNodes==8 ) {
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
int SMDS_QuadraticFaceOfNodes::NbNodes() const
{
  return myNodes.size();
}


//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================
int SMDS_QuadraticFaceOfNodes::NbEdges() const
{
  return NbNodes()/2;
}


//=======================================================================
//function : NbFaces
//purpose  : 
//=======================================================================
int SMDS_QuadraticFaceOfNodes::NbFaces() const
{
  return 1;
}


//=======================================================================
//function : Print
//purpose  : 
//=======================================================================
void SMDS_QuadraticFaceOfNodes::Print(ostream & OS) const
{
  OS << "quadratic face <" << GetID() << " > : ";
  int i, nbNodes = myNodes.size();
  for (i = 0; i < nbNodes - 1; i++)
    OS << myNodes[i] << ",";
  OS << myNodes[i] << ") " << endl;
}


//=======================================================================
//function : interlacedNodesIterator
//purpose  : 
//=======================================================================
class SMDS_QuadraticFaceOfNodes_MyInterlacedNodeIterator:public SMDS_NodeIterator
{
  const std::vector<const SMDS_MeshNode *> mySet;
  int index;
 public:
  SMDS_QuadraticFaceOfNodes_MyInterlacedNodeIterator(const std::vector<const SMDS_MeshNode *> s):
    mySet(s),index(0) {}

  bool more()
  {
    return index < mySet.size();
  }

  const SMDS_MeshNode* next()
  {
    index++;
    int num=0;
    if(mySet.size()==6) {
      if(index==2) num=3;
      else if(index==3) num=1;
      else if(index==4) num=4;
      else if(index==5) num=2;
      else if(index==6) num=5;
    }
    else {
      if(index==2) num=4;
      else if(index==3) num=1;
      else if(index==4) num=5;
      else if(index==5) num=2;
      else if(index==6) num=6;
      else if(index==7) num=3;
      else if(index==8) num=7;
    }
    return mySet[num];
  }
};

SMDS_NodeIteratorPtr SMDS_QuadraticFaceOfNodes::interlacedNodesIterator() const
{
  return SMDS_NodeIteratorPtr
    (new SMDS_QuadraticFaceOfNodes_MyInterlacedNodeIterator(myNodes));
}



//=======================================================================
//function : elementsIterator
//purpose  : 
//=======================================================================
class SMDS_QuadraticFaceOfNodes_MyIterator:public SMDS_ElemIterator
{
  const std::vector<const SMDS_MeshNode *> mySet;
  int index;
 public:
  SMDS_QuadraticFaceOfNodes_MyIterator(const std::vector<const SMDS_MeshNode *> s):
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

SMDS_ElemIteratorPtr SMDS_QuadraticFaceOfNodes::elementsIterator
                                         (SMDSAbs_ElementType type) const
{
  switch(type)
  {
  case SMDSAbs_Face:
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Face);
  case SMDSAbs_Node:
    return SMDS_ElemIteratorPtr(new SMDS_QuadraticFaceOfNodes_MyIterator(myNodes));
  case SMDSAbs_Edge:
    MESSAGE("Error : edge iterator for SMDS_QuadraticFaceOfNodes not implemented");
    break;
  default:
    return SMDS_ElemIteratorPtr
      (new SMDS_IteratorOfElements
       (this,type,SMDS_ElemIteratorPtr
        (new SMDS_QuadraticFaceOfNodes_MyIterator(myNodes))));
  }
  return SMDS_ElemIteratorPtr();
}
