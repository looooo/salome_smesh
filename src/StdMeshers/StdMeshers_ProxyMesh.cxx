//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File      : StdMeshers_ProxyMesh.cxx
// Created   : Thu Dec  2 12:32:53 2010
// Author    : Edward AGAPOV (eap)

#include "StdMeshers_ProxyMesh.hxx"

#include "SMDS_IteratorOnIterators.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMESH_MesherHelper.hxx"

#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp_Explorer.hxx>

//================================================================================
/*!
 * \brief Constructor; mesh must be set by a descendant class
 */
//================================================================================

StdMeshers_ProxyMesh::StdMeshers_ProxyMesh():_mesh(0)
{
}
//================================================================================
/*!
 * \brief Make a proxy mesh from components. Components become empty
 */
//================================================================================

StdMeshers_ProxyMesh::StdMeshers_ProxyMesh(vector<StdMeshers_ProxyMesh::Ptr>& components):
  _mesh(0)
{
  if ( components.empty() ) return;

  for ( unsigned i = 0; i < components.size(); ++i )
  {
    StdMeshers_ProxyMesh* m = components[i].get();

    if ( !_mesh ) _mesh = m->_mesh;
    if ( _allowedTypes.empty() ) _allowedTypes = m->_allowedTypes;

    if ( _subMeshes.size() < m->_subMeshes.size() )
      _subMeshes.resize( m->_subMeshes.size(), 0 );
    for ( unsigned j = 0; j < m->_subMeshes.size(); ++j )
    {
      if ( !m->_subMeshes[j] ) continue;
      if ( _subMeshes[j] )
      {
        // unite 2 sub-meshes
        set< const SMDS_MeshElement * > elems( _subMeshes[j]->_elements.begin(),
                                               _subMeshes[j]->_elements.end());
        elems.insert( m->_subMeshes[j]->_elements.begin(),
                      m->_subMeshes[j]->_elements.end());
        _subMeshes[j]->_elements.assign( elems.begin(), elems.end() );

        if ( !_subMeshes[j]->_n2n )
          _subMeshes[j]->_n2n = m->_subMeshes[j]->_n2n, m->_subMeshes[j]->_n2n = 0;

        else if ( _subMeshes[j]->_n2n && m->_subMeshes[j]->_n2n )
          _subMeshes[j]->_n2n->insert( m->_subMeshes[j]->_n2n->begin(),
                                       m->_subMeshes[j]->_n2n->end());
      }
      else
      {
        _subMeshes[j] = m->_subMeshes[j];
        m->_subMeshes[j] = 0;
      }
    }
  }
}

//================================================================================
/*!
 * \brief Destructor deletes proxy submeshes
 */
//================================================================================

StdMeshers_ProxyMesh::~StdMeshers_ProxyMesh()
{
  for ( unsigned i = 0; i < _subMeshes.size(); ++i )
    delete _subMeshes[i];
  _subMeshes.clear();
}

//================================================================================
/*!
 * \brief Returns index of a shape
 */
//================================================================================

int StdMeshers_ProxyMesh::shapeIndex(const TopoDS_Shape& shape) const
{
  return ( shape.IsNull() || !_mesh->HasShapeToMesh() ? 0 : getMeshDS()->ShapeToIndex(shape));
}

//================================================================================
/*!
 * \brief Returns the submesh of a shape; it can be a proxy sub-mesh
 */
//================================================================================

const SMESHDS_SubMesh* StdMeshers_ProxyMesh::GetSubMesh(const TopoDS_Shape& shape) const
{
  const SMESHDS_SubMesh* sm = 0;

  int i = shapeIndex(shape);
  if ( i < _subMeshes.size() )
    sm = _subMeshes[i];
  if ( !sm )
    sm = getMeshDS()->MeshElements( i );

  return sm;
}

//================================================================================
/*!
 * \brief Returns the proxy sub-mesh of a shape; it can be NULL
 */
//================================================================================

const StdMeshers_ProxyMesh::SubMesh*
StdMeshers_ProxyMesh::GetProxySubMesh(const TopoDS_Shape& shape) const
{
  int i = shapeIndex(shape);
  return i < _subMeshes.size() ? _subMeshes[i] : 0;
}

//================================================================================
/*!
 * \brief Returns the proxy node of a node; the input node is returned if no proxy exists
 */
//================================================================================

const SMDS_MeshNode* StdMeshers_ProxyMesh::GetProxyNode( const SMDS_MeshNode* node ) const
{
  const SMDS_MeshNode* proxy = node;
  if ( node->GetPosition()->GetTypeOfPosition() == SMDS_TOP_FACE )
  {
    if ( const SubMesh* proxySM = findProxySubMesh( node->GetPosition()->GetShapeId() ))
      proxy = proxySM->GetProxyNode( node );
  }
  else
  {
    TopoDS_Shape shape = SMESH_MesherHelper::GetSubShapeByNode( node, getMeshDS());
    TopTools_ListIteratorOfListOfShape ancIt;
    if ( !shape.IsNull() ) ancIt.Initialize( _mesh->GetAncestors( shape ));
    for ( ; ancIt.More() && proxy == node; ancIt.Next() )
      if ( const SubMesh* proxySM = findProxySubMesh( shapeIndex(ancIt.Value())))
        proxy = proxySM->GetProxyNode( node );
  }
  return proxy;
}

namespace
{
  //================================================================================
  /*!
   * \brief Iterator filtering elements by type
   */
  //================================================================================

  class TFilteringIterator : public SMDS_ElemIterator
  {
    SMDS_ElemIteratorPtr        _iter;
    const SMDS_MeshElement *    _curElem;
    vector< SMDSAbs_EntityType> _okTypes;
  public:
    TFilteringIterator( const vector< SMDSAbs_EntityType>& okTypes,
                        const SMDS_ElemIteratorPtr&        elemIterator)
      :_iter(elemIterator), _curElem(0), _okTypes(okTypes)
    {
    }
    virtual bool more()
    {
      return _curElem;
    }
    virtual const SMDS_MeshElement* next()
    {
      const SMDS_MeshElement* res = _curElem;
      _curElem = 0;
      while ( _iter->more() && !_curElem )
      {
        _curElem = _iter->next();
        if ( find( _okTypes.begin(), _okTypes.end(), _curElem->GetEntityType()) != _okTypes.end())
          _curElem = 0;
      }
      return res;
    }
  };
}

//================================================================================
/*!
 * \brief Returns iterator on all faces on the shape taking into account substitutions
 */
//================================================================================

SMDS_ElemIteratorPtr StdMeshers_ProxyMesh::GetFaces(const TopoDS_Shape& shape) const
{
  if ( !_mesh->HasShapeToMesh() )
    return SMDS_ElemIteratorPtr();

  _subContainer.RemoveAllSubmeshes();

  TopExp_Explorer fExp( shape, TopAbs_FACE );
  for ( ; fExp.More(); fExp.Next() )
    if ( const SMESHDS_SubMesh* sm = GetSubMesh( fExp.Current()))
      _subContainer.AddSubMesh( sm );
  return _subContainer.SMESHDS_SubMesh::GetElements();
}

//================================================================================
/*!
 * \brief Returns iterator on all faces of the mesh taking into account substitutions
 * To be used in case of mesh without shape
 */
//================================================================================

SMDS_ElemIteratorPtr StdMeshers_ProxyMesh::GetFaces() const
{
  if ( _mesh->HasShapeToMesh() )
    return SMDS_ElemIteratorPtr();

  _subContainer.RemoveAllSubmeshes();
  for ( unsigned i = 0; i < _subMeshes.size(); ++i )
    if ( _subMeshes[i] )
      _subContainer.AddSubMesh( _subMeshes[i] );

  if ( _subContainer.NbSubMeshes() == 0 ) // no elements substituted
    return getMeshDS()->elementsIterator(SMDSAbs_Face);

  // if _allowedTypes is empty, only elements from _subMeshes are returned,...
  SMDS_ElemIteratorPtr proxyIter = _subContainer.GetElements();
  if ( _allowedTypes.empty() || NbFaces() == _mesh->NbFaces() )
    return proxyIter;

  // ... else elements filtered using allowedTypes are additionally returned
  SMDS_ElemIteratorPtr facesIter = getMeshDS()->elementsIterator(SMDSAbs_Face);
  SMDS_ElemIteratorPtr filterIter( new TFilteringIterator( _allowedTypes, facesIter ));
  vector< SMDS_ElemIteratorPtr > iters(2);
  iters[0] = proxyIter;
  iters[1] = filterIter;
    
  typedef vector< SMDS_ElemIteratorPtr > TElemIterVector;
  typedef SMDS_IteratorOnIterators<const SMDS_MeshElement *, TElemIterVector> TItersIter;
  return SMDS_ElemIteratorPtr( new TItersIter( iters ));
}

//================================================================================
/*!
 * \brief Return total nb of faces taking into account substitutions
 */
//================================================================================

int StdMeshers_ProxyMesh::NbFaces() const
{
  // TODO
}

//================================================================================
/*!
 * \brief Returns a proxy sub-mesh; it is created if not yet exists
 */
//================================================================================

StdMeshers_ProxyMesh::SubMesh* StdMeshers_ProxyMesh::getProxySubMesh(int index)
{
  if ( int(_subMeshes.size()) <= index )
    _subMeshes.resize( index+1, 0 );
  if ( !_subMeshes[index] )
    _subMeshes[index] = new SubMesh;
  return _subMeshes[index];
}

//================================================================================
/*!
 * \brief Returns a proxy sub-mesh; it is created if not yet exists
 */
//================================================================================

StdMeshers_ProxyMesh::SubMesh* StdMeshers_ProxyMesh::getProxySubMesh(const TopoDS_Shape& shape)
{
  return getProxySubMesh( shapeIndex( shape ));
}

//================================================================================
/*!
 * \brief Returns a proxy sub-mesh
 */
//================================================================================

StdMeshers_ProxyMesh::SubMesh* StdMeshers_ProxyMesh::findProxySubMesh(int shapeIndex) const
{
  return shapeIndex < int(_subMeshes.size()) ? _subMeshes[shapeIndex] : 0;
}

//================================================================================
/*!
 * \brief Returns mesh DS
 */
//================================================================================

const SMESHDS_Mesh* StdMeshers_ProxyMesh::getMeshDS() const
{
  return _mesh ? _mesh->GetMeshDS() : 0;
}

//================================================================================
/*!
 * \brief Return a proxy node or an input node
 */
//================================================================================

const SMDS_MeshNode* StdMeshers_ProxyMesh::SubMesh::GetProxyNode( const SMDS_MeshNode* n ) const
{
  TN2NMap::iterator n2n;
  if ( _n2n && ( n2n = _n2n->find( n )) != _n2n->end())
    return n2n->second;
  return n;
}

//================================================================================
/*!
 * \brief Deletes temporary elements
 */
//================================================================================

void StdMeshers_ProxyMesh::SubMesh::Clear()
{
  for ( unsigned i = 0; i < _elements.size(); ++i )
    if ( _elements[i]->GetID() > 0 )
      delete _elements[i];
  _elements.clear();
}

//================================================================================
/*!
 * \brief Return number of elements in a proxy submesh
 */
//================================================================================

int StdMeshers_ProxyMesh::SubMesh::NbElements() const
{
  return _elements.size();
}

//================================================================================
/*!
 * \brief Return elements of a proxy submesh
 */
//================================================================================

SMDS_ElemIteratorPtr StdMeshers_ProxyMesh::SubMesh::GetElements() const
{
  return SMDS_ElemIteratorPtr
    ( new SMDS_ElementVectorIterator( _elements.begin(), _elements.end() ));
}

//================================================================================
/*!
 * \brief Store an element
 */
//================================================================================

void StdMeshers_ProxyMesh::SubMesh::AddElement(const SMDS_MeshElement * e)
{
  _elements.push_back( e );
}
