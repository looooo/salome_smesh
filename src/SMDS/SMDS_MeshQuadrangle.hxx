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
//
#ifndef _SMDS_MeshQuadrangle_HeaderFile
#define _SMDS_MeshQuadrangle_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_MeshQuadrangle_HeaderFile
#include "Handle_SMDS_MeshQuadrangle.hxx"
#endif

#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _SMDS_MeshFace_HeaderFile
#include "SMDS_MeshFace.hxx"
#endif
#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif


class SMDS_MeshQuadrangle : public SMDS_MeshFace {

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
 // Methods PUBLIC
 // 
Standard_EXPORT SMDS_MeshQuadrangle(const Standard_Integer ID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4);
Standard_EXPORT inline   void ComputeKey() ;
Standard_EXPORT inline   void GetEdgeDefinedByNodes(const Standard_Integer rank,Standard_Integer& idnode1,Standard_Integer& idnode2) const;
Standard_EXPORT inline   Standard_Address GetConnections() const;
Standard_EXPORT inline   Standard_Integer GetConnection(const Standard_Integer rank) const;
Standard_EXPORT ~SMDS_MeshQuadrangle();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_MeshQuadrangle_Type_();
 Standard_EXPORT const Handle(Standard_Type)& DynamicType() const;
 Standard_EXPORT Standard_Boolean	       IsKind(const Handle(Standard_Type)&) const;

protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 
Standard_EXPORT   void SetConnections(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) ;


 // Fields PRIVATE
 //
Standard_Integer myNodes[4];


};


#include "SMDS_MeshQuadrangle.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
