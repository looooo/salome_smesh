// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
/**
*  SMESH SMESHGUI
*
*  Copyright (C) 2005  CEA/DEN, EDF R&D
*
*
*
*  File   : SMESHGUI_ConvToQuadDlg.cxx
*  Module : SMESH
*/

#include <SMESHGUI_ConvToQuadDlg.h>
#include <SUIT_Session.h>

#include <qgroupbox.h>
#include <qlayout.h>
#include <qcheckbox.h>

SMESHGUI_ConvToQuadDlg::SMESHGUI_ConvToQuadDlg()
: SMESHGUI_Dialog( 0, false, true )
{

  setCaption( tr( "CAPTION" ) );

  // Create top controls  
  QGroupBox* aGrp = new QGroupBox( 3, Qt::Horizontal, mainFrame() );
  aGrp->setFrameStyle( QFrame::NoFrame );
  aGrp->setInsideMargin( 0 );

  // mesh
  createObject( tr( "MESH" ), aGrp, 0 );

  //Create check box
  myMedNdsOnGeom = new QCheckBox( tr( "MEDIUMNDS" ), mainFrame() );
  
  // Fill layout
  QVBoxLayout* aLay = new QVBoxLayout( mainFrame(), 5, 5 );
  aLay->addWidget( aGrp );
  aLay->addWidget( myMedNdsOnGeom );
}

SMESHGUI_ConvToQuadDlg::~SMESHGUI_ConvToQuadDlg()
{
}

bool SMESHGUI_ConvToQuadDlg::IsMediumNdsOnGeom() const
{
  return !myMedNdsOnGeom->isChecked();
}

void SMESHGUI_ConvToQuadDlg::SetMediumNdsOnGeom(const bool theCheck)
{
  myMedNdsOnGeom->setChecked(theCheck);
}

bool SMESHGUI_ConvToQuadDlg::IsEnabledCheck() const
{
  return myMedNdsOnGeom->isEnabled();
}

void SMESHGUI_ConvToQuadDlg::SetEnabledCheck(const bool theCheck)
{
  myMedNdsOnGeom->setEnabled(theCheck);
}
