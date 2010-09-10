#include "SMDS_VtkVolume.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_VtkCellIterator.hxx"

#include "utilities.h"

#include <vector>

SMDS_VtkVolume::SMDS_VtkVolume()
{
}

SMDS_VtkVolume::SMDS_VtkVolume(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh)
{
  init(nodeIds, mesh);
}
/*!
 * typed used are vtk types (@see vtkCellType.h)
 * see GetEntityType() for conversion in SMDS type (@see SMDSAbs_ElementType.hxx)
 */
void SMDS_VtkVolume::init(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh)
{
  vtkUnstructuredGrid* grid = mesh->getGrid();
  myIdInShape = -1;
  myMeshId = mesh->getMeshId();
  vtkIdType aType = VTK_TETRA;
  switch (nodeIds.size())
  {
    case 4:
      aType = VTK_TETRA;
      break;
    case 5:
      aType = VTK_PYRAMID;
      break;
    case 6:
      aType = VTK_WEDGE;
      break;
    case 8:
      aType = VTK_HEXAHEDRON;
      break;
    case 10:
      aType = VTK_QUADRATIC_TETRA;
      break;
    case 13:
      aType = VTK_QUADRATIC_PYRAMID;
      break;
    case 15:
      aType = VTK_QUADRATIC_WEDGE;
      break;
    case 20:
      aType = VTK_QUADRATIC_HEXAHEDRON;
      break;
    default:
      aType = VTK_HEXAHEDRON;
      break;
  }
  myVtkID = grid->InsertNextLinkedCell(aType, nodeIds.size(), &nodeIds[0]);
}

#ifdef VTK_HAVE_POLYHEDRON
void SMDS_VtkVolume::initPoly(std::vector<vtkIdType> nodeIds, std::vector<int> nbNodesPerFace, SMDS_Mesh* mesh)
{
  MESSAGE("SMDS_VtkVolume::initPoly");
  SMDS_UnstructuredGrid* grid = mesh->getGrid();
  vector<vtkIdType> ptIds;
  ptIds.clear();
  vtkIdType nbFaces = nbNodesPerFace.size();
  int k = 0;
  for (int i = 0; i < nbFaces; i++)
    {
      int nf = nbNodesPerFace[i];
      ptIds.push_back(nf);
      for (int n = 0; n < nf; n++)
        {
          ptIds.push_back(nodeIds[k]);
          k++;
        }
    }
  myVtkID = grid->InsertNextLinkedCell(VTK_POLYHEDRON, nbFaces, &ptIds[0]);
}
#endif

bool SMDS_VtkVolume::ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes)
{
  // TODO utilise dans SMDS_Mesh
  return true;
}

SMDS_VtkVolume::~SMDS_VtkVolume()
{
}

void SMDS_VtkVolume::Print(ostream & OS) const
{
  OS << "volume <" << GetID() << "> : ";
}

int SMDS_VtkVolume::NbFaces() const
{
  // TODO polyedres
  switch (NbNodes())
  {
    case 4:
    case 10:
      return 4;
    case 5:
    case 13:
      return 5;
    case 6:
    case 15:
      return 5;
    case 8:
    case 20:
      return 6;
    default:
      MESSAGE("invalid number of nodes")
      ;
  }
  return 0;
}

int SMDS_VtkVolume::NbNodes() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  int nbPoints = grid->GetCell(myVtkID)->GetNumberOfPoints();
  return nbPoints;
}

int SMDS_VtkVolume::NbEdges() const
{
  // TODO polyedres
  switch (NbNodes())
  {
    case 4:
    case 10:
      return 6;
    case 5:
    case 13:
      return 8;
    case 6:
    case 15:
      return 9;
    case 8:
    case 20:
      return 12;
    default:
      MESSAGE("invalid number of nodes")
      ;
  }
  return 0;
}

SMDS_ElemIteratorPtr SMDS_VtkVolume::elementsIterator(SMDSAbs_ElementType type) const
{
  switch (type)
  {
    case SMDSAbs_Node:
      return SMDS_ElemIteratorPtr(new SMDS_VtkCellIterator(SMDS_Mesh::_meshList[myMeshId], myVtkID, GetEntityType()));
    default:
      MESSAGE("ERROR : Iterator not implemented")
      ;
      return SMDS_ElemIteratorPtr((SMDS_ElemIterator*) NULL);
  }
}

SMDSAbs_ElementType SMDS_VtkVolume::GetType() const
{
  return SMDSAbs_Volume;
}

/*!
 * \brief Return node by its index
 * \param ind - node index
 * \retval const SMDS_MeshNode* - the node
 */
const SMDS_MeshNode* SMDS_VtkVolume::GetNode(const int ind) const
{
  return SMDS_MeshElement::GetNode(ind); // --- a optimiser !
}

bool SMDS_VtkVolume::IsQuadratic() const
{
  // TODO polyedres
  if (this->NbNodes() > 9)
    return true;
  else
    return false;
}

SMDSAbs_EntityType SMDS_VtkVolume::GetEntityType() const
{
  // TODO see SMDS_MeshElementIDFactory::GetVtkCellType
  vtkIdType aVtkType = this->GetVtkType();

  SMDSAbs_EntityType aType = SMDSEntity_Tetra;
  switch (aVtkType)
  {
    case VTK_TETRA:
      aType = SMDSEntity_Tetra;
      break;
    case VTK_PYRAMID:
      aType = SMDSEntity_Pyramid;
      break;
    case VTK_WEDGE:
      aType = SMDSEntity_Penta;
      break;
    case VTK_HEXAHEDRON:
      aType = SMDSEntity_Hexa;
      break;
    case VTK_QUADRATIC_TETRA:
      aType = SMDSEntity_Quad_Tetra;
      break;
    case VTK_QUADRATIC_PYRAMID:
      aType = SMDSEntity_Quad_Pyramid;
      break;
    case VTK_QUADRATIC_WEDGE:
      aType = SMDSEntity_Quad_Penta;
      break;
    case VTK_QUADRATIC_HEXAHEDRON:
      aType = SMDSEntity_Quad_Hexa;
      break;
#ifdef VTK_HAVE_POLYHEDRON
    case VTK_POLYHEDRON:
      aType = SMDSEntity_Polyhedra;
      break;
#endif
    default:
      aType = SMDSEntity_Polyhedra;
      break;
  }
  return aType;
}

vtkIdType SMDS_VtkVolume::GetVtkType() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aType = grid->GetCellType(myVtkID);
  return aType;
}
