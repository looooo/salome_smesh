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

#ifndef _SMESH_PYTHONDUMP_HXX_
#define _SMESH_PYTHONDUMP_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include <TCollection_AsciiString.hxx>

namespace SMESH
{
  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     const char* theArg);

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     int theArg);

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     float theArg);

  class FilterLibrary_i;
  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     SMESH::FilterLibrary_i* theArg);

  class FilterManager_i;
  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     SMESH::FilterManager_i* theArg);

  class Filter_i;
  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     SMESH::Filter_i* theArg);

  class Functor_i;
  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     SMESH::Functor_i* theArg);

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     CORBA::Object_ptr theArg);

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     const SMESH::long_array& theArg);

  class TPythonDump
  {
    TCollection_AsciiString myString;
  public:
    virtual ~TPythonDump();
    
    template<class T>
    TCollection_AsciiString& 
    operator<<(T theArg){
      return myString<<theArg;
    }
  };
}


#endif
