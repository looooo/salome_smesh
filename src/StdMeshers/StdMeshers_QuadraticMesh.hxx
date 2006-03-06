//  SMESH StdMeshers : implementaion of SMESH idl descriptions
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
//  File   : StdMeshers_QuadraticMesh.hxx
//  Module : SMESH
//  $Header$

#ifndef _StdMeshers_QuadraticMesh_HXX_
#define _StdMeshers_QuadraticMesh_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

/*!
 * \brief Hypothesis for StdMeshers_Regular_1D, forcing construction of quadratic edges.
 * If the 2D mesher sees that all boundary edges are quadratic ones,
 * it generates quadratic faces, else it generates linear faces using
 * medium nodes as if they were vertex ones.
 * The 3D mesher generates quadratic volumes only if all boundary faces
 * are quadratic ones, else it fails.
 */
class StdMeshers_QuadraticMesh:public SMESH_Hypothesis
{
 public:
  StdMeshers_QuadraticMesh(int hypId, int studyId, SMESH_Gen * gen);
  virtual ~ StdMeshers_QuadraticMesh();
  
  virtual std::ostream & SaveTo(std::ostream & save);
  virtual std::istream & LoadFrom(std::istream & load);
  friend std::ostream & operator <<(std::ostream & save, StdMeshers_QuadraticMesh & hyp);
  friend std::istream & operator >>(std::istream & load, StdMeshers_QuadraticMesh & hyp);
};

#endif
