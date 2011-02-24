/*
 *  Context.hpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

#include "stable.hpp"

#include "matrix/Translator.hpp"

#include "client/Mesh.hpp"
#include "client/BSP.hpp"
#include "client/Model.hpp"
#include "client/Audio.hpp"

#include <GL/gl.h>

namespace oz
{
namespace client
{

  class SMM;
  class MD2;
  class MD3;

  class Context
  {
    friend class Render;
    friend class Sound;
    friend class Audio;
    friend class Loader;

    private:

      static const int DEFAULT_MAG_FILTER = GL_LINEAR;
      static const int DEFAULT_MIN_FILTER = GL_LINEAR_MIPMAP_LINEAR;
      static const int VORBIS_BUFFER_SIZE = 1024 * 1024;

      template <typename Type>
      struct Resource
      {
        Type id;
        // for sounds:
        //  0: loaded, but no source needs it
        // -1: not loaded
        // -2: scheduled for removal
        int  nUsers;
      };

      template <typename Type>
      struct Resource<Type*>
      {
        Type* object;
        int  nUsers;
      };

      struct Lists
      {
        uint base;
        int  count;
      };

      struct Source
      {
        uint    source;
        Source* next[1];

        explicit Source( uint sourceId ) : source( sourceId )
        {}

        static Pool<Source> pool;

        OZ_STATIC_POOL_ALLOC( pool )
      };

      struct ContSource
      {
        uint source;
        bool isUpdated;

        explicit ContSource( uint sourceId ) : source( sourceId ), isUpdated( true )
        {}
      };

      char                              vorbisBuffer[VORBIS_BUFFER_SIZE];

    private:

      HashString<Model::CreateFunc, 16> modelClasses;
      HashString<Audio::CreateFunc, 8>  audioClasses;

      Resource<uint>*                   textures;
      Resource<uint>*                   sounds;

      Sparse<Lists>                     lists;

      ContSource*                       cachedSource;
      List<Source>                      sources;
      HashIndex<ContSource, 256>        contSources;

      Resource<BSP*>*                   bsps;

      HashString< Resource<SMM*>, 64 >  smms;
      HashString< Resource<MD2*>, 64 >  staticMd2s;
      HashString< Resource<MD2*>, 64 >  md2s;
      HashString< Resource<MD3*>, 64 >  staticMd3s;
      HashString< Resource<MD3*>, 64 >  md3s;

      HashIndex<Model*, 8191>           models;   // currently loaded models
      HashIndex<Audio*, 1021>           audios;   // currently loaded audio models

      int                               maxModels;
      int                               maxAudios;
      int                               maxSources;
      int                               maxContSources;

      static uint buildTexture( const void* data, int width, int height, int bytesPerPixel,
                                bool wrap, int magFilter, int minFilter );
      static uint buildNormalmap( void* data, const Vec3& lightNormal, int width,int height,
                                  int bytesPerPixel, bool wrap, int magFilter, int minFilter );
      void deleteSound( int resource );

    public:

      uint detailTexture;

      static uint createTexture( const void* data, int width, int height, int bytesPerPixel,
                          bool wrap = true, int magFilter = DEFAULT_MAG_FILTER,
                          int minFilter = DEFAULT_MIN_FILTER );

      static uint loadTexture( const char* path, bool wrap = true,
                               int magFilter = DEFAULT_MAG_FILTER,
                               int minFilter = DEFAULT_MIN_FILTER );

      static uint readTexture( InputStream* stream );

      static void getTextureSize( uint id, int* nMipmaps, int* size );
      static void writeTexture( uint id, int nMipmaps, OutputStream* stream );

      uint requestTexture( int id );
      void releaseTexture( int id );

      uint requestSound( int id );
      void releaseSound( int id );

      BSP* loadBSP( int resource );
      void releaseBSP( int resource );

      static void beginArrayMode();
      static void endArrayMode();

      static void setVertexFormat();

      static void bindTextures( uint texture0 = 0, uint texture1 = 0, uint texture2 = 0 );

      uint genList();
      uint genLists( int count );
      void deleteLists( uint listId );

      SMM* loadSMM( const char* name );
      void releaseSMM( const char* name );

      MD2* loadStaticMD2( const char* name );
      void releaseStaticMD2( const char* name );

      MD2* loadMD2( const char* name );
      void releaseMD2( const char* name );

      MD3* loadStaticMD3( const char* name );
      void releaseStaticMD3( const char* name );

      MD3* loadMD3( const char* name );
      void releaseMD3( const char* name );

      void drawBSP( const Struct* str, int mask );

      void drawModel( const Object* obj, const Model* parent );
      void playAudio( const Object* obj, const Audio* parent );

      void updateLoad();
      void printLoad();

      void load();
      void unload();

      void init();
      void free();

  };

  extern Context context;

}
}
