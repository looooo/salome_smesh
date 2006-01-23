// File:      SMDS_QuadraticEdge.cxx
// Created:   16.01.06 16:25:42
// Author:    Sergey KUUL
// Copyright: Open CASCADE 2006


#include "SMDS_QuadraticEdge.hxx"

#include "SMDS_IteratorOfElements.hxx"
#include "SMDS_MeshNode.hxx"

using namespace std;

//=======================================================================
//function : SMDS_QuadraticEdge
//purpose  : 
//=======================================================================

SMDS_QuadraticEdge::SMDS_QuadraticEdge(const SMDS_MeshNode * node1,
                                       const SMDS_MeshNode * node2,
                                       const SMDS_MeshNode * node12)
     :SMDS_MeshEdge(node1,node2)
{	
  myNodes[2]=node12;
}


//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_QuadraticEdge::Print(ostream & OS) const
{
  OS << "quadratic edge <" << GetID() << "> : ( first-" << myNodes[0]
     << " , last-" << myNodes[1] << " , medium-" << myNodes[2] << ") " << endl;
}


//=======================================================================
//function : NbNodes
//purpose  : 
//=======================================================================

int SMDS_QuadraticEdge::NbNodes() const
{
  return 3;
}


//=======================================================================
//function : Iterator
//purpose  : 
//=======================================================================

class SMDS_QuadraticEdge_MyNodeIterator:public SMDS_ElemIterator
{
  const SMDS_MeshNode *const* myNodes;
  int myIndex;
 public:
  SMDS_QuadraticEdge_MyNodeIterator(const SMDS_MeshNode * const* nodes):
    myNodes(nodes),myIndex(0) {}

  bool more()
  {
    return myIndex<3;
  }

  const SMDS_MeshElement* next()
  {
    myIndex++;
    return myNodes[myIndex-1];
  }
};

SMDS_ElemIteratorPtr SMDS_QuadraticEdge::
	elementsIterator(SMDSAbs_ElementType type) const
{
  switch(type)
  {
  case SMDSAbs_Edge:
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Edge); 
  case SMDSAbs_Node:
    return SMDS_ElemIteratorPtr(new SMDS_QuadraticEdge_MyNodeIterator(myNodes));
  default:
    return SMDS_ElemIteratorPtr
      (new SMDS_IteratorOfElements
       (this,type, SMDS_ElemIteratorPtr(new SMDS_QuadraticEdge_MyNodeIterator(myNodes))));
  }
}


//=======================================================================
//function : ChangeNodes
//purpose  : 
//=======================================================================

bool SMDS_QuadraticEdge::ChangeNodes(const SMDS_MeshNode * node1,
                                     const SMDS_MeshNode * node2,
                                     const SMDS_MeshNode * node12)
{
  myNodes[0]=node1;
  myNodes[1]=node2;
  myNodes[2]=node12;
  return true;
}


//=======================================================================
//function : IsMediumNode
//purpose  : 
//=======================================================================

bool SMDS_QuadraticEdge::IsMediumNode(const SMDS_MeshNode * node) const
{
  return (myNodes[2]==node);
}



