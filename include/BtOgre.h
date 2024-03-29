/*
 * =====================================================================================
 *
 *       Filename:  BtOgreGP.h
 *
 *    Description:  The part of BtOgre that handles information transfer from Ogre to
 *                  Bullet (like mesh data for making trimeshes).
 *
 *        Version:  1.0
 *        Created:  27/12/2008 03:29:56 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#ifndef _BtOgrePG_H_
#define _BtOgrePG_H_

#include "btBulletDynamicsCommon.h"
#include "Ogre.h"

namespace BtOgre {

//Converts from and to Bullet and Ogre stuff. Pretty self-explanatory.
struct Convert
{
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

//A MotionState is Bullet's way of informing you about updates to an object.
//Pass this MotionState to a btRigidBody to have your SceneNode updated automaticaly.
class RigidBodyState : public btMotionState
{
    Ogre::Node* mNode;
public:
    RigidBodyState(Ogre::Node* node) : mNode(node) {}

    void getWorldTransform(btTransform& ret) const override
    {
        ret = btTransform(Convert::toBullet(mNode->getOrientation()),
                          Convert::toBullet(mNode->getPosition()));
    }

    void setWorldTransform(const btTransform& in) override
    {
        btQuaternion rot = in.getRotation();
        btVector3 pos = in.getOrigin();
        mNode->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
        mNode->setPosition(pos.x(), pos.y(), pos.z());
    }
};

/// create sphere collider using ogre provided data
btSphereShape* createSphereCollider(const Ogre::MovableObject* mo);
/// create box collider using ogre provided data
btBoxShape* createBoxCollider(const Ogre::MovableObject* mo);
/// create capsule collider using ogre provided data
btCapsuleShape* createCapsuleCollider(const Ogre::MovableObject* mo);
/// create capsule collider using ogre provided data
btCylinderShape* createCylinderCollider(const Ogre::MovableObject* mo);

enum ColliderType
{
	CT_BOX,
	CT_SPHERE,
	CT_CYLINDER,
	CT_CAPSULE,
	CT_TRIMESH,
	CT_HULL
};

struct CollisionListener
{
	virtual ~CollisionListener() {}
    virtual void contact(const Ogre::MovableObject* other, const btManifoldPoint& manifoldPoint) = 0;
};

struct RayResultCallback
{
	virtual ~RayResultCallback() {}
    virtual void addSingleResult(const Ogre::MovableObject* other, float distance) = 0;
};

/// simplified wrapper with automatic memory management
class DynamicsWorld
{
	std::unique_ptr<btCollisionConfiguration> mCollisionConfig;
	std::unique_ptr<btCollisionDispatcher> mDispatcher;
	std::unique_ptr<btConstraintSolver> mSolver;
	std::unique_ptr<btBroadphaseInterface> mBroadphase;
    btDynamicsWorld* mBtWorld;
public:
	explicit DynamicsWorld(const Ogre::Vector3& gravity);
	~DynamicsWorld();
	DynamicsWorld(btDynamicsWorld* btWorld) : mBtWorld(btWorld) {}

	btRigidBody* addRigidBody(float mass, Ogre::Entity* ent, ColliderType ct, CollisionListener* listener = 0,
								int group = 1, int mask = -1);

	btDynamicsWorld* getBtWorld() const { return mBtWorld; }

	void rayTest(const Ogre::Ray& ray, RayResultCallback* callback, float maxDist = 1000);
};

typedef std::vector<Ogre::Vector3> Vector3Array;


class VertexIndexToShape
{
public:
	VertexIndexToShape(const Ogre::Affine3 &transform = Ogre::Affine3::IDENTITY);
	VertexIndexToShape(Ogre::Renderable *rend, const Ogre::Affine3 &transform = Ogre::Affine3::IDENTITY);
	VertexIndexToShape(const Ogre::Entity *entity, const Ogre::Affine3 &transform = Ogre::Affine3::IDENTITY);
	~VertexIndexToShape();

	Ogre::Real getRadius();
	Ogre::Vector3 getSize();

	btBvhTriangleMeshShape* createTrimesh();
	btConvexHullShape* createConvex();

	void addEntity(const Ogre::Entity *entity,const Ogre::Affine3 &transform = Ogre::Affine3::IDENTITY);
	void addMesh(const Ogre::MeshPtr &mesh, const Ogre::Affine3 &transform = Ogre::Affine3::IDENTITY);

	const Ogre::Vector3* getVertices();
	unsigned int getVertexCount();
	const unsigned int* getIndices();
	unsigned int getIndexCount();

protected:

	void addStaticVertexData(const Ogre::VertexData *vertex_data);

	void addAnimatedVertexData(const Ogre::VertexData *vertex_data,
		const Ogre::VertexData *blended_data,
		const Ogre::Mesh::IndexMap *indexMap);

	void addIndexData(Ogre::IndexData *data, const unsigned int offset = 0);


protected:
	Ogre::Vector3*	    mVertexBuffer;
	unsigned int*       mIndexBuffer;
	unsigned int        mVertexCount;
	unsigned int        mIndexCount;

	Ogre::Vector3		mBounds;
	Ogre::Real		    mBoundRadius;

	typedef std::map<unsigned char, Vector3Array*> BoneIndex;
	BoneIndex           *mBoneIndex;

	Ogre::Affine3		mTransform;

	Ogre::Vector3		mScale;
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
		mLines.setCastShadows(false);
        mNode->attachObject(&mLines);
		mWorld->setDebugDrawer(this);
    }

    void update()
    {
		mWorld->debugDrawWorld();
		if(!mLines.getSections().empty()) // begin was called
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
