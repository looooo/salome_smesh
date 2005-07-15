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
//  File   : SMESHGUI_StandardMeshInfos.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header: 

#ifndef SMESHGUI_STANDARDMESHINFOSOP_H
#define SMESHGUI_STANDARDMESHINFOSOP_H

#include <SMESHGUI_SelectionOp.h>

class SMESHGUI_StandardMeshInfosDlg;

class SMESHGUI_StandardMeshInfosOp : public SMESHGUI_SelectionOp
{ 
    Q_OBJECT

public:
    SMESHGUI_StandardMeshInfosOp();
    ~SMESHGUI_StandardMeshInfosOp();

    virtual SalomeApp_Dialog* dlg() const;

protected:
    virtual void startOperation();
    virtual SUIT_SelectionFilter* createFilter( const int ) const;
    void fillMeshInfos();

protected slots:
    virtual void onOk();
    virtual void onSelectionChanged( int );

private:
    SMESHGUI_StandardMeshInfosDlg* myDlg;
};

#endif // SMESHGUI_STANDARDMESHINFOSOP_H
