// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESHDS : management of mesh data and SMESH document
//  File   : SMESHDS_Command.hxx
//  Module : SMESH
//
#ifndef _SMESHDS_Command_HeaderFile
#define _SMESHDS_Command_HeaderFile

#include "SMESH_SMESHDS.hxx"

#include "SMESHDS_CommandType.hxx"
#include <smIdType.hxx>
#include <list>
#include <vector>

class SMESHDS_EXPORT SMESHDS_Command
{

  public:
        SMESHDS_Command(const SMESHDS_CommandType aType);
        void AddNode(int NewNodeID, double x, double y, double z);
        void Add0DElement(int New0DElementID, int idnode);
        void AddEdge(int NewEdgeID, int idnode1, int idnode2);
        void AddFace(int NewFaceID, int idnode1, int idnode2, int idnode3);
        void AddFace(int NewFaceID, int idnode1, int idnode2, int idnode3,
                     int idnode4);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4, smIdType idnode5);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4, smIdType idnode5, smIdType idnode6);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4, smIdType idnode5, smIdType idnode6, smIdType idnode7, smIdType idnode8);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4, smIdType idnode5, smIdType idnode6, smIdType idnode7, smIdType idnode8,
                       smIdType idnode9, smIdType idnode10, smIdType idnode11, smIdType idnode12);
        void AddPolygonalFace (const smIdType               ElementID,
                               const std::vector<smIdType>& nodes_ids);
        void AddQuadPolygonalFace (const smIdType               ElementID,
                                   const std::vector<smIdType>& nodes_ids);
        void AddPolyhedralVolume (const smIdType               ElementID,
                                  const std::vector<smIdType>& nodes_ids,
                                  const std::vector<smIdType>& quantities);
        void AddBall(int NewBallID, int node, double diameter);
        // special methods for quadratic elements
        void AddEdge(int NewEdgeID, int n1, int n2, int n12);
        void AddFace(int NewFaceID, int n1, int n2, int n3,
                     int n12, int n23, int n31);
        void AddFace(int NewFaceID, int n1, int n2, int n3,
                     int n12, int n23, int n31, int nCenter);
        void AddFace(int NewFaceID, int n1, int n2, int n3, int n4,
                     int n12, int n23, int n34, int n41);
        void AddFace(int NewFaceID, int n1, int n2, int n3, int n4,
                     int n12, int n23, int n34, int n41, int nCenter);
        void AddVolume(int NewVolID, int n1, int n2, int n3, int n4,
                       int n12, int n23, int n31, int n14, int n24, int n34);
        void AddVolume(int NewVolID, int n1, int n2, int n3, int n4, int n5,
                       int n12, int n23, int n34, int n41,
                       int n15, int n25, int n35, int n45);
        void AddVolume(int NewVolID, int n1, int n2, int n3,
                       int n4, int n5, int n6,
                       int n12, int n23, int n31,
                       int n45, int n56, int n64,
                       int n14, int n25, int n36);
        void AddVolume(int NewVolID, int n1, int n2, int n3,
                       int n4, int n5, int n6,
                       int n12, int n23, int n31,
                       int n45, int n56, int n64,
                       int n14, int n25, int n36,
                       int n1245, int n2356, int n1346);
        void AddVolume(int NewVolID, int n1, int n2, int n3, int n4,
                       int n5,  int n6,  int n7,  int n8,
                       int n12, int n23, int n34, int n41,
                       int n56, int n67, int n78, int n85,
                       int n15, int n26, int n37, int n48);
        void AddVolume(int NewVolID, int n1, int n2, int n3, int n4,
                       int n5,  int n6,  int n7,  int n8,
                       int n12, int n23, int n34, int n41,
                       int n56, int n67, int n78, int n85,
                       int n15, int n26, int n37, int n48,
                       int n1234,int n1256,int n2367,int n3478,
                       int n1458,int n5678,int nCenter);
        
        void MoveNode(int NewNodeID, double x, double y, double z);
        void RemoveNode(int NodeID);
        void RemoveElement(int ElementID);
        void ChangeElementNodes(int ElementID, int nodes[], int nbnodes);
        void ChangePolyhedronNodes(const int               ElementID,
                                   const std::vector<int>& nodes_ids,
                                   const std::vector<int>& quantities);
        void Renumber (const bool isNodes, const int startID, const int deltaID);
        SMESHDS_CommandType GetType();
        int GetNumber();
        const std::list<int> & GetIndexes();
        const std::list<double> & GetCoords();
         ~SMESHDS_Command();
  private:
        SMESHDS_CommandType myType;
        int myNumber;
        std::list<double> myReals;
        std::list<int> myIntegers;
};
#endif
