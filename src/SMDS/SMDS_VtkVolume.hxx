#ifndef _SMDS_VTKVOLUME_HXX_
#define _SMDS_VTKVOLUME_HXX_

#include "SMESH_SMDS.hxx"

#include "SMDS_MeshVolume.hxx"
#include "SMDS_UnstructuredGrid.hxx"
#include <vector>

class SMDS_EXPORT SMDS_VtkVolume: public SMDS_MeshVolume
{
public:
  SMDS_VtkVolume();
  SMDS_VtkVolume(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh);
  ~SMDS_VtkVolume();
  void init(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh);
#ifdef VTK_HAVE_POLYHEDRON
  void initPoly(std::vector<vtkIdType> nodeIds, std::vector<int> nbNodesPerFace, SMDS_Mesh* mesh);
#endif
  bool ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes);

  void Print(std::ostream & OS) const;
  int NbFaces() const;
  int NbNodes() const;
  int NbEdges() const;
  virtual SMDSAbs_ElementType GetType() const;
  virtual vtkIdType GetVtkType() const;
  virtual SMDSAbs_EntityType GetEntityType() const;
  virtual const SMDS_MeshNode* GetNode(const int ind) const;
  virtual bool IsQuadratic() const;
  virtual bool IsPoly() const;
  static void gravityCenter(SMDS_UnstructuredGrid* grid,
                            vtkIdType *nodeIds,
                            int nbNodes,
                            double* result);
  static bool isForward(double* a,double* b,double* c,double* d);

protected:
  virtual SMDS_ElemIteratorPtr elementsIterator(SMDSAbs_ElementType type) const;
  virtual SMDS_ElemIteratorPtr nodesIteratorToUNV() const;
};

#endif
