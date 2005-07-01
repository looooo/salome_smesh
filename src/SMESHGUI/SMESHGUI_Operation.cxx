//  SALOME SMESHGUI
//
//  Copyright (C) 2005  CEA/DEN, EDF R&D
//
//
//
//  File   : SMESHGUI_Operation.h
//  Author : Sergey LITONIN
//  Module : SALOME

#include "SMESHGUI_Operation.h"
#include "SMESHGUI.h"
#include "SMESHGUI_VTKUtils.h"

#include <SVTK_ViewWindow.h>
#include <SVTK_Selector.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>


/*
  Class       : SMESHGUI_Operation
  Description : Base class for all SMESH operations
*/

//=======================================================================
// name    : SMESHGUI_Operation
// Purpose : Constructor
//=======================================================================
SMESHGUI_Operation::SMESHGUI_Operation( SalomeApp_Application* app )
: SalomeApp_Operation( app )
{
  
}

SMESHGUI_Operation::~SMESHGUI_Operation()
{
  
}

//=======================================================================
// name    : setSelectionMode
// Purpose : Set selection mode
//=======================================================================
void SMESHGUI_Operation::setSelectionMode( const Selection_Mode mode )
{
  SVTK_ViewWindow* wnd = viewWindow();
  if( wnd )
    wnd->SetSelectionMode( mode );
}

//=======================================================================
// name    : highlight
// Purpose : Highlight object in 3d viewer
//=======================================================================
void SMESHGUI_Operation::highlight( const Handle( SALOME_InteractiveObject )& obj,
                                    const bool hilight, const bool immediately )
{
  SVTK_ViewWindow* wnd = viewWindow();
  if( wnd )
    wnd->highlight( obj, hilight, immediately );
}

//=======================================================================
// name    : addOrRemoveIndex
// Purpose : Select/deselect cells of mesh
//=======================================================================
void SMESHGUI_Operation::addOrRemoveIndex( const Handle( SALOME_InteractiveObject )& obj,
                                           const TColStd_MapOfInteger& indices,
                                           const bool isModeShift )
{
  SVTK_Selector* sel = selector();
  if( sel )
    sel->AddOrRemoveIndex( obj, indices, isModeShift );
}

//=======================================================================
// name    : getSMESHGUI
// Purpose : Get SMESH module
//=======================================================================
SMESHGUI* SMESHGUI_Operation::getSMESHGUI() const
{
  return dynamic_cast<SMESHGUI*>( module() );
}

//=======================================================================
// name    : viewWindow
// Purpose : Get active view window
//=======================================================================
SVTK_ViewWindow* SMESHGUI_Operation::viewWindow() const
{
  return SMESH::GetViewWindow( getSMESHGUI() );
}

//=======================================================================
// name    : selector
// Purpose : Get selector
//=======================================================================
SVTK_Selector* SMESHGUI_Operation::selector() const
{
  SVTK_ViewWindow* wnd = viewWindow();
  return wnd ? wnd->GetSelector() : 0;
}

//=======================================================================
// name    : startOperation
// Purpose : Start opeartion
//=======================================================================
void SMESHGUI_Operation::startOperation()
{
  SalomeApp_Operation::startOperation();
}

//=======================================================================
// name    : isReadyToStart
// Purpose : Verify whether operation is ready to start
//=======================================================================
bool SMESHGUI_Operation::isReadyToStart()
{
  if ( !SalomeApp_Operation::isReadyToStart() )
    return false;
    
  if ( getSMESHGUI() == 0 )
  {
    SUIT_MessageBox::warn1( SMESHGUI::desktop(), tr( "SMESH_WRN_WARNING" ),
      tr( "NO_MODULE" ), tr( "SMESH_BUT_OK" ) );
    return false;
  }
}





