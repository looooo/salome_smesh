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
//  File   : SMESHGUI_GroupOpDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_GroupOpDlg.h"

#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>

#define SPACING 5
#define MARGIN  10

/*!
 *  Class       : SMESHGUI_GroupOpDlg
 *  Description : Perform boolean operations on groups
 */

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::SMESHGUI_GroupOpDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_GroupOpDlg::SMESHGUI_GroupOpDlg( const int aMode )
: SMESHGUI_Dialog( 0, false, true )
{
  QString caption;
  if( aMode == 0 )
    caption = "UNION_OF_TWO_GROUPS";
  else if( aMode == 1 )
    caption = "INTERSECTION_OF_TWO_GROUPS";
  else
    caption = "CUT_OF_TWO_GROUPS";
  
  setCaption( tr( caption ) );

  QVBoxLayout* aDlgLay = new QVBoxLayout( mainFrame() );
//  aDlgLay->setMargin( MARGIN );
//  aDlgLay->setSpacing( SPACING );

  QGroupBox* aNameGrp = new QGroupBox(1, Qt::Vertical, tr("NAME"), mainFrame() );
  new QLabel(tr("RESULT_NAME"), aNameGrp);
  myNameEdit = new QLineEdit(aNameGrp);
  connect( myNameEdit, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( nameChanged( const QString& ) ) );

  QGroupBox* anArgGrp = new QGroupBox(3, Qt::Horizontal, tr("ARGUMENTS"), mainFrame() );

  createObject( aMode==2 ? tr( "MAIN_OBJECT" ) : tr( "OBJECT_1" ), anArgGrp, 1 );
  createObject( aMode==2 ? tr( "TOOL_OBJECT" ) : tr( "OBJECT_2" ), anArgGrp, 2 );

  aDlgLay->addWidget( aNameGrp );
  aDlgLay->addWidget( anArgGrp );
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::~SMESHGUI_GroupOpDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_GroupOpDlg::~SMESHGUI_GroupOpDlg()
{
}

//=======================================================================
// name    : setName
// Purpose : 
//=======================================================================
void SMESHGUI_GroupOpDlg::setName( const QString& name )
{
  myNameEdit->setText( name );
}

//=======================================================================
// name    : name
// Purpose : 
//=======================================================================
QString SMESHGUI_GroupOpDlg::name() const
{
  return myNameEdit->text().stripWhiteSpace();
}
