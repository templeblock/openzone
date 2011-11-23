/*
 * OpenZone - Simple Cross-Platform FPS/RTS Game Engine
 * Copyright (C) 2002-2011  Davorin Učakar
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Davorin Učakar
 * <davorin.ucakar@gmail.com>
 */

/**
 * @file client/Sound.cpp
 */

#include "stable.hpp"

#include "client/Sound.hpp"

#include "client/Camera.hpp"
#include "client/OpenAL.hpp"

namespace oz
{
namespace client
{

Sound sound;

const float Sound::MAX_DISTANCE = 160.0f;

void Sound::playCell( int cellX, int cellY )
{
  const Cell& cell = orbis.cells[cellX][cellY];

  for( int i = 0; i < cell.structs.length(); ++i ) {
    int strIndex = cell.structs[i];

    if( !playedStructs.get( strIndex ) ) {
      playedStructs.set( strIndex );

      const Struct* str = orbis.structs[strIndex];
      context.playBSP( str );
    }
  }

  foreach( obj, cell.objects.citer() ) {
    if( obj->flags & Object::AUDIO_BIT ) {
      if( ( camera.p - obj->p ).sqL() < MAX_DISTANCE*MAX_DISTANCE ) {
        context.playAudio( obj, null );
      }
    }
  }
}

bool Sound::loadMusicBuffer( uint buffer )
{
  int bytesRead = 0;
  int result;
  int section;

  do {
    result = int( ov_read( &oggStream, &musicBuffer[bytesRead], MUSIC_BUFFER_SIZE - bytesRead,
                           false, 2, true, &section ) );
    bytesRead += result;
    if( result <= 0 ) {
      return false;
    }
  }
  while( result > 0 && bytesRead < MUSIC_BUFFER_SIZE );

  alBufferData( buffer, musicFormat, musicBuffer, bytesRead, ALsizei( vorbisInfo->rate ) );

  return true;
}

void Sound::setVolume( float volume_ )
{
  volume = volume_;
  alListenerf( AL_GAIN, volume_ );
}

void Sound::setMusicVolume( float volume ) const
{
  alSourcef( musicSource, AL_GAIN, volume );
}

void Sound::playMusic( int track )
{
  hard_assert( track >= 0 );

  selectedTrack = track;
  currentTrack  = track;
}

void Sound::stopMusic()
{
  selectedTrack = -2;
  currentTrack  = -1;
}

bool Sound::isMusicPlaying() const
{
  return currentTrack != -1;
}

void Sound::resume() const
{
  alcProcessContext( soundContext );
  alListenerf( AL_GAIN, volume );
}

void Sound::suspend() const
{
  alListenerf( AL_GAIN, 0.0f );
  alcSuspendContext( soundContext );
}

void Sound::play()
{
  float orientation[] = { camera.at.x, camera.at.y, camera.at.y,
                          camera.up.x, camera.up.y, camera.up.z };
  // add new sounds
  alListenerfv( AL_ORIENTATION, orientation );
  alListenerfv( AL_POSITION, camera.p );

  if( playedStructs.length() < orbis.structs.length() ) {
    playedStructs.dealloc();
    playedStructs.alloc( orbis.structs.length() );
  }
  playedStructs.clearAll();

  Span span = orbis.getInters( camera.p, MAX_DISTANCE + AABB::MAX_DIM );

  for( int x = span.minX ; x <= span.maxX; ++x ) {
    for( int y = span.minY; y <= span.maxY; ++y ) {
      playCell( x, y );
    }
  }

  OZ_AL_CHECK_ERROR();
}

void Sound::update()
{
  OZ_AL_CHECK_ERROR();

  if( selectedTrack != -1 ) {
    alSourceStop( musicSource );

    int nQueued;
    alGetSourcei( musicSource, AL_BUFFERS_QUEUED, &nQueued );

    uint buffer[2];
    alSourceUnqueueBuffers( musicSource, nQueued, buffer );

    ov_clear( &oggStream );

    OZ_AL_CHECK_ERROR();

    currentTrack = -1;

    if( selectedTrack == -2 ) {
      selectedTrack = -1;
    }
    else {
      const char* path = library.musics[selectedTrack].path;

      log.print( "Loading music '%s' ...", path );

      if( ov_fopen( path, &oggStream ) < 0 ) {
        throw Exception( "Failed to open Ogg stream" );
      }

      vorbisInfo = ov_info( &oggStream, -1 );
      if( vorbisInfo == null ) {
        ov_clear( &oggStream );
        throw Exception( "Failed to read Vorbis header" );
      }

      if( vorbisInfo->channels == 1 ) {
        musicFormat = AL_FORMAT_MONO16;
      }
      else if( vorbisInfo->channels == 2 ) {
        musicFormat = AL_FORMAT_STEREO16;
      }
      else {
        ov_clear( &oggStream );
        throw Exception( "Invalid number of channels, should be 1 or 2" );
      }

      loadMusicBuffer( musicBuffers[0] );
      loadMusicBuffer( musicBuffers[1] );

      alSourceQueueBuffers( musicSource, 2, &musicBuffers[0] );
      alSourcePlay( musicSource );

      currentTrack  = selectedTrack;
      selectedTrack = -1;

      OZ_AL_CHECK_ERROR();

      log.printEnd( " OK" );
    }
  }
  else if( currentTrack != -1 ) {
    bool hasLoaded = false;

    int nProcessed;
    alGetSourcei( musicSource, AL_BUFFERS_PROCESSED, &nProcessed );

    for( int i = 0; i < nProcessed; ++i ) {
      uint buffer;
      alSourceUnqueueBuffers( musicSource, 1, &buffer );

      if( loadMusicBuffer( buffer ) ) {
        alSourceQueueBuffers( musicSource, 1, &buffer );
        hasLoaded = true;
      }
    }

    ALint value;
    alGetSourcei( musicSource, AL_SOURCE_STATE, &value );

    if( value == AL_STOPPED ) {
      if( hasLoaded ) {
        alSourcePlay( musicSource );
      }
      else {
        currentTrack = -1;

        ov_clear( &oggStream );
      }
    }
  }
}

void Sound::init()
{
  log.println( "Initialising Sound {" );
  log.indent();

  musicBuffers[0] = 0;
  musicBuffers[1] = 0;

  const char* deviceName = config.getSet( "sound.device", "" );

  log.print( "Initialising device '%s' ...", deviceName );

  soundDevice = alcOpenDevice( deviceName );
  if( soundDevice == null ) {
    throw Exception( "Failed to open OpenAL device" );
  }

  int defaultAttributes[] = {
    ALC_SYNC, AL_FALSE,
    ALC_MONO_SOURCES, 255,
    ALC_STEREO_SOURCES, 1,
    0
  };

  soundContext = alcCreateContext( soundDevice, defaultAttributes );
  if( soundContext == null ) {
    throw Exception( "Failed to create OpenAL context" );
  }

  if( alcMakeContextCurrent( soundContext ) != ALC_TRUE ) {
    throw Exception( "Failed to select OpenAL context" );
  }

  log.printEnd( " OK" );

  OZ_AL_CHECK_ERROR();

  log.println( "OpenAL device: %s", alcGetString( soundDevice, ALC_DEVICE_SPECIFIER ) );

  int nAttributes;
  alcGetIntegerv( soundDevice, ALC_ATTRIBUTES_SIZE, 1, &nAttributes );

  int* attributes = new int[nAttributes];
  alcGetIntegerv( soundDevice, ALC_ALL_ATTRIBUTES, nAttributes, attributes );

  log.println( "OpenAL attributes {" );
  log.indent();

  for( int i = 0; i < nAttributes; i += 2 ) {
    switch( attributes[i] ) {
      case ALC_FREQUENCY: {
        log.println( "ALC_FREQUENCY: %d Hz", attributes[i + 1] );
        break;
      }
      case ALC_REFRESH: {
        log.println( "ALC_REFRESH: %d Hz", attributes[i + 1] );
        break;
      }
      case ALC_SYNC: {
        log.println( "ALC_SYNC: %s", attributes[i + 1] != 0 ? "on" : "off" );
        break;
      }
      case ALC_MONO_SOURCES: {
        log.println( "ALC_MONO_SOURCES: %d", attributes[i + 1] );
        break;
      }
      case ALC_STEREO_SOURCES: {
        log.println( "ALC_STEREO_SOURCES: %d", attributes[i + 1] );
        break;
      }
    }
  }

  delete[] attributes;

  log.unindent();
  log.println( "}" );

  log.println( "OpenAL vendor: %s", alGetString( AL_VENDOR ) );
  log.println( "OpenAL renderer: %s", alGetString( AL_RENDERER ) );
  log.println( "OpenAL version: %s", alGetString( AL_VERSION ) );

  if( log.isVerbose ) {
    String sExtensions = alGetString( AL_EXTENSIONS );
    DArray<String> extensions = sExtensions.trim().split( ' ' );

    log.println( "OpenAL extensions {" );
    log.indent();

    foreach( extension, extensions.citer() ) {
      log.println( "%s", extension->cstr() );
    }

    log.unindent();
    log.println( "}" );
  }

  selectedTrack = -1;
  currentTrack  = -1;

  alGenBuffers( 2, musicBuffers );
  alGenSources( 1, &musicSource );

  alSourcei( musicSource, AL_SOURCE_RELATIVE, AL_TRUE );

  setVolume( config.getSet( "sound.volume", 1.0f ) );
  setMusicVolume( 0.5f );

  log.unindent();
  log.println( "}" );

  OZ_AL_CHECK_ERROR();
}

void Sound::free()
{
  log.print( "Shutting down Sound ..." );

  if( soundContext != null ) {
    stopMusic();

    playedStructs.dealloc();

    alDeleteSources( 1, &musicSource );
    alDeleteBuffers( 2, musicBuffers );

    OZ_AL_CHECK_ERROR();

    alcDestroyContext( soundContext );
    soundContext = null;

    alcCloseDevice( soundDevice );
    soundDevice = null;
  }

  log.printEnd( " OK" );
}

}
}
