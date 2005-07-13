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
//  File   : SMESHGUI_AddSubMeshDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_ADD_SUBMESH_H
#define DIALOGBOX_ADD_SUBMESH_H

#include <SMESHGUI_Dialog.h>

class QLineEdit;

//=================================================================================
// class    : SMESHGUI_AddSubMeshDlg
// purpose  :
//=================================================================================
class SMESHGUI_AddSubMeshDlg : public SMESHGUI_Dialog
{ 
    Q_OBJECT

public:
    enum { MeshObj, GeomObj, Hypo, Algo };
    
public:
    SMESHGUI_AddSubMeshDlg( SMESHGUI* );
    ~SMESHGUI_AddSubMeshDlg();

    void    setSubMeshName( const QString& );
    QString subMeshName() const;

signals:
    void nameChanged( const QString& );

private:
    QLineEdit* myMeshName;

};

#endif // DIALOGBOX_ADD_SUBMESH_H
