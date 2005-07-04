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
#include "SMESHGUI_Selection.h"

#include <SVTK_ViewWindow.h>
#include <SVTK_Selector.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>

#include <SalomeApp_Study.h>
#include <SalomeApp_VTKSelector.h>
#include <SalomeApp_SelectionMgr.h>

#include <SALOMEDS_SObject.hxx>

#include <SMESHDS_Mesh.hxx>

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

  return true;
}

//=======================================================================
// name    : typeById
// Purpose : Find type by id
//=======================================================================
int SMESHGUI_Operation::typeById( const QString& str, const SelectedObjectType objtype ) const
{
  SalomeApp_Study* _study = dynamic_cast<SalomeApp_Study*>( study() );
  if( !_study )
    return -1;

  _PTR( Study ) st = _study->studyDS();

  int res = -1;
  if( objtype == Object )
  {
    SalomeApp_Study* _study = dynamic_cast<SalomeApp_Study*>( study() );
    if( _study )
    {
      int t = SMESHGUI_Selection::type( str, _study->studyDS() );
      if( t<0 )
      {
        //try to get GEOM type
        _PTR( SObject ) sobj = st->FindObjectID( str.latin1() );
        if( sobj )
        {
          GEOM::GEOM_Object_var obj = GEOM::GEOM_Object::_narrow( dynamic_cast<SALOMEDS_SObject*>( sobj.get() )->GetObject() );
          if( !CORBA::is_nil( obj ) )
            res = prefix( "GEOM" ) + obj->GetType();
        }
      }
      else
        res = prefix( "SMESH" ) + t;
    }
  }
  else  
  {
    int pos = str.find( idChar() );
    QString entry = str.left( pos-1 ),
            _id = str.mid( pos+1 );
    bool ok;
    int id = _id.toInt( &ok );
    if( ok )
    {
      _PTR( SObject ) sobj = st->FindObjectID( entry.latin1() );
      SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( dynamic_cast<SALOMEDS_SObject*>( sobj.get() )->GetObject() );
      SMESH::SMESH_subMesh_var submesh = SMESH::SMESH_subMesh::_narrow( dynamic_cast<SALOMEDS_SObject*>( sobj.get() )->GetObject() );
      if( !CORBA::is_nil( mesh ) )
        res = prefix( "SMESH element" ) + mesh->GetElementType( id, objtype==MeshElement );
        
      else if( !CORBA::is_nil( submesh ) )
        res = prefix( "SMESH element" ) + submesh->GetElementType( id, objtype==MeshElement );
    }
  }

  return res;
}

//=======================================================================
// name    : prefix
// Purpose : Get prefix for module types
//=======================================================================
int SMESHGUI_Operation::prefix( const QString& name ) const
{
  if( name == "GEOM" )
    return 100;
  else if( name == "SMESH" )
    return 200;
  else if( name == "SMESH element" )
    return 300;
  else
    return 0;
}

//=======================================================================
// name    : selected
// Purpose : Get names, types and ids of selected objects
//=======================================================================
void SMESHGUI_Operation::selected( QStringList& names, SalomeApp_Dialog::TypesList& types, QStringList& ids ) const
{
  SUIT_DataOwnerPtrList list; selectionMgr()->selected( list );
  SUIT_DataOwnerPtrList::const_iterator anIt = list.begin(),
                                        aLast = list.end();
  for( ; anIt!=aLast; anIt++ )
  {
    SalomeApp_DataOwner* owner = dynamic_cast<SalomeApp_DataOwner*>( (*anIt).operator->() );
    SalomeApp_SVTKDataOwner* vtkowner = dynamic_cast<SalomeApp_SVTKDataOwner*>( (*anIt).operator->() );

    if( vtkowner )
    {
      QString id_str = QString( "%1%2%3" ).arg( vtkowner->entry() ).arg( idChar() ), current_id_str;
      Selection_Mode mode = vtkowner->GetMode();
      SelectedObjectType objtype = mode == NodeSelection ? MeshNode : MeshElement;
      const TColStd_IndexedMapOfInteger& ownerids = vtkowner->GetIds();

      for( int i=1, n=ownerids.Extent(); i<=n; i++ )
      {
        int curid = ownerids( i );
        current_id_str = id_str.arg( curid );
        ids.append( current_id_str );
        types.append( typeById( current_id_str, objtype ) );
        names.append( QString( "%1" ).arg( curid ) );
      }
    }

    else if( owner )
    {
      QString id = owner->entry();
      ids.append( id );
      types.append( typeById( id, Object ) );
      names.append( owner->IO()->getName() );
    }
  }
}

//=======================================================================
// name    : idChar
// Purpose : Char using to divide <entry> and <id> in string id representation. By default, '#'
//=======================================================================
QChar SMESHGUI_Operation::idChar() const
{
  return '#';
}
