// File generated by CPPExt (Value)
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

#ifndef _SMESHDS_ListOfPtrHypothesis_HeaderFile
#define _SMESHDS_ListOfPtrHypothesis_HeaderFile

#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif
#ifndef _SMESHDS_PtrHypothesis_HeaderFile
#include "SMESHDS_PtrHypothesis.hxx"
#endif
#ifndef _Handle_SMESHDS_ListNodeOfListOfPtrHypothesis_HeaderFile
#include "Handle_SMESHDS_ListNodeOfListOfPtrHypothesis.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_NoSuchObject;
class SMESHDS_ListIteratorOfListOfPtrHypothesis;
class SMESHDS_ListNodeOfListOfPtrHypothesis;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMESHDS_ListOfPtrHypothesis  {

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
Standard_EXPORT SMESHDS_ListOfPtrHypothesis();
Standard_EXPORT   void Assign(const SMESHDS_ListOfPtrHypothesis& Other) ;
  void operator=(const SMESHDS_ListOfPtrHypothesis& Other) 
{
  Assign(Other);
}

Standard_EXPORT   Standard_Integer Extent() const;
Standard_EXPORT   void Clear() ;
~SMESHDS_ListOfPtrHypothesis()
{
  Clear();
}

Standard_EXPORT inline   Standard_Boolean IsEmpty() const;
Standard_EXPORT   void Prepend(const SMESHDS_PtrHypothesis& I) ;
Standard_EXPORT   void Prepend(SMESHDS_ListOfPtrHypothesis& Other) ;
Standard_EXPORT   void Append(const SMESHDS_PtrHypothesis& I) ;
Standard_EXPORT   void Append(SMESHDS_ListOfPtrHypothesis& Other) ;
Standard_EXPORT   SMESHDS_PtrHypothesis& First() const;
Standard_EXPORT   SMESHDS_PtrHypothesis& Last() const;
Standard_EXPORT   void RemoveFirst() ;
Standard_EXPORT   void Remove(SMESHDS_ListIteratorOfListOfPtrHypothesis& It) ;
Standard_EXPORT   void InsertBefore(const SMESHDS_PtrHypothesis& I,SMESHDS_ListIteratorOfListOfPtrHypothesis& It) ;
Standard_EXPORT   void InsertBefore(SMESHDS_ListOfPtrHypothesis& Other,SMESHDS_ListIteratorOfListOfPtrHypothesis& It) ;
Standard_EXPORT   void InsertAfter(const SMESHDS_PtrHypothesis& I,SMESHDS_ListIteratorOfListOfPtrHypothesis& It) ;
Standard_EXPORT   void InsertAfter(SMESHDS_ListOfPtrHypothesis& Other,SMESHDS_ListIteratorOfListOfPtrHypothesis& It) ;


friend class SMESHDS_ListIteratorOfListOfPtrHypothesis;



protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 
Standard_EXPORT SMESHDS_ListOfPtrHypothesis(const SMESHDS_ListOfPtrHypothesis& Other);


 // Fields PRIVATE
 //
Standard_Address myFirst;
Standard_Address myLast;


};

#define Item SMESHDS_PtrHypothesis
#define Item_hxx <SMESHDS_PtrHypothesis.hxx>
#define TCollection_ListNode SMESHDS_ListNodeOfListOfPtrHypothesis
#define TCollection_ListNode_hxx <SMESHDS_ListNodeOfListOfPtrHypothesis.hxx>
#define TCollection_ListIterator SMESHDS_ListIteratorOfListOfPtrHypothesis
#define TCollection_ListIterator_hxx <SMESHDS_ListIteratorOfListOfPtrHypothesis.hxx>
#define Handle_TCollection_ListNode Handle_SMESHDS_ListNodeOfListOfPtrHypothesis
#define TCollection_ListNode_Type_() SMESHDS_ListNodeOfListOfPtrHypothesis_Type_()
#define TCollection_List SMESHDS_ListOfPtrHypothesis
#define TCollection_List_hxx <SMESHDS_ListOfPtrHypothesis.hxx>

#include <TCollection_List.lxx>

#undef Item
#undef Item_hxx
#undef TCollection_ListNode
#undef TCollection_ListNode_hxx
#undef TCollection_ListIterator
#undef TCollection_ListIterator_hxx
#undef Handle_TCollection_ListNode
#undef TCollection_ListNode_Type_
#undef TCollection_List
#undef TCollection_List_hxx


// other inline functions and methods (like "C++: function call" methods)
//


#endif
