/*
 *  DynObjectClass.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *
 *  $Id$
 */

#include "precompiled.hpp"

#include "DynObjectClass.hpp"

namespace oz
{

  Class *DynObjectClass::init( Config *config_ )
  {
    Config &config = *config_;
    DynObjectClass *clazz = new DynObjectClass();

    OZ_CLASS_READ_FLOAT( clazz, dim.x, 0.5f );
    OZ_CLASS_READ_FLOAT( clazz, dim.y, 0.5f );
    OZ_CLASS_READ_FLOAT( clazz, dim.z, 0.5f );

    OZ_CLASS_READ_INT( clazz, flags, Object::DYNAMIC_BIT | Object::CLIP_BIT );
    OZ_CLASS_READ_INT( clazz, type, 0 );
    OZ_CLASS_READ_FLOAT( clazz, damage, 1.0f );

    OZ_CLASS_READ_FLOAT( clazz, mass, 10.0f );
    OZ_CLASS_READ_FLOAT( clazz, lift, 0.03f );

    OZ_CLASS_READ_STRING( clazz, model, "mdl/goblin.md2" );

    return clazz;
  }

  Object *DynObjectClass::create( const Vec3 &pos )
  {
    DynObject *obj = new DynObject();

    obj->p = pos;
    obj->dim = dim;

    obj->flags = flags;
    obj->type = type;
    obj->damage = damage;

    obj->mass = mass;
    obj->lift = lift;

    return obj;
  }

}
