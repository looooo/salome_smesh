//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_MeshEditor_i.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESH_MeshEditor_i.hxx"

#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"

#include "SMESH_MeshEditor.hxx"

#include "SMESH_Gen_i.hxx"
#include "SMESH_Filter_i.hxx"

#include "utilities.h"

#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Vec.hxx>

#include <sstream>

using namespace std;

//=======================================================================
//function : addArray
//purpose  : put array contents into theStr like this: "[ 1, 2, 5 ]"
//=======================================================================

template <class _array>
  TCollection_AsciiString& addArray(TCollection_AsciiString& theStr,
                                    const _array &           IDs)
{
  ostringstream sout; // can convert long int, and TCollection_AsciiString cant
  sout << "[ ";
  for (int i = 1; i <= IDs.length(); i++) {
    sout << IDs[i-1];
    if ( i < IDs.length() )
      sout << ", ";
  }
  sout << " ]";
  theStr += (char*) sout.str().c_str();
  return theStr;
}

//=======================================================================
//function : addObject
//purpose  : add object to script string
//=======================================================================

static TCollection_AsciiString& addObject(TCollection_AsciiString& theStr,
                                          CORBA::Object_ptr        theObject)
{
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  SALOMEDS::SObject_var aSO =
    aSMESHGen->ObjectToSObject(aSMESHGen->GetCurrentStudy(), theObject);
  if ( !aSO->_is_nil() )
    theStr += aSO->GetID();
  else if ( !CORBA::is_nil( theObject ) )
    theStr += aSMESHGen->GetORB()->object_to_string( theObject );
  else
    theStr += "None";
  return theStr;
}

//=======================================================================
//function : addAxis
//purpose  : 
//=======================================================================

static TCollection_AsciiString& addAxis(TCollection_AsciiString&  theStr,
                                        const SMESH::AxisStruct & theAxis)
{
  theStr += "SMESH.AxisStruct( ";
  theStr += TCollection_AsciiString( theAxis.x  ) + ", ";
  theStr += TCollection_AsciiString( theAxis.y  ) + ", ";
  theStr += TCollection_AsciiString( theAxis.z  ) + ", ";
  theStr += TCollection_AsciiString( theAxis.vx ) + ", ";
  theStr += TCollection_AsciiString( theAxis.vy ) + ", ";
  theStr += TCollection_AsciiString( theAxis.vz ) + " )";
  return theStr;
}

//=======================================================================
//function : addToPythonScript
//purpose  : add theStr to python script of the current study
//=======================================================================

static inline void addToPythonScript(const TCollection_AsciiString& theStr)
{
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->AddToPythonScript(aSMESHGen->GetCurrentStudy()->StudyId(), theStr);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_MeshEditor_i::SMESH_MeshEditor_i(SMESH_Mesh* theMesh)
{
	_myMesh = theMesh;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::RemoveElements(const SMESH::long_array & IDsOfElements)
{
  ::SMESH_MeshEditor anEditor( _myMesh );
  list< int > IdList;

  for (int i = 0; i < IDsOfElements.length(); i++)
    IdList.push_back( IDsOfElements[i] );

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.RemoveElements(");
  addArray( str, IDsOfElements ) += ")";
  addToPythonScript( str );
#ifdef _DEBUG_
  addToPythonScript( "print \"RemoveElements: \", isDone" );
#endif
  // Remove Elements
  return anEditor.Remove( IdList, false );
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::RemoveNodes(const SMESH::
	long_array & IDsOfNodes)
{
  ::SMESH_MeshEditor anEditor( _myMesh );
  list< int > IdList;
  for (int i = 0; i < IDsOfNodes.length(); i++)
    IdList.push_back( IDsOfNodes[i] );

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.RemoveNodes(");
  addArray( str, IDsOfNodes ) += ")";
  addToPythonScript( str );
#ifdef _DEBUG_
  addToPythonScript( "print \"RemoveNodes: \", isDone" );
#endif

  return anEditor.Remove( IdList, true );
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddEdge(const SMESH::long_array & IDsOfNodes)
{
  int NbNodes = IDsOfNodes.length();
  if (NbNodes == 2)
  {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    GetMeshDS()->AddEdge(GetMeshDS()->FindNode(index1), GetMeshDS()->FindNode(index2));

    // Update Python script
    TCollection_AsciiString str ("isDone = mesh_editor.AddEdge([");
    str += TCollection_AsciiString((int) index1) + ", ";
    str += TCollection_AsciiString((int) index2) + " ])";
    addToPythonScript( str );
  }
  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddNode(CORBA::Double x,
                                           CORBA::Double y, CORBA::Double z)
{
  GetMeshDS()->AddNode(x, y, z);

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.AddNode(");
  str += TCollection_AsciiString( x ) + ", ";
  str += TCollection_AsciiString( y ) + ", ";
  str += TCollection_AsciiString( z ) + " )";
  addToPythonScript( str );

  return true;
}

//=============================================================================
/*!
 *  AddFace
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddFace(const SMESH::long_array & IDsOfNodes)
{
  int NbNodes = IDsOfNodes.length();
  if (NbNodes < 3)
  {
    return false;
  }

  std::vector<const SMDS_MeshNode*> nodes (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    nodes[i] = GetMeshDS()->FindNode(IDsOfNodes[i]);

  if (NbNodes == 3)
  {
    GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2]);
  }
  else if (NbNodes == 4)
  {
    GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3]);
  }
  else
  {
    GetMeshDS()->AddPolygonalFace(nodes);
  }

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.AddFace(");
  addArray( str, IDsOfNodes ) += ")";
  addToPythonScript( str );
#ifdef _DEBUG_
  addToPythonScript( "print \"AddFace: \", isDone" );
#endif

  return true;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddVolume(const SMESH::
                                             long_array & IDsOfNodes)
{
  int NbNodes = IDsOfNodes.length();
  vector< const SMDS_MeshNode*> n(NbNodes);
  for(int i=0;i<NbNodes;i++)
    n[i]=GetMeshDS()->FindNode(IDsOfNodes[i]);

  switch(NbNodes)
  {
  case 4:GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3]); break;
  case 5:GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4]); break;
  case 6:GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5]); break;
  case 8:GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]); break;
  }
  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.AddVolume(");
  addArray( str, IDsOfNodes ) += ")";
  addToPythonScript( str );
#ifdef _DEBUG_
  addToPythonScript( "print \"AddVolume: \", isDone" );
#endif
  
  return true;
};

//=============================================================================
/*!
 *  AddPolyhedralVolume
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::AddPolyhedralVolume
                                   (const SMESH::long_array & IDsOfNodes,
                                    const SMESH::long_array & Quantities)
{
  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> n (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    n[i] = GetMeshDS()->FindNode(IDsOfNodes[i]);

  int NbFaces = Quantities.length();
  std::vector<int> q (NbFaces);
  for (int j = 0; j < NbFaces; j++)
    q[j] = Quantities[j];

  GetMeshDS()->AddPolyhedralVolume(n, q);

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.AddPolyhedralVolume(");
  addArray( str, IDsOfNodes ) += ", ";
  addArray( str, Quantities ) += ")";
  addToPythonScript( str );
#ifdef _DEBUG_
  addToPythonScript( "print \"AddPolyhedralVolume: \", isDone" );
#endif

  return true;
};

//=============================================================================
/*!
 *  AddPolyhedralVolumeByFaces
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::AddPolyhedralVolumeByFaces
                                   (const SMESH::long_array & IdsOfFaces)
{
  int NbFaces = IdsOfFaces.length();
  std::vector<const SMDS_MeshNode*> poly_nodes;
  std::vector<int> quantities (NbFaces);

  for (int i = 0; i < NbFaces; i++) {
    const SMDS_MeshElement* aFace = GetMeshDS()->FindElement(IdsOfFaces[i]);
    quantities[i] = aFace->NbNodes();

    SMDS_ElemIteratorPtr It = aFace->nodesIterator();
    while (It->more()) {
      poly_nodes.push_back(static_cast<const SMDS_MeshNode *>(It->next()));
    }
  }

  GetMeshDS()->AddPolyhedralVolume(poly_nodes, quantities);

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.AddPolyhedralVolumeByFaces(");
  addArray( str, IdsOfFaces ) += ")";
  addToPythonScript( str );
#ifdef _DEBUG_
  addToPythonScript( "print \"AddPolyhedralVolume: \", isDone" );
#endif

  return true;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::MoveNode(CORBA::Long   NodeID,
                                            CORBA::Double x,
                                            CORBA::Double y,
                                            CORBA::Double z)
{
  const SMDS_MeshNode * node = GetMeshDS()->FindNode( NodeID );
  if ( !node )
    return false;
  
  GetMeshDS()->MoveNode(node, x, y, z);

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.MoveNode(");
  str += TCollection_AsciiString((Standard_Integer) NodeID) + ", ";
  str += TCollection_AsciiString((Standard_Real) x) + ", ";
  str += TCollection_AsciiString((Standard_Real) y) + ", ";
  str += TCollection_AsciiString((Standard_Real) z) + " )";
  addToPythonScript( str );

  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::InverseDiag(CORBA::Long NodeID1,
                                               CORBA::Long NodeID2)
{
  const SMDS_MeshNode * n1 = GetMeshDS()->FindNode( NodeID1 );
  const SMDS_MeshNode * n2 = GetMeshDS()->FindNode( NodeID2 );
  if ( !n1 || !n2 )
    return false;

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.InverseDiag(");
  str += TCollection_AsciiString((Standard_Integer) NodeID1) + ", ";
  str += TCollection_AsciiString((Standard_Integer) NodeID2) + " )";
  addToPythonScript( str );

  ::SMESH_MeshEditor aMeshEditor( _myMesh );
  return aMeshEditor.InverseDiag ( n1, n2 );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::DeleteDiag(CORBA::Long NodeID1,
                                              CORBA::Long NodeID2)
{
  const SMDS_MeshNode * n1 = GetMeshDS()->FindNode( NodeID1 );
  const SMDS_MeshNode * n2 = GetMeshDS()->FindNode( NodeID2 );
  if ( !n1 || !n2 )
    return false;

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.DeleteDiag(");
  str += TCollection_AsciiString((Standard_Integer) NodeID1) + ", ";
  str += TCollection_AsciiString((Standard_Integer) NodeID2) + " )";
  addToPythonScript( str );

  ::SMESH_MeshEditor aMeshEditor( _myMesh );
  return aMeshEditor.DeleteDiag ( n1, n2 );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::Reorient(const SMESH::long_array & IDsOfElements)
{
  ::SMESH_MeshEditor anEditor( _myMesh );
  for (int i = 0; i < IDsOfElements.length(); i++)
  {
    CORBA::Long index = IDsOfElements[i];
    const SMDS_MeshElement * elem = GetMeshDS()->FindElement(index);
    if ( elem )
      anEditor.Reorient( elem );
  }
  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.Reorient(");
  addArray( str, IDsOfElements ) += ")";
  addToPythonScript( str );
#ifdef _DEBUG_
  addToPythonScript( "print \"Reorient: \", isDone" );
#endif

  return true;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::ReorientObject(SMESH::SMESH_IDSource_ptr theObject)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  return Reorient(anElementsId);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::TriToQuad (const SMESH::long_array &   IDsOfElements,
                                 SMESH::NumericalFunctor_ptr Criterion,
                                 CORBA::Double               MaxAngle)
{
  set<const SMDS_MeshElement*> faces;
  for (int i = 0; i < IDsOfElements.length(); i++)
  {
    CORBA::Long index = IDsOfElements[i];
    const SMDS_MeshElement * elem = GetMeshDS()->FindElement(index);
    if ( elem && elem->GetType() == SMDSAbs_Face)
      faces.insert( elem );
  }
  SMESH::NumericalFunctor_i* aNumericalFunctor = 
    dynamic_cast<SMESH::NumericalFunctor_i*>( SMESH_Gen_i::GetServant( Criterion ).in() );
  SMESH::Controls::NumericalFunctorPtr aCrit;
  if ( !aNumericalFunctor )
    aCrit.reset( new SMESH::Controls::AspectRatio() );
  else
    aCrit = aNumericalFunctor->GetNumericalFunctor();

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.TriToQuad(");
  addArray( str, IDsOfElements ) += ", None, ";
  str += (Standard_Real) MaxAngle;
  addToPythonScript( str + ")" );
#ifdef _DEBUG_
  addToPythonScript( "print \"TriToQuad: \", isDone" );
#endif

  ::SMESH_MeshEditor anEditor( _myMesh );
  return anEditor.TriToQuad( faces, aCrit, MaxAngle );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::TriToQuadObject (SMESH::SMESH_IDSource_ptr   theObject,
				       SMESH::NumericalFunctor_ptr Criterion,
				       CORBA::Double               MaxAngle)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  return TriToQuad(anElementsId, Criterion, MaxAngle);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::QuadToTri(const SMESH::long_array &   IDsOfElements,
                                SMESH::NumericalFunctor_ptr Criterion)
{
  set<const SMDS_MeshElement*> faces;
  for (int i = 0; i < IDsOfElements.length(); i++)
  {
    CORBA::Long index = IDsOfElements[i];
    const SMDS_MeshElement * elem = GetMeshDS()->FindElement(index);
    if ( elem && elem->GetType() == SMDSAbs_Face)
      faces.insert( elem );
  }
  SMESH::NumericalFunctor_i* aNumericalFunctor = 
    dynamic_cast<SMESH::NumericalFunctor_i*>( SMESH_Gen_i::GetServant( Criterion ).in() );
  SMESH::Controls::NumericalFunctorPtr aCrit;
  if ( !aNumericalFunctor )
    aCrit.reset( new SMESH::Controls::AspectRatio() );
  else
    aCrit = aNumericalFunctor->GetNumericalFunctor();


  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.QuadToTri(");
  addArray( str, IDsOfElements ) += ", None ), ";
  addToPythonScript( str );
#ifdef _DEBUG_
  addToPythonScript( "print \"QuadToTri: \", isDone" );
#endif

  ::SMESH_MeshEditor anEditor( _myMesh );
  return anEditor.QuadToTri( faces, aCrit );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::SplitQuad(const SMESH::long_array & IDsOfElements,
                                CORBA::Boolean            Diag13)
{
  set<const SMDS_MeshElement*> faces;
  for (int i = 0; i < IDsOfElements.length(); i++)
  {
    CORBA::Long index = IDsOfElements[i];
    const SMDS_MeshElement * elem = GetMeshDS()->FindElement(index);
    if ( elem && elem->GetType() == SMDSAbs_Face)
      faces.insert( elem );
  }

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.SplitQuad(");
  addArray( str, IDsOfElements ) += ", ";
  str += TCollection_AsciiString( Diag13 );
  addToPythonScript( str + ")" );
#ifdef _DEBUG_
  addToPythonScript( "print \"SplitQuad: \", isDone" );
#endif

  ::SMESH_MeshEditor anEditor( _myMesh );
  return anEditor.QuadToTri( faces, Diag13 );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::SplitQuadObject(SMESH::SMESH_IDSource_ptr theObject,
				      CORBA::Boolean            Diag13)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  return SplitQuad(anElementsId, Diag13);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::Smooth(const SMESH::long_array &              IDsOfElements,
                             const SMESH::long_array &              IDsOfFixedNodes,
                             CORBA::Long                            MaxNbOfIterations,
                             CORBA::Double                          MaxAspectRatio,
                             SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  set<const SMDS_MeshElement*> elements;
  for (int i = 0; i < IDsOfElements.length(); i++)
  {
    CORBA::Long index = IDsOfElements[i];
    const SMDS_MeshElement * elem = aMesh->FindElement(index);
    if ( elem && elem->GetType() == SMDSAbs_Face)
      elements.insert( elem );
  }

  set<const SMDS_MeshNode*> fixedNodes;
  for (int i = 0; i < IDsOfFixedNodes.length(); i++)
  {
    CORBA::Long index = IDsOfFixedNodes[i];
    const SMDS_MeshNode * node = aMesh->FindNode(index);
    if ( node )
      fixedNodes.insert( node );
  }
  ::SMESH_MeshEditor::SmoothMethod method = ::SMESH_MeshEditor::LAPLACIAN;
  if ( Method != SMESH::SMESH_MeshEditor::LAPLACIAN_SMOOTH )
    method = ::SMESH_MeshEditor::CENTROIDAL;

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.Smooth( elements, fixedNodes, method, MaxNbOfIterations, MaxAspectRatio );

  // Update Python script
  TCollection_AsciiString str ("isDone = mesh_editor.Smooth(");
  addArray( str, IDsOfElements ) += ", ";
  addArray( str, IDsOfFixedNodes ) += ", ";
  str += (Standard_Integer) MaxNbOfIterations;
  str += ", ";
  str += (Standard_Real) MaxAspectRatio;
  str += ", ";
  if ( method == ::SMESH_MeshEditor::CENTROIDAL )
    str += "SMESH.SMESH_MeshEditor.LAPLACIAN_SMOOTH )";
  else
    str += "SMESH.SMESH_MeshEditor.CENTROIDAL_SMOOTH )";
  addToPythonScript( str );
#ifdef _DEBUG_
  addToPythonScript( "print \"SplitQuad: \", isDone" );
#endif

  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::SmoothObject(SMESH::SMESH_IDSource_ptr              theObject,
				   const SMESH::long_array &              IDsOfFixedNodes,
				   CORBA::Long                            MaxNbOfIterations,
				   CORBA::Double                          MaxAspectRatio,
				   SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  return Smooth(anElementsId, IDsOfFixedNodes, MaxNbOfIterations, MaxAspectRatio, Method);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_MeshEditor_i::RenumberNodes()
{
  // Update Python script
  addToPythonScript( "mesh_editor.RenumberNodes()" );

  GetMeshDS()->Renumber( true );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_MeshEditor_i::RenumberElements()
{
  // Update Python script
  addToPythonScript( "mesh_editor.RenumberElements()" );

  GetMeshDS()->Renumber( false );
}

//=======================================================================
//function : RotationSweep
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::RotationSweep(const SMESH::long_array & theIDsOfElements,
                                       const SMESH::AxisStruct & theAxis,
                                       CORBA::Double             theAngleInRadians,
                                       CORBA::Long               theNbOfSteps,
                                       CORBA::Double             theTolerance)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  set<const SMDS_MeshElement*> elements;
  for (int i = 0; i < theIDsOfElements.length(); i++)
  {
    CORBA::Long index = theIDsOfElements[i];
    const SMDS_MeshElement * elem = aMesh->FindElement(index);
    if ( elem )
      elements.insert( elem );
  }
  gp_Ax1 Ax1 (gp_Pnt( theAxis.x, theAxis.y, theAxis.z ),
              gp_Vec( theAxis.vx, theAxis.vy, theAxis.vz ));

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.RotationSweep (elements, Ax1, theAngleInRadians,
                          theNbOfSteps, theTolerance);

  // Update Python script
  TCollection_AsciiString str = "axis = ";
  addAxis( str, theAxis );
  addToPythonScript( str );
  str = ("mesh_editor.RotationSweep(");
  addArray( str, theIDsOfElements ) += ", axis, ";
  str += TCollection_AsciiString( theAngleInRadians ) + ", ";
  str += TCollection_AsciiString( (int)theNbOfSteps ) + ", ";
  str += TCollection_AsciiString( theTolerance      ) + " )";
  addToPythonScript( str );
}

//=======================================================================
//function : RotationSweepObject
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::RotationSweepObject(SMESH::SMESH_IDSource_ptr theObject,
					     const SMESH::AxisStruct & theAxis,
					     CORBA::Double             theAngleInRadians,
					     CORBA::Long               theNbOfSteps,
					     CORBA::Double             theTolerance)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  RotationSweep(anElementsId, theAxis, theAngleInRadians, theNbOfSteps, theTolerance);
}

//=======================================================================
//function : ExtrusionSweep
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweep(const SMESH::long_array & theIDsOfElements,
                                        const SMESH::DirStruct &  theStepVector,
                                        CORBA::Long               theNbOfSteps)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  set<const SMDS_MeshElement*> elements;
  for (int i = 0; i < theIDsOfElements.length(); i++)
  {
    CORBA::Long index = theIDsOfElements[i];
    const SMDS_MeshElement * elem = aMesh->FindElement(index);
    if ( elem )
      elements.insert( elem );
  }
  const SMESH::PointStruct * P = &theStepVector.PS;
  gp_Vec stepVec( P->x, P->y, P->z );

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.ExtrusionSweep (elements, stepVec, theNbOfSteps);

  // Update Python script
  TCollection_AsciiString str = "stepVector = SMESH.DirStruct( SMESH.PointStruct ( ";
  str += (TCollection_AsciiString) stepVec.X() + ", ";
  str += (TCollection_AsciiString) stepVec.Y() + ", ";
  str += (TCollection_AsciiString) stepVec.Z() + " ))";
  addToPythonScript( str );
  str = ("mesh_editor.ExtrusionSweep(");
  addArray( str, theIDsOfElements ) += ", stepVector, ";
  str += TCollection_AsciiString((int)theNbOfSteps) + " )";
  addToPythonScript( str );
}


//=======================================================================
//function : ExtrusionSweepObject
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweepObject(SMESH::SMESH_IDSource_ptr theObject,
					      const SMESH::DirStruct &  theStepVector,
					      CORBA::Long               theNbOfSteps)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  ExtrusionSweep(anElementsId, theStepVector, theNbOfSteps);
}

#define RETCASE(enm) case ::SMESH_MeshEditor::enm: return SMESH::SMESH_MeshEditor::enm;

static SMESH::SMESH_MeshEditor::Extrusion_Error convExtrError( const::SMESH_MeshEditor::Extrusion_Error e )
{
  switch ( e ) {
  RETCASE( EXTR_OK );
  RETCASE( EXTR_NO_ELEMENTS );
  RETCASE( EXTR_PATH_NOT_EDGE );
  RETCASE( EXTR_BAD_PATH_SHAPE );
  RETCASE( EXTR_BAD_STARTING_NODE );
  RETCASE( EXTR_BAD_ANGLES_NUMBER );
  RETCASE( EXTR_CANT_GET_TANGENT );
  }
  return SMESH::SMESH_MeshEditor::EXTR_OK;
}

//=======================================================================
//function : ExtrusionAlongPath
//purpose  : 
//=======================================================================

SMESH::SMESH_MeshEditor::Extrusion_Error
  SMESH_MeshEditor_i::ExtrusionAlongPath(const SMESH::long_array &   theIDsOfElements,
					 SMESH::SMESH_Mesh_ptr       thePathMesh,
					 GEOM::GEOM_Object_ptr       thePathShape,
					 CORBA::Long                 theNodeStart,
					 CORBA::Boolean              theHasAngles,
					 const SMESH::double_array & theAngles,
					 CORBA::Boolean              theHasRefPoint,
					 const SMESH::PointStruct &  theRefPoint)
{
  SMESHDS_Mesh*  aMesh = GetMeshDS();

  if ( thePathMesh->_is_nil() || thePathShape->_is_nil() )
    return SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE;

  SMESH_Mesh_i* aMeshImp = dynamic_cast<SMESH_Mesh_i*>( SMESH_Gen_i::GetServant( thePathMesh ).in() );
  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( thePathShape );
  SMESH_subMesh* aSubMesh = aMeshImp->GetImpl().GetSubMesh( aShape );

  if ( !aSubMesh )
    return SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE;

  SMDS_MeshNode* nodeStart = (SMDS_MeshNode*)aMeshImp->GetImpl().GetMeshDS()->FindNode(theNodeStart);
  if ( !nodeStart )
    return SMESH::SMESH_MeshEditor::EXTR_BAD_STARTING_NODE;

  set<const SMDS_MeshElement*> elements;
  for (int i = 0; i < theIDsOfElements.length(); i++)
  {
    CORBA::Long index = theIDsOfElements[i];
    const SMDS_MeshElement * elem = aMesh->FindElement(index);
    if ( elem )
      elements.insert( elem );
  }

  list<double> angles;
  for (int i = 0; i < theAngles.length(); i++)
  {
    angles.push_back( theAngles[i] );
  }

  gp_Pnt refPnt( theRefPoint.x, theRefPoint.y, theRefPoint.z );

  // Update Python script
  TCollection_AsciiString str = "refPoint = SMESH.PointStruct( ";
  str += (TCollection_AsciiString) refPnt.X() + ", ";
  str += (TCollection_AsciiString) refPnt.Y() + ", ";
  str += (TCollection_AsciiString) refPnt.Z() + " )";
  addToPythonScript( str );
  str = ("error = mesh_editor.ExtrusionAlongPath(");
  addArray ( str, theIDsOfElements ) += ", ";
  addObject( str, thePathMesh ) += ", salome.IDToObject(\"";
  addObject( str, thePathShape ) += "\"), ";
  str += TCollection_AsciiString( (int)theNodeStart ) + ", ";
  str += TCollection_AsciiString( (int)theHasAngles ) + ", ";
  addArray ( str, theAngles ) += ", ";
  str += (TCollection_AsciiString) theHasRefPoint + ", refPoint )";
  addToPythonScript( str );

  ::SMESH_MeshEditor anEditor( _myMesh );
  return convExtrError( anEditor.ExtrusionAlongTrack( elements, aSubMesh, nodeStart, theHasAngles, angles, theHasRefPoint, refPnt ) );
}

//=======================================================================
//function : ExtrusionAlongPathObject
//purpose  : 
//=======================================================================

SMESH::SMESH_MeshEditor::Extrusion_Error
  SMESH_MeshEditor_i::ExtrusionAlongPathObject(SMESH::SMESH_IDSource_ptr   theObject,
					       SMESH::SMESH_Mesh_ptr       thePathMesh,
					       GEOM::GEOM_Object_ptr       thePathShape,
					       CORBA::Long                 theNodeStart,
					       CORBA::Boolean              theHasAngles,
					       const SMESH::double_array & theAngles,
					       CORBA::Boolean              theHasRefPoint,
					       const SMESH::PointStruct &  theRefPoint)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  return ExtrusionAlongPath( anElementsId, thePathMesh, thePathShape, theNodeStart, theHasAngles, theAngles, theHasRefPoint, theRefPoint );
}

//=======================================================================
//function : Mirror
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::Mirror(const SMESH::long_array &           theIDsOfElements,
                                const SMESH::AxisStruct &           theAxis,
                                SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
                                CORBA::Boolean                      theCopy)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  set<const SMDS_MeshElement*> elements;
  for (int i = 0; i < theIDsOfElements.length(); i++)
  {
    CORBA::Long index = theIDsOfElements[i];
    const SMDS_MeshElement * elem = aMesh->FindElement(index);
    if ( elem )
      elements.insert( elem );
  }
  gp_Pnt P ( theAxis.x, theAxis.y, theAxis.z );
  gp_Vec V ( theAxis.vx, theAxis.vy, theAxis.vz );

  gp_Trsf aTrsf;
  TCollection_AsciiString typeStr, copyStr( theCopy );
  switch ( theMirrorType ) {
  case  SMESH::SMESH_MeshEditor::POINT:
    aTrsf.SetMirror( P );
    typeStr = "SMESH.SMESH_MeshEditor.POINT";
    break;
  case  SMESH::SMESH_MeshEditor::AXIS:
    aTrsf.SetMirror( gp_Ax1( P, V ));
    typeStr = "SMESH.SMESH_MeshEditor.AXIS";
    break;
  default:
    aTrsf.SetMirror( gp_Ax2( P, V ));
    typeStr = "SMESH.SMESH_MeshEditor.PLANE";
  }

  // Update Python script
  TCollection_AsciiString str ("mesh_editor.Mirror(");
  addArray( str, theIDsOfElements ) += ", ";
  addAxis( str, theAxis )           += ", ";
  str += typeStr                     + ", ";
  str += copyStr                     + " )";
  addToPythonScript( str );

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.Transform (elements, aTrsf, theCopy);
}

//=======================================================================
//function : MirrorObject
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::MirrorObject(SMESH::SMESH_IDSource_ptr           theObject,
				      const SMESH::AxisStruct &           theAxis,
				      SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
				      CORBA::Boolean                      theCopy)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  Mirror(anElementsId, theAxis, theMirrorType, theCopy);
}

//=======================================================================
//function : Translate
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::Translate(const SMESH::long_array & theIDsOfElements,
                                   const SMESH::DirStruct &  theVector,
                                   CORBA::Boolean            theCopy)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  set<const SMDS_MeshElement*> elements;
  for (int i = 0; i < theIDsOfElements.length(); i++)
  {
    CORBA::Long index = theIDsOfElements[i];
    const SMDS_MeshElement * elem = aMesh->FindElement(index);
    if ( elem )
      elements.insert( elem );
  }
  gp_Trsf aTrsf;
  const SMESH::PointStruct * P = &theVector.PS;
  aTrsf.SetTranslation( gp_Vec( P->x, P->y, P->z ));

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.Transform (elements, aTrsf, theCopy);

  // Update Python script
  TCollection_AsciiString str = "vector = SMESH.DirStruct( SMESH.PointStruct ( ";
  str += (TCollection_AsciiString) P->x + ", ";
  str += (TCollection_AsciiString) P->y + ", ";
  str += (TCollection_AsciiString) P->z + " ))";
  addToPythonScript( str );
  str = ("mesh_editor.Translate(");
  addArray( str, theIDsOfElements ) += ", vector, ";
  str += (TCollection_AsciiString) theCopy + " )";
  addToPythonScript( str );
}

//=======================================================================
//function : TranslateObject
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::TranslateObject(SMESH::SMESH_IDSource_ptr theObject,
					 const SMESH::DirStruct &  theVector,
					 CORBA::Boolean            theCopy)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  Translate(anElementsId, theVector, theCopy);
}

//=======================================================================
//function : Rotate
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::Rotate(const SMESH::long_array & theIDsOfElements,
                                const SMESH::AxisStruct & theAxis,
                                CORBA::Double             theAngle,
                                CORBA::Boolean            theCopy)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  set<const SMDS_MeshElement*> elements;
  for (int i = 0; i < theIDsOfElements.length(); i++)
  {
    CORBA::Long index = theIDsOfElements[i];
    const SMDS_MeshElement * elem = aMesh->FindElement(index);
    if ( elem )
      elements.insert( elem );
  }
  gp_Pnt P ( theAxis.x, theAxis.y, theAxis.z );
  gp_Vec V ( theAxis.vx, theAxis.vy, theAxis.vz );

  gp_Trsf aTrsf;
  aTrsf.SetRotation( gp_Ax1( P, V ), theAngle);
  
  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.Transform (elements, aTrsf, theCopy);

  // Update Python script
  TCollection_AsciiString str ("axis = ");
  addAxis( str, theAxis );
  addToPythonScript( str );
  str = ("mesh_editor.Rotate(");
  addArray( str, theIDsOfElements ) += ", axis, ";
  str += (TCollection_AsciiString) theAngle + ", ";
  str += (TCollection_AsciiString) theCopy + " )";
  addToPythonScript( str );
}

//=======================================================================
//function : RotateObject
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::RotateObject(SMESH::SMESH_IDSource_ptr theObject,
				      const SMESH::AxisStruct & theAxis,
				      CORBA::Double             theAngle,
				      CORBA::Boolean            theCopy)
{
  SMESH::long_array_var anElementsId = theObject->GetIDs();
  Rotate(anElementsId, theAxis, theAngle, theCopy);
}

//=======================================================================
//function : FindCoincidentNodes
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::FindCoincidentNodes (CORBA::Double                  Tolerance,
                                              SMESH::array_of_long_array_out GroupsOfNodes)
{
  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  ::SMESH_MeshEditor anEditor( _myMesh );
  set<const SMDS_MeshNode*> nodes; // no input nodes
  anEditor.FindCoincidentNodes( nodes, Tolerance, aListOfListOfNodes );

  GroupsOfNodes = new SMESH::array_of_long_array;
  GroupsOfNodes->length( aListOfListOfNodes.size() );
  ::SMESH_MeshEditor::TListOfListOfNodes::iterator llIt = aListOfListOfNodes.begin();
  for ( CORBA::Long i = 0; llIt != aListOfListOfNodes.end(); llIt++, i++ )
  {
    list< const SMDS_MeshNode* >& aListOfNodes = *llIt;
    list< const SMDS_MeshNode* >::iterator lIt = aListOfNodes.begin();;
    SMESH::long_array& aGroup = GroupsOfNodes[ i ];
    aGroup.length( aListOfNodes.size() );
    for ( int j = 0; lIt != aListOfNodes.end(); lIt++, j++ )
      aGroup[ j ] = (*lIt)->GetID();
  }
}

//=======================================================================
//function : MergeNodes
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::MergeNodes (const SMESH::array_of_long_array& GroupsOfNodes)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  TCollection_AsciiString str( "mesh_editor.MergeNodes([" );
  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  for (int i = 0; i < GroupsOfNodes.length(); i++)
  {
    const SMESH::long_array& aNodeGroup = GroupsOfNodes[ i ];
    aListOfListOfNodes.push_back( list< const SMDS_MeshNode* >() );
    list< const SMDS_MeshNode* >& aListOfNodes = aListOfListOfNodes.back();
    for ( int j = 0; j < aNodeGroup.length(); j++ )
    {
      CORBA::Long index = aNodeGroup[ j ];
      const SMDS_MeshNode * node = aMesh->FindNode(index);
      if ( node )
        aListOfNodes.push_back( node );
    }
    if ( aListOfNodes.size() < 2 )
      aListOfListOfNodes.pop_back();

    if ( i > 0 )
      str += ",";
    addArray( str, aNodeGroup );
  }
  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.MergeNodes( aListOfListOfNodes );

  // Update Python script
  addToPythonScript( str + "])" );
}

//=======================================================================
//function : MergeEqualElements
//purpose  : 
//=======================================================================

void SMESH_MeshEditor_i::MergeEqualElements()
{
  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.MergeEqualElements();

  // Update Python script
  addToPythonScript( "mesh_editor.MergeEqualElements()" );
}

//=======================================================================
//function : operator
//purpose  : 
//=======================================================================

#define RETCASE(enm) case ::SMESH_MeshEditor::enm: return SMESH::SMESH_MeshEditor::enm;

static SMESH::SMESH_MeshEditor::Sew_Error convError( const::SMESH_MeshEditor::Sew_Error e )
{
  switch ( e ) {
  RETCASE( SEW_OK );
  RETCASE( SEW_BORDER1_NOT_FOUND );
  RETCASE( SEW_BORDER2_NOT_FOUND );
  RETCASE( SEW_BOTH_BORDERS_NOT_FOUND );
  RETCASE( SEW_BAD_SIDE_NODES );
  RETCASE( SEW_VOLUMES_TO_SPLIT );
  RETCASE( SEW_DIFF_NB_OF_ELEMENTS );
  RETCASE( SEW_TOPO_DIFF_SETS_OF_ELEMENTS );
  RETCASE( SEW_BAD_SIDE1_NODES );
  RETCASE( SEW_BAD_SIDE2_NODES );
  }
  return SMESH::SMESH_MeshEditor::SEW_OK;
}

//=======================================================================
//function : SewFreeBorders
//purpose  : 
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
  SMESH_MeshEditor_i::SewFreeBorders(CORBA::Long FirstNodeID1,
                                     CORBA::Long SecondNodeID1,
                                     CORBA::Long LastNodeID1,
                                     CORBA::Long FirstNodeID2,
                                     CORBA::Long SecondNodeID2,
                                     CORBA::Long LastNodeID2,
                                     CORBA::Boolean CreatePolygons,
                                     CORBA::Boolean CreatePolyedrs)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aBorderFirstNode  = aMesh->FindNode( FirstNodeID1  );
  const SMDS_MeshNode* aBorderSecondNode = aMesh->FindNode( SecondNodeID1 );
  const SMDS_MeshNode* aBorderLastNode   = aMesh->FindNode( LastNodeID1   );
  const SMDS_MeshNode* aSide2FirstNode   = aMesh->FindNode( FirstNodeID2  );
  const SMDS_MeshNode* aSide2SecondNode  = aMesh->FindNode( SecondNodeID2 );
  const SMDS_MeshNode* aSide2ThirdNode   = aMesh->FindNode( LastNodeID2   );

  if (!aBorderFirstNode ||
      !aBorderSecondNode||
      !aBorderLastNode)
    return SMESH::SMESH_MeshEditor::SEW_BORDER1_NOT_FOUND;
  if (!aSide2FirstNode  ||
      !aSide2SecondNode ||
      !aSide2ThirdNode)
    return SMESH::SMESH_MeshEditor::SEW_BORDER2_NOT_FOUND;

  // Update Python script
  TCollection_AsciiString str ("error = mesh_editor.SewFreeBorders( ");
  str += TCollection_AsciiString( (int) FirstNodeID1  )  + ", ";
  str += TCollection_AsciiString( (int) SecondNodeID1 )  + ", ";
  str += TCollection_AsciiString( (int) LastNodeID1   )  + ", ";
  str += TCollection_AsciiString( (int) FirstNodeID2  )  + ", ";
  str += TCollection_AsciiString( (int) SecondNodeID2 )  + ", ";
  str += TCollection_AsciiString( (int) LastNodeID2   )  + ", ";
  str += TCollection_AsciiString( CreatePolygons      )  + ", ";
  str += TCollection_AsciiString( CreatePolyedrs      )  + ") ";
  addToPythonScript( str );

  ::SMESH_MeshEditor anEditor( _myMesh );
  return convError( anEditor.SewFreeBorder (aBorderFirstNode,
                                            aBorderSecondNode,
                                            aBorderLastNode,
                                            aSide2FirstNode,
                                            aSide2SecondNode,
                                            aSide2ThirdNode,
                                            true,
                                            CreatePolygons,
                                            CreatePolyedrs) );
}

//=======================================================================
//function : SewConformFreeBorders
//purpose  : 
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
  SMESH_MeshEditor_i::SewConformFreeBorders(CORBA::Long FirstNodeID1,
                                            CORBA::Long SecondNodeID1,
                                            CORBA::Long LastNodeID1,
                                            CORBA::Long FirstNodeID2,
                                            CORBA::Long SecondNodeID2)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aBorderFirstNode  = aMesh->FindNode( FirstNodeID1  );
  const SMDS_MeshNode* aBorderSecondNode = aMesh->FindNode( SecondNodeID1 );
  const SMDS_MeshNode* aBorderLastNode   = aMesh->FindNode( LastNodeID1   );
  const SMDS_MeshNode* aSide2FirstNode   = aMesh->FindNode( FirstNodeID2  );
  const SMDS_MeshNode* aSide2SecondNode  = aMesh->FindNode( SecondNodeID2 );
  const SMDS_MeshNode* aSide2ThirdNode   = 0;

  if (!aBorderFirstNode ||
      !aBorderSecondNode||
      !aBorderLastNode )
    return SMESH::SMESH_MeshEditor::SEW_BORDER1_NOT_FOUND;
  if (!aSide2FirstNode  ||
      !aSide2SecondNode)
    return SMESH::SMESH_MeshEditor::SEW_BORDER2_NOT_FOUND;

  // Update Python script
  TCollection_AsciiString str ("error = mesh_editor.SewConformFreeBorders( ");
  str += TCollection_AsciiString( (int) FirstNodeID1  )  + ", ";
  str += TCollection_AsciiString( (int) SecondNodeID1 )  + ", ";
  str += TCollection_AsciiString( (int) LastNodeID1   )  + ", ";
  str += TCollection_AsciiString( (int) FirstNodeID2  )  + ", ";
  str += TCollection_AsciiString( (int) SecondNodeID2 )  + ")";
  addToPythonScript( str );

  ::SMESH_MeshEditor anEditor( _myMesh );
  return convError( anEditor.SewFreeBorder (aBorderFirstNode,
                                            aBorderSecondNode,
                                            aBorderLastNode,
                                            aSide2FirstNode,
                                            aSide2SecondNode,
                                            aSide2ThirdNode,
                                            true,
                                            false, false) );
}

//=======================================================================
//function : SewBorderToSide
//purpose  : 
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
  SMESH_MeshEditor_i::SewBorderToSide(CORBA::Long FirstNodeIDOnFreeBorder,
                                      CORBA::Long SecondNodeIDOnFreeBorder,
                                      CORBA::Long LastNodeIDOnFreeBorder,
                                      CORBA::Long FirstNodeIDOnSide,
                                      CORBA::Long LastNodeIDOnSide,
                                      CORBA::Boolean CreatePolygons,
                                      CORBA::Boolean CreatePolyedrs)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aBorderFirstNode  = aMesh->FindNode( FirstNodeIDOnFreeBorder  );
  const SMDS_MeshNode* aBorderSecondNode = aMesh->FindNode( SecondNodeIDOnFreeBorder );
  const SMDS_MeshNode* aBorderLastNode   = aMesh->FindNode( LastNodeIDOnFreeBorder   );
  const SMDS_MeshNode* aSide2FirstNode   = aMesh->FindNode( FirstNodeIDOnSide  );
  const SMDS_MeshNode* aSide2SecondNode  = aMesh->FindNode( LastNodeIDOnSide );
  const SMDS_MeshNode* aSide2ThirdNode   = 0;

  if (!aBorderFirstNode ||
      !aBorderSecondNode||
      !aBorderLastNode  )
    return SMESH::SMESH_MeshEditor::SEW_BORDER1_NOT_FOUND;
  if (!aSide2FirstNode  ||
      !aSide2SecondNode)
    return SMESH::SMESH_MeshEditor::SEW_BAD_SIDE_NODES;

  // Update Python script
  TCollection_AsciiString str ("error = mesh_editor.SewBorderToSide( ");
  str += TCollection_AsciiString( (int) FirstNodeIDOnFreeBorder  ) + ", ";
  str += TCollection_AsciiString( (int) SecondNodeIDOnFreeBorder ) + ", ";
  str += TCollection_AsciiString( (int) LastNodeIDOnFreeBorder   ) + ", ";
  str += TCollection_AsciiString( (int) FirstNodeIDOnSide        ) + ", ";
  str += TCollection_AsciiString( (int) LastNodeIDOnSide         ) + ", ";
  str += TCollection_AsciiString( CreatePolygons                 ) + ", ";
  str += TCollection_AsciiString( CreatePolyedrs                 ) + ") ";
  addToPythonScript( str );

  ::SMESH_MeshEditor anEditor( _myMesh );
  return convError( anEditor.SewFreeBorder (aBorderFirstNode,
                                            aBorderSecondNode,
                                            aBorderLastNode,
                                            aSide2FirstNode,
                                            aSide2SecondNode,
                                            aSide2ThirdNode,
                                            false,
                                            CreatePolygons,
                                            CreatePolyedrs) );
}

//=======================================================================
//function : SewSideElements
//purpose  : 
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
  SMESH_MeshEditor_i::SewSideElements(const SMESH::long_array& IDsOfSide1Elements,
                                      const SMESH::long_array& IDsOfSide2Elements,
                                      CORBA::Long NodeID1OfSide1ToMerge,
                                      CORBA::Long NodeID1OfSide2ToMerge,
                                      CORBA::Long NodeID2OfSide1ToMerge,
                                      CORBA::Long NodeID2OfSide2ToMerge)
{
  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aFirstNode1ToMerge  = aMesh->FindNode( NodeID1OfSide1ToMerge );
  const SMDS_MeshNode* aFirstNode2ToMerge  = aMesh->FindNode( NodeID1OfSide2ToMerge );
  const SMDS_MeshNode* aSecondNode1ToMerge = aMesh->FindNode( NodeID2OfSide1ToMerge );
  const SMDS_MeshNode* aSecondNode2ToMerge = aMesh->FindNode( NodeID2OfSide2ToMerge );

  if (!aFirstNode1ToMerge ||
      !aFirstNode2ToMerge )
    return SMESH::SMESH_MeshEditor::SEW_BAD_SIDE1_NODES;
  if (!aSecondNode1ToMerge||
      !aSecondNode2ToMerge)
    return SMESH::SMESH_MeshEditor::SEW_BAD_SIDE2_NODES;

  set<const SMDS_MeshElement*> aSide1Elems, aSide2Elems;
  for (int i = 0; i < IDsOfSide1Elements.length(); i++)
  {
    CORBA::Long index = IDsOfSide1Elements[i];
    const SMDS_MeshElement * elem = aMesh->FindElement(index);
    if ( elem )
      aSide1Elems.insert( elem );
  }
  for (int i = 0; i < IDsOfSide2Elements.length(); i++)
  {
    CORBA::Long index = IDsOfSide2Elements[i];
    const SMDS_MeshElement * elem = aMesh->FindElement(index);
    if ( elem )
      aSide2Elems.insert( elem );
  }
  // Update Python script
  TCollection_AsciiString str ("error = mesh_editor.SewSideElements( ");
  addArray( str, IDsOfSide1Elements ) += ", ";
  addArray( str, IDsOfSide2Elements ) += ", ";
  str += TCollection_AsciiString( (int) NodeID1OfSide1ToMerge ) + ", ";
  str += TCollection_AsciiString( (int) NodeID1OfSide2ToMerge ) + ", ";
  str += TCollection_AsciiString( (int) NodeID2OfSide1ToMerge ) + ", ";
  str += TCollection_AsciiString( (int) NodeID2OfSide2ToMerge ) + ")";
  addToPythonScript( str );

  ::SMESH_MeshEditor anEditor( _myMesh );
  return convError( anEditor.SewSideElements (aSide1Elems, aSide2Elems,
                                              aFirstNode1ToMerge,
                                              aFirstNode2ToMerge,
                                              aSecondNode1ToMerge,
                                              aSecondNode2ToMerge));
}
