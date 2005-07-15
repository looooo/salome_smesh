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
//  File   : SMESHGUI_EditHypothesesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_EditHypothesesDlg.h"

#include <qlistbox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)

//=================================================================================
// function : SMESHGUI_EditHypothesesDlg()
// purpose  : Constructs a SMESHGUI_EditHypothesesDlg which is a child of 'parent', with the
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_EditHypothesesDlg::SMESHGUI_EditHypothesesDlg()
: SMESHGUI_Dialog( 0, false, true )
{
    setName("SMESHGUI_EditHypothesesDlg");
    setCaption(tr("SMESH_EDIT_HYPOTHESES"));
    
    QVBoxLayout* main = new QVBoxLayout( mainFrame(), 0, 5 );

    /***************************************************************/
    QGroupBox* GroupC1 = new QGroupBox(tr("SMESH_ARGUMENTS"), mainFrame(), "GroupC1");
    GroupC1->setColumnLayout( 3, Qt::Horizontal );

    createObject( tr("SMESH_OBJECT_MESHorSUBMESH"), GroupC1, MeshOrSubMesh );
    createObject( tr("SMESH_OBJECT_GEOM"), GroupC1, GeomShape );

    main->addWidget( GroupC1 );

    /***************************************************************/
    GroupHypotheses = new QGroupBox(tr("SMESH_HYPOTHESES"), mainFrame(), "GroupHypotheses");
    GroupHypotheses->setColumnLayout(0, Qt::Vertical);
    GroupHypotheses->layout()->setSpacing(0);
    GroupHypotheses->layout()->setMargin(0);
    QGridLayout* grid_3 = new QGridLayout(GroupHypotheses->layout());
    grid_3->setAlignment(Qt::AlignTop);
    grid_3->setSpacing(6);
    grid_3->setMargin(11);

    TextHypDefinition = new QLabel(tr("SMESH_AVAILABLE"), GroupHypotheses, "TextHypDefinition");
    grid_3->addWidget(TextHypDefinition, 0, 0);

    ListHypDefinition = new QListBox(GroupHypotheses, "ListHypDefinition");
    ListHypDefinition->setMinimumSize(100, 100);
    grid_3->addWidget(ListHypDefinition, 1, 0);

    TextHypAssignation = new QLabel(tr("SMESH_EDIT_USED"), GroupHypotheses, "TextHypAssignation");
    grid_3->addWidget(TextHypAssignation, 0, 1);

    ListHypAssignation = new QListBox(GroupHypotheses, "ListHypAssignation");
    ListHypAssignation->setMinimumSize(100, 100);
    grid_3->addWidget(ListHypAssignation, 1, 1);

    main->addWidget( GroupHypotheses );

    /***************************************************************/
    GroupAlgorithms = new QGroupBox(tr("SMESH_ADD_ALGORITHM"), mainFrame(), "GroupAlgorithms");
    GroupAlgorithms->setColumnLayout(0, Qt::Vertical);
    GroupAlgorithms->layout()->setSpacing(0);
    GroupAlgorithms->layout()->setMargin(0);
    QGridLayout* grid_4 = new QGridLayout(GroupAlgorithms->layout());
    grid_4->setAlignment(Qt::AlignTop);
    grid_4->setSpacing(6);
    grid_4->setMargin(11);

    TextAlgoDefinition = new QLabel(tr("SMESH_AVAILABLE"), GroupAlgorithms, "TextAlgoDefinition");
    grid_4->addWidget(TextAlgoDefinition, 0, 0);

    ListAlgoDefinition = new QListBox(GroupAlgorithms, "ListAlgoDefinition");
    ListAlgoDefinition->setMinimumSize(100, 100);
    grid_4->addWidget(ListAlgoDefinition, 1, 0);

    TextAlgoAssignation = new QLabel(tr("SMESH_EDIT_USED"), GroupAlgorithms, "TextAlgoAssignation");
    grid_4->addWidget(TextAlgoAssignation, 0, 1);

    ListAlgoAssignation = new QListBox(GroupAlgorithms, "ListAlgoAssignation");
    ListAlgoAssignation ->setMinimumSize(100, 100);
    grid_4->addWidget(ListAlgoAssignation, 1, 1);

    main->addWidget( GroupAlgorithms );

    connect(ListHypAssignation,  SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(removeItem(QListBoxItem*)));
    connect(ListAlgoAssignation, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(removeItem(QListBoxItem*)));

    connect(ListHypDefinition,  SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(addItem(QListBoxItem*)));
    connect(ListAlgoDefinition, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(addItem(QListBoxItem*)));
}

//=================================================================================
// function : ~SMESHGUI_EditHypothesesDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_EditHypothesesDlg::~SMESHGUI_EditHypothesesDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=======================================================================
// function : isOld()
// purpose  :
//=======================================================================
bool SMESHGUI_EditHypothesesDlg::isOld (QListBoxItem* hypItem)
{
  if (hypItem->rtti() == ListBoxIOR::RTTI_IOR) {
    ListBoxIOR* hyp = (ListBoxIOR*) hypItem;
    return (myMapOldHypos.find(hyp->GetIOR()) != myMapOldHypos.end() ||
            myMapOldAlgos.find(hyp->GetIOR()) != myMapOldAlgos.end());
  }

  return false;
}

//=================================================================================
// function : removeItem()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::removeItem (QListBoxItem* item)
{
  const QObject* aSender = sender();

  if (!item) return;

  if (aSender == ListHypAssignation) {
    myNbModification += isOld(item) ? 1 : -1;
    ListHypAssignation->removeItem(ListHypAssignation->index(item));
  }
  else if (aSender == ListAlgoAssignation) {
    myNbModification += isOld(item) ? 1 : -1;
    ListAlgoAssignation->removeItem(ListAlgoAssignation->index(item));
  }

  emit( needToUpdate() );
}

//=================================================================================
// function : addItem()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::addItem (QListBoxItem* item)
{
  const QObject* aSender = sender();

  if (!item) return;

  ListBoxIOR* i = 0;
  if (item->rtti() == ListBoxIOR::RTTI_IOR)
    i = (ListBoxIOR*)item;
  if (!i) return;

  bool isFound = false;

  ListBoxIOR* anItem;
  if (aSender == ListHypDefinition) {
    for (int j = 0, n = ListHypAssignation->count(); !isFound && j < n; j++) {
      if (ListHypAssignation->item(j)->rtti() == ListBoxIOR::RTTI_IOR) {
	anItem = (ListBoxIOR*)ListHypAssignation->item(j);
	isFound = !strcmp(anItem->GetIOR(), i->GetIOR());
      }
    }
    if (!isFound)
      anItem = new ListBoxIOR (ListHypAssignation,
                               CORBA::string_dup(i->GetIOR()),
                               CORBA::string_dup(i->text().latin1()));

  } else if (aSender == ListAlgoDefinition) {
    for (int j = 0, n = ListAlgoAssignation->count(); !isFound && j < n; j++) {
      if (ListAlgoAssignation->item(j)->rtti() == ListBoxIOR::RTTI_IOR) {
	anItem = (ListBoxIOR*)ListAlgoAssignation->item(j);
	isFound = !strcmp(anItem->GetIOR(), i->GetIOR());
      }
    }
    if (!isFound)
      anItem = new ListBoxIOR (ListAlgoAssignation,
                               CORBA::string_dup(i->GetIOR()),
                               CORBA::string_dup(i->text().latin1()));
  } else {
  }

  if (!isFound)
    myNbModification += isOld(item) ? -1 : 1;

  emit( needToUpdate() );
}

//=================================================================================
// function : setListsEnabled()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::setListsEnabled( const bool en )
{
  ListHypDefinition  ->setEnabled( en );
  ListHypAssignation ->setEnabled( en );
  ListAlgoDefinition ->setEnabled( en );
  ListAlgoAssignation->setEnabled( en );
}

//=================================================================================
// function : hypoCount()
// purpose  :
//=================================================================================
int SMESHGUI_EditHypothesesDlg::hypoCount() const
{
  return ListHypAssignation->count();
}

//=================================================================================
// function : algoCount()
// purpose  :
//=================================================================================
int SMESHGUI_EditHypothesesDlg::algoCount() const
{
  return ListAlgoAssignation->count();
}

//=================================================================================
// function : isModified()
// purpose  :
//=================================================================================
bool SMESHGUI_EditHypothesesDlg::isModified() const
{
  return myNbModification!=0;
}

//=================================================================================
// function : findItem()
// purpose  :
//=================================================================================
int SMESHGUI_EditHypothesesDlg::findItem( QListBox* listBox, const QString& ior )
{
  for (int i = 0; i < listBox->count(); i++)
  {
    if (listBox->item(i)->rtti() == ListBoxIOR::RTTI_IOR)
    {
      ListBoxIOR* anItem = (ListBoxIOR*)(listBox->item(i));
      if( anItem && ior == anItem->GetIOR() )
        return i;
    }
  }
  return -1;
}
