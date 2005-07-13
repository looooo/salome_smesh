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
//  File   : SMESHGUI_SelectionOp.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH


#ifndef SMESHGUI_SelectionOp_H
#define SMESHGUI_SelectionOp_H

#include <SMESHGUI_Operation.h>

/*
  Class       : SMESHGUI_SelectionOp
  Description : Base operation for all operations using object selection in viewer or objectbrowser
                through common widgets created by SalomeApp_Dialog::createObject
*/

class SUIT_SelectionFilter;

class SMESHGUI_SelectionOp : public SMESHGUI_Operation
{ 
  Q_OBJECT

public:
  SMESHGUI_SelectionOp( const Selection_Mode = ActorSelection );
  virtual ~SMESHGUI_SelectionOp();
  
protected:
  virtual void                  startOperation();
  virtual void                  commitOperation();
  virtual void                  abortOperation();  
  
  virtual void                  selectionDone();

  //! sets the dialog widgets to state just after operation start
  virtual void                  initDialog();

  /*! Creates filter being used when certain object selection widget is active
   *  If no filter must be used, then function must return 0
   *  if id is negative, then function must return filter for common using independently of active widget
   */
  virtual SUIT_SelectionFilter* createFilter( const int ) const;

  //! Remove only filters set by this operation (they are in map myFilters )
  void removeCustomFilters() const;

protected slots:
  //! Installs filter corresponding to certain object selection widget
  virtual void onActivateObject( int );

  //! Removes filter corresponding to certain object selection widget
  virtual void onDeactivateObject( int );

  /*!
    *  Empty default implementation. In successors it may be used for more advanced selection checking.
    *  This slot is connected to signal when the selection changed in some object selection widget
  */
  virtual void onSelectionChanged( int );

private:
  typedef QMap<int, SUIT_SelectionFilter*> Filters;
  
private:
  Filters         myFilters;
  Selection_Mode  myDefSelectionMode, myOldSelectionMode;
};

#endif
