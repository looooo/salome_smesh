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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_Make2DFrom3D.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_Make2DFrom3DOp_H
#define SMESHGUI_Make2DFrom3DOp_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QFrame;
class QCheckBox;
class QLineEdit;
class QRadioButton;
class SMESHGUI_MeshInfosBox;

/*!
 * \brief Dialog to show result mesh statistic
 */

class SMESHGUI_Make2DFrom3DDlg :  public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_Make2DFrom3DDlg( QWidget* );
  virtual ~SMESHGUI_Make2DFrom3DDlg();

  void                   SetMeshName(const QString& theName);
  void                   SetMeshInfo(const SMESH::long_array& theInfo);

private:
  QFrame*                createMainFrame( QWidget* );

private:
  QLabel*                myMeshName;
  SMESHGUI_MeshInfosBox* myFullInfo;
};

/*!
 * \brief Dialog to show result mesh statistic
 */

class SMESHGUI_CopyMeshDlg :  public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  enum { Mesh };

  SMESHGUI_CopyMeshDlg( QWidget* );
  virtual ~SMESHGUI_CopyMeshDlg();

  void enableControls( bool );

private slots:
  void onTargetChanged();
  void onGroupChecked();

private:
  QRadioButton* myThisMeshRB;
  QRadioButton* myNewMeshRB;
  QLineEdit*    myMeshName;
  QCheckBox*    myCopyCheck;
  QCheckBox*    myMissingCheck;
  QCheckBox*    myGroupCheck;
  QLineEdit*    myGroupName;
};

/*!
 * \brief Operation to compute 2D mesh on 3D
 */

class SMESHGUI_Make2DFrom3DOp : public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_Make2DFrom3DOp();
  virtual ~SMESHGUI_Make2DFrom3DOp();

  virtual LightApp_Dialog*           dlg() const;

protected:
  virtual void                       startOperation();
  virtual void                       selectionDone();
  virtual SUIT_SelectionFilter*      createFilter( const int ) const;

protected slots:
  virtual bool                       onApply();

private:
  bool                               compute2DMesh();

private:
  SMESH::SMESH_IDSource_var          mySrc;
  QPointer<SMESHGUI_CopyMeshDlg>     myDlg;
};

#endif // SMESHGUI_Make2DFrom3DOp_H
