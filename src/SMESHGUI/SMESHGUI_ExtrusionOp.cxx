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
//  File   : SMESHGUI_ExtrusionOp.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header:

#include "SMESHGUI_ExtrusionOp.h"
#include <SMESHGUI_ExtrusionDlg.h>
#include <SMESHGUI_VTKUtils.h>

#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>

#include <SUIT_OverrideCursor.h>

#include <SALOMEDS_SObject.hxx>

#include CORBA_SERVER_HEADER(SMESH_Group)

//=================================================================================
// name     : SMESHGUI_ExtrusionOp
// purpose  : constructor
//=================================================================================
SMESHGUI_ExtrusionOp::SMESHGUI_ExtrusionOp()
: SMESHGUI_SelectionOp( EdgeSelection ),
  myDlg( 0 )
{
}

//=================================================================================
// name     : ~SMESHGUI_ExtrusionOp
// purpose  : destructor
//=================================================================================
SMESHGUI_ExtrusionOp::~SMESHGUI_ExtrusionOp()
{
  if( myDlg )
    delete myDlg;
}

//=================================================================================
// name     : dlg
// purpose  : 
//=================================================================================
SalomeApp_Dialog* SMESHGUI_ExtrusionOp::dlg() const
{
  return myDlg;
}

//=================================================================================
// name     : initDialog
// purpose  : 
//=================================================================================
void SMESHGUI_ExtrusionOp::initDialog()
{
  if( myDlg )
  {
    myDlg->init();
    updateDialog();
  }
}

//=================================================================================
// name     : startOperation
// purpose  : 
//=================================================================================
void SMESHGUI_ExtrusionOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_ExtrusionDlg();
    connect( myDlg, SIGNAL( constructorClicked( int ) ), this, SLOT( onChangeType( int ) ) );
    connect( myDlg, SIGNAL( objectChanged( int, const QStringList& ) ),
             this, SLOT( onTextChanged( int, const QStringList& ) ) );
  }

  SMESHGUI_SelectionOp::startOperation();

  myDlg->show();
}

//=================================================================================
// name     : onSelectionChanged
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionOp::onSelectionChanged( int )
{
  updateDialog();
}

//=================================================================================
// name     : createFilter
// purpose  :
//=================================================================================
SUIT_SelectionFilter* SMESHGUI_ExtrusionOp::createFilter( const int id ) const
{
  if( id==1 )
  {
    // Costruction of the logical filter for the elements: mesh/sub-mesh/group
    SMESH_TypeFilter* aMeshOrSubMesh = new SMESH_TypeFilter (MESHorSUBMESH);
    SMESH_TypeFilter* aGroup         = new SMESH_TypeFilter (GROUP);

    QPtrList<SUIT_SelectionFilter> list;
    list.append( aMeshOrSubMesh );
    list.append( aGroup );

    return new SMESH_LogicalFilter( list, SMESH_LogicalFilter::LO_OR );
  }
  else
    return 0;
}

//=================================================================================
// name     : updateDialog
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionOp::updateDialog()
{
  if( !myDlg )
    return;

  bool en = myDlg->hasSelection( myDlg->isSelectMesh() ? 1 : 0 );
  myDlg->setButtonEnabled( en, QtxDialog::OK | QtxDialog::Apply );
} 

//=================================================================================
// name     : onApply
// purpose  :
//=================================================================================
bool SMESHGUI_ExtrusionOp::onApply()
{
  if( isStudyLocked() )
    return false;

  IdList ids;
  SMESH::SMESH_Mesh_var m = extractIds( ids );
  if( ids.count() && !m->_is_nil() )
  {
    SMESH::long_array_var anElementsId = new SMESH::long_array;
    anElementsId->length( ids.count() );
    
    for( int i=0, n=ids.count(); i<n; i++)
      anElementsId[i] = ids[i];

    SMESH::DirStruct aVector;
    double x, y, z;
    myDlg->coords( x, y, z );
    aVector.PS.x = x;
    aVector.PS.y = y;
    aVector.PS.z = z;
    long aNbSteps = (long)myDlg->nbStep();

    try
    {
      SMESH::SMESH_MeshEditor_var aMeshEditor = m->GetMeshEditor();
      SUIT_OverrideCursor c;
      aMeshEditor->ExtrusionSweep( anElementsId.inout(), aVector, aNbSteps );
    }
    catch (...)
    {
    }

    initDialog();
    SMESH::UpdateView();
    return true;
  }
  return false;
}

//=================================================================================
// name     : onChangeType
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionOp::onChangeType( int t )
{
  if( !myDlg->isSelectMesh() )
    setSelectionMode( t==0 ? EdgeSelection : FaceSelection );
}

//=================================================================================
// name     : onActivateObject
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionOp::onActivateObject( int id )
{
  if( id==0 && myDlg )
    onChangeType( myDlg->constructorId() );
  else if( id==1 )
    setSelectionMode( ActorSelection );
}

//=================================================================================
// name     : extractIds
// purpose  :
//=================================================================================
SMESH::SMESH_Mesh_var SMESHGUI_ExtrusionOp::extractIds( IdList& list ) const
{
  if( selectionMode()==ActorSelection )
  {
    SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_nil();
    QStringList sel; myDlg->selectedObject( 1, sel );
    if( sel.isEmpty() )
      return aMesh;
      
    _PTR( SObject ) SO = studyDS()->FindObjectID( sel.first() );
    if( !SO )
      return aMesh;

    CORBA::Object* obj = _CAST(SObject, SO)->GetObject();
    SMESH::SMESH_Mesh_var    selMesh    = SMESH::SMESH_Mesh::_narrow( obj );
    SMESH::SMESH_subMesh_var selSubMesh = SMESH::SMESH_subMesh::_narrow( obj );
    SMESH::SMESH_Group_var   selGroup   = SMESH::SMESH_Group::_narrow( obj );

    SMESH::long_array_var anElementsIds = new SMESH::long_array;    
    if( !selMesh->_is_nil() )
    {
      //MESH
      aMesh = selMesh;

      // get IDs from submesh
      anElementsIds = selMesh->GetElementsByType( myDlg->constructorId()==0 ? SMESH::EDGE : SMESH::FACE );    
    }
    else if( !selSubMesh->_is_nil() )
    {
      //SUBMESH
      aMesh = selSubMesh->GetFather();

      // get IDs from submesh
      anElementsIds = selSubMesh->GetElementsByType( myDlg->constructorId()==0 ? SMESH::EDGE : SMESH::FACE );
    }
    else if( !selGroup->_is_nil() )
    {
      //GROUP
      aMesh = selGroup->GetMesh();

      if( selGroup->GetType()==( myDlg->constructorId()==0 ? SMESH::EDGE : SMESH::FACE ) )
        // get IDs from group
        anElementsIds = selGroup->GetListOfID();
    }

    for (int i=0, n=anElementsIds->length(); i<n; i++)
      list.append( anElementsIds[ i ] );
   
    return aMesh;
  }
  else
  {
    SMESHGUI_SelectionOp::selectedIds( 0, list );
    return mesh();
  }
}
