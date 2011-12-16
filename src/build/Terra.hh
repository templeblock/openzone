/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2011 Davorin Učakar
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
 * @file build/Terra.hh
 */

#pragma once

#include "matrix/Terra.hh"

#include "build/common.hh"

namespace oz
{
namespace build
{

class Terra
{
  private:

    matrix::Terra::Quad quads[matrix::Terra::VERTS][matrix::Terra::VERTS];

    String name;
    String waterTexture;
    String detailTexture;
    String mapTexture;

    void load();
    void saveMatrix();
    void saveClient();

    explicit Terra( const char* name );

  public:

    static void build( const char* name );

};

}
}
