# -*- coding: utf-8 -*-
# Copyright (C) 2013-2015  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# Modules Python
import string,types,os
import traceback

from qtsalome import *

# Import des panels

from MGCleanerViewText_ui import Ui_ViewExe

class MGCleanerMonViewText(Ui_ViewExe, QDialog):
    """
    Classe permettant la visualisation de texte
    """
    def __init__(self, parent, txt, ):
        QDialog.__init__(self,parent)
        self.setupUi(self)
        self.resize( QSize(1000,600).expandedTo(self.minimumSizeHint()) )
        #self.connect( self.PB_Ok,SIGNAL("clicked()"), self, SLOT("close()") )
        self.PB_Ok.clicked.connect( self.theClose )
        self.PB_Save.clicked.connect( self.saveFile )
        self.PB_Save.setToolTip("Save trace in log file")
        self.PB_Ok.setToolTip("Close view")
        self.monExe=QProcess(self)

        self.monExe.readyReadStandardOutput.connect( self.readFromStdOut )
        self.monExe.readyReadStandardError.connect( self.readFromStdErr )
      
        # Je n arrive pas a utiliser le setEnvironment du QProcess
        # fonctionne hors Salome mais pas dans Salome ???
        cmds=''
        cmds+='rm -f '+self.parent().fichierOut+'\n'
        cmds+=txt+'\n'
        cmds+='echo END_OF_MGCleaner\n'
        pid=self.monExe.pid()
        nomFichier='/tmp/MGCleaner_'+str(pid)+'.sh'
        f=open(nomFichier,'w')
        f.write(cmds)
        f.close()

        maBidouille='sh ' + nomFichier
        self.monExe.start(maBidouille)
        self.monExe.closeWriteChannel()
        self.enregistreResultatsDone=False
        self.show()

    def saveFile(self):
        #recuperation du nom du fichier
        savedir=os.environ['HOME']
        fn = QFileDialog.getSaveFileName(None, self.trUtf8("Save File"),savedir)
        if fn.isNull() : return
        ulfile = os.path.abspath(unicode(fn))
        try:
           f = open(fn, 'wb')
           f.write(str(self.TB_Exe.toPlainText()))
           f.close()
        except IOError, why:
           QMessageBox.critical(self, self.trUtf8('Save File'),
                self.trUtf8('The file <b>%1</b> could not be saved.<br>Reason: %2')
                    .arg(unicode(fn)).arg(str(why)))

    def readFromStdErr(self):
        a=self.monExe.readAllStandardError()
        self.TB_Exe.append(unicode(a.data()))

    def readFromStdOut(self) :
        a=self.monExe.readAllStandardOutput()
        aa=unicode(a.data())
        self.TB_Exe.append(aa)
        if "END_OF_MGCleaner" in aa:
          self.parent().enregistreResultat()
          self.enregistreResultatsDone=True
          #self.theClose()
    
    def theClose(self):
      if not self.enregistreResultatsDone:
        self.parent().enregistreResultat()
        self.enregistreResultatsDone=True
      self.close()
