// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
//  File   : SMESHGUI_MeshInfo.cxx
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#include "SMESHGUI_MeshInfo.h"

#include "SMDS_Mesh.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESH_Actor.h"
#include "SMESHGUI.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_IdPreview.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SUIT_FileDlg.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SVTK_ViewWindow.h>
#include <LightApp_SelectionMgr.h>

#include <SALOMEDSClient_Study.hxx>

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextStream>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace
{
  const int SPACING = 6;
  const int MARGIN = 9;

  enum
  {
    GroupsId = 100,
    SubMeshesId
  };

  enum
  {
    TypeRole = Qt::UserRole + 10,
    IdRole,
  };

  enum
  {
    NodeConnectivity = 100,
    ElemConnectivity,
  };

  /*!
    \brief Get item's depth in the tree.
    \param item Tree widget item.
    \return Item's depth in the tree widget (top-level item has zero depth).
    \internal
  */
  int itemDepth( QTreeWidgetItem* item )
  {
    QList<QTreeWidgetItem*> parents;
    parents << item;
    while ( parents.last()->parent() )
      parents << parents.last()->parent();
    return parents.size()-1;
  }

  /*!
    \brief Get chunk size.
    \return Chunk size.
    \internal
  */
  int blockSize()
  {
    return 10;
  }

  /*!
    \brief Get spacer for data information.
    \return Spacer string.
    \internal
  */
  QString spacing()
  {
    static int size = 1;
    static QChar spacer = ' ';
    return QString( size, spacer );
  }

  /*!
    \brief Get indent for data information.
    \param length Indent size. Defaults to 1.
    \return Indentation string.
    \internal
  */
  QString indent( int length = 1 )
  {
    static int size = 4;
    static QChar spacer = ' ';
    return QString( size * length, spacer );
  }

  /*!
    \brief Get group separator for data information.
    \param length Length of ruler (number of symbols). Defaults to 80.
    \return Ruler string.
    \internal
  */
  QString ruler( int length = 80 )
  {
    static QChar ruler = '-';
    return QString( length, ruler );
  }

  /*!
    \brief Get text value from label.
    \param w Widget (QLabel).
    \return Value string.
    \internal
  */
  QString widgetValue( QWidget* w )
  {
    QString v;
    if ( qobject_cast<QLabel*>( w ) )
      v = qobject_cast<QLabel*>( w )->text();
    return v;
  }
} // end of anonymous namespace

////////////////////////////////////////////////////////////////////////////////
/// \class Field
/// \brief Field widget.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class Field : public QLabel
{
public:
  Field( QWidget*, const QString& = QString() );
  bool event( QEvent* );
};

/*!
  \brief Constructor.
  \param parent Parent widget.
  \param name Field name. Defauls to null string.
*/
Field::Field( QWidget* parent, const QString& name ): QLabel( parent )
{
  setFrameStyle( QLabel::StyledPanel | QLabel::Sunken );
  setAlignment( Qt::AlignCenter );
  setAutoFillBackground( true );
  QPalette pal = palette();
  QColor base = QApplication::palette().color( QPalette::Active, QPalette::Base );
  pal.setColor( QPalette::Window, base );
  setPalette( pal );
  setMinimumWidth( 60 );
  if ( !name.isEmpty() )
    setObjectName( name );
}

/*!
  \brief Event handler. Redefined from QLabel.
*/
bool Field::event( QEvent* e )
{
  if ( e->type() == QEvent::DynamicPropertyChange )
  {
    QDynamicPropertyChangeEvent* ce = (QDynamicPropertyChangeEvent*)e;
    if ( ce->propertyName() == "value" && property( "value" ).isValid() )
    {
      setText( QString::number( property( "value" ).toInt() ) );
      setProperty( "value", QVariant() );
      return true;
    }
  }
  return QLabel::event( e );
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_Info
/// \brief Base widget for all information panes.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_Info::SMESHGUI_Info( QWidget* parent ): QWidget( parent )
{
}

/*!
  \brief Get font for given options.
  \param options Font attributes.
  \return Font.
*/
QFont SMESHGUI_Info::fontFromOptions( int options )
{
  QFont f = font();
  f.setBold( options & Bold );
  f.setItalic( options & Italic );
  return f;
}

/*!
  \brief Set font attributes to given widget.
  \param w Widget.
  \param options Font attributes.
*/
void SMESHGUI_Info::setFontAttributes( QWidget* w, int options )
{
  if ( w )
    w->setFont( fontFromOptions( options ) );
}

/*!
  \brief Set font attributes to given tree item.
  \param item Tree widget item.
  \param options Font attributes.
*/
void SMESHGUI_Info::setFontAttributes( QTreeWidgetItem* item, int options )
{
  if ( item && item->treeWidget() )
  {
    for ( int i = 0; i < item->treeWidget()->columnCount(); i++ )
    {
      if ( i == 0 || options & AllColumns )
        item->setFont( i, fontFromOptions( options) );
    }
  }
}

/*!
  \brief Create label.
  \param parent Parent widget.
  \param options Label options. Defaults to 0 (none).
  \return New label.
*/
QLabel* SMESHGUI_Info::createLabel( QWidget* parent, int options )
{
  QLabel* lab = new QLabel( parent );
  setFontAttributes( lab, options );
  return lab;
}

/*!
  \brief Create label.
  \param text Label text.
  \param parent Parent widget.
  \param options Label options. Defaults to 0 (none).
  \return New label.
*/
QLabel* SMESHGUI_Info::createLabel( const QString& text, QWidget* parent, int options )
{
  QLabel* lab = createLabel( parent, options );
  lab->setText( text );
  return lab;
}

/*!
  \brief Create information field.
  \param parent Parent widget.
  \param name Field's object. Default to null string.
  \return New field.
*/
QLabel* SMESHGUI_Info::createField( QWidget* parent, const QString& name )
{
  return new Field( parent, name );
}

/*!
  \brief Create information field.
  \param parent Parent widget.
  \param options Label options.
  \param name Field's object. Default to null string.
  \return New field.
*/
QLabel* SMESHGUI_Info::createField( QWidget* parent, int options, const QString& name )
{
  QLabel* field = createField( parent, name );
  setFontAttributes( field, options );
  return field;
}

/*!
  \brief Create ruler.
  \param parent Parent widget.
  \param orientation Ruler orientation. Defaults to horizontal.
  \return New ruler.
*/
QWidget* SMESHGUI_Info::createSeparator( QWidget* parent, Qt::Orientation orientation )
{
  QFrame* line = new QFrame( parent );
  line->setFrameShape( orientation == Qt::Horizontal ? QFrame::HLine : QFrame::HLine );
  line->setFrameShadow( QFrame::Sunken );
  return line;
}

////////////////////////////////////////////////////////////////////////////////
/// \class ExtraWidget
/// \brief Auxiliary widget to browse between chunks of information.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class ExtraWidget : public QWidget
{
public:
  ExtraWidget( QWidget*, bool = false );
  void updateControls( int, int );

public:
  QLabel* current;
  QPushButton* prev;
  QPushButton* next;
  bool brief;
};

/*
  \brief Constructor.
  \param parent Parent widget.
  \param briefSummary Show summary in brief format. Defaults to \c false.
*/
ExtraWidget::ExtraWidget( QWidget* parent, bool briefSummary ): QWidget( parent ), brief( briefSummary )
{
  current = new QLabel( this );
  current->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  prev = new QPushButton( "<<", this );
  next = new QPushButton( ">>", this );
  QHBoxLayout* hbl = new QHBoxLayout( this );
  hbl->setMargin( 0 );
  hbl->setSpacing( SPACING );
  hbl->addStretch();
  hbl->addWidget( current );
  hbl->addWidget( prev );
  hbl->addWidget( next );
}

/*
  \brief Update controls.
  \param total Total number of items.
  \param index Current index.
*/
void ExtraWidget::updateControls( int total, int index )
{
  setVisible( total > blockSize() );
  QString format = brief ? QString( "%1-%2 / %3" ) : SMESHGUI_MeshInfoDlg::tr( "X_FROM_Y_ITEMS_SHOWN" );
  current->setText( format.arg( index*blockSize()+1 ).arg( qMin( index*blockSize()+blockSize(), total ) ).arg( total ) );
  prev->setEnabled( index > 0 );
  next->setEnabled( (index+1)*blockSize() < total );
}

////////////////////////////////////////////////////////////////////////////////
/// \class DumpFileDlg
/// \brief Save file dialog box, customized for dump info operation.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class DumpFileDlg : public SUIT_FileDlg
{
  QMap<int, QCheckBox*> myControls;
public:
  DumpFileDlg( QWidget*, bool = true );
  bool isChecked( int ) const;
  void setChecked( int, bool );
};

/*!
  \brief Constructor.
  \param parent Parent widget.
  \param showControls Show additional controls. Defaults to \c true.
  \internal
*/
DumpFileDlg::DumpFileDlg( QWidget* parent, bool showControls ): SUIT_FileDlg( parent, false, true, true )
{
  if ( showControls )
  {
    QWidget* hB = new QWidget( this );
    myControls[SMESHGUI_MeshInfoDlg::BaseInfo] = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_BASE_INFO" ), hB );
    myControls[SMESHGUI_MeshInfoDlg::ElemInfo] = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_ELEM_INFO" ), hB );
    myControls[SMESHGUI_MeshInfoDlg::AddInfo] = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_ADD_INFO" ),  hB );
    myControls[SMESHGUI_MeshInfoDlg::CtrlInfo] = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_CTRL_INFO" ), hB );

    QGridLayout* layout = new QGridLayout( hB );
    layout->setMargin( 0 );
    layout->addWidget( myControls[SMESHGUI_MeshInfoDlg::BaseInfo], 0, 0 );
    layout->addWidget( myControls[SMESHGUI_MeshInfoDlg::ElemInfo], 0, 1 );
    layout->addWidget( myControls[SMESHGUI_MeshInfoDlg::AddInfo], 1, 0 );
    layout->addWidget( myControls[SMESHGUI_MeshInfoDlg::CtrlInfo], 1, 1 );

    addWidgets( 0, hB, 0 );
  }
}

/*!
  \brief Get control's value.
  \param option Control identifier.
  \return Control value.
  \internal
*/
bool DumpFileDlg::isChecked( int option ) const
{
  return myControls.contains( option ) ? myControls[option]->isChecked() : false;
}

/*!
  \brief Set control's initial value.
  \param option Control identifier.
  \param value Control value.
  \internal
*/
void DumpFileDlg::setChecked( int option, bool value )
{
  if ( myControls.contains( option ) ) 
    myControls[option]->setChecked( value );
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_BaseInfo
/// \brief Show basic information on selected object.
///
/// Displays the base information about selected object: mesh, sub-mesh, group
/// or arbitrary ID source.
/// \todo Hide unnecessary widgets (e.g. for mesh group).
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_BaseInfo::SMESHGUI_BaseInfo( QWidget* parent ): SMESHGUI_Info( parent )
{
  QGridLayout* l = new QGridLayout( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );

  // object info
  // - name
  addWidget( createLabel( tr( "NAME_LAB" ), this, Bold ), iName, iLabel );
  addWidget( createField( this, "meshName" ), iName, iSingle, 4 )->setMinimumWidth( 150 );
  // - type
  addWidget( createLabel( tr( "OBJECT_LAB" ), this, Bold ), iObject, iLabel );
  addWidget( createField( this, "meshType" ), iObject, iSingle, 4 )->setMinimumWidth( 150 );
  // - --- (separator)
  addWidget( createSeparator( this ), iObjectEnd, iLabel, 5 );

  // node info
  // - info
  addWidget( createLabel( tr( "NODES_LAB" ), this, Bold ), iNodes, iLabel );
  addWidget( createField( this, "nbNodes" ), iNodes, iTotal );
  // - --- (separator)
  addWidget( createSeparator( this ), iNodesEnd, iLabel, 5 );

  // element info
  // - title
  addWidget( createLabel( tr( "ELEMENTS_LAB" ), this, Bold ), iElementsTitle, iLabel );
  addWidget( createLabel( tr( "TOTAL_LAB" ), this, Italic ), iElementsTitle, iTotal );
  addWidget( createLabel( tr( "LINEAR_LAB" ), this, Italic ), iElementsTitle, iLinear );
  addWidget( createLabel( tr( "QUADRATIC_LAB" ), this, Italic ), iElementsTitle, iQuadratic );
  addWidget( createLabel( tr( "BI_QUADRATIC_LAB" ), this, Italic ), iElementsTitle, iBiQuadratic );
  // - --- (separator)
  addWidget( createSeparator( this ), iElementsTitleEnd, iTotal, 4 );
  // - summary
  addWidget( createField( this, "totalNbElems" ), iElementsTotal, iTotal );
  addWidget( createField( this, "totalNbLinearElems" ), iElementsTotal, iLinear );
  addWidget( createField( this, "totalNbQuadraticElems" ), iElementsTotal, iQuadratic );
  addWidget( createField( this, "totalNbBiQuadraticElems" ), iElementsTotal, iBiQuadratic );
  // - --- (separator)
  addWidget( createSeparator( this ), iElementsTotalEnd, iTotal, 4 );
  // - 0D elements info
  addWidget( createLabel( tr( "0D_LAB" ), this, Bold | Italic ), i0D, iLabel );
  addWidget( createField( this, "nb0D" ), i0D, iTotal );
  // - --- (separator)
  addWidget( createSeparator( this ), i0DEnd, iTotal, 4 );
  // - balls info
  addWidget( createLabel( tr( "BALL_LAB" ), this, Bold | Italic ), iBalls, iLabel );
  addWidget( createField( this, "nbBall" ), iBalls, iTotal );
  // - --- (separator)
  addWidget( createSeparator( this ), iBallsEnd, iTotal, 4 );
  // - 1D elements info
  addWidget( createLabel( tr( "1D_LAB" ), this, Bold | Italic ), i1D, iLabel );
  addWidget( createField( this, "nb1D" ), i1D, iTotal );
  addWidget( createField( this, "nbLinear1D" ), i1D, iLinear );
  addWidget( createField( this, "nbQuadratic1D" ), i1D, iQuadratic );
  // - --- (separator)
  addWidget( createSeparator( this ), i1DEnd, iTotal, 4 );
  // - 2D elements info
  // --+ summary
  addWidget( createLabel( tr( "2D_LAB" ), this, Bold | Italic ), i2D, iLabel );
  addWidget( createField( this, "nb2D" ), i2D, iTotal );
  addWidget( createField( this, "nbLinear2D" ), i2D, iLinear );
  addWidget( createField( this, "nbQuadratic2D" ), i2D, iQuadratic );
  addWidget( createField( this, "nbBiQuadratic2D" ), i2D, iBiQuadratic );
  // --+ triangles
  addWidget( createLabel( tr( "TRIANGLES_LAB" ), this, Italic ), i2DTriangles, iLabel );
  addWidget( createField( this, Italic, "nbTriangle" ), i2DTriangles, iTotal );
  addWidget( createField( this, Italic, "nbLinearTriangle" ), i2DTriangles, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticTriangle" ), i2DTriangles, iQuadratic );
  addWidget( createField( this, Italic, "nbBiQuadraticTriangle" ), i2DTriangles, iBiQuadratic );
  // --+ quadrangles
  addWidget( createLabel( tr( "QUADRANGLES_LAB" ), this, Italic ), i2DQuadrangles, iLabel );
  addWidget( createField( this, Italic, "nbQuadrangle" ), i2DQuadrangles, iTotal );
  addWidget( createField( this, Italic, "nbLinearQuadrangle" ), i2DQuadrangles, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticQuadrangle" ), i2DQuadrangles, iQuadratic );
  addWidget( createField( this, Italic, "nbBiQuadraticQuadrangle" ), i2DQuadrangles, iBiQuadratic );
  // --+ polygons
  addWidget( createLabel( tr( "POLYGONS_LAB" ), this, Italic ), i2DPolygons, iLabel );
  addWidget( createField( this, Italic, "nbPolygon" ), i2DPolygons, iTotal );
  addWidget( createField( this, Italic, "nbLinearPolygon" ), i2DPolygons, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticPolygon" ), i2DPolygons, iQuadratic );
  // - --- (separator)
  addWidget( createSeparator( this ), i2DEnd, iTotal, 4 );
  // - 3D elements info
  // --+ summary
  addWidget( createLabel( tr( "3D_LAB" ), this, Bold | Italic ), i3D, iLabel );
  addWidget( createField( this, "nb3D" ), i3D, iTotal );
  addWidget( createField( this, "nbLinear3D" ), i3D, iLinear );
  addWidget( createField( this, "nbQuadratic3D" ), i3D, iQuadratic );
  addWidget( createField( this, "nbBiQuadratic3D" ), i3D, iBiQuadratic );
  // --+ tetras
  addWidget( createLabel( tr( "TETRAHEDRONS_LAB" ), this, Italic ), i3DTetrahedrons, iLabel );
  addWidget( createField( this, Italic, "nbTetrahedron" ), i3DTetrahedrons, iTotal );
  addWidget( createField( this, Italic, "nbLinearTetrahedron" ), i3DTetrahedrons, iLinear );
  addWidget( createField( this, Italic, "nbQudraticTetrahedron" ), i3DTetrahedrons, iQuadratic );
  // --+ hexas
  addWidget( createLabel( tr( "HEXAHEDONRS_LAB" ), this, Italic ), i3DHexahedrons, iLabel );
  addWidget( createField( this, Italic, "nbHexahedron" ), i3DHexahedrons, iTotal );
  addWidget( createField( this, Italic, "nbLinearHexahedron" ), i3DHexahedrons, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticHexahedron" ), i3DHexahedrons, iQuadratic );
  addWidget( createField( this, Italic, "nbBiQuadraticHexahedron" ), i3DHexahedrons, iBiQuadratic );
  // --+ pyramids
  addWidget( createLabel( tr( "PYRAMIDS_LAB" ), this, Italic ), i3DPyramids, iLabel );
  addWidget( createField( this, Italic, "nbPyramid" ), i3DPyramids, iTotal );
  addWidget( createField( this, Italic, "nbLinearPyramid" ), i3DPyramids, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticPyramid" ), i3DPyramids, iQuadratic );
  // --+ prisms
  addWidget( createLabel( tr( "PRISMS_LAB" ), this, Italic ), i3DPrisms, iLabel );
  addWidget( createField( this, Italic, "nbPrism" ), i3DPrisms, iTotal );
  addWidget( createField( this, Italic, "nbLinearPrism" ), i3DPrisms, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticPrism" ), i3DPrisms, iQuadratic );
  addWidget( createField( this, Italic, "nbBiQuadraticPrism" ), i3DPrisms, iBiQuadratic );
  // --+ hexagonal prisms
  addWidget( createLabel( tr( "HEX_PRISMS_LAB" ), this, Italic ), i3DHexaPrisms, iLabel );
  addWidget( createField( this, Italic, "nbHexagonalPrism" ), i3DHexaPrisms, iTotal );
  // --+ polyhedrons
  addWidget( createLabel( tr( "POLYHEDRONS_LAB" ), this, Italic ), i3DPolyhedrons, iLabel );
  addWidget( createField( this, Italic, "nbPolyhedron" ), i3DPolyhedrons, iTotal );

  // load button
  QPushButton* loadBtn = new QPushButton( tr( "BUT_LOAD_MESH" ), this );
  loadBtn->setAutoDefault( true );
  connect( loadBtn, SIGNAL( clicked() ), this, SLOT( loadMesh() ) );
  addWidget( loadBtn, iEnd, iSingle, 4 );

  // additional layout set-up
  l->setColumnStretch( iLabel, 0 );
  l->setColumnStretch( iTotal, 5 );
  l->setColumnStretch( iLinear, 5 );
  l->setColumnStretch( iQuadratic, 5 );
  l->setColumnStretch( iBiQuadratic, 5 );
  l->setRowStretch( iElementsEnd, 5 );

  // set initial values
  clear();
}

/*!
  \brief Destructor.
*/
SMESHGUI_BaseInfo::~SMESHGUI_BaseInfo()
{
}

/*!
  \brief Show information on given object.
  \param proxy Object to show information on (mesh, sub-mesh, group, ID source).
*/
void SMESHGUI_BaseInfo::showInfo( const SMESHGUI_SelectionProxy& proxy )
{
  // reset panel
  clear();

  // then fill panel with data if object is not null
  if ( proxy )
  {
    myProxy = proxy;

    SMESHGUI_MeshInfo info = proxy.meshInfo();

    // object info
    // - name
    widget( iName, iSingle )->setProperty( "text", proxy.name() );
    // - type
    QString typeName;
    SMESHGUI_SelectionProxy::Type type = proxy.type();
    if ( type == SMESHGUI_SelectionProxy::Mesh )
    {
      typeName = tr( "OBJECT_MESH" );
    }
    else if ( type == SMESHGUI_SelectionProxy::Submesh )
    {
      typeName = tr( "OBJECT_SUBMESH" );
    }
    else if ( type >= SMESHGUI_SelectionProxy::Group )
    {
      switch( proxy.elementType() )
      {
      case SMESH::NODE:   typeName = tr( "OBJECT_GROUP_NODES"   ); break;
      case SMESH::EDGE:   typeName = tr( "OBJECT_GROUP_EDGES"   ); break;
      case SMESH::FACE:   typeName = tr( "OBJECT_GROUP_FACES"   ); break;
      case SMESH::VOLUME: typeName = tr( "OBJECT_GROUP_VOLUMES" ); break;
      case SMESH::ELEM0D: typeName = tr( "OBJECT_GROUP_0DELEMS" ); break;
      case SMESH::BALL:   typeName = tr( "OBJECT_GROUP_BALLS"   ); break;
      default:            typeName = tr( "OBJECT_GROUP"         ); break;
      }
      QString subType;
      if ( type == SMESHGUI_SelectionProxy::GroupStd )
        subType = tr( "OBJECT_GROUP_STANDALONE" );
      else if ( type == SMESHGUI_SelectionProxy::GroupGeom )
        subType = tr( "OBJECT_GROUP_GEOM" );
      else if ( type == SMESHGUI_SelectionProxy::GroupFilter )
        subType = tr( "OBJECT_GROUP_FILTER" );
      if ( !subType.isEmpty() )
        typeName = QString( "%1 %2" ).arg( typeName, subType );
    }
    widget( iObject, iSingle )->setProperty( "text", typeName );

    // node info
    uint nbNodes = info[SMDSEntity_Node];
    widget( iNodes, iTotal )->setProperty( "value", nbNodes );

    // element info
    // - 0D elements info
    uint nb0d = info[SMDSEntity_0D];
    widget( i0D, iTotal )->setProperty( "value", nb0d );
    // - balls info
    uint nbBalls = info[SMDSEntity_Ball];
    widget( iBalls, iTotal )->setProperty( "value", nbBalls );
    // - 1D elements info
    uint nb1dLin = info[SMDSEntity_Edge];
    uint nb1dQua = info[SMDSEntity_Quad_Edge];
    uint nb1d = nb1dLin + nb1dQua;
    widget( i1D, iLinear )->setProperty( "value", nb1dLin );
    widget( i1D, iQuadratic )->setProperty( "value", nb1dQua );
    widget( i1D, iTotal )->setProperty( "value", nb1d );
    // - 2D elements info
    // --+ triangles
    uint nbTriLin = info[SMDSEntity_Triangle];
    uint nbTriQua = info[SMDSEntity_Quad_Triangle];
    uint nbTriBiq = info[SMDSEntity_BiQuad_Triangle];
    uint nbTri = nbTriLin + nbTriQua + nbTriBiq;
    widget( i2DTriangles, iLinear )->setProperty( "value", nbTriLin );
    widget( i2DTriangles, iQuadratic )->setProperty( "value", nbTriQua );
    widget( i2DTriangles, iBiQuadratic )->setProperty( "value", nbTriBiq );
    widget( i2DTriangles, iTotal )->setProperty( "value", nbTri );
    // --+ quadrangles
    uint nbQuadLin = info[SMDSEntity_Quadrangle];
    uint nbQuadQua = info[SMDSEntity_Quad_Quadrangle];
    uint nbQuadBiq = info[SMDSEntity_BiQuad_Quadrangle];
    uint nbQuad = nbQuadLin + nbQuadQua + nbQuadBiq;
    widget( i2DQuadrangles, iLinear )->setProperty( "value", nbQuadLin );
    widget( i2DQuadrangles, iQuadratic )->setProperty( "value", nbQuadQua );
    widget( i2DQuadrangles, iBiQuadratic )->setProperty( "value", nbQuadBiq );
    widget( i2DQuadrangles, iTotal )->setProperty( "value", nbQuad );
    // --+ polygons
    uint nbPolyLin = info[SMDSEntity_Polygon];
    uint nbPolyQua = info[SMDSEntity_Quad_Polygon];
    uint nbPoly = nbPolyLin + nbPolyQua;
    widget( i2DPolygons, iLinear )->setProperty( "value", nbPolyLin );
    widget( i2DPolygons, iQuadratic )->setProperty( "value", nbPolyQua );
    widget( i2DPolygons, iTotal )->setProperty( "value", nbPoly );
    // --+ summary
    uint nb2dLin = nbTriLin + nbQuadLin + nbPolyLin;
    uint nb2dQua = nbTriQua + nbQuadQua + nbPolyQua;
    uint nb2dBiq = nbTriBiq + nbQuadBiq;
    uint nb2d = nb2dLin + nb2dQua + nb2dBiq;
    widget( i2D, iLinear )->setProperty( "value", nb2dLin );
    widget( i2D, iQuadratic )->setProperty( "value", nb2dQua );
    widget( i2D, iBiQuadratic )->setProperty( "value", nb2dBiq );
    widget( i2D, iTotal )->setProperty( "value", nb2d );
    // - 3D elements info
    // --+ tetras
    uint nbTetraLin = info[SMDSEntity_Tetra];
    uint nbTetraQua = info[SMDSEntity_Quad_Tetra];
    uint nbTetra = nbTetraLin + nbTetraQua;
    widget( i3DTetrahedrons, iLinear )->setProperty( "value", nbTetraLin );
    widget( i3DTetrahedrons, iQuadratic )->setProperty( "value", nbTetraQua );
    widget( i3DTetrahedrons, iTotal )->setProperty( "value", nbTetra );
    // --+ hexas
    uint nbHexaLin = info[SMDSEntity_Hexa];
    uint nbHexaQua = info[SMDSEntity_Quad_Hexa];
    uint nbHexaBiq = info[SMDSEntity_TriQuad_Hexa];
    uint nbHexa = nbHexaLin + nbHexaQua + nbHexaBiq;
    widget( i3DHexahedrons, iLinear )->setProperty( "value", nbHexaLin );
    widget( i3DHexahedrons, iQuadratic )->setProperty( "value", nbHexaQua );
    widget( i3DHexahedrons, iBiQuadratic )->setProperty( "value", nbHexaBiq );
    widget( i3DHexahedrons, iTotal )->setProperty( "value", nbHexa );
    // --+ pyramids
    uint nbPyraLin = info[SMDSEntity_Pyramid];
    uint nbPyraQua = info[SMDSEntity_Quad_Pyramid];
    uint nbPyra = nbPyraLin + nbPyraQua;
    widget( i3DPyramids, iLinear )->setProperty( "value", nbPyraLin );
    widget( i3DPyramids, iQuadratic )->setProperty( "value", nbPyraQua );
    widget( i3DPyramids, iTotal )->setProperty( "value", nbPyra );
    // --+ prisms
    uint nbPentaLin = info[SMDSEntity_Penta];
    uint nbPentaQua = info[SMDSEntity_Quad_Penta];
    uint nbPentaBiq = info[SMDSEntity_BiQuad_Penta];
    uint nbPenta = nbPentaLin + nbPentaQua + nbPentaBiq;
    widget( i3DPrisms, iLinear )->setProperty( "value", nbPentaLin );
    widget( i3DPrisms, iQuadratic )->setProperty( "value", nbPentaQua );
    widget( i3DPrisms, iBiQuadratic )->setProperty( "value", nbPentaBiq );
    widget( i3DPrisms, iTotal )->setProperty( "value", nbPenta );
    // --+ hexagonal prisms
    uint nbHexaPri = info[SMDSEntity_Hexagonal_Prism];
    widget( i3DHexaPrisms, iTotal )->setProperty( "value", nbHexaPri );
    // --+ polyhedrons
    uint nbPolyhedra = info[SMDSEntity_Polyhedra];
    widget( i3DPolyhedrons, iTotal )->setProperty( "value", nbPolyhedra );
    // --+ summary
    uint nb3dLin = nbTetraLin + nbHexaLin + nbPyraLin + nbPentaLin + nbHexaPri + nbPolyhedra;
    uint nb3dQua = nbTetraQua + nbHexaQua + nbPyraQua + nbPentaQua;
    uint nb3dBiq = nbHexaBiq + nbPentaBiq;
    uint nb3d = nb3dLin + nb3dQua + nb3dBiq;
    widget( i3D, iLinear )->setProperty( "value", nb3dLin );
    widget( i3D, iQuadratic )->setProperty( "value", nb3dQua );
    widget( i3D, iBiQuadratic )->setProperty( "value", nb3dBiq );
    widget( i3D, iTotal )->setProperty( "value", nb3d );
    // - summary
    uint nbElemLin = nb1dLin + nb2dLin + nb3dLin;
    uint nbElemQua = nb1dQua + nb2dQua + nb3dQua;
    uint nbElemBiq = nb2dBiq + nb3dBiq;
    uint nbElem = nb0d + nbBalls + nb1d + nb2d + nb3d;
    widget( iElementsTotal, iLinear )->setProperty( "value", nbElemLin );
    widget( iElementsTotal, iQuadratic )->setProperty( "value", nbElemQua );
    widget( iElementsTotal, iBiQuadratic )->setProperty( "value", nbElemBiq );
    widget( iElementsTotal, iTotal )->setProperty( "value", nbElem );

    // show 'Load' button if data was not loaded yet
    widget( iEnd, iSingle )->setVisible( !proxy.isValid() );

    // until data is loaded from study file, type of elements in a sub-mesh or group
    // can be undefined in some cases
    if ( !proxy.isValid() )
    {
      // two cases are possible:
      // 1. type of 2D or 3D elements is unknown but their nb is OK (for a sub-mesh)
      // 2. there is no info at all (for a group on geom or on filter)
      if ( info.count( SMDSEntity_Node, SMDSEntity_Last ) > 0 ) // believe it is a sub-mesh
      {
        if ( nb2dLin + nb2dQua + nb2dBiq > 0 )
        {
          // we know nothing about triangles, quadranges and polygons
          for ( int i = i2DTriangles; i < i2DEnd; i++ )
          {
            for ( int j = iTotal; j < iNbColumns; j++ )
            {
              if ( widget( i, j ) )
                widget( i, j )->setProperty( "text", "?" );
            }
          }
          // we don't know if elements are linear, quadratic or bi-quadratic
          for ( int j = iLinear; j < iNbColumns; j++ )
          {
            if ( widget( i2D, j ) )
              widget( i2D, j )->setProperty( "text", "?" );
            if ( widget( iElementsTotal, j ) )
              widget( iElementsTotal, j )->setProperty( "text", "?" );
          }
        }
        else if ( nb3dLin + nb3dQua + nb3dBiq > 0 )
        {
          // we know nothing about tetras, hexas, etc.
          for ( int i = i3DTetrahedrons; i < i3DEnd; i++ )
          {
            for ( int j = iTotal; j < iNbColumns; j++ )
            {
              if ( widget( i, j ) )
                widget( i, j )->setProperty( "text", "?" );
            }
          }
          // we don't know if elements are linear, quadratic or bi-quadratic
          for ( int j = iLinear; j < iNbColumns; j++ )
          {
            if ( widget( i3D, j ) )
              widget( i3D, j )->setProperty( "text", "?" );
            if ( widget( iElementsTotal, j ) )
              widget( iElementsTotal, j )->setProperty( "text", "?" );
          }
        }
      }
      else
      {
        // we know nothing about nodes :(
        widget( iNodes, iTotal )->setProperty( "text", "?" );
        // we know nothing about elements :(
        for ( int i = iElementsTotal; i < iElementsEnd; i++ )
        {
          for ( int j = iTotal; j < iNbColumns; j++ )
          {
            if ( widget( i, j ) )
              widget( i, j )->setProperty( "text", "?" );
          }
        }
      }
    }
  }
}

/*!
  \brief Update information in panel.
*/
void SMESHGUI_BaseInfo::updateInfo()
{
  showInfo( myProxy );
}

/*!
  \brief Load mesh from a study file.
*/
void SMESHGUI_BaseInfo::loadMesh()
{
  SUIT_OverrideCursor wc;
  if ( myProxy )
  {
    myProxy.load();
    updateInfo();
  }
}

/*!
  \brief Reset panel (clear all data).
*/
void SMESHGUI_BaseInfo::clear()
{
  // - name
  widget( iName, iSingle )->setProperty( "text", QString() );
  // - type
  widget( iObject, iSingle )->setProperty( "text", QString() );
  // - nodes
  widget( iNodes, iTotal )->setProperty( "value", 0 );
  // - elements
  for ( int i = iElementsTotal; i < iElementsEnd; i++ )
  {
    for ( int j = iTotal; j < iNbColumns; j++ )
    {
      if ( widget( i, j ) )
        widget( i, j )->setProperty( "value", 0 );
    }
  }
}

/*!
  \brief Register widget in a grid.
  \param w Widget being added.
  \param row Row index in a grid.
  \param column Column index in a grid.
  \param colspan Number of columns to span in a grid. Defaults to 1.
  \return Just added widget.
*/
QWidget* SMESHGUI_BaseInfo::addWidget( QWidget* w, int row, int column, int colspan )
{
  if ( !myWidgets.contains( row ) )
    myWidgets[row] = wlist();
  myWidgets[row][column] = w;
  dynamic_cast<QGridLayout*>( layout() )->addWidget( w, row, column, 1, colspan );
  return w;
}

/*!
  \brief Get registered widget.
  \param row Row index in a grid.
  \param column Column index in a grid.
  \return Widget stored in a given grid cell (0 if there's no widget).
*/
QWidget* SMESHGUI_BaseInfo::widget( int row, int column ) const
{
  return myWidgets.contains( row ) && myWidgets[row].contains( column ) ? myWidgets[row][column] : 0;
}

/*!
  \brief Get text value from registered widget.
  \param row Row index in a grid.
  \param column Column index in a grid.
  \return Value string (empty string if no label in given cell).
*/
QString SMESHGUI_BaseInfo::value( int row, int column ) const
{
  return widgetValue( widget( row, column ) );
}

/*!
  \brief Show/hide group(s) of widgets.
  \param startRow Starting grid row.
  \param lastRow Last grid row.
  \param on Visibility flag.
*/
void SMESHGUI_BaseInfo::setFieldsVisible( int startRow, int lastRow, bool on )
{
  startRow = qMax( 0, startRow );
  lastRow = qMin( lastRow, (int)iEnd );
  for ( int i = startRow; i <= lastRow; i++ )
  {
    wlist widgets = myWidgets[i];
    foreach ( QWidget* w, widgets )
      w->setVisible( on );
  }
}

/*!
  \brief Write information from panel to ouput stream.
  \param out Text stream output.
*/
void SMESHGUI_BaseInfo::saveInfo( QTextStream& out )
{
  // title
  QString title = tr( "BASE_INFO" );
  out << ruler( title.size() ) << endl;
  out << title << endl;
  out << ruler( title.size() ) << endl;
  out << endl;

  // object info
  // - name
  out << tr( "NAME_LAB" ) << spacing() << value( iName, iSingle ) << endl;
  // - type
  out << tr( "OBJECT_LAB" ) << spacing() << value( iObject, iSingle ) << endl;
  // - --- (separator)
  out << endl;

  // node info
  out << tr( "NODES_LAB" ) << spacing() << value( iNodes, iTotal ) << endl;
  // - --- (separator)
  out << endl;

  // element info
  QString lin = tr( "LINEAR_LAB" ) + ":" + spacing();
  QString qua = tr( "QUADRATIC_LAB" ) + ":" + spacing();
  QString biq = tr( "BI_QUADRATIC_LAB" ) + ":" + spacing();
  // - summary
  out << tr( "ELEMENTS_LAB" ) << spacing() << value( iElementsTotal, iTotal ) << endl;
  out << indent(1) << lin << value( iElementsTotal, iLinear ) << endl;
  out << indent(1) << qua << value( iElementsTotal, iQuadratic ) << endl;
  out << indent(1) << biq << value( iElementsTotal, iBiQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // - 0D elements info
  out << indent(1) << tr( "0D_LAB" ) << spacing() << value( i0D, iTotal ) << endl;
  // - --- (separator)
  out << endl;
  // - balls info
  out << indent(1) << tr( "BALL_LAB" ) << spacing() << value( iBalls, iTotal ) << endl;
  // - --- (separator)
  out << endl;
  // - 1D elements info
  out << indent(1) << tr( "1D_LAB" ) << spacing() << value( i1D, iTotal ) << endl;
  out << indent(2) << lin << value( i1D, iLinear ) << endl;
  out << indent(2) << qua << value( i1D, iQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // - 2D elements info
  // - summary
  out << indent(1) << tr( "2D_LAB" ) << spacing() << value( i2D, iTotal ) << endl;
  out << indent(2) << lin << value( i2D, iLinear ) << endl;
  out << indent(2) << qua << value( i2D, iQuadratic ) << endl;
  out << indent(2) << biq << value( i2D, iBiQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // --+ triangles
  out << indent(2) << tr( "TRIANGLES_LAB" ) << spacing() << value( i2DTriangles, iTotal ) << endl;
  out << indent(3) << lin << value( i2DTriangles, iLinear ) << endl;
  out << indent(3) << qua << value( i2DTriangles, iQuadratic ) << endl;
  out << indent(3) << biq << value( i2DTriangles, iBiQuadratic ) << endl;
  // --+ quadrangles
  out << indent(2) << tr( "QUADRANGLES_LAB" ) << spacing() << value( i2DQuadrangles, iTotal ) << endl;
  out << indent(3) << lin << value( i2DQuadrangles, iLinear ) << endl;
  out << indent(3) << qua << value( i2DQuadrangles, iQuadratic ) << endl;
  out << indent(3) << biq << value( i2DQuadrangles, iBiQuadratic ) << endl;
  // --+ polygons
  out << indent(2) << tr( "POLYGONS_LAB" ) << spacing() << value( i2DPolygons, iTotal ) << endl;
  out << indent(3) << lin << value( i2DPolygons, iLinear ) << endl;
  out << indent(3) << qua << value( i2DPolygons, iQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // - 3D elements info
  // --+ summary
  out << indent(1) << tr( "3D_LAB" ) << spacing() << value( i3D, iTotal ) << endl;
  out << indent(2) << lin << value( i3D, iLinear ) << endl;
  out << indent(2) << qua << value( i3D, iQuadratic ) << endl;
  out << indent(2) << biq << value( i3D, iBiQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // --+ tetras
  out << indent(2) << tr( "TETRAHEDRONS_LAB" ) << spacing() << value( i3DTetrahedrons, iTotal ) << endl;
  out << indent(3) << lin << value( i3DTetrahedrons, iLinear ) << endl;
  out << indent(3) << qua << value( i3DTetrahedrons, iQuadratic ) << endl;
  // --+ hexas
  out << indent(2) << tr( "HEXAHEDONRS_LAB" ) << spacing() << value( i3DHexahedrons, iTotal ) << endl;
  out << indent(3) << lin << value( i3DHexahedrons, iLinear ) << endl;
  out << indent(3) << qua << value( i3DHexahedrons, iQuadratic ) << endl;
  out << indent(3) << biq << value( i3DHexahedrons, iBiQuadratic ) << endl;
  // --+ pyramids
  out << indent(2) << tr( "PYRAMIDS_LAB" ) << spacing() << value( i3DPyramids, iTotal ) << endl;
  out << indent(3) << lin << value( i3DPyramids, iLinear ) << endl;
  out << indent(3) << qua << value( i3DPyramids, iQuadratic ) << endl;
  // --+ prisms
  out << indent(2) << tr( "PRISMS_LAB" ) << spacing() << value( i3DPrisms, iTotal ) << endl;
  out << indent(3) << lin << value( i3DPrisms, iLinear ) << endl;
  out << indent(3) << qua << value( i3DPrisms, iQuadratic ) << endl;
  out << indent(3) << biq << value( i3DPrisms, iBiQuadratic ) << endl;
  // --+ hexagonal prisms
  out << indent(2) << tr( "HEX_PRISMS_LAB" ) << spacing() << value( i3DHexaPrisms, iTotal ) << endl;
  // --+ polyhedrons
  out << indent(2) << tr( "POLYHEDRONS_LAB" ) << spacing() << value( i3DPolyhedrons, iTotal ) << endl;
  // - --- (separator)
  out << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_ElemInfo
/// \brief Base class for the mesh element information widget.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_ElemInfo::SMESHGUI_ElemInfo( QWidget* parent ): SMESHGUI_Info( parent ), myWhat( ShowNone )
{
  myFrame = new QWidget( this );
  myExtra = new ExtraWidget( this );

  QVBoxLayout* vbl = new QVBoxLayout( this );
  vbl->setMargin( 0 );
  vbl->setSpacing( SPACING );
  vbl->addWidget( myFrame );
  vbl->addWidget( myExtra );

  connect( myExtra->prev, SIGNAL( clicked() ), this, SLOT( showPrevious() ) );
  connect( myExtra->next, SIGNAL( clicked() ), this, SLOT( showNext() ) );

  clear();
}

/*!
  \brief Destructor.
*/
SMESHGUI_ElemInfo::~SMESHGUI_ElemInfo()
{
}

/*!
  \brief Show information on given node / element.
  \param proxy Object to compute information on (mesh, sub-mesh, group, ID source).
  \param id Mesh node / element ID.
  \param isElement If \c true, show element info; otherwise show node info.
*/
void SMESHGUI_ElemInfo::showInfo( const SMESHGUI_SelectionProxy& proxy, uint id, bool isElement )
{
  QSet<uint> ids;
  ids << id;
  showInfo( proxy, ids, isElement );
}

/*!
  \brief Show information on given nodes / elements.
  \param proxy Object to compute information on (mesh, sub-mesh, group, ID source).
  \param ids Mesh nodes / elements IDs.
  \param isElement If \c true, show element info; otherwise show node info.
*/
void SMESHGUI_ElemInfo::showInfo( const SMESHGUI_SelectionProxy& proxy, QSet<uint> ids, bool isElement )
{
  if ( !proxy )
  {
    clear();
    return;
  }

  QList<uint> newIds = ids.toList();
  qSort( newIds );
  int what = isElement ? ShowElements : ShowNodes;
  
  if ( myProxy == proxy && myIDs == newIds && myWhat == what )
    return;
  
  myProxy = proxy;
  myProxy.refresh(); // try to re-initialize actor

  clear();

  myIDs = newIds;
  myWhat = what;
  myIndex = 0;
  
  updateControls();
  information( myIDs.mid( myIndex*blockSize(), blockSize() ));
}

/*!
  \brief Show information on given group.
  \param proxy Object to compute information on (group).
*/
void SMESHGUI_ElemInfo::showInfo( const SMESHGUI_SelectionProxy& proxy )
{
  if ( !proxy || proxy.elementType() == SMESH::ALL ) // null proxy or not a group
  {
    clear();
    return;
  }

  showInfo( proxy, proxy.ids(), proxy.elementType() != SMESH::NODE );
}

/*!
  \brief Clear mesh element information widget
*/
void SMESHGUI_ElemInfo::clear()
{
  myIDs.clear();
  myIndex = 0;
  clearInternal();
  updateControls();
}

/*!
  \brief Get central area widget
  \return central widget
*/
QWidget* SMESHGUI_ElemInfo::frame() const
{
  return myFrame;
}

/*!
  \brief Get current info mode.
  \return Current panel mode.
*/
int SMESHGUI_ElemInfo::what() const
{
  return myWhat;
}

/*!
  \brief Get current mesh proxy object information is shown on.
  \return Current proxy.
*/
SMESHGUI_SelectionProxy SMESHGUI_ElemInfo::proxy() const
{
  return myProxy;
}

/*!
  \fn void SMESHGUI_ElemInfo::information( const QList<uint>& ids )
  \brief Show information on the specified nodes / elements

  This function is to be redefined in sub-classes.

  \param ids nodes / elements identifiers information is to be shown on
*/

/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_ElemInfo::clearInternal()
{
}

/*!
  \brief Get node connectivity
  \param node mesh node
  \return node connectivity map
*/
SMESHGUI_ElemInfo::Connectivity SMESHGUI_ElemInfo::nodeConnectivity( const SMDS_MeshNode* node )
{
  Connectivity elmap;
  if ( node ) {
    SMDS_ElemIteratorPtr it = node->GetInverseElementIterator();
    while ( it && it->more() ) {
      const SMDS_MeshElement* ne = it->next();
      elmap[ ne->GetType() ] << ne->GetID();
    }
  }
  return elmap;
}

/*!
  \brief Format connectivity data to string representation
  \param connectivity connetivity map
  \param type element type
  \return string representation of the connectivity
*/
QString SMESHGUI_ElemInfo::formatConnectivity( Connectivity connectivity, int type )
{
  QStringList str;
  if ( connectivity.contains( type )) {
    QList<int> elements = connectivity[ type ];
    qSort( elements );
    foreach( int id, elements )
      str << QString::number( id );
  }
  return str.join( " " );
}

/*!
  \brief Calculate gravity center of the mesh element
  \param element mesh element
*/
SMESHGUI_ElemInfo::XYZ SMESHGUI_ElemInfo::gravityCenter( const SMDS_MeshElement* element )
{
  XYZ xyz;
  if ( element ) {
    SMDS_ElemIteratorPtr nodeIt = element->nodesIterator();
    while ( nodeIt->more() ) {
      const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
      xyz.add( node->X(), node->Y(), node->Z() );
    }
    xyz.divide( element->NbNodes() );
  }
  return xyz;
}

/*!
  \brief Calculate gravity center of the mesh element
  \param element mesh element
*/
gp_XYZ SMESHGUI_ElemInfo::getGravityCenter( const SMDS_MeshElement* element )
{
  return gravityCenter( element );
}

/*!
  \brief Calculate normal vector to the mesh face
  \param element mesh face
*/
SMESHGUI_ElemInfo::XYZ SMESHGUI_ElemInfo::normal( const SMDS_MeshElement* element )
{
  gp_XYZ n = SMESH::getNormale( SMDS_Mesh::DownCast<SMDS_MeshFace>( element ));
  return XYZ(n.X(), n.Y(), n.Z());
}

/*!
  \brief This slot is called from "Show Previous" button click.
  Shows information on the previous group of the items.
*/
void SMESHGUI_ElemInfo::showPrevious()
{
  myIndex = qMax( 0, myIndex-1 );
  updateControls();
  information( myIDs.mid( myIndex*blockSize(), blockSize() ));
}

/*!
  \brief This slot is called from "Show Next" button click.
  Shows information on the next group of the items.
*/
void SMESHGUI_ElemInfo::showNext()
{
  myIndex = qMin( myIndex+1, myIDs.count() / blockSize() );
  updateControls();
  information( myIDs.mid( myIndex*blockSize(), blockSize() ));
}

/*!
  \brief Update widgets state
*/
void SMESHGUI_ElemInfo::updateControls()
{
  myExtra->updateControls( myIDs.count(), myIndex );
}

/*!
  \class SMESHGUI_SimpleElemInfo
  \brief Represents mesh element information in the simple text area.
*/

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_SimpleElemInfo::SMESHGUI_SimpleElemInfo( QWidget* parent )
  : SMESHGUI_ElemInfo( parent )
{
  myInfo = new QTextBrowser( frame() );
  QVBoxLayout* l = new QVBoxLayout( frame() );
  l->setMargin( 0 );
  l->addWidget( myInfo );
}

/*!
  \brief Show mesh element information
  \param ids mesh nodes / elements identifiers
*/
void SMESHGUI_SimpleElemInfo::information( const QList<uint>& ids )
{
  clearInternal();

  SMESH_Actor* actor = proxy().actor();

  if ( actor ) {
    int grp_details = SMESHGUI::resourceMgr()->booleanValue( "SMESH", "elem_info_grp_details", false );
    int precision   = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
    int cprecision = -1;
    if ( SMESHGUI::resourceMgr()->booleanValue( "SMESH", "use_precision", false ))
      cprecision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "controls_precision", -1 );
    foreach ( uint id, ids ) {
      if ( what() == ShowNodes ) {
        //
        // show node info
        //
        const SMDS_MeshNode* node = actor->GetObject()->GetMesh()->FindNode( id );
        if ( !node ) return;

        // node ID
        myInfo->append( QString( "<b>%1 #%2</b>" ).arg( SMESHGUI_ElemInfo::tr( "NODE" )).arg( id ));
        // separator
        myInfo->append( "" );
        // coordinates
        myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( SMESHGUI_ElemInfo::tr( "COORDINATES" )).
                        arg( node->X(), 0, precision > 0 ? 'f' : 'g', qAbs( precision )).
                        arg( node->Y(), 0, precision > 0 ? 'f' : 'g', qAbs( precision )).
                        arg( node->Z(), 0, precision > 0 ? 'f' : 'g', qAbs( precision )) );
        // separator
        myInfo->append( "" );
        // connectivity
        Connectivity connectivity = nodeConnectivity( node );
        if ( !connectivity.isEmpty() ) {
          myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_ElemInfo::tr( "CONNECTIVITY" )) );
          QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "0D_ELEMENTS" )).arg( con ));
          con = formatConnectivity( connectivity, SMDSAbs_Edge );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "EDGES" )).arg( con ));
          con = formatConnectivity( connectivity, SMDSAbs_Ball );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "BALL_ELEMENTS" )).arg( con ));
          con = formatConnectivity( connectivity, SMDSAbs_Face );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "FACES" )).arg( con ));
          con = formatConnectivity( connectivity, SMDSAbs_Volume );
          if ( !con.isEmpty() )
            myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "VOLUMES" )).arg( con ));
        }
        else {
          myInfo->append( QString( "<b>%1</b>" ).arg( SMESHGUI_ElemInfo::tr( "FREE_NODE" )).arg( id ));
        }
        // node position
        SMESH::SMESH_Mesh_ptr aMeshPtr = actor->GetObject()->GetMeshServer();
        if ( !CORBA::is_nil( aMeshPtr )) {
          SMESH::NodePosition_var pos = aMeshPtr->GetNodePosition( id );
          int shapeID = pos->shapeID;
          if ( shapeID > 0 ) {
            QString shapeType;
            double u = 0, v = 0;
            switch ( pos->shapeType ) {
            case GEOM::EDGE:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_EDGE" );
              if ( pos->params.length() == 1 )
                u = pos->params[0];
              break;
            case GEOM::FACE:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_FACE" );
              if ( pos->params.length() == 2 ) {
                u = pos->params[0];
                v = pos->params[1];
              }
              break;
            case GEOM::VERTEX:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_VERTEX" );
              break;
            default:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SOLID" );
              break;
            }
            // separator
            myInfo->append( "" );
            myInfo->append( QString( "<b>%1:" ).arg( SMESHGUI_ElemInfo::tr( "POSITION" )) );
            myInfo->append( QString( "- <b>%1: #%2</b>" ).arg( shapeType ).arg( shapeID ));
            if ( pos->shapeType == GEOM::EDGE || pos->shapeType == GEOM::FACE ) {
              myInfo->append( QString( "- <b>%1: #%2</b>" ).arg( SMESHGUI_ElemInfo::tr( "U_POSITION" )).
                              arg( QString::number( u, precision > 0 ? 'f' : 'g', qAbs( precision )) ));
              if ( pos->shapeType == GEOM::FACE ) {
                myInfo->append( QString( "- <b>%1: #%2</b>" ).arg( SMESHGUI_ElemInfo::tr( "V_POSITION" )).
                                arg( QString::number( v, precision > 0 ? 'f' : 'g', qAbs( precision )) ));
              }
            }
          }
        }
        // groups node belongs to
        SMESH::SMESH_Mesh_ptr aMesh = actor->GetObject()->GetMeshServer();
        if ( !CORBA::is_nil( aMesh )) {
          SMESH::ListOfGroups_var groups = aMesh->GetGroups();
          myInfo->append( "" ); // separator
          bool top_created = false;
          for ( CORBA::ULong i = 0; i < groups->length(); i++ ) {
            SMESH::SMESH_GroupBase_var aGrp = groups[i];
            if ( CORBA::is_nil( aGrp )) continue;
            QString aName = (char*) (CORBA::String_var) aGrp->GetName();
            if ( aGrp->GetType() == SMESH::NODE && !aName.isEmpty() && aGrp->Contains( id )) {
              if ( !top_created ) {
                myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_AddInfo::tr( "GROUPS" )) );
                top_created = true;
              }
              myInfo->append( QString( "+ <b>%1:</b>" ).arg( aName.trimmed() ));
              if ( grp_details ) {
                SMESH::SMESH_Group_var         aStdGroup  = SMESH::SMESH_Group::_narrow( aGrp );
                SMESH::SMESH_GroupOnGeom_var   aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGrp );
                SMESH::SMESH_GroupOnFilter_var aFltGroup  = SMESH::SMESH_GroupOnFilter::_narrow( aGrp );

                // type : group on geometry, standalone group, group on filter
                if ( !CORBA::is_nil( aStdGroup )) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" )).
                                  arg( SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" )) );
                }
                else if ( !CORBA::is_nil( aGeomGroup )) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" )).
                                  arg( SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" )) );
                  GEOM::GEOM_Object_var gobj = aGeomGroup->GetShape();
                  _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
                  if ( sobj ) {
                    myInfo->append( QString( "  - <b>%1:</b> %2: %3" ).arg( SMESHGUI_AddInfo::tr( "TYPE" )).
                                    arg( SMESHGUI_AddInfo::tr( "GEOM_OBJECT" )).arg( sobj->GetName().c_str() ));
                  }
                }
                else if ( !CORBA::is_nil( aFltGroup )) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" )).
                                  arg( SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" )) );
                }

                // size
                myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "SIZE" )).
                                arg( QString::number( aGrp->Size() )) );

                // color
                SALOMEDS::Color color = aGrp->GetColor();
                myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "COLOR" )).
                                arg( QColor( color.R*255., color.G*255., color.B*255. ).name() ));
              }
            }
          }
        }
      }
      else if ( what() == ShowElements ) {
        //
        // show element info
        //
        const SMDS_MeshElement* e = actor->GetObject()->GetMesh()->FindElement( id );
        SMESH::Controls::NumericalFunctorPtr afunctor;
        if ( !e ) return;

        // Element ID && Type
        QString stype;
        switch( e->GetType() ) {
        case SMDSAbs_0DElement:
          stype = SMESHGUI_ElemInfo::tr( "0D_ELEMENT" ); break;
        case SMDSAbs_Ball:
          stype = SMESHGUI_ElemInfo::tr( "BALL" ); break;
        case SMDSAbs_Edge:
          stype = SMESHGUI_ElemInfo::tr( "EDGE" ); break;
        case SMDSAbs_Face:
          stype = SMESHGUI_ElemInfo::tr( "FACE" ); break;
        case SMDSAbs_Volume:
          stype = SMESHGUI_ElemInfo::tr( "VOLUME" ); break;
        default:
          break;
        }
        if ( stype.isEmpty() ) return;
        myInfo->append( QString( "<b>%1 #%2</b>" ).arg( stype ).arg( id ));
        // separator
        myInfo->append( "" );

        // Geometry type
        QString gtype;
        switch( e->GetEntityType() ) {
        case SMDSEntity_Triangle:
        case SMDSEntity_Quad_Triangle:
        case SMDSEntity_BiQuad_Triangle:
          gtype = SMESHGUI_ElemInfo::tr( "TRIANGLE" ); break;
        case SMDSEntity_Quadrangle:
        case SMDSEntity_Quad_Quadrangle:
        case SMDSEntity_BiQuad_Quadrangle:
          gtype = SMESHGUI_ElemInfo::tr( "QUADRANGLE" ); break;
        case SMDSEntity_Polygon:
        case SMDSEntity_Quad_Polygon:
          gtype = SMESHGUI_ElemInfo::tr( "POLYGON" ); break;
        case SMDSEntity_Tetra:
        case SMDSEntity_Quad_Tetra:
          gtype = SMESHGUI_ElemInfo::tr( "TETRAHEDRON" ); break;
        case SMDSEntity_Pyramid:
        case SMDSEntity_Quad_Pyramid:
          gtype = SMESHGUI_ElemInfo::tr( "PYRAMID" ); break;
        case SMDSEntity_Hexa:
        case SMDSEntity_Quad_Hexa:
        case SMDSEntity_TriQuad_Hexa:
          gtype = SMESHGUI_ElemInfo::tr( "HEXAHEDRON" ); break;
        case SMDSEntity_Penta:
        case SMDSEntity_Quad_Penta:
        case SMDSEntity_BiQuad_Penta:
          gtype = SMESHGUI_ElemInfo::tr( "PRISM" ); break;
        case SMDSEntity_Hexagonal_Prism:
          gtype = SMESHGUI_ElemInfo::tr( "HEX_PRISM" ); break;
        case SMDSEntity_Polyhedra:
        case SMDSEntity_Quad_Polyhedra:
          gtype = SMESHGUI_ElemInfo::tr( "POLYHEDRON" ); break;
        default:
          break;
        }
        if ( !gtype.isEmpty() )
          myInfo->append( QString( "<b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "TYPE" )).arg( gtype ));

        // Quadratic flag (any element except 0D)
        if ( e->GetEntityType() > SMDSEntity_0D && e->GetEntityType() < SMDSEntity_Ball ) {
          myInfo->append( QString( "<b>%1?</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "QUADRATIC" )).arg( e->IsQuadratic() ? SMESHGUI_ElemInfo::tr( "YES" ) : SMESHGUI_ElemInfo::tr( "NO" )) );
        }
        if ( const SMDS_BallElement* ball = SMDS_Mesh::DownCast<SMDS_BallElement>( e )) {
          // Ball diameter
          myInfo->append( QString( "<b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "BALL_DIAMETER" )).arg( ball->GetDiameter() ));
        }
        // separator
        myInfo->append( "" );

        // Connectivity
        SMDS_ElemIteratorPtr nodeIt = e->nodesIterator();
        for ( int idx = 1; nodeIt->more(); idx++ ) {
          const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
          // node number and ID
          myInfo->append( QString( "<b>%1 %2/%3</b> - #%4" ).arg( SMESHGUI_ElemInfo::tr( "NODE" )).arg( idx ).arg( e->NbNodes() ).arg( node->GetID() ));
          // node coordinates
          myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( SMESHGUI_ElemInfo::tr( "COORDINATES" )).
                          arg( node->X(), 0, precision > 0 ? 'f' : 'g', qAbs( precision )).
                          arg( node->Y(), 0, precision > 0 ? 'f' : 'g', qAbs( precision )).
                          arg( node->Z(), 0, precision > 0 ? 'f' : 'g', qAbs( precision )) );
          // node connectivity
          Connectivity connectivity = nodeConnectivity( node );
          if ( !connectivity.isEmpty() ) {
            myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_ElemInfo::tr( "CONNECTIVITY" )) );
            QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
            if ( !con.isEmpty() )
              myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "0D_ELEMENTS" )).arg( con ));
            con = formatConnectivity( connectivity, SMDSAbs_Edge );
            if ( !con.isEmpty() )
              myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "EDGES" )).arg( con ));
            con = formatConnectivity( connectivity, SMDSAbs_Face );
            if ( !con.isEmpty() )
              myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "FACES" )).arg( con ));
            con = formatConnectivity( connectivity, SMDSAbs_Volume );
            if ( !con.isEmpty() )
              myInfo->append( QString( "- <b>%1:</b> %2" ).arg( SMESHGUI_ElemInfo::tr( "VOLUMES" )).arg( con ));
          }
          else {
            myInfo->append( QString( "<b>%1</b>" ).arg( SMESHGUI_ElemInfo::tr( "FREE_NODE" )).arg( id ));
          }
        }
        // separator
        myInfo->append( "" );

        // Controls
        myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_ElemInfo::tr( "CONTROLS" )) );
        //Length
        if ( e->GetType() == SMDSAbs_Edge ) {
          afunctor.reset( new SMESH::Controls::Length() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "LENGTH_EDGES" )).arg( afunctor->GetValue( id )) );
        }
        if( e->GetType() == SMDSAbs_Face ) {
          //Area
          afunctor.reset(  new SMESH::Controls::Area() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "AREA_ELEMENTS" )).arg( afunctor->GetValue( id )) );
          //Taper
          afunctor.reset( new SMESH::Controls::Taper() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "TAPER_ELEMENTS" )).arg( afunctor->GetValue( id )) );
          //AspectRatio2D
          afunctor.reset( new SMESH::Controls::AspectRatio() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "ASPECTRATIO_ELEMENTS" )).arg( afunctor->GetValue( id )) );
          //Minimum angle
          afunctor.reset( new SMESH::Controls::MinimumAngle() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "MINIMUMANGLE_ELEMENTS" )).arg( afunctor->GetValue( id )) );
          //Warping angle
          afunctor.reset( new SMESH::Controls::Warping() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "WARP_ELEMENTS" )).arg( afunctor->GetValue( id )) );
          //Skew
          afunctor.reset( new SMESH::Controls::Skew() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "SKEW_ELEMENTS" )).arg( afunctor->GetValue( id )) );
          //ElemDiam2D
          afunctor.reset( new SMESH::Controls::MaxElementLength2D() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "MAX_ELEMENT_LENGTH_2D" )).arg( afunctor->GetValue( id )) );
          //min edge length
          afunctor.reset( new SMESH::Controls::Length2D() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "MIN_ELEM_EDGE" )).arg( afunctor->GetValue( id )) );
        }
        if( e->GetType() == SMDSAbs_Volume ) {
          //AspectRatio3D
          afunctor.reset(  new SMESH::Controls::AspectRatio3D() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "ASPECTRATIO_3D_ELEMENTS" )).arg( afunctor->GetValue( id )) );
          //Volume
          afunctor.reset(  new SMESH::Controls::Volume() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "VOLUME_3D_ELEMENTS" )).arg( afunctor->GetValue( id )) );
          //ElementDiameter3D
          afunctor.reset(  new SMESH::Controls::Volume() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          myInfo->append( QString( "- <b>%1:</b> %2" ).arg( tr( "MAX_ELEMENT_LENGTH_3D" )).arg( afunctor->GetValue( id )) );
        }
        // separator
        myInfo->append( "" );

        // Gravity center
        XYZ gc = gravityCenter( e );
        myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( SMESHGUI_ElemInfo::tr( "GRAVITY_CENTER" )).arg( gc.x() ).arg( gc.y() ).arg( gc.z() ));

        // Normal vector
        if( e->GetType() == SMDSAbs_Face ) {
          XYZ gc = normal( e );
          myInfo->append( QString( "<b>%1:</b> (%2, %3, %4)" ).arg( SMESHGUI_ElemInfo::tr( "NORMAL_VECTOR" )).arg( gc.x() ).arg( gc.y() ).arg( gc.z() ));
        }

        // Element position
        if ( e->GetType() >= SMDSAbs_Edge && e->GetType() <= SMDSAbs_Volume ) {
          SMESH::SMESH_Mesh_ptr aMesh = actor->GetObject()->GetMeshServer();
          if ( !CORBA::is_nil( aMesh )) {
            SMESH::ElementPosition pos = aMesh->GetElementPosition( id );
            int shapeID = pos.shapeID;
            if ( shapeID > 0 ) {
              myInfo->append( "" ); // separator
              QString shapeType;
              switch ( pos.shapeType ) {
              case GEOM::EDGE:   shapeType = SMESHGUI_ElemInfo::tr( "GEOM_EDGE" );   break;
              case GEOM::FACE:   shapeType = SMESHGUI_ElemInfo::tr( "GEOM_FACE" );   break;
              case GEOM::VERTEX: shapeType = SMESHGUI_ElemInfo::tr( "GEOM_VERTEX" ); break;
              case GEOM::SOLID:  shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SOLID" );  break;
              case GEOM::SHELL:  shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SHELL" );  break;
              default:           shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SHAPE" );  break;
              }
              myInfo->append( QString( "<b>%1:</b> %2 #%3" ).arg( SMESHGUI_ElemInfo::tr( "POSITION" )).arg( shapeType ).arg( shapeID ));
            }
          }
        }

        // Groups the element belongs to
        SMESH::SMESH_Mesh_ptr aMesh = actor->GetObject()->GetMeshServer();
        if ( !CORBA::is_nil( aMesh )) {
          SMESH::ListOfGroups_var  groups = aMesh->GetGroups();
          myInfo->append( "" ); // separator
          bool top_created = false;
          for ( CORBA::ULong i = 0; i < groups->length(); i++ ) {
            SMESH::SMESH_GroupBase_var aGrp = groups[i];
            if ( CORBA::is_nil( aGrp )) continue;
            QString aName = (char*) (CORBA::String_var) aGrp->GetName();
            if ( aGrp->GetType() != SMESH::NODE && !aName.isEmpty() && aGrp->Contains( id )) {
              if ( !top_created ) {
                myInfo->append( QString( "<b>%1:</b>" ).arg( SMESHGUI_AddInfo::tr( "GROUPS" )) );
                top_created = true;
              }
              myInfo->append( QString( "+ <b>%1:</b>" ).arg( aName.trimmed() ));
              if ( grp_details ) {
                SMESH::SMESH_Group_var         aStdGroup  = SMESH::SMESH_Group::_narrow( aGrp );
                SMESH::SMESH_GroupOnGeom_var   aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGrp );
                SMESH::SMESH_GroupOnFilter_var aFltGroup  = SMESH::SMESH_GroupOnFilter::_narrow( aGrp );

                // type : group on geometry, standalone group, group on filter
                if ( !CORBA::is_nil( aStdGroup )) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" )).
                                  arg( SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" )) );
                }
                else if ( !CORBA::is_nil( aGeomGroup )) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" )).
                                  arg( SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" )) );
                  GEOM::GEOM_Object_var gobj = aGeomGroup->GetShape();
                  _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
                  if ( sobj ) {
                    myInfo->append( QString( "  - <b>%1:</b> %2: %3" ).arg( SMESHGUI_AddInfo::tr( "TYPE" )).
                                    arg( SMESHGUI_AddInfo::tr( "GEOM_OBJECT" )).arg( sobj->GetName().c_str() ));
                  }
                }
                else if ( !CORBA::is_nil( aFltGroup )) {
                  myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "TYPE" )).
                                  arg( SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" )) );
                }

                myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "SIZE" )).
                                arg( QString::number( aGrp->Size() )) );

                // color
                SALOMEDS::Color color = aGrp->GetColor();
                myInfo->append( QString( "  - <b>%1:</b> %2" ).arg( SMESHGUI_AddInfo::tr( "COLOR" )).
                                arg( QColor( color.R*255., color.G*255., color.B*255. ).name() ));
              }
            }
          }
        }
      }
      // separator
      if ( ids.count() > 1 ) {
        myInfo->append( "" );
        myInfo->append( "------" );
        myInfo->append( "" );
      }
    }
  }
}

/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_SimpleElemInfo::clearInternal()
{
  myInfo->clear();
}

void SMESHGUI_SimpleElemInfo::saveInfo( QTextStream &out )
{
  out << ruler( 12 ) << endl;
  out << SMESHGUI_ElemInfo::tr( "ELEM_INFO" ) << endl;
  out << ruler( 12 ) << endl;
  out << myInfo->toPlainText();
  out << endl;
}


/*!
  \class SMESHGUI_TreeElemInfo::ItemDelegate
  \brief Item delegate for tree mesh info widget
  \internal
*/
class SMESHGUI_TreeElemInfo::ItemDelegate : public QItemDelegate
{
public:
  ItemDelegate( QObject* );
  QWidget* createEditor( QWidget*, const QStyleOptionViewItem&, const QModelIndex& ) const;
};

/*!
  \brief Constructor
  \internal
*/
SMESHGUI_TreeElemInfo::ItemDelegate::ItemDelegate( QObject* parent ): QItemDelegate( parent )
{
}

/*!
  \brief Create item editor widget
  \internal
*/
QWidget* SMESHGUI_TreeElemInfo::ItemDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QWidget* w = index.column() == 0 ? 0: QItemDelegate::createEditor( parent, option, index );
  if ( qobject_cast<QLineEdit*>( w )) qobject_cast<QLineEdit*>( w )->setReadOnly(  true );
  return w;
}

/*!
  \class SMESHGUI_TreeElemInfo
  \brief Represents mesh element information in the tree-like form.
*/

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_TreeElemInfo::SMESHGUI_TreeElemInfo( QWidget* parent )
  : SMESHGUI_ElemInfo( parent )
{
  myInfo = new QTreeWidget( frame() );
  myInfo->setColumnCount( 2 );
  myInfo->setHeaderLabels( QStringList() << tr( "PROPERTY" ) << tr( "VALUE" ) );
  myInfo->header()->setStretchLastSection( true );
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  myInfo->header()->setResizeMode( 0, QHeaderView::ResizeToContents );
#else
  myInfo->header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
#endif
  myInfo->setItemDelegate( new ItemDelegate( myInfo ));
  QVBoxLayout* l = new QVBoxLayout( frame() );
  l->setMargin( 0 );
  l->addWidget( myInfo );
  connect( myInfo, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int )), this, SLOT( itemDoubleClicked( QTreeWidgetItem*, int )) );
  connect( myInfo, SIGNAL( itemCollapsed( QTreeWidgetItem* )), this, SLOT( saveExpanded( QTreeWidgetItem* )) );
  connect( myInfo, SIGNAL( itemExpanded( QTreeWidgetItem* )), this, SLOT( saveExpanded( QTreeWidgetItem* )) );
}

/*!
  \brief Show mesh element information
  \param ids mesh nodes / elements identifiers
*/
void SMESHGUI_TreeElemInfo::information( const QList<uint>& ids )
{
  clearInternal();

  if ( !proxy() )
    return;

  bool hasShapeToMesh = proxy().hasShapeToMesh();
  SMESH_Actor* actor = proxy().actor();

  if ( actor ) {
    int grp_details = SMESHGUI::resourceMgr()->booleanValue( "SMESH", "elem_info_grp_details", false );
    int precision   = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
    int cprecision = -1;
    if ( SMESHGUI::resourceMgr()->booleanValue( "SMESH", "use_precision", false )) 
      cprecision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "controls_precision", -1 );
    foreach ( uint id, ids ) {
      if ( what() == ShowNodes ) {
        //
        // show node info
        //
        const SMDS_MeshElement* e = actor->GetObject()->GetMesh()->FindNode( id );
        if ( !e ) return;
        const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( e );
      
        // node ID
        QTreeWidgetItem* nodeItem = createItem( 0, Bold | AllColumns );
        nodeItem->setText( 0, SMESHGUI_ElemInfo::tr( "NODE" ));
        nodeItem->setText( 1, QString( "#%1" ).arg( id ));
        // coordinates
        QTreeWidgetItem* coordItem = createItem( nodeItem, Bold );
        coordItem->setText( 0, SMESHGUI_ElemInfo::tr( "COORDINATES" ));
        QTreeWidgetItem* xItem = createItem( coordItem );
        xItem->setText( 0, "X" );
        xItem->setText( 1, QString::number( node->X(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
        QTreeWidgetItem* yItem = createItem( coordItem );
        yItem->setText( 0, "Y" );
        yItem->setText( 1, QString::number( node->Y(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
        QTreeWidgetItem* zItem = createItem( coordItem );
        zItem->setText( 0, "Z" );
        zItem->setText( 1, QString::number( node->Z(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
        // connectivity
        QTreeWidgetItem* conItem = createItem( nodeItem, Bold );
        conItem->setText( 0, SMESHGUI_ElemInfo::tr( "CONNECTIVITY" ));
        Connectivity connectivity = nodeConnectivity( node );
        if ( !connectivity.isEmpty() ) {
          QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "0D_ELEMENTS" ));
            i->setText( 1, con );
          }
          con = formatConnectivity( connectivity, SMDSAbs_Ball );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "BALL_ELEMENTS" ));
            i->setText( 1, con );
            i->setData( 1, TypeRole, NodeConnectivity );
          }
          con = formatConnectivity( connectivity, SMDSAbs_Edge );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "EDGES" ));
            i->setText( 1, con );
            i->setData( 1, TypeRole, NodeConnectivity );
          }
          con = formatConnectivity( connectivity, SMDSAbs_Face );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "FACES" ));
            i->setText( 1, con );
            i->setData( 1, TypeRole, NodeConnectivity );
          }
          con = formatConnectivity( connectivity, SMDSAbs_Volume );
          if ( !con.isEmpty() ) {
            QTreeWidgetItem* i = createItem( conItem );
            i->setText( 0, SMESHGUI_ElemInfo::tr( "VOLUMES" ));
            i->setText( 1, con );
            i->setData( 1, TypeRole, NodeConnectivity );
          }
        }
        else {
          conItem->setText( 1, SMESHGUI_ElemInfo::tr( "FREE_NODE" ));
        }
        // node position
        SMESH::SMESH_Mesh_ptr aMeshPtr = actor->GetObject()->GetMeshServer();   
        if ( !CORBA::is_nil( aMeshPtr )) {
          SMESH::NodePosition_var pos = aMeshPtr->GetNodePosition( id );
          int shapeID = pos->shapeID;
          if ( shapeID > 0 ) {
            QString shapeType;
            double u = 0, v = 0;
            switch ( pos->shapeType ) {
            case GEOM::EDGE:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_EDGE" );
              if ( pos->params.length() == 1 )
                u = pos->params[0];
              break;
            case GEOM::FACE:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_FACE" );
              if ( pos->params.length() == 2 ) {
                u = pos->params[0];
                v = pos->params[1];
              }
              break;
            case GEOM::VERTEX:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_VERTEX" );
              break;
            default:
              shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SOLID" );
              break;
            }
            QTreeWidgetItem* posItem = createItem( nodeItem, Bold );
            posItem->setText( 0, SMESHGUI_ElemInfo::tr("POSITION") );
            posItem->setText( 1, (shapeType + " #%1").arg( shapeID ));
            if ( pos->shapeType == GEOM::EDGE || pos->shapeType == GEOM::FACE ) {
              QTreeWidgetItem* uItem = createItem( posItem );
              uItem->setText( 0, SMESHGUI_ElemInfo::tr("U_POSITION") );
              uItem->setText( 1, QString::number( u, precision > 0 ? 'f' : 'g', qAbs( precision )));
              if ( pos->shapeType == GEOM::FACE ) {
                QTreeWidgetItem* vItem = createItem( posItem );
                vItem->setText( 0, SMESHGUI_ElemInfo::tr("V_POSITION") );
                vItem->setText( 1, QString::number( v, precision > 0 ? 'f' : 'g', qAbs( precision )));
              }
            }
          }
        }
        // groups node belongs to
        SMESH::SMESH_Mesh_ptr aMesh = actor->GetObject()->GetMeshServer();
        if ( !CORBA::is_nil( aMesh )) {
          SMESH::ListOfGroups_var groups = aMesh->GetGroups();
          QTreeWidgetItem* groupsItem = 0;
          for ( CORBA::ULong i = 0; i < groups->length(); i++ ) {
            SMESH::SMESH_GroupBase_var aGrp = groups[i];
            if ( CORBA::is_nil( aGrp )) continue;
            QString aName = (char*) (CORBA::String_var) aGrp->GetName();
            if ( aGrp->GetType() == SMESH::NODE && !aName.isEmpty() && aGrp->Contains( id )) {
              if ( !groupsItem ) {
                groupsItem = createItem( nodeItem, Bold );
                groupsItem->setText( 0, SMESHGUI_AddInfo::tr( "GROUPS" ));
              }
              QTreeWidgetItem* it = createItem( groupsItem, Bold );
              it->setText( 0, aName.trimmed() );
              if ( grp_details ) {
                SMESH::SMESH_Group_var         aStdGroup  = SMESH::SMESH_Group::_narrow( aGrp );
                SMESH::SMESH_GroupOnGeom_var   aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGrp );
                SMESH::SMESH_GroupOnFilter_var aFltGroup  = SMESH::SMESH_GroupOnFilter::_narrow( aGrp );
                
                // type : group on geometry, standalone group, group on filter
                QTreeWidgetItem* typeItem = createItem( it );
                typeItem->setText( 0, SMESHGUI_AddInfo::tr( "TYPE" ));
                if ( !CORBA::is_nil( aStdGroup )) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" ));
                }
                else if ( !CORBA::is_nil( aGeomGroup )) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" ));
                  GEOM::GEOM_Object_var gobj = aGeomGroup->GetShape();
                  _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
                  if ( sobj ) {
                    QTreeWidgetItem* gobjItem = createItem( typeItem );
                    gobjItem->setText( 0, SMESHGUI_AddInfo::tr( "GEOM_OBJECT" ));
                    gobjItem->setText( 1, sobj->GetName().c_str() );
                  }
                }
                else if ( !CORBA::is_nil( aFltGroup )) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" ));
                }
                
                // size
                QTreeWidgetItem* sizeItem = createItem( it );
                sizeItem->setText( 0, SMESHGUI_AddInfo::tr( "SIZE" ));
                sizeItem->setText( 1, QString::number( aGrp->Size() ));
                
                // color
                SALOMEDS::Color color = aGrp->GetColor();
                QTreeWidgetItem* colorItem = createItem( it );
                colorItem->setText( 0, SMESHGUI_AddInfo::tr( "COLOR" ));
                colorItem->setBackground( 1, QBrush( QColor( color.R*255., color.G*255., color.B*255.) ));
              }
            }
          }
        }
      }
      if ( what() == ShowElements ) {
        //
        // show element info
        // 
        const SMDS_MeshElement* e = actor->GetObject()->GetMesh()->FindElement( id );
        SMESH::Controls::NumericalFunctorPtr afunctor;
        if ( !e ) return;
        
        // element ID && type
        QString stype;
        switch( e->GetType() ) {
        case SMDSAbs_0DElement: stype = SMESHGUI_ElemInfo::tr( "0D_ELEMENT" ); break;
        case SMDSAbs_Ball:      stype = SMESHGUI_ElemInfo::tr( "BALL" ); break;
        case SMDSAbs_Edge:      stype = SMESHGUI_ElemInfo::tr( "EDGE" ); break;
        case SMDSAbs_Face:      stype = SMESHGUI_ElemInfo::tr( "FACE" ); break;
        case SMDSAbs_Volume:    stype = SMESHGUI_ElemInfo::tr( "VOLUME" ); break;
        default:;
        }
        if ( stype.isEmpty() ) return;
        QTreeWidgetItem* elemItem = createItem( 0, Bold | AllColumns );
        elemItem->setText( 0, stype );
        elemItem->setText( 1, QString( "#%1" ).arg( id ));
        // geometry type
        QString gtype;
        switch( e->GetEntityType() ) {
        case SMDSEntity_Triangle:
        case SMDSEntity_Quad_Triangle:
        case SMDSEntity_BiQuad_Triangle:
          gtype = SMESHGUI_ElemInfo::tr( "TRIANGLE" ); break;
        case SMDSEntity_Quadrangle:
        case SMDSEntity_Quad_Quadrangle:
        case SMDSEntity_BiQuad_Quadrangle:
          gtype = SMESHGUI_ElemInfo::tr( "QUADRANGLE" ); break;
        case SMDSEntity_Polygon:
        case SMDSEntity_Quad_Polygon:
          gtype = SMESHGUI_ElemInfo::tr( "POLYGON" ); break;
        case SMDSEntity_Tetra:
        case SMDSEntity_Quad_Tetra:
          gtype = SMESHGUI_ElemInfo::tr( "TETRAHEDRON" ); break;
        case SMDSEntity_Pyramid:
        case SMDSEntity_Quad_Pyramid:
          gtype = SMESHGUI_ElemInfo::tr( "PYRAMID" ); break;
        case SMDSEntity_Hexa:
        case SMDSEntity_Quad_Hexa:
        case SMDSEntity_TriQuad_Hexa:
          gtype = SMESHGUI_ElemInfo::tr( "HEXAHEDRON" ); break;
        case SMDSEntity_Penta:
        case SMDSEntity_Quad_Penta:
        case SMDSEntity_BiQuad_Penta:
          gtype = SMESHGUI_ElemInfo::tr( "PRISM" ); break;
        case SMDSEntity_Hexagonal_Prism:
          gtype = SMESHGUI_ElemInfo::tr( "HEX_PRISM" ); break;
        case SMDSEntity_Polyhedra:
        case SMDSEntity_Quad_Polyhedra:
          gtype = SMESHGUI_ElemInfo::tr( "POLYHEDRON" ); break;
        default:
          break;
        }
        if ( !gtype.isEmpty() ) {
          QTreeWidgetItem* typeItem = createItem( elemItem, Bold );
          typeItem->setText( 0, SMESHGUI_ElemInfo::tr( "TYPE" ));
          typeItem->setText( 1, gtype );
        }
        // quadratic flag (for edges, faces and volumes)
        if ( e->GetType() >= SMDSAbs_Edge && e->GetType() <= SMDSAbs_Volume ) {
          // quadratic flag
          QTreeWidgetItem* quadItem = createItem( elemItem, Bold );
          quadItem->setText( 0, SMESHGUI_ElemInfo::tr( "QUADRATIC" ));
          quadItem->setText( 1, e->IsQuadratic() ? SMESHGUI_ElemInfo::tr( "YES" ) : SMESHGUI_ElemInfo::tr( "NO" ));
        }
        if ( const SMDS_BallElement* ball = SMDS_Mesh::DownCast< SMDS_BallElement >( e )) {
          // ball diameter
          QTreeWidgetItem* diamItem = createItem( elemItem, Bold );
          diamItem->setText( 0, SMESHGUI_ElemInfo::tr( "BALL_DIAMETER" ));
          diamItem->setText( 1, QString( "%1" ).arg( ball->GetDiameter() ));
        }
        // connectivity
        QTreeWidgetItem* conItem = createItem( elemItem, Bold );
        conItem->setText( 0, SMESHGUI_ElemInfo::tr( "CONNECTIVITY" ));


        if( e->GetGeomType() != SMDSGeom_POLYHEDRA ) {
          SMDS_ElemIteratorPtr nodeIt = e->nodesIterator();
          for ( int idx = 1; nodeIt->more(); idx++ ) {
            const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
            nodeInfo( node, idx, e->NbNodes(), conItem );
          }
        }
        else {
          SMDS_NodeIteratorPtr nodeIt = e->nodeIterator();
          std::set< const SMDS_MeshNode* > addedNodes;
          QList<const SMDS_MeshElement*> uniqueNodes;
          while ( nodeIt->more() ) {
            const SMDS_MeshNode* node = nodeIt->next();
            if ( addedNodes.insert( node ).second )
              uniqueNodes.append( nodeIt->next() );
          }
          SMDS_VolumeTool vtool( e );
          const int nbFaces = vtool.NbFaces();
          for( int face_id = 0; face_id < nbFaces; face_id++ ) {
            QTreeWidgetItem* faceItem = createItem( conItem, Bold );
            faceItem->setText( 0, QString( "%1 %2 / %3" ).arg( SMESHGUI_ElemInfo::tr( "FACE" )).arg( face_id + 1 ).arg( nbFaces ));
            faceItem->setExpanded( true );

            const SMDS_MeshNode** aNodeIds = vtool.GetFaceNodes( face_id );
            const int              nbNodes = vtool.NbFaceNodes ( face_id );
            for( int node_id = 0; node_id < nbNodes; node_id++ ) {
              const SMDS_MeshNode* node = aNodeIds[node_id];
              nodeInfo( node, uniqueNodes.indexOf(node) + 1, uniqueNodes.size(), faceItem );
            }
          }
        }
        //Controls
        QTreeWidgetItem* cntrItem = createItem( elemItem, Bold );
        cntrItem->setText( 0, SMESHGUI_ElemInfo::tr( "CONTROLS" ));
        //Length
        if( e->GetType()==SMDSAbs_Edge){
          afunctor.reset( new SMESH::Controls::Length() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          QTreeWidgetItem* lenItem = createItem( cntrItem, Bold );
          lenItem->setText( 0, tr( "LENGTH_EDGES" ));
          lenItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
        }
        if( e->GetType() == SMDSAbs_Face ) {
          //Area
          afunctor.reset( new SMESH::Controls::Area() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          QTreeWidgetItem* areaItem = createItem( cntrItem, Bold );
          areaItem->setText( 0, tr( "AREA_ELEMENTS" ));
          areaItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue(id) ));
          //Taper
          if ( e->NbNodes() == 4 ) // see SMESH_Controls.cxx
          {
            afunctor.reset( new SMESH::Controls::Taper() );
            afunctor->SetMesh( actor->GetObject()->GetMesh() );
            afunctor->SetPrecision( cprecision );
            QTreeWidgetItem* taperlItem = createItem( cntrItem, Bold );
            taperlItem->setText( 0, tr( "TAPER_ELEMENTS" ));
            taperlItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
            //Warping angle
            afunctor.reset( new SMESH::Controls::Warping() );
            afunctor->SetMesh( actor->GetObject()->GetMesh() );
            afunctor->SetPrecision( cprecision );
            QTreeWidgetItem* warpItem = createItem( cntrItem, Bold );
            warpItem->setText( 0, tr( "WARP_ELEMENTS" ));
            warpItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
          }
          //AspectRatio2D
          if ( !e->IsPoly() )
          {
            afunctor.reset( new SMESH::Controls::AspectRatio() );
            afunctor->SetMesh( actor->GetObject()->GetMesh() );
            QTreeWidgetItem* ratlItem = createItem( cntrItem, Bold );
            ratlItem->setText( 0, tr( "ASPECTRATIO_ELEMENTS" ));
            ratlItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
          }
          //Minimum angle
          afunctor.reset( new SMESH::Controls::MinimumAngle() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          afunctor->SetPrecision( cprecision );
          QTreeWidgetItem* minanglItem = createItem( cntrItem, Bold );
          minanglItem->setText( 0, tr( "MINIMUMANGLE_ELEMENTS" ));
          minanglItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
          //Skew
          if ( e->NbNodes() == 3 || e->NbNodes() == 4 )
          {
            afunctor.reset( new SMESH::Controls::Skew() );
            afunctor->SetMesh( actor->GetObject()->GetMesh() );
            afunctor->SetPrecision( cprecision );
            QTreeWidgetItem* skewItem = createItem( cntrItem, Bold );
            skewItem->setText( 0, tr( "SKEW_ELEMENTS" ));
            skewItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
          }
          //Deflection
          if ( hasShapeToMesh )
          {
            afunctor.reset( new SMESH::Controls::Deflection2D() );
            afunctor->SetMesh( actor->GetObject()->GetMesh() );
            QTreeWidgetItem* deflItem = createItem( cntrItem, Bold );
            deflItem->setText( 0, tr( "DEFLECTION_2D" ));
            deflItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
          }
          //ElemDiam2D
          if ( !e->IsPoly() )
          {
            afunctor.reset( new SMESH::Controls::MaxElementLength2D() );
            afunctor->SetMesh( actor->GetObject()->GetMesh() );
            QTreeWidgetItem* diamItem = createItem( cntrItem, Bold );
            diamItem->setText( 0, tr( "MAX_ELEMENT_LENGTH_2D" ));
            diamItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
          }
        }
        if( e->GetType() == SMDSAbs_Volume ) {
          if ( !e->IsPoly() )
          {
            //AspectRatio3D
            afunctor.reset( new SMESH::Controls::AspectRatio3D() );
            afunctor->SetMesh( actor->GetObject()->GetMesh() );
            QTreeWidgetItem* ratlItem3 = createItem( cntrItem, Bold );
            ratlItem3->setText( 0, tr( "ASPECTRATIO_3D_ELEMENTS" ));
            ratlItem3->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
          }
          //Volume
          afunctor.reset( new SMESH::Controls::Volume() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          QTreeWidgetItem* volItem = createItem( cntrItem, Bold );
          volItem->setText( 0, tr( "VOLUME_3D_ELEMENTS" ));
          volItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
          //ElementDiameter3D
          afunctor.reset( new SMESH::Controls::MaxElementLength3D() );
          afunctor->SetMesh( actor->GetObject()->GetMesh() );
          QTreeWidgetItem* diam3Item = createItem( cntrItem, Bold );
          diam3Item->setText( 0, tr( "MAX_ELEMENT_LENGTH_3D" ));
          diam3Item->setText( 1, QString( "%1" ).arg( afunctor->GetValue( id )) );
        }

        //min edge length
        afunctor.reset( new SMESH::Controls::Length2D() );
        afunctor->SetMesh( actor->GetObject()->GetMesh() );
        QTreeWidgetItem* minEdgeItem = createItem( cntrItem, Bold );
        minEdgeItem->setText( 0, tr( "MIN_ELEM_EDGE" ));
        SMESH::Controls::TSequenceOfXYZ points;
        afunctor->GetPoints( e, points ); // "non-standard" way, to make it work for all elem types
        minEdgeItem->setText( 1, QString( "%1" ).arg( afunctor->GetValue( points )) );

        // gravity center
        XYZ gc = gravityCenter( e );
        QTreeWidgetItem* gcItem = createItem( elemItem, Bold );
        gcItem->setText( 0, SMESHGUI_ElemInfo::tr( "GRAVITY_CENTER" ));
        QTreeWidgetItem* xItem = createItem( gcItem );
        xItem->setText( 0, "X" );
        xItem->setText( 1, QString::number( gc.x(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
        QTreeWidgetItem* yItem = createItem( gcItem );
        yItem->setText( 0, "Y" );
        yItem->setText( 1, QString::number( gc.y(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
        QTreeWidgetItem* zItem = createItem( gcItem );
        zItem->setText( 0, "Z" );
        zItem->setText( 1, QString::number( gc.z(), precision > 0 ? 'f' : 'g', qAbs( precision )) );

        // normal vector
        if( e->GetType() == SMDSAbs_Face ) {
          XYZ gc = normal( e );
          QTreeWidgetItem* nItem = createItem( elemItem, Bold );
          nItem->setText( 0, SMESHGUI_ElemInfo::tr( "NORMAL_VECTOR" ));
          QTreeWidgetItem* xItem = createItem( nItem );
          xItem->setText( 0, "X" );
          xItem->setText( 1, QString::number( gc.x(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
          QTreeWidgetItem* yItem = createItem( nItem );
          yItem->setText( 0, "Y" );
          yItem->setText( 1, QString::number( gc.y(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
          QTreeWidgetItem* zItem = createItem( nItem );
          zItem->setText( 0, "Z" );
          zItem->setText( 1, QString::number( gc.z(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
        }

        // element position
        SMESH::SMESH_Mesh_ptr aMesh = actor->GetObject()->GetMeshServer();
        if ( e->GetType() >= SMDSAbs_Edge && e->GetType() <= SMDSAbs_Volume ) {
          if ( !CORBA::is_nil( aMesh )) {
            SMESH::ElementPosition pos = aMesh->GetElementPosition( id );
            int shapeID = pos.shapeID;
            if ( shapeID > 0 ) {
              QTreeWidgetItem* shItem = createItem( elemItem, Bold );
              QString shapeType;
              switch ( pos.shapeType ) {
              case GEOM::EDGE:   shapeType = SMESHGUI_ElemInfo::tr( "GEOM_EDGE" );   break;
              case GEOM::FACE:   shapeType = SMESHGUI_ElemInfo::tr( "GEOM_FACE" );   break;
              case GEOM::VERTEX: shapeType = SMESHGUI_ElemInfo::tr( "GEOM_VERTEX" ); break;
              case GEOM::SOLID:  shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SOLID" );  break;
              case GEOM::SHELL:  shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SHELL" );  break;
              default:           shapeType = SMESHGUI_ElemInfo::tr( "GEOM_SHAPE" );  break;
              }
              shItem->setText( 0, SMESHGUI_ElemInfo::tr( "POSITION" ));
              shItem->setText( 1, QString( "%1 #%2" ).arg( shapeType ).arg( shapeID ));
            }
          }
        }
        // groups element belongs to
        if ( !CORBA::is_nil( aMesh )) {
          SMESH::ListOfGroups_var  groups = aMesh->GetGroups();
          QTreeWidgetItem* groupsItem = 0;
          for ( CORBA::ULong i = 0; i < groups->length(); i++ ) {
            SMESH::SMESH_GroupBase_var aGrp = groups[i];
            if ( CORBA::is_nil( aGrp )) continue;
            QString aName = (char*) (CORBA::String_var) aGrp->GetName();
            if ( aGrp->GetType() != SMESH::NODE && !aName.isEmpty() && aGrp->Contains( id )) {
              if ( !groupsItem ) {
                groupsItem = createItem( elemItem, Bold );
                groupsItem->setText( 0, SMESHGUI_AddInfo::tr( "GROUPS" ));
              }
              QTreeWidgetItem* it = createItem( groupsItem, Bold );
              it->setText( 0, aName.trimmed() );
              if ( grp_details ) {
                SMESH::SMESH_Group_var         aStdGroup  = SMESH::SMESH_Group::_narrow( aGrp );
                SMESH::SMESH_GroupOnGeom_var   aGeomGroup = SMESH::SMESH_GroupOnGeom::_narrow( aGrp );
                SMESH::SMESH_GroupOnFilter_var aFltGroup  = SMESH::SMESH_GroupOnFilter::_narrow( aGrp );
                
                // type : group on geometry, standalone group, group on filter
                QTreeWidgetItem* typeItem = createItem( it );
                typeItem->setText( 0, SMESHGUI_AddInfo::tr( "TYPE" ));
                if ( !CORBA::is_nil( aStdGroup )) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" ));
                }
                else if ( !CORBA::is_nil( aGeomGroup )) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" ));
                  GEOM::GEOM_Object_var gobj = aGeomGroup->GetShape();
                  _PTR(SObject) sobj = SMESH::ObjectToSObject( gobj );
                  if ( sobj ) {
                    QTreeWidgetItem* gobjItem = createItem( typeItem );
                    gobjItem->setText( 0, SMESHGUI_AddInfo::tr( "GEOM_OBJECT" ));
                    gobjItem->setText( 1, sobj->GetName().c_str() );
                  }
                }
                else if ( !CORBA::is_nil( aFltGroup )) {
                  typeItem->setText( 1, SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" ));
                }
                
                // size
                QTreeWidgetItem* sizeItem = createItem( it );
                sizeItem->setText( 0, SMESHGUI_AddInfo::tr( "SIZE" ));
                sizeItem->setText( 1, QString::number( aGrp->Size() ));
                
                // color
                SALOMEDS::Color color = aGrp->GetColor();
                QTreeWidgetItem* colorItem = createItem( it );
                colorItem->setText( 0, SMESHGUI_AddInfo::tr( "COLOR" ));
                colorItem->setBackground( 1, QBrush( QColor( color.R*255., color.G*255., color.B*255.) ));
              }
            }
          }
        }
      }
    }
  }
}

/*!
  \brief Show node information
  \param node mesh node for showing
  \param index index of current node
  \param nbNodes number of unique nodes in element
  \param parentItem parent item of tree
*/
void SMESHGUI_TreeElemInfo::nodeInfo( const SMDS_MeshNode* node, int index,
                                      int nbNodes, QTreeWidgetItem* parentItem )
{
  int precision   = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
  // node number and ID
  QTreeWidgetItem* nodeItem = createItem( parentItem, Bold );
  nodeItem->setText( 0, QString( "%1 %2 / %3" ).arg( SMESHGUI_ElemInfo::tr( "NODE" )).arg( index ).arg( nbNodes ));
  nodeItem->setText( 1, QString( "#%1" ).arg( node->GetID() ));
  nodeItem->setData( 1, TypeRole, ElemConnectivity );
  nodeItem->setData( 1, IdRole, node->GetID() );
  nodeItem->setExpanded( false );
  // node coordinates
  QTreeWidgetItem* coordItem = createItem( nodeItem );
  coordItem->setText( 0, SMESHGUI_ElemInfo::tr( "COORDINATES" ));
  QTreeWidgetItem* xItem = createItem( coordItem );
  xItem->setText( 0, "X" );
  xItem->setText( 1, QString::number( node->X(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
  QTreeWidgetItem* yItem = createItem( coordItem );
  yItem->setText( 0, "Y" );
  yItem->setText( 1, QString::number( node->Y(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
  QTreeWidgetItem* zItem = createItem( coordItem );
  zItem->setText( 0, "Z" );
  zItem->setText( 1, QString::number( node->Z(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
  // node connectivity
  QTreeWidgetItem* nconItem = createItem( nodeItem );
  nconItem->setText( 0, SMESHGUI_ElemInfo::tr( "CONNECTIVITY" ));
  Connectivity connectivity = nodeConnectivity( node );
  if ( !connectivity.isEmpty() ) {
    QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "0D_ELEMENTS" ));
      i->setText( 1, con );
    }
    con = formatConnectivity( connectivity, SMDSAbs_Edge );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "EDGES" ));
      i->setText( 1, con );
      i->setData( 1, TypeRole, NodeConnectivity );
    }
    con = formatConnectivity( connectivity, SMDSAbs_Ball );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "BALL_ELEMENTS" ));
      i->setText( 1, con );
      i->setData( 1, TypeRole, NodeConnectivity );
    }
    con = formatConnectivity( connectivity, SMDSAbs_Face );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "FACES" ));
      i->setText( 1, con );
      i->setData( 1, TypeRole, NodeConnectivity );
    }
    con = formatConnectivity( connectivity, SMDSAbs_Volume );
    if ( !con.isEmpty() ) {
      QTreeWidgetItem* i = createItem( nconItem );
      i->setText( 0, SMESHGUI_ElemInfo::tr( "VOLUMES" ));
      i->setText( 1, con );
      i->setData( 1, TypeRole, NodeConnectivity );
    }
  }
}
/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_TreeElemInfo::clearInternal()
{
  myInfo->clear();
  myInfo->repaint();
}

/*!
  \brief Create new tree item.
  \param parent parent tree widget item
  \param flags item flag
  \return new tree widget item
*/
QTreeWidgetItem* SMESHGUI_TreeElemInfo::createItem( QTreeWidgetItem* parent, int flags )
{
  QTreeWidgetItem* item;
  if ( parent )
    item = new QTreeWidgetItem( parent );
  else
    item = new QTreeWidgetItem( myInfo );

  item->setFlags( item->flags() | Qt::ItemIsEditable );

  QFont f = item->font( 0 );
  f.setBold( true );
  for ( int i = 0; i < myInfo->columnCount(); i++ ) {
    if ( ( flags & Bold ) && ( i == 0 || flags & AllColumns ))
      item->setFont( i, f );
  }

  if ( parent && parent->childCount() == 1 && itemDepth( parent ) == 1 )
  {
    QString resName = expandedResource( parent );
    parent->setExpanded( SMESHGUI::resourceMgr()->booleanValue("SMESH", resName, true ));
  }

  item->setExpanded( true );
  return item;
}

void SMESHGUI_TreeElemInfo::contextMenuEvent( QContextMenuEvent* e )
{
  QList< QTreeWidgetItem* > widgets = myInfo->selectedItems();
  if ( widgets.isEmpty() ) return;
  QTreeWidgetItem* aTreeItem = widgets.first();
  int type = aTreeItem->data( 1, TypeRole ).toInt();
  int id   = aTreeItem->data( 1, IdRole ).toInt();
  QMenu menu;
  QAction* a = menu.addAction( tr( "SHOW_ITEM_INFO" ));
  if ( type == ElemConnectivity && id > 0 && menu.exec( e->globalPos() ) == a )
    emit( itemInfo( id ));
  else if ( type == NodeConnectivity && menu.exec( e->globalPos() ) == a )
    emit( itemInfo( aTreeItem->text( 1 )) );
}

void SMESHGUI_TreeElemInfo::itemDoubleClicked( QTreeWidgetItem* theItem, int theColumn )
{
  if ( theItem ) {
    int type = theItem->data( 1, TypeRole ).toInt();
    int id   = theItem->data( 1, IdRole ).toInt();
    if ( type == ElemConnectivity && id > 0 )
      emit( itemInfo( id ));
    else if ( type == NodeConnectivity )
      emit( itemInfo( theItem->text( 1 )) );
  }
}

void SMESHGUI_TreeElemInfo::saveExpanded( QTreeWidgetItem* theItem )
{
  if ( theItem )
    SMESHGUI::resourceMgr()->setValue("SMESH", expandedResource( theItem ), theItem->isExpanded() );
}

QString SMESHGUI_TreeElemInfo::expandedResource( QTreeWidgetItem* theItem )
{
  QString suffix;
  switch( what() )
  {
  case ShowNodes:
    suffix = "N_"; break;
  case ShowElements:
    suffix = "E_"; break;
  default:
    break;
  }
  return QString( "Expanded_" ) + suffix + theItem->text( 0 );
}

void SMESHGUI_TreeElemInfo::saveInfo( QTextStream &out )
{
  out << ruler( 12 ) << endl;
  out << SMESHGUI_ElemInfo::tr( "ELEM_INFO" ) << endl;
  out << ruler( 12 ) << endl;

  QTreeWidgetItemIterator it( myInfo );
  while ( *it ) {
    if ( !( *it )->text(0).isEmpty() ) {
      out << indent( itemDepth( *it ) ) << ( *it )->text(0);
      if ( !( *it )->text(1).isEmpty() ) out << ": " << ( *it )->text(1);
      out << endl;
    }
    ++it;
  }
  out << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// \class InfoComputor
/// \brief Mesh information computor.
/// \internal
///
/// The class is created for different computation operations. Currently it is
/// used to compute size and number of underlying nodes for given group.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent object.
  \param proxy Object to compute information on (group).
  \param item Tree widget item, referenced by this computer.
  \param operation Value to compute.
  \internal
*/
InfoComputor::InfoComputor( QObject* parent, const SMESHGUI_SelectionProxy& proxy, int operation )
  : QObject( parent ), myProxy( proxy ), myOperation( operation )
{
}

/*!
  \brief Compute requested information.
  \internal
*/
void InfoComputor::compute()
{
  if ( myProxy )
  {
    SUIT_OverrideCursor wc;
    switch ( myOperation )
    {
    case GrpSize:
      myProxy.size( true ); // force size computation
      emit computed();
      break;
    case GrpNbNodes:
      myProxy.nbNodes( true ); // force size computation
      emit computed();
      break;
    default:
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_AddInfo
/// \brief Show additional information on selected object.
///
/// Displays an additional information about selected object: mesh, sub-mesh
/// or group.
///
/// \todo Rewrite saveInfo() method to print all data, not currently shown only.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_AddInfo::SMESHGUI_AddInfo( QWidget* parent ): SMESHGUI_Info( parent )
{
  QVBoxLayout* l = new QVBoxLayout( this );
  l->setMargin( 0 );
  l->setSpacing( SPACING );

  myTree = new QTreeWidget( this );

  myTree->setColumnCount( 2 );
  myTree->header()->setStretchLastSection( true );
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  myTree->header()->setResizeMode( 0, QHeaderView::ResizeToContents );
#else
  myTree->header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
#endif
  myTree->header()->hide();

  l->addWidget( myTree );
}

/*!
  \brief Destructor.
*/
SMESHGUI_AddInfo::~SMESHGUI_AddInfo()
{
}

/*!
  \brief Show information on given object.
  \param proxy Object to show information on (mesh, sub-mesh, group).
*/
void SMESHGUI_AddInfo::showInfo( const SMESHGUI_SelectionProxy& proxy )
{
  // reset panel
  setProperty( "group_index", 0 );
  setProperty( "submesh_index",  0 );
  myComputors.clear();
  myTree->clear();

  // then fill panel with data if object is not null
  if ( proxy )
  {
    myProxy = proxy;

    // name
    QTreeWidgetItem* nameItem = createItem( 0, Bold | AllColumns );
    nameItem->setText( 0, tr( "NAME" ) );
    nameItem->setText( 1, proxy.name() );

    // object info
    if ( proxy.type() == SMESHGUI_SelectionProxy::Mesh )
      meshInfo( proxy, nameItem );
    else if ( proxy.type() == SMESHGUI_SelectionProxy::Submesh )
      subMeshInfo( proxy, nameItem );
    else if ( proxy.type() >= SMESHGUI_SelectionProxy::Group )
      groupInfo( proxy, nameItem );
  }
}

/*!
  \brief Update information in panel.
*/
void SMESHGUI_AddInfo::updateInfo()
{
  showInfo( myProxy );
}

/*!
  \brief Reset panel (clear all data).
*/
void SMESHGUI_AddInfo::clear()
{
  myTree->clear();
}

/*!
  \brief Create new item and add it to the tree.
  \param parent Parent tree widget item. Defaults to 0 (top-level item).
  \param options Item flags. Defaults to 0 (none).
  \return New tree widget item.
*/
QTreeWidgetItem* SMESHGUI_AddInfo::createItem( QTreeWidgetItem* parent, int options )
{
  QTreeWidgetItem* item;
  if ( parent )
    item = new QTreeWidgetItem( parent );
  else
    item = new QTreeWidgetItem( myTree );
  setFontAttributes( item, options );
  item->setExpanded( true );
  return item;
}

/*!
  \brief Show information on mesh.
  \param proxy Proxy object (mesh).
  \param parent Parent tree item.
*/
void SMESHGUI_AddInfo::meshInfo( const SMESHGUI_SelectionProxy& proxy, QTreeWidgetItem* parent )
{
  if ( !proxy )
    return;

  QString shapeName = proxy.shapeName();
  SMESHGUI_MedFileInfo inf = proxy.medFileInfo();

  // type
  QTreeWidgetItem* typeItem = createItem( parent, Bold );
  typeItem->setText( 0, tr( "TYPE" ) );
  if ( !shapeName.isEmpty() )
  {
    typeItem->setText( 1, tr( "MESH_ON_GEOMETRY" ) );
    // shape
    QTreeWidgetItem* gobjItem = createItem( parent, Bold );
    gobjItem->setText( 0, tr( "GEOM_OBJECT" ) );
    gobjItem->setText( 1, shapeName );
  }
  else if ( inf.isValid() )
  {
    typeItem->setText( 1, tr( "MESH_FROM_FILE" ) );
    // med file information
    QTreeWidgetItem* fileItem = createItem( parent, Bold );
    fileItem->setText( 0, tr( "FILE_NAME" ) );
    fileItem->setText( 1, inf.fileName() );
    QTreeWidgetItem* sizeItem = createItem( parent, Bold );
    sizeItem->setText( 0, tr( "FILE_SIZE" ) );
    sizeItem->setText( 1, QString::number( inf.size() ) );
    QTreeWidgetItem* versionItem = createItem( parent, Bold );
    versionItem->setText( 0, tr( "FILE_VERSION" ) );
    versionItem->setText( 1, inf.version() != "0" ? inf.version() : tr( "VERSION_UNKNOWN" ) );
  }
  else
  {
    typeItem->setText( 1, tr( "STANDALONE_MESH" ) );
  }
  
  // groups
  myGroups = proxy.groups();
  showGroups();

  // sub-meshes
  mySubMeshes = proxy.submeshes();
  showSubMeshes();
}

/*!
  \brief Show information on sub-mesh.
  \param proxy Proxy object (sub-mesh).
  \param parent Parent tree item.
*/
void SMESHGUI_AddInfo::subMeshInfo( const SMESHGUI_SelectionProxy& proxy, QTreeWidgetItem* parent )
{
  if ( !proxy )
    return;

  bool isShort = parent->parent() != 0;

  if ( !isShort )
  {
    // parent mesh
    SMESHGUI_SelectionProxy meshProxy = proxy.mesh();
    if ( meshProxy )
    {
      QTreeWidgetItem* nameItem = createItem( parent, Bold );
      nameItem->setText( 0, tr( "PARENT_MESH" ) );
      nameItem->setText( 1, meshProxy.name() );
    }
  }
  
  // shape
  QString shapeName = proxy.shapeName();
  if ( !shapeName.isEmpty() )
  {
    QTreeWidgetItem* gobjItem = createItem( parent, Bold );
    gobjItem->setText( 1, shapeName );
  }
}

/*!
  \brief Show information on group.
  \param proxy Proxy object (group).
  \param parent Parent tree item.
*/
void SMESHGUI_AddInfo::groupInfo( const SMESHGUI_SelectionProxy& proxy, QTreeWidgetItem* parent )
{
  if ( !proxy )
    return;

  bool isShort = parent->parent() != 0;

  if ( !isShort )
  {
    // parent mesh
    SMESHGUI_SelectionProxy meshProxy = proxy.mesh();
    if ( meshProxy )
    {
      QTreeWidgetItem* nameItem = createItem( parent, Bold );
      nameItem->setText( 0, tr( "PARENT_MESH" ) );
      nameItem->setText( 1, meshProxy.name() );
    }
  }

  // type
  SMESHGUI_SelectionProxy::Type type = proxy.type();
  QTreeWidgetItem* typeItem = createItem( parent, Bold );
  typeItem->setText( 0, tr( "TYPE" ) );
  if ( type == SMESHGUI_SelectionProxy::GroupStd )
  {
    typeItem->setText( 1, tr( "STANDALONE_GROUP" ) );
  }
  else if ( type == SMESHGUI_SelectionProxy::GroupGeom )
  {
    typeItem->setText( 1, tr( "GROUP_ON_GEOMETRY" ) );
    // shape
    QTreeWidgetItem* gobjItem = createItem( parent, Bold );
    gobjItem->setText( 0, tr( "GEOM_OBJECT" ) );
    gobjItem->setText( 1, proxy.shapeName() );
  }
  else if ( type == SMESHGUI_SelectionProxy::GroupFilter )
  {
    typeItem->setText( 1, tr( "GROUP_ON_FILTER" ) );
  }

  // element type
  int etype = proxy.elementType();
  if ( !isShort )
  {
    QString typeName = tr( "UNKNOWN" );
    switch( etype )
    {
    case SMESH::NODE:
      typeName = tr( "NODE" );
      break;
    case SMESH::EDGE:
      typeName = tr( "EDGE" );
      break;
    case SMESH::FACE:
      typeName = tr( "FACE" );
      break;
    case SMESH::VOLUME:
      typeName = tr( "VOLUME" );
      break;
    case SMESH::ELEM0D:
      typeName = tr( "0DELEM" );
      break;
    case SMESH::BALL:
      typeName = tr( "BALL" );
      break;
    default:
      break;
    }
    QTreeWidgetItem* etypeItem = createItem( parent, Bold );
    etypeItem->setText( 0, tr( "ENTITY_TYPE" ) );
    etypeItem->setText( 1, typeName );
  }

  // size
  // note: size is not computed for group on filter for performance reasons, see IPAL52831
  bool meshLoaded = proxy.isMeshLoaded();
  int size = proxy.size();

  QTreeWidgetItem* sizeItem = createItem( parent, Bold );
  sizeItem->setText( 0, tr( "SIZE" ) );
  if ( size >= 0 )
  {
    sizeItem->setText( 1, QString::number( size ) );
  }
  else
  {
    QPushButton* btn = new QPushButton( meshLoaded ? tr( "COMPUTE" ) : tr( "LOAD" ), this );
    myTree->setItemWidget( sizeItem, 1, btn );
    InfoComputor* comp = new InfoComputor( this, proxy, InfoComputor::GrpSize );
    connect( btn, SIGNAL( clicked() ), comp, SLOT( compute() ) );
    connect( comp, SIGNAL( computed() ), this, SLOT( updateInfo() ) );
    myComputors.append( comp );
  }

  // color
  QTreeWidgetItem* colorItem = createItem( parent, Bold );
  colorItem->setText( 0, tr( "COLOR" ) );
  colorItem->setBackground( 1, proxy.color() );

  // nb of underlying nodes
  if ( etype != SMESH::NODE )
  {
    QTreeWidgetItem* nodesItem = createItem( parent, Bold );
    nodesItem->setText( 0, tr( "NB_NODES" ) );

    int nbNodes = proxy.nbNodes();
    if ( nbNodes >= 0 )
    {
      nodesItem->setText( 1, QString::number( nbNodes ) );
    }
    else
    {
      QPushButton* btn = new QPushButton( meshLoaded ? tr( "COMPUTE" ) : tr( "LOAD" ), this );
      myTree->setItemWidget( nodesItem, 1, btn );
      InfoComputor* comp = new InfoComputor( this, proxy, InfoComputor::GrpNbNodes ); 
      connect( btn, SIGNAL( clicked() ), comp, SLOT( compute() ) );
      connect( comp, SIGNAL( computed() ), this, SLOT( updateInfo() ) );
      myComputors.append( comp );
    }
  }
}

/*!
  \brief Update information on child groups.
*/
void SMESHGUI_AddInfo::showGroups()
{
  // remove all computors
  myComputors.clear();

  // tree root should be the first top level item
  QTreeWidgetItem* parent = myTree->topLevelItemCount() > 0 ? myTree->topLevelItem( 0 ) : 0;
  if ( !parent )
    return;

  int idx = property( "group_index" ).toInt();

  // find sub-meshes top-level container item
  QTreeWidgetItem* itemGroups = 0;
  for ( int i = 0; i < parent->childCount() && !itemGroups; i++ )
  {
    if ( parent->child( i )->data( 0, Qt::UserRole ).toInt() == GroupsId )
    {
      itemGroups = parent->child( i );
      // update controls
      ExtraWidget* extra = dynamic_cast<ExtraWidget*>( myTree->itemWidget( itemGroups, 1 ) );
      if ( extra )
        extra->updateControls( myGroups.count(), idx );
      // clear: remove all group items
      while ( itemGroups->childCount() )
        delete itemGroups->child( 0 );
    }
  }

  QMap<int, QTreeWidgetItem*> grpItems;
  for ( int i = idx*blockSize() ; i < qMin( (idx+1)*blockSize(), (int)myGroups.count() ); i++ )
  {
    SMESHGUI_SelectionProxy grp = myGroups[i];
    if ( !grp )
      continue;

    int grpType = grp.elementType();

    // create top-level groups container item if it does not exist
    if ( !itemGroups )
    {
      itemGroups = createItem( parent, Bold | AllColumns );
      itemGroups->setText( 0, tr( "GROUPS" ) );
      itemGroups->setData( 0, Qt::UserRole, GroupsId );

      // if necessary, create extra widget to show information by chunks
      if ( myGroups.count() > blockSize() )
      {
        ExtraWidget* extra = new ExtraWidget( this, true );
        connect( extra->prev, SIGNAL( clicked() ), this, SLOT( showPreviousGroups() ) );
        connect( extra->next, SIGNAL( clicked() ), this, SLOT( showNextGroups() ) );
        myTree->setItemWidget( itemGroups, 1, extra );
        extra->updateControls( myGroups.count(), idx );
      }
    }

    // create container item corresponding to particular element type
    if ( !grpItems.contains( grpType ) )
    {
      grpItems[ grpType ] = createItem( itemGroups, Bold | AllColumns );
      grpItems[ grpType ]->setText( 0, tr( QString( "GROUPS_%1" ).arg( grpType ).toLatin1().constData() ) );
      itemGroups->insertChild( grpType-1, grpItems[ grpType ] ); // -1 needed since 0 corresponds to SMESH::ALL
    }
  
    // name
    QTreeWidgetItem* nameItem = createItem( grpItems[ grpType ] );
    nameItem->setText( 0, grp.name().trimmed() ); // trim name

    // group info
    groupInfo( grp, nameItem );
  }
}

/*!
  \brief Update information on child sub-meshes.
*/
void SMESHGUI_AddInfo::showSubMeshes()
{
  // tree root should be the first top level item
  QTreeWidgetItem* parent = myTree->topLevelItemCount() > 0 ? myTree->topLevelItem( 0 ) : 0;
  if ( !parent )
    return;

  int idx = property( "submesh_index" ).toInt();

  // find sub-meshes top-level container item
  QTreeWidgetItem* itemSubMeshes = 0;
  for ( int i = 0; i < parent->childCount() && !itemSubMeshes; i++ )
  {
    if ( parent->child( i )->data( 0, Qt::UserRole ).toInt() == SubMeshesId )
    {
      itemSubMeshes = parent->child( i );
      // update controls
      ExtraWidget* extra = dynamic_cast<ExtraWidget*>( myTree->itemWidget( itemSubMeshes, 1 ) );
      if ( extra )
        extra->updateControls( mySubMeshes.count(), idx );
      // clear: remove all sub-mesh items
      while ( itemSubMeshes->childCount() )
        delete itemSubMeshes->child( 0 );
    }
  }

  QMap<int, QTreeWidgetItem*> smItems;
  for ( int i = idx*blockSize() ; i < qMin( (idx+1)*blockSize(), mySubMeshes.count() ); i++ )
  {
    SMESHGUI_SelectionProxy sm = mySubMeshes[i];
    if ( !sm )
      continue;
    
    int smType = sm.shapeType();
    if ( smType < 0 )
      continue;
    else if ( smType == GEOM::COMPSOLID )
      smType = GEOM::COMPOUND;

    // create top-level sub-meshes container item if it does not exist
    if ( !itemSubMeshes )
    {
      itemSubMeshes = createItem( parent, Bold | AllColumns );
      itemSubMeshes->setText( 0, tr( "SUBMESHES" ) );
      itemSubMeshes->setData( 0, Qt::UserRole, SubMeshesId );

      // if necessary, create extra widget to show information by chunks
      if ( mySubMeshes.count() > blockSize() )
      {
        ExtraWidget* extra = new ExtraWidget( this, true );
        connect( extra->prev, SIGNAL( clicked() ), this, SLOT( showPreviousSubMeshes() ) );
        connect( extra->next, SIGNAL( clicked() ), this, SLOT( showNextSubMeshes() ) );
        myTree->setItemWidget( itemSubMeshes, 1, extra );
        extra->updateControls( mySubMeshes.count(), idx );
      }
    }

    // create container item corresponding to particular shape type
    if ( !smItems.contains( smType ) )
    {
      smItems[ smType ] = createItem( itemSubMeshes, Bold | AllColumns );
      smItems[ smType ]->setText( 0, tr( QString( "SUBMESHES_%1" ).arg( smType ).toLatin1().constData() ) );
      itemSubMeshes->insertChild( smType, smItems[ smType ] );
    }
    
    // name
    QTreeWidgetItem* nameItem = createItem( smItems[ smType ] );
    nameItem->setText( 0, sm.name().trimmed() ); // trim name
    
    // submesh info
    subMeshInfo( sm, nameItem );
  }
}

/*!
  \brief Show previous chunk of information on child groups.
*/
void SMESHGUI_AddInfo::showPreviousGroups()
{
  int idx = property( "group_index" ).toInt();
  setProperty( "group_index", idx-1 );
  showGroups();
}

/*!
  \brief Show next chunk of information on child groups.
*/
void SMESHGUI_AddInfo::showNextGroups()
{
  int idx = property( "group_index" ).toInt();
  setProperty( "group_index", idx+1 );
  showGroups();
}

/*!
  \brief Show previous chunk of information on child sub-meshes.
*/
void SMESHGUI_AddInfo::showPreviousSubMeshes()
{
  int idx = property( "submesh_index" ).toInt();
  setProperty( "submesh_index", idx-1 );
  showSubMeshes();
}

/*!
  \brief Show next chunk of information on child sub-meshes.
*/
void SMESHGUI_AddInfo::showNextSubMeshes()
{
  int idx = property( "submesh_index" ).toInt();
  setProperty( "submesh_index", idx+1 );
  showSubMeshes();
}

/*!
  \brief Write information from panel to ouput stream.
  \param out Text stream output.
*/
void SMESHGUI_AddInfo::saveInfo( QTextStream &out )
{
  // title
  QString title = tr( "ADDITIONAL_INFO" );
  out << ruler( title.size() ) << endl;
  out << title << endl;
  out << ruler( title.size() ) << endl;
  out << endl;

  // info
  QTreeWidgetItemIterator it( myTree );
  while ( *it )
  {
    if ( !( ( *it )->text(0) ).isEmpty() )
    {
      out << indent( itemDepth( *it ) ) << ( *it )->text(0);
      if ( ( *it )->text(0)  == tr( "COLOR" ) )
        out << ":" << spacing() << ( ( ( *it )->background(1) ).color() ).name();
      else if ( !( ( *it )->text(1) ).isEmpty() )
        out << ":" << spacing() << ( *it )->text(1);
      out << endl;
    }
    ++it;
  }
  out << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// \class GroupCombo
/// \brief Customized combo box to manage list of mesh groups.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class GroupCombo: public QComboBox
{
  class Item: public QStandardItem
  {
  public:
    SMESHGUI_SelectionProxy myGroup;
    Item( const SMESHGUI_SelectionProxy& group )
    {
      myGroup = group;
      setText( myGroup.name() );
    }
    SMESHGUI_SelectionProxy group()
    {
      return myGroup;
    }
  };

  SMESHGUI_SelectionProxy myProxy;

public:
  GroupCombo( QWidget* );
  void setSource( const SMESHGUI_SelectionProxy& );
  SMESHGUI_SelectionProxy currentGroup() const;
};

/*!
  \brief Contructor.
  \param parent Parent widget.
  \internal
*/
GroupCombo::GroupCombo( QWidget* parent ): QComboBox( parent )
{
  setModel( new QStandardItemModel( this ) );
}

/*!
  \brief Set mesh source.
  \param obj Mesh source.
  \internal
*/
void GroupCombo::setSource( const SMESHGUI_SelectionProxy& proxy )
{
  if ( myProxy == proxy )
    return;

  myProxy = proxy;

  bool blocked = blockSignals( true );
  QStandardItemModel* m = dynamic_cast<QStandardItemModel*>( model() );
  m->clear();

  if ( myProxy )
  {
    if ( myProxy.type() == SMESHGUI_SelectionProxy::Mesh )
    {
      QList<SMESHGUI_SelectionProxy> groups = myProxy.groups();
      for ( int i = 0; i < groups.count(); ++i )
      {
        if ( groups[i] )
        {
          QString name = groups[i].name();
          if ( !name.isEmpty() )
            m->appendRow( new Item( groups[i] ) );
        }
      }
      setCurrentIndex( -1 ); // for performance reasons
    }
    else if ( myProxy.type() >= SMESHGUI_SelectionProxy::Group )
    {
      m->appendRow( new Item( myProxy ) );
      setCurrentIndex( 0 );
    }
  }

  blockSignals( blocked );
}

/*!
  \brief Get currently selected group.
  \return Selected group.
  \internal
*/
SMESHGUI_SelectionProxy GroupCombo::currentGroup() const
{
  SMESHGUI_SelectionProxy group;
  QStandardItemModel* m = dynamic_cast<QStandardItemModel*>( model() );
  if ( currentIndex() >= 0 )
    group = dynamic_cast<Item*>( m->item( currentIndex() ) )->group();
  return group;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_MeshInfoDlg
/// \brief Mesh information dialog box
///
/// \todo Move all business logic for element info to SMESHGUI_ElemInfo class.
/// \todo Add selection button to reactivate selection on move from other dlg.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor
  \param parent Parent widget.
  \param page Dialog page to show at start-up. Defaults to \c BaseInfo.
*/
SMESHGUI_MeshInfoDlg::SMESHGUI_MeshInfoDlg( QWidget* parent, int page )
  : QDialog( parent )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "MESH_INFO" ) );
  setSizeGripEnabled( true );

  myTabWidget = new QTabWidget( this );

  // base info

  myBaseInfo = new SMESHGUI_BaseInfo( myTabWidget );
  myTabWidget->addTab( myBaseInfo, tr( "BASE_INFO" ) );

  // elem info 

  QWidget* w = new QWidget( myTabWidget );

  myMode = new QButtonGroup( this );
  myMode->addButton( new QRadioButton( tr( "NODE_MODE" ), w ), NodeMode );
  myMode->addButton( new QRadioButton( tr( "ELEM_MODE" ), w ), ElemMode );
  myMode->addButton( new QRadioButton( tr( "GROUP_MODE" ), w ), GroupMode );
  myMode->button( NodeMode )->setChecked( true );
  myID = new QLineEdit( w );
  myID->setValidator( new SMESHGUI_IdValidator( this ) );
  myGroups = new GroupCombo( w );
  QStackedWidget* stack = new QStackedWidget( w );
  stack->addWidget( myID );
  stack->addWidget( myGroups );
  myIDPreviewCheck = new QCheckBox( tr( "SHOW_IDS" ), w );
  myIDPreview = new SMESHGUI_IdPreview( SMESH::GetViewWindow( SMESHGUI::GetSMESHGUI() ) );

  int mode = SMESHGUI::resourceMgr()->integerValue( "SMESH", "mesh_elem_info", 1 );
  mode = qMin( 1, qMax( 0, mode ) );

  if ( mode == 0 )
    myElemInfo = new SMESHGUI_SimpleElemInfo( w );
  else
    myElemInfo = new SMESHGUI_TreeElemInfo( w );
  stack->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

  QGridLayout* elemLayout = new QGridLayout( w );
  elemLayout->setMargin( MARGIN );
  elemLayout->setSpacing( SPACING );
  elemLayout->addWidget( myMode->button( NodeMode ), 0, 0 );
  elemLayout->addWidget( myMode->button( ElemMode ), 0, 1 );
  elemLayout->addWidget( myMode->button( GroupMode ), 0, 2 );
  elemLayout->addWidget( stack, 0, 3 );
  elemLayout->addWidget( myIDPreviewCheck, 1, 0, 1, 4 );
  elemLayout->addWidget( myElemInfo, 2, 0, 1, 4 );

  myTabWidget->addTab( w, tr( "ELEM_INFO" ) );

  // additional info

  myAddInfo = new SMESHGUI_AddInfo( myTabWidget );
  myTabWidget->addTab( myAddInfo, tr( "ADDITIONAL_INFO" ) );

  // controls info

  myCtrlInfo = new SMESHGUI_CtrlInfo( myTabWidget );
  myTabWidget->addTab( myCtrlInfo, tr( "CTRL_INFO" ) );

  // buttons

  QPushButton* okBtn = new QPushButton( tr( "SMESH_BUT_OK" ), this );
  okBtn->setAutoDefault( true );
  okBtn->setDefault( true );
  okBtn->setFocus();
  QPushButton* dumpBtn = new QPushButton( tr( "BUT_DUMP_MESH" ), this );
  dumpBtn->setAutoDefault( true );
  QPushButton* helpBtn = new QPushButton( tr( "SMESH_BUT_HELP" ), this );
  helpBtn->setAutoDefault( true );

  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->setSpacing( SPACING );
  btnLayout->setMargin( 0 );

  btnLayout->addWidget( okBtn );
  btnLayout->addWidget( dumpBtn );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( helpBtn );

  // arrange widgets

  QVBoxLayout* l = new QVBoxLayout ( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );
  l->addWidget( myTabWidget );
  l->addLayout( btnLayout );

  // set initial page

  myTabWidget->setCurrentIndex( qMax( (int)BaseInfo, qMin( (int)ElemInfo, page ) ) );

  // set-up connections

  connect( okBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( dumpBtn, SIGNAL( clicked() ), this, SLOT( dump() ) );
  connect( helpBtn, SIGNAL( clicked() ), this, SLOT( help() ) );
  connect( myTabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( updateSelection() ) );
  connect( myMode, SIGNAL( buttonClicked( int ) ), this, SLOT( modeChanged() ) );
  connect( myGroups, SIGNAL( currentIndexChanged( int ) ), this, SLOT( modeChanged() ) );
  connect( myID, SIGNAL( textChanged( QString ) ), this, SLOT( idChanged() ) );
  connect( myIDPreviewCheck, SIGNAL( toggled( bool ) ), this, SLOT( idPreviewChange( bool ) ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( deactivate() ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalCloseAllDialogs() ), this, SLOT( reject() ) );
  connect( myElemInfo, SIGNAL( itemInfo( int ) ), this, SLOT( showItemInfo( int ) ) );
  connect( myElemInfo, SIGNAL( itemInfo( QString ) ), this, SLOT( showItemInfo( QString ) ) );
  connect( this, SIGNAL( switchMode( int ) ), stack, SLOT( setCurrentIndex( int ) ) );

  // initialize

  myIDPreviewCheck->setChecked( SMESHGUI::resourceMgr()->booleanValue( "SMESH", "id_preview_resource", false ) );
  updateSelection();
}

/*!
  \brief Destructor.
*/
SMESHGUI_MeshInfoDlg::~SMESHGUI_MeshInfoDlg()
{
  delete myIDPreview;
}

/*!
  \brief Show mesh information on given object.
  \param io Interactive object.
*/
void SMESHGUI_MeshInfoDlg::showInfo( const Handle(SALOME_InteractiveObject)& io )
{
  if ( !io.IsNull() )
    showInfo( SMESHGUI_SelectionProxy( io ) );
}

/*!
  \brief Show mesh information on given object.
  \param proxy Selection proxy.
*/
void SMESHGUI_MeshInfoDlg::showInfo( const SMESHGUI_SelectionProxy& proxy )
{
  SUIT_OverrideCursor wc;

  if ( !proxy )
    return;

  myProxy = proxy;

  SMESH::SMESH_IDSource_var obj = myProxy.object();

  // "Base info" tab
  myBaseInfo->showInfo( proxy );

  // "Additional info" tab
  myAddInfo->showInfo( proxy );

  // "Quality info" tab
  // Note: for performance reasons we update it only if it is currently active
  if ( myTabWidget->currentIndex() == CtrlInfo )
    myCtrlInfo->showInfo( proxy );

  // "Element info" tab
  myGroups->setSource( proxy );
  if ( myMode->checkedId() == GroupMode ) {
    SMESHGUI_SelectionProxy group = myGroups->currentGroup();
    if ( group )
      myElemInfo->showInfo( group );
    else
      myElemInfo->clear();
    }
  else {
    SVTK_Selector* selector = SMESH::GetSelector();
    QString ID;
    int nb = 0;
    if ( myProxy.actor() && selector ) { //todo: actor()?
      nb = myMode->checkedId() == NodeMode ?
	SMESH::GetNameOfSelectedElements( selector, myProxy.io(), ID ) :
	SMESH::GetNameOfSelectedNodes( selector, myProxy.io(), ID );
    }
    if ( nb > 0 ) {
      myID->setText( ID.trimmed() );
      QSet<uint> ids;
      QStringList idTxt = ID.split( " ", QString::SkipEmptyParts );
      foreach ( ID, idTxt )
	ids << ID.trimmed().toUInt();
      myElemInfo->showInfo( proxy, ids, myMode->checkedId() == ElemMode );
    }
    else {
      myID->clear();
      myElemInfo->clear();
    }
  }
}

/*!
  \brief Update information.
*/
void SMESHGUI_MeshInfoDlg::updateInfo()
{
  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 )
    showInfo( selected.First() );
  else
    showInfo( myProxy );
}

/*!
  \brief Clean-up on dialog closing.
*/
void SMESHGUI_MeshInfoDlg::reject()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();
  selMgr->clearFilters();
  SMESH::SetPointRepresentation( false );
  if ( SVTK_ViewWindow* viewWindow = SMESH::GetViewWindow() )
    viewWindow->SetSelectionMode( ActorSelection );
  QDialog::reject();
  myIDPreview->SetPointsLabeled( false );
}

/*!
  \brief Process keyboard event.
  \param e Key press event.
*/
void SMESHGUI_MeshInfoDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( !e->isAccepted() && e->key() == Qt::Key_F1 ) {
    e->accept();
    help();
  }
}

/*!
  \brief Set-up selection mode for currently selected page.
*/
void SMESHGUI_MeshInfoDlg::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();

  disconnect( selMgr, 0, this, 0 );
  selMgr->clearFilters();

  int selMode = ActorSelection;
  if ( myTabWidget->currentIndex() == ElemInfo && myMode->checkedId() == NodeMode )
    selMode = NodeSelection;
  else if ( myTabWidget->currentIndex() == ElemInfo && myMode->checkedId() == ElemMode )
    selMode = CellSelection;
  SMESH::SetPointRepresentation( selMode == NodeSelection );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
    aViewWindow->SetSelectionMode( selMode );

  SMESHGUI_SelectionProxy previous = myProxy;
  QString ids = myID->text().trimmed();
  myID->clear();
  
  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
  updateInfo();
  
  if ( myProxy && myProxy == previous && !ids.isEmpty() ) {
    myID->setText( ids );
    idChanged();
  }
}

/*!
  \brief Show documentation on selected dialog page.
*/
void SMESHGUI_MeshInfoDlg::help()
{
  QString helpPage = "mesh_infos.html";
  switch ( myTabWidget->currentIndex() )
  {
  case BaseInfo:
    helpPage += "#advanced-mesh-infos-anchor";
    break;
  case ElemInfo:
    helpPage += "#mesh-element-info-anchor";
    break;
  case AddInfo:
    helpPage += "#mesh-addition-info-anchor";
    break;
  case CtrlInfo:
    helpPage += "#mesh-quality-info-anchor";
    break;
  default:
    break;
  }
  SMESH::ShowHelpFile( helpPage );
}

/*!
  \brief Deactivate dialog box.
*/
void SMESHGUI_MeshInfoDlg::deactivate()
{
  disconnect( SMESHGUI::selectionMgr(), SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
}

/*!
  \brief Called when users switches between node / element modes.
*/
void SMESHGUI_MeshInfoDlg::modeChanged()
{
  emit( switchMode( myMode->checkedId() == GroupMode ? 1 : 0 ) );
  myID->clear();
  updateSelection();
}

/*!
  \brief Called when users prints mesh element ID in the corresponding field.
*/
void SMESHGUI_MeshInfoDlg::idChanged()
{
  myIDPreview->SetPointsLabeled( false );

  SVTK_Selector* selector = SMESH::GetSelector();
  if ( myProxy.actor() && selector ) { //todo: actor()?
    Handle(SALOME_InteractiveObject) IO = myProxy.actor()->getIO(); //todo: actor()?
    TColStd_MapOfInteger ID;
    QSet<uint>           ids;
    std::vector<int>     idVec;
    std::list< gp_XYZ >  aGrCentersXYZ;
    QStringList idTxt = myID->text().split( " ", QString::SkipEmptyParts );
    foreach ( QString tid, idTxt ) {
      long id = tid.trimmed().toUInt();
      const SMDS_MeshElement* e = myMode->checkedId() == ElemMode ?
        myProxy.actor()->GetObject()->GetMesh()->FindElement( id ) : //todo: actor()?
        myProxy.actor()->GetObject()->GetMesh()->FindNode( id ); //todo: actor()?
      if ( e ) {
        ID.Add( id );
        ids << id;
        if ( myMode->checkedId() == ElemMode )
        {
          idVec.push_back( id );
          aGrCentersXYZ.push_back( myElemInfo->getGravityCenter( e ) );
        }
      }
    }
    selector->AddOrRemoveIndex( IO, ID, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() ) {

      if ( myMode->checkedId() == NodeMode )
        myIDPreview->SetPointsData( myProxy.actor()->GetObject()->GetMesh(), ID ); //todo: actor()?
      else
        myIDPreview->SetElemsData( idVec, aGrCentersXYZ );

      bool showIDs = ( !ID.IsEmpty() &&
                       myIDPreviewCheck->isChecked() &&
                       myTabWidget->currentIndex() == ElemInfo );
      myIDPreview->SetPointsLabeled( showIDs, myProxy.actor()->GetVisibility() ); //todo: actor()?

      aViewWindow->highlight( IO, true, true );
      aViewWindow->Repaint();
    }
    myElemInfo->showInfo( myProxy, ids, myMode->checkedId() == ElemMode );
  }
}

/*!
 * \brief Show IDs clicked
 */
void SMESHGUI_MeshInfoDlg::idPreviewChange( bool isOn )
{
  myIDPreview->SetPointsLabeled( isOn && !myID->text().simplified().isEmpty() );
  SMESHGUI::resourceMgr()->setValue("SMESH", "id_preview_resource", isOn );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
    aViewWindow->Repaint();
}

void SMESHGUI_MeshInfoDlg::showItemInfo( int id )
{
  if ( id > 0 && myProxy.actor()->GetObject()->GetMesh()->FindNode( id ) ) { //todo: actor()?
    myMode->button( NodeMode )->click();
    myID->setText( QString::number( id ) );
  }
}

void SMESHGUI_MeshInfoDlg::showItemInfo( const QString& theStr )
{
  if ( !theStr.isEmpty() ) {
    myMode->button( ElemMode )->click();
    myID->setText( theStr );
  }
}

/*!
  \brief Dump information to file.
*/
void SMESHGUI_MeshInfoDlg::dump()
{
  DumpFileDlg fd( this );
  fd.setWindowTitle( tr( "SAVE_INFO" ) );
  fd.setNameFilters( QStringList() << tr( "TEXT_FILES" ) );
  fd.setChecked( BaseInfo, SMESHGUI::resourceMgr()->booleanValue( "SMESH", "info_dump_base", true ) );
  fd.setChecked( ElemInfo, SMESHGUI::resourceMgr()->booleanValue( "SMESH", "info_dump_elem", true ) );
  fd.setChecked( AddInfo, SMESHGUI::resourceMgr()->booleanValue( "SMESH", "info_dump_add", true ) );
  fd.setChecked( CtrlInfo, SMESHGUI::resourceMgr()->booleanValue( "SMESH", "info_dump_ctrl", true ) );
  if ( fd.exec() == QDialog::Accepted )
  {
    QString fileName = fd.selectedFile();
    if ( !fileName.isEmpty() ) {
      QFile file( fileName );
      if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
        return;

      QTextStream out( &file );
      if ( fd.isChecked( BaseInfo ) ) myBaseInfo->saveInfo( out );
      if ( fd.isChecked( ElemInfo ) ) myElemInfo->saveInfo( out );
      if ( fd.isChecked( AddInfo ) )  myAddInfo->saveInfo( out );
      if ( fd.isChecked( CtrlInfo ) ) myCtrlInfo->saveInfo( out );
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_CtrlInfo
/// \brief Show quality statistics information on selected object.
///
/// Displays quality controls statistics about selected object: mesh, sub-mesh,
/// group or arbitrary ID source.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_CtrlInfo::SMESHGUI_CtrlInfo( QWidget* parent ): SMESHGUI_Info( parent )
{
  QGridLayout* l = new QGridLayout( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );

  QIcon aComputeIcon( SUIT_Session::session()->resourceMgr()->loadPixmap( "SMESH", tr( "ICON_COMPUTE" ) ) );
  SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();

  // name
  QLabel* aNameLab = createLabel( tr( "NAME_LAB" ), this, Bold );
  QLabel* aName = createField( this, "ctrlName" );
  aName->setMinimumWidth( 150 );
  myWidgets << aName;

  // nodes info
  QLabel* aNodesLab = createLabel( tr( "NODES_INFO" ), this, Bold );
  QLabel* aNodesFreeLab = new QLabel( tr( "NUMBER_OF_THE_FREE_NODES" ), this );
  QLabel* aNodesFree = createField( this, "ctrlNodesFree" );
  myWidgets << aNodesFree;
  myPredicates << aFilterMgr->CreateFreeNodes();
  //
  QLabel* aNodesNbConnLab = new QLabel( tr( "MAX_NODE_CONNECTIVITY" ), this );
  QLabel* aNodesNbConn = createField( this, "ctrlNodesCnty" );
  myWidgets << aNodesNbConn;
  myNodeConnFunctor = aFilterMgr->CreateNodeConnectivityNumber();
  //
  QLabel* aNodesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_NODES" ), this );
  QLabel* aNodesDouble = createField( this, "ctrlNodesDouble" );
  myWidgets << aNodesDouble;
  myPredicates << aFilterMgr->CreateEqualNodes();
  QLabel* aToleranceLab = new QLabel( tr( "DOUBLE_NODES_TOLERANCE" ), this );
  myToleranceWidget = new SMESHGUI_SpinBox( this );
  myToleranceWidget->RangeStepAndValidator(0.0000000001, 1000000.0, 0.0000001, "length_precision" );
  myToleranceWidget->setAcceptNames( false );
  myToleranceWidget->SetValue( SMESHGUI::resourceMgr()->doubleValue( "SMESH", "equal_nodes_tolerance", 1e-7 ) );

  // edges info
  QLabel* anEdgesLab = createLabel( tr( "EDGES_INFO" ), this, Bold );
  QLabel* anEdgesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_EDGES" ),     this );
  QLabel* anEdgesDouble = createField( this, "ctrlEdgesDouble" );
  myWidgets << anEdgesDouble;
  myPredicates << aFilterMgr->CreateEqualEdges();

  // faces info
  QLabel* aFacesLab = createLabel( tr( "FACES_INFO" ), this, Bold );
  QLabel* aFacesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_FACES" ), this );
  QLabel* aFacesDouble = createField( this, "ctrlFacesDouble" );
  myWidgets << aFacesDouble;
  myPredicates << aFilterMgr->CreateEqualFaces();
  QLabel* aFacesOverLab = new QLabel( tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ), this );
  QLabel* aFacesOver = createField( this, "ctrlFacesOver" );
  myWidgets << aFacesOver;
  myPredicates << aFilterMgr->CreateOverConstrainedFace();
  QLabel* anAspectRatioLab = new QLabel( tr( "ASPECT_RATIO_HISTOGRAM" ), this );
  myPlot = createPlot( this );
  myAspectRatio = aFilterMgr->CreateAspectRatio();
 
  // volumes info
  QLabel* aVolumesLab = createLabel( tr( "VOLUMES_INFO" ), this, Bold );
  QLabel* aVolumesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_VOLUMES" ), this );
  QLabel* aVolumesDouble = createField( this, "ctrlVolumesDouble" );
  myWidgets << aVolumesDouble;
  myPredicates << aFilterMgr->CreateEqualVolumes();
  QLabel* aVolumesOverLab = new QLabel( tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ), this );
  QLabel* aVolumesOver = createField( this, "ctrlVolumesOver" );
  myWidgets << aVolumesOver;
  myPredicates << aFilterMgr->CreateOverConstrainedVolume();
  QLabel* anAspectRatio3DLab = new QLabel( tr( "ASPECT_RATIO_3D_HISTOGRAM" ), this );
  myPlot3D = createPlot( this );
  myAspectRatio3D = aFilterMgr->CreateAspectRatio3D();

  QToolButton* aFreeNodesBtn = new QToolButton( this );
  aFreeNodesBtn->setIcon(aComputeIcon);
  myButtons << aFreeNodesBtn;       //0

  QToolButton* aNodesNbConnBtn = new QToolButton( this );
  aNodesNbConnBtn->setIcon(aComputeIcon);
  myButtons << aNodesNbConnBtn;     //1

  QToolButton* aDoubleNodesBtn = new QToolButton( this );
  aDoubleNodesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleNodesBtn;     //2

  QToolButton* aDoubleEdgesBtn = new QToolButton( this );
  aDoubleEdgesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleEdgesBtn;     //3

  QToolButton* aDoubleFacesBtn = new QToolButton( this );
  aDoubleFacesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleFacesBtn;     //4

  QToolButton* aOverContFacesBtn = new QToolButton( this );
  aOverContFacesBtn->setIcon(aComputeIcon);
  myButtons << aOverContFacesBtn;   //5

  QToolButton* aComputeFaceBtn = new QToolButton( this );
  aComputeFaceBtn->setIcon(aComputeIcon);
  myButtons << aComputeFaceBtn;     //6

  QToolButton* aDoubleVolumesBtn = new QToolButton( this );
  aDoubleVolumesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleVolumesBtn;   //7

  QToolButton* aOverContVolumesBtn = new QToolButton( this );
  aOverContVolumesBtn->setIcon(aComputeIcon);
  myButtons << aOverContVolumesBtn; //8

  QToolButton* aComputeVolumeBtn = new QToolButton( this );
  aComputeVolumeBtn->setIcon(aComputeIcon);
  myButtons << aComputeVolumeBtn;   //9

  connect( aComputeFaceBtn,   SIGNAL( clicked() ), this, SLOT( computeAspectRatio() ) );
  connect( aComputeVolumeBtn, SIGNAL( clicked() ), this, SLOT( computeAspectRatio3D() ) );
  connect( aFreeNodesBtn,     SIGNAL( clicked() ), this, SLOT( computeFreeNodesInfo() ) );
  connect( aNodesNbConnBtn,   SIGNAL( clicked() ), this, SLOT( computeNodesNbConnInfo() ) );
  connect( aDoubleNodesBtn,   SIGNAL( clicked() ), this, SLOT( computeDoubleNodesInfo() ) );
  connect( aDoubleEdgesBtn,   SIGNAL( clicked() ), this, SLOT( computeDoubleEdgesInfo() ) );
  connect( aDoubleFacesBtn,   SIGNAL( clicked() ), this, SLOT( computeDoubleFacesInfo() ) );
  connect( aOverContFacesBtn, SIGNAL( clicked() ), this, SLOT( computeOverConstrainedFacesInfo() ) );
  connect( aDoubleVolumesBtn, SIGNAL( clicked() ), this, SLOT( computeDoubleVolumesInfo() ) );
  connect( aOverContVolumesBtn,SIGNAL( clicked() ), this, SLOT( computeOverConstrainedVolumesInfo() ) );
  connect( myToleranceWidget, SIGNAL( valueChanged( double ) ), this, SLOT( setTolerance( double ) ) );

  l->addWidget( aNameLab,           0, 0 );       //0
  l->addWidget( aName,              0, 1, 1, 2 ); //1
  l->addWidget( aNodesLab,          1, 0, 1, 3 ); //2
  l->addWidget( aNodesFreeLab,      2, 0 );       //3
  l->addWidget( aNodesFree,         2, 1 );       //4
  l->addWidget( aFreeNodesBtn,      2, 2 );       //5
  l->addWidget( aNodesNbConnLab,    3, 0 );       //6
  l->addWidget( aNodesNbConn,       3, 1 );       //7
  l->addWidget( aNodesNbConnBtn,    3, 2 );       //8
  l->addWidget( aNodesDoubleLab,    4, 0 );       //9
  l->addWidget( aNodesDouble,       4, 1 );       //10
  l->addWidget( aDoubleNodesBtn,    4, 2 );       //11
  l->addWidget( aToleranceLab,      5, 0 );       //12
  l->addWidget( myToleranceWidget,  5, 1 );       //13
  l->addWidget( anEdgesLab,         6, 0, 1, 3 ); //14
  l->addWidget( anEdgesDoubleLab,   7, 0 );       //15
  l->addWidget( anEdgesDouble,      7, 1 );       //16
  l->addWidget( aDoubleEdgesBtn,    7, 2 );       //17
  l->addWidget( aFacesLab,          8, 0, 1, 3 ); //18
  l->addWidget( aFacesDoubleLab,    9, 0 );       //19
  l->addWidget( aFacesDouble,       9, 1 );       //20
  l->addWidget( aDoubleFacesBtn,    9, 2 );       //21
  l->addWidget( aFacesOverLab,      10, 0 );      //22
  l->addWidget( aFacesOver,         10, 1 );      //23
  l->addWidget( aOverContFacesBtn,  10, 2 );      //24
  l->addWidget( anAspectRatioLab,   11, 0 );      //25
  l->addWidget( aComputeFaceBtn,    11, 2 );      //26
  l->addWidget( myPlot,             12, 0, 1, 3 );//27
  l->addWidget( aVolumesLab,        13, 0, 1, 3 );//28
  l->addWidget( aVolumesDoubleLab,  14, 0 );      //29
  l->addWidget( aVolumesDouble,     14, 1 );      //30
  l->addWidget( aDoubleVolumesBtn,  14, 2 );      //31
  l->addWidget( aVolumesOverLab,    15, 0 );      //32
  l->addWidget( aVolumesOver,       15, 1 );      //33
  l->addWidget( aOverContVolumesBtn,15, 2 );      //34
  l->addWidget( anAspectRatio3DLab, 16, 0 );      //35
  l->addWidget( aComputeVolumeBtn,  16, 2 );      //36
  l->addWidget( myPlot3D,           17, 0, 1, 3 );//37
 
  l->setColumnStretch(  0,  0 );
  l->setColumnStretch(  1,  5 );
  l->setRowStretch   ( 12,  5 );
  l->setRowStretch   ( 17,  5 );
  l->setRowStretch   ( 18,  1 );

  clearInternal();
}

/*!
  \brief Destructor.
*/
SMESHGUI_CtrlInfo::~SMESHGUI_CtrlInfo()
{
}

/*!
  \brief Create plot widget.
  \param parent Parent widget.
  \return New plot widget.
*/
QwtPlot* SMESHGUI_CtrlInfo::createPlot( QWidget* parent )
{
  QwtPlot* aPlot = new QwtPlot( parent );
  aPlot->setMinimumSize( 100, 100 );
  QFont xFont = aPlot->axisFont( QwtPlot::xBottom );
  xFont.setPointSize( 5 );
  QFont yFont = aPlot->axisFont( QwtPlot::yLeft );
  yFont.setPointSize( 5 );
  aPlot->setAxisFont( QwtPlot::xBottom, xFont );
  aPlot->setAxisFont( QwtPlot::yLeft, yFont );
  aPlot->replot();
  return aPlot;
}

/*!
  \brief Show information on given object.
  \param proxy Object to show information on (mesh, sub-mesh, group, ID source).
*/
void SMESHGUI_CtrlInfo::showInfo( const SMESHGUI_SelectionProxy& proxy )
{
  clearInternal();

  if ( !proxy )
    return;

  myProxy = proxy;
  SMESH::SMESH_IDSource_var obj = proxy.object();

  myWidgets[0]->setText( proxy.name() );

  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();
  if ( mesh->_is_nil() ) return;

  const bool meshLoaded = mesh->IsLoaded();
  if ( !meshLoaded ) // mesh not yet loaded from the hdf file
    // enable Compute buttons, just in case obj->GetNbElementsByType() fails
    for ( int i = 0; i < myButtons.count(); ++i )
      myButtons[i]->setEnabled( true );

  SMESH::long_array_var nbElemsByType = obj->GetNbElementsByType();
  if ( ! &nbElemsByType.in() ) return;

  const CORBA::Long ctrlLimit =
    meshLoaded ? SMESHGUI::resourceMgr()->integerValue( "SMESH", "info_controls_limit", 3000 ) : -1;

  // nodes info
  const CORBA::Long nbNodes =   nbElemsByType[ SMESH::NODE ];
  // const CORBA::Long nbElems = ( nbElemsByType[ SMESH::EDGE ] +
  //                               nbElemsByType[ SMESH::FACE ] +
  //                               nbElemsByType[ SMESH::VOLUME ] );
  if ( nbNodes > 0 ) {
    if ( nbNodes <= ctrlLimit ) {
      // free nodes
      computeFreeNodesInfo();
      // node connectivity number
      computeNodesNbConnInfo();
      // double nodes
      computeDoubleNodesInfo();
    }
    else {
      myButtons[0]->setEnabled( true );
      myButtons[1]->setEnabled( true );
      myButtons[2]->setEnabled( true );
    }
  }
  else {
    for( int i=2; i<=13; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
  }

  // edges info
  if ( nbElemsByType[ SMESH::EDGE ] > 0 ) {
    // double edges
    if( nbElemsByType[ SMESH::EDGE ] <= ctrlLimit )
      computeDoubleEdgesInfo();
    else
      myButtons[3]->setEnabled( true );
  }
  else {
    for( int i=14; i<=17; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
  }

  // faces info
  if ( nbElemsByType[ SMESH::FACE ] > 0 ) {
    if ( nbElemsByType[ SMESH::FACE ] <= ctrlLimit ) {
      // double faces
      computeDoubleFacesInfo();
      // over constrained faces
      computeOverConstrainedFacesInfo();
      // aspect Ratio histogram
      computeAspectRatio();
    }
    else {
      myButtons[4]->setEnabled( true );
      myButtons[5]->setEnabled( true );
      myButtons[6]->setEnabled( true );
    }
#ifdef DISABLE_PLOT2DVIEWER
    for( int i=25; i<=27; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
#endif
  }
  else {
    for( int i=18; i<=27; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
  }

  // volumes info
  if ( nbElemsByType[ SMESH::VOLUME ] > 0 ) {
    if ( nbElemsByType[ SMESH::VOLUME ] <= ctrlLimit ) {
      // double volumes
      computeDoubleVolumesInfo();
      // over constrained volumes
      computeOverConstrainedVolumesInfo();
      // aspect Ratio 3D histogram
      computeAspectRatio3D();
    }
    else {
      myButtons[7]->setEnabled( true );
      myButtons[8]->setEnabled( true );
      myButtons[9]->setEnabled( true );
    }
#ifdef DISABLE_PLOT2DVIEWER
    for( int i=35; i<=37; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
#endif
  }
  else {
    for( int i=28; i<=37; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
  }
}

//================================================================================
/*!
 * \brief Computes and shows nb of elements satisfying a given predicate
 *  \param [in] ft - a predicate type (SMESH::FunctorType)
 *  \param [in] iBut - index of one of myButtons to disable
 *  \param [in] iWdg - index of one of myWidgets to show the computed number
 */
//================================================================================

void SMESHGUI_CtrlInfo::computeNb( int ft, int iBut, int iWdg )
{
  myButtons[ iBut ]->setEnabled( false );
  myWidgets[ iWdg ]->setText( "" );

  if ( !myProxy )
    return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  if ( !mesh->_is_nil() && !mesh->IsLoaded() )
  {
    mesh->Load();
    showInfo( myProxy ); // try to show all values
    if ( !myWidgets[ iWdg ]->text().isEmpty() )
      return; // <ft> predicate already computed
  }
  // look for a predicate of type <ft>
  for ( int i = 0; i < myPredicates.count(); ++i )
    if ( myPredicates[i]->GetFunctorType() == ft )
    {
      CORBA::Long nb = myPredicates[i]->NbSatisfying( obj );
      myWidgets[ iWdg ]->setText( QString::number( nb ) );
    }
}

void SMESHGUI_CtrlInfo::computeFreeNodesInfo()
{
  computeNb( SMESH::FT_FreeNodes, 0, 1 );
}

void SMESHGUI_CtrlInfo::computeDoubleNodesInfo()
{
  computeNb( SMESH::FT_EqualNodes, 2, 3 );
}

void SMESHGUI_CtrlInfo::computeDoubleEdgesInfo()
{
  computeNb( SMESH::FT_EqualEdges, 3, 4 );
}

void SMESHGUI_CtrlInfo::computeDoubleFacesInfo()
{
  computeNb( SMESH::FT_EqualFaces, 4, 5 );
}

void SMESHGUI_CtrlInfo::computeOverConstrainedFacesInfo()
{
  computeNb( SMESH::FT_OverConstrainedFace, 5, 6 );
}

void SMESHGUI_CtrlInfo::computeDoubleVolumesInfo()
{
  computeNb( SMESH::FT_EqualVolumes, 7, 7 );
}

void SMESHGUI_CtrlInfo::computeOverConstrainedVolumesInfo()
{
  computeNb( SMESH::FT_OverConstrainedVolume, 8, 8 );
}

void SMESHGUI_CtrlInfo::computeNodesNbConnInfo()
{
  myButtons[ 1 ]->setEnabled( false );
  myWidgets[ 2 ]->setText( "" );

  if ( !myProxy )
    return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  if ( !mesh->IsLoaded() )
  {
    mesh->Load();
    showInfo( myProxy ); // try to show all values
    if ( !myWidgets[ 2 ]->text().isEmpty() )
      return; // already computed
  }
  myNodeConnFunctor->SetMesh( mesh );
  SMESH::Histogram_var histogram =
    myNodeConnFunctor->GetLocalHistogram( 1, /*isLogarithmic=*/false, obj );

  myWidgets[ 2 ]->setText( QString::number( histogram[0].max ) );
}

void SMESHGUI_CtrlInfo::computeAspectRatio()
{
#ifndef DISABLE_PLOT2DVIEWER
  myButtons[6]->setEnabled( false );

  if ( !myProxy )
    return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  Plot2d_Histogram* aHistogram = getHistogram( myAspectRatio );
  if ( aHistogram && !aHistogram->isEmpty() ) {
    QwtPlotItem* anItem = aHistogram->createPlotItem();
    anItem->attach( myPlot );
    myPlot->replot();
  }
  delete aHistogram;
#endif
}

void SMESHGUI_CtrlInfo::computeAspectRatio3D()
{
#ifndef DISABLE_PLOT2DVIEWER
  myButtons[9]->setEnabled( false );

  if ( !myProxy )
    return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  Plot2d_Histogram* aHistogram = getHistogram( myAspectRatio3D );
  if ( aHistogram && !aHistogram->isEmpty() ) {
    QwtPlotItem* anItem = aHistogram->createPlotItem();
    anItem->attach( myPlot3D );
    myPlot3D->replot();
  }
  delete aHistogram;
#endif
}

/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_CtrlInfo::clearInternal()
{
  for( int i=0; i<=37; i++)
    dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( true );
  for( int i=0; i<=9; i++)
    myButtons[i]->setEnabled( false );
  myPlot->detachItems();
  myPlot3D->detachItems();
  myPlot->replot();
  myPlot3D->replot();
  myWidgets[0]->setText( QString() );
  for ( int i = 1; i < myWidgets.count(); i++ )
    myWidgets[i]->setText( "" );
}

void SMESHGUI_CtrlInfo::setTolerance( double theTolerance )
{
  //SMESH::long_array_var anElems = getElementsByType( SMESH::NODE );
  myButtons[1]->setEnabled( true );
  myWidgets[2]->setText("");
}

#ifndef DISABLE_PLOT2DVIEWER
Plot2d_Histogram* SMESHGUI_CtrlInfo::getHistogram( SMESH::NumericalFunctor_ptr aNumFun )
{
  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  if ( !mesh->IsLoaded() )
    mesh->Load();
  aNumFun->SetMesh( mesh );

  CORBA::Long cprecision = 6;
  if ( SMESHGUI::resourceMgr()->booleanValue( "SMESH", "use_precision", false ) )
    cprecision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "controls_precision", -1 );
  aNumFun->SetPrecision( cprecision );

  int nbIntervals = SMESHGUI::resourceMgr()->integerValue( "SMESH", "scalar_bar_num_colors", false );

  SMESH::Histogram_var histogramVar = aNumFun->GetLocalHistogram( nbIntervals,
                                                                  /*isLogarithmic=*/false,
                                                                  obj );
  Plot2d_Histogram* aHistogram = new Plot2d_Histogram();
  aHistogram->setColor( palette().color( QPalette::Highlight ) );
  if ( &histogramVar.in() )
  {
    for ( size_t i = 0, nb = histogramVar->length(); i < nb; i++ )
      aHistogram->addPoint( 0.5 * ( histogramVar[i].min + histogramVar[i].max ), histogramVar[i].nbEvents );
    if ( histogramVar->length() >= 2 )
      aHistogram->setWidth( ( histogramVar[0].max - histogramVar[0].min ) * 0.8 );
  }
  return aHistogram;
}
#endif

void SMESHGUI_CtrlInfo::saveInfo( QTextStream &out ) {
  out << ruler( 20 ) << endl;
  out << tr( "CTRL_INFO"  ) << endl;
  out << ruler( 20 ) << endl;
  out << tr( "NAME_LAB" ) << "  " << myWidgets[0]->text() << endl;
  out << tr( "NODES_INFO" ) << endl;
  out << indent() << tr( "NUMBER_OF_THE_FREE_NODES" ) << ": " << myWidgets[1]->text() << endl;
  out << indent() << tr( "NUMBER_OF_THE_DOUBLE_NODES" ) << ": " << myWidgets[2]->text() << endl;
  out << tr( "EDGES_INFO" ) << endl;
  out << indent() << tr( "NUMBER_OF_THE_DOUBLE_EDGES" ) << ": " << myWidgets[3]->text() << endl;
  out << tr( "FACES_INFO" ) << endl;
  out << indent() << tr( "NUMBER_OF_THE_DOUBLE_FACES" ) << ": " << myWidgets[4]->text() << endl;
  out << indent() << tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ) << ": " << myWidgets[5]->text() << endl;
  out << tr( "VOLUMES_INFO" ) << endl;
  out << indent() << tr( "NUMBER_OF_THE_DOUBLE_VOLUMES" ) << ": " << myWidgets[6]->text() << endl;
  out << indent() << tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ) << ": " << myWidgets[7]->text() << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_CtrlInfoDlg
/// \brief Overall Mesh Quality dialog.
/// \todo Add selection button to reactivate selection on move from other dlg.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_CtrlInfoDlg::SMESHGUI_CtrlInfoDlg( QWidget* parent )
  : QDialog( parent )
{
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "CTRL_INFO" ) );
  setMinimumSize( 400, 600 );

  myCtrlInfo = new SMESHGUI_CtrlInfo( this );
  
  // buttons
  QPushButton* okBtn = new QPushButton( tr( "SMESH_BUT_OK" ), this );
  okBtn->setAutoDefault( true );
  okBtn->setDefault( true );
  okBtn->setFocus();
  QPushButton* dumpBtn = new QPushButton( tr( "BUT_DUMP_MESH" ), this );
  dumpBtn->setAutoDefault( true );
  QPushButton* helpBtn = new QPushButton( tr( "SMESH_BUT_HELP" ), this );
  helpBtn->setAutoDefault( true );

  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->setSpacing( SPACING );
  btnLayout->setMargin( 0 );

  btnLayout->addWidget( okBtn );
  btnLayout->addWidget( dumpBtn );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( helpBtn );

  QVBoxLayout* l = new QVBoxLayout ( this );
  l->setMargin( 0 );
  l->setSpacing( SPACING );
  l->addWidget( myCtrlInfo );
  l->addLayout( btnLayout );

  connect( okBtn,   SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( dumpBtn, SIGNAL( clicked() ), this, SLOT( dump() ) );
  connect( helpBtn, SIGNAL( clicked() ), this, SLOT( help() ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( deactivate() ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( reject() ) );

  updateSelection();
}

/*!
  \brief Destructor
*/
SMESHGUI_CtrlInfoDlg::~SMESHGUI_CtrlInfoDlg()
{
}

/*!
  \brief Show mesh quality information on given object.
  \param io Interactive object.
*/
void SMESHGUI_CtrlInfoDlg::showInfo( const Handle(SALOME_InteractiveObject)& io )
{  
  if ( !io.IsNull() )
    showInfo( SMESHGUI_SelectionProxy( io ) );
}

/*!
  \brief Show mesh quality information on given object.
  \param proxy Selection proxy.
*/
void SMESHGUI_CtrlInfoDlg::showInfo( const SMESHGUI_SelectionProxy& proxy )
{
  SUIT_OverrideCursor wc;

  if ( !proxy )
    return;

  myProxy = proxy;
  myCtrlInfo->showInfo( proxy );
}

/*!
  \brief Show mesh information
*/
void SMESHGUI_CtrlInfoDlg::updateInfo()
{
  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 )
    showInfo( selected.First() );
  else
    showInfo( myProxy );
}

/*!
  \brief Perform clean-up actions on the dialog box closing.
*/
void SMESHGUI_CtrlInfoDlg::reject()
{
  SMESH::SetPointRepresentation( false );
  QDialog::reject();
}

/*!
  \brief Setup selection mode depending on the current dialog box state.
*/
void SMESHGUI_CtrlInfoDlg::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();
  disconnect( selMgr, 0, this, 0 );
  SMESH::SetPointRepresentation( false );  
  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
  updateInfo();  
}

/*!
  \brief Deactivate dialog box.
*/
void SMESHGUI_CtrlInfoDlg::deactivate()
{
  disconnect( SMESHGUI::selectionMgr(), SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
}

/*!
  \brief Dump information to file.
*/
void SMESHGUI_CtrlInfoDlg::dump()
{
  DumpFileDlg fd( this, false );
  fd.setWindowTitle( tr( "SAVE_INFO" ) );
  fd.setNameFilters( QStringList() << tr( "TEXT_FILES" ) );
  if ( fd.exec() == QDialog::Accepted )
  {
    QString fileName = fd.selectedFile();
    if ( !fileName.isEmpty() ) {
      QFile file( fileName );
      if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
        return;

      QTextStream out( &file );
      myCtrlInfo->saveInfo( out );
    }
  }
}

/*!
  \brief Show documentation on dialog.
*/
void SMESHGUI_CtrlInfoDlg::help()
{
  SMESH::ShowHelpFile( "mesh_infos.html#mesh-quality-info-anchor" );
}
