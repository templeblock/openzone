/*
 *  BotClass.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#include "precompiled.h"

#include "BotClass.h"

#include "Bot.h"
#include "Translator.h"

namespace oz
{

  ObjectClass *BotClass::init( const String &name, Config *config )
  {
    BotClass *clazz = new BotClass();

    clazz->name           = name;
    clazz->description    = config->get( "description", "" );

    clazz->dim.x          = config->get( "dim.x", 0.5f );
    clazz->dim.y          = config->get( "dim.y", 0.5f );
    clazz->dim.z          = config->get( "dim.z", 0.5f );

    clazz->flags          = config->get( "flags", DEFAULT_FLAGS ) | BASE_FLAGS;
    clazz->damage         = config->get( "damage", 1.0f );

    clazz->mass           = config->get( "mass", 100.0f );
    clazz->lift           = config->get( "lift", 0.10f );

    clazz->modelType      = config->get( "model.type", "MD2" );
    clazz->modelPath      = config->get( "model.path", "mdl/goblin.md2" );

    clazz->audioType      = config->get( "audio.type", "Simple" );

    char buffer[] = "audio.sample  ";
    for( int i = 0; i < AUDIO_SAMPLES; i++ ) {
      assert( 0 <= i && i < 100 );

      buffer[ sizeof( buffer ) - 2 ] = i / 10;
      buffer[ sizeof( buffer ) - 1 ] = i % 10;

      clazz->audioSamples[i] = translator.soundIndex( config->get( buffer, "" ) );
    }

    clazz->dimCrouch.x    = config->get( "dimCrouch.x", 0.5f );
    clazz->dimCrouch.y    = config->get( "dimCrouch.y", 0.5f );
    clazz->dimCrouch.z    = config->get( "dimCrouch.z", 0.5f );

    clazz->camPos.x       = config->get( "camPos.x", 0.0f );
    clazz->camPos.y       = config->get( "camPos.y", 0.0f );
    clazz->camPos.z       = config->get( "camPos.z", 0.0f );

    clazz->camPosCrouch.x = config->get( "camPosCrouch.x", 0.0f );
    clazz->camPosCrouch.y = config->get( "camPosCrouch.y", 0.0f );
    clazz->camPosCrouch.z = config->get( "camPosCrouch.z", 0.0f );

    clazz->bobInc         = config->get( "bobInc", 0.0f );
    clazz->bobAmplitude   = config->get( "bobAmplitude", 0.0f );

    clazz->walkVelocity   = config->get( "walkVelocity", 1.0f );
    clazz->runVelocity    = config->get( "runVelocity", 4.0f );
    clazz->crouchVelocity = config->get( "crouchVelocity", 1.0f );
    clazz->jumpVelocity   = config->get( "jumpVelocity", 4.0f );

    clazz->stepInc        = config->get( "stepInc", 1.0f );
    clazz->stepMax        = config->get( "stepMax", 1.0f );

    clazz->airControl     = config->get( "airControl", 0.05f );
    clazz->grabDistance   = config->get( "grabDistance", 1.0f );
    clazz->state          = config->get( "state", 0 );

    return clazz;
  }

  Object *BotClass::create( const Vec3 &pos )
  {
    Bot *obj = new Bot();

    obj->p = pos;
    obj->dim = dim;

    obj->flags = flags;
    obj->oldFlags = flags;
    obj->type = this;
    obj->damage = damage;

    obj->mass = mass;
    obj->lift = lift;

    obj->camPos = camPos;
    obj->state = state | Bot::STEPPING_BIT;
    obj->mind = null;

    return obj;
  }

}
