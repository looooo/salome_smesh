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
#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESH_Actor.h"
#include "SMDS_Mesh.hxx"

#include <TopoDS.hxx>
#include <gp_Lin.hxx>
#include <gp_Vec.hxx>
#include <BRepAdaptor_Curve.hxx>

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "QAD_MessageBox.h"
#include "utilities.h"

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qspinbox.h> 
#include <qvalidator.h>
#include <qpixmap.h>

using namespace std;

//=================================================================================
// class    : SMESHGUI_ExtrusionDlg()
// purpose  : 
//=================================================================================
SMESHGUI_ExtrusionDlg::SMESHGUI_ExtrusionDlg( QWidget* parent, const char* name, SALOME_Selection* Sel,
					      bool modal, WFlags fl )
  : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
	     Qt::WDestructiveClose)
{
  QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_EDGE")));
  QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_TRIANGLE")));
  QPixmap image2(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));

  if ( !name )
    setName( "SMESHGUI_ExtrusionDlg" );
  resize( 303, 185 ); 
  setCaption( tr( "EXTRUSION_ALONG_LINE" ) );
  setSizeGripEnabled( TRUE );
  SMESHGUI_ExtrusionDlgLayout = new QGridLayout( this ); 
  SMESHGUI_ExtrusionDlgLayout->setSpacing( 6 );
  SMESHGUI_ExtrusionDlgLayout->setMargin( 11 );

  /***************************************************************/
  GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
  GroupConstructors->setTitle( tr( "SMESH_EXTRUSION"  ) );
  GroupConstructors->setExclusive( TRUE );
  GroupConstructors->setColumnLayout(0, Qt::Vertical );
  GroupConstructors->layout()->setSpacing( 0 );
  GroupConstructors->layout()->setMargin( 0 );
  GroupConstructorsLayout = new QGridLayout( GroupConstructors->layout() );
  GroupConstructorsLayout->setAlignment( Qt::AlignTop );
  GroupConstructorsLayout->setSpacing( 6 );
  GroupConstructorsLayout->setMargin( 11 );
  RadioButton1= new QRadioButton( GroupConstructors, "RadioButton1" );
  RadioButton1->setText( tr( ""  ) );
  RadioButton1->setPixmap( image0 );
  GroupConstructorsLayout->addWidget( RadioButton1, 0, 0 );
  RadioButton2= new QRadioButton( GroupConstructors, "RadioButton2" );
  RadioButton2->setText( tr( ""  ) );
  RadioButton2->setPixmap( image1 );
  GroupConstructorsLayout->addWidget( RadioButton2, 0, 2 );
  SMESHGUI_ExtrusionDlgLayout->addWidget( GroupConstructors, 0, 0 );
  
  /***************************************************************/
  GroupButtons = new QGroupBox( this, "GroupButtons" );
  GroupButtons->setGeometry( QRect( 10, 10, 281, 48 ) ); 
  GroupButtons->setTitle( tr( ""  ) );
  GroupButtons->setColumnLayout(0, Qt::Vertical );
  GroupButtons->layout()->setSpacing( 0 );
  GroupButtons->layout()->setMargin( 0 );
  GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
  GroupButtonsLayout->setAlignment( Qt::AlignTop );
  GroupButtonsLayout->setSpacing( 6 );
  GroupButtonsLayout->setMargin( 11 );
  buttonCancel = new QPushButton( GroupButtons, "buttonCancel" );
  buttonCancel->setText( tr( "SMESH_BUT_CLOSE"  ) );
  buttonCancel->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );
  buttonApply = new QPushButton( GroupButtons, "buttonApply" );
  buttonApply->setText( tr( "SMESH_BUT_APPLY"  ) );
  buttonApply->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonApply, 0, 1 );
  QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupButtonsLayout->addItem( spacer_9, 0, 2 );
  buttonOk = new QPushButton( GroupButtons, "buttonOk" );
  buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
  SMESHGUI_ExtrusionDlgLayout->addWidget( GroupButtons, 2, 0 );

  /***************************************************************/
  GroupArguments = new QGroupBox( this, "GroupArguments" );
  GroupArguments->setTitle( tr( "EXTRUSION_1D"  ) );
  GroupArguments->setColumnLayout(0, Qt::Vertical );
  GroupArguments->layout()->setSpacing( 0 );
  GroupArguments->layout()->setMargin( 0 );
  GroupArgumentsLayout = new QGridLayout( GroupArguments->layout());
  GroupArgumentsLayout->setAlignment( Qt::AlignTop );
  GroupArgumentsLayout->setSpacing( 6 );
  GroupArgumentsLayout->setMargin( 11 );

  // Controls for elements selection
  TextLabelElements  = new QLabel( GroupArguments, "TextLabelElements" );
  TextLabelElements->setText( tr( "SMESH_ID_ELEMENTS"  ) );
  GroupArgumentsLayout->addWidget( TextLabelElements, 0, 0 );

  SelectElementsButton  = new QPushButton( GroupArguments, "SelectElementsButton" );
  SelectElementsButton->setText( tr( ""  ) );
  SelectElementsButton->setPixmap( image2 );
  SelectElementsButton->setToggleButton( FALSE );
  GroupArgumentsLayout->addWidget( SelectElementsButton, 0, 1 );
  
  LineEditElements  = new QLineEdit( GroupArguments, "LineEditElements" );
  LineEditElements->setValidator( new SMESHGUI_IdValidator( this, "validator" ));
  GroupArgumentsLayout->addMultiCellWidget( LineEditElements, 0, 0, 2, 7 );

  // Controls for vector selection
  TextLabelVector = new QLabel( GroupArguments, "TextLabelVector" );
  TextLabelVector->setText( tr( "SMESH_VECTOR" ) );
  GroupArgumentsLayout->addWidget( TextLabelVector, 1, 0 );
  
  TextLabelDx = new QLabel( GroupArguments, "TextLabelDx" );
  TextLabelDx->setText( tr( "SMESH_DX" ) );
  GroupArgumentsLayout->addWidget( TextLabelDx, 1, 2 );

  SpinBox_Dx = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_Dx");
  GroupArgumentsLayout->addWidget( SpinBox_Dx, 1, 3 );

  TextLabelDy = new QLabel( GroupArguments, "TextLabelDy" );
  TextLabelDy->setText( tr( "SMESH_DY" ) );
  GroupArgumentsLayout->addWidget( TextLabelDy, 1, 4 );

  SpinBox_Dy = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_Dy");
  GroupArgumentsLayout->addWidget( SpinBox_Dy, 1, 5 );

  TextLabelDz = new QLabel( GroupArguments, "TextLabelDz" );
  TextLabelDz->setText( tr( "SMESH_DZ" ) );
  GroupArgumentsLayout->addWidget( TextLabelDz, 1, 6 );
  
  SpinBox_Dz = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_Dz");
  GroupArgumentsLayout->addWidget( SpinBox_Dz, 1, 7 );
  
  // Controls for nb. steps defining
  TextLabelNbSteps = new QLabel( GroupArguments, "TextLabelNbSteps" );
  TextLabelNbSteps->setText( tr( "SMESH_NUMBEROFSTEPS"  ) );
  GroupArgumentsLayout->addMultiCellWidget( TextLabelNbSteps, 2, 2, 0, 1 );

  SpinBox_NbSteps = new QSpinBox(GroupArguments, "SpinBox_NbSteps");
  GroupArgumentsLayout->addMultiCellWidget( SpinBox_NbSteps, 2, 2,  2, 7 );
  
  SMESHGUI_ExtrusionDlgLayout->addWidget( GroupArguments, 1, 0 );
  
  /* Initialisations */
  SpinBox_Dx->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  SpinBox_Dy->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  SpinBox_Dz->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );

  QIntValidator* anIntValidator = new QIntValidator(SpinBox_NbSteps);
  SpinBox_NbSteps->setValidator(anIntValidator);
  SpinBox_NbSteps->setRange( 1, 999999 );
 
  GroupArguments->show();
  RadioButton1->setChecked( TRUE );
  mySelection = Sel;  

  mySMESHGUI  = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;
    
  Init();
  /* signals and slots connections */
  connect( buttonOk, SIGNAL( clicked() ),     this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT(ClickOnApply() ) );
  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );
  
  connect( SelectElementsButton, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( LineEditElements, SIGNAL( textChanged( const QString& )),
           SLOT( onTextChange( const QString& )));
  
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* displays Dialog */

  ConstructorsClicked(0);
}

//=================================================================================
// function : ~SMESHGUI_ExtrusionDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_ExtrusionDlg::~SMESHGUI_ExtrusionDlg()
{
  // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::Init()
{
  myBusy = false;    

  SpinBox_NbSteps->setValue(1);
  
  myEditCurrentArgument = LineEditElements;
  LineEditElements->setFocus();
  myNbOkElements = 0 ;
  myActor     = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();
  
  SpinBox_Dx->SetValue(0);
  SpinBox_Dy->SetValue(0);
  SpinBox_Dz->SetValue(0);
  
  SelectionIntoArgument();
}


//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_ExtrusionDlg::ConstructorsClicked(int constructorId)
{
  disconnect(mySelection, 0, this, 0);
  mySelection->ClearIObjects();
  mySelection->ClearFilters();
  myNbOkElements = 0 ;
  myEditCurrentArgument = LineEditElements;
  LineEditElements->setText("");
  LineEditElements->setFocus();
  
  switch(constructorId)
    { 
    case 0 :
      {
	GroupArguments->setTitle( tr( "EXTRUSION_1D" ) );
	QAD_Application::getDesktop()->SetSelectionMode( EdgeSelection, true );
	break;
      }
    case 1 :
      { 
	GroupArguments->setTitle( tr( "EXTRUSION_2D" ) );
	QAD_Application::getDesktop()->SetSelectionMode( FaceSelection, true );
	break;
      }
    }
  connect(mySelection, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
}


//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::ClickOnApply()
{
  if (mySMESHGUI->ActiveStudyLocked())
    return;
  
  if ( myNbOkElements)
    {
      QStringList aListElementsId = QStringList::split( " ", LineEditElements->text(), false);
      
      SMESH::long_array_var anElementsId = new SMESH::long_array;
      
      anElementsId->length( aListElementsId.count() );
      for ( int i = 0; i < aListElementsId.count(); i++ )
	anElementsId[i] = aListElementsId[i].toInt();
      
      SMESH::DirStruct aVector;
      aVector.PS.x = SpinBox_Dx->GetValue();
      aVector.PS.y = SpinBox_Dy->GetValue();
      aVector.PS.z = SpinBox_Dz->GetValue();

      long aNbSteps = (long)SpinBox_NbSteps->value();

      try
	{
	  SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
	  QApplication::setOverrideCursor(Qt::waitCursor);
	  aMeshEditor->ExtrusionSweep(anElementsId.inout(), aVector, aNbSteps);
	  QApplication::restoreOverrideCursor();
	}
      catch( ... )
	{
	}
      
      mySelection->ClearIObjects();
      SMESH::UpdateView();
      Init();
    }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::ClickOnOk()
{
  this->ClickOnApply() ;
  this->ClickOnCancel() ;
}

	
//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::ClickOnCancel()
{
  mySelection->ClearFilters();
  mySelection->ClearIObjects();
  SMESH::SetPointRepresentation(false);
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  reject() ;
}

//=======================================================================
//function : onTextChange
//purpose  : 
//=======================================================================
void SMESHGUI_ExtrusionDlg::onTextChange(const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  if ( myBusy ) return;
  myBusy = true;
  
  if (send == LineEditElements)
    myNbOkElements = 0;
  
  buttonOk->setEnabled( false );
  buttonApply->setEnabled( false );
  
  // hilight entered elements/nodes
  SMDS_Mesh* aMesh = 0;
  if ( myActor )
    aMesh = myActor->GetObject()->GetMesh();
  
  if ( aMesh ) {
    
    mySelection->ClearIObjects();
    mySelection->AddIObject( myActor->getIO() );
    
    QStringList aListId = QStringList::split( " ", theNewText, false);
    
    if (send == LineEditElements) {
      for ( int i = 0; i < aListId.count(); i++ ) {
	const SMDS_MeshElement * e = aMesh->FindElement( aListId[ i ].toInt() );
	if ( e ) {
	  if ( !mySelection->IsIndexSelected( myActor->getIO(), e->GetID() ))
	    mySelection->AddOrRemoveIndex (myActor->getIO(), e->GetID(), true);
	  myNbOkElements++;
	}
      }
    }
  }
  
  if ( myNbOkElements ) {
    buttonOk->setEnabled( true );
    buttonApply->setEnabled( true );
  }
  
  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_ExtrusionDlg::SelectionIntoArgument()
{
  if ( myBusy ) return;
  
  // clear
  
  myActor = 0;
  QString aString = "";

  myBusy = true;
  myEditCurrentArgument->setText( aString );
  myBusy = false;
  
  if ( !GroupButtons->isEnabled() ) // inactive
    return;
  
  buttonOk->setEnabled( false );
  buttonApply->setEnabled( false );
  
  // get selected mesh
  
  int nbSel = SMESH::GetNameOfSelectedIObjects(mySelection, aString);
  if(nbSel != 1)
    return;
  
  Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject();
  
  if (myEditCurrentArgument == LineEditElements)
    {
      myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO) ;
      
      myActor = SMESH::FindActorByEntry( mySelection->firstIObject()->getEntry() );
      
      // get selected elements
      int aNbElements = 0;
      
      aNbElements = SMESH::GetNameOfSelectedElements(mySelection, aString) ;
      
      if(aNbElements < 1)
	return ;
    }
  
  myBusy = true;
  myEditCurrentArgument->setText( aString );
  myBusy = false;
  
  // OK
  
  if (myEditCurrentArgument == LineEditElements )
    myNbOkElements = true;
  
  if(myNbOkElements)
    {
      buttonOk->setEnabled( true );
      buttonApply->setEnabled( true );
    }
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  disconnect( mySelection, 0, this, 0 );
  mySelection->ClearIObjects();
  mySelection->ClearFilters();
  
  if(send == SelectElementsButton) {
    myEditCurrentArgument = LineEditElements;
    int aConstructorId = GetConstructorId();
    if (aConstructorId == 0)
      QAD_Application::getDesktop()->SetSelectionMode( EdgeSelection, true );
    else if (aConstructorId == 1)
      QAD_Application::getDesktop()->SetSelectionMode( FaceSelection, true );
  }
  
  myEditCurrentArgument->setFocus();
  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  SelectionIntoArgument() ;
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {
    GroupConstructors->setEnabled(false) ;
    GroupArguments->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
    mySMESHGUI->ResetState() ;    
    mySMESHGUI->SetActiveDialogBox(0) ;
  }
  return ;
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupConstructors->setEnabled(true) ;
  GroupArguments->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  ConstructorsClicked(GetConstructorId());
  SelectionIntoArgument();
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::enterEvent(QEvent* e)
{
  if ( GroupConstructors->isEnabled() )
    return ;  
  ActivateThisDialog() ;
  return ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionDlg::closeEvent( QCloseEvent* e )
{
  /* same than click on cancel button */
  this->ClickOnCancel() ;
}


//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================

void SMESHGUI_ExtrusionDlg::hideEvent ( QHideEvent * e )
{
  if ( !isMinimized() )
    ClickOnCancel();
}


//=================================================================================
// function : GetConstructorId()
// purpose  : 
//=================================================================================
int SMESHGUI_ExtrusionDlg::GetConstructorId()
{ 
  if ( GroupConstructors != NULL && GroupConstructors->selected() != NULL )
    return GroupConstructors->id( GroupConstructors->selected() );
  return -1;
}
