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
 * @file stable.hh
 *
 * Precompiled header.
 *
 * It should be directly included by all '.cc' source files in the project.
 */

#include "common/common.hh"

// We want to use C++ wrapped C headers, not vanilla ones that are included via SDL.
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define SDL_NO_COMPAT
#include <SDL/SDL.h>
