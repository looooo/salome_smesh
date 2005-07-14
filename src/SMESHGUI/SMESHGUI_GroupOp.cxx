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
//  File   : SMESHGUI_GroupOp.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include <SMESHGUI_GroupOp.h>
#include <SMESHGUI_GroupOpDlg.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESHGUI.h>

#include <SalomeApp_SelectionMgr.h>
#include <SalomeApp_UpdateFlags.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>

#include <SALOMEDS_SObject.hxx>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)


/*
  Class       : SMESHGUI_GroupOp
  Description : Perform boolean operations on groups
*/

//=======================================================================
// name    : SMESHGUI_GroupOp
// Purpose : 
//=======================================================================
SMESHGUI_GroupOp::SMESHGUI_GroupOp( const int mode )
: SMESHGUI_SelectionOp(),
  myMode( mode ),
  myDlg( 0 )
{
}

//=======================================================================
// name    : ~SMESHGUI_GroupOp
// Purpose :
//=======================================================================
SMESHGUI_GroupOp::~SMESHGUI_GroupOp()
{
  if( myDlg )
    delete myDlg;
}

//=======================================================================
// name    : dlg
// Purpose :
//=======================================================================
SalomeApp_Dialog* SMESHGUI_GroupOp::dlg() const
{
  return myDlg;
}

//=======================================================================
// name    : onApply
// Purpose :
//=======================================================================
bool SMESHGUI_GroupOp::onApply()
{
  if( !myDlg || !isValid() || getSMESHGUI()->isActiveStudyLocked() )
    return false;

  QStringList selGroup[2];
  for( int i=1; i<=2; i++ )
    myDlg->selectedObject( i, selGroup[i-1] );
    
  _PTR(SObject) pGroup1 = studyDS()->FindObjectID( selGroup[0].first() ),
                pGroup2 = studyDS()->FindObjectID( selGroup[1].first() );
  SMESH::SMESH_GroupBase_var aGroup1 = SMESH::SMESH_GroupBase::_narrow( _CAST(SObject,pGroup1)->GetObject() ),
                             aGroup2 = SMESH::SMESH_GroupBase::_narrow( _CAST(SObject,pGroup2)->GetObject() );
    
  SMESH::SMESH_Mesh_ptr aMesh = aGroup1->GetMesh();
  QString aName = myDlg->name();
  SMESH::SMESH_Group_ptr aNewGrp = SMESH::SMESH_Group::_nil();

  if( myMode == UNION )
    aNewGrp = aMesh->UnionGroups( aGroup1, aGroup2, aName.latin1() );
    
  else if( myMode == INTERSECT )
    aNewGrp = aMesh->IntersectGroups( aGroup1, aGroup2, aName.latin1() );
    
  else
    aNewGrp = aMesh->CutGroups( aGroup1, aGroup2, aName.latin1() );

  if (!aNewGrp->_is_nil())
  {
    update( UF_Model | UF_ObjBrowser );
    initDialog();
    return true;
  }
  else
  {
    SUIT_MessageBox::warn1( SMESHGUI::desktop(), tr("SMESH_ERROR"),
                            tr("SMESH_OPERATION_FAILED"), tr( "SMESH_BUT_OK" ) );
    return false;
  }
} 

//=======================================================================
// name    : startOperation
// Purpose :
//=======================================================================
void SMESHGUI_GroupOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_GroupOpDlg( myMode );
    connect( myDlg, SIGNAL( nameChanged( const QString& ) ), this, SLOT( onNameChanged( const QString& ) ) );
  }

  SMESHGUI_SelectionOp::startOperation();

  myDlg->show();
}

//=======================================================================
// name    : startOperation
// Purpose :
//=======================================================================
SUIT_SelectionFilter* SMESHGUI_GroupOp::createFilter( const int ) const
{
  return new SMESH_TypeFilter( GROUP );
}

//=======================================================================
// name    : startOperation
// Purpose :
//=======================================================================
void SMESHGUI_GroupOp::initDialog()
{
  SMESHGUI_SelectionOp::initDialog();
  if( myDlg )
  {
    myDlg->setName( QString::null );
    updateDialog();
  }
}

//=======================================================================
// name    : updateDialog
// Purpose :
//=======================================================================
void SMESHGUI_GroupOp::selectionDone()
{
  SMESHGUI_SelectionOp::selectionDone();
  updateDialog();
}

//=======================================================================
// name    : updateDialog
// Purpose :
//=======================================================================
void SMESHGUI_GroupOp::updateDialog()
{
  if( !myDlg )
    return;
    
  bool isEnabled = !myDlg->name().isEmpty() && myDlg->hasSelection( 1 ) && myDlg->hasSelection( 2 );
  myDlg->setButtonEnabled( isEnabled, QtxDialog::OK | QtxDialog::Apply );
}

//=======================================================================
// name    : isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_GroupOp::isValid() const
{
  if( !myDlg )
    return false;
    
  // Verify validity of group name
  if( myDlg->name().isEmpty() )
  {
    SUIT_MessageBox::warn1( SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                            tr("EMPTY_NAME"), tr( "SMESH_BUT_OK" ) );
    return false;
  }

  QStringList selGroup[2];
  for( int i=1; i<=2; i++ )
    myDlg->selectedObject( i, selGroup[i-1] );

  _PTR(SObject) pGroup1 = studyDS()->FindObjectID( selGroup[0].first() ),
                pGroup2 = studyDS()->FindObjectID( selGroup[1].first() );
  SMESH::SMESH_GroupBase_var aGroup1 = SMESH::SMESH_GroupBase::_narrow( _CAST(SObject,pGroup1)->GetObject() ),
                             aGroup2 = SMESH::SMESH_GroupBase::_narrow( _CAST(SObject,pGroup2)->GetObject() );
  
  // Verufy wheter arguments speciffiyed
  if( aGroup1->_is_nil() || aGroup2->_is_nil())
  {
    SUIT_MessageBox::warn1( SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                            tr("INCORRECT_ARGUMENTS"), tr( "SMESH_BUT_OK" ) );
    return false;
  }

  // Verify whether arguments belongs to same mesh
  SMESH::SMESH_Mesh_ptr aMesh1 = aGroup1->GetMesh();
  SMESH::SMESH_Mesh_ptr aMesh2 = aGroup2->GetMesh();

  int aMeshId1 = !aMesh1->_is_nil() ? aMesh1->GetId() : -1;
  int aMeshId2 = !aMesh2->_is_nil() ? aMesh2->GetId() : -1;

  if (aMeshId1 != aMeshId2 || aMeshId1 == -1) {
    SUIT_MessageBox::warn1( SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                            tr("DIFF_MESHES"), tr( "SMESH_BUT_OK" ) );
    return false;
  }

  // Verify whether groups have same types of entities
  if( aGroup1->GetType() != aGroup2->GetType() ) {
    SUIT_MessageBox::warn1( SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                            tr("DIFF_TYPES"), tr( "SMESH_BUT_OK" ) );
    return false;
  }

  return true;
}

//=======================================================================
// name    : onNameChanged
// Purpose :
//=======================================================================
void SMESHGUI_GroupOp::onNameChanged( const QString& )
{
  updateDialog();
}
