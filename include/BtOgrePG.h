/*
 * =====================================================================================
 *
 *       Filename:  BtOgrePG.h
 *
 *    Description:  The part of BtOgre that handles information transfer from Bullet to
 *                  Ogre (like updating graphics object positions).
 *
 *        Version:  1.0
 *        Created:  27/12/2008 03:40:56 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#ifndef _BtOgreGP_H_
#define _BtOgreGP_H_

#include "btBulletDynamicsCommon.h"
#include "OgreSceneNode.h"
#include "BtOgreExtras.h"

namespace BtOgre {

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

//Softbody-Ogre connection goes here!

}

#endif
