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
//  File   : SMESHGUI_EditHypothesesOp.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef OPERATION_EDIT_HYPOTHESES_H
#define OPERATION_EDIT_HYPOTHESES_H

#include <SMESHGUI_SelectionOp.h>

class SMESHGUI_EditHypothesesDlg;

//=================================================================================
// class    : SMESHGUI_EditHypothesesOp
// purpose  :
//=================================================================================
class SMESHGUI_EditHypothesesOp : public SMESHGUI_SelectionOp
{
    Q_OBJECT

public:
  SMESHGUI_EditHypothesesOp();
  ~SMESHGUI_EditHypothesesOp();

  virtual SalomeApp_Dialog* dlg() const;
  virtual void              initDialog();

protected:
  virtual void startOperation();
  virtual void selectionDone();
  virtual SUIT_SelectionFilter* createFilter( const int ) const;
  void updateDialog();

protected slots:
  virtual bool onApply();

private slots:
  void onSelectionChanged( int );
  void onUpdate();

private:
  void initGeom();
  void initHypDefinition();
  void initAlgoDefinition();
  void initHypAssignation();
  void initAlgoAssignation();
  bool storeMeshOrSubMesh();

private:
  SMESHGUI_EditHypothesesDlg   *myDlg;
  bool                          myImportedMesh;
};

#endif
