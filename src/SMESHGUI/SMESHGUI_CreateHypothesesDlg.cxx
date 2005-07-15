//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_CreateHypothesesDlg.cxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#include "SMESHGUI_CreateHypothesesDlg.h"

#include "SUIT_Desktop.h"
#include "SUIT_ResourceMgr.h"

#include "SALOME_ListIteratorOfListIO.hxx"

#include "utilities.h"

// QT Includes
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qheader.h>

/*!
 * \brief Constructor
  * \param theIsAlgo - If TRUE when operator is used for creation of gypotheses
*
* Constructor does nothing
*/
SMESHGUI_CreateHypothesesDlg::SMESHGUI_CreateHypothesesDlg ( bool isAlgo )
: SMESHGUI_Dialog( 0, false, true, Apply | Cancel ),
  myIsAlgo(isAlgo)
{
  setCaption(  isAlgo ? tr( "SMESH_CREATE_ALGORITHMS"  ) : tr( "SMESH_CREATE_HYPOTHESES"  )  );
  setSizeGripEnabled( TRUE  );

  QGridLayout* SMESHGUI_CreateHypothesesDlgLayout = new QGridLayout( mainFrame() );
  SMESHGUI_CreateHypothesesDlgLayout->setSpacing( 6 );
  SMESHGUI_CreateHypothesesDlgLayout->setMargin( 11 );

  /***************************************************************/
  QGroupBox* GroupAlgorithms = new QGroupBox( mainFrame(), "GroupAlgorithms" );
  GroupAlgorithms->setTitle( isAlgo ? tr( "SMESH_AVAILABLE_ALGORITHMS" ) : tr( "SMESH_AVAILABLE_HYPOTHESES" ) );
  GroupAlgorithms->setColumnLayout( 0, Qt::Vertical );
  GroupAlgorithms->layout()->setSpacing( 0 );
  GroupAlgorithms->layout()->setMargin( 0 );

  QGridLayout* hypLayout = new QGridLayout( GroupAlgorithms->layout() );
  hypLayout->setGeometry( QRect( 12, 18, 139, 250 ) );
  hypLayout->setAlignment( Qt::AlignTop );
  hypLayout->setSpacing( 6 );
  hypLayout->setMargin( 11 );

  myList = new QListView( GroupAlgorithms, "myList" );
  myList->setMinimumSize( 400, 200 );
  myList->addColumn( "" );
  myList->header()->hide();
  myList->setSelectionMode( QListView::Single );
  myList->setResizeMode( QListView::AllColumns );
  myList->setRootIsDecorated( true );

  hypLayout->addWidget( myList, 0, 0 );
  SMESHGUI_CreateHypothesesDlgLayout->addWidget( GroupAlgorithms, 0, 0 );

  setButtonText( Apply, tr( "SMESH_BUT_CREATE" ) );

  // connect signals and slots
  connect( myList, SIGNAL( selectionChanged() ), SLOT( onHypSelected() ) );
  connect( myList, SIGNAL( doubleClicked( QListViewItem* ) ), SLOT( onDoubleClicked( QListViewItem* ) ) );

  // update button state
  onHypSelected();
}

/*!
 * \brief Destructor
*/
SMESHGUI_CreateHypothesesDlg::~SMESHGUI_CreateHypothesesDlg()
{
}

/*!
 * \brief Get Name of hypotheses or algorithm
  * \return Name of hypotheses or algorithm
*/
QString SMESHGUI_CreateHypothesesDlg::hypName() const
{
  QListViewItem* item = myList->selectedItem();
  return item ? item->text( 1 ) : "";
}

/*!
 * \brief Enable/Disable "Apply" button 
*
* Private slot called when selection in list box changed enables/disables "Apply" button
*/
void SMESHGUI_CreateHypothesesDlg::onHypSelected()
{
  QListViewItem* item = myList->selectedItem();
  setButtonEnabled( item && item->depth() > 0, Apply );
}

/*!
 * \brief Emits dlgApply signal
  * \param i - clicked item
*
* Private slot called when item of list box is double clicked emits dlgApply signal
*/
void SMESHGUI_CreateHypothesesDlg::onDoubleClicked ( QListViewItem* i )
{
  if ( i && i->depth() > 0 )
    emit dlgApply();
}

/*!
 * \brief Initialize dialog
  * \param theHypList - List of hypotheses
  * \param theHypList - Plugin names
  * \param theHypList - Labels
  * \param theHypList - Icons' identifiers
*
* Initializes dialog with parameters. This method is called by operation before showing
* dialog
*/
void SMESHGUI_CreateHypothesesDlg::init( const QStringList& theHypList,
                                         const QStringList& thePluginNames,
                                         const QStringList& theLabels,
                                         const QStringList& theIconIds )
{
  myList->clear();
  for ( int i = 0; i < theHypList.count(); ++i )
  {
    QListViewItem* parentItem = 0;
    QListViewItem* childItem = myList->firstChild();
    while ( childItem )
    {
      if ( childItem->text( 0 ) == thePluginNames[ i ] )
      {
        parentItem = childItem;
        break;
      }
      childItem = childItem->nextSibling();
    }
    
    if ( !parentItem )
      parentItem = new QListViewItem( myList, thePluginNames[ i ] );
    parentItem->setOpen( true );
    QListViewItem* aItem =
      new QListViewItem( parentItem, theLabels[ i ], theHypList[ i ] );
    QPixmap aPixMap( resMgr()->loadPixmap( "SMESH", tr( theIconIds[ i ] ) ) );
    if ( !aPixMap.isNull() )
      aItem->setPixmap( 0, aPixMap );
  }
}










