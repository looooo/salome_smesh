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

#ifndef _Handle_SMDSControl_MeshBoundary_HeaderFile
#define _Handle_SMDSControl_MeshBoundary_HeaderFile

#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif
#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#ifndef _Handle_MMgt_TShared_HeaderFile
#include <Handle_MMgt_TShared.hxx>
#endif

class Standard_Transient;
class Handle_Standard_Type;
class Handle(MMgt_TShared);
class SMDSControl_MeshBoundary;
Standard_EXPORT Handle_Standard_Type& STANDARD_TYPE(SMDSControl_MeshBoundary);

class Handle(SMDSControl_MeshBoundary) : public Handle(MMgt_TShared) {
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
    Handle(SMDSControl_MeshBoundary)():Handle(MMgt_TShared)() {} 
    Handle(SMDSControl_MeshBoundary)(const Handle(SMDSControl_MeshBoundary)& aHandle) : Handle(MMgt_TShared)(aHandle) 
     {
     }

    Handle(SMDSControl_MeshBoundary)(const SMDSControl_MeshBoundary* anItem) : Handle(MMgt_TShared)((MMgt_TShared *)anItem) 
     {
     }

    Handle(SMDSControl_MeshBoundary)& operator=(const Handle(SMDSControl_MeshBoundary)& aHandle)
     {
      Assign(aHandle.Access());
      return *this;
     }

    Handle(SMDSControl_MeshBoundary)& operator=(const SMDSControl_MeshBoundary* anItem)
     {
      Assign((Standard_Transient *)anItem);
      return *this;
     }

    SMDSControl_MeshBoundary* operator->() 
     {
      return (SMDSControl_MeshBoundary *)ControlAccess();
     }

    SMDSControl_MeshBoundary* operator->() const 
     {
      return (SMDSControl_MeshBoundary *)ControlAccess();
     }

   Standard_EXPORT ~Handle(SMDSControl_MeshBoundary)();
 
   Standard_EXPORT static const Handle(SMDSControl_MeshBoundary) DownCast(const Handle(Standard_Transient)& AnObject);
};
#endif
