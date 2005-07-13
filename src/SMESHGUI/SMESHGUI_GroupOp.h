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
//  File   : SMESHGUI_GroupOp.h
//  Author : Sergey LITONIN
//  Module : SMESH


#ifndef SMESHGUI_GroupOp_H
#define SMESHGUI_GroupOp_H

#include <SMESHGUI_SelectionOp.h>

class SMESHGUI_GroupOpDlg;

/*
  Class       : SMESHGUI_GroupOp
  Description : Perform boolean operations on groups
*/

class SMESHGUI_GroupOp : public SMESHGUI_SelectionOp
{ 
  Q_OBJECT

public:
  enum { UNION, INTERSECT, CUT };
  
public:
  SMESHGUI_GroupOp( const int );
  virtual ~SMESHGUI_GroupOp();
  
  virtual SalomeApp_Dialog* dlg() const;  

protected:
  virtual void startOperation();
  virtual void initDialog();
  virtual void selectionDone();
  
  virtual SUIT_SelectionFilter* createFilter( const int ) const;

  bool isValid() const;
  void updateDialog();
  
protected slots:
  virtual bool onApply();

private slots:
  void onNameChanged( const QString& );

private:
  SMESHGUI_GroupOpDlg  *myDlg;
  int                   myMode;
};

#endif
