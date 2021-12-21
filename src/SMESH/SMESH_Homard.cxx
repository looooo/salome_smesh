// SMESH HOMARD : implementation of SMESHHOMARD idl descriptions
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

////

#include <MCAuto.hxx>
#include <MEDCouplingMemArray.hxx>
#include <MEDFileMesh.hxx>

#include <XAO_Xao.hxx>
#include <XAO_BrepGeometry.hxx>
#include <XAO_Group.hxx>

#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <limits>

#include <fcntl.h>
#include <boost/filesystem.hpp>

#include <OSD_Parallel.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBndLib.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>
#include <ElCLib.hxx>
#include <ElSLib.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <gp_Circ.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Sphere.hxx>
#include <gp_Vec.hxx>

namespace boofs = boost::filesystem;

////

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

    std::vector<double> coor = cas.GetBoundingBox();
    os << separator() << coor.size();
    for ( unsigned int i = 0; i < coor.size(); i++ )
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

    os << separator() << 0; //cas.GetPyram()

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
      for ( unsigned int i = 0; i < coor.size(); i++ )
            os << separator() << coor[i];
      std::vector<double> limit = boundary.GetLimit();
      for ( unsigned int i = 0; i < limit.size(); i++ )
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
    //cas.SetPyram( atoi( chunk.c_str() ) );

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
  switch (_Type) {
    case -1:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryCAO(\"" << _Name << "\", ";
      aScript << "\"" << _DataFile << "\")\n";
      break ;
    }
    case 0:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryDi(\"" << _Name << "\", ";
      aScript << "\"" << _MeshName << "\", ";
      aScript << "\"" << _DataFile << "\")\n";
      break ;
    }
    case 1:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryCylinder(\"" << _Name << "\", ";
      aScript << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ", " << _Xaxe << ", " << _Yaxe << ", " << _Zaxe << ", " << _rayon << ")\n";
      break ;
    }
    case 2:
    {
      aScript << _Name << " = smeshhomard.CreateBoundarySphere(\"" << _Name << "\", ";
      aScript << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ", " << _rayon << ")\n";
      break ;
    }
    case 3:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryConeA(\"" << _Name << "\", ";
      aScript << _Xaxe << ", " << _Yaxe << ", " << _Zaxe << ", " << _Angle << ", " << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ")\n";
      break ;
    }
    case 4:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryConeR(\"" << _Name << "\", ";
      aScript << _Xcentre1 << ", " << _Ycentre1 << ", " << _Zcentre1 << ", " << _Rayon1 << ", " << _Xcentre2 << ", " << _Ycentre2 << ", " << _Zcentre2 << ", " << _Rayon2 << ")\n";
      break ;
    }
    case 5:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryTorus(\"" << _Name << "\", ";
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
  _Name(""), _NomDir("/tmp"), _ConfType(0)
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
  //aScript << _Name << ".SetDirName(\"" << _NomDir << "\")\n";
  aScript << _Name << ".SetConfType(" << _ConfType << ")\n";
  // Suivi de frontieres
  std::list<std::string>::const_iterator it = _ListBoundaryGroup.begin();
  while (it != _ListBoundaryGroup.end()) {
    aScript << _Name << ".AddBoundaryGroup(\"" << *it << "\", \"";
    it++;
    aScript << *it << "\")\n";
    it++;
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
// La boite englobante
//
//=============================================================================
void HOMARD_Cas::SetBoundingBox( const std::vector<double>& extremas )
{
  _Boite.clear();
  _Boite.resize( extremas.size() );
  for ( unsigned int i = 0; i < extremas.size(); i++ )
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
  // TODO?
  if ( getenv("HOMARD_ROOT_DIR") != NULL ) { dir = getenv("HOMARD_ROOT_DIR") ; }
  dir += "/bin/salome";
  MESSAGE("dir ="<<dir);
  // L'executable HOMARD
  std::string executable = "homard";
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
void HomardDriver::TexteAdap()
{
  MESSAGE("TexteAdap");

  _Texte += "Action   homa\n";
  _Texte += "CCAssoci med\n";
  _Texte += "ModeHOMA 1\n";
  _Texte += "NumeIter " + _siter + "\n";
  _modeHOMARD = 1;
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
void HomardDriver::TexteAdvanced( int NivMax, double DiamMin, int AdapInit, int ExtraOutput )
{
  MESSAGE("TexteAdvanced, NivMax ="<<NivMax<<", DiamMin ="<<DiamMin<<", AdapInit ="<<AdapInit<<", ExtraOutput ="<<ExtraOutput);

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
int HomardDriver::ExecuteHomard()
{
  MESSAGE("ExecuteHomard");
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
  _TypeThR(0), _TypeThC(0),
  _ThreshR(0), _ThreshC(0),
  _UsField(0), _UsCmpI(0), _TypeFieldInterp(0),

  _NivMax(-1), _DiamMin(-1.0), _AdapInit(0), _ExtraOutput(1)
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


/*!
 * \brief Relocate nodes to lie on geometry
 *  \param [in] theInputMedFile - a MED file holding a mesh including nodes that will be
 *         moved onto the geometry
 *  \param [in] theOutputMedFile - a MED file to create, that will hold a modified mesh
 *  \param [in] theInputNodeFiles - an array of names of files describing groups of nodes that
 *         will be moved onto the geometry
 *  \param [in] theXaoFileName - a path to a file in XAO format holding the geometry and
 *         the geometrical groups.
 *  \param [in] theIsParallel - if \c true, all processors are used to treat boundary shapes
 *          in parallel.
 */
void FrontTrack::track( const std::string&                 theInputMedFile,
                        const std::string&                 theOutputMedFile,
                        const std::vector< std::string > & theInputNodeFiles,
                        const std::string&                 theXaoFileName,
                        bool                               theIsParallel )
{
  // check arguments
#ifdef _DEBUG_
  std::cout << "FrontTrack::track" << std::endl;
#endif

  if ( theInputNodeFiles.empty() )
    return;

#ifdef _DEBUG_
  std::cout << "Input MED file: " << theInputMedFile << std::endl;
#endif
  if ( !FT_Utils::fileExists( theInputMedFile ))
    throw std::invalid_argument( "Input MED file does not exist: " + theInputMedFile );

#ifdef _DEBUG_
  std::cout << "Output MED file: " << theOutputMedFile << std::endl;
#endif
  if ( !FT_Utils::canWrite( theOutputMedFile ))
    throw std::invalid_argument( "Can't create the output MED file: " + theOutputMedFile );

  std::vector< std::string > theNodeFiles ;
  for ( size_t i = 0; i < theInputNodeFiles.size(); ++i )
  {
#ifdef _DEBUG_
    std::cout << "Initial input node file #"<<i<<": " << theInputNodeFiles[i] << std::endl;
#endif
    if ( !FT_Utils::fileExists( theInputNodeFiles[i] ))
      throw std::invalid_argument( "Input node file does not exist: " + theInputNodeFiles[i] );
    // the name of the groupe on line #1, then the numbers of nodes on line #>1
    // keep only files with more than 1 line:
    std::ifstream fichier(theInputNodeFiles[i].c_str());
    std::string s;
    unsigned int nb_lines = 0;
    while(std::getline(fichier,s)) ++nb_lines;
//     std::cout << ". nb_lines: " << nb_lines << std::endl;
    if ( nb_lines >= 2 ) { theNodeFiles.push_back( theInputNodeFiles[i] ); }
  }
#ifdef _DEBUG_
  for ( size_t i = 0; i < theNodeFiles.size(); ++i )
  { std::cout << "Valid input node file #"<<i<<": " << theNodeFiles[i] << std::endl; }
#endif

#ifdef _DEBUG_
  std::cout << "XAO file: " << theXaoFileName << std::endl;
#endif
  if ( !FT_Utils::fileExists( theXaoFileName ))
    throw std::invalid_argument( "Input XAO file does not exist: " + theXaoFileName );

  // read a mesh

#ifdef _DEBUG_
  std::cout << "Lecture du maillage" << std::endl;
#endif
  MEDCoupling::MCAuto< MEDCoupling::MEDFileUMesh >
    mfMesh( MEDCoupling::MEDFileUMesh::New( theInputMedFile ));
  if ( mfMesh.isNull() )
    throw std::invalid_argument( "Failed to read the input MED file: " + theInputMedFile );

  MEDCoupling::DataArrayDouble * nodeCoords = mfMesh->getCoords();
  if ( !nodeCoords || nodeCoords->empty() )
    throw std::invalid_argument( "No nodes in the input mesh" );


  // read a geometry

#ifdef _DEBUG_
  std::cout << "Lecture de la geometrie" << std::endl;
#endif
  XAO::Xao xao;
  if ( !xao.importXAO( theXaoFileName ) || !xao.getGeometry() )
    throw std::invalid_argument( "Failed to read the XAO input file: " + theXaoFileName );

#ifdef _DEBUG_
  std::cout << "Conversion en BREP" << std::endl;
#endif
  XAO::BrepGeometry* xaoGeom = dynamic_cast<XAO::BrepGeometry*>( xao.getGeometry() );
  if ( !xaoGeom || xaoGeom->getTopoDS_Shape().IsNull() )
    throw std::invalid_argument( "Failed to get a BREP shape from the XAO input file" );


  // read groups of nodes and associate them with boundary shapes using names (no projection so far)

#ifdef _DEBUG_
  std::cout << "Lecture des groupes" << std::endl;
#endif
  FT_NodeGroups nodeGroups;
  nodeGroups.read( theNodeFiles, &xao, nodeCoords );
#ifdef _DEBUG_
  std::cout << "Nombre de groupes : " << nodeGroups.nbOfGroups() << std::endl;
#endif

  // project nodes to the boundary shapes and change their coordinates

#ifdef _DEBUG_
  std::cout << "Projection des noeuds, theIsParallel=" << theIsParallel << std::endl;
#endif
  OSD_Parallel::For( 0, nodeGroups.nbOfGroups(), nodeGroups, !theIsParallel );

  // save the modified mesh

#ifdef _DEBUG_
  std::cout << "Ecriture du maillage" << std::endl;
#endif
  const int erase = 2;
  mfMesh->write( theOutputMedFile, /*mode=*/erase );

  if ( !nodeGroups.isOK() )
    throw std::runtime_error("Unable to project some nodes");
}

  //================================================================================
  /*!
   * \brief Initialize FT_Projector's with all sub-shapes of given type
   *  \param [in] theMainShape - the shape to explore
   *  \param [in] theSubType - the type of sub-shapes
   *  \param [out] theProjectors - the projectors
   */
  //================================================================================

  void getProjectors( const TopoDS_Shape&           theMainShape,
                      const TopAbs_ShapeEnum        theSubType,
                      std::vector< FT_Projector > & theProjectors )
  {
    TopTools_IndexedMapOfShape subShapes;
    TopExp::MapShapes( theMainShape, theSubType, subShapes );
#ifdef _DEBUG_
    std::cout << ". Nombre de subShapes : " << subShapes.Size() << std::endl;
#endif

    theProjectors.resize( subShapes.Size() );
    for ( int i = 1; i <= subShapes.Size(); ++i )
      theProjectors[ i-1 ].setBoundaryShape( subShapes( i ));
  }

//================================================================================
/*!
 * \brief Load node groups from files
 *  \param [in] theNodeFiles - an array of names of files describing groups of nodes that
 *         will be moved onto geometry
 *  \param [in] theXaoGeom - the whole geometry to project on
 *  \param [inout] theNodeCoords - array of node coordinates
 */
//================================================================================

void FT_NodeGroups::read( const std::vector< std::string >& theNodeFiles,
                          const XAO::Xao*                   theXao,
                          MEDCoupling::DataArrayDouble*     theNodeCoords )
{
  // get projectors for all boundary sub-shapes;
  // index of a projector in the vector corresponds to a XAO index of a sub-shape
  XAO::BrepGeometry* xaoGeom = dynamic_cast<XAO::BrepGeometry*>( theXao->getGeometry() );
  getProjectors( xaoGeom->getTopoDS_Shape(), TopAbs_EDGE, _projectors[0] );
  getProjectors( xaoGeom->getTopoDS_Shape(), TopAbs_FACE, _projectors[1] );

  _nodesOnGeom.resize( theNodeFiles.size() );

  // read node IDs and look for projectors to boundary sub-shapes by group name
  FT_Utils::XaoGroups xaoGroups( theXao );
  for ( size_t i = 0; i < theNodeFiles.size(); ++i )
  {
    _nodesOnGeom[i].read( theNodeFiles[i], xaoGroups, theNodeCoords, _projectors );
  }
}

//================================================================================
/*!
 * \brief Project and move nodes of a given group of nodes
 */
//================================================================================

void FT_NodeGroups::projectAndMove( const int groupIndex )
{
  _nodesOnGeom[ groupIndex ].projectAndMove();
}

//================================================================================
/*!
 * \brief Return true if all nodes were successfully relocated
 */
//================================================================================

bool FT_NodeGroups::isOK() const
{
  for ( size_t i = 0; i < _nodesOnGeom.size(); ++i )
    if ( ! _nodesOnGeom[ i ].isOK() )
      return false;

  return true;
}

//================================================================================
/*!
 * \brief Print some statistics on node groups
 */
//================================================================================

void FT_NodeGroups::dumpStat() const
{
  for ( size_t i = 0; i < _nodesOnGeom.size(); ++i )
  {
    std::cout << _nodesOnGeom[i].getShapeDim() << "D "
              << _nodesOnGeom[i].nbNodes() << " nodes" << std::endl;
  }
}

  /*!
   * \brief Close a file at destruction
   */
  struct FileCloser
  {
    FILE * _file;

    FileCloser( FILE * file ): _file( file ) {}
    ~FileCloser() { if ( _file ) ::fclose( _file ); }
  };

//================================================================================
/*!
 * \brief Read node ids from a file and find shapes for projection
 *  \param [in] theNodeFile - a name of file holding IDs of nodes that
 *         will be moved onto geometry
 *  \param [in] theXaoGroups - a tool returning FT_Projector's by XAO group name
 *  \param [inout] theNodeCoords - array of node coordinates
 *  \param [in] theAllProjectorsByDim - all projectors of 2 dimensions, ordered so that
 *         a vector index corresponds to a XAO sub-shape ID
 */
//================================================================================

void FT_NodesOnGeom::read( const std::string&            theNodeFile,
                           const FT_Utils::XaoGroups&    theXaoGroups,
                           MEDCoupling::DataArrayDouble* theNodeCoords,
                           std::vector< FT_Projector > * theAllProjectorsByDim )
{
  _nodeCoords = theNodeCoords;

  FILE * file = ::fopen( theNodeFile.c_str(), "r" );
  if ( !file )
    throw std::invalid_argument( "Can't open an input node file: " + theNodeFile );

  FileCloser fileCloser( file );

  // -------------------------------------
  // get shape dimension by the file name
  // -------------------------------------

  // hope the file name is something like "frnD.**" with n in (1,2)
  int dimPos = theNodeFile.size() - 5;
  if ( theNodeFile[ dimPos ] == '2' )
    _shapeDim = 2;
  else if ( theNodeFile[ dimPos ] == '1' )
    _shapeDim = 1;
  else
    throw std::invalid_argument( "Can't define dimension by node file name " + theNodeFile );
#ifdef _DEBUG_
  std::cout << ". Dimension of the file " << theNodeFile << ": " << _shapeDim << std::endl;
#endif

  // -------------------------------------
  // read geom group names; several lines
  // -------------------------------------

  std::vector< std::string > geomNames;

  const int maxLineLen = 256;
  char line[ maxLineLen ];

  long int pos = ::ftell( file );
  while ( ::fgets( line, maxLineLen, file )) // read a line
  {
    if ( ::feof( file ))
    {
      return; // no nodes in the file
    }

    // check if the line describes node ids in format 3I10 (e.g. "       120         1        43\n")
    size_t lineLen = strlen( line );
    if ( lineLen  >= 31        &&
         ::isdigit( line[9] )  &&
         line[10] == ' '       &&
         ::isdigit( line[19] ) &&
         line[20] == ' '       &&
         ::isdigit( line[29] ) &&
         ::isspace( line[30] ))
      break;

    geomNames.push_back( line + 1 ); // skip the 1st white space

    pos = ::ftell( file ); // remember the position to return if the next line holds node ids
  }

  ::fseek( file, pos, SEEK_SET ); // return to the 1st line holding nodes ids


  // --------------
  // read node ids
  // --------------

  FT_NodeToMove nodeIds;
  std::vector< int > ids;

  const int nbNodes = theNodeCoords->getNumberOfTuples(); // to check validity of node IDs

  while ( ::fgets( line, maxLineLen, file )) // read a line
  {
    // find node ids in the line

    char *beg = line, *end = 0;
    long int id;

    ids.clear();
    while (( id = ::strtol( beg, &end, 10 )) &&
           ( beg != end ))
    {
      ids.push_back( id );
      if ( id > nbNodes )
        throw std::invalid_argument( "Too large node ID: " + FT_Utils::toStr( id ));
      beg = end;
    }

    if ( ids.size() >= 3 )
    {
      std::vector< int >::iterator i = ids.begin();
      nodeIds._nodeToMove = *i;
      nodeIds._neighborNodes.assign( ++i, ids.end() );

      _nodes.push_back( nodeIds );
    }

    if ( ::feof( file ))
      break;
  }

  // -----------------------------------------------------------------
  // try to find FT_Projector's to boundary sub-shapes by group names
  // -----------------------------------------------------------------

  _allProjectors = & theAllProjectorsByDim[ _shapeDim - 1 ];

  _projectors.reserve( geomNames.size() );
  std::vector< const FT_Projector* >  projectors;

  for ( size_t i = 0; i < geomNames.size(); ++i )
  {
    std::string & groupName = geomNames[i];
#ifdef _DEBUG_
    std::cout << ". Group name: " << groupName << std::endl;
#endif

    // remove trailing white spaces
    for ( int iC = groupName.size() - 1; iC >= 0; --iC )
    {
      if ( ::isspace( groupName[iC] ) )
        groupName.resize( iC );
      else
        break;
    }
    if ( groupName.empty() )
      continue;

    _groupNames.push_back( groupName ); // keep _groupNames for easier debug :)

    // get projectors by group name
    theXaoGroups.getProjectors( groupName, _shapeDim,
                                theAllProjectorsByDim[ _shapeDim-1 ], projectors );
  }

  // ------------------------------
  // check the found FT_Projector's
  // ------------------------------

  if ( projectors.size() == 1 )
  {
    _projectors.push_back( *projectors[ 0 ]);
  }
  else
  {
    Bnd_Box nodesBox;
    for ( size_t i = 0; i < _nodes.size(); ++i )
      nodesBox.Add( getPoint( _nodes[i]._nodeToMove ));

    if ( projectors.size() > 1 )
    {
      // more than one boundary shape;
      // try to filter off unnecessary projectors using a bounding box of nodes
      for ( size_t i = 0; i < projectors.size(); ++i )
        if ( !nodesBox.IsOut( projectors[ i ]->getBoundingBox() ))
          _projectors.push_back( *projectors[ i ]);
    }

    if ( _projectors.empty() )
    {
      // select projectors using a bounding box of nodes
      std::vector< FT_Projector > & allProjectors = *_allProjectors;
      for ( size_t i = 0; i < allProjectors.size(); ++i )
        if ( !nodesBox.IsOut( allProjectors[ i ].getBoundingBox() ))
          _projectors.push_back( allProjectors[ i ]);

      if ( _projectors.empty() && !_nodes.empty() )
        throw std::runtime_error("No boundary shape found for nodes in file " + theNodeFile );
    }
  }

  // prepare for projection - create real projectors
  for ( size_t i = 0; i < _projectors.size(); ++i )
    _projectors[ i ].prepareForProjection();

}

//================================================================================
/*!
 * \brief Project nodes to the shapes and move them to new positions
 */
//================================================================================

void FT_NodesOnGeom::projectAndMove()
{
  _OK = true;
//
// 1. Préalables
//
  // check if all the shapes are planar
  bool isAllPlanar = true;
  for ( size_t i = 0; i < _projectors.size() &&  isAllPlanar; ++i )
    isAllPlanar = _projectors[i].isPlanarBoundary();
  if ( isAllPlanar )
    return;

  // set nodes in the order suitable for optimal projection
  putNodesInOrder();

  // project and move nodes

  std::vector< FT_NodeToMove* > notProjectedNodes;
  size_t iP, iProjector;
  gp_Pnt newXyz;

#ifdef _DEBUG_
    std::cout << ".. _projectors.size() = " << _projectors.size() << std::endl;
    std::cout << ".. _nodesOrder.size() = " << _nodesOrder.size() << std::endl;
#endif
//
// 2. Calculs
// 2.1. Avec plusieurs shapes
//
  if ( _projectors.size() > 1 )
  {
    // the nodes are to be projected onto several boundary shapes;
    // in addition to the projecting, classification on a shape is necessary
    // in order to find out on which of the shapes a node is to be projected

    iProjector = 0;
    for ( size_t i = 0; i < _nodesOrder.size(); ++i )
    {
      FT_NodeToMove& nn = _nodes[ _nodesOrder[ i ]];
      gp_Pnt        xyz = getPoint( nn._nodeToMove );
      gp_Pnt       xyz1 = getPoint( nn._neighborNodes[0] );
      gp_Pnt       xyz2 = getPoint( nn._neighborNodes[1] );
      double   maxDist2 = xyz1.SquareDistance( xyz2 ) / 4.;
      if ( _projectors[ iProjector ].projectAndClassify( xyz, maxDist2, newXyz,
                                                         nn._params, nn._nearParams ))
      {
        moveNode( nn._nodeToMove, newXyz );
      }
      else // a node is not on iProjector-th shape, find the shape it is on
      {
        for ( iP = 1; iP < _projectors.size(); ++iP ) // check _projectors other than iProjector
        {
          iProjector = ( iProjector + 1 ) % _projectors.size();
          if ( _projectors[ iProjector ].projectAndClassify( xyz, maxDist2, newXyz,
                                                             nn._params, nn._nearParams ))
          {
            moveNode( nn._nodeToMove, newXyz );
            break;
          }
        }
        if ( iP == _projectors.size() )
        {
          notProjectedNodes.push_back( &nn );

#ifdef _DEBUG_
          std::cerr << "Warning: no shape found for node " << nn._nodeToMove << std::endl;
          if ( !_groupNames.empty() )
            std::cerr << "Warning:    group -- " << _groupNames[0] << std::endl;
#endif
        }
      }
    }
  }
//
// 2.2. Avec une seule shape
//
  else // one shape
  {
    for ( size_t i = 0; i < _nodesOrder.size(); ++i )
    {
      FT_NodeToMove& nn = _nodes[ _nodesOrder[ i ]];
      gp_Pnt        xyz = getPoint( nn._nodeToMove );
      gp_Pnt       xyz1 = getPoint( nn._neighborNodes[0] );
      gp_Pnt       xyz2 = getPoint( nn._neighborNodes[1] );

// maxDist2 : le quart du carré de la distance entre les deux voisins du noeud à bouger
      double   maxDist2 = xyz1.SquareDistance( xyz2 ) / 4.;
#ifdef _DEBUG_
    std::cout << "\n.. maxDist2 = " << maxDist2 << " entre " << nn._neighborNodes[0] << " et " << nn._neighborNodes[1] << " - milieu " << nn._nodeToMove << " - d/2 = " << sqrt(maxDist2) << " - d = " << sqrt(xyz1.SquareDistance( xyz2 )) << std::endl;
#endif
      if ( _projectors[ 0 ].project( xyz, maxDist2, newXyz,
                                     nn._params, nn._nearParams ))
        moveNode( nn._nodeToMove, newXyz );
      else
        notProjectedNodes.push_back( &nn );
    }
  }
//
// 3. Bilan
//
  if ( !notProjectedNodes.empty() )
  {
    // project nodes that are not projected by any of _projectors;
    // a proper projector is selected by evaluation of a distance between neighbor nodes
    // and a shape

    std::vector< FT_Projector > & projectors = *_allProjectors;

    iProjector = 0;
    for ( size_t i = 0; i < notProjectedNodes.size(); ++i )
    {
      FT_NodeToMove& nn = *notProjectedNodes[ i ];
      gp_Pnt        xyz = getPoint( nn._nodeToMove );
      gp_Pnt       xyz1 = getPoint( nn._neighborNodes[0] );
      gp_Pnt       xyz2 = getPoint( nn._neighborNodes[1] );
      double   maxDist2 = xyz1.SquareDistance( xyz2 ) / 4.;
      double       tol2 = 1e-6 * maxDist2;

      bool ok;
      for ( iP = 0; iP < projectors.size(); ++iP )
      {
        projectors[ iProjector ].prepareForProjection();
        projectors[ iProjector ].tryWithoutPrevSolution( true );

        if (( ok = projectors[ iProjector ].isOnShape( xyz1, tol2, nn._params, nn._nearParams )) &&
            ( ok = projectors[ iProjector ].isOnShape( xyz2, tol2, nn._params, nn._params )))
        {
          if ( nn._neighborNodes.size() == 4 )
          {
            gp_Pnt xyz1 = getPoint( nn._neighborNodes[2] );
            gp_Pnt xyz2 = getPoint( nn._neighborNodes[3] );
            if (( ok = projectors[ iProjector ].isOnShape( xyz1, tol2, nn._params, nn._params )))
              ok     = projectors[ iProjector ].isOnShape( xyz2, tol2, nn._params, nn._params );
          }
        }

        if ( ok && projectors[iProjector].project( xyz, maxDist2, newXyz, nn._params, nn._params ))
        {
          moveNode( nn._nodeToMove, newXyz );
          break;
        }
        iProjector = ( iProjector + 1 ) % projectors.size();
      }
      if ( iP == projectors.size() )
      {
        _OK = false;

        std::cerr << "Error: not projected node " << nn._nodeToMove << std::endl;
      }
    }
  }
}

//================================================================================
/*!
 * \brief Put nodes in the order for optimal projection and set FT_NodeToMove::_nearParams
 *        to point to a FT_NodeToMove::_params of a node that will be projected earlier
 */
//================================================================================

void FT_NodesOnGeom::putNodesInOrder()
{
  if ( !_nodesOrder.empty() )
    return;

  // check if any of projectors can use parameters of a previously projected node on a shape
  // to speed up projection

  bool isPrevSolutionUsed = false;
  for ( size_t i = 0; i < _projectors.size() &&  !isPrevSolutionUsed; ++i )
    isPrevSolutionUsed = _projectors[i].canUsePrevSolution();

  if ( !isPrevSolutionUsed )
  {
    _nodesOrder.resize( _nodes.size() );
    for ( size_t i = 0; i < _nodesOrder.size(); ++i )
      _nodesOrder[ i ] = i;
    return;
  }

  // make a map to find a neighbor projected node

  // map of { FT_NodeToMove::_neighborNodes[i] } to { FT_NodeToMove* };
  // here we call FT_NodeToMove a 'link' as this data links a _neighborNodes[i] node to other nodes
  typedef NCollection_DataMap< int, std::vector< FT_NodeToMove* > > TNodeIDToLinksMap;
  TNodeIDToLinksMap neigborsMap;

  int mapSize = ( _shapeDim == 1 ) ? _nodes.size() + 1 : _nodes.size() * 3;
  neigborsMap.Clear();
  neigborsMap.ReSize( mapSize );

  std::vector< FT_NodeToMove* > linkVec, *linkVecPtr;
  const int maxNbLinks = ( _shapeDim == 1 ) ? 2 : 6; // usual nb of links

  for ( size_t i = 0; i < _nodes.size(); ++i )
  {
    FT_NodeToMove& nn = _nodes[i];
    for ( size_t iN = 0; iN < nn._neighborNodes.size(); ++iN )
    {
      if ( !( linkVecPtr = neigborsMap.ChangeSeek( nn._neighborNodes[ iN ] )))
      {
        linkVecPtr = neigborsMap.Bound( nn._neighborNodes[ iN ], linkVec );
        linkVecPtr->reserve( maxNbLinks );
      }
      linkVecPtr->push_back( & nn );
    }
  }

  // fill in _nodesOrder

  _nodesOrder.reserve( _nodes.size() );

  std::list< FT_NodeToMove* > queue;
  queue.push_back( &_nodes[0] );
  _nodes[0]._nearParams = _nodes[0]._params; // to avoid re-adding to the queue

  while ( !queue.empty() )
  {
    FT_NodeToMove* nn = queue.front();
    queue.pop_front();

    _nodesOrder.push_back( nn - & _nodes[0] );

    // add neighbors to the queue and set their _nearParams = nn->_params
    for ( size_t iN = 0; iN < nn->_neighborNodes.size(); ++iN )
    {
      std::vector< FT_NodeToMove* >& linkVec = neigborsMap( nn->_neighborNodes[ iN ]);
      for ( size_t iL = 0; iL < linkVec.size(); ++iL )
      {
        FT_NodeToMove* nnn = linkVec[ iL ];
        if ( nnn != nn && nnn->_nearParams == 0 )
        {
          nnn->_nearParams = nn->_params;
          queue.push_back( nnn );
        }
      }
    }
  }
  _nodes[0]._nearParams = 0; // reset
}

//================================================================================
/*!
 * \brief Get node coordinates. Node IDs count from a unit
 */
//================================================================================

gp_Pnt FT_NodesOnGeom::getPoint( const int nodeID )
{
  const size_t dim = _nodeCoords->getNumberOfComponents();
  const double * xyz = _nodeCoords->getConstPointer() + ( dim * ( nodeID - 1 ));
  return gp_Pnt( xyz[0], xyz[1], dim == 2 ? 0 : xyz[2] );
}

//================================================================================
/*!
 * \brief change node coordinates
 */
//================================================================================

void FT_NodesOnGeom::moveNode( const int nodeID, const gp_Pnt& newXyz )
{
  const size_t dim = _nodeCoords->getNumberOfComponents();
  double z, *xyz = _nodeCoords->getPointer() + ( dim * ( nodeID - 1 ));
  newXyz.Coord( xyz[0], xyz[1], dim == 2 ? z : xyz[2] );
}

//-----------------------------------------------------------------------------
/*!
 * \brief Root class of a projector of a point to a boundary shape
 */
struct FT_RealProjector
{
  virtual ~FT_RealProjector() {}

  /*!
   * \brief Project a point to a boundary shape
   *  \param [in] point - the point to project
   *  \param [out] newSolution - position on the shape (U or UV) found during the projection
   *  \param [in] prevSolution - position already found during the projection of a neighbor point
   *  \return gp_Pnt - the projection point
   */
  virtual gp_Pnt project( const gp_Pnt& point,
                          double*       newSolution,
                          const double* prevSolution = 0) = 0;

  /*!
   * \brief Project a point to a boundary shape and check if the projection is within
   *        the shape boundary
   *  \param [in] point - the point to project
   *  \param [in] maxDist2 - the maximal allowed square distance between point and projection
   *  \param [out] projection - the projection point
   *  \param [out] newSolution - position on the shape (U or UV) found during the projection
   *  \param [in] prevSolution - position already found during the projection of a neighbor point
   *  \return bool - false if the projection point lies out of the shape boundary or
   the distance the point and the projection is more than sqrt(maxDist2)
  */
  virtual bool projectAndClassify( const gp_Pnt& point,
                                   const double  maxDist2,
                                   gp_Pnt&       projection,
                                   double*       newSolution,
                                   const double* prevSolution = 0) = 0;

  // return true if a previously found solution can be used to speed up the projection

  virtual bool canUsePrevSolution() const { return false; }


  double _dist; // distance between the point being projected and its projection
};

namespace // actual projection algorithms
{
  const double theEPS = 1e-12;

  //================================================================================
  /*!
   * \brief Projector to any curve
   */
  //================================================================================

  struct CurveProjector : public FT_RealProjector
  {
    BRepAdaptor_Curve   _curve;
    double              _tol;
    ShapeAnalysis_Curve _projector;
    double              _uRange[2];

    //-----------------------------------------------------------------------------
    CurveProjector( const TopoDS_Edge& e, const double tol ):
      _curve( e ), _tol( tol )
    {
      BRep_Tool::Range( e, _uRange[0], _uRange[1] );
    }

    //-----------------------------------------------------------------------------
    // project a point to the curve
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
#ifdef _DEBUG_
    std::cout << ".. project a point to the curve prevSolution = " << prevSolution << std::endl;
#endif
      gp_Pnt         proj;
      Standard_Real param;

      if ( prevSolution )
      {
        _dist = _projector.NextProject( prevSolution[0], _curve, P, _tol, proj, param );
      }
      else
      {
        _dist = _projector.Project( _curve, P, _tol, proj, param, false );
      }
#ifdef _DEBUG_
    std::cout << "..    _dist : " << _dist << std::endl;
#endif
      proj = _curve.Value( param );

      newSolution[0] = param;

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to a curve and check if the projection is within the curve boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
#ifdef _DEBUG_
    std::cout << ".. project a point to a curve and check " << std::endl;
#endif
      projection = project( point, newSolution, prevSolution );
      return ( _uRange[0] < newSolution[0] && newSolution[0] < _uRange[1] &&
               _dist * _dist < maxDist2 );
    }

    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return true; }
  };

  //================================================================================
  /*!
   * \brief Projector to a straight curve. Don't project, classify only
   */
  //================================================================================

  struct LineProjector : public FT_RealProjector
  {
    gp_Pnt _p0, _p1;

    //-----------------------------------------------------------------------------
    LineProjector( TopoDS_Edge e )
    {
      e.Orientation( TopAbs_FORWARD );
      _p0 = BRep_Tool::Pnt( TopExp::FirstVertex( e ));
      _p1 = BRep_Tool::Pnt( TopExp::LastVertex ( e ));
    }

    //-----------------------------------------------------------------------------
    // does nothing
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      return P;
    }
    //-----------------------------------------------------------------------------
    // check if a point lies within the line segment
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      gp_Vec edge( _p0, _p1 );
      gp_Vec p0p ( _p0, point  );
      double u = ( edge * p0p ) / edge.SquareMagnitude();  // param [0,1] on the edge
      projection = ( 1. - u ) * _p0.XYZ() + u * _p1.XYZ(); // projection of the point on the edge
      if ( u < 0 || 1 < u )
        return false;

      // check distance
      return point.SquareDistance( projection ) < theEPS * theEPS;
    }
  };

  //================================================================================
  /*!
   * \brief Projector to a circular edge
   */
  //================================================================================

  struct CircleProjector : public FT_RealProjector
  {
    gp_Circ _circle;
    double _uRange[2];

    //-----------------------------------------------------------------------------
    CircleProjector( const gp_Circ& c, const double f, const double l ):
      _circle( c )
    {
      _uRange[0] = f;
      _uRange[1] = l;
    }

    //-----------------------------------------------------------------------------
    // project a point to the circle
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      // assume that P is already on the the plane of circle, since
      // it is in the middle of two points lying on the circle

      // move P to the circle
      const gp_Pnt& O = _circle.Location();
      gp_Vec radiusVec( O, P );
      double radius = radiusVec.Magnitude();
      if ( radius < std::numeric_limits<double>::min() )
        return P; // P in on the axe

      gp_Pnt proj = O.Translated( radiusVec.Multiplied( _circle.Radius() / radius ));

      _dist = _circle.Radius() - radius;

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project and check if a projection lies within the circular edge
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      _dist = -1;
      projection = project( point, newSolution );
      if ( _dist < 0 || // ?
           _dist * _dist > maxDist2 )
        return false;

      newSolution[0] = ElCLib::Parameter( _circle, projection );
      return ( _uRange[0] < newSolution[0] && newSolution[0] < _uRange[1] );
    }
  };

  //================================================================================
  /*!
   * \brief Projector to any surface
   */
  //================================================================================

  struct SurfaceProjector : public FT_RealProjector
  {
    ShapeAnalysis_Surface    _projector;
    double                   _tol;
    BRepTopAdaptor_FClass2d* _classifier;

    //-----------------------------------------------------------------------------
    SurfaceProjector( const TopoDS_Face& face, const double tol, BRepTopAdaptor_FClass2d* cls ):
      _projector( BRep_Tool::Surface( face )),
      _tol( tol ),
      _classifier( cls )
    {
    }
    //-----------------------------------------------------------------------------
    // delete _classifier
    ~SurfaceProjector()
    {
      delete _classifier;
    }

    //-----------------------------------------------------------------------------
    // project a point to a surface
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      gp_Pnt2d uv;

      if ( prevSolution )
      {
        gp_Pnt2d prevUV( prevSolution[0], prevSolution[1] );
        uv = _projector.NextValueOfUV( prevUV, P, _tol );
      }
      else
      {
        uv = _projector.ValueOfUV( P, _tol );
      }

      uv.Coord( newSolution[0], newSolution[1] );

      gp_Pnt proj = _projector.Value( uv );

      _dist = _projector.Gap();

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to a surface and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      projection = project( point, newSolution, prevSolution );
      return ( _dist * _dist < maxDist2 )  &&  classify( newSolution );
    }

    //-----------------------------------------------------------------------------
    // check if the projection is within the shape boundary
    bool classify( const double* newSolution )
    {
      TopAbs_State state = _classifier->Perform( gp_Pnt2d( newSolution[0], newSolution[1]) );
      return ( state != TopAbs_OUT );
    }

    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return true; }
  };

  //================================================================================
  /*!
   * \brief Projector to a plane. Don't project, classify only
   */
  //================================================================================

  struct PlaneProjector : public SurfaceProjector
  {
    gp_Pln _plane;
    bool   _isRealPlane; // false means that a surface is planar but parametrization is different

    //-----------------------------------------------------------------------------
    PlaneProjector( const gp_Pln&            pln,
                    const TopoDS_Face&       face,
                    BRepTopAdaptor_FClass2d* cls,
                    bool                     isRealPlane=true):
      SurfaceProjector( face, 0, cls ),
      _plane( pln ),
      _isRealPlane( isRealPlane )
    {}

    //-----------------------------------------------------------------------------
    // does nothing
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      return P;
    }
    //-----------------------------------------------------------------------------
    // check if a point lies within the boundry of the planar face
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      if ( _isRealPlane )
      {
        ElSLib::PlaneParameters( _plane.Position(), point, newSolution[0], newSolution[1]);
        projection = ElSLib::PlaneValue ( newSolution[0], newSolution[1], _plane.Position() );
        if ( projection.SquareDistance( point ) > theEPS * theEPS )
          return false;

        return SurfaceProjector::classify( newSolution );
      }
      else
      {
        return SurfaceProjector::projectAndClassify( point, maxDist2, projection,
                                                     newSolution, prevSolution );
      }
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Projector to a cylinder
   */
  //================================================================================

  struct CylinderProjector : public SurfaceProjector
  {
    gp_Cylinder _cylinder;

    //-----------------------------------------------------------------------------
    CylinderProjector( const gp_Cylinder&       c,
                       const TopoDS_Face&       face,
                       BRepTopAdaptor_FClass2d* cls ):
      SurfaceProjector( face, 0, cls ),
      _cylinder( c )
    {}

    //-----------------------------------------------------------------------------
    // project a point to the cylinder
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      // project the point P to the cylinder axis -> Pp
      const gp_Pnt& O   = _cylinder.Position().Location();
      const gp_Dir& axe = _cylinder.Position().Direction();
      gp_Vec       trsl = gp_Vec( axe ).Multiplied( gp_Vec( O, P ).Dot( axe ));
      gp_Pnt       Pp   = O.Translated( trsl );

      // move Pp to the cylinder
      gp_Vec radiusVec( Pp, P );
      double radius = radiusVec.Magnitude();
      if ( radius < std::numeric_limits<double>::min() )
        return P; // P in on the axe

      gp_Pnt proj = Pp.Translated( radiusVec.Multiplied( _cylinder.Radius() / radius ));

      _dist = _cylinder.Radius() - radius;

      return proj;
    }
    //-----------------------------------------------------------------------------
    // project a point to the cylinder and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      ElSLib::CylinderParameters( _cylinder.Position(), _cylinder.Radius(), point,
                                  newSolution[0], newSolution[1]);
      projection = ElSLib::CylinderValue( newSolution[0], newSolution[1],
                                          _cylinder.Position(), _cylinder.Radius() );

      return ( _dist * _dist < maxDist2 )  &&  SurfaceProjector::classify( newSolution );
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Projector to a cone
   */
  //================================================================================

  struct ConeProjector : public SurfaceProjector
  {
    gp_Cone _cone;

    //-----------------------------------------------------------------------------
    ConeProjector( const gp_Cone&           c,
                   const TopoDS_Face&       face,
                   BRepTopAdaptor_FClass2d* cls ):
      SurfaceProjector( face, 0, cls ),
      _cone( c )
    {}

    //-----------------------------------------------------------------------------
    // project a point to the cone
    virtual gp_Pnt project( const gp_Pnt& point,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      ElSLib::ConeParameters( _cone.Position(), _cone.RefRadius(), _cone.SemiAngle(),
                              point, newSolution[0], newSolution[1]);
      gp_Pnt proj = ElSLib::ConeValue( newSolution[0], newSolution[1],
                                       _cone.Position(), _cone.RefRadius(), _cone.SemiAngle() );
      _dist = point.Distance( proj );

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to the cone and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      projection = project( point, newSolution, prevSolution );

      return ( _dist * _dist < maxDist2 )  &&  SurfaceProjector::classify( newSolution );
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Projector to a sphere
   */
  //================================================================================

  struct SphereProjector : public SurfaceProjector
  {
    gp_Sphere _sphere;

    //-----------------------------------------------------------------------------
    SphereProjector( const gp_Sphere&         s,
                     const TopoDS_Face&       face,
                     BRepTopAdaptor_FClass2d* cls ):
      SurfaceProjector( face, 0, cls ),
      _sphere( s )
    {}

    //-----------------------------------------------------------------------------
    // project a point to the sphere
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      // move Pp to the Sphere
      const gp_Pnt& O = _sphere.Location();
      gp_Vec radiusVec( O, P );
      double radius = radiusVec.Magnitude();
      if ( radius < std::numeric_limits<double>::min() )
        return P; // P is on O

      gp_Pnt proj = O.Translated( radiusVec.Multiplied( _sphere.Radius() / radius ));

      _dist = _sphere.Radius() - radius;

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to the sphere and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      ElSLib::SphereParameters( _sphere.Position(), _sphere.Radius(), point,
                                  newSolution[0], newSolution[1]);
      projection = ElSLib::SphereValue( newSolution[0], newSolution[1],
                                        _sphere.Position(), _sphere.Radius() );

      return ( _dist * _dist < maxDist2 )  &&  SurfaceProjector::classify( newSolution );
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Projector to a torus
   */
  //================================================================================

  struct TorusProjector : public SurfaceProjector
  {
    gp_Torus _torus;

    //-----------------------------------------------------------------------------
    TorusProjector( const gp_Torus&          t,
                    const TopoDS_Face&       face,
                    BRepTopAdaptor_FClass2d* cls ):
      SurfaceProjector( face, 0, cls ),
      _torus( t )
    {}

    //-----------------------------------------------------------------------------
    // project a point to the torus
    virtual gp_Pnt project( const gp_Pnt& point,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      ElSLib::TorusParameters( _torus.Position(), _torus.MajorRadius(), _torus.MinorRadius(),
                               point, newSolution[0], newSolution[1]);
      gp_Pnt proj = ElSLib::TorusValue( newSolution[0], newSolution[1],
                                        _torus.Position(), _torus.MajorRadius(), _torus.MinorRadius() );
      _dist = point.Distance( proj );

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to the torus and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      projection = project( point, newSolution, prevSolution );

      return ( _dist * _dist < maxDist2 )  &&  SurfaceProjector::classify( newSolution );
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Check if a curve can be considered straight
   */
  //================================================================================

  bool isStraight( const GeomAdaptor_Curve& curve, const double tol )
  {
    // rough check: evaluate how far from a straight line connecting the curve ends
    // stand several internal points of the curve

    const double  f = curve.FirstParameter();
    const double  l = curve.LastParameter();
    const gp_Pnt pf = curve.Value( f );
    const gp_Pnt pl = curve.Value( l );
    const gp_Vec lineVec( pf, pl );
    const double lineLen2 = lineVec.SquareMagnitude();
    if ( lineLen2 < std::numeric_limits< double >::min() )
      return false; // E seems closed

    const double nbSamples = 7;
    for ( int i = 0; i < nbSamples; ++i )
    {
      const double  r = ( i + 1 ) / nbSamples;
      const gp_Pnt pi = curve.Value( f * r + l * ( 1 - r ));
      const gp_Vec vi( pf, pi );
      const double h2 = lineVec.Crossed( vi ).SquareMagnitude() / lineLen2;
      if ( h2 > tol * tol )
        return false;
    }

    // thorough check
    GCPnts_UniformDeflection divider( curve, tol );
    return ( divider.IsDone() && divider.NbPoints() < 3 );
  }
}

//================================================================================
/*!
 * \brief Initialize with a boundary shape
 */
//================================================================================

FT_Projector::FT_Projector(const TopoDS_Shape& shape)
{
  _realProjector = 0;
  setBoundaryShape( shape );
  _tryWOPrevSolution = false;
}

//================================================================================
/*!
 * \brief Copy another projector
 */
//================================================================================

FT_Projector::FT_Projector(const FT_Projector& other)
{
  _realProjector = 0;
  _shape = other._shape;
  _bndBox = other._bndBox;
  _tryWOPrevSolution = false;
}

//================================================================================
/*!
 * \brief Destructor. Delete _realProjector
 */
//================================================================================

FT_Projector::~FT_Projector()
{
  delete _realProjector;
}

//================================================================================
/*!
 * \brief Initialize with a boundary shape. Compute the bounding box
 */
//================================================================================

void FT_Projector::setBoundaryShape(const TopoDS_Shape& shape)
{
  delete _realProjector; _realProjector = 0;
  _shape = shape;
  if ( shape.IsNull() )
    return;

  BRepBndLib::Add( shape, _bndBox );
  _bndBox.Enlarge( 1e-5 * sqrt( _bndBox.SquareExtent() ));
}

//================================================================================
/*!
 * \brief Create a real projector
 */
//================================================================================

void FT_Projector::prepareForProjection()
{
  if ( _shape.IsNull() || _realProjector )
    return;

  if ( _shape.ShapeType() == TopAbs_EDGE )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( _shape );

    double tol = 1e-6 * sqrt( _bndBox.SquareExtent() );

    double f,l;
    Handle(Geom_Curve) curve = BRep_Tool::Curve( edge, f,l );
    if ( curve.IsNull() )
      return; // degenerated edge

    GeomAdaptor_Curve acurve( curve, f, l );
    switch ( acurve.GetType() )
    {
    case GeomAbs_Line:
      _realProjector = new LineProjector( edge );
      break;
    case GeomAbs_Circle:
      _realProjector = new CircleProjector( acurve.Circle(), f, l );
      break;
    case GeomAbs_BezierCurve:
    case GeomAbs_BSplineCurve:
    case GeomAbs_OffsetCurve:
    case GeomAbs_OtherCurve:
      if ( isStraight( acurve, tol ))
      {
        _realProjector = new LineProjector( edge );
        break;
      }
    case GeomAbs_Ellipse:
    case GeomAbs_Hyperbola:
    case GeomAbs_Parabola:
      _realProjector = new CurveProjector( edge, tol );
    }
  }
  else if ( _shape.ShapeType() == TopAbs_FACE )
  {
    TopoDS_Face face = TopoDS::Face( _shape );

    Handle(Geom_Surface) surface = BRep_Tool::Surface( face );
    if ( surface.IsNull() )
      return;

    GeomAdaptor_Surface asurface( surface );
    Standard_Real tol   = BRep_Tool::Tolerance( face );
    Standard_Real toluv = Min( asurface.UResolution( tol ), asurface.VResolution( tol ));
    BRepTopAdaptor_FClass2d* classifier = new BRepTopAdaptor_FClass2d( face, toluv );

    switch ( asurface.GetType() )
    {
    case GeomAbs_Plane:
      _realProjector = new PlaneProjector( asurface.Plane(), face, classifier );
      break;
    case GeomAbs_Cylinder:
      _realProjector = new CylinderProjector( asurface.Cylinder(), face, classifier );
      break;
    case GeomAbs_Sphere:
      _realProjector = new SphereProjector( asurface.Sphere(), face, classifier );
      break;
    case GeomAbs_Cone:
      _realProjector = new ConeProjector( asurface.Cone(), face, classifier );
      break;
    case GeomAbs_Torus:
      _realProjector = new TorusProjector( asurface.Torus(), face, classifier );
      break;
    case GeomAbs_BezierSurface:
    case GeomAbs_BSplineSurface:
    case GeomAbs_SurfaceOfRevolution:
    case GeomAbs_SurfaceOfExtrusion:
    case GeomAbs_OffsetSurface:
    case GeomAbs_OtherSurface:
      GeomLib_IsPlanarSurface isPlaneCheck( surface, tol );
      if ( isPlaneCheck.IsPlanar() )
      {
        _realProjector = new PlaneProjector( isPlaneCheck.Plan(), face, classifier,
                                             /*isRealPlane=*/false);
      }
      else
      {
        _realProjector = new SurfaceProjector( face, tol, classifier );
      }
      break;
    }

    if ( !_realProjector )
      delete classifier;
  }
}

//================================================================================
/*!
 * \brief Return true if projection is not needed
 */
//================================================================================

bool FT_Projector::isPlanarBoundary() const
{
  return ( dynamic_cast< LineProjector*  >( _realProjector ) ||
           dynamic_cast< PlaneProjector* >( _realProjector ) );
}

//================================================================================
/*!
 * \brief Check if a point lies on the boundary shape
 *  \param [in] point - the point to check
 *  \param [in] tol2 - a square tolerance allowing to decide whether a point is on the shape
 *  \param [in] newSolution - position on the shape (U or UV) of the point found
 *         during projecting
 *  \param [in] prevSolution - position on the shape (U or UV) of a neighbor point
 *  \return bool - \c true if the point lies on the boundary shape
 *
 * This method is used to select a shape by checking if all neighbor nodes of a node to move
 * lie on a shape.
 */
//================================================================================

bool FT_Projector::isOnShape( const gp_Pnt& point,
                              const double  tol2,
                              double*       newSolution,
                              const double* prevSolution)
{
  if ( _bndBox.IsOut( point ) || !_realProjector )
    return false;

  gp_Pnt proj;
  if ( isPlanarBoundary() )
    return projectAndClassify( point, tol2, proj, newSolution, prevSolution );

  return project( point, tol2, proj, newSolution, prevSolution );
}

//================================================================================
/*!
 * \brief Project a point to the boundary shape
 *  \param [in] point - the point to project
 *  \param [in] maxDist2 - the maximal square distance between the point and the projection
 *  \param [out] projection - the projection
 *  \param [out] newSolution - position on the shape (U or UV) of the point found
 *         during projecting
 *  \param [in] prevSolution - already found position on the shape (U or UV) of a neighbor point
 *  \return bool - false if the distance between the point and the projection
 *         is more than sqrt(maxDist2)
 *
 * This method is used to project a node in the case where only one shape is found by name
 */
//================================================================================

bool FT_Projector::project( const gp_Pnt& point,
                            const double  maxDist2,
                            gp_Pnt&       projection,
                            double*       newSolution,
                            const double* prevSolution)
{
  if ( !_realProjector )
    return false;

  _realProjector->_dist = 1e100;
  projection = _realProjector->project( point, newSolution, prevSolution );

  bool ok = ( _realProjector->_dist * _realProjector->_dist < maxDist2 );
  if ( !ok && _tryWOPrevSolution && prevSolution )
  {
    projection = _realProjector->project( point, newSolution );
    ok = ( _realProjector->_dist * _realProjector->_dist < maxDist2 );
  }
  return ok;
}

//================================================================================
/*!
 * \brief Project a point to the boundary shape and check if the projection lies within
 *        the shape boundary
 *  \param [in] point - the point to project
 *  \param [in] maxDist2 - the maximal square distance between the point and the projection
 *  \param [out] projection - the projection
 *  \param [out] newSolution - position on the shape (U or UV) of the point found
 *         during projecting
 *  \param [in] prevSolution - already found position on the shape (U or UV) of a neighbor point
 *  \return bool - false if the projection point lies out of the shape boundary or
 *          the distance between the point and the projection is more than sqrt(maxDist2)
 *
 * This method is used to project a node in the case where several shapes are selected for
 * projection of a node group
 */
//================================================================================

bool FT_Projector::projectAndClassify( const gp_Pnt& point,
                                       const double  maxDist2,
                                       gp_Pnt&       projection,
                                       double*       newSolution,
                                       const double* prevSolution)
{
  if ( _bndBox.IsOut( point ) || !_realProjector )
    return false;

  bool ok = _realProjector->projectAndClassify( point, maxDist2, projection,
                                                newSolution, prevSolution );
  if ( !ok && _tryWOPrevSolution && prevSolution )
    ok = _realProjector->projectAndClassify( point, maxDist2, projection, newSolution );

  return ok;
}

//================================================================================
/*!
 * \brief Return true if a previously found solution can be used to speed up the projection
 */
//================================================================================

bool FT_Projector::canUsePrevSolution() const
{
  return ( _realProjector && _realProjector->canUsePrevSolution() );
}

//================================================================================
/*
 * \brief Check if a file exists
 */
//================================================================================

bool FT_Utils::fileExists( const std::string& path )
{
  if ( path.empty() )
    return false;

  boost::system::error_code err;
  bool res = boofs::exists( path, err );

  return err ? false : res;
}

//================================================================================
/*!
 * \brief Check if a file can be created/overwritten
 */
//================================================================================

bool FT_Utils::canWrite( const std::string& path )
{
  if ( path.empty() )
    return false;

  bool can = false;
#ifdef WIN32

  HANDLE file = CreateFile( path.c_str(),           // name of the write
                            GENERIC_WRITE,          // open for writing
                            0,                      // do not share
                            NULL,                   // default security
                            OPEN_ALWAYS,            // CREATE NEW or OPEN EXISTING
                            FILE_ATTRIBUTE_NORMAL,  // normal file
                            NULL);                  // no attr. template
  can = ( file != INVALID_HANDLE_VALUE );
  CloseHandle( file );

#else

  int file = ::open( path.c_str(),
                     O_WRONLY | O_CREAT,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ); // rw-r--r--
  can = ( file >= 0 );

#endif

  return can;
}

//================================================================================
/*!
 * \brief Make a map of XAO groups
 */
//================================================================================

FT_Utils::XaoGroups::XaoGroups( const XAO::Xao* theXao )
{
  XAO::Xao* xao = const_cast< XAO::Xao* >( theXao );

  for ( int iG = 0; iG < theXao->countGroups(); ++iG )
  {
    XAO::Group* group = xao->getGroup( iG );

    if ( group->getDimension() == 1 )
      
      _xaoGroups[ 0 ].insert( std::make_pair( group->getName(), group ));

    else if ( group->getDimension() == 2 )

      _xaoGroups[ 1 ].insert( std::make_pair( group->getName(), group ));
  }
}

//================================================================================
/*!
 * \brief Return FT_Projector's by a group name
 *  \param [in] groupName - the group name
 *  \param [in] dim - the group dimension
 *  \param [in] allProjectors - the projector of all shapes of \a dim dimension
 *  \param [out] groupProjectors - projectors to shapes of the group
 *  \return int - number of found shapes
 */
//================================================================================

int FT_Utils::XaoGroups::getProjectors( const std::string&                   groupName,
                                        const int                            dim,
                                        const std::vector< FT_Projector > &  allProjectors,
                                        std::vector< const FT_Projector* > & groupProjectors) const
{
  // get namesake groups

  const TGroupByNameMap* groupMap = 0;
  if ( dim == 1 )
    groupMap = &_xaoGroups[ 0 ];
  else if ( dim == 2 )
    groupMap = &_xaoGroups[ 1 ];
  else
    return 0;

  TGroupByNameMap::const_iterator name2gr = groupMap->find( groupName );
  if ( name2gr == groupMap->end() )
    return 0;

  std::vector< XAO::Group* > groups;
  groups.push_back( name2gr->second );

  for ( ++name2gr; name2gr != groupMap->end(); ++name2gr )
  {
    if ( name2gr->second->getName() == groupName )
      groups.push_back( name2gr->second );
    else
      break;
  }

  // get projectors

  int nbFound = 0;
  for ( size_t i = 0; i < groups.size(); ++i )
  {
    // IDs in XAO correspond to indices of allProjectors
    std::set<int>::iterator id = groups[i]->begin(), end = groups[i]->end();
    for ( ; id != end; ++id, ++nbFound )
      if ( *id < (int) allProjectors.size() )
        groupProjectors.push_back ( & allProjectors[ *id ]);
  }

  return nbFound;
}

} // namespace SMESHHOMARDImpl /end/
