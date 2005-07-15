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
//  File   : SMESHGUI_NodesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_NodesDlg.h"
#include <SMESHGUI.h>
#include <SMESHGUI_SpinBox.h>

#include <SUIT_ResourceMgr.h>

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qradiobutton.h>

//=================================================================================
// class    : SMESHGUI_NodesDlg()
// purpose  :
//=================================================================================
SMESHGUI_NodesDlg::SMESHGUI_NodesDlg()
: SMESHGUI_Dialog( 0, false, false, OK | Apply | Close )
{
  QPixmap image0( resMgr()->loadPixmap("SMESH", tr("ICON_DLG_NODE")));
  setName("SMESHGUI_NodesDlg");
  resize(303, 185);
  setCaption(tr("MESH_NODE_TITLE"));
  setSizeGripEnabled(TRUE);
  QGridLayout* SMESHGUI_NodesDlgLayout = new QGridLayout(mainFrame());
  SMESHGUI_NodesDlgLayout->setSpacing(0);
  SMESHGUI_NodesDlgLayout->setMargin(0);

  /***************************************************************/
  QButtonGroup* GroupConstructors = new QButtonGroup(mainFrame(), "GroupConstructors");
  GroupConstructors->setTitle(tr("MESH_NODE" ));
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
  GroupConstructorsLayout->addWidget(Constructor1, 0, 0);
  QSpacerItem* spacer_2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupConstructorsLayout->addItem(spacer_2, 0, 1);
  SMESHGUI_NodesDlgLayout->addWidget(GroupConstructors, 0, 0);

  /***************************************************************/
  QGroupBox* GroupCoordinates = new QGroupBox(mainFrame(), "GroupCoordinates");
  GroupCoordinates->setTitle(tr("SMESH_COORDINATES" ));
  GroupCoordinates->setColumnLayout(0, Qt::Vertical);
  GroupCoordinates->layout()->setSpacing(0);
  GroupCoordinates->layout()->setMargin(0);
  QGridLayout* GroupCoordinatesLayout = new QGridLayout(GroupCoordinates->layout());
  GroupCoordinatesLayout->setAlignment(Qt::AlignTop);
  GroupCoordinatesLayout->setSpacing(6);
  GroupCoordinatesLayout->setMargin(11);
  QLabel* TextLabel_X = new QLabel(GroupCoordinates, "TextLabel_X");
  TextLabel_X->setText(tr("SMESH_X" ));
  GroupCoordinatesLayout->addWidget(TextLabel_X, 0, 0);
  QLabel* TextLabel_Y = new QLabel(GroupCoordinates, "TextLabel_Y");
  TextLabel_Y->setText(tr("SMESH_Y" ));
  GroupCoordinatesLayout->addWidget(TextLabel_Y, 0, 2);

  QLabel* TextLabel_Z = new QLabel(GroupCoordinates, "TextLabel_Z");
  TextLabel_Z->setText(tr("SMESH_Z" ));
  GroupCoordinatesLayout->addWidget(TextLabel_Z, 0, 4);

  SpinBox_X = new SMESHGUI_SpinBox(GroupCoordinates, "SpinBox_X");
  GroupCoordinatesLayout->addWidget(SpinBox_X, 0, 1);

  SpinBox_Y = new SMESHGUI_SpinBox(GroupCoordinates, "SpinBox_Y");
  GroupCoordinatesLayout->addWidget(SpinBox_Y, 0, 3);

  SpinBox_Z = new SMESHGUI_SpinBox(GroupCoordinates, "SpinBox_Z");
  GroupCoordinatesLayout->addWidget(SpinBox_Z, 0, 5);

  SpinBox_X->RangeStepAndValidator(-999.999, +999.999, 25.0, 3);
  SpinBox_Y->RangeStepAndValidator(-999.999, +999.999, 25.0, 3);
  SpinBox_Z->RangeStepAndValidator(-999.999, +999.999, 25.0, 3);
  
  connect(SpinBox_X, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged(double)));
  connect(SpinBox_Y, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged(double)));
  connect(SpinBox_Z, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged(double)));
  
  SMESHGUI_NodesDlgLayout->addWidget(GroupCoordinates, 1, 0);

  setFocusProxy( SpinBox_X );
}

//=======================================================================
// function : ~SMESHGUI_NodesDlg()
// purpose  : Destructor
//=======================================================================
SMESHGUI_NodesDlg::~SMESHGUI_NodesDlg()
{
}

//=======================================================================
// function : coords()
// purpose  : 
//=======================================================================
void SMESHGUI_NodesDlg::coords( double& x, double& y, double& z ) const
{
  x = SpinBox_X->value();
  y = SpinBox_Y->value();
  z = SpinBox_Z->value();
}

//=======================================================================
// function : setCoords()
// purpose  :
//=======================================================================
void SMESHGUI_NodesDlg::setCoords( const double x, const double y, const double z )
{
  SpinBox_X->setValue( x );
  SpinBox_Y->setValue( y );
  SpinBox_Z->setValue( z );
}
