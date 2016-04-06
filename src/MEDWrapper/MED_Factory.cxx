// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#include "MED_Factory.hxx"
#include "MED_Utilities.hxx"
#include "MED_V2_2_Wrapper.hxx"

#include <stdio.h>
#include <errno.h>
#include <sstream>

#include <med.h>
extern "C"
{
#ifndef WIN32
  #include <unistd.h>
#endif
}

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

namespace MED
{
  enum EStatus { eBad, eDeprecated, eGood };

  EStatus GetVersionId(const std::string& theFileName)
  {
    INITMSG(MYDEBUG,"GetVersionId - theFileName = '"<<theFileName<<"'"<<std::endl);

    EStatus status = eBad;    

    bool ok = true;
    // 1. Check that file is accessible
#ifdef WIN32
    ok = (GetFileAttributes(xmlPath.c_str()) != INVALID_FILE_ATTRIBUTES);
#else
    ok = (access(theFileName.c_str(), F_OK) == 0);
#endif
    // 2. Check compatibility of hdf and med versions
    if ( ok ) {
      med_bool hdfok, medok;
      MEDfileCompatibility(theFileName.c_str(), &hdfok, &medok);
      ok = hdfok /*&& medok*/; // med-2.1 is KO since med-3.0.0
    }
    // 3. Try to open the file with MED API
    if ( ok ) {
      med_idt aFid = MEDfileOpen(theFileName.c_str(), MED_ACC_RDONLY);

      MSG(MYDEBUG,"GetVersionId - theFileName = '"<<theFileName<<"'; aFid = "<<aFid<<std::endl);
      if (aFid >= 0) {
        med_int aMajor, aMinor, aRelease;
        med_err aRet = MEDfileNumVersionRd(aFid, &aMajor, &aMinor, &aRelease);
        INITMSG(MYDEBUG,"GetVersionId - theFileName = '"<<theFileName<<"'; aRet = "<<aRet<<std::endl);
        if (aRet >= 0) {
          if (aMajor == 2 && aMinor == 1)
            status = eDeprecated;
          else
            status = eGood;
        }
        else {
          // VSR: simulate med 2.3.6 behavior, med file version is assumed to 2.1
          status = eDeprecated;
        }
      }
      MEDfileClose(aFid);
      ok = true;
      BEGMSG(MYDEBUG,"GetVersionId - theFileName = '"<<theFileName<<"'; status = "<<status<<std::endl);
    }
    return status;
  }

  bool GetMEDVersion(const std::string& fname, int& major, int& minor, int& release)
  {
    med_idt f = MEDfileOpen(fname.c_str(), MED_ACC_RDONLY);
    if ( f<0 )
      return false;

    med_int aMajor, aMinor, aRelease;
    med_err aRet = MEDfileNumVersionRd(f, &aMajor, &aMinor, &aRelease);
    major = aMajor;
    minor = aMinor;
    release = aRelease;
    MEDfileClose( f );
    if ( aRet<0 ) {
      // VSR: simulate med 2.3.6 behavior, med file version is assumed to 2.1
      major = 2; minor = release = -1;
      //return false;
    }
    return true;
  }

  PWrapper CrWrapperR(const std::string& theFileName)
  {
    PWrapper aWrapper;
    EStatus status = GetVersionId(theFileName);
    switch (status) {
    case eGood:
      aWrapper.reset(new MED::V2_2::TVWrapper(theFileName));
      break;
    case eDeprecated:
      EXCEPTION(std::runtime_error,"Cannot open file '"<<theFileName<<"'. Med version 2.1 is not supported any more.");
      break;
    default:
      EXCEPTION(std::runtime_error,"MED::CrWrapper - theFileName = '"<<theFileName<<"'");
      break;
    }
    return aWrapper;
  }

  PWrapper CrWrapperW(const std::string& theFileName)
  {
    EStatus status = GetVersionId(theFileName);

    if (status != eGood)
      remove(theFileName.c_str());
    
    PWrapper aWrapper;
    aWrapper.reset(new MED::V2_2::TVWrapper(theFileName));

    return aWrapper;
  }
}
