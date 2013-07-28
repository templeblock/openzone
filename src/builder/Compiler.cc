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
 * @file builder/Compiler.cc
 */

#include <builder/Compiler.hh>

#include <builder/Context.hh>

namespace oz
{
namespace builder
{

struct Triangle
{
  float depth;
  int   index;

  bool operator < ( const Triangle& t ) const
  {
    return depth < t.depth;
  }
};

void Vertex::write( OutputStream* ostream ) const
{
  ostream->writeFloat( pos.x );
  ostream->writeFloat( pos.y );
  ostream->writeFloat( pos.z );

  ostream->writeFloat( texCoord.u );
  ostream->writeFloat( texCoord.v );

  ostream->writeFloat( normal.x );
  ostream->writeFloat( normal.y );
  ostream->writeFloat( normal.z );
}

void Compiler::enable( Capability cap )
{
  caps |= cap;
}

void Compiler::disable( Capability cap )
{
  caps &= ~cap;
}

void Compiler::beginMesh()
{
  hard_assert( !( flags & MESH_BIT ) );
  hard_assert( !( flags & PART_BIT ) );

  parts.clear();
  vertices.clear();
  positions.clear();
  normals.clear();

  bounds.mins     = Point( +Math::INF, +Math::INF, +Math::INF );
  bounds.maxs     = Point( -Math::INF, -Math::INF, -Math::INF );

  vert.pos        = Point::ORIGIN;
  vert.texCoord   = TexCoord( 0.0f, 0.0f );
  vert.normal     = Vec3::ZERO;

  part.component  = 0;
  part.material   = Mesh::SOLID_BIT;
  part.texture    = "";

  caps            = 0;
  flags          |= MESH_BIT;
  componentId     = 0;
  mode            = TRIANGLES;
  shaderName      = "mesh";
  vertNum         = 0;
  nFrames         = 0;
  nFramePositions = 0;
}

void Compiler::endMesh()
{
  hard_assert( flags & MESH_BIT );
  hard_assert( !( flags & PART_BIT ) );

  flags &= ~MESH_BIT;
}

void Compiler::anim( int nFrames_, int nFramePositions_ )
{
  hard_assert( flags & MESH_BIT );
  hard_assert( !( flags & PART_BIT ) );

  if( nFrames <= 1 ) {
    nFrames         = 0;
    nFramePositions = 0;
  }

  nFrames         = nFrames_;
  nFramePositions = nFramePositions_;

  positions.resize( nFrames * nFramePositions );
  normals.resize( nFrames * nFramePositions );
}

void Compiler::component( int id )
{
  hard_assert( flags & MESH_BIT );
  hard_assert( !( flags & PART_BIT ) );

  if( id != componentId && id != componentId + 1 ) {
    OZ_ERROR( "Non-consecutive components %d and %d\n", componentId, id );
  }

  componentId = id;
}

void Compiler::blend( bool doBlend )
{
  hard_assert( flags & MESH_BIT );
  hard_assert( !( flags & PART_BIT ) );

  part.material = doBlend ? int( Mesh::ALPHA_BIT ) : int( Mesh::SOLID_BIT );
}

void Compiler::shader( const char* shaderName_ )
{
  hard_assert( flags & MESH_BIT );
  hard_assert( !( flags & PART_BIT ) );

  shaderName = shaderName_;
}

void Compiler::texture( const char* texture )
{
  hard_assert( flags & MESH_BIT );
  hard_assert( !( flags & PART_BIT ) );

  part.texture = texture;
}

void Compiler::begin( PolyMode mode_ )
{
  hard_assert( flags & MESH_BIT );
  hard_assert( !( flags & PART_BIT ) );

  flags |= PART_BIT;

  mode = mode_;
  vertNum = 0;

  part.component = componentId;
}

void Compiler::end()
{
  hard_assert( flags & MESH_BIT );
  hard_assert( flags & PART_BIT );

  flags &= ~PART_BIT;

  if( caps & CLOCKWISE ) {
    part.indices.reverse();
  }

  switch( mode ) {
    case TRIANGLE_STRIP: {
      hard_assert( vertNum >= 3 );

      for( int i = 3; i < part.indices.length(); i += 3 ) {
        part.indices.insert( i + 0, part.indices[i - 1] );
        part.indices.insert( i + 1, part.indices[i - 2] );
      }
      break;
    }
    case TRIANGLE_FAN: {
      hard_assert( vertNum >= 3 );

      for( int i = 3; i < part.indices.length(); i += 3 ) {
        part.indices.insert( i + 0, part.indices[0] );
        part.indices.insert( i + 1, part.indices[i - 1] );
      }
      break;
    }
    case TRIANGLES: {
      hard_assert( vertNum >= 3 && vertNum % 3 == 0 );
      break;
    }
    case QUADS: {
      hard_assert( vertNum >= 4 && vertNum % 4 == 0 );

      for( int i = 0; i < part.indices.length(); i += 6 ) {
        part.indices.insert( i + 3, part.indices[i + 2] );
        part.indices.insert( i + 5, part.indices[i + 4] );
      }
      break;
    }
    case POLYGON: {
      hard_assert( vertNum >= 3 );

      List<ushort> polyIndices = static_cast< List<ushort>&& >( part.indices );
      part.indices.clear();

      int last[2] = { 0, 1 };

      for( int i = 0; i < vertNum / 2; ++i ) {
        int j = ( i + 3 ) / 2;

        if( i & 1 ) {
          part.indices.add( polyIndices[ last[0] ] );
          part.indices.add( polyIndices[ last[1] ] );
          part.indices.add( polyIndices[j] );

          last[1] = j;
        }
        else {
          part.indices.add( polyIndices[ last[0] ] );
          part.indices.add( polyIndices[ last[1] ] );
          part.indices.add( polyIndices[vertNum - j] );

          last[0] = vertNum - j;
        }
      }
      break;
    }
  }

  int partIndex = parts.index( part );

  if( partIndex < 0 ) {
    parts.add( part );
  }
  else {
    parts[partIndex].indices.addAll( part.indices.begin(), part.indices.length() );
  }

  part.indices.clear();
}

void Compiler::texCoord( float u, float v )
{
  hard_assert( flags & MESH_BIT );

  vert.texCoord[0] = u;
  vert.texCoord[1] = v;
}

void Compiler::texCoord( const float* v )
{
  texCoord( v[0], v[1] );
}

void Compiler::normal( float nx, float ny, float nz )
{
  hard_assert( flags & MESH_BIT );
  hard_assert( nFrames == 0 );

  vert.normal.x = nx;
  vert.normal.y = ny;
  vert.normal.z = nz;
}

void Compiler::normal( const float* v )
{
  normal( v[0], v[1], v[2] );
}

void Compiler::vertex( float x, float y, float z )
{
  hard_assert( flags & MESH_BIT );
  hard_assert( nFrames == 0 || ( y == 0.0f && z == 0.0f ) );

  if( nFrames == 0 ) {
    bounds.mins.x = min( bounds.mins.x, x );
    bounds.mins.y = min( bounds.mins.y, y );
    bounds.mins.z = min( bounds.mins.z, z );

    bounds.maxs.x = max( bounds.maxs.x, x );
    bounds.maxs.y = max( bounds.maxs.y, y );
    bounds.maxs.z = max( bounds.maxs.z, z );
  }

  vert.pos.x = x;
  vert.pos.y = y;
  vert.pos.z = z;

  if( !( flags & PART_BIT ) ) {
    vertices.add( vert );
  }
  else {
    bool doRestart = false;

    if( mode == QUADS && vertNum != 0 && vertNum % 4 == 0 ) {
      doRestart = true;
    }

    if( doRestart ) {
      part.indices.add( part.indices.last() );
    }

    int index;

    if( caps & UNIQUE ) {
      index = vertices.include( vert );

      part.indices.add( ushort( index ) );
    }
    else {
      index = vertices.length();

      vertices.add( vert );
      part.indices.add( ushort( index ) );
    }

    if( doRestart ) {
      part.indices.add( ushort( index ) );
    }

    ++vertNum;
  }
}

void Compiler::vertex( const float* v )
{
  vertex( v[0], v[1], v[2] );
}

void Compiler::animVertex( int i )
{
  hard_assert( nFrames != 0 );
  hard_assert( nFrames > 1 && uint( i ) < uint( positions.length() ) );

  vertex( float( i ), 0.0f, 0.0f );
}

void Compiler::animPositions( const float* positions_ )
{
  hard_assert( nFrames != 0 );

  for( int i = 0; i < positions.length(); ++i ) {
    positions[i].x = *positions_++;
    positions[i].y = *positions_++;
    positions[i].z = *positions_++;

    if( i < nFramePositions ) {
      bounds.mins.x = min( bounds.mins.x, positions[i].x );
      bounds.mins.y = min( bounds.mins.y, positions[i].y );
      bounds.mins.z = min( bounds.mins.z, positions[i].z );

      bounds.maxs.x = max( bounds.maxs.x, positions[i].x );
      bounds.maxs.y = max( bounds.maxs.y, positions[i].y );
      bounds.maxs.z = max( bounds.maxs.z, positions[i].z );
    }
  }
}

void Compiler::animNormals( const float* normals_ )
{
  hard_assert( nFrames != 0 );

  for( int i = 0; i < normals.length(); ++i ) {
    normals[i].x = *normals_++;
    normals[i].y = *normals_++;
    normals[i].z = *normals_++;
  }
}

void Compiler::writeMesh( OutputStream* os, bool globalTextures )
{
  hard_assert( !( flags & MESH_BIT ) && !( flags & PART_BIT ) );
  hard_assert( parts.length() > 0 && vertices.length() > 0 );
  hard_assert( positions.length() == normals.length() );

  Log::print( "Writing mesh ..." );

  List<String> textures;
  List<ushort> indices;

  int nIndices    = 0;
  int nComponents = 0;

  for( int i = 0; i < parts.length(); ++i ) {
    textures.include( parts[i].texture );

    parts[i].firstIndex = nIndices;
    parts[i].nIndices   = parts[i].indices.length();

    // Sort triangles per every (+/-x, +/-y, +/-z) direction.
    List<ushort>   dirIndices[8];
    List<Triangle> triangles;

    for( int j = 0; j < 8; ++j ) {
      Vec3 dir = Vec3( j & 1 ? -1.0f : +1.0f, j & 2 ? -1.0f : +1.0f, j & 4 ? -1.0f : +1.0f );

      for( int k = 0; k < parts[i].indices.length(); k += 3 ) {
        Vec3 centre = ( ( vertices[ parts[i].indices[k + 0] ].pos - Point::ORIGIN ) +
                        ( vertices[ parts[i].indices[k + 1] ].pos - Point::ORIGIN ) +
                        ( vertices[ parts[i].indices[k + 2] ].pos - Point::ORIGIN ) ) / 3.0f;

        triangles.add( { centre * dir, k } );
      }

      triangles.sort();

      for( int k = 0; k < triangles.length(); ++k ) {
        dirIndices[j].add( parts[i].indices[ triangles[k].index + 0 ] );
        dirIndices[j].add( parts[i].indices[ triangles[k].index + 1 ] );
        dirIndices[j].add( parts[i].indices[ triangles[k].index + 2 ] );
      }

      hard_assert( dirIndices[j].length() == parts[i].indices.length() );

      triangles.clear();
      indices.addAll( dirIndices[j].begin(), dirIndices[j].length() );
    }

    nIndices   += 8 * parts[i].nIndices;
    nComponents = max( nComponents, parts[i].component + 1 );
  }

  if( nComponents == 0 ) {
    OZ_ERROR( "Model should have at least one component" );
  }

  os->writeVec3( bounds.dim() );

  os->writeInt( globalTextures ? ~textures.length() : textures.length() );
  os->writeInt( vertices.length() );
  os->writeInt( indices.length() );
  os->writeInt( nFrames );
  os->writeInt( nFramePositions );

  os->writeInt( nComponents );
  os->writeInt( parts.length() );

  os->writeString( shaderName );

  foreach( texture, textures.citer() ) {
    os->writeString( *texture );
  }

  foreach( vertex, vertices.iter() ) {
    if( nFrames != 0 ) {
      vertex->pos = Point( ( vertex->pos[0] + 0.5f ) / float( nFramePositions ), 0.0f, 0.0f );
    }
    vertex->write( os );
  }
  for( int i = 0; i < 8; ++i ) {
    foreach( index, indices.citer() ) {
      os->writeUShort( *index );
    }
  }

  if( nFrames != 0 ) {
    hard_assert( positions.length() == nFrames * nFramePositions );
    hard_assert( normals.length() == nFrames * nFramePositions );

    foreach( position, positions.citer() ) {
      os->writePoint( *position );
    }
    foreach( normal, normals.citer() ) {
      os->writeVec3( *normal );
    }
  }

  foreach( part, parts.citer() ) {
    os->writeInt( part->component | part->material );
    os->writeInt( textures.index( part->texture ) );

    os->writeInt( part->nIndices );
    os->writeInt( part->firstIndex );
  }

  Log::printEnd( " OK" );
}

void Compiler::buildMeshTextures( const char* destDir )
{
  List<String> textures;

  for( int i = 0; i < parts.length(); ++i ) {
    textures.include( parts[i].texture );
  }

  for( int i = 0; i < textures.length(); ++i ) {
    context.buildTexture( textures[i], destDir + ( "/" + textures[i].fileName() ) );
  }
}

void Compiler::init()
{}

void Compiler::destroy()
{
  parts.clear();
  parts.deallocate();

  part.texture = "";
  part.indices.clear();
  part.indices.deallocate();

  vertices.clear();
  vertices.deallocate();

  positions.clear();
  normals.clear();
}

Compiler compiler;

}
}
