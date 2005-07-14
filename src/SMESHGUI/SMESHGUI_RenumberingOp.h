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
//  File   : SMESHGUI_RenumberingOp.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header:

#ifndef OPERATION_RENUMBERING_H
#define OPERATION_RENUMBERING_H

#include <SMESHGUI_SelectionOp.h>

class SMESHGUI_RenumberingDlg;

//=================================================================================
// class    : SMESHGUI_RenumberingOp
// purpose  : If the unit == 0 nodes will be renumbered, if the unit == 1 the elements will.
//=================================================================================
class SMESHGUI_RenumberingOp : public SMESHGUI_SelectionOp
{ 
  Q_OBJECT

public:
  SMESHGUI_RenumberingOp( const int );
  ~SMESHGUI_RenumberingOp();

  virtual SalomeApp_Dialog* dlg() const;

protected:
  virtual void startOperation();
  virtual void selectionDone();
  virtual SUIT_SelectionFilter* createFilter( const int ) const;

  void updateDialog();

protected slots:
  virtual bool onApply();

private:
  int                       myMode;
  SMESHGUI_RenumberingDlg*  myDlg;
};

#endif // DIALOGBOX_RENUMBERING_H
