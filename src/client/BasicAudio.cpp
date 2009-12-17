/*
 *  BasicAudio.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#include "precompiled.h"

#include "BasicAudio.h"

#include "matrix/Physics.h"
#include "Camera.h"
#include "Sound.h"

namespace oz
{
namespace client
{

  Audio *BasicAudio::create( const Object *obj )
  {
    return new BasicAudio( obj );
  }

  void BasicAudio::update()
  {
    const Dynamic *dyn = static_cast<const Dynamic*>( obj );
    int ( &samples )[ObjectClass::AUDIO_SAMPLES] = obj->type->audioSamples;

    // friction
    if( ( obj->flags & ( Object::DYNAMIC_BIT | Object::FRICTING_BIT | Object::ON_SLICK_BIT ) ) ==
        ( Object::DYNAMIC_BIT | Object::FRICTING_BIT ) &&
        samples[SND_FRICTING] != -1 && dyn->depth == 0.0f )
    {
      float dv = Math::sqrt( dyn->velocity.x*dyn->velocity.x +
                             dyn->velocity.y*dyn->velocity.y );
      playContSound( samples[SND_FRICTING], dv, reinterpret_cast<uint>( &*dyn ) );
    }

    // events
    foreach( event, obj->events.iterator() ) {
      assert( event->id < ObjectClass::AUDIO_SAMPLES );

      if( event->id >= 0 && samples[event->id] != -1 ) {
        assert( 0.0f <= event->intensity );

        playSound( samples[event->id], event->intensity );
      }
    }
  }

}
}
