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
//  File   : SMESHGUI_NodesDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_NODES_H
#define DIALOGBOX_NODES_H

#include <SMESHGUI_Dialog.h>

class SMESHGUI_SpinBox;

//=================================================================================
// class    : SMESHGUI_NodesDlg
// purpose  :
//=================================================================================
class SMESHGUI_NodesDlg : public SMESHGUI_Dialog
{ 
    Q_OBJECT

public:
    SMESHGUI_NodesDlg( SMESHGUI* );
    ~SMESHGUI_NodesDlg();

    void coords( double&, double&, double& ) const;
    void setCoords( const double, const double, const double );
    
signals:
    void valueChanged( double );

private:
    SMESHGUI_SpinBox* SpinBox_X;
    SMESHGUI_SpinBox* SpinBox_Y;
    SMESHGUI_SpinBox* SpinBox_Z;
};

#endif // DIALOGBOX_NODES_H
