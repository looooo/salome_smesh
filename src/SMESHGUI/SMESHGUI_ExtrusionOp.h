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
//  File   : SMESHGUI_ExtrusionOp.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header: 

#ifndef OPERATION_EXTRUSION_H
#define OPERATION_EXTRUSION_H

#include <SMESHGUI_SelectionOp.h>

class SMESHGUI_ExtrusionDlg;

//=================================================================================
// class    : SMESHGUI_ExtrusionOp
// purpose  :
//=================================================================================
class SMESHGUI_ExtrusionOp : public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_ExtrusionOp();
  ~SMESHGUI_ExtrusionOp();
  
  virtual SalomeApp_Dialog* dlg() const;
  virtual void initDialog();

protected:
  virtual void startOperation();
  virtual SUIT_SelectionFilter* createFilter( const int ) const;

  void updateDialog();
  SMESH::SMESH_Mesh_var extractIds( IdList& ) const;

protected slots:
  virtual bool onApply();
  virtual void onChangeType( int );
  virtual void onSelectionChanged( int );
  virtual void onActivateObject( int );

private:
  SMESHGUI_ExtrusionDlg*      myDlg;
};

#endif // DIALOGBOX_EXTRUSION_H
