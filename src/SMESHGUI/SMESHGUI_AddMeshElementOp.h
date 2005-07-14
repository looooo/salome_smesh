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
//  File   : SMESHGUI_AddMeshElementOp.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef OPERATION_ADD_FACE_H
#define OPERATION_ADD_FACE_H

namespace SMESH{
  struct TElementSimulation;
}

#include <SMESHGUI_SelectionOp.h>
#include <SMDSAbs_ElementType.hxx>

class SMESHGUI_AddMeshElementDlg;

//=================================================================================
// class    : SMESHGUI_AddMeshElementOp
// purpose  :
//=================================================================================
class SMESHGUI_AddMeshElementOp : public SMESHGUI_SelectionOp
{ 
    Q_OBJECT

public:
    SMESHGUI_AddMeshElementOp( const SMDSAbs_ElementType, const int );
    ~SMESHGUI_AddMeshElementOp();

    virtual SalomeApp_Dialog* dlg() const;

protected:
    virtual void startOperation();
    virtual void commitOperation();
    virtual void abortOperation();

    void updateDialog();
    
protected slots:
    virtual bool onApply();
    virtual void onSelectionChanged( int );

    void onReverse( int );

private:
    void displaySimulation();

private:
    int                         myElementType;
    int                         myNbNodes;
    bool                        myIsPoly;

    SMESH::TElementSimulation*  mySimulation;

    SMESHGUI_AddMeshElementDlg* myDlg;
};

#endif // DIALOGBOX_ADD_FACE_H
