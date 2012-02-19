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
 * @file common/common.hh
 *
 * @defgroup common Common layer
 */

#pragma once

#include "oz/oz.hh"
#include "oz/windefs.h"

#include "configuration.hh"

#include "common/Span.hh"
#include "common/Timer.hh"
#include "common/Lingua.hh"

// We want to use C++ wrapped C headers, not vanilla ones that are included via SDL.
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Doxygen should skip those macros when generating documentation.
#ifndef OZ_DOXYGEN

# ifdef _WIN32
#  include <windows.h>
// Fix M$ crap from Windows headers.
#  undef ERROR
#  undef PLANES
#  undef near
#  undef far
# endif

# define SDL_NO_COMPAT
# include <SDL.h>

#endif

/**
 * Just a handy macro for declaring %Lua API.
 *
 * @ingroup common
 */
#define OZ_LUA_API( func ) \
  static int func( lua_State* )

struct lua_State;

namespace oz
{

/**
 * Margin for collision detection.
 *
 * 8 * max_rounding_error per kilometre should do. Hence max_world_coord * 4 * Math::FLOAT_EPS,
 * since max_rounding_error = Math::FLOAT_EPS / 2.
 *
 * @ingroup common
 */
const float EPSILON = 2048.0f * 4.0f * Math::FLOAT_EPS;

/**
 * Lua C API.
 *
 * @ingroup common
 */
typedef int LuaAPI( lua_State* );

}
