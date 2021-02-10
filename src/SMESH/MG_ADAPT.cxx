// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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
// file : MG_ADAPT.cxx

#include "MG_ADAPT.hxx"

#include "MeshFormatReader.hxx"
#include "MeshFormatWriter.hxx"
#include "MEDFileMesh.hxx"
#include "MCAuto.hxx"
#include "MEDFileData.hxx"
#include "MEDFileField.hxx"
#include "MEDCouplingFieldDouble.hxx"

#include <SALOME_NamingService.hxx>
#include <Utils_SALOME_Exception.hxx>
#include "Utils_CorbaException.hxx"

#include <utilities.h>
#include <iostream>
#include <unistd.h>
#include <TCollection_AsciiString.hxx>
#include <cstring>
#include <cstdlib>
#include <boost/filesystem.hpp>

using namespace MG_ADAPT;
static std::string removeFile(std::string fileName, int& notOk)
{
  std::string errStr;
  notOk = std::remove(fileName.c_str());
  if (notOk) errStr = ToComment("\n error while removing file : ") << fileName;
        else errStr = ToComment("\n file : ") << fileName << " succesfully deleted! \n ";

  return errStr;
}
std::string remove_extension(const std::string& filename) {
  size_t lastdot = filename.find_last_of(".");
  if (lastdot == std::string::npos) return filename;
  return filename.substr(0, lastdot);
}
namespace
{
struct GET_DEFAULT // struct used to get default value from GetOptionValue()
{
  bool isDefault;
  operator bool* () {
      return &isDefault;
  }
};
}

//----------------------------------------------------------------------------------------
MgAdapt::MgAdapt()
{
  data = new MgAdaptHypothesisData();
  data->myInMeshName = "";
  data->fromMedFile = defaultFromMedFile();
  data->myFileInDir = defaultWorkingDirectory();
  data->myMeshFileIn = "";
  data->myFileOutDir = defaultWorkingDirectory();
  data->myOutMeshName = "";
  data->myMeshFileOut = "";
  data->myMeshOutMed = defaultMeshOutMed();
  data->myPublish = defaultPublish();
  data->myUseLocalMap = defaultUseLocalMap();
  data->myUseBackgroundMap = defaultUseBackgroundMap();
  data->myFileSizeMapDir = defaultWorkingDirectory();
  data->myMeshFileBackground = "";
  data->myUseConstantValue = defaultUseConstantValue();
  data->myConstantValue = 0.0;
  data->myFieldName = "";
  data->myUseNoTimeStep = defaultUseNoTimeStep();
  data->myUseLastTimeStep = defaultUseLastTimeStep();
  data->myUseChosenTimeStep = defaultUseChosenTimeStep();
  data->myTimeStep = -2;
  data->myRank =  -2;
  data->myWorkingDir = defaultWorkingDirectory();
  data->myLogFile = defaultLogFile();
  data->myVerboseLevel = defaultVerboseLevel();
  data->myPrintLogInFile = defaultPrintLogInFile();
  data->myKeepFiles = defaultKeepFiles();
  data->myRemoveLogOnSuccess = defaultRemoveLogOnSuccess();

  buildModel();
  setAll();
}
MgAdapt::MgAdapt(MgAdaptHypothesisData* myData)
{
  data = new MgAdaptHypothesisData();
  setData(myData);
  buildModel();
}

MgAdapt::MgAdapt( const MgAdapt& copy)
{
  data = new MgAdaptHypothesisData();
  MgAdaptHypothesisData *copyData = copy.getData();
  copyMgAdaptHypothesisData(copyData);
  setAll();

  this->_option2value = copy._option2value;
  this->_customOption2value = copy._customOption2value;
  this->_defaultOptionValues = copy._defaultOptionValues;
  this->_doubleOptions = copy._doubleOptions;
  this->_charOptions = copy._charOptions;
  this->_boolOptions = copy._boolOptions;
}

//-----------------------------------------------------------------------------------------
MgAdapt::~MgAdapt()
{
  delete data;
}
void MgAdapt::buildModel()
{

  const char* boolOptionNames[] = { "compute_ridges",                          // yes
                                    "" // mark of end
                                  };
  // const char* intOptionNames[] = { "max_number_of_errors_printed", // 1
  //                                  "max_number_of_threads",        // 4
  //                                  "" // mark of end
  // };
  const char* doubleOptionNames[] = { "max_memory",  // 0
                                      "" // mark of end
                                    };
  const char* charOptionNames[] = { "components",                    // "yes"
                                    "adaptation",            // both
                                    "" // mark of end
                                  };

  int i = 0;
  while (boolOptionNames[i][0])
  {
    _boolOptions.insert( boolOptionNames[i] );
    _option2value[boolOptionNames[i++]].clear();
  }
  // i = 0;
  // while (intOptionNames[i][0])
  //   _option2value[intOptionNames[i++]].clear();

  i = 0;
  while (doubleOptionNames[i][0]) {
    _doubleOptions.insert(doubleOptionNames[i]);
    _option2value[doubleOptionNames[i++]].clear();
  }
  i = 0;
  while (charOptionNames[i][0]) {
    _charOptions.insert(charOptionNames[i]);
    _option2value[charOptionNames[i++]].clear();
  }

  // default values to be used while MG-Adapt

  _defaultOptionValues["adaptation"                         ] = "both";
  _defaultOptionValues["components"                         ] = "outside components";
  _defaultOptionValues["compute_ridges"                     ] = "yes";
  _defaultOptionValues["max_memory"                         ] = ToComment(defaultMaximumMemory());
}

//=============================================================================
TOptionValues MgAdapt::getOptionValues() const
{
  TOptionValues vals;
  TOptionValues::const_iterator op_val = _option2value.begin();
  for ( ; op_val != _option2value.end(); ++op_val )
    vals.insert( make_pair( op_val->first, getOptionValue( op_val->first, GET_DEFAULT() )));

  return vals;
}

std::vector <std::string> MgAdapt::getOptionValuesStrVec() const
{
  std::vector <std::string> vals;
  TOptionValues::const_iterator op_val = _option2value.begin();
  for ( ; op_val != _option2value.end(); ++op_val )
    vals.push_back(op_val->first+":"+getOptionValue( op_val->first, GET_DEFAULT() ));

  return vals;
}

std::vector <std::string> MgAdapt::getCustomOptionValuesStrVec() const
{
  std::vector <std::string> vals;
  TOptionValues::const_iterator op_val;
  for ( op_val = _customOption2value.begin(); op_val != _customOption2value.end(); ++op_val )
  {
    vals.push_back(op_val->first+":"+getOptionValue( op_val->first, GET_DEFAULT() ));
  }
  return vals;
}
const TOptionValues& MgAdapt::getCustomOptionValues() const
{
  return _customOption2value;
}
void MgAdapt::setData(MgAdaptHypothesisData* myData)
{
  copyMgAdaptHypothesisData(myData);
  setAll();
}
MgAdaptHypothesisData* MgAdapt::getData() const
{
  return data;
}
void MgAdapt::setMedFileIn(std::string fileName)
{
  if ( isFileExist(fileName) )
  {
    medFileIn = fileName;

    if (medFileOut == "") // default MED file Out
      medFileOut = remove_extension( fileName )+ ".adapt.med";
  }
  else
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text = "\nThe file " + fileName + " does not exist.\n" ;
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }
}

std::string MgAdapt::getMedFileIn()
{
  return medFileIn;
}

void MgAdapt::setMedFileOut(std::string fileOut)
{
  medFileOut = fileOut;
}
std::string MgAdapt::getMedFileOut()
{
  return medFileOut;
}
void MgAdapt::setMeshOutMed(bool mybool)
{
  meshOutMed = mybool;
}
bool MgAdapt::getMeshOutMed()
{
  return meshOutMed;
}
void MgAdapt::setPublish(bool mybool)
{
  publish = mybool;
}
bool MgAdapt::getPublish()
{
  return publish;
}
void MgAdapt::setFieldName(std::string myFieldName)
{
  fieldName = myFieldName;
}
std::string MgAdapt::getFieldName()
{
  return fieldName;
}
void MgAdapt::setTimeStep(int time)
{
  timeStep = time;
}
int MgAdapt::getTimeStep() const
{
  return timeStep;
}

void MgAdapt::setRankTimeStep(int time, int myRank)
{
  timeStep = time;
  rank = myRank;
}

int MgAdapt::getRank()
{
  return rank;
}
void MgAdapt::setTimeStepRankLast()
{
  myUseLastTimeStep = true;
  myUseChosenTimeStep = false;
  myUseNoTimeStep = false;
  //~med_int aRank, tmst;
     //~std::string fieldFile = useBackgroundMap ? sizeMapFile : medFileIn;
  //~getTimeStepInfos(fieldFile, tmst, aRank);
  //~setRankTimeStep((int) tmst, (int) aRank);
}
void MgAdapt::setNoTimeStep()
{
  myUseLastTimeStep = false;
  myUseChosenTimeStep = false;
  myUseNoTimeStep = true;
  //~int aRank = (int)MED_NO_IT;
  //~int tmst  = (int)MED_NO_DT ;
  //~setRankTimeStep(tmst, aRank);
}
void MgAdapt::setChosenTimeStepRank()
{
  myUseLastTimeStep = false;
  myUseChosenTimeStep = true;
  myUseNoTimeStep = false;
  //~int aRank = (int)MED_NO_IT;
  //~int tmst  = (int)MED_NO_DT ;
  //~setRankTimeStep(tmst, aRank);
}
void MgAdapt::setUseLocalMap(bool myLocal)
{
  useLocalMap = myLocal;
}

bool MgAdapt::getUseLocalMap()
{
  return useLocalMap;
}

void MgAdapt::setUseBackgroundMap(bool bckg)
{
  useBackgroundMap = bckg;
}

bool MgAdapt::getUseBackgroundMap()
{
  return useBackgroundMap;
}

void MgAdapt::setUseConstantValue(bool cnst)
{
  useConstantValue = cnst;
}
bool MgAdapt::getUseConstantValue()
{
  return useConstantValue;
}
void MgAdapt::setLogFile(std::string myLogFile)
{
  logFile = myLogFile;
}
std::string MgAdapt::getLogFile()
{
  return logFile;
}
void MgAdapt::setVerbosityLevel(int verboLevel)
{
  verbosityLevel = verboLevel;
}
int MgAdapt::getVerbosityLevel()
{
  return verbosityLevel;
}
void MgAdapt::setRemoveOnSuccess(bool rmons)
{
  removeOnSuccess = rmons;
}
bool MgAdapt::getRemoveOnSuccess()
{
  return removeOnSuccess;
}
void MgAdapt::setSizeMapFile(std::string mapFile)
{
  if ( mapFile == "" || isFileExist(mapFile) )
  {
    sizeMapFile = mapFile;
  }
  else
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text = "\nThe file " + mapFile + " does not exist.\n" ;
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }
}
std::string MgAdapt::getSizeMapFile()
{
  return sizeMapFile;
}

void MgAdapt::setMeshName(std::string name)
{
  meshName = name;
}
std::string MgAdapt::getMeshName()
{
  return meshName;
}
void MgAdapt::setMeshNameOut(std::string name)
{
  meshNameOut = name;
}
std::string MgAdapt::getMeshNameOut()
{
  return meshNameOut;
}
void MgAdapt::setFromMedFile(bool mybool)
{
  fromMedFile = mybool;
}
bool MgAdapt::isFromMedFile()
{
  return fromMedFile;
}
void MgAdapt::setConstantValue(double cnst)
{
  constantValue = cnst;
}
double MgAdapt::getConstantValue() const
{
  return constantValue;
}

void MgAdapt::setWorkingDir(std::string dir)
{
  workingDir = dir;
}
std::string MgAdapt::getWorkingDir() const
{
  return workingDir;
}
void MgAdapt::setKeepWorkingFiles(bool mybool)
{
  toKeepWorkingFiles = mybool;
}
bool MgAdapt::getKeepWorkingFiles()
{
  return toKeepWorkingFiles;
}
void MgAdapt::setPrintLogInFile(bool print)
{
  printLogInFile = print;
}
bool MgAdapt::getPrintLogInFile()
{
  return printLogInFile;
}

bool MgAdapt::setAll()
{

  setFromMedFile(data->fromMedFile);
  std::string file;
  checkDirPath(data->myFileInDir);
  file = data->myFileInDir+data->myMeshFileIn;
  setMedFileIn(file);
  setMeshName(data->myInMeshName);
  setMeshNameOut(data->myOutMeshName);
  checkDirPath(data->myFileOutDir);
  std::string out = data->myFileOutDir+data->myMeshFileOut;
  setMedFileOut(out);
  setPublish(data->myPublish);
  setMeshOutMed(data->myMeshOutMed);
  setUseLocalMap(data->myUseLocalMap);
  setUseBackgroundMap(data->myUseBackgroundMap);
  setUseConstantValue(data->myUseConstantValue);

  std::string mapfile;
  if (useBackgroundMap)
  {
    checkDirPath(data->myFileSizeMapDir);
    mapfile = data->myFileSizeMapDir+data->myMeshFileBackground;
    setFieldName(data->myFieldName);
  }
  else if (useConstantValue)
  {
    setConstantValue(data->myConstantValue);
  }
  else
  {
    mapfile ="";
    setConstantValue(0.0);
    setFieldName(data->myFieldName);
  }

  setSizeMapFile(mapfile);
  if (data->myUseNoTimeStep)
    setNoTimeStep();
  else if (data->myUseLastTimeStep)
    setTimeStepRankLast();
  else
  {
    setChosenTimeStepRank();
    setRankTimeStep(data->myTimeStep, data->myRank);
  }
  /* Advanced options */
  setWorkingDir(data->myWorkingDir);
  checkDirPath(data->myWorkingDir);
  setLogFile(data->myWorkingDir+defaultLogFile());
  setVerbosityLevel(data->myVerboseLevel);
  setRemoveOnSuccess(data->myRemoveLogOnSuccess);
  setPrintLogInFile(data->myPrintLogInFile);
  setKeepWorkingFiles(data->myKeepFiles);

  return true;
}

void MgAdapt::checkDirPath(std::string& dirPath)
{
  const char lastChar = *dirPath.rbegin();
#ifdef WIN32
  if(lastChar != '\\') dirPath+='\\';
#else
  if(lastChar != '/') dirPath+='/';
#endif
}
//=============================================================================
void MgAdapt::setOptionValue(const std::string& optionName,
                             const std::string& optionValue)
throw (std::invalid_argument)
{
  TOptionValues::iterator op_val = _option2value.find(optionName);
  if (op_val == _option2value.end())
  {
    op_val = _customOption2value.find( optionName );
    _customOption2value[ optionName ] = optionValue;
    return;
  }

  if (op_val->second != optionValue)
  {

    std::string lowerOptionValue = toLowerStr(optionValue);
    const char* ptr = lowerOptionValue.c_str();
    // strip white spaces
    while (ptr[0] == ' ')
      ptr++;
    int i = strlen(ptr);
    while (i != 0 && ptr[i - 1] == ' ')
      i--;
    // check value type
    bool typeOk = true;
    std::string typeName;
    if (i == 0)
    {
      // empty string
    }
    else if (_charOptions.count(optionName))
    {
      // do not check strings
    }
    else if (_doubleOptions.count(optionName))
    {
      // check if value is double
      toDbl(ptr, &typeOk);
      typeName = "real";
    }
    else if (_boolOptions.count(optionName))
    {
      // check if value is bool
      toBool(ptr, &typeOk);
      typeName = "bool";
    }
    else
    {
      // check if value is int
      toInt(ptr, &typeOk);
      typeName = "integer";
    }
    if ( typeOk ) // check some specific values ?
    {
    }
    if ( !typeOk )
    {
      std::string msg = "Advanced option '" + optionName + "' = '" + optionValue + "' but must be " + typeName;
      throw std::invalid_argument(msg);
    }
    std::string value( ptr, i );
    if ( _defaultOptionValues[ optionName ] == value ) value.clear();

    op_val->second = value;

  }
}
//=============================================================================
//! Return option value. If isDefault provided, it can be a default value,
//  then *isDefault == true. If isDefault is not provided, the value will be
//  empty if it equals a default one.
std::string MgAdapt::getOptionValue(const std::string& optionName, bool*              isDefault) const
throw (std::invalid_argument)
{
  TOptionValues::const_iterator op_val = _option2value.find(optionName);
  if (op_val == _option2value.end())
  {
    op_val = _customOption2value.find(optionName);
    if (op_val == _customOption2value.end())
    {
      std::string msg = "Unknown MG-Adapt option: <" + optionName + ">";
      throw std::invalid_argument(msg);
    }
  }
  std::string val = op_val->second;
  if ( isDefault ) *isDefault = ( val.empty() );

  if ( val.empty() && isDefault )
  {
    op_val = _defaultOptionValues.find( optionName );
    if (op_val != _defaultOptionValues.end()) val = op_val->second;
  }
  return val;
}
//================================================================================
/*!
 * \brief Converts a string to a real value
 */
//================================================================================

double MgAdapt::toDbl(const std::string& str, bool* isOk )
throw (std::invalid_argument)
{
  if ( str.empty() ) throw std::invalid_argument("Empty value provided");

  char * endPtr;
  double val = strtod(&str[0], &endPtr);
  bool ok = (&str[0] != endPtr);

  if ( isOk ) *isOk = ok;

  if ( !ok )
  {
    std::string msg = "Not a real value:'" + str + "'";
    throw std::invalid_argument(msg);
  }
  return val;
}
//================================================================================
/*!
 * \brief Converts a string to a lower
 */
//================================================================================
std::string MgAdapt::toLowerStr(const std::string& str)
{
  std::string s = str;
  for ( size_t i = 0; i <= s.size(); ++i )
    s[i] = tolower( s[i] );
  return s;
}
//================================================================================
/*!
 * \brief Converts a string to a bool
 */
//================================================================================

bool MgAdapt::toBool(const std::string& str, bool* isOk )
throw (std::invalid_argument)
{
  std::string s = str;
  if ( isOk ) *isOk = true;

  for ( size_t i = 0; i <= s.size(); ++i )
    s[i] = tolower( s[i] );

  if ( s == "1" || s == "true" || s == "active" || s == "yes" )
    return true;

  if ( s == "0" || s == "false" || s == "inactive" || s == "no" )
    return false;

  if ( isOk )
    *isOk = false;
  else
  {
    std::string msg = "Not a Boolean value:'" + str + "'";
    throw std::invalid_argument(msg);
  }
  return false;
}
//================================================================================
/*!
 * \brief Converts a string to a integer value
 */
//================================================================================

int MgAdapt::toInt(const std::string& str, bool* isOk )
throw (std::invalid_argument)
{
  if ( str.empty() ) throw std::invalid_argument("Empty value provided");

  char * endPtr;
  int val = (int)strtol( &str[0], &endPtr, 10);
  bool ok = (&str[0] != endPtr);

  if ( isOk ) *isOk = ok;

  if ( !ok )
  {
    std::string msg = "Not an integer value:'" + str + "'";
    throw std::invalid_argument(msg);
  }
  return val;
}
//=============================================================================
bool MgAdapt::hasOptionDefined( const std::string& optionName ) const
{
  bool isDefault = false;
  try
  {
    getOptionValue( optionName, &isDefault );
  }
  catch ( std::invalid_argument )
  {
    return false;
  }
  return !isDefault;
}
//================================================================================
/*!
 * \brief Return command to run MG-Tetra mesher excluding file prefix (-f)
 */
//================================================================================

std::string MgAdapt::getCommandToRun(MgAdapt* hyp)
{
  return hyp ? hyp->getCommandToRun() : ToComment("error with hypothesis!");
}



int MgAdapt::compute(std::string& errStr)
{
  std::string cmd = getCommandToRun();
  int err = 0;
  execCmd( cmd.c_str(), err ); // run

  if ( err )
  {
    errStr = ToComment("system(mg-adapt.exe ...) command failed with error: ") << strerror( errno );
  }
  else
  {
    convertMeshFile(meshFormatOutputMesh, solFormatOutput);
  }
  if (!err) cleanUp();
  return err;
}

void MgAdapt::execCmd( const char* cmd, int& err)
{
  err = 1;
  std::array <char, 128> buffer;
  std::streambuf* buf;
outFileStream fileStream;
  if (printLogInFile)
  {
  fileStream.open(logFile);
  buf = fileStream.rdbuf();
  }
  else
  {
    buf = std::cout.rdbuf();
  }
  std::ostream logStream(buf);

  std::unique_ptr <FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose );
  if(!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }
  while(fgets(buffer.data(), buffer.size(), pipe.get()) !=nullptr )
  {
    logStream<<buffer.data() ;
  }
  err = 0;
}
/*
 * to delete tmp files .mesh, .sol and if needed
 * the log file
 *
 */
void MgAdapt::cleanUp()
{
  int notOk;
  std::string errStr;
  if(toKeepWorkingFiles)
    return;
  if(removeOnSuccess && printLogInFile)
    tmpFilesToBeDeleted.push_back(logFile);

  std::vector< std::string>::iterator it = tmpFilesToBeDeleted.begin();
  for (; it!=tmpFilesToBeDeleted.end(); ++it)
  {
    errStr=removeFile(*it, notOk);
    if (notOk)
    {
      appendMsgToLogFile(errStr);
    }

  }
}

void MgAdapt::appendMsgToLogFile(std::string& msg)
{
  std::ofstream logStream;
  logStream.open(logFile, std::ofstream::out | std::ofstream::app);
  logStream<< msg;
  logStream.close();
}
//================================================================================
/*!
 * \brief Return command to run MG-Tetra mesher excluding file prefix (-f)
 */
//================================================================================

std::string MgAdapt::getCommandToRun()
{
  /*
  || return system command with args and options
  ||
  */
  std::string errStr;
  std::string cmd = getExeName();
  std::string meshIn(""), sizeMapIn(""), solFileIn("");
  updateTimeStepRank();
  convertMedFile(meshIn, solFileIn, sizeMapIn);
  if (!isFileExist(meshIn) || !isFileExist(solFileIn))
  {
    errStr = ToComment(" failed to find .mesh or .sol file from converter ")<< strerror( errno );
    return errStr;
  }
  tmpFilesToBeDeleted.push_back(meshIn);
  tmpFilesToBeDeleted.push_back(solFileIn);
  if(useBackgroundMap && !isFileExist(sizeMapIn))
  {
    errStr = ToComment(" failed to find .mesh size map file from converter ")<< strerror( errno );
    return errStr;
  }

  cmd+= " --in "+ meshIn;
  meshFormatOutputMesh = getFileName()+".mesh";
  tmpFilesToBeDeleted.push_back(meshFormatOutputMesh);
  cmd+= " --out "+ meshFormatOutputMesh;
  if (useLocalMap || useConstantValue) cmd+= " --sizemap "+ solFileIn;
  else //  (useBackgroundMap)
  {
    cmd+= " --background_mesh "+ sizeMapIn ;
    cmd+= " --background_sizemap "+ solFileIn;
    tmpFilesToBeDeleted.push_back(sizeMapIn);
  }
  //~else
  //~{
      //~// constant value TODO
  //~}
  /* sizemap file is not adapted in case of only surface adaptation see MeshGems docs */
  std::string adapOp   = "adaptation";
  std::string adpOpVal = getOptionValue(adapOp);
  std::string surfaceAdapt = "surface";
  if(surfaceAdapt != adpOpVal )
  {
    std::string solFileOut = getFileName()+".sol";
    cmd+= " --write_sizemap "+ solFileOut;
    solFormatOutput.push_back(solFileOut);
    tmpFilesToBeDeleted.push_back(solFileOut);
  }
  if (verbosityLevel != defaultVerboseLevel())
  {
    cmd+= " --verbose "+ ToComment(verbosityLevel);
  }

  std::string option, value;
  bool isDefault;
  const TOptionValues* options[] = { &_option2value, &_customOption2value };
  for ( int iOp = 0; iOp < 2; ++iOp )
  {
    TOptionValues::const_iterator o2v = options[iOp]->begin();
    for ( ; o2v != options[iOp]->end(); ++o2v )
    {
      option = o2v->first;
      value = getOptionValue( option, &isDefault );

      if ( isDefault )
          continue;
      if ( value.empty() )//value == NoValue() )
      {
        if ( _defaultOptionValues.count( option ))
            continue; // non-custom option with no value
        //value.clear();
      }
      if ( strncmp( "no", option.c_str(), 2 ) == 0 ) // options w/o values: --no_*
      {
        if ( !value.empty() && toBool( value ) == false )
            continue;
        value.clear();
      }
      if ( option[0] != '-' )
        cmd += " --";
      else
        cmd += " ";
      cmd += option + " " + value;
    }
  }
    //~}
//~cmd+= " >"
#ifdef WIN32
    cmd += " < NUL";
#endif

  return cmd;
}

bool MgAdapt::isFileExist(const std::string& fName)
{

  if ( fName.empty() ) return false;

  boost::system::error_code err;
  bool res = boost::filesystem::exists( fName, err );

  return err ? false : res;
}
//=======================================================================
//function : defaultMaximumMemory
//=======================================================================

#if defined(WIN32)
#include <windows.h>
#elif !defined(__APPLE__)
#include <sys/sysinfo.h>
#endif

double MgAdapt::defaultMaximumMemory()
{
#if defined(WIN32)
  // See http://msdn.microsoft.com/en-us/library/aa366589.aspx
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof (statex);
  long err = GlobalMemoryStatusEx (&statex);
  if (err != 0)
  {
    double totMB = (double)statex.ullAvailPhys / 1024. / 1024.;
    return (double)( 0.7 * totMB );
  }
#elif !defined(__APPLE__)
  struct sysinfo si;
  long err = sysinfo( &si );
  if ( err == 0 )
  {
    long ramMB = si.totalram * si.mem_unit / 1024 / 1024;
    return ( 0.7 * ramMB );
  }
#endif
  return 1024;
}

//=======================================================================
//function : defaultWorkingDirectory
//=======================================================================

std::string MgAdapt::defaultWorkingDirectory()
{
  TCollection_AsciiString aTmpDir;

  char *Tmp_dir = getenv("SALOME_TMP_DIR");
  if(Tmp_dir != NULL)
  {
    aTmpDir = Tmp_dir;
  }
  else {
#ifdef WIN32
    aTmpDir = TCollection_AsciiString("C:\\");
#else
    aTmpDir = TCollection_AsciiString("/tmp/");
#endif
  }
  return aTmpDir.ToCString();
}
//================================================================================
/*!
 * \brief Return a unique file name
 */
//================================================================================

std::string MgAdapt::getFileName() const
{
  std::string aTmpDir = workingDir;
  const char lastChar = *aTmpDir.rbegin();
#ifdef WIN32
  if(lastChar != '\\') aTmpDir+='\\';
#else
  if(lastChar != '/') aTmpDir+='/';
#endif

  TCollection_AsciiString aGenericName = (char*)aTmpDir.c_str();
  aGenericName += "MgAdapt_";
  aGenericName += getpid();
  aGenericName += "_";
  aGenericName += Abs((Standard_Integer)(long) aGenericName.ToCString());

  return aGenericName.ToCString();
}
//=======================================================================
//function : defaultLogFile
//=======================================================================

std::string MgAdapt::defaultLogFile()
{
  std::string alogFile("MG_ADAPT.log");
  return alogFile;
}
//=======================================================================
//function : defaultUseConstantValue
//=======================================================================

bool MgAdapt::defaultUseConstantValue()
{
  return false;
}
//=======================================================================
//function : defaultUseNoTimeStep
//=======================================================================

bool MgAdapt::defaultUseNoTimeStep()
{
  return true;
}
//=======================================================================
//function : defaultRemoveLogOnSuccess
//=======================================================================

bool MgAdapt::defaultRemoveLogOnSuccess()
{
  return true;
}
//=======================================================================
//function : defaultPrintLogInFile
//=======================================================================

bool MgAdapt::defaultPrintLogInFile()
{
  return false;
}
//=======================================================================
//function : defaultUseChosenTimeStep
//=======================================================================

bool MgAdapt::defaultUseChosenTimeStep()
{
  return false;
}
//=======================================================================
//function : UseLastTimeStep
//=======================================================================

bool MgAdapt::defaultUseLastTimeStep()
{
  return false;
}
//=======================================================================
//function : defaultUseBackgroundMap
//=======================================================================

bool MgAdapt::defaultUseBackgroundMap()
{
  return false;
}
//=======================================================================
//function : defaultKeepFiles
//=======================================================================

bool MgAdapt::defaultKeepFiles()
{
  return false;
}
//=======================================================================
//function : defaultUseLocalMap
//=======================================================================

bool MgAdapt::defaultUseLocalMap()
{
  return true;
}
//=======================================================================
//function : defaultPublish
//=======================================================================

bool MgAdapt::defaultPublish()
{
  return false;
}
//=======================================================================
//function : defaultMeshOutMed
//=======================================================================

bool MgAdapt::defaultMeshOutMed()
{
  return true;
}
//=======================================================================
//function : defaultFromMedFile
//=======================================================================

bool MgAdapt::defaultFromMedFile()
{
  return true;
}
//=======================================================================
//function : defaultVerboseLevel
//=======================================================================

int  MgAdapt::defaultVerboseLevel()
{
  return 3;
}
std::string MgAdapt::getExeName()
{
  return "mg-adapt.exe";
}
void MgAdapt::copyMgAdaptHypothesisData( const MgAdaptHypothesisData* from)
{
  data->myFileInDir = from->myFileInDir;
  data->myMeshFileIn = from->myMeshFileIn;
  data->myMeshFileBackground = from->myMeshFileBackground;
  data->myOutMeshName = from->myOutMeshName;
  data->myMeshFileOut = from->myMeshFileOut;
  data->myFileOutDir = from->myFileOutDir;
  data->myFileSizeMapDir = from->myFileSizeMapDir;
  data->myFieldName = from->myFieldName;
  data->fromMedFile = from->fromMedFile;
  data->myPublish = from->myPublish;
  data->myMeshOutMed = from->myMeshOutMed;
  data->myUseLocalMap = from->myUseLocalMap;
  data->myUseBackgroundMap = from->myUseBackgroundMap;
  data->myUseConstantValue = from->myUseConstantValue;
  data->myConstantValue = from->myConstantValue;
  data->myTimeStep = from->myTimeStep;
  data->myRank = from->myRank;
  data->myUseNoTimeStep = from->myUseNoTimeStep;
  data->myUseLastTimeStep = from->myUseLastTimeStep;
  data->myUseChosenTimeStep = from->myUseChosenTimeStep;
  data->myWorkingDir = from->myWorkingDir;
  data->myLogFile = from->myLogFile;
  data->myPrintLogInFile = from->myPrintLogInFile;
  data->myKeepFiles = from->myKeepFiles;
  data->myRemoveLogOnSuccess = from->myRemoveLogOnSuccess;
  data->myVerboseLevel = from->myVerboseLevel;
}

bool MgAdapt::checkTimeStepRank(std::string fileIn)
{
  INFOS("checkTimeStepRank");
  bool ret = false ;
  MEDCoupling::MCAuto<MEDCoupling::MEDFileData> mfd = MEDCoupling::MEDFileData::New(fileIn);
  MEDCoupling::MCAuto<MEDCoupling::MEDFileAnyTypeFieldMultiTS> fts = dynamic_cast<MEDCoupling::MEDFileFieldMultiTS *>( mfd->getFields()->getFieldWithName(fieldName) );
//   std::cout << "--- timeStep " << timeStep << std::endl;
//   std::cout << "--- rank " << rank << std::endl;
  std::vector<double> timevalue;
  std::vector< std::pair<int,int> > timesteprank = fts->getTimeSteps(timevalue);
  std::size_t jaux(timesteprank.size());
  for(std::size_t j=0;j<jaux;j++)
  {
//     std::cout << "--- l[j]first  " << timesteprank[j].first << std::endl;
//     std::cout << "--- l[j]second " << timesteprank[j].second << std::endl;
    if ( ( timeStep == timesteprank[j].first ) & ( rank == timesteprank[j].second ) )
    {
      ret = true ;
    }
  }
  if ( ! ret )
  {
    std::cout << "Available (Time step, Rank) :" << std::endl;
    for(std::size_t j=0;j<jaux;j++)
    { std::cout << "(Time step = " << timesteprank[j].first << ", Rank = " << timesteprank[j].second << ")" << std::endl;}
   SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text = "(Time step = " + std::to_string(timeStep) + ", Rank = " + std::to_string(rank) + ") is not found." ;
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }
  return ret ;
}

void MgAdapt::convertMedFile(std::string& meshFormatMeshFileName, std::string& solFormatFieldFileName, std::string& meshFormatsizeMapFile)
{
  std::vector<std::string> fieldFileNames;
  MEDCoupling::MeshFormatWriter writer;
  MEDCoupling::MCAuto<MEDCoupling::MEDFileData> mfd = MEDCoupling::MEDFileData::New(medFileIn);
  MEDCoupling::MEDFileMeshes* meshes = mfd->getMeshes();
  MEDCoupling::MEDFileMesh* fileMesh = meshes->getMeshAtPos(0); // ok only one mesh in file!
  if (meshNameOut =="")
      meshNameOut = fileMesh->getName();
  storeGroupsAndFams(fileMesh);

  MEDCoupling::MCAuto<MEDCoupling::MEDFileFields> fields = MEDCoupling::MEDFileFields::New();
  solFormatFieldFileName = getFileName();
  solFormatFieldFileName+=".sol";
  fieldFileNames.push_back(solFormatFieldFileName);

  if (useBackgroundMap)
  {
    bool ret = checkTimeStepRank(sizeMapFile) ;
    meshFormatsizeMapFile = getFileName();
    meshFormatsizeMapFile += ".mesh";
    buildBackGroundMeshAndSolFiles(fieldFileNames, meshFormatsizeMapFile);
  }
  else if(useLocalMap)
  {
    bool ret = checkTimeStepRank(medFileIn) ;
    MEDCoupling::MCAuto<MEDCoupling::MEDFileAnyTypeFieldMultiTS> fts = dynamic_cast<MEDCoupling::MEDFileFieldMultiTS *>( mfd->getFields()->getFieldWithName(fieldName) );
    MEDCoupling::MCAuto<MEDCoupling::MEDFileAnyTypeField1TS> f = fts->getTimeStep(timeStep, rank);
    MEDCoupling::MCAuto<MEDCoupling::MEDFileFieldMultiTS> tmFts = MEDCoupling::MEDFileFieldMultiTS::New();
    tmFts->pushBackTimeStep(f);

    fields->pushField(tmFts);

    writer.setFieldFileNames( fieldFileNames);
  }
  else
  {
    MEDCoupling::MCAuto<MEDCoupling::MEDCouplingMesh> mesh = fileMesh->getMeshAtLevel(1); // nodes mesh
    MEDCoupling::MCAuto<MEDCoupling::MEDCouplingUMesh> umesh = mesh->buildUnstructured(); // nodes mesh
    int dim  =  umesh->getSpaceDimension();
    int version =  sizeof(double) < 8 ? 1 : 2;
    mcIdType nbNodes =  umesh->getNumberOfNodes();
    buildConstantSizeMapSolFile(solFormatFieldFileName, dim, version, nbNodes);
  }

  mfd->setFields( fields );
  meshFormatMeshFileName = getFileName();
  meshFormatMeshFileName+=".mesh";
  writer.setMeshFileName(meshFormatMeshFileName);
  writer.setMEDFileDS( mfd);
  writer.write();

}

void MgAdapt::convertMeshFile(std::string& meshFormatIn, std::vector< std::string>& solFieldFileNames) const
{
  MEDCoupling::MeshFormatReader reader(meshFormatIn, solFieldFileNames);

  MEDCoupling::MCAuto<MEDCoupling::MEDFileData> mfd = reader.loadInMedFileDS();
  // write MED
  MEDCoupling::MEDFileMeshes* meshes = mfd->getMeshes();
  MEDCoupling::MEDFileMesh* fileMesh = meshes->getMeshAtPos(0); // ok only one mesh in file!
  fileMesh->setName(meshNameOut);
  restoreGroupsAndFams(fileMesh);
  mfd->write(medFileOut, 2);
}

void MgAdapt::storeGroupsAndFams(MEDCoupling::MEDFileMesh* fileMesh)
{
  storefams(fileMesh);
  storeGroups(fileMesh);
}

void MgAdapt::restoreGroupsAndFams(MEDCoupling::MEDFileMesh* fileMesh) const
{
  restorefams(fileMesh);
  restoreGroups(fileMesh);
}
void MgAdapt::storeGroups(MEDCoupling::MEDFileMesh* fileMesh)
{
  std::map<std::string, std::vector<std::string> > grpFams = fileMesh->getGroupInfo();
  std::map<std::string, std::vector<std::string> >::iterator g2ff = grpFams.begin();

  for ( ; g2ff != grpFams.end(); ++g2ff )
  {
    std::string        groupName = g2ff->first;
    std::vector<std::string> famNames = g2ff->second;

    if ( famNames.empty() ) continue;
    std::size_t k = 0;
    std::vector< mcIdType> famListId;
    for ( size_t i = 0; i < famNames.size(); ++i )
    {
      famListId.push_back( fileMesh->getFamilyId( famNames[i].c_str() ) );
    }
    group grp(groupName, famListId, famNames);
    groupVec.push_back(grp);
  }
}

void MgAdapt::storefams(MEDCoupling::MEDFileMesh* fileMesh)
{
  std::map<std::string, mcIdType> grpFams = fileMesh->getFamilyInfo();
  std::map<std::string, mcIdType >::iterator f = grpFams.begin();

  for ( ; f != grpFams.end(); ++f )
  {
    if(!f->second) continue;  // FAMILLE_ZERO
    family fs(f->first, f->second);
    famVec.push_back(fs);
  }

}

void MgAdapt::restorefams(MEDCoupling::MEDFileMesh* fileMesh) const
{
  std::vector<family>::const_iterator fIt = famVec.begin();

  for (; fIt!=famVec.end(); ++fIt)
  {
    try  //
    {
      std::string givenFamNameFromMeshGemConverter = fileMesh->getFamilyNameGivenId( std::abs(fIt->_famId) );
      fileMesh->changeFamilyId(std::abs(fIt->_famId), fIt->_famId);
      fileMesh->changeFamilyName(givenFamNameFromMeshGemConverter, fIt->_famName);
    }
    catch (const std::exception& e)
    {
      std::cerr<<e.what();
    }
  }
}

void MgAdapt::restoreGroups(MEDCoupling::MEDFileMesh* fileMesh) const
{
  std::map<std::string, std::vector<std::string> > info;
  std::vector <group>::const_iterator grpFams = groupVec.begin();

  for (; grpFams!=groupVec.end(); ++grpFams)
  {
    info.insert(std::pair <std::string, std::vector<std::string> > (grpFams->_name, grpFams->_famNames) );
  }

  fileMesh->setGroupInfo(info);
}

void MgAdapt::buildConstantSizeMapSolFile(const std::string& solFormatFieldFileName, const int dim, const int version, const mcIdType nbNodes) const
{
  MeshFormat::Localizer loc;
  MeshFormat::MeshFormatParser writer;
  int fileId = writer.GmfOpenMesh( solFormatFieldFileName.c_str(), GmfWrite, version, dim);
  int typTab[] = {GmfSca};
  writer.GmfSetKwd(fileId, MeshFormat::GmfSolAtVertices, (int)nbNodes, 1, typTab);
  for (mcIdType i = 0; i<nbNodes; i++)
  {
    double valTab[1] = {constantValue};
    writer.GmfSetLin( fileId, MeshFormat::GmfSolAtVertices, valTab);
  }
  writer.GmfCloseMesh(fileId);
}

void MgAdapt::buildBackGroundMeshAndSolFiles(const std::vector<std::string>& fieldFileNames, const std::string& meshFormatsizeMapFile) const
{
  MEDCoupling::MCAuto<MEDCoupling::MEDFileData> tmpMfd = MEDCoupling::MEDFileData::New(sizeMapFile);
  MEDCoupling::MEDFileFields* tmpFields = tmpMfd->getFields();
  MEDCoupling::MEDFileAnyTypeFieldMultiTS* fts = tmpFields->getFieldWithName(fieldName);
  MEDCoupling::MCAuto<MEDCoupling::MEDFileFieldMultiTS>  fts1 = dynamic_cast<MEDCoupling::MEDFileFieldMultiTS *>(fts);
  MEDCoupling::MCAuto<MEDCoupling::MEDFileAnyTypeField1TS> f = fts1->getTimeStep(timeStep, rank);
  MEDCoupling::MCAuto<MEDCoupling::MEDFileFieldMultiTS> tmFts = MEDCoupling::MEDFileFieldMultiTS::New();
  tmFts->pushBackTimeStep(f);

  MEDCoupling::MCAuto<MEDCoupling::MEDFileFields> tmp_fields = MEDCoupling::MEDFileFields::New();
  tmp_fields->pushField(tmFts);

  tmpMfd->setFields( tmp_fields );
  MEDCoupling::MeshFormatWriter tmpWriter;
  tmpWriter.setMeshFileName(meshFormatsizeMapFile);
  tmpWriter.setFieldFileNames( fieldFileNames);
  tmpWriter.setMEDFileDS(tmpMfd);
  tmpWriter.write();
}
// =======================================================================
med_idt MgAdapt::openMedFile(const std::string aFile)
// =======================================================================
// renvoie le medId associe au fichier Med apres ouverture
{
  med_idt medIdt = MEDfileOpen(aFile.c_str(),MED_ACC_RDONLY);
  if (medIdt <0)
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text = "\nThe med file " + aFile + " cannot be opened.\n" ;
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }
  return medIdt;
}

MgAdapt::Status MgAdapt::addMessage(const std::string& msg,
                                  const bool         isFatal/*=false*/)
{
  if ( isFatal )
    _myErrorMessages.clear(); // warnings are useless if a fatal error encounters

  _myErrorMessages.push_back( msg );

//~MESSAGE(msg);
#ifdef _DEBUG_
  std::cout << msg << std::endl;
#endif
  return ( _myStatus = isFatal ? MgAdapt::DRS_FAIL : MgAdapt::DRS_WARN_SKIP_ELEM );
}

// =======================================================================
void MgAdapt::getTimeStepInfos(std::string aFile, med_int& numdt, med_int& numit)
// =======================================================================
{
// Il faut voir si plusieurs maillages

  herr_t erreur = 0 ;
  med_idt medIdt ;


  // Ouverture du fichier
  //~SCRUTE(aFile.toStdString());
  medIdt = openMedFile(aFile);
  if ( medIdt < 0 ) return ;
  // Lecture du nombre de champs
  med_int ncha = MEDnField(medIdt) ;
  if (ncha < 1 )
  {
    //~addMessage( ToComment(" error: there is no field in  ") << aFile, /*fatal=*/true );
    return;
  }
  // Lecture des caracteristiques du champs

  //       Lecture du type du champ, des noms des composantes et du nom de l'unite
  char nomcha  [MED_NAME_SIZE+1];
  strcpy(nomcha, fieldName.c_str());
//       Lecture du nombre de composantes
  med_int ncomp = MEDfieldnComponentByName(medIdt, nomcha);
  char meshname[MED_NAME_SIZE+1];
  char * comp = (char*) malloc(ncomp*MED_SNAME_SIZE+1);
  char * unit = (char*) malloc(ncomp*MED_SNAME_SIZE+1);
  char dtunit[MED_SNAME_SIZE+1];
  med_bool local;
  med_field_type typcha;
  med_int nbofcstp;
  erreur =  MEDfieldInfoByName (medIdt, nomcha, meshname,&local,&typcha,comp,unit,dtunit, &nbofcstp);
  free(comp);
  free(unit);
  if ( erreur < 0 )
  {
      //~addMessage( ToComment(" error: error while reading field  ") << nomcha << " in file " << aFile , /*fatal=*/true );
    return;
  }

  med_float dt;
  med_int tmp_numdt, tmp_numit;

  //~med_int step = data->myUseLastTimeStep ? nbofcstp : data->myTimeStep+1;
  //~myPrint("step ", step);
  erreur = MEDfieldComputingStepInfo ( medIdt, nomcha, 1, &numdt, &numit, &dt );
  for(med_int step = 1; step <= nbofcstp; step++ )
  {
    erreur = MEDfieldComputingStepInfo ( medIdt, nomcha, step, &tmp_numdt, &tmp_numit, &dt );
    if(tmp_numdt > numdt)
    {
      numdt = tmp_numdt;
      numit = tmp_numit;
    }
  }
  if ( erreur < 0 )
  {
    //~addMessage( ToComment(" error: error while reading field ") << nomcha << "step (numdt, numit) = " <<"("<< numdt<< ", " \
    numit<< ")" <<" in file " << aFile , /*fatal=*/true );
    return;
  }

  // Fermeture du fichier
  if ( medIdt > 0 ) MEDfileClose(medIdt);

}

void MgAdapt::updateTimeStepRank()
{

  med_int arank;
  med_int tmst;
  if (myUseNoTimeStep)
  {
    arank = MED_NO_IT;
    tmst  = MED_NO_DT ;
    setRankTimeStep((int)tmst, (int)arank);
  }
  else if (myUseLastTimeStep)
  {
    std::string fieldFile = useBackgroundMap ? sizeMapFile : medFileIn;
    getTimeStepInfos(fieldFile, tmst, arank);
    setRankTimeStep((int)tmst, (int)arank);
  }
}
