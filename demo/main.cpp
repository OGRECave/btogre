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
 *    Namespace:  Globals
 *  Description:  A dirty 'globals' hack.
 * =====================================================================================
 */

namespace Globals
{
    btDynamicsWorld *phyWorld;
    BtOgre::DebugDrawer *dbgdraw;
}

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

    public:
	BtOgreTestApplication() : OgreBites::ApplicationContext("BtOgre", false)
	{
	    //Bullet initialisation.
	    mBroadphase = new btAxisSweep3(btVector3(-10000,-10000,-10000), btVector3(10000,10000,10000), 1024);
	    mCollisionConfig = new btDefaultCollisionConfiguration();
	    mDispatcher = new btCollisionDispatcher(mCollisionConfig);
	    mSolver = new btSequentialImpulseConstraintSolver();

	    Globals::phyWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
	    Globals::phyWorld->setGravity(btVector3(0,-9.8,0));
	}

	void setupInput(bool) {}

	void shutdown()
	{
            //Free rigid bodies
            Globals::phyWorld->removeRigidBody(mNinjaBody);
            delete mNinjaBody->getMotionState();
            delete mNinjaBody;
            delete mNinjaShape;

            Globals::phyWorld->removeRigidBody(mGroundBody);
            delete mGroundBody->getMotionState();
            delete mGroundBody;
            delete mGroundShape->getMeshInterface();
            delete mGroundShape;

	    //Free Bullet stuff.
            delete Globals::dbgdraw;
            delete Globals::phyWorld;

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

	    Globals::dbgdraw = new BtOgre::DebugDrawer(mSceneMgr->getRootSceneNode(), Globals::phyWorld);
	    Globals::phyWorld->setDebugDrawer(Globals::dbgdraw);

	    //----------------------------------------------------------
	    // Ninja!
	    //----------------------------------------------------------

	    Vector3 pos = Vector3(0,10,0);
	    Quaternion rot = Quaternion::IDENTITY;

	    //Create Ogre stuff.

	    mNinjaEntity = mSceneMgr->createEntity("ninjaEntity", "Player.mesh");
	    mNinjaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("ninjaSceneNode", pos, rot);
	    mNinjaNode->attachObject(mNinjaEntity);

	    //Create shape.
	    BtOgre::StaticMeshToShapeConverter converter(mNinjaEntity);
	    mNinjaShape = converter.createSphere();

	    //Calculate inertia.
	    btScalar mass = 5;
	    btVector3 inertia;
	    mNinjaShape->calculateLocalInertia(mass, inertia);

	    //Create BtOgre MotionState (connects Ogre and Bullet).
	    BtOgre::RigidBodyState *ninjaState = new BtOgre::RigidBodyState(mNinjaNode);

	    //Create the Body.
	    mNinjaBody = new btRigidBody(mass, ninjaState, mNinjaShape, inertia);
	    Globals::phyWorld->addRigidBody(mNinjaBody);

	    //----------------------------------------------------------
	    // Ground!
	    //----------------------------------------------------------

	    //Create Ogre stuff.
	    //MeshManager::getSingleton().createPlane("groundPlane", "General", Plane(Vector3::UNIT_Y, 0), 100, 100, 
	    //10, 10, true, 1, 5, 5, Vector3::UNIT_Z);
	    mGroundEntity = mSceneMgr->createEntity("groundEntity", "TestLevel_b0.mesh");
	    //mGroundEntity->setMaterialName("Examples/Rockwall");
	    mSceneMgr->getRootSceneNode()->createChildSceneNode("groundNode")->attachObject(mGroundEntity);

	    //Create the ground shape.
	    BtOgre::StaticMeshToShapeConverter converter2(mGroundEntity);
	    mGroundShape = converter2.createTrimesh();

	    //Create MotionState (no need for BtOgre here, you can use it if you want to though).
	    btDefaultMotionState* groundState = new btDefaultMotionState(
		    btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));

	    //Create the Body.
	    mGroundBody = new btRigidBody(0, groundState, mGroundShape, btVector3(0,0,0));
	    Globals::phyWorld->addRigidBody(mGroundBody);
	}

	bool keyPressed(const OgreBites::KeyboardEvent& evt)
	{
	    if (evt.keysym.sym == SDLK_ESCAPE)
	    {
	        getRoot()->queueEndRendering();
	    }
	    else if(evt.keysym.sym == SDLK_F3) {
	        static bool draw = true;
	        draw = !draw;
	        Globals::dbgdraw->setDebugMode(draw);
	    }
	    return true;
	}

    bool frameStarted(const FrameEvent &evt)
    {
        OgreBites::ApplicationContext::frameStarted(evt);

        //Update Bullet world. Don't forget the debugDrawWorld() part!
        Globals::phyWorld->stepSimulation(evt.timeSinceLastFrame, 10);
        Globals::phyWorld->debugDrawWorld();

        //Shows debug if F3 key down.

        Globals::dbgdraw->step();

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
