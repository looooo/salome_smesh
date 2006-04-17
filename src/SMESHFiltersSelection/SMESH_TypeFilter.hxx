// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
//  File   : SMESH_TypeFilter.hxx
//  Module : SMESH

#ifndef _SMESH_TypeFilter_HeaderFile
#define _SMESH_TypeFilter_HeaderFile

#include "SMESH_Type.h"
#include "SUIT_SelectionFilter.h"

class SUIT_DataOwner;

class SMESHFILTERSSELECTION_EXPORT SMESH_TypeFilter : public SUIT_SelectionFilter
{
public:
  SMESH_TypeFilter (MeshObjectType theType);
  ~SMESH_TypeFilter();

  virtual bool isOk (const SUIT_DataOwner*) const;
  MeshObjectType type() const;

protected:
  MeshObjectType myType;
};

#endif
