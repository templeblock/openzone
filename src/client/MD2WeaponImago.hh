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
 * @file client/MD2WeaponImago.hh
 */

#pragma once

#include <client/Imago.hh>
#include <client/Model.hh>

namespace oz
{
namespace client
{

class MD2WeaponImago : public Imago
{
private:

  Model* model;

  explicit MD2WeaponImago( const Object* obj ) :
    Imago( obj )
  {}

  ~MD2WeaponImago() override;

public:

  static Pool<MD2WeaponImago, 256> pool;

  static Imago* create( const Object* obj );

  void draw( const Imago* parent ) override;

  OZ_STATIC_POOL_ALLOC( pool )
};

}
}
