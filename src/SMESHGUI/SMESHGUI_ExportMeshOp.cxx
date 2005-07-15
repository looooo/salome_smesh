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
//  File   : SMESHGUI_ExportMeshOp.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#include "SMESHGUI_ExportMeshOp.h"
#include <SMESHGUI_MeshUtils.h>
#include <SMESHGUI_Utils.h>

#include <SalomeApp_SelectionMgr.h>
#include <SUIT_FileDlg.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>
#include <SUIT_OverrideCursor.h>

#include <SALOME_ListIO.hxx>

//=================================================================================
// function :
// purpose  :
//=================================================================================
SMESHGUI_ExportMeshOp::SMESHGUI_ExportMeshOp( const Type t )
: SMESHGUI_Operation(),
  myType( t )
{
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
SMESHGUI_ExportMeshOp::~SMESHGUI_ExportMeshOp()
{
}

//=================================================================================
// function :
// purpose  :
//=================================================================================
void SMESHGUI_ExportMeshOp::startOperation()
{
  SMESHGUI_Operation::startOperation();

  SalomeApp_SelectionMgr *aSel = selectionMgr();
  SALOME_ListIO selected;
  if( aSel )
    aSel->selectedObjects( selected );

  if( selected.Extent()==0 )
  {
    abort();
    return;
  }
  
  Handle(SALOME_InteractiveObject) anIObject = selected.First();
  SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIObject);
  if( !aMesh->_is_nil() )
  {
    QString aFilter, aTitle = QObject::tr("Export mesh");
    QMap<QString, SMESH::MED_VERSION> aFilterMap;
    switch( myType )
    {
      case MED:
        aFilterMap.insert( QObject::tr("MED 2.1 (*.med)"), SMESH::MED_V2_1 );
        aFilterMap.insert( QObject::tr("MED 2.2 (*.med)"), SMESH::MED_V2_2 );
        break;
        
      case DAT:
        aFilter = QObject::tr("DAT files (*.dat)");
        break;
        
      case UNV:
        if( aMesh->NbPyramids() )
        {
          int aRet = SUIT_MessageBox::warn2( desktop(), tr("SMESH_WRN_WARNING"),
                    tr("SMESH_EXPORT_UNV").arg(anIObject->getName()), tr("SMESH_BUT_YES"),
                    tr("SMESH_BUT_NO"), 0,1,0 );
          if(aRet)
          {
            abort();
            return;
          }
        }
        aFilter = QObject::tr("IDEAS files (*.unv)");
        break;
      default:
        abort();
        return;
    }

    QString aFilename;
    SMESH::MED_VERSION aFormat;

    if( myType!=MED )
      aFilename = SUIT_FileDlg::getFileName( desktop(), "", aFilter, aTitle, false );
    else
    {
      QStringList filters;
      for( QMap<QString, SMESH::MED_VERSION>::const_iterator it = aFilterMap.begin(); it != aFilterMap.end(); ++it )
        filters.push_back( it.key() );

      SUIT_FileDlg* fd = new SUIT_FileDlg( desktop(), false, true, true );
      fd->setCaption( aTitle );
      fd->setFilters( filters );
      bool is_ok = false;
      while( !is_ok )
      {
        fd->exec();
        aFilename = fd->selectedFile();
        aFormat = aFilterMap[fd->selectedFilter()];
        is_ok = true;
        if( !aFilename.isEmpty() && (aMesh->NbPolygons()>0 or aMesh->NbPolyhedrons()>0 ) && aFormat==SMESH::MED_V2_1)
        {
          int aRet = SUIT_MessageBox::warn2( desktop(), tr("SMESH_WRN_WARNING"),
                     tr("SMESH_EXPORT_MED_V2_1").arg(anIObject->getName()), tr("SMESH_BUT_YES"),
                     tr("SMESH_BUT_NO"), 0,1,0 );
          if(aRet)
            is_ok = false;
        }
      }
      delete fd;
    }
  
    if( !aFilename.isEmpty() )
    {
      // Check whether the file already exists and delete it if yes
      QFile aFile( aFilename );
      if( aFile.exists() )
        aFile.remove();
      SUIT_OverrideCursor wc;
      switch( myType )
      {
        case MED:
          aMesh->ExportToMED( aFilename.latin1(), false, aFormat ); // currently, automatic groups are never created
          break;
        
        case DAT:
          aMesh->ExportDAT( aFilename.latin1() );
          break;

        case UNV:
          aMesh->ExportUNV( aFilename.latin1() );
          break;
        
        default:
          abort();
          return;
      }
      commit();
      return;  
    }
  }
  abort();
}
