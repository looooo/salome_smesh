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

#ifndef _INCLUDE_DRIVER_SMDS_MESH
#define _INCLUDE_DRIVER_SMDS_MESH

#include "Driver_Mesh.h"

class SMDS_Mesh;

class Driver_SMDS_Mesh: public Driver_Mesh
{
 public:
  Driver_SMDS_Mesh();
  void SetMesh(SMDS_Mesh *theMesh);
  
 protected:
  SMDS_Mesh *myMesh;

};

#endif
