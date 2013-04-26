# -*- coding: utf-8 -*-
# Copyright (C) 2007-2013  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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
# Modules Eficas

import os, subprocess
from MGCleanerPlugDialog import Ui_MGCleanerPlugDialog
from MGCleanerMonViewText import MGCleanerMonViewText
from PyQt4.QtGui import *
from PyQt4.QtCore import *


class MGCleanerMonPlugDialog(Ui_MGCleanerPlugDialog,QWidget):
  """
  """
  def __init__(self):
        QWidget.__init__(self)
        self.setupUi(self)
        self.connecterSignaux()
        self.fichierIn=""
        self.fichierOut=""
        self.MeshIn="" 
        self.num=1

        # complex whith QResources: not used
        # The icon are supposed to be located in the $SMESH_ROOT_DIR/share/salome/resources/smesh folder,
        # other solution could be in the same folder than this python module file:
        # iconfolder=os.path.dirname(os.path.abspath(__file__))

        iconfolder=os.environ['SMESH_ROOT_DIR']+'/share/salome/resources/smesh'
        #print "MGCleanerMonPlugDialog iconfolder",iconfolder
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"select1.png"))
        self.PB_MeshSmesh.setIcon(icon)
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"open.png"))
        self.PB_ParamsFileExplorer.setIcon(icon)

        #Ces parametres ne sont pas remis à rien par le clean
        self.paramsFile= os.path.abspath(os.path.join(os.environ['HOME'],'.MGCleaner.dat'))
        self.LE_ParamsFile.setText(self.paramsFile)
        self.LE_MeshFile.setText("")
        self.LE_MeshSmesh.setText("")
        self.resize(800, 500)
        self.clean()

  def connecterSignaux(self) :
        self.connect(self.PB_Cancel,SIGNAL("clicked()"),self.PBCancelPressed)
        self.connect(self.PB_Default,SIGNAL("clicked()"),self.clean)
        self.connect(self.PB_Help,SIGNAL("clicked()"),self.PBHelpPressed)
        self.connect(self.PB_Load,SIGNAL("clicked()"),self.PBLoadPressed)
        self.connect(self.PB_OK,SIGNAL("clicked()"),self.PBOKPressed)
        self.connect(self.PB_Save,SIGNAL("clicked()"),self.PBSavePressed)
        self.connect(self.PB_MeshFile,SIGNAL("clicked()"),self.PBMeshFilePressed)
        self.connect(self.PB_MeshSmesh,SIGNAL("clicked()"),self.PBMeshSmeshPressed)
        self.connect(self.PB_ParamsFileExplorer,SIGNAL("clicked()"),self.setParamsFileName)
        self.connect(self.LE_MeshFile,SIGNAL("returnPressed()"),self.meshFileNameChanged)
        self.connect(self.LE_ParamsFile,SIGNAL("returnPressed()"),self.paramsFileNameChanged)

        #QtCore.QObject.connect(self.checkBox, QtCore.SIGNAL('stateChanged(int)'), self.change) 
        self.connect(self.CB_FillHoles,SIGNAL("stateChanged(int)"),self.SP_minHoleSize.setEnabled)
        self.connect(self.CB_computedToleranceDisplacement,SIGNAL("stateChanged(int)"),self.SP_toleranceDisplacement.setDisabled)
        self.connect(self.CB_computedResolutionLength,SIGNAL("stateChanged(int)"),self.SP_resolutionLength.setDisabled)
        self.connect(self.CB_computedOverlapDistance,SIGNAL("stateChanged(int)"),self.SP_overlapDistance.setDisabled)
        
  def PBHelpPressed(self):
        try :
          mydir=os.environ['SMESH_ROOT_DIR']
        except Exception:
          QMessageBox.warning( self, "Help unavailable $SMESH_ROOT_DIR not found")
        maDoc=mydir+"/share/doc/salome/gui/SMESH/MGCleaner/_downloads/mg-cleaner_user_manual.pdf"
        command="xdg-open "+maDoc+";"
        subprocess.call(command, shell=True)


  def PBOKPressed(self):
        if not(self.PrepareLigneCommande()): return
        self.PBSavePressed(NomHypo=True)
        maFenetre=MGCleanerMonViewText(self,self.commande)
        if os.path.isfile(self.fichierOut): self.enregistreResultat()

  def enregistreResultat(self):
        import smesh
        import SMESH
        import salome
        from salome.kernel import studyedit

        maStudy=studyedit.getActiveStudy()
        smesh.SetCurrentStudy(maStudy)
        (outputMesh, status) = smesh.CreateMeshesFromGMF(self.fichierOut)
        meshname = 'MGCleaner'+str(self.num)
        smesh.SetName(outputMesh.GetMesh(), meshname)
        outputMesh.Compute()


        self.editor = studyedit.getStudyEditor()    # 
        moduleEntry=self.editor.findOrCreateComponent("SMESH","SMESH")
        HypReMeshEntry = self.editor.findOrCreateItem( moduleEntry, name = 'HypoForRemesh',
                                           comment = 'HypoForRemeshing')
        monStudyBuilder=maStudy.NewBuilder();
        monStudyBuilder.NewCommand();
        newStudyIter=monStudyBuilder.NewObject(HypReMeshEntry)
        aNameAttrib=monStudyBuilder.FindOrCreateAttribute(newStudyIter,"AttributeName")
        hypoName = 'anHypo_MGCleaner_'+str(self.num)
        aNameAttrib.SetValue(hypoName)
        aCommentAttrib=monStudyBuilder.FindOrCreateAttribute(newStudyIter,"AttributeComment")
        aCommentAttrib.SetValue(str(self.commande))
        
        SOMesh=maStudy.FindObjectByName(meshname ,"SMESH")[0]
        newLink=monStudyBuilder.NewObject(SOMesh)
        monStudyBuilder.Addreference(newLink, newStudyIter);
        if salome.sg.hasDesktop(): salome.sg.updateObjBrowser(0)
        self.num+=1
        return True

  def PBSavePressed(self,NomHypo=False):
        if NomHypo: 
          text = '# Params for Hypothese : anHypo_MGCleaner_'+str(self.num - 1)+"\n"
        else:
          text = '# Save intermediate params \n' 
        text += "# Params for mesh : " +  self.LE_MeshSmesh.text() +'\n'
        
        """
        for RB in self.GBOptim.findChildren(QRadioButton,):
            if RB.isChecked()==True:
               text+="Optimisation ='"+RB.text()+"'\n"
               break
        for RB in self.GBUnit.findChildren(QRadioButton,):
            if RB.isChecked()==True:
               text+="Units ='"+RB.text()+"'\n"
        text+='Chordal_Tolerance_Deviation='+str(self.SP_Tolerance.value())+'\n'
        text+='Ridge_Detection=' + str(self.CB_Ridge.isChecked())+'\n'
        text+='Split_Edge='      + str(self.CB_SplitEdge.isChecked())+'\n'
        text+='Point_Smoothing=' + str(self.CB_Point.isChecked())+'\n'
        text+='Geometrical_Approximation='+ str(self.SP_Geomapp.value())  +'\n'
        text+='Ridge_Angle='              + str(self.SP_Ridge.value())    +'\n'
        text+='Maximum_Size='             + str(self.SP_MaxSize.value())  +'\n'
        text+='Minimum_Size='             + str(self.SP_MaxSize.value())  +'\n'
        text+='Mesh_Gradation='           + str(self.SP_Gradation.value())+'\n'
        text+='Verbosity='                + str(self.SP_Verbosity.value())+'\n'
        text+='\n\n'
        """
        try:
           f=open(self.paramsFile,'a')
        except:
           QMessageBox.warning(self, "File", "Unable to open "+self.paramsFile)
           return
        try:
           f.write(text)
        except:
           QMessageBox.warning(self, "File", "Unable to write "+self.paramsFile)
           return
        f.close()

  def PBLoadPressed(self):
        try:
           f=open(self.paramsFile,'r')
        except :
           QMessageBox.warning(self, "File", "Unable to open "+self.paramsFile)
           return
        try:
           text=f.read()
        except :
           QMessageBox.warning(self, "File", "Unable to read "+self.paramsFile)
           return
        f.close()
        d={}
        exec text in d
        for RB in self.GBOptim.findChildren(QRadioButton,):
            if d['Optimisation']== RB.text():
               RB.setChecked(True)
               break
        for RB in self.GBUnit.findChildren(QRadioButton,):
            if d['Units']==RB.text():
               RB.setChecked(True)
               break
        self.SP_Tolerance.setValue(d['Chordal_Tolerance_Deviation'])

        self.CB_Ridge.setChecked(d['Ridge_Detection'])
        self.CB_Point.setChecked(d['Point_Smoothing'])
        self.CB_SplitEdge.setChecked(d['Split_Edge'])
        self.SP_Geomapp.setValue(d['Geometrical_Approximation'])
        self.SP_Ridge.setValue(d['Ridge_Angle'])
        self.SP_MaxSize.setValue(d['Maximum_Size'])
        self.SP_MinSize.setValue(d['Minimum_Size'])
        self.SP_Gradation.setValue(d['Mesh_Gradation'])

        self.SP_Verbosity.setValue(d['Verbosity'])
        self.SP_Memory.setValue(d['Memory'])


  def PBCancelPressed(self):
        self.close()

  def PBMeshFilePressed(self):
       fd = QFileDialog(self, "select an existing Mesh file", self.LE_MeshFile.text(), "Mesh-Files (*.mesh);;All Files (*)")
       if fd.exec_():
          infile = fd.selectedFiles()[0]
          self.LE_MeshFile.setText(infile)
          self.fichierIn=infile.toLatin1()

  def setParamsFileName(self):
       fd = QFileDialog(self, "select a file", self.LE_ParamsFile.text(), "dat Files (*.dat);;All Files (*)")
       if fd.exec_():
          infile = fd.selectedFiles()[0]
          self.LE_ParamsFile.setText(infile)
          self.paramsFile=infile.toLatin1()

  def meshFileNameChanged(self):
      self.fichierIn=self.LE_MeshFile.text()
      if os.path.exists(self.fichierIn): return
      QMessageBox.warning( self, "Unknown File", "File doesn't exist")

  def paramsFileNameChanged(self):
      self.paramsFile=self.LE_ParamsFile.text()

  def PBMeshSmeshPressed(self):
      import salome
      import smesh
      from salome.kernel import studyedit
      from salome.smesh.smeshstudytools import SMeshStudyTools
      from salome.gui import helper as guihelper
      from omniORB import CORBA

      mySObject, myEntry = guihelper.getSObjectSelected()
      if CORBA.is_nil(mySObject) or mySObject==None:
         QMessageBox.critical(self, "Mesh", "select an input mesh")
         return
      self.smeshStudyTool = SMeshStudyTools()
      self.__selectedMesh = self.smeshStudyTool.getMeshObjectFromSObject(mySObject)
      if CORBA.is_nil(self.__selectedMesh):
         QMessageBox.critical(self, "Mesh", "select an input mesh")
         return
      myName = mySObject.GetName()
      self.MeshIn=myName
      self.LE_MeshSmesh.setText(myName)

  def prepareFichier(self):
      self.fichierIn="/tmp/PourMGCleaner_"+str(self.num)+".mesh"
      import SMESH
      self.__selectedMesh.ExportGMF(self.__selectedMesh,self.fichierIn, True)

  def PrepareLigneCommande(self):
      """
      #use doc examples of mg-cleaner:
      ls -al /data/tmplgls/salome/prerequis/install/COMMON_64/MeshGems-1.0/bin
      source /data/tmplgls/salome/prerequis/install/LICENSE/dlim8.var.sh
      export PATH=/data/tmplgls/salome/prerequis/install/COMMON_64/MeshGems-1.0/bin/Linux_64:$PATH
      cp -r /data/tmplgls/salome/prerequis/install/COMMON_64/MeshGems-1.0/examples .
      cd examples
      mg-cleaner.exe --help
      mg-cleaner.exe --in case7.mesh --out case7-test.mesh --check
      mg-cleaner.exe case7.mesh case7-fix.mesh --fix
      mg-cleaner.exe --in Porsche.mesh --out Porsche-test.mesh --check
      mg-cleaner.exe --in Porsche.mesh --out Porschefix.mesh --fix
      mg-cleaner.exe --in Porsche.mesh --out PorscheNewfix.mesh --fix --resolution_length 0.03
      """
      
      #self.commande="mg-cleaner.exe --in " + self.fichierIn + " --out " + self.fichierOut + " --fix2pass" 
      #return True
      if self.fichierIn=="" and self.MeshIn=="" :
        QMessageBox.critical(self, "Mesh", "select an input mesh")
        return False
      if self.MeshIn!="" : self.prepareFichier()
      if not (os.path.isfile(self.fichierIn)):
        QMessageBox.critical(self, "File", "unable to read GMF Mesh in "+str(self.fichierIn))
        return False
     
      self.commande="mg-cleaner.exe"
      verbosity=str(self.SP_Verbosity.value())
      self.commande+=" --verbose " + verbosity
      self.commande+=" --in " + self.fichierIn
      deb=os.path.splitext(self.fichierIn)
      self.fichierOut=deb[0] + "_fix.mesh"
      self.commande+=" --out "+self.fichierOut
      if self.RB_Fix1.isChecked():
        self.commande+=" --fix1pass"
      else:
        if self.RB_Fix2.isChecked():
          self.commande+=" --fix2pass"
        else:
          self.commande+=" --check"
      if self.CB_PreserveTopology.isChecked():
        self.commande+=" --topology respect"
      else:
        self.commande+=" --topology ignore"
      if self.CB_FillHoles.isChecked(): #no fill holes default
        self.commande+=" --min_hole_size " + str(self.SP_minHoleSize.value())
      if not self.CB_computedToleranceDisplacement.isChecked(): #computed default
        self.commande+=" --tolerance_displacement " + str(self.SP_toleranceDisplacement.value())
      if not self.CB_computedResolutionLength.isChecked(): #computed default
        self.commande+=" --tolerance_displacement " + str(self.SP_resolutionLength.value())
      self.commande+=" --folding_angle " + str(self.SP_foldingAngle.value())
      if self.CB_RemeshPlanes.isChecked(): #no remesh default
        self.commande+=" --remesh_planes"
      if not self.CB_computedOverlapDistance.isChecked(): #computed default
        self.commande+=" --overlap_distance " + str(self.SP_overlapDistance.value())
      self.commande+=" --overlap_angle " + str(self.SP_overlapAngle.value())
      return True
      
  def clean(self):
      self.RB_Check.setChecked(False)
      self.RB_Fix1.setChecked(False)
      self.RB_Fix2.setChecked(True)
      self.CB_PreserveTopology.setChecked(False)
      self.CB_FillHoles.setChecked(False)
      self.CB_RemeshPlanes.setChecked(False)
      
      self.SP_minHoleSize.setProperty("value", 0)
      self.SP_toleranceDisplacement.setProperty("value", 0)
      self.SP_resolutionLength.setProperty("value", 0)
      self.SP_foldingAngle.setProperty("value", 15)
      self.SP_overlapDistance.setProperty("value", 0)
      self.SP_overlapAngle.setProperty("value", 15)
      self.SP_Verbosity.setProperty("value", 3)
      
      self.CB_computedToleranceDisplacement.setChecked(True)
      self.CB_computedResolutionLength.setChecked(True)
      self.CB_computedOverlapDistance.setChecked(True)

__dialog=None
def getDialog():
    """
    This function returns a singleton instance of the plugin dialog.
    c est obligatoire pour faire un show sans parent...
    """
    global __dialog
    if __dialog is None:
        __dialog = MGCleanerMonPlugDialog()
    #else :
    #   __dialog.clean()
    return __dialog


#
# ==============================================================================
# Basic use cases and unit test functions
# ==============================================================================
#
def TEST_MGCleanerMonPlugDialog():
    print 'TEST_MGCleanerMonPlugDialog'
    import sys
    from PyQt4.QtGui import QApplication
    from PyQt4.QtCore import QObject, SIGNAL, SLOT
    app = QApplication(sys.argv)
    QObject.connect(app, SIGNAL("lastWindowClosed()"), app, SLOT("quit()"))

    dlg=MGCleanerMonPlugDialog()
    dlg.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    TEST_MGCleanerMonPlugDialog()
