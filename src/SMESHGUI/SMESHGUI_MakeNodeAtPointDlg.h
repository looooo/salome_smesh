// Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
// File   : SMESHGUI_MakeNodeAtPointDlg.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//

#ifndef SMESHGUI_MAKENODEATPOINTDLG_H
#define SMESHGUI_MAKENODEATPOINTDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"

class QGroupBox;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QRadioButton;
class SMESHGUI_SpinBox;
class SMESHGUI_MeshEditPreview;
class SMESHGUI_MakeNodeAtPointDlg;

/*!
 * \brief Operation to make a mesh pass through a point
 */
class SMESHGUI_EXPORT SMESHGUI_MakeNodeAtPointOp: public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_MakeNodeAtPointOp();
  virtual ~SMESHGUI_MakeNodeAtPointOp();

  virtual LightApp_Dialog*       dlg() const;  

protected:

  virtual void                   startOperation();
  virtual void                   stopOperation();

  virtual void                   activateSelection();

  bool                           isValid( QString& );

protected slots:
  virtual bool                   onApply();

private slots:
  void                           onSelectionDone();
  void                           redisplayPreview();

private:
  SMESHGUI_MakeNodeAtPointDlg*  myDlg;

  SUIT_SelectionFilter*         myFilter;
  int                           myMeshOldDisplayMode;
  SMESHGUI_MeshEditPreview*     mySimulation;
  SMESH_Actor*                  myMeshActor;
  bool                          myNoPreview;
};

/*!
 * \brief Dialog to make a mesh pass through a point
 */

class SMESHGUI_EXPORT SMESHGUI_MakeNodeAtPointDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_MakeNodeAtPointDlg();

private:
  QWidget*                      createMainFrame( QWidget* );

  QPushButton*                  myCoordBtn;
  SMESHGUI_SpinBox*             myX;
  SMESHGUI_SpinBox*             myY;
  SMESHGUI_SpinBox*             myZ;
  QRadioButton*                 myMoveRBtn;
  QRadioButton*                 myCreateRBtn;
  QGroupBox*                    myNodeToMoveGrp;
  QPushButton*                  myIdBtn;
  QLineEdit*                    myId;
  QCheckBox*                    myAutoSearchChkBox;
  QCheckBox*                    myPreviewChkBox;

  QString                       myHelpFileName;

  friend class SMESHGUI_MakeNodeAtPointOp;

private slots:
  void                          ButtonToggled( bool );
};

#endif // SMESHGUI_MAKENODEATPOINTDLG_H
