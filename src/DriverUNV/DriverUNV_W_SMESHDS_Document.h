#ifndef _INCLUDE_DRIVERUNV_W_SMESHDS_DOCUMENT
#define _INCLUDE_DRIVERUNV_W_SMESHDS_DOCUMENT

#include <stdio.h>
#include <string>

#include "SMESHDS_Document.hxx"
#include "Document_Writer.h"

class DriverUNV_W_SMESHDS_Document : public Document_Writer {
  
public :
  DriverUNV_W_SMESHDS_Document();
  ~DriverUNV_W_SMESHDS_Document();
 
  void Write();
  //void SetFile(string);
  //void SetDocument(Handle(SMESHDS_Document)&);

private :
  //Handle_SMESHDS_Document myDocument;
  //string myFile; 

};
#endif
