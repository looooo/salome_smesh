//  SMESH SMESHClient : tool to update client mesh structure by mesh from server
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
//  File   : SMESH_Client.cxx
//  Author : Pavel TELKOV
//  Module : SMESH

#include "SMESH_Client.hxx"

#include <SALOMEconfig.h>
#include "OpUtil.hxx"
#include "utilities.h"

#include <SMESH_Mesh.hxx>

#include CORBA_SERVER_HEADER(SALOME_Exception)

#ifdef WNT
#include <process.h>
#else
#include <unistd.h>
#endif

#include <stdexcept>

#ifndef EXCEPTION
#define EXCEPTION(TYPE, MSG) {\
  std::ostringstream aStream;\
  aStream<<__FILE__<<"["<<__LINE__<<"]::"<<MSG;\
  throw TYPE(aStream.str());\
}
#endif

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

//=======================================================================
// function : Create()
// purpose  : Create in client not in a container
//=======================================================================
SMESH_Client::SMESH_Client()
{
  pid_client = 
#ifdef WNT
    (long)_getpid();
#else
    (long)getpid();
#endif
}

//=======================================================================
// function : Create()
// purpose  : 
//=======================================================================
SMESH_Client::SMESH_Client(Engines::Container_ptr client)
{
  pid_client = client->getPID();
}

namespace{

  inline const SMDS_MeshNode* FindNode(const SMDS_Mesh* theMesh, int theId){
    if(const SMDS_MeshNode* anElem = theMesh->FindNode(theId)) return anElem;
    EXCEPTION(runtime_error,"SMDS_Mesh::FindNode - cannot find a SMDS_MeshNode for ID = "<<theId);
  }


  inline const SMDS_MeshElement* FindElement(const SMDS_Mesh* theMesh, int theId){
    if(const SMDS_MeshElement* anElem = theMesh->FindElement(theId)) return anElem;
    EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot find a SMDS_MeshElement for ID = "<<theId);
  }


  inline void AddNodesWithID(SMDS_Mesh* theMesh, 
			     SMESH::log_array_var& theSeq,
			     CORBA::Long theId)
  {
    const SMESH::double_array& aCoords = theSeq[theId].coords;
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(3*aNbElems != aCoords.length())
      EXCEPTION(runtime_error,"AddNodesWithID - 3*aNbElems != aCoords.length()");
    for(CORBA::Long aCoordId = 0; anElemId < aNbElems; anElemId++, aCoordId+=3){
      SMDS_MeshElement* anElem = theMesh->AddNodeWithID(aCoords[aCoordId],
							aCoords[aCoordId+1],
							aCoords[aCoordId+2],
							anIndexes[anElemId]);
      if(!anElem)
	EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddNodeWithID for ID = "<<anElemId);
    }
  }


  inline void AddEdgesWithID(SMDS_Mesh* theMesh, 
			     SMESH::log_array_var& theSeq,
			     CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(3*aNbElems != anIndexes.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - 3*aNbElems != aCoords.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=3){
      SMDS_MeshElement* anElem = theMesh->AddEdgeWithID(anIndexes[anIndexId+1],
							anIndexes[anIndexId+2],
							anIndexes[anIndexId]);
      if(!anElem)
	EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddEdgeWithID for ID = "<<anElemId);
    }
  }


  inline void AddTriasWithID(SMDS_Mesh* theMesh, 
			     SMESH::log_array_var& theSeq,
			     CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(4*aNbElems != anIndexes.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - 4*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=4){
      SMDS_MeshElement* anElem = theMesh->AddFaceWithID(anIndexes[anIndexId+1],
							anIndexes[anIndexId+2],
							anIndexes[anIndexId+3],
							anIndexes[anIndexId]);
      if(!anElem)
	EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddFaceWithID for ID = "<<anElemId);
    }
  }


  inline void AddQuadsWithID(SMDS_Mesh* theMesh, 
			     SMESH::log_array_var theSeq,
			     CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(5*aNbElems != anIndexes.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - 4*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=5){
      SMDS_MeshElement* anElem = theMesh->AddFaceWithID(anIndexes[anIndexId+1],
							anIndexes[anIndexId+2],
							anIndexes[anIndexId+3],
							anIndexes[anIndexId+4],
							anIndexes[anIndexId]);
      if(!anElem)
	EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddFaceWithID for ID = "<<anElemId);
    }
  }


  inline void AddPolygonsWithID(SMDS_Mesh* theMesh, 
                                SMESH::log_array_var& theSeq,
                                CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anIndexId = 0, aNbElems = theSeq[theId].number;

    for (CORBA::Long anElemId = 0; anElemId < aNbElems; anElemId++) {
      int aFaceId = anIndexes[anIndexId++];

      int aNbNodes = anIndexes[anIndexId++];
      std::vector<int> nodes_ids (aNbNodes);
      for (int i = 0; i < aNbNodes; i++) {
        nodes_ids[i] = anIndexes[anIndexId++];
      }

      SMDS_MeshElement* anElem = theMesh->AddPolygonalFaceWithID(nodes_ids, aFaceId);
      if (!anElem)
	EXCEPTION(runtime_error, "SMDS_Mesh::FindElement - cannot AddPolygonalFaceWithID for ID = "
                  << anElemId);
    }
  }


  inline void AddTetrasWithID(SMDS_Mesh* theMesh, 
			      SMESH::log_array_var& theSeq,
			      CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(5*aNbElems != anIndexes.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - 5*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=5){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
							  anIndexes[anIndexId+2],
							  anIndexes[anIndexId+3],
							  anIndexes[anIndexId+4],
							  anIndexes[anIndexId]);
      if(!anElem)
	EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  inline void AddPiramidsWithID(SMDS_Mesh* theMesh, 
				SMESH::log_array_var& theSeq,
				CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(6*aNbElems != anIndexes.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - 6*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=6){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
							  anIndexes[anIndexId+2],
							  anIndexes[anIndexId+3],
							  anIndexes[anIndexId+4],
							  anIndexes[anIndexId+5],
							  anIndexes[anIndexId]);
      if(!anElem)
	EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  inline void AddPrismsWithID(SMDS_Mesh* theMesh, 
			      SMESH::log_array_var& theSeq,
			      CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(7*aNbElems != anIndexes.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - 7*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=7){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
							  anIndexes[anIndexId+2],
							  anIndexes[anIndexId+3],
							  anIndexes[anIndexId+4],
							  anIndexes[anIndexId+5],
							  anIndexes[anIndexId+6],
							  anIndexes[anIndexId]);
      if(!anElem)
	EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  inline void AddHexasWithID(SMDS_Mesh* theMesh, 
			     SMESH::log_array_var& theSeq,
			     CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(9*aNbElems != anIndexes.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - 9*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=9){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
							  anIndexes[anIndexId+2],
							  anIndexes[anIndexId+3],
							  anIndexes[anIndexId+4],
							  anIndexes[anIndexId+5],
							  anIndexes[anIndexId+6],
							  anIndexes[anIndexId+7],
							  anIndexes[anIndexId+8],
							  anIndexes[anIndexId]);
      if(!anElem)
	EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  inline void AddPolyhedronsWithID (SMDS_Mesh* theMesh, 
                                    SMESH::log_array_var& theSeq,
                                    CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anIndexId = 0, aNbElems = theSeq[theId].number;

    for (CORBA::Long anElemId = 0; anElemId < aNbElems; anElemId++) {
      int aFaceId = anIndexes[anIndexId++];

      int aNbNodes = anIndexes[anIndexId++];
      std::vector<int> nodes_ids (aNbNodes);
      for (int i = 0; i < aNbNodes; i++) {
        nodes_ids[i] = anIndexes[anIndexId++];
      }

      int aNbFaces = anIndexes[anIndexId++];
      std::vector<int> quantities (aNbFaces);
      for (int i = 0; i < aNbFaces; i++) {
        quantities[i] = anIndexes[anIndexId++];
      }

      SMDS_MeshElement* anElem =
        theMesh->AddPolyhedralVolumeWithID(nodes_ids, quantities, aFaceId);
      if (!anElem)
	EXCEPTION(runtime_error, "SMDS_Mesh::FindElement - cannot AddPolyhedralVolumeWithID for ID = "
                  << anElemId);
    }
  }


  inline void ChangePolyhedronNodes (SMDS_Mesh* theMesh, 
                                     SMESH::log_array_var& theSeq,
                                     CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long iind = 0, aNbElems = theSeq[theId].number;

    for (CORBA::Long anElemId = 0; anElemId < aNbElems; anElemId++)
    {
      // find element
      const SMDS_MeshElement* elem = FindElement(theMesh, anIndexes[iind++]);
      // nb nodes
      int nbNodes = anIndexes[iind++];
      // nodes
      std::vector<const SMDS_MeshNode*> aNodes (nbNodes);
      for (int iNode = 0; iNode < nbNodes; iNode++) {
        aNodes[iNode] = FindNode(theMesh, anIndexes[iind++]);
      }
      // nb faces
      int nbFaces = anIndexes[iind++];
      // quantities
      std::vector<int> quantities (nbFaces);
      for (int iFace = 0; iFace < nbFaces; iFace++) {
        quantities[iFace] = anIndexes[iind++];
      }
      // change
      theMesh->ChangePolyhedronNodes(elem, aNodes, quantities);
    }
  }
}

//=================================================================================
// function : Update
// purpose  : Update mesh
//=================================================================================
SMDS_Mesh* SMESH_Client::Update( SMESH::SMESH_Gen_ptr smesh,
                                 SMESH::SMESH_Mesh_ptr theMeshServer, 
                                 SMDS_Mesh* theMesh,
                                 int theIsClear,
				 int& theIsUpdated )
{
  SMDS_Mesh* meshPtr = theMesh;
  theIsUpdated = 0;
 
  SMESH::log_array_var aSeq = theMeshServer->GetLog( theIsClear );
  CORBA::Long aLength = aSeq->length();
  
  if( MYDEBUG )
    MESSAGE( "Update: length of the script is "<<aLength );
  
  if( !aLength )
    return meshPtr;  // nothing to update

  string hst_client = GetHostname();
  Engines::Container_var ctn_server = smesh->GetContainerRef();
  long pid_server = ctn_server->getPID();
  if ( (pid_client==pid_server) && (strcmp(hst_client.c_str(), ctn_server->getHostName())==0) )
  {
    if ( MYDEBUG )
      MESSAGE("Info: The same process, update mesh by pointer ");
    // just set client mesh pointer to server mesh pointer
    meshPtr = ((SMESH_Mesh*)theMeshServer->GetMeshPtr())->GetMeshDS();
    if ( meshPtr )
      theIsUpdated = 1;
  }
  else
  {
    // update client mesh structure by logged changes commands
    try
    {
      for ( CORBA::Long anId = 0; anId < aLength; anId++)
      {
	const SMESH::double_array& aCoords = aSeq[anId].coords;
	const SMESH::long_array& anIndexes = aSeq[anId].indexes;
	CORBA::Long anElemId = 0, aNbElems = aSeq[anId].number;
	CORBA::Long aCommand = aSeq[anId].commandType;

	switch(aCommand)
        {
	case SMESH::ADD_NODE       : AddNodesWithID      ( meshPtr, aSeq, anId ); break;
        case SMESH::ADD_EDGE       : AddEdgesWithID      ( meshPtr, aSeq, anId ); break;
        case SMESH::ADD_TRIANGLE   : AddTriasWithID      ( meshPtr, aSeq, anId ); break;
        case SMESH::ADD_QUADRANGLE : AddQuadsWithID      ( meshPtr, aSeq, anId ); break;
        case SMESH::ADD_POLYGON    : AddPolygonsWithID   ( meshPtr, aSeq, anId ); break;
        case SMESH::ADD_TETRAHEDRON: AddTetrasWithID     ( meshPtr, aSeq, anId ); break;
        case SMESH::ADD_PYRAMID    : AddPiramidsWithID   ( meshPtr, aSeq, anId ); break;
        case SMESH::ADD_PRISM      : AddPrismsWithID     ( meshPtr, aSeq, anId ); break;
        case SMESH::ADD_HEXAHEDRON : AddHexasWithID      ( meshPtr, aSeq, anId ); break;
        case SMESH::ADD_POLYHEDRON : AddPolyhedronsWithID( meshPtr, aSeq, anId ); break;

        case SMESH::REMOVE_NODE:
          for( ; anElemId < aNbElems; anElemId++ )
            meshPtr->RemoveNode( FindNode( meshPtr, anIndexes[anElemId] ) );
        break;
        
        case SMESH::REMOVE_ELEMENT:
          for( ; anElemId < aNbElems; anElemId++ )
            meshPtr->RemoveElement( FindElement( meshPtr, anIndexes[anElemId] ) );
        break;

        case SMESH::MOVE_NODE:
          for(CORBA::Long aCoordId=0; anElemId < aNbElems; anElemId++, aCoordId+=3)
          {
            SMDS_MeshNode* node =
              const_cast<SMDS_MeshNode*>( FindNode( meshPtr, anIndexes[anElemId] ));
            node->setXYZ( aCoords[aCoordId], aCoords[aCoordId+1], aCoords[aCoordId+2] );
          }
        break;

        case SMESH::CHANGE_ELEMENT_NODES:
          for ( CORBA::Long i = 0; anElemId < aNbElems; anElemId++ )
          {
            // find element
            const SMDS_MeshElement* elem = FindElement( meshPtr, anIndexes[i++] );
            // nb nodes
            int nbNodes = anIndexes[i++];
            // nodes
            //ASSERT( nbNodes < 9 );
            const SMDS_MeshNode* aNodes[ nbNodes ];
            for ( int iNode = 0; iNode < nbNodes; iNode++ )
              aNodes[ iNode ] = FindNode( meshPtr, anIndexes[i++] );
            // change
            meshPtr->ChangeElementNodes( elem, aNodes, nbNodes );
          }
          break;

        case SMESH::CHANGE_POLYHEDRON_NODES:
          ChangePolyhedronNodes(meshPtr, aSeq, anId);
          break;
        case SMESH::RENUMBER:
          for(CORBA::Long i=0; anElemId < aNbElems; anElemId++, i+=3)
          {
            meshPtr->Renumber( anIndexes[i], anIndexes[i+1], anIndexes[i+2] );
          }
          break;
          
        default:;
	}
      }
      theIsUpdated = 1;
    }
    catch ( SALOME::SALOME_Exception& exc )
    {
      INFOS("Following exception was cought:\n\t"<<exc.details.text);
    }
    catch( const std::exception& exc)
    {
      INFOS("Following exception was cought:\n\t"<<exc.what());
    }
    catch(...)
    {
      INFOS("Unknown exception was cought !!!");
    }
  } // end of else (update mesh by log script
  
  if ( MYDEBUG && meshPtr )
  {
    MESSAGE("Update - meshPtr->NbNodes() = "<<meshPtr->NbNodes());
    MESSAGE("Update - meshPtr->NbEdges() = "<<meshPtr->NbEdges());
    MESSAGE("Update - meshPtr->NbFaces() = "<<meshPtr->NbFaces());
    MESSAGE("Update - meshPtr->NbVolumes() = "<<meshPtr->NbVolumes());
  }

  return meshPtr;
}
