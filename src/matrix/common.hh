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
 * @file matrix/common.hh
 *
 * Common matrix includes and definitions for matrix.
 */

#pragma once

#include "stable.hh"

namespace oz
{
namespace matrix
{

enum Heading
{
  NORTH          = 0x00,
  WEST           = 0x01,
  SOUTH          = 0x02,
  EAST           = 0x03,
  WEST_EAST_MASK = 0x01
};

}
}