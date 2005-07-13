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
//  File   : SMESHGUI_GroupOpDlg.h
//  Author : Sergey LITONIN
//  Module : SMESH


#ifndef SMESHGUI_GroupOpDlg_H
#define SMESHGUI_GroupOpDlg_H

#include <SMESHGUI_Dialog.h>

/*
  Class       : SMESHGUI_GroupOpDlg
  Description : Perform boolean operations on groups
*/

class SMESHGUI;
class QLineEdit;

class SMESHGUI_GroupOpDlg : public SMESHGUI_Dialog
{ 
  Q_OBJECT

public:
  SMESHGUI_GroupOpDlg( SMESHGUI*, const int );
  virtual ~SMESHGUI_GroupOpDlg();

  void    setName( const QString& );
  QString name() const;

signals:
  void nameChanged( const QString& );

private:
  QLineEdit*                myNameEdit;
};

#endif
