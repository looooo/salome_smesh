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
#ifndef _SMDS_Position_HeaderFile
#define _SMDS_Position_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_Position_HeaderFile
#include "Handle_SMDS_Position.hxx"
#endif

#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _SMDS_TypeOfPosition_HeaderFile
#include "SMDS_TypeOfPosition.hxx"
#endif
#ifndef _MMgt_TShared_HeaderFile
#include <MMgt_TShared.hxx>
#endif
class gp_Pnt;


class SMDS_Position : public MMgt_TShared {

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
Standard_EXPORT virtual  gp_Pnt Coords() const = 0;
Standard_EXPORT inline   SMDS_TypeOfPosition GetTypeOfPosition() const;
Standard_EXPORT inline   void SetShapeId(const Standard_Integer aShapeId) ;
Standard_EXPORT inline   Standard_Integer GetShapeId() const;
Standard_EXPORT ~SMDS_Position();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_Position_Type_();
 Standard_EXPORT const Handle(Standard_Type)& DynamicType() const;
 Standard_EXPORT Standard_Boolean	       IsKind(const Handle(Standard_Type)&) const;

protected:

 // Methods PROTECTED
 // 
Standard_EXPORT SMDS_Position(const Standard_Integer aShapeId,const SMDS_TypeOfPosition aType = SMDS_TOP_UNSPEC);


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 


 // Fields PRIVATE
 //
Standard_Integer myShapeId;
SMDS_TypeOfPosition myType;


};


#include "SMDS_Position.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
