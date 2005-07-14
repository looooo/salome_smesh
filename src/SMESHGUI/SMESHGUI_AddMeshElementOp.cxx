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
//  File   : SMESHGUI_AddMeshElementOp.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#include "SMESHGUI_AddMeshElementOp.h"
#include <SMESHGUI_AddMeshElementDlg.h>
#include <SMESHGUI.h>

#include <SalomeApp_SelectionMgr.h>

#include <SVTK_ViewWindow.h>
#include <SVTK_ViewModel.h>
#include <SVTK_Selector.h>
#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESHGUI_VTKUtils.h>

#include <vtkCell.h>
#include <vtkDataSetMapper.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>

namespace SMESH {

  class TElementSimulation {
    SVTK_ViewWindow* myVTKViewWindow;
    SALOME_Actor* myPreviewActor;
    vtkDataSetMapper* myMapper;
    vtkUnstructuredGrid* myGrid;

  public:
    TElementSimulation( SVTK_ViewWindow* wnd )
    {
      myVTKViewWindow = wnd;

      myGrid = vtkUnstructuredGrid::New();

      // Create and display actor
      myMapper = vtkDataSetMapper::New();
      myMapper->SetInput(myGrid);

      myPreviewActor = SALOME_Actor::New();
      myPreviewActor->PickableOff();
      myPreviewActor->VisibilityOff();
      myPreviewActor->SetMapper(myMapper);

      float anRGB[3];
      vtkProperty* aProp = vtkProperty::New();
      GetColor( "SMESH", "fill_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
      aProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );
      myPreviewActor->SetProperty( aProp );
      aProp->Delete();

      vtkProperty* aBackProp = vtkProperty::New();
      GetColor( "SMESH", "backface_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 0, 255 ) );
      aBackProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );
      myPreviewActor->SetBackfaceProperty( aBackProp );
      aBackProp->Delete();

      myVTKViewWindow->AddActor(myPreviewActor);
    }

    typedef std::vector<vtkIdType> TVTKIds;
    void SetPosition (SMESH_Actor* theActor,
                      vtkIdType theType,
                      const TVTKIds& theIds)
    {
      vtkUnstructuredGrid *aGrid = theActor->GetUnstructuredGrid();
      myGrid->SetPoints(aGrid->GetPoints());

      const int* aConn = NULL;
      switch (theType) {
      case VTK_TETRA:
        {
          static int anIds[] = {0,2,1,3};
          aConn = anIds;
          break;
        }
      case VTK_PYRAMID:
        {
          static int anIds[] = {0,3,2,1,4};
          aConn = anIds;
          break;
        }
      case VTK_HEXAHEDRON:
        {
          static int anIds[] = {0,3,2,1,4,7,6,5};
          aConn = anIds;
          break;
        }
      }

      myGrid->Reset();
      vtkIdList *anIds = vtkIdList::New();

      if(aConn)
	for (int i = 0, iEnd = theIds.size(); i < iEnd; i++)
	  anIds->InsertId(i,theIds[aConn[i]]);
      else
	for (int i = 0, iEnd = theIds.size(); i < iEnd; i++)
	  anIds->InsertId(i,theIds[i]);

      myGrid->InsertNextCell(theType,anIds);
      anIds->Delete();

      myGrid->Modified();

      SetVisibility(true);
    }


    void SetVisibility (bool theVisibility)
    {
      myPreviewActor->SetVisibility(theVisibility);
      RepaintCurrentView();
    }


    ~TElementSimulation()
    {
      SetVisibility( false );
      if( myVTKViewWindow )
        myVTKViewWindow->RemoveActor(myPreviewActor);
        
      myPreviewActor->Delete();

      myMapper->RemoveAllInputs();
      myMapper->Delete();

      myGrid->Delete();
    }
  };
}

//=================================================================================
// function : SMESHGUI_AddMeshElementDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_AddMeshElementOp::SMESHGUI_AddMeshElementOp( const SMDSAbs_ElementType t, const int nbNodes )
: SMESHGUI_SelectionOp( NodeSelection ),
  myElementType( t ),
  myNbNodes( nbNodes ),
  myIsPoly( myElementType==SMDSAbs_Face && myNbNodes==5 ),
  mySimulation( 0 ),
  myDlg( 0 )
{
}

//=================================================================================
// function : ~SMESHGUI_AddMeshElementDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_AddMeshElementOp::~SMESHGUI_AddMeshElementOp()
{
  if( myDlg )
    delete myDlg;
}

//=================================================================================
// function : dlg()
// purpose  : 
//=================================================================================
SalomeApp_Dialog* SMESHGUI_AddMeshElementOp::dlg() const
{
  return myDlg;
}

//=================================================================================
// function : startOperation()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementOp::startOperation()
{
  if( !myDlg )
  {
    QString elemName;
    switch( myNbNodes )
    {
      case 2: elemName = "EDGE"; break;
      case 3: elemName = "TRIANGLE"; break;
      case 4: if (myElementType == SMDSAbs_Face)
                elemName = "QUADRANGLE";
              else
                elemName = "TETRAS";
              break;
      case 5: elemName = "POLYGON"; break;
      case 8: elemName = "HEXAS"; break;
    }
    
    myDlg = new SMESHGUI_AddMeshElementDlg( elemName, myElementType == SMDSAbs_Face );
    connect( myDlg, SIGNAL( objectChanged( int, const QStringList& ) ), this, SLOT( onTextChanged( int, const QStringList& ) ) );
    connect( myDlg, SIGNAL( reverse( int ) ), this, SLOT( onReverse( int ) ) );
  }

  SMESHGUI_SelectionOp::startOperation();

  mySimulation = new SMESH::TElementSimulation( viewWindow() );
  updateDialog();
  myDlg->show();
}

//=================================================================================
// function : onSelectionChanged()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementOp::onSelectionChanged( int id )
{
  if( !mySimulation )
    return;
    
  mySimulation->SetVisibility(false);

  SMESHGUI_SelectionOp::onSelectionChanged( id );

  updateDialog();
  displaySimulation();
}

//=================================================================================
// function : commitOperation()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementOp::commitOperation()
{
  if( mySimulation )
    delete mySimulation;
  mySimulation = 0;
  SMESHGUI_SelectionOp::commitOperation();
}

//=================================================================================
// function : abortOperation()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementOp::abortOperation()
{
  if( mySimulation )
    delete mySimulation;
  mySimulation = 0;
  SMESHGUI_SelectionOp::abortOperation();
}

//=================================================================================
// function : onApply()
// purpose  :
//=================================================================================
bool SMESHGUI_AddMeshElementOp::onApply()
{
  if( !mySimulation )
    return false;

  IdList ids; selectedIds( 0, ids );
  if( ids.count()>=myNbNodes && !getSMESHGUI()->isActiveStudyLocked() ) {
    //myBusy = true;
    SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
    anArrayOfIdeces->length(myNbNodes);
    bool reverse = myDlg->isReverse();
    
    for (int i = 0; i < myNbNodes; i++)
    {
      if (reverse)
        anArrayOfIdeces[ myNbNodes - i - 1 ] = ids[i];
      else
        anArrayOfIdeces[i] = ids[i];
    }

    SMESH::SMESH_MeshEditor_var aMeshEditor = mesh()->GetMeshEditor();
    switch (myElementType) {
    case SMDSAbs_Edge:
      aMeshEditor->AddEdge(anArrayOfIdeces.inout()); break;
    case SMDSAbs_Face:
      aMeshEditor->AddFace(anArrayOfIdeces.inout()); break;
    case SMDSAbs_Volume:
      aMeshEditor->AddVolume(anArrayOfIdeces.inout()); break;
    default:;
    }

    //SALOME_ListIO aList; aList.Append( myActor->getIO() );
    //mySelectionMgr->setSelectedObjects( aList, false );

    SMESH::UpdateView();
    mySimulation->SetVisibility(false);

    initDialog();
    updateDialog();

    return true;
  }
  return false;
}

//=================================================================================
// function : displaySimulation()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementOp::displaySimulation()
{
  IdList ids; selectedIds( 0, ids );
  
  if( ids.count()>=myNbNodes ) {
    SMESH::TElementSimulation::TVTKIds anIds;

    SMESH_Actor* anActor = actor();
    if( !anActor )
      return;
    
    for (int i = 0; i < myNbNodes; i++)    
      anIds.push_back( anActor->GetObject()->GetNodeVTKId( ids[i] ) );

    if( myDlg->isReverse() )
      reverse( anIds.begin(), anIds.end() );

    vtkIdType aType = 0;
    if (myIsPoly)
      switch ( myElementType ) {
      case SMDSAbs_Face  : aType = VTK_POLYGON; break;
      default: return;
      }
    else {
      switch (myNbNodes) {
      case 2: aType = VTK_LINE; break;
      case 3: aType = VTK_TRIANGLE; break;
      case 4: aType = myElementType == SMDSAbs_Face ? VTK_QUAD : VTK_TETRA; break;
      case 8: aType = VTK_HEXAHEDRON; break;
      default: return;
      }
    }

    mySimulation->SetPosition( anActor, aType, anIds );
    SMESH::UpdateView();
  }  
}

//=================================================================================
// function : onReverse()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementOp::onReverse( int )
{
  if( mySimulation )
  {
    mySimulation->SetVisibility( false );
    displaySimulation();
  }
}

//=================================================================================
// function : updateDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddMeshElementOp::updateDialog()
{
  if( myDlg )
  {
    IdList ids; selectedIds( 0, ids );
    myDlg->setButtonEnabled( ids.count()>=myNbNodes, QtxDialog::OK | QtxDialog::Apply );
  }
}
