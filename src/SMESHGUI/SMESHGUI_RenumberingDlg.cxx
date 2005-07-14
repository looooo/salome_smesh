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
//  File   : SMESHGUI_RenumberingDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_RenumberingDlg.h"

#include <SUIT_ResourceMgr.h>
#include <SMESH_Type.h>

#include <qlayout.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

//=================================================================================
// class    : SMESHGUI_RenumberingDlg()
// purpose  :
//=================================================================================
SMESHGUI_RenumberingDlg::SMESHGUI_RenumberingDlg( const int unit )
{
  setName("SMESHGUI_RenumberingDlg");
  resize(303, 185);

  QString cap = unit==0 ? "%1RENUMBERING_NODES%2" : "%1RENUMBERING_ELEMENTS%2";
  QPixmap image0 = resMgr()->loadPixmap( "SMESH", tr( cap.arg( "ICON_DLG_" ).arg( "" ) ) );
  setCaption( tr( cap.arg( "SMESH_" ).arg( "_TITLE" ) ) );
  
  QVBoxLayout* main = new QVBoxLayout( mainFrame() );

  /***************************************************************/
  QButtonGroup* GroupConstructors = new QButtonGroup( mainFrame(), "GroupConstructors");
  GroupConstructors->setTitle( tr( QString( "SMESH_%1" ). arg( unit==0 ? "NODES" : "ELEMENTS" ) ) );
  GroupConstructors->setExclusive(TRUE);
  GroupConstructors->setColumnLayout(0, Qt::Vertical);
  GroupConstructors->layout()->setSpacing(0);
  GroupConstructors->layout()->setMargin(0);
  QGridLayout* GroupConstructorsLayout = new QGridLayout(GroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6);
  GroupConstructorsLayout->setMargin(11);
  QRadioButton* Constructor1 = new QRadioButton(GroupConstructors, "Constructor1");
  Constructor1->setText(tr("" ));
  Constructor1->setPixmap(image0);
  Constructor1->setChecked(TRUE);
  Constructor1->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, Constructor1->sizePolicy().hasHeightForWidth()));
  Constructor1->setMinimumSize(QSize(50, 0));
  GroupConstructorsLayout->addWidget(Constructor1, 0, 0);
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupConstructorsLayout->addItem(spacer, 0, 1);
  
  main->addWidget(GroupConstructors);

  /***************************************************************/
  QGroupBox* GroupMesh = new QGroupBox( mainFrame(), "GroupMesh");
  GroupMesh->setTitle(tr("SMESH_RENUMBERING" ));
  GroupMesh->setMinimumSize(QSize(0, 0));
  GroupMesh->setFrameShape(QGroupBox::Box);
  GroupMesh->setFrameShadow(QGroupBox::Sunken);
  GroupMesh->setColumnLayout( 3, Qt::Horizontal );

  createObject( tr( "SMESH_MESH" ), GroupMesh, 0 );
  setNameIndication( 0, OneName );
  setObjectType( 0, prefix( "SMESH" ) + MESH, -1 );
  
  main->addWidget( GroupMesh );
}

//=================================================================================
// function : ~SMESHGUI_RenumberingDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RenumberingDlg::~SMESHGUI_RenumberingDlg()
{
}
