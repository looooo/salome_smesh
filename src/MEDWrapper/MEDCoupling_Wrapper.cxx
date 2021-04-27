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
  _coords = DataArrayDouble::New();
  _coords->alloc(aNbElem,this->_space_dim);
  std::copy(aCoord->data(),aCoord->data()+aNbElem*this->_space_dim,_coords->getPointer());
}

void MCTWrapper::GetFamilyInfo(TInt theFamId, TFamilyInfo& theInfo)
{
  std::string aFamilyName(theInfo.myName.data());
  MED::TInt aFamilyId(theInfo.myId);
  std::string aGroupNames(theInfo.myGroupNames.data());
}
