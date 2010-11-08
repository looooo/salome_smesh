//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMDS : implementaion of Salome mesh data structure
//  File   : SMDS_MeshElement.hxx
//  Module : SMESH
//
#ifndef _SMDS_MeshElement_HeaderFile
#define _SMDS_MeshElement_HeaderFile

#include "SMESH_SMDS.hxx"
        
#include "SMDSAbs_ElementType.hxx"
#include "SMDS_MeshObject.hxx"
#include "SMDS_ElemIterator.hxx"
#include "SMDS_MeshElementIDFactory.hxx"
#include "SMDS_StdIterator.hxx"

#include <vector>
#include <iostream>

#include <vtkType.h>
#include <vtkCellType.h>

//typedef unsigned short UShortType;
typedef short ShortType;

class SMDS_MeshNode;
class SMDS_MeshEdge;
class SMDS_MeshFace;
class SMDS_Mesh;

// ============================================================
/*!
 * \brief Base class for elements
 */
// ============================================================


class SMDS_EXPORT SMDS_MeshElement:public SMDS_MeshObject
{
public:

  SMDS_ElemIteratorPtr nodesIterator() const;
  SMDS_ElemIteratorPtr edgesIterator() const;
  SMDS_ElemIteratorPtr facesIterator() const;
  virtual SMDS_ElemIteratorPtr elementsIterator(SMDSAbs_ElementType type) const;
  virtual SMDS_ElemIteratorPtr nodesIteratorToUNV() const;
  virtual SMDS_ElemIteratorPtr interlacedNodesElemIterator() const;

  // std-like iteration on nodes
  typedef SMDS_StdIterator< const SMDS_MeshNode*, SMDS_ElemIteratorPtr > iterator;
  iterator begin_nodes() const { return iterator( nodesIterator() ); }
  iterator end_nodes()   const { return iterator(); }

  virtual int NbNodes() const;
  virtual int NbEdges() const;
  virtual int NbFaces() const;
  inline int GetID() const { return myID; };

  ///Return the type of the current element
  virtual SMDSAbs_ElementType GetType() const = 0;
  virtual vtkIdType GetVtkType() const = 0;
  virtual bool IsPoly() const { return false; };
  virtual bool IsQuadratic() const;
  //! Return type of entity
  virtual SMDSAbs_EntityType  GetEntityType() const = 0;

  virtual bool IsMediumNode(const SMDS_MeshNode* node) const;
  virtual int  NbCornerNodes() const;

  friend SMDS_EXPORT std::ostream & operator <<(std::ostream & OS, const SMDS_MeshElement *);
  friend SMDS_EXPORT bool SMDS_MeshElementIDFactory::BindID(int ID,SMDS_MeshElement* elem);
  friend class SMDS_Mesh;
  friend class SMESHDS_Mesh;

  // ===========================
  //  Access to nodes by index
  // ===========================
  /*!
   * \brief Return node by its index
    * \param ind - node index
    * \retval const SMDS_MeshNode* - the node
   */
  virtual const SMDS_MeshNode* GetNode(const int ind) const;

  /*!
   * \brief Return node by its index
    * \param ind - node index
    * \retval const SMDS_MeshNode* - the node
   * 
   * Index is wrapped if it is out of a valid range
   */
  const SMDS_MeshNode* GetNodeWrap(const int ind) const { return GetNode( WrappedIndex( ind )); }

  /*!
   * \brief Return true if index of node is valid (0 <= ind < NbNodes())
    * \param ind - node index
    * \retval bool - index check result
   */
  virtual bool IsValidIndex(const int ind) const;

  /*!
   * \brief Return a valid node index, fixing the given one if necessary
    * \param ind - node index
    * \retval int - valid node index
   */
  int WrappedIndex(const int ind) const {
    if ( ind < 0 ) return NbNodes() + ind % NbNodes();
    if ( ind >= NbNodes() ) return ind % NbNodes();
    return ind;
  }

  /*!
   * \brief Check if a node belongs to the element
    * \param node - the node to check
    * \retval int - node index within the element, -1 if not found
   */
  int GetNodeIndex( const SMDS_MeshNode* node ) const;

  //inline int getId() const {return myID; };
  inline ShortType getMeshId() const {return myMeshId; };
  inline ShortType getshapeId() const {return myShapeId; };
  inline void setShapeId(ShortType shapeId) {myShapeId = shapeId; };
  inline int getIdInShape() const { return myIdInShape; };
  inline void setIdInShape(int id) { myIdInShape = id; };

  inline void setVtkId(int vtkId)
  {
    myVtkID = vtkId;
  }

  inline int getVtkId() const
  {
    return myVtkID;
  }

protected:
  inline void setId(int id) {myID = id; };
  SMDS_MeshElement(int ID=-1);
  SMDS_MeshElement(int id, ShortType meshId, ShortType shapeId=-1);
  virtual void Print(std::ostream & OS) const;

  //! Element index in vector SMDS_Mesh::myNodes or SMDS_Mesh::myCells
  int myID;

  // index in vtkUnstructuredGrid
  int myVtkID;

  //! SMDS_Mesh identification in SMESH
  ShortType myMeshId;

  //! SubShape and SubMesh identification in SMESHDS (not in use?)
  ShortType myShapeId;

  //! Element index in SMESHDS_SubMesh vector
  int myIdInShape;
};


// ============================================================
/*!
 * \brief Comparator of elements by ID for usage in std containers
 */
// ============================================================

struct TIDCompare {
  bool operator () (const SMDS_MeshElement* e1, const SMDS_MeshElement* e2) const
  { return e1->GetID() < e2->GetID(); }
};

#endif
