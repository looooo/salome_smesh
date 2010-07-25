/* 
 * File:   SMDS_UnstructuredGrid.hxx
 * Author: prascle
 *
 * Created on September 16, 2009, 10:28 PM
 */

#ifndef _SMDS_UNSTRUCTUREDGRID_HXX
#define	_SMDS_UNSTRUCTUREDGRID_HXX

#include <vector>
#include <set>
#include <map>

#include <vtkUnstructuredGrid.h>
#include "chrono.hxx"

#define NBMAXNEIGHBORS 10

class SMDS_Downward;
class SMDS_Mesh;

class SMDS_UnstructuredGrid: public vtkUnstructuredGrid
{
public:
  void setSMDS_mesh(SMDS_Mesh *mesh);
  void compactGrid(std::vector<int>& idNodesOldToNew, int newNodeSize, std::vector<int>& idCellsOldToNew,
                   int newCellSize);

  virtual unsigned long GetMTime();
  virtual void Update();
  virtual void UpdateInformation();

  int CellIdToDownId(int vtkCellId);
  void setCellIdToDownId(int vtkCellId, int downId);
  void BuildDownwardConnectivity(bool withEdges);
  int GetNeighbors(int* neighborsVtkIds, int* downIds, unsigned char* downTypes, int vtkId);
  void GetNodeIds(std::set<int>& nodeSet, int downId, unsigned char downType);
  void ModifyCellNodes(int vtkVolId, std::map<int, int> localClonedNodeIds);
  int getOrderedNodesOfFace(int vtkVolId, std::vector<int>& orderedNodes);
  vtkCellLinks* GetLinks()
  {
    return Links;
  }
  SMDS_Downward* getDownArray(unsigned char vtkType)
  {
    return _downArray[vtkType];
  }
  static SMDS_UnstructuredGrid* New();
  SMDS_Mesh *_mesh;
protected:
  SMDS_UnstructuredGrid();
  ~SMDS_UnstructuredGrid();
  void copyNodes(vtkPoints *newPoints, std::vector<int>& idNodesOldToNew, int& alreadyCopied, int start, int end);
  void copyBloc(vtkUnsignedCharArray *newTypes, std::vector<int>& idCellsOldToNew, std::vector<int>& idNodesOldToNew,
                vtkCellArray* newConnectivity, vtkIdTypeArray* newLocations, vtkIdType* pointsCell, int& alreadyCopied,
                int start, int end);

  std::vector<int> _cellIdToDownId; //!< convert vtk Id to downward[vtkType] id, initialized with -1
  std::vector<unsigned char> _downTypes;
  std::vector<SMDS_Downward*> _downArray;
  counters *_counters;
};

#endif	/* _SMDS_UNSTRUCTUREDGRID_HXX */

