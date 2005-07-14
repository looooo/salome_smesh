//  SALOME SMESHGUI
//
//  Copyright (C) 2005  CEA/DEN, EDF R&D
//
//
//
//  File   : SMESHGUI_DeleteOp.cxx
//  Author : Sergey LITONIN
//  Module : SALOME


#include "SMESHGUI_DeleteOp.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI.h"
#include "SMESHGUI_VTKUtils.h"
#include <SMESH_Actor.h>
#include <SalomeApp_UpdateFlags.h>
#include <SalomeApp_Tools.h>
#include <SALOME_ListIteratorOfListIO.hxx>
#include <SALOME_ListIO.hxx>
#include <SalomeApp_SelectionMgr.h>
#include <SalomeApp_Application.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ViewManager.h>
#include <SUIT_ViewWindow.h>
#include <SUIT_Desktop.h>
#include <qapplication.h>

/*!
 * \brief Constructor
*/
SMESHGUI_DeleteOp::SMESHGUI_DeleteOp()
: SMESHGUI_Operation()
{
}

/*!
 * \brief Destructor
*/
SMESHGUI_DeleteOp::~SMESHGUI_DeleteOp()
{
}

/*!
 * \brief Verifies whether operation is ready to start
  * \return TRUE if operation is ready to start, FALSE otherwise
*
* Virtual method redefined from the base class verifies whether operation is ready to start
*/
bool SMESHGUI_DeleteOp::isReadyToStart() const
{
  if ( !SMESHGUI_Operation::isReadyToStart() )
    return false;
    
  // VSR 17/11/04: check if all objects selected belong to SMESH component --> start
  SALOME_ListIO selected;
  selectionMgr()->selectedObjects( selected );

  QString aParentComponent = QString::null;
  for ( SALOME_ListIteratorOfListIO anIt( selected ); anIt.More(); anIt.Next() )
  {
    QString cur = anIt.Value()->getComponentDataType();
    if ( aParentComponent.isNull() )
      aParentComponent = cur;
    else if( !aParentComponent.isEmpty() && aParentComponent!=cur )
      aParentComponent = "";
  }

  if ( aParentComponent != getSMESHGUI()->name() )
  {
    QString aMess = QObject::tr("NON_SMESH_OBJECTS_SELECTED").arg(
      getSMESHGUI()->moduleName() );
    SUIT_MessageBox::warn1 ( desktop(), QObject::tr( "ERR_ERROR" ), aMess, tr( "BUT_OK" ) );
    return false;
  }
  
  // VSR 17/11/04: check if all objects selected belong to SMESH component <-- finish
  if ( SUIT_MessageBox::warn2 ( desktop(), QObject::tr( "SMESH_WRN_WARNING" ),
         QObject::tr( "SMESH_REALLY_DELETE" ), tr( "SMESH_BUT_YES"),
         QObject::tr("SMESH_BUT_NO"), 1, 0, 0) != 1 )
    return false;

  if ( isStudyLocked() )
    return false;

  return true;
}

/*!
 * \brief Deletes selected objects
  * \return TRUE if operation is ready to start, FALSE otherwise
*
* Virtual method redefined from the base deletes selected objects
*/
void SMESHGUI_DeleteOp::startOperation()
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  
  try
  {
    SUIT_ViewManager* vm =
      ( (SalomeApp_Application*) application() )->activeViewManager();
    int nbSf = vm->getViewsCount();

    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    _PTR(StudyBuilder) aStudyBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr;
    _PTR(AttributeIOR) anIOR;

    SALOME_ListIO selected;
    selectionMgr()->selectedObjects( selected );
    SALOME_ListIteratorOfListIO It( selected );

    aStudyBuilder->NewCommand();  // There is a transaction
    for ( ; It.More(); It.Next() )
    {
      Handle(SALOME_InteractiveObject) IObject = It.Value();
      if ( IObject->hasEntry())
      {
        _PTR(SObject) SO = aStudy->FindObjectID(IObject->getEntry());

        /* Erase child graphical objects */
        _PTR(ChildIterator) it = aStudy->NewChildIterator(SO);
        for ( it->InitEx(true); it->More(); it->Next())
        {
          _PTR(SObject) CSO = it->Value();
          if ( CSO->FindAttribute( anAttr, "AttributeIOR" ) )
          {
            anIOR = anAttr;
            QPtrVector<SUIT_ViewWindow> aViews = vm->getViews();
            for ( int i = 0; i < nbSf; i++ )
            {
              SUIT_ViewWindow *sf = aViews[ i ];
              CORBA::String_var anEntry = CSO->GetID().c_str();
              if ( SMESH_Actor* anActor = SMESH::FindActorByEntry( sf,anEntry.in() ) )
                SMESH::RemoveActor(sf,anActor);
            }
          }
        }

        /* Erase main graphical object */
        QPtrVector<SUIT_ViewWindow> aViews = vm->getViews();
        for ( int i = 0; i < nbSf; i++ )
        {
          SUIT_ViewWindow *sf = aViews[ i ];
          if ( SMESH_Actor* anActor = SMESH::FindActorByEntry( sf,IObject->getEntry() ) )
            SMESH::RemoveActor(sf,anActor);
        }

        // Remove object(s) from data structures
        _PTR(SObject) obj = aStudy->FindObjectID(IObject->getEntry());
        if ( obj )
        {
          SMESH::SMESH_GroupBase_var aGroup =
            SMESH::SMESH_GroupBase::_narrow( SMESH::SObjectToObject( obj ) );
          SMESH::SMESH_subMesh_var   aSubMesh =
            SMESH::SMESH_subMesh::_narrow( SMESH::SObjectToObject( obj ) );

          if ( !aGroup->_is_nil() )  // DELETE GROUP
          {
            SMESH::SMESH_Mesh_var aMesh = aGroup->GetMesh();
            aMesh->RemoveGroup( aGroup );
          }
          else if ( !aSubMesh->_is_nil() ) // DELETE SUBMESH
          {
            SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
            aMesh->RemoveSubMesh( aSubMesh );
          }
          else
          {
            // default action: remove SObject from the study
            // san - it's no use opening a transaction here until UNDO/REDO is provided in SMESH
            //SUIT_Operation *op = new SALOMEGUI_ImportOperation(myActiveStudy);
            //op->start();
            aStudyBuilder->RemoveObjectWithChildren( obj );
            //op->finish();
          }
        }
      } /* IObject->hasEntry() */
    } /* more/next */

    aStudyBuilder->CommitCommand();

    /* Clear any previous selection */
    SALOME_ListIO l1;
    selectionMgr()->setSelectedObjects( l1 );

    update( UF_Model | UF_ObjBrowser );
  }
  catch ( const SALOME::SALOME_Exception& S_ex )
  {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  catch (...)
  {
  }

  QApplication::restoreOverrideCursor();
  commit();
}






