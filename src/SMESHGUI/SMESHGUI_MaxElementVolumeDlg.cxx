using namespace std;
//  File      : SMESHGUI_MaxElementVolumeDlg.cxx
//  Created   : Wed Jun 12 21:17:51 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#include "SMESHGUI_MaxElementVolumeDlg.h"
#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "utilities.h"

// QT Includes
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

//=================================================================================
// class    : SMESHGUI_MaxElementVolumeDlg()
// purpose  : Constructs a SMESHGUI_MaxElementVolumeDlg which is a child of 'parent', with the 
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_MaxElementVolumeDlg::SMESHGUI_MaxElementVolumeDlg( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
    QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESHGUI",tr("ICON_DLG_MAX_ELEMENT_VOLUME")));
    
    if ( !name )
      setName( "SMESHGUI_MaxElementVolumeDlg" );
    resize( 303, 175 ); 
    setCaption( tr( "SMESH_MAX_ELEMENT_VOLUME_TITLE"  ) );
    setSizeGripEnabled( TRUE );
    SMESHGUI_MaxElementVolumeDlgLayout = new QGridLayout( this ); 
    SMESHGUI_MaxElementVolumeDlgLayout->setSpacing( 6 );
    SMESHGUI_MaxElementVolumeDlgLayout->setMargin( 11 );

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
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupButtonsLayout->addItem( spacer, 0, 2 );
    buttonOk = new QPushButton( GroupButtons, "buttonOk" );
    buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
    SMESHGUI_MaxElementVolumeDlgLayout->addWidget( GroupButtons, 2, 0 );

    /***************************************************************/
    GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
    GroupConstructors->setTitle( tr( "SMESH_MAX_ELEMENT_VOLUME_HYPOTHESIS"  ) );
    GroupConstructors->setExclusive( TRUE );
    GroupConstructors->setColumnLayout(0, Qt::Vertical );
    GroupConstructors->layout()->setSpacing( 0 );
    GroupConstructors->layout()->setMargin( 0 );
    GroupConstructorsLayout = new QGridLayout( GroupConstructors->layout() );
    GroupConstructorsLayout->setAlignment( Qt::AlignTop );
    GroupConstructorsLayout->setSpacing( 6 );
    GroupConstructorsLayout->setMargin( 11 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupConstructorsLayout->addItem( spacer_2, 0, 1 );
    Constructor1 = new QRadioButton( GroupConstructors, "Constructor1" );
    Constructor1->setText( tr( ""  ) );
    Constructor1->setPixmap( image0 );
    Constructor1->setChecked( TRUE );
    Constructor1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, Constructor1->sizePolicy().hasHeightForWidth() ) );
    Constructor1->setMinimumSize( QSize( 50, 0 ) );
    GroupConstructorsLayout->addWidget( Constructor1, 0, 0 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupConstructorsLayout->addItem( spacer_3, 0, 1 );
    SMESHGUI_MaxElementVolumeDlgLayout->addWidget( GroupConstructors, 0, 0 );
    
    /***************************************************************/
    GroupC1 = new QGroupBox( this, "GroupC1" );
    GroupC1->setTitle( tr( "SMESH_ARGUMENTS"  ) );
    GroupC1->setColumnLayout(0, Qt::Vertical );
    GroupC1->layout()->setSpacing( 0 );
    GroupC1->layout()->setMargin( 0 );
    GroupC1Layout = new QGridLayout( GroupC1->layout() );
    GroupC1Layout->setAlignment( Qt::AlignTop );
    GroupC1Layout->setSpacing( 6 );
    GroupC1Layout->setMargin( 11 );
    TextLabel_MaxElementVolume = new QLabel(GroupC1 , "TextLabel_MaxElementVolume" );
    TextLabel_MaxElementVolume->setText( tr( "SMESH_MAX_ELEMENT_VOLUME"  ) );
    GroupC1Layout->addWidget( TextLabel_MaxElementVolume, 1, 0 );
    TextLabel_NameHypothesis = new QLabel( GroupC1, "TextLabel_NameHypothesis" );
    TextLabel_NameHypothesis->setText( tr( "SMESH_NAME"  ) );
    GroupC1Layout->addWidget( TextLabel_NameHypothesis, 0, 0 );

    // LineEdit_MaxElementVolume = new QLineEdit(GroupC1 , "LineEdit_MaxElementVolume" );
    // GroupC1Layout->addWidget( LineEdit_MaxElementVolume, 1, 1 );
    SpinBox_MaxElementVolume = new SMESHGUI_SpinBox( GroupC1, "SpinBox_MaxElementVolume" ) ;
    GroupC1Layout->addWidget( SpinBox_MaxElementVolume, 1, 1 );

    LineEdit_NameHypothesis = new QLineEdit( GroupC1, "LineEdit_NameHypothesis" );
    GroupC1Layout->addWidget( LineEdit_NameHypothesis, 0, 1 );
    QSpacerItem* spacer1 = new QSpacerItem( 20, 24, QSizePolicy::Minimum, QSizePolicy::Fixed );
    GroupC1Layout->addItem( spacer1, 1, 3 );
    SMESHGUI_MaxElementVolumeDlgLayout->addWidget(GroupC1 , 1, 0 );

    /***************************************************************/

    Init() ;
}


//=================================================================================
// function : ~SMESHGUI_MaxElementVolumeDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_MaxElementVolumeDlg::~SMESHGUI_MaxElementVolumeDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_MaxElementVolumeDlg::Init()
{
  
  GroupC1->show();
  myConstructorId = 0 ;
  Constructor1->setChecked( TRUE );
  myEditCurrentArgument = LineEdit_NameHypothesis ;


  /* Get setting of step value from file configuration */
  double step ;
  // QString St = QAD_CONFIG->getSetting( "xxxxxxxxxxxxx" ) ;  TODO
  // step = St.toDouble() ;                                    TODO
  step = 1.0 ;

  /* min, max, step and decimals for spin boxes */
  SpinBox_MaxElementVolume->setPrecision( 10 );
  SpinBox_MaxElementVolume->RangeStepAndValidator( 0.001, 999999.999, step, 3 ) ;
  SpinBox_MaxElementVolume->SetValue( 1.0 ) ;  /* is myMaxElementVolume */

  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  LineEdit_NameHypothesis->setText("") ;

  myNameHypothesis = "" ;
  myMaxElementVolume = 1.0 ;

  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  /* signals and slots connections */
  connect( buttonOk, SIGNAL( clicked() ),     this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT(ClickOnApply() ) );
  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );  

  connect( LineEdit_NameHypothesis, SIGNAL (textChanged(const QString&)  ), this, SLOT( TextChangedInLineEdit(const QString&) ) ) ;

  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;  
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;

  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ; 
  this->show() ; /* displays Dialog */

  return ;
}



//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_MaxElementVolumeDlg::ConstructorsClicked(int constructorId)
{
  return;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_MaxElementVolumeDlg::ClickOnOk()
{
  this->ClickOnApply() ;
  this->ClickOnCancel() ;

  return ;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_MaxElementVolumeDlg::ClickOnApply()
{
  switch(myConstructorId)
    { 
    case 0 :
      { 
	myMaxElementVolume = SpinBox_MaxElementVolume->GetValue() ;
	mySMESHGUI->CreateMaxElementVolume( "MaxElementVolume", myNameHypothesis, myMaxElementVolume );
      	break ;
      }
    }
  return ;
}


//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_MaxElementVolumeDlg::ClickOnCancel()
{
  mySMESHGUI->ResetState() ;
  reject() ;
}

//=================================================================================
// function : TextChangedInLineEdit()
// purpose  :
//=================================================================================
void SMESHGUI_MaxElementVolumeDlg::TextChangedInLineEdit(const QString& newText)
{  
  QLineEdit* send = (QLineEdit*)sender();
  QString newT = strdup(newText) ;
  
  if (send == LineEdit_NameHypothesis) {
    myNameHypothesis = newText ;
  }
  return ;
}


//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MaxElementVolumeDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {
    GroupConstructors->setEnabled(false) ;
    GroupC1->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
  } 
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MaxElementVolumeDlg::ActivateThisDialog()
{
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupConstructors->setEnabled(true) ;
  GroupC1->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  return ;
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MaxElementVolumeDlg::enterEvent(QEvent* e)
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
void SMESHGUI_MaxElementVolumeDlg::closeEvent( QCloseEvent* e )
{
  this->ClickOnCancel() ;
  return ;
}
