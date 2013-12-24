// Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : StdMeshers_Cartesian_3D.cxx
//  Module : SMESH
//
#include "StdMeshers_Cartesian_3D.hxx"

#include "SMDS_MeshNode.hxx"
#include "SMESH_Block.hxx"
#include "SMESH_Comment.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "StdMeshers_CartesianParameters3D.hxx"

#include <utilities.h>
#include <Utils_ExceptHandlers.hxx>
#include <Basics_OCCTVersion.hxx>

#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <ElSLib.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <IntAna_IntConicQuad.hxx>
#include <IntAna_IntLinTorus.hxx>
#include <IntAna_Quadric.hxx>
#include <IntCurveSurface_TransitionOnCurve.hxx>
#include <IntCurvesFace_Intersector.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_TShape.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>

#undef WITH_TBB
#ifdef WITH_TBB
#include <tbb/parallel_for.h>
//#include <tbb/enumerable_thread_specific.h>
#endif

using namespace std;

#ifdef _DEBUG_
//#define _MY_DEBUG_
#endif

#if OCC_VERSION_LARGE <= 0x06050300
// workaround is required only for OCCT6.5.3 and older (see OCC22809)
#define ELLIPSOLID_WORKAROUND
#endif

#ifdef ELLIPSOLID_WORKAROUND
#include <BRepIntCurveSurface_Inter.hxx>
#include <BRepTopAdaptor_TopolTool.hxx>
#include <BRepAdaptor_HSurface.hxx>
#endif

//=============================================================================
/*!
 * Constructor
 */
//=============================================================================

StdMeshers_Cartesian_3D::StdMeshers_Cartesian_3D(int hypId, int studyId, SMESH_Gen * gen)
  :SMESH_3D_Algo(hypId, studyId, gen)
{
  _name = "Cartesian_3D";
  _shapeType = (1 << TopAbs_SOLID);       // 1 bit /shape type
  _compatibleHypothesis.push_back("CartesianParameters3D");

  _onlyUnaryInput = false;          // to mesh all SOLIDs at once
  _requireDiscreteBoundary = false; // 2D mesh not needed
  _supportSubmeshes = false;        // do not use any existing mesh
}

//=============================================================================
/*!
 * Check presence of a hypothesis
 */
//=============================================================================

bool StdMeshers_Cartesian_3D::CheckHypothesis (SMESH_Mesh&          aMesh,
                                               const TopoDS_Shape&  aShape,
                                               Hypothesis_Status&   aStatus)
{
  aStatus = SMESH_Hypothesis::HYP_MISSING;

  const list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape);
  list <const SMESHDS_Hypothesis* >::const_iterator h = hyps.begin();
  if ( h == hyps.end())
  {
    return false;
  }

  for ( ; h != hyps.end(); ++h )
  {
    if (( _hyp = dynamic_cast<const StdMeshers_CartesianParameters3D*>( *h )))
    {
      aStatus = _hyp->IsDefined() ? HYP_OK : HYP_BAD_PARAMETER;
      break;
    }
  }

  return aStatus == HYP_OK;
}

namespace
{
  typedef int TGeomID;

  //=============================================================================
  // Definitions of internal utils
  // --------------------------------------------------------------------------
  enum Transition {
    Trans_TANGENT = IntCurveSurface_Tangent,
    Trans_IN      = IntCurveSurface_In,
    Trans_OUT     = IntCurveSurface_Out,
    Trans_APEX
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Common data of any intersection between a Grid and a shape
   */
  struct B_IntersectPoint
  {
    mutable const SMDS_MeshNode* _node;
    mutable vector< TGeomID >    _faceIDs;

    B_IntersectPoint(): _node(NULL) {}
    void Add( const vector< TGeomID >& fIDs, const SMDS_MeshNode* n=0 ) const;
    bool HasCommonFace( const B_IntersectPoint * other ) const;
    bool IsOnFace( int faceID ) const;
    virtual ~B_IntersectPoint() {}
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Data of intersection between a GridLine and a TopoDS_Face
   */
  struct F_IntersectPoint : public B_IntersectPoint
  {
    double             _paramOnLine;
    mutable Transition _transition;
    mutable size_t     _indexOnLine;

    bool operator< ( const F_IntersectPoint& o ) const { return _paramOnLine < o._paramOnLine; }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Data of intersection between GridPlanes and a TopoDS_EDGE
   */
  struct E_IntersectPoint : public B_IntersectPoint
  {
    gp_Pnt  _point;
    double  _uvw[3];
    TGeomID _shapeID;
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief A line of the grid and its intersections with 2D geometry
   */
  struct GridLine
  {
    gp_Lin _line;
    double _length; // line length
    multiset< F_IntersectPoint > _intPoints;

    void RemoveExcessIntPoints( const double tol );
    bool GetIsOutBefore( multiset< F_IntersectPoint >::iterator ip, bool prevIsOut );
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Planes of the grid used to find intersections of an EDGE with a hexahedron
   */
  struct GridPlanes
  {
    double _factor;
    gp_XYZ _uNorm, _vNorm, _zNorm;
    vector< gp_XYZ > _origins; // origin points of all planes in one direction
    vector< double > _zProjs;  // projections of origins to _zNorm

    gp_XY GetUV( const gp_Pnt& p, const gp_Pnt& origin );
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Iterator on the parallel grid lines of one direction
   */
  struct LineIndexer
  {
    size_t _size  [3];
    size_t _curInd[3];
    size_t _iVar1, _iVar2, _iConst;
    string _name1, _name2, _nameConst;
    LineIndexer() {}
    LineIndexer( size_t sz1, size_t sz2, size_t sz3,
                 size_t iv1, size_t iv2, size_t iConst,
                 const string& nv1, const string& nv2, const string& nConst )
    {
      _size[0] = sz1; _size[1] = sz2; _size[2] = sz3;
      _curInd[0] = _curInd[1] = _curInd[2] = 0;
      _iVar1 = iv1; _iVar2 = iv2; _iConst = iConst; 
      _name1 = nv1; _name2 = nv2; _nameConst = nConst;
    }

    size_t I() const { return _curInd[0]; }
    size_t J() const { return _curInd[1]; }
    size_t K() const { return _curInd[2]; }
    void SetIJK( size_t i, size_t j, size_t k )
    {
      _curInd[0] = i; _curInd[1] = j; _curInd[2] = k;
    }
    void operator++()
    {
      if ( ++_curInd[_iVar1] == _size[_iVar1] )
        _curInd[_iVar1] = 0, ++_curInd[_iVar2];
    }
    bool More() const { return _curInd[_iVar2] < _size[_iVar2]; }
    size_t LineIndex   () const { return _curInd[_iVar1] + _curInd[_iVar2]* _size[_iVar1]; }
    size_t LineIndex10 () const { return (_curInd[_iVar1] + 1 ) + _curInd[_iVar2]* _size[_iVar1]; }
    size_t LineIndex01 () const { return _curInd[_iVar1] + (_curInd[_iVar2] + 1 )* _size[_iVar1]; }
    size_t LineIndex11 () const { return (_curInd[_iVar1] + 1 ) + (_curInd[_iVar2] + 1 )* _size[_iVar1]; }
    void SetIndexOnLine (size_t i)  { _curInd[ _iConst ] = i; }
    size_t NbLines() const { return _size[_iVar1] * _size[_iVar2]; }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Container of GridLine's
   */
  struct Grid
  {
    vector< double >   _coords[3]; // coordinates of grid nodes
    gp_XYZ             _axes  [3]; // axis directions
    vector< GridLine > _lines [3]; //  in 3 directions
    double             _tol, _minCellSize;

    vector< const SMDS_MeshNode* >    _nodes; // mesh nodes at grid nodes
    vector< const F_IntersectPoint* > _gridIntP; // grid node intersection with geometry

    list< E_IntersectPoint >          _edgeIntP; // intersections with EDGEs
    TopTools_IndexedMapOfShape        _shapes;

    size_t CellIndex( size_t i, size_t j, size_t k ) const
    {
      return i + j*(_coords[0].size()-1) + k*(_coords[0].size()-1)*(_coords[1].size()-1);
    }
    size_t NodeIndex( size_t i, size_t j, size_t k ) const
    {
      return i + j*_coords[0].size() + k*_coords[0].size()*_coords[1].size();
    }
    size_t NodeIndexDX() const { return 1; }
    size_t NodeIndexDY() const { return _coords[0].size(); }
    size_t NodeIndexDZ() const { return _coords[0].size() * _coords[1].size(); }

    LineIndexer GetLineIndexer(size_t iDir) const;

    void SetCoordinates(const vector<double>& xCoords,
                        const vector<double>& yCoords,
                        const vector<double>& zCoords,
                        const double*         axesDirs,
                        const TopoDS_Shape&   shape );
    void ComputeNodes(SMESH_MesherHelper& helper);
  };
#ifdef ELLIPSOLID_WORKAROUND
  // --------------------------------------------------------------------------
  /*!
   * \brief struct temporary replacing IntCurvesFace_Intersector until
   *        OCCT bug 0022809 is fixed
   *        http://tracker.dev.opencascade.org/view.php?id=22809
   */
  struct TMP_IntCurvesFace_Intersector
  {
    BRepAdaptor_Surface                       _surf;
    double                                    _tol;
    BRepIntCurveSurface_Inter                 _intcs;
    vector<IntCurveSurface_IntersectionPoint> _points;
    BRepTopAdaptor_TopolTool                  _clsf;

    TMP_IntCurvesFace_Intersector(const TopoDS_Face& face, const double tol)
      :_surf( face ), _tol( tol ), _clsf( new BRepAdaptor_HSurface(_surf) ) {}
    Bnd_Box Bounding() const { Bnd_Box b; BRepBndLib::Add (_surf.Face(), b); return b; }
    void Perform( const gp_Lin& line, const double w0, const double w1 )
    {
      _points.clear();
      for ( _intcs.Init( _surf.Face(), line, _tol ); _intcs.More(); _intcs.Next() )
        if ( w0 <= _intcs.W() && _intcs.W() <= w1 )
          _points.push_back( _intcs.Point() );
    }
    bool IsDone() const { return true; }
    int  NbPnt()  const { return _points.size(); }
    IntCurveSurface_TransitionOnCurve Transition( const int i ) const { return _points[ i-1 ].Transition(); }
    double       WParameter( const int i ) const { return _points[ i-1 ].W(); }
    TopAbs_State ClassifyUVPoint(const gp_Pnt2d& p) { return _clsf.Classify( p, _tol ); }
  };
#define __IntCurvesFace_Intersector TMP_IntCurvesFace_Intersector
#else
#define __IntCurvesFace_Intersector IntCurvesFace_Intersector
#endif
  // --------------------------------------------------------------------------
  /*!
   * \brief Intersector of TopoDS_Face with all GridLine's
   */
  struct FaceGridIntersector
  {
    TopoDS_Face _face;
    TGeomID     _faceID;
    Grid*       _grid;
    Bnd_Box     _bndBox;
    __IntCurvesFace_Intersector* _surfaceInt;
    vector< std::pair< GridLine*, F_IntersectPoint > > _intersections;

    FaceGridIntersector(): _grid(0), _surfaceInt(0) {}
    void Intersect();
    bool IsInGrid(const Bnd_Box& gridBox);

    void StoreIntersections()
    {
      for ( size_t i = 0; i < _intersections.size(); ++i )
      {
        multiset< F_IntersectPoint >::iterator ip = 
          _intersections[i].first->_intPoints.insert( _intersections[i].second );
        ip->_faceIDs.reserve( 1 );
        ip->_faceIDs.push_back( _faceID );
      }
    }
    const Bnd_Box& GetFaceBndBox()
    {
      GetCurveFaceIntersector();
      return _bndBox;
    }
    __IntCurvesFace_Intersector* GetCurveFaceIntersector()
    {
      if ( !_surfaceInt )
      {
        _surfaceInt = new __IntCurvesFace_Intersector( _face, Precision::PConfusion() );
        _bndBox     = _surfaceInt->Bounding();
        if ( _bndBox.IsVoid() )
          BRepBndLib::Add (_face, _bndBox);
      }
      return _surfaceInt;
    }
    bool IsThreadSafe(set< const Standard_Transient* >& noSafeTShapes) const;
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Intersector of a surface with a GridLine
   */
  struct FaceLineIntersector
  {
    double      _tol;
    double      _u, _v, _w; // params on the face and the line
    Transition  _transition; // transition of at intersection (see IntCurveSurface.cdl)
    Transition  _transIn, _transOut; // IN and OUT transitions depending of face orientation

    gp_Pln      _plane;
    gp_Cylinder _cylinder;
    gp_Cone     _cone;
    gp_Sphere   _sphere;
    gp_Torus    _torus;
    __IntCurvesFace_Intersector* _surfaceInt;

    vector< F_IntersectPoint > _intPoints;

    void IntersectWithPlane   (const GridLine& gridLine);
    void IntersectWithCylinder(const GridLine& gridLine);
    void IntersectWithCone    (const GridLine& gridLine);
    void IntersectWithSphere  (const GridLine& gridLine);
    void IntersectWithTorus   (const GridLine& gridLine);
    void IntersectWithSurface (const GridLine& gridLine);

    bool UVIsOnFace() const;
    void addIntPoint(const bool toClassify=true);
    bool isParamOnLineOK( const double linLength )
    {
      return -_tol < _w && _w < linLength + _tol;
    }
    FaceLineIntersector():_surfaceInt(0) {}
    ~FaceLineIntersector() { if (_surfaceInt ) delete _surfaceInt; _surfaceInt = 0; }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Class representing topology of the hexahedron and creating a mesh
   *        volume basing on analysis of hexahedron intersection with geometry
   */
  class Hexahedron
  {
    // --------------------------------------------------------------------------------
    struct _Face;
    struct _Link;
    // --------------------------------------------------------------------------------
    struct _Node //!< node either at a hexahedron corner or at intersection
    {
      const SMDS_MeshNode*       _node; // mesh node at hexahedron corner
      const B_IntersectPoint* _intPoint;

      _Node(const SMDS_MeshNode* n=0, const B_IntersectPoint* ip=0):_node(n), _intPoint(ip) {} 
      const SMDS_MeshNode*    Node() const
      { return _intPoint ? _intPoint->_node : _node; }
      const F_IntersectPoint* FaceIntPnt() const
      { return static_cast< const F_IntersectPoint* >( _intPoint ); }
      const E_IntersectPoint* EdgeIntPnt() const
      { return static_cast< const E_IntersectPoint* >( _intPoint ); }
      void Add( const E_IntersectPoint* ip )
      {
        if ( !_intPoint ) {
          _intPoint = ip;
        }
        else if ( !_intPoint->_node ) {
          ip->Add( _intPoint->_faceIDs );
          _intPoint = ip;
        }
        else  {
          _intPoint->Add( ip->_faceIDs );
        }
      }
      bool IsLinked( const B_IntersectPoint* other ) const
      {
        // node inside a SOLID is considered "linked" with any other node
        return ( !other || !_intPoint ) ? true : _intPoint->HasCommonFace( other );
      }
      bool IsOnFace( int faceID ) const // returns true if faceID is found
      {
        return _intPoint ? _intPoint->IsOnFace( faceID ) : false;
      }
      gp_Pnt Point() const
      {
        if ( const SMDS_MeshNode* n = Node() )
          return SMESH_TNodeXYZ( n );
        if ( const E_IntersectPoint* eip =
             dynamic_cast< const E_IntersectPoint* >( _intPoint ))
          return eip->_point;
        return gp_Pnt( 1e100, 0, 0 );
      }
    };
    // --------------------------------------------------------------------------------
    struct _Link // link connecting two _Node's
    {
      _Node* _nodes[2];
      vector< _Node > _intNodes; // _Node's at GridLine intersections
      vector< _Link > _splits;
      vector< _Face*> _faces;
    };
    // --------------------------------------------------------------------------------
    struct _OrientedLink
    {
      _Link* _link;
      bool   _reverse;
      _OrientedLink( _Link* link=0, bool reverse=false ): _link(link), _reverse(reverse) {}
      void Reverse() { _reverse = !_reverse; }
      int NbResultLinks() const { return _link->_splits.size(); }
      _OrientedLink ResultLink(int i) const
      {
        return _OrientedLink(&_link->_splits[_reverse ? NbResultLinks()-i-1 : i],_reverse);
      }
      _Node* FirstNode() const { return _link->_nodes[ _reverse ]; }
      _Node* LastNode() const { return _link->_nodes[ !_reverse ]; }
      vector< TGeomID > GetNotUsedFace(const set<TGeomID>& usedIDs ) const // returns a supporting FACEs
      {
        vector< TGeomID > faces;
        const B_IntersectPoint *ip0, *ip1;
        if (( ip0 = _link->_nodes[0]->_intPoint ) &&
            ( ip1 = _link->_nodes[1]->_intPoint ))
        {
          for ( size_t i = 0; i < ip0->_faceIDs.size(); ++i )
            if ( ip1->IsOnFace ( ip0->_faceIDs[i] ) &&
                 !usedIDs.count( ip0->_faceIDs[i] ) )
              faces.push_back( ip0->_faceIDs[i] );
        }
        return faces;
      }
      // TGeomID GetNotUsedFace( set<TGeomID>& usedIDs ) const // returns a supporting FACE
      // {
      //   const B_IntersectPoint *ip0, *ip1;
      //   if (( ip0 = _link->_nodes[0]->_intPoint ) &&
      //       ( ip1 = _link->_nodes[1]->_intPoint ))
      //   {
      //     for ( size_t i = 0; i < ip0->_faceIDs.size(); ++i )
      //       if ( ip1->IsOnFace ( ip0->_faceIDs[i] ) &&
      //            usedIDs.insert( ip0->_faceIDs[i] ).second )
      //         return ip0->_faceIDs[i];
      //   }
      //   return -100;
      // }
    };
    // --------------------------------------------------------------------------------
    struct _Face
    {
      vector< _OrientedLink > _links;       // links on GridLine's
      vector< _Link >         _polyLinks;   // links added to close a polygonal face
      vector< _Node >         _edgeNodes;   // nodes at intersection with EDGEs
    };
    // --------------------------------------------------------------------------------
    struct _volumeDef // holder of nodes of a volume mesh element
    {
      //vector< const SMDS_MeshNode* > _nodes;
      vector< _Node* > _nodes;
      vector< int >    _quantities;
      typedef boost::shared_ptr<_volumeDef> Ptr;
      void set( const vector< _Node* >& nodes,
                const vector< int >&    quant = vector< int >() )
      { _nodes = nodes; _quantities = quant; }
      // static Ptr New( const vector< const SMDS_MeshNode* >& nodes,
      //                 const vector< int > quant = vector< int >() )
      // {
      //   _volumeDef* def = new _volumeDef;
      //   def->_nodes = nodes;
      //   def->_quantities = quant;
      //   return Ptr( def );
      // }
    };

    // topology of a hexahedron
    int   _nodeShift[8];
    _Node _hexNodes [8];
    _Link _hexLinks [12];
    _Face _hexQuads [6];

    // faces resulted from hexahedron intersection
    vector< _Face > _polygons;

    // intresections with EDGEs
    vector< const E_IntersectPoint* > _edgeIntPnts;

    // nodes inside the hexahedron (at VERTEXes)
    vector< _Node > _vertexNodes;

    // computed volume elements
    //vector< _volumeDef::Ptr > _volumeDefs;
    _volumeDef _volumeDefs;

    Grid*       _grid;
    double      _sizeThreshold, _sideLength[3];
    int         _nbCornerNodes, _nbIntNodes, _nbBndNodes;
    int         _origNodeInd; // index of _hexNodes[0] node within the _grid
    size_t      _i,_j,_k;

  public:
    Hexahedron(const double sizeThreshold, Grid* grid);
    int MakeElements(SMESH_MesherHelper&                      helper,
                     const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap);
    void ComputeElements();
    void Init() { init( _i, _j, _k ); }

  private:
    Hexahedron(const Hexahedron& other );
    void init( size_t i, size_t j, size_t k );
    void init( size_t i );
    void addEdges(SMESH_MesherHelper&                      helper,
                  vector< Hexahedron* >&                   intersectedHex,
                  const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap);
    int  getEntity( const E_IntersectPoint* ip, int* facets, int& sub );
    void addIntersection( const E_IntersectPoint& ip );
    bool findChain( _Node* n1, _Node* n2, _Face& quad, vector<_Node*>& chainNodes );
    int  addElements(SMESH_MesherHelper& helper);
    bool isInHole() const;
    bool checkPolyhedronSize() const;
    bool addHexa ();
    bool addTetra();
    bool addPenta();
    bool addPyra ();
    _Node* FindEqualNode( vector< _Node >&        nodes,
                          const E_IntersectPoint* ip,
                          const double            tol2 )
    {
      for ( size_t i = 0; i < nodes.size(); ++i )
        if ( nodes[i].Point().SquareDistance( ip->_point ) <= tol2 )
          return & nodes[i];
      return 0;
    }
  };

#ifdef WITH_TBB
  // --------------------------------------------------------------------------
  /*!
   * \brief Hexahedron computing volumes in one thread
   */
  struct ParallelHexahedron
  {
    vector< Hexahedron* >& _hexVec;
    vector<int>&           _index;
    ParallelHexahedron( vector< Hexahedron* >& hv, vector<int>& ind): _hexVec(hv), _index(ind) {}
    void operator() ( const tbb::blocked_range<size_t>& r ) const
    {
      for ( size_t i = r.begin(); i != r.end(); ++i )
        if ( Hexahedron* hex = _hexVec[ _index[i]] )
          hex->ComputeElements();
    }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Structure intersecting certain nb of faces with GridLine's in one thread
   */
  struct ParallelIntersector
  {
    vector< FaceGridIntersector >& _faceVec;
    ParallelIntersector( vector< FaceGridIntersector >& faceVec): _faceVec(faceVec){}
    void operator() ( const tbb::blocked_range<size_t>& r ) const
    {
      for ( size_t i = r.begin(); i != r.end(); ++i )
        _faceVec[i].Intersect();
    }
  };
#endif

  //=============================================================================
  // Implementation of internal utils
  //=============================================================================
  /*!
   * \brief adjust \a i to have \a val between values[i] and values[i+1]
   */
  inline void locateValue( int & i, double val, const vector<double>& values )
  {
    val += values[0]; // input \a val is measured from 0.
    if ( i > values.size()-2 )
      i = values.size()-2;
    else
      while ( i+2 < values.size() && val > values[ i+1 ])
        ++i;
    while ( i > 0 && val < values[ i ])
      --i;
  }
  //=============================================================================
  /*
   * Remove coincident intersection points
   */
  void GridLine::RemoveExcessIntPoints( const double tol )
  {
    if ( _intPoints.size() < 2 ) return;

    set< Transition > tranSet;
    multiset< F_IntersectPoint >::iterator ip1, ip2 = _intPoints.begin();
    while ( ip2 != _intPoints.end() )
    {
      tranSet.clear();
      ip1 = ip2++;
      while ( ip2->_paramOnLine - ip1->_paramOnLine <= tol  && ip2 != _intPoints.end())
      {
        tranSet.insert( ip1->_transition );
        tranSet.insert( ip2->_transition );
        ip2->Add( ip1->_faceIDs );
        _intPoints.erase( ip1 );
        ip1 = ip2++;
      }
      if ( tranSet.size() > 1 ) // points with different transition coincide
      {
        bool isIN  = tranSet.count( Trans_IN );
        bool isOUT = tranSet.count( Trans_OUT );
        if ( isIN && isOUT )
          (*ip1)._transition = Trans_TANGENT;
        else
          (*ip1)._transition = isIN ? Trans_IN : Trans_OUT;
      }
    }
  }
  //================================================================================
  /*
   * Return "is OUT" state for nodes before the given intersection point
   */
  bool GridLine::GetIsOutBefore( multiset< F_IntersectPoint >::iterator ip, bool prevIsOut )
  {
    if ( ip->_transition == Trans_IN )
      return true;
    if ( ip->_transition == Trans_OUT )
      return false;
    if ( ip->_transition == Trans_APEX )
    {
      // singularity point (apex of a cone)
      if ( _intPoints.size() == 1 || ip == _intPoints.begin() )
        return true;
      multiset< F_IntersectPoint >::iterator ipBef = ip, ipAft = ++ip;
      if ( ipAft == _intPoints.end() )
        return false;
      --ipBef;
      if ( ipBef->_transition != ipAft->_transition )
        return ( ipBef->_transition == Trans_OUT );
      return ( ipBef->_transition != Trans_OUT );
    }
    return prevIsOut; // _transition == Trans_TANGENT
  }
  //================================================================================
  /*
   * Returns parameters of a point in i-th plane
   */
  gp_XY GridPlanes::GetUV( const gp_Pnt& p, const gp_Pnt& origin )
  {
    gp_Vec v( origin, p );
    return gp_XY( v.Dot( _uNorm ) * _factor,
                  v.Dot( _vNorm ) * _factor );
  }
  //================================================================================
  /*
   * Adds face IDs
   */
  void B_IntersectPoint::Add( const vector< TGeomID >& fIDs,
                              const SMDS_MeshNode*     n) const
  {
    if ( _faceIDs.empty() )
      _faceIDs = fIDs;
    else
      for ( size_t i = 0; i < fIDs.size(); ++i )
      {
        vector< TGeomID >::iterator it =
          std::find( _faceIDs.begin(), _faceIDs.end(), fIDs[i] );
        if ( it == _faceIDs.end() )
          _faceIDs.push_back( fIDs[i] );
      }
    if ( !_node )
      _node = n;
  }
  //================================================================================
  /*
   * Returns \c true if \a other B_IntersectPoint holds the same face ID
   */
  bool B_IntersectPoint::HasCommonFace( const B_IntersectPoint * other ) const
  {
    if ( other )
      for ( size_t i = 0; i < other->_faceIDs.size(); ++i )
        if ( IsOnFace( other->_faceIDs[i] ) )
          return true;
    return false;
  }
  //================================================================================
  /*
   * Returns \c true if \a faceID in in this->_faceIDs
   */
  bool B_IntersectPoint::IsOnFace( int faceID ) const // returns true if faceID is found
  {
    vector< TGeomID >::const_iterator it =
      std::find( _faceIDs.begin(), _faceIDs.end(), faceID );
    return ( it != _faceIDs.end() );
  }
  //================================================================================
  /*
   * Return an iterator on GridLine's in a given direction
   */
  LineIndexer Grid::GetLineIndexer(size_t iDir) const
  {
    const size_t indices[] = { 1,2,0, 0,2,1, 0,1,2 };
    const string s      [] = { "X", "Y", "Z" };
    LineIndexer li( _coords[0].size(),  _coords[1].size(),    _coords[2].size(),
                    indices[iDir*3],    indices[iDir*3+1],    indices[iDir*3+2],
                    s[indices[iDir*3]], s[indices[iDir*3+1]], s[indices[iDir*3+2]]);
    return li;
  }
  //=============================================================================
  /*
   * Creates GridLine's of the grid
   */
  void Grid::SetCoordinates(const vector<double>& xCoords,
                            const vector<double>& yCoords,
                            const vector<double>& zCoords,
                            const double*         axesDirs,
                            const TopoDS_Shape&   shape)
  {
    _coords[0] = xCoords;
    _coords[1] = yCoords;
    _coords[2] = zCoords;
    _axes[0].SetCoord( axesDirs[0],
                       axesDirs[1],
                       axesDirs[2]);
    _axes[1].SetCoord( axesDirs[3],
                       axesDirs[4],
                       axesDirs[5]);
    _axes[2].SetCoord( axesDirs[6],
                       axesDirs[7],
                       axesDirs[8]);

    // compute tolerance
    _minCellSize = Precision::Infinite();
    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      for ( size_t i = 1; i < _coords[ iDir ].size(); ++i )
      {
        double cellLen = _coords[ iDir ][ i ] - _coords[ iDir ][ i-1 ];
        if ( cellLen < _minCellSize )
          _minCellSize = cellLen;
      }
    }
    if ( _minCellSize < Precision::Confusion() )
      throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                SMESH_Comment("Too small cell size: ") << _tol );
    _tol = _minCellSize / 1000.;

    // attune grid extremities to shape bounding box computed by vertices
    Bnd_Box shapeBox;
    for ( TopExp_Explorer vExp( shape, TopAbs_VERTEX ); vExp.More(); vExp.Next() )
      shapeBox.Add( BRep_Tool::Pnt( TopoDS::Vertex( vExp.Current() )));
    
    double sP[6]; // aXmin, aYmin, aZmin, aXmax, aYmax, aZmax
    shapeBox.Get(sP[0],sP[1],sP[2],sP[3],sP[4],sP[5]);
    double* cP[6] = { &_coords[0].front(), &_coords[1].front(), &_coords[2].front(),
                      &_coords[0].back(),  &_coords[1].back(),  &_coords[2].back() };
    for ( int i = 0; i < 6; ++i )
      if ( fabs( sP[i] - *cP[i] ) < _tol )
        *cP[i] = sP[i] + _tol/1000. * ( i < 3 ? +1 : -1 );

    // create lines
    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      LineIndexer li = GetLineIndexer( iDir );
      _lines[iDir].resize( li.NbLines() );
      double len = _coords[ iDir ].back() - _coords[iDir].front();
      gp_Vec dir( iDir==0, iDir==1, iDir==2 );
      for ( ; li.More(); ++li )
      {
        GridLine& gl = _lines[iDir][ li.LineIndex() ];
        gl._line.SetLocation(gp_Pnt(_coords[0][li.I()], _coords[1][li.J()], _coords[2][li.K()])); 
        gl._line.SetDirection( dir );
        gl._length = len;
      }
    }
  }
  //================================================================================
  /*
   * Creates all nodes
   */
  void Grid::ComputeNodes(SMESH_MesherHelper& helper)
  {
    // state of each node of the grid relative to the geometry
    const size_t nbGridNodes = _coords[0].size() * _coords[1].size() * _coords[2].size();
    vector< bool > isNodeOut( nbGridNodes, false );
    _nodes.resize( nbGridNodes, 0 );
    _gridIntP.resize( nbGridNodes, NULL );

    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      LineIndexer li = GetLineIndexer( iDir );

      // find out a shift of node index while walking along a GridLine in this direction
      li.SetIndexOnLine( 0 );
      size_t nIndex0 = NodeIndex( li.I(), li.J(), li.K() );
      li.SetIndexOnLine( 1 );
      const size_t nShift = NodeIndex( li.I(), li.J(), li.K() ) - nIndex0;
      
      const vector<double> & coords = _coords[ iDir ];
      for ( ; li.More(); ++li ) // loop on lines in iDir
      {
        li.SetIndexOnLine( 0 );
        nIndex0 = NodeIndex( li.I(), li.J(), li.K() );

        GridLine& line = _lines[ iDir ][ li.LineIndex() ];
        line.RemoveExcessIntPoints( _tol );
        multiset< F_IntersectPoint >& intPnts = _lines[ iDir ][ li.LineIndex() ]._intPoints;
        multiset< F_IntersectPoint >::iterator ip = intPnts.begin();

        bool isOut = true;
        const double* nodeCoord = & coords[0], *coord0 = nodeCoord, *coordEnd = coord0 + coords.size();
        double nodeParam = 0;
        for ( ; ip != intPnts.end(); ++ip )
        {
          // set OUT state or just skip IN nodes before ip
          if ( nodeParam < ip->_paramOnLine - _tol )
          {
            isOut = line.GetIsOutBefore( ip, isOut );

            while ( nodeParam < ip->_paramOnLine - _tol )
            {
              if ( isOut )
                isNodeOut[ nIndex0 + nShift * ( nodeCoord-coord0 ) ] = isOut;
              if ( ++nodeCoord <  coordEnd )
                nodeParam = *nodeCoord - *coord0;
              else
                break;
            }
            if ( nodeCoord == coordEnd ) break;
          }
          // create a mesh node on a GridLine at ip if it does not coincide with a grid node
          if ( nodeParam > ip->_paramOnLine + _tol )
          {
            li.SetIndexOnLine( 0 );
            double xyz[3] = { _coords[0][ li.I() ], _coords[1][ li.J() ], _coords[2][ li.K() ]};
            xyz[ li._iConst ] += ip->_paramOnLine;
            ip->_node = helper.AddNode( xyz[0], xyz[1], xyz[2] );
            ip->_indexOnLine = nodeCoord-coord0-1;
          }
          // create a mesh node at ip concident with a grid node
          else
          {
            int nodeIndex = nIndex0 + nShift * ( nodeCoord-coord0 );
            if ( !_nodes[ nodeIndex ] )
            {
              li.SetIndexOnLine( nodeCoord-coord0 );
              double xyz[3] = { _coords[0][ li.I() ], _coords[1][ li.J() ], _coords[2][ li.K() ]};
              _nodes   [ nodeIndex ] = helper.AddNode( xyz[0], xyz[1], xyz[2] );
              _gridIntP[ nodeIndex ] = & * ip;
            }
            if ( _gridIntP[ nodeIndex ] )
              _gridIntP[ nodeIndex ]->Add( ip->_faceIDs );
            else
              _gridIntP[ nodeIndex ] = & * ip;
            // ip->_node        = _nodes[ nodeIndex ]; -- to differ from ip on links
            ip->_indexOnLine = nodeCoord-coord0;
            if ( ++nodeCoord < coordEnd )
              nodeParam = *nodeCoord - *coord0;
          }
        }
        // set OUT state to nodes after the last ip
        for ( ; nodeCoord < coordEnd; ++nodeCoord )
          isNodeOut[ nIndex0 + nShift * ( nodeCoord-coord0 ) ] = true;
      }
    }

    // Create mesh nodes at !OUT nodes of the grid

    for ( size_t z = 0; z < _coords[2].size(); ++z )
      for ( size_t y = 0; y < _coords[1].size(); ++y )
        for ( size_t x = 0; x < _coords[0].size(); ++x )
        {
          size_t nodeIndex = NodeIndex( x, y, z );
          if ( !isNodeOut[ nodeIndex ] && !_nodes[ nodeIndex] )
            _nodes[ nodeIndex ] = helper.AddNode( _coords[0][x], _coords[1][y], _coords[2][z] );
        }

#ifdef _MY_DEBUG_
    // check validity of transitions
    const char* trName[] = { "TANGENT", "IN", "OUT", "APEX" };
    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      LineIndexer li = GetLineIndexer( iDir );
      for ( ; li.More(); ++li )
      {
        multiset< F_IntersectPoint >& intPnts = _lines[ iDir ][ li.LineIndex() ]._intPoints;
        if ( intPnts.empty() ) continue;
        if ( intPnts.size() == 1 )
        {
          if ( intPnts.begin()->_transition != Trans_TANGENT &&
               intPnts.begin()->_transition != Trans_APEX )
          throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                    SMESH_Comment("Wrong SOLE transition of GridLine (")
                                    << li._curInd[li._iVar1] << ", " << li._curInd[li._iVar2]
                                    << ") along " << li._nameConst
                                    << ": " << trName[ intPnts.begin()->_transition] );
        }
        else
        {
          if ( intPnts.begin()->_transition == Trans_OUT )
            throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                      SMESH_Comment("Wrong START transition of GridLine (")
                                      << li._curInd[li._iVar1] << ", " << li._curInd[li._iVar2]
                                      << ") along " << li._nameConst
                                      << ": " << trName[ intPnts.begin()->_transition ]);
          if ( intPnts.rbegin()->_transition == Trans_IN )
            throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                      SMESH_Comment("Wrong END transition of GridLine (")
                                      << li._curInd[li._iVar1] << ", " << li._curInd[li._iVar2]
                                      << ") along " << li._nameConst
                                    << ": " << trName[ intPnts.rbegin()->_transition ]);
        }
      }
    }
#endif
  }

  //=============================================================================
  /*
   * Checks if the face is encosed by the grid
   */
  bool FaceGridIntersector::IsInGrid(const Bnd_Box& gridBox)
  {
    double x0,y0,z0, x1,y1,z1;
    const Bnd_Box& faceBox = GetFaceBndBox();
    faceBox.Get(x0,y0,z0, x1,y1,z1);

    if ( !gridBox.IsOut( gp_Pnt( x0,y0,z0 )) &&
         !gridBox.IsOut( gp_Pnt( x1,y1,z1 )))
      return true;

    double X0,Y0,Z0, X1,Y1,Z1;
    gridBox.Get(X0,Y0,Z0, X1,Y1,Z1);
    double faceP[6] = { x0,y0,z0, x1,y1,z1 };
    double gridP[6] = { X0,Y0,Z0, X1,Y1,Z1 };
    gp_Dir axes[3]  = { gp::DX(), gp::DY(), gp::DZ() };
    for ( int iDir = 0; iDir < 6; ++iDir )
    {
      if ( iDir < 3  && gridP[ iDir ] <= faceP[ iDir ] ) continue;
      if ( iDir >= 3 && gridP[ iDir ] >= faceP[ iDir ] ) continue;

      // check if the face intersects a side of a gridBox

      gp_Pnt p = iDir < 3 ? gp_Pnt( X0,Y0,Z0 ) : gp_Pnt( X1,Y1,Z1 );
      gp_Ax1 norm( p, axes[ iDir % 3 ] );
      if ( iDir < 3 ) norm.Reverse();

      gp_XYZ O = norm.Location().XYZ(), N = norm.Direction().XYZ();

      TopLoc_Location loc = _face.Location();
      Handle(Poly_Triangulation) aPoly = BRep_Tool::Triangulation(_face,loc);
      if ( !aPoly.IsNull() )
      {
        if ( !loc.IsIdentity() )
        {
          norm.Transform( loc.Transformation().Inverted() );
          O = norm.Location().XYZ(), N = norm.Direction().XYZ();
        }
        const double deflection = aPoly->Deflection();

        const TColgp_Array1OfPnt& nodes = aPoly->Nodes();
        for ( int i = nodes.Lower(); i <= nodes.Upper(); ++i )
          if (( nodes( i ).XYZ() - O ) * N > _grid->_tol + deflection )
            return false;
      }
      else
      {
        BRepAdaptor_Surface surf( _face );
        double u0, u1, v0, v1, du, dv, u, v;
        BRepTools::UVBounds( _face, u0, u1, v0, v1);
        if ( surf.GetType() == GeomAbs_Plane ) {
          du = u1 - u0, dv = v1 - v0;
        }
        else {
          du = surf.UResolution( _grid->_minCellSize / 10. );
          dv = surf.VResolution( _grid->_minCellSize / 10. );
        }
        for ( u = u0, v = v0; u <= u1 && v <= v1; u += du, v += dv )
        {
          gp_Pnt p = surf.Value( u, v );
          if (( p.XYZ() - O ) * N > _grid->_tol )
          {
            TopAbs_State state = GetCurveFaceIntersector()->ClassifyUVPoint(gp_Pnt2d( u, v ));
            if ( state == TopAbs_IN || state == TopAbs_ON )
              return false;
          }
        }
      }
    }
    return true;
  }
  //=============================================================================
  /*
   * Intersects TopoDS_Face with all GridLine's
   */
  void FaceGridIntersector::Intersect()
  {
    FaceLineIntersector intersector;
    intersector._surfaceInt = GetCurveFaceIntersector();
    intersector._tol        = _grid->_tol;
    intersector._transOut   = _face.Orientation() == TopAbs_REVERSED ? Trans_IN : Trans_OUT;
    intersector._transIn    = _face.Orientation() == TopAbs_REVERSED ? Trans_OUT : Trans_IN;

    typedef void (FaceLineIntersector::* PIntFun )(const GridLine& gridLine);
    PIntFun interFunction;

    BRepAdaptor_Surface surf( _face );
    switch ( surf.GetType() ) {
    case GeomAbs_Plane:
      intersector._plane = surf.Plane();
      interFunction = &FaceLineIntersector::IntersectWithPlane;
      break;
    case GeomAbs_Cylinder:
      intersector._cylinder = surf.Cylinder();
      interFunction = &FaceLineIntersector::IntersectWithCylinder;
      break;
    case GeomAbs_Cone:
      intersector._cone = surf.Cone();
      interFunction = &FaceLineIntersector::IntersectWithCone;
      break;
    case GeomAbs_Sphere:
      intersector._sphere = surf.Sphere();
      interFunction = &FaceLineIntersector::IntersectWithSphere;
      break;
    case GeomAbs_Torus:
      intersector._torus = surf.Torus();
      interFunction = &FaceLineIntersector::IntersectWithTorus;
      break;
    default:
      interFunction = &FaceLineIntersector::IntersectWithSurface;
    }

    _intersections.clear();
    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      if ( surf.GetType() == GeomAbs_Plane )
      {
        // check if all lines in this direction are parallel to a plane
        if ( intersector._plane.Axis().IsNormal( _grid->_lines[iDir][0]._line.Position(),
                                                 Precision::Angular()))
          continue;
        // find out a transition, that is the same for all lines of a direction
        gp_Dir plnNorm = intersector._plane.Axis().Direction();
        gp_Dir lineDir = _grid->_lines[iDir][0]._line.Direction();
        intersector._transition =
          ( plnNorm * lineDir < 0 ) ? intersector._transIn : intersector._transOut;
      }
      if ( surf.GetType() == GeomAbs_Cylinder )
      {
        // check if all lines in this direction are parallel to a cylinder
        if ( intersector._cylinder.Axis().IsParallel( _grid->_lines[iDir][0]._line.Position(),
                                                      Precision::Angular()))
          continue;
      }

      // intersect the grid lines with the face
      for ( size_t iL = 0; iL < _grid->_lines[iDir].size(); ++iL )
      {
        GridLine& gridLine = _grid->_lines[iDir][iL];
        if ( _bndBox.IsOut( gridLine._line )) continue;

        intersector._intPoints.clear();
        (intersector.*interFunction)( gridLine );
        for ( size_t i = 0; i < intersector._intPoints.size(); ++i )
          _intersections.push_back( make_pair( &gridLine, intersector._intPoints[i] ));
      }
    }
  }
  //================================================================================
  /*
   * Return true if (_u,_v) is on the face
   */
  bool FaceLineIntersector::UVIsOnFace() const
  {
    TopAbs_State state = _surfaceInt->ClassifyUVPoint(gp_Pnt2d( _u,_v ));
    return ( state == TopAbs_IN || state == TopAbs_ON );
  }
  //================================================================================
  /*
   * Store an intersection if it is IN or ON the face
   */
  void FaceLineIntersector::addIntPoint(const bool toClassify)
  {
    if ( !toClassify || UVIsOnFace() )
    {
      F_IntersectPoint p;
      p._paramOnLine = _w;
      p._transition  = _transition;
      _intPoints.push_back( p );
    }
  }
  //================================================================================
  /*
   * Intersect a line with a plane
   */
  void FaceLineIntersector::IntersectWithPlane   (const GridLine& gridLine)
  {
    IntAna_IntConicQuad linPlane( gridLine._line, _plane, Precision::Angular());
    _w = linPlane.ParamOnConic(1);
    if ( isParamOnLineOK( gridLine._length ))
    {
      ElSLib::Parameters(_plane, linPlane.Point(1) ,_u,_v);
      addIntPoint();
    }
  }
  //================================================================================
  /*
   * Intersect a line with a cylinder
   */
  void FaceLineIntersector::IntersectWithCylinder(const GridLine& gridLine)
  {
    IntAna_IntConicQuad linCylinder( gridLine._line,_cylinder);
    if ( linCylinder.IsDone() && linCylinder.NbPoints() > 0 )
    {
      _w = linCylinder.ParamOnConic(1);
      if ( linCylinder.NbPoints() == 1 )
        _transition = Trans_TANGENT;
      else
        _transition = _w < linCylinder.ParamOnConic(2) ? _transIn : _transOut;
      if ( isParamOnLineOK( gridLine._length ))
      {
        ElSLib::Parameters(_cylinder, linCylinder.Point(1) ,_u,_v);
        addIntPoint();
      }
      if ( linCylinder.NbPoints() > 1 )
      {
        _w = linCylinder.ParamOnConic(2);
        if ( isParamOnLineOK( gridLine._length ))
        {
          ElSLib::Parameters(_cylinder, linCylinder.Point(2) ,_u,_v);
          _transition = ( _transition == Trans_OUT ) ? Trans_IN : Trans_OUT;
          addIntPoint();
        }
      }
    }
  }
  //================================================================================
  /*
   * Intersect a line with a cone
   */
  void FaceLineIntersector::IntersectWithCone (const GridLine& gridLine)
  {
    IntAna_IntConicQuad linCone(gridLine._line,_cone);
    if ( !linCone.IsDone() ) return;
    gp_Pnt P;
    gp_Vec du, dv, norm;
    for ( int i = 1; i <= linCone.NbPoints(); ++i )
    {
      _w = linCone.ParamOnConic( i );
      if ( !isParamOnLineOK( gridLine._length )) continue;
      ElSLib::Parameters(_cone, linCone.Point(i) ,_u,_v);
      if ( UVIsOnFace() )
      {
        ElSLib::D1( _u, _v, _cone, P, du, dv );
        norm = du ^ dv;
        double normSize2 = norm.SquareMagnitude();
        if ( normSize2 > Precision::Angular() * Precision::Angular() )
        {
          double cos = norm.XYZ() * gridLine._line.Direction().XYZ();
          cos /= sqrt( normSize2 );
          if ( cos < -Precision::Angular() )
            _transition = _transIn;
          else if ( cos > Precision::Angular() )
            _transition = _transOut;
          else
            _transition = Trans_TANGENT;
        }
        else
        {
          _transition = Trans_APEX;
        }
        addIntPoint( /*toClassify=*/false);
      }
    }
  }
  //================================================================================
  /*
   * Intersect a line with a sphere
   */
  void FaceLineIntersector::IntersectWithSphere  (const GridLine& gridLine)
  {
    IntAna_IntConicQuad linSphere(gridLine._line,_sphere);
    if ( linSphere.IsDone() && linSphere.NbPoints() > 0 )
    {
      _w = linSphere.ParamOnConic(1);
      if ( linSphere.NbPoints() == 1 )
        _transition = Trans_TANGENT;
      else
        _transition = _w < linSphere.ParamOnConic(2) ? _transIn : _transOut;
      if ( isParamOnLineOK( gridLine._length ))
      {
        ElSLib::Parameters(_sphere, linSphere.Point(1) ,_u,_v);
        addIntPoint();
      }
      if ( linSphere.NbPoints() > 1 )
      {
        _w = linSphere.ParamOnConic(2);
        if ( isParamOnLineOK( gridLine._length ))
        {
          ElSLib::Parameters(_sphere, linSphere.Point(2) ,_u,_v);
          _transition = ( _transition == Trans_OUT ) ? Trans_IN : Trans_OUT;
          addIntPoint();
        }
      }
    }
  }
  //================================================================================
  /*
   * Intersect a line with a torus
   */
  void FaceLineIntersector::IntersectWithTorus   (const GridLine& gridLine)
  {
    IntAna_IntLinTorus linTorus(gridLine._line,_torus);
    if ( !linTorus.IsDone()) return;
    gp_Pnt P;
    gp_Vec du, dv, norm;
    for ( int i = 1; i <= linTorus.NbPoints(); ++i )
    {
      _w = linTorus.ParamOnLine( i );
      if ( !isParamOnLineOK( gridLine._length )) continue;
      linTorus.ParamOnTorus( i, _u,_v );
      if ( UVIsOnFace() )
      {
        ElSLib::D1( _u, _v, _torus, P, du, dv );
        norm = du ^ dv;
        double normSize = norm.Magnitude();
        double cos = norm.XYZ() * gridLine._line.Direction().XYZ();
        cos /= normSize;
        if ( cos < -Precision::Angular() )
          _transition = _transIn;
        else if ( cos > Precision::Angular() )
          _transition = _transOut;
        else
          _transition = Trans_TANGENT;
        addIntPoint( /*toClassify=*/false);
      }
    }
  }
  //================================================================================
  /*
   * Intersect a line with a non-analytical surface
   */
  void FaceLineIntersector::IntersectWithSurface (const GridLine& gridLine)
  {
    _surfaceInt->Perform( gridLine._line, 0.0, gridLine._length );
    if ( !_surfaceInt->IsDone() ) return;
    for ( int i = 1; i <= _surfaceInt->NbPnt(); ++i )
    {
      _transition = Transition( _surfaceInt->Transition( i ) );
      _w = _surfaceInt->WParameter( i );
      addIntPoint(/*toClassify=*/false);
    }
  }
  //================================================================================
  /*
   * check if its face can be safely intersected in a thread
   */
  bool FaceGridIntersector::IsThreadSafe(set< const Standard_Transient* >& noSafeTShapes) const
  {
    bool isSafe = true;

    // check surface
    TopLoc_Location loc;
    Handle(Geom_Surface) surf = BRep_Tool::Surface( _face, loc );
    Handle(Geom_RectangularTrimmedSurface) ts =
      Handle(Geom_RectangularTrimmedSurface)::DownCast( surf );
    while( !ts.IsNull() ) {
      surf = ts->BasisSurface();
      ts = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);
    }
    if ( surf->IsKind( STANDARD_TYPE(Geom_BSplineSurface )) ||
         surf->IsKind( STANDARD_TYPE(Geom_BezierSurface )))
      if ( !noSafeTShapes.insert((const Standard_Transient*) _face.TShape() ).second )
        isSafe = false;

    double f, l;
    TopExp_Explorer exp( _face, TopAbs_EDGE );
    for ( ; exp.More(); exp.Next() )
    {
      bool edgeIsSafe = true;
      const TopoDS_Edge& e = TopoDS::Edge( exp.Current() );
      // check 3d curve
      {
        Handle(Geom_Curve) c = BRep_Tool::Curve( e, loc, f, l);
        if ( !c.IsNull() )
        {
          Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(c);
          while( !tc.IsNull() ) {
            c = tc->BasisCurve();
            tc = Handle(Geom_TrimmedCurve)::DownCast(c);
          }
          if ( c->IsKind( STANDARD_TYPE(Geom_BSplineCurve )) ||
               c->IsKind( STANDARD_TYPE(Geom_BezierCurve )))
            edgeIsSafe = false;
        }
      }
      // check 2d curve
      if ( edgeIsSafe )
      {
        Handle(Geom2d_Curve) c2 = BRep_Tool::CurveOnSurface( e, surf, loc, f, l);
        if ( !c2.IsNull() )
        {
          Handle(Geom2d_TrimmedCurve) tc = Handle(Geom2d_TrimmedCurve)::DownCast(c2);
          while( !tc.IsNull() ) {
            c2 = tc->BasisCurve();
            tc = Handle(Geom2d_TrimmedCurve)::DownCast(c2);
          }
          if ( c2->IsKind( STANDARD_TYPE(Geom2d_BSplineCurve )) ||
               c2->IsKind( STANDARD_TYPE(Geom2d_BezierCurve )))
            edgeIsSafe = false;
        }
      }
      if ( !edgeIsSafe && !noSafeTShapes.insert((const Standard_Transient*) e.TShape() ).second )
        isSafe = false;
    }
    return isSafe;
  }
  //================================================================================
  /*!
   * \brief Creates topology of the hexahedron
   */
  Hexahedron::Hexahedron(const double sizeThreshold, Grid* grid)
    : _grid( grid ), _sizeThreshold( sizeThreshold ), _nbIntNodes(0)
  {
    _polygons.reserve(100); // to avoid reallocation;

    //set nodes shift within grid->_nodes from the node 000 
    size_t dx = _grid->NodeIndexDX();
    size_t dy = _grid->NodeIndexDY();
    size_t dz = _grid->NodeIndexDZ();
    size_t i000 = 0;
    size_t i100 = i000 + dx;
    size_t i010 = i000 + dy;
    size_t i110 = i010 + dx;
    size_t i001 = i000 + dz;
    size_t i101 = i100 + dz;
    size_t i011 = i010 + dz;
    size_t i111 = i110 + dz;
    _nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V000 )] = i000;
    _nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V100 )] = i100;
    _nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V010 )] = i010;
    _nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V110 )] = i110;
    _nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V001 )] = i001;
    _nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V101 )] = i101;
    _nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V011 )] = i011;
    _nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V111 )] = i111;

    vector< int > idVec;
    // set nodes to links
    for ( int linkID = SMESH_Block::ID_Ex00; linkID <= SMESH_Block::ID_E11z; ++linkID )
    {
      SMESH_Block::GetEdgeVertexIDs( linkID, idVec );
      _Link& link = _hexLinks[ SMESH_Block::ShapeIndex( linkID )];
      link._nodes[0] = &_hexNodes[ SMESH_Block::ShapeIndex( idVec[0] )];
      link._nodes[1] = &_hexNodes[ SMESH_Block::ShapeIndex( idVec[1] )];
      link._intNodes.reserve( 10 ); // to avoid reallocation
      link._splits.reserve( 10 );
    }

    // set links to faces
    int interlace[4] = { 0, 3, 1, 2 }; // to walk by links around a face: { u0, 1v, u1, 0v }
    for ( int faceID = SMESH_Block::ID_Fxy0; faceID <= SMESH_Block::ID_F1yz; ++faceID )
    {
      SMESH_Block::GetFaceEdgesIDs( faceID, idVec );
      _Face& quad = _hexQuads[ SMESH_Block::ShapeIndex( faceID )];
      bool revFace = ( faceID == SMESH_Block::ID_Fxy0 ||
                       faceID == SMESH_Block::ID_Fx1z ||
                       faceID == SMESH_Block::ID_F0yz );
      quad._links.resize(4);
      vector<_OrientedLink>::iterator         frwLinkIt = quad._links.begin();
      vector<_OrientedLink>::reverse_iterator revLinkIt = quad._links.rbegin();
      for ( int i = 0; i < 4; ++i )
      {
        bool revLink = revFace;
        if ( i > 1 ) // reverse links u1 and v0
          revLink = !revLink;
        _OrientedLink& link = revFace ? *revLinkIt++ : *frwLinkIt++;
        link = _OrientedLink( & _hexLinks[ SMESH_Block::ShapeIndex( idVec[interlace[i]] )],
                              revLink );
      }
    }
  }
  //================================================================================
  /*!
   * \brief Copy constructor
   */
  Hexahedron::Hexahedron( const Hexahedron& other )
    :_grid( other._grid ), _sizeThreshold( other._sizeThreshold ), _nbIntNodes(0)
  {
    _polygons.reserve(100); // to avoid reallocation;

    for ( int i = 0; i < 8; ++i )
      _nodeShift[i] = other._nodeShift[i];

    for ( int i = 0; i < 12; ++i )
    {
      const _Link& srcLink = other._hexLinks[ i ];
      _Link&       tgtLink = this->_hexLinks[ i ];
      tgtLink._nodes[0] = _hexNodes + ( srcLink._nodes[0] - other._hexNodes );
      tgtLink._nodes[1] = _hexNodes + ( srcLink._nodes[1] - other._hexNodes );
      tgtLink._intNodes.reserve( 10 ); // to avoid reallocation
      tgtLink._splits.reserve( 10 );
    }

    for ( int i = 0; i < 6; ++i )
    {
      const _Face& srcQuad = other._hexQuads[ i ];
      _Face&       tgtQuad = this->_hexQuads[ i ];
      tgtQuad._links.resize(4);
      for ( int j = 0; j < 4; ++j )
      {
        const _OrientedLink& srcLink = srcQuad._links[ j ];
        _OrientedLink&       tgtLink = tgtQuad._links[ j ];
        tgtLink._reverse = srcLink._reverse;
        tgtLink._link    = _hexLinks + ( srcLink._link - other._hexLinks );
      }
    }
  }
  
  //================================================================================
  /*!
   * \brief Initializes its data by given grid cell
   */
  void Hexahedron::init( size_t i, size_t j, size_t k )
  {
    _i = i; _j = j; _k = k;
    // set nodes of grid to nodes of the hexahedron and
    // count nodes at hexahedron corners located IN and ON geometry
    _nbCornerNodes = _nbBndNodes = 0;
    _origNodeInd   = _grid->NodeIndex( i,j,k );
    for ( int iN = 0; iN < 8; ++iN )
    {
      _hexNodes[iN]._node     = _grid->_nodes   [ _origNodeInd + _nodeShift[iN] ];
      _hexNodes[iN]._intPoint = _grid->_gridIntP[ _origNodeInd + _nodeShift[iN] ];
      _nbCornerNodes += bool( _hexNodes[iN]._node );
      if ( _hexNodes[iN]._intPoint )
      {
        ++_nbBndNodes;
        _hexNodes[iN]._intPoint->_node = _hexNodes[iN]._node;
      }
    }

    _sideLength[0] = _grid->_coords[0][i+1] - _grid->_coords[0][i];
    _sideLength[1] = _grid->_coords[1][j+1] - _grid->_coords[1][j];
    _sideLength[2] = _grid->_coords[2][k+1] - _grid->_coords[2][k];

    if ( _nbCornerNodes < 8 && _nbIntNodes + _nbCornerNodes + _edgeIntPnts.size() > 3)
    {
      _Link split;
      // create sub-links (_splits) by splitting links with _intNodes
      for ( int iLink = 0; iLink < 12; ++iLink )
      {
        _Link& link = _hexLinks[ iLink ];
        link._splits.clear();
        split._nodes[ 0 ] = link._nodes[0];
        for ( size_t i = 0; i < link._intNodes.size(); ++i )
        {
          if ( split._nodes[ 0 ]->Node() )
          {
            split._nodes[ 1 ] = &link._intNodes[i];
            link._splits.push_back( split );
          }
          split._nodes[ 0 ] = &link._intNodes[i];
        }
        if ( link._nodes[ 1 ]->Node() && split._nodes[ 0 ]->Node() )
        {
          split._nodes[ 1 ] = link._nodes[1];
          link._splits.push_back( split );
        }
      }

      // create _Node's at intersections with EDGEs

      const double tol2 = _grid->_tol * _grid->_tol;
      int facets[3], nbFacets, subEntity;

      for ( size_t iP = 0; iP < _edgeIntPnts.size(); ++iP )
      {
        nbFacets = getEntity( _edgeIntPnts[iP], facets, subEntity );
        _Node* equalNode = 0;
        switch( nbFacets ) {
        case 1: // in a _Face
        {
          _Face& quad = _hexQuads[ facets[0] - SMESH_Block::ID_FirstF ];
          equalNode = FindEqualNode( quad._edgeNodes, _edgeIntPnts[ iP ], tol2 );
          if ( equalNode ) {
            equalNode->Add( _edgeIntPnts[ iP ] );
          }
          else {
            quad._edgeNodes.push_back( _Node( 0, _edgeIntPnts[ iP ]));
            ++_nbIntNodes;
          }
          break;
        }
        case 2: // on a _Link
        {
          _Link& link = _hexLinks[ subEntity - SMESH_Block::ID_FirstE ];
          if ( link._splits.size() > 0 )
          {
            equalNode = FindEqualNode( link._intNodes, _edgeIntPnts[ iP ], tol2 );
            if ( equalNode )
              equalNode->Add( _edgeIntPnts[ iP ] );
          }
          else
          {
            for ( int iF = 0; iF < 2; ++iF )
            {
              _Face& quad = _hexQuads[ facets[iF] - SMESH_Block::ID_FirstF ];
              equalNode = FindEqualNode( quad._edgeNodes, _edgeIntPnts[ iP ], tol2 );
              if ( equalNode ) {
                equalNode->Add( _edgeIntPnts[ iP ] );
              }
              else {
                quad._edgeNodes.push_back( _Node( 0, _edgeIntPnts[ iP ]));
                ++_nbIntNodes;
              }
            }
          }
          break;
        }
        case 3: // at a corner
        {
          _Node& node = _hexNodes[ subEntity - SMESH_Block::ID_FirstV ];
          if ( node.Node() > 0 )
          {
            if ( node._intPoint )
              node._intPoint->Add( _edgeIntPnts[ iP ]->_faceIDs, _edgeIntPnts[ iP ]->_node );
          }
          else
          {
            for ( int iF = 0; iF < 3; ++iF )
            {
              _Face& quad = _hexQuads[ facets[iF] - SMESH_Block::ID_FirstF ];
              equalNode = FindEqualNode( quad._edgeNodes, _edgeIntPnts[ iP ], tol2 );
              if ( equalNode ) {
                equalNode->Add( _edgeIntPnts[ iP ] );
              }
              else {
                quad._edgeNodes.push_back( _Node( 0, _edgeIntPnts[ iP ]));
                ++_nbIntNodes;
              }
            }
          }
          break;
        }
        default: // inside a hex
        {
          equalNode = FindEqualNode( _vertexNodes, _edgeIntPnts[ iP ], tol2 );
          if ( equalNode ) {
            equalNode->Add( _edgeIntPnts[ iP ] );
          }
          else {
            _vertexNodes.push_back( _Node( 0, _edgeIntPnts[iP] ));
            ++_nbIntNodes;
          }
        }
        } // switch( nbFacets )

      } // loop on _edgeIntPnts

      //SMESHUtils::FreeVector( _edgeIntPnts );
    }
  }
  //================================================================================
  /*!
   * \brief Initializes its data by given grid cell (countered from zero)
   */
  void Hexahedron::init( size_t iCell )
  {
    size_t iNbCell = _grid->_coords[0].size() - 1;
    size_t jNbCell = _grid->_coords[1].size() - 1;
    _i = iCell % iNbCell;
    _j = ( iCell % ( iNbCell * jNbCell )) / iNbCell;
    _k = iCell / iNbCell / jNbCell;
    init( _i, _j, _k );
  }

  //================================================================================
  /*!
   * \brief Compute mesh volumes resulted from intersection of the Hexahedron
   */
  void Hexahedron::ComputeElements()
  {
    Init();

    if ( _nbCornerNodes + _nbIntNodes < 4 )
      return;

    if ( _nbBndNodes == _nbCornerNodes && isInHole() )
      return;

    _polygons.clear();
    _polygons.reserve( 10 );

    // create polygons from quadrangles and get their nodes

    vector<_Node*> chainNodes;
    //vector<_Node*> nodes; // nodes of a face
    //nodes.reserve( _nbCornerNodes + _nbIntNodes );

    _Link polyLink;

    bool hasEdgeIntersections = !_vertexNodes.empty();

    for ( int iF = 0; iF < 6; ++iF ) // loop on 6 sides of a hexahedron
    {
      _Face& quad = _hexQuads[ iF ] ;

      if ( !quad._edgeNodes.empty() )
        hasEdgeIntersections = true;

      _polygons.resize( _polygons.size() + 1 );
      _Face& polygon = _polygons.back();
      polygon._polyLinks.reserve( 20 );

      // add splits of links to a polygon and add _polyLinks to make
      // polygon's boundary closed
      for ( int iE = 0; iE < 4; ++iE ) // loop on 4 sides of a quadrangle
      {
        int nbSpits = quad._links[ iE ].NbResultLinks();
        for ( int iS = 0; iS < nbSpits; ++iS )
        {
          _OrientedLink split = quad._links[ iE ].ResultLink( iS );
          _Node* n1 = split.FirstNode();
          _Node* n2 = split.LastNode();
          if ( !n1->IsLinked( n2->_intPoint ))
          {
            if ( findChain( n1, n2, quad, chainNodes ))
            {
              for ( size_t i = 1; i < chainNodes.size(); ++i )
              {
                polyLink._nodes[0] = chainNodes[i-1];
                polyLink._nodes[1] = chainNodes[i];
                polygon._polyLinks.push_back( polyLink );
                polygon._links.push_back( _OrientedLink( &polygon._polyLinks.back() ));
              }
              continue;
            }
          }
          if ( !polygon._links.empty() )
          {
            _Node* nPrev = polygon._links.back().LastNode();
            if ( nPrev != n1 )
            {
              findChain( nPrev, n1, quad, chainNodes );
              for ( size_t i = 1; i < chainNodes.size(); ++i )
              {
                polyLink._nodes[0] = chainNodes[i-1];
                polyLink._nodes[1] = chainNodes[i];
                polygon._polyLinks.push_back( polyLink );
                polygon._links.push_back( _OrientedLink( &polygon._polyLinks.back() ));
              }
            }
          }
          polygon._links.push_back( split );
        }
      }
      if ( polygon._links.size() > 1 )
      {
        _Node* n1 = polygon._links.back().LastNode();
        _Node* n2 = polygon._links.front().FirstNode();
        if ( n1 != n2 )
        {
          findChain( n1, n2, quad, chainNodes );
          for ( size_t i = 1; i < chainNodes.size(); ++i )
          {
            polyLink._nodes[0] = chainNodes[i-1];
            polyLink._nodes[1] = chainNodes[i];
            polygon._polyLinks.push_back( polyLink );
            polygon._links.push_back( _OrientedLink( &polygon._polyLinks.back() ));
          }
        }
        // add polygon to its links
        for ( size_t iL = 0; iL < polygon._links.size(); ++iL )
        {
          polygon._links[ iL ]._link->_faces.reserve( 2 );
          polygon._links[ iL ]._link->_faces.push_back( &polygon );
        }
      }
      else
      {
        _polygons.resize( _polygons.size() - 1 );
      }
    }

    // create polygons closing holes in a polyhedron

    // find free links
    vector< _OrientedLink* > freeLinks;
    freeLinks.reserve(20);
    for ( size_t iP = 0; iP < _polygons.size(); ++iP )
    {
      _Face& polygon = _polygons[ iP ];
      for ( size_t iL = 0; iL < polygon._links.size(); ++iL )
        if ( polygon._links[ iL ]._link->_faces.size() < 2 )
          freeLinks.push_back( & polygon._links[ iL ]);
    }
    int nbFreeLinks = freeLinks.size();
    if ( 0 < nbFreeLinks && nbFreeLinks < 3 ) return;

     set<TGeomID> usedFaceIDs;
    // if ( hasEdgeIntersections )
    // {
    // // detect FACEs supporting remaining polygons
    //   map<TGeomID,int> nbUsesOfFace;
    //   map<TGeomID,int>::iterator f2nb;
    //   for ( size_t iL = 0; iL < freeLinks.size(); ++iL )
    //     if ( const B_IntersectPoint* ip = freeLinks[ iL ]->FirstNode()->_intPoint )
    //       for ( size_t i = 0; i < ip->_faceIDs.size(); ++i )
    //       {
    //         f2nb = nbUsesOfFace.insert( make_pair( ip->_faceIDs[i], 0 )).first;
    //         f2nb->second++;
    //       }
    //   for ( f2nb = nbUsesOfFace.begin(); f2nb != nbUsesOfFace.end(); ++f2nb )
    //     if ( f2nb->second >= 3 )
    //       usefulFaceIDs.insert( usefulFaceIDs.end(), f2nb->first );
    // }
    // make closed chains of free links
    while ( nbFreeLinks > 0 )
    {
      _polygons.resize( _polygons.size() + 1 );
      _Face& polygon = _polygons.back();
      polygon._polyLinks.reserve( 20 );

      _OrientedLink* curLink = 0;
      _Node*         curNode;
      if ( !hasEdgeIntersections )
      {
        // get a remaining link to start from
        for ( size_t iL = 0; iL < freeLinks.size() && !curLink; ++iL )
          if (( curLink = freeLinks[ iL ] ))
            freeLinks[ iL ] = 0;
        polygon._links.push_back( *curLink );
        --nbFreeLinks;
        do
        {
          // find all links connected to curLink
          curNode = curLink->FirstNode();
          curLink = 0;
          for ( size_t iL = 0; iL < freeLinks.size() && !curLink; ++iL )
            if ( freeLinks[ iL ] && freeLinks[ iL ]->LastNode() == curNode )
            {
              curLink = freeLinks[ iL ];
              freeLinks[ iL ] = 0;
              polygon._links.push_back( *curLink );
              --nbFreeLinks;
            }
        } while ( curLink );
      }
      else // there are intersections with EDGEs
      {
        TGeomID curFace;
        // get a remaining link to start from, one lying on minimal
        // nb of FACEs
        {
          map< vector< TGeomID >, int > facesOfLink;
          map< vector< TGeomID >, int >::iterator f2l;
          for ( size_t iL = 0; iL < freeLinks.size(); ++iL )
            if ( freeLinks[ iL ] )
            {
              f2l = facesOfLink.insert
                ( make_pair( freeLinks[ iL ]->GetNotUsedFace( usedFaceIDs ), iL )).first;
              if ( f2l->first.size() == 1 )
                break;
            }
          f2l = facesOfLink.begin();
          if ( f2l->first.empty() )
            return;
          curFace = f2l->first[0];
          curLink = freeLinks[ f2l->second ];
          freeLinks[ f2l->second ] = 0;
        }
        usedFaceIDs.insert( curFace );
        polygon._links.push_back( *curLink );
        --nbFreeLinks;

        // find all links bounding a FACE of curLink
        do
        {
          // go forward from curLink
          curNode = curLink->LastNode();
          curLink = 0;
          for ( size_t iL = 0; iL < freeLinks.size() && !curLink; ++iL )
            if ( freeLinks[ iL ] &&
                 freeLinks[ iL ]->FirstNode() == curNode &&
                 freeLinks[ iL ]->LastNode()->IsOnFace( curFace ))
            {
              curLink = freeLinks[ iL ];
              freeLinks[ iL ] = 0;
              polygon._links.push_back( *curLink );
              --nbFreeLinks;
            }
        } while ( curLink );

        std::reverse( polygon._links.begin(), polygon._links.end() );

        curLink = & polygon._links.back();
        do
        {
          // go backward from curLink
          curNode = curLink->FirstNode();
          curLink = 0;
          for ( size_t iL = 0; iL < freeLinks.size() && !curLink; ++iL )
            if ( freeLinks[ iL ] &&
                 freeLinks[ iL ]->LastNode() == curNode &&
                 freeLinks[ iL ]->FirstNode()->IsOnFace( curFace ))
            {
              curLink = freeLinks[ iL ];
              freeLinks[ iL ] = 0;
              polygon._links.push_back( *curLink );
              --nbFreeLinks;
            }
        } while ( curLink );

        curNode = polygon._links.back().FirstNode();

        if ( polygon._links[0].LastNode() != curNode )
        {
          if ( !_vertexNodes.empty() )
          {
            // add links with _vertexNodes if not already used
            for ( size_t iN = 0; iN < _vertexNodes.size(); ++iN )
              if ( _vertexNodes[ iN ].IsOnFace( curFace ))
              {
                bool used = ( curNode == &_vertexNodes[ iN ] );
                for ( size_t iL = 0; iL < polygon._links.size() && !used; ++iL )
                  used = ( &_vertexNodes[ iN ] ==  polygon._links[ iL ].LastNode() );
                if ( !used )
                {
                  polyLink._nodes[0] = &_vertexNodes[ iN ];
                  polyLink._nodes[1] = curNode;
                  polygon._polyLinks.push_back( polyLink );
                  polygon._links.push_back( _OrientedLink( &polygon._polyLinks.back() ));
                  freeLinks.push_back( &polygon._links.back() );
                  ++nbFreeLinks;
                  curNode = &_vertexNodes[ iN ];
                }
                // TODO: to reorder _vertexNodes within polygon, if there are several ones
              }
          }
          polyLink._nodes[0] = polygon._links[0].LastNode();
          polyLink._nodes[1] = curNode;
          polygon._polyLinks.push_back( polyLink );
          polygon._links.push_back( _OrientedLink( &polygon._polyLinks.back() ));
          freeLinks.push_back( &polygon._links.back() );
          ++nbFreeLinks;
        }

      } // if there are intersections with EDGEs

      if ( polygon._links.size() < 3 ||
           polygon._links[0].LastNode() != polygon._links.back().FirstNode() )
        return; // closed polygon not found -> invalid polyhedron

      // add polygon to its links
      for ( size_t iL = 0; iL < polygon._links.size(); ++iL )
      {
        polygon._links[ iL ]._link->_faces.reserve( 2 );
        polygon._links[ iL ]._link->_faces.push_back( &polygon );
        polygon._links[ iL ].Reverse();
      }
    } // while ( nbFreeLinks > 0 )

    if ( ! checkPolyhedronSize() )
    {
      return;
    }

    // create a classic cell if possible
    const int nbNodes = _nbCornerNodes + _nbIntNodes;
    bool isClassicElem = false;
    if (      nbNodes == 8 && _polygons.size() == 6 ) isClassicElem = addHexa();
    else if ( nbNodes == 4 && _polygons.size() == 4 ) isClassicElem = addTetra();
    else if ( nbNodes == 6 && _polygons.size() == 5 ) isClassicElem = addPenta();
    else if ( nbNodes == 5 && _polygons.size() == 5 ) isClassicElem = addPyra ();
    if ( !isClassicElem )
    {
      _volumeDefs._nodes.clear();
      _volumeDefs._quantities.clear();

      for ( size_t iF = 0; iF < _polygons.size(); ++iF )
      {
        const size_t nbLinks = _polygons[ iF ]._links.size();
        _volumeDefs._quantities.push_back( nbLinks );
        for ( size_t iL = 0; iL < nbLinks; ++iL )
          _volumeDefs._nodes.push_back( _polygons[ iF ]._links[ iL ].FirstNode() );
      }
    }
  }
  //================================================================================
  /*!
   * \brief Create elements in the mesh
   */
  int Hexahedron::MakeElements(SMESH_MesherHelper&                      helper,
                               const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap)
  {
    SMESHDS_Mesh* mesh = helper.GetMeshDS();

    size_t nbCells[3] = { _grid->_coords[0].size() - 1,
                          _grid->_coords[1].size() - 1,
                          _grid->_coords[2].size() - 1 };
    const size_t nbGridCells = nbCells[0] * nbCells[1] * nbCells[2];
    vector< Hexahedron* > intersectedHex( nbGridCells, 0 );
    int nbIntHex = 0;

    // set intersection nodes from GridLine's to links of intersectedHex
    int i,j,k, iDirOther[3][2] = {{ 1,2 },{ 0,2 },{ 0,1 }};
    for ( int iDir = 0; iDir < 3; ++iDir )
    {
      int dInd[4][3] = { {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} };
      dInd[1][ iDirOther[iDir][0] ] = -1;
      dInd[2][ iDirOther[iDir][1] ] = -1;
      dInd[3][ iDirOther[iDir][0] ] = -1; dInd[3][ iDirOther[iDir][1] ] = -1;
      // loop on GridLine's parallel to iDir
      LineIndexer lineInd = _grid->GetLineIndexer( iDir );
      for ( ; lineInd.More(); ++lineInd )
      {
        GridLine& line = _grid->_lines[ iDir ][ lineInd.LineIndex() ];
        multiset< F_IntersectPoint >::const_iterator ip = line._intPoints.begin();
        for ( ; ip != line._intPoints.end(); ++ip )
        {
          if ( !ip->_node ) continue;
          lineInd.SetIndexOnLine( ip->_indexOnLine );
          for ( int iL = 0; iL < 4; ++iL ) // loop on 4 cells sharing a link
          {
            i = int(lineInd.I()) + dInd[iL][0];
            j = int(lineInd.J()) + dInd[iL][1];
            k = int(lineInd.K()) + dInd[iL][2];
            if ( i < 0 || i >= nbCells[0] ||
                 j < 0 || j >= nbCells[1] ||
                 k < 0 || k >= nbCells[2] ) continue;

            const size_t hexIndex = _grid->CellIndex( i,j,k );
            Hexahedron *& hex = intersectedHex[ hexIndex ];
            if ( !hex)
            {
              hex = new Hexahedron( *this );
              hex->_i = i;
              hex->_j = j;
              hex->_k = k;
              ++nbIntHex;
            }
            const int iLink = iL + iDir * 4;
            hex->_hexLinks[iLink]._intNodes.push_back( _Node( 0, &(*ip) ));
            hex->_nbIntNodes++;
          }
        }
      }
    }

    // implement geom edges into the mesh
    addEdges( helper, intersectedHex, edge2faceIDsMap );

    // add not split hexadrons to the mesh
    int nbAdded = 0;
    vector<int> intHexInd( nbIntHex );
    nbIntHex = 0;
    for ( size_t i = 0; i < intersectedHex.size(); ++i )
    {
      Hexahedron * & hex = intersectedHex[ i ];
      if ( hex )
      {
        intHexInd[ nbIntHex++ ] = i;
        if ( hex->_nbIntNodes > 0 ) continue;
        init( hex->_i, hex->_j, hex->_k );
      }
      else
      {    
        init( i );
      }
      if ( _nbCornerNodes == 8 && ( _nbBndNodes < _nbCornerNodes || !isInHole() ))
      {
        // order of _hexNodes is defined by enum SMESH_Block::TShapeID
        SMDS_MeshElement* el =
          mesh->AddVolume( _hexNodes[0].Node(), _hexNodes[2].Node(),
                           _hexNodes[3].Node(), _hexNodes[1].Node(),
                           _hexNodes[4].Node(), _hexNodes[6].Node(),
                           _hexNodes[7].Node(), _hexNodes[5].Node() );
        mesh->SetMeshElementOnShape( el, helper.GetSubShapeID() );
        ++nbAdded;
        if ( hex )
        {
          delete hex;
          intersectedHex[ i ] = 0;
          --nbIntHex;
        }
      }
      else if ( _nbCornerNodes > 3  && !hex )
      {
        // all intersection of hex with geometry are at grid nodes
        hex = new Hexahedron( *this );
        hex->init( i );
        intHexInd.push_back(0);
        intHexInd[ nbIntHex++ ] = i;
      }
    }

    // add elements resulted from hexadron intersection
#ifdef WITH_TBB
    intHexInd.resize( nbIntHex );
    tbb::parallel_for ( tbb::blocked_range<size_t>( 0, nbIntHex ),
                        ParallelHexahedron( intersectedHex, intHexInd ),
                        tbb::simple_partitioner()); // ComputeElements() is called here
    for ( size_t i = 0; i < intHexInd.size(); ++i )
      if ( Hexahedron * hex = intersectedHex[ intHexInd[ i ]] )
        nbAdded += hex->addElements( helper );
#else
    for ( size_t i = 0; i < intHexInd.size(); ++i )
      if ( Hexahedron * hex = intersectedHex[ intHexInd[ i ]] )
      {
        hex->ComputeElements();
        nbAdded += hex->addElements( helper );
      }
#endif

    for ( size_t i = 0; i < intersectedHex.size(); ++i )
      if ( intersectedHex[ i ] )
        delete intersectedHex[ i ];

    return nbAdded;
  }

  //================================================================================
  /*!
   * \brief Implements geom edges into the mesh
   */
  void Hexahedron::addEdges(SMESH_MesherHelper&                      helper,
                            vector< Hexahedron* >&                   hexes,
                            const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap)
  {
    if ( edge2faceIDsMap.empty() ) return;

    // Prepare planes for intersecting with EDGEs
    GridPlanes pln[3];
    {
      gp_XYZ origPnt = ( _grid->_coords[0][0] * _grid->_axes[0] +
                         _grid->_coords[1][0] * _grid->_axes[1] +
                         _grid->_coords[2][0] * _grid->_axes[2] );
      for ( int iDirZ = 0; iDirZ < 3; ++iDirZ ) // iDirZ gives normal direction to planes
      {
        GridPlanes& planes = pln[ iDirZ ];
        int iDirX = ( iDirZ + 1 ) % 3;
        int iDirY = ( iDirZ + 2 ) % 3;
        planes._uNorm  = ( _grid->_axes[ iDirY ] ^ _grid->_axes[ iDirZ ] ).Normalized();
        planes._vNorm  = ( _grid->_axes[ iDirZ ] ^ _grid->_axes[ iDirX ] ).Normalized();
        planes._zNorm  = ( _grid->_axes[ iDirX ] ^ _grid->_axes[ iDirY ] ).Normalized();
        double   uvDot = planes._uNorm * planes._vNorm;
        planes._factor = sqrt( 1. - uvDot * uvDot );
        planes._origins.resize( _grid->_coords[ iDirZ ].size() );
        planes._zProjs.resize ( _grid->_coords[ iDirZ ].size() );
        planes._origins[0] = origPnt;
        planes._zProjs [0] = 0;
        const double       zFactor = _grid->_axes[ iDirZ ] * planes._zNorm;
        const vector< double > & u = _grid->_coords[ iDirZ ];
        for ( int i = 1; i < planes._origins.size(); ++i )
        {
          planes._origins[i] = origPnt + _grid->_axes[ iDirZ ] * ( u[i] - u[0] );
          planes._zProjs [i] = zFactor * ( u[i] - u[0] );
        }
      }
    }
    const double deflection = _grid->_minCellSize / 20.;
    // int facets[6] = { SMESH_Block::ID_F0yz, SMESH_Block::ID_F1yz,
    //                   SMESH_Block::ID_Fx0z, SMESH_Block::ID_Fx1z,
    //                   SMESH_Block::ID_Fxy0, SMESH_Block::ID_Fxy1 };
    E_IntersectPoint ip;
    //ip._faceIDs.reserve(2);

    // Intersect EDGEs with the planes
    map< TGeomID, vector< TGeomID > >::const_iterator e2fIt = edge2faceIDsMap.begin();
    for ( ; e2fIt != edge2faceIDsMap.end(); ++e2fIt )
    {
      const TGeomID  edgeID = e2fIt->first;
      const TopoDS_Edge & E = TopoDS::Edge( _grid->_shapes( edgeID ));
      BRepAdaptor_Curve curve( E );

      ip._faceIDs = e2fIt->second;
      ip._shapeID = edgeID;

      // discretize the EGDE
      GCPnts_UniformDeflection discret( curve, deflection, true );
      if ( !discret.IsDone() || discret.NbPoints() < 2 )
        continue;

      // perform intersection
      for ( int iDirZ = 0; iDirZ < 3; ++iDirZ )
      {
        GridPlanes& planes = pln[ iDirZ ];
        int      iDirX = ( iDirZ + 1 ) % 3;
        int      iDirY = ( iDirZ + 2 ) % 3;
        double    xLen = _grid->_coords[ iDirX ].back() - _grid->_coords[ iDirX ][0];
        double    yLen = _grid->_coords[ iDirY ].back() - _grid->_coords[ iDirY ][0];
        double zFactor = _grid->_axes[ iDirZ ] * planes._zNorm;
        //int*   zFacets = facets + iDirZ * 2;

        // locate the 1st point of a segment within the grid
        gp_XYZ p1     = discret.Value( 1 ).XYZ();
        double u1     = discret.Parameter( 1 );
        double zProj1 = planes._zNorm * ( p1 - planes._origins[0] );
        gp_Pnt orig   = planes._origins[0] + planes._zNorm * zProj1;
        gp_XY uv      = planes.GetUV( p1, orig );
        int iX1       = int( uv.X() / xLen * ( _grid->_coords[ iDirX ].size() - 1. ));
        int iY1       = int( uv.Y() / yLen * ( _grid->_coords[ iDirY ].size() - 1. ));
        int iZ1       = int( zProj1 / planes._zProjs.back() * ( planes._zProjs.size() - 1. ));
        locateValue( iX1, uv.X(), _grid->_coords[ iDirX ] );
        locateValue( iY1, uv.Y(), _grid->_coords[ iDirY ] );
        locateValue( iZ1, zProj1, planes._zProjs );

        int ijk[3]; // grid index where a segment intersect a plane
        ijk[ iDirX ] = iX1;
        ijk[ iDirY ] = iY1;
        ijk[ iDirZ ] = iZ1;
        ip._uvw[ iDirX ] = uv.X()           + _grid->_coords[ iDirX ][0];
        ip._uvw[ iDirY ] = uv.Y()           + _grid->_coords[ iDirY ][0];
        ip._uvw[ iDirZ ] = zProj1 / zFactor + _grid->_coords[ iDirZ ][0];

        // add the 1st vertex point to a hexahedron
        size_t hexIndex = _grid->CellIndex( ijk[0], ijk[1], ijk[2] );
        if ( iDirZ == 0 && hexIndex < hexes.size() && hexes[ hexIndex ] )
        {
          //ip._shapeID = _grid->_shapes.Add( helper.IthVertex( 0, curve.Edge(),/*CumOri=*/false));
          ip._point   = p1;
          _grid->_edgeIntP.push_back( ip );
          hexes[ hexIndex ]->addIntersection( _grid->_edgeIntP.back() );
        }

        for ( int iP = 2; iP <= discret.NbPoints(); ++iP )
        {
          // locate the 2nd point of a segment within the grid
          gp_XYZ p2     = discret.Value( iP ).XYZ();
          double u2     = discret.Parameter( iP );
          double zProj2 = planes._zNorm * ( p2 - planes._origins[0] );
          int iZ2       = iZ1;
          locateValue( iZ2, zProj2, planes._zProjs );

          // treat intersections with planes between 2 points of a segment
          int dZ = ( iZ1 <= iZ2 ) ? +1 : -1;
          int iZ = iZ1 + ( iZ1 < iZ2 );
          for ( int i = 0, nb = Abs( iZ1 - iZ2 ); i < nb; ++i, iZ += dZ )
          {
            double r  = (( planes._zProjs[ iZ ] - zProj1 ) / ( zProj2 - zProj1 ));
            ip._point = curve.Value( u1 * ( 1 - r ) + u2 * r );
            gp_XY uv  = planes.GetUV( ip._point, planes._origins[ iZ ]);
            locateValue( ijk[ iDirX ], uv.X(), _grid->_coords[ iDirX ] );
            locateValue( ijk[ iDirY ], uv.Y(), _grid->_coords[ iDirY ] );
            ijk[ iDirZ ] = iZ;
            ip._uvw[ iDirX ] = uv.X()                         + _grid->_coords[ iDirX ][0];
            ip._uvw[ iDirY ] = uv.Y()                         + _grid->_coords[ iDirY ][0];
            ip._uvw[ iDirZ ] = planes._zProjs[ iZ ] / zFactor + _grid->_coords[ iDirZ ][0];

            _grid->_edgeIntP.push_back( ip );

            // add ip to hex "above" the plane
            hexIndex = _grid->CellIndex( ijk[0], ijk[1], ijk[2] );
            if ( hexIndex < hexes.size() && hexes[ hexIndex ] )
              hexes[ hexIndex ]->addIntersection( _grid->_edgeIntP.back() );

            // add ip to hex "below" the plane
            ijk[ iDirZ ] = iZ-1;
            hexIndex = _grid->CellIndex( ijk[0], ijk[1], ijk[2] );
            if ( hexIndex < hexes.size() && hexes[ hexIndex ] )
              hexes[ hexIndex ]->addIntersection( _grid->_edgeIntP.back() );
          }
          iZ1    = iZ2;
          p1     = p2;
          u1     = u2;
          zProj1 = zProj2;
        }
        // add the 2nd vertex point to a hexahedron
        if ( iDirZ == 0 )
        {
          orig = planes._origins[0] + planes._zNorm * zProj1;
          uv   = planes.GetUV( p1, orig );
          locateValue( ijk[ iDirX ], uv.X(), _grid->_coords[ iDirX ] );
          locateValue( ijk[ iDirY ], uv.Y(), _grid->_coords[ iDirY ] );
          ijk[ iDirZ ] = iZ1;
          hexIndex = _grid->CellIndex( ijk[0], ijk[1], ijk[2] );
          if ( hexIndex < hexes.size() && hexes[ hexIndex ] )
          {
            ip._uvw[ iDirX ] = uv.X()           + _grid->_coords[ iDirX ][0];
            ip._uvw[ iDirY ] = uv.Y()           + _grid->_coords[ iDirY ][0];
            ip._uvw[ iDirZ ] = zProj1 / zFactor + _grid->_coords[ iDirZ ][0];
            ip._point   = p1;
            _grid->_edgeIntP.push_back( ip );
            hexes[ hexIndex ]->addIntersection( _grid->_edgeIntP.back() );
          }
        }
      } // loop on 3 grid directions
    } // loop on EDGEs

    // Create nodes at found intersections
    // const E_IntersectPoint* eip;
    // for ( size_t i = 0; i < hexes.size(); ++i )
    // {
    //   Hexahedron* h = hexes[i];
    //   if ( !h ) continue;
    //   for ( int iF = 0; iF < 6; ++iF )
    //   {
    //     _Face& quad = h->_hexQuads[ iF ];
    //     for ( size_t iP = 0; iP < quad._edgeNodes.size(); ++iP )
    //       if ( !quad._edgeNodes[ iP ]._node )
    //         if (( eip = quad._edgeNodes[ iP ].EdgeIntPnt() ))
    //           quad._edgeNodes[ iP ]._intPoint->_node = helper.AddNode( eip->_point.X(),
    //                                                                    eip->_point.Y(),
    //                                                                    eip->_point.Z() );
    //   }
    //   for ( size_t iP = 0; iP < hexes[i]->_vertexNodes.size(); ++iP )
    //     if (( eip = h->_vertexNodes[ iP ].EdgeIntPnt() ))
    //       h->_vertexNodes[ iP ]._intPoint->_node = helper.AddNode( eip->_point.X(),
    //                                                                eip->_point.Y(),
    //                                                                eip->_point.Z() );
    // }
  }

  //================================================================================
  /*!
   * \brief Returns index of a hexahedron sub-entities holding a point
   *  \param [in] ip - intersection point
   *  \param [out] facets - 0-3 facets holding a point
   *  \param [out] sub - index of a vertex or an edge holding a point
   *  \return int - number of facets holding a point
   */
  int Hexahedron::getEntity( const E_IntersectPoint* ip, int* facets, int& sub )
  {
    enum { X = 4, Y = 2, Z = 1 }; // == 100, 010, 001
    int nbFacets = 0;
    int vertex = 0, egdeMask = 0;

    if ( Abs( _grid->_coords[0][ _i   ] - ip->_uvw[0] ) < _grid->_tol ) {
      facets[ nbFacets++ ] = SMESH_Block::ID_F0yz;
      egdeMask |= X;
    }
    if ( Abs( _grid->_coords[0][ _i+1 ] - ip->_uvw[0] ) < _grid->_tol ) {
      facets[ nbFacets++ ] = SMESH_Block::ID_F1yz;
      vertex   |= X;
      egdeMask |= X;
    }
    if ( Abs( _grid->_coords[1][ _j   ] - ip->_uvw[1] ) < _grid->_tol ) {
      facets[ nbFacets++ ] = SMESH_Block::ID_Fx0z;
      egdeMask |= Y;
    }
    if ( Abs( _grid->_coords[1][ _j+1 ] - ip->_uvw[1] ) < _grid->_tol ) {
      facets[ nbFacets++ ] = SMESH_Block::ID_Fx1z;
      vertex   |= Y;
      egdeMask |= Y;
    }
    if ( Abs( _grid->_coords[2][ _k   ] - ip->_uvw[2] ) < _grid->_tol ) {
      facets[ nbFacets++ ] = SMESH_Block::ID_Fxy0;
      egdeMask |= Z;
    }
    if ( Abs( _grid->_coords[2][ _k+1 ] - ip->_uvw[2] ) < _grid->_tol ) {
      facets[ nbFacets++ ] = SMESH_Block::ID_Fxy1;
      vertex   |= Z;
      egdeMask |= Z;
    }
    if ( nbFacets == 3 )
    {
      sub = vertex + SMESH_Block::ID_FirstV;
    }
    else if ( nbFacets == 2 )
    {
      const int edge [3][8] = {
        { SMESH_Block::ID_E00z, 0, SMESH_Block::ID_E01z, 0,
          SMESH_Block::ID_E10z, 0, SMESH_Block::ID_E11z },
        { SMESH_Block::ID_E0y0, SMESH_Block::ID_E0y1, 0, 0,
          SMESH_Block::ID_E1y0, SMESH_Block::ID_E1y1, 0, 0 },
        { SMESH_Block::ID_Ex00, SMESH_Block::ID_Ex01,
          SMESH_Block::ID_Ex10, SMESH_Block::ID_Ex11, 0, 0, 0, 0 }
      };
      switch ( egdeMask ) {
      case X | Y: sub = edge[ 0 ][ vertex ]; break;
      case X | Z: sub = edge[ 1 ][ vertex ]; break;
      default:    sub = edge[ 2 ][ vertex ];
      }
    }
    else if ( nbFacets == 1 )
    {
      sub = facets[0];
    }
    else // nbFacets == 0
    {
    }
    return nbFacets;
  }
  //================================================================================
  /*!
   * \brief Adds intersection with an EDGE, either to a _Face or inside a hex
   */
  void Hexahedron::addIntersection( const E_IntersectPoint& ip )
  {
    // check if ip is really inside the hex
#ifdef _DEBUG_
    if (( _grid->_coords[0][ _i   ] - _grid->_tol > ip._uvw[0] ) ||
        ( _grid->_coords[0][ _i+1 ] + _grid->_tol < ip._uvw[0] ) ||
        ( _grid->_coords[1][ _j   ] - _grid->_tol > ip._uvw[1] ) ||
        ( _grid->_coords[1][ _j+1 ] + _grid->_tol < ip._uvw[1] ) ||
        ( _grid->_coords[2][ _k   ] - _grid->_tol > ip._uvw[2] ) ||
        ( _grid->_coords[2][ _k+1 ] + _grid->_tol < ip._uvw[2] ))
      throw SALOME_Exception("ip outside a hex");
#endif
    _edgeIntPnts.push_back( & ip );
  }
  //================================================================================
  /*!
   * \brief Finds nodes at a path from one node to another via intersections with EDGEs
   */
  bool Hexahedron::findChain( _Node*          n1,
                              _Node*          n2,
                              _Face&          quad,
                              vector<_Node*>& chn )
  {
    chn.clear();
    chn.push_back( n1 );
    bool found = false;
    do
    {
      found = false;
      for ( size_t iP = 0; iP < quad._edgeNodes.size(); ++iP )
        if (( std::find( ++chn.begin(), chn.end(), & quad._edgeNodes[iP]) == chn.end()) &&
            chn.back()->IsLinked( quad._edgeNodes[ iP ]._intPoint ))
        {
          chn.push_back( & quad._edgeNodes[ iP ]);
          found = true;
          break;
        }
    } while ( found && chn.back() != n2 );

    if ( chn.back() != n2 )
      chn.push_back( n2 );

    return chn.size() > 2;
  }
  //================================================================================
  /*!
   * \brief Adds computed elements to the mesh
   */
  int Hexahedron::addElements(SMESH_MesherHelper& helper)
  {
    int nbAdded = 0;
    // add elements resulted from hexahedron intersection
    //for ( size_t i = 0; i < _volumeDefs.size(); ++i )
    {
      vector< const SMDS_MeshNode* > nodes( _volumeDefs._nodes.size() );
      for ( size_t iN = 0; iN < nodes.size(); ++iN )
        if ( !( nodes[iN] = _volumeDefs._nodes[iN]->Node() ))
        {
          if ( const E_IntersectPoint* eip = _volumeDefs._nodes[iN]->EdgeIntPnt() )
            nodes[iN] = _volumeDefs._nodes[iN]->_intPoint->_node =
              helper.AddNode( eip->_point.X(),
                              eip->_point.Y(),
                              eip->_point.Z() );
          else
            throw SALOME_Exception("Bug: no node at intersection point");
        }

      if ( !_volumeDefs._quantities.empty() )
      {
        helper.AddPolyhedralVolume( nodes, _volumeDefs._quantities );
      }
      else
      {
        switch ( nodes.size() )
        {
        case 8: helper.AddVolume( nodes[0],nodes[1],nodes[2],nodes[3],
                                  nodes[4],nodes[5],nodes[6],nodes[7] );
          break;
        case 4: helper.AddVolume( nodes[0],nodes[1],nodes[2],nodes[3] );
          break;
        case 6: helper.AddVolume( nodes[0],nodes[1],nodes[2],nodes[3], nodes[4],nodes[5] );
          break;
        case 5:
          helper.AddVolume( nodes[0],nodes[1],nodes[2],nodes[3],nodes[4] );
          break;
        }
      }
      nbAdded += int ( _volumeDefs._nodes.size() > 0 );
    }

    return nbAdded;
  }
  //================================================================================
  /*!
   * \brief Return true if the element is in a hole
   */
  bool Hexahedron::isInHole() const
  {
    const int ijk[3] = { _i, _j, _k };
    F_IntersectPoint curIntPnt;

    // consider a cell to be in a hole if all links in any direction
    // comes OUT of geometry
    for ( int iDir = 0; iDir < 3; ++iDir )
    {
      const vector<double>& coords = _grid->_coords[ iDir ];
      LineIndexer               li = _grid->GetLineIndexer( iDir );
      li.SetIJK( _i,_j,_k );
      size_t lineIndex[4] = { li.LineIndex  (),
                              li.LineIndex10(),
                              li.LineIndex01(),
                              li.LineIndex11() };
      bool allLinksOut = true, hasLinks = false;
      for ( int iL = 0; iL < 4 && allLinksOut; ++iL ) // loop on 4 links parallel to iDir
      {
        const _Link& link = _hexLinks[ iL + 4*iDir ];
        // check transition of the first node of a link
        const F_IntersectPoint* firstIntPnt = 0;
        if ( link._nodes[0]->Node() ) // 1st node is a hexa corner
        {
          curIntPnt._paramOnLine = coords[ ijk[ iDir ]] - coords[0];
          const GridLine& line = _grid->_lines[ iDir ][ lineIndex[ iL ]];
          multiset< F_IntersectPoint >::const_iterator ip =
            line._intPoints.upper_bound( curIntPnt );
          --ip;
          firstIntPnt = &(*ip);
        }
        else if ( !link._intNodes.empty() )
        {
          firstIntPnt = link._intNodes[0].FaceIntPnt();
        }

        if ( firstIntPnt )
        {
          hasLinks = true;
          allLinksOut = ( firstIntPnt->_transition == Trans_OUT );
        }
      }
      if ( hasLinks && allLinksOut )
        return true;
    }
    return false;
  }

  //================================================================================
  /*!
   * \brief Return true if a polyhedron passes _sizeThreshold criterion
   */
  bool Hexahedron::checkPolyhedronSize() const
  {
    double volume = 0;
    for ( size_t iP = 0; iP < _polygons.size(); ++iP )
    {
      const _Face& polygon = _polygons[iP];
      gp_XYZ area (0,0,0);
      gp_XYZ p1 = polygon._links[ 0 ].FirstNode()->Point().XYZ();
      for ( size_t iL = 0; iL < polygon._links.size(); ++iL )
      {
        gp_XYZ p2 = polygon._links[ iL ].LastNode()->Point().XYZ();
        area += p1 ^ p2;
        p1 = p2;
      }
      volume += p1 * area;
    }
    volume /= 6;

    double initVolume = _sideLength[0] * _sideLength[1] * _sideLength[2];

    return volume > initVolume / _sizeThreshold;
  }
  //================================================================================
  /*!
   * \brief Tries to create a hexahedron
   */
  bool Hexahedron::addHexa()
  {
    if ( _polygons[0]._links.size() != 4 ||
         _polygons[1]._links.size() != 4 ||
         _polygons[2]._links.size() != 4 ||
         _polygons[3]._links.size() != 4 ||
         _polygons[4]._links.size() != 4 ||
         _polygons[5]._links.size() != 4   )
      return false;
    _Node* nodes[8];
    int nbN = 0;
    for ( int iL = 0; iL < 4; ++iL )
    {
      // a base node
      nodes[iL] = _polygons[0]._links[iL].FirstNode();
      ++nbN;

      // find a top node above the base node
      _Link* link = _polygons[0]._links[iL]._link;
      ASSERT( link->_faces.size() > 1 );
      // a quadrangle sharing <link> with _polygons[0]
      _Face* quad = link->_faces[ bool( link->_faces[0] == & _polygons[0] )];
      for ( int i = 0; i < 4; ++i )
        if ( quad->_links[i]._link == link )
        {
          // 1st node of a link opposite to <link> in <quad>
          nodes[iL+4] = quad->_links[(i+2)%4].FirstNode();
          ++nbN;
          break;
        }
    }
    if ( nbN == 8 )
      _volumeDefs.set( vector< _Node* >( nodes, nodes+8 ));

    return nbN == 8;
  }
  //================================================================================
  /*!
   * \brief Tries to create a tetrahedron
   */
  bool Hexahedron::addTetra()
  {
    _Node* nodes[4];
    nodes[0] = _polygons[0]._links[0].FirstNode();
    nodes[1] = _polygons[0]._links[1].FirstNode();
    nodes[2] = _polygons[0]._links[2].FirstNode();

    _Link* link = _polygons[0]._links[0]._link;
    ASSERT( link->_faces.size() > 1 );

    // a triangle sharing <link> with _polygons[0]
    _Face* tria = link->_faces[ bool( link->_faces[0] == & _polygons[0] )];
    for ( int i = 0; i < 3; ++i )
      if ( tria->_links[i]._link == link )
      {
        nodes[3] = tria->_links[(i+1)%3].LastNode();
        _volumeDefs.set( vector< _Node* >( nodes, nodes+4 ));
        return true;
      }

    return false;
  }
  //================================================================================
  /*!
   * \brief Tries to create a pentahedron
   */
  bool Hexahedron::addPenta()
  {
    // find a base triangular face
    int iTri = -1;
    for ( int iF = 0; iF < 5 && iTri < 0; ++iF )
      if ( _polygons[ iF ]._links.size() == 3 )
        iTri = iF;
    if ( iTri < 0 ) return false;

    // find nodes
    _Node* nodes[6];
    int nbN = 0;
    for ( int iL = 0; iL < 3; ++iL )
    {
      // a base node
      nodes[iL] = _polygons[ iTri ]._links[iL].FirstNode();
      ++nbN;

      // find a top node above the base node
      _Link* link = _polygons[ iTri ]._links[iL]._link;
      ASSERT( link->_faces.size() > 1 );
      // a quadrangle sharing <link> with a base triangle
      _Face* quad = link->_faces[ bool( link->_faces[0] == & _polygons[ iTri ] )];
      if ( quad->_links.size() != 4 ) return false;
      for ( int i = 0; i < 4; ++i )
        if ( quad->_links[i]._link == link )
        {
          // 1st node of a link opposite to <link> in <quad>
          nodes[iL+3] = quad->_links[(i+2)%4].FirstNode();
          ++nbN;
          break;
        }
    }
    if ( nbN == 6 )
      _volumeDefs.set( vector< _Node* >( nodes, nodes+6 ));

    return ( nbN == 6 );
  }
  //================================================================================
  /*!
   * \brief Tries to create a pyramid
   */
  bool Hexahedron::addPyra()
  {
    // find a base quadrangle
    int iQuad = -1;
    for ( int iF = 0; iF < 5 && iQuad < 0; ++iF )
      if ( _polygons[ iF ]._links.size() == 4 )
        iQuad = iF;
    if ( iQuad < 0 ) return false;

    // find nodes
    _Node* nodes[5];
    nodes[0] = _polygons[iQuad]._links[0].FirstNode();
    nodes[1] = _polygons[iQuad]._links[1].FirstNode();
    nodes[2] = _polygons[iQuad]._links[2].FirstNode();
    nodes[3] = _polygons[iQuad]._links[3].FirstNode();

    _Link* link = _polygons[iQuad]._links[0]._link;
    ASSERT( link->_faces.size() > 1 );

    // a triangle sharing <link> with a base quadrangle
    _Face* tria = link->_faces[ bool( link->_faces[0] == & _polygons[ iQuad ] )];
    if ( tria->_links.size() != 3 ) return false;
    for ( int i = 0; i < 3; ++i )
      if ( tria->_links[i]._link == link )
      {
        nodes[4] = tria->_links[(i+1)%3].LastNode();
        _volumeDefs.set( vector< _Node* >( nodes, nodes+5 ));
        return true;
      }

    return false;
  }

} // namespace

//=============================================================================
/*!
 * \brief Generates 3D structured Cartesian mesh in the internal part of
 * solid shapes and polyhedral volumes near the shape boundary.
 *  \param theMesh - mesh to fill in
 *  \param theShape - a compound of all SOLIDs to mesh
 *  \retval bool - true in case of success
 */
//=============================================================================

bool StdMeshers_Cartesian_3D::Compute(SMESH_Mesh &         theMesh,
                                      const TopoDS_Shape & theShape)
{
  // The algorithm generates the mesh in following steps:

  // 1) Intersection of grid lines with the geometry boundary.
  // This step allows to find out if a given node of the initial grid is
  // inside or outside the geometry.

  // 2) For each cell of the grid, check how many of it's nodes are outside
  // of the geometry boundary. Depending on a result of this check
  // - skip a cell, if all it's nodes are outside
  // - skip a cell, if it is too small according to the size threshold
  // - add a hexahedron in the mesh, if all nodes are inside
  // - add a polyhedron in the mesh, if some nodes are inside and some outside

  _computeCanceled = false;

  try
  {
    Grid grid;

    vector< TopoDS_Shape > faceVec;
    {
      TopTools_MapOfShape faceMap;
      for ( TopExp_Explorer fExp( theShape, TopAbs_FACE ); fExp.More(); fExp.Next() )
        if ( faceMap.Add( fExp.Current() )) // skip a face shared by two solids
          faceVec.push_back( fExp.Current() );
    }
    Bnd_Box shapeBox;
    vector<FaceGridIntersector> facesItersectors( faceVec.size() );
    map< TGeomID, vector< TGeomID > > edge2faceIDsMap;
    TopExp_Explorer eExp;
    for ( int i = 0; i < faceVec.size(); ++i )
    {
      facesItersectors[i]._face   = TopoDS::Face    ( faceVec[i] );
      facesItersectors[i]._faceID = grid._shapes.Add( faceVec[i] );
      facesItersectors[i]._grid   = &grid;
      shapeBox.Add( facesItersectors[i].GetFaceBndBox() );

      if ( _hyp->GetToAddEdges() )
        for ( eExp.Init( faceVec[i], TopAbs_EDGE ); eExp.More(); eExp.Next() )
        {
          const TopoDS_Edge& edge = TopoDS::Edge( eExp.Current() );
          if ( !SMESH_Algo::isDegenerated( edge ))
            edge2faceIDsMap[ grid._shapes.Add( edge )].push_back( facesItersectors[i]._faceID );
        }
    }

    vector<double> xCoords, yCoords, zCoords;
    _hyp->GetCoordinates( xCoords, yCoords, zCoords, shapeBox );

    grid.SetCoordinates( xCoords, yCoords, zCoords, _hyp->GetAxisDirs(), theShape );

    // check if the grid encloses the shape
    if ( !_hyp->IsGridBySpacing(0) ||
         !_hyp->IsGridBySpacing(1) ||
         !_hyp->IsGridBySpacing(2) )
    {
      Bnd_Box gridBox;
      gridBox.Add( gp_Pnt( xCoords[0], yCoords[0], zCoords[0] ));
      gridBox.Add( gp_Pnt( xCoords.back(), yCoords.back(), zCoords.back() ));
      double x0,y0,z0, x1,y1,z1;
      shapeBox.Get(x0,y0,z0, x1,y1,z1);
      if ( gridBox.IsOut( gp_Pnt( x0,y0,z0 )) ||
           gridBox.IsOut( gp_Pnt( x1,y1,z1 )))
        for ( size_t i = 0; i < facesItersectors.size(); ++i )
        {
          if ( !facesItersectors[i].IsInGrid( gridBox ))
            return error("The grid doesn't enclose the geometry");
#ifdef ELLIPSOLID_WORKAROUND
          delete facesItersectors[i]._surfaceInt, facesItersectors[i]._surfaceInt = 0;
#endif
        }
    }
    if ( _computeCanceled ) return false;

#ifdef WITH_TBB
    { // copy partner faces and curves of not thread-safe types
      set< const Standard_Transient* > tshapes;
      BRepBuilderAPI_Copy copier;
      for ( size_t i = 0; i < facesItersectors.size(); ++i )
      {
        if ( !facesItersectors[i].IsThreadSafe(tshapes) )
        {
          copier.Perform( facesItersectors[i]._face );
          facesItersectors[i]._face = TopoDS::Face( copier );
        }
      }
    }
    // Intersection of grid lines with the geometry boundary.
    tbb::parallel_for ( tbb::blocked_range<size_t>( 0, facesItersectors.size() ),
                        ParallelIntersector( facesItersectors ),
                        tbb::simple_partitioner());
#else
    for ( size_t i = 0; i < facesItersectors.size(); ++i )
      facesItersectors[i].Intersect();
#endif

    // put interesection points onto the GridLine's; this is done after intersection
    // to avoid contention of facesItersectors for writing into the same GridLine
    // in case of parallel work of facesItersectors
    for ( size_t i = 0; i < facesItersectors.size(); ++i )
      facesItersectors[i].StoreIntersections();

    SMESH_MesherHelper helper( theMesh );
    TopExp_Explorer solidExp (theShape, TopAbs_SOLID);
    helper.SetSubShape( solidExp.Current() );
    helper.SetElementsOnShape( true );

    if ( _computeCanceled ) return false;

    // create nodes on the geometry
    grid.ComputeNodes(helper);

    if ( _computeCanceled ) return false;

    // create volume elements
    Hexahedron hex( _hyp->GetSizeThreshold(), &grid );
    int nbAdded = hex.MakeElements( helper, edge2faceIDsMap );

    SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();
    if ( nbAdded > 0 )
    {
      // make all SOLIDs computed
      if ( SMESHDS_SubMesh* sm1 = meshDS->MeshElements( solidExp.Current()) )
      {
        SMDS_ElemIteratorPtr volIt = sm1->GetElements();
        for ( ; solidExp.More() && volIt->more(); solidExp.Next() )
        {
          const SMDS_MeshElement* vol = volIt->next();
          sm1->RemoveElement( vol, /*isElemDeleted=*/false );
          meshDS->SetMeshElementOnShape( vol, solidExp.Current() );
        }
      }
      // make other sub-shapes computed
      setSubmeshesComputed( theMesh, theShape );
    }

    // remove free nodes
    if ( SMESHDS_SubMesh * smDS = meshDS->MeshElements( helper.GetSubShapeID() ))
    {
      TIDSortedNodeSet nodesToRemove;
      // get intersection nodes
      for ( int iDir = 0; iDir < 3; ++iDir )
      {
        vector< GridLine >& lines = grid._lines[ iDir ];
        for ( size_t i = 0; i < lines.size(); ++i )
        {
          multiset< F_IntersectPoint >::iterator ip = lines[i]._intPoints.begin();
          for ( ; ip != lines[i]._intPoints.end(); ++ip )
            if ( ip->_node && ip->_node->NbInverseElements() == 0 )
              nodesToRemove.insert( nodesToRemove.end(), ip->_node );
        }
      }
      // get grid nodes
      for ( size_t i = 0; i < grid._nodes.size(); ++i )
        if ( grid._nodes[i] && grid._nodes[i]->NbInverseElements() == 0 )
          nodesToRemove.insert( nodesToRemove.end(), grid._nodes[i] );

      // do remove
      TIDSortedNodeSet::iterator n = nodesToRemove.begin();
      for ( ; n != nodesToRemove.end(); ++n )
        meshDS->RemoveFreeNode( *n, smDS, /*fromGroups=*/false );
    }

    return nbAdded;

  }
  // SMESH_ComputeError is not caught at SMESH_submesh level for an unknown reason
  catch ( SMESH_ComputeError& e)
  {
    return error( SMESH_ComputeErrorPtr( new SMESH_ComputeError( e )));
  }
  return false;
}

//=============================================================================
/*!
 *  Evaluate
 */
//=============================================================================

bool StdMeshers_Cartesian_3D::Evaluate(SMESH_Mesh &         theMesh,
                                       const TopoDS_Shape & theShape,
                                       MapShapeNbElems&     theResMap)
{
  // TODO
//   std::vector<int> aResVec(SMDSEntity_Last);
//   for(int i=SMDSEntity_Node; i<SMDSEntity_Last; i++) aResVec[i] = 0;
//   if(IsQuadratic) {
//     aResVec[SMDSEntity_Quad_Cartesian] = nb2d_face0 * ( nb2d/nb1d );
//     int nb1d_face0_int = ( nb2d_face0*4 - nb1d ) / 2;
//     aResVec[SMDSEntity_Node] = nb0d_face0 * ( 2*nb2d/nb1d - 1 ) - nb1d_face0_int * nb2d/nb1d;
//   }
//   else {
//     aResVec[SMDSEntity_Node] = nb0d_face0 * ( nb2d/nb1d - 1 );
//     aResVec[SMDSEntity_Cartesian] = nb2d_face0 * ( nb2d/nb1d );
//   }
//   SMESH_subMesh * sm = aMesh.GetSubMesh(aShape);
//   aResMap.insert(std::make_pair(sm,aResVec));

  return true;
}

//=============================================================================
namespace
{
  /*!
   * \brief Event listener setting/unsetting _alwaysComputed flag to
   *        submeshes of inferior levels to prevent their computing
   */
  struct _EventListener : public SMESH_subMeshEventListener
  {
    string _algoName;

    _EventListener(const string& algoName):
      SMESH_subMeshEventListener(/*isDeletable=*/true,"StdMeshers_Cartesian_3D::_EventListener"),
      _algoName(algoName)
    {}
    // --------------------------------------------------------------------------------
    // setting/unsetting _alwaysComputed flag to submeshes of inferior levels
    //
    static void setAlwaysComputed( const bool     isComputed,
                                   SMESH_subMesh* subMeshOfSolid)
    {
      SMESH_subMeshIteratorPtr smIt =
        subMeshOfSolid->getDependsOnIterator(/*includeSelf=*/false, /*complexShapeFirst=*/false);
      while ( smIt->more() )
      {
        SMESH_subMesh* sm = smIt->next();
        sm->SetIsAlwaysComputed( isComputed );
      }
      subMeshOfSolid->ComputeStateEngine( SMESH_subMesh::CHECK_COMPUTE_STATE );
    }

    // --------------------------------------------------------------------------------
    // unsetting _alwaysComputed flag if "Cartesian_3D" was removed
    //
    virtual void ProcessEvent(const int          event,
                              const int          eventType,
                              SMESH_subMesh*     subMeshOfSolid,
                              SMESH_subMeshEventListenerData* data,
                              const SMESH_Hypothesis*         hyp = 0)
    {
      if ( eventType == SMESH_subMesh::COMPUTE_EVENT )
      {
        setAlwaysComputed( subMeshOfSolid->GetComputeState() == SMESH_subMesh::COMPUTE_OK,
                           subMeshOfSolid );
      }
      else
      {
        SMESH_Algo* algo3D = subMeshOfSolid->GetAlgo();
        if ( !algo3D || _algoName != algo3D->GetName() )
          setAlwaysComputed( false, subMeshOfSolid );
      }
    }

    // --------------------------------------------------------------------------------
    // set the event listener
    //
    static void SetOn( SMESH_subMesh* subMeshOfSolid, const string& algoName )
    {
      subMeshOfSolid->SetEventListener( new _EventListener( algoName ),
                                        /*data=*/0,
                                        subMeshOfSolid );
    }

  }; // struct _EventListener

} // namespace

//================================================================================
/*!
 * \brief Sets event listener to submeshes if necessary
 *  \param subMesh - submesh where algo is set
 * This method is called when a submesh gets HYP_OK algo_state.
 * After being set, event listener is notified on each event of a submesh.
 */
//================================================================================

void StdMeshers_Cartesian_3D::SetEventListener(SMESH_subMesh* subMesh)
{
  _EventListener::SetOn( subMesh, GetName() );
}

//================================================================================
/*!
 * \brief Set _alwaysComputed flag to submeshes of inferior levels to avoid their computing
 */
//================================================================================

void StdMeshers_Cartesian_3D::setSubmeshesComputed(SMESH_Mesh&         theMesh,
                                                   const TopoDS_Shape& theShape)
{
  for ( TopExp_Explorer soExp( theShape, TopAbs_SOLID ); soExp.More(); soExp.Next() )
    _EventListener::setAlwaysComputed( true, theMesh.GetSubMesh( soExp.Current() ));
}

