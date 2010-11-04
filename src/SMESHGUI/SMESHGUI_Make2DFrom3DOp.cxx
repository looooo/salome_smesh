//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_Make2DFrom3DOp.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_Make2DFrom3DOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_MeshInfosBox.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_LogicalFilter.hxx"

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Tools.h>

#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>

#include <SALOME_ListIO.hxx>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>
#include <SALOMEDSClient_SObject.hxx>

// Qt includes
// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGridLayout>

// MESH includes
#include "SMDSAbs_ElementType.hxx"
#include "SMDSAbs_ElementType.hxx"

#define SPACING 6
#define MARGIN  11

// =========================================================================================
/*!
 * \brief Dialog to show creted mesh statistic
 */
//=======================================================================

SMESHGUI_Make2DFrom3DDlg::SMESHGUI_Make2DFrom3DDlg( QWidget* parent )
 : SMESHGUI_Dialog( parent, false, true, Close/* | Help*/ )
{
  setWindowTitle( tr("CAPTION") );
  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame());
  aDlgLay->setMargin( 0 );
  aDlgLay->setSpacing( SPACING );
  QFrame* aMainFrame = createMainFrame(mainFrame());
  aDlgLay->addWidget(aMainFrame);
  aDlgLay->setStretchFactor(aMainFrame, 1);
}

// =========================================================================================
/*!
 * \brief Dialog destructor
 */
//=======================================================================

SMESHGUI_Make2DFrom3DDlg::~SMESHGUI_Make2DFrom3DDlg()
{
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's fields
//=======================================================================

QFrame* SMESHGUI_Make2DFrom3DDlg::createMainFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);

  SUIT_ResourceMgr* rm = resourceMgr();
  QPixmap iconCompute (rm->loadPixmap("SMESH", tr("ICON_2D_FROM_3D")));

  // Mesh name
  QGroupBox* nameBox = new QGroupBox(tr("SMESH_MESHINFO_NAME"), aFrame );
  QHBoxLayout* nameBoxLayout = new QHBoxLayout(nameBox);
  nameBoxLayout->setMargin(MARGIN); nameBoxLayout->setSpacing(SPACING);
  myMeshName = new QLabel(nameBox);
  nameBoxLayout->addWidget(myMeshName);

  // Mesh Info

  myFullInfo = new SMESHGUI_MeshInfosBox(true,  aFrame);

  // add all widgets to aFrame
  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->setMargin( 0 );
  aLay->setSpacing( 0 );
  aLay->addWidget( nameBox );
  aLay->addWidget( myFullInfo );

  ((QPushButton*) button( OK ))->setDefault( true );
  return aFrame;
}

//================================================================================
/*!
 * \brief set name of the mesh
*/
//================================================================================

void SMESHGUI_Make2DFrom3DDlg::SetMeshName(const QString& theName)
{
  myMeshName->setText( theName );
}

//================================================================================
/*!
 * \brief set mesh info
*/
//================================================================================

void SMESHGUI_Make2DFrom3DDlg::SetMeshInfo(const SMESH::long_array& theInfo)
{
  myFullInfo->SetMeshInfo( theInfo );
}

// =========================================================================================
/*!
 * \brief Copy Mesh dialog box
 */
//=======================================================================

SMESHGUI_CopyMeshDlg::SMESHGUI_CopyMeshDlg( QWidget* parent )
 : SMESHGUI_Dialog( parent, false, true, OK | Apply | Close | Help )
{
  setWindowTitle( tr("CAPTION") );

  // mesh
  setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
  createObject( tr( "MESH" ), mainFrame(), Mesh );

  // mode
  QGroupBox* aModeGrp = new QGroupBox( tr( "MODE" ), mainFrame() );
  QHBoxLayout* aModeGrpLayout = new QHBoxLayout( aModeGrp );
  aModeGrpLayout->setMargin( MARGIN );
  aModeGrpLayout->setSpacing( SPACING );
  QRadioButton* a2dFrom3dRB = new QRadioButton( tr( "2D_FROM_3D" ), aModeGrp );
  QRadioButton* a1dFrom3dRB = new QRadioButton( tr( "1D_FROM_3D" ), aModeGrp );
  QRadioButton* a1dFrom2dRB = new QRadioButton( tr( "1D_FROM_2D" ), aModeGrp );
  aModeGrpLayout->addWidget( a2dFrom3dRB );
  aModeGrpLayout->addWidget( a1dFrom3dRB );
  aModeGrpLayout->addWidget( a1dFrom2dRB );

  // target
  QGroupBox* aTargetGrp = new QGroupBox( tr( "TARGET" ), mainFrame() );
  QGridLayout* aTargetGrpLayout = new QGridLayout( aTargetGrp );
  aTargetGrpLayout->setMargin( MARGIN );
  aTargetGrpLayout->setSpacing( SPACING );
  myThisMeshRB   = new QRadioButton( tr( "THIS_MESH" ), aTargetGrp );
  myNewMeshRB    = new QRadioButton( tr( "NEW_MESH" ),  aTargetGrp );
  myMeshName     = new QLineEdit( aTargetGrp );
  myCopyCheck    = new QCheckBox( tr( "COPY_SRC" ),     aTargetGrp );
  myMissingCheck = new QCheckBox( tr( "MISSING_ONLY" ), aTargetGrp );
  aTargetGrpLayout->addWidget( myThisMeshRB,    0, 0 );
  aTargetGrpLayout->addWidget( myNewMeshRB,     1, 0 );
  aTargetGrpLayout->addWidget( myMeshName,     1, 1 );
  aTargetGrpLayout->addWidget( myCopyCheck,    2, 0 );
  aTargetGrpLayout->addWidget( myMissingCheck, 2, 1 );
  myGroupCheck = new QCheckBox( tr( "CREATE_GROUP" ), mainFrame() );
  myGroupName  = new QLineEdit( mainFrame() );

  // layout
  QGridLayout* aDlgLay = new QGridLayout( mainFrame() );
  aDlgLay->setMargin( 0 );
  aDlgLay->setSpacing( SPACING );
  aDlgLay->addWidget( objectWg( Mesh,  Label ),   0, 0 );
  aDlgLay->addWidget( objectWg( Mesh,  Btn ),     0, 1 );
  aDlgLay->addWidget( objectWg( Mesh,  Control ), 0, 2 );
  aDlgLay->addWidget( aModeGrp,     1, 0, 1, 3 );
  aDlgLay->addWidget( aTargetGrp,   2, 0, 1, 3 );
  aDlgLay->addWidget( myGroupCheck, 3, 0 );
  aDlgLay->addWidget( myGroupName,  3, 1, 1, 2 );
  //aDlgLay->setStretchFactor(aMainFrame, 1);

  connect( myThisMeshRB, SIGNAL( clicked() ), this, SLOT( onTargetChanged() ) );
  connect( myNewMeshRB,  SIGNAL( clicked() ), this, SLOT( onTargetChanged() ) );
  connect( myGroupCheck, SIGNAL( clicked() ), this, SLOT( onGroupChecked() ) );

  a2dFrom3dRB->setChecked( true );
  myThisMeshRB->setChecked( true );
  onTargetChanged();
  onGroupChecked();
  enableControls( false );
}

SMESHGUI_CopyMeshDlg::~SMESHGUI_CopyMeshDlg()
{
}

void SMESHGUI_CopyMeshDlg::enableControls( bool on )
{
  printf("SMESHGUI_CopyMeshDlg::enableControls:%d\n",on);
  setButtonEnabled( on, QtxDialog::OK | QtxDialog::Apply );
}

void SMESHGUI_CopyMeshDlg::onTargetChanged()
{
  myMeshName->setEnabled( myNewMeshRB->isChecked() );
  myCopyCheck->setEnabled( myNewMeshRB->isChecked() );
  myMissingCheck->setEnabled( myNewMeshRB->isChecked() );
}

void SMESHGUI_CopyMeshDlg::onGroupChecked()
{
  myGroupName->setEnabled( myGroupCheck->isChecked() );
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_Make2DFrom3DOp::SMESHGUI_Make2DFrom3DOp()
 : SMESHGUI_SelectionOp()
{
}

//================================================================================
/*!
 * \brief Desctructor
*/
//================================================================================

SMESHGUI_Make2DFrom3DOp::~SMESHGUI_Make2DFrom3DOp()
{
  if ( myDlg )
    delete myDlg;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
  * \retval LightApp_Dialog* - pointer to dialog of this operation
*/
//================================================================================
LightApp_Dialog* SMESHGUI_Make2DFrom3DOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*!
 * \brief perform it's intention action: compute 2D mesh on 3D
 */
//================================================================================

void SMESHGUI_Make2DFrom3DOp::startOperation()
{
  if( !myDlg )
    myDlg = new SMESHGUI_CopyMeshDlg( desktop() );

  mySrc = SMESH::SMESH_IDSource::_nil();
  
  myHelpFileName = "copy_mesh_page.html";

  SMESHGUI_SelectionOp::startOperation();

  myDlg->activateObject( SMESHGUI_CopyMeshDlg::Mesh );
  myDlg->show();

  selectionDone();
  /*
  // backup mesh info before 2D mesh computation
  SMESH::long_array_var anOldInfo = myMesh->GetMeshInfo();
  

  if (0){//!compute2DMesh()) {
    SUIT_MessageBox::warning(desktop(),
                             tr("SMESH_WRN_WARNING"),
                             tr("SMESH_WRN_COMPUTE_FAILED"));
    onCancel();
    return;
  }
  // get new mesh statistic
  SMESH::long_array_var aNewInfo = myMesh->GetMeshInfo();
  // get difference in mesh statistic from old to new
  for ( int i = SMDSEntity_Node; i < SMDSEntity_Last; i++ )
    aNewInfo[i] -= anOldInfo[i];

  // update presentation
  SMESH::Update(anIO, SMESH::eDisplay);

  // show computated result
  _PTR(SObject) aMeshSObj = SMESH::FindSObject(myMesh);
  if ( aMeshSObj )
    ;//myDlg->SetMeshName( aMeshSObj->GetName().c_str() );
  //myDlg->SetMeshInfo( aNewInfo );
  myDlg->show();
  //commit();
  //SMESHGUI::Modified();
*/
}

//================================================================================
/*!
 * \brief Updates dialog's look and feel
 *
 * Virtual method redefined from the base class updates dialog's look and feel
 */
//================================================================================
void SMESHGUI_Make2DFrom3DOp::selectionDone()
{
  bool on = false;
  if ( dlg()->isVisible() ) {
    SMESHGUI_SelectionOp::selectionDone();
    try {
      QString entry = myDlg->selectedObject( SMESHGUI_CopyMeshDlg::Mesh );
      _PTR(SObject) sobj = studyDS()->FindObjectID( entry.toLatin1().constData() );
      if ( sobj ) {
	SMESH::SMESH_IDSource_var obj = SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( sobj );  
	on = !obj->_is_nil();
      }
    }
    catch ( const SALOME::SALOME_Exception& S_ex ) {
      SalomeApp_Tools::QtCatchCorbaException( S_ex );
    }
    catch ( ... ) {
    }
  }
  myDlg->enableControls( on );
}

//================================================================================
/*!
 * \brief Creates selection filter
  * \param theId - identifier of current selection widget
  * \retval SUIT_SelectionFilter* - pointer to the created filter or null
 *
 * Creates selection filter in accordance with identifier of current selection widget
 */
//================================================================================
SUIT_SelectionFilter* SMESHGUI_Make2DFrom3DOp::createFilter( const int theId ) const
{
  SUIT_SelectionFilter* f = 0;
  if ( theId == SMESHGUI_CopyMeshDlg::Mesh ) {
    QList<SUIT_SelectionFilter*> filters;
    filters.append( new SMESH_TypeFilter( MESHorSUBMESH ) );
    filters.append( new SMESH_TypeFilter( GROUP ) );
    f = new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );
  }
  return f;
}

//================================================================================
/*!
 * \brief compute 2D mesh on initial 3D
 */
//================================================================================

bool SMESHGUI_Make2DFrom3DOp::compute2DMesh()
{
//   SUIT_OverrideCursor wc;
//   SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
//   return aMeshEditor->Make2DMeshFrom3D();
}

bool SMESHGUI_Make2DFrom3DOp::onApply()
{
  return false;
}
