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
//  File   : SMESHGUI_AddSubMeshOp.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_AddSubMeshOp.h"
#include <SMESHGUI_AddSubMeshDlg.h>
#include <SMESHGUI_GEOMGenUtils.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI.h>

#include <SMESH_NumberFilter.hxx>
#include <SMESH_TypeFilter.hxx>

#include <SUIT_SelectionFilter.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_MessageBox.h>

#include <SalomeApp_SelectionMgr.h>
#include <SalomeApp_UpdateFlags.h>
#include <SalomeApp_Tools.h>

#include <TColStd_MapOfInteger.hxx>

#include <SALOMEDS_SObject.hxx>

#include <utilities.h>

//=================================================================================
// function : Constructor
// purpose  :
//=================================================================================
SMESHGUI_AddSubMeshOp::SMESHGUI_AddSubMeshOp()
: SMESHGUI_SelectionOp(),
  myDlg( 0 )
{
  setAutoResumed( true );
}

//=================================================================================
// function : Destructor
// purpose  :
//=================================================================================
SMESHGUI_AddSubMeshOp::~SMESHGUI_AddSubMeshOp()
{
  if( myDlg )
    delete myDlg;
}

//=================================================================================
// function : startOperation
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_AddSubMeshDlg();
    connect( myDlg, SIGNAL( nameChanged( const QString& ) ), this, SLOT( onNameChanged( const QString& ) ) );
  }

  SMESHGUI_SelectionOp::startOperation();

  myDlg->show();
}

//=================================================================================
// function : dlg
// purpose  :
//=================================================================================
SalomeApp_Dialog* SMESHGUI_AddSubMeshOp::dlg() const
{
  return myDlg;
}

//=================================================================================
// function : selectionDone
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshOp::selectionDone()
{
  SMESHGUI_SelectionOp::selectionDone();
  if( myDlg )
    updateDialog();
}

//=================================================================================
// function : createFilter
// purpose  :
//=================================================================================
SUIT_SelectionFilter* SMESHGUI_AddSubMeshOp::createFilter( const int id ) const
{
  if( id==SMESHGUI_AddSubMeshDlg::GeomObj )
  {
    TColStd_MapOfInteger allTypesMap;
    for (int i = 0; i < 10; i++)
      allTypesMap.Add(i);
    return new SMESH_NumberFilter ("GEOM", TopAbs_SHAPE, 0, allTypesMap);
  }

  if( id==SMESHGUI_AddSubMeshDlg::MeshObj )
    return new SMESH_TypeFilter (MESH);

  else if( id==SMESHGUI_AddSubMeshDlg::Algo )
    return new SMESH_TypeFilter (ALGORITHM);

  else if( id==SMESHGUI_AddSubMeshDlg::Hypo )
    return new SMESH_TypeFilter (HYPOTHESIS);

  else
    return 0;
}

//=================================================================================
// function : commitOperation
// purpose  :
//=================================================================================
bool SMESHGUI_AddSubMeshOp::onApply()
{
  if( getSMESHGUI()->isActiveStudyLocked() )
    return false;

  QString myNameSubMesh = myDlg->subMeshName();
  if (myNameSubMesh.isEmpty())
  {
    SUIT_MessageBox::warn1( dlg(), tr("SMESH_WRN_WARNING"),
                            tr("SMESH_WRN_EMPTY_NAME"), tr("SMESH_BUT_OK"));
    return false;
  }

  QStringList selMesh, selGeom, selHypo, selAlgo;
  myDlg->selectedObject( SMESHGUI_AddSubMeshDlg::MeshObj, selMesh );
  myDlg->selectedObject( SMESHGUI_AddSubMeshDlg::GeomObj, selGeom );
  myDlg->selectedObject( SMESHGUI_AddSubMeshDlg::Hypo, selHypo );
  myDlg->selectedObject( SMESHGUI_AddSubMeshDlg::Algo, selAlgo );
  
  _PTR(Study) study = studyDS();
  _PTR(SObject) aMeshSO = study->FindObjectID( selMesh.first() );
  SMESH::SMESH_Mesh_var myMesh = SMESH::SMESH_Mesh::_narrow( _CAST( SObject, aMeshSO )->GetObject() );
  GEOM::GEOM_Object_var myMainShape = SMESH::GetShapeOnMeshOrSubMesh(aMeshSO);
  if (myMainShape->_is_nil())
    return false;

  SUIT_OverrideCursor wc;

  _PTR(SObject) aGeomSO = study->FindObjectID( selGeom.first() );
  GEOM::GEOM_Object_var myGeomShape = GEOM::GEOM_Object::_narrow( _CAST( SObject, aGeomSO )->GetObject() );
  
  // create submesh
  SMESH::SMESH_subMesh_var aSubMesh = addSubMesh(myMesh, myGeomShape, myNameSubMesh);
  int nbSuccess = 0;

  if (!aSubMesh->_is_nil())
  {
    // assign hypotheses
    int nbAlgo = selAlgo.count();
    int nbHyps = selHypo.count() + nbAlgo;
    for (int i = 0; i < nbHyps; i++)
    {
      _PTR(SObject) aHypSOClient =
        study->FindObjectID
          (i < nbAlgo ? selAlgo[i].latin1() : selHypo[i-nbAlgo].latin1());
      if (aHypSOClient)
      {
        CORBA::Object_var anObject = _CAST(SObject,aHypSOClient)->GetObject();
        if(!CORBA::is_nil(anObject))
        {
          SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow(anObject);
          if(!aHyp->_is_nil())
            if (SMESH::AddHypothesisOnSubMesh(aSubMesh, aHyp))
              nbSuccess++;
        }
        else
        {
          SCRUTE(CORBA::is_nil(anObject));
        }
      }
      else
      {
        SCRUTE(!aHypSOClient);
      }
    }
  }
  else
  {
    SCRUTE(aSubMesh->_is_nil());
  }
  
  update( UF_Model | UF_ObjBrowser );
  initDialog();
  
  return (nbSuccess > 0);
}

//=================================================================================
// function : initDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshOp::initDialog()
{
  SMESHGUI_SelectionOp::initDialog();
  if( myDlg )
  {
    myDlg->setSubMeshName( tr( "SMESH_SUBMESH" ) );
    updateDialog();
  }
}

//=================================================================================
// function : IsFatherOf()
// purpose  :
//=================================================================================
static bool IsFatherOf (_PTR(SObject) SO, _PTR(SObject) fatherSO)
{
  if( SO && fatherSO )
  {
    _PTR(SObject) aSO = SO->GetFather();
    while (aSO->GetID().length() >= fatherSO->GetID().length())
    {
      if (aSO->GetID() == fatherSO->GetID())
        return true;
      aSO = aSO->GetFather();
    }
  }
  return false;
}

//=================================================================================
// function : onSelectionChanged()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshOp::onSelectionChanged( int id )
{
  if( !myDlg->hasSelection( id ) )
    return;
    
  if( id==SMESHGUI_AddSubMeshDlg::MeshObj )
    myDlg->clearSelection( SMESHGUI_AddSubMeshDlg::GeomObj );

  else if( id==SMESHGUI_AddSubMeshDlg::GeomObj )
  {
    QStringList selMesh, selGeom;
    myDlg->selectedObject( SMESHGUI_AddSubMeshDlg::MeshObj, selMesh );
    myDlg->selectedObject( SMESHGUI_AddSubMeshDlg::GeomObj, selGeom );
    
    _PTR(SObject) aMeshSO = studyDS()->FindObjectID( selMesh.first() );
    
    GEOM::GEOM_Object_var aMainGeomShape = SMESH::GetShapeOnMeshOrSubMesh(aMeshSO);
    _PTR(SObject) aMainGeomShapeSO = SMESH::FindSObject(aMainGeomShape);
    if( !aMainGeomShapeSO || !IsFatherOf( studyDS()->FindObjectID( selGeom.first() ), aMainGeomShapeSO ) )
      myDlg->clearSelection( id );
  }
}

//=================================================================================
// function : addSubMesh()
// purpose  :
//=================================================================================
SMESH::SMESH_subMesh_var SMESHGUI_AddSubMeshOp::addSubMesh( SMESH::SMESH_Mesh_ptr theMesh,
                                                            GEOM::GEOM_Object_ptr theShapeObject,
                                                            const QString& theMeshName )
{
  SMESH::SMESH_subMesh_var aSubMesh;
  try
  {
    if(!theMesh->_is_nil() && !theShapeObject->_is_nil())
      aSubMesh = theMesh->GetSubMesh(theShapeObject, theMeshName);
  }
  catch (const SALOME::SALOME_Exception& S_ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }

  return aSubMesh._retn();
}

//=================================================================================
// function : updateDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshOp::updateDialog()
{
  if( !myDlg )
    return;
    
  bool isEnabled = !myDlg->subMeshName().isEmpty() &&
                   myDlg->hasSelection( SMESHGUI_AddSubMeshDlg::MeshObj ) &&
                   myDlg->hasSelection( SMESHGUI_AddSubMeshDlg::GeomObj ) &&
                   myDlg->hasSelection( SMESHGUI_AddSubMeshDlg::Hypo ) &&
                   myDlg->hasSelection( SMESHGUI_AddSubMeshDlg::Algo );

  bool isImportedMesh = false;

  QStringList selMesh;
  myDlg->selectedObject( SMESHGUI_AddSubMeshDlg::MeshObj, selMesh );
  _PTR(SObject) SO = studyDS()->FindObjectID( selMesh.first() );
  GEOM::GEOM_Object_var myGeomShape = SMESH::GetShapeOnMeshOrSubMesh(SO);
  isImportedMesh = myGeomShape->_is_nil();

  isEnabled = isEnabled && !isImportedMesh;
  
  myDlg->setButtonEnabled( isEnabled, QtxDialog::OK | QtxDialog::Apply );
}

//=================================================================================
// function : onNameChanged()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshOp::onNameChanged( const QString& )
{
  updateDialog();
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_AddSubMeshOp::isValid( SUIT_Operation* theOtherOp ) const
{
  //if ( theOtherOp && theOtherOp->inherits( "SMESHGUI_InitMeshOp" ) )
    return true;
  /*else
    return false;*/
}
