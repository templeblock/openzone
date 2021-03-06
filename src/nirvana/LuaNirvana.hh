/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2014 Davorin Učakar
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
 * @file nirvana/LuaNirvana.hh
 *
 * Lua scripting engine for Nirvana
 */

#pragma once

#include <nirvana/Mind.hh>

namespace oz
{

class LuaNirvana : public Lua
{
public:

  void mindCall(const char* functionName, Mind* mind, Bot* self);

  void registerMind(int botIndex);
  void unregisterMind(int botIndex);

  void read(Stream* is);
  void write(Stream* os);

  void init();
  void destroy();

};

extern LuaNirvana luaNirvana;

}
