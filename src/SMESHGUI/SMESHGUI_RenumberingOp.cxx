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
//  File   : SMESHGUI_RenumberingOp.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#include "SMESHGUI_RenumberingOp.h"
#include <SMESHGUI_RenumberingDlg.h>
#include <SMESHGUI.h>
#include <SMESHGUI_VTKUtils.h>

#include <SMESH_TypeFilter.hxx>
#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>

#include <SUIT_OverrideCursor.h>

#include <SALOMEDS_SObject.hxx>

//=================================================================================
// name     : SMESHGUI_RenumberingOp
// purpose  : 
//=================================================================================
SMESHGUI_RenumberingOp::SMESHGUI_RenumberingOp( const int mode )
: SMESHGUI_SelectionOp(),
  myMode( mode ),
  myDlg( 0 )
{ 
}

//=================================================================================
// name     : ~SMESHGUI_RenumberingOp
// purpose  :
//=================================================================================
SMESHGUI_RenumberingOp::~SMESHGUI_RenumberingOp()
{
  if( myDlg )
    delete myDlg;
}

//=================================================================================
// name     : dlg
// purpose  :
//=================================================================================
SalomeApp_Dialog* SMESHGUI_RenumberingOp::dlg() const
{
  return myDlg;
}

//=================================================================================
// name     : startOperation
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingOp::startOperation()
{
  if( !myDlg )
    myDlg = new SMESHGUI_RenumberingDlg( myMode );
    
  SMESHGUI_SelectionOp::startOperation();
  
  updateDialog();
  myDlg->show();
}

//=================================================================================
// name     : selectionDone
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingOp::selectionDone()
{
  SMESHGUI_SelectionOp::selectionDone();
  updateDialog();
}

//=================================================================================
// name     : createFilter
// purpose  :
//=================================================================================
SUIT_SelectionFilter* SMESHGUI_RenumberingOp::createFilter( const int id ) const
{
  if( id==0 )
    return new SMESH_TypeFilter( MESH );
  else
    return 0;
}

//=================================================================================
// name     : updateDialog
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingOp::updateDialog()
{
  if( !myDlg )
    return;
    
  bool en = myDlg->hasSelection( 0 );
  myDlg->setButtonEnabled( en, QtxDialog::OK | QtxDialog::Apply );
}

//=================================================================================
// name     : onApply
// purpose  :
//=================================================================================
bool SMESHGUI_RenumberingOp::onApply()
{
  if( isStudyLocked() )
    return false;

  if( myDlg->hasSelection( 0 ) )
  {
    QStringList ids; myDlg->selectedObject( 0, ids );
    _PTR(SObject) meshSO = studyDS()->FindObjectID( ids.first() );
    
    SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( _CAST(SObject,meshSO)->GetObject() );
    if( aMesh->_is_nil() )
      return false;

    SUIT_OverrideCursor c;
    try
    {
      SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
      SMESH_Actor* anActor = SMESH::FindActorByObject( aMesh );
      bool isUnitsLabeled = false;

      if( myMode == 0 && anActor )
      {
        isUnitsLabeled = anActor->GetPointsLabeled();
        if( isUnitsLabeled )
          anActor->SetPointsLabeled( false );
      }
      else if( myMode == 1 && anActor )
      {
        isUnitsLabeled = anActor->GetCellsLabeled();
        if( isUnitsLabeled )
          anActor->SetCellsLabeled( false );
      }

      if( myMode == 0 )
      {
        aMeshEditor->RenumberNodes();
        if( isUnitsLabeled && anActor )
          anActor->SetPointsLabeled( true );
      }
      else if( myMode == 1 )
      {
        aMeshEditor->RenumberElements();
        if( isUnitsLabeled && anActor )
          anActor->SetCellsLabeled(true);
      }
    }
    catch(...)
    {
    }

    initDialog();
    updateDialog();
    SMESH::UpdateView();
    return true;
  }
  return false;
}

