// File generated by CPPExt (Transient)
//
//                     Copyright (C) 1991,1995 by
//  
//                      MATRA DATAVISION, FRANCE
//  
// This software is furnished in accordance with the terms and conditions
// of the contract and with the inclusion of the above copyright notice.
// This software or any other copy thereof may not be provided or otherwise
// be made available to any other person. No title to an ownership of the
// software is hereby transferred.
//  
// At the termination of the contract, the software and all copies of this
// software must be deleted.
//
#ifndef _SMDS_Mesh_HeaderFile
#define _SMDS_Mesh_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif

#ifndef _SMDS_MapOfMeshOrientedElement_HeaderFile
#include "SMDS_MapOfMeshOrientedElement.hxx"
#endif
#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
#ifndef _SMDS_ListOfMesh_HeaderFile
#include "SMDS_ListOfMesh.hxx"
#endif
#ifndef _Handle_SMDS_MeshNodeIDFactory_HeaderFile
#include "Handle_SMDS_MeshNodeIDFactory.hxx"
#endif
#ifndef _Handle_SMDS_MeshElementIDFactory_HeaderFile
#include "Handle_SMDS_MeshElementIDFactory.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include "Standard_Boolean.hxx"
#endif
#ifndef _SMDS_MeshObject_HeaderFile
#include "SMDS_MeshObject.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _Handle_SMDS_MeshNode_HeaderFile
#include "Handle_SMDS_MeshNode.hxx"
#endif
class SMDS_MeshNodeIDFactory;
class SMDS_MeshElementIDFactory;
class Standard_NoSuchObject;
class SMDS_MeshVolumesIterator;
class SMDS_MeshFacesIterator;
class SMDS_MeshEdgesIterator;
class SMDS_MeshNodesIterator;
class SMDS_MeshElement;
class SMDS_MeshNode;
class SMDS_ListIteratorOfListOfMesh;
class SMDS_MapOfMeshOrientedElement;


class SMDS_Mesh : public SMDS_MeshObject {

public:

    inline void* operator new(size_t,void* anAddress) 
      {
        return anAddress;
      }
    inline void* operator new(size_t size) 
      { 
        return Standard::Allocate(size); 
      }
    inline void  operator delete(void *anAddress) 
      { 
        if (anAddress) Standard::Free((Standard_Address&)anAddress); 
      }
//    inline void  operator delete(void *anAddress, size_t size) 
//      { 
//        if (anAddress) Standard::Free((Standard_Address&)anAddress,size); 
//      }
 // Methods PUBLIC
 // 
Standard_EXPORT SMDS_Mesh(const Standard_Integer nbnodes = 10,const Standard_Integer nbedges = 10,const Standard_Integer nbfaces = 10,const Standard_Integer nbvolumes = 10);
Standard_EXPORT   Handle_SMDS_Mesh AddSubMesh() ;
Standard_EXPORT virtual  Standard_Integer AddNode(const Standard_Real x,const Standard_Real y,const Standard_Real z) ;
Standard_EXPORT virtual  Standard_Boolean AddNodeWithID(const Standard_Real x,const Standard_Real y,const Standard_Real z,const Standard_Integer ID) ;
Standard_EXPORT virtual  Standard_Boolean AddNode(const Standard_Integer ID) ;
Standard_EXPORT virtual  Standard_Integer AddEdge(const Standard_Integer idnode1,const Standard_Integer idnode2) ;
Standard_EXPORT virtual  Standard_Boolean AddEdgeWithID(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer ID) ;
Standard_EXPORT virtual  Standard_Integer AddFace(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3) ;
Standard_EXPORT virtual  Standard_Boolean AddFaceWithID(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer ID) ;
Standard_EXPORT virtual  Standard_Integer AddFace(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) ;
Standard_EXPORT virtual  Standard_Boolean AddFaceWithID(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer ID) ;
Standard_EXPORT virtual  Standard_Integer AddVolume(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) ;
Standard_EXPORT virtual  Standard_Boolean AddVolumeWithID(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer ID) ;
Standard_EXPORT virtual  Standard_Integer AddVolume(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5) ;
Standard_EXPORT virtual  Standard_Boolean AddVolumeWithID(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer ID) ;
Standard_EXPORT virtual  Standard_Integer AddVolume(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6) ;
Standard_EXPORT virtual  Standard_Boolean AddVolumeWithID(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6,const Standard_Integer ID) ;
Standard_EXPORT virtual  Standard_Integer AddVolume(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6,const Standard_Integer idnode7,const Standard_Integer idnode8) ;
Standard_EXPORT virtual  Standard_Boolean AddVolumeWithID(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6,const Standard_Integer idnode7,const Standard_Integer idnode8,const Standard_Integer ID) ;
Standard_EXPORT   Handle_SMDS_MeshElement AddVolume(const Handle(SMDS_MeshElement)& node1,const Handle(SMDS_MeshElement)& node2,const Handle(SMDS_MeshElement)& node3,const Handle(SMDS_MeshElement)& node4) ;
Standard_EXPORT   Handle_SMDS_MeshElement AddVolume(const Handle(SMDS_MeshElement)& node1,const Handle(SMDS_MeshElement)& node2,const Handle(SMDS_MeshElement)& node3,const Handle(SMDS_MeshElement)& node4,const Handle(SMDS_MeshElement)& node5) ;
Standard_EXPORT   Handle_SMDS_MeshElement AddVolume(const Handle(SMDS_MeshElement)& node1,const Handle(SMDS_MeshElement)& node2,const Handle(SMDS_MeshElement)& node3,const Handle(SMDS_MeshElement)& node4,const Handle(SMDS_MeshElement)& node5,const Handle(SMDS_MeshElement)& node6) ;
Standard_EXPORT   Handle_SMDS_MeshElement AddVolume(const Handle(SMDS_MeshElement)& node1,const Handle(SMDS_MeshElement)& node2,const Handle(SMDS_MeshElement)& node3,const Handle(SMDS_MeshElement)& node4,const Handle(SMDS_MeshElement)& node5,const Handle(SMDS_MeshElement)& node6,const Handle(SMDS_MeshElement)& node7,const Handle(SMDS_MeshElement)& node8) ;
Standard_EXPORT virtual  void RemoveNode(const Standard_Integer IDnode) ;
Standard_EXPORT virtual  void RemoveEdge(const Standard_Integer idnode1,const Standard_Integer idnode2) ;
Standard_EXPORT virtual  void RemoveFace(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3) ;
Standard_EXPORT virtual  void RemoveFace(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) ;
Standard_EXPORT virtual  void RemoveElement(const Standard_Integer IDelem,const Standard_Boolean removenodes = Standard_False) ;
Standard_EXPORT   void RemoveElement(const Handle(SMDS_MeshElement)& elem,const Standard_Boolean removenodes = Standard_False) ;
Standard_EXPORT virtual  Standard_Boolean RemoveFromParent() ;
Standard_EXPORT virtual  Standard_Boolean RemoveSubMesh(const Handle(SMDS_Mesh)& aMesh) ;
Standard_EXPORT   Handle_SMDS_MeshNode GetNode(const Standard_Integer rank,const Handle(SMDS_MeshElement)& ME) const;
Standard_EXPORT   Handle_SMDS_MeshElement FindNode(const Standard_Integer idnode) const;
Standard_EXPORT   Handle_SMDS_MeshElement FindEdge(const Standard_Integer idnode1,const Standard_Integer idnode2) const;
Standard_EXPORT   Handle_SMDS_MeshElement FindFace(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3) const;
Standard_EXPORT   Handle_SMDS_MeshElement FindFace(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) const;
Standard_EXPORT   Handle_SMDS_MeshElement FindElement(const Standard_Integer IDelem) const;
Standard_EXPORT   Standard_Boolean Contains(const Handle(SMDS_MeshElement)& elem) const;
Standard_EXPORT   void RebuildAllInverseConnections() ;
Standard_EXPORT   void SubMeshIterator(SMDS_ListIteratorOfListOfMesh& itmsh) const;
Standard_EXPORT inline   Standard_Integer NbNodes() const;
Standard_EXPORT inline   Standard_Integer NbEdges() const;
Standard_EXPORT inline   Standard_Integer NbFaces() const;
Standard_EXPORT inline   Standard_Integer NbVolumes() const;
Standard_EXPORT inline   Standard_Integer NbSubMesh() const;
Standard_EXPORT   void DumpNodes() const;
Standard_EXPORT   void DumpEdges() const;
Standard_EXPORT   void DumpFaces() const;
Standard_EXPORT   void DumpVolumes() const;
Standard_EXPORT   void DebugStats() const;
Standard_EXPORT   Handle_SMDS_MeshElement CreateEdge(const Standard_Integer ID,const Standard_Integer idnode1,const Standard_Integer idnode2) const;
Standard_EXPORT   Handle_SMDS_MeshElement CreateFace(const Standard_Integer ID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3) const;
Standard_EXPORT   Handle_SMDS_MeshElement CreateFace(const Standard_Integer ID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) const;
Standard_EXPORT   Handle_SMDS_MeshElement CreateVolume(const Standard_Integer ID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) const;
Standard_EXPORT   Handle_SMDS_MeshElement CreateVolume(const Standard_Integer ID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5) const;
Standard_EXPORT   Handle_SMDS_MeshElement CreateVolume(const Standard_Integer ID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6) const;
Standard_EXPORT   Handle_SMDS_MeshElement CreateVolume(const Standard_Integer ID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6,const Standard_Integer idnode7,const Standard_Integer idnode8) const;
Standard_EXPORT ~SMDS_Mesh();


friend class SMDS_MeshVolumesIterator;
friend class SMDS_MeshFacesIterator;
friend class SMDS_MeshEdgesIterator;
friend class SMDS_MeshNodesIterator;


 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_Mesh_Type_();
 Standard_EXPORT const Handle(Standard_Type)& DynamicType() const;
 Standard_EXPORT Standard_Boolean	       IsKind(const Handle(Standard_Type)&) const;

protected:

 // Methods PROTECTED
 // 
Standard_EXPORT   Handle_SMDS_MeshElement FindNode(const Handle(SMDS_MeshElement)& node) const;
Standard_EXPORT   Handle_SMDS_MeshElement FindEdge(const Handle(SMDS_MeshElement)& edge) const;
Standard_EXPORT   Handle_SMDS_MeshElement FindFace(const Handle(SMDS_MeshElement)& face) const;
Standard_EXPORT   Handle_SMDS_MeshElement FindVolume(const Handle(SMDS_MeshElement)& volume) const;
Standard_EXPORT   void RemoveInverseElement(const Handle(SMDS_MeshElement)& node,const Handle(SMDS_MeshElement)& parent) const;
Standard_EXPORT   Standard_Boolean RemoveNode(const Handle(SMDS_MeshElement)& node) ;
Standard_EXPORT   void RemoveEdge(const Handle(SMDS_MeshElement)& edge) ;
Standard_EXPORT   void RemoveFace(const Handle(SMDS_MeshElement)& face) ;
Standard_EXPORT   void RemoveVolume(const Handle(SMDS_MeshElement)& volume) ;
Standard_EXPORT   Handle_SMDS_MeshElement GetNode(const Standard_Integer ID) const;
Standard_EXPORT   void RemoveAncestors(const Handle(SMDS_MeshElement)& ME,const SMDS_MapOfMeshOrientedElement& map) ;


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 
Standard_EXPORT SMDS_Mesh(const Handle(SMDS_Mesh)& parent,const Standard_Integer nbnodes = 10);
Standard_EXPORT   Standard_Boolean AddNode(const Handle(SMDS_MeshElement)& node) ;
Standard_EXPORT   void FreeNode(const Handle(SMDS_MeshElement)& node) ;
Standard_EXPORT   void BuildMapNodeAncestors(const Handle(SMDS_MeshElement)& ME,SMDS_MapOfMeshOrientedElement& map) const;
Standard_EXPORT   void BuildMapEdgeAncestors(const Handle(SMDS_MeshElement)& ME,SMDS_MapOfMeshOrientedElement& map) const;
Standard_EXPORT   void BuildMapFaceAncestors(const Handle(SMDS_MeshElement)& ME,SMDS_MapOfMeshOrientedElement& map) const;
Standard_EXPORT   void RebuildInverseConnections() ;


 // Fields PRIVATE
 //
SMDS_MapOfMeshOrientedElement myNodes;
SMDS_MapOfMeshOrientedElement myEdges;
SMDS_MapOfMeshOrientedElement myFaces;
SMDS_MapOfMeshOrientedElement myVolumes;
Handle_SMDS_Mesh myParent;
SMDS_ListOfMesh myChildren;
Handle_SMDS_MeshNodeIDFactory myNodeIDFactory;
Handle_SMDS_MeshElementIDFactory myElementIDFactory;
Standard_Boolean myHasInverse;


};


#include "SMDS_Mesh.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
