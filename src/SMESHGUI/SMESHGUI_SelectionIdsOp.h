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
//  File   : SMESHGUI_SelectionIdsOp.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH


#ifndef SMESHGUI_SelectionIdsOp_H
#define SMESHGUI_SelectionIdsOp_H

#include <SMESHGUI_SelectionOp.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SMESH_Actor;

/*
  Class       : SMESHGUI_SelectionIdsOp
  Description : Base operation for all operations using selection mesh elements by id on one mesh
                In order to allow user to edit ids "by hands" the dialog must call setReadOnly( .., false )
                with id or object selection widget instead '..'
                All other necessary actions are implemented here
*/

class SUIT_SelectionFilter;

class SMESHGUI_SelectionIdsOp : public SMESHGUI_SelectionOp
{ 
  Q_OBJECT

public:
  typedef QValueList<int> IdList;

public:
  SMESHGUI_SelectionIdsOp( const Selection_Mode = ActorSelection );
  virtual ~SMESHGUI_SelectionIdsOp();

  static void   extractIds( const QStringList&, IdList&, const QChar );
    
protected:
  virtual void  startOperation();
  virtual void  commitOperation();
  virtual void  abortOperation();
  virtual void  selectionDone();

  SMESH::SMESH_Mesh_var  mesh() const;
  SMESH_Actor*           actor() const;
  void                   selectedIds( const int, IdList& ) const;
  void                   extractIds( const QStringList&, IdList& ) const;

protected slots:
  virtual void  onTextChanged( int, const QStringList& );

private:
  SMESH::SMESH_Mesh_var       myMesh;
};

#endif
