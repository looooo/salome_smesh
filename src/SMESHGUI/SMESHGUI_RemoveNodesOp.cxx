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
//  File   : SMESHGUI_RemoveNodesOp.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#include "SMESHGUI_RemoveNodesOp.h"
#include <SMESHGUI_RemoveNodesDlg.h>
#include <SMESHGUI.h>
#include <SMESHGUI_VTKUtils.h>

//=================================================================================
// function : 
// purpose  : 
//=================================================================================
SMESHGUI_RemoveNodesOp::SMESHGUI_RemoveNodesOp()
: SMESHGUI_SelectionOp( NodeSelection ),
  myDlg( 0 )
{
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
SMESHGUI_RemoveNodesOp::~SMESHGUI_RemoveNodesOp()
{
  if( myDlg )
    delete myDlg;
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
SalomeApp_Dialog* SMESHGUI_RemoveNodesOp::dlg() const
{
  return myDlg;
}  

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_RemoveNodesDlg();
    connect( myDlg, SIGNAL( objectChanged( int, const QStringList& ) ), this, SLOT( onTextChanged( int, const QStringList& ) ) );
  }

  SMESHGUI_SelectionOp::startOperation();

  myDlg->show();
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesOp::selectionDone()
{
  SMESHGUI_SelectionOp::selectionDone();
  updateDialog();
} 

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_RemoveNodesOp::updateDialog()
{
  if( !myDlg )
    return;
    
  bool en = myDlg->hasSelection( 0 );
  myDlg->setButtonEnabled( en, QtxDialog::OK | QtxDialog::Apply );
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
bool SMESHGUI_RemoveNodesOp::onApply()
{
  if( getSMESHGUI()->isActiveStudyLocked() )
    return false;

  IdList ids; selectedIds( 0, ids );

  SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
  anArrayOfIdeces->length(ids.count());
  for (int i=0, n=ids.count(); i<n; i++)
    anArrayOfIdeces[i] = ids[i];

  bool aResult = false;
  try
  {
    SMESH::SMESH_MeshEditor_var aMeshEditor = mesh()->GetMeshEditor();
    aResult = aMeshEditor->RemoveNodes( anArrayOfIdeces.inout() );
  }
  catch(...)
  {
  }

  if (aResult)
  {
    initDialog();
    SMESH::UpdateView();
  }

  SMESH::SetPointRepresentation(true);
  return aResult;
}

