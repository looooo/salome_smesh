//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
//  File   : SMESH_Gen_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_GEN_I_HXX_
#define _SMESH_GEN_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_CLIENT_HEADER(GEOM_Gen)
#include CORBA_CLIENT_HEADER(SALOMEDS)
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)

#include "SMESH_Mesh_i.hxx"
#include "SMESH_Hypothesis_i.hxx"
#include "SALOME_Component_i.hxx"
#include "SALOME_NamingService.hxx"

#include "SMESH_Gen.hxx"
#include "GEOM_Client.hxx"

#include <map>

class SMESH_Mesh_i;
class SALOME_LifeCycleCORBA;

// ===========================================================
// Study context - stores study-connected objects references
// ==========================================================
class StudyContext
{
public:
  // constructor
  StudyContext() {}
  // destructor
  ~StudyContext() 
  { 
    mapIdToIOR.clear();
    mapIdToId.clear();
  }
  // register object in the internal map and return its id
  int addObject( string theIOR )
  {
    int nextId = getNextId();
    mapIdToIOR[ nextId ]  = theIOR;
    return nextId;
  }
  // find the object id in the internal map by the IOR
  int findId( string theIOR )
  {
    map<int, string>::iterator imap;
    for ( imap = mapIdToIOR.begin(); imap != mapIdToIOR.end(); ++imap ) {
      if ( imap->second == theIOR )
        return imap->first;
    }
    return 0;
  }
  // get object's IOR by id
  string getIORbyId( const int theId )
  {
    if ( mapIdToIOR.find( theId ) != mapIdToIOR.end() )
      return mapIdToIOR[ theId ];
    return string( "" );
  }
  // get object's IOR by old id
  string getIORbyOldId( const int theOldId )
  {
    if ( mapIdToId.find( theOldId ) != mapIdToId.end() )
      return getIORbyId( mapIdToId[ theOldId ] );
    return string( "" );
  }
  // maps old object id to the new one (used when restoring data)
  void mapOldToNew( const int oldId, const int newId ) {
    mapIdToId[ oldId ] = newId;
  }
    
private:
  // get next free object identifier
  int getNextId()
  {
    int id = 1;
    while( mapIdToIOR.find( id ) != mapIdToIOR.end() )
      id++;
    return id;
  }

  map<int, string> mapIdToIOR;      // persistent-to-transient map
  map<int, int>    mapIdToId;       // used to translate object from persistent to transient form
};

// ===========================================================
// SMESH module's engine
// ==========================================================
class SMESH_Gen_i:
  public virtual POA_SMESH::SMESH_Gen,
  public virtual Engines_Component_i 
{
public:
  // Get last created instance of the class
  static SMESH_Gen_i* GetSMESHGen() { return mySMESHGen;}
  // Get ORB object
  static CORBA::ORB_var GetORB() { return myOrb;}
  // Get SMESH module's POA object
  static PortableServer::POA_var GetPOA() { return myPoa;}
  // Get Naming Service object
  static SALOME_NamingService* GetNS();
  // Get SALOME_LifeCycleCORBA object
  static SALOME_LifeCycleCORBA* GetLCC();
  // Retrieve and get GEOM engine reference
  static GEOM::GEOM_Gen_ptr GetGeomEngine();
  // Get object of the CORBA reference
  static PortableServer::ServantBase_var GetServant( CORBA::Object_ptr theObject );
  // Get CORBA object corresponding to the SALOMEDS::SObject
  static CORBA::Object_var SObjectToObject( SALOMEDS::SObject_ptr theSObject );

  // Default constructor
  SMESH_Gen_i();
  // Standard constructor
  SMESH_Gen_i( CORBA::ORB_ptr            orb,
               PortableServer::POA_ptr   poa,
               PortableServer::ObjectId* contId, 
               const char*               instanceName, 
               const char*               interfaceName );
  // Destructor
  virtual ~SMESH_Gen_i();
  
  // *****************************************
  // Interface methods
  // *****************************************

  // Set current study
  void SetCurrentStudy( SALOMEDS::Study_ptr theStudy );
  // Get current study
  SALOMEDS::Study_ptr GetCurrentStudy();

  // Create hypothesis/algorothm of given type
  SMESH::SMESH_Hypothesis_ptr CreateHypothesis (const char* theHypType,
                                                const char* theLibName)
    throw ( SALOME::SALOME_Exception );
  
  // Create empty mesh on a shape
  SMESH::SMESH_Mesh_ptr CreateMesh( GEOM::GEOM_Object_ptr theShapeObject )
    throw ( SALOME::SALOME_Exception );

  //  Create mesh(es) and import data from UNV file
  SMESH::SMESH_Mesh_ptr CreateMeshesFromUNV( const char* theFileName )
    throw ( SALOME::SALOME_Exception );

  //  Create mesh(es) and import data from MED file
  SMESH::mesh_array* CreateMeshesFromMED( const char* theFileName,
                                          SMESH::DriverMED_ReadStatus& theStatus )
    throw ( SALOME::SALOME_Exception );

  //  Create mesh(es) and import data from STL file
  SMESH::SMESH_Mesh_ptr CreateMeshesFromSTL( const char* theFileName )
    throw ( SALOME::SALOME_Exception );

  // Compute mesh on a shape
  CORBA::Boolean Compute( SMESH::SMESH_Mesh_ptr theMesh,
                          GEOM::GEOM_Object_ptr  theShapeObject )
    throw ( SALOME::SALOME_Exception );

  // Returns true if mesh contains enough data to be computed
  CORBA::Boolean IsReadyToCompute( SMESH::SMESH_Mesh_ptr theMesh,
                                   GEOM::GEOM_Object_ptr theShapeObject )
    throw ( SALOME::SALOME_Exception );

  // Get sub-shapes unique ID's list
  SMESH::long_array* GetSubShapesId( GEOM::GEOM_Object_ptr      theMainShapeObject,
                                     const SMESH::object_array& theListOfSubShape )
    throw ( SALOME::SALOME_Exception );


  // ****************************************************
  // Interface inherited methods (from SALOMEDS::Driver)
  // ****************************************************

  // Save SMESH data
  SALOMEDS::TMPFile* Save( SALOMEDS::SComponent_ptr theComponent,
			 const char*              theURL,
			 bool                     isMultiFile );
  // Load SMESH data
  bool Load( SALOMEDS::SComponent_ptr theComponent,
	     const SALOMEDS::TMPFile& theStream,
	     const char*              theURL,
	     bool                     isMultiFile );
  // Save SMESH data in ASCII format
  SALOMEDS::TMPFile* SaveASCII( SALOMEDS::SComponent_ptr theComponent,
			        const char*              theURL,
			        bool                     isMultiFile );
  // Load SMESH data in ASCII format
  bool LoadASCII( SALOMEDS::SComponent_ptr theComponent,
		  const SALOMEDS::TMPFile& theStream,
		  const char*              theURL,
		  bool                     isMultiFile );

  // Create filter manager
  SMESH::FilterManager_ptr CreateFilterManager();

  // Return a pattern mesher
  SMESH::SMESH_Pattern_ptr GetPattern();

  // Clears study-connected data when it is closed
  void Close( SALOMEDS::SComponent_ptr theComponent );
  
  // Get component data type
  char* ComponentDataType();
    
  // Transform data from transient form to persistent
  char* IORToLocalPersistentID( SALOMEDS::SObject_ptr theSObject,
			        const char*           IORString,
			        CORBA::Boolean        isMultiFile,
			        CORBA::Boolean        isASCII );
  // Transform data from persistent form to transient
  char* LocalPersistentIDToIOR( SALOMEDS::SObject_ptr theSObject,
			        const char*           aLocalPersistentID,
			        CORBA::Boolean        isMultiFile,
			        CORBA::Boolean        isASCII );

  // Returns true if object can be published in the study
  bool CanPublishInStudy( CORBA::Object_ptr theIOR );
  // Publish object in the study
  SALOMEDS::SObject_ptr PublishInStudy( SALOMEDS::Study_ptr   theStudy,
		                        SALOMEDS::SObject_ptr theSObject,
		                        CORBA::Object_ptr     theObject,
		                        const char*           theName ) 
    throw ( SALOME::SALOME_Exception );

  // Copy-paste methods - returns true if object can be copied to the clipboard
  CORBA::Boolean CanCopy( SALOMEDS::SObject_ptr theObject ) { return false; }
  // Copy-paste methods - copy object to the clipboard
  SALOMEDS::TMPFile* CopyFrom( SALOMEDS::SObject_ptr theObject, CORBA::Long& theObjectID ) { return false; }
  // Copy-paste methods - returns true if object can be pasted from the clipboard
  CORBA::Boolean CanPaste( const char* theComponentName, CORBA::Long theObjectID ) { return false; }
  // Copy-paste methods - paste object from the clipboard
  SALOMEDS::SObject_ptr PasteInto( const SALOMEDS::TMPFile& theStream,
				   CORBA::Long              theObjectID,
				   SALOMEDS::SObject_ptr    theObject ) {
    SALOMEDS::SObject_var aResultSO;
    return aResultSO._retn();
  }

  // *****************************************
  // Internal methods
  // *****************************************
public:
  // Get shape reader
  GEOM_Client* GetShapeReader();

  // Tags definition 
  static long GetHypothesisRootTag();
  static long GetAlgorithmsRootTag();
  static long GetRefOnShapeTag();
  static long GetRefOnAppliedHypothesisTag();
  static long GetRefOnAppliedAlgorithmsTag();
  static long GetSubMeshOnVertexTag();
  static long GetSubMeshOnEdgeTag();
  static long GetSubMeshOnFaceTag();
  static long GetSubMeshOnSolidTag();
  static long GetSubMeshOnCompoundTag();
  static long GetSubMeshOnWireTag();
  static long GetSubMeshOnShellTag();
  static long GetNodeGroupsTag();
  static long GetEdgeGroupsTag();
  static long GetFaceGroupsTag();
  static long GetVolumeGroupsTag();

  //  Get study context
  StudyContext* GetCurrentStudyContext();
  
private:
  // Create hypothesis of given type
  SMESH::SMESH_Hypothesis_ptr createHypothesis( const char* theHypName,
                                                const char* theLibName)
    throw ( SALOME::SALOME_Exception );
  
  // Create empty mesh on shape
  SMESH::SMESH_Mesh_ptr createMesh()
    throw ( SALOME::SALOME_Exception );

  static void loadGeomData( SALOMEDS::SComponent_ptr theCompRoot );
  
private:

  static CORBA::ORB_var          myOrb;         // ORB reference
  static PortableServer::POA_var myPoa;         // POA reference
  static SALOME_NamingService*   myNS;          // Naming Service
  static SALOME_LifeCycleCORBA*  myLCC;         // Life Cycle CORBA
  static SMESH_Gen_i*            mySMESHGen;    // Point to last created instance of the class
  ::SMESH_Gen               myGen;              // SMESH_Gen local implementation

  // hypotheses managing
  map<string, GenericHypothesisCreator_i*> myHypCreatorMap;

  map<int, StudyContext*>   myStudyContextMap;  // Map of study context objects

  GEOM_Client*              myShapeReader;      // Shape reader
  SALOMEDS::Study_var       myCurrentStudy;     // Current study
};

#endif
