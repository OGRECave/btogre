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
#include "BtOgrePG.h"
#include "BtOgreGP.h"
#include "BtOgreExtras.h"

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
	btAxisSweep3 *mBroadphase;
	btDefaultCollisionConfiguration *mCollisionConfig;
	btCollisionDispatcher *mDispatcher;
	btSequentialImpulseConstraintSolver *mSolver;

    btDynamicsWorld *mBtWorld;

	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mCamera;

	Ogre::SceneNode *mNinjaNode;
	Ogre::Entity *mNinjaEntity;
	btRigidBody *mNinjaBody;
	btCollisionShape *mNinjaShape;

	Ogre::Entity *mGroundEntity;
	btRigidBody *mGroundBody;
	btBvhTriangleMeshShape *mGroundShape;

	OgreBites::CameraMan *mCamMan;

	bool mDebugOn;
    BtOgre::DebugDrawer *mDbgDraw;

    public:
	BtOgreTestApplication() : OgreBites::ApplicationContext("BtOgre")
	{
	    //Bullet initialisation.
	    mBroadphase = new btAxisSweep3(btVector3(-10000,-10000,-10000), btVector3(10000,10000,10000), 1024);
	    mCollisionConfig = new btDefaultCollisionConfiguration();
	    mDispatcher = new btCollisionDispatcher(mCollisionConfig);
	    mSolver = new btSequentialImpulseConstraintSolver();

	    mBtWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
	    mBtWorld->setGravity(btVector3(0,-9.8,0));

		mDebugOn = true;
	}

	void setupInput(bool) {}

	void shutdown()
	{
            //Free rigid bodies
            mBtWorld->removeRigidBody(mNinjaBody);
            delete mNinjaBody->getMotionState();
            delete mNinjaBody;
            delete mNinjaShape;

            mBtWorld->removeRigidBody(mGroundBody);
            delete mGroundBody->getMotionState();
            delete mGroundBody;
            delete mGroundShape->getMeshInterface();
            delete mGroundShape;

	    //Free Bullet stuff.
            delete mDbgDraw;
            delete mBtWorld;

	    delete mSolver;
	    delete mDispatcher;
	    delete mCollisionConfig;
	    delete mBroadphase;

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

	    mDbgDraw = new BtOgre::DebugDrawer(mSceneMgr->getRootSceneNode(), mBtWorld);

	    //----------------------------------------------------------
	    // Ninja!
	    //----------------------------------------------------------

		//Create Ogre stuff.
	    mNinjaEntity = mSceneMgr->createEntity("ninjaEntity", "Player.mesh");
	    mNinjaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,10,0));
	    mNinjaNode->attachObject(mNinjaEntity);

	    //Create shape.
	    mNinjaShape = BtOgre::createSphereCollider(mNinjaEntity);

	    //Calculate inertia.
	    btScalar mass = 5;
	    btVector3 inertia;
	    mNinjaShape->calculateLocalInertia(mass, inertia);

	    //Create BtOgre MotionState (connects Ogre and Bullet).
	    BtOgre::RigidBodyState *ninjaState = new BtOgre::RigidBodyState(mNinjaNode);

	    //Create the Body.
	    mNinjaBody = new btRigidBody(mass, ninjaState, mNinjaShape, inertia);
	    mBtWorld->addRigidBody(mNinjaBody);

	    //----------------------------------------------------------
	    // Ground!
	    //----------------------------------------------------------

	    //Create Ogre stuff.
	    mGroundEntity = mSceneMgr->createEntity("groundEntity", "TestLevel_b0.mesh");
	    mSceneMgr->getRootSceneNode()->createChildSceneNode("groundNode")->attachObject(mGroundEntity);

	    //Create the ground shape.
	    mGroundShape = BtOgre::StaticMeshToShapeConverter(mGroundEntity).createTrimesh();

	    //Create MotionState (no need for BtOgre here, you can use it if you want to though).
	    btDefaultMotionState* groundState = new btDefaultMotionState(
		    btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));

	    //Create the Body.
	    mGroundBody = new btRigidBody(0, groundState, mGroundShape, btVector3(0,0,0));
	    mBtWorld->addRigidBody(mGroundBody);
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
        mBtWorld->stepSimulation(evt.timeSinceLastFrame, 10);

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
