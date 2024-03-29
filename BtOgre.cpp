/*
 * =============================================================================================
 *
 *       Filename:  BtOgre.cpp
 *
 *    Description:  BtOgre implementation.
 *
 *        Version:  1.0
 *        Created:  27/12/2008 01:47:56 PM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =============================================================================================
 */

#include "BtOgre.h"

using namespace Ogre;

namespace BtOgre {

typedef std::pair<unsigned short, Vector3Array*> BoneKeyIndex;

btSphereShape* createSphereCollider(const Ogre::MovableObject* mo)
{
	OgreAssert(mo->getParentSceneNode(), "MovableObject must be attached");

	auto shape = new btSphereShape(mo->getBoundingRadius());
	shape->setLocalScaling(Convert::toBullet(mo->getParentSceneNode()->getScale()));

	return shape;
}
btBoxShape* createBoxCollider(const Ogre::MovableObject* mo)
{
	OgreAssert(mo->getParentSceneNode(), "MovableObject must be attached");

	auto shape = new btBoxShape(Convert::toBullet(mo->getBoundingBox().getHalfSize()));
	shape->setLocalScaling(Convert::toBullet(mo->getParentSceneNode()->getScale()));

	return shape;
}

btCapsuleShape* createCapsuleCollider(const Ogre::MovableObject* mo)
{
    OgreAssert(mo->getParentSceneNode(), "MovableObject must be attached");

    auto sz = mo->getBoundingBox().getHalfSize();

    btScalar height = std::max(sz.x, std::max(sz.y, sz.z));
    btScalar radius;
    btCapsuleShape* shape;
    // Orient the capsule such that its height is aligned with the largest dimension.
    if (height == sz.y)
    {
        radius = std::max(sz.x, sz.z);
        shape = new btCapsuleShape(radius, 2 * height - 2 * radius);
    }
    else if (height == sz.x)
    {
        radius = std::max(sz.y, sz.z);
        shape = new btCapsuleShapeX(radius, 2 * height - 2 * radius);
    }
    else
    {
        radius = std::max(sz.x, sz.y);
        shape = new btCapsuleShapeZ(radius, 2 * height - 2 * radius);
    }

    shape->setLocalScaling(Convert::toBullet(mo->getParentSceneNode()->getScale()));

    return shape;
}

/// create capsule collider using ogre provided data
btCylinderShape* createCylinderCollider(const Ogre::MovableObject* mo)
{
    OgreAssert(mo->getParentSceneNode(), "MovableObject must be attached");

    auto sz = Convert::toBullet(mo->getBoundingBox().getHalfSize());

    btScalar height = std::max(sz.x(), std::max(sz.y(), sz.z()));
    btCylinderShape* shape;
    // Orient the capsule such that its height is aligned with the largest dimension.
    if (height == sz.y())
    {
        shape = new btCylinderShape(sz);
    }
    else if (height == sz.x())
    {
        shape = new btCylinderShapeX(sz);
    }
    else
    {
        shape = new btCylinderShapeZ(sz);
    }

    shape->setLocalScaling(Convert::toBullet(mo->getParentSceneNode()->getScale()));

    return shape;
}

struct EntityCollisionListener
{
	const Ogre::MovableObject* entity;
	CollisionListener* listener;
};

static void onTick(btDynamicsWorld* world, btScalar timeStep)
{
	int numManifolds = world->getDispatcher()->getNumManifolds();
	auto manifolds = world->getDispatcher()->getInternalManifoldPointer();
    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* manifold = manifolds[i];

        for (int j = 0; j < manifold->getNumContacts(); j++)
        {
			const btManifoldPoint& mp = manifold->getContactPoint(i);
           	auto body0 = static_cast<EntityCollisionListener*>(manifold->getBody0()->getUserPointer());
			auto body1 = static_cast<EntityCollisionListener*>(manifold->getBody1()->getUserPointer());
			if(body0->listener)
				body0->listener->contact(body1->entity, mp);
			if(body1->listener)
				body1->listener->contact(body0->entity, mp);
        }
    }
}

/// wrapper with automatic memory management
class RigidBody
{
	btRigidBody* mBtBody;
	btDynamicsWorld* mBtWorld;
public:
    RigidBody(btRigidBody* btBody, btDynamicsWorld* btWorld) : mBtBody(btBody), mBtWorld(btWorld) {}
	~RigidBody()
	{
		mBtWorld->removeRigidBody(mBtBody);
		delete (EntityCollisionListener*)mBtBody->getUserPointer();
		delete mBtBody->getMotionState();
		delete mBtBody->getCollisionShape();
		delete mBtBody;
	}

	btRigidBody* getBtBody() const { return mBtBody; }
};

DynamicsWorld::DynamicsWorld(const Ogre::Vector3& gravity)
{
	//Bullet initialisation.
	mCollisionConfig.reset(new btDefaultCollisionConfiguration());
	mDispatcher.reset(new btCollisionDispatcher(mCollisionConfig.get()));
	mSolver.reset(new btSequentialImpulseConstraintSolver());
	mBroadphase.reset(new btDbvtBroadphase());

	mBtWorld = new btDiscreteDynamicsWorld(mDispatcher.get(), mBroadphase.get(), mSolver.get(),
											mCollisionConfig.get());
	mBtWorld->setGravity(Convert::toBullet(gravity));
	mBtWorld->setInternalTickCallback(onTick);
}

btRigidBody* DynamicsWorld::addRigidBody(float mass, Ogre::Entity* ent, ColliderType ct,
                                         CollisionListener* listener, int group, int mask)
{
    auto node = ent->getParentSceneNode();
	OgreAssert(node, "entity must be attached");
    RigidBodyState* state = new RigidBodyState(node);

	if(ent->hasSkeleton())
	{
		ent->addSoftwareAnimationRequest(false);
		ent->_updateAnimation();
		ent->setUpdateBoundingBoxFromSkeleton(true);
	}

    btCollisionShape* cs = NULL;
    switch (ct)
    {
    case CT_BOX:
        cs = createBoxCollider(ent);
        break;
    case CT_SPHERE:
        cs = createSphereCollider(ent);
        break;
	case CT_CYLINDER:
		cs = createCylinderCollider(ent);
		break;
	case CT_CAPSULE:
		cs = createCapsuleCollider(ent);
        break;
    case CT_TRIMESH:
        cs = VertexIndexToShape(ent).createTrimesh();
        break;
    case CT_HULL:
        cs = VertexIndexToShape(ent).createConvex();
        break;
    }

	if(ent->hasSkeleton())
		ent->removeSoftwareAnimationRequest(false);

    btVector3 inertia(0, 0, 0);
    if(mass != 0) // mass = 0 -> static
        cs->calculateLocalInertia(mass, inertia);
    
    auto rb = new btRigidBody(mass, state, cs, inertia);
    mBtWorld->addRigidBody(rb, group, mask);
	rb->setUserPointer(new EntityCollisionListener{ent, listener});

    // transfer ownership to node
    auto bodyWrapper = std::make_shared<RigidBody>(rb, mBtWorld);
    node->getUserObjectBindings().setUserAny("BtRigidBody", bodyWrapper);

	return rb;
}

struct RayResultCallbackWrapper : public btCollisionWorld::RayResultCallback
{
    BtOgre::RayResultCallback* mCallback;
    float mMaxDistance;
    RayResultCallbackWrapper(BtOgre::RayResultCallback* callback, float maxDist)
        : mCallback(callback), mMaxDistance(maxDist)
    {
    }
    btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
    {
        auto body0 = static_cast<const EntityCollisionListener*>(rayResult.m_collisionObject->getUserPointer());
        mCallback->addSingleResult(body0->entity, rayResult.m_hitFraction * mMaxDistance);
        return rayResult.m_hitFraction;
    }
};

void DynamicsWorld::rayTest(const Ogre::Ray& ray, RayResultCallback* callback, float maxDist)
{
	RayResultCallbackWrapper wrapper(callback, maxDist);
	btVector3 from = Convert::toBullet(ray.getOrigin());
	btVector3 to = Convert::toBullet(ray.getPoint(maxDist));
	mBtWorld->rayTest(from, to, wrapper);
}

DynamicsWorld::~DynamicsWorld()
{
    delete mBtWorld;
}

/*
 * =============================================================================================
 * BtOgre::VertexIndexToShape
 * =============================================================================================
 */

	void VertexIndexToShape::addStaticVertexData(const VertexData *vertex_data)
	{
		if (!vertex_data)
			return;

		const VertexData *data = vertex_data;

		const unsigned int prev_size = mVertexCount;
		mVertexCount += (unsigned int)data->vertexCount;

		Ogre::Vector3* tmp_vert = new Ogre::Vector3[mVertexCount];
		if (mVertexBuffer)
		{
			memcpy(tmp_vert, mVertexBuffer, sizeof(Vector3) * prev_size);
			delete[] mVertexBuffer;
		}
		mVertexBuffer = tmp_vert;

		// Get the positional buffer element
		{
			const Ogre::VertexElement* posElem = data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
			Ogre::HardwareVertexBufferSharedPtr vbuf = data->vertexBufferBinding->getBuffer(posElem->getSource());
			const unsigned int vSize = (unsigned int)vbuf->getVertexSize();

			unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			float* pReal;
			Ogre::Vector3 * curVertices = &mVertexBuffer[prev_size];
			const unsigned int vertexCount = (unsigned int)data->vertexCount;
			for(unsigned int j = 0; j < vertexCount; ++j)
			{
				posElem->baseVertexPointerToElement(vertex, &pReal);
				vertex += vSize;

				curVertices->x = (*pReal++);
				curVertices->y = (*pReal++);
				curVertices->z = (*pReal++);

				*curVertices = mTransform * (*curVertices);

				curVertices++;
			}
			vbuf->unlock();
		}
	}
	//------------------------------------------------------------------------------------------------
	void VertexIndexToShape::addAnimatedVertexData(const Ogre::VertexData *vertex_data,
												   const Ogre::VertexData *blend_data,
												   const Ogre::Mesh::IndexMap *indexMap)
	{
		// Get the bone index element
		assert(vertex_data);

		const VertexData *data = blend_data;
		const unsigned int prev_size = mVertexCount;
		mVertexCount += (unsigned int)data->vertexCount;
		Ogre::Vector3* tmp_vert = new Ogre::Vector3[mVertexCount];
		if (mVertexBuffer)
		{
			memcpy(tmp_vert, mVertexBuffer, sizeof(Vector3) * prev_size);
			delete[] mVertexBuffer;
		}
		mVertexBuffer = tmp_vert;

		// Get the positional buffer element
		{
			const Ogre::VertexElement* posElem = data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
			assert (posElem);
			Ogre::HardwareVertexBufferSharedPtr vbuf = data->vertexBufferBinding->getBuffer(posElem->getSource());
			const unsigned int vSize = (unsigned int)vbuf->getVertexSize();

			unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			float* pReal;
			Ogre::Vector3 * curVertices = &mVertexBuffer[prev_size];
			const unsigned int vertexCount = (unsigned int)data->vertexCount;
			for(unsigned int j = 0; j < vertexCount; ++j)
			{
				posElem->baseVertexPointerToElement(vertex, &pReal);
				vertex += vSize;

				curVertices->x = (*pReal++);
				curVertices->y = (*pReal++);
				curVertices->z = (*pReal++);

				*curVertices = mTransform * (*curVertices);

				curVertices++;
			}
			vbuf->unlock();
		}
		{
			const Ogre::VertexElement* bneElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_BLEND_INDICES);
			assert (bneElem);

			Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(bneElem->getSource());
			const unsigned int vSize = (unsigned int)vbuf->getVertexSize();
			unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

			unsigned char* pBone;

			if (!mBoneIndex)
				mBoneIndex = new BoneIndex();
			BoneIndex::iterator i;

			Ogre::Vector3 * curVertices = &mVertexBuffer[prev_size];

			const unsigned int vertexCount = (unsigned int)vertex_data->vertexCount;
			for(unsigned int j = 0; j < vertexCount; ++j)
			{
				bneElem->baseVertexPointerToElement(vertex, &pBone);
				vertex += vSize;

				const unsigned char currBone = (indexMap) ? (*indexMap)[*pBone] : *pBone;
				i = mBoneIndex->find (currBone);
				Vector3Array* l = 0;
				if (i == mBoneIndex->end())
				{
					l = new Vector3Array;
					mBoneIndex->insert(BoneKeyIndex(currBone, l));
				}
				else
				{
					l = i->second;
				}

				l->push_back(*curVertices);

				curVertices++;
			}
			vbuf->unlock();
		}
	}
	//------------------------------------------------------------------------------------------------
	void VertexIndexToShape::addIndexData(IndexData *data, const unsigned int offset)
	{
		const unsigned int prev_size = mIndexCount;
		mIndexCount += (unsigned int)data->indexCount;

		unsigned int* tmp_ind = new unsigned int[mIndexCount];
		if (mIndexBuffer)
		{
			memcpy (tmp_ind, mIndexBuffer, sizeof(unsigned int) * prev_size);
			delete[] mIndexBuffer;
		}
		mIndexBuffer = tmp_ind;

		const unsigned int numTris = (unsigned int) data->indexCount / 3;
		HardwareIndexBufferSharedPtr ibuf = data->indexBuffer;
		const bool use32bitindexes = (ibuf->getType() == HardwareIndexBuffer::IT_32BIT);
		unsigned int index_offset = prev_size;

		if (use32bitindexes)
		{
			const unsigned int* pInt = static_cast<unsigned int*>(ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
			for(unsigned int k = 0; k < numTris; ++k)
			{
				mIndexBuffer[index_offset ++] = offset + *pInt++;
				mIndexBuffer[index_offset ++] = offset + *pInt++;
				mIndexBuffer[index_offset ++] = offset + *pInt++;
			}
			ibuf->unlock();
		}
		else
		{
			const unsigned short* pShort = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
			for(unsigned int k = 0; k < numTris; ++k)
			{
				mIndexBuffer[index_offset ++] = offset + static_cast<unsigned int> (*pShort++);
				mIndexBuffer[index_offset ++] = offset + static_cast<unsigned int> (*pShort++);
				mIndexBuffer[index_offset ++] = offset + static_cast<unsigned int> (*pShort++);
			}
			ibuf->unlock();
		}

	}
	//------------------------------------------------------------------------------------------------
	Real VertexIndexToShape::getRadius()
	{
		if (mBoundRadius == (-1))
		{
			getSize();
			mBoundRadius = (std::max(mBounds.x,std::max(mBounds.y,mBounds.z)) * 0.5);
		}
		return mBoundRadius;
	}
	//------------------------------------------------------------------------------------------------
	Vector3 VertexIndexToShape::getSize()
	{
		const unsigned int vCount = getVertexCount();
		if (mBounds == Ogre::Vector3(-1,-1,-1) && vCount > 0)
		{

			const Ogre::Vector3 * const v = getVertices();

			Ogre::Vector3 vmin(v[0]);
			Ogre::Vector3 vmax(v[0]);

			for(unsigned int j = 1; j < vCount; j++)
			{
				vmin.x = std::min(vmin.x, v[j].x);
				vmin.y = std::min(vmin.y, v[j].y);
				vmin.z = std::min(vmin.z, v[j].z);

				vmax.x = std::max(vmax.x, v[j].x);
				vmax.y = std::max(vmax.y, v[j].y);
				vmax.z = std::max(vmax.z, v[j].z);
			}

			mBounds.x = vmax.x - vmin.x;
			mBounds.y = vmax.y - vmin.y;
			mBounds.z = vmax.z - vmin.z;
		}

		return mBounds;
	}
	//------------------------------------------------------------------------------------------------
	const Ogre::Vector3* VertexIndexToShape::getVertices()
	{
		return mVertexBuffer;
	}
	//------------------------------------------------------------------------------------------------
	unsigned int VertexIndexToShape::getVertexCount()
	{
		return mVertexCount;
	}
	//------------------------------------------------------------------------------------------------
	const unsigned int* VertexIndexToShape::getIndices()
	{
		return mIndexBuffer;
	}
	//------------------------------------------------------------------------------------------------
	unsigned int VertexIndexToShape::getIndexCount()
	{
		return mIndexCount;
	}

	//------------------------------------------------------------------------------------------------
	btConvexHullShape* VertexIndexToShape::createConvex()
	{
		assert(mVertexCount && (mIndexCount >= 6) &&
			("Mesh must have some vertices and at least 6 indices (2 triangles)"));

		btConvexHullShape* shape = new btConvexHullShape((btScalar*) &mVertexBuffer[0].x, mVertexCount, sizeof(Vector3));

		shape->setLocalScaling(Convert::toBullet(mScale));

		return shape;
	}
	//------------------------------------------------------------------------------------------------
	btBvhTriangleMeshShape* VertexIndexToShape::createTrimesh()
	{
		assert(mVertexCount && (mIndexCount >= 6) &&
			("Mesh must have some vertices and at least 6 indices (2 triangles)"));

		unsigned int numFaces = mIndexCount / 3;

		btTriangleMesh *trimesh = new btTriangleMesh();
		unsigned int *indices = mIndexBuffer;
		Vector3 *vertices = mVertexBuffer;

		btVector3    vertexPos[3];
		for (unsigned int n = 0; n < numFaces; ++n)
		{
			{
				const Vector3 &vec = vertices[*indices];
				vertexPos[0][0] = vec.x;
				vertexPos[0][1] = vec.y;
				vertexPos[0][2] = vec.z;
			}
			{
				const Vector3 &vec = vertices[*(indices + 1)];
				vertexPos[1][0] = vec.x;
				vertexPos[1][1] = vec.y;
				vertexPos[1][2] = vec.z;
			}
			{
				const Vector3 &vec = vertices[*(indices + 2)];
				vertexPos[2][0] = vec.x;
				vertexPos[2][1] = vec.y;
				vertexPos[2][2] = vec.z;
			}

			indices += 3;

			trimesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2]);
		}

		const bool useQuantizedAABB = true;
		btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(trimesh, useQuantizedAABB);

	        shape->setLocalScaling(Convert::toBullet(mScale));

		return shape;
	}
	//------------------------------------------------------------------------------------------------
	VertexIndexToShape::~VertexIndexToShape()
	{
		delete[] mVertexBuffer;
		delete[] mIndexBuffer;

		if (mBoneIndex)
		{
			for(BoneIndex::iterator i = mBoneIndex->begin();
				i != mBoneIndex->end();
				++i)
			{
				delete i->second;
			}
			delete mBoneIndex;
		}
	}
	//------------------------------------------------------------------------------------------------
	VertexIndexToShape::VertexIndexToShape(const Affine3 &transform) :
		mVertexBuffer (0),
		mIndexBuffer (0),
		mVertexCount (0),
		mIndexCount (0),
		mBounds (Vector3(-1,-1,-1)),
		mBoundRadius (-1),
		mBoneIndex (0),
		mTransform (transform),
		mScale(1)
	{
	}
	//------------------------------------------------------------------------------------------------
	VertexIndexToShape::VertexIndexToShape(const Entity *entity,  const Affine3 &transform) :
		VertexIndexToShape(transform)
	{
		addEntity(entity, transform);
	}
	//------------------------------------------------------------------------------------------------
	VertexIndexToShape::VertexIndexToShape(Renderable *rend, const Affine3 &transform) :
		VertexIndexToShape(transform)
	{
		RenderOperation op;
		rend->getRenderOperation(op);
		addStaticVertexData(op.vertexData);
		if(op.useIndexes)
			addIndexData(op.indexData);
	}
	//------------------------------------------------------------------------------------------------
	void VertexIndexToShape::addEntity(const Entity *entity,const Affine3 &transform)
	{
		// Each entity added need to reset size and radius
		// next time getRadius and getSize are asked, they're computed.
		mBounds  = Ogre::Vector3(-1,-1,-1);
		mBoundRadius = -1;

		auto node = entity->getParentSceneNode();
		mTransform = transform;
		mScale = node ? node->getScale() : Ogre::Vector3(1,1,1);

		bool hasSkeleton = entity->hasSkeleton();

		if (entity->getMesh()->sharedVertexData)
		{
			if (hasSkeleton)
				addAnimatedVertexData(entity->getMesh()->sharedVertexData, entity->_getSkelAnimVertexData(),
										&entity->getMesh()->sharedBlendIndexToBoneIndexMap);
			else
				addStaticVertexData(entity->getMesh()->sharedVertexData);
		}

		for (unsigned int i = 0;i < entity->getNumSubEntities();++i)
		{
			SubMesh *sub_mesh = entity->getSubEntity(i)->getSubMesh();

			if (!sub_mesh->useSharedVertices)
			{
				addIndexData(sub_mesh->indexData, mVertexCount);

				if (hasSkeleton)
					addAnimatedVertexData(sub_mesh->vertexData,
											entity->getSubEntity(i)->_getSkelAnimVertexData(),
											&sub_mesh->blendIndexToBoneIndexMap);
				else
					addStaticVertexData(sub_mesh->vertexData);
			}
			else
			{
				addIndexData (sub_mesh->indexData);
			}

		}
	}
	//------------------------------------------------------------------------------------------------
	void VertexIndexToShape::addMesh(const MeshPtr &mesh, const Affine3 &transform)
	{
		// Each entity added need to reset size and radius
		// next time getRadius and getSize are asked, they're computed.
		mBounds  = Ogre::Vector3(-1,-1,-1);
		mBoundRadius = -1;

		mTransform = transform;

		if (mesh->hasSkeleton ())
			Ogre::LogManager::getSingleton().logWarning("Mesh " + mesh->getName () + " has a skeleton but added non animated");

		if (mesh->sharedVertexData)
		{
			VertexIndexToShape::addStaticVertexData (mesh->sharedVertexData);
		}

		for(unsigned int i = 0;i < mesh->getNumSubMeshes();++i)
		{
			SubMesh *sub_mesh = mesh->getSubMesh(i);

			if (!sub_mesh->useSharedVertices)
			{
				VertexIndexToShape::addIndexData(sub_mesh->indexData, mVertexCount);
				VertexIndexToShape::addStaticVertexData (sub_mesh->vertexData);
			}
			else
			{
				VertexIndexToShape::addIndexData (sub_mesh->indexData);
			}

		}
	}

/*
 * =============================================================================================
 * BtOgre::DebugDrawer
 * =============================================================================================
 */
	//------------------------------------------------------------------------------------------------
	void DebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
        if (mLines.getSections().empty())
        {
            const char* matName = "Ogre/Debug/LinesMat";
            auto mat = Ogre::MaterialManager::getSingleton().getByName(matName, Ogre::RGN_INTERNAL);
            if (!mat)
            {
                mat = Ogre::MaterialManager::getSingleton().create(matName, Ogre::RGN_INTERNAL);
                auto p = mat->getTechnique(0)->getPass(0);
                p->setLightingEnabled(false);
                p->setVertexColourTracking(Ogre::TVC_AMBIENT);
            }
            mLines.setBufferUsage(Ogre::HBU_CPU_TO_GPU);
            mLines.begin(mat, Ogre::RenderOperation::OT_LINE_LIST);
        }
        else if (mLines.getCurrentVertexCount() == 0)
            mLines.beginUpdate(0);
		
		Ogre::ColourValue col(color.x(), color.x(), color.z());
		mLines.position(Convert::toOgre(from));
        mLines.colour(col);
        mLines.position(Convert::toOgre(to));
        mLines.colour(col);
	}
}
