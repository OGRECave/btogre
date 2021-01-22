
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