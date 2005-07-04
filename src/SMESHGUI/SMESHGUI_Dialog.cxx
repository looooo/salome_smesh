
#include <SMESHGUI_Dialog.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI.h>

#include <SUIT_Desktop.h>

SMESHGUI_Dialog::SMESHGUI_Dialog( SMESHGUI* theModule, const bool modal, const bool allowResize,
                                  const int flags )
: SalomeApp_Dialog( SMESH::GetDesktop( theModule ), "", modal, allowResize, flags )
{
}

SMESHGUI_Dialog::~SMESHGUI_Dialog()
{
}

