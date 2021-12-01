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

#include "SMESH_Homard_i.hxx"
#include "SMESH_Homard.hxx"

#include <SMESH_Gen_i.hxx>

//#include "FrontTrack.hxx"

#include "utilities.h"
#include "Basics_Utils.hxx"
#include "Basics_DirUtils.hxx"
#include "Utils_SINGLETON.hxx"
#include "Utils_CorbaException.hxx"
#include "SALOMEDS_Tool.hxx"
#include "SALOME_LifeCycleCORBA.hxx"
#include "SALOMEconfig.h"

#include <vector>
#include <cmath>
#include <cstdlib>
#include <stdlib.h>
#include <sys/stat.h>
#include <algorithm>

#include <med.h>

#ifdef WIN32
#include <direct.h>
#else
#include <dirent.h>
#endif

#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <set>
#include <vector>
#include <stdio.h>

using namespace std;

SMESHHOMARD::HOMARD_Gen_ptr SMESH_Gen_i::CreateHOMARD_ADAPT()
{
  SMESHHOMARD_I::HOMARD_Gen_i* aHomardGen = new SMESHHOMARD_I::HOMARD_Gen_i();
  SMESHHOMARD::HOMARD_Gen_var anObj = aHomardGen->_this();
  return anObj._retn();
}

namespace SMESHHOMARD_I
{

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Boundary_i::HOMARD_Boundary_i()
{
  MESSAGE( "Default constructor, not for use" );
  ASSERT( 0 );
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Boundary_i::HOMARD_Boundary_i( CORBA::ORB_ptr orb,
                                      SMESHHOMARD::HOMARD_Gen_var engine )
{
  MESSAGE( "HOMARD_Boundary_i" );
  _gen_i = engine;
  _orb = orb;
  myHomardBoundary = new SMESHHOMARDImpl::HOMARD_Boundary();
  ASSERT( myHomardBoundary );
}
//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Boundary_i::~HOMARD_Boundary_i()
{
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Boundary_i::SetName( const char* Name )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetName( Name );
}
//=============================================================================
char* HOMARD_Boundary_i::GetName()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetName().c_str() );
}
//=============================================================================
CORBA::Long HOMARD_Boundary_i::Delete()
{
  ASSERT( myHomardBoundary );
  char* BoundaryName = GetName();
  MESSAGE ( "Delete : destruction de la frontiere " << BoundaryName );
  return _gen_i->DeleteBoundary(BoundaryName);
}
//=============================================================================
char* HOMARD_Boundary_i::GetDumpPython()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetDumpPython().c_str() );
}
//=============================================================================
std::string HOMARD_Boundary_i::Dump() const
{
  return SMESHHOMARDImpl::Dump( *myHomardBoundary );
}
//=============================================================================
bool HOMARD_Boundary_i::Restore( const std::string& stream )
{
  return SMESHHOMARDImpl::Restore( *myHomardBoundary, stream );
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Boundary_i::SetType( CORBA::Long Type )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetType( Type );
}
//=============================================================================
CORBA::Long HOMARD_Boundary_i::GetType()
{
  ASSERT( myHomardBoundary );
  return  CORBA::Long( myHomardBoundary->GetType() );
}
//=============================================================================
void HOMARD_Boundary_i::SetMeshName( const char* MeshName )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetMeshName( MeshName );
}
//=============================================================================
char* HOMARD_Boundary_i::GetMeshName()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetMeshName().c_str() );
}
//=============================================================================
void HOMARD_Boundary_i::SetDataFile( const char* DataFile )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetDataFile( DataFile );
  int PublisMeshIN = _gen_i->GetPublisMeshIN ();
  if ( PublisMeshIN != 0 ) { _gen_i->PublishResultInSmesh(DataFile, 0); }
}
//=============================================================================
char* HOMARD_Boundary_i::GetDataFile()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetDataFile().c_str() );
}
//=============================================================================
void HOMARD_Boundary_i::SetCylinder( double X0, double X1, double X2, double X3, double X4, double X5, double X6 )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetCylinder( X0, X1, X2, X3, X4, X5, X6 );
}
//=============================================================================
void HOMARD_Boundary_i::SetSphere( double Xcentre, double Ycentre, double ZCentre, double rayon )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetSphere( Xcentre, Ycentre, ZCentre, rayon );
}
//=============================================================================
void HOMARD_Boundary_i::SetConeR( double Xcentre1, double Ycentre1, double Zcentre1, double Rayon1, double Xcentre2, double Ycentre2, double Zcentre2, double Rayon2)
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetConeR( Xcentre1, Ycentre1, Zcentre1, Rayon1, Xcentre2, Ycentre2, Zcentre2, Rayon2 );
}
//=============================================================================
void HOMARD_Boundary_i::SetConeA( double Xaxe, double Yaxe, double Zaxe, double Angle, double Xcentre, double Ycentre, double Zcentre)
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetConeA( Xaxe, Yaxe, Zaxe, Angle, Xcentre, Ycentre, Zcentre );
}
//=============================================================================
void HOMARD_Boundary_i::SetTorus( double X0, double X1, double X2, double X3, double X4, double X5, double X6, double X7 )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetTorus( X0, X1, X2, X3, X4, X5, X6, X7 );
}
//=============================================================================
SMESHHOMARD::double_array* HOMARD_Boundary_i::GetCoords()
{
  ASSERT( myHomardBoundary );
  SMESHHOMARD::double_array_var aResult = new SMESHHOMARD::double_array();
  std::vector<double> mesCoor = myHomardBoundary->GetCoords();
  aResult->length( mesCoor .size() );
  std::vector<double>::const_iterator it;
  int i = 0;
  for ( it = mesCoor.begin(); it != mesCoor.end(); it++ )
    aResult[i++] = (*it);
  return aResult._retn();
}
//=============================================================================
void HOMARD_Boundary_i::SetLimit( double Xincr, double Yincr, double Zincr )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetLimit( Xincr, Yincr, Zincr );
}
//=============================================================================
SMESHHOMARD::double_array* HOMARD_Boundary_i::GetLimit()
{
  ASSERT( myHomardBoundary );
  SMESHHOMARD::double_array_var aResult = new SMESHHOMARD::double_array();
  std::vector<double> mesCoor = myHomardBoundary->GetLimit();
  aResult->length( mesCoor .size() );
  std::vector<double>::const_iterator it;
  int i = 0;
  for ( it = mesCoor.begin(); it != mesCoor.end(); it++ )
    aResult[i++] = (*it);
  return aResult._retn();
}
//=============================================================================
void HOMARD_Boundary_i::AddGroup( const char* Group)
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->AddGroup( Group );
}
//=============================================================================
void HOMARD_Boundary_i::SetGroups(const SMESHHOMARD::ListGroupType& ListGroup)
{
  ASSERT( myHomardBoundary );
  std::list<std::string> ListString;
  for ( int i = 0; i < ListGroup.length(); i++ )
  {
      ListString.push_back(std::string(ListGroup[i]));
  }
  myHomardBoundary->SetGroups( ListString );
}
//=============================================================================
SMESHHOMARD::ListGroupType*  HOMARD_Boundary_i::GetGroups()
{
  ASSERT( myHomardBoundary );
  const std::list<std::string>& ListString = myHomardBoundary->GetGroups();
  SMESHHOMARD::ListGroupType_var aResult = new SMESHHOMARD::ListGroupType;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Boundary_i::SetCaseCreation( const char* NomCaseCreation )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetCaseCreation( NomCaseCreation );
}
//=============================================================================
char* HOMARD_Boundary_i::GetCaseCreation()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetCaseCreation().c_str() );
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Cas_i::HOMARD_Cas_i()
{
  MESSAGE( "Default constructor, not for use" );
  ASSERT( 0 );
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Cas_i::HOMARD_Cas_i( CORBA::ORB_ptr orb,
                            SMESHHOMARD::HOMARD_Gen_var engine )
{
  MESSAGE( "HOMARD_Cas_i" );
  _gen_i = engine;
  _orb = orb;
  myHomardCas = new SMESHHOMARDImpl::HOMARD_Cas();
  ASSERT( myHomardCas );
}

//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Cas_i::~HOMARD_Cas_i()
{
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Cas_i::SetName( const char* Name )
{
  ASSERT( myHomardCas );
  myHomardCas->SetName( Name );
}
//=============================================================================
char* HOMARD_Cas_i::GetName()
{
  ASSERT( myHomardCas );
  return CORBA::string_dup( myHomardCas->GetName().c_str() );
}
//=============================================================================
CORBA::Long HOMARD_Cas_i::Delete( CORBA::Long Option )
{
  ASSERT( myHomardCas );
  char* CaseName = GetName();
  MESSAGE ( "Delete : destruction du cas " << CaseName << ", Option = " << Option );
  return _gen_i->DeleteCase(CaseName, Option);
}
//=============================================================================
char* HOMARD_Cas_i::GetDumpPython()
{
  ASSERT( myHomardCas );
  return CORBA::string_dup( myHomardCas->GetDumpPython().c_str() );
}
//=============================================================================
std::string HOMARD_Cas_i::Dump() const
{
  return SMESHHOMARDImpl::Dump( *myHomardCas );
}
//=============================================================================
bool HOMARD_Cas_i::Restore( const std::string& stream )
{
  return SMESHHOMARDImpl::Restore( *myHomardCas, stream );
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Cas_i::SetDirName( const char* NomDir )
{
  ASSERT( myHomardCas );
  int codret;
  // A. recuperation du nom; on ne fait rien si c'est le meme
  char* oldrep = GetDirName();
  if ( strcmp(oldrep,NomDir) == 0 ) return;
  MESSAGE ( "SetDirName : passage de oldrep = "<< oldrep << " a NomDir = "<<NomDir);
  // B. controle de l'usage du repertoire
  char* CaseName = GetName();
  char* casenamedir = _gen_i->VerifieDir(NomDir);
  if ( ( std::string(casenamedir).size() > 0 ) & ( strcmp(CaseName,casenamedir)!=0 ) )
  {
    INFOS ( "Le repertoire " << NomDir << " est deja utilise pour le cas "<< casenamedir );
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text;
    text = "The directory " + std::string(NomDir) + " is already used for the case " + std::string(casenamedir);
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }
  // C. Changement/creation du repertoire
  codret = myHomardCas->SetDirName( NomDir );
  if ( codret != 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text;
    if ( codret == 1 ) { text = "The directory for the case cannot be modified because some iterations are already defined."; }
    else               { text = "The directory for the case cannot be reached."; }
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }
  // D. En cas de reprise, deplacement du point de depart
  if ( GetState() != 0 )
  {
    MESSAGE ( "etat : " << GetState() );
    // D.1. Nom local du repertoire de l'iteration de depart dans le repertoire actuel du cas
    SMESHHOMARD::HOMARD_Iteration_ptr Iter = GetIter0();
    char* DirNameIter = Iter->GetDirNameLoc();
    MESSAGE ( "SetDirName : nom actuel pour le repertoire de l iteration, DirNameIter = "<< DirNameIter);
    // D.2. Recherche d'un nom local pour l'iteration de depart dans le futur repertoire du cas
    char* nomDirIter = _gen_i->CreateDirNameIter(NomDir, 0 );
    MESSAGE ( "SetDirName : nom futur pour le repertoire de l iteration, nomDirIter = "<< nomDirIter);
    // D.3. Creation du futur repertoire local pour l'iteration de depart
    std::string nomDirIterTotal;
    nomDirIterTotal = std::string(NomDir) + "/" + std::string(nomDirIter);
#ifndef WIN32
    if (mkdir(nomDirIterTotal.c_str(), S_IRWXU|S_IRGRP|S_IXGRP) != 0)
#else
    if (_mkdir(nomDirIterTotal.c_str()) != 0)
#endif
    {
      MESSAGE ( "nomDirIterTotal : " << nomDirIterTotal );
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      std::string text = "The directory for the starting iteration cannot be created.";
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
    }
    // D.4. Deplacement du contenu du repertoire
    std::string oldnomDirIterTotal;
    oldnomDirIterTotal = std::string(oldrep) + "/" + std::string(DirNameIter);
    std::string commande = "mv " + std::string(oldnomDirIterTotal) + "/*" + " " + std::string(nomDirIterTotal);
    codret = system(commande.c_str());
    if ( codret != 0 )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      std::string text = "The starting point for the case cannot be moved into the new directory.";
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
    }
    commande = "rm -rf " + std::string(oldnomDirIterTotal);
    codret = system(commande.c_str());
    if ( codret != 0 )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      std::string text = "The starting point for the case cannot be deleted.";
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
    }
    // D.5. Memorisation du nom du repertoire de l'iteration
    Iter->SetDirNameLoc(nomDirIter);
  }
}
//=============================================================================
char* HOMARD_Cas_i::GetDirName()
{
  ASSERT( myHomardCas );
  return CORBA::string_dup( myHomardCas->GetDirName().c_str() );
}
//=============================================================================
CORBA::Long HOMARD_Cas_i::GetState()
{
  ASSERT( myHomardCas );
// Nom de l'iteration initiale
  char* Iter0Name = GetIter0Name();
  SMESHHOMARD::HOMARD_Iteration_ptr Iter = _gen_i->GetIteration(Iter0Name);
  int state = Iter->GetNumber();
  return state;
}
//=============================================================================
CORBA::Long HOMARD_Cas_i::GetNumberofIter()
{
  ASSERT( myHomardCas );
  return myHomardCas->GetNumberofIter();
}
//=============================================================================
void HOMARD_Cas_i::SetConfType( CORBA::Long ConfType )
{
  ASSERT( myHomardCas );
//   VERIFICATION( (ConfType>=-2) && (ConfType<=3) );
  myHomardCas->SetConfType( ConfType );
}
//=============================================================================
CORBA::Long HOMARD_Cas_i::GetConfType()
{
  ASSERT( myHomardCas );
  return myHomardCas->GetConfType();
}
//=============================================================================
void HOMARD_Cas_i::SetExtType( CORBA::Long ExtType )
{
  ASSERT( myHomardCas );
//   VERIFICATION( (ExtType>=0) && (ExtType<=2) );
  myHomardCas->SetExtType( ExtType );
}
//=============================================================================
CORBA::Long HOMARD_Cas_i::GetExtType()
{
  ASSERT( myHomardCas );
  return myHomardCas->GetExtType();
}
//=============================================================================
void HOMARD_Cas_i::SetBoundingBox( const SMESHHOMARD::extrema& LesExtrema )
{
  ASSERT( myHomardCas );
  std::vector<double> VExtrema;
  ASSERT( LesExtrema.length() == 10 );
  VExtrema.resize( LesExtrema.length() );
  for ( int i = 0; i < LesExtrema.length(); i++ )
  {
    VExtrema[i] = LesExtrema[i];
  }
  myHomardCas->SetBoundingBox( VExtrema );
}
//=============================================================================
SMESHHOMARD::extrema* HOMARD_Cas_i::GetBoundingBox()
{
  ASSERT(myHomardCas );
  SMESHHOMARD::extrema_var aResult = new SMESHHOMARD::extrema();
  std::vector<double> LesExtremes = myHomardCas->GetBoundingBox();
  ASSERT( LesExtremes.size() == 10 );
  aResult->length( 10 );
  for ( int i = 0; i < LesExtremes.size(); i++ )
  {
    aResult[i] = LesExtremes[i];
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Cas_i::AddGroup( const char* Group)
{
  ASSERT( myHomardCas );
  myHomardCas->AddGroup( Group );
}
//=============================================================================
void HOMARD_Cas_i::SetGroups( const SMESHHOMARD::ListGroupType& ListGroup )
{
  ASSERT( myHomardCas );
  std::list<std::string> ListString;
  for ( int i = 0; i < ListGroup.length(); i++ )
  {
    ListString.push_back(std::string(ListGroup[i]));
  }
  myHomardCas->SetGroups( ListString );
}
//=============================================================================
SMESHHOMARD::ListGroupType* HOMARD_Cas_i::GetGroups()
{
  ASSERT(myHomardCas );
  const std::list<std::string>& ListString = myHomardCas->GetGroups();
  SMESHHOMARD::ListGroupType_var aResult = new SMESHHOMARD::ListGroupType();
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Cas_i::AddBoundary(const char* BoundaryName)
{
  MESSAGE ("HOMARD_Cas_i::AddBoundary : BoundaryName = "<< BoundaryName );
  const char * Group = "";
  AddBoundaryGroup( BoundaryName, Group);
}
//=============================================================================
void HOMARD_Cas_i::AddBoundaryGroup( const char* BoundaryName, const char* Group)
{
  MESSAGE ("HOMARD_Cas_i::AddBoundaryGroup : BoundaryName = "<< BoundaryName << ", Group = " << Group );
  ASSERT( myHomardCas );
  // A. Préalables
  // A.1. Caractéristiques de la frontière à ajouter
  SMESHHOMARD::HOMARD_Boundary_ptr myBoundary = _gen_i->GetBoundary(BoundaryName);
  ASSERT(!CORBA::is_nil(myBoundary));
  int BoundaryType = myBoundary->GetType();
  MESSAGE ( ". BoundaryType = " << BoundaryType );
  // A.2. La liste des frontiere+groupes
  const std::list<std::string>& ListBoundaryGroup = myHomardCas->GetBoundaryGroup();
  std::list<std::string>::const_iterator it;
  // B. Controles
  const char * boun;
  int erreur = 0;
  while ( erreur == 0 )
  {
  // B.1. Si on ajoute une frontière CAO, elle doit être la seule frontière
    if ( BoundaryType == -1 )
    {
      for ( it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++ )
      {
        boun = (*it).c_str();
        MESSAGE ("..  Frontiere enregistrée : "<< boun );
        if ( *it != BoundaryName )
        { erreur = 1;
          break; }
        // On saute le nom du groupe
        it++;
      }
    }
    if ( erreur != 0 ) { break; }
  // B.2. Si on ajoute une frontière non CAO, il ne doit pas y avoir de frontière CAO
    if ( BoundaryType != -1 )
    {
      for ( it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++ )
      {
        boun = (*it).c_str();
        MESSAGE ("..  Frontiere enregistrée : "<< boun );
        SMESHHOMARD::HOMARD_Boundary_ptr myBoundary_0 = _gen_i->GetBoundary(boun);
        int BoundaryType_0 = myBoundary_0->GetType();
        MESSAGE ( ".. BoundaryType_0 = " << BoundaryType_0 );
        if ( BoundaryType_0 == -1 )
        { erreur = 2;
          break; }
        // On saute le nom du groupe
        it++;
      }
      if ( erreur != 0 ) { break; }
    }
  // B.3. Si on ajoute une frontière discrète, il ne doit pas y avoir d'autre frontière discrète
    if ( BoundaryType == 0 )
    {
      for ( it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++ )
      {
        boun = (*it).c_str();
        MESSAGE ("..  Frontiere enregistrée : "<< boun );
        if ( boun != BoundaryName )
        {
          SMESHHOMARD::HOMARD_Boundary_ptr myBoundary_0 = _gen_i->GetBoundary(boun);
          int BoundaryType_0 = myBoundary_0->GetType();
          MESSAGE ( ".. BoundaryType_0 = " << BoundaryType_0 );
          if ( BoundaryType_0 == 0 )
          { erreur = 3;
            break; }
        }
        // On saute le nom du groupe
        it++;
      }
      if ( erreur != 0 ) { break; }
    }
  // B.4. Pour une nouvelle frontiere, publication dans l'arbre d'etudes sous le cas
    bool existe = false;
    for ( it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++ )
    {
      MESSAGE ("..  Frontiere : "<< *it );
      if ( *it == BoundaryName ) { existe = true; }
      // On saute le nom du groupe
      it++;
    }
  // B.5. Le groupe est-il deja enregistre pour une frontiere de ce cas ?
    for ( it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++ )
    {
      boun = (*it).c_str();
      it++;
      MESSAGE ("..  Groupe enregistré : "<< *it );
      if ( *it == Group )
      { erreur = 5;
        break; }
    }
    if ( erreur != 0 ) { break; }
    //
    break;
  }
  // F. Si aucune erreur, enregistrement du couple (frontiere,groupe) dans la reference du cas
  //    Sinon, arrêt
  if ( erreur == 0 )
  { myHomardCas->AddBoundaryGroup( BoundaryName, Group ); }
  else
  {
    std::stringstream ss;
    ss << erreur;
    std::string str = ss.str();
    std::string texte;
    texte = "Erreur numéro " + str + " pour la frontière à enregistrer : " + std::string(BoundaryName);
    if ( erreur == 1 ) { texte += "\nIl existe déjà la frontière "; }
    else if ( erreur == 2 ) { texte += "\nIl existe déjà la frontière CAO "; }
    else if ( erreur == 3 ) { texte += "\nIl existe déjà une frontière discrète : "; }
    else if ( erreur == 5 ) { texte += "\nLe groupe " + std::string(Group) + " est déjà enregistré pour la frontière "; }
    texte += std::string(boun);
    //
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
#ifdef _DEBUG_
    texte += "\nInvalid AddBoundaryGroup";
#endif
    INFOS(texte);
    es.text = CORBA::string_dup(texte.c_str());
    throw SALOME::SALOME_Exception(es);
  }
}
//=============================================================================
SMESHHOMARD::ListBoundaryGroupType* HOMARD_Cas_i::GetBoundaryGroup()
{
  MESSAGE ("GetBoundaryGroup");
  ASSERT(myHomardCas );
  const std::list<std::string>& ListBoundaryGroup = myHomardCas->GetBoundaryGroup();
  SMESHHOMARD::ListBoundaryGroupType_var aResult = new SMESHHOMARD::ListBoundaryGroupType();
  aResult->length( ListBoundaryGroup.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Cas_i::SupprBoundaryGroup()
{
  MESSAGE ("SupprBoundaryGroup");
  ASSERT(myHomardCas );
  myHomardCas->SupprBoundaryGroup();
}
//=============================================================================
void HOMARD_Cas_i::SetPyram( CORBA::Long Pyram )
{
  MESSAGE ("SetPyram, Pyram = " << Pyram );
  ASSERT( myHomardCas );
  myHomardCas->SetPyram( Pyram );
}
//=============================================================================
CORBA::Long HOMARD_Cas_i::GetPyram()
{
  MESSAGE ("GetPyram");
  ASSERT( myHomardCas );
  return myHomardCas->GetPyram();
}
//=============================================================================
void HOMARD_Cas_i::MeshInfo(CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte)
{
  MESSAGE ( "MeshInfo : information sur le maillage initial du cas" );
  ASSERT( myHomardCas );
//
// Nom de l'iteration
  char* IterName = GetIter0Name();
  CORBA::Long etatMenage = -1;
  CORBA::Long modeHOMARD = 7;
  CORBA::Long Option1 = 1;
  CORBA::Long Option2 = 1;
  if ( Qual != 0 ) { modeHOMARD = modeHOMARD*5; }
  if ( Diam != 0 ) { modeHOMARD = modeHOMARD*19; }
  if ( Conn != 0 ) { modeHOMARD = modeHOMARD*11; }
  if ( Tail != 0 ) { modeHOMARD = modeHOMARD*13; }
  if ( Inte != 0 ) { modeHOMARD = modeHOMARD*3; }
  CORBA::Long codret = _gen_i->Compute(IterName, etatMenage, modeHOMARD, Option1, Option2);
  MESSAGE ( "MeshInfo : codret = " << codret );
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
char* HOMARD_Cas_i::GetIter0Name()
{
  ASSERT( myHomardCas );
  return CORBA::string_dup( myHomardCas->GetIter0Name().c_str() );
}
//=============================================================================
SMESHHOMARD::HOMARD_Iteration_ptr HOMARD_Cas_i::GetIter0()
{
  // Nom de l'iteration initiale
  char* Iter0Name = GetIter0Name();
  MESSAGE ( "GetIter0 : Iter0Name      = " << Iter0Name );
  return _gen_i->GetIteration(Iter0Name);
}
//=============================================================================
SMESHHOMARD::HOMARD_Iteration_ptr HOMARD_Cas_i::NextIteration( const char* IterName )
{
  MESSAGE ( "NextIteration : IterName      = " << IterName );
  // Nom de l'iteration parent
  char* NomIterParent = GetIter0Name();
  MESSAGE ( "NextIteration : NomIterParent = " << NomIterParent );
  return _gen_i->CreateIteration(IterName, NomIterParent);
}
//=============================================================================
SMESHHOMARD::HOMARD_Iteration_ptr HOMARD_Cas_i::LastIteration( )
{
  SMESHHOMARD::HOMARD_Iteration_ptr Iter;
  SMESHHOMARD::listeIterFilles_var ListeIterFilles;
  char* IterName;
// Iteration initiale du cas
  IterName = GetIter0Name();
// On va explorer la descendance de cette iteration initiale
// jusqu'a trouver celle qui n'a pas de filles
  int nbiterfilles = 1;
  while ( nbiterfilles == 1 )
  {
// L'iteration associee
//     MESSAGE ( ".. IterName = " << IterName );
    Iter = _gen_i->GetIteration(IterName);
// Les filles de cette iteration
    ListeIterFilles = Iter->GetIterations();
    nbiterfilles = ListeIterFilles->length();
//     MESSAGE ( ".. nbiterfilles = " << nbiterfilles );
// S'il y a au moins 2 filles, arret : on ne sait pas faire
    VERIFICATION( nbiterfilles <= 1 );
// S'il y a une fille unique, on recupere le nom de la fille et on recommence
    if ( nbiterfilles == 1 )
    { IterName = ListeIterFilles[0]; }
  }
//
  return Iter;
}
//=============================================================================
void HOMARD_Cas_i::AddIteration( const char* NomIteration )
{
  ASSERT( myHomardCas );
  myHomardCas->AddIteration( NomIteration );
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Hypothesis_i::HOMARD_Hypothesis_i()
{
  MESSAGE( "Default constructor, not for use" );
  ASSERT( 0 );
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Hypothesis_i::HOMARD_Hypothesis_i( CORBA::ORB_ptr orb,
                                          SMESHHOMARD::HOMARD_Gen_var engine )
{
  MESSAGE( "standard constructor" );
  _gen_i = engine;
  _orb = orb;
  myHomardHypothesis = new SMESHHOMARDImpl::HOMARD_Hypothesis();
  ASSERT( myHomardHypothesis );
}

//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Hypothesis_i::~HOMARD_Hypothesis_i()
{
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Hypothesis_i::SetName( const char* Name )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetName( Name );
}
//=============================================================================
char* HOMARD_Hypothesis_i::GetName()
{
  ASSERT( myHomardHypothesis );
  return CORBA::string_dup( myHomardHypothesis->GetName().c_str() );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::Delete()
{
  ASSERT( myHomardHypothesis );
  char* HypoName = GetName();
  MESSAGE ( "Delete : destruction de l'hypothese " << HypoName );
  return _gen_i->DeleteHypo(HypoName);
}
//=============================================================================
char* HOMARD_Hypothesis_i::GetDumpPython()
{
  ASSERT( myHomardHypothesis );
  return CORBA::string_dup( myHomardHypothesis->GetDumpPython().c_str() );
}
//=============================================================================
std::string HOMARD_Hypothesis_i::Dump() const
{
  return SMESHHOMARDImpl::Dump( *myHomardHypothesis );
}
//=============================================================================
bool HOMARD_Hypothesis_i::Restore( const std::string& stream )
{
  return SMESHHOMARDImpl::Restore( *myHomardHypothesis, stream );
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Hypothesis_i::SetUnifRefinUnRef( CORBA::Long TypeRaffDera )
{
  ASSERT( myHomardHypothesis );
  VERIFICATION( (TypeRaffDera==1) || (TypeRaffDera==-1) );
  int TypeRaff, TypeDera;
  if ( TypeRaffDera == 1 )
  {
    TypeRaff = 1;
    TypeDera = 0;
  }
  else if ( TypeRaffDera == -1 )
  {
    TypeRaff = 0;
    TypeDera = 1;
  }
  myHomardHypothesis->SetAdapType( -1 );
  myHomardHypothesis->SetRefinTypeDera( TypeRaff, TypeDera );
}
//=============================================================================
SMESHHOMARD::listeTypes* HOMARD_Hypothesis_i::GetAdapRefinUnRef()
{
  ASSERT( myHomardHypothesis );
  SMESHHOMARD::listeTypes_var aResult = new SMESHHOMARD::listeTypes;
  aResult->length( 3 );
  aResult[0] = CORBA::Long( myHomardHypothesis->GetAdapType() );
  aResult[1] = CORBA::Long( myHomardHypothesis->GetRefinType() );
  aResult[2] = CORBA::Long( myHomardHypothesis->GetUnRefType() );
  return aResult._retn();
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetAdapType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetAdapType() );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetRefinType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetRefinType() );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetUnRefType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetUnRefType() );
}
//=============================================================================
void HOMARD_Hypothesis_i::SetField( const char* FieldName )
{
  myHomardHypothesis->SetField( FieldName );
}
//=============================================================================
char* HOMARD_Hypothesis_i::GetFieldName()
{
  ASSERT( myHomardHypothesis );
  return CORBA::string_dup( myHomardHypothesis->GetFieldName().c_str() );
}
//=============================================================================
void HOMARD_Hypothesis_i::SetUseField( CORBA::Long UsField )
{
  myHomardHypothesis->SetUseField( UsField );
}
//=============================================================================
SMESHHOMARD::InfosHypo* HOMARD_Hypothesis_i::GetField()
{
  ASSERT( myHomardHypothesis );
  SMESHHOMARD::InfosHypo* aInfosHypo = new SMESHHOMARD::InfosHypo();
  aInfosHypo->FieldName  = CORBA::string_dup( myHomardHypothesis->GetFieldName().c_str() );
  aInfosHypo->TypeThR    = CORBA::Long( myHomardHypothesis->GetRefinThrType() );
  aInfosHypo->ThreshR    = CORBA::Double( myHomardHypothesis->GetThreshR() );
  aInfosHypo->TypeThC    = CORBA::Long( myHomardHypothesis->GetUnRefThrType() );
  aInfosHypo->ThreshC    = CORBA::Double( myHomardHypothesis->GetThreshC() );
  aInfosHypo->UsField    = CORBA::Long( myHomardHypothesis->GetUseField() );
  aInfosHypo->UsCmpI     = CORBA::Long( myHomardHypothesis->GetUseComp() );
  return aInfosHypo;
}
//=============================================================================
void HOMARD_Hypothesis_i::SetUseComp( CORBA::Long UsCmpI )
{
  myHomardHypothesis->SetUseComp( UsCmpI );
}
//=============================================================================
void HOMARD_Hypothesis_i::AddComp( const char* NomComp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->AddComp( NomComp );
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprComp( const char* NomComp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprComp(NomComp);
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprComps()
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprComps();
}
//=============================================================================
SMESHHOMARD::listeComposantsHypo* HOMARD_Hypothesis_i::GetComps()
{
  ASSERT( myHomardHypothesis );
  const std::list<std::string>& ListString = myHomardHypothesis->GetComps();
  SMESHHOMARD::listeComposantsHypo_var aResult = new SMESHHOMARD::listeComposantsHypo;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetRefinThr( CORBA::Long TypeThR, CORBA::Double ThreshR )
{
  myHomardHypothesis->SetAdapType( 1 );
  if ( TypeThR > 0 )
  {
    int TypeDera = myHomardHypothesis->GetUnRefType();
    myHomardHypothesis->SetRefinTypeDera( 1, TypeDera );
  }
  myHomardHypothesis->SetRefinThr( TypeThR, ThreshR );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetRefinThrType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetRefinThrType() );
}
//=============================================================================
void HOMARD_Hypothesis_i::SetUnRefThr( CORBA::Long TypeThC, CORBA::Double ThreshC )
{
  myHomardHypothesis->SetAdapType( 1 );
  if ( TypeThC > 0 )
  {
    int TypeRaff = myHomardHypothesis->GetRefinType();
    myHomardHypothesis->SetRefinTypeDera( TypeRaff, 1 );
  }
  myHomardHypothesis->SetUnRefThr( TypeThC, ThreshC );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetUnRefThrType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetUnRefThrType() );
}
//=============================================================================
void HOMARD_Hypothesis_i::SetNivMax( CORBA::Long NivMax )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetNivMax( NivMax );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetNivMax()
{
  ASSERT( myHomardHypothesis );
  return myHomardHypothesis->GetNivMax();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetDiamMin( CORBA::Double DiamMin )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetDiamMin( DiamMin );
}
//=============================================================================
CORBA::Double HOMARD_Hypothesis_i::GetDiamMin()
{
  ASSERT( myHomardHypothesis );
  return myHomardHypothesis->GetDiamMin();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetAdapInit( CORBA::Long AdapInit )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetAdapInit( AdapInit );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetAdapInit()
{
  ASSERT( myHomardHypothesis );
  return myHomardHypothesis->GetAdapInit();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetExtraOutput( CORBA::Long ExtraOutput )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetExtraOutput( ExtraOutput );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetExtraOutput()
{
  ASSERT( myHomardHypothesis );
  return myHomardHypothesis->GetExtraOutput();
}
//=============================================================================
void HOMARD_Hypothesis_i::AddGroup( const char* Group)
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->AddGroup( Group );
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprGroup( const char* Group )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprGroup(Group);
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprGroups()
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprGroups();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetGroups(const SMESHHOMARD::ListGroupType& ListGroup)
{
  ASSERT( myHomardHypothesis );
  std::list<std::string> ListString;
  for ( int i = 0; i < ListGroup.length(); i++ )
  {
    ListString.push_back(std::string(ListGroup[i]));
  }
  myHomardHypothesis->SetGroups( ListString );
}
//=============================================================================
SMESHHOMARD::ListGroupType*  HOMARD_Hypothesis_i::GetGroups()
{
  ASSERT( myHomardHypothesis );
  const std::list<std::string>& ListString = myHomardHypothesis->GetGroups();
  SMESHHOMARD::ListGroupType_var aResult = new SMESHHOMARD::ListGroupType;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetTypeFieldInterp( CORBA::Long TypeFieldInterp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetTypeFieldInterp( TypeFieldInterp );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetTypeFieldInterp()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetTypeFieldInterp() );
}
//=============================================================================
void HOMARD_Hypothesis_i::AddFieldInterp( const char* FieldInterp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->AddFieldInterpType( FieldInterp, 0 );
}
//=============================================================================
void HOMARD_Hypothesis_i::AddFieldInterpType( const char* FieldInterp, CORBA::Long TypeInterp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->AddFieldInterpType( FieldInterp, TypeInterp );
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprFieldInterp( const char* FieldInterp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprFieldInterp(FieldInterp);
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprFieldInterps()
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprFieldInterps();
}
//=============================================================================
SMESHHOMARD::listeFieldInterpsHypo* HOMARD_Hypothesis_i::GetFieldInterps()
{
  ASSERT( myHomardHypothesis );
  const std::list<std::string>& ListString = myHomardHypothesis->GetFieldInterps();
  SMESHHOMARD::listeFieldInterpsHypo_var aResult = new SMESHHOMARD::listeFieldInterpsHypo;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Hypothesis_i::SetCaseCreation( const char* NomCaseCreation )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetCaseCreation( NomCaseCreation );
}
//=============================================================================
char* HOMARD_Hypothesis_i::GetCaseCreation()
{
  ASSERT( myHomardHypothesis );
  return CORBA::string_dup( myHomardHypothesis->GetCaseCreation().c_str() );
}
//=============================================================================
void HOMARD_Hypothesis_i::LinkIteration( const char* NomIteration )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->LinkIteration( NomIteration );
}
//=============================================================================
void HOMARD_Hypothesis_i::UnLinkIteration( const char* NomIteration )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->UnLinkIteration( NomIteration );
}
//=============================================================================
SMESHHOMARD::listeIters* HOMARD_Hypothesis_i::GetIterations()
{
  ASSERT( myHomardHypothesis );
  const std::list<std::string>& ListString = myHomardHypothesis->GetIterations();
  SMESHHOMARD::listeIters_var aResult = new SMESHHOMARD::listeIters;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Iteration_i::HOMARD_Iteration_i()
{
  MESSAGE( "Default constructor, not for use" );
  ASSERT( 0 );
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Iteration_i::HOMARD_Iteration_i( CORBA::ORB_ptr orb,
                                        SMESHHOMARD::HOMARD_Gen_var engine )
{
  MESSAGE("constructor");
  _gen_i = engine;
  _orb = orb;
  myHomardIteration = new SMESHHOMARDImpl::HOMARD_Iteration();
  ASSERT( myHomardIteration );
}
//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Iteration_i::~HOMARD_Iteration_i()
{
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Iteration_i::SetName( const char* Name )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetName( Name );
}
//=============================================================================
char* HOMARD_Iteration_i::GetName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetName().c_str() );
}

  //=============================================================================
  CORBA::Long  HOMARD_Iteration_i::Delete( CORBA::Long Option, bool doRemoveWorkingFiles )
  {
    ASSERT( myHomardIteration );
    char* IterName = GetName();
    MESSAGE ( "Delete : destruction de l'iteration " << IterName << ", Option = " << Option );
    return _gen_i->DeleteIteration(IterName, Option, doRemoveWorkingFiles);
  }

//=============================================================================
char* HOMARD_Iteration_i::GetDumpPython()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetDumpPython().c_str() );
}
//=============================================================================
std::string HOMARD_Iteration_i::Dump() const
{
  return SMESHHOMARDImpl::Dump( *myHomardIteration );
}
//=============================================================================
bool HOMARD_Iteration_i::Restore( const std::string& stream )
{
  return SMESHHOMARDImpl::Restore( *myHomardIteration, stream );
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Iteration_i::SetDirNameLoc( const char* NomDir )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetDirNameLoc( NomDir );
}
//=============================================================================
char* HOMARD_Iteration_i::GetDirNameLoc()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetDirNameLoc().c_str() );
}
//=============================================================================
char* HOMARD_Iteration_i::GetDirName()
{
  ASSERT( myHomardIteration );
  std::string casename = myHomardIteration->GetCaseName();
  SMESHHOMARD::HOMARD_Cas_ptr caseiter = _gen_i->GetCase(casename.c_str());
  std::string dirnamecase = caseiter->GetDirName();
  std::string dirname = dirnamecase + "/" +  GetDirNameLoc();
  return CORBA::string_dup( dirname.c_str() );
}
//=============================================================================
void HOMARD_Iteration_i::SetNumber( CORBA::Long NumIter )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetNumber( NumIter );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetNumber()
{
  ASSERT( myHomardIteration );
  return myHomardIteration->GetNumber();
}
//=============================================================================
void HOMARD_Iteration_i::SetState( CORBA::Long Etat )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetState( Etat );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetState()
{
  ASSERT( myHomardIteration );
  return myHomardIteration->GetState();
}
//=============================================================================
void HOMARD_Iteration_i::SetMeshName( const char* NomMesh )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetMeshName( NomMesh );
}
//=============================================================================
char* HOMARD_Iteration_i::GetMeshName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetMeshName().c_str() );
}
//=============================================================================
void HOMARD_Iteration_i::SetMeshFile( const char* MeshFile )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetMeshFile( MeshFile );
}
//=============================================================================
char* HOMARD_Iteration_i::GetMeshFile()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetMeshFile().c_str() );
}
//=============================================================================
void HOMARD_Iteration_i::SetFieldFile( const char* FieldFile )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetFieldFile( FieldFile );
}
//=============================================================================
char* HOMARD_Iteration_i::GetFieldFile()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetFieldFile().c_str() );
}
//=============================================================================
// Instants pour le champ de pilotage
//=============================================================================
void HOMARD_Iteration_i::SetTimeStep( CORBA::Long TimeStep )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetTimeStep( TimeStep );
}
//=============================================================================
void HOMARD_Iteration_i::SetTimeStepRank( CORBA::Long TimeStep, CORBA::Long Rank )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetTimeStepRank( TimeStep, Rank );
}
//=============================================================================
void HOMARD_Iteration_i::SetTimeStepRankLast()
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetTimeStepRankLast();
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetTimeStep()
{
  ASSERT( myHomardIteration );
  return  CORBA::Long( myHomardIteration->GetTimeStep() );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetRank()
{
  ASSERT( myHomardIteration );
  return  CORBA::Long( myHomardIteration->GetRank() );
}
//=============================================================================
// Instants pour un champ a interpoler
//=============================================================================
void HOMARD_Iteration_i::SetFieldInterpTimeStep( const char* FieldInterp, CORBA::Long TimeStep )
{
  SetFieldInterpTimeStepRank( FieldInterp, TimeStep, TimeStep );
}
//=============================================================================
void HOMARD_Iteration_i::SetFieldInterpTimeStepRank( const char* FieldInterp, CORBA::Long TimeStep, CORBA::Long Rank )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetFieldInterpTimeStepRank( FieldInterp, TimeStep, Rank );
}
//=============================================================================
SMESHHOMARD::listeFieldInterpTSRsIter* HOMARD_Iteration_i::GetFieldInterpsTimeStepRank()
{
  ASSERT( myHomardIteration );
  const std::list<std::string>& ListString = myHomardIteration->GetFieldInterpsTimeStepRank();
  SMESHHOMARD::listeFieldInterpTSRsIter_var aResult = new SMESHHOMARD::listeFieldInterpTSRsIter;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Iteration_i::SetFieldInterp( const char* FieldInterp )
{
  myHomardIteration->SetFieldInterp( FieldInterp );
}
//=============================================================================
SMESHHOMARD::listeFieldInterpsIter* HOMARD_Iteration_i::GetFieldInterps()
{
  ASSERT( myHomardIteration );
  const std::list<std::string>& ListString = myHomardIteration->GetFieldInterps();
  SMESHHOMARD::listeFieldInterpsIter_var aResult = new SMESHHOMARD::listeFieldInterpsIter;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Iteration_i::SupprFieldInterps()
{
  ASSERT( myHomardIteration );
  myHomardIteration->SupprFieldInterps();
}
//=============================================================================
void HOMARD_Iteration_i::SetLogFile( const char* LogFile )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetLogFile( LogFile );
}
//=============================================================================
char* HOMARD_Iteration_i::GetLogFile()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetLogFile().c_str() );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::Compute(CORBA::Long etatMenage, CORBA::Long Option)
{
  MESSAGE ( "Compute : calcul d'une iteration, etatMenage = "<<etatMenage<<", Option = "<<Option );
  ASSERT( myHomardIteration );
//
// Nom de l'iteration
  char* IterName = GetName();
  CORBA::Long modeHOMARD = 1;
  CORBA::Long Option1 = 1;
  MESSAGE ( "Compute : calcul de l'teration " << IterName );
  return _gen_i->Compute(IterName, etatMenage, modeHOMARD, Option1, Option);
}
//=============================================================================
void HOMARD_Iteration_i::MeshInfo(CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte)
{
  MESSAGE ( "MeshInfo : information sur le maillage associe a une iteration" );
  ASSERT( myHomardIteration );
  //
  int Option = 1;
  MeshInfoOption( Qual, Diam, Conn, Tail, Inte, Option );
}
//=============================================================================
void HOMARD_Iteration_i::MeshInfoOption(CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte, CORBA::Long Option)
{
  MESSAGE ( "MeshInfoOption : information sur le maillage associe a une iteration" );
  ASSERT( myHomardIteration );
//
// Nom de l'iteration
  char* IterName = GetName();
  CORBA::Long etatMenage = -1;
  CORBA::Long modeHOMARD = 7;
  if ( Qual != 0 ) { modeHOMARD = modeHOMARD*5; }
  if ( Diam != 0 ) { modeHOMARD = modeHOMARD*19; }
  if ( Conn != 0 ) { modeHOMARD = modeHOMARD*11; }
  if ( Tail != 0 ) { modeHOMARD = modeHOMARD*13; }
  if ( Inte != 0 ) { modeHOMARD = modeHOMARD*3; }
  MESSAGE ( "MeshInfoOption : information sur le maillage de l'iteration " << IterName );
  CORBA::Long Option2 = 1;
  CORBA::Long codret = _gen_i->Compute(IterName, etatMenage, modeHOMARD, Option, Option2);
  MESSAGE ( "MeshInfoOption : codret = " << codret );
}
//=============================================================================
void HOMARD_Iteration_i::SetFileInfo( const char* FileInfo )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetFileInfo( FileInfo );
}
//=============================================================================
char* HOMARD_Iteration_i::GetFileInfo()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetFileInfo().c_str() );
}
//=============================================================================
//=============================================================================
// Liens avec les autres iterations
//=============================================================================
//=============================================================================
SMESHHOMARD::HOMARD_Iteration_ptr HOMARD_Iteration_i::NextIteration( const char* IterName )
{
// Nom de l'iteration parent
  char* NomIterParent = GetName();
  MESSAGE ( "NextIteration : creation de l'iteration " << IterName << " comme fille de " << NomIterParent );
  return _gen_i->CreateIteration(IterName, NomIterParent);
}
//=============================================================================
void HOMARD_Iteration_i::LinkNextIteration( const char* NomIteration )
{
  ASSERT( myHomardIteration );
  myHomardIteration->LinkNextIteration( NomIteration );
}
//=============================================================================
void HOMARD_Iteration_i::UnLinkNextIteration( const char* NomIteration )
{
  ASSERT( myHomardIteration );
  myHomardIteration->UnLinkNextIteration( NomIteration );
}
//=============================================================================
SMESHHOMARD::listeIterFilles* HOMARD_Iteration_i::GetIterations()
{
  ASSERT( myHomardIteration );
  const std::list<std::string>& maListe = myHomardIteration->GetIterations();
  SMESHHOMARD::listeIterFilles_var aResult = new SMESHHOMARD::listeIterFilles;
  aResult->length( maListe.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = maListe.begin(); it != maListe.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Iteration_i::SetIterParentName( const char* NomIterParent )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetIterParentName( NomIterParent );
}
//=============================================================================
char* HOMARD_Iteration_i::GetIterParentName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetIterParentName().c_str() );
}
//=============================================================================
SMESHHOMARD::HOMARD_Iteration_ptr HOMARD_Iteration_i::GetIterParent()
{
// Nom de l'iteration parent
  char* NomIterParent = GetIterParentName();
  MESSAGE ( "GetIterParent : NomIterParent = " << NomIterParent );
  return _gen_i->GetIteration(NomIterParent);
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Iteration_i::SetCaseName( const char* NomCas )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetCaseName( NomCas );
}
//=============================================================================
char* HOMARD_Iteration_i::GetCaseName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetCaseName().c_str() );
}
//=============================================================================
void HOMARD_Iteration_i::AssociateHypo( const char* NomHypo )
{
  ASSERT( myHomardIteration );
//
// Nom de l'iteration
  char* IterName = GetName();
  MESSAGE ( ". IterName = " << IterName );
  return _gen_i->AssociateIterHypo(IterName, NomHypo);
}
//=============================================================================
void HOMARD_Iteration_i::SetHypoName( const char* NomHypo )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetHypoName( NomHypo );
}
//=============================================================================
char* HOMARD_Iteration_i::GetHypoName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetHypoName().c_str() );
}
//=============================================================================
//=============================================================================
// Divers
//=============================================================================
//=============================================================================
void HOMARD_Iteration_i::SetInfoCompute( CORBA::Long MessInfo )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetInfoCompute( MessInfo );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetInfoCompute()
{
  ASSERT( myHomardIteration );
  return myHomardIteration->GetInfoCompute();
}

//=============================================================================
//functions
//=============================================================================
std::string RemoveTabulation( std::string theScript )
{
  std::string::size_type aPos = 0;
  while( aPos < theScript.length() )
  {
    aPos = theScript.find( "\n\t", aPos );
    if( aPos == std::string::npos )
      break;
    theScript.replace( aPos, 2, "\n" );
    aPos++;
  }
  return theScript;
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Gen_i::HOMARD_Gen_i() :
SALOME::GenericObj_i( SMESH_Gen_i::GetPOA() )
{
  MESSAGE("constructor de HOMARD_Gen_i");

  myHomard = new SMESHHOMARDImpl::HOMARD_Gen;

  SetPreferences();
}
//=================================
/*!
 *  standard destructor
 */
//================================
HOMARD_Gen_i::~HOMARD_Gen_i()
{
}

//=============================================================================
//=============================================================================
// Utilitaires pour l'iteration
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::SetEtatIter(const char* nomIter, const CORBA::Long Etat)
//=====================================================================================
{
  MESSAGE( "SetEtatIter : affectation de l'etat " << Etat << " a l'iteration " << nomIter );
  SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  if (CORBA::is_nil(myIteration)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid iteration";
    throw SALOME::SALOME_Exception(es);
  }

  myIteration->SetState(Etat);
}
//=============================================================================
//=============================================================================
//
//=============================================================================
//=============================================================================
// Destruction des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteBoundary(const char* BoundaryName)
{
  MESSAGE ( "DeleteBoundary : BoundaryName = " << BoundaryName );
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
  if (CORBA::is_nil(myBoundary))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid boundary";
    throw SALOME::SALOME_Exception(es);
    return 1;
  };

// On verifie que la frontiere n'est plus utilisee
  SMESHHOMARD::listeCases* maListe = GetAllCasesName();
  int numberOfCases = maListe->length();
  MESSAGE ( ".. Nombre de cas = " << numberOfCases );
  std::string CaseName;
  SMESHHOMARD::ListBoundaryGroupType* ListBoundaryGroupType;
  int numberOfitems;
  SMESHHOMARD::HOMARD_Cas_var myCase;
  for (int NumeCas = 0; NumeCas< numberOfCases; NumeCas++)
  {
    CaseName = std::string((*maListe)[NumeCas]);
    MESSAGE ( "... Examen du cas = " << CaseName.c_str() );
    myCase = myStudyContext._mesCas[CaseName];
    ASSERT(!CORBA::is_nil(myCase));
    ListBoundaryGroupType = myCase->GetBoundaryGroup();
    numberOfitems = ListBoundaryGroupType->length();
    MESSAGE ( "... number of string for Boundary+Group = " << numberOfitems);
    for (int NumBoundary = 0; NumBoundary< numberOfitems; NumBoundary=NumBoundary+2)
    {
      if ( std::string((*ListBoundaryGroupType)[NumBoundary]) == BoundaryName )
      {
        SALOME::ExceptionStruct es;
        es.type = SALOME::BAD_PARAM;
        es.text = "This boundary is used in a case and cannot be deleted.";
        throw SALOME::SALOME_Exception(es);
        return 2;
      };
    };
  }

  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesBoundarys.erase(BoundaryName);

  return 0;
}
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteCase(const char* nomCas, CORBA::Long Option)
{
  // Pour detruire un cas
  MESSAGE ( "DeleteCase : nomCas = " << nomCas << ", avec option = " << Option );
  SMESHHOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  if (CORBA::is_nil(myCase)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid case context";
    throw SALOME::SALOME_Exception(es);
    return 1;
  }
  // On commence par detruire toutes les iterations en partant de l'initiale et y compris elle
  CORBA::String_var nomIter = myCase->GetIter0Name();
  CORBA::Long Option1 = 0;
  if ( DeleteIterationOption(nomIter, Option1, Option, true) != 0 )
  {
    return 2;
  };

  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesCas.erase(nomCas);

  return 0;
}
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteHypo(const char* nomHypo)
{
  MESSAGE ( "DeleteHypo : nomHypo = " << nomHypo );
  SMESHHOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
  if (CORBA::is_nil(myHypo))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid hypothesis";
    throw SALOME::SALOME_Exception(es);
    return 1;
  };

// On verifie que l'hypothese n'est plus utilisee
  SMESHHOMARD::listeIters* maListeIter = myHypo->GetIterations();
  int numberOfIter = maListeIter->length();
  if ( numberOfIter > 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This hypothesis is used in an iteration and cannot be deleted.";
    throw SALOME::SALOME_Exception(es);
    return 2;
  };

  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesHypotheses.erase(nomHypo);

  return 0;
}

  //=============================================================================
  CORBA::Long HOMARD_Gen_i::DeleteIteration(const char* nomIter, CORBA::Long Option,
                                            bool doRemoveWorkingFiles)
  {
    //  Option = 0 : On ne supprime pas le fichier du maillage associe
    //  Option = 1 : On supprime le fichier du maillage associe
    MESSAGE ( "DeleteIteration : nomIter = " << nomIter << ", avec option = " << Option );
    CORBA::Long Option1 = 1;
    return DeleteIterationOption(nomIter, Option1, Option, doRemoveWorkingFiles);
  }

//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteIterationOption(const char* nomIter,
                                                CORBA::Long Option1,
                                                CORBA::Long Option2,
                                                bool doRemoveWorkingFiles)
{
  //  Option1 = 0 : On autorise la destruction de l'iteration 0
  //  Option1 = 1 : On interdit la destruction de l'iteration 0

  //  Option2 = 0 : On ne supprime pas le fichier du maillage associe
  //  Option2 = 1 : On supprime le fichier du maillage associe
  MESSAGE ( "DeleteIterationOption : nomIter = " << nomIter << ", avec options = " << Option1<< ", " << Option2 );
  SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  if (CORBA::is_nil(myIteration)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid iteration";
    throw SALOME::SALOME_Exception(es);
  }

  int numero = myIteration->GetNumber();
  MESSAGE ( "DeleteIterationOption : numero = " << numero );
  if ( numero == 0 && Option1 == 1 ) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This iteration cannot be deleted.";
    throw SALOME::SALOME_Exception(es);
  }

  // On detruit recursivement toutes les filles
  SMESHHOMARD::listeIterFilles* maListe = myIteration->GetIterations();
  int numberOfIter = maListe->length();
  for (int NumeIter = 0; NumeIter < numberOfIter; NumeIter++) {
    std::string nomIterFille = std::string((*maListe)[NumeIter]);
    MESSAGE ( ".. appel recursif de DeleteIterationOption pour nomIter = " << nomIterFille.c_str() );
    DeleteIterationOption(nomIterFille.c_str(), Option1, Option2, doRemoveWorkingFiles);
  }

  // On arrive ici pour une iteration sans fille
  MESSAGE ( "Destruction effective de " << nomIter );
  // On commence par invalider l'iteration pour faire le menage des dependances
  // et eventuellement du maillage associe
  int option;
  if ( numero == 0 ) { option = 0; }
  else               { option = Option2; }
  InvalideIterOption(nomIter, option, doRemoveWorkingFiles);

  // Retrait dans la descendance de l'iteration parent
  if ( numero > 0 )
  {
    std::string nomIterationParent = myIteration->GetIterParentName();
    MESSAGE ( "Retrait dans la descendance de nomIterationParent " << nomIterationParent );
    SMESHHOMARD::HOMARD_Iteration_var myIterationParent = myStudyContext._mesIterations[nomIterationParent];
    if (CORBA::is_nil(myIterationParent))
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iteration";
      throw SALOME::SALOME_Exception(es);
      return 3;
    };
    myIterationParent->UnLinkNextIteration(nomIter);
  }

  // suppression du lien avec l'hypothese
  if ( numero > 0 )
  {
    std::string nomHypo = myIteration->GetHypoName();
    SMESHHOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
    ASSERT(!CORBA::is_nil(myHypo));
    myHypo->UnLinkIteration(nomIter);
  }

  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesIterations.erase(nomIter);

  // on peut aussi faire RemoveObject
//   MESSAGE ( "Au final" );
//   SMESHHOMARD::listeIterations* Liste = GetAllIterationsName();
//   numberOfIter = Liste->length();
//   for (int NumeIter = 0; NumeIter< numberOfIter; NumeIter++)
//   {
//       std::string nomIterFille = std::string((*Liste)[NumeIter]);
//       MESSAGE ( ".. nomIter = " << nomIterFille.c_str() );
//   }

  return 0;
}
//=============================================================================
//=============================================================================
// Invalidation des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::InvalideBoundary(const char* BoundaryName)
{
  MESSAGE( "InvalideBoundary : BoundaryName = " << BoundaryName  );
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
  if (CORBA::is_nil(myBoundary)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid boundary";
    throw SALOME::SALOME_Exception(es);
  }
  else {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "No change is allowed in a boundary. Ask for evolution.";
    throw SALOME::SALOME_Exception(es);
  }
}
//=============================================================================
void HOMARD_Gen_i::InvalideHypo(const char* nomHypo)
{
  MESSAGE( "InvalideHypo : nomHypo    = " << nomHypo  );
  SMESHHOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
  if (CORBA::is_nil(myHypo)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid hypothesis";
    throw SALOME::SALOME_Exception(es);
  }

  SMESHHOMARD::listeIters* maListe = myHypo->GetIterations();
  int numberOfIter = maListe->length();
  for (int NumeIter = 0; NumeIter< numberOfIter; NumeIter++) {
    std::string nomIter = std::string((*maListe)[NumeIter]);
    MESSAGE( ".. nomIter = " << nomIter );
    InvalideIter(nomIter.c_str());
  }
}
//=============================================================================
void HOMARD_Gen_i::InvalideIter(const char* nomIter)
{
  MESSAGE("InvalideIter : nomIter = " << nomIter);
  // Pour invalider totalement une iteration courante
  CORBA::Long Option = 1;
  return InvalideIterOption(nomIter, Option, true);
}

  //=============================================================================
  void HOMARD_Gen_i::InvalideIterOption(const char* nomIter, CORBA::Long Option,
                                        bool doRemoveWorkingFiles)
  {
    //  Option = 0 : On ne supprime pas le fichier du maillage associe
    //  Option = 1 : On supprime le fichier du maillage associe
    MESSAGE ( "InvalideIterOption : nomIter = " << nomIter << ", avec option = " << Option );
    SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
    if (CORBA::is_nil(myIteration)) {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iteration";
      throw SALOME::SALOME_Exception(es);
    }

    SMESHHOMARD::listeIterFilles* maListe = myIteration->GetIterations();
    int numberOfIter = maListe->length();
    for (int NumeIter = 0; NumeIter< numberOfIter; NumeIter++) {
      std::string nomIterFille = std::string((*maListe)[NumeIter]);
      MESSAGE ( ".. appel recursif de InvalideIter pour nomIter = " << nomIterFille.c_str() );
      InvalideIter(nomIterFille.c_str());
    }

    // On arrive ici pour une iteration sans fille
    MESSAGE ( "Invalidation effective de " << nomIter );

    int etat = myIteration->GetState();
    if ( etat > 0 ) {
      SetEtatIter(nomIter,1);
      //const char * nomCas = myIteration->GetCaseName();
      //SMESHHOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
      //if (CORBA::is_nil(myCase)) {
      //  SALOME::ExceptionStruct es;
      //  es.type = SALOME::BAD_PARAM;
      //  es.text = "Invalid case context";
      //  throw SALOME::SALOME_Exception(es);
      //}
      if (doRemoveWorkingFiles) {
        std::string nomDir     = myIteration->GetDirName();
        std::string nomFichier = myIteration->GetMeshFile();
        std::string commande = "rm -rf " + std::string(nomDir);
        if ( Option == 1 ) { commande = commande + ";rm -rf " + std::string(nomFichier); }
        MESSAGE ( "commande = " << commande );
        if ((system(commande.c_str())) != 0) {
          SALOME::ExceptionStruct es;
          es.type = SALOME::BAD_PARAM;
          es.text = "The directory for the calculation cannot be cleared.";
          throw SALOME::SALOME_Exception(es);
        }
      }
      // Suppression du maillage publie dans SMESH
      //std::string MeshName = myIteration->GetMeshName();
      //DeleteResultInSmesh(nomFichier, MeshName);
    }
  }

//=============================================================================
void HOMARD_Gen_i::InvalideIterInfo(const char* nomIter)
{
  MESSAGE("InvalideIterInfo : nomIter = " << nomIter);
  SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  if (CORBA::is_nil(myIteration)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid iteration";
    throw SALOME::SALOME_Exception(es);
  }

  const char * nomCas = myIteration->GetCaseName();
  SMESHHOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  if (CORBA::is_nil(myCase)) {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid case context";
      throw SALOME::SALOME_Exception(es);
  };
  const char* nomDir   = myIteration->GetDirName();
  std::string commande = "rm -f " + std::string(nomDir) + "/info* ";
  commande += std::string(nomDir) + "/Liste.*info";
/*  MESSAGE ( "commande = " << commande );*/
  if ((system(commande.c_str())) != 0)
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory for the calculation cannot be cleared.";
    throw SALOME::SALOME_Exception(es);
  }
}
//=============================================================================
//=============================================================================
// Association de lien entre des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::AssociateCaseIter(const char* nomCas, const char* nomIter, const char* labelIter)
{
  MESSAGE( "AssociateCaseIter : " << nomCas << ", " << nomIter << ", "  << labelIter );

  SMESHHOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  if (CORBA::is_nil(myCase)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid case";
    throw SALOME::SALOME_Exception(es);
  }

  SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  if (CORBA::is_nil(myIteration)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid iteration";
    throw SALOME::SALOME_Exception(es);
  }

  myCase->AddIteration(nomIter);
  myIteration->SetCaseName(nomCas);
}
//=============================================================================
void HOMARD_Gen_i::AssociateIterHypo(const char* nomIter, const char* nomHypo)
{
  MESSAGE("AssociateIterHypo : nomHypo = " << nomHypo << " nomIter = " << nomIter);

  // Verification de l'existence de l'hypothese
  SMESHHOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
  ASSERT(!CORBA::is_nil(myHypo));

  // Verification de l'existence de l'iteration
  SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  ASSERT(!CORBA::is_nil(myIteration));

  // Liens reciproques
  myIteration->SetHypoName(nomHypo);
  myHypo->LinkIteration(nomIter);

  // On stocke les noms des champ a interpoler pour
  // le futur controle de la donnée des pas de temps
  myIteration->SupprFieldInterps();
  SMESHHOMARD::listeFieldInterpsHypo* ListField = myHypo->GetFieldInterps();
  int numberOfFieldsx2 = ListField->length();
  for (int iaux = 0; iaux < numberOfFieldsx2; iaux++) {
    std::string FieldName = std::string((*ListField)[iaux]);
    myIteration->SetFieldInterp(FieldName.c_str());
    iaux++;
  }
}

//=============================================================================
//=============================================================================
// Recuperation des listes
//=============================================================================
//=============================================================================
SMESHHOMARD::listeBoundarys* HOMARD_Gen_i::GetAllBoundarysName()
{
  MESSAGE("GetAllBoundarysName");

  SMESHHOMARD::listeBoundarys_var ret = new SMESHHOMARD::listeBoundarys;
  ret->length(myStudyContext._mesBoundarys.size());
  std::map<std::string, SMESHHOMARD::HOMARD_Boundary_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesBoundarys.begin();
  it != myStudyContext._mesBoundarys.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}
//=============================================================================
SMESHHOMARD::listeCases* HOMARD_Gen_i::GetAllCasesName()
{
  MESSAGE("GetAllCasesName");

  SMESHHOMARD::listeCases_var ret = new SMESHHOMARD::listeCases;
  ret->length(myStudyContext._mesCas.size());
  std::map<std::string, SMESHHOMARD::HOMARD_Cas_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesCas.begin();
  it != myStudyContext._mesCas.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}
//=============================================================================
SMESHHOMARD::listeHypotheses* HOMARD_Gen_i::GetAllHypothesesName()
{
  MESSAGE("GetAllHypothesesName");

  SMESHHOMARD::listeHypotheses_var ret = new SMESHHOMARD::listeHypotheses;
  ret->length(myStudyContext._mesHypotheses.size());
  std::map<std::string, SMESHHOMARD::HOMARD_Hypothesis_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesHypotheses.begin();
  it != myStudyContext._mesHypotheses.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}
//=============================================================================
SMESHHOMARD::listeIterations* HOMARD_Gen_i::GetAllIterationsName()
{
  MESSAGE("GetAllIterationsName");

  SMESHHOMARD::listeIterations_var ret = new SMESHHOMARD::listeIterations;
  ret->length(myStudyContext._mesIterations.size());
  std::map<std::string, SMESHHOMARD::HOMARD_Iteration_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesIterations.begin();
  it != myStudyContext._mesIterations.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}

//=============================================================================
//=============================================================================
// Recuperation des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::GetBoundary(const char* nomBoundary)
{
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[nomBoundary];
  ASSERT(!CORBA::is_nil(myBoundary));
  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::GetCase(const char* nomCas)
{
  SMESHHOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  ASSERT(!CORBA::is_nil(myCase));
  return SMESHHOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
SMESHHOMARD::HOMARD_Hypothesis_ptr HOMARD_Gen_i::GetHypothesis(const char* nomHypothesis)
{
  SMESHHOMARD::HOMARD_Hypothesis_var myHypothesis = myStudyContext._mesHypotheses[nomHypothesis];
  ASSERT(!CORBA::is_nil(myHypothesis));
  return SMESHHOMARD::HOMARD_Hypothesis::_duplicate(myHypothesis);
}
//=============================================================================
SMESHHOMARD::HOMARD_Iteration_ptr  HOMARD_Gen_i::GetIteration(const char* NomIterationation)
{
  SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[NomIterationation];
  ASSERT(!CORBA::is_nil(myIteration));
  return SMESHHOMARD::HOMARD_Iteration::_duplicate(myIteration);
}
//=============================================================================
//=============================================================================
// Informations
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::MeshInfo(const char* nomCas, const char* MeshName, const char* MeshFile, const char* DirName, CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte)
{
  MESSAGE ( "MeshInfo : nomCas = " << nomCas << ", MeshName = " << MeshName << ", MeshFile = " << MeshFile  );
  MESSAGE ( "Qual = " << Qual << ", Diam = " << Diam << ", Conn = " << Conn << ", Tail = " << Tail << ", Inte = " << Inte  );

  // Creation du cas
  int option = 1;
  if ( _PublisMeshIN != 0 ) option = 2;
  SMESHHOMARD::HOMARD_Cas_ptr myCase = CreateCase0(nomCas, MeshName, MeshFile, 1, 0, option);
  myCase->SetDirName(DirName);
  // Analyse
  myCase->MeshInfo(Qual, Diam, Conn, Tail, Inte);
}
//=============================================================================
//=============================================================================

//=============================================================================
//=============================================================================
// Recuperation des structures par le contexte
//=============================================================================
//=============================================================================
SMESHHOMARD::HOMARD_Iteration_ptr HOMARD_Gen_i::LastIteration(const char* nomCas)
{
  SMESHHOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  ASSERT(!CORBA::is_nil(myCase));
//
  SMESHHOMARD::HOMARD_Iteration_var myIteration = myCase->LastIteration();
  ASSERT(!CORBA::is_nil(myIteration));
//
  return SMESHHOMARD::HOMARD_Iteration::_duplicate(myIteration);
}
//=============================================================================
//=============================================================================

//=============================================================================
//=============================================================================
// Nouvelles structures
//=============================================================================
//=============================================================================
SMESHHOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::newCase()
{
  SMESHHOMARD::HOMARD_Gen_var engine = POA_SMESHHOMARD::HOMARD_Gen::_this();
  HOMARD_Cas_i* aServant = new HOMARD_Cas_i(SMESH_Gen_i::GetORB(), engine);
  SMESHHOMARD::HOMARD_Cas_var aCase = SMESHHOMARD::HOMARD_Cas::_narrow(aServant->_this());
  return aCase._retn();
}
//=============================================================================
SMESHHOMARD::HOMARD_Hypothesis_ptr HOMARD_Gen_i::newHypothesis()
{
  SMESHHOMARD::HOMARD_Gen_var engine = POA_SMESHHOMARD::HOMARD_Gen::_this();
  HOMARD_Hypothesis_i* aServant = new HOMARD_Hypothesis_i(SMESH_Gen_i::GetORB(), engine);
  SMESHHOMARD::HOMARD_Hypothesis_var aHypo = SMESHHOMARD::HOMARD_Hypothesis::_narrow(aServant->_this());
  return aHypo._retn();
}
//=============================================================================
SMESHHOMARD::HOMARD_Iteration_ptr HOMARD_Gen_i::newIteration()
{
  SMESHHOMARD::HOMARD_Gen_var engine = POA_SMESHHOMARD::HOMARD_Gen::_this();
  HOMARD_Iteration_i* aServant = new HOMARD_Iteration_i(SMESH_Gen_i::GetORB(), engine);
  SMESHHOMARD::HOMARD_Iteration_var aIter = SMESHHOMARD::HOMARD_Iteration::_narrow(aServant->_this());
  return aIter._retn();
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::newBoundary()
{
  SMESHHOMARD::HOMARD_Gen_var engine = POA_SMESHHOMARD::HOMARD_Gen::_this();
  HOMARD_Boundary_i* aServant = new HOMARD_Boundary_i(SMESH_Gen_i::GetORB(), engine);
  SMESHHOMARD::HOMARD_Boundary_var aBoundary = SMESHHOMARD::HOMARD_Boundary::_narrow(aServant->_this());
  return aBoundary._retn();
}

//=============================================================================
//=============================================================================
// Creation des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
SMESHHOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::CreateCase(const char* nomCas, const char* MeshName, const char* MeshFile)
//
// Creation d'un cas initial
// nomCas : nom du cas a creer
// MeshName, MeshFile : nom et fichier du maillage correspondant
//
{
  INFOS ( "CreateCase : nomCas = " << nomCas << ", MeshName = " << MeshName << ", MeshFile = " << MeshFile );

  int option = 1;
  if ( _PublisMeshIN != 0 ) option = 2;
  SMESHHOMARD::HOMARD_Cas_ptr myCase = CreateCase0(nomCas, MeshName, MeshFile, 0, 0, option);

  // Valeurs par defaut des filtrages
  myCase->SetPyram(0);

  return SMESHHOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
std::string HOMARD_Gen_i::CreateCase1(const char* DirNameStart, CORBA::Long Number)
//
// Retourne le nom du répertoire ou se trouve l'iteration voulue.
// DirNameStart : nom du répertoire du cas contenant l'iteration de reprise
// Number : numero de l'iteration de depart ou -1 si on cherche la derniere
//
{
  MESSAGE ( "CreateCase1 : DirNameStart = " << DirNameStart << ", Number = " << Number );
  std::string nomDirWork = getenv("PWD");
  std::string DirNameStartIter;
  int codret;
  int NumeIterMax = -1;

  // A.1. Controle du répertoire de depart du cas
  codret = CHDIR(DirNameStart);
  if ( codret != 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory of the case for the pursuit does not exist.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  // A.2. Reperage des sous-répertoire du répertoire de reprise
  bool existe = false;
#ifndef WIN32
  DIR *dp;
  struct dirent *dirp;
  dp  = opendir(DirNameStart);
  while ( (dirp = readdir(dp)) != NULL ) {
    std::string DirName_1(dirp->d_name);
#else
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd;
  hFind = FindFirstFile(DirNameStart, &ffd);
  if (INVALID_HANDLE_VALUE != hFind) {
    while (FindNextFile(hFind, &ffd) != 0) {
      std::string DirName_1 = "";
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        DirName_1 = std::string(ffd.cFileName);
      }
#endif
    if ( ( DirName_1 != "." ) && ( DirName_1 != ".." ) )
    {
      if ( CHDIR(DirName_1.c_str()) == 0 )
      {
//      On cherche le fichier de configuration dans ce sous-répertoire
        codret = CHDIR(DirNameStart);
#ifndef WIN32
        DIR *dp_1;
        struct dirent *dirp_1;
        dp_1  = opendir(DirName_1.c_str());
        while ( (dirp_1 = readdir(dp_1)) != NULL )
        {
          std::string file_name_1(dirp_1->d_name);
#else
        HANDLE hFind1 = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA ffd1;
        hFind1 = FindFirstFile(DirName_1.c_str(), &ffd1);
        while (FindNextFile(hFind1, &ffd1) != 0)
        {
          if (ffd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue; //skip directories
          std::string file_name_1(ffd1.cFileName);
#endif
          int bilan = file_name_1.find("HOMARD.Configuration.");
          if ( bilan != string::npos )
          {
  // Decodage du fichier pour trouver le numero d'iteration
            CHDIR(DirName_1.c_str());

            std::ifstream fichier( file_name_1.c_str() );
            if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
            {
              int NumeIter;
              std::string ligne; // variable contenant chaque ligne lue
              std::string mot_cle;
              // cette boucle sur les lignes s'arrête dès qu'une erreur de lecture survient
              while ( std::getline( fichier, ligne ) )
              {
                // B.1. Pour la ligne courante, on identifie le premier mot : le mot-cle
                std::istringstream ligne_bis(ligne); // variable contenant chaque ligne sous forme de flux
                ligne_bis >> mot_cle;
                if ( mot_cle == "NumeIter" )
                {
                  ligne_bis >> NumeIter;
                  NumeIter += 1;
//                   MESSAGE ( "==> NumeIter   : " << NumeIter );
                  if ( Number == - 1 )
                  {
                    if ( NumeIter >= NumeIterMax )
                    {
                      NumeIterMax = NumeIter;
                      DirNameStartIter = DirName_1;
                    }
                  }
                  else
                  {
                    if ( NumeIter == Number )
                    {
                      DirNameStartIter = DirName_1;
                      existe = true;
                      break;
                    }
                  }
                }
              }
            }
            else
            {
              SALOME::ExceptionStruct es;
              es.type = SALOME::BAD_PARAM;
              std::string text = "The configuration file cannot be read.";
              es.text = CORBA::string_dup(text.c_str());
              throw SALOME::SALOME_Exception(es);
            }
            CHDIR(DirNameStart);
          }
          if ( existe ) { break; }
        }
#ifndef WIN32
        closedir(dp_1);
#else
        FindClose(hFind1);
#endif
        if ( existe ) { break; }
     }
    }
  }
#ifndef WIN32
  closedir(dp);
#else
  FindClose(hFind);
#endif
  CHDIR(nomDirWork.c_str());

  if ( ( Number >= 0 && ( !existe ) ) || ( Number < 0 && ( NumeIterMax == -1 ) ) )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory of the iteration does not exist.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  return DirNameStartIter;
}
//=============================================================================
SMESHHOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::CreateCase0(const char* nomCas, const char* MeshName, const char* MeshFile, CORBA::Long MeshOption, CORBA::Long NumeIter, CORBA::Long Option)
//
// nomCas : nom du cas a creer
// MeshName, MeshFile : nom et fichier du maillage correspondant
// MeshOption : 0 : le maillage fourni est obligatoirement present ==> erreur si absent
//              1 : le maillage fourni peut ne pas exister ==> on continue si absent
//             -1 : le maillage n'est pas fourni
// NumeIter : numero de l'iteration correspondante : 0, pour un depart, n>0 pour une poursuite
// Option : multiple de nombres premiers
//         1 : aucune option
//        x2 : publication du maillage dans SMESH
{
  MESSAGE ( "CreateCase0 : nomCas = " << nomCas );
  MESSAGE ( "CreateCase0 : MeshName = " << MeshName << ", MeshFile = " << MeshFile << ", MeshOption = " << MeshOption );
  MESSAGE ( "CreateCase0 : NumeIter = " << NumeIter << ", Option = " << Option );
//
  // A. Controles
  // A.2. Controle du nom :
  if ((myStudyContext._mesCas).find(nomCas)!=(myStudyContext._mesCas).end())
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This case has already been defined.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  // A.3. Controle du fichier du maillage
  int existeMeshFile;
  if ( MeshOption >= 0 )
  {
    existeMeshFile = MEDFileExist ( MeshFile );
    MESSAGE ( "CreateCase0 : existeMeshFile = " << existeMeshFile );
    if ( ( existeMeshFile == 0 ) && ( MeshOption == 0 ) )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "The mesh file does not exist.";
      throw SALOME::SALOME_Exception(es);
      return 0;
    }
  }
  else { existeMeshFile = 0; }

  // B. Creation de l'objet cas et publication
  SMESHHOMARD::HOMARD_Cas_var myCase = newCase();
  myCase->SetName(nomCas);
  myStudyContext._mesCas[nomCas] = myCase;

  // C. Caracteristiques du maillage
  if ( existeMeshFile != 0 ) {
    // Les valeurs extremes des coordonnées
    //MESSAGE ( "CreateCase0 : Les valeurs extremes des coordonnées" );
    std::vector<double> LesExtremes =GetBoundingBoxInMedFile(MeshFile);
    SMESHHOMARD::extrema_var aSeq = new SMESHHOMARD::extrema();
    if (LesExtremes.size()!=10) { return 0; }
    aSeq->length(10);
    for (int i =0; i< LesExtremes.size(); i++)
        aSeq[i]=LesExtremes[i];
    myCase->SetBoundingBox(aSeq);
    // Les groupes
    //MESSAGE ( "CreateCase0 : Les groupes" );
    std::set<std::string> LesGroupes  =GetListeGroupesInMedFile(MeshFile);
    SMESHHOMARD::ListGroupType_var aSeqGroupe = new SMESHHOMARD::ListGroupType;
    aSeqGroupe->length(LesGroupes.size());
    std::set<std::string>::const_iterator it;
    int i = 0;
    for (it=LesGroupes.begin(); it != LesGroupes.end(); it++)
      aSeqGroupe[i++]=(*it).c_str();
    myCase->SetGroups(aSeqGroupe);
  }

  // D. L'iteration initiale du cas
  MESSAGE ( "CreateCase0 : iteration initiale du cas" );
  // D.1. Recherche d'un nom : par defaut, on prend le nom du maillage correspondant.
  // Si ce nom d'iteration existe deja, on incremente avec 0, 1, 2, etc.
  int monNum = 0;
  std::string NomIteration = std::string(MeshName);
  while ( (myStudyContext._mesIterations).find(NomIteration) != (myStudyContext._mesIterations.end()) )
  {
    std::ostringstream nom;
    nom << MeshName << monNum;
    NomIteration = nom.str();
    monNum += 1;
  }
  MESSAGE ( "CreateCas0 : ==> NomIteration = " << NomIteration );

  // D.2. Creation de l'iteration
  SMESHHOMARD::HOMARD_Iteration_var anIter = newIteration();
  myStudyContext._mesIterations[NomIteration] = anIter;
  anIter->SetName(NomIteration.c_str());
  AssociateCaseIter (nomCas, NomIteration.c_str(), "IterationHomard");

  // D.4. Maillage correspondant
  if ( existeMeshFile != 0 )
  {
    anIter->SetMeshFile(MeshFile);
    if ( Option % 2 == 0 ) { PublishResultInSmesh(MeshFile, 0); }
  }
  anIter->SetMeshName(MeshName);

  // D.5. Numero d'iteration
  anIter->SetNumber(NumeIter);

  // D.6. Etat
  SetEtatIter(NomIteration.c_str(), -NumeIter);
//

  return SMESHHOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
SMESHHOMARD::HOMARD_Hypothesis_ptr HOMARD_Gen_i::CreateHypothesis(const char* nomHypothesis)
{
  MESSAGE ( "CreateHypothesis : nomHypothesis = " << nomHypothesis );

  // A. Controle du nom :
  if ((myStudyContext._mesHypotheses).find(nomHypothesis) != (myStudyContext._mesHypotheses).end())
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This hypothesis has already been defined.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  }

  // B. Creation de l'objet
  SMESHHOMARD::HOMARD_Hypothesis_var myHypothesis = newHypothesis();
  if (CORBA::is_nil(myHypothesis))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Unable to create the hypothesis";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  myHypothesis->SetName(nomHypothesis);

  // C. Enregistrement
  myStudyContext._mesHypotheses[nomHypothesis] = myHypothesis;

  // D. Valeurs par defaut des options avancees
  myHypothesis->SetNivMax(-1);
  myHypothesis->SetDiamMin(-1.0);
  myHypothesis->SetAdapInit(0);
  myHypothesis->SetExtraOutput(1);

  return SMESHHOMARD::HOMARD_Hypothesis::_duplicate(myHypothesis);
}

//=============================================================================
SMESHHOMARD::HOMARD_Iteration_ptr HOMARD_Gen_i::CreateIteration(const char* NomIteration, const char* nomIterParent)
//=============================================================================
{
  MESSAGE ("CreateIteration : NomIteration  = " << NomIteration << ", nomIterParent = " << nomIterParent);

  SMESHHOMARD::HOMARD_Iteration_var myIterationParent = myStudyContext._mesIterations[nomIterParent];
  if (CORBA::is_nil(myIterationParent))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The parent iteration is not defined.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  const char* nomCas = myIterationParent->GetCaseName();
  MESSAGE ("CreateIteration : nomCas = " << nomCas);
  SMESHHOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  if (CORBA::is_nil(myCase)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid case context";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  const char* nomDirCase = myCase->GetDirName();

  // Controle du nom :
  if ((myStudyContext._mesIterations).find(NomIteration) !=
      (myStudyContext._mesIterations).end())
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This iteration has already been defined.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  }

  SMESHHOMARD::HOMARD_Iteration_var myIteration = newIteration();
  if (CORBA::is_nil(myIteration)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Unable to create the iteration";
    throw SALOME::SALOME_Exception(es);
    return 0;
  }

  myStudyContext._mesIterations[std::string(NomIteration)] = myIteration;
  // Nom de l'iteration et du maillage
  myIteration->SetName(NomIteration);
  myIteration->SetMeshName(NomIteration);
  myIteration->SetState(1);

  int numero = myIterationParent->GetNumber() + 1;
  myIteration->SetNumber(numero);

  // Nombre d'iterations deja connues pour le cas, permettant
  // la creation d'un sous-répertoire unique
  int nbitercase = myCase->GetNumberofIter();
  char* nomDirIter = CreateDirNameIter(nomDirCase, nbitercase );
  myIteration->SetDirNameLoc(nomDirIter);

  // Le nom du fichier du maillage MED est indice par le nombre d'iterations du cas.
  // Si on a une chaine unique depuis le depart, ce nombre est le meme que le
  // numero d'iteration dans la sucession : maill.01.med, maill.02.med, etc... C'est la
  // situation la plus frequente.
  // Si on a plusieurs branches, donc des iterations du meme niveau d'adaptation, utiliser
  // le nombre d'iterations du cas permet d'eviter les collisions.
  int jaux;
  if      ( nbitercase <    100 ) { jaux = 2; }
  else if ( nbitercase <   1000 ) { jaux = 3; }
  else if ( nbitercase <  10000 ) { jaux = 4; }
  else if ( nbitercase < 100000 ) { jaux = 5; }
  else                            { jaux = 9; }
  std::ostringstream iaux;
  iaux << std::setw(jaux) << std::setfill('0') << nbitercase;
  std::stringstream MeshFile;
  MeshFile << nomDirCase << "/maill." << iaux.str() << ".med";
  myIteration->SetMeshFile(MeshFile.str().c_str());

  // Association avec le cas
  std::string label = "IterationHomard_" + std::string(nomIterParent);
  AssociateCaseIter(nomCas, NomIteration, label.c_str());

  // Lien avec l'iteration precedente
  myIterationParent->LinkNextIteration(NomIteration);
  myIteration->SetIterParentName(nomIterParent);

  return SMESHHOMARD::HOMARD_Iteration::_duplicate(myIteration);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundary(const char* BoundaryName, CORBA::Long BoundaryType)
{
  MESSAGE ("CreateBoundary : BoundaryName  = " << BoundaryName << ", BoundaryType = " << BoundaryType);

  // Controle du nom :
  if ((myStudyContext._mesBoundarys).find(BoundaryName)!=(myStudyContext._mesBoundarys).end())
  {
    MESSAGE ("CreateBoundary : la frontiere " << BoundaryName << " existe deja");
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This boundary has already been defined";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  SMESHHOMARD::HOMARD_Boundary_var myBoundary = newBoundary();
  myBoundary->SetName(BoundaryName);
  myBoundary->SetType(BoundaryType);

  myStudyContext._mesBoundarys[BoundaryName] = myBoundary;

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryCAO(const char* BoundaryName, const char* CAOFile)
{
  MESSAGE ("CreateBoundaryCAO : BoundaryName  = " << BoundaryName << ", CAOFile = " << CAOFile );
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, -1);
  myBoundary->SetDataFile( CAOFile );

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryDi(const char* BoundaryName, const char* MeshName, const char* MeshFile)
{
  MESSAGE ("CreateBoundaryDi : BoundaryName  = " << BoundaryName << ", MeshName = " << MeshName << ", MeshFile = " << MeshFile );
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 0);
  myBoundary->SetDataFile( MeshFile );
  myBoundary->SetMeshName( MeshName );

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryCylinder(const char* BoundaryName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe,
                                      CORBA::Double Rayon)
{
  MESSAGE ("CreateBoundaryCylinder : BoundaryName  = " << BoundaryName );
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if ( Rayon <= 0.0 )
  { es.text = "The radius must be positive.";
    error = 1; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe);
  if ( daux < 0.0000001 )
  { es.text = "The axis must be a non 0 vector.";
    error = 2; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 1);
  myBoundary->SetCylinder( Xcentre, Ycentre, Zcentre, Xaxe, Yaxe, Zaxe, Rayon );

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundarySphere(const char* BoundaryName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Rayon)
{
  MESSAGE ("CreateBoundarySphere : BoundaryName  = " << BoundaryName );
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if ( Rayon <= 0.0 )
  { es.text = "The radius must be positive.";
    error = 1; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 2);
  myBoundary->SetSphere( Xcentre, Ycentre, Zcentre, Rayon );

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryConeA(const char* BoundaryName,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe, CORBA::Double Angle,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre)
{
  MESSAGE ("CreateBoundaryConeA : BoundaryName  = " << BoundaryName );
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if ( Angle <= 0.0 || Angle >= 90.0 )
  { es.text = "The angle must be included higher than 0 degree and lower than 90 degrees.";
    error = 1; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe);
  if ( daux < 0.0000001 )
  { es.text = "The axis must be a non 0 vector.";
    error = 2; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 3);
  myBoundary->SetConeA( Xaxe, Yaxe, Zaxe, Angle, Xcentre, Ycentre, Zcentre );

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryConeR(const char* BoundaryName,
                                      CORBA::Double Xcentre1, CORBA::Double Ycentre1, CORBA::Double Zcentre1, CORBA::Double Rayon1,
                                      CORBA::Double Xcentre2, CORBA::Double Ycentre2, CORBA::Double Zcentre2, CORBA::Double Rayon2)
{
  MESSAGE ("CreateBoundaryConeR : BoundaryName  = " << BoundaryName );
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if ( Rayon1 < 0.0 || Rayon2 < 0.0 )
  { es.text = "The radius must be positive.";
    error = 1; }
  double daux = fabs(Rayon2-Rayon1);
  if ( daux < 0.0000001 )
  { es.text = "The radius must be different.";
    error = 2; }
  daux = fabs(Xcentre2-Xcentre1) + fabs(Ycentre2-Ycentre1) + fabs(Zcentre2-Zcentre1);
  if ( daux < 0.0000001 )
  { es.text = "The centers must be different.";
    error = 3; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 4);
  myBoundary->SetConeR( Xcentre1, Ycentre1, Zcentre1, Rayon1, Xcentre2, Ycentre2, Zcentre2, Rayon2 );

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryTorus(const char* BoundaryName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe,
                                      CORBA::Double RayonRev, CORBA::Double RayonPri)
{
  MESSAGE ("CreateBoundaryTorus : BoundaryName  = " << BoundaryName );
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if ( ( RayonRev <= 0.0 ) || ( RayonPri <= 0.0 ) )
  { es.text = "The radius must be positive.";
    error = 1; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe);
  if ( daux < 0.0000001 )
  { es.text = "The axis must be a non 0 vector.";
    error = 2; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 5);
  myBoundary->SetTorus( Xcentre, Ycentre, Zcentre, Xaxe, Yaxe, Zaxe, RayonRev, RayonPri );

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}

//=============================================================================
//=============================================================================
// Traitement d'une iteration
// etatMenage = 1 : destruction du répertoire d'execution
// modeHOMARD  = 1 : adaptation
//            != 1 : information avec les options modeHOMARD
// Option1 >0 : appel depuis python
//         <0 : appel depuis GUI
// Option2 : multiple de nombres premiers
//         1 : aucune option
//        x2 : publication du maillage dans SMESH
//=============================================================================
CORBA::Long HOMARD_Gen_i::Compute(const char* NomIteration, CORBA::Long etatMenage, CORBA::Long modeHOMARD, CORBA::Long Option1, CORBA::Long Option2)
{
  INFOS ( "Compute : traitement de " << NomIteration << ", avec modeHOMARD = " << modeHOMARD << ", Option1 = " << Option1 << ", Option2 = " << Option2 );

  // A. Prealable
  int codret = 0;

  // A.1. L'objet iteration
  SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[NomIteration];
  ASSERT(!CORBA::is_nil(myIteration));

  // A.2. Controle de la possibilite d'agir
  // A.2.1. Etat de l'iteration
  int etat = myIteration->GetState();
  MESSAGE ( "etat = "<<etat );
  // A.2.2. On ne calcule pas l'iteration initiale, ni une iteration deja calculee
  if ( modeHOMARD == 1 )
  {
    if ( etat <= 0 )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "This iteration is the first of the case and cannot be computed.";
      throw SALOME::SALOME_Exception(es);
      return 1;
    }
    else if ( ( etat == 2 ) & ( modeHOMARD == 1 ) )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "This iteration is already computed.";
      throw SALOME::SALOME_Exception(es);
      return 1;
    }
  }
  // A.2.3. On n'analyse pas une iteration non calculee
  else
  {
    if ( etat == 1 )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "This iteration is not computed.";
      throw SALOME::SALOME_Exception(es);
      return 1;
    }
  }

  // A.3. Numero de l'iteration
  //     siterp1 : numero de l'iteration a traiter
  //     Si adaptation :
  //        siter   : numero de l'iteration parent, ou 0 si deja au debut mais cela ne servira pas !
  //     Ou si information :
  //        siter = siterp1
  int NumeIter = myIteration->GetNumber();
  std::string siterp1;
  std::stringstream saux1;
  saux1 << NumeIter;
  siterp1 = saux1.str();
  if (NumeIter < 10) { siterp1 = "0" + siterp1; }

  std::string siter;
  if ( modeHOMARD==1 )
  {
    std::stringstream saux0;
    int iaux = max(0, NumeIter-1);
    saux0 << iaux;
    siter = saux0.str();
    if (NumeIter < 11) { siter = "0" + siter; }
  }
  else
  { siter = siterp1; }

  // A.4. Le cas
  const char* nomCas = myIteration->GetCaseName();
  SMESHHOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  ASSERT(!CORBA::is_nil(myCase));

  // B. Les répertoires
  // B.1. Le répertoire courant
  std::string nomDirWork = getenv("PWD");
  // B.2. Le sous-répertoire de l'iteration a traiter
  char* DirCompute = ComputeDirManagement(myCase, myIteration, etatMenage);
  MESSAGE( ". DirCompute = " << DirCompute );

  // C. Le fichier des messages
  // C.1. Le deroulement de l'execution de HOMARD
  std::string LogFile = myIteration->GetLogFile();
  if (LogFile.empty()) {
    LogFile = DirCompute;
    LogFile += "/Liste";
    if ( modeHOMARD == 1 ) { LogFile += "." + siter + ".vers." + siterp1; }
    LogFile += ".log";
    if ( modeHOMARD == 1 ) myIteration->SetLogFile(LogFile.c_str());
  }
  MESSAGE (". LogFile = " << LogFile);
  // C.2. Le bilan de l'analyse du maillage
  std::string FileInfo = DirCompute;
  FileInfo += "/";
  if ( modeHOMARD == 1 ) { FileInfo += "apad"; }
  else
  { if ( NumeIter == 0 ) { FileInfo += "info_av"; }
    else                 { FileInfo += "info_ap"; }
  }
  FileInfo += "." + siterp1 + ".bilan";
  myIteration->SetFileInfo(FileInfo.c_str());

   // D. On passe dans le répertoire de l'iteration a calculer
  MESSAGE ( ". On passe dans DirCompute = " << DirCompute );
  CHDIR(DirCompute);

  // E. Les données de l'exécution HOMARD
  // E.1. L'objet du texte du fichier de configuration
  SMESHHOMARDImpl::HomardDriver* myDriver = new SMESHHOMARDImpl::HomardDriver(siter, siterp1);
  myDriver->TexteInit(DirCompute, LogFile, _Langue);

  // E.2. Le maillage associe a l'iteration
  const char* NomMesh = myIteration->GetMeshName();
  MESSAGE ( ". NomMesh = " << NomMesh );
  const char* MeshFile = myIteration->GetMeshFile();
  MESSAGE ( ". MeshFile = " << MeshFile );

  // E.3. Les données du traitement HOMARD
  int iaux;
  if ( modeHOMARD == 1 )
  {
    iaux = 1;
    myDriver->TexteMaillageHOMARD( DirCompute, siterp1, iaux );
    myDriver->TexteMaillage(NomMesh, MeshFile, 1);
    codret = ComputeAdap(myCase, myIteration, etatMenage, myDriver, Option1, Option2);
  }
  else
  {
    InvalideIterInfo(NomIteration);
    myDriver->TexteInfo( modeHOMARD, NumeIter );
    iaux = 0;
    myDriver->TexteMaillageHOMARD( DirCompute, siterp1, iaux );
    myDriver->TexteMaillage(NomMesh, MeshFile, 0);
    myDriver->CreeFichierDonn();
  }

  // E.4. Ajout des informations liees a l'eventuel suivi de frontiere
  int BoundaryOption = DriverTexteBoundary(myCase, myDriver);

  // E.5. Ecriture du texte dans le fichier
  MESSAGE ( ". Ecriture du texte dans le fichier de configuration; codret = "<<codret );
  if (codret == 0)
  { myDriver->CreeFichier(); }

// G. Execution
//
  int codretexec = 1789;
  if (codret == 0)
  {
    codretexec = myDriver->ExecuteHomard(Option1);
//
    MESSAGE ( "Erreur en executant HOMARD : " << codretexec );
    // En mode adaptation, on ajuste l'etat de l'iteration
    if ( modeHOMARD == 1 )
    {
      if (codretexec == 0) { SetEtatIter(NomIteration,2); }
      else                 { SetEtatIter(NomIteration,1); }
      // GERALD -- QMESSAGE BOX
    }
  }

  // H. Gestion des resultats
  if (codret == 0)
  {
    std::string Commentaire;
    // H.1. Le fichier des messages, dans tous les cas
    Commentaire = "log";
    if ( modeHOMARD == 1 ) { Commentaire += " " + siterp1; }
    else                   { Commentaire += "Info"; }

    // H.2. Si tout s'est bien passe :
    if (codretexec == 0)
    {
    // H.2.1. Le fichier de bilan
      Commentaire = "Summary";
      if ( modeHOMARD == 1 ) { Commentaire += " " + siterp1; }
      else                   { Commentaire += "Info"; }
    // H.2.2. Le fichier de  maillage obtenu
      if ( modeHOMARD == 1 )
      {
        std::stringstream saux0;
        Commentaire = "Mesh";
        Commentaire += " " + siterp1;
        if ( Option2 % 2 == 0 ) { PublishResultInSmesh(MeshFile, 1); }
      }
    }
    // H.3 Message d'erreur
    if (codretexec != 0) {
      std::string text = "";
      // Message d'erreur en cas de probleme en adaptation
      if ( modeHOMARD == 1 ) {
        text = "Error during the adaptation.\n";
        bool stopvu = false;
        std::ifstream fichier( LogFile.c_str() );
        if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
        {
          std::string ligne; // variable contenant chaque ligne lue
          while ( std::getline( fichier, ligne ) )
          {
//             INFOS(ligne);
            if ( stopvu )
            { text += ligne+ "\n"; }
            else
            {
              int position = ligne.find( "===== HOMARD ===== STOP =====" );
              if ( position > 0 ) { stopvu = true; }
            }
          }
        }
      }
      text += "\n\nSee the file " + LogFile + "\n";
      INFOS ( text );
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
//
      // En mode information, on force le succes pour pouvoir consulter le fichier log
      if ( modeHOMARD != 1 ) { codretexec = 0; }
    }
  }

  // I. Menage et retour dans le répertoire du cas
  if (codret == 0)
  {
    delete myDriver;
    MESSAGE ( ". On retourne dans nomDirWork = " << nomDirWork );

    CHDIR(nomDirWork.c_str());
  }

  // J. Suivi de la frontière CAO
//   std::cout << "- codret : " << codret << std::endl;
//   std::cout << "- modeHOMARD : " << modeHOMARD << std::endl;
//   std::cout << "- BoundaryOption : " << BoundaryOption << std::endl;
//   std::cout << "- codretexec : " << codretexec << std::endl;
  if (codret == 0)
  {
    if ( ( modeHOMARD == 1 ) && ( BoundaryOption % 5 == 0 ) && (codretexec == 0) )
    {
      MESSAGE ( "Suivi de frontière CAO" );
      codret = ComputeCAO(myCase, myIteration, Option1, Option2);
    }
  }

  return codretexec;
}
//=============================================================================
// Calcul d'une iteration : partie spécifique à l'adaptation
//=============================================================================
CORBA::Long HOMARD_Gen_i::ComputeAdap(SMESHHOMARD::HOMARD_Cas_var myCase,
                                      SMESHHOMARD::HOMARD_Iteration_var myIteration,
                                      CORBA::Long etatMenage,
                                      SMESHHOMARDImpl::HomardDriver* myDriver,
                                      CORBA::Long Option1,
                                      CORBA::Long Option2)
{
  MESSAGE ( "ComputeAdap avec Option1 = " << Option1 << ", Option2 = " << Option2 );

  // A. Prealable
  // A.1. Bases
  int codret = 0;
  // Numero de l'iteration
  int NumeIter = myIteration->GetNumber();
  std::stringstream saux0;
  saux0 << NumeIter-1;
  std::string siter = saux0.str();
  if (NumeIter < 11) { siter = "0" + siter; }

  // A.2. On verifie qu il y a une hypothese (erreur improbable);
  const char* nomHypo = myIteration->GetHypoName();
  if (std::string(nomHypo) == std::string(""))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "This iteration does not have any associated hypothesis.";
      throw SALOME::SALOME_Exception(es);
      return 2;
  };
  SMESHHOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
  ASSERT(!CORBA::is_nil(myHypo));

  // B. L'iteration parent
  const char* nomIterationParent = myIteration->GetIterParentName();
  SMESHHOMARD::HOMARD_Iteration_var myIterationParent = myStudyContext._mesIterations[nomIterationParent];
  ASSERT(!CORBA::is_nil(myIterationParent));
  // Si l'iteration parent n'est pas calculee, on le fait (recursivite amont)
  if ( myIterationParent->GetState() == 1 )
  {
    int iaux = 1;
    int codret = Compute(nomIterationParent, etatMenage, iaux, Option1, Option2);
    if (codret != 0)
    {
      // GERALD -- QMESSAGE BOX
      VERIFICATION("Pb au calcul de l'iteration precedente" == 0);
    }
  };

  // C. Le sous-répertoire de l'iteration precedente
  char* DirComputePa = ComputeDirPaManagement(myCase, myIteration);
  MESSAGE( ". DirComputePa = " << DirComputePa );

  // D. Les données de l'adaptation HOMARD
  // D.1. Le type de conformite
  int ConfType = myCase->GetConfType();
  MESSAGE ( ". ConfType = " << ConfType );

  // D.1. Le type externe
  int ExtType = myCase->GetExtType();
  MESSAGE ( ". ExtType = " << ExtType );

  // D.3. Le maillage de depart
  const char* NomMeshParent = myIterationParent->GetMeshName();
  MESSAGE ( ". NomMeshParent = " << NomMeshParent );
  const char* MeshFileParent = myIterationParent->GetMeshFile();
  MESSAGE ( ". MeshFileParent = " << MeshFileParent );

  // D.4. Le maillage associe a l'iteration
  const char* MeshFile = myIteration->GetMeshFile();
  MESSAGE ( ". MeshFile = " << MeshFile );
  FILE *file = fopen(MeshFile,"r");
  if (file != NULL)
  {
    fclose(file);
    if (etatMenage == 0)
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      std::string text = "MeshFile : " + std::string(MeshFile) + " already exists ";
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
      return 4;
    }
    else
    {
      std::string commande = "rm -f " + std::string(MeshFile);
      codret = system(commande.c_str());
      if (codret != 0)
      {
        SALOME::ExceptionStruct es;
        es.type = SALOME::BAD_PARAM;
        es.text = "The mesh file cannot be deleted.";
        throw SALOME::SALOME_Exception(es);
        return 5;
      }
    }
  }

  // D.5. Les types de raffinement et de deraffinement
  // Les appels corba sont lourds, il vaut mieux les grouper
  SMESHHOMARD::listeTypes* ListTypes = myHypo->GetAdapRefinUnRef();
  ASSERT(ListTypes->length() == 3);
  int TypeAdap = (*ListTypes)[0];
  int TypeRaff = (*ListTypes)[1];
  int TypeDera = (*ListTypes)[2];
//   MESSAGE ( ". TypeAdap = " << TypeAdap << ", TypeRaff = " << TypeRaff << ", TypeDera = " << TypeDera  );

  // E. Texte du fichier de configuration
  // E.1. Incontournables du texte
  myDriver->TexteAdap(ExtType);
  int iaux = 0;
  myDriver->TexteMaillageHOMARD( DirComputePa, siter, iaux );
  myDriver->TexteMaillage(NomMeshParent, MeshFileParent, 0);
  myDriver->TexteConfRaffDera(ConfType, TypeAdap, TypeRaff, TypeDera);

  // E.3. Ajout des informations liees aux champs eventuels
  if ( TypeAdap == 1 )
  { DriverTexteField(myIteration, myHypo, myDriver); }

  // E.4. Ajout des informations liees au filtrage eventuel par les groupes
  SMESHHOMARD::ListGroupType* listeGroupes = myHypo->GetGroups();
  int numberOfGroups = listeGroupes->length();
  MESSAGE( ". Filtrage par " << numberOfGroups << " groupes");
  if (numberOfGroups > 0)
  {
    for (int NumGroup = 0; NumGroup< numberOfGroups; NumGroup++)
    {
      std::string GroupName = std::string((*listeGroupes)[NumGroup]);
      MESSAGE( "... GroupName = " << GroupName );
      myDriver->TexteGroup(GroupName);
    }
  }

  // E.5. Ajout des informations liees a l'eventuelle interpolation des champs
  DriverTexteFieldInterp(myIteration, myHypo, myDriver);

  // E.6. Ajout des options avancees
  int Pyram = myCase->GetPyram();
  MESSAGE ( ". Pyram = " << Pyram );
  int NivMax = myHypo->GetNivMax();
  MESSAGE ( ". NivMax = " << NivMax );
  double DiamMin = myHypo->GetDiamMin();
  MESSAGE ( ". DiamMin = " << DiamMin );
  int AdapInit = myHypo->GetAdapInit();
  MESSAGE ( ". AdapInit = " << AdapInit );
  int ExtraOutput = myHypo->GetExtraOutput();
  MESSAGE ( ". ExtraOutput = " << ExtraOutput );
  myDriver->TexteAdvanced(Pyram, NivMax, DiamMin, AdapInit, ExtraOutput);

  // E.7. Ajout des informations sur le deroulement de l'execution
  int MessInfo = myIteration->GetInfoCompute();
  MESSAGE ( ". MessInfo = " << MessInfo );
  myDriver->TexteInfoCompute(MessInfo);

  return codret;
}
//=============================================================================
// Calcul d'une iteration : partie spécifique au suivi de frontière CAO
//=============================================================================
CORBA::Long HOMARD_Gen_i::ComputeCAO(SMESHHOMARD::HOMARD_Cas_var myCase, SMESHHOMARD::HOMARD_Iteration_var myIteration, CORBA::Long Option1, CORBA::Long Option2)
{
  MESSAGE ( "ComputeCAO avec Option1 = " << Option1 << ", Option2 = " << Option2 );

  // A. Prealable
  // A.1. Bases
  int codret = 0;
  // A.2. Le sous-répertoire de l'iteration en cours de traitement
  char* DirCompute = myIteration->GetDirName();
  // A.3. Le maillage résultat de l'iteration en cours de traitement
  char* MeshFile = myIteration->GetMeshFile();

  // B. Les données pour FrontTrack
  // B.1. Le maillage à modifier
  const std::string theInputMedFile = MeshFile;
  MESSAGE ( ". theInputMedFile  = " << theInputMedFile );

  // B.2. Le maillage après modification : fichier identique
  const std::string theOutputMedFile = MeshFile;
  MESSAGE ( ". theOutputMedFile = " << theInputMedFile );

  // B.3. La liste des fichiers contenant les numéros des noeuds à bouger
  std::vector< std::string > theInputNodeFiles;
  MESSAGE ( ". DirCompute = " << DirCompute );
  int bilan;
  int icpt = 0;
#ifndef WIN32
  DIR *dp;
  struct dirent *dirp;
  dp  = opendir(DirCompute);
  while ( (dirp = readdir(dp)) != NULL )
  {
    std::string file_name(dirp->d_name);
    bilan = file_name.find("fr");
    if ( bilan != string::npos )
    {
      std::stringstream filename_total;
      filename_total << DirCompute << "/" << file_name;
      theInputNodeFiles.push_back(filename_total.str());
      icpt += 1;
    }
  }
#else
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd;
  hFind = FindFirstFile(DirNameStart, &ffd);
  if (INVALID_HANDLE_VALUE != hFind) {
    while (FindNextFile(hFind, &ffd) != 0) {
      std::string file_name(ffd.cFileName);
      bilan = file_name.find("fr");
      if ( bilan != string::npos )
      {
        std::stringstream filename_total;
        filename_total << DirCompute << "/" << file_name;
        theInputNodeFiles.push_back(filename_total.str());
        icpt += 1;
      }
    }
    FindClose(hFind);
  }
#endif
  for ( int i = 0; i < icpt; i++ )
  { MESSAGE ( ". theInputNodeFiles["<< i << "] = " << theInputNodeFiles[i] ); }

  // B.4. Le fichier de la CAO
  SMESHHOMARD::ListBoundaryGroupType* ListBoundaryGroupType = myCase->GetBoundaryGroup();
  std::string BoundaryName = std::string((*ListBoundaryGroupType)[0]);
  MESSAGE ( ". BoundaryName = " << BoundaryName );
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
  const std::string theXaoFileName = myBoundary->GetDataFile();
  MESSAGE ( ". theXaoFileName = " << theXaoFileName );

  // B.5. Parallélisme
  bool theIsParallel = false;

  // C. Lancement des projections
  MESSAGE ( ". Lancement des projections" );
  //FrontTrack* myFrontTrack = new FrontTrack();
  //myFrontTrack->track(theInputMedFile, theOutputMedFile, theInputNodeFiles, theXaoFileName, theIsParallel);

  // D. Transfert des coordonnées modifiées dans le fichier historique de HOMARD
  //    On lance une exécution spéciale de HOMARD en attendant de savoir le faire avec MEDCoupling
  MESSAGE ( ". Transfert des coordonnées" );
  codret = ComputeCAObis(myIteration, Option1, Option2);

  return codret;
}
//=============================================================================
//=============================================================================
// Transfert des coordonnées en suivi de frontière CAO
// Option1 >0 : appel depuis python
//         <0 : appel depuis GUI
// Option2 : multiple de nombres premiers
//         1 : aucune option
//        x2 : publication du maillage dans SMESH
//=============================================================================
CORBA::Long HOMARD_Gen_i::ComputeCAObis(SMESHHOMARD::HOMARD_Iteration_var myIteration, CORBA::Long Option1, CORBA::Long Option2)
{
  MESSAGE ( "ComputeCAObis, avec Option1 = " << Option1 << ", Option2 = " << Option2 );

  // A. Prealable
  int codret = 0;

  // A.1. Controle de la possibilite d'agir
  // A.1.1. Etat de l'iteration
  int etat = myIteration->GetState();
  MESSAGE ( "etat = "<<etat );
  // A.1.2. L'iteration doit être calculee
  if ( etat == 1 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This iteration is not computed.";
    throw SALOME::SALOME_Exception(es);
    return 1;
  }
  // A.2. Numero de l'iteration
  //     siterp1 : numero de l'iteration a traiter
  int NumeIter = myIteration->GetNumber();
  std::string siterp1;
  std::stringstream saux1;
  saux1 << NumeIter;
  siterp1 = saux1.str();
  if (NumeIter < 10) { siterp1 = "0" + siterp1; }
  MESSAGE ( "siterp1 = "<<siterp1 );

  // A.3. Le cas
  const char* CaseName = myIteration->GetCaseName();
  SMESHHOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[CaseName];
  ASSERT(!CORBA::is_nil(myCase));

  // A.4. Le sous-répertoire de l'iteration a traiter
  char* DirCompute = myIteration->GetDirName();
  MESSAGE( ". DirCompute = " << DirCompute );

  // C. Le fichier des messages
  std::string LogFile = DirCompute;
  LogFile += "/Liste." + siterp1 + ".maj_coords.log";
  MESSAGE (". LogFile = " << LogFile);
  myIteration->SetFileInfo(LogFile.c_str());

   // D. On passe dans le répertoire de l'iteration a calculer
  MESSAGE ( ". On passe dans DirCompute = " << DirCompute );
  CHDIR(DirCompute);

  // E. Les données de l'exécution HOMARD
  // E.1. L'objet du texte du fichier de configuration
  SMESHHOMARDImpl::HomardDriver* myDriver = new SMESHHOMARDImpl::HomardDriver("", siterp1);
  myDriver->TexteInit(DirCompute, LogFile, _Langue);

  // E.2. Le maillage associe a l'iteration
  const char* NomMesh = myIteration->GetMeshName();
  MESSAGE ( ". NomMesh = " << NomMesh );
  const char* MeshFile = myIteration->GetMeshFile();
  MESSAGE ( ". MeshFile = " << MeshFile );

  // E.3. Les données du traitement HOMARD
  int iaux;
  myDriver->TexteMajCoords( NumeIter );
  iaux = 0;
  myDriver->TexteMaillageHOMARD( DirCompute, siterp1, iaux );
  myDriver->TexteMaillage(NomMesh, MeshFile, 0);
//
  // E.4. Ecriture du texte dans le fichier
  MESSAGE ( ". Ecriture du texte dans le fichier de configuration; codret = "<<codret );
  if (codret == 0)
  { myDriver->CreeFichier(); }

// F. Execution
//
  int codretexec = 1789;
  if (codret == 0)
  {
    codretexec = myDriver->ExecuteHomard(Option1);
    MESSAGE ( "Erreur en executant HOMARD : " << codretexec );
  }

  // G. Gestion des resultats
  if (codret == 0)
  {
    // G.1. Le fichier des messages, dans tous les cas
    const char* NomIteration = myIteration->GetName();
    std::string Commentaire = "logmaj_coords";
    // G.2 Message d'erreur
    if (codretexec != 0)
    {
      std::string text = "\n\nSee the file " + LogFile + "\n";
      INFOS ( text );
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);

      // On force le succes pour pouvoir consulter le fichier log
      codretexec = 0;
    }
  }

  // H. Menage et retour dans le répertoire du cas
  if (codret == 0) { delete myDriver; }

  return codret;
}
//=============================================================================
// Creation d'un nom de sous-répertoire pour l'iteration au sein d'un répertoire parent
//  nomrep : nom du répertoire parent
//  num : le nom du sous-répertoire est sous la forme 'In', n est >= num
//=============================================================================
char* HOMARD_Gen_i::CreateDirNameIter(const char* nomrep, CORBA::Long num )
{
  MESSAGE ( "CreateDirNameIter : nomrep ="<< nomrep << ", num = "<<num);
  // On verifie que le répertoire parent existe
  int codret = CHDIR(nomrep);
  if ( codret != 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory of the case does not exist.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  std::string nomDirActuel = getenv("PWD");
  std::string DirName;
  // On boucle sur tous les noms possibles jusqu'a trouver un nom correspondant a un répertoire inconnu
  bool a_chercher = true;
  while ( a_chercher )
  {
    // On passe dans le répertoire parent

    CHDIR(nomrep);
    // On recherche un nom sous la forme Iabc, avec abc representant le numero
    int jaux;
    if      ( num <    100 ) { jaux = 2; }
    else if ( num <   1000 ) { jaux = 3; }
    else if ( num <  10000 ) { jaux = 4; }
    else if ( num < 100000 ) { jaux = 5; }
    else                     { jaux = 9; }
    std::ostringstream iaux;
    iaux << std::setw(jaux) << std::setfill('0') << num;
    std::ostringstream DirNameA;
    DirNameA << "I" << iaux.str();
    // Si on ne pas peut entrer dans le répertoire, on doit verifier
    // que c'est bien un probleme d'absence
    if ( CHDIR(DirNameA.str().c_str()) != 0 )
    {
      bool existe = false;
#ifndef WIN32
      DIR *dp;
      struct dirent *dirp;
      dp  = opendir(nomrep);
      while ( (dirp = readdir(dp)) != NULL )
      {
        std::string file_name(dirp->d_name);
#else
      HANDLE hFind = INVALID_HANDLE_VALUE;
      WIN32_FIND_DATA ffd;
      hFind = FindFirstFile(nomrep, &ffd);
      if (INVALID_HANDLE_VALUE != hFind) {
        while (FindNextFile(hFind, &ffd) != 0) {
         if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue; //skip directories
         std::string file_name(ffd.cFileName);
#endif
        if ( file_name == DirNameA.str() ) { existe = true; }
      }
#ifndef WIN32
      closedir(dp);
#else
      FindClose(hFind);
#endif
      if ( !existe )
      {
        DirName = DirNameA.str();
        a_chercher = false;
        break;
      }
    }
    num += 1;
  }

  MESSAGE ( "==> DirName = " << DirName);
  MESSAGE ( ". On retourne dans nomDirActuel = " << nomDirActuel );
  CHDIR(nomDirActuel.c_str());
  return CORBA::string_dup( DirName.c_str() );
}
//=============================================================================
// Calcul d'une iteration : gestion du répertoire de calcul
//        Si le sous-répertoire existe :
//         etatMenage =  0 : on sort en erreur si le répertoire n'est pas vide
//         etatMenage =  1 : on fait le menage du répertoire
//         etatMenage = -1 : on ne fait rien
//=============================================================================
char* HOMARD_Gen_i::ComputeDirManagement(SMESHHOMARD::HOMARD_Cas_var myCase, SMESHHOMARD::HOMARD_Iteration_var myIteration, CORBA::Long etatMenage)
{
  MESSAGE ( "ComputeDirManagement : répertoires pour le calcul" );
  // B.2. Le répertoire du cas
  const char* nomDirCase = myCase->GetDirName();
  MESSAGE ( ". nomDirCase = " << nomDirCase );

  // B.3. Le sous-répertoire de l'iteration a calculer, puis le répertoire complet a creer
  // B.3.1. Le nom du sous-répertoire
  const char* nomDirIt = myIteration->GetDirNameLoc();

  // B.3.2. Le nom complet du sous-répertoire
  std::stringstream DirCompute;
  DirCompute << nomDirCase << "/" << nomDirIt;
  MESSAGE (". DirCompute = " << DirCompute.str() );

  // B.3.3. Si le sous-répertoire n'existe pas, on le cree
  if (CHDIR(DirCompute.str().c_str()) != 0)
  {
#ifndef WIN32
    if (mkdir(DirCompute.str().c_str(), S_IRWXU|S_IRGRP|S_IXGRP) != 0)
#else
    if (_mkdir(DirCompute.str().c_str()) != 0)
#endif
    {
       // GERALD -- QMESSAGE BOX
       std::cerr << "Pb Creation du répertoire DirCompute = " << DirCompute.str() << std::endl;
       VERIFICATION("Pb a la creation du répertoire" == 0);
    }
  }
  else
  {
//  Le répertoire existe
//  On demande de faire le menage de son contenu :
    if (etatMenage == 1)
    {
      MESSAGE (". Menage du répertoire DirCompute = " << DirCompute.str());
      std::string commande = "rm -rf " + DirCompute.str()+"/*";
      int codret = system(commande.c_str());
      if (codret != 0)
      {
        // GERALD -- QMESSAGE BOX
        std::cerr << ". Menage du répertoire de calcul" << DirCompute.str() << std::endl;
        VERIFICATION("Pb au menage du répertoire de calcul" == 0);
      }
    }
//  On n'a pas demande de faire le menage de son contenu : on sort en erreur :
    else
    {
      if (etatMenage == 0)
      {
#ifndef WIN32
        DIR *dp;
        struct dirent *dirp;
        dp  = opendir(DirCompute.str().c_str());
        bool result = true;
        while ((dirp = readdir(dp)) != NULL && result )
        {
          std::string file_name(dirp->d_name);
          result = file_name.empty() || file_name == "." || file_name == ".."; //if any file - break and return false
        }
        closedir(dp);
#else
       HANDLE hFind = INVALID_HANDLE_VALUE;
       WIN32_FIND_DATA ffd;
       hFind = FindFirstFile(DirCompute.str().c_str(), &ffd);
       bool result = true;
       if (INVALID_HANDLE_VALUE != hFind) {
         while (FindNextFile(hFind, &ffd) != 0) {
          if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue; //skip directories
          std::string file_name(ffd.cFileName);
          result = file_name.empty() || file_name == "." || file_name == ".."; //if any file - break and return false
         }
       }
       FindClose(hFind);
#endif
        if ( result == false)
        {
          SALOME::ExceptionStruct es;
          es.type = SALOME::BAD_PARAM;
          std::string text = "Directory : " + DirCompute.str() + " is not empty";
          es.text = CORBA::string_dup(text.c_str());
          throw SALOME::SALOME_Exception(es);
          VERIFICATION("Directory is not empty" == 0);
        }
      }
    }
  }

  return CORBA::string_dup( DirCompute.str().c_str() );
}
//=============================================================================
// Calcul d'une iteration : gestion du répertoire de calcul de l'iteration parent
//=============================================================================
char* HOMARD_Gen_i::ComputeDirPaManagement(SMESHHOMARD::HOMARD_Cas_var myCase, SMESHHOMARD::HOMARD_Iteration_var myIteration)
{
  MESSAGE ( "ComputeDirPaManagement : répertoires pour le calcul" );
  // Le répertoire du cas
  const char* nomDirCase = myCase->GetDirName();
  MESSAGE ( ". nomDirCase = " << nomDirCase );

  // Le sous-répertoire de l'iteration precedente

  const char* nomIterationParent = myIteration->GetIterParentName();
  SMESHHOMARD::HOMARD_Iteration_var myIterationParent = myStudyContext._mesIterations[nomIterationParent];
  const char* nomDirItPa = myIterationParent->GetDirNameLoc();
  std::stringstream DirComputePa;
  DirComputePa << nomDirCase << "/" << nomDirItPa;
  MESSAGE( ". nomDirItPa = " << nomDirItPa);
  MESSAGE( ". DirComputePa = " << DirComputePa.str() );

  return CORBA::string_dup( DirComputePa.str().c_str() );
}
//=============================================================================
// Calcul d'une iteration : ecriture des champs dans le fichier de configuration
//=============================================================================
void HOMARD_Gen_i::DriverTexteField(SMESHHOMARD::HOMARD_Iteration_var myIteration, SMESHHOMARD::HOMARD_Hypothesis_var myHypo, SMESHHOMARDImpl::HomardDriver* myDriver)
{
  MESSAGE ( "... DriverTexteField" );
//  Le fichier du champ
  char* FieldFile = myIteration->GetFieldFile();
  MESSAGE ( ". FieldFile = " << FieldFile );
  if (strlen(FieldFile) == 0)
  {
    // GERALD -- QMESSAGE BOX
    std::cerr << "Le fichier du champ n'a pas ete fourni." << std::endl;
    VERIFICATION("The file for the field is not given." == 0);
  }
//  Les caracteristiques d'instants du champ de pilotage
  int TimeStep = myIteration->GetTimeStep();
  MESSAGE( ". TimeStep = " << TimeStep );
  int Rank = myIteration->GetRank();
  MESSAGE( ". Rank = " << Rank );
//  Les informations sur les champs
  SMESHHOMARD::InfosHypo* aInfosHypo = myHypo->GetField();
//  Le nom
  const char* FieldName = aInfosHypo->FieldName;
//  Les seuils
  int TypeThR = aInfosHypo->TypeThR;
  double ThreshR = aInfosHypo->ThreshR;
  int TypeThC = aInfosHypo->TypeThC;
  double ThreshC = aInfosHypo->ThreshC;
//  Saut entre mailles ou non ?
  int UsField = aInfosHypo->UsField;
  MESSAGE( ". UsField = " << UsField );
//  L'usage des composantes
  int UsCmpI = aInfosHypo->UsCmpI;
  MESSAGE( ". UsCmpI = " << UsCmpI );
//
  myDriver->TexteField(FieldName, FieldFile, TimeStep, Rank, TypeThR, ThreshR, TypeThC, ThreshC, UsField, UsCmpI);
//
//  Les composantes
  SMESHHOMARD::listeComposantsHypo* mescompo = myHypo->GetComps();
  int numberOfCompos = mescompo->length();
  MESSAGE( ". numberOfCompos = " << numberOfCompos );
  for (int NumeComp = 0; NumeComp< numberOfCompos; NumeComp++)
  {
    std::string nomCompo = std::string((*mescompo)[NumeComp]);
    MESSAGE( "... nomCompo = " << nomCompo );
    myDriver->TexteCompo(NumeComp, nomCompo);
  }
}
//=============================================================================
// Calcul d'une iteration : ecriture des frontieres dans le fichier de configuration
// On ecrit dans l'ordre :
//    1. la definition des frontieres
//    2. les liens avec les groupes
//    3. un entier resumant le type de comportement pour les frontieres
//=============================================================================
int HOMARD_Gen_i::DriverTexteBoundary(SMESHHOMARD::HOMARD_Cas_var myCase, SMESHHOMARDImpl::HomardDriver* myDriver)
{
  MESSAGE ( "... DriverTexteBoundary" );
  // 1. Recuperation des frontieres
  std::list<std::string>  ListeBoundaryTraitees;
  SMESHHOMARD::ListBoundaryGroupType* ListBoundaryGroupType = myCase->GetBoundaryGroup();
  int numberOfitems = ListBoundaryGroupType->length();
  MESSAGE ( "... number of string for Boundary+Group = " << numberOfitems);
  int BoundaryOption = 1;
  // 2. Parcours des frontieres pour ecrire leur description
  int NumBoundaryAnalytical = 0;
  for (int NumBoundary = 0; NumBoundary< numberOfitems; NumBoundary=NumBoundary+2)
  {
    std::string BoundaryName = std::string((*ListBoundaryGroupType)[NumBoundary]);
    MESSAGE ( "... BoundaryName = " << BoundaryName);
    // 2.1. La frontiere a-t-elle deja ete ecrite ?
    //      Cela arrive quand elle est liee a plusieurs groupes. Il ne faut l'ecrire que la premiere fois
    int A_faire = 1;
    std::list<std::string>::const_iterator it = ListeBoundaryTraitees.begin();
    while (it != ListeBoundaryTraitees.end())
    {
      MESSAGE ( "..... BoundaryNameTraitee = " << *it);
      if ( BoundaryName == *it ) { A_faire = 0; }
      it++;
    }
    // 2.2. Ecriture de la frontiere
    if ( A_faire == 1 )
    {
      // 2.2.1. Caracteristiques de la frontiere
      SMESHHOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
      ASSERT(!CORBA::is_nil(myBoundary));
      int BoundaryType = myBoundary->GetType();
      MESSAGE ( "... BoundaryType = " << BoundaryType );
      // 2.2.2. Ecriture selon le type
      // 2.2.2.1. Cas d une frontiere CAO
      if (BoundaryType == -1)
      {
//         const char* CAOFile = myBoundary->GetDataFile();
//         MESSAGE ( ". CAOFile = " << CAOFile );
        if ( BoundaryOption % 5 != 0 ) { BoundaryOption = BoundaryOption*5; }
      }
      // 2.2.2.2. Cas d une frontiere discrete
      else if (BoundaryType == 0)
      {
        const char* MeshName = myBoundary->GetMeshName();
        MESSAGE ( ". MeshName = " << MeshName );
        const char* MeshFile = myBoundary->GetDataFile();
        MESSAGE ( ". MeshFile = " << MeshFile );
        myDriver->TexteBoundaryDi( MeshName, MeshFile);
        if ( BoundaryOption % 2 != 0 ) { BoundaryOption = BoundaryOption*2; }
      }
      // 2.2.2.3. Cas d une frontiere analytique
      else
      {
        NumBoundaryAnalytical++;
        SMESHHOMARD::double_array* coor = myBoundary->GetCoords();
        if (BoundaryType == 1) // Cas d un cylindre
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], 0.);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3; }
        }
        else if (BoundaryType == 2) // Cas d une sphere
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], 0., 0., 0., 0.);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3; }
        }
        else if (BoundaryType == 3) // Cas d un cone defini par un axe et un angle
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], 0.);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3; }
        }
        else if (BoundaryType == 4) // Cas d un cone defini par les 2 rayons
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], (*coor)[7]);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3; }
        }
        else if (BoundaryType == 5) // Cas d un tore
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], (*coor)[7]);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3; }
        }
      }
      // 2.2.3. Memorisation du traitement
      ListeBoundaryTraitees.push_back( BoundaryName );
    }
  }
  // 3. Parcours des frontieres pour ecrire les liens avec les groupes
  NumBoundaryAnalytical = 0;
  for (int NumBoundary = 0; NumBoundary< numberOfitems; NumBoundary=NumBoundary+2)
  {
    std::string BoundaryName = std::string((*ListBoundaryGroupType)[NumBoundary]);
    MESSAGE ( "... BoundaryName = " << BoundaryName);
    SMESHHOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
    ASSERT(!CORBA::is_nil(myBoundary));
    int BoundaryType = myBoundary->GetType();
    MESSAGE ( "... BoundaryType = " << BoundaryType );
    // 3.1. Recuperation du nom du groupe
    std::string GroupName = std::string((*ListBoundaryGroupType)[NumBoundary+1]);
    MESSAGE ( "... GroupName = " << GroupName);
    // 3.2. Cas d une frontiere CAO
    if ( BoundaryType == -1 )
    {
      if ( GroupName.size() > 0 ) { myDriver->TexteBoundaryCAOGr ( GroupName ); }
    }
    // 3.3. Cas d une frontiere discrete
    else if ( BoundaryType == 0 )
    {
      if ( GroupName.size() > 0 ) { myDriver->TexteBoundaryDiGr ( GroupName ); }
    }
    // 3.4. Cas d une frontiere analytique
    else
    {
      NumBoundaryAnalytical++;
      myDriver->TexteBoundaryAnGr ( BoundaryName, NumBoundaryAnalytical, GroupName );
    }
  }
  // 4. Ecriture de l'option finale
  myDriver->TexteBoundaryOption(BoundaryOption);
//
  return BoundaryOption;
}
//=============================================================================
// Calcul d'une iteration : ecriture des interpolations dans le fichier de configuration
//=============================================================================
void HOMARD_Gen_i::DriverTexteFieldInterp(SMESHHOMARD::HOMARD_Iteration_var myIteration, SMESHHOMARD::HOMARD_Hypothesis_var myHypo, SMESHHOMARDImpl::HomardDriver* myDriver)
{
  MESSAGE ( "... DriverTexteFieldInterp" );
  int TypeFieldInterp = myHypo->GetTypeFieldInterp();
  MESSAGE ( "... TypeFieldInterp = " << TypeFieldInterp);
  if (TypeFieldInterp != 0)
  {
//  Le fichier des champs
    char* FieldFile = myIteration->GetFieldFile();
    MESSAGE ( ". FieldFile = " << FieldFile );
    if (strlen(FieldFile) == 0)
    {
      // GERALD -- QMESSAGE BOX
      VERIFICATION("The file for the field is not given." == 0);
    }
  //
    const char* MeshFile = myIteration->GetMeshFile();
    myDriver->TexteFieldInterp(FieldFile, MeshFile);

  // Les champs
  // Interpolation de tous les champs
    if ( TypeFieldInterp == 1 )
    {
      myDriver->TexteFieldInterpAll();
    }
  // Interpolation de certains champs
    else if (TypeFieldInterp == 2)
    {
      // Les champs et leurs instants pour l'iteration
      SMESHHOMARD::listeFieldInterpTSRsIter* ListFieldTSR = myIteration->GetFieldInterpsTimeStepRank();
      int numberOfFieldsx3 = ListFieldTSR->length();
      MESSAGE( ". pour iteration, numberOfFields = " << numberOfFieldsx3/3 );
      // Les champs pour l'hypothese
      SMESHHOMARD::listeFieldInterpsHypo* ListField = myHypo->GetFieldInterps();
      int numberOfFieldsx2 = ListField->length();
      MESSAGE( ". pour hypothese, numberOfFields = " << numberOfFieldsx2/2 );
      // On parcourt tous les champs de  l'hypothese
      int NumField = 0;
      for (int iaux = 0; iaux< numberOfFieldsx2; iaux++)
      {
        // Le nom du champ
        std::string FieldName = std::string((*ListField)[iaux]);
        // Le type d'interpolation
        std::string TypeInterpstr = std::string((*ListField)[iaux+1]);
        MESSAGE( "... FieldName = " << FieldName << ", TypeInterp = " << TypeInterpstr );
        // On cherche ?? savoir si des instants ont ??t?? pr??cis??s pour cette it??ration
        int tsrvu = 0;
        for (int jaux = 0; jaux< numberOfFieldsx3; jaux++)
        {
        // Le nom du champ
          std::string FieldName2 = std::string((*ListFieldTSR)[jaux]);
          MESSAGE( "..... FieldName2 = " << FieldName2 );
        // Quand c'est le bon champ, on ecrit le pas de temps
          if ( FieldName == FieldName2 )
          {
            tsrvu = 1;
            // Le pas de temps
            std::string TimeStepstr = std::string((*ListFieldTSR)[jaux+1]);
            // Le numero d'ordre
            std::string Rankstr = std::string((*ListFieldTSR)[jaux+2]);
            MESSAGE( "..... TimeStepstr = " << TimeStepstr <<", Rankstr = "<<Rankstr );
            NumField += 1;
            int TimeStep = atoi( TimeStepstr.c_str() );
            int Rank = atoi( Rankstr.c_str() );
            myDriver->TexteFieldInterpNameType(NumField, FieldName, TypeInterpstr, TimeStep, Rank);
          }
          jaux += 2;
        }
        // Si aucun instant n'a été défini
        if ( tsrvu == 0 )
        {
          NumField += 1;
          myDriver->TexteFieldInterpNameType(NumField, FieldName, TypeInterpstr, -1, -1);
        }
        iaux++;
      }
    }
  }
}
//===========================================================================
//===========================================================================


//===========================================================================
//===========================================================================
// Publications
//===========================================================================
//=====================================================================================
void HOMARD_Gen_i::PublishResultInSmesh(const char* NomFich, CORBA::Long Option)
//  Option = 0 : fichier issu d'une importation
//  Option = 1 : fichier issu d'une execution HOMARD
{
  MESSAGE( "PublishResultInSmesh " << NomFich << ", avec Option = " << Option);
  if (CORBA::is_nil(SMESH_Gen_i::GetSMESHGen()->getStudyServant())) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid study context";
    throw SALOME::SALOME_Exception(es);
  }

  // Le module SMESH est-il actif ?
  SALOMEDS::SObject_var aSmeshSO =
    SMESH_Gen_i::GetSMESHGen()->getStudyServant()->FindComponent("SMESH");
  //
  if (!CORBA::is_nil(aSmeshSO)) {
    // On verifie que le fichier n est pas deja publie
    SALOMEDS::ChildIterator_var aIter =
      SMESH_Gen_i::GetSMESHGen()->getStudyServant()->NewChildIterator(aSmeshSO);
    for (; aIter->More(); aIter->Next()) {
      SALOMEDS::SObject_var aSO = aIter->Value();
      SALOMEDS::GenericAttribute_var aGAttr;
      if (aSO->FindAttribute(aGAttr, "AttributeExternalFileDef")) {
        SALOMEDS::AttributeExternalFileDef_var anAttr =
          SALOMEDS::AttributeExternalFileDef::_narrow(aGAttr);
        CORBA::String_var value = anAttr->Value();
        if (strcmp((const char*)value, NomFich) == 0) {
          MESSAGE ( "PublishResultInSmesh : le fichier " << NomFich << " est deja publie." );
          // Pour un fichier importe, on ne republie pas
          if ( Option == 0 ) { return; }
          // Pour un fichier calcule, on commence par faire la depublication
          else {
            MESSAGE("PublishResultInSmesh : depublication");
            if (aSO->FindAttribute(aGAttr, "AttributeName")) {
              SALOMEDS::AttributeName_var anAttr2 = SALOMEDS::AttributeName::_narrow(aGAttr);
              CORBA::String_var value2 = anAttr2->Value();
              std::string MeshName = string(value2);
              MESSAGE("PublishResultInSmesh : depublication de " << MeshName);
              DeleteResultInSmesh(NomFich, MeshName);
            }
          }
        }
      }
    }
  }

  // On enregistre le fichier
  MESSAGE( "Enregistrement du fichier");
  //
  //SMESH::SMESH_Gen_var aSmeshEngine = this->retrieveSMESHInst();
  SMESH_Gen_i* aSmeshEngine = SMESH_Gen_i::GetSMESHGen();
  //
  //ASSERT(!CORBA::is_nil(aSmeshEngine));
  aSmeshEngine->UpdateStudy();
  SMESH::DriverMED_ReadStatus theStatus;

  // On met a jour les attributs AttributeExternalFileDef et AttributePixMap
  SMESH::mesh_array* mesMaillages = aSmeshEngine->CreateMeshesFromMED(NomFich, theStatus);
  for (int i = 0; i < mesMaillages->length(); i++) {
    MESSAGE(". Mise a jour des attributs du maillage");
    SMESH::SMESH_Mesh_var monMaillage = (*mesMaillages)[i];
    SALOMEDS::SObject_var aSO = SALOMEDS::SObject::_narrow
      (SMESH_Gen_i::GetSMESHGen()->getStudyServant()->FindObjectIOR
       (SMESH_Gen_i::GetORB()->object_to_string(monMaillage)));
    SALOMEDS::StudyBuilder_var aStudyBuilder =
      SMESH_Gen_i::GetSMESHGen()->getStudyServant()->NewBuilder();
    SALOMEDS::GenericAttribute_var aGAttr =
      aStudyBuilder->FindOrCreateAttribute(aSO, "AttributeExternalFileDef");
    SALOMEDS::AttributeExternalFileDef_var anAttr =
      SALOMEDS::AttributeExternalFileDef::_narrow(aGAttr);
    anAttr->SetValue(NomFich);
    SALOMEDS::GenericAttribute_var aPixMap =
      aStudyBuilder->FindOrCreateAttribute(aSO, "AttributePixMap" );
    SALOMEDS::AttributePixMap_var anAttr2 = SALOMEDS::AttributePixMap::_narrow(aPixMap);
    const char* icone;
    if ( Option == 0 ) { icone = "mesh_tree_importedmesh.png"; }
    else               { icone = "mesh_tree_mesh.png"; }
    anAttr2->SetPixMap( icone );
  }
}
//=============================================================================
void HOMARD_Gen_i::DeleteResultInSmesh(std::string NomFich, std::string MeshName)
{
  MESSAGE ("DeleteResultInSmesh pour le maillage " << MeshName << " dans le fichier " << NomFich );
  if (CORBA::is_nil(SMESH_Gen_i::GetSMESHGen()->getStudyServant())) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid study context";
    throw SALOME::SALOME_Exception(es);
  }

  // Le module SMESH est-il actif ?
  SALOMEDS::SObject_var aSmeshSO =
    SMESH_Gen_i::GetSMESHGen()->getStudyServant()->FindComponent("SMESH");
  //
  if (CORBA::is_nil(aSmeshSO)) return;
  // On verifie que le fichier est deja publie
  SALOMEDS::StudyBuilder_var myBuilder =
    SMESH_Gen_i::GetSMESHGen()->getStudyServant()->NewBuilder();
  SALOMEDS::ChildIterator_var aIter =
    SMESH_Gen_i::GetSMESHGen()->getStudyServant()->NewChildIterator(aSmeshSO);
  for (; aIter->More(); aIter->Next()) {
    SALOMEDS::SObject_var  aSO = aIter->Value();
    SALOMEDS::GenericAttribute_var aGAttr;
    if (aSO->FindAttribute(aGAttr, "AttributeExternalFileDef")) {
      SALOMEDS::AttributeExternalFileDef_var anAttr =
        SALOMEDS::AttributeExternalFileDef::_narrow(aGAttr);
      CORBA::String_var value = anAttr->Value();
      if (strcmp((const char*)value, NomFich.c_str()) == 0) {
        if (aSO->FindAttribute(aGAttr, "AttributeName")) {
          SALOMEDS::AttributeName_var anAttr2 = SALOMEDS::AttributeName::_narrow(aGAttr);
          CORBA::String_var value2 = anAttr2->Value();
          if (strcmp((const char*)value2, MeshName.c_str()) == 0) {
            myBuilder->RemoveObjectWithChildren( aSO );
          }
        }
      }
    }
  }
}
//=============================================================================
void HOMARD_Gen_i::PublishMeshIterInSmesh(const char* NomIter)
{
  MESSAGE( "PublishMeshIterInSmesh " << NomIter);
  SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[NomIter];

  SALOMEDS::SObject_var aIterSO = SALOMEDS::SObject::_narrow
    (SMESH_Gen_i::GetSMESHGen()->getStudyServant()->FindObjectIOR
     (SMESH_Gen_i::GetORB()->object_to_string(myIteration)));
  if (CORBA::is_nil(myIteration)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid iterationStudy Object";
    throw SALOME::SALOME_Exception(es);
  }
  const char* MeshFile = myIteration->GetMeshFile();
  const char* MeshName = myIteration->GetMeshName();
  CORBA::Long Option = -1;
  int etat = myIteration->GetState();
// Iteration initiale
  if ( etat <= 0 )      { Option = 0; }
// ou iteration calculee
  else if ( etat == 2 ) { Option = 1; }
// Publication effective apres menage eventuel
  if ( Option >= 0 )
  {
    DeleteResultInSmesh(MeshFile, MeshName);
    PublishResultInSmesh(MeshFile, Option);
  }

}

/*
Engines::TMPFile* HOMARD_Gen_i::DumpPython(CORBA::Boolean isPublished,
                                           CORBA::Boolean isMultiFile,
                                           CORBA::Boolean& isValidScript)
{
   MESSAGE ("Entree dans DumpPython");
   isValidScript=1;
   std::string aScript = "\"\"\"\n";

   SALOMEDS::SObject_var aSO = myStudy->FindComponent("HOMARD");
   if(CORBA::is_nil(aSO))
      return new Engines::TMPFile(0);

   aScript += "Python script for HOMARD\n";
   aScript += "Copyright 1996, 2011, 2015 EDF\n";
   aScript += "\"\"\"\n";
   aScript += "__revision__ = \"V1.2\"\n";
   aScript += "import HOMARD\n";
   if( isMultiFile )
      aScript += "import salome\n";
   aScript += "homard = salome.lcc.FindOrLoadComponent('FactoryServer','HOMARD')\n";
   aScript += "\thomard.UpdateStudy()\n";
   MESSAGE (". Au depart \n"<<aScript);


   if (myStudyContext._mesBoundarys.size() > 0)
   {
    MESSAGE (". Ecritures des frontieres");
    aScript += "#\n# Creation of the boundaries";
    aScript +=  "\n# ==========================";
   }
   std::map<std::string, SMESHHOMARD::HOMARD_Boundary_var>::const_iterator it_boundary;
   for (it_boundary  = myStudyContext._mesBoundarys.begin();
        it_boundary != myStudyContext._mesBoundarys.end(); ++it_boundary)
   {
    SMESHHOMARD::HOMARD_Boundary_var maBoundary = (*it_boundary).second;
    CORBA::String_var dumpCorbaBoundary = maBoundary->GetDumpPython();
    std::string dumpBoundary = dumpCorbaBoundary.in();
    MESSAGE (dumpBoundary<<"\n");
    aScript += dumpBoundary;
   }

   if (myStudyContext._mesHypotheses.size() > 0)
   {
    MESSAGE (". Ecritures des hypotheses");
    aScript += "#\n# Creation of the hypotheses";
    aScript +=  "\n# ==========================";
   }
   std::map<std::string, SMESHHOMARD::HOMARD_Hypothesis_var>::const_iterator it_hypo;
   for ( it_hypo  = myStudyContext._mesHypotheses.begin();
         it_hypo != myStudyContext._mesHypotheses.end(); it_hypo++)
   {
    SMESHHOMARD::HOMARD_Hypothesis_var monHypo = (*it_hypo).second;
    CORBA::String_var dumpCorbaHypo = monHypo->GetDumpPython();
    std::string dumpHypo = dumpCorbaHypo.in();
    MESSAGE (dumpHypo<<"\n");
    aScript += dumpHypo;
   }

   if (myStudyContext._mesCas.size() > 0)
   {
    MESSAGE (". Ecritures des cas");
    aScript += "#\n# Creation of the cases";
    aScript += "\n# =====================";
   }
   std::map<std::string, SMESHHOMARD::HOMARD_Cas_var>::const_iterator it_cas;
   for (it_cas  = myStudyContext._mesCas.begin();
        it_cas != myStudyContext._mesCas.end(); it_cas++)
        {
           std::string nomCas = (*it_cas).first;
           std::string dumpCas = std::string("\n# Creation of the case ");
           dumpCas +=  nomCas + std::string("\n");
           dumpCas += std::string("\t") + nomCas;
           dumpCas += std::string(" = homard.CreateCase(\"") + nomCas + std::string("\", \"");

           SMESHHOMARD::HOMARD_Cas_var myCase = (*it_cas).second;
           CORBA::String_var cIter0= myCase->GetIter0Name();
           std::string iter0 = cIter0.in();

           SMESHHOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[iter0];
           CORBA::String_var cMesh0= myIteration->GetMeshFile();
           std::string mesh0 = cMesh0.in();
           CORBA::String_var cMeshName0= myIteration->GetMeshName();
           std::string meshName0 = cMeshName0.in();
           dumpCas += meshName0 + std::string("\", \"")+ mesh0 + std::string("\")\n");
           CORBA::String_var dumpCorbaCase = myCase->GetDumpPython();
           std::string dumpCas2= dumpCorbaCase.in();

           MESSAGE (dumpCas<<dumpCas2<<"\n");
           aScript += dumpCas + dumpCas2;
        };


   if (myStudyContext._mesIterations.size() > 0)
   {
    MESSAGE (". Ecritures des iterations");
    aScript += "#\n# Creation of the iterations";
    aScript += "\n# ==========================";
   }
   std::map<std::string, SMESHHOMARD::HOMARD_Iteration_var>::const_iterator it_iter;
   for (it_iter  = myStudyContext._mesIterations.begin();
        it_iter != myStudyContext._mesIterations.end(); ++it_iter)
   {
    SMESHHOMARD::HOMARD_Iteration_var aIter = (*it_iter).second;
    CORBA::String_var dumpCorbaIter = aIter->GetDumpPython();
    std::string dumpIter = dumpCorbaIter.in();
    MESSAGE (dumpIter<<"\n");
    aScript += dumpIter;
   }

  MESSAGE (". Ecritures finales");
  if( isMultiFile )
    aScript += "\n\tpass";
  aScript += "\n";

  if( !isMultiFile ) // remove unnecessary tabulation
    aScript = RemoveTabulation( aScript );

  const size_t aLen = strlen(aScript.c_str());
  char* aBuffer = new char[aLen+1];
  strcpy(aBuffer, aScript.c_str());

  CORBA::Octet* anOctetBuf =  (CORBA::Octet*)aBuffer;
  Engines::TMPFile_var aStreamFile = new Engines::TMPFile(aLen+1, aLen+1, anOctetBuf, 1);

  MESSAGE ("Sortie de DumpPython");
  return aStreamFile._retn();
}
*/

//=============================================================================
//=============================================================================
// Utilitaires
//=============================================================================
//=============================================================================
char* HOMARD_Gen_i::VerifieDir(const char* nomDir)
{
  MESSAGE("HOMARD_Gen_i::VerifieDir");
  std::string casename = std::string("");
  std::map<std::string, SMESHHOMARD::HOMARD_Cas_var>::const_iterator it;
  for (it = myStudyContext._mesCas.begin();
  it != myStudyContext._mesCas.end(); it++)
  {
   if (std::string(nomDir) == std::string(it->second->GetDirName()))
   {
     casename = std::string(it->second->GetName());
     break;
   }
  }
  return CORBA::string_dup( casename.c_str() );
}
//===============================================================================
// Recuperation de la chaine de caracteres par rapport a l'apparition d'un caractere
// ligne : la ligne a manipuler
// caractere : le caractere a reperer
// option : 0 : la chaine avant la premiere apparition du caractere
//          1 : la chaine apres la premiere apparition du caractere
//          2 : la chaine avant la derniere apparition du caractere
//          3 : la chaine apres la derniere apparition du caractere
// Si le caractere est absent, on retourne la chaine totale
//===============================================================================
std::string HOMARD_Gen_i::GetStringInTexte( const std::string ligne, const std::string caractere, int option )
{
//   MESSAGE("GetStringInTexte, recherche de '"<<caractere<<"' dans '"<<ligne<<"'"<<", option = "<<option);
//
  std::string chaine = ligne;
  int position;
  if ( option < 2 ) { position = ligne.find_first_of( caractere ); }
  else              { position = ligne.find_last_of( caractere ); }
//   MESSAGE("position = "<<position);
//   MESSAGE("a = "<<ligne.substr( 0, position ).c_str());
//   MESSAGE("b = "<<ligne.substr( position+1 ).c_str());
//
  if ( position != std::string::npos )
  {
    if ( ( option == 0 ) || ( option == 2 ) ) { chaine = ligne.substr( 0, position ); }
    else                                      { chaine = ligne.substr( position+1 ); }
  }
  return chaine;
//
}
//=============================================================================
//=============================================================================
// Gestion des preferences
//=============================================================================
//=============================================================================
// Decodage du fichier d'arcihvage des preferences
//
void HOMARD_Gen_i::SetPreferences( )
{
  MESSAGE ( "SetPreferences" );

  std::string ligne, mot_cle, salome_version;
  bool ok = true;

  // A. Les valeurs par defaut; elles doivent etre coherentes
  std::string LanguageShort = "en";
  int PublisMeshIN = 0;
  int PublisMeshOUT = 0;

  // B. La version de salome
  // Cela se presente sous la forme :
  // [SALOME KERNEL] : 7.3.0
  std::string File;
  File  = getenv("KERNEL_ROOT_DIR");
  File += "/bin/salome/VERSION";
  MESSAGE ( "File = "<<File );
  std::ifstream fichier0( File.c_str() );
  if ( fichier0 ) // ce test échoue si le fichier n'est pas ouvert
  {
    std::string ligne; // variable contenant chaque ligne lue
    while ( std::getline( fichier0, ligne ) )
    {
      std::istringstream ligne_bis(ligne); // variable contenant chaque ligne sous forme de flux
      ligne_bis >> mot_cle;
      if ( mot_cle == "[SALOME" )
      {
        salome_version = GetStringInTexte ( ligne, " ", 3 );
//         MESSAGE ( "salome_version = "<<salome_version<<"|||");
        break;
      }
    }
  }
  else { ok = false; }

  // B. Decodage du fichier de preferences
  if ( ok )
  {
    std::string PrefFile;
    PrefFile  = Kernel_Utils::HomePath();
    PrefFile += "/.config/salome/SalomeApprc." + salome_version;
    MESSAGE ( "PrefFile = "<<PrefFile );

    std::ifstream fichier( PrefFile.c_str() );
    if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
    {
      bool section_langue = false;
      bool section_homard = false;
      while ( std::getline( fichier, ligne ) )
      {
        std::string chaine;
        // 1. Pour la ligne courante, on identifie le premier mot : le mot-cle eventuel
        std::istringstream ligne_bis(ligne); // variable contenant chaque ligne sous forme de flux
        ligne_bis >> mot_cle;

        // 2. Les sections
        // 2.1. Debut d'une section
  //       MESSAGE(mot_cle);
        if ( mot_cle == "<section" )
        { /*MESSAGE ( "Debut de la section : "<< ligne);*/
          ligne_bis >> mot_cle;
          chaine = GetStringInTexte ( mot_cle, "\"", 1 );
          chaine = GetStringInTexte ( chaine,  "\"", 0 );
          if ( chaine == "language" ) { section_langue = true; }
          if ( chaine == "HOMARD" )   { section_homard = true; }
  //         MESSAGE ( "section_langue = "<<section_langue<<", section_homard = "<<section_homard);
        }
        // 2.2. Fin d'une section
        else if ( mot_cle == "</section>" )
        { /*MESSAGE ( "Fin de la section : "<< ligne<<", section_langue = "<<section_langue<<", section_homard = "<<section_homard);*/
          section_langue = false;
          section_homard = false; }

        // 3. Parametres
        // 3.1. La langue
        else if ( section_langue || section_homard )
        { MESSAGE ( "a decoder : "<< ligne);
//        La valeur : entre les deux premieres quotes
          chaine = GetStringInTexte ( ligne, "\"", 1 );
//           MESSAGE("chaine 1 = |"<<chaine<<"|");
          chaine = GetStringInTexte ( chaine,  "\"", 0 );
//           MESSAGE("chaine = |"<<chaine<<"|");
//        Le mot_cle : entre les deux dernieres quotes
          std::string chaine2 = GetStringInTexte ( ligne, "\"", 2 );
//           MESSAGE("chaine2 1 = |"<<chaine2<<"|");
          chaine2 = GetStringInTexte ( chaine2,  "\"", 3 );
//           MESSAGE("chaine2 = |"<<chaine2<<"|");
          // 3.1. La langue
          if ( section_langue )
          { if ( chaine2 == "language" ) { LanguageShort = chaine; } }
          // 3.2. HOMARD
          if ( section_homard )
          {
            std::istringstream chainebis( chaine );
            // 3.2.1. Les publications
            if ( chaine2 == "publish_mesh_in" )  { chainebis >> PublisMeshIN; }
            if ( chaine2 == "publish_mesh_out" ) { chainebis >> PublisMeshOUT; }
          }
        }
      }
    }
  }

  // C. Enregistrements
  MESSAGE ("Enregistrement de LanguageShort = " << LanguageShort );
  SetLanguageShort( LanguageShort.c_str() );

  MESSAGE ("Enregistrement de PublisMeshIN = " << PublisMeshIN<<", PublisMeshOUT = "<< PublisMeshOUT);
  SetPublisMesh(PublisMeshIN, PublisMeshOUT);
}
//===============================================================================
// Langue de SALOME
//===============================================================================
void HOMARD_Gen_i::SetLanguageShort(const char* LanguageShort)
{
  //MESSAGE ("SetLanguageShort pour LanguageShort = " << LanguageShort );
  _LangueShort = LanguageShort;
  if ( _LangueShort == "fr" ) { _Langue = "Francais"; }
  else                        { _Langue = "English"; }
}
char* HOMARD_Gen_i::GetLanguageShort()
{
  //MESSAGE ("GetLanguageShort");
  return CORBA::string_dup( _LangueShort.c_str() );
}
//===============================================================================
// Options de publications
//===============================================================================
void HOMARD_Gen_i::SetPublisMesh(CORBA::Long PublisMeshIN, CORBA::Long PublisMeshOUT)
{
  _PublisMeshIN  = PublisMeshIN;
  _PublisMeshOUT = PublisMeshOUT;
}
CORBA::Long HOMARD_Gen_i::GetPublisMeshIN()
{
  return _PublisMeshIN;
}
CORBA::Long HOMARD_Gen_i::GetPublisMeshOUT()
{
  return _PublisMeshOUT;
}

// =======================================================================
int MEDFileExist( const char * aFile )
// Retourne 1 si le fichier existe, 0 sinon
// =======================================================================
{
  int existe;
  med_idt medIdt = MEDfileOpen(aFile,MED_ACC_RDONLY);
  if ( medIdt < 0 ) { existe = 0; }
  else              { MEDfileClose(medIdt);
                      existe = 1; }
  return existe;
}
// =======================================================================
std::set<std::string> GetListeGroupesInMedFile(const char * aFile)
// =======================================================================
{
  std::set<std::string> ListeGroupes;
  med_err erreur = 0;
  med_idt medIdt;
  while ( erreur == 0 )
  {
    //  Ouverture du fichier
    medIdt = MEDfileOpen(aFile,MED_ACC_RDONLY);
    if ( medIdt < 0 )
    {
      erreur = 1;
      break;
    }
    // Caracteristiques du maillage
    char meshname[MED_NAME_SIZE+1];
    med_int spacedim,meshdim;
    med_mesh_type meshtype;
    char descriptionription[MED_COMMENT_SIZE+1];
    char dtunit[MED_SNAME_SIZE+1];
    med_sorting_type sortingtype;
    med_int nstep;
    med_axis_type axistype;
    int naxis = MEDmeshnAxis(medIdt,1);
    char *axisname=new char[naxis*MED_SNAME_SIZE+1];
    char *axisunit=new char[naxis*MED_SNAME_SIZE+1];
    erreur = MEDmeshInfo(medIdt,
                            1,
                            meshname,
                            &spacedim,
                            &meshdim,
                            &meshtype,
                            descriptionription,
                            dtunit,
                            &sortingtype,
                            &nstep,
                            &axistype,
                            axisname,
                            axisunit);
    delete[] axisname;
    delete[] axisunit;
    if ( erreur < 0 ) { break; }
    // Nombre de familles
    med_int nfam;
    nfam = MEDnFamily(medIdt,meshname);
    if ( nfam < 0 )
    {
      erreur = 2;
      break;
    }
  // Lecture des caracteristiques des familles
    for (int i=0;i<nfam;i++)
    {
//       Lecture du nombre de groupes
      med_int ngro = MEDnFamilyGroup(medIdt,meshname,i+1);
      if ( ngro < 0 )
      {
        erreur = 3;
        break;
      }
//       Lecture de la famille
      else if ( ngro > 0 )
      {
        char familyname[MED_NAME_SIZE+1];
        med_int numfam;
        char* gro = (char*) malloc(MED_LNAME_SIZE*ngro+1);
        erreur = MEDfamilyInfo(medIdt,
                               meshname,
                               i+1,
                               familyname,
                               &numfam,
                               gro);
        if ( erreur < 0 )
        {
          free(gro);
          break;
        }
        // Lecture des groupes pour une famille de mailles
        if ( numfam < 0)
        {
          for (int j=0;j<ngro;j++)
          {
            char str2[MED_LNAME_SIZE+1];
            strncpy(str2,gro+j*MED_LNAME_SIZE,MED_LNAME_SIZE);
            str2[MED_LNAME_SIZE] = '\0';
            ListeGroupes.insert(std::string(str2));
          }
        }
        free(gro);
      }
    }
    break;
  }
  // Fermeture du fichier
  if ( medIdt > 0 ) MEDfileClose(medIdt);

  return ListeGroupes;
}

// =======================================================================
std::vector<double> GetBoundingBoxInMedFile(const char * aFile)
// =======================================================================
{
// Le vecteur en retour contiendra les informations suivantes :
// en position 0 et 1 Xmin, Xmax et en position 2 Dx si < 0  2D
// en position 3 et 4 Ymin, Ymax et en position 5 Dy si < 0  2D
// en position 6 et 7 Zmin, Zmax et en position 8 Dz si < 0  2D
//  9 distance max dans le maillage

  std::vector<double> LesExtremes;
  med_err erreur = 0;
  med_idt medIdt;
  while ( erreur == 0 )
  {
    //  Ouverture du fichier
    medIdt = MEDfileOpen(aFile,MED_ACC_RDONLY);
    if ( medIdt < 0 )
    {
      erreur = 1;
      break;
    }
    //Nombre de maillage : on ne peut en lire qu'un seul
    med_int numberOfMeshes = MEDnMesh(medIdt);
    if (numberOfMeshes != 1 )
    {
      erreur = 2;
      break;
    }
    // Caracteristiques du maillage
    char meshname[MED_NAME_SIZE+1];
    med_int spacedim,meshdim;
    med_mesh_type meshtype;
    char descriptionription[MED_COMMENT_SIZE+1];
    char dtunit[MED_SNAME_SIZE+1];
    med_sorting_type sortingtype;
    med_int nstep;
    med_axis_type axistype;
    int naxis = MEDmeshnAxis(medIdt,1);
    char *axisname=new char[naxis*MED_SNAME_SIZE+1];
    char *axisunit=new char[naxis*MED_SNAME_SIZE+1];
    erreur = MEDmeshInfo(medIdt,
                            1,
                            meshname,
                            &spacedim,
                            &meshdim,
                            &meshtype,
                            descriptionription,
                            dtunit,
                            &sortingtype,
                            &nstep,
                            &axistype,
                            axisname,
                            axisunit);
    delete[] axisname;
    delete[] axisunit;
    if ( erreur < 0 ) { break; }

    // Nombre de noeuds
    med_bool chgt,trsf;
    med_int nnoe  = MEDmeshnEntity(medIdt,
                              meshname,
                              MED_NO_DT,
                              MED_NO_IT,
                              MED_NODE,
                              MED_NO_GEOTYPE,
                              MED_COORDINATE,
                              MED_NO_CMODE,
                              &chgt,
                              &trsf);
    if ( nnoe < 0 )
    {
      erreur =  4;
      break;
    }

    // Les coordonnees
    med_float* coo    = (med_float*) malloc(sizeof(med_float)*nnoe*spacedim);

    erreur = MEDmeshNodeCoordinateRd(medIdt,
                                      meshname,
                                      MED_NO_DT,
                                      MED_NO_IT,
                                      MED_NO_INTERLACE,
                                      coo);
    if ( erreur < 0 )
    {
      free(coo);
      break;
    }

    // Calcul des extremes
    med_float xmin,xmax,ymin,ymax,zmin,zmax;

    xmin=coo[0];
    xmax=coo[0];
    for (int i=1;i<nnoe;i++)
    {
        xmin = std::min(xmin,coo[i]);
        xmax = std::max(xmax,coo[i]);
    }
  //
    if (spacedim > 1)
    {
        ymin=coo[nnoe]; ymax=coo[nnoe];
        for (int i=nnoe+1;i<2*nnoe;i++)
        {
            ymin = std::min(ymin,coo[i]);
            ymax = std::max(ymax,coo[i]);
        }
    }
    else
    {
        ymin=0;
        ymax=0;
        zmin=0;
        zmax=0;
    }
//
    if (spacedim > 2)
    {
        zmin=coo[2*nnoe]; zmax=coo[2*nnoe];
        for (int i=2*nnoe+1;i<3*nnoe;i++)
        {
            zmin = std::min(zmin,coo[i]);
            zmax = std::max(zmax,coo[i]);
        }
    }
    else
    {
        zmin=0;
        zmax=0;
    }

    MESSAGE( "_______________________________________");
    MESSAGE( "xmin : " << xmin << " xmax : " << xmax );
    MESSAGE( "ymin : " << ymin << " ymax : " << ymax );
    MESSAGE( "zmin : " << zmin << " zmax : " << zmax );
    MESSAGE( "_______________________________________" );
    double epsilon = 1.e-6;
    LesExtremes.push_back(xmin);
    LesExtremes.push_back(xmax);
    LesExtremes.push_back(0);
    LesExtremes.push_back(ymin);
    LesExtremes.push_back(ymax);
    LesExtremes.push_back(0);
    LesExtremes.push_back(zmin);
    LesExtremes.push_back(zmax);
    LesExtremes.push_back(0);


   double max1=std::max ( LesExtremes[1] - LesExtremes[0] , LesExtremes[4] - LesExtremes[3] );
   double max2=std::max ( max1 , LesExtremes[7] - LesExtremes[6] );
   LesExtremes.push_back(max2);

// LesExtremes[0] = Xmini du maillage
// LesExtremes[1] = Xmaxi du maillage
// LesExtremes[2] = increment de progression en X
// LesExtremes[3,4,5] : idem pour Y
// LesExtremes[6,7,8] : idem pour Z
// LesExtremes[9] = ecart maximal entre coordonnees
// On fait un traitement pour dans le cas d'une coordonnee constante
// inhiber ce cas en mettant un increment negatif
//
    double diff = LesExtremes[1] - LesExtremes[0];
    if ( fabs(diff) > epsilon*max2 ) { LesExtremes[2] = diff/100.; }
    else                             { LesExtremes[2] = -1.; }

    diff = LesExtremes[4] - LesExtremes[3];
    if ( fabs(diff) > epsilon*max2 ) { LesExtremes[5]=diff/100.; }
    else                             { LesExtremes[5] = -1.; }

    diff = LesExtremes[7] - LesExtremes[6];
    if ( fabs(diff) > epsilon*max2 ) { LesExtremes[8]=diff/100.; }
    else                             { LesExtremes[8] = -1.;  }

    MESSAGE ( "_______________________________________" );
    MESSAGE ( "xmin : " << LesExtremes[0] << " xmax : " << LesExtremes[1] << " xincr : " << LesExtremes[2] );
    MESSAGE ( "ymin : " << LesExtremes[3] << " ymax : " << LesExtremes[4] << " yincr : " << LesExtremes[5] );
    MESSAGE ( "zmin : " << LesExtremes[6] << " zmax : " << LesExtremes[7] << " zincr : " << LesExtremes[8] );
    MESSAGE ( "dmax : " << LesExtremes[9] );
    MESSAGE ( "_______________________________________" );

    free(coo);
    break;
  }
  // Fermeture du fichier
  if ( medIdt > 0 ) MEDfileClose(medIdt);

   return  LesExtremes;
}

}; // namespace SMESHHOMARD_I
