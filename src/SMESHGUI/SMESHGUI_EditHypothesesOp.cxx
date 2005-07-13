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
//  File   : SMESHGUI_EditHypothesesOp.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_EditHypothesesOp.h"
#include <SMESHGUI_EditHypothesesDlg.h>
#include <SMESH_NumberFilter.hxx>
#include <SMESH_TypeFilter.hxx>
#include <SMESHGUI_GEOMGenUtils.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI.h>

#include <SalomeApp_UpdateFlags.h>

#include <SUIT_OverrideCursor.h>

#include <qlistbox.h>

#include <SALOMEDS_SObject.hxx>

#define ALLOW_CHANGE_SHAPE 0

//=================================================================================
// function : 
// purpose  :
//=================================================================================
SMESHGUI_EditHypothesesOp::SMESHGUI_EditHypothesesOp()
: SMESHGUI_SelectionOp(),
  myDlg( 0 ),
  myImportedMesh( false )
{
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
SMESHGUI_EditHypothesesOp::~SMESHGUI_EditHypothesesOp()
{
  if( myDlg )
    delete myDlg;
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
SalomeApp_Dialog* SMESHGUI_EditHypothesesOp::dlg() const
{
  return myDlg;
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_EditHypothesesDlg();
    connect( myDlg, SIGNAL( needToUpdate() ), this, SLOT( onUpdate() ) );
  }

  SMESHGUI_SelectionOp::startOperation();

  myDlg->show();
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::selectionDone()
{
  SMESHGUI_SelectionOp::selectionDone();
  if( myDlg )
    updateDialog();
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
bool SMESHGUI_EditHypothesesOp::onApply()
{
  if( getSMESHGUI()->isActiveStudyLocked() )
    return false;

  bool aRes = false;

  SUIT_OverrideCursor wc;

  aRes = storeMeshOrSubMesh();
  if( aRes )
  {
    initHypAssignation();
    initAlgoAssignation();
  }

  update( UF_Model | UF_ObjBrowser );

  initDialog();
  return aRes;
}

//=================================================================================
// function : onSelectionChanged
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::onSelectionChanged( int id )
{
  if( id==SMESHGUI_EditHypothesesDlg::MeshOrSubMesh )
  {
    initGeom();
    myImportedMesh = !myDlg->hasSelection( SMESHGUI_EditHypothesesDlg::GeomShape );
    initHypAssignation();
    initAlgoAssignation();
    updateDialog();
  }
  else if( id==SMESHGUI_EditHypothesesDlg::GeomShape )
  {
  }
}

//=================================================================================
// function : initGeom
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::initGeom()
{
  QStringList selMesh; myDlg->selectedObject( SMESHGUI_EditHypothesesDlg::MeshOrSubMesh, selMesh );
  if( selMesh.isEmpty() )
    return;
    
  _PTR(SObject) aMesh = studyDS()->FindObjectID( selMesh.first() );
  GEOM::GEOM_Object_var aGeomShape = SMESH::GetShapeOnMeshOrSubMesh( aMesh );
  if( !aGeomShape->_is_nil() )
  {
    QString name, id = aGeomShape->GetStudyEntry();
    
    _PTR(GenericAttribute) anAttr;
    _PTR(AttributeName)    aName;
    _PTR(SObject) aSO = studyDS()->FindObjectID(aGeomShape->GetStudyEntry());
    if (aSO)
      if (aSO->FindAttribute(anAttr, "AttributeName"))
      {
        aName = anAttr;
        name = aName->Value().c_str();
      }
      
    int type = aGeomShape->GetType();
    myDlg->selectObject( SMESHGUI_EditHypothesesDlg::GeomShape, name, type, id );
  }
} 

//=================================================================================
// function : initHypDefinition
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::initHypDefinition()
{
  myDlg->ListHypDefinition->clear();

  _PTR(Study) study = studyDS();  
  _PTR(SComponent) father = study->FindComponent("SMESH");
  if (!father)
    return;

  _PTR(SObject)          HypothesisRoot;
  _PTR(GenericAttribute) anAttr;
  _PTR(AttributeName)    aName;
  _PTR(AttributeIOR)     anIOR;

  //int Tag_HypothesisRoot = 1;
  if (father->FindSubObject(1, HypothesisRoot)) {
    _PTR(ChildIterator) it =
      study->NewChildIterator(HypothesisRoot);
    ListBoxIOR* anItem;
    for (; it->More();it->Next()) {
      _PTR(SObject) Obj = it->Value();
      if (Obj->FindAttribute(anAttr, "AttributeName")) {
        aName = anAttr;
        if (Obj->FindAttribute(anAttr, "AttributeIOR")) {
          anIOR = anAttr;
          anItem = new ListBoxIOR (myDlg->ListHypDefinition,
                                   anIOR->Value().c_str(),
                                   aName->Value().c_str());
        }
      }
    }
  }
}

//=================================================================================
// function : initAlgoDefinition
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::initAlgoDefinition()
{
  myDlg->ListAlgoDefinition->clear();

  _PTR(Study) study = studyDS();  
  _PTR(SComponent) father = study->FindComponent("SMESH");
  if (!father)
    return;

  _PTR(SObject)          AlgorithmsRoot;
  _PTR(GenericAttribute) anAttr;
  _PTR(AttributeName)    aName;
  _PTR(AttributeIOR)     anIOR;

  if (father->FindSubObject (2, AlgorithmsRoot)) {
    _PTR(ChildIterator) it =
      study->NewChildIterator(AlgorithmsRoot);
    ListBoxIOR* anItem;
    for (; it->More();it->Next()) {
      _PTR(SObject) Obj = it->Value();
      if (Obj->FindAttribute(anAttr, "AttributeName")) {
        aName = anAttr;
        if (Obj->FindAttribute(anAttr, "AttributeIOR")) {
          anIOR = anAttr;
          anItem = new ListBoxIOR (myDlg->ListAlgoDefinition,
                                   anIOR->Value().c_str(),
                                   aName->Value().c_str());
        }
      }
    }
  }
}

//=================================================================================
// function : initHypAssignation
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::initHypAssignation()
{
  myDlg->myNbModification = 0;

  myDlg->myMapOldHypos.clear();
  myDlg->ListHypAssignation->clear();
  if (myImportedMesh)
    return;

  _PTR(Study) study = studyDS();  
  _PTR(SObject)          aMorSM, AHR, aRef;
  _PTR(GenericAttribute) anAttr;
  _PTR(AttributeName)    aName;
  _PTR(AttributeIOR)     anIOR;

  QStringList selMesh;
  myDlg->selectedObject( SMESHGUI_EditHypothesesDlg::MeshOrSubMesh, selMesh );
  aMorSM = studyDS()->FindObjectID( selMesh.first() );
  
  if (aMorSM && aMorSM->FindSubObject(2, AHR)) {
    _PTR(ChildIterator) it =
      study->NewChildIterator(AHR);
    for (; it->More();it->Next()) {
      _PTR(SObject) Obj = it->Value();
      if (Obj->ReferencedObject(aRef)) {
        if (aRef->FindAttribute(anAttr, "AttributeName")) {
          aName = anAttr;
	  if (aRef->FindAttribute(anAttr, "AttributeIOR")) {
            anIOR = anAttr;
	    ListBoxIOR* anItem = new ListBoxIOR (myDlg->ListHypAssignation,
						 anIOR->Value().c_str(),
						 aName->Value().c_str());
	    myDlg->myMapOldHypos[ anIOR->Value() ] = myDlg->ListHypAssignation->index(anItem);
	  }
	}
      }
    }
  }
}

//=================================================================================
// function : initAlgoAssignation
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::initAlgoAssignation()
{
  myDlg->myMapOldAlgos.clear();
  myDlg->ListAlgoAssignation->clear();
  if (myImportedMesh)
    return;

  _PTR(Study) study = studyDS();  
  _PTR(SObject)          aMorSM, AHR, aRef;
  _PTR(GenericAttribute) anAttr;
  _PTR(AttributeName)    aName;
  _PTR(AttributeIOR)     anIOR;

  QStringList selMesh;
  myDlg->selectedObject( SMESHGUI_EditHypothesesDlg::MeshOrSubMesh, selMesh );
  aMorSM = studyDS()->FindObjectID( selMesh.first() );
  
  if (aMorSM && aMorSM->FindSubObject(3, AHR)) {
    _PTR(ChildIterator) it =
      study->NewChildIterator(AHR);
    for (; it->More();it->Next()) {
      _PTR(SObject) Obj = it->Value();
      if (Obj->ReferencedObject(aRef)) {
        if (aRef->FindAttribute(anAttr, "AttributeName")) {
	  aName = anAttr;
	  if (aRef->FindAttribute(anAttr, "AttributeIOR")) {
	    anIOR = anAttr;
	    ListBoxIOR* anItem = new ListBoxIOR (myDlg->ListAlgoAssignation,
						 anIOR->Value().c_str(),
						 aName->Value().c_str());
	    myDlg->myMapOldAlgos[ anIOR->Value() ] = myDlg->ListAlgoAssignation->index(anItem);
	  }
	}
      }
    }
  }
}

//=================================================================================
// function : createFilter
// purpose  :
//=================================================================================
SUIT_SelectionFilter* SMESHGUI_EditHypothesesOp::createFilter( const int id ) const
{
  if( id==SMESHGUI_EditHypothesesDlg::GeomShape )
  {
    TColStd_MapOfInteger allTypesMap;
    for (int i = 0; i < 10; i++)
      allTypesMap.Add(i);
    return new SMESH_NumberFilter ("GEOM", TopAbs_SHAPE, 0, allTypesMap);
  }
  
  else if( id==SMESHGUI_EditHypothesesDlg::MeshOrSubMesh )
    return new SMESH_TypeFilter (MESHorSUBMESH);

  else
    return 0;
}

//=================================================================================
// function : updateDialog
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::updateDialog()
{
  bool isEnabled = myDlg->hasSelection( SMESHGUI_EditHypothesesDlg::MeshOrSubMesh ) &&
                   myDlg->hasSelection( SMESHGUI_EditHypothesesDlg::GeomShape ) &&
                   ( myDlg->hypoCount() || myDlg->algoCount() );

  myDlg->setButtonEnabled( myDlg->isModified() && isEnabled && !myImportedMesh, QtxDialog::OK | QtxDialog::Apply );
  myDlg->setObjectEnabled( SMESHGUI_EditHypothesesDlg::GeomShape, ALLOW_CHANGE_SHAPE && !myImportedMesh );
  myDlg->setListsEnabled( myDlg->hasSelection( SMESHGUI_EditHypothesesDlg::MeshOrSubMesh ) && !myImportedMesh );
}

//=================================================================================
// function : initDialog
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::initDialog()
{
  SMESHGUI_SelectionOp::initDialog();

  initHypDefinition();
  initAlgoDefinition();
  
  updateDialog();
}

//=================================================================================
// function : storeMeshOrSubMesh
// purpose  :
//=================================================================================
bool SMESHGUI_EditHypothesesOp::storeMeshOrSubMesh()
{
  MapIOR anOldHypos, aNewHypos;
  if( !myDlg->hasSelection( SMESHGUI_EditHypothesesDlg::GeomShape ) )
    return false;

  QStringList selMesh, selGeom;
  myDlg->selectedObject( SMESHGUI_EditHypothesesDlg::MeshOrSubMesh, selMesh );
  myDlg->selectedObject( SMESHGUI_EditHypothesesDlg::GeomShape, selGeom );

  // 1. Check whether the geometric shape has changed
  _PTR(SObject) SO = studyDS()->FindObjectID( selMesh.first() );
  _PTR(SObject) aGeom = studyDS()->FindObjectID( selGeom.first() );

  GEOM::GEOM_Object_var aGeomShape = GEOM::GEOM_Object::_narrow( _CAST(SObject,aGeom)->GetObject() );
  
  GEOM::GEOM_Object_var aIniGeomShape = SMESH::GetShapeOnMeshOrSubMesh(SO);
  bool bShapeChanged = aIniGeomShape->_is_nil() || !aIniGeomShape->_is_equivalent(aGeomShape);
  if (bShapeChanged) {
    // VSR : TODO : Set new shape - not supported yet by engine
    // 1. remove all old hypotheses and algorithms
    // 2. set new shape
  }
  int nbFail = 0;
  MapIOR::iterator it;

  // 2. remove not used hypotheses from the submesh
  for (it = myDlg->myMapOldHypos.begin(); it != myDlg->myMapOldHypos.end(); ++it) {
    QString ior = it.key();
    int index = myDlg->findItem(myDlg->ListHypAssignation, ior);
    if (index < 0) {
      SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.latin1());
      if (!aHyp->_is_nil()) {
	if (!SMESH::RemoveHypothesisOrAlgorithmOnMesh( SO, aHyp))
	  nbFail++;
      }
    }
  }

  // 3. remove not used algorithms from the submesh
  for (it = myDlg->myMapOldAlgos.begin(); it != myDlg->myMapOldAlgos.end(); ++it) {
    QString ior = it.key();
    int index = myDlg->findItem(myDlg->ListAlgoAssignation, ior);
    if (index < 0) {
      SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.latin1());
      if (!aHyp->_is_nil()){
	if (!SMESH::RemoveHypothesisOrAlgorithmOnMesh( SO, aHyp ) )
	  nbFail++;
      }
    }
  }

  SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( _CAST(SObject,SO)->GetObject() );
  SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow( _CAST(SObject,SO)->GetObject() );

  // 4. Add new algorithms
  for (int i = 0; i < myDlg->ListAlgoAssignation->count(); i++) {
    if (myDlg->ListAlgoAssignation->item(i)->rtti() == ListBoxIOR::RTTI_IOR) {
      ListBoxIOR* anItem = (ListBoxIOR*)(myDlg->ListAlgoAssignation->item(i));
      if (anItem) {
	QString ior = anItem->GetIOR();
	if (myDlg->myMapOldAlgos.find(ior) == myDlg->myMapOldAlgos.end()) {
	  SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.latin1());
	  if (!aHyp->_is_nil()){
      if( !aMesh->_is_nil() && !SMESH::AddHypothesisOnMesh( aMesh, aHyp))
	      nbFail++;
      if( !aSubMesh->_is_nil() && !SMESH::AddHypothesisOnSubMesh( aSubMesh, aHyp))
	      nbFail++;
	  }
	}
      }
    }
  }

  // 5. Add new hypotheses
  for (int i = 0; i < myDlg->ListHypAssignation->count(); i++) {
    if (myDlg->ListHypAssignation->item(i)->rtti() == ListBoxIOR::RTTI_IOR) {
      ListBoxIOR* anItem = (ListBoxIOR*)(myDlg->ListHypAssignation->item(i));
      if (anItem) {
	QString ior = anItem->GetIOR();
	if (myDlg->myMapOldHypos.find(ior) == myDlg->myMapOldHypos.end()) {
	  SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.latin1());
	  if (!aHyp->_is_nil()){
      if( !aMesh->_is_nil() && !SMESH::AddHypothesisOnMesh( aMesh, aHyp))
	      nbFail++;
      if( !aSubMesh->_is_nil() && !SMESH::AddHypothesisOnSubMesh( aSubMesh, aHyp))
	      nbFail++;
	  }
	}
      }
    }
  }
  return (nbFail == 0);
}

//=================================================================================
// function : onUpdate
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesOp::onUpdate()
{
  updateDialog();
}
