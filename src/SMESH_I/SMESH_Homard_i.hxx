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

// Remarques :
// L'ordre de description des fonctions est le meme dans tous les fichiers
// HOMARD_aaaa.idl, HOMARD_aaaa.hxx, HOMARD_aaaa.cxx, HOMARD_aaaa_i.hxx, HOMARD_aaaa_i.cxx :
// 1. Les generalites : Name, Delete, DumpPython, Dump, Restore
// 2. Les caracteristiques
// 3. Le lien avec les autres structures
//
// Quand les 2 fonctions Setxxx et Getxxx sont presentes, Setxxx est decrit en premier
//

#ifndef _SMESH_HOMARD_I_HXX_
#define _SMESH_HOMARD_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Homard)
#include CORBA_CLIENT_HEADER(SALOMEDS)
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(SALOME_ModuleCatalog)
#include CORBA_CLIENT_HEADER(SMESH_Gen)

#include "SMESH.hxx"
#include "SMESH_Homard.hxx"
//#include "FrontTrack.hxx"
#include "SALOME_Component_i.hxx"
#include "SALOME_NamingService.hxx"
#include "Utils_CorbaException.hxx"

#include <map>
#include <set>
#include <string>
#include <vector>

//namespace SMESHHOMARDImpl
//{
//  class HOMARD_Boundary;
//  class HOMARD_Cas;
//  class HOMARD_Hypothesis;
//  class HOMARD_Iteration;
//};

namespace SMESHHOMARD_I
{

class SMESH_I_EXPORT HOMARD_Boundary_i:
  public virtual Engines_Component_i,
  public virtual POA_SMESHHOMARD::HOMARD_Boundary,
  public virtual PortableServer::ServantBase
{
public:
  HOMARD_Boundary_i( CORBA::ORB_ptr orb, SMESHHOMARD::HOMARD_Gen_var gen_i );
  HOMARD_Boundary_i();

  virtual ~HOMARD_Boundary_i();

  // Generalites
  void                   SetName( const char* Name );
  char*                  GetName();

  CORBA::Long            Delete();

  char*                  GetDumpPython();

  std::string            Dump() const;
  bool                   Restore( const std::string& stream );

  // Caracteristiques
  void                   SetType( CORBA::Long Type );
  CORBA::Long            GetType();

  void                   SetMeshName( const char* MeshName );
  char*                  GetMeshName();

  void                   SetDataFile( const char* DataFile );
  char*                  GetDataFile();

  void                   SetCylinder( double Xcentre, double Ycentre, double ZCentre,
                                      double Xaxe, double Yaxe, double Zaxe,
                                      double rayon );
  void                   SetSphere( double Xcentre, double Ycentre, double ZCentre,
                                    double rayon );
  void                   SetConeR( double Xcentre1, double Ycentre1, double Zcentre1, double Rayon1,
                                   double Xcentre2, double Ycentre2, double Zcentre2, double Rayon2);
  void                   SetConeA( double Xaxe, double Yaxe, double Zaxe, double Angle,
                                   double Xcentre, double Ycentre, double ZCentre);
  void                   SetTorus( double Xcentre, double Ycentre, double ZCentre,
                                      double Xaxe, double Yaxe, double Zaxe,
                                      double rayonRev, double rayonPri );

  SMESHHOMARD::double_array*  GetCoords();

  void                   SetLimit( double Xincr, double Yincr, double Zincr);
  SMESHHOMARD::double_array*  GetLimit();

  void                   AddGroup( const char* Group);
  void                   SetGroups(const SMESHHOMARD::ListGroupType& ListGroup);
  SMESHHOMARD::ListGroupType* GetGroups();

// Liens avec les autres structures
  void                   SetCaseCreation( const char* NomCaseCreation );
  char*                  GetCaseCreation();


private:
  SMESHHOMARDImpl::HOMARD_Boundary*     myHomardBoundary;

  CORBA::ORB_ptr         _orb;
  SMESHHOMARD::HOMARD_Gen_var _gen_i;
};

class SMESH_I_EXPORT HOMARD_Cas_i:
  public virtual Engines_Component_i,
  public virtual POA_SMESHHOMARD::HOMARD_Cas,
  public virtual PortableServer::ServantBase
{
public:
  HOMARD_Cas_i( CORBA::ORB_ptr orb, SMESHHOMARD::HOMARD_Gen_var gen_i );
  HOMARD_Cas_i();

  virtual ~HOMARD_Cas_i();

  // Generalites
  void                   SetName( const char* Name );
  char*                  GetName();

  CORBA::Long            Delete( CORBA::Long Option );

  char*                  GetDumpPython();

  std::string            Dump() const;
  bool                   Restore( const std::string& stream );

  // Caracteristiques
  void                   SetDirName( const char* NomDir );
  char*                  GetDirName();

  CORBA::Long            GetState();

  CORBA::Long            GetNumberofIter();

  void                   SetConfType( CORBA::Long ConfType );
  CORBA::Long            GetConfType();

  void                   SetExtType( CORBA::Long ExtType );
  CORBA::Long            GetExtType();

  void                   SetBoundingBox( const SMESHHOMARD::extrema& LesExtremes );
  SMESHHOMARD::extrema*       GetBoundingBox();

  void                   AddGroup( const char* Group);
  void                   SetGroups(const SMESHHOMARD::ListGroupType& ListGroup);
  SMESHHOMARD::ListGroupType* GetGroups();

  void                   AddBoundary(const char* Boundary);
  void                   AddBoundaryGroup(const char* Boundary, const char* Group);
  SMESHHOMARD::ListBoundaryGroupType*  GetBoundaryGroup();
  void                   SupprBoundaryGroup( );

  void                   SetPyram( CORBA::Long Pyram );
  CORBA::Long            GetPyram();

  void                   MeshInfo(CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte);

// Liens avec les autres structures
  char*                  GetIter0Name();
  SMESHHOMARD::HOMARD_Iteration_ptr GetIter0();

  SMESHHOMARD::HOMARD_Iteration_ptr  NextIteration( const char* Name );

  SMESHHOMARD::HOMARD_Iteration_ptr  LastIteration();

  void                   AddIteration( const char* NomIteration );

private:
  SMESHHOMARDImpl::HOMARD_Cas*          myHomardCas;

  CORBA::ORB_ptr         _orb;
  SMESHHOMARD::HOMARD_Gen_var _gen_i;
};

class SMESH_I_EXPORT HOMARD_Hypothesis_i:
  public virtual Engines_Component_i,
  public virtual POA_SMESHHOMARD::HOMARD_Hypothesis,
  public virtual PortableServer::ServantBase
{
public:
  HOMARD_Hypothesis_i( CORBA::ORB_ptr orb, SMESHHOMARD::HOMARD_Gen_var gen_i );
  HOMARD_Hypothesis_i();

  virtual ~HOMARD_Hypothesis_i();

// Generalites
  void                   SetName( const char* Name );
  char*                  GetName();

  CORBA::Long            Delete();

  char*                  GetDumpPython();

  std::string            Dump() const;
  bool                   Restore( const std::string& stream );

// Caracteristiques
  void                   SetUnifRefinUnRef( CORBA::Long TypeRaffDera );
  SMESHHOMARD::listeTypes*    GetAdapRefinUnRef();
  CORBA::Long            GetAdapType();
  CORBA::Long            GetRefinType();
  CORBA::Long            GetUnRefType();

  void                   SetField( const char* FieldName );
  char*                  GetFieldName();
  void                   SetUseField(CORBA::Long UsField);
  SMESHHOMARD::InfosHypo*     GetField();

  void                   SetUseComp(CORBA::Long UsCmpI);
  void                   AddComp( const char* NomComp );
  void                   SupprComp( const char* NomComp );
  void                   SupprComps();
  SMESHHOMARD::listeComposantsHypo* GetComps();

  void                   SetRefinThr(CORBA::Long TypeThR, CORBA::Double ThreshR);
  CORBA::Long            GetRefinThrType();
  void                   SetUnRefThr(CORBA::Long TypeThC, CORBA::Double ThreshC);
  CORBA::Long            GetUnRefThrType();

  void                   SetNivMax( CORBA::Long NivMax );
  CORBA::Long            GetNivMax();

  void                   SetDiamMin( CORBA::Double DiamMin );
  CORBA::Double          GetDiamMin();

  void                   SetAdapInit( CORBA::Long AdapInit );
  CORBA::Long            GetAdapInit();

  void                   SetExtraOutput( CORBA::Long ExtraOutput );
  CORBA::Long            GetExtraOutput();

  void                   AddGroup( const char* Group);
  void                   SupprGroup( const char* Group );
  void                   SupprGroups();
  void                   SetGroups(const SMESHHOMARD::ListGroupType& ListGroup);
  SMESHHOMARD::ListGroupType* GetGroups();

  void                   SetTypeFieldInterp( CORBA::Long TypeFieldInterp );
  CORBA::Long            GetTypeFieldInterp();
  void                   AddFieldInterp( const char* FieldInterp );
  void                   AddFieldInterpType( const char* FieldInterp, CORBA::Long TypeInterp );
  void                   SupprFieldInterp( const char* FieldInterp );
  void                   SupprFieldInterps();
  SMESHHOMARD::listeFieldInterpsHypo* GetFieldInterps();

// Liens avec les autres structures
  void                   SetCaseCreation( const char* NomCaseCreation );
  char*                  GetCaseCreation();

  void                   LinkIteration( const char* NomIteration );
  void                   UnLinkIteration( const char* NomIteration );
  SMESHHOMARD::listeIters*    GetIterations();

private:
  SMESHHOMARDImpl::HOMARD_Hypothesis* myHomardHypothesis;

  CORBA::ORB_ptr               _orb;
  SMESHHOMARD::HOMARD_Gen_var  _gen_i;
};

class SMESH_I_EXPORT HOMARD_Iteration_i:
  public virtual Engines_Component_i,
  public virtual POA_SMESHHOMARD::HOMARD_Iteration,
  public virtual PortableServer::ServantBase
{
public:
  HOMARD_Iteration_i( CORBA::ORB_ptr orb,
                      SMESHHOMARD::HOMARD_Gen_var gen_i );
  HOMARD_Iteration_i();

  virtual ~HOMARD_Iteration_i();

// Generalites
  void                   SetName( const char* Name );
  char*                  GetName();

  CORBA::Long            Delete( CORBA::Long Option, bool doRemoveWorkingFiles );

  char*                  GetDumpPython();

  std::string            Dump() const;
  bool                   Restore( const std::string& stream );

// Caracteristiques
  void                   SetDirNameLoc( const char* NomDir );
  char*                  GetDirNameLoc();

  char*                  GetDirName();

  void                   SetNumber( CORBA::Long NumIter );
  CORBA::Long            GetNumber();

  void                   SetState( CORBA::Long etat );
  CORBA::Long            GetState();

  void                   SetMeshName( const char* NomMesh );
  char*                  GetMeshName();

  void                   SetMeshFile( const char* MeshFile );
  char*                  GetMeshFile();

  void                   SetFieldFile( const char* FieldFile );
  char*                  GetFieldFile();
// Instants pour le champ de pilotage
  void                   SetTimeStep( CORBA::Long TimeStep );
  void                   SetTimeStepRank( CORBA::Long TimeStep, CORBA::Long Rank );
  void                   SetTimeStepRankLast();
  CORBA::Long            GetTimeStep();
  CORBA::Long            GetRank();
// Instants pour un champ a interpoler
  void                   SetFieldInterpTimeStep( const char* FieldInterp, CORBA::Long TimeStep );
  void                   SetFieldInterpTimeStepRank( const char* FieldInterp, CORBA::Long TimeStep, CORBA::Long Rank );
  SMESHHOMARD::listeFieldInterpTSRsIter* GetFieldInterpsTimeStepRank();
  void                   SetFieldInterp( const char* FieldInterp );
  SMESHHOMARD::listeFieldInterpsIter* GetFieldInterps();
  void                   SupprFieldInterps();

  void                   SetLogFile( const char* LogFile );
  char*                  GetLogFile();

  CORBA::Long            Compute(CORBA::Long etatMenage, CORBA::Long Option);

  void                   MeshInfo(CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte);

  void                   MeshInfoOption(CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte, CORBA::Long Option);

  void                   SetFileInfo( const char* FileInfo );
  char*                  GetFileInfo();

  // Liens avec les autres iterations
  SMESHHOMARD::HOMARD_Iteration_ptr NextIteration( const char* Name);

  void                   LinkNextIteration( const char* NomIteration );
  void                   UnLinkNextIteration( const char* NomIteration );
  SMESHHOMARD::listeIterFilles* GetIterations();

  void                   SetIterParentName( const char* NomIterParent );
  char*                  GetIterParentName();
  SMESHHOMARD::HOMARD_Iteration_ptr GetIterParent();

// Liens avec les autres structures
  void                   SetCaseName( const char* NomCas );
  char*                  GetCaseName();

  void                   AssociateHypo( const char* NomHypo);
  void                   SetHypoName( const char* NomHypo );
  char*                  GetHypoName();

// Divers
  void                   SetInfoCompute( CORBA::Long MessInfo );
  CORBA::Long            GetInfoCompute();

private:
  SMESHHOMARDImpl::HOMARD_Iteration*    myHomardIteration;

  CORBA::ORB_ptr         _orb;
  SMESHHOMARD::HOMARD_Gen_var _gen_i;
};

class SMESH_I_EXPORT HOMARD_Gen_i : public virtual SALOME::GenericObj_i,
                                         public virtual POA_SMESHHOMARD::HOMARD_Gen
{
public:
  HOMARD_Gen_i();
  virtual ~HOMARD_Gen_i();


// Generalites

// Les creations
  SMESHHOMARD::HOMARD_Boundary_ptr     CreateBoundary (const char* nomBoundary, CORBA::Long typeBoundary);
  SMESHHOMARD::HOMARD_Boundary_ptr     CreateBoundaryCAO(const char* nomBoundary,
                                                    const char* DataFile);
  SMESHHOMARD::HOMARD_Boundary_ptr     CreateBoundaryDi (const char* nomBoundary,
                                                    const char* MeshName, const char* DataFile);
  SMESHHOMARD::HOMARD_Boundary_ptr     CreateBoundaryCylinder (const char* nomBoundary,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxis, CORBA::Double Yaxis, CORBA::Double Zaxis,
                                      CORBA::Double Rayon);
  SMESHHOMARD::HOMARD_Boundary_ptr     CreateBoundarySphere (const char* nomBoundary,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Rayon);
  SMESHHOMARD::HOMARD_Boundary_ptr     CreateBoundaryConeR (const char* nomBoundary,
                                      CORBA::Double Xcentre1, CORBA::Double Ycentre1, CORBA::Double Zcentre1, CORBA::Double Rayon1,
                                      CORBA::Double Xcentre2, CORBA::Double Ycentre2, CORBA::Double Zcentre2, CORBA::Double Rayon2);
  SMESHHOMARD::HOMARD_Boundary_ptr     CreateBoundaryConeA (const char* nomBoundary,
                                      CORBA::Double Xaxis, CORBA::Double Yaxis, CORBA::Double Zaxis, CORBA::Double Angle,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre);
  SMESHHOMARD::HOMARD_Boundary_ptr     CreateBoundaryTorus (const char* nomBoundary,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxis, CORBA::Double Yaxis, CORBA::Double Zaxis,
                                      CORBA::Double RayonRev, CORBA::Double RayonPri);

  SMESHHOMARD::HOMARD_Cas_ptr CreateCase (const char* nomCas, const char* MeshName,
                                          const char* FileName);
  SMESHHOMARD::HOMARD_Cas_ptr CreateCase0 (const char* nomCas, const char* MeshName,
                                           const char* FileName, CORBA::Long MeshOption,
                                           CORBA::Long NumeIter, CORBA::Long Option);
  std::string                 CreateCase1 (const char* DirNameStart, CORBA::Long Number);

  SMESHHOMARD::HOMARD_Hypothesis_ptr   CreateHypothesis(const char* nomHypothesis);

  SMESHHOMARD::HOMARD_Iteration_ptr    CreateIteration (const char* nomIter, const char* nomIterParent);

  // Les informations
  SMESHHOMARD::HOMARD_Boundary_ptr     GetBoundary   (const char* nomBoundary);
  SMESHHOMARD::HOMARD_Cas_ptr          GetCase       (const char* nomCas);
  SMESHHOMARD::HOMARD_Hypothesis_ptr   GetHypothesis (const char* nomHypothesis);
  SMESHHOMARD::HOMARD_Iteration_ptr    GetIteration  (const char* nomIteration);

  SMESHHOMARD::listeBoundarys*         GetAllBoundarysName();
  SMESHHOMARD::listeCases*             GetAllCasesName();
  SMESHHOMARD::listeHypotheses*        GetAllHypothesesName();
  SMESHHOMARD::listeIterations*        GetAllIterationsName();

  void                            MeshInfo      (const char* nomCas,
                                                 const char* MeshName, const char* FileName, const char* DirName,
                                                 CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte);

  SMESHHOMARD::HOMARD_Iteration_ptr    LastIteration  (const char* nomCas);

  // Liens entre structures
  void                            InvalideBoundary(const char* nomBoundary);
  void                            InvalideHypo(const char* nomHypo);
  void                            InvalideIter(const char* nomIter);
  void                            InvalideIterOption(const char* nomIter, CORBA::Long Option,
                                                     bool doRemoveWorkingFiles);
  void                            InvalideIterInfo(const char* nomIter);

  CORBA::Long                     DeleteBoundary(const char* nomBoundary);
  CORBA::Long                     DeleteCase(const char* nomCas, CORBA::Long Option);
  CORBA::Long                     DeleteHypo(const char* nomHypothesis);
  CORBA::Long                     DeleteIteration(const char* nomIter,
                                                  CORBA::Long Option,
                                                  bool doRemoveWorkingFiles);
  CORBA::Long                     DeleteIterationOption(const char* nomIter,
                                                        CORBA::Long Option1,
                                                        CORBA::Long Option2,
                                                        bool doRemoveWorkingFiles);

  void  AssociateIterHypo(const char* nomIter, const char* nomHypothesis);

  void  AssociateCaseIter(const char* nomCas, const char* nomIter,
                          const char* labelIter);

  // Actions
  void  SetEtatIter(const char* nomIter,const CORBA::Long Etat);
  char* CreateDirNameIter(const char* nomrep, CORBA::Long num );

  CORBA::Long Compute(const char* nomIteration, CORBA::Long etatMenage,
                      CORBA::Long modeHOMARD,
                      CORBA::Long Option1, CORBA::Long Option2);
  CORBA::Long ComputeAdap(SMESHHOMARD::HOMARD_Cas_var myCase,
                          SMESHHOMARD::HOMARD_Iteration_var myIteration,
                          CORBA::Long etatMenage, SMESHHOMARDImpl::HomardDriver* myDriver,
                          CORBA::Long Option1, CORBA::Long Option2);
  CORBA::Long ComputeCAO(SMESHHOMARD::HOMARD_Cas_var myCase,
                         SMESHHOMARD::HOMARD_Iteration_var myIteration,
                         CORBA::Long Option1, CORBA::Long Option2);
  CORBA::Long ComputeCAObis(SMESHHOMARD::HOMARD_Iteration_var myIteration,
                            CORBA::Long Option1, CORBA::Long Option2);
  char* ComputeDirManagement(SMESHHOMARD::HOMARD_Cas_var myCase,
                             SMESHHOMARD::HOMARD_Iteration_var myIteration,
                             CORBA::Long etatMenage);
  char* ComputeDirPaManagement(SMESHHOMARD::HOMARD_Cas_var myCase,
                               SMESHHOMARD::HOMARD_Iteration_var myIteration);
  void  DriverTexteField(SMESHHOMARD::HOMARD_Iteration_var myIteration,
                         SMESHHOMARD::HOMARD_Hypothesis_var myHypo,
                         SMESHHOMARDImpl::HomardDriver* myDriver);
  int   DriverTexteBoundary(SMESHHOMARD::HOMARD_Cas_var myCase,
                            SMESHHOMARDImpl::HomardDriver* myDriver);
  void  DriverTexteFieldInterp(SMESHHOMARD::HOMARD_Iteration_var myIteration,
                               SMESHHOMARD::HOMARD_Hypothesis_var myHypo,
                               SMESHHOMARDImpl::HomardDriver* myDriver);

  char* VerifieDir(const char* nomDir);

  void  PublishResultInSmesh(const char* NomFich, CORBA::Long Option);
  void  DeleteResultInSmesh(std::string NomFich, std::string MeshName);
  void  PublishMeshIterInSmesh(const char* NomIter);

  // Preferences
  void                            SetPreferences();
  void                            SetLanguageShort(const char* LanguageShort);
  char*                           GetLanguageShort();
  void                            SetPublisMesh(CORBA::Long PublisMeshIN, CORBA::Long PublisMeshOUT);
  CORBA::Long                     GetPublisMeshIN();
  CORBA::Long                     GetPublisMeshOUT();

private:
  SMESHHOMARD::HOMARD_Boundary_ptr     newBoundary();
  SMESHHOMARD::HOMARD_Cas_ptr          newCase();
  SMESHHOMARD::HOMARD_Hypothesis_ptr   newHypothesis();
  SMESHHOMARD::HOMARD_Iteration_ptr    newIteration();

  std::string GetStringInTexte( const std::string Texte, const std::string String, int option );

private:
  struct StudyContext
  {
    std::map<std::string, SMESHHOMARD::HOMARD_Boundary_var>   _mesBoundarys;
    std::map<std::string, SMESHHOMARD::HOMARD_Cas_var>        _mesCas;
    std::map<std::string, SMESHHOMARD::HOMARD_Hypothesis_var> _mesHypotheses;
    std::map<std::string, SMESHHOMARD::HOMARD_Iteration_var>  _mesIterations;
  };

  SMESHHOMARDImpl::HOMARD_Gen* myHomard;
  StudyContext                 myStudyContext;

  // Preferences
  std::string _Langue;
  std::string _LangueShort;
  int _PublisMeshIN;
  int _PublisMeshOUT;
};

SMESH_I_EXPORT int                    MEDFileExist( const char * aFile );
SMESH_I_EXPORT std::vector<double>    GetBoundingBoxInMedFile( const char * aFile);
SMESH_I_EXPORT std::set<std::string>  GetListeGroupesInMedFile(const char * aFile);

}; // namespace SMESHHOMARD_I

#endif
