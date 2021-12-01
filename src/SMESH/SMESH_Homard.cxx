//  HOMARD HOMARD : implementation of HOMARD idl descriptions
//
// Copyright (C) 2011-2021  CEA/DEN, EDF R&D
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
//  File   : HOMARD_Boundary.cxx
//  Author : Gerald NICOLAS, EDF
//  Module : HOMARD
//
// Remarques :
// L'ordre de description des fonctions est le meme dans tous les fichiers
// HOMARD_aaaa.idl, HOMARD_aaaa.hxx, HOMARD_aaaa.cxx, HOMARD_aaaa_i.hxx, HOMARD_aaaa_i.cxx :
// 1. Les generalites : Name, Delete, DumpPython, Dump, Restore
// 2. Les caracteristiques
// 3. Le lien avec les autres structures
//
// Quand les 2 fonctions Setxxx et Getxxx sont presentes, Setxxx est decrit en premier

#include "SMESH_Homard.hxx"

#include <Utils_SALOME_Exception.hxx>
#include <utilities.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif

namespace SMESHHOMARDImpl
{

  std::string SEPARATOR = "|" ;

  /*!
    \brief Read next chunk of data from the string
    \internal

    The function tries to read next chunk of the data from the input string \a str.
    The parameter \a start specifies the start position of next chunk. If the operation
    read the chunk successfully, after its completion this parameter will refer to the
    start position of the next chunk. The function returns resulting chunk as a string.
    The status of the operation is returned via \a ok parameter.

    \param str source data stream string
    \param start start position to get next chunk
    \param ok in this variable the status of the chunk reading operation is returned
    \return next chunk read from the string
  */
  static std::string getNextChunk( const std::string& str, std::string::size_type& start, bool& ok )
  {
    std::string chunk = "";
    ok = false;
    if ( start <= str.size() ) {
      std::string::size_type end = str.find( separator(), start );
      chunk = str.substr( start, end == std::string::npos ? std::string::npos : end-start );
      start = end == std::string::npos ? str.size()+1 : end + separator().size();
      ok = true;
    }
    return chunk;
  }

  /*!
    \brief Get persistence signature
    \param type persistence entity type
    \return persistence signature
  */
  std::string GetSignature( SignatureType type )
  {
    std::string signature = "";
    switch ( type ) {
    case Case:       signature = "CASE"; break;
    case Zone:       signature = "ZONE"; break;
    case Hypothesis: signature = "HYPO"; break;
    case Iteration:  signature = "ITER"; break;
    case Boundary:   signature = "BOUNDARY"; break;
    default: break;
    }
    signature += separator();
    return signature;
  }

  /*!
    \brief Get data separator
    \return string that is used to separate data entities in the stream
  */
  std::string separator()
  {
    return SEPARATOR ;
  }

// =======================
// 1.1. Case
// =======================
  /*!
    \brief Dump case to the string
    \param cas case being dumped
    \return string representation of the case
  */
  std::string Dump( const HOMARD_Cas& cas )
  {
    std::stringstream os;
    std::string saux ;
    // ...
    MESSAGE( ". Sauvegarde du cas "<<cas.GetName());
    os << cas.GetName();
    os << separator() << cas.GetDirName();
    os << separator() << cas.GetConfType();
    os << separator() << cas.GetExtType();

    std::vector<double> coor = cas.GetBoundingBox();
    os << separator() << coor.size();
    for ( int i = 0; i < coor.size(); i++ )
          os << separator() << coor[i];

    std::list<std::string> ListString = cas.GetIterations();
    os << separator() << ListString.size();
    std::list<std::string>::const_iterator it;
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    ListString = cas.GetGroups();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
         os << separator() << *it;
    ListString = cas.GetBoundaryGroup();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
         os << separator() << *it;

    os << separator() << cas.GetPyram();

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }
//
// ==============
// 1.2. Iteration
// ==============
//
  /*!
    \brief Dump iteration to the string
    \param iteration iteration being dumped
    \return string representation of the iteration
  */
  std::string Dump( const HOMARD_Iteration& iteration )
  {
    std::stringstream os;
    std::string saux ;
    // ...
    MESSAGE( ". Sauvegarde de l'iteration "<<iteration.GetName());
    os << iteration.GetName();
    os << separator() << iteration.GetState();
    os << separator() << iteration.GetNumber();
    os << separator() << iteration.GetMeshFile();
    os << separator() << iteration.GetLogFile();
    os << separator() << iteration.GetMeshName();
    os << separator() << iteration.GetFieldFile();
    os << separator() << iteration.GetTimeStep();
    os << separator() << iteration.GetRank();
    os << separator() << iteration.GetIterParentName();
    //
    std::list<std::string> ListString = iteration.GetIterations();
    os << separator() << ListString.size();
    std::list<std::string>::const_iterator it;
    for ( it = ListString.begin(); it != ListString.end(); ++it )
      os << separator() << *it;

    os << separator() << iteration.GetHypoName();
    os << separator() << iteration.GetCaseName();
    os << separator() << iteration.GetDirNameLoc();

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }
//
// ==============
// 1.3. hypothese
// ==============
  /*!
    \brief Dump hypothesis to the string
    \param hypothesis hypothesis being dumped
    \return string representation of the hypothesis
  */
  std::string Dump( const HOMARD_Hypothesis& hypothesis )
  {
    std::stringstream os;
    std::string saux ;
    // ...
    MESSAGE( ". Sauvegarde de l'hypothese "<<hypothesis.GetName());
    os << hypothesis.GetName();
    os << separator() << hypothesis.GetCaseCreation();
    os << separator() << hypothesis.GetAdapType();
    os << separator() << hypothesis.GetRefinType();
    os << separator() << hypothesis.GetUnRefType();
    os << separator() << hypothesis.GetFieldName();
    os << separator() << hypothesis.GetRefinThrType();
    os << separator() << hypothesis.GetThreshR();
    os << separator() << hypothesis.GetUnRefThrType();
    os << separator() << hypothesis.GetThreshC();
    os << separator() << hypothesis.GetUseField();
    os << separator() << hypothesis.GetUseComp();
    os << separator() << hypothesis.GetTypeFieldInterp();

    std::list<std::string> ListString = hypothesis.GetIterations();
    std::list<std::string>::const_iterator it;
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
         os << separator() << *it;

    ListString = hypothesis.GetZones();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    ListString = hypothesis.GetComps();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
         os << separator() << *it;

    ListString = hypothesis.GetGroups();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    ListString = hypothesis.GetFieldInterps();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    os << separator() << hypothesis.GetNivMax();
    os << separator() << hypothesis.GetDiamMin();
    os << separator() << hypothesis.GetAdapInit();
    os << separator() << hypothesis.GetExtraOutput();

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }
//
// ==============================
// 1.5. Archivage d'une frontiere
// ==============================

  /*!
    \brief Dump boundary to the string
    \param boundary boundary being dumped
    \return string representation of the boundary
  */
  std::string Dump( const HOMARD_Boundary& boundary )
  {
    std::stringstream os;
    std::string saux ;
    MESSAGE( ". Sauvegarde de la frontiere "<<boundary.GetName());

    int BoundaryType = boundary.GetType() ;

    os << boundary.GetName() ;
    os << separator() << BoundaryType ;
    os << separator() << boundary.GetCaseCreation() ;

    if ( BoundaryType == -1 )
    {
      os << separator() << boundary.GetDataFile();
    }
    else if ( BoundaryType == 0 )
    {
      os << separator() << boundary.GetMeshName();
      os << separator() << boundary.GetDataFile();
    }
    else {
      std::vector<double> coor = boundary.GetCoords() ;
      for ( int i = 0; i < coor.size(); i++ )
            os << separator() << coor[i];
      std::vector<double> limit = boundary.GetLimit();
      for ( int i = 0; i < limit.size(); i++ )
            os << separator() << limit[i];
    }

    std::list<std::string> ListString = boundary.GetGroups();
    std::list<std::string>::const_iterator it;
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }

//
// 2. Restauration des objets
// ==========================
// 2.1. Case
// ==========================
//
  /*!
    \brief Restore case from the string
    \param cas case being restored
    \param stream string representation of the case
    \return \c true if case is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_Cas& cas, const std::string& stream )
  {
    MESSAGE( ". Restoration du cas ");
    std::string::size_type start = 0;
    std::string chunk, chunkNext;
    bool ok;
    // ...
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetName( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetDirName( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetConfType( atoi( chunk.c_str() ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetExtType( atoi( chunk.c_str() ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;

    int size = atoi( chunk.c_str() );
    std::vector<double> boite;
    boite.resize( size );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boite[i] = strtod( chunk.c_str(), 0 );
    }
    cas.SetBoundingBox( boite );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;

    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      cas.AddIteration( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ )
    {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      cas.AddGroup( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      i++;
      chunkNext = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      cas.AddBoundaryGroup( chunk.c_str(), chunkNext.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetPyram( atoi( chunk.c_str() ) );

    return true;
  }
//
// ==============
// 2.2. Iteration
// ==============
  /*!
    \brief Restore iteration from the string
    \param iteration iteration being restored
    \param stream string representation of the iteration
    \return \c true if iteration is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_Iteration& iteration, const std::string& stream )
  {
    std::string::size_type start = 0;
    std::string chunk;
    bool ok;
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;

    iteration.SetName( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetState( atoi( chunk.c_str() ) );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetNumber( atoi( chunk.c_str() ) );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetMeshFile( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetLogFile( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetMeshName( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetFieldFile( chunk.c_str() );
    // .
    int timestep, rank;
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    timestep = atoi( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    rank = atoi( chunk.c_str() );
    iteration.SetTimeStepRank( timestep, rank );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetIterParentName( chunk.c_str() );
    //
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      iteration.LinkNextIteration( chunk.c_str() );
    }
    //
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetHypoName( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetCaseName( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetDirNameLoc( chunk.c_str() );
    return true;
  }

//
// ==============
// 2.3. hypothese
// ==============
  /*!
    \brief Restore hypothesis from the string
    \param hypothesis hypothesis being restored
    \param stream string representation of the hypothesis
    \return \c true if hypothesis is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_Hypothesis& hypothesis, const std::string& stream )
  {
    std::string::size_type start = 0;
    std::string chunk, chunkNext;
    bool ok;

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetName( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetCaseCreation( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetAdapType( atoi( chunk.c_str() ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int typeraff = atoi( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int typedera = atoi( chunk.c_str() );
    hypothesis.SetRefinTypeDera( typeraff, typedera );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetField( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int typethr = atoi( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    double threshr = strtod( chunk.c_str(), 0 );
    hypothesis.SetRefinThr( typethr, threshr );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int typethc = atoi( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    double threshc = strtod( chunk.c_str(), 0 );
    hypothesis.SetUnRefThr( typethc, threshc );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetUseField(atoi(chunk.c_str()));

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetUseComp(atoi(chunk.c_str()));

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetTypeFieldInterp(atoi(chunk.c_str()));

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      hypothesis.LinkIteration( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      i++;
      chunkNext = getNextChunk( stream, start, ok );
      int typeuse = atoi( chunkNext.c_str() );
      if ( !ok ) return false;
      hypothesis.AddZone( chunk.c_str(), typeuse );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      hypothesis.AddComp( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      hypothesis.AddGroup( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      i++;
      chunkNext = getNextChunk( stream, start, ok );
      int TypeInterp = atoi( chunkNext.c_str() );
      if ( !ok ) return false;
      hypothesis.AddFieldInterpType( chunk.c_str(), TypeInterp );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetNivMax( atoi( chunk.c_str() ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetDiamMin( strtod( chunk.c_str(), 0 ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetAdapInit( strtod( chunk.c_str(), 0 ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetExtraOutput( strtod( chunk.c_str(), 0 ) );

    return true;
  }

//
// =================================
// 2.5. Restauration d'une frontiere
// =================================

  /*!
    \brief Restore boundary from the string
    \param boundary boundary being restored
    \param stream string representation of the boundary
    \return \c true if the boundary is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_Boundary& boundary, const std::string& stream )
  {
    std::string::size_type start = 0;
    std::string chunk;
    bool ok;

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    boundary.SetName( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int BoundaryType = atoi( chunk.c_str() ) ;
    boundary.SetType( BoundaryType );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    boundary.SetCaseCreation( chunk.c_str() );

    // Si analytique, les coordonnees des frontieres : le nombre depend du type
    // Si discret, le maillage
    // Si CAO, la géométrie
    int lgcoords ;
    if ( BoundaryType == -1 ) { lgcoords = -1 ; }
    else if ( BoundaryType == 1 ) { lgcoords = 7 ; }
    else if ( BoundaryType == 2 ) { lgcoords = 4 ; }
    else { lgcoords = 0 ; }
//
    if ( lgcoords == -1 )
    {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boundary.SetDataFile( chunk.c_str() );
    }
    else if ( lgcoords == 0 )
    {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boundary.SetMeshName( chunk.c_str() );

      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boundary.SetDataFile( chunk.c_str() );
    }
    else
    { std::vector<double> coords;
      coords.resize( lgcoords );
      for ( int i = 0; i < lgcoords; i++ ) {
        chunk = getNextChunk( stream, start, ok );
        if ( !ok ) return false;
        coords[i] = strtod( chunk.c_str(), 0 );
      }
      if ( BoundaryType == 1 )
      { boundary.SetCylinder(coords[0],coords[1],coords[2],coords[3],coords[4],coords[5],coords[6]); }
      else if ( BoundaryType == 2 )
      { boundary.SetSphere( coords[0], coords[1], coords[2], coords[3]); }
      else if ( BoundaryType == 3 )
      { boundary.SetConeA( coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], coords[6]); }
      else if ( BoundaryType == 4 )
      { boundary.SetConeR( coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], coords[6], coords[7]); }
      // Remarque : la taille de coords est suffisante pour les limites
      for ( int i = 0; i < 3; i++ ) {
        chunk = getNextChunk( stream, start, ok );
        if ( !ok ) return false;
        coords[i] = strtod( chunk.c_str(), 0 );
      }
      boundary.SetLimit( coords[0], coords[1], coords[2]);
    }
    // Les groupes
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boundary.AddGroup( chunk.c_str() );
    }

    return true;
  }

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================
HOMARD_Boundary::HOMARD_Boundary():
  _Name( "" ),_Type( 1 ),
  _Xmin( 0 ), _Xmax( 0 ), _Ymin( 0 ), _Ymax( 0 ), _Zmin( 0 ), _Zmax( 0 ),
  _Xaxe( 0 ), _Yaxe( 0 ), _Zaxe( 0 ),
  _Xcentre( 0 ), _Ycentre( 0 ), _Zcentre( 0 ), _rayon( 0 ),
  _Xincr( 0 ), _Yincr( 0 ), _Zincr( 0 )
{
  MESSAGE("HOMARD_Boundary");
}

//=============================================================================
HOMARD_Boundary::~HOMARD_Boundary()
{
  MESSAGE("~HOMARD_Boundary");
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Boundary::SetName( const char* Name )
{
  _Name = std::string( Name );
}
//=============================================================================
std::string HOMARD_Boundary::GetName() const
{
  return _Name;
}
//=============================================================================
std::string HOMARD_Boundary::GetDumpPython() const
{
  std::ostringstream aScript;
  aScript << "\n# Creation of the ";
//
  switch (_Type)
  {
    case -1:
    {
      aScript << "CAO boundary " << _Name << "\n";
      aScript << "\t" << _Name << " = homard.CreateBoundaryCAO(\"" << _Name << "\", ";
      aScript << "\"" << _DataFile << "\")\n";
      break ;
    }
    case 0:
    {
      aScript << "discrete boundary " << _Name << "\n";
      aScript << "\t" << _Name << " = homard.CreateBoundaryDi(\"" << _Name << "\", ";
      aScript << "\"" << _MeshName << "\", ";
      aScript << "\"" << _DataFile << "\")\n";
      break ;
    }
    case 1:
    {
      aScript << "cylinder " << _Name << "\n";
      aScript << "\t" << _Name << " = homard.CreateBoundaryCylinder(\"" << _Name << "\", ";
      aScript << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ", " << _Xaxe << ", " << _Yaxe << ", " << _Zaxe << ", " << _rayon << ")\n";
      break ;
    }
    case 2:
    {
      aScript << "sphere " << _Name << "\n";
      aScript << "\t" << _Name << " = homard.CreateBoundarySphere(\"" << _Name << "\", ";
      aScript << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ", " << _rayon << ")\n";
      break ;
    }
    case 3:
    {
      aScript << "cone " << _Name << "\n";
      aScript << "\t" << _Name << " = homard.CreateBoundaryConeA(\"" << _Name << "\", ";
      aScript << _Xaxe << ", " << _Yaxe << ", " << _Zaxe << ", " << _Angle << ", " << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ")\n";
      break ;
    }
    case 4:
    {
      aScript << "cone " << _Name << "\n";
      aScript << "\t" << _Name << " = homard.CreateBoundaryConeR(\"" << _Name << "\", ";
      aScript << _Xcentre1 << ", " << _Ycentre1 << ", " << _Zcentre1 << ", " << _Rayon1 << ", " << _Xcentre2 << ", " << _Ycentre2 << ", " << _Zcentre2 << ", " << _Rayon2 << ")\n";
      break ;
    }
    case 5:
    {
      aScript << "tore " << _Name << "\n";
      aScript << "\t" << _Name << " = homard.CreateBoundaryTorus(\"" << _Name << "\", ";
      aScript << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ", " << _Xaxe << ", " << _Yaxe << ", " << _Zaxe << ", " << _Rayon1 << ", " << _Rayon2 << ")\n";
      break ;
    }
  }

  return aScript.str();
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Boundary::SetType( int Type )
{
  _Type = Type;
}
//=============================================================================
int HOMARD_Boundary::GetType() const
{
  return _Type;
}
//=============================================================================
void HOMARD_Boundary::SetMeshName( const char* MeshName )
{
  _MeshName = std::string( MeshName );
}
//=============================================================================
std::string HOMARD_Boundary::GetMeshName() const
{
  return _MeshName;
}
//=============================================================================
void HOMARD_Boundary::SetDataFile( const char* DataFile )
{
  _DataFile = std::string( DataFile );
}
//=============================================================================
std::string HOMARD_Boundary::GetDataFile() const
{
  return _DataFile;
}
//=======================================================================================
void HOMARD_Boundary::SetCylinder( double X0, double X1, double X2,
                                   double X3, double X4, double X5, double X6 )
{
  _Xcentre = X0; _Ycentre = X1; _Zcentre = X2;
  _Xaxe = X3; _Yaxe = X4; _Zaxe = X5;
  _rayon = X6;
}
//======================================================================
void HOMARD_Boundary::SetSphere( double X0, double X1, double X2, double X3 )
{
  _Xcentre = X0; _Ycentre = X1; _Zcentre = X2;
  _rayon = X3;
}
//======================================================================
void HOMARD_Boundary::SetConeR( double Xcentre1, double Ycentre1, double Zcentre1, double Rayon1,
                                double Xcentre2, double Ycentre2, double Zcentre2, double Rayon2)
{
  _Xcentre1 = Xcentre1; _Ycentre1 = Ycentre1; _Zcentre1 = Zcentre1;
  _Rayon1 = Rayon1;
  _Xcentre2 = Xcentre2; _Ycentre2 = Ycentre2; _Zcentre2 = Zcentre2;
  _Rayon2 = Rayon2;
}
//======================================================================
void HOMARD_Boundary::SetConeA( double Xaxe, double Yaxe, double Zaxe, double Angle,
                                double Xcentre, double Ycentre, double Zcentre)
{
  _Xaxe = Xaxe; _Yaxe = Yaxe; _Zaxe = Zaxe;
  _Angle = Angle;
  _Xcentre = Xcentre; _Ycentre = Ycentre; _Zcentre = Zcentre;
}
//=======================================================================================
void HOMARD_Boundary::SetTorus( double X0, double X1, double X2,
                                double X3, double X4, double X5, double X6, double X7 )
{
  _Xcentre = X0; _Ycentre = X1; _Zcentre = X2;
  _Xaxe = X3; _Yaxe = X4; _Zaxe = X5;
  _Rayon1 = X6;
  _Rayon2 = X7;
}
//=======================================================================================
std::vector<double> HOMARD_Boundary::GetCoords() const
{
  std::vector<double> mesCoor;
//
  switch (_Type)
  {
//  Cylindre
    case 1:
    {
      mesCoor.push_back( _Xcentre );
      mesCoor.push_back( _Ycentre );
      mesCoor.push_back( _Zcentre );
      mesCoor.push_back( _Xaxe );
      mesCoor.push_back( _Yaxe );
      mesCoor.push_back( _Zaxe );
      mesCoor.push_back( _rayon );
      break ;
    }
//  Sphere
    case 2:
    {
      mesCoor.push_back( _Xcentre );
      mesCoor.push_back( _Ycentre );
      mesCoor.push_back( _Zcentre );
      mesCoor.push_back( _rayon );
      break ;
    }
//  Cone defini par un axe et un angle
    case 3:
    {
      mesCoor.push_back( _Xaxe );
      mesCoor.push_back( _Yaxe );
      mesCoor.push_back( _Zaxe );
      mesCoor.push_back( _Angle );
      mesCoor.push_back( _Xcentre );
      mesCoor.push_back( _Ycentre );
      mesCoor.push_back( _Zcentre );
      break ;
    }
//  Cone defini par les 2 rayons
    case 4:
    {
      mesCoor.push_back( _Xcentre1 );
      mesCoor.push_back( _Ycentre1 );
      mesCoor.push_back( _Zcentre1 );
      mesCoor.push_back( _Rayon1 );
      mesCoor.push_back( _Xcentre2 );
      mesCoor.push_back( _Ycentre2 );
      mesCoor.push_back( _Zcentre2 );
      mesCoor.push_back( _Rayon2 );
      break ;
    }
//  Tore
    case 5:
    {
      mesCoor.push_back( _Xcentre );
      mesCoor.push_back( _Ycentre );
      mesCoor.push_back( _Zcentre );
      mesCoor.push_back( _Xaxe );
      mesCoor.push_back( _Yaxe );
      mesCoor.push_back( _Zaxe );
      mesCoor.push_back( _Rayon1 );
      mesCoor.push_back( _Rayon2 );
      break ;
    }
    VERIFICATION( (_Type>=1) && (_Type<=5) ) ;
  }
  return mesCoor;
}
//======================================================================
void HOMARD_Boundary::SetLimit( double X0, double X1, double X2 )
{
  _Xincr = X0; _Yincr = X1; _Zincr = X2;
}
//=======================================================================================
std::vector<double> HOMARD_Boundary::GetLimit() const
{
  std::vector<double> mesLimit;
  mesLimit.push_back( _Xincr );
  mesLimit.push_back( _Yincr );
  mesLimit.push_back( _Zincr );
  return mesLimit;
}
//=============================================================================
void HOMARD_Boundary::AddGroup( const char* Group)
{
  _ListGroupSelected.push_back(Group);
}
//=============================================================================
void HOMARD_Boundary::SetGroups( const std::list<std::string>& ListGroup )
{
  _ListGroupSelected.clear();
  std::list<std::string>::const_iterator it = ListGroup.begin();
  while(it != ListGroup.end())
    _ListGroupSelected.push_back((*it++));
}
//=============================================================================
const std::list<std::string>& HOMARD_Boundary::GetGroups() const
{
  return _ListGroupSelected;
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Boundary::SetCaseCreation( const char* NomCasCreation )
{
  _NomCasCreation = std::string( NomCasCreation );
}
//=============================================================================
std::string HOMARD_Boundary::GetCaseCreation() const
{
  return _NomCasCreation;
}
//=============================================================================

//=============================================================================
/*!
 *  default constructor:
 *  Par defaut, l'adaptation est conforme, sans suivi de frontiere
 */
//=============================================================================
HOMARD_Cas::HOMARD_Cas():
  _Name(""), _NomDir("/tmp"), _ConfType(0), _ExtType(0)
{
  MESSAGE("HOMARD_Cas");
}
//=============================================================================
HOMARD_Cas::~HOMARD_Cas()
//=============================================================================
{
  MESSAGE("~HOMARD_Cas");
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Cas::SetName( const char* Name )
{
  _Name = std::string( Name );
}
//=============================================================================
std::string HOMARD_Cas::GetName() const
{
  return _Name;
}
//=============================================================================
std::string HOMARD_Cas::GetDumpPython() const
{
  std::ostringstream aScript;
  aScript << "\t" <<_Name << ".SetDirName(\"";
  aScript << _NomDir << "\")\n";
  aScript << "\t" <<_Name << ".SetConfType(";
  aScript << _ConfType << ")\n";
  aScript << "\t" <<_Name << ".SetExtType(";
  aScript << _ExtType << ")\n";
// Suivi de frontieres
  std::list<std::string>::const_iterator it = _ListBoundaryGroup.begin();
  while(it != _ListBoundaryGroup.end())
  {
    aScript << "\t" <<_Name << ".AddBoundaryGroup(\"";
    aScript << *it << "\", \"";
    it++;
    aScript << *it << "\")\n";
    it++;
  }
  if ( _Pyram > 0 )
  {
    aScript << "\t" <<_Name << ".SetPyram(";
    aScript << _Pyram << ")\n";
  }

  return aScript.str();
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
int HOMARD_Cas::SetDirName( const char* NomDir )
{
//   MESSAGE("SetDirName,  NomDir : "<<NomDir);
//   MESSAGE("SetDirName, _NomDir : "<<_NomDir);
  int erreur = 0 ;
  // On vérifie qu'aucun calcul n'a eu lieu pour ce cas
//   MESSAGE("SetDirName, _ListIter.size() : "<<_ListIter.size());
  if ( _ListIter.size() > 1 ) { erreur = 1 ; }
  // Creation
  if ( CHDIR(NomDir) == 0 )
  { _NomDir = std::string( NomDir ); }
  else
  {

#ifndef WIN32
    if ( mkdir(NomDir, S_IRWXU|S_IRGRP|S_IXGRP) == 0 )
#else
    if ( _mkdir(NomDir) == 0 )
#endif
    {
      if ( CHDIR(NomDir) == 0 ) { _NomDir = std::string( NomDir ); }
      else                      { erreur = 2 ; }
    }
    else { erreur = 2 ; }
  };
  return erreur ;
}
//=============================================================================
std::string HOMARD_Cas::GetDirName() const
{
  return _NomDir;
}
//=============================================================================
int HOMARD_Cas::GetNumberofIter()
{
  return _ListIter.size();
}
//
// Le type de conformite ou non conformite
//
//=============================================================================
void HOMARD_Cas::SetConfType( int Conftype )
{
//   VERIFICATION( (Conftype>=-2) && (Conftype<=3) );
  _ConfType = Conftype;
}
//=============================================================================
const int HOMARD_Cas::GetConfType() const
{
  return _ConfType;
}
//
// Le type exterieur
//
//=============================================================================
void HOMARD_Cas::SetExtType( int ExtType )
{
//   VERIFICATION( (ExtType>=0) && (ExtType<=2) );
  _ExtType = ExtType;
}
//=============================================================================
const int HOMARD_Cas::GetExtType() const
{
  return _ExtType;
}
//
// La boite englobante
//
//=============================================================================
void HOMARD_Cas::SetBoundingBox( const std::vector<double>& extremas )
{
  _Boite.clear();
  _Boite.resize( extremas.size() );
  for ( int i = 0; i < extremas.size(); i++ )
    _Boite[i] = extremas[i];
}
//=============================================================================
const std::vector<double>& HOMARD_Cas::GetBoundingBox() const
{
  return _Boite;
}
//
// Les groupes
//
//=============================================================================
void HOMARD_Cas::AddGroup( const char* Group )
{
  _ListGroup.push_back(Group);
}
//=============================================================================
void HOMARD_Cas::SetGroups( const std::list<std::string>& ListGroup )
{
  _ListGroup.clear();
  std::list<std::string>::const_iterator it = ListGroup.begin();
  while(it != ListGroup.end())
  {
    _ListGroup.push_back((*it++));
  }
}
//=============================================================================
const std::list<std::string>& HOMARD_Cas::GetGroups() const
{
  return _ListGroup;
}
//=============================================================================
void HOMARD_Cas::SupprGroups()
{
  _ListGroup.clear();
}
//
// Les frontieres
//
//=============================================================================
void HOMARD_Cas::AddBoundary( const char* Boundary )
{
//   MESSAGE ( ". HOMARD_Cas::AddBoundary : Boundary = " << Boundary );
  const char* Group = "";
  AddBoundaryGroup( Boundary, Group );
}
//=============================================================================
void HOMARD_Cas::AddBoundaryGroup( const char* Boundary, const char* Group )
{
//   MESSAGE ( ". HOMARD_Cas::AddBoundaryGroup : Boundary = " << Boundary );
//   MESSAGE ( ". HOMARD_Cas::AddBoundaryGroup : Group = " << Group );
  _ListBoundaryGroup.push_back( Boundary );
  _ListBoundaryGroup.push_back( Group    );
}
//=============================================================================
const std::list<std::string>& HOMARD_Cas::GetBoundaryGroup() const
{
  return _ListBoundaryGroup;
}
//=============================================================================
void HOMARD_Cas::SupprBoundaryGroup()
{
  _ListBoundaryGroup.clear();
}
//=============================================================================
void HOMARD_Cas::SetPyram( int Pyram )
{
  _Pyram = Pyram;
}
//=============================================================================
const int HOMARD_Cas::GetPyram() const
{
  return _Pyram;
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
std::string HOMARD_Cas::GetIter0Name() const
{
// Par construction de la liste, l'iteration a ete mise en tete.
  return (*(_ListIter.begin()));
}
//=============================================================================
void HOMARD_Cas::AddIteration( const char* NomIteration )
{
  _ListIter.push_back( std::string( NomIteration ) );
}
//=============================================================================
const std::list<std::string>& HOMARD_Cas::GetIterations() const
{
  return _ListIter;
}
//=============================================================================
void HOMARD_Cas::SupprIterations()
{
  _ListIter.clear();
}

//=============================================================================
//=============================================================================
HomardDriver::HomardDriver(const std::string siter, const std::string siterp1):
  _HOMARD_Exec( "" ), _NomDir( "" ), _NomFichierConfBase( "HOMARD.Configuration" ),
  _NomFichierConf( "" ), _NomFichierDonn( "" ), _siter( "" ), _siterp1( "" ),
  _Texte( "" ), _bLu( false )
{
  MESSAGE("siter = "<<siter<<", siterp1 = "<<siterp1);
// Le repertoire ou se trouve l'executable HOMARD
  std::string dir ;
  if ( getenv("HOMARD_REP_EXE_PRIVATE") != NULL ) { dir = getenv("HOMARD_REP_EXE_PRIVATE") ; }
  else                                            { dir = getenv("HOMARD_REP_EXE") ; }
  MESSAGE("dir ="<<dir);
// L'executable HOMARD
  std::string executable ;
  if ( getenv("HOMARD_EXE_PRIVATE") != NULL ) { executable = getenv("HOMARD_EXE_PRIVATE") ; }
  else                                        { executable = getenv("HOMARD_EXE") ; }
  MESSAGE("executable ="<<executable);
// Memorisation du nom complet de l'executable HOMARD
  _HOMARD_Exec = dir + "/" + executable ;
  MESSAGE("==> _HOMARD_Exec ="<<_HOMARD_Exec) ;
//
  _siter = siter ;
  _siterp1 = siterp1 ;
}
//=============================================================================
//=============================================================================
HomardDriver::~HomardDriver()
{
}
//===============================================================================
// A. Generalites
//===============================================================================
void HomardDriver::TexteInit( const std::string DirCompute, const std::string LogFile, const std::string Langue )
{
  MESSAGE("TexteInit, DirCompute ="<<DirCompute<<", LogFile ="<<LogFile);
//
  _Texte  = "ListeStd \"" + LogFile + "\"\n" ;
  _Texte += "RepeTrav \"" + DirCompute + "\"\n" ;
  _Texte += "RepeInfo \"" + DirCompute + "\"\n" ;
  _Texte += "Langue \"" + Langue + "\"\n" ;
//
}
//===============================================================================
void HomardDriver::TexteAdap( int ExtType )
{
  MESSAGE("TexteAdap");
//
  _Texte += "Action   homa\n" ;
  if ( ExtType ==  0 )      { _Texte += "CCAssoci med\n" ; }
  else if ( ExtType ==  1 ) { _Texte += "CCAssoci saturne\n" ; }
  else                      { _Texte += "CCAssoci saturne_2d\n" ; }
  _Texte += "ModeHOMA 1\n" ;
  _Texte += "NumeIter " + _siter + "\n" ;
  _modeHOMARD = 1 ;
//
}
//===============================================================================
void HomardDriver::TexteInfo( int TypeBila, int NumeIter )
{
  MESSAGE("TexteInfo: TypeBila ="<<TypeBila<<", NumeIter ="<<NumeIter);
//
  _Texte += "ModeHOMA 2\n" ;
  std::stringstream saux1 ;
  saux1 << TypeBila ;
  std::string saux2 = saux1.str() ;
  _Texte += "TypeBila " + saux2 + "\n" ;
  if ( NumeIter ==  0 )
  {
    _Texte += "NumeIter 0\n" ;
    _Texte += "Action   info_av\n" ;
    _Texte += "CCAssoci med\n" ;
  }
  else
  {
    _Texte += "NumeIter " + _siter + "\n" ;
    _Texte += "Action   info_ap\n" ;
    _Texte += "CCAssoci homard\n" ;
  }
  _modeHOMARD = 2 ;
//
}
//===============================================================================
void HomardDriver::TexteMajCoords( int NumeIter )
{
  MESSAGE("TexteMajCoords: NumeIter ="<<NumeIter);
//
  _Texte += "ModeHOMA 5\n" ;
  _Texte += "NumeIter " + _siterp1 + "\n" ;
  _Texte += "Action   homa\n" ;
  _Texte += "CCAssoci med\n" ;
  _Texte += "EcriFiHO N_SANS_FRONTIERE\n" ;
  _modeHOMARD = 5 ;
//
}
//===============================================================================
// B. Les maillages en entree et en sortie
//===============================================================================
void HomardDriver::TexteMaillage( const std::string NomMesh, const std::string MeshFile, int apres )
{
  MESSAGE("TexteMaillage, NomMesh  = "<<NomMesh);
  MESSAGE("TexteMaillage, MeshFile = "<<MeshFile);
  MESSAGE("TexteMaillage, apres = "<<apres);
  std::string saux ;
  saux = "P1" ;
  if ( apres < 1 ) { saux = "__" ; }

  _Texte += "# Maillages Med " + saux + "\n" ;
  _Texte += "CCNoMN" + saux + " \"" + NomMesh  + "\"\n" ;
  _Texte += "CCMaiN" + saux + " \"" + MeshFile + "\"\n" ;
}

//===============================================================================
void HomardDriver::TexteMaillageHOMARD( const std::string Dir, const std::string liter, int apres )
{
  MESSAGE("TexteMaillageHOMARD, Dir ="<<Dir<<", liter ="<<liter<<", apres ="<<apres);
  std::string saux ;
  if ( apres < 1 ) { saux = "__" ; }
  else             { saux = "P1" ; }

  _Texte += "# Maillage HOMARD " + liter + "\n" ;
  _Texte += "HOMaiN" + saux + " Mai" + liter   + " \"" + Dir + "/maill." + liter   + ".hom.med\"\n" ;
}

//===============================================================================
// C. Le pilotage de l'adaptation
//===============================================================================
void HomardDriver::TexteConfRaffDera( int ConfType, int TypeAdap, int TypeRaff, int TypeDera )
{
  MESSAGE("TexteConfRaffDera, ConfType ="<<ConfType);
  MESSAGE("TexteConfRaffDera, TypeAdap ="<<TypeAdap<<", TypeRaff ="<<TypeRaff<<", TypeDera ="<<TypeDera);
//
// Type de conformite
//
  std::string saux ;
  switch (ConfType)
  {
    case -2: //
    {
      saux = "NON_CONFORME_1_ARETE" ;
      break;
    }
    case -1: //
    {
      saux = "CONFORME_BOITES" ;
      break;
    }
    case 0: //
    {
      saux = "CONFORME" ;
      break;
    }
    case 1: //
    {
      saux = "NON_CONFORME" ;
      break;
    }
    case 2: //
    {
      saux = "NON_CONFORME_1_NOEUD" ;
      break;
    }
    case 3: //
    {
      saux = "NON_CONFORME_INDICATEUR" ;
      break;
    }
  }
  _Texte += "# Type de conformite\nTypeConf " + saux + "\n" ;
//
// Type de raffinement/deraffinement
//
  if ( TypeAdap == -1 )
  {
    if ( TypeRaff == 1 )
    {
      saux = "TypeRaff uniforme\n" ;
    }
    else
    {
      saux = "TypeRaff non\n" ;
    }
    if ( TypeDera == 1 )
    {
      saux += "TypeDera uniforme" ;
    }
    else
    {
      saux += "TypeDera non" ;
    }
  }
  else
  {
    if ( TypeRaff == 1 )
    {
      saux = "TypeRaff libre\n" ;
    }
    else
    {
      saux = "TypeRaff non\n" ;
    }
    if ( TypeDera == 1 )
    {
      saux += "TypeDera libre" ;
    }
    else
    {
      saux += "TypeDera non" ;
    }
  }
  _Texte += "# Type de raffinement/deraffinement\n" + saux + "\n" ;
//
//   MESSAGE("A la fin de HomardDriver::TexteConfRaffDera, _Texte ="<<_Texte);
}
//===============================================================================
void HomardDriver::TexteCompo( int NumeComp, const std::string NomCompo)
{
  MESSAGE("TexteCompo, NumeComp = "<<NumeComp<<", NomCompo = "<<NomCompo);
  _Texte +="CCCoChaI \"" + NomCompo + "\"\n" ;
}
//===============================================================================
void HomardDriver::TexteZone( int NumeZone, int ZoneType, int TypeUse, double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7, double x8 )
{
  MESSAGE("TexteZone, NumeZone = "<<NumeZone<<", ZoneType = "<<ZoneType<<", TypeUse = "<<TypeUse);
  MESSAGE("TexteZone, coor = "<< x0<<","<<x1<< ","<< x2<< ","<< x3<<","<<x4<<","<<x5<<","<<x6<<","<<x7<<","<<x8);
//
  std::string saux, saux2 ;
//
// Type de zones
// On convertit le type de zone au sens du module HOMARD dans Salome, ZoneType, dans le
// type au sens de l'executable HOMARD, ZoneTypeHOMARD
// Attention a mettre le bon signe a ZoneTypeHOMARD :
//    >0 signifie que l'on raffinera les mailles contenues dans la zone,
//    <0 signifie que l'on deraffinera
//
  int ZoneTypeHOMARD ;
  if ( ZoneType >= 11 && ZoneType <= 13 ) { ZoneTypeHOMARD = 1 ; }
  else if ( ZoneType >= 31 && ZoneType <= 33 ) { ZoneTypeHOMARD = 3 ; }
  else if ( ZoneType >= 61 && ZoneType <= 63 ) { ZoneTypeHOMARD = 6 ; }
  else { ZoneTypeHOMARD = ZoneType ; }
//
  if ( TypeUse < 0 ) { ZoneTypeHOMARD = -ZoneTypeHOMARD ; }
//
  std::stringstream saux1 ;
  saux1 << NumeZone ;
  saux = "#\n# Zone numero " + saux1.str() + "\n" ;
//
  { std::stringstream saux1 ;
    saux1 << NumeZone << " " << ZoneTypeHOMARD ;
    saux += "ZoRaType " + saux1.str() + "\n" ;
  }
//
// Cas du rectangle
//
  if ( ZoneType == 11 ) // Z est constant X Homard <=> X Salome
//                                        Y Homard <=> Y Salome
  {
    saux += "#Rectangle\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaXmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaYmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaYmax " + saux1.str() + "\n" ;
    }
  }
//
  else if ( ZoneType == 12 ) // X est constant X Homard <=> Y Salome
//                                             Y Homard <=> Z Salome
  {
    saux += "#Rectangle\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaXmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaXmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x4 ;
      saux += "ZoRaYmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x5 ;
      saux += "ZoRaYmax " + saux1.str() + "\n" ;
    }
  }
//
  else if ( ZoneType == 13 ) // Y est constant X Homard <=> X Salome
//                                             Y Homard <=> Z Salome
  {
    saux += "#Rectangle\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaXmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x4 ;
      saux += "ZoRaYmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x5 ;
      saux += "ZoRaYmax " + saux1.str() + "\n" ;
    }
  }
//
// Cas du parallelepipede
//
  else if ( ZoneType == 2 )
  {
    saux += "# Boite\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaXmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaYmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaYmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x4 ;
      saux += "ZoRaZmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x5 ;
      saux += "ZoRaZmax " + saux1.str() + "\n" ;
    }
  }
//
// Cas du disque
//
  else if ( ZoneType == 31 || ZoneType == 61 )
  {
    saux += "# Sphere\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x6 ;
      saux2 = saux1.str() ;
      if ( ZoneType == 61 ) { saux += "ZoRaRayE " + saux2 + "\n" ; }
      else                  { saux += "ZoRaRayo " + saux2 + "\n" ; }
    }
    if ( ZoneType == 61 )
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x8 ;
      saux += "ZoRaRayI " + saux1.str() + "\n" ;
    }
  }
  else if ( ZoneType == 32 || ZoneType == 62 )
  {
    saux += "# Sphere\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x6 ;
      saux2 = saux1.str() ;
      if ( ZoneType == 62 ) { saux += "ZoRaRayE " + saux2 + "\n" ; }
      else                  { saux += "ZoRaRayo " + saux2 + "\n" ; }
    }
    if ( ZoneType == 62 )
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x8 ;
      saux += "ZoRaRayI " + saux1.str() + "\n" ;
    }
  }
  else if ( ZoneType == 33 || ZoneType == 63 )
  {
    saux += "# Sphere\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x6 ;
      saux2 = saux1.str() ;
      if ( ZoneType == 63 ) { saux += "ZoRaRayE " + saux2 + "\n" ; }
      else                  { saux += "ZoRaRayo " + saux2 + "\n" ; }
    }
    if ( ZoneType == 63 )
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x8 ;
      saux += "ZoRaRayI " + saux1.str() + "\n" ;
    }
  }
//
// Cas de la sphere
//
  else if ( ZoneType == 4 )
  {
    saux += "# Sphere\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaRayo " + saux1.str() + "\n" ;
    }
  }
//
// Cas du cylindre ou du tuyau
//
  else if ( ZoneType == 5 || ZoneType == 7 )
  {
    if ( ZoneType == 5 ) { saux += "# Cylindre\n" ; }
    else                 { saux += "# Tuyau\n" ; }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXBas " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaYBas " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaZBas " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaXAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x4 ;
      saux += "ZoRaYAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x5 ;
      saux += "ZoRaZAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x6 ;
      saux2 = saux1.str() ;
     if ( ZoneType == 5 ) { saux += "ZoRaRayo " + saux2 + "\n" ; }
     else                 { saux += "ZoRaRayE " + saux2 + "\n" ; }
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x7 ;
      saux += "ZoRaHaut " + saux1.str() + "\n" ;
    }
    if ( ZoneType == 7 )
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x8 ;
      saux += "ZoRaRayI " + saux1.str() + "\n" ;
    }
  }
//
  _Texte += saux + "#\n" ;
//
//   MESSAGE("A la fin de HomardDriver::TexteZone, _Texte ="<<_Texte);
}
//===============================================================================
void HomardDriver::TexteField( const std::string FieldName, const std::string FieldFile, int TimeStep, int Rank,
               int TypeThR, double ThreshR, int TypeThC, double ThreshC,
               int UsField, int UsCmpI )
{
  MESSAGE("TexteField, FieldName = "<<FieldName<<", FieldFile = "<<FieldFile);
  MESSAGE("TexteField, TimeStep = "<<TimeStep<<", Rank = "<<Rank);

  std::string saux, saux2 ;
//
//
  _Texte += "# Champ d'indicateurs\n" ;
  _Texte += "CCIndica \"" + FieldFile  + "\"\n" ;
  _Texte += "CCNoChaI \"" + FieldName  + "\"\n" ;

// Cas ou on prend le dernier pas de temps
  if ( TimeStep == -2 )
  { _Texte += "CCNumPTI Last\n" ; }
// Cas avec pas de temps
  else if ( TimeStep >= 0 )
  {
    {
      std::stringstream saux1 ;
      saux1 << TimeStep ;
      saux2 = saux1.str() ;
      _Texte += "CCNumPTI " + saux2  + "\n" ;
    }
    if ( Rank >= 0 )
    {
      std::stringstream saux1 ;
      saux1 << Rank ;
      saux2 = saux1.str() ;
      _Texte += "CCNumOrI " + saux2  + "\n" ;
    }
  }
//
  saux = " " ;
  if ( TypeThR == 1 )
  { saux = "Hau" ; }
  if ( TypeThR == 2 )
  { saux = "HRe" ; }
  if ( TypeThR == 3 )
  { saux = "HPE" ; }
  if ( TypeThR == 4 )
  { saux = "HMS" ; }
  if ( saux != " " )
  {
    std::stringstream saux1 ;
    saux1 << ThreshR ;
    _Texte += "Seuil" + saux + " " + saux1.str()  + "\n" ;
  }
//
  saux = " " ;
  if ( TypeThC == 1 )
  { saux = "Bas" ; }
  if ( TypeThC == 2 )
  { saux = "BRe" ; }
  if ( TypeThC == 3 )
  { saux = "BPE" ; }
  if ( TypeThC == 4 )
  { saux = "BMS" ; }
  if ( saux != " " )
  {
    std::stringstream saux1 ;
    saux1 << ThreshC ;
    _Texte += "Seuil" + saux + " " + saux1.str()  + "\n" ;
  }
//
  saux = " " ;
  if ( UsField == 0 )
  { saux = "MAILLE" ; }
  if ( UsField == 1 )
  { saux = "SAUT" ; }
  if ( saux != " " )
  {
    _Texte += "CCModeFI " + saux  + "\n" ;
  }
//
  saux = " " ;
  if ( UsCmpI == 0 )
  { saux = "L2" ; }
  if ( UsCmpI == 1 )
  { saux = "INFINI" ; }
  if ( UsCmpI == 2 )
  { saux = "RELATIF" ; }
  if ( saux != " " )
  {
    _Texte += "CCUsCmpI " + saux  + "\n" ;
  }
}
//===============================================================================
void HomardDriver::TexteGroup( const std::string GroupName )
{
  MESSAGE("TexteGroup, GroupName = "<<GroupName);
//
  _Texte += "CCGroAda \"" + GroupName  + "\"\n" ;
//
}
//===============================================================================
// D. Les frontieres
//===============================================================================
void HomardDriver::TexteBoundaryOption( int BoundaryOption )
{
  MESSAGE("TexteBoundaryOption, BoundaryOption = "<<BoundaryOption);
//
// Type de suivi de frontiere
//
  std::stringstream saux1 ;
  saux1 << BoundaryOption ;
  std::string saux = saux1.str() ;
  _Texte += "SuivFron " + saux + "\n" ;
//
}//===============================================================================
void HomardDriver::TexteBoundaryCAOGr(  const std::string GroupName )
{
  MESSAGE("TexteBoundaryCAOGr, GroupName  = "<<GroupName);
//
  _Texte += "GrFroCAO \"" + GroupName + "\"\n" ;
//
}

//===============================================================================
void HomardDriver::TexteBoundaryDi(  const std::string MeshName, const std::string MeshFile )
{
  MESSAGE("TexteBoundaryDi, MeshName  = "<<MeshName);
  MESSAGE("TexteBoundaryDi, MeshFile  = "<<MeshFile);
//
  _Texte += "#\n# Frontiere discrete\n" ;
  _Texte += "CCNoMFro \"" + MeshName + "\"\n" ;
  _Texte += "CCFronti \"" + MeshFile + "\"\n" ;
//
}
//===============================================================================
void HomardDriver::TexteBoundaryDiGr(  const std::string GroupName )
{
  MESSAGE("TexteBoundaryDiGr, GroupName  = "<<GroupName);
//
  _Texte += "CCGroFro \"" + GroupName + "\"\n" ;
//
}
//===============================================================================
void HomardDriver::TexteBoundaryAn( const std::string NameBoundary, int NumeBoundary, int BoundaryType, double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7 )
{
  MESSAGE("TexteBoundaryAn, NameBoundary = "<<NameBoundary);
//   MESSAGE("TexteBoundaryAn, NumeBoundary = "<<NumeBoundary);
  MESSAGE("TexteBoundaryAn, BoundaryType = "<<BoundaryType);
//   MESSAGE("TexteBoundaryAn, coor         = "<< x0<<","<<x1<< ","<< x2<< ","<< x3<<","<<x4<<","<<x5<<","<<x6","<<x7);
//
  std::string saux, saux2 ;
//
// Commentaires
//
  std::stringstream saux1 ;
  saux1 << NumeBoundary ;
  saux2 = saux1.str() ;
  saux = "#\n# Frontiere numero " + saux2 + "\n" ;
  if ( BoundaryType == 1 )
  { saux += "# Cylindre\n" ; }
  if ( BoundaryType == 2 )
  { saux += "# Sphere\n" ; }
  if ( BoundaryType == 3 || BoundaryType == 4 )
  { saux += "# Cone\n" ; }
  if ( BoundaryType == 5 )
  { saux += "# Tore\n" ; }
//
// Le nom de la frontiere
//
  { std::stringstream saux1 ;
    saux1 << NumeBoundary ;
    saux += "FANom " + saux1.str() + " \"" + NameBoundary + "\"\n" ;
  }
//
// Type de frontiere
//
  { std::stringstream saux1 ;
    saux1 << NumeBoundary << " " << BoundaryType ;
    saux += "FAType " + saux1.str() + "\n" ;
  }
//
// Cas du cylindre
//
  if ( BoundaryType == 1 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux2 = saux1.str() ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FAXAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x4 ;
      saux += "FAYAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x5 ;
      saux += "FAZAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x6 ;
      saux += "FARayon " + saux1.str() + "\n" ;
    }
 }
//
// Cas de la sphere
//
  else if ( BoundaryType == 2 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FARayon " + saux1.str() + "\n" ;
    }
  }
//
// Cas du cone defini par un axe et un angle
//
  if ( BoundaryType == 3 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux += "FAXAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FAAngle " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x4 ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x5 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x6 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
 }
//
// Cas du cone defini par les 2 rayons
//
  if ( BoundaryType == 4 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FARayon " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x4 ;
      saux += "FAXCen2 " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x5 ;
      saux += "FAYCen2 " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x6 ;
      saux += "FAZCen2 " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x7 ;
      saux += "FARayon2 " + saux1.str() + "\n" ;
    }
 }
//
// Cas du tore
//
  if ( BoundaryType == 5 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux2 = saux1.str() ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FAXAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x4 ;
      saux += "FAYAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x5 ;
      saux += "FAZAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x6 ;
      saux += "FARayon  " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x7 ;
      saux += "FARayon2 " + saux1.str() + "\n" ;
    }
 }
//
  _Texte += saux + "#\n" ;
//
}
//===============================================================================
void HomardDriver::TexteBoundaryAnGr( const std::string NameBoundary, int NumeBoundary, const std::string GroupName )
{
  MESSAGE("TexteBoundaryAnGr, NameBoundary  = "<<NameBoundary);
//   MESSAGE("TexteBoundaryAnGr, NumeBoundary  = "<<NumeBoundary);
//   MESSAGE("TexteBoundaryAnGr, GroupName  = "<<GroupName);
//
// Commentaires
//
  std::string saux, saux2 ;
  std::stringstream saux1 ;
  saux1 << NumeBoundary ;
  saux2 = saux1.str() ;
  saux = "#\n# Lien Frontiere/Groupe numero " + saux2 + "\n" ;
//
  saux += "FGNomFro " + saux2 + " \"" + NameBoundary + "\"\n" ;
  saux += "FGNomGro " + saux2 + " \"" + GroupName + "\"\n" ;
//
  _Texte += saux + "#\n" ;
//
}
//===============================================================================
// E. Les interpolations
//===============================================================================
// Les fichiers d'entree et de sortie des champs a interpoler
void HomardDriver::TexteFieldInterp( const std::string FieldFile, const std::string MeshFile )
{
  MESSAGE("TexteFieldInterp, FieldFile = "<<FieldFile<<", MeshFile = "<<MeshFile);
//
  _Texte += "#\n# Interpolations des champs\n" ;
//
// Fichier en entree
  _Texte += "CCSolN__ \"" + FieldFile + "\"\n" ;
// Fichier en sortie
  _Texte += "CCSolNP1 \"" + MeshFile  + "\"\n" ;
//
//  std::cerr << "A la fin de TexteFieldInterp _Texte ="<<_Texte << std::endl;
}
//===============================================================================
// Tous les champs sont a interpoler
void HomardDriver::TexteFieldInterpAll( )
{
  MESSAGE("TexteFieldInterpAll");
//
  _Texte += "CCChaTou oui\n" ;
}
//===============================================================================
// Ecrit les caracteristiques de chaque interpolation sous la forme :
//   CCChaNom 1 "DEPL"     ! Nom du 1er champ a interpoler
//   CCChaTIn 1 0          ! Mode d'interpolation : automatique
//   CCChaNom 2 "VOLUME"   ! Nom du 2nd champ a interpoler
//   CCChaTIn 2 1          ! Mode d'interpolation : une variable extensive
//   CCChaPdT 2 14         ! Pas de temps 14
//   CCChaNuO 2 14         ! Numero d'ordre 14
//   etc.
//
// NumeChamp : numero d'ordre du champ a interpoler
// FieldName : nom du champ
// TypeInterp : type d'interpolation
// TimeStep : pas de temps retenu (>0 si pas de precision)
// Rank : numero d'ordre retenu
//
void HomardDriver::TexteFieldInterpNameType( int NumeChamp, const std::string FieldName, const std::string TypeInterp, int TimeStep, int Rank)
{
  MESSAGE("TexteFieldInterpNameType, NumeChamp = "<<NumeChamp<<", FieldName = "<<FieldName<<", TypeInterp = "<<TypeInterp);
  MESSAGE("TexteFieldInterpNameType, TimeStep = "<<TimeStep<<", Rank = "<<Rank);
// Numero d'ordre du champ a interpoler
  std::stringstream saux1 ;
  saux1 << NumeChamp ;
  std::string saux = saux1.str() ;
// Nom du champ
  _Texte +="CCChaNom " + saux + " \"" + FieldName + "\"\n" ;
// Type d'interpolation pour le champ
  _Texte +="CCChaTIn " + saux + " " + TypeInterp + "\n" ;
//
  if ( TimeStep >= 0 )
  {
    {
      std::stringstream saux1 ;
      saux1 << TimeStep ;
      _Texte += "CCChaPdT " + saux + " " + saux1.str()  + "\n" ;
    }
    {
      std::stringstream saux1 ;
      saux1 << Rank ;
      _Texte += "CCChaNuO " + saux + " " + saux1.str()  + "\n" ;
    }
  }
}
//===============================================================================
// F. Les options avancees
//===============================================================================
void HomardDriver::TexteAdvanced( int Pyram, int NivMax, double DiamMin, int AdapInit, int ExtraOutput )
{
  MESSAGE("TexteAdvanced, Pyram ="<<Pyram<<", NivMax ="<<NivMax<<", DiamMin ="<<DiamMin<<", AdapInit ="<<AdapInit<<", ExtraOutput ="<<ExtraOutput);

  if ( Pyram > 0 )
  {
    _Texte += "# Autorisation de pyramides dans le maillage initial\n" ;
    _Texte += "TypeElem ignore_pyra\n" ;
  }
  if ( NivMax > 0 )
  {
    _Texte += "# Niveaux extremes\n" ;
    { std::stringstream saux1 ;
      saux1 << NivMax ;
      _Texte += "NiveauMa " + saux1.str() + "\n" ;
    }
  }
  if ( DiamMin > 0 )
  {
    _Texte += "# Diametre minimal\n" ;
    { std::stringstream saux1 ;
      saux1 << DiamMin ;
      _Texte += "DiametMi " + saux1.str()  + "\n" ;
    }
  }
  if ( AdapInit != 0 )
  {
    if ( AdapInit > 0 )
    { _Texte += "# Raffinement" ; }
    else
    { _Texte += "# Deraffinement" ; }
    _Texte += " des regions sans indicateur\n" ;
    { std::stringstream saux1 ;
      saux1 << AdapInit ;
      _Texte += "AdapInit " + saux1.str() + "\n" ;
    }
  }
  if ( ExtraOutput % 2 == 0 )
  {
    _Texte += "# Sortie des niveaux de raffinement\n" ;
    _Texte += "NCNiveau NIVEAU\n" ;
  }
  if ( ExtraOutput % 3 == 0 )
  {
    _Texte += "# Sortie des qualités des mailles\n" ;
    _Texte += "NCQualit QUAL\n" ;
  }
  if ( ExtraOutput % 5 == 0 )
  {
    _Texte += "# Sortie des diamètres des mailles\n" ;
    _Texte += "NCDiamet DIAM\n" ;
  }
  if ( ExtraOutput % 7 == 0 )
  {
    _Texte += "# Sortie des parents des mailles\n" ;
    _Texte += "NCParent PARENT\n" ;
  }
  if ( ExtraOutput % 11 == 0 )
  {
    _Texte += "# Volumes voisins par recollement\n" ;
    _Texte += "NCVoisRc Voisin-Recollement\n" ;
  }
}
//===============================================================================
// G. Les messages
//===============================================================================
void HomardDriver::TexteInfoCompute( int MessInfo )
{
  MESSAGE("TexteAdvanced, MessInfo ="<<MessInfo);

  if ( MessInfo != 0 )
  {
     _Texte += "# Messages d'informations\n" ;
    { std::stringstream saux1 ;
      saux1 << MessInfo ;
      _Texte += "MessInfo " + saux1.str()  + "\n" ;
    }
   }
}
//===============================================================================
void HomardDriver::CreeFichier( )
{
//
  if ( _modeHOMARD == 1 )
  { _NomFichierConf = _NomFichierConfBase + "." + _siter + ".vers." + _siterp1 ; }
  else if ( _modeHOMARD == 2 )
  { _NomFichierConf = _NomFichierConfBase + "." + _siter + ".info" ; }
  else if ( _modeHOMARD == 5 )
  { _NomFichierConf = _NomFichierConfBase + ".majc" ; }
//
  std::ofstream Fic(_NomFichierConf.c_str(), std::ios::out ) ;
  if (Fic.is_open() == true) { Fic << _Texte << std::endl ; }
  Fic.close() ;
//
}
//===============================================================================
// Creation du fichier de donnees pour l'information
//===============================================================================
void HomardDriver::CreeFichierDonn( )
{
//
  MESSAGE("CreeFichierDonn");
  _NomFichierDonn = "info.donn" ;
//
  std::string data ;
  data  = "0\n" ;
  data += "0\n" ;
  data += "q\n" ;
  std::ofstream Fic(_NomFichierDonn.c_str(), std::ios::out ) ;
  if (Fic.is_open() == true) { Fic << data << std::endl ; }
  Fic.close() ;
//
}
//===============================================================================
int HomardDriver::ExecuteHomard(int option)
{
  MESSAGE("ExecuteHomard, avec option = "<<option);
  std::string commande ;
  int codret ;
// Copie des Fichiers HOMARD
  commande = "cp " + _NomFichierConf + " " + _NomFichierConfBase ;
  codret = system(commande.c_str()) ;

// Execution de HOMARD
  if ( codret == 0)
  {
    commande = _HOMARD_Exec.c_str() ;
    if ( _NomFichierDonn != "" ) { commande += " < " + _NomFichierDonn ; }
    codret = system(commande.c_str());
    if ( codret != 0) { MESSAGE ( "Erreur en executant HOMARD : " << codret ); };
    _NomFichierDonn = "" ;
  };
  return codret ;
}

//=============================================================================
//=============================================================================
HOMARD_Gen::HOMARD_Gen()
{
  MESSAGE("HOMARD_Gen");
}

//=============================================================================
//=============================================================================
HOMARD_Gen::~HOMARD_Gen()
{
  MESSAGE("~HOMARD_Gen");
}
//=============================================================================

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================
HOMARD_Hypothesis::HOMARD_Hypothesis():
  _Name(""), _NomCasCreation(""),
  _TypeAdap(-1), _TypeRaff(0), _TypeDera(0),
  _Field(""),
  _TypeThR(0), _ThreshR(0),
  _TypeThC(0), _ThreshC(0),
  _UsField(0), _UsCmpI(0),  _TypeFieldInterp(0)
{
  MESSAGE("HOMARD_Hypothesis");
}

//=============================================================================
/*!
 */
//=============================================================================
HOMARD_Hypothesis::~HOMARD_Hypothesis()
{
  MESSAGE("~HOMARD_Hypothesis");
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Hypothesis::SetName( const char* Name )
{
  _Name = std::string( Name );
}
//=============================================================================
std::string HOMARD_Hypothesis::GetName() const
{
  return _Name;
}
//=============================================================================
std::string HOMARD_Hypothesis::GetDumpPython() const
{
  std::ostringstream aScript;
  aScript << "\n# Creation of the hypothesis " << _Name << "\n" ;
  aScript << "\t" << _Name << " = homard.CreateHypothesis(\"" << _Name << "\")\n";
  if ( _TypeAdap == -1 )
  {
    int TypeRaffDera ;
    if ( _TypeRaff == 1 ) { TypeRaffDera = 1 ; }
    else                  { TypeRaffDera = -1 ; }
    aScript << "\t" << _Name << ".SetUnifRefinUnRef(" << TypeRaffDera << ")\n";
  }

// Raffinement selon des zones geometriques
  std::list<std::string>::const_iterator it = _ListZone.begin();
  int TypeUse ;
  while(it != _ListZone.end())
  {
      aScript << "\t" << _Name << ".AddZone(\"" << *it;
      it++;
      if ( *it == "1" ) { TypeUse =  1 ; }
      else              { TypeUse = -1 ; }
      aScript << "\", " << TypeUse << ")\n";
      it++;
  }

// Raffinement selon un champ
  if ( _TypeAdap == 1 )
  {
    aScript << "\t" << _Name << ".SetField(\"" << _Field << "\")\n";
    aScript << "\t" << _Name << ".SetUseField(" << _UsField << ")\n";
    aScript << "\t" << _Name << ".SetUseComp(" << _UsCmpI << ")\n";
    std::list<std::string>::const_iterator it_comp = _ListComp.begin();
    while(it_comp != _ListComp.end())
    {
      aScript << "\t" << _Name << ".AddComp(\"" << *it_comp << "\")\n";
      it_comp++;
    }
    if ( _TypeRaff == 1 )
    {
      aScript << "\t" << _Name << ".SetRefinThr(" << _TypeThR << ", " << _ThreshR << ")\n";
    }
    if ( _TypeDera == 1 )
    {
      aScript << "\t" << _Name << ".SetUnRefThr(" << _TypeThC << ", " << _ThreshC << ")\n";
    }
  }

// Filtrage du raffinement par des groupes
   for ( it=_ListGroupSelected.begin(); it!=_ListGroupSelected.end();it++)
       aScript << "\t" << _Name << ".AddGroup(\""  << (*it) <<  "\")\n" ;

// Interpolation des champs
  if ( _TypeFieldInterp == 2 )
  {
    std::list<std::string>::const_iterator it_champ = _ListFieldInterp.begin();
    while(it_champ != _ListFieldInterp.end())
    {
      aScript << "\t" << _Name << ".AddFieldInterpType( \"" << *it_champ  <<  "\" " ;
      it_champ++;
      aScript << ", " << *it_champ << ")\n";
      it_champ++;
    }
  }
  else if ( _TypeFieldInterp != 0 )
  {
    aScript << "\t" << _Name << ".SetTypeFieldInterp(" << _TypeFieldInterp << ")\n";
  }
  if ( _NivMax > 0 )
  {
    aScript << "\t" <<_Name << ".SetNivMax(" << _NivMax << ")\n";
  }
  if ( _DiamMin > 0 )
  {
    aScript << "\t" <<_Name << ".SetDiamMin(" << _DiamMin << ")\n";
  }
  if ( _AdapInit != 0 )
  {
    aScript << "\t" <<_Name << ".SetAdapInit(" << _AdapInit << ")\n";
  }
  if ( _ExtraOutput != 1 )
  {
    aScript << "\t" <<_Name << ".SetExtraOutput(" << _ExtraOutput << ")\n";
  }

  return aScript.str();
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Hypothesis::SetAdapType( int TypeAdap )
{
  VERIFICATION( (TypeAdap>=-1) && (TypeAdap<=1) );
  _TypeAdap = TypeAdap;
}
//=============================================================================
int HOMARD_Hypothesis::GetAdapType() const
{
  return _TypeAdap;
}
//=============================================================================
void HOMARD_Hypothesis::SetRefinTypeDera( int TypeRaff, int TypeDera )
{
  VERIFICATION( (TypeRaff>=-1) && (TypeRaff<=1) );
  _TypeRaff = TypeRaff;
  VERIFICATION( (TypeDera>=-1) && (TypeDera<=1) );
  _TypeDera = TypeDera;
}
//=============================================================================
int HOMARD_Hypothesis::GetRefinType() const
{
  return _TypeRaff;
}
//=============================================================================
int HOMARD_Hypothesis::GetUnRefType() const
{
  return _TypeDera;
}
//=============================================================================
void HOMARD_Hypothesis::SetField( const char* FieldName )
{
  _Field = std::string( FieldName );
  MESSAGE( "SetField : FieldName = " << FieldName );
}
//=============================================================================
std::string HOMARD_Hypothesis::GetFieldName() const
{
  return _Field;
}
//=============================================================================
void HOMARD_Hypothesis::SetUseField( int UsField )
{
  VERIFICATION( (UsField>=0) && (UsField<=1) );
  _UsField = UsField;
}
//=============================================================================
int HOMARD_Hypothesis::GetUseField() const
{
  return _UsField;
}
//=============================================================================
void HOMARD_Hypothesis::SetUseComp( int UsCmpI )
{
  MESSAGE ("SetUseComp pour UsCmpI = "<<UsCmpI) ;
  VERIFICATION( (UsCmpI>=0) && (UsCmpI<=2) );
  _UsCmpI = UsCmpI;
}
//=============================================================================
int HOMARD_Hypothesis::GetUseComp() const
{
  return _UsCmpI;
}
//=============================================================================
void HOMARD_Hypothesis::AddComp( const char* NomComp )
{
// On commence par supprimer la composante au cas ou elle aurait deja ete inseree
// Cela peut se produire dans un schema YACS quand on repasse plusieurs fois par la
// definition de l'hypothese
  SupprComp( NomComp ) ;
// Insertion veritable
  _ListComp.push_back( std::string( NomComp ) );
}
//=============================================================================
void HOMARD_Hypothesis::SupprComp( const char* NomComp )
{
  MESSAGE ("SupprComp pour "<<NomComp) ;
  std::list<std::string>::iterator it = find( _ListComp.begin(), _ListComp.end(), NomComp );
  if ( it != _ListComp.end() ) { it = _ListComp.erase( it ); }
}
//=============================================================================
void HOMARD_Hypothesis::SupprComps()
{
  _ListComp.clear();
}
//=============================================================================
const std::list<std::string>& HOMARD_Hypothesis::GetComps() const
{
  return _ListComp;
}
//=============================================================================
void HOMARD_Hypothesis::SetRefinThr( int TypeThR, double ThreshR )
{
  MESSAGE( "SetRefinThr : TypeThR = " << TypeThR << ", ThreshR = " << ThreshR );
  VERIFICATION( (TypeThR>=0) && (TypeThR<=4) );
  _TypeThR = TypeThR;
  _ThreshR = ThreshR;
}
//=============================================================================
int HOMARD_Hypothesis::GetRefinThrType() const
{
  return _TypeThR;
}
//=============================================================================
double HOMARD_Hypothesis::GetThreshR() const
{
  return _ThreshR;
}
//=============================================================================
void HOMARD_Hypothesis::SetUnRefThr( int TypeThC, double ThreshC )
{
  VERIFICATION( (TypeThC>=0) && (TypeThC<=4) );
  _TypeThC = TypeThC;
  _ThreshC = ThreshC;
}
//=============================================================================
int HOMARD_Hypothesis::GetUnRefThrType() const
{
  return _TypeThC;
}
//=============================================================================
double HOMARD_Hypothesis::GetThreshC() const
{
  return _ThreshC;
}
//=============================================================================
void HOMARD_Hypothesis::SetNivMax( int NivMax )
//=============================================================================
{
  _NivMax = NivMax;
}
//=============================================================================
const int HOMARD_Hypothesis::GetNivMax() const
//=============================================================================
{
  return _NivMax;
}
//=============================================================================
void HOMARD_Hypothesis::SetDiamMin( double DiamMin )
//=============================================================================
{
  _DiamMin = DiamMin;
}
//=============================================================================
const double HOMARD_Hypothesis::GetDiamMin() const
//=============================================================================
{
  return _DiamMin;
}
//=============================================================================
void HOMARD_Hypothesis::SetAdapInit( int AdapInit )
//=============================================================================
{
  _AdapInit = AdapInit;
}
//=============================================================================
const int HOMARD_Hypothesis::GetAdapInit() const
//=============================================================================
{
  return _AdapInit;
}
//=============================================================================
void HOMARD_Hypothesis::SetExtraOutput( int ExtraOutput )
//=============================================================================
{
  _ExtraOutput = ExtraOutput;
}
//=============================================================================
const int HOMARD_Hypothesis::GetExtraOutput() const
//=============================================================================
{
  return _ExtraOutput;
}
//=============================================================================
void HOMARD_Hypothesis::AddGroup( const char* Group)
{
// On commence par supprimer le groupe au cas ou il aurait deja ete insere
// Cela peut se produire dans un schema YACS quand on repasse plusieurs fois par la
// definition de l'hypothese
  SupprGroup( Group ) ;
// Insertion veritable
  _ListGroupSelected.push_back(Group);
}
//=============================================================================
void HOMARD_Hypothesis::SupprGroup( const char* Group )
{
  MESSAGE ("SupprGroup pour "<<Group) ;
  std::list<std::string>::iterator it = find( _ListGroupSelected.begin(), _ListGroupSelected.end(), Group );
  if ( it != _ListGroupSelected.end() ) { it = _ListGroupSelected.erase( it ); }
}
//=============================================================================
void HOMARD_Hypothesis::SupprGroups()
{
  _ListGroupSelected.clear();
}
//=============================================================================
void HOMARD_Hypothesis::SetGroups( const std::list<std::string>& ListGroup )
{
  _ListGroupSelected.clear();
  std::list<std::string>::const_iterator it = ListGroup.begin();
  while(it != ListGroup.end())
    _ListGroupSelected.push_back((*it++));
}
//=============================================================================
const std::list<std::string>& HOMARD_Hypothesis::GetGroups() const
{
  return _ListGroupSelected;
}
//=============================================================================
// Type d'interpolation des champs :
//   0 : aucun champ n'est interpole
//   1 : tous les champs sont interpoles
//   2 : certains champs sont interpoles
void HOMARD_Hypothesis::SetTypeFieldInterp( int TypeFieldInterp )
{
  VERIFICATION( (TypeFieldInterp>=0) && (TypeFieldInterp<=2) );
  _TypeFieldInterp = TypeFieldInterp;
}
//=============================================================================
int HOMARD_Hypothesis::GetTypeFieldInterp() const
{
  return _TypeFieldInterp;
}
//=============================================================================
void HOMARD_Hypothesis::AddFieldInterpType( const char* FieldInterp, int TypeInterp )
{
  MESSAGE ("Dans AddFieldInterpType pour " << FieldInterp << " et TypeInterp = " << TypeInterp) ;
// On commence par supprimer le champ au cas ou il aurait deja ete insere
// Cela peut se produire dans un schema YACS quand on repasse plusieurs fois par la
// definition de l'hypothese
  SupprFieldInterp( FieldInterp ) ;
// Insertion veritable
// . Nom du champ
  _ListFieldInterp.push_back( std::string( FieldInterp ) );
// . Usage du champ
  std::stringstream saux1 ;
  saux1 << TypeInterp ;
  _ListFieldInterp.push_back( saux1.str() );
// . Indication generale : certains champs sont a interpoler
  SetTypeFieldInterp ( 2 ) ;
}
//=============================================================================
void HOMARD_Hypothesis::SupprFieldInterp( const char* FieldInterp )
{
  MESSAGE ("Dans SupprFieldInterp pour " << FieldInterp) ;
  std::list<std::string>::iterator it = find( _ListFieldInterp.begin(), _ListFieldInterp.end(), FieldInterp ) ;
// Attention a supprimer le nom du champ et le type d'usage
  if ( it != _ListFieldInterp.end() )
  {
    it = _ListFieldInterp.erase( it ) ;
    it = _ListFieldInterp.erase( it ) ;
  }
// Decompte du nombre de champs restant a interpoler
  it = _ListFieldInterp.begin() ;
  int cpt = 0 ;
  while(it != _ListFieldInterp.end())
  {
    cpt += 1 ;
    (*it++);
  }
  MESSAGE("Nombre de champ restants = "<<cpt/2);
// . Indication generale : aucun champ ne reste a interpoler
  if ( cpt == 0 )
  {
    SetTypeFieldInterp ( 0 ) ;
  }
}
//=============================================================================
void HOMARD_Hypothesis::SupprFieldInterps()
{
  MESSAGE ("SupprFieldInterps") ;
  _ListFieldInterp.clear();
// . Indication generale : aucun champ ne reste a interpoler
  SetTypeFieldInterp ( 0 ) ;
}
//=============================================================================
const std::list<std::string>& HOMARD_Hypothesis::GetFieldInterps() const
{
  return _ListFieldInterp;
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Hypothesis::SetCaseCreation( const char* NomCasCreation )
{
  _NomCasCreation = std::string( NomCasCreation );
}
//=============================================================================
std::string HOMARD_Hypothesis::GetCaseCreation() const
{
  return _NomCasCreation;
}
//=============================================================================
void HOMARD_Hypothesis::LinkIteration( const char* NomIteration )
{
  _ListIter.push_back( std::string( NomIteration ) );
}
//=============================================================================
void HOMARD_Hypothesis::UnLinkIteration( const char* NomIteration )
{
  std::list<std::string>::iterator it = find( _ListIter.begin(), _ListIter.end(), NomIteration ) ;
  if ( it != _ListIter.end() )
  {
    MESSAGE ("Dans UnLinkIteration pour " << NomIteration) ;
    it = _ListIter.erase( it ) ;
  }
}
//=============================================================================
void HOMARD_Hypothesis::UnLinkIterations()
{
  _ListIter.clear();
}
//=============================================================================
const std::list<std::string>& HOMARD_Hypothesis::GetIterations() const
{
  return _ListIter;
}
//=============================================================================
void HOMARD_Hypothesis::AddZone( const char* NomZone, int TypeUse )
{
  MESSAGE ("Dans AddZone pour " << NomZone << " et TypeUse = " << TypeUse) ;
// On commence par supprimer la zone au cas ou elle aurait deja ete inseree
// Cela peut se produire dans un schema YACS quand on repasse plusieurs fois par la
// definition de l'hypothese
  SupprZone( NomZone ) ;
// Insertion veritable
// . Nom de la zone
  _ListZone.push_back( std::string( NomZone ) );
// . Usage de la zone
  std::stringstream saux1 ;
  saux1 << TypeUse ;
  _ListZone.push_back( saux1.str() );
}
//=============================================================================
void HOMARD_Hypothesis::SupprZone( const char* NomZone )
{
  MESSAGE ("Dans SupprZone pour " << NomZone) ;
  std::list<std::string>::iterator it = find( _ListZone.begin(), _ListZone.end(), NomZone );
// Attention a supprimer le nom de zone et le type d'usage
  if ( it != _ListZone.end() )
  {
    it = _ListZone.erase( it );
    it = _ListZone.erase( it );
  }
}
//=============================================================================
void HOMARD_Hypothesis::SupprZones()
{
  _ListZone.clear();
}
//=============================================================================
const std::list<std::string>& HOMARD_Hypothesis::GetZones() const
{
  return _ListZone;
}

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================
HOMARD_Iteration::HOMARD_Iteration():
  _Name( "" ), _Etat( 0 ),
 _NumIter( -1 ),
  _NomMesh( "" ), _MeshFile( "" ),
  _FieldFile( "" ), _TimeStep( -1 ), _Rank( -1 ),
  _LogFile( "" ),
  _IterParent( "" ),
  _NomHypo( "" ), _NomCas( "" ), _NomDir( "" ),
  _FileInfo( "" ),
 _MessInfo( 1 )
{
  MESSAGE("HOMARD_Iteration");
}
//=============================================================================
/*!
 *
 */
//=============================================================================
HOMARD_Iteration::~HOMARD_Iteration()
{
  MESSAGE("~HOMARD_Iteration");
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Iteration::SetName( const char* Name )
{
  _Name = std::string( Name );
}
//=============================================================================
std::string HOMARD_Iteration::GetName() const
{
  return _Name;
}
//=============================================================================
std::string HOMARD_Iteration::GetDumpPython() const
{
  if (_IterParent == "") return std::string(" ") ;   // Pas de creation explicite de iteration 0";

  MESSAGE (". Ecriture de l iteration " << _Name );
  std::ostringstream aScript;
  aScript << "\n# Creation of the iteration " << _Name << "\n";
  if( _NumIter == 1 )
  {
       aScript << "\t" << _Name << " = " << _NomCas << ".NextIteration(\"" << _Name << "\")\n";
  }
   else
  {
       aScript << "\t" << _Name << " = " << _IterParent << ".NextIteration(\"" << _Name << "\")\n";
  }
// L'hypothese (doit etre au debut)
  aScript << "\t" << _Name << ".AssociateHypo(\"" << _NomHypo << "\")\n";
// Le nom du maillage produit
//   MESSAGE (".. maillage produit " << _NomMesh );
  aScript << "\t" << _Name << ".SetMeshName(\"" << _NomMesh << "\")\n" ;
// Le fichier du maillage produit
  aScript << "\t" << _Name << ".SetMeshFile(\"" << _MeshFile << "\")\n";
// Le fichier des champs
  if ( _FieldFile != "" )
  {
    aScript << "\t" << _Name << ".SetFieldFile(\"" << _FieldFile << "\")\n";
  }
// Si champ de pilotage, valeurs de pas de temps
  MESSAGE (". champ de pilotage : _TimeStep = " << _TimeStep << ", _Rank : " << _Rank);
  if ( _TimeStep != -1 )
  {
    if ( _TimeStep == -2 ) {
      aScript << "\t" << _Name << ".SetTimeStepRankLast()\n";
    }
    else
    {
      if ( _TimeStep != -1 )
      {
        if ( _Rank == -1 )
        {
          aScript << "\t" << _Name << ".SetTimeStep( " << _TimeStep << " )\n";
        }
        else
        {
          aScript << "\t" << _Name << ".SetTimeStepRank( " << _TimeStep << ", " << _Rank << " )\n";
        }
      }
    }
  }
// Les instants d'interpolation
  MESSAGE (". instants d'interpolation ");
  std::list<std::string>::const_iterator it = _ListFieldInterpTSR.begin() ;
  while(it != _ListFieldInterpTSR.end())
  {
    std::string FieldName = std::string((*it)) ;
//     MESSAGE ("... FieldName = "<< FieldName);
    (*it++);
    std::string TimeStepstr = std::string((*it)) ;
//     MESSAGE ("... TimeStepstr = "<< TimeStepstr);
    (*it++);
    std::string Rankstr = std::string((*it)) ;
//     MESSAGE ("... Rankstr = "<< Rankstr);
    (*it++);
    aScript << "\t" << _Name << ".SetFieldInterpTimeStepRank( \"" << FieldName << "\"" ;
    aScript << ", " << TimeStepstr ;
    aScript << ", " << Rankstr << " )\n" ;
  }

// Compute
  MESSAGE (". Compute ");
  if ( _Etat == 2 ) { aScript << "\tcodret = "  <<_Name << ".Compute(1, 1)\n"; }
  else              { aScript << "\t#codret = " <<_Name << ".Compute(1, 1)\n"; }
//   MESSAGE (". Fin de l ecriture de l iteration " << _Name );

  return aScript.str();
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Iteration::SetDirNameLoc( const char* NomDir )
{
  _NomDir = std::string( NomDir );
}
//=============================================================================
std::string HOMARD_Iteration::GetDirNameLoc() const
{
   return _NomDir;
}
//=============================================================================
void HOMARD_Iteration::SetNumber( int NumIter )
{
  _NumIter = NumIter;
}
//=============================================================================
int HOMARD_Iteration::GetNumber() const
{
  return _NumIter;
}
//=============================================================================
void HOMARD_Iteration::SetState( int etat )
{
  _Etat = etat;
}
//=============================================================================
int HOMARD_Iteration::GetState() const
{
  return _Etat;
}
//=============================================================================
void HOMARD_Iteration::SetMeshName( const char* NomMesh )
{
  _NomMesh = std::string( NomMesh );
}
//=============================================================================
std::string HOMARD_Iteration::GetMeshName() const
{
  return _NomMesh;
}
//=============================================================================
void HOMARD_Iteration::SetMeshFile( const char* MeshFile )
{
  _MeshFile = std::string( MeshFile );
}
//=============================================================================
std::string HOMARD_Iteration::GetMeshFile() const
{
  return _MeshFile;
}
//=============================================================================
void HOMARD_Iteration::SetFieldFile( const char* FieldFile )
{
  _FieldFile = std::string( FieldFile );
}
//=============================================================================
std::string HOMARD_Iteration::GetFieldFile() const
{
  return _FieldFile;
}
//=============================================================================
// Instants pour le champ de pilotage
//=============================================================================
void HOMARD_Iteration::SetTimeStep( int TimeStep )
{
  _TimeStep = TimeStep;
}
//=============================================================================
void HOMARD_Iteration::SetTimeStepRank( int TimeStep, int Rank )
{
  _TimeStep = TimeStep;
  _Rank = Rank;
}
//=============================================================================
void HOMARD_Iteration::SetTimeStepRankLast()
{
  _TimeStep = -2;
}
//=============================================================================
int HOMARD_Iteration::GetTimeStep() const
{
  return _TimeStep;
}
//=============================================================================
int HOMARD_Iteration::GetRank() const
{
  return _Rank;
}
//=============================================================================
// Instants pour un champ a interpoler
//=============================================================================
void HOMARD_Iteration::SetFieldInterpTimeStep( const char* FieldInterp, int TimeStep )
{
  SetFieldInterpTimeStepRank( FieldInterp, TimeStep, TimeStep ) ;
}
//=============================================================================
void HOMARD_Iteration::SetFieldInterpTimeStepRank( const char* FieldInterp, int TimeStep, int Rank )
{
  MESSAGE("Champ " << FieldInterp << ", hypothese " << _NomHypo )
// Verification de la presence du champ dans l'hypothese
  std::list<std::string>::iterator it = find( _ListFieldInterp.begin(), _ListFieldInterp.end(), FieldInterp );
  if ( it == _ListFieldInterp.end() )
  {
    INFOS("Champ " << FieldInterp << " ; hypothese " << _NomHypo )
    VERIFICATION("Le champ est inconnu dans l'hypothese associee a cette iteration." == 0);
  }

// . Nom du champ
  _ListFieldInterpTSR.push_back( std::string( FieldInterp ) );
// . Pas de temps
  std::stringstream saux1 ;
  saux1 << TimeStep ;
  _ListFieldInterpTSR.push_back( saux1.str() );
// . Numero d'ordre
  std::stringstream saux2 ;
  saux2 << Rank ;
  _ListFieldInterpTSR.push_back( saux2.str() );
}
//=============================================================================
const std::list<std::string>& HOMARD_Iteration::GetFieldInterpsTimeStepRank() const
{
  return _ListFieldInterpTSR;
}
//=============================================================================
void HOMARD_Iteration::SetFieldInterp( const char* FieldInterp )
{
  _ListFieldInterp.push_back( std::string( FieldInterp ) );
}
//=============================================================================
const std::list<std::string>& HOMARD_Iteration::GetFieldInterps() const
{
  return _ListFieldInterp;
}
//=============================================================================
void HOMARD_Iteration::SupprFieldInterps()
{
  _ListFieldInterp.clear();
}
//=============================================================================
void HOMARD_Iteration::SetLogFile( const char* LogFile )
{
  _LogFile = std::string( LogFile );
}
//=============================================================================
std::string HOMARD_Iteration::GetLogFile() const
{
  return _LogFile;
}
//=============================================================================
void HOMARD_Iteration::SetFileInfo( const char* FileInfo )
{
  _FileInfo = std::string( FileInfo );
}
//=============================================================================
std::string HOMARD_Iteration::GetFileInfo() const
{
  return _FileInfo;
}
//=============================================================================
//=============================================================================
// Liens avec les autres iterations
//=============================================================================
//=============================================================================
void HOMARD_Iteration::LinkNextIteration( const char* NomIteration )
{
  _mesIterFilles.push_back( std::string( NomIteration ) );
}
//=============================================================================
void HOMARD_Iteration::UnLinkNextIteration( const char* NomIteration )
{
  std::list<std::string>::iterator it = find( _mesIterFilles.begin(), _mesIterFilles.end(), NomIteration ) ;
  if ( it != _mesIterFilles.end() )
  {
    MESSAGE ("Dans UnLinkNextIteration pour " << NomIteration) ;
    it = _mesIterFilles.erase( it ) ;
  }
}
//=============================================================================
void HOMARD_Iteration::UnLinkNextIterations()
{
  _mesIterFilles.clear();
}
//=============================================================================
const std::list<std::string>& HOMARD_Iteration::GetIterations() const
{
  return _mesIterFilles;
}
//=============================================================================
void HOMARD_Iteration::SetIterParentName( const char* IterParent )
{
  _IterParent = IterParent;
}
//=============================================================================
std::string HOMARD_Iteration::GetIterParentName() const
{
  return _IterParent;
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Iteration::SetCaseName( const char* NomCas )
{
  _NomCas = std::string( NomCas );
}
//=============================================================================
std::string HOMARD_Iteration::GetCaseName() const
{
  return _NomCas;
}
//=============================================================================
void HOMARD_Iteration::SetHypoName( const char* NomHypo )
{
  _NomHypo = std::string( NomHypo );
}
//=============================================================================
std::string HOMARD_Iteration::GetHypoName() const
{
  return _NomHypo;
}
//=============================================================================
//=============================================================================
// Divers
//=============================================================================
//=============================================================================
void HOMARD_Iteration::SetInfoCompute( int MessInfo )
{
  _MessInfo = MessInfo;
}
//=============================================================================
int HOMARD_Iteration::GetInfoCompute() const
{
  return _MessInfo;
}

} // namespace SMESHHOMARDImpl /end/
