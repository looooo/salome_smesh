//  SMESH SMESHGUI : GUI for SMESH component
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
//  File   : SMESHGUI_ExportMeshOp.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#ifndef OPERATION_EXPORT_MESH_H
#define OPERATION_EXPORT_MESH_H

#include <SMESHGUI_Operation.h>

class SMESHGUI_ExportMeshOp : public SMESHGUI_Operation
{
public:
  typedef enum
  {
    MED, DAT, UNV
    
  } Type;
  
public:
  SMESHGUI_ExportMeshOp( const Type );
  ~SMESHGUI_ExportMeshOp();

protected:
  virtual void startOperation();

private:
  Type myType;
};

#endif
