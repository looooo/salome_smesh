using namespace std;
// File generated by CPPExt (Transient)
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
#include "SMDS_DataMapNodeOfDataMapOfPntInteger.hxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _gp_Pnt_HeaderFile
#include <gp_Pnt.hxx>
#endif
#ifndef _SMDS_PntHasher_HeaderFile
#include "SMDS_PntHasher.hxx"
#endif
#ifndef _SMDS_DataMapOfPntInteger_HeaderFile
#include "SMDS_DataMapOfPntInteger.hxx"
#endif
#ifndef _SMDS_DataMapIteratorOfDataMapOfPntInteger_HeaderFile
#include "SMDS_DataMapIteratorOfDataMapOfPntInteger.hxx"
#endif
SMDS_DataMapNodeOfDataMapOfPntInteger::~SMDS_DataMapNodeOfDataMapOfPntInteger() {}
 


Standard_EXPORT Handle_Standard_Type& SMDS_DataMapNodeOfDataMapOfPntInteger_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_MapNode);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TCollection_MapNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMDS_DataMapNodeOfDataMapOfPntInteger",
			                                 sizeof(SMDS_DataMapNodeOfDataMapOfPntInteger),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMDS_DataMapNodeOfDataMapOfPntInteger) Handle(SMDS_DataMapNodeOfDataMapOfPntInteger)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMDS_DataMapNodeOfDataMapOfPntInteger) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMDS_DataMapNodeOfDataMapOfPntInteger))) {
       _anOtherObject = Handle(SMDS_DataMapNodeOfDataMapOfPntInteger)((Handle(SMDS_DataMapNodeOfDataMapOfPntInteger)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMDS_DataMapNodeOfDataMapOfPntInteger::DynamicType() const 
{ 
  return STANDARD_TYPE(SMDS_DataMapNodeOfDataMapOfPntInteger) ; 
}
Standard_Boolean SMDS_DataMapNodeOfDataMapOfPntInteger::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMDS_DataMapNodeOfDataMapOfPntInteger) == AType || TCollection_MapNode::IsKind(AType)); 
}
Handle_SMDS_DataMapNodeOfDataMapOfPntInteger::~Handle_SMDS_DataMapNodeOfDataMapOfPntInteger() {}
#define TheKey gp_Pnt
#define TheKey_hxx <gp_Pnt.hxx>
#define TheItem Standard_Integer
#define TheItem_hxx <Standard_Integer.hxx>
#define Hasher SMDS_PntHasher
#define Hasher_hxx <SMDS_PntHasher.hxx>
#define TCollection_DataMapNode SMDS_DataMapNodeOfDataMapOfPntInteger
#define TCollection_DataMapNode_hxx <SMDS_DataMapNodeOfDataMapOfPntInteger.hxx>
#define TCollection_DataMapIterator SMDS_DataMapIteratorOfDataMapOfPntInteger
#define TCollection_DataMapIterator_hxx <SMDS_DataMapIteratorOfDataMapOfPntInteger.hxx>
#define Handle_TCollection_DataMapNode Handle_SMDS_DataMapNodeOfDataMapOfPntInteger
#define TCollection_DataMapNode_Type_() SMDS_DataMapNodeOfDataMapOfPntInteger_Type_()
#define TCollection_DataMap SMDS_DataMapOfPntInteger
#define TCollection_DataMap_hxx <SMDS_DataMapOfPntInteger.hxx>
#include <TCollection_DataMapNode.gxx>

