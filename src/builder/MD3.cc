/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2012 Davorin Učakar
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
 * @file builder/MD3.cc
 */

#include "stable.hh"

#include "builder/MD3.hh"

#include "client/Context.hh"

#include "builder/Compiler.hh"

namespace oz
{
namespace builder
{

void MD3::readAnimData()
{
  PFile animFile( sPath + "/animation.cfg" );

  String realPath = animFile.realDir() + "/" + animFile.path();

  FILE* fs = fopen( realPath, "r" );
  if( fs == null ) {
    throw Exception( "Reading animation data failed" );
  }

  char line[1024];

  while( fgets( line, 1024, fs ) != null ) {

  }

  fclose( fs );
}

void MD3::buildMesh( const char* name, int frame )
{
  Log::print( "Mesh '%s' ...", name );

  PFile file( String::str( "%s/%s.md3", sPath.cstr(), name ) );
  if( !file.map() ) {
    throw Exception( "Cannot mmap MD3 model part file '%s'", file.path().cstr() );
  }

  InputStream is = file.inputStream( Endian::LITTLE );

  MD3Header header;

  header.id[0]   = is.readChar();
  header.id[1]   = is.readChar();
  header.id[2]   = is.readChar();
  header.id[3]   = is.readChar();
  header.version = is.readInt();

  if( header.id[0] != 'I' || header.id[1] != 'D' || header.id[2] != 'P' || header.id[3] != '3' ) {
    throw Exception( "MD3 model part file has an invalid format" );
  }

  // header.fileName
  is.forward( 64 );

  header.flags       = is.readInt();
  header.nFrames     = is.readInt();
  header.nTags       = is.readInt();
  header.nSurfaces   = is.readInt();
  header.nSkins      = is.readInt();
  header.offFrames   = is.readInt();
  header.offTags     = is.readInt();
  header.offSurfaces = is.readInt();
  header.offEnd      = is.readInt();

  if( header.nFrames == 0 || header.nSurfaces == 0 ) {
    throw Exception( "Invalid MD3 header counts" );
  }

  if( String::equals( name, "lower" ) ) {
    nLowerFrames = header.nFrames;
  }
  else if( String::equals( name, "upper" ) ) {
    nUpperFrames = header.nFrames;
  }

  if( header.nTags != 0 ) {
    is.reset();
    is.forward( header.offTags );

    for( int i = 0; i < header.nTags; ++i ) {
      const char* tag = is.forward( 64 );

      float tx  = is.readFloat();
      float ty  = is.readFloat();
      float tz  = is.readFloat();

      float m00 = is.readFloat();
      float m01 = is.readFloat();
      float m02 = is.readFloat();
      float m10 = is.readFloat();
      float m11 = is.readFloat();
      float m12 = is.readFloat();
      float m20 = is.readFloat();
      float m21 = is.readFloat();
      float m22 = is.readFloat();

      Vec3  transl = Vec3( scale * -ty, scale * tx, scale * tz );
      Mat44 rotMat = Mat44( +m11, -m10, -m12, 0.0f,
                            -m01, +m00, +m02, 0.0f,
                            -m21, +m20, +m22, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f );

      if( String::equals( name, "lower" ) ) {
        if( String::equals( name, "tag_torso" ) ) {
          joints[i][client::MD3::JOINT_HIP].transl = transl;
          joints[i][client::MD3::JOINT_HIP].rot    = rotMat.toQuat();
        }
      }
      else if( String::equals( name, "upper" ) ) {
        if( String::equals( tag, "tag_head" ) ) {
          joints[i][client::MD3::JOINT_NECK].transl = transl;
          joints[i][client::MD3::JOINT_NECK].rot    = rotMat.toQuat();
        }
        else if( String::equals( tag, "tag_weapon" ) ) {
          joints[i][client::MD3::JOINT_WEAPON].transl = transl;
          joints[i][client::MD3::JOINT_WEAPON].rot    = rotMat.toQuat();
        }
      }
    }
  }

  // FIXME indexBase unused
  int indexBase = 0;

  is.reset();
  is.forward( header.offSurfaces );

  for( int i = 0; i < header.nSurfaces; ++i ) {
    int surfaceStart = is.length();

    MD3Surface surface;

    surface.id           = is.readInt();
    aCopy( surface.name, is.forward( 64 ), 64 );
    surface.flags        = is.readInt();

    surface.nFrames      = is.readInt();
    surface.nShaders     = is.readInt();
    surface.nVertices    = is.readInt();
    surface.nTriangles   = is.readInt();

    surface.offTriangles = is.readInt();
    surface.offShaders   = is.readInt();
    surface.offTexCoords = is.readInt();
    surface.offVertices  = is.readInt();
    surface.offEnd       = is.readInt();

    if( surface.nFrames == 0 || surface.nTriangles == 0 || surface.nShaders == 0 ||
        surface.nVertices == 0 )
    {
      throw Exception( "Invalid MD3 surface counts" );
    }

    if( surface.nFrames != header.nFrames ) {
      throw Exception( "Invalid MD3 surface # of frames" );
    }

    String texture;

    DArray<MD3Triangle> surfaceTriangles( surface.nTriangles );
    DArray<MD3Shader>   surfaceShaders( surface.nShaders );
    DArray<TexCoord>    surfaceTexCoords( surface.nVertices );
    DArray<MD3Vertex>   surfaceVertices( surface.nFrames * surface.nVertices );
    DArray<Vec3>        normals( surfaceVertices.length() );
    DArray<Point>       vertices( surfaceVertices.length() );

    is.reset();
    is.forward( surfaceStart + surface.offTriangles );

    for( int j = 0; j < surfaceTriangles.length(); ++j ) {
      surfaceTriangles[j].vertices[0] = is.readInt();
      surfaceTriangles[j].vertices[1] = is.readInt();
      surfaceTriangles[j].vertices[2] = is.readInt();
    }

    is.reset();
    is.forward( surfaceStart + surface.offShaders );

    for( int i = 0; i < surfaceShaders.length(); ++i ) {
      aCopy( surfaceShaders[i].name, is.forward( 64 ), 64 );
      surfaceShaders[i].index = is.readInt();
    }

    if( skin.isEmpty() ) {
      PFile skinFile( String::replace( surfaceShaders[0].name, '\\', '/' ) );
      texture = skinFile.baseName();
    }
    else {
      PFile skinFile( skin );
      texture = skinFile.baseName();
    }

    is.reset();
    is.forward( surfaceStart + surface.offTexCoords );

    for( int j = 0; j < surfaceTexCoords.length(); ++j ) {
      surfaceTexCoords[j].u = is.readFloat();
      surfaceTexCoords[j].v = 1.0f - is.readFloat();
    }

    is.reset();
    is.forward( surfaceStart + surface.offVertices );

    for( int j = 0; j < surfaceVertices.length(); ++j ) {
      vertices[j].y = float( +is.readShort() ) / 64.0f * scale;
      vertices[j].x = float( -is.readShort() ) / 64.0f * scale;
      vertices[j].z = float( +is.readShort() ) / 64.0f * scale;

      float h  = float( is.readChar() ) / 255.0f * Math::TAU;
      float v  = float( is.readChar() ) / 255.0f * Math::TAU;
      float xy = Math::sin( v );

      normals[j].y = +Math::cos( h ) * xy;
      normals[j].x = -Math::sin( h ) * xy;
      normals[j].z = +Math::cos( v );
    }

    is.reset();
    is.forward( surfaceStart + surface.offEnd );

    compiler.texture( sPath + "/" + texture );

    compiler.begin( Compiler::TRIANGLES );

    for( int j = 0; j < surfaceTriangles.length(); ++j ) {
      for( int k = 0; k < 3; ++k ) {
        int l = surfaceTriangles[j].vertices[k];
        int m = frame < 0 ? l : frame * surface.nVertices + l;

        compiler.texCoord( surfaceTexCoords[l] );
        compiler.normal( meshTransf * normals[m] );
        compiler.vertex( meshTransf * vertices[m] );
      }
    }

    compiler.end();

    indexBase += surface.nVertices;
  }

  file.unmap();

  Log::printEnd( " OK" );
}

void MD3::load()
{
  PFile configFile( sPath + "/config.json" );

  JSON config;
  config.load( &configFile );

  scale      = config["scale"].get( 0.04f );
  skin       = config["skin"].get( "" );

  model      = config["model"].get( "" );
  frame      = config["frame"].get( -1 );
  lowerFrame = config["lowerFrame"].get( -1 );
  upperFrame = config["upperFrame"].get( -1 );
  shaderName = config["shader"].get( frame < 0 ? "md3" : "mesh" );

  Vec3 weaponTranslation = Vec3::ZERO;
  Vec3 weaponRotation    = Vec3::ZERO;

  config["weaponTranslate"].get( weaponTranslation, 3 );
  config["weaponRotate"].get( weaponRotation, 3 );

  Mat44 weaponTransf = Mat44::translation( weaponTranslation );
  weaponTransf.rotateX( Math::rad( weaponRotation.x ) );
  weaponTransf.rotateZ( Math::rad( weaponRotation.z ) );
  weaponTransf.rotateY( Math::rad( weaponRotation.y ) );

  config.clear( true );
}

void MD3::save()
{
  BufferStream os;

  compiler.beginMesh();

  compiler.enable( Compiler::UNIQUE );
  compiler.enable( Compiler::CLOCKWISE );
  compiler.shader( shaderName );

  if( !String::isEmpty( model ) ) {
    if( frame < 0 ) {
      throw Exception( "Custom models can only be static. Must specify frame" );
    }

    meshTransf = Mat44::ID;

    buildMesh( model, frame );
  }
  else if( frame >= 0 ) {
    meshTransf = Mat44::ID;

    buildMesh( "lower", frame );

    meshTransf = meshTransf * Mat44::translation( joints[frame][client::MD3::JOINT_HIP].transl );
    meshTransf = meshTransf * Mat44::rotation( joints[frame][client::MD3::JOINT_HIP].rot );

    buildMesh( "upper", frame );

    meshTransf = meshTransf * Mat44::translation( joints[frame][client::MD3::JOINT_NECK].transl );
    meshTransf = meshTransf * Mat44::rotation( joints[frame][client::MD3::JOINT_NECK].rot );

    buildMesh( "head", 0 );
  }
  else {
    compiler.component( 0 );
    buildMesh( "lower", frame );

    compiler.component( 1 );
    buildMesh( "upper", frame );

    compiler.component( 2 );
    buildMesh( "head", 0 );

    os.writeInt( nLowerFrames );
    os.writeInt( nUpperFrames );

    for( int i = 0; i < nLowerFrames; ++i ) {
      os.writeVec3( joints[i][client::MD3::JOINT_HIP].transl );
      os.writeQuat( joints[i][client::MD3::JOINT_HIP].rot );
    }
    for( int i = 0; i < nUpperFrames; ++i ) {
      os.writeVec3( joints[i][client::MD3::JOINT_NECK].transl );
      os.writeQuat( joints[i][client::MD3::JOINT_NECK].rot );
    }
    for( int i = 0; i < nUpperFrames; ++i ) {
      os.writeVec3( joints[i][client::MD3::JOINT_WEAPON].transl );
      os.writeQuat( joints[i][client::MD3::JOINT_WEAPON].rot );
    }
  }

  compiler.endMesh();
  compiler.writeMesh( &os );

  File::mkdir( sPath );

  if( frame < 0 ) {
    File destFile( sPath + "/data.ozcMD3" );

    Log::print( "Writing to '%s' ...", destFile.path().cstr() );

    if( !destFile.write( os.begin(), os.length() ) ) {
      throw Exception( "Failed to write '%s'", destFile.path().cstr() );
    }

    Log::printEnd( " OK" );
  }
  else {
    File destFile( sPath + "/data.ozcSMM" );

    Log::print( "Writing to '%s' ...", destFile.path().cstr() );

    if( !destFile.write( os.begin(), os.length() ) ) {
      throw Exception( "Failed to write '%s'", destFile.path().cstr() );
    }

    Log::printEnd( " OK" );
  }
}

void MD3::build( const char* path )
{
  Log::println( "Prebuilding MD3 model '%s' {", path );
  Log::indent();

  sPath = path;

  load();
  save();

  sPath      = "";
  skin       = "";
  masks      = "";
  model      = "";
  shaderName = "";

  Log::unindent();
  Log::println( "}" );
}

MD3 md3;

}
}
