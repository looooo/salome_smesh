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
: SMESHGUI_SelectionOp(),
  myDlg( 0 )
{
  setAutoResumed( true );
}

//=================================================================================
// function : Destructor
// purpose  :
//=================================================================================
SMESHGUI_InitMeshOp::~SMESHGUI_InitMeshOp()
{
  if( myDlg )
    delete myDlg;
}

//=================================================================================
// function : startOperation
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_InitMeshDlg();
    connect( myDlg, SIGNAL( nameChanged( const QString& ) ), this, SLOT( onNameChanged( const QString& ) ) );
  }

  SMESHGUI_SelectionOp::startOperation();

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
  SMESHGUI_SelectionOp::selectionDone();
  if( myDlg )
    updateDialog();
}

//=================================================================================
// function : createFilter
// purpose  :
//=================================================================================
SUIT_SelectionFilter* SMESHGUI_InitMeshOp::createFilter( const int id ) const
{
  if( id==SMESHGUI_InitMeshDlg::GeomObj )
  {
    TColStd_MapOfInteger allTypesMap;
    for (int i = 0; i < 10; i++)
      allTypesMap.Add(i);
    return new SMESH_NumberFilter( "GEOM", TopAbs_SHAPE, 0, allTypesMap );
  }

  else if( id==SMESHGUI_InitMeshDlg::Hypo )
    return new SMESH_TypeFilter (HYPOTHESIS);

  else if( id==SMESHGUI_InitMeshDlg::Algo )
    return new SMESH_TypeFilter (ALGORITHM);

  else
    return 0;
}

//=================================================================================
// function : commitOperation
// purpose  :
//=================================================================================
bool SMESHGUI_InitMeshOp::onApply()
{
  if( isStudyLocked() )
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

  initDialog();
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
void SMESHGUI_InitMeshOp::initDialog()
{
  SMESHGUI_SelectionOp::initDialog();
  if( myDlg )
  {
    myDlg->setMeshName( defaultMeshName() );
    updateDialog();
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

//=================================================================================
// function : onNameChanged()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshOp::onNameChanged( const QString& )
{
  updateDialog();
}

//=================================================================================
// function : updateDialog()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshOp::updateDialog()
{
  if( !myDlg )
    return;
    
  bool isEnabled = !myDlg->meshName().isEmpty() &&
                   myDlg->hasSelection( SMESHGUI_InitMeshDlg::GeomObj ) &&
                   myDlg->hasSelection( SMESHGUI_InitMeshDlg::Hypo ) &&
                   myDlg->hasSelection( SMESHGUI_InitMeshDlg::Algo );
  myDlg->setButtonEnabled( isEnabled, QtxDialog::OK | QtxDialog::Apply );
}
