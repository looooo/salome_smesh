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
//  SMESH SMESHGUI : GUI for SMESH component
//  File   : SMESHGUI_ScaleDlg.cxx
//  Author : Michael ZORIN, Open CASCADE S.A.S.
//  SMESH includes

#include "SMESHGUI_ScaleDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_FilterDlg.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

// SALOME KERNEL includes
#include <SALOMEDSClient_SObject.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpinBox>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

enum { MOVE_ELEMS_BUTTON = 0, COPY_ELEMS_BUTTON, MAKE_MESH_BUTTON }; //!< action type

/*!
  \class BusyLocker
  \brief Simple 'busy state' flag locker.
  \internal
*/

class BusyLocker
{
public:
  //! Constructor. Sets passed boolean flag to \c true.
  BusyLocker( bool& busy ) : myBusy( busy ) { myBusy = true; }
  //! Destructor. Clear external boolean flag passed as parameter to the constructor to \c false.
  ~BusyLocker() { myBusy = false; }
private:
  bool& myBusy; //! External 'busy state' boolean flag
};

#define SPACING 6
#define MARGIN  11

//To disable automatic genericobj management, the following line should be commented.
//Otherwise, it should be uncommented. Refer to KERNEL_SRC/src/SALOMEDSImpl/SALOMEDSImpl_AttributeIOR.cxx
#define WITHGENERICOBJ

//=================================================================================
// class    : SMESHGUI_ScaleDlg()
// purpose  :
//=================================================================================
SMESHGUI_ScaleDlg::SMESHGUI_ScaleDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myFilterDlg(0),
    mySelectedObject(SMESH::SMESH_IDSource::_nil())
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_MESH_SCALE")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_SCALE_ALONG_AXES")));
  QPixmap image2 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_SCALE_TITLE"));
  setSizeGripEnabled(true);

  QVBoxLayout* SMESHGUI_ScaleDlgLayout = new QVBoxLayout(this);
  SMESHGUI_ScaleDlgLayout->setSpacing(SPACING);
  SMESHGUI_ScaleDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  ConstructorsBox = new QGroupBox(tr("SMESH_SCALE"), this);
  GroupConstructors = new QButtonGroup(this);
  QHBoxLayout* ConstructorsBoxLayout = new QHBoxLayout(ConstructorsBox);
  ConstructorsBoxLayout->setSpacing(SPACING);
  ConstructorsBoxLayout->setMargin(MARGIN);

  RadioButton1= new QRadioButton(ConstructorsBox);
  RadioButton1->setIcon(image0);
  RadioButton2= new QRadioButton(ConstructorsBox);
  RadioButton2->setIcon(image1);

  ConstructorsBoxLayout->addWidget(RadioButton1);
  ConstructorsBoxLayout->addWidget(RadioButton2);
  GroupConstructors->addButton(RadioButton1, 0);
  GroupConstructors->addButton(RadioButton2, 1);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);
  SelectElementsButton = new QPushButton(GroupArguments);
  SelectElementsButton->setIcon(image2);
  LineEditElements = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(myIdValidator);
  LineEditElements->setMaxLength(-1);
  myFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupArguments );
  connect(myFilterBtn,   SIGNAL(clicked()), this, SLOT(setFilters()));

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Controls for vector and points selection
  TextLabel1 = new QLabel(tr("SMESH_BASE_POINT"), GroupArguments);
  SelectButton1 = new QPushButton(GroupArguments);
  SelectButton1->setIcon(image2);

  TextLabel1_1 = new QLabel(tr("SMESH_X"), GroupArguments);
  SpinBox1_1 = new SMESHGUI_SpinBox(GroupArguments);
  TextLabel1_2 = new QLabel(tr("SMESH_Y"), GroupArguments);
  SpinBox1_2 = new SMESHGUI_SpinBox(GroupArguments);
  TextLabel1_3 = new QLabel(tr("SMESH_Z"), GroupArguments);
  SpinBox1_3 = new SMESHGUI_SpinBox(GroupArguments);

  TextLabel2 = new QLabel(tr("SMESH_SCALE_FACTOR"), GroupArguments);
  SpinBox_FX = new SMESHGUI_SpinBox(GroupArguments);

  TextLabel3 = new QLabel(tr("SMESH_SCALE_FACTOR_Y"), GroupArguments);
  SpinBox_FY = new SMESHGUI_SpinBox(GroupArguments);

  TextLabel4 = new QLabel(tr("SMESH_SCALE_FACTOR_Z"), GroupArguments);
  SpinBox_FZ = new SMESHGUI_SpinBox(GroupArguments);


  // switch of action type
  ActionBox = new QGroupBox(GroupArguments);
  ActionGroup = new QButtonGroup(GroupArguments);
  QVBoxLayout* ActionBoxLayout = new QVBoxLayout(ActionBox);
  ActionBoxLayout->addSpacing(SPACING);
  ActionBoxLayout->setMargin(MARGIN);

  QRadioButton* aMoveElements = new QRadioButton(tr("SMESH_MOVE_ELEMENTS"), ActionBox);
  QRadioButton* aCopyElements = new QRadioButton(tr("SMESH_COPY_ELEMENTS"), ActionBox);
  QRadioButton* aCreateMesh   = new QRadioButton(tr("SMESH_CREATE_MESH"),   ActionBox);

  ActionBoxLayout->addWidget(aMoveElements);
  ActionBoxLayout->addWidget(aCopyElements);
  ActionBoxLayout->addWidget(aCreateMesh);
  ActionGroup->addButton(aMoveElements, MOVE_ELEMS_BUTTON);
  ActionGroup->addButton(aCopyElements, COPY_ELEMS_BUTTON);
  ActionGroup->addButton(aCreateMesh,   MAKE_MESH_BUTTON);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);
  MakeGroupsCheck->setChecked(false);

  // Name of a mesh to create
  LineEditNewMesh = new QLineEdit(GroupArguments);

  // layout
  GroupArgumentsLayout->addWidget(TextLabelElements,    0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,     0, 2, 1, 5);
  GroupArgumentsLayout->addWidget(myFilterBtn,          0, 7);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,         1, 0, 1, 8);
  GroupArgumentsLayout->addWidget(TextLabel1,           2, 0);
  GroupArgumentsLayout->addWidget(SelectButton1,        2, 1);
  GroupArgumentsLayout->addWidget(TextLabel1_1,         2, 2);
  GroupArgumentsLayout->addWidget(SpinBox1_1,           2, 3);
  GroupArgumentsLayout->addWidget(TextLabel1_2,         2, 4);
  GroupArgumentsLayout->addWidget(SpinBox1_2,           2, 5);
  GroupArgumentsLayout->addWidget(TextLabel1_3,         2, 6);
  GroupArgumentsLayout->addWidget(SpinBox1_3,           2, 7);
  GroupArgumentsLayout->addWidget(TextLabel2,           3, 0);
  GroupArgumentsLayout->addWidget(SpinBox_FX,           3, 3);
  GroupArgumentsLayout->addWidget(TextLabel3,           4, 0);
  GroupArgumentsLayout->addWidget(SpinBox_FY,           4, 3);
  GroupArgumentsLayout->addWidget(TextLabel4,           5, 0);
  GroupArgumentsLayout->addWidget(SpinBox_FZ,           5, 3);
  GroupArgumentsLayout->addWidget(ActionBox,            7, 0, 3, 4);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      7, 5, 1, 4);
  GroupArgumentsLayout->addWidget(LineEditNewMesh,      8, 5, 1, 4);

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  buttonApply->setAutoDefault(true);
  buttonCancel = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  buttonCancel->setAutoDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);

  GroupButtonsLayout->addWidget(buttonOk);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget(buttonApply);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(buttonCancel);
  GroupButtonsLayout->addWidget(buttonHelp);

  /***************************************************************/
  SMESHGUI_ScaleDlgLayout->addWidget(ConstructorsBox);
  SMESHGUI_ScaleDlgLayout->addWidget(GroupArguments);
  SMESHGUI_ScaleDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  SpinBox1_1->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox1_2->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox1_3->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  SpinBox_FX->RangeStepAndValidator(1.e-6, 1.e+6, 1.0, "parametric_precision");
  SpinBox_FX->SetStep(0.1);
  SpinBox_FY->RangeStepAndValidator(1.e-6, 1.e+6, 1.0, "parametric_precision");
  SpinBox_FY->SetStep(0.1);
  SpinBox_FZ->RangeStepAndValidator(1.e-6, 1.e+6, 1.0, "parametric_precision");
  SpinBox_FZ->SetStep(0.1);

  RadioButton1->setChecked(true);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // Costruction of the logical filter
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);

  QList<SUIT_SelectionFilter*> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myMeshOrSubMeshOrGroupFilter =
    new SMESH_LogicalFilter(aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "scale_page.html";

  Init();

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));
  connect(GroupConstructors, SIGNAL(buttonClicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectElementsButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton1,        SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),   this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI,       SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),    SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh,     SIGNAL(toggled(bool)),                  SLOT(onSelectMesh(bool)));
  connect(ActionGroup,      SIGNAL(buttonClicked(int)),             SLOT(onActionClicked(int)));

  ConstructorsClicked(0);
  SelectionIntoArgument();
  onActionClicked(MOVE_ELEMS_BUTTON);
}

//=================================================================================
// function : ~SMESHGUI_ScaleDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_ScaleDlg::~SMESHGUI_ScaleDlg()
{
  if ( myFilterDlg ) {
    myFilterDlg->setParent( 0 );
    delete myFilterDlg;
    myFilterDlg = 0;
  }
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::Init (bool ResetControls)
{
  myBusy = false;

  myEditCurrentArgument = 0;
  LineEditElements->clear();
  myElementsId = "";
  myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  myActor = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();

  if (ResetControls) {
    SpinBox1_1->SetValue(0.0);
    SpinBox1_2->SetValue(0.0);
    SpinBox1_3->SetValue(0.0);
    SpinBox_FX->SetValue(1.0);
    SpinBox_FY->SetValue(1.0);
    SpinBox_FZ->SetValue(1.0);

    ActionGroup->button( MOVE_ELEMS_BUTTON )->setChecked(true);
    CheckBoxMesh->setChecked(false);
    onSelectMesh(false);
  }
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_ScaleDlg::ConstructorsClicked (int constructorId)
{
  disconnect(mySelectionMgr, 0, this, 0);

  switch (constructorId) {
  case 0:
    {
      TextLabel2->setText(tr("SMESH_SCALE_FACTOR"));
      TextLabel3->hide();
      TextLabel4->hide();
      SpinBox_FY->hide();
      SpinBox_FZ->hide();
      break;
    }
  case 1:
    {
      TextLabel2->setText(tr("SMESH_SCALE_FACTOR_X"));
      TextLabel3->show();
      TextLabel4->show();
      SpinBox_FY->show();
      SpinBox_FZ->show();
      break;
    }
  }

  if (myEditCurrentArgument != (QWidget*)LineEditElements) {
    SMESH::SetPointRepresentation(false);
    if (!CheckBoxMesh->isChecked())
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode( CellSelection );
  }

  myEditCurrentArgument = (QWidget*)LineEditElements;
  LineEditElements->setFocus();

  if (CheckBoxMesh->isChecked())
    onSelectMesh(true);

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));

  QApplication::instance()->processEvents();
  updateGeometry();
  resize(100,100);
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_ScaleDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if( !isValid() )
    return false;

  if (myNbOkElements) {
    QStringList aListElementsId = myElementsId.split(" ", QString::SkipEmptyParts);

    SMESH::long_array_var anElementsId = new SMESH::long_array;

    anElementsId->length(aListElementsId.count());
    for (int i = 0; i < aListElementsId.count(); i++)
      anElementsId[i] = aListElementsId[i].toInt();

    SMESH::PointStruct aPoint;
    aPoint.x = SpinBox1_1->GetValue();
    aPoint.y = SpinBox1_2->GetValue();
    aPoint.z = SpinBox1_3->GetValue();
    SMESH::double_array_var aScaleFact = new SMESH::double_array;
    aScaleFact->length(3);
    aScaleFact[0] = SpinBox_FX->GetValue();
    if (GetConstructorId() == 0) {
      aScaleFact[1] = SpinBox_FX->GetValue();
      aScaleFact[2] = SpinBox_FX->GetValue();
    }
    else {
      aScaleFact[1] = SpinBox_FY->GetValue();
      aScaleFact[2] = SpinBox_FZ->GetValue();
    }

    QStringList aParameters;
    aParameters << SpinBox1_1->text();
    aParameters << SpinBox1_2->text();
    aParameters << SpinBox1_3->text();
    aParameters << SpinBox_FX->text();
    if (GetConstructorId() == 1) {
      aParameters << SpinBox_FX->text();
      aParameters << SpinBox_FX->text();
    }
    else {
      aParameters << SpinBox_FY->text();
      aParameters << SpinBox_FZ->text();
    }

    int actionButton = ActionGroup->checkedId();
    bool makeGroups = ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() );
    try {
      SUIT_OverrideCursor aWaitCursor;
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      SMESH::SMESH_IDSource_var obj;
      if ( CheckBoxMesh->isChecked() )
        obj = mySelectedObject;
      else
        obj = aMeshEditor->MakeIDSource(anElementsId, SMESH::ALL);

      switch ( actionButton ) {

      case MOVE_ELEMS_BUTTON:
        aMeshEditor->Scale(obj, aPoint, aScaleFact, false);
        if( !myMesh->_is_nil())
          myMesh->SetParameters( aParameters.join(":").toLatin1().constData() );
        break;

      case COPY_ELEMS_BUTTON:
        if ( makeGroups )
          SMESH::ListOfGroups_var groups = 
            aMeshEditor->ScaleMakeGroups(obj, aPoint, aScaleFact);
        else 
          aMeshEditor->Scale(obj, aPoint, aScaleFact, true);
        if( !myMesh->_is_nil())
          myMesh->SetParameters( aParameters.join(":").toLatin1().constData() );
        break;

      case MAKE_MESH_BUTTON: {
        SMESH::SMESH_Mesh_var mesh =
          aMeshEditor->ScaleMakeMesh(obj, aPoint, aScaleFact, makeGroups,
                                     LineEditNewMesh->text().toLatin1().data());
        if (!mesh->_is_nil()) {
          mesh->SetParameters(aParameters.join(":").toLatin1().constData());
#ifdef WITHGENERICOBJ
          // obj has been published in study. Its refcount has been incremented.
          // It is safe to decrement its refcount
          // so that it will be destroyed when the entry in study will be removed
          mesh->Destroy();
#endif
        }
        break;
      }
      }
    } catch (...) {
    }

    SMESH::UpdateView();
    if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() ||
         actionButton == MAKE_MESH_BUTTON )
      mySMESHGUI->updateObjBrowser(true); // new groups may appear
    Init(false);
    ConstructorsClicked(GetConstructorId());
    mySelectedObject = SMESH::SMESH_IDSource::_nil();
    SelectionIntoArgument();

    SMESHGUI::Modified();
  }

  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::ClickOnOk()
{
  if( ClickOnApply() )
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::ClickOnCancel()
{
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
  }
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app)
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser",
                                                                 platform)).
                             arg(myHelpFileName));
  }
}

//=======================================================================
// function : onTextChange()
// purpose  :
//=======================================================================
void SMESHGUI_ScaleDlg::onTextChange (const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  if (myBusy) return;
  BusyLocker lock( myBusy );

  if (send == LineEditElements)
    myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // hilight entered elements
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  if (aMesh) {
    Handle(SALOME_InteractiveObject) anIO = myActor->getIO();

    TColStd_MapOfInteger newIndices;

    QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);

    if (send == LineEditElements) {
      for (int i = 0; i < aListId.count(); i++) {
        const SMDS_MeshElement * e = aMesh->FindElement(aListId[ i ].toInt());
        if (e)
          newIndices.Add(e->GetID());
        myNbOkElements++;
      }
    }

    mySelector->AddOrRemoveIndex( anIO, newIndices, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->highlight( anIO, true, true );

    myElementsId = theNewText;
  }

  if (myNbOkElements) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_ScaleDlg::SelectionIntoArgument()
{
  if (myBusy) return;
  BusyLocker lock( myBusy );
  // clear
  myActor = 0;
  QString aString = "";

  if (myEditCurrentArgument == (QWidget*)LineEditElements) {
    LineEditElements->setText(aString);
    myNbOkElements = 0;
    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);
  }

  if (!GroupButtons->isEnabled()) // inactive
    return;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());

  int nbSel = aList.Extent();
  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();
  myMesh = SMESH::GetMeshByIO(IO);
  if (myMesh->_is_nil())
    return;

  myActor = SMESH::FindActorByObject(myMesh);
  if (!myActor)
    myActor = SMESH::FindActorByEntry(IO->getEntry());
  if (!myActor && !CheckBoxMesh->isChecked())
    return;

  int aNbUnits = 0;

  if (myEditCurrentArgument == (QWidget*)LineEditElements) {
    myElementsId = "";

    // MakeGroups is available if there are groups and "Copy"
    if ( myMesh->NbGroups() == 0 ) {
      MakeGroupsCheck->setChecked(false);
      MakeGroupsCheck->setEnabled(false);
    }
    else if ( ActionGroup->checkedId() != MOVE_ELEMS_BUTTON ) {
      MakeGroupsCheck->setEnabled(true);
    }

    if (CheckBoxMesh->isChecked()) {
      SMESH::GetNameOfSelectedIObjects( mySelectionMgr, aString );

      if (!SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO)->_is_nil()) { //MESH, SUBMESH, OR GROUP
        mySelectedObject = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
      }
      else
        return;
        // get IDs from mesh
        /*
        SMDS_Mesh* aSMDSMesh = myActor->GetObject()->GetMesh();
        if (!aSMDSMesh)
          return;

        for (int i = aSMDSMesh->MinElementID(); i <= aSMDSMesh->MaxElementID(); i++) {
          const SMDS_MeshElement * e = aSMDSMesh->FindElement(i);
          if (e) {
            myElementsId += QString(" %1").arg(i);
            aNbUnits++;
          }
        }
      } else if (!SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO)->_is_nil()) { //SUBMESH
        // get submesh
        SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO);

        // get IDs from submesh
        SMESH::long_array_var anElementsIds = new SMESH::long_array;
        anElementsIds = aSubMesh->GetElementsId();
        for (int i = 0; i < anElementsIds->length(); i++) {
          myElementsId += QString(" %1").arg(anElementsIds[i]);
        }
        aNbUnits = anElementsIds->length();
      } else { // GROUP
        // get smesh group
        SMESH::SMESH_GroupBase_var aGroup =
          SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO);
        if (aGroup->_is_nil())
          return;

        // get IDs from smesh group
        SMESH::long_array_var anElementsIds = new SMESH::long_array;
        anElementsIds = aGroup->GetListOfID();
        for (int i = 0; i < anElementsIds->length(); i++) {
          myElementsId += QString(" %1").arg(anElementsIds[i]);
        }
        aNbUnits = anElementsIds->length();
      }
        */
    } else {
      aNbUnits = SMESH::GetNameOfSelectedElements(mySelector, IO, aString);
      myElementsId = aString;
      if (aNbUnits < 1)
        return;
    }

    myNbOkElements = true;
  } else {
    aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, IO, aString);
    if (aNbUnits != 1)
      return;

    SMDS_Mesh* aMesh =  myActor->GetObject()->GetMesh();
    if (!aMesh)
      return;

    const SMDS_MeshNode * n = aMesh->FindNode(aString.toInt());
    if (!n)
      return;

    double x = n->X();
    double y = n->Y();
    double z = n->Z();

    if (myEditCurrentArgument == (QWidget*)SpinBox1_1) {
      SpinBox1_1->SetValue(x);
      SpinBox1_2->SetValue(y);
      SpinBox1_3->SetValue(z);
    }
    else if (myEditCurrentArgument == (QWidget*)SpinBox_FX) {
      SpinBox_FX->SetValue(x);
      SpinBox_FY->SetValue(y);
      SpinBox_FZ->SetValue(z);
    }
  }

  if (myEditCurrentArgument == (QWidget*)LineEditElements) {
    LineEditElements->setText(aString);
    LineEditElements->repaint();
    LineEditElements->setEnabled(false); // to fully update lineedit IPAL 19809
    LineEditElements->setEnabled(true);
    setNewMeshName();
  }

  // OK
  if (myNbOkElements) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();

  if (send == SelectElementsButton) {
    myEditCurrentArgument = (QWidget*)LineEditElements;
    SMESH::SetPointRepresentation(false);
    if (CheckBoxMesh->isChecked()) {
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode( ActorSelection );
      mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
    }
    else {
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode( CellSelection );
    }
  }
  else if (send == SelectButton1) {
    myEditCurrentArgument = (QWidget*)SpinBox1_1;
    SMESH::SetPointRepresentation(true);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( NodeSelection );
  }

  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::DeactivateActiveDialog()
{
  if (ConstructorsBox->isEnabled()) {
    ConstructorsBox->setEnabled(false);
    GroupArguments->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  ConstructorsBox->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( CellSelection );

  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::enterEvent (QEvent*)
{
  if (!ConstructorsBox->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_ScaleDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}

//=======================================================================
//function : onSelectMesh
//purpose  :
//=======================================================================
void SMESHGUI_ScaleDlg::onSelectMesh (bool toSelectMesh)
{
  if (toSelectMesh)
    TextLabelElements->setText(tr("SMESH_NAME"));
  else
    TextLabelElements->setText(tr("SMESH_ID_ELEMENTS"));
  myFilterBtn->setEnabled(!toSelectMesh);

  if (myEditCurrentArgument != LineEditElements) {
    LineEditElements->clear();
    return;
  }

  mySelectionMgr->clearFilters();
  SMESH::SetPointRepresentation(false);

  if (toSelectMesh) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( ActorSelection );
    mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
    LineEditElements->setReadOnly(true);
    LineEditElements->setValidator(0);
  } else {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( CellSelection );
    LineEditElements->setReadOnly(false);
    LineEditElements->setValidator(myIdValidator);
    onTextChange(LineEditElements->text());
  }

  SelectionIntoArgument();
}

//=======================================================================
//function : onActionClicked
//purpose  : slot called when an action type changed
//=======================================================================

void SMESHGUI_ScaleDlg::onActionClicked(int button)
{
  switch ( button ) {
  case MOVE_ELEMS_BUTTON:
    MakeGroupsCheck->setEnabled(false);
    LineEditNewMesh->setEnabled(false);
    break;
  case COPY_ELEMS_BUTTON:
    LineEditNewMesh->setEnabled(false);
    MakeGroupsCheck->setText( tr("SMESH_MAKE_GROUPS"));
    if ( myMesh->_is_nil() || myMesh->NbGroups() > 0)
      MakeGroupsCheck->setEnabled(true);
    else
      MakeGroupsCheck->setEnabled(false);
    break;
  case MAKE_MESH_BUTTON:
    LineEditNewMesh->setEnabled(true);
    MakeGroupsCheck->setText( tr("SMESH_COPY_GROUPS"));
    if ( myMesh->_is_nil() || myMesh->NbGroups() > 0)
      MakeGroupsCheck->setEnabled(true);
    else
      MakeGroupsCheck->setEnabled(false);
    break;
  }
  setNewMeshName();
}

//=======================================================================
//function : setNewMeshName
//purpose  : update contents of LineEditNewMesh
//=======================================================================

void SMESHGUI_ScaleDlg::setNewMeshName()
{
  LineEditNewMesh->setText("");
  if ( LineEditNewMesh->isEnabled() && !myMesh->_is_nil() ) {
    QString name;
    if ( CheckBoxMesh->isChecked() ) {
      name = LineEditElements->text();
    }
    else {
      _PTR(SObject) meshSO = SMESH::FindSObject( myMesh );
      name = meshSO->GetName().c_str();
    }
    if ( !name.isEmpty() )
      LineEditNewMesh->setText( SMESH::UniqueMeshName( name, "scaled"));
  }
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_ScaleDlg::GetConstructorId()
{
  return GroupConstructors->checkedId();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ScaleDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}

//=================================================================================
// function : setFilters()
// purpose  : SLOT. Called when "Filter" button pressed.
//=================================================================================
void SMESHGUI_ScaleDlg::setFilters()
{
  if(myMesh->_is_nil()) {
    SUIT_MessageBox::critical(this,
                              tr("SMESH_ERROR"),
                              tr("NO_MESH_SELECTED"));
   return;
  }
  if ( !myFilterDlg )
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, SMESH::ALL );

  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( LineEditElements );

  myFilterDlg->show();
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_ScaleDlg::isValid()
{
  bool ok = true;
  QString msg;

  ok = SpinBox1_1->isValid( msg, true ) && ok;
  ok = SpinBox1_2->isValid( msg, true ) && ok;
  ok = SpinBox1_3->isValid( msg, true ) && ok;
  ok = SpinBox_FX->isValid( msg, true ) && ok;
  if (GetConstructorId() == 1) {
    ok = SpinBox_FY->isValid( msg, true ) && ok;
    ok = SpinBox_FZ->isValid( msg, true ) && ok;
  }

  if( !ok ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return false;
  }
  return true;
}
