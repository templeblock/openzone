/*
 *  ExplosionModel.cpp
 *
 *  [description]
 *
 *  Copyright (C) 2002-2009, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3.0. See COPYING for details.
 */

#include "precompiled.h"

#include "ExplosionModel.h"

#include "matrix/Timer.h"
#include "Context.h"

#include <GL/glu.h>

namespace oz
{
namespace client
{

  Model *ExplosionModel::create( const Object *obj )
  {
    ExplosionModel *model = new ExplosionModel();

    model->obj = obj;
    model->texId = context.requestTexture( translator.textureIndex( "explosion" ) );
    model->quadric = gluNewQuadric();
    model->startMillis = timer.millis;

    gluQuadricTexture( model->quadric, GL_TRUE );

    return model;
  }

  ExplosionModel::~ExplosionModel()
  {
    gluDeleteQuadric( quadric );
    context.releaseTexture( translator.textureIndex( "explosion" ) );
  }

  void ExplosionModel::draw()
  {
    float millis = timer.millis - startMillis;
    float radius = millis * obj->dim.z * 0.006f;
    float alpha  = 1.0f - 0.001f * millis;
    float color[] = { 1.0f, 1.0f, 1.0f, alpha*alpha };

    glEnable( GL_BLEND );
    glDisable( GL_CULL_FACE );
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color );
    glBindTexture( GL_TEXTURE_2D, texId );
    gluSphere( quadric, radius, 32, 32 );
    glEnable( GL_CULL_FACE );
    glDisable( GL_BLEND );
  }

}
}