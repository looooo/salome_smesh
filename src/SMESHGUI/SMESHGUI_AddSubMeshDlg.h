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

#include "SALOME_Selection.h"
#include "SALOME_TypeFilter.hxx"
#include "SMESH_TypeFilter.hxx"

// QT Includes
#include <qvariant.h>
#include <qdialog.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(GEOM_Shape)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class SMESHGUI;


//=================================================================================
// class    : SMESHGUI_AddSubMeshDlg
// purpose  :
//=================================================================================
class SMESHGUI_AddSubMeshDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_AddSubMeshDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_AddSubMeshDlg();

private:

    void Init( SALOME_Selection* Sel ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;

    SMESHGUI*                     mySMESHGUI ;
    SALOME_Selection*             mySelection ;
             
    GEOM::GEOM_Shape_var                myGeomShape ;
    int                           myConstructorId ; 
    QLineEdit*                    myEditCurrentArgument; 

    SMESH::SMESH_Mesh_var         myMesh;
    SMESH::SMESH_subMesh_var      mySubMesh;

    QString                       myNameSubMesh ;

    Handle(SALOME_TypeFilter)     myGeomFilter;
    Handle(SMESH_TypeFilter)      myMeshFilter;
    Handle(SMESH_TypeFilter)      myHypothesisFilter;
    Handle(SMESH_TypeFilter)      myAlgorithmFilter;

    SALOME_ListIO                 HypoList;
    SALOME_ListIO                 AlgoList;

    bool                          myOkHypothesis;
    bool                          myOkAlgorithm;

    SMESH::SMESH_Hypothesis_var   myHypothesis;
    SMESH::SMESH_Hypothesis_var   myAlgorithm;

    QButtonGroup* GroupConstructors;
    QRadioButton* Constructor1;
    QGroupBox*    GroupButtons;
    QPushButton*  buttonOk;
    QPushButton*  buttonCancel;
    QPushButton*  buttonApply;
    QGroupBox*    GroupC1;
    QLabel*       TextLabel_NameMesh ;
    QLineEdit*    LineEdit_NameMesh ;
    QLabel*       TextLabelC1A1;
    QPushButton*  SelectButtonC1A1;
    QLineEdit*    LineEditC1A1;
    QLabel*       TextLabelC1A2;
    QPushButton*  SelectButtonC1A2;
    QLineEdit*    LineEditC1A2;

    QLabel*       TextLabelC1A1Hyp;
    QPushButton*  SelectButtonC1A1Hyp;
    QLineEdit*    LineEditC1A1Hyp;

    QLabel*       TextLabelC1A1Algo;
    QPushButton*  SelectButtonC1A1Algo;
    QLineEdit*    LineEditC1A1Algo;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void TextChangedInLineEdit(const QString& newText) ;

protected:
    QGridLayout* SMESHGUI_AddSubMeshDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupC1Layout;
};

#endif // DIALOGBOX_ADD_SUBMESH_H
