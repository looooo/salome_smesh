// File:      StdMeshers_Helper.hxx
// Created:   15.02.06 14:48:09
// Author:    Sergey KUUL
// Copyright: Open CASCADE 2006


#ifndef StdMeshers_Helper_HeaderFile
#define StdMeshers_Helper_HeaderFile

#include <SMESH_Mesh.hxx>
#include <TopoDS_Shape.hxx>
#include <SMDS_MeshNode.hxx>
#include <TopoDS_Face.hxx>
#include <gp_XY.hxx>

#include <map>

typedef pair<const SMDS_MeshNode*, const SMDS_MeshNode*> NLink;
typedef map<NLink, const SMDS_MeshNode*> NLinkNodeMap;
typedef map<NLink, const SMDS_MeshNode*>::iterator ItNLinkNode;

/*!
 * \brief It helps meshers to add elements
 *
 * It allow meshers not to care about creation of medium nodes
 * when filling a quadratic mesh. Helper does it itself.
 * It defines degree of elements to create when IsQuadraticSubMesh()
 * is called.
 */

class StdMeshers_Helper 
{
 public:
  // ---------- PUBLIC METHODS ----------

  /// Empty constructor
  StdMeshers_Helper(SMESH_Mesh& theMesh)
    { myMesh=(void *)&theMesh; myCreateQuadratic = false; }

  SMESH_Mesh* GetMesh() const
    { return (SMESH_Mesh*)myMesh; }
    
  /// Copy constructor
  //Standard_EXPORT StdMeshers_Helper (const StdMeshers_Helper& theOther);

  /// Destructor
  //Standard_EXPORT virtual ~StdMeshers_Helper ();

  /**
   * Check submesh for given shape
   * Check if all elements on this shape
   * are quadratic, if yes => set true to myCreateQuadratic 
   * (default value is false). Also fill myNLinkNodeMap
   * Returns myCreateQuadratic
   */
  bool IsQuadraticSubMesh(const TopoDS_Shape& theShape);

  /*!
   * \brief Returns true if given node is medium
    * \param n - node to check
    * \param typeToCheck - type of elements containing the node to ask about node status
    * \retval bool - check result
   */
  static bool IsMedium(const SMDS_MeshNode*      node,
                       const SMDSAbs_ElementType typeToCheck = SMDSAbs_All);

  /**
   * Auxilary function for filling myNLinkNodeMap
   */
  void AddNLinkNode(const SMDS_MeshNode* n1,
                    const SMDS_MeshNode* n2,
                    const SMDS_MeshNode* n12);

  /**
   * Auxilary function for filling myNLinkNodeMap
   */
  void AddNLinkNodeMap(const NLinkNodeMap& aMap)
    { myNLinkNodeMap.insert(aMap.begin(), aMap.end()); }

  /**
   * Returns myNLinkNodeMap
   */
  const NLinkNodeMap& GetNLinkNodeMap() { return myNLinkNodeMap; }

  /**
   * Auxilary function for GetMediumNode()
   */
  gp_XY GetNodeUV(const TopoDS_Face& F,
                  const SMDS_MeshNode* n);

  /**
   * Special function for search or creation medium node
   */
  const SMDS_MeshNode* GetMediumNode(const SMDS_MeshNode* n1,
                                     const SMDS_MeshNode* n2,
                                     const bool force3d);
  /**
   * Special function for creation quadratic triangle
   */
  SMDS_MeshFace* AddFace(const SMDS_MeshNode* n1,
                         const SMDS_MeshNode* n2,
                         const SMDS_MeshNode* n3);

  /**
   * Special function for creation quadratic quadrangle
   */
  SMDS_MeshFace* AddFace(const SMDS_MeshNode* n1,
                         const SMDS_MeshNode* n2,
                         const SMDS_MeshNode* n3,
                         const SMDS_MeshNode* n4);

  /**
   * Special function for creation quadratic tetraahedron
   */
  SMDS_MeshVolume* AddVolume(const SMDS_MeshNode* n1,
                             const SMDS_MeshNode* n2,
                             const SMDS_MeshNode* n3,
                             const SMDS_MeshNode* n4);

  /**
   * Special function for creation quadratic pentahedron
   */
  SMDS_MeshVolume* AddVolume(const SMDS_MeshNode* n1,
                             const SMDS_MeshNode* n2,
                             const SMDS_MeshNode* n3,
                             const SMDS_MeshNode* n4,
                             const SMDS_MeshNode* n5,
                             const SMDS_MeshNode* n6);

  /**
   * Special function for creation quadratic hexahedron
   */
  SMDS_MeshVolume* AddVolume(const SMDS_MeshNode* n1,
                             const SMDS_MeshNode* n2,
                             const SMDS_MeshNode* n3,
                             const SMDS_MeshNode* n4,
                             const SMDS_MeshNode* n5,
                             const SMDS_MeshNode* n6,
                             const SMDS_MeshNode* n7,
                             const SMDS_MeshNode* n8);



 private:

  void* myMesh;

  int myShapeID;

  // Key for creation quadratic faces
  bool myCreateQuadratic;

  // special map for using during creation quadratic faces
  NLinkNodeMap myNLinkNodeMap;

};


#endif
