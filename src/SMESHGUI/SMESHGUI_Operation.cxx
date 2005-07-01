//  SALOME SMESHGUI
//
//  Copyright (C) 2005  CEA/DEN, EDF R&D
//
//
//
//  File   : SMESHGUI_Operation.h
//  Author : Sergey LITONIN
//  Module : SALOME

#include <SMESHGUI_Operation.h>
#include <SMESHGUI.h>
#include <SMESHGUI_VTKUtils.h>

#include <SVTK_ViewWindow.h>
#include <SVTK_Selector.h>


/*
  Class       : SMESHGUI_Operation
  Description : Base class for all SMESH operations
*/

//=======================================================================
// name    : SMESHGUI_Operation
// Purpose : Constructor
//=======================================================================
SMESHGUI_Operation::SMESHGUI_Operation( SalomeApp_Application* app )
: SalomeApp_Operation( app ),
  myViewWindow( 0 ),
  mySelector( 0 )
{
  
}

SMESHGUI_Operation::~SMESHGUI_Operation()
{
  
}

void SMESHGUI_Operation::setSelectionMode( const Selection_Mode mode )
{
  if( myViewWindow )
    myViewWindow->SetSelectionMode( mode );
}

void SMESHGUI_Operation::highlight( const Handle( SALOME_InteractiveObject )& obj,
                                    const bool hilight, const bool immediately )
{
  if( myViewWindow )
    myViewWindow->highlight( obj, hilight, immediately );
}

void SMESHGUI_Operation::addOrRemoveIndex( const Handle( SALOME_InteractiveObject )& obj,
                                           const TColStd_MapOfInteger& indices, const bool isModeShift )
{
  if( mySelector )
    mySelector->AddOrRemoveIndex( obj, indices, isModeShift );
}

SMESHGUI* SMESHGUI_Operation::getSMESHGUI() const
{
  return dynamic_cast<SMESHGUI*>( module() );
}

SVTK_ViewWindow* SMESHGUI_Operation::getViewWindow() const
{
  return myViewWindow;
}

SVTK_Selector* SMESHGUI_Operation::getSelector() const
{
  return mySelector;
}

void SMESHGUI_Operation::startOperation()
{
  SalomeApp_Operation::startOperation();
  myViewWindow = SMESH::GetViewWindow( getSMESHGUI() );
  mySelector = myViewWindow ? myViewWindow->GetSelector() : 0;
}
