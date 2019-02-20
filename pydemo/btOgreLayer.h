#pragma once
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include "BtOgrePG.h"
#include "BtOgreGP.h"
#include "BtOgreExtras.h"
#include <vector>

namespace Ogre
{
	class btOgreLayer
	{
	public:
		btDynamicsWorld *phyWorld;
		BtOgre::DebugDrawer *dbgdraw=NULL;
		btAxisSweep3 *mBroadphase;
		btDefaultCollisionConfiguration *mCollisionConfig;
		btCollisionDispatcher *mDispatcher;
		btSequentialImpulseConstraintSolver *mSolver;
		btBvhTriangleMeshShape *mGroundShape;
		std::vector<btCollisionShape *>mShaps;
		std::vector<btRigidBody *>mRigidbodys;

		void basicConfig();
		void initWorld();
		void destoryWorld();
		void debugDrawer(Ogre::SceneNode *);
		void addRigidToWorld(Ogre::Entity *, Ogre::SceneNode *);
		void addGroundToWorld(Ogre::Entity *);
		void step(const FrameEvent &evt);
	};
}
