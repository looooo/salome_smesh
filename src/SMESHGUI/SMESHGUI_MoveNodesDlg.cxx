//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_MoveNodesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_MoveNodesDlg.h"
#include <SMESHGUI_SpinBox.h>

#include <SUIT_ResourceMgr.h>

#include <SMDSAbs_ElementType.hxx>

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>

//=================================================================================
// name    : SMESHGUI_MoveNodesDlg::SMESHGUI_MoveNodesDlg
// Purpose :
//=================================================================================
SMESHGUI_MoveNodesDlg::SMESHGUI_MoveNodesDlg()
: SMESHGUI_Dialog()
{
  setCaption( tr( "CAPTION" ) );

  QVBoxLayout* main = new QVBoxLayout( mainFrame() );

  QPixmap iconMoveNode( resMgr()->loadPixmap( "SMESH", tr( "ICON_DLG_MOVE_NODE" ) ) );
  
  QButtonGroup* aPixGrp = new QButtonGroup(1, Qt::Vertical, tr("MESH_NODE"), mainFrame() );
  aPixGrp->setExclusive(TRUE);
  QRadioButton* aRBut = new QRadioButton(aPixGrp);
  aRBut->setPixmap(iconMoveNode);
  aRBut->setChecked(TRUE);

  main->addWidget( aPixGrp );

  QGroupBox* anIdGrp = new QGroupBox( 3, Qt::Horizontal, tr( "SMESH_MOVE" ), mainFrame() );
  createObject( tr( "NODE_ID" ), anIdGrp, 0 );
  setNameIndication( 0, OneName );
  setObjectType( 0, prefix( "SMESH element" ) + SMDSAbs_Node, -1 );

  main->addWidget( anIdGrp );

  QGroupBox* aCoordGrp = new QGroupBox(1, Qt::Vertical, tr("SMESH_COORDINATES"), mainFrame() );
  new QLabel(tr("SMESH_X"), aCoordGrp);
  myX = new SMESHGUI_SpinBox(aCoordGrp);
  new QLabel(tr("SMESH_Y"), aCoordGrp);
  myY = new SMESHGUI_SpinBox(aCoordGrp);
  new QLabel(tr("SMESH_Z"), aCoordGrp);
  myZ = new SMESHGUI_SpinBox(aCoordGrp);

  main->addWidget( aCoordGrp );

  myX->RangeStepAndValidator(-999999.999, +999999.999, 25.0, 3);
  myY->RangeStepAndValidator(-999999.999, +999999.999, 25.0, 3);
  myZ->RangeStepAndValidator(-999999.999, +999999.999, 25.0, 3); 

  connect( myX, SIGNAL( valueChanged( double ) ), this, SIGNAL( valueChanged() ) );
  connect( myY, SIGNAL( valueChanged( double ) ), this, SIGNAL( valueChanged() ) );
  connect( myZ, SIGNAL( valueChanged( double ) ), this, SIGNAL( valueChanged() ) );
}

//=================================================================================
// name    : SMESHGUI_MoveNodesDlg::~SMESHGUI_MoveNodesDlg
// Purpose :
//=================================================================================
SMESHGUI_MoveNodesDlg::~SMESHGUI_MoveNodesDlg()
{
}

//=================================================================================
// name    : SMESHGUI_MoveNodesDlg::setCoords
// Purpose :
//=================================================================================
void SMESHGUI_MoveNodesDlg::setCoords( const double x, const double y, const double z )
{
  myX->setValue( x );
  myY->setValue( y );
  myZ->setValue( z );
}

//=================================================================================
// name    : SMESHGUI_MoveNodesDlg::coords
// Purpose :
//=================================================================================
void SMESHGUI_MoveNodesDlg::coords( double& x, double& y, double& z ) const
{
  x = myX->value();
  y = myY->value();
  z = myZ->value();
}
