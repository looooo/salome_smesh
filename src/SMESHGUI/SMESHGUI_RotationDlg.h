//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_RotationDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_ROTATIONDLG_H
#define SMESHGUI_ROTATIONDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_IdValidator;
class SMESHGUI_SpinBox;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SMESH_LogicalFilter;

//=================================================================================
// class    : SMESHGUI_RotationDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_RotationDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_RotationDlg( SMESHGUI* );
  ~SMESHGUI_RotationDlg();

private:
  void                   Init( bool = true );
  void                   closeEvent( QCloseEvent* );
  void                   enterEvent( QEvent* );         /* mouse enter the QWidget */
  void                   hideEvent( QHideEvent* );      /* ESC key */
  void                   keyPressEvent( QKeyEvent* );
  bool                   IsAxisOk();
  void                   setNewMeshName();
  
  SMESHGUI*              mySMESHGUI;              /* Current SMESHGUI object */
  SMESHGUI_IdValidator*  myIdValidator;
  LightApp_SelectionMgr* mySelectionMgr;          /* User shape selection */
  int                    myNbOkElements;          /* to check when elements are defined */
  QString                myElementsId;
  SVTK_Selector*         mySelector;
  
  QWidget*               myEditCurrentArgument;   /* Current  LineEdit */
  int                    myConstructorId;
  bool                   myBusy;
  SMESH::SMESH_Mesh_var  myMesh;
  SMESH_Actor*           myActor;
  SMESH_LogicalFilter*   myMeshOrSubMeshOrGroupFilter;
  
  QGroupBox*             GroupConstructors;
  QRadioButton*          RadioButton1;
  QGroupBox*             GroupButtons;
  QPushButton*           buttonOk;
  QPushButton*           buttonCancel;
  QPushButton*           buttonApply;
  QPushButton*           buttonHelp;
  QGroupBox*             GroupArguments;
  QGroupBox*             GroupAxis;
  QLabel*                TextLabelElements;
  QPushButton*           SelectElementsButton;
  QLineEdit*             LineEditElements;
  QCheckBox*             CheckBoxMesh;
  
  QLabel*                TextLabelPoint;
  QPushButton*           SelectPointButton;
  QLabel*                TextLabelX;
  SMESHGUI_SpinBox*      SpinBox_X;
  QLabel*                TextLabelY;
  SMESHGUI_SpinBox*      SpinBox_Y;
  QLabel*                TextLabelZ;
  SMESHGUI_SpinBox*      SpinBox_Z;
  QLabel*                TextLabelVector;
  QPushButton*           SelectVectorButton;
  QLabel*                TextLabelDX;
  SMESHGUI_SpinBox*      SpinBox_DX;
  QLabel*                TextLabelDY;
  SMESHGUI_SpinBox*      SpinBox_DY;
  QLabel*                TextLabelDZ;
  SMESHGUI_SpinBox*      SpinBox_DZ;

  QLabel*                TextLabelAngle;
  SMESHGUI_SpinBox*      SpinBox_Angle;
  //QCheckBox* CheckBoxCopy;
  QGroupBox*             ActionBox;
  QButtonGroup*          ActionGroup;
  QCheckBox*             MakeGroupsCheck;
  QLineEdit*             LineEditNewMesh;

  QString                myHelpFileName;

private slots:
  void                   ClickOnOk();
  void                   ClickOnCancel();
  void                   ClickOnApply();
  void                   ClickOnHelp();
  void                   SetEditCurrentArgument();
  void                   SelectionIntoArgument();
  void                   DeactivateActiveDialog();
  void                   ActivateThisDialog();
  void                   onTextChange( const QString& );
  void                   onSelectMesh( bool );
  void                   onVectorChanged();
  void                   onActionClicked( int );
};

#endif // SMESHGUI_ROTATIONDLG_H
