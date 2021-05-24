
%module(directors="1") BtOgre
%{
#include "Ogre.h"
#include "BtOgre.h"
%}

%include std_string.i
%include exception.i
%include stdint.i
%include typemaps.i
%import "Ogre.i"

%feature("director") BtOgre::CollisionListener;

// avoid wrapping BtWorld for now..
%extend BtOgre::DynamicsWorld
{
  int stepSimulation(float timeStep)
  {
    return $self->getBtWorld()->stepSimulation(timeStep);
  }
}

%include "BtOgre.h"

// bullet subset
#define SIMD_FORCE_INLINE
#define ATTRIBUTE_ALIGNED16(a) a
typedef float btScalar;
%include "LinearMath/btVector3.h"
%include "BulletCollision/NarrowPhaseCollision/btManifoldPoint.h"
%include "BulletCollision/CollisionDispatch/btCollisionObject.h"
%include "BulletDynamics/Dynamics/btRigidBody.h"