import Ogre
from Ogre import RTShader, Overlay, Bites
import pyBtOgreLayer


class btOgreExample(Bites.ApplicationContext, Bites.InputListener):

    def __init__(self):
        Bites.ApplicationContext.__init__(self, "btOgreExample")
        Bites.InputListener.__init__(self)
    def loadResources(self):
        self.enableShaderCache()

        # load essential resources for trays/ loading bar
        Ogre.ResourceGroupManager.getSingleton().initialiseResourceGroup("Essential")
        self.createDummyScene()
        self.trays = Bites.TrayManager("Interface", self.getRenderWindow())
        self.addInputListener(self.trays)

        # show loading progress
        self.trays.showLoadingBar(1, 0)
        ret = Bites.ApplicationContext.loadResources(self)

        # clean up
        self.trays.hideLoadingBar()
        self.destroyDummyScene()
        return ret
    def keyPressed(self, evt):
        if evt.keysym.sym == Bites.SDLK_ESCAPE:
            self.getRoot().queueEndRendering()
        return True

    def frameStarted(self, evt):
        Bites.ApplicationContext.frameStarted(self, evt)
        self.bt.step(evt)
        return True

    def shutdown(self):
        print("destoryWorld1")
        self.bt.destoryWorld()
        print("destoryWorld2")
        del app.ctrls
        del app.trays


    def setup(self):
        Bites.ApplicationContext.setup(self)
        self.addInputListener(self)

        root = self.getRoot()
        scn_mgr = root.createSceneManager()

        shadergen = RTShader.ShaderGenerator.getSingleton()
        shadergen.addSceneManager(scn_mgr)  # must be done before we do anything with the scene

        cam = scn_mgr.createCamera("myCam")
        cam.setNearClipDistance(5)
        cam.setAutoAspectRatio(True)
        camnode = scn_mgr.getRootSceneNode().createChildSceneNode()
        camnode.attachObject(cam)
        self.camman = Bites.CameraMan(camnode)
        self.camman.setStyle(Bites.CS_ORBIT)
        self.camman.setYawPitchDist(Ogre.Radian(0), Ogre.Radian(0.3), 15)
        self.addInputListener(self.camman)
        # must keep a reference to ctrls so it does not get deleted
        self.ctrls = Bites.AdvancedRenderControls(self.trays, cam)
        self.addInputListener(self.ctrls)
        vp = self.getRenderWindow().addViewport(cam)
        vp.setBackgroundColour(Ogre.ColourValue(.3, .3, .3))
        
        
        scn_mgr.setAmbientLight(Ogre.ColourValue(.1, .1, .1))

        light = scn_mgr.createLight("MainLight")
        lightnode = scn_mgr.getRootSceneNode().createChildSceneNode()
        lightnode.setPosition(0, 10, 15)
        lightnode.attachObject(light)







   

        ent = scn_mgr.createEntity("ninja.mesh")
        btnode = scn_mgr.getRootSceneNode().createChildSceneNode('ninjabtnode',Ogre.Vector3(0,500,0),Ogre.Quaternion.IDENTITY)
        node = btnode.createChildSceneNode('ninjaScenenode',Ogre.Vector3(0,-ent.getBoundingRadius()/2,0),Ogre.Quaternion.IDENTITY)      
        node.attachObject(ent)
        #print('dir pyBtOgreLayer=========>>')
        #print(dir(pyBtOgreLayer))
        #print(dir(pyBtOgreLayer.btOgreLayer()))



        plane=Ogre.Plane(Ogre.Vector3(0,1,0),0)
        Ogre.MeshManager.getSingleton().createPlane('ground','General',
                                                    plane,100,100,10,10,
                                                    True,
                                                    1,1,1,
                                                    Ogre.Vector3(0,0,1))
        gdent=scn_mgr.createEntity('ground')
        gdent.setMaterialName('Examples/Rockwall')
        scn_mgr.getRootSceneNode().createChildSceneNode().attachObject(gdent)

        
        self.bt=bt=pyBtOgreLayer.btOgreLayer()
        bt.initWorld()
        bt.debugDrawer(scn_mgr.getRootSceneNode())

        bt.addRigidToWorld(ent,btnode)
        bt.addGroundToWorld(gdent)

if __name__ == "__main__":
    app = btOgreExample()
    app.initApp()
    app.getRoot().startRendering()
    app.closeApp()
