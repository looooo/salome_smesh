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
//  File   : SMESHGUI_StandardMeshInfosOp.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#include "SMESHGUI_StandardMeshInfosOp.h"
#include <SMESHGUI_StandardMeshInfosDlg.h>
#include <SMESHGUI_Utils.h>

#include <SMESH.hxx>
#include <SMESH_TypeFilter.hxx>

#include <SUIT_OverrideCursor.h>

#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Group)

#include <SALOMEDS_SObject.hxx>

//=================================================================================
// function : SMESHGUI_StandardMeshInfosOp()
// purpose  : constructor
//=================================================================================
SMESHGUI_StandardMeshInfosOp::SMESHGUI_StandardMeshInfosOp()
: SMESHGUI_SelectionOp(),
  myDlg( 0 )
{
}

//=================================================================================
// function : ~SMESHGUI_StandardMeshInfosOp()
// purpose  : destructor
//=================================================================================
SMESHGUI_StandardMeshInfosOp::~SMESHGUI_StandardMeshInfosOp()
{
  if( myDlg )
    delete myDlg;
}

//=================================================================================
// function : dlg
// purpose  : 
//=================================================================================
SalomeApp_Dialog* SMESHGUI_StandardMeshInfosOp::dlg() const
{
  return myDlg;
}

//=================================================================================
// function : startOperation
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosOp::startOperation()
{
  if( !myDlg )
    myDlg = new SMESHGUI_StandardMeshInfosDlg();

  SMESHGUI_SelectionOp::startOperation();

  myDlg->show();
}

//=================================================================================
// function : createFilter
// purpose  :
//=================================================================================
SUIT_SelectionFilter* SMESHGUI_StandardMeshInfosOp::createFilter( const int ) const
{
  return new SMESH_TypeFilter( MESH );
}

//=================================================================================
// function : fillMeshInfos
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosOp::fillMeshInfos()
{
  if( !myDlg )
    return;
    
  SUIT_OverrideCursor wc;

  QStringList ids; myDlg->selectedObject( 0, ids );
  if( ids.count()==0 )
  {
    myDlg->setInfo( "" );
    return;
  }

  _PTR(SObject) SO = studyDS()->FindObjectID( ids.first() ), anObj;
  if( !SO )
  {
    myDlg->setInfo( "" );
    return;
  }
    
  SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( _CAST(SObject,SO)->GetObject() );

  if( aMesh->_is_nil() )
  {
    myDlg->setInfo( "" );
    return;
  }


  QString anInfo;
  int aNbNodes =   (int)aMesh->NbNodes();
  int aNbEdges =   (int)aMesh->NbEdges();
  int aNbFaces =   (int)aMesh->NbFaces();
  int aNbVolumes = (int)aMesh->NbVolumes();

  int aDimension = 0;
  double aNbDimElements = 0;
  if( aNbVolumes > 0 )
  {
    aNbDimElements = aNbVolumes;
    aDimension = 3;
  }
  else if( aNbFaces > 0 )
  {
    aNbDimElements = aNbFaces;
    aDimension = 2;
  }
  else if( aNbEdges > 0 )
  {
    aNbDimElements = aNbEdges;
    aDimension = 1;
  }
  else if( aNbNodes > 0 )
  {
    aNbDimElements = aNbNodes;
    aDimension = 0;
  }

  // information about the mesh
  anInfo.append(QString("Nb of element of dimension %1:<b> %2</b><br>").arg(aDimension).arg(aNbDimElements));
  anInfo.append(QString("Nb of nodes: <b>%1</b><br><br>").arg(aNbNodes));

  bool hasGroup = false;

  // info about groups on nodes
  SO->FindSubObject( Tag_NodeGroups, anObj );
  if( anObj )
  {
    _PTR(ChildIterator) it = studyDS()->NewChildIterator(anObj);
    if( it->More() )
    {
      anInfo.append(QString("Groups:<br><br>"));
      hasGroup = true;
    
      for( ; it->More(); it->Next() )
      {
        _PTR(SObject) subObj = it->Value();
        CORBA::Object_var anObject = SMESH::SObjectToObject(subObj);
        SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
        if( !aGroup->_is_nil() )
        {
          anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
          anInfo.append(QString("%1<br>").arg("on nodes"));
          anInfo.append(QString("%1<br>").arg(aGroup->Size()));
          // check if the group based on geometry
          SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
          if (!aGroupOnGeom->_is_nil())
          {
            GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
            QString aShapeName = "<unknown>";
            _PTR(SObject) aGeomObj, aRef;
            if (subObj->FindSubObject(1, aGeomObj) &&  aGeomObj->ReferencedObject(aRef))
              aShapeName = aRef->GetName().c_str();
            anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
          }
          else
            anInfo.append(QString("<br>"));
        }
      }
    }

    // info about groups on edges
    anObj.reset();
    SO->FindSubObject(Tag_EdgeGroups, anObj);
    if( anObj )
    {
      _PTR(ChildIterator) it = studyDS()->NewChildIterator(anObj);
      if( !hasGroup && it->More() )
      {
        anInfo.append(QString("Groups:<br><br>"));
        hasGroup = true;
      }
      for( ; it->More(); it->Next() )
      {
        _PTR(SObject) subObj = it->Value();
        CORBA::Object_var anObject = SMESH::SObjectToObject(subObj);
        SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
        if( !aGroup->_is_nil() )
        {
          anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
          anInfo.append(QString("%1<br>").arg("on edges"));
          anInfo.append(QString("%1<br>").arg(aGroup->Size()));
          // check if the group based on geometry
          SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
          if( !aGroupOnGeom->_is_nil() )
          {
            GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
            QString aShapeName = "<unknown>";
            _PTR(SObject) aGeomObj, aRef;
            if( subObj->FindSubObject( 1, aGeomObj ) && aGeomObj->ReferencedObject( aRef ) )
              aShapeName = aRef->GetName().c_str();
            anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
          }
          else
            anInfo.append(QString("<br>"));
        }
      }
    }

    // info about groups on faces
    anObj.reset();
    SO->FindSubObject( Tag_FaceGroups, anObj );
    if (anObj)
    {
      _PTR(ChildIterator) it = studyDS()->NewChildIterator(anObj);
      if( !hasGroup && it->More() )
      {
        anInfo.append(QString("Groups:<br><br>"));
        hasGroup = true;
      }
      for( ; it->More(); it->Next() )
      {
        _PTR(SObject) subObj = it->Value();
        CORBA::Object_var anObject = SMESH::SObjectToObject(subObj);
        SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
        if( !aGroup->_is_nil() )
        {
          anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
          anInfo.append(QString("%1<br>").arg("on faces"));
          anInfo.append(QString("%1<br>").arg(aGroup->Size()));
          // check if the group based on geometry
          SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
          if( !aGroupOnGeom->_is_nil() )
          {
            GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
            QString aShapeName = "<unknown>";
            _PTR(SObject) aGeomObj, aRef;
            if (subObj->FindSubObject(1, aGeomObj) && aGeomObj->ReferencedObject(aRef))
              aShapeName = aRef->GetName().c_str();
            anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
          }
          else
            anInfo.append(QString("<br>"));
        }
      }
    }

    // info about groups on volumes
    anObj.reset();
    SO->FindSubObject( Tag_VolumeGroups, anObj );
    if( anObj )
    {
      _PTR(ChildIterator) it = studyDS()->NewChildIterator(anObj);
      if( !hasGroup && it->More() )
        anInfo.append(QString("Groups:<br>"));
      for( ; it->More(); it->Next() )
      {
        _PTR(SObject) subObj = it->Value();
        CORBA::Object_var anObject = SMESH::SObjectToObject(subObj);
        SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
        if( !aGroup->_is_nil() )
        {
          anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
          anInfo.append(QString("%1<br>").arg("on volumes"));
          anInfo.append(QString("%1<br>").arg(aGroup->Size()));
          // check if the group based on geometry
          SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
          if( !aGroupOnGeom->_is_nil() )
          {
            GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
            QString aShapeName = "<unknown>";
            _PTR(SObject) aGeomObj, aRef;
            if (subObj->FindSubObject(1, aGeomObj) &&  aGeomObj->ReferencedObject(aRef))
              aShapeName = aRef->GetName().c_str();
            anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
          }
          else
            anInfo.append(QString("<br>"));
        }
      }
    }
  }
  myDlg->setInfo(anInfo);
}

//=================================================================================
// function : onOk
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosOp::onOk()
{
  commit();
}

//=================================================================================
// function : onSelectionChanged
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosOp::onSelectionChanged( int )
{
  fillMeshInfos();
}


