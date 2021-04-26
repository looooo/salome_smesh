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

#pragma once

#include "MED_Wrapper.hxx"

#include "med.h"

#include <memory>

namespace MED
{
  class MEDWRAPPER_EXPORT MCTWrapper
  {
    public:
      //! Create a MEDWrapper MED Mesh representation
    PMeshInfo
    CrMeshInfo(TInt theDim = 0,
               TInt theSpaceDim = 0,
               const std::string& theValue = "",
               EMaillage theType = eNON_STRUCTURE,
               const std::string& theDesc = "");
    //! Write the MEDWrapper MED Mesh representation into the MED file
    void
    SetMeshInfo(const TMeshInfo& theInfo,
                TErr* theErr = NULL);
    //! Write a MEDWrapper MED Family representation into the MED file
    void
    SetFamilyInfo(const TFamilyInfo& theInfo,
                  TErr* theErr = NULL);
    //! Create a MEDWrapper MED Nodes representation
    PNodeInfo
    CrNodeInfo(const PMeshInfo& theMeshInfo,
               TInt theNbElem,
               EModeSwitch theMode = eFULL_INTERLACE,
               ERepere theSystem = eCART,
               EBooleen theIsElemNum = eVRAI,
               EBooleen theIsElemNames = eFAUX);
    //! Write the MEDWrapper MED Nodes representation into the MED file
    void
    SetNodeInfo(const TNodeInfo& theInfo,
                TErr* theErr = NULL);
                
    //! Read a MEDWrapper MED Family representation by its numbers
    void
    GetFamilyInfo(TInt theFamId,
                  TFamilyInfo& theInfo,
                  TErr* theErr = NULL);
                  
    //! Create a MEDWrapper MED Family representation
    PFamilyInfo
    CrFamilyInfo(const PMeshInfo& theMeshInfo,
                 const std::string& theValue,
                 TInt theId,
                 const TStringSet& theGroupNames,
                 const TStringVector& theAttrDescs = TStringVector(),
                 const TIntVector& theAttrIds = TIntVector(),
                 const TIntVector& theAttrVals = TIntVector());
    
    //! Create a MEDWrapper MED Polygones representation
    virtual
    PPolygoneInfo
    CrPolygoneInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   TInt theNbElem,
                   TInt theConnSize,
                   EConnectivite theConnMode = eNOD,
                   EBooleen theIsElemNum = eVRAI,
                   EBooleen theIsElemNames = eVRAI);
    
    //! Create a MEDWrapper MED Polygones representation
    virtual
    PPolygoneInfo
    CrPolygoneInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   const TIntVector& theIndexes,
                   const TIntVector& theConnectivities,
                   EConnectivite theConnMode = eNOD,
                   const TIntVector& theFamilyNums = TIntVector(),
                   const TIntVector& theElemNums = TIntVector(),
                   const TStringVector& theElemNames = TStringVector());
                   
                   
    //! Write a MEDWrapper MED Polygones representation into the MED file
    virtual
    void
    SetPolygoneInfo(const TPolygoneInfo& theInfo,
                    TErr* theErr = NULL);
    

        //! Create a MEDWrapper MED Polyedres representation
    virtual
    PPolyedreInfo
    CrPolyedreInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   TInt theNbElem,
                   TInt theNbFaces,
                   TInt theConnSize,
                   EConnectivite theConnMode = eNOD,
                   EBooleen theIsElemNum = eVRAI,
                   EBooleen theIsElemNames = eVRAI);
                   
                   
    //! Write a MEDWrapper MED Polyedres representation into the MED file
    virtual
    void
    SetPolyedreInfo(const TPolyedreInfo& theInfo,
                    TErr* theErr = NULL);
                    
                    
    //! Create a MEDWrapper MED Balls representation
    /*! This feature is supported since version 3.0 */
    virtual
    PBallInfo
    CrBallInfo(const PMeshInfo& theMeshInfo,
               TInt theNbBalls,
               EBooleen theIsElemNum = eVRAI);

    //! Write a MEDWrapper representation of MED_BALL into the MED file
    /*! This feature is supported since version 3.0 */
    virtual
    void
    SetBallInfo(const TBallInfo& theInfo,
                TErr* theErr = NULL);

    
    //! Create a MEDWrapper MED Cells representation
    virtual
    PCellInfo
    CrCellInfo(const PMeshInfo& theMeshInfo,
               EEntiteMaillage theEntity,
               EGeometrieElement theGeom,
               TInt theNbElem,
               EConnectivite theConnMode = eNOD,
               EBooleen theIsElemNum = eVRAI,
               EBooleen theIsElemNames = eFAUX,
               EModeSwitch theMode = eFULL_INTERLACE);

    //! Write the MEDWrapper MED Cells representation into the MED file
    virtual
    void
    SetCellInfo(const TCellInfo& theInfo,
                TErr* theErr = NULL);
    
  };

  using MCPWrapper = std::shared_ptr<MCTWrapper>;
}
