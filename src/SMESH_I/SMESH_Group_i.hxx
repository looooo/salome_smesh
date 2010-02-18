//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : SMESH_Group_i.hxx
//  Author : Sergey ANIKIN, OCC
//  Module : SMESH
//
#ifndef SMESH_Group_i_HeaderFile
#define SMESH_Group_i_HeaderFile

#include "SMESH.hxx"
#include "SMESH_Mesh_i.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_CLIENT_HEADER(GEOM_Gen)

#include <kernel/SALOME_GenericObj_i.hh>

class SMESH_Group;
class SMESHDS_GroupBase;

// ===========
// Group Base
// ===========
class SMESH_I_EXPORT SMESH_GroupBase_i:
  public virtual POA_SMESH::SMESH_GroupBase,
  public virtual SALOME::GenericObj_i
{
 public:
  SMESH_GroupBase_i(PortableServer::POA_ptr thePOA,
                    SMESH_Mesh_i* theMeshServant,
                    const int theLocalID );
  virtual ~SMESH_GroupBase_i();

  // CORBA interface implementation
  void SetName(const char* name);
  char* GetName();
  SMESH::ElementType GetType();
  CORBA::Long Size();
  CORBA::Boolean IsEmpty();
  CORBA::Boolean Contains(CORBA::Long elem_id);
  CORBA::Long GetID(CORBA::Long elem_index);
  SMESH::long_array* GetListOfID();
  SMESH::SMESH_Mesh_ptr GetMesh();

  /*!
   * Returns statistic of mesh elements
   * Result array of number enityties
   * Inherited from SMESH_IDSource
   */
  virtual SMESH::long_array* GetMeshInfo();

  // Inherited from SMESH_IDSource interface
  virtual SMESH::long_array* GetIDs();

  // Internal C++ interface
  int GetLocalID() const { return myLocalID; }
  SMESH_Mesh_i* GetMeshServant() const { return myMeshServant; }
  SMESH_Group* GetSmeshGroup() const;
  SMESHDS_GroupBase* GetGroupDS() const;

  void SetColor(const SALOMEDS::Color& color);
  SALOMEDS::Color GetColor();

  void SetColorNumber(CORBA::Long color);
  CORBA::Long GetColorNumber();

private:
  SMESH_Mesh_i* myMeshServant;
  int myLocalID;

  void changeLocalId(int localId) { myLocalID = localId; }
  friend void SMESH_Mesh_i::CheckGeomGroupModif();
};

// ======
// Group
// ======

class SMESH_I_EXPORT SMESH_Group_i:
  public virtual POA_SMESH::SMESH_Group,
  public SMESH_GroupBase_i
{
 public:
  SMESH_Group_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID );

  // CORBA interface implementation
  void Clear();
  CORBA::Long Add( const SMESH::long_array& theIDs );
  CORBA::Long Remove( const SMESH::long_array& theIDs );

  CORBA::Long AddByPredicate( SMESH::Predicate_ptr thePredicate );
  CORBA::Long RemoveByPredicate( SMESH::Predicate_ptr thePredicate );
};

// =========================
// Group linked to geometry
// =========================

class SMESH_I_EXPORT SMESH_GroupOnGeom_i:
  public virtual POA_SMESH::SMESH_GroupOnGeom,
  public SMESH_GroupBase_i
{
 public:
  SMESH_GroupOnGeom_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID );

  // CORBA interface implementation
  GEOM::GEOM_Object_ptr GetShape();
};
#endif
