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

#include "MED_Utilities.hxx"
#include "MED_Common.hxx"

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

int MED::PrefixPrinter::myCounter = 0;

MED::PrefixPrinter::PrefixPrinter(bool theIsActive):
  myIsActive(theIsActive)
{
  if(myIsActive)
    myCounter++;
  MSG(MYDEBUG,"MED::PrefixPrinter::PrefixPrinter(...)- "<<myCounter);
}

MED::PrefixPrinter::~PrefixPrinter()
{
  if(myIsActive){
    myCounter--;
    if(myCounter < 0)
      EXCEPTION(std::runtime_error,"PrefixPrinter::~PrefixPrinter() - myCounter("<<myCounter<<") < 0");
  }
}

std::string MED::PrefixPrinter::GetPrefix()
{
  if(myCounter){
    if(myCounter < 0)
      EXCEPTION(std::runtime_error,"PrefixPrinter::~PrefixPrinter() - myCounter("<<myCounter<<") < 0");
    return std::string(myCounter*2,' ');
  }
  return "";
}
