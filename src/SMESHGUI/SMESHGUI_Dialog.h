
#ifndef SMESHGUI_DIALOG_H
#define SMESHGUI_DIALOG_H

#include <SalomeApp_Dialog.h>

class SMESHGUI;

class SMESHGUI_Dialog : public SalomeApp_Dialog
{
  Q_OBJECT
  
public:
  SMESHGUI_Dialog( SMESHGUI*, const bool = false, const bool = false, const int = OK | Cancel | Apply );
  virtual ~SMESHGUI_Dialog();
};

#endif
