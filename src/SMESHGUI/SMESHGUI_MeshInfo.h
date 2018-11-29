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
//  File   : SMESHGUI_MeshInfo.h
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#ifndef SMESHGUI_MESHINFO_H
#define SMESHGUI_MESHINFO_H

#include "SMESH_SMESHGUI.hxx"
#include "SMESH_ControlsDef.hxx"
#include "SMESHGUI_SelectionProxy.h"

#ifndef DISABLE_PLOT2DVIEWER
  #include <Plot2d_Histogram.h>
#else
  #include <qwt_plot.h>
#endif

#include <QDialog>
#include <QList>
#include <QMap>
#include <QSet>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Filter)

#include <SALOME_InteractiveObject.hxx>
#include <SALOME_GenericObj_wrap.hxx>

class QAbstractButton;
class QButtonGroup;
class QCheckBox;
class QContextMenuEvent;
class QLabel;
class QLineEdit;
class QTabWidget;
class QTextBrowser;
class QTreeWidget;
class QTreeWidgetItem;
class SMDS_MeshElement;
class SMDS_MeshNode;
class SMESHGUI_IdPreview;
class SMESHGUI_SpinBox;

class ExtraWidget;
class GroupCombo;

class SMESHGUI_EXPORT SMESHGUI_Info : public QWidget
{
public:
  SMESHGUI_Info( QWidget* = 0 );

  virtual void saveInfo( QTextStream& ) = 0;

protected:
  enum { Bold = 0x01, Italic = 0x02, AllColumns = 0x80 };
  QFont fontFromOptions( int );
  void setFontAttributes( QWidget*, int );
  void setFontAttributes( QTreeWidgetItem*, int );
  QLabel* createLabel( QWidget*, int = 0 );
  QLabel* createLabel( const QString&, QWidget*, int = 0 );
  QLabel* createField( QWidget*, const QString& = QString() );
  QLabel* createField( QWidget*, int, const QString& = QString() );
  QWidget* createSeparator( QWidget*, Qt::Orientation = Qt::Horizontal );
};

class SMESHGUI_EXPORT SMESHGUI_BaseInfo : public SMESHGUI_Info
{
  Q_OBJECT;
  
  enum
  {
    iStart,
    iObjectStart = iStart,
    iName = iObjectStart,
    iObject,
    iObjectEnd,
    iNodesStart = iObjectEnd,
    iNodes,
    iNodesEnd,
    iElementsStart = iNodesEnd,
    iElementsTitleStart = iElementsStart,
    iElementsTitle,
    iElementsTitleEnd,
    iElementsTotalStart = iElementsTitleEnd,
    iElementsTotal,
    iElementsTotalEnd,
    i0DStart = iElementsTotalEnd,
    i0D,
    i0DEnd,
    iBallsStart = i0DEnd,
    iBalls,
    iBallsEnd,
    i1DStart = iBallsEnd,
    i1D,
    i1DEnd,
    i2DStart = i1DEnd,
    i2D,
    i2DTriangles,
    i2DQuadrangles,
    i2DPolygons,
    i2DEnd,
    i3DStart = i2DEnd,
    i3D,
    i3DTetrahedrons,
    i3DHexahedrons,
    i3DPyramids,
    i3DPrisms,
    i3DHexaPrisms,
    i3DPolyhedrons,
    i3DEnd,
    iElementsEnd = i3DEnd,
    iEnd,
    iOther = iEnd
  };

  enum
  {
    iLabel,
    iSingle,
    iTotal = iSingle,
    iLinear,
    iQuadratic,
    iBiQuadratic,
    iNbColumns
  };

  typedef QMap<int, QWidget*> wlist;
  typedef QMap<int, wlist> iwlist;

public:
  SMESHGUI_BaseInfo( QWidget* = 0 );
  ~SMESHGUI_BaseInfo();

  void showInfo( const SMESHGUI_SelectionProxy& );
  void clear();
  void saveInfo( QTextStream& );

private:
  QWidget* addWidget( QWidget*, int, int, int = 1 );
  QWidget* widget( int, int ) const;
  QString value( int, int ) const;
  void setFieldsVisible( int, int, bool );

private slots:
  void updateInfo();
  void loadMesh();

private:
  iwlist myWidgets;
  SMESHGUI_SelectionProxy myProxy;
};

class SMESHGUI_EXPORT SMESHGUI_ElemInfo : public SMESHGUI_Info
{
  Q_OBJECT;

public:
  SMESHGUI_ElemInfo( QWidget* = 0 );
  ~SMESHGUI_ElemInfo();

  void showInfo( const SMESHGUI_SelectionProxy&, uint, bool );
  void showInfo( const SMESHGUI_SelectionProxy&, QSet<uint>, bool );
  void showInfo( const SMESHGUI_SelectionProxy& );
  void clear();

  gp_XYZ getGravityCenter( const SMDS_MeshElement* );

protected:
  enum { ShowNone, ShowNodes, ShowElements };

  struct XYZ
  {
    double myX, myY, myZ;
    XYZ() { myX = myY = myZ = 0.0; }
    XYZ(double x, double y, double z) { myX = x; myY = y; myZ = z; }
    void add( double x, double y, double z ) { myX += x; myY += y; myZ += z; }
    void divide( double a ) { if ( a != 0.) { myX /= a; myY /= a; myZ /= a; } }
    double x() const { return myX; }
    double y() const { return myY; }
    double z() const { return myZ; }
    operator gp_XYZ() const { return gp_XYZ( myX, myY, myZ ); }
  };
  typedef QMap< int, QList<int> > Connectivity;

  QWidget* frame() const;

  int what() const;
  SMESHGUI_SelectionProxy proxy() const;

  virtual void information( const QList<uint>& ) = 0;
  virtual void clearInternal();

  Connectivity nodeConnectivity( const SMDS_MeshNode* );
  QString formatConnectivity( Connectivity, int );
  XYZ gravityCenter( const SMDS_MeshElement* );
  XYZ normal( const SMDS_MeshElement* );

signals:
  void itemInfo( int );
  void itemInfo( const QString& );

private slots:
  void showPrevious();
  void showNext();
  void updateControls();

private:
  QWidget* myFrame;
  ExtraWidget* myExtra;
  SMESHGUI_SelectionProxy myProxy;
  int myWhat;
  QList<uint> myIDs;
  int myIndex;
};

class SMESHGUI_EXPORT SMESHGUI_SimpleElemInfo : public SMESHGUI_ElemInfo
{
  Q_OBJECT;

public:
  SMESHGUI_SimpleElemInfo( QWidget* = 0 );
  void saveInfo( QTextStream& );

protected:
  void information( const QList<uint>& );
  void clearInternal();

private:
  QTextBrowser* myInfo;
};

class SMESHGUI_EXPORT SMESHGUI_TreeElemInfo : public SMESHGUI_ElemInfo
{
  Q_OBJECT;

  class ItemDelegate;

public:
  SMESHGUI_TreeElemInfo( QWidget* = 0 );
  void saveInfo( QTextStream& );

protected:
  void contextMenuEvent( QContextMenuEvent* );
  void information( const QList<uint>& );
  void nodeInfo( const SMDS_MeshNode*, int, int, QTreeWidgetItem* );
  void clearInternal();

private slots:
  void itemDoubleClicked( QTreeWidgetItem*, int );
  void saveExpanded( QTreeWidgetItem* );
  
private:
  QTreeWidgetItem* createItem( QTreeWidgetItem* = 0, int = 0 );
  QString expandedResource( QTreeWidgetItem* );
  
private:
  QTreeWidget* myInfo;
};

class InfoComputor: public QObject
{
  Q_OBJECT;

public:
  enum { GrpSize, GrpNbNodes };
  
  InfoComputor( QObject*, const SMESHGUI_SelectionProxy&, int );

signals:
  void computed();

public slots:
  void compute();

private:
  SMESHGUI_SelectionProxy myProxy;
  int myOperation;
};

class SMESHGUI_EXPORT SMESHGUI_AddInfo : public SMESHGUI_Info
{
  Q_OBJECT;

public:
  SMESHGUI_AddInfo( QWidget* = 0 );
  ~SMESHGUI_AddInfo();

  void showInfo( const SMESHGUI_SelectionProxy& );
  void clear();
  void saveInfo( QTextStream& );

private slots:
  void updateInfo();
  void showPreviousGroups();
  void showNextGroups();
  void showPreviousSubMeshes();
  void showNextSubMeshes();

private:
  QTreeWidgetItem* createItem( QTreeWidgetItem* = 0, int = 0 );
  void meshInfo( const SMESHGUI_SelectionProxy&, QTreeWidgetItem* );
  void subMeshInfo( const SMESHGUI_SelectionProxy&, QTreeWidgetItem* );
  void groupInfo( const SMESHGUI_SelectionProxy&, QTreeWidgetItem* );

  void showGroups();
  void showSubMeshes();

private:
  SMESHGUI_SelectionProxy myProxy;
  QTreeWidget* myTree;
  QList<InfoComputor*> myComputors;
  QList<SMESHGUI_SelectionProxy> myGroups;
  QList<SMESHGUI_SelectionProxy> mySubMeshes;
};

class SMESHGUI_EXPORT SMESHGUI_CtrlInfo : public SMESHGUI_Info
{
  Q_OBJECT;

public:
  SMESHGUI_CtrlInfo( QWidget* = 0 );
  ~SMESHGUI_CtrlInfo();

  void showInfo( const SMESHGUI_SelectionProxy& );
  void saveInfo( QTextStream& );

private:
  enum ObjectType { Mesh, SubMesh, Group };
  QwtPlot* createPlot( QWidget* );
  void clearInternal();
#ifndef DISABLE_PLOT2DVIEWER
  Plot2d_Histogram* getHistogram( SMESH::NumericalFunctor_ptr );
#endif
  void computeNb( int, int, int );

private slots:
  void computeAspectRatio();
  void computeAspectRatio3D();
  void computeFreeNodesInfo();
  void computeNodesNbConnInfo();
  void computeDoubleNodesInfo();
  void computeDoubleEdgesInfo();
  void computeDoubleFacesInfo();
  void computeOverConstrainedFacesInfo();
  void computeDoubleVolumesInfo();
  void computeOverConstrainedVolumesInfo();
  void setTolerance( double );

private:
  typedef SALOME::GenericObj_wrap< SMESH::Predicate > TPredicate;
  typedef SALOME::GenericObj_wrap< SMESH::NumericalFunctor > TNumFunctor;
  SMESHGUI_SelectionProxy myProxy;
  ObjectType myObjectType;
  SMESHGUI_SpinBox* myToleranceWidget;
  QList<QLabel*> myWidgets;
  QwtPlot* myPlot;
  QwtPlot* myPlot3D;
  QList<QAbstractButton*> myButtons;
  QList<TPredicate> myPredicates;
  TNumFunctor myAspectRatio, myAspectRatio3D, myNodeConnFunctor;
};

class SMESHGUI_EXPORT SMESHGUI_MeshInfoDlg : public QDialog
{ 
  Q_OBJECT;

  enum { NodeMode, ElemMode, GroupMode };

public:
  //! Information type
  enum
  { 
    BaseInfo,  //!< base mesh information
    ElemInfo,  //!< mesh element information
    AddInfo,   //!< additional information
    CtrlInfo   //!< controls information
  };

  SMESHGUI_MeshInfoDlg( QWidget* = 0, int = BaseInfo );
  ~SMESHGUI_MeshInfoDlg();

  void showInfo( const Handle(SALOME_InteractiveObject)& );
  void reject();

protected:
  void keyPressEvent( QKeyEvent* );

signals:
  void switchMode( int );

private slots:
  void help();
  void updateSelection();
  void updateInfo();
  void deactivate();
  void modeChanged();
  void idChanged();
  void idPreviewChange( bool );
  void showItemInfo( int );
  void showItemInfo( const QString& );
  void dump();

private:
  void showInfo( const SMESHGUI_SelectionProxy& );

  SMESHGUI_SelectionProxy myProxy;
  QTabWidget* myTabWidget;
  SMESHGUI_BaseInfo* myBaseInfo;
  SMESHGUI_ElemInfo* myElemInfo;
  SMESHGUI_AddInfo* myAddInfo;
  SMESHGUI_CtrlInfo* myCtrlInfo;
  QButtonGroup* myMode;
  QLineEdit* myID;
  QCheckBox* myIDPreviewCheck;
  GroupCombo* myGroups;
  SMESHGUI_IdPreview* myIDPreview;
};

class SMESHGUI_EXPORT SMESHGUI_CtrlInfoDlg : public QDialog
{ 
  Q_OBJECT;

public:
  SMESHGUI_CtrlInfoDlg( QWidget* = 0 );
  ~SMESHGUI_CtrlInfoDlg();

  void showInfo( const Handle(SALOME_InteractiveObject)& );
  void reject();

private slots:
  void updateInfo();
  void deactivate();
  void updateSelection();
  void help();
  void dump();

private:
  void showInfo( const SMESHGUI_SelectionProxy& );

  SMESHGUI_SelectionProxy myProxy;
  SMESHGUI_CtrlInfo* myCtrlInfo;
};

#endif // SMESHGUI_MESHINFO_H
