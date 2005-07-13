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
//  File   : SMESHGUI_SelectionOp.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH

#include <SMESHGUI_SelectionOp.h>
#include <SUIT_SelectionFilter.h>
#include <SalomeApp_SelectionMgr.h>

/*
  Class       : SMESHGUI_SelectionOp
  Description : Base operation for all operations using object selection in viewer or objectbrowser
                through common widgets created by SalomeApp_Dialog::createObject
*/

//=================================================================================
// name     : SMESHGUI_SelectionOp
// purpose  : 
//=================================================================================
SMESHGUI_SelectionOp::SMESHGUI_SelectionOp( const Selection_Mode mode )
: SMESHGUI_Operation(),
  myDefSelectionMode( mode )
{
}

//=================================================================================
// name     : ~SMESHGUI_SelectionOp
// purpose  :
//=================================================================================
SMESHGUI_SelectionOp::~SMESHGUI_SelectionOp()
{
  Filters::const_iterator anIt = myFilters.begin(),
                          aLast = myFilters.end();
  for( ; anIt!=aLast; anIt++ )
    if( anIt.data() )
      delete anIt.data();
}

//=================================================================================
// name     : startOperation
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::startOperation()
{
  SMESHGUI_Operation::startOperation();
  if( dlg() )
  {
    disconnect( dlg(), SIGNAL( objectActivated( int ) ), this, SLOT( onActivateObject( int ) ) );
    disconnect( dlg(), SIGNAL( objectDeactivated( int ) ), this, SLOT( onDeactivateObject( int ) ) );
    disconnect( dlg(), SIGNAL( selectionChanged( int ) ), this, SLOT( onSelectionChanged( int ) ) );
    connect( dlg(), SIGNAL( objectActivated( int ) ), this, SLOT( onActivateObject( int ) ) );
    connect( dlg(), SIGNAL( objectDeactivated( int ) ), this, SLOT( onDeactivateObject( int ) ) );
    connect( dlg(), SIGNAL( selectionChanged( int ) ), this, SLOT( onSelectionChanged( int ) ) );
  }

  myOldSelectionMode = selectionMode();
  setSelectionMode( myDefSelectionMode );
}

//=================================================================================
// name     : removeCustomFilters
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::removeCustomFilters() const
{
  SalomeApp_SelectionMgr* mgr = selectionMgr();
  if( !mgr )
    return;
    
  Filters::const_iterator anIt = myFilters.begin(),
                          aLast = myFilters.end();
  for( ; anIt!=aLast; anIt++ )
    if( anIt.data() )
      mgr->removeFilter( anIt.data() );
}

//=================================================================================
// name     : commitOperation
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::commitOperation()
{
  removeCustomFilters();
  setSelectionMode( myOldSelectionMode );
  SMESHGUI_Operation::commitOperation();  
}

//=================================================================================
// name     : abortOperation
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::abortOperation()
{
  removeCustomFilters();
  setSelectionMode( myOldSelectionMode );  
  SMESHGUI_Operation::abortOperation();
}

//=================================================================================
// name     : selectionDone
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::selectionDone()
{
  if( !dlg() )
    return;
    
  QStringList names, ids;
  SalomeApp_Dialog::TypesList types;
  selected( names, types, ids );
  dlg()->selectObject( names, types, ids );
}

//=================================================================================
// name     : createFilter
// purpose  :
//=================================================================================
SUIT_SelectionFilter* SMESHGUI_SelectionOp::createFilter( const int ) const
{
  return 0;
}

//=================================================================================
// name     : onActivateObject
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::onActivateObject( int id )
{
  SalomeApp_SelectionMgr* mgr = selectionMgr();
  if( !mgr )
    return;
    
  if( !myFilters.contains( id ) )
    myFilters[ id ] = createFilter( id );

  if( myFilters[ id ] )
    mgr->installFilter( myFilters[ id ] );

  selectionDone();
}

//=================================================================================
// name     : onDeactivateObject
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::onDeactivateObject( int id )
{
  SalomeApp_SelectionMgr* mgr = selectionMgr();
  if( mgr && myFilters.contains( id ) && myFilters[ id ] )
    mgr->removeFilter( myFilters[ id ] );
}

//=================================================================================
// name     : initDialog
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::initDialog()
{
  if( dlg() )
  {
    dlg()->clearSelection();
    dlg()->deactivateAll();
  }
}

//=================================================================================
// name     : initDialog
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::onSelectionChanged( int )
{
}
