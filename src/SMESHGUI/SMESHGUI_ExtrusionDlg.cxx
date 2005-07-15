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
//  File   : SMESHGUI_ExtrusionDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header:

#include "SMESHGUI_ExtrusionDlg.h"
#include <SMESHGUI_SpinBox.h>

#include <SUIT_ResourceMgr.h>

#include <SMDSAbs_ElementType.hxx>
#include <SMESH_Type.h>

#include <qcheckbox.h>
#include <qvalidator.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

//=================================================================================
// function : SMESHGUI_ExtrusionDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_ExtrusionDlg::SMESHGUI_ExtrusionDlg()
: SMESHGUI_Dialog( false, true, OK | Apply | Close )
{
  QPixmap image0( resMgr()->loadPixmap( "SMESH", tr( "ICON_DLG_EDGE") ) ),
          image1( resMgr()->loadPixmap( "SMESH", tr( "ICON_DLG_TRIANGLE") ) );

  resize(303, 185);
  setCaption(tr("EXTRUSION_ALONG_LINE"));
  
  QVBoxLayout* main = new QVBoxLayout( mainFrame() );

  /***************************************************************/
  myGroupConstructors = new QButtonGroup("GroupConstructors", mainFrame() );
  myGroupConstructors->setTitle(tr("SMESH_EXTRUSION"));
  myGroupConstructors->setExclusive(TRUE);
  myGroupConstructors->setColumnLayout(0, Qt::Vertical);
  myGroupConstructors->layout()->setSpacing(0);
  myGroupConstructors->layout()->setMargin(0);
  QGridLayout* GroupConstructorsLayout = new QGridLayout(myGroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6);
  GroupConstructorsLayout->setMargin(11);
  QRadioButton* RadioButton1 = new QRadioButton(myGroupConstructors, "RadioButton1");
  RadioButton1->setText(tr(""));
  RadioButton1->setPixmap(image0);
  GroupConstructorsLayout->addWidget(RadioButton1, 0, 0);
  QRadioButton* RadioButton2 = new QRadioButton(myGroupConstructors, "RadioButton2");
  RadioButton2->setText(tr(""));
  RadioButton2->setPixmap(image1);
  GroupConstructorsLayout->addWidget(RadioButton2, 0, 2);
  main->addWidget(myGroupConstructors);

  /***************************************************************/
  myGroupArguments = new QGroupBox( mainFrame(), "GroupArguments");
  myGroupArguments->setTitle(tr("EXTRUSION_1D"));
  myGroupArguments->setColumnLayout( 1, Qt::Horizontal );
  myGroupArguments->layout()->setSpacing(0);
  myGroupArguments->layout()->setMargin(11);

  QGroupBox* sel = new QGroupBox( myGroupArguments );
  sel->setColumnLayout( 3, Qt::Horizontal );
  sel->setFrameShape( QFrame::NoFrame );
  sel->layout()->setSpacing(0);
  sel->layout()->setMargin(0);
  
  // Controls for elements selection
  createObject( tr( "SMESH_ID_ELEMENTS" ), sel, 0 );
  createObject( tr( "SMESH_NAME" ), sel, 1 );

  int epr = prefix( "SMESH element" );
  setObjectType( 0, epr + SMDSAbs_Edge, epr + SMDSAbs_Face, -1 );
  setNameIndication( 0, ListOfNames );
  setReadOnly( 0, false );
  int mpr = prefix( "SMESH" );
  setObjectType( 1, mpr + MESH, mpr + SUBMESH, mpr + SUBMESH_EDGE, mpr + GROUP, -1 );
  setNameIndication( 1, OneName );

  QGroupBox* box1 = new QGroupBox( myGroupArguments );
  box1->setColumnLayout( 1, Qt::Vertical );
  box1->setFrameShape( QFrame::NoFrame );  
  box1->layout()->setSpacing(0);
  box1->layout()->setMargin(0);
  
  // Control for the whole mesh selection
  myCheckBoxMesh = new QCheckBox( box1, "CheckBoxMesh" );
  myCheckBoxMesh->setText(tr("SMESH_SELECT_WHOLE_MESH" ));

  QGroupBox* box2 = new QGroupBox( myGroupArguments );
  box2->setColumnLayout( 1, Qt::Vertical );    
  box2->setFrameShape( QFrame::NoFrame );
  box2->layout()->setSpacing(0);
  box2->layout()->setMargin(0);
  
  // Controls for vector selection
  QLabel* TextLabelVector = new QLabel( box2, "TextLabelVector" );
  TextLabelVector->setText( tr( "SMESH_VECTOR" ) );

  QLabel* TextLabelDx = new QLabel( box2, "TextLabelDx" );
  TextLabelDx->setText(tr("SMESH_DX"));

  myDx = new SMESHGUI_SpinBox( box2, "SpinBox_Dx" );

  QLabel* TextLabelDy = new QLabel( box2, "TextLabelDy" );
  TextLabelDy->setText(tr("SMESH_DY"));

  myDy = new SMESHGUI_SpinBox( box2, "SpinBox_Dy" );

  QLabel* TextLabelDz = new QLabel( box2, "TextLabelDz" );
  TextLabelDz->setText(tr("SMESH_DZ"));

  myDz = new SMESHGUI_SpinBox( box2, "SpinBox_Dz" );

  QGroupBox* box3 = new QGroupBox( myGroupArguments );
  box3->setColumnLayout( 1, Qt::Vertical );
  box3->setFrameShape( QFrame::NoFrame );
  box3->layout()->setSpacing(0);
  box3->layout()->setMargin(0);
  
  // Controls for nb. steps defining
  QLabel* TextLabelNbSteps = new QLabel( box3, "TextLabelNbSteps" );
  TextLabelNbSteps->setText(tr("SMESH_NUMBEROFSTEPS" ));

  myNbSteps = new QSpinBox( box3, "SpinBox_NbSteps" );

  main->addWidget( myGroupArguments );

  /* Initialisations */
  myDx->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);
  myDy->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);
  myDz->RangeStepAndValidator(-999999.999, +999999.999, 10.0, 3);

  QIntValidator* anIntValidator = new QIntValidator( myNbSteps );
  myNbSteps->setValidator( anIntValidator );
  myNbSteps->setRange( 1, 999999 );

  connect( myGroupConstructors, SIGNAL( clicked( int ) ),  this, SLOT  ( onConstructorsClicked( int ) ) );
  connect( myGroupConstructors, SIGNAL( clicked( int ) ),  this, SIGNAL( constructorClicked( int ) ) );
  connect( myCheckBoxMesh,      SIGNAL( toggled( bool ) ), this, SLOT  ( onSelectMesh( bool ) ) );
}

//=================================================================================
// function : ~SMESHGUI_ExtrusionDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_ExtrusionDlg::~SMESHGUI_ExtrusionDlg()
{
}

//=================================================================================
// function : onConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_ExtrusionDlg::onConstructorsClicked( int id )
{
  myGroupArguments->setTitle( tr( QString( "EXTRUSION_%1D" ).arg( id+1 ) ) );
  int mpr = prefix( "SMESH" );
  setObjectType( 1, mpr + MESH, mpr + SUBMESH, mpr + ( id==0 ? SUBMESH_EDGE : SUBMESH_FACE ), mpr + GROUP, -1 );
  clearSelection();
}

//=================================================================================
// function : onSelectMesh()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::onSelectMesh( bool toSelectMesh )
{
  setObjectShown( 0, !toSelectMesh );
  setObjectShown( 1, toSelectMesh );
  clearSelection();
}

//=================================================================================
// function : constructorId()
// purpose  :
//=================================================================================
int SMESHGUI_ExtrusionDlg::constructorId() const
{
  if( myGroupConstructors && myGroupConstructors->selected() )
    return myGroupConstructors->id( myGroupConstructors->selected() );
  else
    return -1;
}

//=================================================================================
// function : init()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::init()
{
  myGroupConstructors->setButton( 0 );
  myCheckBoxMesh->setChecked( false );
  onSelectMesh( false );
  myDx->setValue( 0.0 );
  myDy->setValue( 0.0 );
  myDz->setValue( 0.0 );
}  

//=================================================================================
// function : coords()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::coords( double& x, double& y, double& z ) const
{
  x = myDx->value();
  y = myDy->value();
  z = myDz->value();
}

//=================================================================================
// function : nbStep()
// purpose  :
//=================================================================================
int SMESHGUI_ExtrusionDlg::nbStep() const
{
  return myNbSteps->value();
}  

//=================================================================================
// function : isSelectMesh()
// purpose  :
//=================================================================================
bool SMESHGUI_ExtrusionDlg::isSelectMesh() const
{
  return myCheckBoxMesh->isChecked();
}
