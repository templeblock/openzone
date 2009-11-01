/*
 *  Sky.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 */

#include "precompiled.h"

#include "Sky.h"

#include "matrix/Timer.h"
#include "matrix/World.h"
#include "Context.h"
#include "Camera.h"

namespace oz
{
namespace client
{

  Sky sky;

  const float Sky::DAY_BIAS = 0.4f;

  const float Sky::AMBIENT_COEF = 0.40f;

  const float Sky::RED_COEF   = +0.05f;
  const float Sky::GREEN_COEF = -0.05f;
  const float Sky::BLUE_COEF  = -0.10f;

  const float Sky::DAY_COLOR[]   = { 0.45f, 0.60f, 0.90f, 1.0f };
  const float Sky::NIGHT_COLOR[] = { 0.02f, 0.02f, 0.05f, 1.0f };
  const float Sky::WATER_COLOR[] = { 0.00f, 0.05f, 0.25f, 1.0f };
  const float Sky::STAR_COLOR[]  = { 0.80f, 0.80f, 0.80f, 1.0f };

  void Sky::load()
  {
    float heading = Math::rad( world.sky.heading );

    axis = Vec3( -Math::sin( heading ), Math::cos( heading ), 0.0f );
    originalLightDir = Vec3( -Math::cos( heading ), -Math::sin( heading ), 0.0f );

    Quat *tempStars = new Quat[MAX_STARS];
    for( int i = 0; i < MAX_STARS; i++ ) {
      float length;
      do {
        tempStars[i].x = 20.0f * Math::frand() - 10.0f;
        tempStars[i].y = 20.0f * Math::frand() - 10.0f;
        tempStars[i].z = 20.0f * Math::frand() - 10.0f;
        tempStars[i].w = Math::atan2( tempStars[i].z, tempStars[i].x );
        length = static_cast<Vec3>( tempStars[i] ).sqL();
      }
      while( Math::isNaN( length ) || length < 1.0f || length > 100.0f );
    }

    // sort stars
    aSort( tempStars, MAX_STARS );

    for( int i = 0; i < MAX_STARS; i++ ) {
      stars[i] = Vec3( tempStars[i] );
    }

    delete[] tempStars;

    sunTexId  = context.loadTexture( "sky/simplesun.png", false, GL_LINEAR, GL_LINEAR );
    moonTexId = context.loadTexture( "sky/moon18.png", false, GL_LINEAR, GL_LINEAR );

    sunList  = context.genLists( 2 );
    moonList = sunList + 1;

    glNewList( sunList, GL_COMPILE );

    glBindTexture( GL_TEXTURE_2D, sunTexId );
    glBegin( GL_QUADS );
      glTexCoord2f( 0.0f, 1.0f );
      glVertex3f( 0.0f, -1.0f, +1.0f );
      glTexCoord2f( 1.0f, 1.0f );
      glVertex3f( 0.0f, -1.0f, -1.0f );
      glTexCoord2f( 1.0f, 0.0f );
      glVertex3f( 0.0f, +1.0f, -1.0f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( 0.0f, +1.0f, +1.0f );
    glEnd();

    glEndList();

    glNewList( moonList, GL_COMPILE );

    glColor3f( 1.0f, 1.0f, 1.0f );
    glBindTexture( GL_TEXTURE_2D, moonTexId );
    glBegin( GL_QUADS );
      glTexCoord2f( 0.0f, 1.0f );
      glVertex3f( 0.0f, -1.0f, -1.0f );
      glTexCoord2f( 1.0f, 1.0f );
      glVertex3f( 0.0f, -1.0f, +1.0f );
      glTexCoord2f( 1.0f, 0.0f );
      glVertex3f( 0.0f, +1.0f, +1.0f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( 0.0f, +1.0f, -1.0f );
    glEnd();

    glEndList();

    update();
  }

  void Sky::unload()
  {
    context.freeLists( sunList );
    context.freeTexture( sunTexId );
    context.freeTexture( moonTexId );
  }

  void Sky::update()
  {
    angle = 2.0f * Math::PI * ( world.sky.time / world.sky.period );
    Vec3  dir = Quat::rotAxis( axis, angle ).rotate( originalLightDir );
    ratio = bound( dir.z + DAY_BIAS, 0.0f, 1.0f );
    ratio_1 = 1.0f - ratio;
    float ratioDiff = ( 1.0f - abs( ratio - ratio_1 ) );

    skyColor[0] = ratio * DAY_COLOR[0] + ratio_1 * NIGHT_COLOR[0] + RED_COEF * ratioDiff;
    skyColor[1] = ratio * DAY_COLOR[1] + ratio_1 * NIGHT_COLOR[1] + GREEN_COEF * ratioDiff;
    skyColor[2] = ratio * DAY_COLOR[2] + ratio_1 * NIGHT_COLOR[2] + BLUE_COEF * ratioDiff;
    skyColor[3] = 1.0f;

    waterColor[0] = ratio * WATER_COLOR[0] + ratio_1 * NIGHT_COLOR[0];
    waterColor[1] = ratio * WATER_COLOR[1] + ratio_1 * NIGHT_COLOR[1];
    waterColor[2] = ratio * WATER_COLOR[2] + ratio_1 * NIGHT_COLOR[2];
    waterColor[3] = ratio * WATER_COLOR[3] + ratio_1 * NIGHT_COLOR[3];

    lightDir[0] = dir.x;
    lightDir[1] = dir.y;
    lightDir[2] = dir.z;
    lightDir[3] = 0.0f;

    diffuseColor[0] = ratio + RED_COEF * ratioDiff;
    diffuseColor[1] = ratio + GREEN_COEF * ratioDiff;
    diffuseColor[2] = ratio + BLUE_COEF * ratioDiff;
    diffuseColor[3] = 1.0f;

    ambientColor[0] = AMBIENT_COEF * diffuseColor[0];
    ambientColor[1] = AMBIENT_COEF * diffuseColor[1];
    ambientColor[2] = AMBIENT_COEF * diffuseColor[2];
    ambientColor[3] = 1.0f;
  }

  void Sky::draw()
  {
    float color[3] = {
      ratio * DAY_COLOR[0] + ratio_1 * STAR_COLOR[0],
      ratio * DAY_COLOR[1] + ratio_1 * STAR_COLOR[0],
      ratio * DAY_COLOR[2] + ratio_1 * STAR_COLOR[0]
    };

    // we need the transformation matrix for occlusion of stars below horizon
    Mat44 transf = Mat44::rotZ( Math::rad( world.sky.heading ) ) * Mat44::rotY( angle );

    glDisable( GL_BLEND );
    glColor3fv( color );

    glPushMatrix();
    glMultMatrixf( transf );
    transf.trans();

    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 0, stars );

    const Vec3 &tz = static_cast<Vec3>( transf.z );
    int start, end;
    if( tz * stars[0] > 0.0f ) {
      for( end = 1; end < MAX_STARS && tz * stars[end] > 0.0f; end++ );
      for( start = end + 1; start < MAX_STARS && tz * stars[start] <= 0.0f; start++ );

      glDrawArrays( GL_POINTS, 0, end );
      glDrawArrays( GL_POINTS, start, MAX_STARS - start );
    }
    else {
      for( start = 1; start < MAX_STARS && tz * stars[start] <= 0.0f; start++ );
      for( end = start; end < MAX_STARS && tz * stars[end] > 0.0f; end++ );

      glDrawArrays( GL_POINTS, start, end - start );
    }

    glDisableClientState( GL_VERTEX_ARRAY );

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );

    glTranslatef( -15.0f, 0.0f, 0.0f );
    glColor3f( 2.0f * diffuseColor[0] + ambientColor[0],
               diffuseColor[1] + ambientColor[1],
               diffuseColor[2] + ambientColor[2] );
    glCallList( sunList );

    glTranslatef( 30.0f, 0.0f, 0.0f );
    glCallList( moonList );

    glPopMatrix();

    assert( glGetError() == GL_NO_ERROR );
  }

}
}
