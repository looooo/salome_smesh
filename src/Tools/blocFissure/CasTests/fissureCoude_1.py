# -*- coding: utf-8 -*-

from blocFissure.gmu.fissureCoude  import fissureCoude

class fissureCoude_1(fissureCoude):
  """
  probleme de fissure du Coude :
  adaptation maillage
  """

# ---------------------------------------------------------------------------
  def setParamGeometrieSaine(self):
    """
    Parametres geometriques du tuyau coude sain:
    angleCoude
    r_cintr
    l_tube_p1
    l_tube_p2
    epais
    de
    """
    self.geomParams = dict(angleCoude = 60,
                           r_cintr    = 1200,
                           l_tube_p1  = 1600,
                           l_tube_p2  = 1200,
                           epais      = 40,
                           de         = 760)

  # ---------------------------------------------------------------------------
  def setParamMaillageSain(self):
    self.meshParams = dict(n_long_p1    = 16,
                           n_ep         = 3,
                           n_long_coude = 15,
                           n_circ_g     = 20,
                           n_circ_d     = 20,
                           n_long_p2    = 12)

# ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    parametres de la fissure pour le tuyau coude
    profondeur  : 0 < profondeur <= epaisseur
    rayonPipe   : rayon du pipe correspondant au maillage rayonnant
    lenSegPipe  : longueur des mailles rayonnantes le long du fond de fissure (= rayonPipe par defaut)
    azimut      : entre 0 et 360°
    alpha       : 0 < alpha < angleCoude
    longueur    : <=2*profondeur ==> force une fissure elliptique (longueur/profondeur = grand axe/petit axe).
    orientation : 0° : longitudinale, 90° : circonferentielle, autre : uniquement fissures elliptiques
    lgInfluence : distance autour de la shape de fissure a remailler (si 0, pris egal a profondeur. A ajuster selon le maillage)
    elliptique  : True : fissure elliptique (longueur/profondeur = grand axe/petit axe); False : fissure longue (fond de fissure de profondeur constante, demi-cercles aux extremites)
    pointIn_x   : optionnel coordonnees x d'un point dans le solide, pas trop loin du centre du fond de fissure (idem y,z)
    externe     : True : fissure face externe, False : fissure face interne
    """
    self.shapeFissureParams = dict(profondeur  = 10,
                                   rayonPipe   = 2.5,
                                   lenSegPipe  = 2.5,
                                   azimut      = 160,
                                   alpha       = 20,
                                   longueur    = 400,
                                   orientation = 90,
                                   lgInfluence = 50,
                                   elliptique  = False,
                                   externe     = True)

# ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    """
    Parametres du maillage de la fissure pour le tuyau coude
    Voir egalement setParamShapeFissure, parametres rayonPipe et lenSegPipe.
    nbSegRad = nombre de couronnes
    nbSegCercle = nombre de secteurs
    areteFaceFissure = taille cible de l'arete des triangles en face de fissure.
    """
    self.maillageFissureParams = dict(nomRep        = '.',
                                      nomFicSain    = self.nomCas,
                                      nomFicFissure = 'fissure_' + self.nomCas,
                                      nbsegRad      = 5,
                                      nbsegCercle   = 6,
                                      areteFaceFissure = 5)

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict(Entity_Node            = 77917,
                                          Entity_Quad_Edge       = 975,
                                          Entity_Quad_Triangle   = 2182,
                                          Entity_Quad_Quadrangle = 6842,
                                          Entity_Quad_Tetra      = 20135,
                                          Entity_Quad_Hexa       = 8994,
                                          Entity_Quad_Penta      = 972,
                                          Entity_Quad_Pyramid    = 1038)

