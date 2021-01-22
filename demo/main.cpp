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
#include "BtOgre.h"

using namespace Ogre;

/*
 * =====================================================================================
 *        Class:  BtOgreTestApplication
 *  Description:  Derives from ExampleApplication and overrides stuff.
 * =====================================================================================
 */

class BtOgreTestApplication : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
    std::unique_ptr<BtOgre::DynamicsWorld> mDynWorld;

	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mCamera;

	Ogre::SceneNode *mNinjaNode;
	Ogre::Entity *mNinjaEntity;
	Ogre::Entity *mGroundEntity;

	OgreBites::CameraMan *mCamMan;

	bool mDebugOn;
    std::unique_ptr<BtOgre::DebugDrawer> mDbgDraw;

    public:
	BtOgreTestApplication() : OgreBites::ApplicationContext("BtOgre")
	{
        mDynWorld.reset(new BtOgre::DynamicsWorld(Ogre::Vector3(0,-9.8,0)));
		mDebugOn = true;
	}

	void shutdown()
	{
		OgreBites::ApplicationContext::shutdown();
        mDbgDraw->clear();
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

	    mDbgDraw.reset(new BtOgre::DebugDrawer(mSceneMgr->getRootSceneNode(), mDynWorld->getBtWorld()));

	    //----------------------------------------------------------
	    // Ninja!
	    //----------------------------------------------------------

		//Create Ogre stuff.
	    mNinjaEntity = mSceneMgr->createEntity("ninjaEntity", "Player.mesh");
	    mNinjaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,10,0));
	    mNinjaNode->attachObject(mNinjaEntity);

	    //Create the Body.
        mDynWorld->addRigidBody(5, mNinjaEntity, BtOgre::CT_SPHERE);

	    //----------------------------------------------------------
	    // Ground!
	    //----------------------------------------------------------

	    //Create Ogre stuff.
	    mGroundEntity = mSceneMgr->createEntity("groundEntity", "TestLevel_b0.mesh");
	    mSceneMgr->getRootSceneNode()->createChildSceneNode("groundNode")->attachObject(mGroundEntity);

	    //Create the Body.
        mDynWorld->addRigidBody(0, mGroundEntity, BtOgre::CT_TRIMESH);
	}

	bool keyPressed(const OgreBites::KeyboardEvent& evt)
	{
		using namespace OgreBites;

	    if (evt.keysym.sym == SDLK_ESCAPE)
	    {
	        getRoot()->queueEndRendering();
	    }
	    else if(evt.keysym.sym == SDLK_F3)
		{
	        mDebugOn = !mDebugOn;

			if (!mDebugOn)
				mDbgDraw->clear();
		}
	    return true;
	}

    bool frameStarted(const FrameEvent &evt)
    {
        OgreBites::ApplicationContext::frameStarted(evt);

        //Update Bullet world. Don't forget the debugDrawWorld() part!
        mDynWorld->getBtWorld()->stepSimulation(evt.timeSinceLastFrame, 10);

        if(mDebugOn)
        	mDbgDraw->update();

        return true;
    }
};

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  main() function. Need say more?
 * =====================================================================================
 */

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    BtOgreTestApplication app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();

    return 0;
}
