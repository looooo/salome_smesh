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
#include "SMDS_MeshTetrahedron.jxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

SMDS_MeshTetrahedron::~SMDS_MeshTetrahedron() {}
 


Standard_EXPORT Handle_Standard_Type& SMDS_MeshTetrahedron_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(SMDS_MeshVolume);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(SMDS_MeshVolume);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(SMDS_MeshElement);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(SMDS_MeshElement);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(SMDS_MeshObject);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(SMDS_MeshObject);
  static Handle_Standard_Type aType4 = STANDARD_TYPE(MMgt_TShared);
  if ( aType4.IsNull()) aType4 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType5 = STANDARD_TYPE(Standard_Transient);
  if ( aType5.IsNull()) aType5 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,aType4,aType5,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMDS_MeshTetrahedron",
			                                 sizeof(SMDS_MeshTetrahedron),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMDS_MeshTetrahedron) Handle(SMDS_MeshTetrahedron)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMDS_MeshTetrahedron) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMDS_MeshTetrahedron))) {
       _anOtherObject = Handle(SMDS_MeshTetrahedron)((Handle(SMDS_MeshTetrahedron)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMDS_MeshTetrahedron::DynamicType() const 
{ 
  return STANDARD_TYPE(SMDS_MeshTetrahedron) ; 
}
Standard_Boolean SMDS_MeshTetrahedron::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMDS_MeshTetrahedron) == AType || SMDS_MeshVolume::IsKind(AType)); 
}
Handle_SMDS_MeshTetrahedron::~Handle_SMDS_MeshTetrahedron() {}

