// File    : SMESH_Gen_i_DumpPython.cxx
// Created : Thu Mar 24 17:17:59 2005
// Author  : Julia DOROVSKIKH
// Module  : SMESH
// $Header : $

#include "SMESH_PythonDump.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Filter_i.hxx"

#include <TColStd_HSequenceOfInteger.hxx>

namespace SMESH
{
  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, const char* theArg){
    theString += Standard_CString(theArg);
    return theString;
  }

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, int theArg){
    theString += TCollection_AsciiString(theArg);
    return theString;
  }

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, float theArg){
    theString += TCollection_AsciiString(theArg);
    return theString;
  }

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     const SMESH::long_array& theArg)
  {
    theString<<"[ ";
    CORBA::Long i = 1, iEnd = theArg.length();
    for(; i <= iEnd; i++) {
      theString<<int(theArg[i-1]);
      if(i < iEnd)
	theString<< ", ";
    }
    theString<<" ]";
    return theString;
  }


  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     CORBA::Object_ptr theArg)
  {
    CORBA::String_var aString("None");
    SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_ptr aStudy = aSMESHGen->GetCurrentStudy();
    SALOMEDS::SObject_var aSObject = SMESH_Gen_i::ObjectToSObject(aStudy,theArg);
    if(!aSObject->_is_nil()){
      aString = aSObject->GetID();
    }else if(!CORBA::is_nil(theArg)){
      aString = SMESH_Gen_i::GetORB()->object_to_string(theArg);
    }
    theString<<aString.in();
    return theString;
  }

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     SMESH::FilterLibrary_i* theArg)
  {
    theString += TCollection_AsciiString("aFilterLibrary_");
    theString += TCollection_AsciiString(int(theArg));
    return theString;
  }

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     SMESH::FilterManager_i* theArg)
  {
    theString += TCollection_AsciiString("aFilterManager_");
    theString += TCollection_AsciiString(int(theArg));
    return theString;
  }

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     SMESH::Filter_i* theArg)
  {
    theString += TCollection_AsciiString("aFilter_");
    theString += TCollection_AsciiString(int(theArg));
    return theString;
  }

  TCollection_AsciiString& 
  operator<<(TCollection_AsciiString& theString, 
	     SMESH::Functor_i* theArg)
  {
    FunctorType aFunctorType = theArg->GetFunctorType();
    switch(aFunctorType){
    case FT_AspectRatio:
      theString += TCollection_AsciiString("anAspectRatio");
      break;
    case FT_AspectRatio3D:
      theString += TCollection_AsciiString("anAspectRatio3D");
      break;
    case FT_Warping:
      theString += TCollection_AsciiString("aWarping");
      break;
    case FT_MinimumAngle:
      theString += TCollection_AsciiString("aMinimumAngle");
      break;
    case FT_Taper:
      theString += TCollection_AsciiString("aTaper");
      break;
    case FT_Skew:
      theString += TCollection_AsciiString("aSkew");
      break;
    case FT_Area:
      theString += TCollection_AsciiString("aArea");
      break;
    case FT_FreeBorders:
      theString += TCollection_AsciiString("aFreeBorders");
      break;
    case FT_FreeEdges:
      theString += TCollection_AsciiString("aFreeEdges");
      break;
    case FT_MultiConnection:
      theString += TCollection_AsciiString("aMultiConnection");
      break;
    case FT_MultiConnection2D:
      theString += TCollection_AsciiString("aMultiConnection2D");
      break;
    case FT_Length:
      theString += TCollection_AsciiString("aLength");
      break;
    case FT_Length2D:
      theString += TCollection_AsciiString("aLength");
      break;
    case FT_BelongToGeom:
      theString += TCollection_AsciiString("aBelongToGeom");
      break;
    case FT_BelongToPlane:
      theString += TCollection_AsciiString("aBelongToPlane");
      break;
    case FT_BelongToCylinder:
      theString += TCollection_AsciiString("aBelongToCylinder");
      break;
    case FT_LyingOnGeom:
      theString += TCollection_AsciiString("aLyingOnGeom");
      break;
    case FT_RangeOfIds:
      theString += TCollection_AsciiString("aRangeOfIds");
      break;
    case FT_BadOrientedVolume:
      theString += TCollection_AsciiString("aBadOrientedVolume");
      break;
    case FT_LessThan:
      theString += TCollection_AsciiString("aLessThan");
      break;
    case FT_MoreThan:
      theString += TCollection_AsciiString("aMoreThan");
      break;
    case FT_EqualTo:
      theString += TCollection_AsciiString("anEqualTo");
      break;
    case FT_LogicalNOT:
      theString += TCollection_AsciiString("aLogicalNOT");
      break;
    case FT_LogicalAND:
      theString += TCollection_AsciiString("aLogicalAND");
      break;
    case FT_LogicalOR:
      theString += TCollection_AsciiString("aLogicalOR");
      break;
    case FT_Undefined:
      theString += TCollection_AsciiString("anUndefined");
      break;
    }
    theString += Standard_CString("_");
    theString += TCollection_AsciiString(int(theArg));
    return theString;
  }

  TPythonDump::
  ~TPythonDump()
  {
    SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
    SALOMEDS::Study_ptr aStudy = aSMESHGen->GetCurrentStudy();
    if(!aStudy->_is_nil()){
      aSMESHGen->AddToPythonScript(aStudy->StudyId(),myString);
    }
  }
}

//=======================================================================
//function : DumpPython
//purpose  : 
//=======================================================================
Engines::TMPFile* SMESH_Gen_i::DumpPython (CORBA::Object_ptr theStudy,
                                           CORBA::Boolean isPublished,
                                           CORBA::Boolean& isValidScript)
{
  SALOMEDS::Study_var aStudy = SALOMEDS::Study::_narrow(theStudy);
  if (CORBA::is_nil(aStudy))
    return new Engines::TMPFile(0);

  SALOMEDS::SObject_var aSO = aStudy->FindComponent(ComponentDataType());
  if (CORBA::is_nil(aSO))
    return new Engines::TMPFile(0);

  // Map study entries to object names
  Resource_DataMapOfAsciiStringAsciiString aMap;
  TCollection_AsciiString s ("qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM0987654321_");

  SALOMEDS::ChildIterator_var Itr = aStudy->NewChildIterator(aSO);
  for (Itr->InitEx(true); Itr->More(); Itr->Next()) {
    SALOMEDS::SObject_var aValue = Itr->Value();

    TCollection_AsciiString aName (aValue->GetName());
    if (aName.Length() > 0) {
      int p, p2 = 1, e = aName.Length();
      while ((p = aName.FirstLocationNotInSet(s, p2, e))) {
        aName.SetValue(p, '_');
        p2 = p;
      }
      aMap.Bind(TCollection_AsciiString(aValue->GetID()), aName);
    }
  }

  // Get trace of restored study
  //SALOMEDS::SObject_var aSO = SMESH_Gen_i::ObjectToSObject(theStudy, _this());
  SALOMEDS::StudyBuilder_var aStudyBuilder = aStudy->NewBuilder();
  SALOMEDS::GenericAttribute_var anAttr =
    aStudyBuilder->FindOrCreateAttribute(aSO, "AttributePythonObject");

  char* oldValue = SALOMEDS::AttributePythonObject::_narrow(anAttr)->GetObject();
  TCollection_AsciiString aSavedTrace (oldValue);

  // Add trace of API methods calls and replace study entries by names
  bool aValidScript;
  //TCollection_AsciiString aScript = myGen.DumpPython
  TCollection_AsciiString aScript = DumpPython_impl
    (aStudy->StudyId(), aMap, isPublished, aValidScript, aSavedTrace);

  int aLen = aScript.Length(); 
  unsigned char* aBuffer = new unsigned char[aLen+1];
  strcpy((char*)aBuffer, aScript.ToCString());

  CORBA::Octet* anOctetBuf =  (CORBA::Octet*)aBuffer;
  Engines::TMPFile_var aStreamFile = new Engines::TMPFile(aLen+1, aLen+1, anOctetBuf, 1); 
  isValidScript = aValidScript;

  return aStreamFile._retn(); 
}

//=============================================================================
/*!
 *  AddToPythonScript
 */
//=============================================================================
void SMESH_Gen_i::AddToPythonScript (int theStudyID, const TCollection_AsciiString& theString)
{
  if (myPythonScripts.find(theStudyID) == myPythonScripts.end()) {
    myPythonScripts[theStudyID] = new TColStd_HSequenceOfAsciiString;
  }
  myPythonScripts[theStudyID]->Append(theString);
}

//=============================================================================
/*!
 *  RemoveLastFromPythonScript
 */
//=============================================================================
void SMESH_Gen_i::RemoveLastFromPythonScript (int theStudyID)
{
  if (myPythonScripts.find(theStudyID) != myPythonScripts.end()) {
    int aLen = myPythonScripts[theStudyID]->Length();
    myPythonScripts[theStudyID]->Remove(aLen);
  }
}

//=======================================================================
//function : AddToCurrentPyScript
//purpose  : 
//=======================================================================

void SMESH_Gen_i::AddToCurrentPyScript (const TCollection_AsciiString& theString)
{
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  SALOMEDS::Study_ptr aStudy = aSMESHGen->GetCurrentStudy();
  if (aStudy->_is_nil()) return;
  aSMESHGen->AddToPythonScript(aStudy->StudyId(), theString);
}


//=======================================================================
//function : AddObject
//purpose  : add object to script string
//=======================================================================

TCollection_AsciiString& SMESH_Gen_i::AddObject(TCollection_AsciiString& theStr,
                                                CORBA::Object_ptr        theObject)
{
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  SALOMEDS::SObject_var aSO =
    aSMESHGen->ObjectToSObject(aSMESHGen->GetCurrentStudy(), theObject);
  if ( !aSO->_is_nil() )
    theStr += aSO->GetID();
  else if ( !CORBA::is_nil( theObject ) )
    theStr += GetORB()->object_to_string( theObject );
  else
    theStr += "None";

  return theStr;
}

//=======================================================================
//function : SavePython
//purpose  : 
//=======================================================================
void SMESH_Gen_i::SavePython (SALOMEDS::Study_ptr theStudy)
{
  // Dump trace of API methods calls
  TCollection_AsciiString aScript = GetNewPythonLines(theStudy->StudyId());

  // Check contents of PythonObject attribute
  SALOMEDS::SObject_var aSO = theStudy->FindComponent(ComponentDataType());
  //SALOMEDS::SObject_var aSO = SMESH_Gen_i::ObjectToSObject(theStudy, _this());
  SALOMEDS::StudyBuilder_var aStudyBuilder = theStudy->NewBuilder();
  SALOMEDS::GenericAttribute_var anAttr =
    aStudyBuilder->FindOrCreateAttribute(aSO, "AttributePythonObject");

  char* oldValue = SALOMEDS::AttributePythonObject::_narrow(anAttr)->GetObject();
  TCollection_AsciiString oldScript (oldValue);

  if (oldScript.Length() > 0) {
    oldScript += "\n";
    oldScript += aScript;
  } else {
    oldScript = aScript;
  }

  // Store in PythonObject attribute
  SALOMEDS::AttributePythonObject::_narrow(anAttr)->SetObject(oldScript.ToCString(), 1);

  // Clean trace of API methods calls
  CleanPythonTrace(theStudy->StudyId());
}


// impl


//=============================================================================
/*!
 *  FindEntries: Returns a sequence of start/end positions of entries in the string
 */
//=============================================================================
Handle(TColStd_HSequenceOfInteger) FindEntries (TCollection_AsciiString& theString)
{
  Handle(TColStd_HSequenceOfInteger) aSeq = new TColStd_HSequenceOfInteger;
  Standard_Integer aLen = theString.Length();
  Standard_Boolean isFound = Standard_False;

  char* arr = theString.ToCString();
  Standard_Integer i = 0, j;

  while(i < aLen) {
    int c = (int)arr[i];
    j = i+1;
    if(c >= 48 && c <= 57) { //Is digit?
 
      isFound = Standard_False;
      while((j < aLen) && ((c >= 48 && c <= 57) || c == 58) ) { //Check if it is an entry
	c = (int)arr[j++];  
	if(c == 58) isFound = Standard_True;
      }

      if (isFound) {
        int prev = (i < 1) ? 0 : (int)arr[i - 1];
        // last char should be a diggit,
        // previous char should not be '"'.
        if (arr[j-2] != 58 && prev != 34) {
          aSeq->Append(i+1); // +1 because AsciiString starts from 1
          aSeq->Append(j-1);
        }
      }
    }

    i = j;
  }

  return aSeq;
}

//=============================================================================
/*!
 *  DumpPython
 */
//=============================================================================
TCollection_AsciiString SMESH_Gen_i::DumpPython_impl
                        (int theStudyID, 
                         Resource_DataMapOfAsciiStringAsciiString& theObjectNames,
                         bool isPublished, 
                         bool& aValidScript,
                         const TCollection_AsciiString& theSavedTrace)
{
  TCollection_AsciiString aScript;
  aScript += "import salome\n";
  aScript += "import geompy\n\n";
  aScript += "import SMESH\n";
  aScript += "import StdMeshers\n\n";
  aScript += "#import GEOM module\n";
  aScript += "import string\n";
  aScript += "import os\n";
  aScript += "import sys\n";
  aScript += "sys.path.append( os.path.dirname(__file__) )\n";
  aScript += "exec(\"from \"+string.replace(__name__,\"SMESH\",\"GEOM\")+\" import *\")\n\n";
  
  aScript += "def RebuildData(theStudy):";
  aScript += "\n\tsmesh = salome.lcc.FindOrLoadComponent(\"FactoryServer\", \"SMESH\")";
  if ( isPublished )
    aScript += "\n\tsmesh.SetCurrentStudy(theStudy)";
  else
    aScript += "\n\tsmesh.SetCurrentStudy(None)";

  TCollection_AsciiString globalVars;

  // Dump trace of restored study
  if (theSavedTrace.Length() > 0) {
    aScript += "\n";
    aScript += theSavedTrace;
  }

  // Dump trace of API methods calls
  TCollection_AsciiString aNewLines = GetNewPythonLines(theStudyID);
  if (aNewLines.Length() > 0) {
    aScript += "\n";
    aScript += aNewLines;
  }

  // Find entries to be replaced by names
  Handle(TColStd_HSequenceOfInteger) aSeq = FindEntries(aScript);
  Standard_Integer aLen = aSeq->Length();

  if (aLen == 0)
    return aScript;

  // Replace entries by the names
  GEOM::GEOM_Gen_ptr geom = GetGeomEngine();
  TColStd_SequenceOfAsciiString seqRemoved;
  Resource_DataMapOfAsciiStringAsciiString mapRemoved;
  Resource_DataMapOfAsciiStringAsciiString aNames;
  Standard_Integer objectCounter = 0, aStart = 1, aScriptLength = aScript.Length();
  TCollection_AsciiString anUpdatedScript, anEntry, aName, aBaseName("smeshObj_");

  for (Standard_Integer i = 1; i <= aLen; i += 2) {
    anUpdatedScript += aScript.SubString(aStart, aSeq->Value(i) - 1);
    anEntry = aScript.SubString(aSeq->Value(i), aSeq->Value(i + 1));
    if (theObjectNames.IsBound(anEntry)) {
      aName = theObjectNames.Find(anEntry);
      if (theObjectNames.IsBound(aName) && anEntry != theObjectNames(aName)) {
        // diff objects have same name - make a new name
        TCollection_AsciiString aName2;
        Standard_Integer i = 0;
        do {
          aName2 = aName + "_" + ++i;
        } while (theObjectNames.IsBound(aName2) && anEntry != theObjectNames(aName2));
        aName = aName2;
        theObjectNames(anEntry) = aName;
      }
    } else {
      // is a GEOM object?
      aName = geom->GetDumpName( anEntry.ToCString() );
      if ( aName.IsEmpty() ) {
      // ? Removed Object ?
        do {
          aName = aBaseName + TCollection_AsciiString(++objectCounter);
        } while (theObjectNames.IsBound(aName));
        seqRemoved.Append(aName);
        mapRemoved.Bind(anEntry, "1");
      }
      theObjectNames.Bind(anEntry, aName);
    }
    theObjectNames.Bind(aName, anEntry); // to detect same name of diff objects

    anUpdatedScript += aName;
    aNames.Bind(aName, "1");
    aStart = aSeq->Value(i + 1) + 1;
  }

  // add final part of aScript
  if (aSeq->Value(aLen) < aScriptLength)
    anUpdatedScript += aScript.SubString(aSeq->Value(aLen) + 1, aScriptLength);

  // Remove removed objects
  anUpdatedScript += "\n\taStudyBuilder = theStudy.NewBuilder()";
  for (int ir = 1; ir <= seqRemoved.Length(); ir++) {
    anUpdatedScript += "\n\tSO = theStudy.FindObjectIOR(theStudy.ConvertObjectToIOR(";
    anUpdatedScript += seqRemoved.Value(ir);
    anUpdatedScript += "))\n\tif SO is not None: aStudyBuilder.RemoveObjectWithChildren(SO)";
  }
  anUpdatedScript += "\n";

  // Set object names
  anUpdatedScript += "\n\tisGUIMode = ";
  anUpdatedScript += isPublished;
  anUpdatedScript += "\n\tif isGUIMode:";
  anUpdatedScript += "\n\t\tsmeshgui = salome.ImportComponentGUI(\"SMESH\")";
  anUpdatedScript += "\n\t\tsmeshgui.Init(theStudy._get_StudyId())";
  anUpdatedScript += "\n";

  Resource_DataMapOfAsciiStringAsciiString mapEntries;
  for (Standard_Integer i = 1; i <= aLen; i += 2) {
    anEntry = aScript.SubString(aSeq->Value(i), aSeq->Value(i + 1));
    if (theObjectNames.IsBound(anEntry) &&
        !mapEntries.IsBound(anEntry) &&
        !mapRemoved.IsBound(anEntry)) {
      aName = theObjectNames.Find(anEntry);
      mapEntries.Bind(anEntry, aName);
      anUpdatedScript += "\n\t\tsmeshgui.SetName(salome.ObjectToID(";
      anUpdatedScript += aName + "), \"" + aName + "\")";
    }
  }
  anUpdatedScript += "\n\n\t\tsalome.sg.updateObjBrowser(0)";

  anUpdatedScript += "\n\n\tpass\n";

  aValidScript = true;

  return anUpdatedScript;
}

//=============================================================================
/*!
 *  GetNewPythonLines
 */
//=============================================================================
TCollection_AsciiString SMESH_Gen_i::GetNewPythonLines (int theStudyID)
{
  TCollection_AsciiString aScript;

  // Dump trace of API methods calls
  if (myPythonScripts.find(theStudyID) != myPythonScripts.end()) {
    Handle(TColStd_HSequenceOfAsciiString) aPythonScript = myPythonScripts[theStudyID];
    Standard_Integer istr, aLen = aPythonScript->Length();
    for (istr = 1; istr <= aLen; istr++) {
      aScript += "\n\t";
      aScript += aPythonScript->Value(istr);
    }
    aScript += "\n";
  }

  return aScript;
}

//=============================================================================
/*!
 *  CleanPythonTrace
 */
//=============================================================================
void SMESH_Gen_i::CleanPythonTrace (int theStudyID)
{
  TCollection_AsciiString aScript;

  // Clean trace of API methods calls
  if (myPythonScripts.find(theStudyID) != myPythonScripts.end()) {
    myPythonScripts[theStudyID]->Clear();
  }
}
