/*
 * =====================================================================================
 *
 *       Filename:  BtOgreExtras.h
 *
 *    Description:  Contains the Ogre Mesh to Bullet Shape converters.
 *
 *        Version:  1.0
 *        Created:  27/12/2008 01:45:56 PM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#ifndef _BtOgreShapes_H_
#define _BtOgreShapes_H_

#include "btBulletDynamicsCommon.h"
#include "OgreSceneNode.h"
#include "OgreManualObject.h"

#include "OgreLogManager.h"

namespace BtOgre
{

typedef std::vector<Ogre::Vector3> Vector3Array;

//Converts from and to Bullet and Ogre stuff. Pretty self-explanatory.
class Convert
{
public:
	static btQuaternion toBullet(const Ogre::Quaternion &q)
	{
		return btQuaternion(q.x, q.y, q.z, q.w);
	}
	static btVector3 toBullet(const Ogre::Vector3 &v)
	{
		return btVector3(v.x, v.y, v.z);
	}

	static Ogre::Quaternion toOgre(const btQuaternion &q)
	{
		return Ogre::Quaternion(q.w(), q.x(), q.y(), q.z());
	}
	static Ogre::Vector3 toOgre(const btVector3 &v)
	{
		return Ogre::Vector3(v.x(), v.y(), v.z());
	}
};

class DebugDrawer : public btIDebugDraw
{
	Ogre::SceneNode *mNode;
	btDynamicsWorld *mWorld;

    Ogre::ManualObject mLines;
	int mDebugMode;
public:
    DebugDrawer(Ogre::SceneNode* node, btDynamicsWorld* world)
        : mNode(node), mWorld(world), mLines(""), mDebugMode(DBG_DrawWireframe)
    {
        mNode->attachObject(&mLines);
		mWorld->setDebugDrawer(this);
    }

    void update()
    {
		mWorld->debugDrawWorld();
		mLines.end();
	}

	void drawLine(const btVector3& from,const btVector3& to,const btVector3& color);

	void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
	{
        drawLine(PointOnB, PointOnB + normalOnB * distance * 20, color);
	}

	void reportErrorWarning(const char* warningString)
	{
		Ogre::LogManager::getSingleton().logWarning(warningString);
	}

	void draw3dText(const btVector3& location,const char* textString)
	{
	}

	void setDebugMode(int mode)
	{
		mDebugMode = mode;

		if (mDebugMode == DBG_NoDebug)
			clear();
	}

	void clear() { mLines.clear(); }

	int getDebugMode() const { return mDebugMode; }
};

}

#endif





