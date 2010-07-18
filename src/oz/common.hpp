/*
 *  common.hpp
 *
 *  Common macros, types and templates
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

#pragma once

/**
 *  \file base/common.hpp
 *
 *  \brief Common types and templates
 *
 *  You may add <code>null</code>, <code>foreach</code> and the types to your
 *  <code>~/.kde/share/apps/katepart/syntax/cpp.xml</code> or global file
 *  <code>$KDEDIR/share/apps/katepart/syntax/cpp.xml</code> to look like reserved words in
 *  Katepart (Kate/KWrite/KDevelop).
 */

namespace oz
{

  //***********************************
  //*          BASIC MACROS           *
  //***********************************

  /**
   * \def null
   * It is equivalent to nullptr/NULL but it looks prettier.
   */
#ifdef __GNUG__
# define null __null
#else
# define null nullptr
#endif

  /**
   * \def S
   * "Stringify" the given identifier/type/reserved word/...
   */
# define S( s ) #s

  /**
   * \def soft_assert
   * Resembles output of assert macro (on GNU/Linux) but does not abort.
   */
#ifndef OZ_ENABLE_ASSERT

# define assert( cond ) \
  static_cast<void>( 0 )

# define soft_assert( cond ) \
  static_cast<void>( 0 )

#else

# define assert( cond ) \
  ( ( cond ) ? \
      static_cast<void>( 0 ) : \
      oz::_assert( #cond, __FILE__, __LINE__, __PRETTY_FUNCTION__ ) )

  void _assert( const char* message, const char* file, int line, const char* function );

# define soft_assert( cond ) \
  ( ( cond ) ? \
      static_cast<void>( 0 ) : \
      oz::_softAssert( #cond, __FILE__, __LINE__, __PRETTY_FUNCTION__ ) )

  void _softAssert( const char* message, const char* file, int line, const char* function );

#endif

  //***********************************
  //*             TYPES               *
  //***********************************

  /**
   * Unit type
   */
  # define nil nil
  struct nil
  {
    /**
     * Always return true as all instances of this type are the same.
     * @param
     * @return
     */
    bool operator == ( const nil& ) const
    {
      return true;
    }

    /**
     * Always return false as all instances of this type are the same.
     * @param
     * @return
     */
    bool operator != ( const nil& ) const
    {
      return false;
    }
  };

  /**
   * signed byte
   * It should be used where char must be signed (otherwise char may be either signed or unsigned
   * depending on the platform).
   */
# define byte byte
  typedef signed   char  byte;

  /**
   * unsigned byte
   * It should be used where char must be unsigned (otherwise char may be either signed or unsigned
   * depending on the platform).
   */
# define ubyte ubyte
  typedef unsigned char  ubyte;

  /**
   * unsigned short integer
   */
# define ushort ushort
  typedef unsigned short ushort;

  /**
   * unsigned integer
   */
# define uint uint
  typedef unsigned int   uint;

  /**
   * unsigned long integer
   */
# define ulong ulong
  typedef unsigned long  ulong;

  //***********************************
  //*        BASIC ALGORITHMS         *
  //***********************************

  /**
   * Swap values of a and b with move semantics.
   * @param a reference to the first variable
   * @param b reference to the second variable
   */
  template <typename Value>
  inline void swap( Value& a, Value& b )
  {
    Value t( a );
    a = b;
    b = t;
  }

  /**
   * Minimum.
   * @param a
   * @param b
   * @return a if a < b, b otherwise
   */
  template <typename Value>
  inline const Value& min( const Value& a, const Value& b )
  {
    return a < b ? a : b;
  }

  /**
   * Minimum
   * Non-const version, can be used as lvalue.
   * @param a
   * @param b
   * @return a if a < b, b otherwise
   */
  template <typename Value>
  inline Value& min( Value& a, Value& b )
  {
    return a < b ? a : b;
  }

  /**
   * Maximum.
   * @param a
   * @param b
   * @return a if a > b, b otherwise
   */
  template <typename Value>
  inline const Value& max( const Value& a, const Value& b )
  {
    return b < a ? a : b;
  }

  /**
   * Maximum
   * Non-const version, can be used as lvalue.
   * @param a
   * @param b
   * @return a if a > b, b otherwise
   */
  template <typename Value>
  inline Value& max( Value& a, Value& b )
  {
    return b < a ? a : b;
  }

  /**
   * Bound c between a and b. Equals to max( a, min( b, c ) ).
   * @param c
   * @param a
   * @param b
   * @return c, if a <= c <= b, respective boundary otherwise
   */
  template <typename Value>
  inline const Value& bound( const Value& c, const Value& a, const Value& b )
  {
    assert( a <= b );

    return c < a ? a : ( b < c ? b : c );
  }

  /**
   * Bound c between a and b. Equals to max( a, min( b, c ) ).
   * Non-const version, can be used as lvalue.
   * @param c
   * @param a
   * @param b
   * @return c, if a <= c <= b, respective boundary otherwise
   */
  template <typename Value>
  inline Value& bound( Value& c, Value& a, Value& b )
  {
    assert( a <= b );

    return c < a ? a : ( b < c ? b : c );
  }

}
