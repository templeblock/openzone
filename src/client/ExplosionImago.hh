/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright (C) 2002-2011  Davorin Učakar
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
 * @file client/ExplosionImago.hh
 */

#pragma once

#include "client/Imago.hh"
#include "client/SMM.hh"

namespace oz
{
namespace client
{

class ExplosionImago : public Imago
{
  protected:

    static int modelId;

    SMM* smm;
    uint startMillis;

    explicit ExplosionImago( const Object* obj ) : Imago( obj )
    {}

    virtual ~ExplosionImago();

  public:

    static Pool<ExplosionImago, 64> pool;

    static Imago* create( const Object* obj );

    virtual void draw( const Imago* parent, int mask );

  OZ_STATIC_POOL_ALLOC( pool )

};

}
}
