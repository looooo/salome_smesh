
#include <SMESHGUI_Dialog.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI_Operation.h>
#include <SMESH_Type.h>
#include <SMESHGUI.h>

#include <SUIT_Desktop.h>

SMESHGUI_Dialog::SMESHGUI_Dialog( SMESHGUI* theModule, const bool modal, const bool allowResize,
                                  const int flags )
: SalomeApp_Dialog( SMESH::GetDesktop( theModule ), "", modal, allowResize, flags ),
  mySMESHGUI( theModule )
{
  int prefix = SMESHGUI_Operation::prefix( "SMESH" );
  typeName( prefix + MESH ) = tr( "DLG_MESH" );
  typeName( prefix + HYPOTHESIS ) = tr( "DLG_HYPO" );
  typeName( prefix + ALGORITHM ) = tr( "DLG_ALGO" );
}

SMESHGUI_Dialog::~SMESHGUI_Dialog()
{
}

void SMESHGUI_Dialog::show()
{
  int x, y;
  mySMESHGUI->DefineDlgPosition(this, x, y);
  move(x, y);
  SalomeApp_Dialog::show();
}

void SMESHGUI_Dialog::setContentActive( const bool active ) const
{
  mainFrame()->setEnabled( active );
}
