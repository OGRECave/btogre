/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  BtOgre test application, main file.
 *
 *        Version:  1.0
 *        Created:  01/14/2009 05:48:31 PM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#include "OgreApplicationContext.h"
#include "OgreCameraMan.h"
#include "btOgreLayer.h"
#include <iostream>
using namespace std;

using namespace Ogre;

/*
 * =====================================================================================
 *        Class:  BtOgreTestApplication
 *  Description:  Derives from ExampleApplication and overrides stuff.
 * =====================================================================================
 */

class BtOgreTestApplication : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
    protected:

	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mCamera;

	Ogre::SceneNode *mNinjaNode;
	Ogre::Entity *mNinjaEntity;
	Ogre::SceneNode* mNinjaBtNode;

	Ogre::Entity *mGroundEntity;

	OgreBites::CameraMan *mCamMan;
	

    public:
	btOgreLayer *layer;

	BtOgreTestApplication() : OgreBites::ApplicationContext("BtOgre")
	{
		layer = new btOgreLayer();
		layer->initWorld();
	}

	void setupInput(bool) {}

	void shutdown()
	{
		layer->destoryWorld();
	    OgreBites::ApplicationContext::shutdown();
	}

	void setup(void)
	{
	    OgreBites::ApplicationContext::setup();
	    addInputListener(this);

	    mSceneMgr = getRoot()->createSceneManager();

	    // register our scene with the RTSS
	    Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
	    shadergen->addSceneManager(mSceneMgr);

	    // without light we would just get a black screen
        Ogre::Light* light = mSceneMgr->createLight("MainLight");
        Ogre::SceneNode* lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        lightNode->setPosition(0, 10, 15);
        lightNode->attachObject(light);

	    // create the camera
	    mCamera = mSceneMgr->createCamera("myCam");
	    mCamera->setAutoAspectRatio(true);

	    Ogre::SceneNode* camnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	    camnode->attachObject(mCamera);

	    // and tell it to render into the main window
	    getRenderWindow()->addViewport(mCamera);

	    mCamMan = new OgreBites::CameraMan(camnode);
	    mCamMan->setStyle(OgreBites::CS_ORBIT);
	    mCamMan->setYawPitchDist(Ogre::Degree(45), Ogre::Degree(45), 20);
	    addInputListener(mCamMan);

	    //Some normal stuff.
	    mSceneMgr->setAmbientLight(ColourValue(0.7,0.7,0.7));

	    mCamera->setNearClipDistance(0.05);
	    LogManager::getSingleton().setLogDetail(LL_BOREME);

	    //----------------------------------------------------------
	    // Debug drawing!
	    //----------------------------------------------------------

		layer->debugDrawer(mSceneMgr->getRootSceneNode());

	    //----------------------------------------------------------
	    // Ninja!
	    //----------------------------------------------------------

	    Ogre::Vector3 pos = Vector3(0,500,0);
	    Quaternion rot = Quaternion::IDENTITY;

	    //Create Ogre stuff.

	    mNinjaEntity = mSceneMgr->createEntity("ninjaEntity", "ninja.mesh");
		mNinjaBtNode= mSceneMgr->getRootSceneNode()->createChildSceneNode("ninjaBtNode", pos, rot);

		
		mNinjaNode = mNinjaBtNode->createChildSceneNode("ninjaSceneNode", Vector3(0, -mNinjaEntity->getBoundingRadius()/2,0), rot);
		mNinjaNode->attachObject(mNinjaEntity);

		layer->addRigidToWorld(mNinjaEntity, mNinjaBtNode);
	    //----------------------------------------------------------
	    // Ground!
	    //----------------------------------------------------------

	    //Create Ogre stuff.
	    MeshManager::getSingleton().createPlane("groundPlane", "General", Plane(Vector3(0,1,0), 0), 100, 100, 
			10, 10, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
		//10, 10, true, 1, 5, 5, Vector3(0,0,1));
			
	    mGroundEntity = mSceneMgr->createEntity("groundEntity", "groundPlane");
	    mGroundEntity->setMaterialName("Examples/Rockwall");
	    mSceneMgr->getRootSceneNode()->createChildSceneNode("groundNode")->attachObject(mGroundEntity);

		layer->addGroundToWorld(mGroundEntity);
	}

	bool keyPressed(const OgreBites::KeyboardEvent& evt)
	{
		using namespace OgreBites;

	    if (evt.keysym.sym == SDLK_ESCAPE)
	    {
	        getRoot()->queueEndRendering();
	    }
	    else if(evt.keysym.sym == SDLK_F3) {
	        static bool draw = true;
	        draw = !draw;
	        //Globals::dbgdraw->setDebugMode(draw);
	    }
	    return true;
	}

    bool frameStarted(const FrameEvent &evt)
    {
        OgreBites::ApplicationContext::frameStarted(evt);

        //Update Bullet world. Don't forget the debugDrawWorld() part!
		layer->step(evt);
		Vector3 v = mNinjaNode->_getDerivedPosition();
		Vector3 v2 = mNinjaBtNode->_getDerivedPosition();
		cout << v[0] << "," << v[1] << "," << v[2] <<"\t"<<v2[0]<<" "<<v2[1]<<" "<<v2[2]<<endl;
        return true;
    }
};

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  main() function. Need say more?
 * =====================================================================================
 */

 //! [main]
int main(int argc, char *argv[])

{
    // Create application object
    BtOgreTestApplication app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();

    return 0;
}
