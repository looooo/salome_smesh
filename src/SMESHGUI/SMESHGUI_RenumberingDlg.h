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
//  File   : SMESHGUI_RenumberingDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header:

#ifndef DIALOGBOX_RENUMBERING_H
#define DIALOGBOX_RENUMBERING_H

#include <SMESHGUI_Dialog.h>

//=================================================================================
// class    : SMESHGUI_RenumberingDlg
// purpose  : If the unit == 0 nodes will be renumbered, if the unit == 1 the elements will.
//=================================================================================
class SMESHGUI_RenumberingDlg : public SMESHGUI_Dialog
{ 
    Q_OBJECT

public:
    SMESHGUI_RenumberingDlg( const int );
    ~SMESHGUI_RenumberingDlg();
};

#endif // DIALOGBOX_RENUMBERING_H
