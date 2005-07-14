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
//  File   : SMESHGUI_MoveNodesOp.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#ifndef OPERATION_MOVE_NODES_H
#define OPERATION_MOVE_NODES_H

#include <SMESHGUI_SelectionOp.h>

class SMESHGUI_MoveNodesDlg;
class SALOME_Actor;

//=================================================================================
// class    : SMESHGUI_MoveNodesOp
// purpose  :
//=================================================================================
class SMESHGUI_MoveNodesOp : public SMESHGUI_SelectionOp
{ 
  Q_OBJECT

public:
  SMESHGUI_MoveNodesOp();
  virtual ~SMESHGUI_MoveNodesOp();

  virtual SalomeApp_Dialog* dlg() const;

protected:
  virtual void startOperation();
  virtual void selectionDone();
  virtual void initDialog();
  virtual void commitOperation();
  virtual void abortOperation();  

  void updateDialog();

protected slots:
  virtual bool onApply();
  virtual void redisplayPreview();

private:
  void erasePreview();
  
private:
  SALOME_Actor*               myPreviewActor;
  SMESHGUI_MoveNodesDlg*      myDlg;  
};

#endif // DIALOGBOX_MOVE_NODES_H
