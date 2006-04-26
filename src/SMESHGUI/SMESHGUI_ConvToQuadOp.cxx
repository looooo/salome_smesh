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
*  File   : SMESHGUI_ConvToQuadOp.h
*  Module : SMESHGUI
*/

#include "SMESHGUI_ConvToQuadOp.h"
#include "SMESHGUI_ConvToQuadDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"

#include "SMESH_TypeFilter.hxx"

#include "SalomeApp_Tools.h"

#include "SUIT_MessageBox.h"

#include "LightApp_UpdateFlags.h"
       
//================================================================================
/*!
 * \brief Constructor
 *
 * Initialize operation
*/
//================================================================================
SMESHGUI_ConvToQuadOp::SMESHGUI_ConvToQuadOp()
  : SMESHGUI_SelectionOp(), 
    myDlg( 0 )
{
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================
SMESHGUI_ConvToQuadOp::~SMESHGUI_ConvToQuadOp()
{
  if ( myDlg )
    delete myDlg;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
  * \retval LightApp_Dialog* - pointer to dialog of this operation
*/
//================================================================================
LightApp_Dialog* SMESHGUI_ConvToQuadOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*!
 * \brief Creates dialog if necessary and shows it
 *
 * Virtual method redefined from base class called when operation is started creates
 * dialog if necessary and shows it, activates selection
 */
//================================================================================
void SMESHGUI_ConvToQuadOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_ConvToQuadDlg( );
  }
  SMESHGUI_SelectionOp::startOperation();

  myDlg->SetMediumNdsOnGeom( false );
  myDlg->activateObject( 0 );
  myDlg->show();

  selectionDone();
}

//================================================================================
/*!
 * \brief Updates dialog's look and feel
 *
 * Virtual method redefined from the base class updates dialog's look and feel
 */
//================================================================================
void SMESHGUI_ConvToQuadOp::selectionDone()
{
  if ( !dlg()->isShown() )
    return;

  SMESHGUI_SelectionOp::selectionDone();
  try
  {
    QString anMeshEntry = myDlg->selectedObject( 0 );
    _PTR(SObject) pMesh = studyDS()->FindObjectID( anMeshEntry.latin1() );
    if ( !pMesh ) return;

    SMESH::SMESH_Mesh_var mesh =
    SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( pMesh );  

    GEOM::GEOM_Object_var mainGeom;
    mainGeom = mesh->GetShapeToMesh();

    if( mesh->_is_nil() || 
	( !mesh->NbEdgesOfOrder(SMESH::ORDER_LINEAR) && 
	  !mesh->NbFacesOfOrder(SMESH::ORDER_LINEAR) &&
	  !mesh->NbVolumesOfOrder(SMESH::ORDER_LINEAR) ) )
    {
      myDlg->setButtonEnabled( false, QtxDialog::OK | QtxDialog::Apply );
      myDlg->SetEnabledCheck( false );
    }
    else if( mainGeom->_is_nil() && myDlg->IsEnabledCheck() )
      myDlg->SetEnabledCheck( false );
    else 
    {
      myDlg->setButtonEnabled( true, QtxDialog::OK | QtxDialog::Apply );
      myDlg->SetEnabledCheck( true );
    }
  }
  catch ( const SALOME::SALOME_Exception& S_ex )
  {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }
  catch ( ... )
  {
  }
}

//================================================================================
/*!
 * \brief Creates selection filter
  * \param theId - identifier of current selection widget
  * \retval SUIT_SelectionFilter* - pointer to the created filter or null
 *
 * Creates selection filter in accordance with identifier of current selection widget
 */
//================================================================================
SUIT_SelectionFilter* SMESHGUI_ConvToQuadOp::createFilter( const int theId ) const
{
  if ( theId == 0 )
    return new SMESH_TypeFilter( MESH );
  else
    return 0;
}

//================================================================================
/*!
 * \brief Edits mesh
 *
 * Virtual slot redefined from the base class called when "Apply" button is clicked
 */
//================================================================================
bool SMESHGUI_ConvToQuadOp::onApply()
{

  QString aMess;

  QString anMeshEntry = myDlg->selectedObject( 0 );
  _PTR(SObject) pMesh = studyDS()->FindObjectID( anMeshEntry.latin1() );
  if ( !pMesh )
  {
    dlg()->show();
    SUIT_MessageBox::warn1( myDlg,
        tr( "SMESH_WRN_WARNING" ), tr("MESH_IS_NOT_SELECTED"), tr( "SMESH_BUT_OK" ) );
   
    return false;
  }

  SMESH::SMESH_Mesh_var mesh =
  SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( pMesh );  

  if( CORBA::is_nil(mesh) )
  {
    SUIT_MessageBox::warn1( myDlg,
        tr( "SMESH_WRN_WARNING" ), tr("REF_IS_NULL"), tr( "SMESH_BUT_OK" ) );

    return false;
  } 

  bool aResult = false;

  try
  {
    bool aParam = true;
    if( myDlg->IsEnabledCheck() )
      aParam = myDlg->IsMediumNdsOnGeom();

    SMESH::SMESH_MeshEditor_var aEditor = mesh->GetMeshEditor();
    aEditor->ConvertToQuadratic( aParam );
    aResult = true; 
  }
  catch ( const SALOME::SALOME_Exception& S_ex )
  {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
    aResult = false;
  }
  catch ( ... )
  {
    aResult = false;
  }
  if(aResult)
  {
    update( UF_ObjBrowser | UF_Model );
    myDlg->setButtonEnabled( false, QtxDialog::Apply );
  }
  return aResult;
}

