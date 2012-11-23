/*
 * ozCore - OpenZone Core Library.
 *
 * Copyright © 2002-2012 Davorin Učakar
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file ozCore/Plane.hh
 *
 * Plane class.
 */

#pragma once

#include "Point.hh"

namespace oz
{

/**
 * 3D plane.
 */
class Plane
{
  public:

    Vec3  n; ///< Normal.
    float d; ///< Distance from origin.

  public:

    /**
     * Create an uninitialised instance.
     */
    OZ_ALWAYS_INLINE
    explicit Plane() = default;

    /**
     * Create form a pair of normal and distance from the origin.
     */
    OZ_ALWAYS_INLINE
    explicit Plane( const Vec3& n_, float d_ ) :
      n( n_ ), d( d_ )
    {}

    /**
     * Create from an array of 4 floats.
     */
    OZ_ALWAYS_INLINE
    explicit Plane( const float* p ) :
      n( p[0], p[1], p[2] ), d( p[3] )
    {}

    /**
     * Create with the given member values.
     */
    OZ_ALWAYS_INLINE
    explicit Plane( float nx, float ny, float nz, float d_ ) :
      n( nx, ny, nz ), d( d_ )
    {}

    /**
     * Equality.
     */
    OZ_ALWAYS_INLINE
    bool operator == ( const Plane& p ) const
    {
      return n.x == p.n.x && n.y == p.n.y && n.z == p.n.z && d == p.d;
    }

    /**
     * Inequality.
     */
    OZ_ALWAYS_INLINE
    bool operator != ( const Plane& p ) const
    {
      return n.x != p.n.x || n.y != p.n.y || n.z != p.n.z || d != p.d;
    }

    /**
     * Projection of a vector to the plane's normal.
     */
    OZ_ALWAYS_INLINE
    friend scalar operator * ( const Vec3& v, const Plane& plane )
    {
#ifdef OZ_SIMD_MATH
      return vDot( v.f4, plane.n.f4 );
#else
      return v.x*plane.n.x + v.y*plane.n.y + v.z*plane.n.z;
#endif
    }

    /**
     * Distance between a point and the plane.
     */
    OZ_ALWAYS_INLINE
    friend scalar operator * ( const Point& p, const Plane& plane )
    {
#ifdef OZ_SIMD_MATH
      return vDot( p.f4, plane.n.f4 ) - vFill( plane.d, plane.d, plane.d, plane.d );
#else
      return p.x*plane.n.x + p.y*plane.n.y + p.z*plane.n.z - plane.d;
#endif
    }

};

}