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
//  File   : SMESHGUI_InitMeshDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_InitMeshDlg.h"
#include <SMESHGUI.h>
#include <SMESHGUI_Operation.h>

#include <SMESH_Type.h>

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>

//=================================================================================
// class    : SMESHGUI_InitMeshDlg()
// purpose  : Constructs a SMESHGUI_InitMeshDlg which is a child of 'parent', with the
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_InitMeshDlg::SMESHGUI_InitMeshDlg( SMESHGUI* theModule )
: SMESHGUI_Dialog( theModule, false, true )
{
    setCaption(tr("SMESH_INIT_MESH"));

    QVBoxLayout* main = new QVBoxLayout( mainFrame() );

    /***************************************************************/
    QGroupBox* GroupC1 = new QGroupBox( tr( "SMESH_ARGUMENTS" ), mainFrame(), "GroupC1" );
    GroupC1->setColumnLayout(3, Qt::Horizontal);
    GroupC1->layout()->setSpacing(6);
    GroupC1->layout()->setMargin(11);
   
    new QLabel(tr("SMESH_NAME"), GroupC1, "TextLabel_NameMesh");
    GroupC1->addSpace(1);
    myMeshName = new QLineEdit(GroupC1, "LineEdit_NameMesh");
    
    createObject( tr("SMESH_OBJECT_GEOM"), GroupC1, GeomObj );
    createObject( tr("SMESH_OBJECT_HYPOTHESIS"), GroupC1, Hypo );
    createObject( tr("SMESH_OBJECT_ALGORITHM"), GroupC1, Algo );

    setNameIndication( GeomObj, OneName );
    
    int _smesh = SMESHGUI_Operation::prefix( "SMESH" );
    setObjectType( Hypo, _smesh + HYPOTHESIS, -1 );
    setObjectType( Algo, _smesh + ALGORITHM, -1 );
    
    main->addWidget( GroupC1 );
}

//=================================================================================
// function : ~SMESHGUI_InitMeshDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_InitMeshDlg::~SMESHGUI_InitMeshDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : updateControlState
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::updateControlState()
{
  bool isEnabled = !meshName().isEmpty() &&
                   hasSelection( GeomObj ) && hasSelection( Hypo ) && hasSelection( Algo );
  setButtonEnabled( isEnabled, OK | Apply );
}

//=================================================================================
// function : meshName
// purpose  :
//=================================================================================
QString SMESHGUI_InitMeshDlg::meshName() const
{
  return myMeshName->text().stripWhiteSpace();
}

//=================================================================================
// function : setMeshName
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::setMeshName( const QString& name )
{
  myMeshName->setText( name.stripWhiteSpace() );
}

