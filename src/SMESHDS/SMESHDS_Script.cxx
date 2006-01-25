//  SMESH SMESHDS : management of mesh data and SMESH document
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
//  File   : SMESH_Script.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//  $Header: 

#include "SMESHDS_Script.hxx"

using namespace std;

//=======================================================================
//function : getCommand
//purpose  : 
//=======================================================================
SMESHDS_Command* SMESHDS_Script::getCommand(const SMESHDS_CommandType aType)
{
  SMESHDS_Command* com;
  if (myCommands.empty())
  {
    com = new SMESHDS_Command(aType);
    myCommands.insert(myCommands.end(),com);
  }
  else
  {
    com = myCommands.back();
    if (com->GetType() != aType)
    {
      com = new SMESHDS_Command(aType);
      myCommands.insert(myCommands.end(),com);
    }
  }
  return com;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddNode(int NewNodeID, double x, double y, double z)
{
  getCommand(SMESHDS_AddNode)->AddNode(NewNodeID, x, y, z);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddEdge(int NewEdgeID, int idnode1, int idnode2)
{
  getCommand(SMESHDS_AddEdge)->AddEdge(NewEdgeID, idnode1, idnode2);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(int NewFaceID,
                             int idnode1, int idnode2, int idnode3)
{
  getCommand(SMESHDS_AddTriangle)->AddFace(NewFaceID,
                                           idnode1, idnode2, idnode3);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(int NewFaceID,
                             int idnode1, int idnode2,
                             int idnode3, int idnode4)
{
  getCommand(SMESHDS_AddQuadrangle)->AddFace(NewFaceID,
                                             idnode1, idnode2,
                                             idnode3, idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
                               int idnode1, int idnode2,
                               int idnode3, int idnode4)
{
  getCommand(SMESHDS_AddTetrahedron)->AddVolume(NewID,
                                                idnode1, idnode2,
                                                idnode3, idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
                               int idnode1, int idnode2,
                               int idnode3, int idnode4, int idnode5)
{
  getCommand(SMESHDS_AddPyramid)->AddVolume(NewID,
                                            idnode1, idnode2,
                                            idnode3, idnode4, idnode5);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
                               int idnode1, int idnode2, int idnode3,
                               int idnode4, int idnode5, int idnode6)
{
  getCommand(SMESHDS_AddPrism)->AddVolume(NewID,
                                          idnode1, idnode2, idnode3,
                                          idnode4, idnode5, idnode6);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
                               int idnode1, int idnode2, int idnode3, int idnode4,
                               int idnode5, int idnode6, int idnode7, int idnode8)
{
  getCommand(SMESHDS_AddHexahedron)->AddVolume(NewID,
                                               idnode1, idnode2, idnode3, idnode4,
                                               idnode5, idnode6, idnode7, idnode8);
}

//=======================================================================
//function : AddPolygonalFace
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddPolygonalFace (int NewFaceID, std::vector<int> nodes_ids)
{
  getCommand(SMESHDS_AddPolygon)->AddPolygonalFace(NewFaceID, nodes_ids);
}

//=======================================================================
//function : AddPolyhedralVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddPolyhedralVolume (int NewID,
                                          std::vector<int> nodes_ids,
                                          std::vector<int> quantities)
{
  getCommand(SMESHDS_AddPolyhedron)->AddPolyhedralVolume
    (NewID, nodes_ids, quantities);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::MoveNode(int NewNodeID, double x, double y, double z)
{
  getCommand(SMESHDS_MoveNode)->MoveNode(NewNodeID, x, y, z);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::RemoveNode(int ID)
{
  getCommand(SMESHDS_RemoveNode)->RemoveNode(ID);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::RemoveElement(int ElementID)
{
  getCommand(SMESHDS_RemoveElement)->RemoveElement(ElementID);
}

//=======================================================================
//function : ChangeElementNodes
//purpose  : 
//=======================================================================

void SMESHDS_Script::ChangeElementNodes(int ElementID, int nodes[], int nbnodes)
{
  getCommand(SMESHDS_ChangeElementNodes)->ChangeElementNodes( ElementID, nodes, nbnodes );
}

//=======================================================================
//function : ChangePolyhedronNodes
//purpose  : 
//=======================================================================
void SMESHDS_Script::ChangePolyhedronNodes (const int        ElementID,
                                            std::vector<int> nodes_ids,
                                            std::vector<int> quantities)
{
  getCommand(SMESHDS_ChangePolyhedronNodes)->ChangePolyhedronNodes
    (ElementID, nodes_ids, quantities);
}

//=======================================================================
//function : Renumber
//purpose  : 
//=======================================================================

void SMESHDS_Script::Renumber (const bool isNodes, const int startID, const int deltaID)
{
  getCommand(SMESHDS_Renumber)->Renumber( isNodes, startID, deltaID );
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::Clear()
{
	myCommands.clear();
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
const list<SMESHDS_Command*>& SMESHDS_Script::GetCommands()
{
	return myCommands;
}


//********************************************************************
//*****             Methods for quadratic elements              ******
//********************************************************************

//=======================================================================
//function : AddEdge
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddEdge(int NewEdgeID, int n1, int n2, int n12)
{
  getCommand(SMESHDS_AddQuadEdge)->AddEdge(NewEdgeID, n1, n2, n12);
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(int NewFaceID, int n1, int n2, int n3,
                             int n12, int n23, int n31)
{
  getCommand(SMESHDS_AddQuadTriangle)->AddFace(NewFaceID, n1, n2, n3,
                                               n12, n23, n31);
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(int NewFaceID, int n1, int n2, int n3, int n4,
                             int n12, int n23, int n34, int n41)
{
  getCommand(SMESHDS_AddQuadQuadrangle)->AddFace(NewFaceID, n1, n2, n3, n4,
                                                 n12, n23, n34, n41);
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewVolID, int n1, int n2, int n3, int n4,
                               int n12, int n23, int n31,
                               int n14, int n24, int n34)
{
  getCommand(SMESHDS_AddQuadTetrahedron)->AddVolume(NewVolID, n1, n2, n3, n4,
                                                    n12, n23, n31,
                                                    n14, n24, n34);
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewVolID, int n1, int n2, int n3, int n4,
                               int n5, int n12, int n23, int n34, int n41,
                               int n15, int n25, int n35, int n45)
{
  getCommand(SMESHDS_AddQuadPyramid)->AddVolume(NewVolID, n1, n2, n3, n4, n5,
                                                n12, n23, n34, n41,
                                                n15, n25, n35, n45);
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewVolID, int n1, int n2, int n3, int n4,
                                int n5,int n6, int n12, int n23, int n31,
                                int n45, int n56, int n64,
                                int n14, int n25, int n36)
{
  getCommand(SMESHDS_AddQuadPentahedron)->AddVolume(NewVolID, n1,n2,n3,n4,n5,n6,
                                                    n12, n23, n31,
                                                    n45, n56, n64,
                                                    n14, n25, n36);
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewVolID, int n1, int n2, int n3,
                               int n4, int n5, int n6, int n7, int n8,
                               int n12, int n23, int n34, int n41,
                               int n56, int n67, int n78, int n85,
                               int n15, int n26, int n37, int n48)
{
  getCommand(SMESHDS_AddQuadHexahedron)->AddVolume(NewVolID, n1, n2, n3, n4,
                                                   n5, n6, n7, n8,
                                                   n12, n23, n34, n41,
                                                   n56, n67, n78, n85,
                                                   n15, n26, n37, n48);
}

