//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
// File      : SMESH_Pattern_i.hxx
// Created   : Fri Aug 20 16:03:15 2004
// Author    : Edward AGAPOV (eap)

//  $Header: 

#ifndef SMESH_Pattern_I_HeaderFile
#define SMESH_Pattern_I_HeaderFile

#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Pattern)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_CLIENT_HEADER(GEOM_Gen)

#include <TopoDS_Shape.hxx>
#include "SMESH_Pattern.hxx"

class SMESH_Gen_i;
class SMESH_Mesh;

class SMESH_Pattern_i:
  public virtual POA_SMESH::SMESH_Pattern
{
 public:

  SMESH_Pattern_i (SMESH_Gen_i* theGen_i);

  CORBA::Boolean LoadFromFile(const char* theFileContents);

  CORBA::Boolean LoadFromFace(SMESH::SMESH_Mesh_ptr theMesh,
                              GEOM::GEOM_Object_ptr theFace,
                              CORBA::Boolean        theProject);

  CORBA::Boolean LoadFrom3DBlock(SMESH::SMESH_Mesh_ptr theMesh,
                                 GEOM::GEOM_Object_ptr theBlock);

  SMESH::point_array* ApplyToFace(GEOM::GEOM_Object_ptr theFace,
                                  GEOM::GEOM_Object_ptr theVertexOnKeyPoint1,
                                  CORBA::Boolean        theReverse);

  SMESH::point_array* ApplyTo3DBlock(GEOM::GEOM_Object_ptr theBlock,
                                     GEOM::GEOM_Object_ptr theVertex000,
                                     GEOM::GEOM_Object_ptr theVertex001);

  CORBA::Boolean MakeMesh(SMESH::SMESH_Mesh_ptr theMesh);

  SMESH::SMESH_Pattern::ErrorCode GetErrorCode();

  char* GetString();

  CORBA::Boolean Is2D();

  SMESH::point_array* GetPoints();

  SMESH::long_array* GetKeyPoints();

  SMESH::array_of_long_array* GetElementPoints();


 private:

  TopoDS_Shape getShape( GEOM::GEOM_Object_ptr & theGeomObject );

  ::SMESH_Mesh* getMesh( SMESH::SMESH_Mesh_ptr & theMesh );

  ::SMESH_Pattern myPattern;

  SMESH_Gen_i*    myGen;
};
#endif
