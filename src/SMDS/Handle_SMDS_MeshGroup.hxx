// File generated by CPPExt (Transient)
//
//                     Copyright (C) 1991,1995 by
//  
//                      MATRA DATAVISION, FRANCE
//  
// This software is furnished in accordance with the terms and conditions
// of the contract and with the inclusion of the above copyright notice.
// This software or any other copy thereof may not be provided or otherwise
// be made available to any other person. No title to an ownership of the
// software is hereby transferred.
//  
// At the termination of the contract, the software and all copies of this
// software must be deleted.

#ifndef _Handle_SMDS_MeshGroup_HeaderFile
#define _Handle_SMDS_MeshGroup_HeaderFile

#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif
#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#ifndef _Handle_SMDS_MeshObject_HeaderFile
#include "Handle_SMDS_MeshObject.hxx"
#endif

class Standard_Transient;
class Handle_Standard_Type;
class Handle(SMDS_MeshObject);
class SMDS_MeshGroup;
Standard_EXPORT Handle_Standard_Type& STANDARD_TYPE(SMDS_MeshGroup);

class Handle(SMDS_MeshGroup) : public Handle(SMDS_MeshObject) {
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
//    inline void  operator delete(void *anAddress, size_t size) 
//      { 
//        if (anAddress) Standard::Free((Standard_Address&)anAddress,size); 
//      }
    Handle(SMDS_MeshGroup)():Handle(SMDS_MeshObject)() {} 
    Handle(SMDS_MeshGroup)(const Handle(SMDS_MeshGroup)& aHandle) : Handle(SMDS_MeshObject)(aHandle) 
     {
     }

    Handle(SMDS_MeshGroup)(const SMDS_MeshGroup* anItem) : Handle(SMDS_MeshObject)((SMDS_MeshObject *)anItem) 
     {
     }

    Handle(SMDS_MeshGroup)& operator=(const Handle(SMDS_MeshGroup)& aHandle)
     {
      Assign(aHandle.Access());
      return *this;
     }

    Handle(SMDS_MeshGroup)& operator=(const SMDS_MeshGroup* anItem)
     {
      Assign((Standard_Transient *)anItem);
      return *this;
     }

    SMDS_MeshGroup* operator->() 
     {
      return (SMDS_MeshGroup *)ControlAccess();
     }

    SMDS_MeshGroup* operator->() const 
     {
      return (SMDS_MeshGroup *)ControlAccess();
     }

   Standard_EXPORT ~Handle(SMDS_MeshGroup)();
 
   Standard_EXPORT static const Handle(SMDS_MeshGroup) DownCast(const Handle(Standard_Transient)& AnObject);
};
#endif
