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
//  File   : SMESHGUI_InitMeshDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_InitMeshOp.h"
#include <SMESHGUI_InitMeshDlg.h>
#include <SMESHGUI.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>

#include <SMESH_NumberFilter.hxx>
#include <SMESH_TypeFilter.hxx>

#include <SALOMEDS_SObject.hxx>

#include <SUIT_SelectionFilter.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>

#include <SalomeApp_SelectionMgr.h>
#include <SalomeApp_Tools.h>
#include <SalomeApp_UpdateFlags.h>

#include <TColStd_MapOfInteger.hxx>

#include <GEOMBase.h>

//=================================================================================
// function : Constructor
// purpose  :
//=================================================================================
SMESHGUI_InitMeshOp::SMESHGUI_InitMeshOp()
: SMESHGUI_Operation(),
  myDlg( 0 ),
  myGeomFilter( 0 ),
  myHypothesisFilter( 0 ),
  myAlgorithmFilter( 0 )
{
}

//=================================================================================
// function : Destructor
// purpose  :
//=================================================================================
SMESHGUI_InitMeshOp::~SMESHGUI_InitMeshOp()
{
  if( myDlg )
    delete myDlg;
    
  if( myGeomFilter )
    delete myGeomFilter;
  
  if( myAlgorithmFilter )
    delete myAlgorithmFilter;

  if( myHypothesisFilter )
    delete myHypothesisFilter;    
}

//=================================================================================
// function : startOperation
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_InitMeshDlg( getSMESHGUI() );
    connect( myDlg, SIGNAL( objectActivated( int ) ), this, SLOT( onActivateObject( int ) ) );
  }

  SMESHGUI_Operation::startOperation();

  if( !myGeomFilter )
  {
    TColStd_MapOfInteger allTypesMap;
    for (int i = 0; i < 10; i++)
      allTypesMap.Add(i);
    myGeomFilter       = new SMESH_NumberFilter ("GEOM", TopAbs_SHAPE, 0, allTypesMap);
  }

  if( !myAlgorithmFilter )
    myAlgorithmFilter  = new SMESH_TypeFilter (ALGORITHM);

  if( !myHypothesisFilter )
    myHypothesisFilter = new SMESH_TypeFilter (HYPOTHESIS);
    
  init();
  myDlg->show();
}

//=================================================================================
// function : dlg
// purpose  :
//=================================================================================
SalomeApp_Dialog* SMESHGUI_InitMeshOp::dlg() const
{
  return myDlg;
}

//=================================================================================
// function : selectionDone
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshOp::selectionDone()
{
  QStringList names, ids;
  SMESHGUI_Dialog::TypesList types;
  selected( names, types, ids );
  if( myDlg )
  {
    myDlg->selectObject( names, types, ids );
    myDlg->updateControlState();
  }
}

//=================================================================================
// function : onActivateObject
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshOp::onActivateObject( int obj )
{
  SalomeApp_SelectionMgr* mgr = selectionMgr();

  if( !mgr )
    return;
    
  mgr->clearFilters();
  if( obj==SMESHGUI_InitMeshDlg::GeomObj )
    mgr->installFilter( myGeomFilter );
    
  else if( obj==SMESHGUI_InitMeshDlg::Hypo )
    mgr->installFilter( myHypothesisFilter );

  else if( obj==SMESHGUI_InitMeshDlg::Algo )
    mgr->installFilter( myAlgorithmFilter );
}

//=================================================================================
// function : commitOperation
// purpose  :
//=================================================================================
bool SMESHGUI_InitMeshOp::onApply()
{
  if( getSMESHGUI()->isActiveStudyLocked() )
    return false;

  QString myNameMesh = myDlg->meshName();
  if (myNameMesh.isEmpty())
  {
    SUIT_MessageBox::warn1( dlg(), tr("SMESH_WRN_WARNING"),
                            tr("SMESH_WRN_EMPTY_NAME"), tr("SMESH_BUT_OK") );
    return false;
  }

  SUIT_OverrideCursor wc;

  QStringList selGeom, selHypo, selAlgo;
  myDlg->selectedObject( SMESHGUI_InitMeshDlg::GeomObj, selGeom );
  myDlg->selectedObject( SMESHGUI_InitMeshDlg::Hypo, selHypo );
  myDlg->selectedObject( SMESHGUI_InitMeshDlg::Algo, selAlgo );

  _PTR(Study) study = studyDS();
  _PTR(SObject) aGeomSO = study->FindObjectID( selGeom.first() );
  GEOM::GEOM_Object_var myGeomShape = GEOM::GEOM_Object::_narrow( _CAST(SObject,aGeomSO)->GetObject() );
    
  // create mesh
  SMESH::SMESH_Mesh_var aMesh = initMesh(myGeomShape, myNameMesh);

  if (!aMesh->_is_nil())
  {
    // assign hypotheses
    for (int i = 0; i < selHypo.count(); i++)
    {
      _PTR(SObject) aHypSOClient =
        study->FindObjectID(selHypo[i].latin1());
      if (aHypSOClient)
      {
        CORBA::Object_var anObject = _CAST(SObject,aHypSOClient)->GetObject();
        if (!CORBA::is_nil(anObject))
        {
          SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow(anObject);
          if (!aHyp->_is_nil())
          {
            if (!SMESH::AddHypothesisOnMesh(aMesh, aHyp))
              return false;
          }
        }
      }
    }
    // assign algorithms
    for (int i = 0; i < selAlgo.count(); i++)
    {
      _PTR(SObject) aHypSOClient =
        study->FindObjectID(selAlgo[i].latin1());
      if (aHypSOClient)
      {
        CORBA::Object_var anObject = _CAST(SObject,aHypSOClient)->GetObject();
        if (!CORBA::is_nil(anObject))
        {
          SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow(anObject);
          if (!aHyp->_is_nil())
          {
            if (!SMESH::AddHypothesisOnMesh(aMesh, aHyp))
              return false;
          }
        }
      }
    }
  }

  update( UF_Model | UF_ObjBrowser );

  init();
  return true;
}

//=================================================================================
// function : defaultMeshName()
// purpose  :
//=================================================================================
QString SMESHGUI_InitMeshOp::defaultMeshName() const
{
  _PTR(Study) aStudy = studyDS();
  int aNumber = 0;
  QString aMeshName;
  _PTR(SObject) obj;

  do {
    aMeshName = QString(tr("SMESH_OBJECT_MESH")) + "_" + QString::number(++aNumber);
    obj = aStudy->FindObject(aMeshName.latin1());
  } while (obj);

  return aMeshName;
}

//=================================================================================
// function : init()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshOp::init()
{
  if( myDlg )
  {
    myDlg->setMeshName( defaultMeshName() );
    myDlg->clearSelection();
    myDlg->updateControlState();
  }    
}

//=================================================================================
// function : defaultMeshName()
// purpose  :
//=================================================================================
SMESH::SMESH_Mesh_var SMESHGUI_InitMeshOp::initMesh ( GEOM::GEOM_Object_ptr theShapeObject,
                                                      const QString& theMeshName )
{
  SMESH::SMESH_Mesh_var aMesh;
  try
  {
    SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
    if (!aSMESHGen->_is_nil() && !theShapeObject->_is_nil())
    {
      aMesh = aSMESHGen->CreateMesh(theShapeObject);
      if (!aMesh->_is_nil())
      {
        _PTR(SObject) aMeshSObject = SMESH::FindSObject(aMesh.in());
        SMESH::SetName(aMeshSObject, theMeshName);
      }
    }
  }
  catch( const SALOME::SALOME_Exception& S_ex )
  {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  
  return aMesh._retn();
}

//=================================================================================
// function : defaultMeshName()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshOp::onSelectionChanged( int id )
{
  if( !myDlg->hasSelection( id ) )
    return;
    
  if( id==SMESHGUI_InitMeshDlg::GeomObj )
  {
    QStringList selGeom;
    myDlg->selectedObject( SMESHGUI_InitMeshDlg::GeomObj, selGeom );

    _PTR(SObject) aGeomSO = studyDS()->FindObjectID( selGeom.first() );
    GEOM::GEOM_Object_var myGeomShape = GEOM::GEOM_Object::_narrow( _CAST(SObject,aGeomSO)->GetObject() );
    if( myGeomShape->_is_nil() || !GEOMBase::IsShape( myGeomShape ) )
      myDlg->clearSelection( id );
  }
}
