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
//  File   : SMDS_MeshNode.hxx
//  Module : SMESH
//
#ifndef _SMDS_MeshNode_HeaderFile
#define _SMDS_MeshNode_HeaderFile

#include "SMESH_SMDS.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_Position.hxx"
#include "ObjectPool.hxx"
#include <NCollection_List.hxx>

class SMDS_EXPORT SMDS_MeshNode:public SMDS_MeshElement
{
public:
  friend class SMESHDS_Mesh;
  friend class SMDS_Mesh;
  friend class ObjectPool<SMDS_MeshNode>;

  void Print(std::ostream & OS) const;
  double X() const;
  double Y() const;
  double Z() const;
  SMDS_ElemIteratorPtr GetInverseElementIterator(SMDSAbs_ElementType type=SMDSAbs_All) const;
  int NbInverseElements(SMDSAbs_ElementType type=SMDSAbs_All) const;
  const SMDS_PositionPtr& GetPosition() const;
  SMDSAbs_ElementType GetType() const;
  virtual vtkIdType GetVtkType() const;
  SMDSAbs_EntityType  GetEntityType() const {return SMDSEntity_Node;}
  int NbNodes() const;

  friend bool operator<(const SMDS_MeshNode& e1, const SMDS_MeshNode& e2);

  void SetPosition(const SMDS_PositionPtr& aPos);
  void setXYZ(double x, double y, double z);

  static int nbNodes;

protected:
  SMDS_MeshNode();
  SMDS_MeshNode(int id, int meshId, int shapeId = -1, double x=0, double y=0, double z=0);
  virtual ~SMDS_MeshNode();
  void init(int id, int meshId, int shapeId = -1, double x=0, double y=0, double z=0);
  inline void setVtkId(int vtkId) { myVtkID = vtkId; };
  double* getCoord() const;
  void AddInverseElement(const SMDS_MeshElement * ME);
  void RemoveInverseElement(const SMDS_MeshElement * parent);
  void ClearInverseElements();
  bool emptyInverseElements();

  SMDS_ElemIteratorPtr
  elementsIterator(SMDSAbs_ElementType type) const;

private:
  SMDS_PositionPtr myPosition;
};

#endif
