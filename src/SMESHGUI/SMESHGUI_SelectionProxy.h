// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#ifndef SMESHGUI_OBJECTPROXY_H
#define SMESHGUI_OBJECTPROXY_H

#include "SMESH_SMESHGUI.hxx"

#include <SALOME_InteractiveObject.hxx>
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(GEOM_Gen)

#include <QColor>
#include <QMap>
#include <QSet>
#include <QString>

class SMESH_Actor;

class SMESHGUI_EXPORT SMESHGUI_MeshInfo
{
  QMap<int, long> myInfo;

public:
  SMESHGUI_MeshInfo();

  void addInfo( int, long );
  uint info( int ) const;
  uint operator[] ( int );
  uint count( int, int ) const;
};

class SMESHGUI_EXPORT SMESHGUI_MedFileInfo
{
  QString myFileName;
  uint mySize;
  uint myMajor, myMinor, myRelease;

public:
  SMESHGUI_MedFileInfo();

  bool isValid() const;
  QString fileName() const;
  void setFileName( const QString& );
  uint size() const;
  void setSize( uint );
  QString version() const;
  void setVersion( uint, uint, uint );
};

class SMESHGUI_EXPORT SMESHGUI_SelectionProxy
{
  Handle(SALOME_InteractiveObject) myIO;
  SMESH::SMESH_IDSource_var myObject;
  SMESH_Actor* myActor;
  bool myDirty;

public:
  enum Type
  {
    Unknown,
    Mesh,
    Submesh,
    Group,
    GroupStd,
    GroupGeom,
    GroupFilter
  };

  SMESHGUI_SelectionProxy();
  SMESHGUI_SelectionProxy( const Handle(SALOME_InteractiveObject)& );
  SMESHGUI_SelectionProxy( SMESH::SMESH_IDSource_ptr );
  SMESHGUI_SelectionProxy( const SMESHGUI_SelectionProxy& );

  SMESHGUI_SelectionProxy& operator= ( const SMESHGUI_SelectionProxy& );
  bool operator== ( const SMESHGUI_SelectionProxy& );

  void refresh();

  bool isNull() const;
  operator bool() const;

  SMESH::SMESH_IDSource_ptr object() const;
  const Handle(SALOME_InteractiveObject)& io() const;
  SMESH_Actor* actor() const;

  bool isValid() const;
  void load();

  QString name() const;
  Type type() const;
  SMESHGUI_MeshInfo meshInfo() const;
  SMESHGUI_SelectionProxy mesh() const;
  bool hasShapeToMesh() const;
  GEOM::GEOM_Object_ptr shape() const;
  QString shapeName() const;
  int shapeType() const;

  bool isMeshLoaded() const;
  SMESHGUI_MedFileInfo medFileInfo() const;
  QList<SMESHGUI_SelectionProxy> submeshes() const;
  QList<SMESHGUI_SelectionProxy> groups() const;

  SMESH::ElementType elementType() const;
  QColor color() const;
  int size( bool = false ) const;
  int nbNodes( bool = false ) const;
  QSet<uint> ids() const;

private:
  void init();
};

#endif // SMESHGUI_OBJECTPROXY_H
