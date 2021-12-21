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
//  File   : HOMARD.hxx
//  Author : Gerald NICOLAS, EDF
//  Module : HOMARD

#ifndef _SMESH_HOMARD_ADAPT_HXX_
#define _SMESH_HOMARD_ADAPT_HXX_

// C'est le ASSERT de SALOMELocalTrace/utilities.h dans KERNEL
#ifndef VERIFICATION
#define VERIFICATION(condition) \
        if (!(condition)){INTERRUPTION("CONDITION "<<#condition<<" NOT VERIFIED")}
#endif /* VERIFICATION */

#ifdef WIN32
  #if defined HOMARDIMPL_EXPORTS || defined HOMARDImpl_EXPORTS
    #define HOMARDIMPL_EXPORT __declspec( dllexport )
  #else
    #define HOMARDIMPL_EXPORT __declspec( dllimport )
  #endif
#else
   #define HOMARDIMPL_EXPORT
#endif

// La gestion des repertoires
#ifndef CHDIR
  #ifdef WIN32
    #define CHDIR _chdir
  #else
    #define CHDIR chdir
  #endif
#endif

#include <vector>
#include <string>
#include <list>
#include <map>

#include <iostream>
#include <fstream>

#include <TopoDS_Shape.hxx>
#include <Bnd_Box.hxx>
#include <NCollection_DataMap.hxx>
#include <TColStd_DataMapOfIntegerInteger.hxx>

#if defined WIN32
#pragma warning ( disable: 4251 )
#endif

namespace MEDCoupling {
  class DataArrayDouble;
}
namespace XAO {
  class Xao;
  class Group;
  class BrepGeometry;
}

namespace SMESHHOMARDImpl
{

class HOMARDIMPL_EXPORT HOMARD_Boundary
{
public:
  HOMARD_Boundary();
  ~HOMARD_Boundary();

  // Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

  std::string                   GetDumpPython() const;

  // Caracteristiques
  void                          SetType( int Type );
  int                           GetType() const;

  void                          SetMeshName( const char* MeshName );
  std::string                   GetMeshName() const;

  void                          SetDataFile( const char* DataFile );
  std::string                   GetDataFile() const;

  void                          SetCylinder( double X0, double X1, double X2, double X3,
                                             double X4, double X5, double X6 );
  void                          SetSphere( double X0, double X1, double X2, double X3 );
  void                          SetConeR( double Xcentre1, double Ycentre1, double Zcentre1, double Rayon1,
                                          double Xcentre2, double Ycentre2, double Zcentre2, double Rayon2);
  void                          SetConeA( double Xaxe, double Yaxe, double Zaxe, double Angle,
                                          double Xcentre, double Ycentre, double ZCentre);
  void                          SetTorus( double X0, double X1, double X2, double X3,
                                             double X4, double X5, double X6, double X7 );

  std::vector<double>           GetCoords() const;

  void                          SetLimit( double X0, double X1, double X2 );
  std::vector<double>           GetLimit() const;

  void                          AddGroup( const char* LeGroupe);
  void                          SetGroups(const std::list<std::string>& ListGroup );
  const std::list<std::string>& GetGroups() const;

// Liens avec les autres structures
  std::string                   GetCaseCreation() const;
  void                          SetCaseCreation( const char* NomCasCreation );

private:
  std::string                   _Name;
  std::string                   _NomCasCreation;
  std::string                   _DataFile;
  std::string                   _MeshName;
  int                           _Type;
  double                        _Xmin, _Xmax, _Ymin, _Ymax, _Zmin, _Zmax;
  double                        _Xaxe, _Yaxe, _Zaxe;
  double                        _Xcentre, _Ycentre, _Zcentre, _rayon;
  double                        _Xincr, _Yincr, _Zincr;
  double                        _Xcentre1, _Ycentre1, _Zcentre1, _Rayon1;
  double                        _Xcentre2, _Ycentre2, _Zcentre2, _Rayon2;
  double                        _Angle;

  std::list<std::string>        _ListGroupSelected;

};

class HOMARDIMPL_EXPORT HOMARD_Cas
{
public:
  HOMARD_Cas();
  ~HOMARD_Cas();

  // Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

  std::string                   GetDumpPython() const;

  // Caracteristiques
  int                           SetDirName( const char* NomDir );
  std::string                   GetDirName() const;

  int                           GetNumberofIter();

  void                          SetConfType( int ConfType );
  const int                     GetConfType() const;

  void                          SetBoundingBox( const std::vector<double>& extremas );
  const std::vector<double>&    GetBoundingBox() const;

  void                          AddGroup( const char* Group);
  void                          SetGroups( const std::list<std::string>& ListGroup );
  const std::list<std::string>& GetGroups() const;
  void                          SupprGroups();

  void                          AddBoundary( const char* Boundary );
  void                          AddBoundaryGroup( const char* Boundary, const char* Group );
  const std::list<std::string>& GetBoundaryGroup() const;
  void                          SupprBoundaryGroup();

// Liens avec les autres structures
  std::string                   GetIter0Name() const;

  void                          AddIteration( const char* NomIteration );
  const std::list<std::string>& GetIterations() const;
  void                          SupprIterations();

private:
  std::string                   _Name;
  std::string                   _NomDir;
  int                           _ConfType;
  int                           _Etat;

  std::vector<double>           _Boite;         // cf HomardQTCommun pour structure du vecteur
  std::list<std::string>        _ListGroup;
  std::list<std::string>        _ListBoundaryGroup;

  typedef std::string           IterName;
  typedef std::list<IterName>   IterNames;
  IterNames                     _ListIter;
};

class HOMARDIMPL_EXPORT HomardDriver
{
public:
  HomardDriver(const std::string siter, const std::string siterp1);
  ~HomardDriver();
  //
  void        TexteInit( const std::string DirCompute, const std::string LogFile, const std::string Langue );
  void        TexteInfo( int TypeBila, int NumeIter );
  void        TexteMajCoords( int NumeIter );
  void        CreeFichierDonn();
  void        TexteAdap();
  void        CreeFichier();
  void        TexteMaillage( const std::string NomMesh, const std::string MeshFile, int apres );
  void        TexteMaillageHOMARD( const std::string Dir, const std::string liter, int apres );
  void        TexteConfRaffDera( int ConfType, int TypeAdap, int TypeRaff, int TypeDera );
  void        TexteZone( int NumeZone, int ZoneType, int TypeUse, double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7, double x8 );
  void        TexteGroup( const std::string GroupName );
  void        TexteField( const std::string FieldName, const std::string FieldFile,
                          int TimeStep, int Rank,
                          int TypeThR, double ThreshR, int TypeThC, double ThreshC,
                          int UsField, int UsCmpI );
  void        TexteCompo( int NumeComp, const std::string NomCompo);

  void        TexteBoundaryOption( int BoundaryOption );
  void        TexteBoundaryCAOGr( const std::string GroupName );
  void        TexteBoundaryDi( const std::string MeshName, const std::string MeshFile );
  void        TexteBoundaryDiGr( const std::string GroupName );
  void        TexteBoundaryAn( const std::string NameBoundary, int NumeBoundary, int BoundaryType, double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7 );
  void        TexteBoundaryAnGr( const std::string NameBoundary, int NumeBoundary, const std::string GroupName );

  void        TexteFieldInterp( const std::string FieldFile, const std::string MeshFile );
  void        TexteFieldInterpAll();
  void        TexteFieldInterpNameType( int NumeChamp, const std::string FieldName, const std::string TypeInterp, int TimeStep, int Rank );
  void        TexteAdvanced( int NivMax, double DiamMin, int AdapInit, int LevelOutput );
  void        TexteInfoCompute( int MessInfo );
  //
  int         ExecuteHomard();

public:
  int         _modeHOMARD;
  std::string _HOMARD_Exec;
  std::string _NomDir;
  std::string _NomFichierConfBase;
  std::string _NomFichierConf;
  std::string _NomFichierDonn;
  std::string _siter;
  std::string _siterp1;
  std::string _Texte;
  int         _TimeStep;
  int         _Rank;
  bool _bLu;
};

class HOMARD_Iteration;
class HOMARD_Hypothesis;

  //! persistence entity type
  typedef enum { Case, Zone, Hypothesis, Iteration, Boundary } SignatureType;

  //! get persistence signature
  HOMARDIMPL_EXPORT std::string GetSignature( SignatureType type );

  //! get data separator
  HOMARDIMPL_EXPORT std::string separator();

  //! dump boundary to the string
  HOMARDIMPL_EXPORT std::string Dump( const SMESHHOMARDImpl::HOMARD_Boundary& boundary );
  //! dump case to the string
  HOMARDIMPL_EXPORT std::string Dump( const SMESHHOMARDImpl::HOMARD_Cas& cas );
  //! dump iteration to the string
  HOMARDIMPL_EXPORT std::string Dump( const SMESHHOMARDImpl::HOMARD_Iteration& iteration );
  //! dump hypothesis to the string
  HOMARDIMPL_EXPORT std::string Dump( const SMESHHOMARDImpl::HOMARD_Hypothesis& hypothesis );

  //! restore boundary from the string
  HOMARDIMPL_EXPORT bool Restore( SMESHHOMARDImpl::HOMARD_Boundary& boundary, const std::string& stream );
  //! restore case from the string
  HOMARDIMPL_EXPORT bool Restore( SMESHHOMARDImpl::HOMARD_Cas& cas, const std::string& stream );
  //! restore hypothesis from the string
  HOMARDIMPL_EXPORT bool Restore( SMESHHOMARDImpl::HOMARD_Hypothesis& hypothesis, const std::string& stream );
  //! restore iteration from the string
  HOMARDIMPL_EXPORT bool Restore( SMESHHOMARDImpl::HOMARD_Iteration& iteration, const std::string& stream );

class HOMARDIMPL_EXPORT HOMARD_Gen
{
public :
  HOMARD_Gen();
  ~HOMARD_Gen();
};

class HOMARDIMPL_EXPORT HOMARD_Hypothesis
{
public:
  HOMARD_Hypothesis();
  ~HOMARD_Hypothesis();

// Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

// Caracteristiques
  void                          SetAdapType( int TypeAdap );
  int                           GetAdapType() const;
  void                          SetRefinTypeDera( int TypeRaff, int TypeDera );
  int                           GetRefinType() const;
  int                           GetUnRefType() const;

  void                          SetField( const char* FieldName );
  std::string                   GetFieldName() const;
  void                          SetUseField( int UsField );
  int                           GetUseField()    const;

  void                          SetUseComp( int UsCmpI );
  int                           GetUseComp()    const;
  void                          AddComp( const char* NomComp );
  void                          SupprComp( const char* NomComp );
  void                          SupprComps();
  const std::list<std::string>& GetComps() const;

  void                          SetRefinThr( int TypeThR, double ThreshR );
  int                           GetRefinThrType()   const;
  double                        GetThreshR()   const;
  void                          SetUnRefThr( int TypeThC, double ThreshC );
  int                           GetUnRefThrType()   const;
  double                        GetThreshC()   const;

  void                          SetNivMax( int NivMax );
  const int                     GetNivMax() const;

  void                          SetDiamMin( double DiamMin );
  const double                  GetDiamMin() const;

  void                          SetAdapInit( int AdapInit );
  const int                     GetAdapInit() const;

  void                          SetExtraOutput( int ExtraOutput );
  const int                     GetExtraOutput() const;

  void                          AddGroup( const char* Group);
  void                          SupprGroup( const char* Group );
  void                          SupprGroups();
  void                          SetGroups(const std::list<std::string>& ListGroup );
  const std::list<std::string>& GetGroups() const;

  void                          SetTypeFieldInterp( int TypeFieldInterp );
  int                           GetTypeFieldInterp() const;
  void                          AddFieldInterpType( const char* FieldInterp, int TypeInterp );
  void                          SupprFieldInterp( const char* FieldInterp );
  void                          SupprFieldInterps();
  const std::list<std::string>& GetFieldInterps() const;

// Liens avec les autres structures
  void                          SetCaseCreation( const char* NomCasCreation );
  std::string                   GetCaseCreation() const;

  void                          LinkIteration( const char* NomIter );
  void                          UnLinkIteration( const char* NomIter );
  void                          UnLinkIterations();
  const std::list<std::string>& GetIterations() const;

  void                          AddZone( const char* NomZone, int TypeUse );
  void                          SupprZone( const char* NomZone );
  void                          SupprZones();
  const std::list<std::string>& GetZones() const;

private:
  std::string                   _Name;
  std::string                   _NomCasCreation;

  int                           _TypeAdap; // -1 pour une adapation Uniforme,
                                           //  0 si l adaptation depend des zones,
                                           //  1 pour des champs

  int                           _TypeRaff;
  int                           _TypeDera;

  std::string                   _Field;
  int                           _TypeThR;
  int                           _TypeThC;
  double                        _ThreshR;
  double                        _ThreshC;
  int                           _UsField;
  int                           _UsCmpI;
  int                           _TypeFieldInterp; // 0 pour aucune interpolation,
                                                  // 1 pour interpolation de tous les champs,
                                                  // 2 pour une liste
  int                           _NivMax;
  double                        _DiamMin;
  int                           _AdapInit;
  int                           _ExtraOutput;

  std::list<std::string>        _ListIter;
  std::list<std::string>        _ListZone;
  std::list<std::string>        _ListComp;
  std::list<std::string>        _ListGroupSelected;
  std::list<std::string>        _ListFieldInterp;
};

class HOMARDIMPL_EXPORT HOMARD_Iteration
{
public:
  HOMARD_Iteration();
  ~HOMARD_Iteration();

  // Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

  // Caracteristiques
  void                          SetDirNameLoc( const char* NomDir );
  std::string                   GetDirNameLoc() const;

  void                          SetNumber( int NumIter );
  int                           GetNumber() const;

  void                          SetState( int etat );
  int                           GetState() const;

  void                          SetMeshName( const char* NomMesh );
  std::string                   GetMeshName() const;

  void                          SetMeshFile( const char* MeshFile );
  std::string                   GetMeshFile() const;

  void                          SetFieldFile( const char* FieldFile );
  std::string                   GetFieldFile() const;
// Instants pour le champ de pilotage
  void                          SetTimeStep( int TimeStep );
  void                          SetTimeStepRank( int TimeStep, int Rank );
  void                          SetTimeStepRankLast();
  int                           GetTimeStep() const;
  int                           GetRank() const;
// Instants pour un champ a interpoler
  void                          SetFieldInterpTimeStep( const char* FieldInterp, int TimeStep );
  void                          SetFieldInterpTimeStepRank( const char* FieldInterp, int TimeStep, int Rank );
  const std::list<std::string>& GetFieldInterpsTimeStepRank() const;
  void                          SetFieldInterp( const char* FieldInterp );
  const std::list<std::string>& GetFieldInterps() const;
  void                          SupprFieldInterps();

  void                          SetLogFile( const char* LogFile );
  std::string                   GetLogFile() const;

  void                          SetFileInfo( const char* FileInfo );
  std::string                   GetFileInfo() const;

// Liens avec les autres iterations
  void                          LinkNextIteration( const char* NomIteration );
  void                          UnLinkNextIteration( const char* NomIteration );
  void                          UnLinkNextIterations();
  const std::list<std::string>& GetIterations() const;

  void                          SetIterParentName( const char* iterParent );
  std::string                   GetIterParentName() const;

// Liens avec les autres structures
  void                          SetCaseName( const char* NomCas );
  std::string                   GetCaseName() const;

  void                          SetHypoName( const char* NomHypo );
  std::string                   GetHypoName() const;

// Divers
  void                          SetInfoCompute( int MessInfo );
  int                           GetInfoCompute() const;

private:
  std::string                   _Name;
  int                           _Etat;
  int                           _NumIter;
  std::string                   _NomMesh;
  std::string                   _MeshFile;
  std::string                   _FieldFile;
  int                           _TimeStep;
  int                           _Rank;
  std::string                   _LogFile;
  std::string                   _IterParent;
  std::string                   _NomHypo;
  std::string                   _NomCas;
  std::string                   _NomDir;
  std::list<std::string>        _mesIterFilles;
  std::string                   _FileInfo;
  int                           _MessInfo;
  // La liste des champs retenus par l'hypothese
  std::list<std::string>        _ListFieldInterp;
  // La liste des triplets (champs, pas de temps, numero d'ordre) retenus par l'iteration
  std::list<std::string>        _ListFieldInterpTSR;
};

// HOMARD/FrontTrack

class FrontTrack
{
public:

  /*!
   * \brief Relocate nodes to lie on geometry
   *  \param [in] theInputMedFile - a MED file holding a mesh including nodes that will be
   *         moved onto the geometry
   *  \param [in] theOutputMedFile - a MED file to create, that will hold a modified mesh
   *  \param [in] theInputNodeFiles - an array of names of files describing groups of nodes that
   *         will be moved onto the geometry
   *  \param [in] theXaoFileName - a path to a file in XAO format  holding the geometry and
   *         the geometrical groups.
   *  \param [in] theIsParallel - if \c true, all processors are used to treat boundary shapes
   *          in parallel.
   */
  void track( const std::string&                 theInputMedFile,
              const std::string&                 theOutputMedFile,
              const std::vector< std::string > & theInputNodeFiles,
              const std::string&                 theXaoFileName,
              bool                               theIsParallel=true);

};


struct FT_RealProjector;

/*!
 * \brief Projector of a point to a boundary shape. Wrapper of a real projection algo
 */
class FT_Projector
{
public:

  FT_Projector(const TopoDS_Shape& shape = TopoDS_Shape());
  FT_Projector(const FT_Projector& other);
  ~FT_Projector();

  // initialize with a boundary shape, compute the bounding box
  void setBoundaryShape(const TopoDS_Shape& shape);

  // return the boundary shape
  const TopoDS_Shape& getShape() const { return _shape; }

  // return the bounding box
  const Bnd_Box getBoundingBox() const { return _bndBox; }


  // create a real projector
  void prepareForProjection();

  // return true if a previously found solution can be used to speed up the projection
  bool canUsePrevSolution() const;

  // return true if projection is not needed
  bool isPlanarBoundary() const;


  // switch a mode of usage of prevSolution.
  // If projection fails, to try to project without usage of prevSolution.
  // By default this mode is off
  void tryWithoutPrevSolution( bool toTry ) { _tryWOPrevSolution = toTry; }

  // project a point to the boundary shape
  bool project( const gp_Pnt& point,
                const double  maxDist2,
                gp_Pnt&       projection,
                double*       newSolution,
                const double* prevSolution = 0);

  // project a point to the boundary shape and check if the projection is within the shape boundary
  bool projectAndClassify( const gp_Pnt& point,
                           const double  maxDist2,
                           gp_Pnt&       projection,
                           double*       newSolution,
                           const double* prevSolution = 0);

  // check if a point lies on the boundary shape
  bool isOnShape( const gp_Pnt& point,
                  const double  tol2,
                  double*       newSolution,
                  const double* prevSolution = 0);

private:

  FT_RealProjector* _realProjector;
  Bnd_Box           _bndBox;
  TopoDS_Shape      _shape;
  bool              _tryWOPrevSolution;
};

namespace FT_Utils
{
  // Check if a file exists
  bool fileExists( const std::string& path );

  // Check if a file can be created/overwritten
  bool canWrite( const std::string& path );

  // Transform anything printable to a string
  template< typename T> std::string toStr( const T& t )
  {
    std::ostringstream s;
    s << t;
    return s.str();
  }

  //--------------------------------------------------------------------------------------------
  /*!
   * \brief Return projectors by group name
   */
  struct XaoGroups
  {
    XaoGroups( const XAO::Xao* xao );

    int getProjectors( const std::string&                   groupName,
                       const int                            dim,
                       const std::vector< FT_Projector > &  allProjectors,
                       std::vector< const FT_Projector* > & groupProjectors ) const;
  private:

    typedef std::multimap< std::string, XAO::Group* > TGroupByNameMap;
    TGroupByNameMap _xaoGroups[ 2 ]; // by dim
  };
} // namespace FT_Utils

/*!
 * \brief Node group and geometry to project onto
 */
class FT_NodesOnGeom
{
public:

  // read node IDs form a file and try to find a boundary sub-shape by name
  void read( const std::string&            nodesFile,
             const FT_Utils::XaoGroups&    xaoGroups,
             MEDCoupling::DataArrayDouble* nodeCoords,
             std::vector< FT_Projector > * allProjectorsByDim);

  // chose boundary shapes by evaluating distance between nodes and shapes
  //void choseShape( const std::vector< FT_Utils::ShapeAndBndBox >& shapeAndBoxList );

  // project nodes to the shapes and move them to new positions
  void projectAndMove();

  // return true if all nodes were successfully relocated
  bool isOK() const { return _OK; }

  // return dimension of boundary shapes
  int getShapeDim() const { return _shapeDim; }

  // return nb of nodes to move
  int nbNodes() const { return _nodes.size(); }


private:

  // put nodes in the order for optimal projection
  void putNodesInOrder();

  // get node coordinates
  gp_Pnt getPoint( const int nodeID );

  // change node coordinates
  void moveNode( const int nodeID, const gp_Pnt& xyz );


  // Ids of a node to move and its 2 or 4 neighbors
  struct FT_NodeToMove
  {
    int                _nodeToMove;
    std::vector< int > _neighborNodes;

    double             _params[2];   // parameters on shape (U or UV) found by projection
    double            *_nearParams; // _params of a neighbor already projected node

    FT_NodeToMove(): _nearParams(0) {}
  };

  std::vector< std::string >    _groupNames;
  int                           _shapeDim;   // dimension of boundary shapes
  std::vector< FT_NodeToMove >  _nodes;      // ids of nodes to move and their neighbors
  std::vector< FT_Projector >   _projectors; // FT_Projector's initialized with boundary shapes
  std::vector< FT_Projector > * _allProjectors; // FT_Projector's for all shapes of _shapeDim
  MEDCoupling::DataArrayDouble* _nodeCoords;
  bool                          _OK;          // projecting is successful 

  // map of { FT_NodeToMove::_neighborNodes[i] } to { FT_NodeToMove* }
  // this map is used to find neighbor nodes
  typedef NCollection_DataMap< int, std::vector< FT_NodeToMove* > > TNodeIDToLinksMap;
  TNodeIDToLinksMap             _neigborsMap;
  std::vector<int>              _nodesOrder;

};

/*!
 * \brief Container of node groups.
 */
class FT_NodeGroups
{
public:

  // Load node groups from files
  void read( const std::vector< std::string >& nodeFiles,
             const XAO::Xao*                   xaoGeom,
             MEDCoupling::DataArrayDouble*     nodeCoords );

  // return number of groups of nodes to move
  int nbOfGroups() const { return _nodesOnGeom.size(); }

  // Move nodes of a group in parallel mode
  void operator() ( const int groupIndex ) const
  {
    const_cast< FT_NodeGroups* >( this )->projectAndMove( groupIndex );
  }

  // Project and move nodes of a given group of nodes
  void projectAndMove( const int groupIndex );

  // return true if all nodes were successfully relocated
  bool isOK() const;

  // print some statistics on node groups
  void dumpStat() const;

private:

  std::vector< FT_NodesOnGeom > _nodesOnGeom;
  std::vector< FT_Projector >   _projectors[2]; // curves and surfaces separately

};

}; // namespace SMESHHOMARDImpl

#endif
