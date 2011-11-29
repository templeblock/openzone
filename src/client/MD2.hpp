/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
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
 *
 * Davorin Učakar
 * <davorin.ucakar@gmail.com>
 */

/**
 * @file client/MD2.hpp
 */

#pragma once

#include "matrix/Bot.hpp"

#include "client/Mesh.hpp"

namespace oz
{
namespace client
{

class MD2
{
  public:

    static const int MAX_VERTS = 2048;

    enum Anim
    {
      ANIM_STAND,
      ANIM_RUN,
      ANIM_ATTACK,
      ANIM_PAIN_A,
      ANIM_PAIN_B,
      ANIM_PAIN_C,
      ANIM_JUMP,
      ANIM_FLIP,
      ANIM_SALUTE,
      ANIM_FALLBACK,
      ANIM_WAVE,
      ANIM_POINT,
      ANIM_CROUCH_STAND,
      ANIM_CROUCH_WALK,
      ANIM_CROUCH_ATTACK,
      ANIM_CROUCH_PAIN,
      ANIM_CROUCH_DEATH,
      ANIM_DEATH_FALLBACK,
      ANIM_DEATH_FALLFORWARD,
      ANIM_DEATH_FALLBACKSLOW,
      ANIM_MAX
    };

    struct AnimInfo
    {
      int   firstFrame;
      int   lastFrame;
      bool  repeat;
      float fps;
    };

    struct AnimState
    {
      Anim  type;
      bool  repeat;

      int   firstFrame;
      int   lastFrame;
      int   currFrame;
      int   nextFrame;

      float fps;
      float frameTime;
      float currTime;
    };

    static const AnimInfo ANIM_LIST[];

  private:

    static Vertex animBuffer[MAX_VERTS];

    int     id;

    int     nFrames;
    int     nFrameVertices;
    int     nFramePositions;

    uint    vertexTexId;
    uint    normalTexId;
    int     shaderId;

    Vertex* vertices;
    Vec4*   positions;
    Vec4*   normals;

    Mesh    mesh;

  public:

    Mat44   weaponTransf;
    bool    isLoaded;

    explicit MD2( int id );
    ~MD2();

    void load();

    void advance( AnimState* anim, float dt ) const;

    void drawFrame( int frame ) const;
    void draw( const AnimState* anim ) const;

};

}
}
