// Copyright (C) 2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include "SMeshHelper.h"

#include "SMESH_Gen_i.hxx"
#include "SALOME_Container_i.hxx"

#include <cstring>

CORBA::ORB_var orb;

std::string BuildSMESHInstance()
{
    { int argc(0); orb = CORBA::ORB_init(argc,nullptr); }
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
    PortableServer::POAManager_var pman = poa->the_POAManager();
    CORBA::PolicyList policies;
    policies.length(0);
    PortableServer::POA_var poa2 = poa->create_POA("POA2",pman,policies);
    PortableServer::ObjectId_var conId;
    //
    {
        char *argv[4] = {"Container","FactoryServer","toto",nullptr};
        Engines_Container_i *cont = new Engines_Container_i(orb,poa2,"FactoryServer",2,argv,false,false);
        conId = poa2->activate_object(cont);
    }
    //
    pman->activate();
    //
    SMESH_Gen_i *servant = new SMESH_Gen_i(orb,poa2,const_cast<PortableServer::ObjectId*>(&conId.in()),"SMESH_inst_2","SMESH");
    PortableServer::ObjectId *zeId = servant->getId();
    CORBA::Object_var zeRef = poa2->id_to_reference(*zeId);
    CORBA::String_var ior = orb->object_to_string(zeRef);
    return std::string(ior.in());
}
