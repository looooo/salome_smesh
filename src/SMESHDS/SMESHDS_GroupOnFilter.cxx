// Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

//  File   : SMESHDS_GroupOnFilter.cxx
//  Module : SMESH
//
#include "SMESHDS_GroupOnFilter.hxx"

#include "SMESHDS_Mesh.hxx"
#include "SMDS_SetIterator.hxx"

using namespace std;

//=============================================================================
/*!
 * Creates a group based on thePredicate
 */
//=============================================================================

SMESHDS_GroupOnFilter::SMESHDS_GroupOnFilter (const int                 theID,
                                              const SMESHDS_Mesh*       theMesh,
                                              const SMDSAbs_ElementType theType,
                                              const SMESH_PredicatePtr& thePredicate)
  : SMESHDS_GroupBase(theID,theMesh,theType), myMeshModifTime(0)
{
  setChanged();
  SetPredicate( thePredicate );
}

//================================================================================
/*!
 * \brief Sets a new predicate
 */
//================================================================================

void SMESHDS_GroupOnFilter::SetPredicate( const SMESH_PredicatePtr& thePredicate)
{
  myPredicate = thePredicate;
  setChanged();
  if ( myPredicate )
    myPredicate->SetMesh( GetMesh() );
}

//================================================================================
/*!
 * \brief Returns nb of elements
 */
//================================================================================

int SMESHDS_GroupOnFilter::Extent()
{
  update();
  return myElements.size();
}

//================================================================================
/*!
 * \brief Checks if the element belongs to the group
 */
//================================================================================

bool SMESHDS_GroupOnFilter::Contains (const int theID)
{
  return myPredicate ? myPredicate->IsSatisfy( theID ) : false;
}

//================================================================================
/*!
 * \brief Checks if the element belongs to the group
 */
//================================================================================

bool SMESHDS_GroupOnFilter::Contains (const SMDS_MeshElement* elem)
{
  return myPredicate ? myPredicate->IsSatisfy( elem->GetID() ) : false;
}

//================================================================================
/*!
 * \brief Return iterator on all elements
 */
//================================================================================

SMDS_ElemIteratorPtr SMESHDS_GroupOnFilter::GetElements() const
{
  update();
  return SMDS_ElemIteratorPtr
    ( new SMDS_ElementVectorIterator( myElements.begin(), myElements.end() ));
}

//================================================================================
/*!
 * \brief return ID of theIndex-th element
 *  \param theIndex - index countered from 1
 *  \retval int - element ID
 */
//================================================================================

int SMESHDS_GroupOnFilter::GetID (const int theIndex)
{
  update();
  if ( theIndex < 1 || theIndex > myElements.size() )
    return -1;
  return myElements[ theIndex-1 ]->GetID();
}

//================================================================================
/*!
 * \brief Updates myElements if necessary
 */
//================================================================================

void SMESHDS_GroupOnFilter::update() const
{
  if ( myMeshModifTime < GetMesh()->GetMTime() )
  {
    SMESHDS_GroupOnFilter* me = const_cast<SMESHDS_GroupOnFilter*>( this );
    me->myElements.clear();
    if ( myPredicate )
    {
      me->myElements.reserve( GetMesh()->GetMeshInfo().NbElements(GetType()));
      SMDS_ElemIteratorPtr elIt = GetMesh()->elementsIterator(GetType());
      while ( elIt->more() )
      {
        const SMDS_MeshElement* e = elIt->next();
        if ( myPredicate->IsSatisfy( e->GetID() ))
          me->myElements.push_back( e );
      }
      me->myElements.resize( myElements.size() );
    }
    me->setChanged( false );
  }
}

//================================================================================
/*!
 * \brief Sets myMeshModifTime according to modification state
 */
//================================================================================

void SMESHDS_GroupOnFilter::setChanged(bool changed)
{
  myMeshModifTime = GetMesh()->GetMTime();
  if ( changed && myMeshModifTime != 0 )
    --myMeshModifTime;
}
