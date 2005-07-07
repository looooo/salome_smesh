
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

  virtual void show();

  //! set all content to enable (parameter is true) or disable state
  void setContentActive( const bool ) const;

private:
  SMESHGUI* mySMESHGUI;
};

#endif
