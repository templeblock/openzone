/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2014 Davorin Učakar
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
 * @file builder/Context.cc
 */

#include <builder/Context.hh>

#include <client/SMMImago.hh>
#include <client/SMMVehicleImago.hh>
#include <client/ExplosionImago.hh>
#include <client/MD2Imago.hh>
#include <client/MD2WeaponImago.hh>
#include <client/BasicAudio.hh>
#include <client/BotAudio.hh>
#include <client/VehicleAudio.hh>

namespace oz
{
namespace builder
{

const char* const IMAGE_EXTENSIONS[] = {
  ".dds",
  ".png",
  ".jpeg",
  ".jpg",
  ".bmp",
  ".tga",
  ".tiff"
};

bool Context::isBaseTexture( const char* path_ )
{
  String path = path_;
  return !path.endsWith( "_d" ) && !path.endsWith( "_m" ) && !path.endsWith( "_s" ) &&
         !path.endsWith( "_spec" ) && !path.endsWith( "_g" ) && !path.endsWith( "_glow" ) &&
         !path.endsWith( ".blend" ) && !path.endsWith( "_n" ) && !path.endsWith( "_nm" ) &&
         !path.endsWith( "_normal" ) && !path.endsWith( "_local" );
}

void Context::buildTexture( const char* basePath_, const char* destBasePath_, bool allLayers )
{
  Log::print( "Building texture(s) '%s' -> '%s' ...", basePath_, destBasePath_ );

  int imageOptions = ImageBuilder::MIPMAPS_BIT;
  if( useS3TC ) {
    imageOptions |= ImageBuilder::COMPRESSION_BIT;
  }

  String destBasePath      = destBasePath_;
  String basePath          = basePath_;
  String diffuseBasePath   = basePath;
  String masksBasePath     = basePath + "_m";
  String specular1BasePath = basePath + "_s";
  String specular2BasePath = basePath + "_spec";
  String emission1BasePath = basePath + "_g";
  String emission2BasePath = basePath + "_glow";
  String emission3BasePath = basePath + ".blend";
  String normals1BasePath  = basePath + "_n";
  String normals2BasePath  = basePath + "_nm";
  String normals3BasePath  = basePath + "_normal";
  String normals4BasePath  = basePath + "_local";

  File diffuse, masks, specular, emission, normals;

  for( int i = 0; i < aLength( IMAGE_EXTENSIONS ); ++i ) {
    if( diffuse.path().isEmpty() || diffuse.type() == File::MISSING ) {
      diffuse = File( diffuseBasePath + IMAGE_EXTENSIONS[i] );
    }

    if( allLayers ) {
      if( masks.path().isEmpty() || masks.type() == File::MISSING ) {
        masks = File( masksBasePath + IMAGE_EXTENSIONS[i] );
      }

      if( specular.path().isEmpty() || specular.type() == File::MISSING ) {
        specular = File( specular1BasePath + IMAGE_EXTENSIONS[i] );
      }
      if( specular.type() == File::MISSING ) {
        specular = File( specular2BasePath + IMAGE_EXTENSIONS[i] );
      }

      if( emission.path().isEmpty() || emission.type() == File::MISSING ) {
        emission = File( emission1BasePath + IMAGE_EXTENSIONS[i] );
      }
      if( emission.type() == File::MISSING ) {
        emission = File( emission2BasePath + IMAGE_EXTENSIONS[i] );
      }
      if( emission.type() == File::MISSING ) {
        emission = File( emission3BasePath + IMAGE_EXTENSIONS[i] );
      }

      if( normals.path().isEmpty() || normals.type() == File::MISSING ) {
        normals = File( normals1BasePath + IMAGE_EXTENSIONS[i] );
      }
      if( normals.type() == File::MISSING ) {
        normals = File( normals2BasePath + IMAGE_EXTENSIONS[i] );
      }
      if( normals.type() == File::MISSING ) {
        normals = File( normals3BasePath + IMAGE_EXTENSIONS[i] );
      }
      if( normals.type() == File::MISSING ) {
        normals = File( normals4BasePath + IMAGE_EXTENSIONS[i] );
      }
    }
  }

  if( diffuse.type() != File::MISSING ) {
    ImageBuilder::convertToDDS( diffuse, imageOptions, destBasePath + ".dds" );
  }
  else {
    OZ_ERROR( "Missing texture '%s' (.png, .jpeg, .jpg and .tga checked)", basePath.cstr() );
  }

  if( masks.type() != File::MISSING ) {
    ImageBuilder::convertToDDS( masks, imageOptions, destBasePath + "_m.dds" );
  }
  else {
    ImageData specularImage;
    ImageData emissionImage;

    if( specular.type() != File::MISSING ) {
      specularImage = ImageBuilder::loadImage( specular );
      specularImage.flags = 0;
    }
    if( emission.type() != File::MISSING ) {
      emissionImage = ImageBuilder::loadImage( emission );
      emissionImage.flags = 0;
    }

    if( specularImage.isEmpty() && emissionImage.isEmpty() ) {
      // Drop through.
    }
    else if( emissionImage.isEmpty() ) {
      for( int i = 0; i < specularImage.width * specularImage.height; ++i ) {
        char& r = specularImage.pixels[i*4 + 0];
        char& g = specularImage.pixels[i*4 + 1];
        char& b = specularImage.pixels[i*4 + 2];
        char& a = specularImage.pixels[i*4 + 3];

        char s = char( ( r + g + b ) / 3 );

        r = s;
        g = 0;
        b = 0;
        a = 0;
      }

      ImageBuilder::createDDS( &specularImage, 1, imageOptions, destBasePath + "_m.dds" );
    }
    else if( specularImage.isEmpty() ) {
      for( int i = 0; i < emissionImage.width * emissionImage.height; ++i ) {
        char& r = emissionImage.pixels[i*4 + 0];
        char& g = emissionImage.pixels[i*4 + 1];
        char& b = emissionImage.pixels[i*4 + 2];
        char& a = emissionImage.pixels[i*4 + 3];

        char e = char( ( r + g + b ) / 3 );

        r = 0;
        g = e;
        b = 0;
        a = 0;
      }

      ImageBuilder::createDDS( &emissionImage, 1, imageOptions, destBasePath + "_m.dds" );
    }
    else {
      if( specularImage.width != emissionImage.width ||
          specularImage.height != emissionImage.height )
      {
        OZ_ERROR( "Specular and emission texture masks must have the same size." );
      }

      for( int i = 0; i < specularImage.width * specularImage.height; ++i ) {
        char& r = specularImage.pixels[i*4 + 0];
        char& g = specularImage.pixels[i*4 + 1];
        char& b = specularImage.pixels[i*4 + 2];
        char& a = specularImage.pixels[i*4 + 3];

        char& er = emissionImage.pixels[i*4 + 0];
        char& eg = emissionImage.pixels[i*4 + 1];
        char& eb = emissionImage.pixels[i*4 + 2];

        char s = char( ( r + g + b ) / 3 );
        char e = char( ( er + eg + eb ) / 3 );

        r = s;
        g = e;
        b = 0;
        a = 0;
      }

      ImageBuilder::createDDS( &specularImage, 1, imageOptions, destBasePath + "_m.dds" );
    }
  }

  if( normals.type() != File::MISSING ) {
    ImageBuilder::convertToDDS( normals, imageOptions, destBasePath + "_n.dds" );
  }

  Log::printEnd( " OK" );
}

void Context::init()
{}

void Context::destroy()
{
  usedTextures.clear();
  usedTextures.deallocate();
  usedSounds.clear();
  usedSounds.deallocate();
  usedModels.clear();
  usedModels.deallocate();
}

Context context;

}
}
