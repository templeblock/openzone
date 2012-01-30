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
 * @file client/ui/GalileoFrame.cc
 */

#include "stable.hh"

#include "client/ui/GalileoFrame.hh"

#include "client/Camera.hh"
#include "client/Context.hh"
#include "client/QuestList.hh"
#include "client/OpenGL.hh"

#include "client/ui/QuestFrame.hh"

namespace oz
{
namespace client
{
namespace ui
{

uint GalileoFrame::loadTexture( const char* path ) const
{
  PhysFile file( path );
  if( !file.map() ) {
    throw Exception( "Failed reading galileo texture '%s'", path );
  }

  InputStream istream = file.inputStream();
  uint texId = context.readTexture( &istream, path );

  file.unmap();
  return texId;
}

bool GalileoFrame::onMouseEvent()
{
  return isVisible ? Frame::onMouseEvent() : false;
}

void GalileoFrame::onDraw()
{
  if( camera.state == Camera::BOT && camera.botObj != null ) {
    if( camera.botObj->clazz->attributes & ObjectClass::GALILEO_BIT ) {
      goto galileoEnabled;
    }
    else {
      if( camera.botObj->parent != -1 ) {
        const Object* veh = orbis.objects[camera.botObj->parent];

        if( veh != null && ( veh->clazz->attributes & ObjectClass::GALILEO_BIT ) ) {
          goto galileoEnabled;
        }
      }

      foreach( i, camera.botObj->items.citer() ) {
        const Object* item = orbis.objects[*i];

        if( item != null && ( item->clazz->attributes & ObjectClass::GALILEO_BIT ) ) {
          goto galileoEnabled;
        }
      }
    }

    isVisible = false;
    return;
  }

galileoEnabled:;

  isVisible = true;

  if( mapTexId == 0 ) {
    mapTexId = loadTexture( "terra/" + library.terrae[orbis.terra.id].name + ".ozcTex" );
  }

  float pX;
  float pY;
  float h;

  if( camera.state == Camera::BOT && camera.botObj != null ) {
    pX = camera.botObj->p.x;
    pY = camera.botObj->p.y;
    h  = camera.botObj->h;
  }
  else {
    pX = camera.p.x;
    pY = camera.p.y;
    h  = camera.h;
  }

  glUniform4f( param.oz_Colour, 1.0f, 1.0f, 1.0f, 0.6f );
  glBindTexture( GL_TEXTURE_2D, mapTexId );
  fill( 0, 0, width, height );

  glUniform4f( param.oz_Colour, 1.0f, 1.0f, 1.0f, 1.0f );

  float oX      = float( x );
  float oY      = float( y );
  float fWidth  = float( width );
  float fHeight = float( height );

  if( questFrame != null && questFrame->currentQuest != -1 ) {
    const Quest& quest = questList.quests[questFrame->currentQuest];

    glBindTexture( GL_TEXTURE_2D, markerTexId );

    float mapX = oX + ( Orbis::DIM + quest.place.x ) / ( 2.0f*Orbis::DIM ) * fWidth;
    float mapY = oY + ( Orbis::DIM + quest.place.y ) / ( 2.0f*Orbis::DIM ) * fHeight;

    tf.model = Mat44::translation( Vec3( mapX, mapY, 0.0f ) );
    tf.model.scale( Vec3( 16.0f, 16.0f, 0.0f ) );
    tf.model.translate( Vec3( -0.5f, -0.5f, 0.0f ) );
    tf.apply();

    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
  }

  glBindTexture( GL_TEXTURE_2D, arrowTexId );

  float mapX = oX + ( Orbis::DIM + pX ) / ( 2.0f*Orbis::DIM ) * fWidth;
  float mapY = oY + ( Orbis::DIM + pY ) / ( 2.0f*Orbis::DIM ) * fHeight;

  tf.model = Mat44::translation( Vec3( mapX, mapY, 0.0f ) );
  tf.model.rotateZ( h );
  tf.model.scale( Vec3( 10.0f, 10.0f, 0.0f ) );
  tf.model.translate( Vec3( -0.5f, -0.5f, 0.0f ) );
  tf.apply();

  glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

  glBindTexture( GL_TEXTURE_2D, 0 );
}

GalileoFrame::GalileoFrame( const QuestFrame* questFrame_ ) :
  Frame( 8, -8, 240, 232 - Font::INFOS[Font::LARGE].height, "" ),
  questFrame( questFrame_ ), mapTexId( 0 ), arrowTexId( 0 ), markerTexId( 0 ), isVisible( true )

{
  flags = PINNED_BIT;

  arrowTexId = loadTexture( "ui/galileo/arrow.ozcTex" );
  markerTexId = loadTexture( "ui/galileo/marker.ozcTex" );
}

GalileoFrame::~GalileoFrame()
{
  if( mapTexId != 0 ) {
    glDeleteTextures( 1, &mapTexId );
  }
  if( arrowTexId != 0 ) {
    glDeleteTextures( 1, &arrowTexId );
  }
  if( markerTexId != 0 ) {
    glDeleteTextures( 1, &markerTexId );
  }
}

}
}
}