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
//  File   : SMESHGUI_SelectionIdsOp.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH

#include <SMESHGUI_SelectionIdsOp.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI_VTKUtils.h>
#include <SMESHGUI_MeshUtils.h>

#include <SalomeApp_SelectionMgr.h>

#include <SALOME_ListIO.hxx>
#include <SVTK_ViewModel.h>
#include <SVTK_Selector.h>

#include <SMDS_Mesh.hxx>
#include <SMESH_Actor.h>

#include <TColStd_MapOfInteger.hxx>

/*
  Class       : SMESHGUI_SelectionIdsOp
  Description : 
*/

//=================================================================================
// name     : SMESHGUI_SelectionIdsOp
// purpose  : 
//=================================================================================
SMESHGUI_SelectionIdsOp::SMESHGUI_SelectionIdsOp( const Selection_Mode mode )
: SMESHGUI_SelectionOp( mode )
{
}

//=================================================================================
// name     : ~SMESHGUI_SelectionIdsOp
// purpose  :
//=================================================================================
SMESHGUI_SelectionIdsOp::~SMESHGUI_SelectionIdsOp()
{
}

//=================================================================================
// name     : startOperation
// purpose  :
//=================================================================================
void SMESHGUI_SelectionIdsOp::startOperation()
{
  SMESHGUI_SelectionOp::startOperation();
  if( dlg() )
  {
    disconnect( dlg(), SIGNAL( objectChanged( int, const QStringList& ) ),
                this,  SLOT( onTextChanged( int, const QStringList& ) ) );
    connect( dlg(), SIGNAL( objectChanged( int, const QStringList& ) ),
             this,  SLOT( onTextChanged( int, const QStringList& ) ) );    
  }
}

//=================================================================================
// name     : commitOperation
// purpose  :
//=================================================================================
void SMESHGUI_SelectionIdsOp::commitOperation()
{
  SMESHGUI_SelectionOp::commitOperation();
  myMesh = SMESH::SMESH_Mesh::_nil();
}

//=================================================================================
// name     : abortOperation
// purpose  :
//=================================================================================
void SMESHGUI_SelectionIdsOp::abortOperation()
{
  SMESHGUI_SelectionOp::abortOperation();
  myMesh = SMESH::SMESH_Mesh::_nil();
}

//=================================================================================
// name     : selectionDone
// purpose  :
//=================================================================================
void SMESHGUI_SelectionIdsOp::selectionDone()
{
  if( !dlg() )
    return;
    
  // get selected mesh
  SALOME_ListIO aList;
  selectionMgr()->selectedObjects(aList,SVTK_Viewer::Type());

  if( aList.Extent() != 1)
  {
    myMesh = SMESH::SMESH_Mesh::_nil();
    dlg()->clearSelection();
    return;
  }

  Handle(SALOME_InteractiveObject) anIO = aList.First();
  myMesh = SMESH::GetMeshByIO(anIO);

  QStringList names, ids; SalomeApp_Dialog::TypesList types;
  selected( names, types, ids );  
  dlg()->selectObject( names, types, ids );
}

//=================================================================================
// name     : mesh
// purpose  :
//=================================================================================
SMESH::SMESH_Mesh_var SMESHGUI_SelectionIdsOp::mesh() const
{
  return myMesh;
}

//=================================================================================
// name     : actor
// purpose  :
//=================================================================================
SMESH_Actor* SMESHGUI_SelectionIdsOp::actor() const
{
  return SMESH::FindActorByObject( myMesh.in() );
}

//=================================================================================
// name     : onTextChanged
// purpose  :
//=================================================================================
void SMESHGUI_SelectionIdsOp::onTextChanged( int, const QStringList& list )
{
    if( !dlg() )
      return;

    TColStd_MapOfInteger newIndices;

    SALOME_ListIO sel; selectionMgr()->selectedObjects( sel );
    SMESH_Actor* anActor = actor();
    if( sel.Extent()==0 || !anActor )
      return;

    SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();

    IdList ids; extractIds( list, ids, '\0' );
    IdList::const_iterator anIt = ids.begin(),
                           aLast = ids.end();
    for( ; anIt!=aLast; anIt++ )
      if( const SMDS_MeshNode * n = aMesh->FindNode( *anIt ) )
        newIndices.Add( n->GetID() );

    selector()->AddOrRemoveIndex( sel.First(), newIndices, false );
    highlight( sel.First(), true, true );

    QStringList names, _ids; SalomeApp_Dialog::TypesList types;
    selected( names, types, _ids );
    dlg()->selectObject( names, types, _ids, false );
}

//=================================================================================
// name     : selectedIds
// purpose  :
//=================================================================================
void SMESHGUI_SelectionIdsOp::selectedIds( const int id, IdList& list ) const
{
  if( !dlg() )
    return;
    
  QStringList ids; dlg()->selectedObject( id, ids );
  extractIds( ids, list );
}

//=================================================================================
// name     : extractIds
// purpose  :
//=================================================================================
void SMESHGUI_SelectionIdsOp::extractIds( const QStringList& ids, IdList& list, const QChar idchar )
{
  QStringList::const_iterator anIt = ids.begin(),
                              aLast = ids.end();
  QString id_str;
  for( ; anIt!=aLast; anIt++ )
  {
    id_str = *anIt;
    int pos = idchar=='\0' ? -1 : id_str.find( idchar );
    int id = -1;
    if( idchar=='\0' || pos>=0 )
    {
      id = id_str.mid( pos+1 ).toInt();
      list.append( id );
    }
  }  
}

//=================================================================================
// name     : extractIds
// purpose  :
//=================================================================================
void SMESHGUI_SelectionIdsOp::extractIds( const QStringList& ids, IdList& list ) const
{
  extractIds( ids, list, idChar() );
}
