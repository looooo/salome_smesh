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
//  File   : SMESHGUI_RemoveOp.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef OPERATION_REMOVE_H
#define OPERATION_REMOVE_H

#include <SMESHGUI_SelectionOp.h>

class SMESHGUI_RemoveDlg;

//=================================================================================
// class    : SMESHGUI_RemoveOp
// purpose  :
//=================================================================================
class SMESHGUI_RemoveOp : public SMESHGUI_SelectionOp
{
    Q_OBJECT

public:
    SMESHGUI_RemoveOp( const bool elems );
    ~SMESHGUI_RemoveOp();

    virtual SalomeApp_Dialog* dlg() const;
    virtual void initDialog();

protected:
    virtual void startOperation();
    virtual void selectionDone();

    void updateDialog();

protected slots:
    virtual bool onApply();

private:
    SMESHGUI_RemoveDlg*      myDlg;
    bool                     myIsElem;
};

#endif // DIALOGBOX_REMOVE_H
