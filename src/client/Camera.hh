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
 * @file client/Camera.hh
 */

#pragma once

#include "client/StrategicProxy.hh"
#include "client/BotProxy.hh"

namespace oz
{
namespace client
{

class Camera
{
  public:

    static const float ROT_LIMIT;
    static const float MIN_DISTANCE;
    static const float SMOOTHING_COEF;

    enum State
    {
      NONE,
      STRATEGIC,
      BOT
    };

    float mouseXSens;
    float mouseYSens;
    float keyXSens;
    float keyYSens;

    static StrategicProxy strategicProxy;
    static BotProxy       botProxy;

    Proxy* proxy;

  public:

    Point3 p;
    Point3 newP;
    Point3 oldP;

    // relative to the object the camera is bound to
    float  h;
    float  v;
    float  w;
    float  mag;

    // camera rotation change (from input)
    float  relH;
    float  relV;

    // global rotation quaternion, matrix and it's inverse
    Quat   rot;
    Mat44  rotMat;
    Mat44  rotTMat;

    Vec3   right;
    Vec3   up;
    Vec3   at;

    int    object;
    const Object* objectObj;

    int    entity;
    const Entity* entityObj;

    int    bot;
    Bot*   botObj;

    int    width;
    int    height;
    int    centreX;
    int    centreY;

    float  coeff;
    float  aspect;
    float  vertPlane;
    float  horizPlane;
    float  maxDist;

    bool   isExternal;
    bool   allowReincarnation;
    bool   nightVision;

    State  state;
    State  newState;

    void setState( State state )
    {
      newState = state;
    }

    void setTaggedObj( const Object* obj )
    {
      object    = obj == null ? -1 : obj->index;
      objectObj = obj;
    }

    void setTaggedEnt( const Entity* ent )
    {
      entityObj = ent;

      if( ent == null ) {
        entity = -1;
      }
      else {
        int strIndex = ent->str->index;
        int entIndex = int( ent - ent->str->entities );

        entity = strIndex * Struct::MAX_ENTITIES + entIndex;
      }
    }

    void setBot( Bot* botObj_ )
    {
      if( botObj != null ) {
        botObj->state &= ~Bot::PLAYER_BIT;
      }

      if( botObj_ == null ) {
        bot    = -1;
        botObj = null;
      }
      else {
        bot    = botObj_->index;
        botObj = botObj_;

        botObj_->state |= Bot::PLAYER_BIT;
      }

      hard_assert( botObj == null || ( botObj->flags & Object::BOT_BIT ) );
    }

    void move( const Point3& pos )
    {
      p    = Math::mix( oldP, pos, SMOOTHING_COEF );
      newP = pos;
      oldP = p;
    }

    void warp( const Point3& pos )
    {
      oldP = pos;
      newP = pos;
      p    = pos;
    }

    void warpMoveZ( const Point3& pos )
    {
      p.x  = pos.x;
      p.y  = pos.y;
      p.z  = Math::mix( oldP.z, pos.z, SMOOTHING_COEF );
      newP = pos;
      oldP = p;
    }

    void updateReferences();

    /**
     * Re-calculate quaternions, transformation matrices and <tt>at</tt>, <tt>up</tt>, <tt>left</tt>
     * vectors.
     */
    void align();

    /**
     * Process input.
     *
     * This pass is performed before matrix update. It should e.g. update player's bot orientation
     * and actions.
     */
    void prepare();

    /**
     * Update camera after matrix simulation step.
     *
     * This pass should align camera to match bot position and orientation etc.
     */
    void update();

    void reset();

    void read( InputStream* istream );
    void write( BufferStream* ostream ) const;

    void init( int screenWidth, int screenHeight );

};

extern Camera camera;

}
}
