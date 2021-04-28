// Copyright (C) 2021  CEA/DEN, EDF R&D
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

#include "MEDCoupling_Wrapper.hxx"
#include "MED_TStructures.hxx"

using namespace MED;
using namespace MEDCoupling;

//Copie directe
PMeshInfo MCTWrapper::CrMeshInfo(TInt theDim,
            TInt theSpaceDim,
            const std::string& theValue,
            EMaillage theType,
            const std::string& theDesc)
{
  return PMeshInfo(new TTMeshInfo(theDim, theSpaceDim, theValue, theType, theDesc));
}

void MCTWrapper::SetMeshInfo(const TMeshInfo& theInfo)
{
  _mesh_name = theInfo.GetName();
  _mesh_dim = theInfo.GetDim();
  _space_dim = theInfo.GetSpaceDim();
}

//Copie directe
PNodeInfo MCTWrapper::CrNodeInfo(const PMeshInfo& theMeshInfo,
              TInt theNbElem,
              EModeSwitch theMode,
              ERepere theSystem,
              EBooleen theIsElemNum,
              EBooleen theIsElemNames)
{
  return PNodeInfo(new TTNodeInfo
                    (theMeshInfo,
                    theNbElem,
                    theMode,
                    theSystem,
                    theIsElemNum,
                    theIsElemNames));
}

void MCTWrapper::SetNodeInfo(const TNodeInfo& theInfo)
{
  MED::TNodeInfo& anInfo = const_cast<MED::TNodeInfo&>(theInfo);
  MED::PNodeCoord aCoord(anInfo.myCoord);
  MED::TInt aNbElem(anInfo.myNbElem);
  std::string aCoordNames(anInfo.myCoordNames.data());
  std::string aCoordUnits(anInfo.myCoordUnits.data());
  std::vector<std::string> comps( DataArray::SplitStringInChuncks(aCoordNames,MED_SNAME_SIZE) );
  std::vector<std::string> units( DataArray::SplitStringInChuncks(aCoordUnits,MED_SNAME_SIZE) );
  std::size_t nbComps( comps.size() );
  if( nbComps != units.size() )
    THROW_IK_EXCEPTION("MCTWrapper::SetNodeInfo : mismatch length " << nbComps << " != " << units.size() << " !");
  std::vector<std::string> compUnits(nbComps);
  for( std::size_t i = 0 ; i < nbComps ; ++i )
  {
    compUnits[i] = DataArray::BuildInfoFromVarAndUnit(comps[i],units[i]) ;
  }
  _coords = DataArrayDouble::New();
  _coords->alloc(aNbElem,this->_space_dim);
  std::copy(aCoord->data(),aCoord->data()+aNbElem*this->_space_dim,_coords->getPointer());
  _coords->setInfoOnComponents(compUnits);
}

void MCTWrapper::GetFamilyInfo(TInt theFamId, TFamilyInfo& theInfo)
{
  std::string aFamilyName(theInfo.myName.data());
  MED::TInt aFamilyId(theInfo.myId);
  std::string aGroupNames(theInfo.myGroupNames.data());
}

//copie
  PFamilyInfo
  MCTWrapper::CrFamilyInfo(const PMeshInfo& theMeshInfo,
                 const std::string& theValue,
                 TInt theId,
                 const MED::TStringSet& theGroupNames,
                 const MED::TStringVector& theAttrDescs,
                 const MED::TIntVector& theAttrIds,
                 const MED::TIntVector& theAttrVals)
  {
    return PFamilyInfo(new TTFamilyInfo
                       (theMeshInfo,
                        theValue,
                        theId,
                        theGroupNames,
                        theAttrDescs,
                        theAttrIds,
                        theAttrVals));
  }

  //copie
  PPolygoneInfo
  MCTWrapper::CrPolygoneInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   TInt theNbElem,
                   TInt theConnSize,
                   EConnectivite theConnMode,
                   EBooleen theIsElemNum,
                   EBooleen theIsElemNames)
  {
    return PPolygoneInfo(new TTPolygoneInfo
                         (theMeshInfo,
                          theEntity,
                          theGeom,
                          theNbElem,
                          theConnSize,
                          theConnMode,
                          theIsElemNum,
                          theIsElemNames));
  }

  //copie
  PPolygoneInfo
  MCTWrapper
  ::CrPolygoneInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   const TIntVector& theIndexes,
                   const TIntVector& theConnectivities,
                   EConnectivite theConnMode,
                   const TIntVector& theFamilyNums,
                   const TIntVector& theElemNums,
                   const TStringVector& theElemNames)
  {
    return PPolygoneInfo(new TTPolygoneInfo
                         (theMeshInfo,
                          theEntity,
                          theGeom,
                          theIndexes,
                          theConnectivities,
                          theConnMode,
                          theFamilyNums,
                          theElemNums,
                          theElemNames));
  }

  //copie
  PPolyedreInfo
  MCTWrapper
  ::CrPolyedreInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   TInt theNbElem,
                   TInt theNbFaces,
                   TInt theConnSize,
                   EConnectivite theConnMode,
                   EBooleen theIsElemNum,
                   EBooleen theIsElemNames)
  {
    return PPolyedreInfo(new TTPolyedreInfo
                         (theMeshInfo,
                          theEntity,
                          theGeom,
                          theNbElem,
                          theNbFaces,
                          theConnSize,
                          theConnMode,
                          theIsElemNum,
                          theIsElemNames));
  }

//copie
    PBallInfo
  MCTWrapper
  ::CrBallInfo(const PMeshInfo& theMeshInfo,
               TInt theNbBalls,
               EBooleen theIsElemNum)
  {
    return PBallInfo(new TTBallInfo(theMeshInfo, theNbBalls, theIsElemNum));
  }

    PCellInfo
  MCTWrapper
  ::CrCellInfo(const PMeshInfo& theMeshInfo,
               EEntiteMaillage theEntity,
               EGeometrieElement theGeom,
               TInt theNbElem,
               EConnectivite theConnMode,
               EBooleen theIsElemNum,
               EBooleen theIsElemNames,
               EModeSwitch theMode)
  {
    return PCellInfo(new TTCellInfo
                     (theMeshInfo,
                      theEntity,
                      theGeom,
                      theNbElem,
                      theConnMode,
                      theIsElemNum,
                      theIsElemNames,
                      theMode));
  }
