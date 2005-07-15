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
//  File   : SMESHGUI_AddSubMeshOp.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef OPERATION_ADD_SUB_MESH_H
#define OPERATION_ADD_SUB_MESH_H

#include <SMESHGUI_SelectionOp.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SMESH_Gen)


class SMESHGUI_AddSubMeshDlg;
class SUIT_SelectionFilter;

//=================================================================================
// class    : SMESHGUI_AddSubMeshOp
// purpose  :
//=================================================================================
class SMESHGUI_AddSubMeshOp : public SMESHGUI_SelectionOp
{
    Q_OBJECT

public:
  SMESHGUI_AddSubMeshOp();
  ~SMESHGUI_AddSubMeshOp();

  virtual SalomeApp_Dialog* dlg() const;
  virtual void              initDialog();
  
protected:
  virtual void startOperation();
  virtual void selectionDone();
  virtual SUIT_SelectionFilter* createFilter( const int ) const;

  void updateDialog();

  SMESH::SMESH_subMesh_var addSubMesh( SMESH::SMESH_Mesh_ptr,
                                       GEOM::GEOM_Object_ptr,
                                       const QString& );

protected slots:
  virtual bool onApply();
  virtual void onSelectionChanged( int );

private slots:
  void onNameChanged( const QString& );

private:
  SMESHGUI_AddSubMeshDlg  *myDlg;
};

#endif // OPERATION_INIT_MESH_H
