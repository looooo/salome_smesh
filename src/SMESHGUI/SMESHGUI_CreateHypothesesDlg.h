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
//  File   : SMESHGUI_CreateHypothesesDlg.h
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_CREATE_HYPOTHESES_H
#define DIALOGBOX_CREATE_HYPOTHESES_H

#include "SMESHGUI_Dialog.h"

class QListView;
class QListViewItem;

/*!
 * \brief Dialog for creation of hypotheses and algorithms
*/
class SMESHGUI_CreateHypothesesDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:

  SMESHGUI_CreateHypothesesDlg ( bool isAlgo );
  virtual ~SMESHGUI_CreateHypothesesDlg();

  void          init( const QStringList& theHypList,
                      const QStringList& thePluginNames,
                      const QStringList& theLabels,
                      const QStringList& theIconIds );

  QString       hypName() const;

private slots:

  void          onHypSelected();
  void          onDoubleClicked( QListViewItem* );

private:

  QListView*    myList;
  bool          myIsAlgo;
};

#endif // DIALOGBOX_CREATE_HYPOTHESES_H



