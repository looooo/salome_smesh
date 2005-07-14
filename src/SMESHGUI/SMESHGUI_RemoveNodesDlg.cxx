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
//  File   : SMESHGUI_RemoveNodesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_RemoveNodesDlg.h"

#include <SUIT_ResourceMgr.h>

#include <SMDSAbs_ElementType.hxx>

#include <qlayout.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

//=================================================================================
// class    : SMESHGUI_RemoveNodesDlg()
// purpose  :
//=================================================================================
SMESHGUI_RemoveNodesDlg::SMESHGUI_RemoveNodesDlg()
: SMESHGUI_Dialog( false, true, OK | Apply | Close )
{
    QPixmap image0( resMgr()->loadPixmap("SMESH", tr("ICON_DLG_REM_NODE")));
    QPixmap image1( resMgr()->loadPixmap("SMESH", tr("ICON_SELECT")));

    setName("SMESHGUI_RemoveNodesDlg");
    resize(303, 185);
    setCaption(tr("SMESH_REMOVE_NODES_TITLE" ));
    setSizeGripEnabled(TRUE);

    QVBoxLayout* main = new QVBoxLayout( mainFrame() );

    /***************************************************************/
    QButtonGroup* GroupConstructors = new QButtonGroup( mainFrame(), "GroupConstructors");
    GroupConstructors->setTitle(tr("SMESH_NODES" ));
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
    QGroupBox* GroupC1 = new QGroupBox( mainFrame(), "GroupC1");
    GroupC1->setTitle(tr("SMESH_REMOVE" ));
    GroupC1->setMinimumSize(QSize(0, 0));
    GroupC1->setFrameShape(QGroupBox::Box);
    GroupC1->setFrameShadow(QGroupBox::Sunken);
    GroupC1->setColumnLayout( 3, Qt::Horizontal );
//    GroupC1->layout()->setSpacing(0);
//    GroupC1->layout()->setMargin(0);

    createObject( tr("SMESH_ID_NODES" ), GroupC1, 0 );
    setNameIndication( 0, ListOfNames );
    setReadOnly( 0, false );
    setObjectType( 0, prefix( "SMESH element" ) + SMDSAbs_Node, -1 );

    main->addWidget(GroupC1);
}

//=================================================================================
// function : ~SMESHGUI_RemoveNodesDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RemoveNodesDlg::~SMESHGUI_RemoveNodesDlg()
{
}
