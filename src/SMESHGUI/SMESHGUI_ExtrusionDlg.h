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
//  File   : SMESHGUI_ExtrusionDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef DIALOGBOX_EXTRUSION_H
#define DIALOGBOX_EXTRUSION_H

#include <SMESHGUI_Dialog.h>

class QButtonGroup;
class QCheckBox;
class QSpinBox;
class QGroupBox;
class SMESHGUI_SpinBox;

//=================================================================================
// class    : SMESHGUI_ExtrusionDlg
// purpose  :
//=================================================================================
class SMESHGUI_ExtrusionDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_ExtrusionDlg();
  ~SMESHGUI_ExtrusionDlg();

  void init();
  int  constructorId() const;
  void coords( double&, double&, double& ) const;
  int  nbStep() const;
  bool isSelectMesh() const;

signals:
  void constructorClicked( int );
  
private slots:
  void onConstructorsClicked( int );
  void onSelectMesh( bool );

private:
  QButtonGroup*     myGroupConstructors;
  QGroupBox*        myGroupArguments;

  QCheckBox*        myCheckBoxMesh;
  SMESHGUI_SpinBox *myDx, *myDy, *myDz;
  QSpinBox         *myNbSteps;
};

#endif // DIALOGBOX_EXTRUSION_H
