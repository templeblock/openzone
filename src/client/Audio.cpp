/*
 *  Audio.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "client/Audio.hpp"

#include "client/Camera.hpp"
#include "client/Context.hpp"
#include "client/Sound.hpp"

#include <AL/alut.h>

namespace oz
{
namespace client
{

  const float Audio::REFERENCE_DISTANCE = 4.0f;

  void Audio::playSound( int sample, float volume, const Object* obj ) const
  {
    assert( 0 <= sample && sample < translator.sounds.length() );
    assert( alGetError() == AL_NO_ERROR );

    uint srcId;

    alGenSources( 1, &srcId );
    if( alGetError() != AL_NO_ERROR ) {
      log.println( "AL: Too many sources" );
      return;
    }

    alSourcei( srcId, AL_BUFFER, context.sounds[sample].id );
    alSourcef( srcId, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE );

    // If the object moves since source starts playing and source stands still, it's usually
    // not noticeable for short-time source. After all, sound source many times does't move
    // together with the object in many cases (e.g. the sound when an objects hits something).
    //
    // However, when the sound is generated by player (e.g. cries, talk) it is often annoying
    // if the sound source doesn't move with the player. That's why we position the sounds
    // generated by the player at the origin of the coordinate system relative to player.
    if( obj->index == camera.bot ) {
      alSourcei( srcId, AL_SOURCE_RELATIVE, AL_TRUE );
      alSourcefv( srcId, AL_POSITION, Vec3::zero() );
    }
    else {
      alSourcefv( srcId, AL_POSITION, obj->p );
    }
    alSourcef( srcId, AL_GAIN, volume );
    alSourcePlay( srcId );

    assert( alGetError() == AL_NO_ERROR );

    sound.addSource( srcId );
  }

  void Audio::playContSound( int sample, float volume, uint key, const Object* obj ) const
  {
    assert( 0 <= sample && sample < translator.sounds.length() );
    assert( alGetError() == AL_NO_ERROR );

    if( sound.updateContSource( key ) ) {
      alSourcef( sound.getCachedContSourceId(), AL_GAIN, volume );
      alSourcefv( sound.getCachedContSourceId(), AL_POSITION, obj->p );
    }
    else {
      uint srcId;

      alGenSources( 1, &srcId );
      if( alGetError() != AL_NO_ERROR ) {
        log.println( "AL: Too many sources" );
        return;
      }

      alSourcei( srcId, AL_BUFFER, context.sounds[sample].id );
      alSourcei( srcId, AL_LOOPING, AL_TRUE );
      alSourcef( srcId, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE );
      alSourcefv( srcId, AL_POSITION, obj->p );
      alSourcef( srcId, AL_GAIN, volume );
      alSourcePlay( srcId );

      sound.addContSource( key, srcId );
    }

    assert( alGetError() == AL_NO_ERROR );
  }

  Audio::Audio( const Object* obj_ ) : obj( obj_ ), flags( 0 )
  {
    const int* samples = obj->type->audioSamples;

    for( int i = 0; i < ObjectClass::AUDIO_SAMPLES; ++i ) {
      if( samples[i] != -1 ) {
        context.requestSound( samples[i] );
      }
    }
    assert( alGetError() == AL_NO_ERROR );
  }

  Audio::~Audio()
  {
    const int* samples = obj->type->audioSamples;

    for( int i = 0; i < ObjectClass::AUDIO_SAMPLES; ++i ) {
      if( samples[i] != -1 ) {
        context.releaseSound( samples[i] );
      }
    }
    assert( alGetError() == AL_NO_ERROR );
  }

}
}
