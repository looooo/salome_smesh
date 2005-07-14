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
//  File   : SMESHGUI_NodesOp.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_NodesOp.h"
#include <SMESHGUI_NodesDlg.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI_VTKUtils.h>
#include <SMESHGUI_MeshUtils.h>
#include <SMESHGUI.h>

#include <SMESH_ActorUtils.h>
#include <SMESH_Actor.h>
#include <SMESH_ObjectDef.h>

#include <SalomeApp_SelectionMgr.h>

#include <SALOME_ListIO.hxx>

#include <SUIT_OverrideCursor.h>
#include <SUIT_MessageBox.h>

#include <SVTK_ViewWindow.h>
#include <SVTK_Selector.h>

#include <SMDS_Mesh.hxx>
#include <SMDS_MeshNode.hxx>

#include "utilities.h"

#include <vtkDataSetMapper.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkIntArray.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkActorCollection.h>
#include <vtkRenderer.h>

namespace SMESH {

  void AddNode (SMESH::SMESH_Mesh_ptr theMesh, float x, float y, float z)
  {
    SUIT_OverrideCursor wc;
    try {
      _PTR(SObject) aSobj = SMESH::FindSObject(theMesh);
      SMESH::SMESH_MeshEditor_var aMeshEditor = theMesh->GetMeshEditor();
      aMeshEditor->AddNode(x,y,z);
      _PTR(Study) aStudy = GetActiveStudyDocument();
      CORBA::Long anId = aStudy->StudyId();
      if (TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId, aSobj->GetID().c_str())) {
	aVisualObj->Update(true);
      }
    } catch (SALOME::SALOME_Exception& exc) {
      INFOS("Follow exception was cought:\n\t" << exc.details.text);
    } catch (const std::exception& exc) {
      INFOS("Follow exception was cought:\n\t" << exc.what());
    } catch (...) {
      INFOS("Unknown exception was cought !!!");
    }
  }

  class TNodeSimulation {
    SVTK_ViewWindow* myViewWindow;

    SALOME_Actor *myPreviewActor;
    vtkDataSetMapper* myMapper;
    vtkPoints* myPoints;

  public:
    TNodeSimulation(SVTK_ViewWindow* theViewWindow):
      myViewWindow(theViewWindow)
    {
      vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();

      // Create points
      myPoints = vtkPoints::New();
      myPoints->SetNumberOfPoints(1);
      myPoints->SetPoint(0,0.0,0.0,0.0);

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

      vtkIntArray* aCellLocationsArray = vtkIntArray::New();
      aCellLocationsArray->SetNumberOfComponents(1);
      aCellLocationsArray->SetNumberOfTuples(1);

      aCells->InitTraversal();
      vtkIdType npts;
      aCellLocationsArray->SetValue(0, aCells->GetTraversalLocation(npts));

      aGrid->SetCells(aCellTypesArray, aCellLocationsArray, aCells);

      aGrid->SetPoints(myPoints);
      aGrid->SetCells(aCellTypesArray, aCellLocationsArray,aCells);
      aCellLocationsArray->Delete();
      aCellTypesArray->Delete();
      aCells->Delete();
      anIdList->Delete();

      // Create and display actor
      myMapper = vtkDataSetMapper::New();
      myMapper->SetInput(aGrid);
      aGrid->Delete();

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->SetInfinitive(true);
      myPreviewActor->VisibilityOff();
      myPreviewActor->PickableOff();
      myPreviewActor->SetMapper(myMapper);

      vtkProperty* aProp = vtkProperty::New();
      aProp->SetRepresentationToPoints();

      float anRGB[3];
      GetColor( "SMESH", "node_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 255, 0 ) );
      aProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );

      float aPointSize = GetFloat( "SMESH:node_size", 3 );
      aProp->SetPointSize( aPointSize );

      myPreviewActor->SetProperty( aProp );
      aProp->Delete();

      myViewWindow->AddActor(myPreviewActor);
    }

    void SetPosition (float x, float y, float z)
    {
      myPoints->SetPoint(0,x,y,z);
      myPoints->Modified();
      SetVisibility(true);
    }

    void SetVisibility (bool theVisibility)
    {
      myPreviewActor->SetVisibility(theVisibility);
      RepaintCurrentView();
    }

    ~TNodeSimulation()
    {
      SetVisibility( false );
      myViewWindow->RemoveActor(myPreviewActor);
      myPreviewActor->Delete();

      myMapper->RemoveAllInputs();
      myMapper->Delete();

      myPoints->Delete();
    }
  };
}


//=================================================================================
// function : 
// purpose  :
//=================================================================================
SMESHGUI_NodesOp::SMESHGUI_NodesOp()
: SMESHGUI_SelectionOp( NodeSelection ),
  myDlg( 0 ),
  mySimulation( 0 )
{
}

//=================================================================================
// function : 
// purpose  :
//=================================================================================
SMESHGUI_NodesOp::~SMESHGUI_NodesOp()
{
  if( myDlg )
    delete myDlg;
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
SalomeApp_Dialog* SMESHGUI_NodesOp::dlg() const
{
  return myDlg;
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_NodesOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_NodesDlg();
    connect( myDlg, SIGNAL( valueChanged( double ) ), this, SLOT( onValueChanged( double ) ) );
  }

  SMESHGUI_SelectionOp::startOperation();

  mySimulation = new SMESH::TNodeSimulation( viewWindow() );
  myDlg->show();
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_NodesOp::commitOperation()
{
  if( mySimulation )
    delete mySimulation;
  mySimulation = 0;
  SMESHGUI_SelectionOp::commitOperation();
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_NodesOp::abortOperation()
{
  if( mySimulation )
    delete mySimulation;
  mySimulation = 0;
  SMESHGUI_SelectionOp::abortOperation();
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_NodesOp::selectionDone()
{
  if( !mySimulation )
    return;
    
  mySimulation->SetVisibility(false);
  SMESH::SetPointRepresentation(true);

  const SALOME_ListIO& aList = selector()->StoredIObjects();
  if (aList.Extent() == 1) {
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    if (anIO->hasEntry()) {
      myMesh = SMESH::GetMeshByIO(anIO);
      if (myMesh->_is_nil()) return;
      QString aText;
      if (SMESH::GetNameOfSelectedNodes(selector(),anIO,aText) == 1) {
	if (SMESH_Actor* anActor = SMESH::FindActorByObject(myMesh.in())) {
	  if (SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh()) {
	    if (const SMDS_MeshNode* aNode = aMesh->FindNode(aText.toInt())) {
        myDlg->setCoords( aNode->X(), aNode->Y(), aNode->Z() );
            }
	  }
	}
      }
      double x, y, z;
      myDlg->coords( x, y, z );
      mySimulation->SetPosition( x, y, z );
    }
  } 
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_NodesOp::initDialog()
{
  myDlg->setCoords( 0, 0, 0 );
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
bool SMESHGUI_NodesOp::onApply()
{
  if( getSMESHGUI()->isActiveStudyLocked() || !mySimulation )
    return false;

  if (myMesh->_is_nil()) {
    SUIT_MessageBox::warn1( myDlg, tr("SMESH_WRN_WARNING"),
                            tr("MESH_IS_NOT_SELECTED"), tr("SMESH_BUT_OK"));
    return false;
  }

  double x, y, z; myDlg->coords( x, y, z );
  mySimulation->SetVisibility(false);
  SMESH::AddNode(myMesh,x,y,z);
  SMESH::SetPointRepresentation(true);

  // select myMesh
  SALOME_ListIO aList;
  selectionMgr()->selectedObjects(aList);
  if (aList.Extent() != 1) {
    if (SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView()) {
      vtkActorCollection *aCollection = aViewWindow->getRenderer()->GetActors();
      aCollection->InitTraversal();
      while (vtkActor *anAct = aCollection->GetNextActor()) {
        if (SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)) {
          if (anActor->hasIO()) {
            if (SMESH_MeshObj *aMeshObj = dynamic_cast<SMESH_MeshObj*>(anActor->GetObject().get())) {
              if (myMesh->_is_equivalent(aMeshObj->GetMeshServer())) {
                aList.Clear();
                aList.Append(anActor->getIO());
                selectionMgr()->setSelectedObjects(aList, false);
                break;
              }
            }
          }
        }
      }
    }
  }
  return true; 
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_NodesOp::onValueChanged( double )
{
  double vx, vy, vz; myDlg->coords( vx, vy, vz );
  if( mySimulation )
    mySimulation->SetPosition( vx, vy, vz );
}



