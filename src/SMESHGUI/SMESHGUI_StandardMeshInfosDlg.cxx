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
//  File   : SMESHGUI_StandardMeshInfosDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header$

#include <SMESHGUI_StandardMeshInfosDlg.h>

#include <SMESH_Type.h>

#include <qlayout.h>
#include <qgroupbox.h>
#include <qtextbrowser.h>

//=================================================================================
/*!
 *  SMESHGUI_StandardMeshInfosDlg::SMESHGUI_StandardMeshInfosDlg
 *
 *  Constructor
 */
//=================================================================================
SMESHGUI_StandardMeshInfosDlg::SMESHGUI_StandardMeshInfosDlg()
: SMESHGUI_Dialog( false, false, OK )
{
  setCaption(tr("SMESH_STANDARD_MESHINFO_TITLE" ));

  resize( 300, 200 );
  
  // dialog layout
  QVBoxLayout* main = new QVBoxLayout( mainFrame(), 0, 5 );

  // mesh group box
  QGroupBox* MeshGroup = new QGroupBox( mainFrame(), "myMeshGroup");
  MeshGroup->setTitle(tr("SMESH_MESH"));
  MeshGroup->setColumnLayout(3, Qt::Horizontal);
  MeshGroup->layout()->setSpacing(0);
  MeshGroup->layout()->setMargin(5);

  createObject( tr("SMESH_NAME" ), MeshGroup, 0 );
  setNameIndication( 0, OneName );
  setObjectType( 0, prefix( "SMESH" ) + MESH, -1 );

  main->addWidget( MeshGroup );

  // information group box
  QGroupBox* InfoGroup  = new QGroupBox( mainFrame(), "myInfoGroup");
  InfoGroup->setTitle(tr("SMESH_INFORMATION"));
  InfoGroup->setColumnLayout(1, Qt::Vertical);
  InfoGroup->layout()->setSpacing(0);
  InfoGroup->layout()->setMargin(5);

  // information text browser
  myInfo = new QTextBrowser( InfoGroup, "myInfo");

  main->addWidget( InfoGroup );

}

//=================================================================================
/*!
 *  SMESHGUI_StandardMeshInfosDlg::~SMESHGUI_StandardMeshInfosDlg
 *
 *  Destructor
 */
//=================================================================================
SMESHGUI_StandardMeshInfosDlg::~SMESHGUI_StandardMeshInfosDlg()
{
}

//=================================================================================
/*!
 *  SMESHGUI_StandardMeshInfosDlg::setInfo
 *
 *  Set text to text browser
 */
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::setInfo( const QString& text )
{
  myInfo->setText( text );
}
