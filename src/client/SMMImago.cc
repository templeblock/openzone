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
 * @file client/SMMImago.cc
 */

#include <client/SMMImago.hh>

#include <client/Context.hh>

namespace oz
{
namespace client
{

Pool<SMMImago, 4096> SMMImago::pool;

Imago* SMMImago::create( const Object* obj )
{
  SMMImago* imago = new SMMImago( obj );

  imago->model = context.requestModel( obj->clazz->imagoModel );

  return imago;
}

SMMImago::~SMMImago()
{
  context.releaseModel( clazz->imagoModel );
}

void SMMImago::draw( const Imago* )
{
  if( !model->isLoaded() ) {
    return;
  }

  tf.model = Mat4::translation( obj->p - Point::ORIGIN );
  tf.model.rotateZ( float( obj->flags & Object::HEADING_MASK ) * Math::TAU / 4.0f );

  model->schedule( 0, Model::SCENE_QUEUE );
}

}
}
