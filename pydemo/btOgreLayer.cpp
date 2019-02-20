#include "BtOgrePG.h"
#include "BtOgreGP.h"
#include "BtOgreExtras.h"
#include "btOgreLayer.h"
#include <vector>

using namespace Ogre;

void btOgreLayer::basicConfig()
{
	//todo: config the gravity or broadphase in future
}

void btOgreLayer::initWorld()
{
	//Bullet initialisation.
	mBroadphase = new btAxisSweep3(btVector3(-10000, -10000, -10000), btVector3(10000, 10000, 10000), 1024);
	mCollisionConfig = new btDefaultCollisionConfiguration();
	mDispatcher = new btCollisionDispatcher(mCollisionConfig);
	mSolver = new btSequentialImpulseConstraintSolver();

	phyWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
	phyWorld->setGravity(btVector3(0, -9.8, 0));

}

void btOgreLayer::debugDrawer(Ogre::SceneNode *rootNode)
{
	// Debug drawing!
	dbgdraw = new BtOgre::DebugDrawer(rootNode, phyWorld);
	phyWorld->setDebugDrawer(dbgdraw);
}

void btOgreLayer::addRigidToWorld(Ogre::Entity *ent, Ogre::SceneNode *node)
{

	//todo: add shape and body into list in future
	btCollisionShape *tShape;
	BtOgre::StaticMeshToShapeConverter converter(ent);
	tShape = converter.createSphere();
	mShaps.push_back(tShape);
	//Calculate inertia.
	btScalar mass = 5;
	btVector3 inertia;
	tShape->calculateLocalInertia(mass, inertia);

	//Create BtOgre MotionState (connects Ogre and Bullet).
	BtOgre::RigidBodyState *tState = new BtOgre::RigidBodyState(node);
	//Create the Body.
	btRigidBody *tBody;
	tBody = new btRigidBody(mass, tState, tShape, inertia);
	mRigidbodys.push_back(tBody);
	phyWorld->addRigidBody(tBody);

}

void btOgreLayer::addGroundToWorld(Ogre::Entity *ent)
{
	//Create the ground shape.
	BtOgre::StaticMeshToShapeConverter converter2(ent);
	mGroundShape = converter2.createTrimesh();

	//Create MotionState (no need for BtOgre here, you can use it if you want to though).
	btDefaultMotionState* groundState = new btDefaultMotionState(
		btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

	//Create the Body.
	btRigidBody *mGroundBody;
	mGroundBody = new btRigidBody(0, groundState, mGroundShape, btVector3(0, 0, 0));
	phyWorld->addRigidBody(mGroundBody);
	mRigidbodys.push_back(mGroundBody);
}
void btOgreLayer::step(const FrameEvent &evt)
{
	phyWorld->stepSimulation(evt.timeSinceLastFrame, 10);
	phyWorld->debugDrawWorld();
	//Shows debug if F3 key down.
	dbgdraw->step();
}

void btOgreLayer::destoryWorld()
{
	//Free rigid bodies
	vector<btCollisionShape *>::iterator itShaps;
	vector<btRigidBody *>::iterator itRigidbodys;
	for (itRigidbodys = mRigidbodys.begin();itRigidbodys != mRigidbodys.end();itRigidbodys++) {
		phyWorld->removeRigidBody(*itRigidbodys);
		delete (*itRigidbodys)->getMotionState();
		delete (*itRigidbodys);
	}
	for (itShaps = mShaps.begin();itShaps != mShaps.end();itShaps++)
		delete (*itShaps);
	delete mGroundShape->getMeshInterface();
	delete mGroundShape;

	//Free Bullet stuff.
	delete dbgdraw;
	delete phyWorld;
	delete mSolver;
	delete mDispatcher;
	delete mCollisionConfig;
	delete mBroadphase;

}