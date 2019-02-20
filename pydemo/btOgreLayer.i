%module(directors="1") pyBtOgreLayer
%{
#include "Ogre.h"
#include "OgreEntity.h"
#include "OgreSceneNode.h"
#include "BtOgrePG.h"
#include "BtOgreGP.h"
#include "BtOgreExtras.h"
#include "btOgreLayer.h"
%}

%include std_string.i
%include exception.i
%include stdint.i
%include typemaps.i
%import "Ogre.i"

%include "btOgreLayer.h"
%ignore Ogre::btOgreLayer::phyWorld;
%ignore Ogre::btOgreLayer::dbgdraw;
%ignore Ogre::btOgreLayer::mBroadphase;
%ignore Ogre::btOgreLayer::mCollisionConfig;
%ignore Ogre::btOgreLayer::mDispatcher;
%ignore Ogre::btOgreLayer::mSolver;
%ignore Ogre::btOgreLayer::mGroundShape;
%ignore Ogre::btOgreLayer::mShaps;
%ignore Ogre::btOgreLayer::mRigidbodys;




#ifdef SWIGPYTHON
%pythoncode
%{
    __version__ = "0.0"
%}
#endif



