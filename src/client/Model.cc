/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2013 Davorin Učakar
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
 */

/**
 * @file client/Model.cc
 */

#include <client/Model.hh>

#include <client/Terra.hh>
#include <client/Context.hh>
#include <client/Camera.hh>

namespace oz
{
namespace client
{

void Vertex::setFormat()
{
  glEnableVertexAttribArray( Attrib::POSITION );
  glVertexAttribPointer( Attrib::POSITION, 3, GL_FLOAT, GL_FALSE, int( sizeof( Vertex ) ),
                         static_cast<char*>( nullptr ) + offsetof( Vertex, pos ) );

  glEnableVertexAttribArray( Attrib::TEXCOORD );
  glVertexAttribPointer( Attrib::TEXCOORD, 2, GL_FLOAT, GL_FALSE, int( sizeof( Vertex ) ),
                         static_cast<char*>( nullptr ) + offsetof( Vertex, texCoord ) );

  glEnableVertexAttribArray( Attrib::NORMAL );
  glVertexAttribPointer( Attrib::NORMAL, 3, GL_BYTE, GL_TRUE, int( sizeof( Vertex ) ),
                         static_cast<char*>( nullptr ) + offsetof( Vertex, normal ) );

  glEnableVertexAttribArray( Attrib::TANGENT );
  glVertexAttribPointer( Attrib::TANGENT, 3, GL_BYTE, GL_TRUE, int( sizeof( Vertex ) ),
                         static_cast<char*>( nullptr ) + offsetof( Vertex, tangent ) );

  glEnableVertexAttribArray( Attrib::BINORMAL );
  glVertexAttribPointer( Attrib::BINORMAL, 3, GL_BYTE, GL_TRUE, int( sizeof( Vertex ) ),
                         static_cast<char*>( nullptr ) + offsetof( Vertex, binormal ) );
}

struct Model::LightEntry
{
  const Light* light;
  Mat44        transf;
  float        weight;
};

struct Model::PreloadData
{
  struct TexFiles
  {
    File diffuse;
    File masks;
    File normals;
  };

  File           modelFile;
  List<TexFiles> textures;
};

Set<Model*>             Model::loadedModels;
List<Model::Instance>   Model::instances[2];
List<Model::LightEntry> Model::sceneLights;
Vertex*                 Model::vertexAnimBuffer       = nullptr;
int                     Model::vertexAnimBufferLength = 0;
Model::Collation        Model::collation              = DEPTH_MAJOR;

void Model::addSceneLights()
{
  foreach( light, lights.citer() ) {
    const Node* node = &nodes[light->node];
    Mat44 transf = node->transf;

    while( node->parent >= 0 ) {
      node   = &nodes[node->parent];
      transf = node->transf ^ transf;
    }

    sceneLights.add( { light, transf, 0.0f } );
  }
}

void Model::animate( const Instance* instance )
{
  if( shader.hasVertexTexture ) {
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, animationTexId );

    glUniform3f( uniform.meshAnimation,
                 float( instance->firstFrame  ) / float( nFrames ),
                 float( instance->secondFrame ) / float( nFrames ),
                 instance->interpolation );
  }
  else {
    const Point* currFramePositions = &positions[instance->firstFrame * nFramePositions];
    const Vec3*  currFrameNormals   = &normals[instance->firstFrame * nFramePositions];

    if( instance->interpolation == 0.0f ) {
      for( int i = 0; i < nVertices; ++i ) {
        int j = Math::lround( vertices[i].pos[0] * float( nFramePositions - 1 ) );

        Point pos    = currFramePositions[j];
        Vec3  normal = currFrameNormals[j];

        vertexAnimBuffer[i].pos[0] = pos.x;
        vertexAnimBuffer[i].pos[1] = pos.y;
        vertexAnimBuffer[i].pos[2] = pos.z;

        vertexAnimBuffer[i].texCoord[0] = vertices[i].texCoord[0];
        vertexAnimBuffer[i].texCoord[1] = vertices[i].texCoord[1];

        vertexAnimBuffer[i].normal[0] = byte( normal.x * 127.0f );
        vertexAnimBuffer[i].normal[1] = byte( normal.y * 127.0f );
        vertexAnimBuffer[i].normal[2] = byte( normal.z * 127.0f );
      }
    }
    else {
      const Point* nextFramePositions = &positions[instance->secondFrame * nFramePositions];
      const Vec3*  nextFrameNormals   = &normals[instance->secondFrame * nFramePositions];

      for( int i = 0; i < nVertices; ++i ) {
        int j = Math::lround( vertices[i].pos[0] * float( nFramePositions - 1 ) );

        Point pos    = Math::mix( currFramePositions[j], nextFramePositions[j], instance->interpolation );
        Vec3  normal = Math::mix( currFrameNormals[j],   nextFrameNormals[j],   instance->interpolation );

        vertexAnimBuffer[i].pos[0] = pos.x;
        vertexAnimBuffer[i].pos[1] = pos.y;
        vertexAnimBuffer[i].pos[2] = pos.z;

        vertexAnimBuffer[i].texCoord[0] = vertices[i].texCoord[0];
        vertexAnimBuffer[i].texCoord[1] = vertices[i].texCoord[1];

        vertexAnimBuffer[i].normal[0] = byte( normal.x * 127.0f );
        vertexAnimBuffer[i].normal[1] = byte( normal.y * 127.0f );
        vertexAnimBuffer[i].normal[2] = byte( normal.z * 127.0f );
      }
    }

    upload( vertexAnimBuffer, nVertices, GL_STREAM_DRAW );
  }
}

void Model::drawNode( const Node* node, int dir, int mask )
{
  tf.push();
  tf.model = tf.model ^ node->transf;

  if( node->mesh >= 0 ) {
    const Mesh& mesh = meshes[node->mesh];

    if( mesh.flags & mask ) {
      const Texture& texture = textures[mesh.texture];

      tf.apply();

      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, texture.diffuse );
      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, texture.masks );

      glDrawElements( GL_TRIANGLES, mesh.nIndices, GL_UNSIGNED_SHORT,
                      static_cast<ushort*>( nullptr ) + dir * nIndices + mesh.firstIndex );
    }
  }

  for( int i = 0; i < node->nChildren; ++i ) {
    drawNode( &nodes[node->firstChild + i], dir, mask );
  }

  tf.pop();
}

void Model::draw( const Instance* instance, int mask )
{
  tf.model  = instance->transf;
  tf.colour = instance->colour;
  tf.applyColour();

  Vec3 localDir = ~instance->transf * camera.at;
  int  dir      = ( localDir.x < 0.0f ) | ( localDir.y < 0.0f ) << 1 | ( localDir.z < 0.0f ) << 2;

  drawNode( &nodes[instance->node], dir, mask );
}

void Model::setCollation( Collation collation_ )
{
  collation = collation_;
}

void Model::drawScheduled( QueueType queue, int mask )
{
  if( collation == MODEL_MAJOR ) {
    foreach( i, loadedModels.citer() ) {
      Model* model = *i;

      if( model->modelInstances[queue].isEmpty() ) {
        continue;
      }

      glBindBuffer( GL_ARRAY_BUFFER, model->vbo );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, model->ibo );

      Vertex::setFormat();

      shader.program( model->shaderId );

      foreach( instance, model->modelInstances[queue].citer() ) {
        // HACK This is not a nice way to draw non-transparent parts with alpha < 1.
        int instanceMask = mask;

        if( instance->colour.w.w != 1.0f ) {
          if( mask & ALPHA_BIT ) {
            instanceMask |= SOLID_BIT;
          }
          else {
            continue;
          }
        }

        if( !( model->flags & instanceMask ) ) {
          continue;
        }

        if( model->nFrames != 0 ) {
          model->animate( instance );
        }

        model->draw( instance, instanceMask );
      }
    }
  }
  else {
    Model* model = nullptr;

    foreach( instance, instances[queue].citer() ) {
      if( instance->model != model ) {
        model = instance->model;

        glBindBuffer( GL_ARRAY_BUFFER, model->vbo );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, model->ibo );

        Vertex::setFormat();

        shader.program( model->shaderId );
      }

      // HACK This is not a nice way to draw non-transparent parts for which alpha < 1 has been set.
      int instanceMask = mask;

      if( instance->colour.w.w != 1.0f ) {
        if( mask & ALPHA_BIT ) {
          instanceMask |= SOLID_BIT;
        }
        else {
          continue;
        }
      }

      if( !( model->flags & instanceMask ) ) {
        continue;
      }

      if( model->nFrames != 0 ) {
        model->animate( instance );
      }

      model->draw( instance, instanceMask );
    }
  }

  glActiveTexture( GL_TEXTURE2 );
  glBindTexture( GL_TEXTURE_2D, 0 );

  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, shader.defaultMasks );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, shader.defaultTexture );

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void Model::clearScheduled( QueueType queue )
{
  if( collation == MODEL_MAJOR ) {
    foreach( i, loadedModels.citer() ) {
      Model* model = *i;

      model->modelInstances[queue].clear();
    }
  }
  else {
    instances[queue].clear();
  }

  sceneLights.clear();
}

void Model::deallocate()
{
  loadedModels.deallocate();

  delete[] vertexAnimBuffer;
  vertexAnimBuffer = nullptr;
  vertexAnimBufferLength = 0;

  instances[SCENE_QUEUE].deallocate();
  instances[OVERLAY_QUEUE].deallocate();

  sceneLights.deallocate();
}

Model::Model( const String& path_ ) :
  path( path_ ), vbo( 0 ), ibo( 0 ), animationTexId( 0 ),
  nTextures( 0 ), nVertices( 0 ), nIndices( 0 ), nFrames( 0 ), nFramePositions( 0 ),
  vertices( nullptr ), positions( nullptr ), normals( nullptr ),
  preloadData( nullptr ), dim( Vec3::ONE )
{}

Model::~Model()
{
  unload();
}

int Model::findNode( const char* name ) const
{
  for( int i = 0; i < nodes.length(); ++i ) {
    if( nodes[i].name.equals( name ) ) {
      return i;
    }
  }
  return -1;
}

void Model::schedule( int mesh, QueueType queue )
{
  List<Instance>& list = collation == MODEL_MAJOR ? modelInstances[queue] : instances[queue];

  if( shader.nLights != 0 && lights.isEmpty() != 0 ) {
    addSceneLights();
  }

  list.add( { this, tf.model, tf.colour, mesh, 0, 0, 0.0f } );
}

void Model::scheduleFrame( int mesh, int frame, QueueType queue )
{
  List<Instance>& list = collation == MODEL_MAJOR ? modelInstances[queue] : instances[queue];

  if( shader.nLights != 0 && lights.isEmpty() != 0 ) {
    addSceneLights();
  }

  list.add( { this, tf.model, tf.colour, mesh, frame, 0, 0.0f } );
}

void Model::scheduleAnimated( int mesh, int firstFrame, int secondFrame, float interpolation,
                              QueueType queue )
{
  List<Instance>& list = collation == MODEL_MAJOR ? modelInstances[queue] : instances[queue];

  if( shader.nLights != 0 && lights.isEmpty() != 0 ) {
    addSceneLights();
  }

  list.add( { this, tf.model, tf.colour, mesh, firstFrame, secondFrame, interpolation } );
}

const File* Model::preload()
{
  hard_assert( preloadData == nullptr );

  preloadData = new PreloadData();
  preloadData->modelFile = path;

  if( !preloadData->modelFile.map() ) {
    OZ_ERROR( "Failed to map '%s'", path.cstr() );
  }

  InputStream is = preloadData->modelFile.inputStream( Endian::LITTLE );

  dim             = is.readVec3();
  shaderId        = liber.shaderIndex( is.readString() );
  nTextures       = is.readInt();
  nVertices       = is.readInt();
  nIndices        = is.readInt();
  nFrames         = is.readInt();
  nFramePositions = is.readInt();

  is.readInt();
  is.readInt();
  is.readInt();

  if( nTextures >= 0 ) {
    for( int i = 0; i < nTextures; ++i ) {
      const String& name = is.readString();

      preloadData->textures.add();

      if( !name.isEmpty() ) {
        PreloadData::TexFiles& texFiles = preloadData->textures.last();

        texFiles.diffuse = name + ".dds";
        texFiles.masks   = name + "_m.dds";
//         texFiles.normals = name + "_n.dds";

        if( !texFiles.diffuse.map() ) {
          OZ_ERROR( "Failed to map '%s'", texFiles.diffuse.path().cstr() );
        }

        texFiles.masks.map();
//         texFiles.normals.map();
      }
    }
  }

  return &preloadData->modelFile;
}

void Model::upload( const Vertex* vertices, int nVertices, uint usage ) const
{
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  glBufferData( GL_ARRAY_BUFFER, nVertices * int( sizeof( Vertex ) ), vertices, usage );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void Model::load()
{
  flags = 0;

  hard_assert( preloadData != nullptr && preloadData->modelFile.isMapped() );
  InputStream is = preloadData->modelFile.inputStream( Endian::LITTLE );

  OZ_GL_CHECK_ERROR();

  is.readVec3();
  is.readString();
  is.readInt();
  is.readInt();
  is.readInt();
  is.readInt();
  is.readInt();

  int nMeshes = is.readInt();
  int nLights = is.readInt();
  int nNodes  = is.readInt();

  if( nTextures < 0 ) {
    nTextures = ~nTextures;

    textures.resize( nTextures );

    for( int i = 0; i < nTextures; ++i ) {
      const String& name = is.readString();

      int id = name.beginsWith( "@sea:" ) ? terra.liquidTexId : liber.textureIndex( name );
      textures[i] = context.requestTexture( id );
    }
  }
  else {
    textures.resize( nTextures );

    for( int i = 0; i < nTextures; ++i ) {
      is.readString();

      textures[i] = context.loadTexture( preloadData->textures[i].diffuse,
                                         preloadData->textures[i].masks,
                                         preloadData->textures[i].normals );
    }
  }

  uint usage   = nFrames != 0 && shader.hasVertexTexture ? GL_STREAM_DRAW : GL_STATIC_DRAW;
  int  vboSize = nVertices  * int( sizeof( Vertex ) );
  int  iboSize = 8*nIndices * int( sizeof( ushort ) );

  const void* vertexBuffer = is.forward( vboSize );

  glGenBuffers( 1, &vbo );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  glBufferData( GL_ARRAY_BUFFER, vboSize, vertexBuffer, usage );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  glGenBuffers( 1, &ibo );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, iboSize, is.forward( iboSize ), GL_STATIC_DRAW );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  if( nFrames != 0 ) {
    if( shader.hasVertexTexture ) {
#ifndef GL_ES_VERSION_2_0
      int vertexBufferSize = nFramePositions * nFrames * int( sizeof( float[3] ) );
      int normalBufferSize = nFramePositions * nFrames * int( sizeof( float[3] ) );

      glGenTextures( 1, &animationTexId );
      glBindTexture( GL_TEXTURE_2D, animationTexId );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, nFramePositions, 2 * nFrames, 0, GL_RGB,
                    GL_FLOAT, is.forward( vertexBufferSize + normalBufferSize ) );
      glBindTexture( GL_TEXTURE_2D, shader.defaultTexture );

      OZ_GL_CHECK_ERROR();
#endif
    }
    else {
      vertices  = new Vertex[nVertices];
      positions = new Point[nFramePositions * nFrames];
      normals   = new Vec3[nFramePositions * nFrames];

      for( int i = 0; i < nFramePositions * nFrames; ++i ) {
        positions[i] = is.readPoint();
      }
      for( int i = 0; i < nFramePositions * nFrames; ++i ) {
        normals[i] = is.readVec3();
      }

      mCopy( vertices, vertexBuffer, size_t( nVertices ) * sizeof( Vertex ) );

      if( nVertices > vertexAnimBufferLength ) {
        delete[] vertexAnimBuffer;

        vertexAnimBuffer = new Vertex[nVertices];
        vertexAnimBufferLength = nVertices;
      }
    }
  }

  meshes.resize( nMeshes );

  for( int i = 0; i < nMeshes; ++i ) {
    meshes[i].flags      = is.readInt();
    meshes[i].texture    = is.readInt();

    meshes[i].nIndices   = is.readInt();
    meshes[i].firstIndex = is.readInt();

    flags |= meshes[i].flags & ( SOLID_BIT | ALPHA_BIT );
  }

  lights.resize( nLights );

  for( int i = 0; i < nLights; ++i ) {
    lights[i].node           = is.readInt();
    lights[i].type           = Light::Type( is.readInt() );

    lights[i].pos            = is.readPoint();
    lights[i].dir            = is.readVec3();
    lights[i].colour         = is.readVec3();

    lights[i].attenuation[0] = is.readFloat();
    lights[i].attenuation[1] = is.readFloat();
    lights[i].attenuation[2] = is.readFloat();

    lights[i].coneCoeff[0]   = is.readFloat();
    lights[i].coneCoeff[1]   = is.readFloat();
  }

  nodes.resize( nNodes );

  for( int i = 0; i < nNodes; ++i ) {
    nodes[i].transf     = is.readMat44();
    nodes[i].mesh       = is.readInt();

    nodes[i].parent     = is.readInt();
    nodes[i].firstChild = is.readInt();
    nodes[i].nChildren  = is.readInt();

    nodes[i].name       = is.readString();
  }

  loadedModels.add( this );

  delete preloadData;
  preloadData = nullptr;

  OZ_GL_CHECK_ERROR();
}

void Model::unload()
{
  if( preloadData != nullptr ) {
    delete preloadData;
    preloadData = nullptr;
  }

  if( vbo == 0 ) {
    return;
  }

  foreach( texture, textures.citer() ) {
    if( texture->id >= -1 ) {
      context.releaseTexture( texture->id );
    }
    else {
      context.unloadTexture( texture );
    }
  }

  nodes.clear();
  meshes.clear();
  textures.clear();

  if( nFrames != 0 ) {
    if( shader.hasVertexTexture ) {
      glDeleteTextures( 1, &animationTexId );
    }
    else {
      delete[] normals;
      delete[] positions;
      delete[] vertices;
    }
  }

  glDeleteBuffers( 1, &ibo );
  glDeleteBuffers( 1, &vbo );

  ibo = 0;
  vbo = 0;

  loadedModels.exclude( this );

  OZ_GL_CHECK_ERROR();
}

}
}
