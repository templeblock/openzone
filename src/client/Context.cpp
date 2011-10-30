/*
 *  Context.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011  Davorin Učakar
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#include "stable.hpp"

#include "client/Context.hpp"

#include "client/MD2.hpp"
#include "client/MD3.hpp"

#include "client/SMMImago.hpp"
#include "client/SMMVehicleImago.hpp"
#include "client/ExplosionImago.hpp"
#include "client/MD2Imago.hpp"
#include "client/MD2WeaponImago.hpp"
#include "client/MD3Imago.hpp"

#include "client/BasicAudio.hpp"
#include "client/BotAudio.hpp"
#include "client/VehicleAudio.hpp"

#include "client/OpenGL.hpp"
#include "client/OpenAL.hpp"

#define OZ_REGISTER_IMAGOCLASS( name ) \
  imagoClasses.add( #name, &name##Imago::create )

#define OZ_REGISTER_AUDIOCLASS( name ) \
  audioClasses.add( #name, &name##Audio::create )

namespace oz
{
namespace client
{

Context context;

Pool<Context::Source> Context::Source::pool;
Buffer Context::buffer;

void Context::addSource( uint srcId, int sample )
{
  hard_assert( sounds[sample].nUsers > 0 );

  ++sounds[sample].nUsers;
  sources.add( new Source( srcId, sample ) );
}

void Context::addBSPSource( uint srcId, int sample, int key )
{
  hard_assert( sounds[sample].nUsers > 0 );

  ++sounds[sample].nUsers;
  bspSources.add( key, ContSource( srcId, sample ) );
}

void Context::addObjSource( uint srcId, int sample, int key )
{
  hard_assert( sounds[sample].nUsers > 0 );

  ++sounds[sample].nUsers;
  objSources.add( key, ContSource( srcId, sample ) );
}

void Context::removeSource( Source* source, Source* prev )
{
  int sample = source->sample;

  hard_assert( sounds[sample].nUsers > 0 );

  sources.remove( source, prev );
  delete source;
  releaseSound( sample );
}

void Context::removeBSPSource( ContSource* contSource, int key )
{
  int sample = contSource->sample;

  hard_assert( sounds[sample].nUsers > 0 );

  bspSources.exclude( key );
  releaseSound( sample );
}

void Context::removeObjSource( ContSource* contSource, int key )
{
  int sample = contSource->sample;

  hard_assert( sounds[sample].nUsers > 0 );

  objSources.exclude( key );
  releaseSound( sample );
}

Context::Context() : textures( null ), sounds( null ), bsps( null )
{}

uint Context::loadTexture( const char* path )
{
  log.print( "Loading texture '%s' ...", path );

  if( !buffer.read( path ) ) {
    log.printEnd( " No such file" );
    throw Exception( "Texture loading failed" );
  }

  InputStream is = buffer.inputStream();
  uint id = readTexture( &is );

  log.printEnd( " OK" );
  return id;
}

uint Context::readTexture( InputStream* stream )
{
  OZ_GL_CHECK_ERROR();

  uint texId;
  glGenTextures( 1, &texId );
  glBindTexture( GL_TEXTURE_2D, texId );

  int wrap           = stream->readInt();
  int magFilter      = stream->readInt();
  int minFilter      = stream->readInt();
  int nMipmaps       = stream->readInt();
  int internalFormat = stream->readInt();

  bool usesS3TC = internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
      internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

  if( !shader.hasS3TC && usesS3TC ) {
    throw Exception( "Texture uses S3 texture compression but texture compression disabled" );
  }

  if( !wrap ) {
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
  }

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter );

  for( int i = 0; i < nMipmaps; ++i ) {
    int width = stream->readInt();
    int height = stream->readInt();
    int size = stream->readInt();

    if( usesS3TC ) {
      glCompressedTexImage2D( GL_TEXTURE_2D, i, uint( internalFormat ), width, height, 0,
                              size, stream->prepareRead( size ) );
    }
    else {
      glTexImage2D( GL_TEXTURE_2D, i, internalFormat, width, height, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, stream->prepareRead( size ) );
    }
  }

  if( glGetError() != GL_NO_ERROR || !glIsTexture( texId ) ) {
    glDeleteTextures( 1, &texId );

    throw Exception( "Texture loading failed" );
  }

  return texId;
}

uint Context::requestTexture( int id )
{
  Resource<uint>& resource = textures[id];

  if( resource.nUsers != 0 ) {
    ++resource.nUsers;
    return resource.id;
  }

  resource.nUsers = 1;

  const String& name = library.textures[id].name;

  log.print( "Loading texture '%s' ...", name.cstr() );

  resource.id = GL_NONE;

  if( buffer.read( "bsp/" + name + ".ozcTex" ) ) {
    InputStream is = buffer.inputStream();

    resource.id = readTexture( &is );
  }

  if( resource.id == 0 ) {
    log.printEnd( " Failed" );
    throw Exception( "Texture loading failed" );
  }

  log.printEnd( " OK" );
  return resource.id;
}

void Context::releaseTexture( int id )
{
  Resource<uint>& resource = textures[id];

  hard_assert( uint( id ) < uint( library.textures.length() ) && resource.nUsers > 0 );

  --resource.nUsers;

  if( resource.nUsers == 0 ) {
    log.print( "Unloading texture '%s' ...", library.textures[id].name.cstr() );
    glDeleteTextures( 1, &resource.id );

    OZ_GL_CHECK_ERROR();

    log.printEnd( " OK" );
  }
}

uint Context::requestSound( int id )
{
  Resource<uint>& resource = sounds[id];

  if( resource.nUsers != 0 ) {
    ++resource.nUsers;
    return resource.id;
  }

  resource.nUsers = 1;

  OZ_AL_CHECK_ERROR();

  const String& name = library.sounds[id].name;
  const String& path = library.sounds[id].path;

  log.print( "Loading sound '%s' ...", name.cstr() );

  uint   length;
  ubyte* data;

  SDL_AudioSpec audioSpec;

  audioSpec.freq     = DEFAULT_AUDIO_FREQ;
  audioSpec.format   = DEFAULT_AUDIO_FORMAT;
  audioSpec.channels = 1;
  audioSpec.samples  = 0;

  if( SDL_LoadWAV( path, &audioSpec, &data, &length ) == null ) {
    throw Exception( "Failed to load sound" );
  }

  if( audioSpec.channels != 1 ||
      ( audioSpec.format != AUDIO_U8 && audioSpec.format != AUDIO_S16 ) )
  {
    log.printEnd( " Failed, format should be mono U8 mono or S16LE mono" );
      throw Exception( "Invalid sound format, should be U8 mono or S16LE mono" );
  }

  ALenum format = audioSpec.format == AUDIO_U8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;

  alGenBuffers( 1, &resource.id );
  alBufferData( resource.id, format, data, int( length ), audioSpec.freq );

  SDL_FreeWAV( data );

  OZ_AL_CHECK_ERROR();

  if( resource.id == 0 ) {
    log.printEnd( " Failed" );
    throw Exception( "Sound loading failed" );
  }

  log.printEnd( " %s %d Hz ... OK", format == AL_FORMAT_MONO8 ? "U8" : "S16LE", audioSpec.freq );
  return resource.id;
}

void Context::releaseSound( int id )
{
  Resource<uint>& resource = sounds[id];

  hard_assert( uint( id ) < uint( library.sounds.length() ) && resource.nUsers > 0 );

  --resource.nUsers;

  if( resource.nUsers == 0 ) {
    log.print( "Unloading sound '%s' ...", library.sounds[id].name.cstr() );
    alDeleteBuffers( 1, &resource.id );

    OZ_AL_CHECK_ERROR();

    log.printEnd( " OK" );
  }
}

SMM* Context::requestSMM( int id )
{
  Resource<SMM*>& resource = smms[id];

  if( resource.object == null ) {
    resource.object = new SMM( id );

    hard_assert( resource.nUsers == 0 );
  }

  ++resource.nUsers;
  return resource.object;
}

void Context::releaseSMM( int id )
{
  Resource<SMM*>& resource = smms[id];

  hard_assert( resource.object != null && resource.nUsers > 0 );

  --resource.nUsers;
}

MD2* Context::requestMD2( int id )
{
  Resource<MD2*>& resource = md2s[id];

  if( resource.object == null ) {
    resource.object = new MD2( id );

    hard_assert( resource.nUsers == 0 );
  }

  ++resource.nUsers;
  return resource.object;
}

void Context::releaseMD2( int id )
{
  Resource<MD2*>& resource = md2s[id];

  hard_assert( resource.object != null && resource.nUsers > 0 );

  --resource.nUsers;
}

MD3* Context::requestMD3( int id )
{
  Resource<MD3*>& resource = md3s[id];

  if( resource.object == null ) {
    resource.object = new MD3( id );

    hard_assert( resource.nUsers == 0 );
  }

  ++resource.nUsers;
  return resource.object;
}

void Context::releaseMD3( int id )
{
  Resource<MD3*>& resource = md3s[id];

  hard_assert( resource.object != null && resource.nUsers > 0 );

  --resource.nUsers;
}

void Context::drawBSP( const Struct* str, int mask )
{
  Resource<BSP*>& resource = bsps[str->id];

  // we don't count users, just to show there is at least one
  resource.nUsers = 1;

  if( resource.object == null ) {
    resource.object = new BSP( str->id );
  }
  else if( resource.object->isLoaded ) {
    resource.object->draw( str, mask );
  }
}

void Context::playBSP( const Struct* str )
{
  Resource<BSP*>& resource = bsps[str->id];

  // we don't count users, just to show there is at least one
  resource.nUsers = 1;

  if( resource.object == null ) {
    resource.object = new BSP( str->id );
  }
  else if( resource.object->isLoaded ) {
    resource.object->play( str );
  }
}

void Context::drawImago( const Object* obj, const Imago* parent, int mask )
{
  hard_assert( obj->flags & Object::IMAGO_BIT );

  Imago* const* value = imagines.find( obj->index );

  if( value == null ) {
    const Imago::CreateFunc* createFunc = imagoClasses.find( obj->clazz->imagoType );
    if( createFunc == null ) {
      throw Exception( "Invalid Imago '" + obj->clazz->imagoType + "'" );
    }

    value = imagines.add( obj->index, ( *createFunc )( obj ) );
  }

  Imago* imago = *value;

  imago->flags |= Imago::UPDATED_BIT;
  imago->draw( parent, mask );
}

void Context::playAudio( const Object* obj, const Audio* parent )
{
  hard_assert( obj->flags & Object::AUDIO_BIT );

  Audio* const* value = audios.find( obj->index );

  if( value == null ) {
    const Audio::CreateFunc* createFunc = audioClasses.find( obj->clazz->audioType );
    if( createFunc == null ) {
      throw Exception( "Invalid Audio '" + obj->clazz->audioType + "'" );
    }

    value = audios.add( obj->index, ( *createFunc )( obj ) );
  }

  Audio* audio = *value;

  audio->flags |= Audio::UPDATED_BIT;
  audio->play( parent );
}

# ifndef NDEBUG
void Context::updateLoad()
{
  maxImagines   = max( maxImagines, imagines.length() );
  maxAudios     = max( maxAudios, audios.length() );
  maxSources    = max( maxSources, sources.length() );
  maxBSPSources = max( maxBSPSources, bspSources.length() );
  maxObjSources = max( maxObjSources, objSources.length() );
}

void Context::printLoad()
{
  log.println( "Context maximum load {" );
  log.indent();
  log.println( "Imagines     %d (hashtable load %.2f)", maxImagines,
               float( maxImagines ) / float( imagines.capacity() ) );
  log.println( "Audios       %d (hashtable load %.2f)", maxAudios,
               float( maxAudios ) / float( audios.capacity() ) );
  log.println( "Sources      %d", maxSources );
  log.println( "BSPSources   %d (hashtable load %.2f)", maxBSPSources,
               float( maxBSPSources ) / float( bspSources.capacity() ) );
  log.println( "ObjSources   %d (hashtable load %.2f)", maxObjSources,
               float( maxObjSources ) / float( bspSources.capacity() ) );
  log.unindent();
  log.println( "}" );
}
# endif

void Context::load()
{}

void Context::unload()
{
  log.println( "Unloading Context {" );
  log.indent();

  imagines.free();
  imagines.dealloc();
  audios.free();
  audios.dealloc();

  OZ_AL_CHECK_ERROR();

  for( int i = 0; i < library.bsps.length(); ++i ) {
    delete bsps[i].object;
    bsps[i].object = null;
    bsps[i].nUsers = 0;
  }
  for( int i = 0; i < library.models.length(); ++i ) {
    delete smms[i].object;
    smms[i].object = null;
    smms[i].nUsers = 0;

    delete md2s[i].object;
    md2s[i].object = null;
    md2s[i].nUsers = 0;

    delete md3s[i].object;
    md3s[i].object = null;
    md3s[i].nUsers = 0;
  }

  while( !sources.isEmpty() ) {
    alDeleteSources( 1, &sources.first()->id );
    removeSource( sources.first(), null );
    OZ_AL_CHECK_ERROR();
  }
  for( auto i = bspSources.iter(); i.isValid(); ) {
    auto src = i;
    ++i;

    alDeleteSources( 1, &src.value().id );
    removeBSPSource( &src.value(), src.key() );
    OZ_AL_CHECK_ERROR();
  }
  for( auto i = objSources.iter(); i.isValid(); ) {
    auto src = i;
    ++i;

    alDeleteSources( 1, &src.value().id );
    removeObjSource( &src.value(), src.key() );
    OZ_AL_CHECK_ERROR();
  }

  sources.free();
  bspSources.clear();
  bspSources.dealloc();
  objSources.clear();
  objSources.dealloc();

  for( int i = 0; i < library.textures.length(); ++i ) {
    hard_assert( textures[i].nUsers == 0 );
  }
  for( int i = 0; i < library.sounds.length(); ++i ) {
    hard_assert( sounds[i].nUsers == 0 );
  }

  hard_assert( glGetError() == AL_NO_ERROR );
  OZ_AL_CHECK_ERROR();

  Source::pool.free();

  SMMImago::pool.free();
  SMMVehicleImago::pool.free();
  ExplosionImago::pool.free();
  MD2Imago::pool.free();
  MD2WeaponImago::pool.free();
  MD3Imago::pool.free();

  BasicAudio::pool.free();
  BotAudio::pool.free();
  VehicleAudio::pool.free();

  log.unindent();
  log.println( "}" );
}

void Context::init()
{
  log.print( "Initialising Context ..." );

  textures = null;
  sounds   = null;
  bsps     = null;
  smms     = null;
  md2s     = null;
  md3s     = null;

  OZ_REGISTER_IMAGOCLASS( SMM );
  OZ_REGISTER_IMAGOCLASS( SMMVehicle );
  OZ_REGISTER_IMAGOCLASS( Explosion );
  OZ_REGISTER_IMAGOCLASS( MD2 );
  OZ_REGISTER_IMAGOCLASS( MD2Weapon );
  OZ_REGISTER_IMAGOCLASS( MD3 );

  OZ_REGISTER_AUDIOCLASS( Basic );
  OZ_REGISTER_AUDIOCLASS( Bot );
  OZ_REGISTER_AUDIOCLASS( Vehicle );

  if( library.textures.length() == 0 ) {
    throw Exception( "Context: textures missing!" );
  }
  if( library.sounds.length() == 0 ) {
    throw Exception( "Context: sounds missing!" );
  }
  if( library.bsps.length() == 0 ) {
    throw Exception( "Context: BSPs missing!" );
  }
  if( library.models.length() == 0 ) {
    throw Exception( "Context: models missing!" );
  }

  textures = new Resource<uint>[library.textures.length()];
  sounds   = new Resource<uint>[library.sounds.length()];
  bsps     = new Resource<BSP*>[library.bsps.length()];
  smms     = new Resource<SMM*>[library.models.length()];
  md2s     = new Resource<MD2*>[library.models.length()];
  md3s     = new Resource<MD3*>[library.models.length()];

  buffer.alloc( BUFFER_SIZE );

  for( int i = 0; i < library.textures.length(); ++i ) {
    textures[i].nUsers = 0;
  }
  for( int i = 0; i < library.sounds.length(); ++i ) {
    sounds[i].nUsers = 0;
  }
  for( int i = 0; i < library.bsps.length(); ++i ) {
    bsps[i].object = null;
    bsps[i].nUsers = 0;
  }
  for( int i = 0; i < library.models.length(); ++i ) {
    smms[i].object = null;
    smms[i].nUsers = 0;

    md2s[i].object = null;
    md2s[i].nUsers = 0;

    md3s[i].object = null;
    md3s[i].nUsers = 0;
  }

  maxImagines   = 0;
  maxAudios     = 0;
  maxSources    = 0;
  maxBSPSources = 0;
  maxObjSources = 0;

  log.printEnd( " OK" );
}

void Context::free()
{
  log.print( "Freeing Context ..." );

  buffer.dealloc();

  delete[] textures;
  delete[] sounds;
  delete[] bsps;
  delete[] smms;
  delete[] md2s;
  delete[] md3s;

  textures = null;
  sounds   = null;
  bsps     = null;
  smms     = null;
  md2s     = null;
  md3s     = null;

  imagoClasses.clear();
  imagoClasses.dealloc();
  audioClasses.clear();
  audioClasses.dealloc();

  log.printEnd( " OK" );
}

}
}
