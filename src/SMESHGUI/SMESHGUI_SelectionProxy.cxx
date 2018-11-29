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

#include "SMESHGUI_SelectionProxy.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

#include <SALOMEDSClient_Study.hxx>
#include <SUIT_ResourceMgr.h>

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_SelectionProxy
/// \brief Provide operations over the selected object.
///
/// The selection proxy class is aimed to use in dialogs to access mesh object
/// data either from actor or directly from CORBA reference, depending on what
/// of them is accessible.
/// This is useful in situations when some information is needed, but an actor
/// was not created yet.
/// 
/// Selection proxy can be constructed in two ways:
/// - From interactive object: this performs full proxy initialization including
///   pick-up of an actor from the current viewer if it exists.
/// - From mesh source object (CORBA reference); for performance reasons in this
///   case full initialization is not immediately done and performed only when
///   needed.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Default constructor. Creates null proxy.
*/
SMESHGUI_SelectionProxy::SMESHGUI_SelectionProxy(): myActor(0), myDirty(false)
{
}

/*!
  \brief Constructor.
  \param io Interactive object.
*/
SMESHGUI_SelectionProxy::SMESHGUI_SelectionProxy( const Handle(SALOME_InteractiveObject)& io ): myActor(0), myDirty(true)
{
  myObject = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( io );
  init();
}

/*!
  \brief Constructor.
  \param object Mesh source.
*/
SMESHGUI_SelectionProxy::SMESHGUI_SelectionProxy( SMESH::SMESH_IDSource_ptr object ): myActor(0), myDirty(true)
{
  if ( !CORBA::is_nil( object ) )
    myObject = SMESH::SMESH_IDSource::_duplicate( object );
}

/*!
  \brief Copy constructor.
  \param other Proxy being copied.
*/
SMESHGUI_SelectionProxy::SMESHGUI_SelectionProxy( const SMESHGUI_SelectionProxy& other )
{
  myIO = other.myIO;
  myObject = other.myObject;
  myActor = other.myActor;
  myDirty = other.myDirty;
}

/*!
  \brief Perform internal initialization.
  \internal
*/
void SMESHGUI_SelectionProxy::init()
{
  if ( myIO.IsNull() )
    myIO = new SALOME_InteractiveObject(); // create dummy IO to avoid crashes when accesing it

  if ( !CORBA::is_nil( myObject ) )
  {
    if ( !myIO->hasEntry() )
    {
      _PTR(SObject) sobj = SMESH::ObjectToSObject( myObject.in() );
      if ( sobj )
        myIO = new SALOME_InteractiveObject( sobj->GetID().c_str(), "SMESH", sobj->GetName().c_str() );
    }
    if ( !myActor && myIO->hasEntry() )
      myActor = SMESH::FindActorByEntry( myIO->getEntry() );
  }
  myDirty = false;
}

/*!
  \brief Assignment operator.
  \param other Proxy being copied.
*/
SMESHGUI_SelectionProxy& SMESHGUI_SelectionProxy::operator= ( const SMESHGUI_SelectionProxy& other )
{
  myIO = other.myIO;
  myObject = other.myObject;
  myActor = other.myActor;
  myDirty = other.myDirty;
  return *this;
}

/*!
  \brief Equality comparison operator.
  \param other Proxy to compare with.
  \return \c true if two proxies are equal; \c false otherwise.
*/
bool SMESHGUI_SelectionProxy::operator== ( const SMESHGUI_SelectionProxy& other )
{
  return !CORBA::is_nil( myObject ) && !CORBA::is_nil( other.myObject ) && 
    myObject->_is_equivalent( other.myObject );
}

/*!
  \brief Try to re-initialize proxy.
*/
void SMESHGUI_SelectionProxy::refresh()
{
  init();
}

/*!
  \brief Check if proxy is null.
  \return \c true if proxy is null; \c false otherwise.
*/
bool SMESHGUI_SelectionProxy::isNull() const
{
  return CORBA::is_nil( myObject );
}

/*!
  \brief Boolean conversion operator.
  \return \c true if proxy is not null; \c false otherwise.
*/
SMESHGUI_SelectionProxy::operator bool() const
{
  return !isNull();
}

/*!
  \brief Get interactive object.
  \return Interactive object referenced by proxy.
*/
const Handle(SALOME_InteractiveObject)& SMESHGUI_SelectionProxy::io() const
{
  if ( myDirty )
    const_cast<SMESHGUI_SelectionProxy*>(this)->init();
  return myIO;
}

/*!
  \brief Get mesh object.
  \return Mesh object (mesh, sub-mesh, group, etc.) referenced by proxy.
*/
SMESH::SMESH_IDSource_ptr SMESHGUI_SelectionProxy::object() const
{
  return SMESH::SMESH_IDSource::_duplicate( myObject );
}

/*!
  \brief Get actor.
  \return Actor referenced by proxy.
*/
SMESH_Actor* SMESHGUI_SelectionProxy::actor() const
{
  if ( myDirty )
    const_cast<SMESHGUI_SelectionProxy*>(this)->init();
  return myActor;
}

/*!
  \brief Get object's validity.

  Mesh object is valid if it is null and information stored in it is valid.

  \return \c true if object is valid; \c false otherwise.
*/
bool SMESHGUI_SelectionProxy::isValid() const
{
  return !isNull() && myObject->IsMeshInfoCorrect();
}

/*!
  \brief Load mesh object from study file.
*/
void SMESHGUI_SelectionProxy::load()
{
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
    if ( !CORBA::is_nil( mesh ) )
      mesh->Load();
  }
}

/*!
  \brief Get name.
  \return Mesh object's name.
*/
QString SMESHGUI_SelectionProxy::name() const
{
  QString value;
  if ( !isNull() )
    value = io()->getName();
  return value;
}

/*!
  \brief Get type.
  \return Mesh object's type.
*/
SMESHGUI_SelectionProxy::Type SMESHGUI_SelectionProxy::type() const
{
  Type value = Unknown;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    SMESH::SMESH_subMesh_var submesh = SMESH::SMESH_subMesh::_narrow( myObject );
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    SMESH::SMESH_Group_var sGroup = SMESH::SMESH_Group::_narrow( myObject );
    SMESH::SMESH_GroupOnGeom_var gGroup = SMESH::SMESH_GroupOnGeom::_narrow( myObject );
    SMESH::SMESH_GroupOnFilter_var fGroup = SMESH::SMESH_GroupOnFilter::_narrow( myObject );
    
    if ( !CORBA::is_nil( mesh ) )
      value = Mesh;
    else if ( !CORBA::is_nil( submesh ) )
      value = Submesh;
    else if ( !CORBA::is_nil( sGroup ) )
      value = GroupStd;
    else if ( !CORBA::is_nil( gGroup ) )
      value = GroupGeom;
    else if ( !CORBA::is_nil( fGroup ) )
      value = GroupFilter;
    else if ( !CORBA::is_nil( group ) )
      value = Group;
  }
  return value;
}

/*!
  \brief Get mesh information.
  \return Statistics on stored mesh object.
*/
SMESHGUI_MeshInfo SMESHGUI_SelectionProxy::meshInfo() const
{
  SMESHGUI_MeshInfo info;
  if ( !isNull() )
  {
    SMESH::long_array_var data = myObject->GetMeshInfo();
    for ( uint type = SMESH::Entity_Node; type < SMESH::Entity_Last; type++ )
    {
      if ( type < data->length() )
	info.addInfo( type, data[ type ] );
    }
  }
  return info;
}

/*!
  \brief Get parent mesh.
  \return Proxy object that stores parent mesh object.
  \note For proxy that stores a mesh, returns its copy.
*/
SMESHGUI_SelectionProxy SMESHGUI_SelectionProxy::mesh() const
{
  SMESHGUI_SelectionProxy parent;
  if ( isValid() )
    parent = SMESHGUI_SelectionProxy( (SMESH::SMESH_Mesh_var) myObject->GetMesh() ); // cast to var to avoid leaks
  return parent;
}

/*!
  \brief Check if parent mesh has shape to mesh.
  \return \c true if Parent mesh is built on a shape; \c false otherwise.
  \note For group or submesh, this method checks that parent mesh has a shape.
*/
bool SMESHGUI_SelectionProxy::hasShapeToMesh() const
{
  bool result = false;
  if ( !isNull() )
  {
    SMESHGUI_SelectionProxy parent = mesh();
    if ( parent )
    {
      SMESH::SMESH_Mesh_var m = SMESH::SMESH_Mesh::_narrow( (SMESH::SMESH_IDSource_var)parent.object() ); // cast to var to avoid leaks
      result = m->HasShapeToMesh();
    }
  }
  return result;
}

/*!
  \brief Get referenced GEOM object.
  \return GEOM object referenced by selection proxy.

  The result contains valid pointer only if proxy refers to mesh, sub-mesh
  or group created on a geometry.
*/
GEOM::GEOM_Object_ptr SMESHGUI_SelectionProxy::shape() const
{
  GEOM::GEOM_Object_var shape;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    SMESH::SMESH_subMesh_var submesh = SMESH::SMESH_subMesh::_narrow( myObject );
    SMESH::SMESH_GroupOnGeom_var group = SMESH::SMESH_GroupOnGeom::_narrow( myObject );
    if ( !CORBA::is_nil( mesh ) )
      shape = mesh->GetShapeToMesh();
    else if ( !CORBA::is_nil( submesh ) )
      shape = submesh->GetSubShape();
    else if ( !CORBA::is_nil( group ) )
      shape = group->GetShape();
  }
  return shape._retn();
}

/*!
  \brief Get name of referenced GEOM object.
  \return Name of GEOM object referenced by selection proxy.

  The result contains non-empty name only if proxy refers to mesh, sub-mesh
  or group created on a geometry, and if that geometry has valid name.
*/
QString SMESHGUI_SelectionProxy::shapeName() const
{
  QString name;
  if ( !isNull() )
  {
    GEOM::GEOM_Object_var gobj = shape();
    _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
    if ( sobj )
      name = QString::fromStdString( sobj->GetName() );
  }
  return name;
}

/*!
  \brief Get type of referenced GEOM object.
  \return Type of GEOM object referenced by selection proxy.

  The result contains valid type only if proxy refers to mesh, sub-mesh
  or group created on a geometry.
*/
int SMESHGUI_SelectionProxy::shapeType() const
{
  int type = -1;
  if ( !isNull() )
  {
    GEOM::GEOM_Object_var gobj = shape();
    if ( !CORBA::is_nil( gobj ) )
      type = gobj->GetShapeType();
  }
  return type;
}

/*!
  \brief Check if mesh has been loaded from study file.
  \return \c true if mesh was loaded; \c false otherwise.
*/
bool SMESHGUI_SelectionProxy::isMeshLoaded() const
{
  bool result = true; // set default to true to avoid side effects
  if ( isValid() )
  {
    SMESH::SMESH_Mesh_var mesh = myObject->GetMesh();
    if ( !CORBA::is_nil( mesh ) )
      result = mesh->IsLoaded();
  }
  return result;
}

/*!
  \brief Get MED file information.
  \return MED file information.

  The result contains valid data only if proxy refers to mesh object
  which has been imported from the MED file.
*/
SMESHGUI_MedFileInfo SMESHGUI_SelectionProxy::medFileInfo() const
{
  SMESHGUI_MedFileInfo info;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::MedFileInfo_var inf = mesh->GetMEDFileInfo();
      info.setFileName( inf->fileName.in() );
      info.setSize( inf->fileSize );
      info.setVersion( inf->major, inf->minor, inf->release );
    }
  }
  return info;
}

/*!
  \brief Get child sub-meshes.
  \return List of sub-meshes for mesh object; empty list for other types.
*/
QList<SMESHGUI_SelectionProxy> SMESHGUI_SelectionProxy::submeshes() const
{
  QList<SMESHGUI_SelectionProxy> lst;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::submesh_array_var array = mesh->GetSubMeshes();
      for ( uint i = 0 ; i < array->length(); i++ )
        lst << SMESHGUI_SelectionProxy( array[i].in() );
    }
  }
  return lst;
}

/*!
  \brief Get child groups.
  \return List of groups for mesh object; empty list for other types.
*/
QList<SMESHGUI_SelectionProxy> SMESHGUI_SelectionProxy::groups() const
{
  QList<SMESHGUI_SelectionProxy> lst;
  if ( !isNull() )
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( myObject );
    if ( !CORBA::is_nil( mesh ) )
    {
      SMESH::ListOfGroups_var array = mesh->GetGroups();
      for ( uint i = 0 ; i < array->length(); i++ )
        lst << SMESHGUI_SelectionProxy( array[i].in() );
    }
  }
  return lst;
}

/*!
  \brief Get element type (for group). For other mesh objects result is undefined.
  \return Group's element type.
*/
SMESH::ElementType SMESHGUI_SelectionProxy::elementType() const
{
  SMESH::ElementType value = SMESH::ALL;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) )
      value = group->GetType();
  }
  return value;
}

/*!
  \brief Get color (for group). For other mesh objects result is undefined.
  \return Group's color.
*/
QColor SMESHGUI_SelectionProxy::color() const
{
  QColor result;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) )
    {
      SALOMEDS::Color c = group->GetColor();
      result = QColor::fromRgbF( c.R, c.G, c.B );
    }
  }
  return result;
}

/*!
  \brief Get size (for group). For other mesh objects result is undefined.
  \param autoCompute Compute size if it is unavailable. Defaults to \c false.
  \return Group's size.
*/
int SMESHGUI_SelectionProxy::size( bool autoCompute ) const
{
  // note: size is not computed for group on filter for performance reasons, see IPAL52831
  int result = -1;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) )
    {
      if ( type() == GroupFilter )
        // for group on filter we check if value is already computed and cached
        autoCompute |= group->IsNodeInfoAvailable();
      else
        // for other groups we force autoCompute to true
        autoCompute = true;
      if ( autoCompute )
        result = group->Size();
    }
  }
  return result;
}

/*!
  \brief Get number of underlying nodes (for group of elements). For other
  mesh objects result is undefined.
  \param autoCompute Compute size if it is unavailable. Defaults to \c false.
  \return Number of nodes contained in group.
*/
int SMESHGUI_SelectionProxy::nbNodes( bool autoCompute ) const
{
  // note: nb of nodes is not computed automatically for performance reasons
  int result = -1;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) && ( autoCompute || isMeshLoaded() ) )
    {
      int groupSize = size( autoCompute );
      if ( groupSize >= 0 )
      {
        int limit = SMESHGUI::resourceMgr()->integerValue( "SMESH", "info_groups_nodes_limit", 100000 );
        if ( group->IsNodeInfoAvailable() || limit <= 0 || groupSize <= limit )
          result = group->GetNumberOfNodes();
      }
    }
  }
  return result;
}

/*!
  \brief Get list of nodes / elements IDs for the mesh group.
  \return List of IDs for group object; empty list for other types.
*/
QSet<uint> SMESHGUI_SelectionProxy::ids() const
{
  QSet<uint> result;
  if ( !isNull() )
  {
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow( myObject );
    if ( !CORBA::is_nil( group ) )
    {
      SMESH::long_array_var seq = group->GetListOfID();
      for ( int i = 0, n = seq->length(); i < n; i++ )
        result << (uint)seq[i];
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_MeshInfo
/// \brief Store statistics on mesh object.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
*/
SMESHGUI_MeshInfo::SMESHGUI_MeshInfo()
{
}

/*!
  \brief Add information on given entity type.
  \param type Entity type.
  \param value Number of entities.
*/
void SMESHGUI_MeshInfo::addInfo( int type, long value )
{
  myInfo[type] = value;
}

/*!
  \brief Get number of entities of given type.
  \param type Entity type.
  \return Number of entities.
*/
uint SMESHGUI_MeshInfo::info( int type ) const
{
  return myInfo.value( type, 0U );
}

/*!
  \brief Access operator.
  \param type Entity type.
  \return Number of entities.
*/
uint SMESHGUI_MeshInfo::operator[] ( int type )
{
  return (uint)info( type );
}

/*!
  \brief Get total number of entities for types in given range.
  \param fromType Lower entity type.
  \param toType Upper entity type.
  \return Number of entities.
*/
uint SMESHGUI_MeshInfo::count( int fromType, int toType ) const
{
  uint value = 0;
  for ( int type = fromType; type <= toType; type++ )
    value += info( type );
  return value;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_MedFileInfo
/// \brief Provide operations over the selected object
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
*/
SMESHGUI_MedFileInfo::SMESHGUI_MedFileInfo()
{
}

/*!
  \brief Get validity status.

  MED file information is valid if at least stored file name is not null.

  \return \c true if MED file info is valid; \c false otherwise.
*/
bool SMESHGUI_MedFileInfo::isValid() const
{
  return !myFileName.isEmpty();
}

/*!
  \brief Get file name for mesh imported from MED file.
  \return MED file name.
*/
QString SMESHGUI_MedFileInfo::fileName() const
{
  return myFileName;
}

/*!
  \brief Set file name for mesh imported from MED file.
  \param fileName MED file name.
*/
void SMESHGUI_MedFileInfo::setFileName( const QString& fileName )
{
  myFileName = fileName;
}

/*!
  \brief Get file size for mesh imported from MED file.
  \return MED file size.
*/
uint SMESHGUI_MedFileInfo::size() const
{
  return mySize;
}

/*!
  \brief Set file size for mesh imported from MED file.
  \param size MED file size.
*/
void SMESHGUI_MedFileInfo::setSize( uint size )
{
  mySize = size;
}

/*!
  \brief Get version of format for mesh imported from MED file.
  \return MED file format version.
*/
QString SMESHGUI_MedFileInfo::version() const
{
  QString v = QString( "%1" ).arg( myMajor );
  if ( myMinor > 0 || myRelease > 0 ) v += QString( ".%1" ).arg( myMinor );
  else if ( myMajor > 0 ) v += ".0";
  if ( myRelease > 0 ) v += QString( ".%1" ).arg( myRelease );
  return v;
}

/*!
  \brief Set version of format for mesh imported from MED file.
  \param major Major version number.
  \param minor Minor version number.
  \param release Release version number.
*/
void SMESHGUI_MedFileInfo::setVersion( uint major, uint minor, uint release )
{
  myMajor = major;
  myMinor = minor;
  myRelease = release;
}
