//  Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File      : StdMeshers_ViscousLayers.cxx
// Created   : Wed Dec  1 15:15:34 2010
// Author    : Edward AGAPOV (eap)

#include "StdMeshers_ViscousLayers.hxx"

#include "SMDS_EdgePosition.hxx"
#include "SMDS_FaceOfNodes.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_Hypothesis.hxx"
#include "SMESH_Algo.hxx"
#include "SMESH_ComputeError.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "StdMeshers_ProxyMesh.hxx"

#include "utilities.h"

#include <BRep_Tool.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Ax1.hxx>
#include <gp_Vec.hxx>
#include <gp_XY.hxx>
#include <gp_XYZ.hxx>

#include <list>
#include <string>
#include <math.h>

using namespace std;

//================================================================================
namespace VISCOUS
{
  typedef int TGeomID;

  enum UIndex { U_TGT = 1, U_SRC, LEN_TGT };

  /*!
   * \brief StdMeshers_ProxyMesh computed by _ViscousBuilder for a solid.
   * It is stored in a SMESH_subMesh of the SOLID as SMESH_subMeshEventListenerData
   */
  struct _MeshOfSolid : public StdMeshers_ProxyMesh,
                        public SMESH_subMeshEventListenerData
  {
    _MeshOfSolid( SMESH_Mesh* mesh):SMESH_subMeshEventListenerData( /*isDeletable=*/true)
    {
      StdMeshers_ProxyMesh::setMesh( *mesh );
    }

    // returns submesh for a geom face
    StdMeshers_ProxyMesh::SubMesh* getFaceData(const TopoDS_Face& F, bool create=false)
    {
      TGeomID i = StdMeshers_ProxyMesh::shapeIndex(F);
      return create ? StdMeshers_ProxyMesh::getProxySubMesh(i) : findProxySubMesh(i);
    }
  };
  //--------------------------------------------------------------------------------
  /*!
   * \brief Listener of events of 3D sub-meshes computed with viscous layers.
   * It is used to clear an inferior dim sub-mesh modified by viscous layers
   */
  class _SrinkShapeListener : SMESH_subMeshEventListener
  {
    _SrinkShapeListener(): SMESH_subMeshEventListener(/*isDeletable=*/false) {}
    static SMESH_subMeshEventListener* Get() { static _SrinkShapeListener l; return &l; }
  public:
    virtual void ProcessEvent(const int                       event,
                              const int                       eventType,
                              SMESH_subMesh*                  solidSM,
                              SMESH_subMeshEventListenerData* data,
                              const SMESH_Hypothesis*         hyp)
    {
      if ( SMESH_subMesh::COMPUTE_EVENT == eventType && solidSM->IsEmpty() && data )
      {
        SMESH_subMeshEventListener::ProcessEvent(event,eventType,solidSM,data,hyp);
      }
    }
    static void ToClearSubMeshWithSolid( SMESH_subMesh*      sm,
                                         const TopoDS_Shape& solid)
    {
      SMESH_subMesh* solidSM = sm->GetFather()->GetSubMesh( solid );
      SMESH_subMeshEventListenerData* data = solidSM->GetEventListenerData( Get());
      if ( data )
      {
        if ( find( data->mySubMeshes.begin(), data->mySubMeshes.end(), sm ) ==
             data->mySubMeshes.end())
          data->mySubMeshes.push_back( sm );
      }
      else
      {
        data = SMESH_subMeshEventListenerData::MakeData( /*dependent=*/sm );
        sm->SetEventListener( Get(), data, /*whereToListenTo=*/solidSM );
      }
    }
  };
  //--------------------------------------------------------------------------------
  /*!
   * \brief Listener of events of 3D sub-meshes computed with viscous layers.
   * It is used to store data computed by _ViscousBuilder for a sub-mesh and to
   * delete the data as soon as it has been used
   */
  class _ViscousListener : SMESH_subMeshEventListener
  {
    _ViscousListener(): SMESH_subMeshEventListener(/*isDeletable=*/false) {}
    static SMESH_subMeshEventListener* Get() { static _ViscousListener l; return &l; }
  public:
    virtual void ProcessEvent(const int                       event,
                              const int                       eventType,
                              SMESH_subMesh*                  subMesh,
                              SMESH_subMeshEventListenerData* data,
                              const SMESH_Hypothesis*         hyp)
    {
      if ( SMESH_subMesh::COMPUTE_EVENT == eventType )
      {
        // delete StdMeshers_ProxyMesh containing temporary faces
        subMesh->DeleteEventListener( this );
      }
    }
    // Finds or creates proxy mesh of the solid
    static _MeshOfSolid* GetSolidMesh(SMESH_Mesh*         mesh,
                                      const TopoDS_Shape& solid,
                                      bool                toCreate=false)
    {
      if ( !mesh ) return 0;
      SMESH_subMesh* sm = mesh->GetSubMesh(solid);
      _MeshOfSolid* data = (_MeshOfSolid*) sm->GetEventListenerData( Get() );
      if ( !data && toCreate )
        ( data = new _MeshOfSolid(mesh)), sm->SetEventListener( Get(), data, sm );
      return data;
    }
    // Removes proxy mesh of the solid
    static void RemoveSolidMesh(SMESH_Mesh* mesh, const TopoDS_Shape& solid)
    {
      mesh->GetSubMesh(solid)->DeleteEventListener( _ViscousListener::Get() );
    }
  };
  
  //--------------------------------------------------------------------------------
  /*!
   * \brief Simplex (triangle or tetrahedron) based on 1 (tria) or 2 (tet) nodes of
   * _LayerEdge and 2 nodes of the mesh surface beening smoothed.
   * The class is used to check validity of face or volumes around a smoothed node;
   * it stores only 2 nodes as the other nodes are stored by _LayerEdge.
   */
  struct _Simplex
  {
    const SMDS_MeshNode *_nPrev, *_nNext; // nodes on a smoothed mesh surface
    _Simplex(const SMDS_MeshNode* nPrev=0, const SMDS_MeshNode* nNext=0)
      : _nPrev(nPrev), _nNext(nNext) {}
    bool IsForward(const SMDS_MeshNode* nSrc, const gp_XYZ* pntTgt) const
    {
      const double M[3][3] =
        {{ _nNext->X() - nSrc->X(), _nNext->Y() - nSrc->Y(), _nNext->Z() - nSrc->Z() },
         { pntTgt->X() - nSrc->X(), pntTgt->Y() - nSrc->Y(), pntTgt->Z() - nSrc->Z() },
         { _nPrev->X() - nSrc->X(), _nPrev->Y() - nSrc->Y(), _nPrev->Z() - nSrc->Z() }};
      double determinant = ( + M[0][0]*M[1][1]*M[2][2]
                             + M[0][1]*M[1][2]*M[2][0]
                             + M[0][2]*M[1][0]*M[2][1]
                             - M[0][0]*M[1][2]*M[2][1]
                             - M[0][1]*M[1][0]*M[2][2]
                             - M[0][2]*M[1][1]*M[2][0]);
      return determinant > 1e-100;
    }
    bool IsForward(const gp_XY&        tgtUV,
                   const TopoDS_Face&  face,
                   SMESH_MesherHelper& helper,
                   const double        refSign) const
    {
      gp_XY prevUV = helper.GetNodeUV( face, _nPrev );
      gp_XY nextUV = helper.GetNodeUV( face, _nNext );
      gp_Vec2d v1( tgtUV, prevUV ), v2( tgtUV, nextUV );
      double d = v1 ^ v2;
      return d*refSign > 1e-100;
    }
  };
  //--------------------------------------------------------------------------------
  /*!
   * Structure used to smooth a _LayerEdge (master) based on an EDGE.
   */
  struct _2NearEdges
  {
    // target nodes of 2 neighbour _LayerEdge's based on the same EDGE
    const SMDS_MeshNode* _nodes[2];
    // vectors from source nodes of 2 _LayerEdge's to the source node of master _LayerEdge
    gp_XYZ               _vec[2];

    _2NearEdges() { _nodes[0]=_nodes[1]=0; }
  };
  //--------------------------------------------------------------------------------
  /*!
   * \brief Edge normal to surface, connecting a node on solid surface (_nodes[0])
   * and a node of the most internal layer (_nodes.back())
   */
  struct _LayerEdge
  {
    vector< const SMDS_MeshNode*> _nodes;

    gp_XYZ              _normal; // to solid surface
    vector<gp_XYZ>      _pos; // points computed during inflation
    double              _len; // length achived with the last step
    double              _cosin; // of angle (_normal ^ surface)

    // face or edge w/o layer along or near which _LayerEdge is inflated
    TopoDS_Shape        _sWOL;
    // simplices connected to _nodes[0]; for smoothing and quality check
    // of _LayerEdge's based on the FACE
    vector<_Simplex>    _simplices;
    // data for smoothing of _LayerEdge's based on the EDGE
    _2NearEdges*        _2neibors;

    void SetNewLength( double len, SMESH_MesherHelper& helper );
    bool SetNewLength2d( Handle(Geom_Surface)& surface,
                         const TopoDS_Face&    F,
                         SMESH_MesherHelper&   helper );
    void InvalidateStep( int curStep );
    bool Smooth(int& badNb);
    bool SmoothOnEdge(Handle(Geom_Surface)& surface,
                      const TopoDS_Face&    F,
                      SMESH_MesherHelper&   helper);
    bool SegTriaInter( const gp_Ax1&        lastSegment,
                       const SMDS_MeshNode* n0,
                       const SMDS_MeshNode* n1,
                       const SMDS_MeshNode* n2 ) const;
    gp_Ax1 LastSegment() const;
    bool IsOnEdge() const { return _2neibors; }
  };
  //--------------------------------------------------------------------------------

  typedef map< const SMDS_MeshNode*, _LayerEdge*, TIDCompare > TNode2Edge;
  
  //--------------------------------------------------------------------------------
  /*!
   * \brief Data of a SOLID
   */
  struct _SolidData
  {
    TopoDS_Shape                    _solid;
    const StdMeshers_ViscousLayers* _hyp;
    _MeshOfSolid*                   _proxyMesh;
    set<TGeomID>                    _reversedFaceIds;

    double                          _stepSize;
    const SMDS_MeshNode*            _stepSizeNodes[2];

    TNode2Edge                      _n2eMap;
    // edges of _n2eMap. We keep same data in two containers because
    // iteration over the map is 5 time longer that over the vector
    vector< _LayerEdge* >           _edges;

    // key: an id of shape (edge or vertex) shared by a FACE with
    // layers and a FACE w/o layers
    // value: the shape (FACE or EDGE) to shrink mesh on.
    // _LayerEdge's basing on nodes on key shape are inflated along the value shape
    map< TGeomID, TopoDS_Shape >     _shrinkShape2Shape;

    // end index in _edges of _LayerEdge's based on EDGE (map key) to
    // FACE (maybe NULL) they are inflated along
    map< int, TopoDS_Face >          _endEdge2Face;

    int                              _index; // for debug

    _SolidData(const TopoDS_Shape&             s=TopoDS_Shape(),
               const StdMeshers_ViscousLayers* h=0,
               _MeshOfSolid*                   m=0) :_solid(s), _hyp(h), _proxyMesh(m) {}
    ~_SolidData();
  };
  //--------------------------------------------------------------------------------
  /*!
   * \brief Data of node on a shrinked FACE
   */
  struct _SmoothNode
  {
    const SMDS_MeshNode*         _node;
    //vector<const SMDS_MeshNode*> _nodesAround;
    vector<_Simplex>             _simplices; // for quality check

    bool Smooth(int&                  badNb,
                Handle(Geom_Surface)& surface,
                SMESH_MesherHelper&   helper,
                const double          refSign,
                bool                  set3D);
  };
  //--------------------------------------------------------------------------------
  /*!
   * \brief Builder of viscous layers
   */
  class _ViscousBuilder
  {
  public:
    _ViscousBuilder();
    // does it's job
    SMESH_ComputeErrorPtr Compute(SMESH_Mesh& mesh, const TopoDS_Shape& theShape);

    // restore event listeners used to clear an inferior dim sub-mesh modified by viscous layers
    void RestoreListeners();

  private:

    bool findSolidsWithLayers();
    bool findFacesWithLayers();
    bool makeLayer(_SolidData& data);
    bool setEdgeData(_LayerEdge& edge, const set<TGeomID>& subIds,
                     SMESH_MesherHelper& helper, _SolidData& data);
    void getSimplices( const SMDS_MeshNode* node, vector<_Simplex>& simplices,
                       const set<TGeomID>& ingnoreShapes,
                       const _SolidData* dataToCheckOri = 0);
    bool inflate(_SolidData& data);
    bool smoothAndCheck(_SolidData& data, int nbSteps);
    bool refine(_SolidData& data);
    bool shrink();
    bool prepareEdgeToShrink( _LayerEdge& edge, const TopoDS_Face& F,
                              SMESH_MesherHelper& helper,
                              const SMESHDS_SubMesh* faceSubMesh );

    bool error( const string& text, _SolidData* data );
    SMESHDS_Mesh* getMeshDS() { return _mesh->GetMeshDS(); }

    // debug
    void makeGroupOfLE();

    SMESH_Mesh*           _mesh;
    SMESH_ComputeErrorPtr _error;

    vector< _SolidData >  _sdVec;
    set<TGeomID>          _ignoreShapeIds;
  };
  //--------------------------------------------------------------------------------
  /*!
   * \brief Shrinker of nodes on the EDGE
   */
  class _Shrinker1D
  {
    vector<double>                _initU;
    vector<double>                _normPar;
    vector<const SMDS_MeshNode*>  _nodes;
    const _LayerEdge*             _edges[2];
    bool                          _done;
  public:
    void AddEdge( const _LayerEdge* e, SMESH_MesherHelper& helper );
    void Compute(bool set3D, SMESH_MesherHelper& helper);
    void RestoreParams();
    void SwapSrcTgt(SMESHDS_Mesh* mesh);
  };
}

//================================================================================
// StdMeshers_ViscousLayers hypothesis
//
StdMeshers_ViscousLayers::StdMeshers_ViscousLayers(int hypId, int studyId, SMESH_Gen* gen)
  :SMESH_Hypothesis(hypId, studyId, gen),
   _nbLayers(1), _thickness(1), _stretchFactor(1)
{
  _name = StdMeshers_ViscousLayers::GetHypType();
  _param_algo_dim = -3; // auxiliary hyp used by 3D algos
} // --------------------------------------------------------------------------------
void StdMeshers_ViscousLayers::SetIgnoreFaces(const std::vector<int>& faceIds)
{
  if ( faceIds != _ignoreFaceIds )
    _ignoreFaceIds = faceIds, NotifySubMeshesHypothesisModification();
} // --------------------------------------------------------------------------------
void StdMeshers_ViscousLayers::SetTotalThickness(double thickness)
{
  if ( thickness != _thickness )
    _thickness = thickness, NotifySubMeshesHypothesisModification();
} // --------------------------------------------------------------------------------
void StdMeshers_ViscousLayers::SetNumberLayers(int nb)
{
  if ( _nbLayers != nb )
    _nbLayers = nb, NotifySubMeshesHypothesisModification();
} // --------------------------------------------------------------------------------
void StdMeshers_ViscousLayers::SetStretchFactor(double factor)
{
  if ( _stretchFactor != factor )
    _stretchFactor = factor, NotifySubMeshesHypothesisModification();
} // --------------------------------------------------------------------------------
StdMeshers_ProxyMesh::Ptr StdMeshers_ViscousLayers::Compute(SMESH_Mesh&         theMesh,
                                                            const TopoDS_Shape& theShape) const
{
  using namespace VISCOUS;
  _ViscousBuilder bulder;
  SMESH_ComputeErrorPtr err = bulder.Compute( theMesh, theShape );
  if ( err && !err->IsOK() )
    return StdMeshers_ProxyMesh::Ptr();

  vector<StdMeshers_ProxyMesh::Ptr> components;
  TopExp_Explorer exp( theShape, TopAbs_SOLID );
  for ( ; exp.More(); exp.Next() )
  {
    if ( _MeshOfSolid* pm =
         _ViscousListener::GetSolidMesh( &theMesh, exp.Current(), /*toCreate=*/false))
    {
      components.push_back( StdMeshers_ProxyMesh::Ptr( pm ));
      pm->myIsDeletable  =false; // it will de deleted by boost::shared_ptr
    }
    _ViscousListener::RemoveSolidMesh ( &theMesh, exp.Current() );
  }
  switch ( components.size() )
  {
  case 0: break;

  case 1: return components[0];

  default: return StdMeshers_ProxyMesh::Ptr( new StdMeshers_ProxyMesh( components ));
  }
  return StdMeshers_ProxyMesh::Ptr();
} // --------------------------------------------------------------------------------
std::ostream & StdMeshers_ViscousLayers::SaveTo(std::ostream & save)
{
  save << " " << _nbLayers
       << " " << _thickness
       << " " << _stretchFactor
       << " " << _ignoreFaceIds.size();
  for ( unsigned i = 0; i < _ignoreFaceIds.size(); ++i )
    save << " " << _ignoreFaceIds[i];
  return save;
} // --------------------------------------------------------------------------------
std::istream & StdMeshers_ViscousLayers::LoadFrom(std::istream & load)
{
  int nbFaces, faceID;
  load >> _nbLayers >> _thickness >> _stretchFactor >> nbFaces;
  while ( _ignoreFaceIds.size() < nbFaces && load >> faceID )
    _ignoreFaceIds.push_back( faceID );
  return load;
} // --------------------------------------------------------------------------------
bool StdMeshers_ViscousLayers::SetParametersByMesh(const SMESH_Mesh*   theMesh,
                                                   const TopoDS_Shape& theShape)
{
  // TODO
  return false;
}
// END StdMeshers_ViscousLayers hypothesis
//================================================================================

namespace
{
  gp_XYZ getEdgeDir( const TopoDS_Edge& E, const TopoDS_Vertex& fromV )
  {
    gp_Vec dir;
    double f,l;
    Handle(Geom_Curve) c = BRep_Tool::Curve( E, f, l );
    gp_Pnt p = BRep_Tool::Pnt( fromV );
    double distF = p.SquareDistance( c->Value( f ));
    double distL = p.SquareDistance( c->Value( l ));
    c->D1(( distF < distL ? f : l), p, dir );
    if ( distL < distF ) dir.Reverse();
    return dir.XYZ();
  }
  //--------------------------------------------------------------------------------
  gp_XYZ getFaceDir( const TopoDS_Face& F, const TopoDS_Vertex& fromV,
                     SMESH_MesherHelper& helper, bool& ok, double* cosin=0)
  {
    double f,l; TopLoc_Location loc;
    vector< TopoDS_Edge > edges; // sharing a vertex
    PShapeIteratorPtr eIt = helper.GetAncestors( fromV, *helper.GetMesh(), TopAbs_EDGE);
    while ( eIt->more())
    {
      const TopoDS_Edge* e = static_cast<const TopoDS_Edge*>( eIt->next() );
      if ( helper.IsSubShape( *e, F ) && BRep_Tool::Curve( *e, loc,f,l))
        edges.push_back( *e );
    }
    gp_XYZ dir(0,0,0);
    gp_Vec edgeDir;
    ok = ( edges.size() == 2 );
    for ( unsigned i = 0; i < edges.size(); ++i )
    {
      edgeDir = getEdgeDir( edges[i], fromV );
      double size2 = edgeDir.SquareMagnitude();
      if ( size2 > numeric_limits<double>::min() )
        edgeDir /= sqrt( size2 );
      else
        ok = false;
      dir += edgeDir.XYZ();
    }
    if ( ok )
    {
      dir /= edges.size();
      if ( cosin ) {
        double angle = edgeDir.Angle( dir );
        *cosin = cos( angle );
      }
    }
    return dir;
  }
  //--------------------------------------------------------------------------------
  gp_XYZ getFaceDir( const TopoDS_Face& F, const TopoDS_Edge& fromE,
                     const SMDS_MeshNode* node, SMESH_MesherHelper& helper, bool& ok)
  {
    gp_XY uv = helper.GetNodeUV( F, node, 0, &ok );
    Handle(Geom_Surface) surface = BRep_Tool::Surface( F );
    gp_Pnt p; gp_Vec du, dv, norm;
    surface->D1( uv.X(),uv.Y(), p, du,dv );
    norm = du ^ dv;
    double f,l;
    Handle(Geom_Curve) c = BRep_Tool::Curve( fromE, f, l );
    f = helper.GetNodeU( fromE, node, 0, &ok );
    c->D1( f, p, du );
    TopAbs_Orientation o = helper.GetSubShapeOri( F.Oriented(TopAbs_FORWARD), fromE);
    if ( o == TopAbs_REVERSED )
      du.Reverse();
    return ( norm ^ du ).XYZ();
  }
  //--------------------------------------------------------------------------------
  // DEBUG. Dump intermediate nodes positions into a python script
#define __PY_DUMP
#ifdef __PY_DUMP
  ofstream* py;
  void initPyDump()
  {
    const char* fname = "/tmp/viscous.py";
    cout << "execfile('"<<fname<<"')"<<endl;
    py = new ofstream(fname);
    *py << "from smesh import *" << endl
        << "meshSO = GetCurrentStudy().FindObjectID('0:1:2:3')" << endl
        << "mesh = Mesh( meshSO.GetObject()._narrow( SMESH.SMESH_Mesh ))"<<endl;
  }
  void dumpFunction(const string& fun) { *py<< "def "<<fun<<"():" <<endl; cout<<fun<<"()"<<endl;}
  void dumpFunctionEnd() { *py << "  return"<< endl; }
  void dumpMove(const SMDS_MeshNode* n ) {
    *py<<"  mesh.MoveNode( "<<n->GetID()<< ", "<< n->X()<< ", "<<n->Y()<<", "<< n->Z()<< ") "<<endl;
  }
#else
  void initPyDump() {}
  void dumpFunction(const string& fun ){}
  void dumpFunctionEnd() {}
  void dumpMove(const SMDS_MeshNode* n ){}
#endif
}

using namespace VISCOUS;

//================================================================================
/*!
 * \brief Constructor of _ViscousBuilder
 */
//================================================================================

_ViscousBuilder::_ViscousBuilder()
{
  _error = SMESH_ComputeError::New(COMPERR_OK);

}
//================================================================================
/*!
 * \brief Stores error description and returns false
 */
//================================================================================

bool _ViscousBuilder::error(const string& text, _SolidData* data )
{
  _error->myName    = COMPERR_ALGO_FAILED;
  _error->myComment = string("Viscous layers builder: ") + text;
  if ( _mesh )
  {
    if ( !data && !_sdVec.empty() )
      data = & _sdVec[0];
    if ( data )
    {
      SMESH_subMesh* sm = _mesh->GetSubMesh( data->_solid );
      SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
      if ( smError && smError->myAlgo )
        _error->myAlgo = smError->myAlgo;
      smError = _error;
    }
  }
  return false;
}

//================================================================================
/*!
 * \brief At study restoration, restore event listeners used to clear an inferior
 *  dim sub-mesh modified by viscous layers
 */
//================================================================================

void _ViscousBuilder::RestoreListeners()
{
  // TODO
}

//================================================================================
/*!
 * \brief Does its job
 */
//================================================================================

SMESH_ComputeErrorPtr _ViscousBuilder::Compute(SMESH_Mesh&         theMesh,
                                               const TopoDS_Shape& theShape)
{
#ifdef __PY_DUMP
  initPyDump();
#endif
  // TODO: set priority of solids during Gen::Compute()

  _mesh = & theMesh;

  // check if proxy mesh already computed
  TopExp_Explorer exp( theShape, TopAbs_SOLID );
  if ( !exp.More() )
    return error("No SOLID's in theShape", 0), _error;

  if ( _ViscousListener::GetSolidMesh( _mesh, exp.Current(), /*toCreate=*/false))
    return SMESH_ComputeErrorPtr(); // everything already computed

  // TODO: ignore already computed SOLIDs 
  findSolidsWithLayers();
  if ( _error->IsOK() )
    findFacesWithLayers();

  for ( unsigned i = 0; i < _sdVec.size(); ++i )
  {
    if ( ! makeLayer(_sdVec[i]) )
      break;
    
    if ( ! inflate(_sdVec[i]) ) {
      makeGroupOfLE();
      break;
    }
    if ( ! refine(_sdVec[i]) )
      break;
  }
  if ( _error->IsOK() )
    shrink();

  //addBoundaryElements() // TODO:
  
#ifdef __PY_DUMP
  delete py; py = 0;
#endif
  return _error;
}

//================================================================================
/*!
 * \brief Finds SOLIDs to compute using viscous layers. Fill _sdVec
 */
//================================================================================

bool _ViscousBuilder::findSolidsWithLayers()
{
  // get all solids
  TopTools_IndexedMapOfShape allSolids;
  TopExp::MapShapes( _mesh->GetShapeToMesh(), TopAbs_SOLID, allSolids );
  _sdVec.reserve( allSolids.Extent());

  SMESH_Gen* gen = _mesh->GetGen();
  for ( int i = 1; i <= allSolids.Extent(); ++i )
  {
    // find StdMeshers_ViscousLayers hyp assigned to the i-th solid
    SMESH_Algo* algo = gen->GetAlgo( *_mesh, allSolids(i) );
    if ( !algo ) continue;
    // TODO: check if algo is hidden
    const list <const SMESHDS_Hypothesis *> & allHyps =
      algo->GetUsedHypothesis(*_mesh, allSolids(i), /*ignoreAuxiliary=*/false);
    list< const SMESHDS_Hypothesis *>::const_iterator hyp = allHyps.begin();
    const StdMeshers_ViscousLayers* viscHyp = 0;
    for ( ; hyp != allHyps.end() && !viscHyp; ++hyp )
      viscHyp = dynamic_cast<const StdMeshers_ViscousLayers*>( *hyp );
    if ( viscHyp )
    {
      _MeshOfSolid* proxyMesh = _ViscousListener::GetSolidMesh( _mesh,
                                                                allSolids(i),
                                                                /*toCreate=*/true);
      _sdVec.push_back( _SolidData( allSolids(i), viscHyp, proxyMesh ));
      _sdVec.back()._index = _sdVec.size()-1; // debug
    }
  }
  if ( _sdVec.empty() )
    return error
      ( SMESH_Comment(StdMeshers_ViscousLayers::GetHypType()) << " hypothesis not found",0);

  return true;
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

bool _ViscousBuilder::findFacesWithLayers()
{
  // collect all faces to ignore defined by hyp
  vector<TopoDS_Shape> ignoreFaces;
  for ( unsigned i = 0; i < _sdVec.size(); ++i )
  {
    vector<TGeomID> ids = _sdVec[i]._hyp->GetIgnoreFaces();
    for ( unsigned i = 0; i < ids.size(); ++i )
    {
      const TopoDS_Shape& s = getMeshDS()->IndexToShape( ids[i] );
      if ( !s.IsNull() && s.ShapeType() == TopAbs_FACE )
      {
        _ignoreShapeIds.insert( ids[i] );
        ignoreFaces.push_back( s );
      }
    }
  }

  // ignore internal faces
  SMESH_MesherHelper helper( *_mesh );
  TopExp_Explorer exp;
  for ( unsigned i = 0; i < _sdVec.size(); ++i )
  {
    exp.Init( _sdVec[i]._solid.Oriented( TopAbs_FORWARD ), TopAbs_FACE );
    for ( ; exp.More(); exp.Next() )
    {
      TGeomID faceInd = getMeshDS()->ShapeToIndex( exp.Current() );
      if ( helper.NbAncestors( exp.Current(), *_mesh, TopAbs_SOLID ) > 1 )
      {     
        _ignoreShapeIds.insert( faceInd );
        ignoreFaces.push_back( exp.Current() );
        if ( SMESH_Algo::IsReversedSubMesh( TopoDS::Face( exp.Current() ), getMeshDS()))
          _sdVec[i]._reversedFaceIds.insert( faceInd );
      }
    }
  }

  // Find faces to shrink mesh on (solution 2 in issue 0020832);
  // we use solution 1 for shared faces w/o layers since there is no generic way
  // to know whether after shrinking 2D mesh remains valid for other 3D algo
  TopTools_IndexedMapOfShape shapes;
  for ( unsigned i = 0; i < _sdVec.size(); ++i )
  {
    TopExp::MapShapes(_sdVec[i]._solid, TopAbs_EDGE, shapes);
    for ( int iE = 1; iE <= shapes.Extent(); ++iE )
    {
      const TopoDS_Shape& edge = shapes(iE);
      // find 2 faces sharing an edge
      TopoDS_Shape FF[2];
      PShapeIteratorPtr fIt = helper.GetAncestors(edge, *_mesh, TopAbs_FACE);
      while ( fIt->more())
      {
        const TopoDS_Shape* f = fIt->next();
        if ( helper.IsSubShape( *f, _sdVec[i]._solid))
          FF[ int( !FF[0].IsNull()) ] = *f;
      }
      ASSERT( !FF[1].IsNull() );
      // check presence of layers on them
      int ignore[2];
      for ( int j = 0; j < 2; ++j )
        ignore[j] = _ignoreShapeIds.count ( getMeshDS()->ShapeToIndex( FF[j] ));
      if ( ignore[0] == ignore[1] ) continue; // nothing interesting
      // check if a face w/o layers (WOL) is shared
      if ( !ignore[0] ) std::swap( FF[0], FF[1] ); // set face WOL to FF[0]
//       bool shareWOL = ( helper.NbAncestors( FF[0], *_mesh, TopAbs_SOLID ) > 1 );
//       if ( shareWOL )
      {
        // add edge to maps
        TGeomID edgeInd = getMeshDS()->ShapeToIndex( edge );
        _sdVec[i]._shrinkShape2Shape.insert( make_pair( edgeInd, FF[0] ));
      }
    }

    // Find shapes along which to inflate _LayerEdge on vertex

    shapes.Clear();
    TopExp::MapShapes(_sdVec[i]._solid, TopAbs_VERTEX, shapes);
    for ( int iV = 1; iV <= shapes.Extent(); ++iV )
    {
      const TopoDS_Shape& vertex = shapes(iV);
      // find faces WOL sharing the vertex
      vector< TopoDS_Shape > faces;
      int totalNbFaces = 0;
      PShapeIteratorPtr fIt = helper.GetAncestors(vertex, *_mesh, TopAbs_FACE);
      while ( fIt->more())
      {
        const TopoDS_Shape* f = fIt->next();
        totalNbFaces++;
        if ( helper.IsSubShape( *f, _sdVec[i]._solid) &&
             _ignoreShapeIds.count ( getMeshDS()->ShapeToIndex( *f )))
          faces.push_back( *f );
      }
      if ( faces.size() == totalNbFaces || faces.empty() )
        continue; // not layers at this vertex or no WOL
      TGeomID vInd = getMeshDS()->ShapeToIndex( vertex );
      switch ( faces.size() )
      {
      case 1:
        {
          _sdVec[i]._shrinkShape2Shape.insert( make_pair( vInd, faces[0] )); break;
        }
      case 2:
        {
          // find an edge shared by 2 faces
          PShapeIteratorPtr eIt = helper.GetAncestors(vertex, *_mesh, TopAbs_EDGE);
          while ( eIt->more())
          {
            const TopoDS_Shape* e = eIt->next();
            if ( helper.IsSubShape( *e, faces[0]) &&
                 helper.IsSubShape( *e, faces[1]) &&
                 helper.IsSubShape( vertex, *e ))
            {
              _sdVec[i]._shrinkShape2Shape.insert( make_pair( vInd, *e )); break;
            }
          }
          break;
        }
      default:
        return error("Not yet supported case", &_sdVec[i]);
      }
    }
  }
  // TODO: treat cases with solution 1.

  return true;
}

//================================================================================
/*!
 * \brief Create the inner surface of the viscous layer and prepare data for infation
 */
//================================================================================

bool _ViscousBuilder::makeLayer(_SolidData& data)
{
  // get all sub-shapes to make layers on
  set<TGeomID> subIds, faceIds;
  set<TGeomID>::iterator id;
  TopExp_Explorer exp( data._solid, TopAbs_FACE );
  for ( ; exp.More(); exp.Next() )
    if ( ! _ignoreShapeIds.count( getMeshDS()->ShapeToIndex( exp.Current() )))
    {
      SMESH_subMesh* fSubM = _mesh->GetSubMesh( exp.Current() );
      faceIds.insert( fSubM->GetId() );
      SMESH_subMeshIteratorPtr subIt =
        fSubM->getDependsOnIterator(/*includeSelf=*/true, /*complexShapeFirst=*/false);
      while ( subIt->more() )
        subIds.insert( subIt->next()->GetId() );
    }

  // make a map to find new nodes on sub-shapes shared with other solid
  map< TGeomID, TNode2Edge* > s2neMap;
  map< TGeomID, TNode2Edge* >::iterator s2ne;
  map< TGeomID, TopoDS_Shape >::iterator s2s = data._shrinkShape2Shape.begin();
  for (; s2s != data._shrinkShape2Shape.end(); ++s2s )
  {
    TGeomID shapeInd = s2s->first;
    for ( unsigned i = 0; i < _sdVec.size(); ++i )
    {
      if ( _sdVec[i]._index == data._index ) continue;
      map< TGeomID, TopoDS_Shape >::iterator s2s2 = _sdVec[i]._shrinkShape2Shape.find( shapeInd );
      if ( *s2s == *s2s2 && !_sdVec[i]._n2eMap.empty() )
      {
        s2neMap.insert( make_pair( shapeInd, &_sdVec[i]._n2eMap ));
        break;
      }
    }
  }

  // Create temporary faces and _LayerEdge's

  //dumpFunction(SMESH_Comment("makeLayers_")<<data._index); 

  data._stepSize = numeric_limits<double>::max();

  SMESH_MesherHelper helper( *_mesh );
  helper.SetSubShape( data._solid );
  helper.SetElementsOnShape(true);

  vector< const SMDS_MeshNode*> newNodes;
  TNode2Edge::iterator n2e2;

  // collect all _LayerEdge's to inflate along a FACE
  TopTools_IndexedMapOfShape srinkFaces; // to index only srinkFaces
  vector< vector<_LayerEdge*> > edgesBySrinkFace( subIds.size() );

  for ( set<TGeomID>::iterator id = faceIds.begin(); id != faceIds.end(); ++id )
  {
    SMESHDS_SubMesh* smDS = getMeshDS()->MeshElements( *id );
    if ( !smDS ) return error(SMESH_Comment("Not meshed face ") << *id, &data );

    const TopoDS_Face& F = TopoDS::Face( getMeshDS()->IndexToShape( *id ));
    StdMeshers_ProxyMesh::SubMesh* proxySub =
      data._proxyMesh->getFaceData( F, /*create=*/true);

    SMDS_ElemIteratorPtr eIt = smDS->GetElements();
    while ( eIt->more() )
    {
      const SMDS_MeshElement* face = eIt->next();
      newNodes.resize( face->NbCornerNodes() );
      double faceMaxCosin = -1;
      for ( int i = 0 ; i < face->NbCornerNodes(); ++i )
      {
        const SMDS_MeshNode* n = face->GetNode(i);
        TNode2Edge::iterator n2e = data._n2eMap.insert( make_pair( n, (_LayerEdge*)0 )).first;
        if ( !(*n2e).second )
        {
          // add a _LayerEdge
          _LayerEdge* edge = new _LayerEdge();
          n2e->second = edge;
          edge->_nodes.push_back( n );

          // set edge data or find already refined _LayerEdge and get data from it
          if ( n->GetPosition()->GetTypeOfPosition() != SMDS_TOP_FACE &&
               ( s2ne = s2neMap.find( n->GetPosition()->GetShapeId() )) != s2neMap.end() &&
               ( n2e2 = (*s2ne).second->find( n )) != s2ne->second->end())
          {
            _LayerEdge* foundEdge = (*n2e2).second;
            edge->_nodes    = foundEdge->_nodes;
            edge->_normal   = foundEdge->_normal;
            edge->_len      = 0;
            edge->_cosin    = foundEdge->_cosin;
            edge->_sWOL     = foundEdge->_sWOL;
            edge->_2neibors = foundEdge->_2neibors;
            if ( foundEdge->_2neibors )
              edge->_2neibors = new _2NearEdges( *foundEdge->_2neibors );
            // location of the last node is modified but we can restore
            // it by node position on _sWOL stored by the node
            const_cast< SMDS_MeshNode* >
              ( edge->_nodes.back() )->setXYZ( n->X(), n->Y(), n->Z() );
          }
          else
          {
            edge->_nodes.push_back( helper.AddNode( n->X(), n->Y(), n->Z() ));
            if ( !setEdgeData( *edge, subIds, helper, data ))
              return false;
            //dumpMove(edge->_nodes.back());
          }
          if ( edge->_cosin > 0.01 )
          {
            if ( edge->_cosin > faceMaxCosin )
              faceMaxCosin = edge->_cosin;
          }
          if ( edge->IsOnEdge() ) // _LayerEdge is based on EDGE
          {
            TGeomID faceId = ( edge->_sWOL.IsNull() ? 0 : srinkFaces.Add( edge->_sWOL ));
            edgesBySrinkFace[ faceId ].push_back( edge );
          }
        }
        newNodes[ i ] = n2e->second->_nodes.back();
      }
      // create a temporary face
      const SMDS_MeshElement* newFace;
      if ( newNodes.size() == 3 )
        newFace = new SMDS_FaceOfNodes( newNodes[0], newNodes[1], newNodes[2] );
      else
        newFace = new SMDS_FaceOfNodes( newNodes[0], newNodes[1], newNodes[2], newNodes[3] );
      proxySub->AddElement( newFace );

      // compute inflation step size by min size of element on a convex surface
      if ( faceMaxCosin > 0.1 )
      {
        double elemMinSize = numeric_limits<double>::max();
        int minNodeInd = 0;
        for ( unsigned i = 1; i < newNodes.size(); ++i )
        {
          double len = SMESH_MeshEditor::TNodeXYZ( newNodes[i-1] ).Distance( newNodes[i] );
          if ( len < elemMinSize && len > numeric_limits<double>::min() )
            elemMinSize = len, minNodeInd = i;
        }
        double newStep = 0.8 * elemMinSize / faceMaxCosin;
        if ( newStep < data._stepSize )
        {
          data._stepSize = newStep;
          data._stepSizeNodes[0] = newNodes[minNodeInd-1];
          data._stepSizeNodes[1] = newNodes[minNodeInd];
        }
      }
    } // loop on 2D elements on a FACE
  } // loop on FACEs of a SOLID

  // Put _LayerEdge's into a vector

  data._edges.reserve( data._n2eMap.size() );
  {
    // first we put _LayerEdge's based on EDGE's to smooth them before others,
    TopoDS_Face F;
    for ( unsigned i = 0; i < edgesBySrinkFace.size(); ++i )
    {
      vector<_LayerEdge*>& eVec = edgesBySrinkFace[i];
      if ( eVec.empty() ) continue;
      if ( i ) F = TopoDS::Face( srinkFaces(i));
      data._edges.insert( data._edges.end(), eVec.begin(), eVec.end() );
      data._endEdge2Face[ data._edges.size() ] = F;
    }
    // then the rest _LayerEdge's
    TNode2Edge::iterator n2e = data._n2eMap.begin();
    for ( ; n2e != data._n2eMap.end(); ++n2e )
    {
      _LayerEdge* e = n2e->second;
      if ( !e->IsOnEdge()  )
        data._edges.push_back( e );
    }
  }

  // Set target nodes into _Simplex and _2NearEdges
  for ( unsigned i = 0; i < data._edges.size(); ++i )
  {
    if ( data._edges[i]->IsOnEdge())
      for ( int j = 0; j < 2; ++j )
      {
        const SMDS_MeshNode* & n = data._edges[i]->_2neibors->_nodes[j];
        n = data._n2eMap[ n ]->_nodes.back();
      }
    else
      for ( unsigned j = 0; j < data._edges[i]->_simplices.size(); ++j )
      {
        _Simplex& s = data._edges[i]->_simplices[j];
        s._nPrev = data._n2eMap[ s._nPrev ]->_nodes.back();
        s._nNext = data._n2eMap[ s._nNext ]->_nodes.back();
      }
  }

  //dumpFunctionEnd();
  return true;
}

//================================================================================
/*!
 * \brief Set data of _LayerEdge needed for smoothing
 *  \param subIds - ids of sub-shapes of a SOLID to take into account faces from
 */
//================================================================================

bool _ViscousBuilder::setEdgeData(_LayerEdge&         edge,
                                  const set<TGeomID>& subIds,
                                  SMESH_MesherHelper& helper,
                                  _SolidData&         data)
{
  SMESH_MeshEditor editor(_mesh);

  const SMDS_MeshNode* node = edge._nodes[0]; // source node

  edge._len = 0;
  edge._2neibors = 0;

  // --------------------------
  // Compute _normal and _cosin
  // --------------------------

  edge._cosin = 0;
  edge._normal.SetCoord(0,0,0);

  int totalNbFaces = 0;
  gp_Pnt p;
  gp_Vec du, dv, geomNorm;
  bool normOK = true;

  TGeomID shapeInd = node->GetPosition()->GetShapeId();
  map< TGeomID, TopoDS_Shape >::const_iterator s2s = data._shrinkShape2Shape.find( shapeInd );
  bool onShrinkShape ( s2s != data._shrinkShape2Shape.end() );
  TopoDS_Shape vertEdge;

  if ( onShrinkShape ) // one of faces the node is on has no layers
  {
    vertEdge = getMeshDS()->IndexToShape( s2s->first ); // vertex or edge
    if ( s2s->second.ShapeType() == TopAbs_EDGE )
    {
      // inflate from VERTEX along EDGE
      edge._normal = getEdgeDir( TopoDS::Edge( s2s->second ), TopoDS::Vertex( vertEdge ));
    }
    else if ( vertEdge.ShapeType() == TopAbs_VERTEX )
    {
      // inflate from VERTEX along FACE
      edge._normal = getFaceDir( TopoDS::Face( s2s->second ), TopoDS::Vertex( vertEdge ),
                                 helper, normOK, &edge._cosin);
    }
    else
    {
      // inflate from EDGE along FACE
      edge._normal = getFaceDir( TopoDS::Face( s2s->second ), TopoDS::Edge( vertEdge ),
                                 node, helper, normOK);
    }
  }
  else // layers are on all faces of SOLID the node is on
  {
    // find indices of geom faces the node lies on
    set<TGeomID> faceIds;
    if  ( node->GetPosition()->GetTypeOfPosition() == SMDS_TOP_FACE )
    {
      faceIds.insert( node->GetPosition()->GetShapeId() );
    }
    else
    {
      SMDS_ElemIteratorPtr fIt = node->GetInverseElementIterator(SMDSAbs_Face);
      while ( fIt->more() )
        faceIds.insert( editor.FindShape(fIt->next()));
    }

    set<TGeomID>::iterator id = faceIds.begin();
    TopoDS_Face F;
    for ( ; id != faceIds.end(); ++id )
    {
      const TopoDS_Shape& s = getMeshDS()->IndexToShape( *id );
      if ( s.IsNull() || s.ShapeType() != TopAbs_FACE || !subIds.count( *id ))
        continue;
      totalNbFaces++;
      //nbLayerFaces += subIds.count( *id );
      F = TopoDS::Face( s );

      gp_XY uv = helper.GetNodeUV( F, node, 0, &normOK );
      Handle(Geom_Surface) surface = BRep_Tool::Surface( F );
      surface->D1( uv.X(),uv.Y(), p, du,dv );
      geomNorm = du ^ dv;
      double size2 = geomNorm.SquareMagnitude();
      if ( size2 > numeric_limits<double>::min() )
        geomNorm /= sqrt( size2 );
      else
        normOK = false;
      if ( helper.GetSubShapeOri( data._solid, F ) != TopAbs_REVERSED )
        geomNorm.Reverse();
      edge._normal += geomNorm.XYZ();
    }
    if ( totalNbFaces == 0 )
      return error(SMESH_Comment("Can't get normal to node ") << node->GetID(), &data);

    edge._normal /= totalNbFaces;

    switch ( node->GetPosition()->GetTypeOfPosition())
    {
    case SMDS_TOP_FACE:
      edge._cosin = 0; break;

    case SMDS_TOP_EDGE: {
      TopoDS_Edge E = TopoDS::Edge( helper.GetSubShapeByNode( node, getMeshDS()));
      gp_Vec inFaceDir = getFaceDir( F, E, node, helper, normOK);
      double angle = inFaceDir.Angle( edge._normal ); // [0,PI]
      edge._cosin = cos( angle );
      //cout << "Cosin on EDGE " << edge._cosin << " node " << node->GetID() << endl;
      break;
    }
    case SMDS_TOP_VERTEX: {
      TopoDS_Vertex V = TopoDS::Vertex( helper.GetSubShapeByNode( node, getMeshDS()));
      gp_Vec inFaceDir = getFaceDir( F, V, helper, normOK);
      double angle = inFaceDir.Angle( edge._normal ); // [0,PI]
      edge._cosin = cos( angle );
      //cout << "Cosin on VERTEX " << edge._cosin << " node " << node->GetID() << endl;
      break;
    }
    default:
      return error(SMESH_Comment("Invalid shape position of node ")<<node,&data);
    }
  }

  double normSize = edge._normal.SquareModulus();
  if ( normSize < numeric_limits<double>::min() )
    return error(SMESH_Comment("Bad normal at node ")<< node->GetID(), &data );

  edge._normal /= sqrt( normSize );

  // TODO: if ( !normOK ) then get normal by mesh faces

  // Set the rest data
  // --------------------
  if ( onShrinkShape )
  {
    edge._sWOL = (*s2s).second;

    SMDS_MeshNode* tgtNode = const_cast<SMDS_MeshNode*>( edge._nodes.back() );
    if ( SMESHDS_SubMesh* sm = getMeshDS()->MeshElements( data._solid ))
      sm->RemoveNode( tgtNode , /*isNodeDeleted=*/false );

    // set initial position which is parameters on _sWOL in this case
    if ( edge._sWOL.ShapeType() == TopAbs_EDGE )
    {
      double u = helper.GetNodeU( TopoDS::Edge( edge._sWOL ), node, 0, &normOK );
      edge._pos.push_back( gp_XYZ( u, 0, 0));
      getMeshDS()->SetNodeOnEdge( tgtNode, TopoDS::Edge( edge._sWOL ), u );
    }
    else // TopAbs_FACE
    {
      gp_XY uv = helper.GetNodeUV( TopoDS::Face( edge._sWOL ), node, 0, &normOK );
      edge._pos.push_back( gp_XYZ( uv.X(), uv.Y(), 0));
      getMeshDS()->SetNodeOnFace( tgtNode, TopoDS::Face( edge._sWOL ), uv.X(), uv.Y() );
    }
  }
  else
  {
    edge._pos.push_back( SMESH_MeshEditor::TNodeXYZ( node ));

    if ( node->GetPosition()->GetTypeOfPosition() == SMDS_TOP_FACE )
      getSimplices( node, edge._simplices, _ignoreShapeIds, &data );
  }

  // Set neighbour nodes for a _LayerEdge based on EDGE

  if ( node->GetPosition()->GetTypeOfPosition() == SMDS_TOP_EDGE )
  {
    SMESHDS_SubMesh* edgeSM = getMeshDS()->MeshElements( shapeInd );
    if ( !edgeSM || edgeSM->NbElements() == 0 )
      return error(SMESH_Comment("Not meshed EDGE ") << shapeInd, &data);

    edge._2neibors = new _2NearEdges;
    SMDS_ElemIteratorPtr eIt = node->GetInverseElementIterator(SMDSAbs_Edge);
    while ( eIt->more() && !edge._2neibors->_nodes[1] )
    {
      const SMDS_MeshElement* e = eIt->next();
      if ( !edgeSM->Contains(e))
        continue;
      const SMDS_MeshNode* n2 = e->GetNode( 0 );
      if ( n2 == node ) n2 = e->GetNode( 1 );
      const int iN = edge._2neibors->_nodes[0] ? 1 : 0;
      edge._2neibors->_nodes[ iN ] = n2; // target nodes will be set later
      gp_XYZ pos2;
      if ( onShrinkShape )
      {
        gp_XY uv = helper.GetNodeUV( TopoDS::Face( edge._sWOL ), n2, 0, &normOK );
        pos2.SetCoord( uv.X(), uv.Y(), 0 );
      }
      else
      {
        pos2 = SMESH_MeshEditor::TNodeXYZ( n2 );
      }
      edge._2neibors->_vec[iN] = edge._pos[0] - pos2;
    }
    if ( !edge._2neibors->_nodes[1] )
      return error(SMESH_Comment("Wrongly meshed EDGE ") << shapeInd, &data);
  }
  return true;
}

//================================================================================
/*!
 * \brief Fills a vector<_Simplex > 
 */
//================================================================================

void _ViscousBuilder::getSimplices( const SMDS_MeshNode* node,
                                    vector<_Simplex>&    simplices,
                                    const set<TGeomID>&  ingnoreShapes,
                                    const _SolidData*    dataToCheckOri)
{
  SMESH_MeshEditor editor( _mesh );
  SMDS_ElemIteratorPtr fIt = node->GetInverseElementIterator(SMDSAbs_Face);
  while ( fIt->more() )
  {
    const SMDS_MeshElement* f = fIt->next();
    const TGeomID shapeInd = editor.FindShape( f );
    if ( ingnoreShapes.count( shapeInd )) continue;
    const int nbNodes = f->NbCornerNodes();
    int srcInd = f->GetNodeIndex( node );
    const SMDS_MeshNode* nPrev = f->GetNode( SMESH_MesherHelper::WrapIndex( srcInd-1, nbNodes ));
    const SMDS_MeshNode* nNext = f->GetNode( SMESH_MesherHelper::WrapIndex( srcInd+1, nbNodes ));
    if ( dataToCheckOri && dataToCheckOri->_reversedFaceIds.count( shapeInd ))
      std::swap( nPrev, nNext );
    simplices.push_back( _Simplex( nPrev, nNext ));
  }
  simplices.resize( simplices.size() );
}

//================================================================================
/*!
 * \brief DEBUG. Create groups of edges contating segments of _LayerEdge's
 */
//================================================================================

void _ViscousBuilder::makeGroupOfLE()
{
#ifdef _DEBUG_
  for ( unsigned i = 0 ; i < _sdVec.size(); ++i )
  {
    if ( _sdVec[i]._edges.empty() ) continue;
    string name = SMESH_Comment("_LayerEdge's_") << i;
    int id;
    SMESH_Group* g = _mesh->AddGroup(SMDSAbs_Edge, name.c_str(), id );
    SMESHDS_Group* gDS = (SMESHDS_Group*)g->GetGroupDS();
    SMESHDS_Mesh* mDS = _mesh->GetMeshDS();

    for ( unsigned j = 0 ; j < _sdVec[i]._edges.size(); ++j )
    {
      _LayerEdge* le = _sdVec[i]._edges[j];
      for ( unsigned iN = 1; iN < le->_nodes.size(); ++iN )
        gDS->SMDSGroup().Add( mDS->AddEdge( le->_nodes[iN-1], le->_nodes[iN]));
    }

    name = SMESH_Comment("Normals_") << i;
    g = _mesh->AddGroup(SMDSAbs_Edge, name.c_str(), id );
    gDS = (SMESHDS_Group*)g->GetGroupDS();
    SMESH_MesherHelper helper( *_mesh );
    for ( unsigned j = 0 ; j < _sdVec[i]._edges.size(); ++j )
    {
      _LayerEdge leCopy = *_sdVec[i]._edges[j];
      leCopy._len = 0;
      SMESH_MeshEditor::TNodeXYZ nXYZ( leCopy._nodes[0] );
      SMDS_MeshNode* n = const_cast< SMDS_MeshNode*>( leCopy._nodes.back() );
      n->setXYZ( nXYZ.X(), nXYZ.Y(), nXYZ.Z() );
      leCopy.SetNewLength( _sdVec[i]._stepSize, helper );
      gDS->SMDSGroup().Add( mDS->AddEdge( leCopy._nodes[0], n ));
    }
  }
#endif  
}

//================================================================================
/*!
 * \brief Increase length of _LayerEdge's to reach the required thickness of layers
 */
//================================================================================

bool _ViscousBuilder::inflate(_SolidData& data)
{
  // TODO: update normals after the sirst step
  SMESH_MesherHelper helper( *_mesh );

  const double tgtThick = data._hyp->GetTotalThickness();
  if ( data._stepSize > tgtThick )
  {
    data._stepSize = tgtThick;
    data._stepSizeNodes[0] = 0;
  }
  double avgThick = 0, curThick = 0;
  int nbSteps = 0, nbRepeats = 0;
  while ( 1.01 * avgThick < tgtThick )
  {
    dumpFunction(SMESH_Comment("inflate")<<data._index<<"_step"<<nbSteps); // debug

    // new target length
    if ( data._stepSizeNodes[0] )
      data._stepSize =
        0.8 * SMESH_MeshEditor::TNodeXYZ(data._stepSizeNodes[0]).Distance(data._stepSizeNodes[1]);
    curThick += data._stepSize;
    if ( curThick > tgtThick )
    {
      curThick = tgtThick + ( tgtThick-avgThick ) * nbRepeats;
      nbRepeats++;
    }
    // elongate _LayerEdge's
    for ( unsigned i = 0; i < data._edges.size(); ++i )
      data._edges[i]->SetNewLength( curThick, helper );

    dumpFunctionEnd();

    // improve aand check quality
    if ( !smoothAndCheck( data, nbSteps ))
    {
      if ( nbSteps == 0 )
        return error("Viscous builder failed at the very first inflation step", &data);

      for ( unsigned i = 0; i < data._edges.size(); ++i )
        data._edges[i]->InvalidateStep( nbSteps+1 );

      break; // no more inflating possible
    }
    // evaluate achieved thickness
    avgThick = 0;
    for ( unsigned i = 0; i < data._edges.size(); ++i )
      avgThick += data._edges[i]->_len;
    avgThick /= data._edges.size();

    nbSteps++;
  }
  return true;
}

//================================================================================
/*!
 * \brief Improve quality of layer inner surface and check intersection
 */
//================================================================================

bool _ViscousBuilder::smoothAndCheck(_SolidData& data, int nbSteps)
{
  // Smoothing

  bool moved = true, improved = true;
  int iOnEdgeEnd = data._endEdge2Face.empty() ? 0 : data._endEdge2Face.rbegin()->first;
  if ( !data._endEdge2Face.empty() )
  {
    // first we smooth _LayerEdge's based on EDGES
    SMESH_MesherHelper helper(*_mesh);
    Handle(Geom_Surface) surface;
    int iBeg = 0;
    map< int, TopoDS_Face >::iterator nb2f = data._endEdge2Face.begin();
    for ( ; nb2f != data._endEdge2Face.end(); ++nb2f )
    {
      helper.SetSubShape( nb2f->second );
      if ( !nb2f->second.IsNull() )
        surface = BRep_Tool::Surface( nb2f->second );
      dumpFunction(SMESH_Comment("smooth")<<data._index
                   << "_OnFace" << helper.GetSubShapeID() <<"_step"<<nbSteps); // debug
      int nb = nb2f->first;
      do {
        moved = false;
        for ( int i = iBeg; i < nb; ++i )
          moved |= data._edges[i]->SmoothOnEdge(surface, nb2f->second, helper);
      }
      while ( moved );

      iBeg = nb;

      dumpFunctionEnd();
    }
  }
  // smooth in 3D
  int step = 0, badNb = 0; moved = true;
  while (( ++step <= 5 && moved ) || improved )
  {
    dumpFunction(SMESH_Comment("smooth")<<data._index<<"_step"<<nbSteps<<"_"<<step); // debug
    int oldBadNb = badNb;
    badNb = 0;
    moved = false;
    for ( unsigned i = iOnEdgeEnd; i < data._edges.size(); ++i )
      moved = data._edges[i]->Smooth(badNb) || moved;
    improved = ( badNb < oldBadNb );

    dumpFunctionEnd();
  }
  if ( badNb > 0 )
    return false;

  // Check if the last segments of _LayerEdge intersects 2D elements,
  // checked elements are either temporary faces or faces on surfaces w/o the layers

  SMESH_MeshEditor editor( _mesh );
  auto_ptr<SMESH_ElementSearcher> searcher
    ( editor.GetElementSearcher( data._proxyMesh->GetFaces( data._solid )) );

  vector< const SMDS_MeshElement* > suspectFaces;
  for ( unsigned i = 0; i < data._edges.size(); ++i )
  {
    const _LayerEdge& edge = * data._edges[i];
    gp_Ax1 lastSegment = edge.LastSegment();
    searcher->GetElementsNearLine( lastSegment, SMDSAbs_Face, suspectFaces );

    for ( unsigned j = 0 ; j < suspectFaces.size(); ++j )
    {
      const SMDS_MeshElement* face = suspectFaces[j];
      if ( face->GetNodeIndex( edge._nodes.back() ) >= 0 )
        continue; // face sharing _LayerEdge node
      const int nbNodes = face->NbCornerNodes();
      bool intFound = false;
      if ( nbNodes == 3 )
      {
        SMDS_MeshElement::iterator nIt = face->begin_nodes();
        intFound = edge.SegTriaInter( lastSegment, *nIt++, *nIt++, *nIt++ );
      }
      else
      {
        const SMDS_MeshNode* tria[3];
        tria[0] = face->GetNode(0);
        tria[1] = face->GetNode(1);
        for ( int n2 = 2; n2 < nbNodes && !intFound; ++n2 )
        {
          tria[2] = face->GetNode(n2);
          intFound = edge.SegTriaInter(lastSegment, tria[0], tria[1], tria[2] );
          tria[1] = tria[2];
        }
      }
      if ( intFound )
        return false;
    }
  }
  return true;
}

//================================================================================
/*!
 * \brief Returns direction of the last segment
 */
//================================================================================

gp_Ax1 _LayerEdge::LastSegment() const
{
  // find two non-coincident positions
  gp_XYZ orig = _pos.back();
  gp_XYZ dir;
  int iPrev = _pos.size() - 2;
  while ( iPrev > 0 )
  {
    dir = orig - _pos[iPrev--];
    if ( dir.SquareModulus() > 1e-100 )
      break;
  }

  // make gp_Ax1
  gp_Ax1 segDir;
  segDir.SetLocation( SMESH_MeshEditor::TNodeXYZ( _nodes.back() ));
  if ( iPrev < 1 )
  {
    segDir.SetDirection( _normal );
  }
  else
  {
    if ( !_sWOL.IsNull() )
    {
      gp_Pnt pPrev;
      const gp_XYZ& par = _pos[ iPrev ];
      TopLoc_Location loc;
      if ( _sWOL.ShapeType() == TopAbs_EDGE )
      {
        double f,l;
        Handle(Geom_Curve) curve = BRep_Tool::Curve( TopoDS::Edge( _sWOL ), loc, f,l);
        pPrev = curve->Value( par.X() ).Transformed( loc );
      }
      else
      {
        Handle(Geom_Surface) surface = BRep_Tool::Surface( TopoDS::Face(_sWOL), loc );
        pPrev = surface->Value( par.X(), par.Y() ).Transformed( loc );
      }
      dir = segDir.Location().XYZ() - pPrev.XYZ();
    }
    segDir.SetDirection( dir );
  }
  return segDir;
}

//================================================================================
/*!
 * \brief Perform smooth of _LayerEdge's based on EDGE's
 *  \retval bool - true if node has been moved
 */
//================================================================================

bool _LayerEdge::SmoothOnEdge(Handle(Geom_Surface)& surface,
                              const TopoDS_Face&    F,
                              SMESH_MesherHelper&   helper)
{
  ASSERT( IsOnEdge() );

  SMDS_MeshNode* tgtNode = const_cast<SMDS_MeshNode*>( _nodes.back() );
  double dist01, distNewOld;
  if ( F.IsNull() )
  {
    SMESH_MeshEditor::TNodeXYZ p0( _2neibors->_nodes[0]);
    SMESH_MeshEditor::TNodeXYZ p1( _2neibors->_nodes[1]);
    dist01 = p0.Distance( _2neibors->_nodes[1] );

    p0 += _2neibors->_vec[0];
    p1 += _2neibors->_vec[1];
    gp_Pnt newPos = 0.5 * ( p0 + p1 );
    distNewOld = newPos.Distance( _pos.back() );

    _pos.back() = newPos.XYZ();
    tgtNode->setXYZ( newPos.X(), newPos.Y(), newPos.Z() );
  }
  else
  {
    // smooth _LayerEdge based on EDGE and inflated along FACE

    gp_XYZ& uv = _pos.back();

    gp_XY uv0 = helper.GetNodeUV( F, _2neibors->_nodes[0], tgtNode );
    gp_XY uv1 = helper.GetNodeUV( F, _2neibors->_nodes[1], tgtNode );
    dist01 = (uv0 - uv1).Modulus();

    uv0 += gp_XY( _2neibors->_vec[0].X(), _2neibors->_vec[0].Y() );
    uv1 += gp_XY( _2neibors->_vec[1].X(), _2neibors->_vec[1].Y() );
    gp_Pnt2d newUV = 0.5 * ( uv0 + uv1 );
    distNewOld = newUV.Distance( gp_XY( uv.X(), uv.Y() ));

    SMDS_FacePosition* pos = static_cast<SMDS_FacePosition*>( tgtNode->GetPosition().get() );
    pos->SetUParameter( newUV.X() );
    pos->SetVParameter( newUV.Y() );
    uv.SetCoord( newUV.X(), newUV.Y(), 0 );

    gp_Pnt p = surface->Value( newUV.X(), newUV.Y() );
    tgtNode->setXYZ( p.X(), p.Y(), p.Z() );
  }
  bool moved = distNewOld > dist01/50;
  if ( moved )
    dumpMove( tgtNode ); // debug

  return moved;
}

//================================================================================
/*!
 * \brief Perform laplacian smooth in 3D of nodes inflated from FACE
 *  \retval bool - true if _tgtNode has been moved
 */
//================================================================================

bool _LayerEdge::Smooth(int& badNb)
{
  if ( _simplices.size() < 2 )
    return false; // _LayerEdge inflated along EDGE or FACE

  // compute new position for the last _pos
  gp_XYZ newPos (0,0,0);
  for ( unsigned i = 0; i < _simplices.size(); ++i )
    newPos += SMESH_MeshEditor::TNodeXYZ( _simplices[i]._nPrev );
  newPos /= _simplices.size();

  gp_Pnt prevPos( _pos[ _pos.size()-2 ]);
  if ( _cosin < -0.1)
  {
    // Avoid decreasing length of edge on concave surface
    //gp_Vec oldMove( _pos[ _pos.size()-2 ], _pos.back() );
    gp_Vec newMove( prevPos, newPos );
    newPos = _pos.back() + newMove.XYZ();
  }
  else if ( _cosin > 0.3 )
  {
    // Avoid increasing length of edge too much

  }
  // count quality metrics (orientation) of tetras around _tgtNode
  int nbOkBefore = 0;
  SMESH_MeshEditor::TNodeXYZ tgtXYZ( _nodes.back() );
  for ( unsigned i = 0; i < _simplices.size(); ++i )
    nbOkBefore += _simplices[i].IsForward( _nodes[0], &tgtXYZ );

  int nbOkAfter = 0;
  for ( unsigned i = 0; i < _simplices.size(); ++i )
    nbOkAfter += _simplices[i].IsForward( _nodes[0], &newPos );

  if ( nbOkAfter < nbOkBefore )
    return false;

  SMDS_MeshNode* n = const_cast< SMDS_MeshNode* >( _nodes.back() );

  _len -= prevPos.Distance(SMESH_MeshEditor::TNodeXYZ( n ));
  _len += prevPos.Distance(newPos);

  n->setXYZ( newPos.X(), newPos.Y(), newPos.Z());
  _pos.back() = newPos;

  badNb += _simplices.size() - nbOkAfter;

  dumpMove( n );

  return true;
}

//================================================================================
/*!
 * \brief Test intersection of the last segment with a given triangle
 *   using Moller-Trumbore algorithm
 */
//================================================================================

bool _LayerEdge::SegTriaInter( const gp_Ax1&        lastSegment,
                               const SMDS_MeshNode* n0,
                               const SMDS_MeshNode* n1,
                               const SMDS_MeshNode* n2 ) const
{
  const double EPSILON = 1e-6;

  gp_XYZ orig = lastSegment.Location().XYZ();
  gp_XYZ dir  = lastSegment.Direction().XYZ();

  SMESH_MeshEditor::TNodeXYZ vert0( n0 );
  SMESH_MeshEditor::TNodeXYZ vert1( n1 );
  SMESH_MeshEditor::TNodeXYZ vert2( n2 );

  gp_XYZ edge1 = vert1 - vert0;
  gp_XYZ edge2 = vert2 - vert0;

  /* begin calculating determinant - also used to calculate U parameter */
  gp_XYZ pvec = dir ^ edge2;

  /* if determinant is near zero, ray lies in plane of triangle */
  double det = edge1 * pvec;

  if (det > -EPSILON && det < EPSILON)
    return 0;
  double inv_det = 1.0 / det;

  /* calculate distance from vert0 to ray origin */
  gp_XYZ tvec = orig - vert0;

  /* calculate U parameter and test bounds */
  double u = ( tvec * pvec ) * inv_det;
  if (u < 0.0 || u > 1.0)
    return 0;

  /* prepare to test V parameter */
  gp_XYZ qvec = tvec ^ edge1;

  /* calculate V parameter and test bounds */
  double v = (dir * qvec) * inv_det;
  if ( v < 0.0 || u + v > 1.0 )
    return 0;

  /* calculate t, ray intersects triangle */
  double t = (edge2 * qvec) * inv_det;

  //   if (det < EPSILON)
  //     return false;

  //   /* calculate distance from vert0 to ray origin */
  //   gp_XYZ tvec = orig - vert0;

  //   /* calculate U parameter and test bounds */
  //   double u = tvec * pvec;
  //   if (u < 0.0 || u > det)
//     return 0;

//   /* prepare to test V parameter */
//   gp_XYZ qvec = tvec ^ edge1;

//   /* calculate V parameter and test bounds */
//   double v = dir * qvec;
//   if (v < 0.0 || u + v > det)
//     return 0;

//   /* calculate t, scale parameters, ray intersects triangle */
//   double t = edge2 * qvec;
//   double inv_det = 1.0 / det;
//   t *= inv_det;
//   //u *= inv_det;
//   //v *= inv_det;

  bool intersection = t < _len;
  if ( intersection )
  {
    gp_XYZ intP( orig + dir * t );
    cout << "src node " << _nodes.back()->GetID() << ", intersection with face "
         << n0->GetID() << " " << n1->GetID() << " " << n2->GetID() << endl
         << " at point " << intP.X() << ", " << intP.Y() << ", " << intP.Z() << endl;
  }
  return intersection;
}

//================================================================================
/*!
 * \brief Add a new segment to _LayerEdge during inflation
 */
//================================================================================

void _LayerEdge::SetNewLength( double len, SMESH_MesherHelper& helper )
{
  if ( _cosin > 0.1 ) // elongate at convex places
    len /= sqrt(1-_cosin*_cosin);

  if ( _len - len > -1e-6 )
  {
    _pos.push_back( _pos.back() );
    return;
  }
  SMDS_MeshNode* n = const_cast< SMDS_MeshNode*>( _nodes.back() );
  SMESH_MeshEditor::TNodeXYZ oldXYZ( n );
  gp_XYZ nXYZ = oldXYZ + _normal * ( len - _len );
  n->setXYZ( nXYZ.X(), nXYZ.Y(), nXYZ.Z() );

  _pos.push_back( nXYZ );
  if ( _sWOL.IsNull() )
  {
    _len = len;
  }
  else
  {
    double distXYZ[4];
    if ( _sWOL.ShapeType() == TopAbs_EDGE )
    {
      double u = Precision::Infinite(); // to force projection w/o distance check
      helper.CheckNodeU( TopoDS::Edge( _sWOL ), n, u, 1e-10, /*force=*/true, distXYZ );
      _pos.back().SetCoord( u, 0, 0 );
      SMDS_EdgePosition* pos = static_cast<SMDS_EdgePosition*>( n->GetPosition().get() );
      pos->SetUParameter( u );
    }
    else //  TopAbs_FACE
    {
      gp_XY uv( Precision::Infinite(), 0 );
      helper.CheckNodeUV( TopoDS::Face( _sWOL ), n, uv, 1e-10, /*force=*/true, distXYZ );
      _pos.back().SetCoord( uv.X(), uv.Y(), 0 );
      SMDS_FacePosition* pos = static_cast<SMDS_FacePosition*>( n->GetPosition().get() );
      pos->SetUParameter( uv.X() );
      pos->SetVParameter( uv.Y() );
    }
    n->setXYZ( distXYZ[1], distXYZ[2], distXYZ[3]);
    _len += oldXYZ.Distance( n );
  }
  dumpMove( n ); //debug
}

//================================================================================
/*!
 * \brief Remove last inflation step
 */
//================================================================================

void _LayerEdge::InvalidateStep( int curStep )
{
  if ( _pos.size() > curStep+1 )
  {
    _pos.resize( curStep+1 );
    gp_Pnt nXYZ = _pos.back();
    SMDS_MeshNode* n = const_cast< SMDS_MeshNode*>( _nodes.back() );
    if ( !_sWOL.IsNull() )
    {
      TopLoc_Location loc;
      if ( _sWOL.ShapeType() == TopAbs_EDGE )
      {
        double f,l;
        Handle(Geom_Curve) curve = BRep_Tool::Curve( TopoDS::Edge( _sWOL ), loc, f,l);
        nXYZ = curve->Value( nXYZ.X() ).Transformed( loc );
      }
      else
      {
        Handle(Geom_Surface) surface = BRep_Tool::Surface( TopoDS::Face(_sWOL), loc );
        nXYZ = surface->Value( nXYZ.X(), nXYZ.Y() ).Transformed( loc );
      }
    }
    n->setXYZ( nXYZ.X(), nXYZ.Y(), nXYZ.Z() );
  }
}

//================================================================================
/*!
 * \brief Create layers of prisms
 */
//================================================================================

bool _ViscousBuilder::refine(_SolidData& data)
{
  SMESH_MesherHelper helper( *_mesh );
  helper.SetSubShape( data._solid );
  helper.SetElementsOnShape(false);

  Handle(Geom_Curve) curve;
  Handle(Geom_Surface) surface;
  TopoDS_Edge geomEdge;
  TopoDS_Face geomFace;
  TopLoc_Location loc;
  double f,l, u/*, distXYZ[4]*/;
  gp_XY uv;
  bool isOnEdge;

  for ( unsigned i = 0; i < data._edges.size(); ++i )
  {
    _LayerEdge& edge = *data._edges[i];

    // get accumulated length of segments
    vector< double > segLen( edge._pos.size() );
    segLen[0] = 0.0;
    for ( unsigned j = 1; j < edge._pos.size(); ++j )
      segLen[j] = segLen[j-1] + (edge._pos[j-1] - edge._pos[j] ).Modulus();

    // allocate memory for new nodes if it is not yet refined
    const SMDS_MeshNode* tgtNode = edge._nodes.back();
    if ( edge._nodes.size() == 2 )
    {
      edge._nodes.resize( data._hyp->GetNumberLayers() + 1, 0 );
      edge._nodes[1] = 0;
      edge._nodes.back() = tgtNode;
    }
    if ( !edge._sWOL.IsNull() )
    {
      isOnEdge = ( edge._sWOL.ShapeType() == TopAbs_EDGE );
      // restore position of the last node
//       gp_Pnt p;
      if ( isOnEdge )
      {
        geomEdge = TopoDS::Edge( edge._sWOL );
        curve = BRep_Tool::Curve( geomEdge, loc, f,l);
//         double u = helper.GetNodeU( tgtNode );
//         p = curve->Value( u );
      }
      else
      {
        geomFace = TopoDS::Face( edge._sWOL );
        surface = BRep_Tool::Surface( geomFace, loc );
//         gp_XY uv = helper.GetNodeUV( tgtNode );
//         p = surface->Value( uv.X(), uv.Y() );
      }
//       p.Transform( loc );
//       const_cast< SMDS_MeshNode* >( tgtNode )->setXYZ( p.X(), p.Y(), p.Z() );
    }
    // calculate height of the first layer
    const double T = segLen.back(); //data._hyp.GetTotalThickness();
    const double f = data._hyp->GetStretchFactor();
    const int    N = data._hyp->GetNumberLayers();
    double h0 = T * ( f - 1 )/( pow( f, N ) - 1 );

    const double zeroLen = std::numeric_limits<double>::min();

    // create intermediate nodes
    double hSum = 0, hi = h0/f;
    unsigned iSeg = 1;
    for ( unsigned iStep = 1; iStep < edge._nodes.size(); ++iStep )
    {
      // compute an intermediate position
      hi *= f;
      hSum += hi;
      while ( hSum > segLen[iSeg] && iSeg < segLen.size()-1)
        ++iSeg;
      int iPrevSeg = iSeg-1;
      while ( fabs( segLen[iPrevSeg] - segLen[iSeg]) <= zeroLen && iPrevSeg > 0 )
        --iPrevSeg;
      double r = ( segLen[iSeg] - hSum ) / ( segLen[iSeg] - segLen[iPrevSeg] );
      gp_Pnt pos = r * edge._pos[iPrevSeg] + (1-r) * edge._pos[iSeg];

      SMDS_MeshNode*& node = const_cast< SMDS_MeshNode*& >(edge._nodes[ iStep ]);
      if ( !edge._sWOL.IsNull() )
      {
        // compute XYZ by parameters <pos>
        if ( isOnEdge )
        {
          u = pos.X();
          pos = curve->Value( u ).Transformed(loc);
        }
        else
        {
          uv.SetCoord( pos.X(), pos.Y() );
          pos = surface->Value( pos.X(), pos.Y() ).Transformed(loc);
        }
      }
      // create or update the node
      if ( !node )
      {
        node = helper.AddNode( pos.X(), pos.Y(), pos.Z());
        if ( !edge._sWOL.IsNull() )
        {
          if ( isOnEdge )
            getMeshDS()->SetNodeOnEdge( node, geomEdge, u );
          else
            getMeshDS()->SetNodeOnFace( node, geomFace, uv.X(), uv.Y() );
        }
        else
        {
          getMeshDS()->SetNodeInVolume( node, helper.GetSubShapeID() );
        }
      }
      else
      {
        if ( !edge._sWOL.IsNull() )
        {
          // make average pos from new and current parameters
          if ( isOnEdge )
          {
            u = 0.5 * ( u + helper.GetNodeU( geomEdge, node ));
            pos = curve->Value( u );
          }
          else
          {
            uv = 0.5 * ( uv + helper.GetNodeUV( geomFace, node ));
            pos = surface->Value( uv.X(), uv.Y());
          }
        }
        node->setXYZ( pos.X(), pos.Y(), pos.Z() );
      }
    }
  }

  // TODO: make quadratic prisms and polyhedrons(?)

  TopExp_Explorer exp( data._solid, TopAbs_FACE );
  for ( ; exp.More(); exp.Next() )
  {
    if ( _ignoreShapeIds.count( getMeshDS()->ShapeToIndex( exp.Current() )))
      continue;
    SMESHDS_SubMesh* fSubM = getMeshDS()->MeshElements( exp.Current() );
    SMDS_ElemIteratorPtr fIt = fSubM->GetElements();
    vector< vector<const SMDS_MeshNode*>* > nnVec;
    while ( fIt->more() )
    {
      const SMDS_MeshElement* face = fIt->next();
      int nbNodes = face->NbCornerNodes();
      nnVec.resize( nbNodes );
      SMDS_ElemIteratorPtr nIt = face->nodesIterator();
      for ( int iN = 0; iN < nbNodes; ++iN )
      {
        const SMDS_MeshNode* n = static_cast<const SMDS_MeshNode*>( nIt->next() );
        nnVec[ iN ] = & data._n2eMap[ n ]->_nodes;
      }

      int nbZ = nnVec[0]->size();
      switch ( nbNodes )
      {
      case 3:
        for ( int iZ = 1; iZ < nbZ; ++iZ )
          helper.AddVolume( (*nnVec[0])[iZ-1], (*nnVec[1])[iZ-1], (*nnVec[2])[iZ-1],
                            (*nnVec[0])[iZ],   (*nnVec[1])[iZ],   (*nnVec[2])[iZ]);
        break;
      case 4:
        for ( int iZ = 1; iZ < nbZ; ++iZ )
          helper.AddVolume( (*nnVec[0])[iZ-1], (*nnVec[1])[iZ-1],
                            (*nnVec[2])[iZ-1], (*nnVec[3])[iZ-1],
                            (*nnVec[0])[iZ],   (*nnVec[1])[iZ],
                            (*nnVec[2])[iZ],   (*nnVec[3])[iZ]);
        break;
      default:
        return error("Not supported type of element", &data);
      }
    }
  }
  return true;
}

//================================================================================
/*!
 * \brief Shrink 2D mesh on faces to let space for inflated layers
 */
//================================================================================

bool _ViscousBuilder::shrink()
{
  // make map of (ids of faces to shrink mesh on) to (_SolidData containing _LayerEdge's
  // inflated along face or edge)
  map< TGeomID, _SolidData* > f2sdMap;
  for ( unsigned i = 0 ; i < _sdVec.size(); ++i )
  {
    _SolidData& data = _sdVec[i];
    map< TGeomID, TopoDS_Shape >::iterator s2s = data._shrinkShape2Shape.begin();
    for (; s2s != data._shrinkShape2Shape.end(); ++s2s )
      if ( s2s->second.ShapeType() == TopAbs_FACE )
        f2sdMap.insert( make_pair( getMeshDS()->ShapeToIndex( s2s->second ), &data ));
  }

  SMESH_MesherHelper helper( *_mesh );

  // EDGE's to shrink
  map< int, _Shrinker1D > e2shrMap;

  // loop on FACES to srink mesh on
  map< TGeomID, _SolidData* >::iterator f2sd = f2sdMap.begin();
  for ( ; f2sd != f2sdMap.end(); ++f2sd )
  {
    _SolidData&     data = *f2sd->second;
    TNode2Edge&   n2eMap = data._n2eMap;
    const TopoDS_Face& F = TopoDS::Face( getMeshDS()->IndexToShape( f2sd->first ));
    const bool   reverse = ( data._reversedFaceIds.count( f2sd->first ));

    Handle(Geom_Surface) surface = BRep_Tool::Surface(F);

    SMESH_subMesh*     sm = _mesh->GetSubMesh( F );
    SMESHDS_SubMesh* smDS = sm->GetSubMeshDS();

    helper.SetSubShape(F);

    // ===========================
    // Prepare data for shrinking
    // ===========================

    // Collect nodes to smooth as src nodes are not yet replaced by tgt ones
    vector < const SMDS_MeshNode* > smoothNodes;
    {
      SMDS_NodeIteratorPtr nIt = smDS->GetNodes();
      while ( nIt->more() )
      {
        const SMDS_MeshNode* n = nIt->next();
        if ( n->NbInverseElements( SMDSAbs_Face ) > 0 )
          smoothNodes.push_back( n );
      }
    }
    // Find out face orientation
    double refSign = 1;
    const set<TGeomID> ignoreShapes;
    if ( !smoothNodes.empty() )
    {
      gp_XY uv = helper.GetNodeUV( F, smoothNodes[0] );
      vector<_Simplex> simplices;
      getSimplices( smoothNodes[0], simplices, ignoreShapes );
      if ( simplices[0].IsForward(uv, F, helper,refSign) != (!reverse))
        refSign = -1;
    }

    // Find _LayerEdge's inflated along F
    vector< _LayerEdge* > lEdges;
    {
      SMESH_subMeshIteratorPtr subIt =
        sm->getDependsOnIterator(/*includeSelf=*/false, /*complexShapeFirst=*/false);
      while ( subIt->more() )
      {
        SMESH_subMesh* sub = subIt->next();
        SMESHDS_SubMesh* subDS = sub->GetSubMeshDS();
        if ( subDS->NbNodes() == 0 || !n2eMap.count( subDS->GetNodes()->next() ))
          continue;
        SMDS_NodeIteratorPtr nIt = subDS->GetNodes();
        while ( nIt->more() )
        {
          _LayerEdge* edge = n2eMap[ nIt->next() ];
          lEdges.push_back( edge );
          prepareEdgeToShrink( *edge, F, helper, smDS );
        }
      }
    }

    // Replace source nodes by target nodes in mesh faces to shrink
    const SMDS_MeshNode* nodes[20];
    for ( unsigned i = 0; i < lEdges.size(); ++i )
    {
      _LayerEdge& edge = *lEdges[i];
      const SMDS_MeshNode* srcNode = edge._nodes[0];
      const SMDS_MeshNode* tgtNode = edge._nodes.back();
      SMDS_ElemIteratorPtr fIt = srcNode->GetInverseElementIterator(SMDSAbs_Face);
      while ( fIt->more() )
      {
        const SMDS_MeshElement* f = fIt->next();
        if ( !smDS->Contains( f ))
          continue;
        SMDS_ElemIteratorPtr nIt = f->nodesIterator();
        for ( int iN = 0; iN < f->NbNodes(); ++iN )
        {
          const SMDS_MeshNode* n = static_cast<const SMDS_MeshNode*>( nIt->next() );
          nodes[iN] = ( n == srcNode ? tgtNode : n );
        }
        helper.GetMeshDS()->ChangeElementNodes( f, nodes, f->NbNodes() );
      }
    }

    // Create _SmoothNode's on face F
    vector< _SmoothNode > nodesToSmooth( smoothNodes.size() );
    {
      dumpFunction(SMESH_Comment("beforeShrinkFace")<<f2sd->first); // debug
      for ( unsigned i = 0; i < smoothNodes.size(); ++i )
      {
        const SMDS_MeshNode* n = smoothNodes[i];
        nodesToSmooth[ i ]._node = n;
        // src nodes must be replaced by tgt nodes to have tgt nodes in _simplices
        getSimplices( n, nodesToSmooth[ i ]._simplices, ignoreShapes );
        dumpMove( n );
      }
      dumpFunctionEnd();
    }
    //if ( nodesToSmooth.empty() ) continue;

    // Find EDGE's to shrink
    set< _Shrinker1D* > eShri1D;
    {
      for ( unsigned i = 0; i < lEdges.size(); ++i )
      {
        _LayerEdge* edge = lEdges[i];
        if ( edge->_sWOL.ShapeType() == TopAbs_EDGE )
        {
          TGeomID edgeIndex = getMeshDS()->ShapeToIndex( edge->_sWOL );
          _Shrinker1D& srinker = e2shrMap[ edgeIndex ];
          eShri1D.insert( & srinker );
          srinker.AddEdge( edge, helper );
          // restore params of nodes on EGDE if the EDGE has been  already
          // srinked while srinking another FACE
          srinker.RestoreParams();
        }
      }
    }

    // ==================
    // Perform shrinking
    // ==================

    bool shrinked = true;
    int badNb = 1, shriStep=0, smooStep=0;
    while ( shrinked )
    {
      // Move boundary nodes (actually just set new UV)
      // -----------------------------------------------
      dumpFunction(SMESH_Comment("moveBoundaryOnF")<<f2sd->first<<"_st"<<shriStep++ ); // debug
      shrinked = false;
      for ( unsigned i = 0; i < lEdges.size(); ++i )
      {
        shrinked |= lEdges[i]->SetNewLength2d( surface,F,helper );
      }
      dumpFunctionEnd();
      if ( !shrinked )
        break;

      // Move nodes on EDGE's
      set< _Shrinker1D* >::iterator shr = eShri1D.begin();
      for ( ; shr != eShri1D.end(); ++shr )
        (*shr)->Compute( /*set3D=*/false, helper );

      // Smoothing in 2D
      // -----------------
      int nbNoImpSteps = 0;
      bool moved = true;
      while (( nbNoImpSteps < 5 && badNb > 0) && moved)
      {
        dumpFunction(SMESH_Comment("shrinkFace")<<f2sd->first<<"_st"<<++smooStep); // debug

        int oldBadNb = badNb;
        badNb = 0;
        moved = false;
        for ( unsigned i = 0; i < nodesToSmooth.size(); ++i )
        {
          moved |= nodesToSmooth[i].Smooth( badNb,surface,helper,refSign,/*set3D=*/false );
        }
        if ( badNb < oldBadNb )
          nbNoImpSteps = 0;
        else
          nbNoImpSteps++;

        dumpFunctionEnd();
      }
      if ( badNb > 0 )
        return error(SMESH_Comment("Can't shrink 2D mesh on face ") << f2sd->first, 0 );
    }
    // No wrongly shaped faces remain; final smooth. Set node XYZ
    for ( int st = 3; st; --st )
    {
      dumpFunction(SMESH_Comment("shrinkFace")<<f2sd->first<<"_st"<<++smooStep); // debug
      for ( unsigned i = 0; i < nodesToSmooth.size(); ++i )
        nodesToSmooth[i].Smooth( badNb,surface,helper,refSign,/*set3D=*/st==0 );
      dumpFunctionEnd();
    }
    // Set event listener to clear FACE sub-mesh together with SOLID sub-mesh
    _SrinkShapeListener::ToClearSubMeshWithSolid( sm, data._solid );

  }// loop on FACES to srink mesh on


  // Replace source nodes by target nodes in shrinked mesh edges

  map< int, _Shrinker1D >::iterator e2shr = e2shrMap.begin();
  for ( ; e2shr != e2shrMap.end(); ++e2shr )
    e2shr->second.SwapSrcTgt( getMeshDS() );

  return true;
}

//================================================================================
/*!
 * \brief Computes 2d shrink direction and finds nodes limiting shrinking
 */
//================================================================================

bool _ViscousBuilder::prepareEdgeToShrink( _LayerEdge&            edge,
                                           const TopoDS_Face&     F,
                                           SMESH_MesherHelper&    helper,
                                           const SMESHDS_SubMesh* faceSubMesh)
{
  const SMDS_MeshNode* srcNode = edge._nodes[0];
  const SMDS_MeshNode* tgtNode = edge._nodes.back();

  edge._pos.clear();

  if ( edge._sWOL.ShapeType() == TopAbs_FACE )
  {
    gp_XY srcUV = helper.GetNodeUV( F, srcNode );
    gp_XY tgtUV = helper.GetNodeUV( F, tgtNode );
    gp_Vec2d uvDir( srcUV, tgtUV );
    double uvLen = uvDir.Magnitude();
    uvDir /= uvLen;
    edge._normal.SetCoord( uvDir.X(),uvDir.Y(), 0);

    // IMPORTANT to have src nodes NOT yet REPLACED by tgt nodes in shrinked faces
    multimap< double, const SMDS_MeshNode* > proj2node;
    SMDS_ElemIteratorPtr fIt = srcNode->GetInverseElementIterator(SMDSAbs_Face);
    while ( fIt->more() )
    {
      const SMDS_MeshElement* f = fIt->next();
      if ( !faceSubMesh->Contains( f )) continue;
      const int nbNodes = f->NbCornerNodes();
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* n = f->GetNode(i);
        if ( n->GetPosition()->GetTypeOfPosition() != SMDS_TOP_FACE || n == srcNode)
          continue;
        gp_Pnt2d uv = helper.GetNodeUV( F, n );
        gp_Vec2d uvDirN( srcUV, uv );
        double proj = uvDirN * uvDir;
        proj2node.insert( make_pair( proj, n ));
      }
    }

    multimap< double, const SMDS_MeshNode* >::iterator p2n = proj2node.begin(), p2nEnd;
    const double minProj = p2n->first;
    const double projThreshold = 1.1 * uvLen;
    if ( minProj > projThreshold )
    {
      // tgtNode is located so that it does not make faces with wrong orientation
      return true;
    }
    edge._pos.resize(1);
    edge._pos[0].SetCoord( tgtUV.X(), tgtUV.Y(), 0 );

    // store most risky nodes in _simplices
    p2nEnd = proj2node.lower_bound( projThreshold );
    int nbSimpl = ( std::distance( p2n, p2nEnd ) + 1) / 2;
    edge._simplices.resize( nbSimpl );
    for ( int i = 0; i < nbSimpl; ++i )
    {
      edge._simplices[i]._nPrev = p2n->second;
      if ( ++p2n != p2nEnd )
        edge._simplices[i]._nNext = p2n->second;
    }
    // set UV of source node to target node
    SMDS_FacePosition* pos = static_cast<SMDS_FacePosition*>( tgtNode->GetPosition().get() );
    pos->SetUParameter( srcUV.X() );
    pos->SetVParameter( srcUV.Y() );
  }
  else // _sWOL is TopAbs_EDGE
  {
    TopoDS_Edge E = TopoDS::Edge( edge._sWOL);
    SMESHDS_SubMesh* edgeSM = getMeshDS()->MeshElements( E );
    if ( !edgeSM || edgeSM->NbElements() == 0 )
      return error(SMESH_Comment("Not meshed EDGE ") << getMeshDS()->ShapeToIndex( E ), 0);

    const SMDS_MeshNode* n2 = 0;
    SMDS_ElemIteratorPtr eIt = srcNode->GetInverseElementIterator(SMDSAbs_Edge);
    while ( eIt->more() && !n2 )
    {
      const SMDS_MeshElement* e = eIt->next();
      if ( !edgeSM->Contains(e)) continue;
      n2 = e->GetNode( 0 );
      if ( n2 == srcNode ) n2 = e->GetNode( 1 );
    }
    if ( !n2 )
      return error(SMESH_Comment("Wrongly meshed EDGE ") << getMeshDS()->ShapeToIndex( E ), 0);

    double uSrc = helper.GetNodeU( E, srcNode, n2 );
    double uTgt = helper.GetNodeU( E, tgtNode, srcNode );
    double u2   = helper.GetNodeU( E, n2,      srcNode );

    if ( fabs( uSrc-uTgt ) < 0.99 * fabs( uSrc-u2 ))
    {
      // tgtNode is located so that it does not make faces with wrong orientation
      return true;
    }
    edge._pos.resize(1);
    edge._pos[0].SetCoord( U_TGT, uTgt );
    edge._pos[0].SetCoord( U_SRC, uSrc );
    edge._pos[0].SetCoord( LEN_TGT, fabs( uSrc-uTgt ));

    edge._simplices.resize( 1 );
    edge._simplices[0]._nPrev = n2;

    // set UV of source node to target node
    SMDS_EdgePosition* pos = static_cast<SMDS_EdgePosition*>( tgtNode->GetPosition().get() );
    pos->SetUParameter( uSrc );
  }
  return true;

  //================================================================================
  /*!
   * \brief Compute positions (UV) to set to a node on edge moved during shrinking
   */
  //================================================================================
  
  // Compute UV to follow during shrinking

//   const SMDS_MeshNode* srcNode = edge._nodes[0];
//   const SMDS_MeshNode* tgtNode = edge._nodes.back();

//   gp_XY srcUV = helper.GetNodeUV( F, srcNode );
//   gp_XY tgtUV = helper.GetNodeUV( F, tgtNode );
//   gp_Vec2d uvDir( srcUV, tgtUV );
//   double uvLen = uvDir.Magnitude();
//   uvDir /= uvLen;

//   // Select shrinking step such that not to make faces with wrong orientation.
//   // IMPORTANT to have src nodes NOT yet REPLACED by tgt nodes in shrinked faces
//   const double minStepSize = uvLen / 20;
//   double stepSize = uvLen;
//   SMDS_ElemIteratorPtr fIt = srcNode->GetInverseElementIterator(SMDSAbs_Face);
//   while ( fIt->more() )
//   {
//     const SMDS_MeshElement* f = fIt->next();
//     if ( !faceSubMesh->Contains( f )) continue;
//     const int nbNodes = f->NbCornerNodes();
//     for ( int i = 0; i < nbNodes; ++i )
//     {
//       const SMDS_MeshNode* n = f->GetNode(i);
//       if ( n->GetPosition()->GetTypeOfPosition() != SMDS_TOP_FACE || n == srcNode)
//         continue;
//       gp_XY uv = helper.GetNodeUV( F, n );
//       gp_Vec2d uvDirN( srcUV, uv );
//       double proj = uvDirN * uvDir;
//       if ( proj < stepSize && proj > minStepSize )
//         stepSize = proj;
//     }
//   }
//   stepSize *= 0.8;

//   const int nbSteps = ceil( uvLen / stepSize );
//   gp_XYZ srcUV0( srcUV.X(), srcUV.Y(), 0 );
//   gp_XYZ tgtUV0( tgtUV.X(), tgtUV.Y(), 0 );
//   edge._pos.resize( nbSteps );
//   edge._pos[0] = tgtUV0;
//   for ( int i = 1; i < nbSteps; ++i )
//   {
//     double r = i / double( nbSteps );
//     edge._pos[i] = (1-r) * tgtUV0 + r * srcUV0;
//   }
//   return true;
}

//================================================================================
/*!
 * \brief Move target node to it's final position on the FACE during shrinking
 */
//================================================================================

bool _LayerEdge::SetNewLength2d( Handle(Geom_Surface)& surface,
                                 const TopoDS_Face&    F,
                                 SMESH_MesherHelper&   helper )
{
  if ( _pos.empty() )
    return false; // already at the target position

  SMDS_MeshNode* tgtNode = const_cast< SMDS_MeshNode*& >( _nodes.back() );

  if ( _sWOL.ShapeType() == TopAbs_FACE )
  {
    gp_XY    curUV = helper.GetNodeUV( F, tgtNode );
    gp_Pnt2d tgtUV( _pos[0].X(), _pos[0].Y());
    gp_Vec2d uvDir( _normal.X(), _normal.Y() );
    const double uvLen = tgtUV.Distance( curUV );

    // Select shrinking step such that not to make faces with wrong orientation.
    const double kSafe = 0.8;
    const double minStepSize = uvLen / 10;
    double stepSize = uvLen;
    for ( unsigned i = 0; i < _simplices.size(); ++i )
    {
      const SMDS_MeshNode* nn[2] = { _simplices[i]._nPrev, _simplices[i]._nNext };
      for ( int j = 0; j < 2; ++j )
        if ( const SMDS_MeshNode* n = nn[j] )
        {
          gp_XY uv = helper.GetNodeUV( F, n );
          gp_Vec2d uvDirN( curUV, uv );
          double proj = uvDirN * uvDir * kSafe;
          if ( proj < stepSize && proj > minStepSize )
            stepSize = proj;
        }
    }

    gp_Pnt2d newUV;
    if ( stepSize == uvLen )
    {
      newUV = tgtUV;
      _pos.clear();
    }
    else
    {
      newUV = curUV + uvDir.XY() * stepSize;
    }

    SMDS_FacePosition* pos = static_cast<SMDS_FacePosition*>( tgtNode->GetPosition().get() );
    pos->SetUParameter( newUV.X() );
    pos->SetVParameter( newUV.Y() );

#ifdef __PY_DUMP
    gp_Pnt p = surface->Value( newUV.X(), newUV.Y() );
    tgtNode->setXYZ( p.X(), p.Y(), p.Z() );
    dumpMove( tgtNode );
#endif
  }
  else // _sWOL is TopAbs_EDGE
  {
    TopoDS_Edge E = TopoDS::Edge( _sWOL );
    const SMDS_MeshNode* n2 = _simplices[0]._nPrev;

    const double u2 = helper.GetNodeU( E, n2, tgtNode );
    const double uSrc   = _pos[0].Coord( U_SRC );
    const double lenTgt = _pos[0].Coord( LEN_TGT );

    double newU = _pos[0].Coord( U_TGT );
    if ( lenTgt < 0.99 * fabs( uSrc-u2 ))
    {
      _pos.clear();
    }
    else
    {
      newU = 0.1 * uSrc + 0.9 * u2;
    }
    SMDS_EdgePosition* pos = static_cast<SMDS_EdgePosition*>( tgtNode->GetPosition().get() );
    pos->SetUParameter( newU );
#ifdef __PY_DUMP
    gp_XY newUV = helper.GetNodeUV( F, tgtNode, _nodes[0]);
    gp_Pnt p = surface->Value( newUV.X(), newUV.Y() );
    tgtNode->setXYZ( p.X(), p.Y(), p.Z() );
    dumpMove( tgtNode );
#endif
  }
  return true;
}

//================================================================================
/*!
 * \brief Perform laplacian smooth on the FACE
 *  \retval bool - true if the node has been moved
 */
//================================================================================

bool _SmoothNode::Smooth(int&                  badNb,
                         Handle(Geom_Surface)& surface,
                         SMESH_MesherHelper&   helper,
                         const double          refSign,
                         bool                  set3D)
{
  const TopoDS_Face& face = TopoDS::Face( helper.GetSubShape() );

  // compute new UV for the node
  gp_XY newPos (0,0);
  for ( unsigned i = 0; i < _simplices.size(); ++i )
    newPos += helper.GetNodeUV( face, _simplices[i]._nPrev );
  newPos /= _simplices.size();

  // count quality metrics (orientation) of triangles around the node
  int nbOkBefore = 0;
  gp_XY tgtUV = helper.GetNodeUV( face, _node );
  for ( unsigned i = 0; i < _simplices.size(); ++i )
    nbOkBefore += _simplices[i].IsForward( tgtUV, face, helper, refSign );

  int nbOkAfter = 0;
  for ( unsigned i = 0; i < _simplices.size(); ++i )
    nbOkAfter += _simplices[i].IsForward( newPos, face, helper, refSign );

  if ( nbOkAfter < nbOkBefore )
    return false;

  SMDS_FacePosition* pos = static_cast<SMDS_FacePosition*>( _node->GetPosition().get() );
  pos->SetUParameter( newPos.X() );
  pos->SetVParameter( newPos.Y() );

#ifdef __PY_DUMP
  set3D = true;
#endif
  if ( set3D )
  {
    gp_Pnt p = surface->Value( newPos.X(), newPos.Y() );
    const_cast< SMDS_MeshNode* >( _node )->setXYZ( p.X(), p.Y(), p.Z() );
    dumpMove( _node );
  }

  badNb += _simplices.size() - nbOkAfter;
  return ( (tgtUV-newPos).SquareModulus() > 1e-10 );
}

//================================================================================
/*!
 * \brief Delete _SolidData
 */
//================================================================================

_SolidData::~_SolidData()
{
  for ( unsigned i = 0; i < _edges.size(); ++i )
  {
    if ( _edges[i] && _edges[i]->_2neibors )
      delete _edges[i]->_2neibors;
    delete _edges[i];
  }
  _edges.clear();
}
//================================================================================
/*!
 * \brief Add a _LayerEdge inflated along the EDGE
 */
//================================================================================

void _Shrinker1D::AddEdge( const _LayerEdge* e, SMESH_MesherHelper& helper )
{
  // init
  if ( _nodes.empty() )
  {
    _edges[0] = _edges[1] = 0;
    _done = false;
  }
  // check _LayerEdge
  if ( e == _edges[0] || e == _edges[1] )
    return;
  if ( e->_sWOL.IsNull() || e->_sWOL.ShapeType() != TopAbs_EDGE )
    throw SALOME_Exception(LOCALIZED("Wrong _LayerEdge is added"));
  if ( _edges[0] && _edges[0]->_sWOL != e->_sWOL )
    throw SALOME_Exception(LOCALIZED("Wrong _LayerEdge is added"));

  // store _LayerEdge
  const TopoDS_Edge& E = TopoDS::Edge( e->_sWOL );
  double f,l;
  BRep_Tool::Range( E, f,l );
  double u = helper.GetNodeU( E, e->_nodes[0], e->_nodes.back());
  _edges[ u < 0.5*(f+l) ? 0 : 1 ] = e;

  // Update _nodes

  const SMDS_MeshNode* tgtNode0 = _edges[0] ? _edges[0]->_nodes.back() : 0;
  const SMDS_MeshNode* tgtNode1 = _edges[1] ? _edges[1]->_nodes.back() : 0;

  if ( _nodes.empty() )
  {
    SMESHDS_SubMesh * eSubMesh = helper.GetMeshDS()->MeshElements( E );
    if ( !eSubMesh || eSubMesh->NbNodes() < 1 )
      return;
    TopLoc_Location loc;
    Handle(Geom_Curve) C = BRep_Tool::Curve(E, loc, f,l);
    GeomAdaptor_Curve aCurve(C);
    const double totLen = GCPnts_AbscissaPoint::Length(aCurve, f, l);

    int nbExpectNodes = eSubMesh->NbNodes() - e->_nodes.size();
    _initU  .reserve( nbExpectNodes );
    _normPar.reserve( nbExpectNodes );
    _nodes  .reserve( nbExpectNodes );
    SMDS_NodeIteratorPtr nIt = eSubMesh->GetNodes();
    while ( nIt->more() )
    {
      const SMDS_MeshNode* node = nIt->next();
      if ( node->NbInverseElements(SMDSAbs_Edge) == 0 ||
           node == tgtNode0 || node == tgtNode1 )
        continue; // refinement nodes
      _nodes.push_back( node );
      _initU.push_back( helper.GetNodeU( E, node ));
      double len = GCPnts_AbscissaPoint::Length(aCurve, f, _initU.back());
      _normPar.push_back(  len / totLen );
    }
  }
  else
  {
    // remove target node of the _LayerEdge from _nodes
    int nbFound = 0;
    for ( unsigned i = 0; i < _nodes.size(); ++i )
      if ( !_nodes[i] || _nodes[i] == tgtNode0 || _nodes[i] == tgtNode1 )
        _nodes[i] = 0, nbFound++;
    if ( nbFound == _nodes.size() )
      _nodes.clear();
  }
}

//================================================================================
/*!
 * \brief Move nodes on EDGE from ends where _LayerEdge's are inflated
 */
//================================================================================

void _Shrinker1D::Compute(bool set3D, SMESH_MesherHelper& helper)
{
  if ( _done || _nodes.empty())
    return;
  const _LayerEdge* e = _edges[0];
  if ( !e ) e = _edges[1];
  if ( !e ) return;

  _done =  (( !_edges[0] || _edges[0]->_pos.empty() ) &&
            ( !_edges[1] || _edges[1]->_pos.empty() ));

  const TopoDS_Edge& E = TopoDS::Edge( e->_sWOL );
  double f,l;
  if ( set3D || _done )
  {
    Handle(Geom_Curve) C = BRep_Tool::Curve(E, f,l);
    GeomAdaptor_Curve aCurve(C);

    if ( _edges[0] )
      f = helper.GetNodeU( E, _edges[0]->_nodes.back(), _nodes[0] );
    if ( _edges[1] )
      l = helper.GetNodeU( E, _edges[1]->_nodes.back(), _nodes.back() );
    double totLen = GCPnts_AbscissaPoint::Length( aCurve, f, l );

    for ( unsigned i = 0; i < _nodes.size(); ++i )
    {
      if ( !_nodes[i] ) continue;
      double len = totLen * _normPar[i];
      GCPnts_AbscissaPoint discret( aCurve, len, f );
      if ( !discret.IsDone() )
        return throw SALOME_Exception(LOCALIZED("GCPnts_AbscissaPoint failed"));
      double u = discret.Parameter();
      SMDS_EdgePosition* pos = static_cast<SMDS_EdgePosition*>( _nodes[i]->GetPosition().get() );
      pos->SetUParameter( u );
      gp_Pnt p = C->Value( u );
      const_cast< SMDS_MeshNode*>( _nodes[i] )->setXYZ( p.X(), p.Y(), p.Z() );
    }
  }
  else
  {
    BRep_Tool::Range( E, f,l );
    if ( _edges[0] )
      f = helper.GetNodeU( E, _edges[0]->_nodes.back(), _nodes[0] );
    if ( _edges[1] )
      l = helper.GetNodeU( E, _edges[1]->_nodes.back(), _nodes.back() );
    
    for ( unsigned i = 0; i < _nodes.size(); ++i )
    {
      if ( !_nodes[i] ) continue;
      double u = f * ( 1-_normPar[i] ) + l * _normPar[i];
      SMDS_EdgePosition* pos = static_cast<SMDS_EdgePosition*>( _nodes[i]->GetPosition().get() );
      pos->SetUParameter( u );
    }
  }
}

//================================================================================
/*!
 * \brief Restore initial parameters of nodes on EDGE
 */
//================================================================================

void _Shrinker1D::RestoreParams()
{
  if ( _done )
    for ( unsigned i = 0; i < _nodes.size(); ++i )
    {
      if ( !_nodes[i] ) continue;
      SMDS_EdgePosition* pos = static_cast<SMDS_EdgePosition*>( _nodes[i]->GetPosition().get() );
      pos->SetUParameter( _initU[i] );
    }
  _done = false;
}
//================================================================================
/*!
 * \brief Replace source nodes by target nodes in shrinked mesh edges
 */
//================================================================================

void _Shrinker1D::SwapSrcTgt( SMESHDS_Mesh* mesh )
{
  const SMDS_MeshNode* nodes[3];
  for ( int i = 0; i < 2; ++i )
  {
    if ( !_edges[i] ) continue;

    SMESHDS_SubMesh * eSubMesh = mesh->MeshElements( _edges[i]->_sWOL );
    if ( !eSubMesh ) return;
    const SMDS_MeshNode* srcNode = _edges[i]->_nodes[0];
    const SMDS_MeshNode* tgtNode = _edges[i]->_nodes.back();
    SMDS_ElemIteratorPtr eIt = srcNode->GetInverseElementIterator(SMDSAbs_Edge);
    while ( eIt->more() )
    {
      const SMDS_MeshElement* e = eIt->next();
      if ( !eSubMesh->Contains( e ))
          continue;
      SMDS_ElemIteratorPtr nIt = e->nodesIterator();
      for ( int iN = 0; iN < e->NbNodes(); ++iN )
      {
        const SMDS_MeshNode* n = static_cast<const SMDS_MeshNode*>( nIt->next() );
        nodes[iN] = ( n == srcNode ? tgtNode : n );
      }
      mesh->ChangeElementNodes( e, nodes, e->NbNodes() );
    }
  }
}
