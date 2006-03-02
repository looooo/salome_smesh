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
//  File   : SMESH_Client.hxx
//  Author : Pavel TELKOV
//  Module : SMESH

#ifndef _SMESH_Client_HeaderFile
#define _SMESH_Client_HeaderFile

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
//#include CORBA_SERVER_HEADER(SALOME_Container)

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#if defined WNT && defined WIN32 && defined SALOME_WNT_EXPORTS
#define SMESHCLIENT_WNT_EXPORT __declspec( dllexport )
#else
#define SMESHCLIENT_WNT_EXPORT
#endif

class SMDS_Mesh;

//=====================================================================
// SMESH_Client : class definition
//=====================================================================
class SMESHCLIENT_WNT_EXPORT SMESH_Client  {

public:
  
  inline void* operator new(size_t,void* anAddress) 
  {
    return anAddress;
  }
  inline void* operator new(size_t size) 
  { 
    return Standard::Allocate(size); 
  }
  inline void  operator delete(void *anAddress) 
  { 
    if (anAddress) Standard::Free((Standard_Address&)anAddress); 
  }
  // Methods PUBLIC
  // 
  SMESH_Client();
  SMESH_Client(Engines::Container_ptr client);
  SMDS_Mesh* Update( SMESH::SMESH_Gen_ptr smesh,
                     SMESH::SMESH_Mesh_ptr theMeshServer, 
                     SMDS_Mesh* theMesh, 
                     int theIsClear,
		     int& theIsUpdated );

private: 
  // Fields PRIVATE
  //
  long  pid_client;
};


#endif
