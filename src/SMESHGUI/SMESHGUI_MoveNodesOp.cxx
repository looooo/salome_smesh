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
//  File   : SMESHGUI_MoveNodesOp.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#include "SMESHGUI_MoveNodesOp.h"
#include <SMESHGUI_MoveNodesDlg.h>
#include <SMESHGUI.h>
#include <SMESHGUI_VTKUtils.h>

#include <SalomeApp_SelectionMgr.h>

#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>

#include <SALOME_ListIO.hxx>

#include <SVTK_ViewWindow.h>

#include <SMDS_Mesh.hxx>
#include <SMDS_MeshNode.hxx>

#include <SMESH_Actor.h>

#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedCharArray.h>

//=======================================================================
// name    : SMESHGUI_MoveNodesOp
// Purpose :
//=======================================================================
SMESHGUI_MoveNodesOp::SMESHGUI_MoveNodesOp()
: SMESHGUI_SelectionOp( NodeSelection ),
  myDlg( 0 ),
  myPreviewActor( 0 )
{
}

//=======================================================================
// name    : ~SMESHGUI_MoveNodesOp
// Purpose : 
//=======================================================================
SMESHGUI_MoveNodesOp::~SMESHGUI_MoveNodesOp()
{
  if( myDlg )
    delete myDlg;
}

//=======================================================================
// name    : dlg
// Purpose :
//=======================================================================
SalomeApp_Dialog* SMESHGUI_MoveNodesOp::dlg() const
{
  return myDlg;
}

//=======================================================================
// name    : startOperation
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_MoveNodesDlg();
    connect( myDlg, SIGNAL( valueChanged() ), this, SLOT( redisplayPreview() ) );
  }

  SMESHGUI_SelectionOp::startOperation();

  updateDialog();
  myDlg->show();
}

//=======================================================================
// name    : selectionDone
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesOp::selectionDone()
{
  SMESHGUI_SelectionOp::selectionDone();

  SMESH_Actor* a = actor();
  if( myDlg && myDlg->hasSelection( 0 ) && a )
  {
    SMDS_Mesh* aMesh = a->GetObject()->GetMesh();
    if( aMesh )
    {
      IdList ids; selectedIds( 0, ids );
      if( const SMDS_MeshNode* aNode = aMesh->FindNode( ids.first() ) )
        myDlg->setCoords( aNode->X(), aNode->Y(), aNode->Z() );
    }
  }
  updateDialog();
}

//=======================================================================
// name    : updateDialog
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesOp::updateDialog()
{
  if( !myDlg )
    return;
    
  bool en = myDlg->hasSelection( 0 );
  myDlg->setButtonEnabled( en, QtxDialog::OK | QtxDialog::Apply );
}

//=======================================================================
// name    : initDialog
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesOp::initDialog()
{
  SMESHGUI_SelectionOp::initDialog();
  if( myDlg )
    myDlg->setCoords( 0, 0, 0 );
}
  

//=======================================================================
// name    : onApply
// Purpose :
//=======================================================================
bool SMESHGUI_MoveNodesOp::onApply()
{
  if( isStudyLocked() )
    return false;

  if( !myDlg->hasSelection( 0 ) )
    return false;

  SMESH::SMESH_Mesh_var aMesh = mesh();
  if (aMesh->_is_nil()) {
    SUIT_MessageBox::info1( desktop(), tr("SMESH_ERROR"),
                            tr("SMESHG_NO_MESH"), tr( "OK" ) );
    return false;
  }

  SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
  if (aMeshEditor->_is_nil())
    return false;

  IdList ids; selectedIds( 0, ids );
  int anId = ids.first();
  bool aResult = false;
  try {
    double x, y, z;
    myDlg->coords( x, y, z );
    aResult = aMeshEditor->MoveNode( anId, x, y, z );
  } catch (...) {
  }

  if (aResult) {
    SALOME_ListIO aList;
    aList.Append( actor()->getIO());
    selectionMgr()->setSelectedObjects(aList,false);
    SMESH::UpdateView();
    initDialog();
  }

  return aResult;
}  

//=======================================================================
// name    : redisplayPreview
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesOp::redisplayPreview()
{
  if (myPreviewActor != 0)
    erasePreview();

  if( !myDlg && !myDlg->hasSelection( 0 ) )
    return;

  vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();

  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints(1);
  double x, y, z; myDlg->coords( x, y, z );
  aPoints->SetPoint( 0, x, y, z );

  // Create cells

  vtkIdList *anIdList = vtkIdList::New();
  anIdList->SetNumberOfIds(1);

  vtkCellArray *aCells = vtkCellArray::New();
  aCells->Allocate(2, 0);

  vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
  aCellTypesArray->SetNumberOfComponents(1);
  aCellTypesArray->Allocate(1);

  anIdList->SetId(0, 0);
  aCells->InsertNextCell(anIdList);
  aCellTypesArray->InsertNextValue(VTK_VERTEX);
  anIdList->Delete();

  vtkIntArray* aCellLocationsArray = vtkIntArray::New();
  aCellLocationsArray->SetNumberOfComponents(1);
  aCellLocationsArray->SetNumberOfTuples(1);

  aCells->InitTraversal();
  vtkIdType npts;
  aCellLocationsArray->SetValue(0, aCells->GetTraversalLocation(npts));

  aGrid->SetPoints(aPoints);
  aPoints->Delete();

  aGrid->SetCells(aCellTypesArray,aCellLocationsArray,aCells);
  aCellLocationsArray->Delete();
  aCellTypesArray->Delete();
  aCells->Delete();

  // Create and display actor
  vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
  aMapper->SetInput(aGrid);
  aGrid->Delete();

  myPreviewActor = SALOME_Actor::New();
  myPreviewActor->PickableOff();
  myPreviewActor->SetMapper(aMapper);
  aMapper->Delete();

  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor(250, 0, 250);
  aProp->SetPointSize(5);
  myPreviewActor->SetProperty(aProp);
  aProp->Delete();

  viewWindow()->AddActor(myPreviewActor);
  viewWindow()->Repaint();
}  

//=======================================================================
// name    : erasePreview
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesOp::erasePreview()
{
  if (myPreviewActor == 0)
    return;

  viewWindow()->RemoveActor(myPreviewActor);
  myPreviewActor->Delete();
  myPreviewActor = 0;
  viewWindow()->Repaint();
}  

//=======================================================================
// name    : commitOperation
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesOp::commitOperation()
{
  SMESHGUI_SelectionOp::commitOperation();
  erasePreview();
}

//=======================================================================
// name    : abortOperation
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesOp::abortOperation()
{
  SMESHGUI_SelectionOp::abortOperation();
  erasePreview();
}

