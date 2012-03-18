/*
 * liboz - OpenZone core library.
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
 * @file oz/Array.hh
 *
 * Array template class.
 */

#pragma once

#include "arrays.hh"

namespace oz
{

/**
 * Static array.
 *
 * The advantage over C++ arrays is it that is has bounds checking and iterator.
 *
 * @ingroup oz
 */
template <typename Elem, int SIZE>
class Array
{
  static_assert( SIZE > 0, "Array size must be at least 1" );

  public:

    /**
     * %Iterator with constant access to container elements.
     */
    class CIterator : public oz::CIterator<Elem>
    {
      friend class Array;

      private:

        /// Base class type, convenience definition to make code cleaner.
        typedef oz::CIterator<Elem> B;

        /**
         * %Iterator for the given container, points to its first element.
         */
        OZ_ALWAYS_INLINE
        explicit CIterator( const Array& a ) :
          B( a.data, a.data + SIZE )
        {}

      public:

        /**
         * Default constructor, creates an invalid iterator.
         */
        OZ_ALWAYS_INLINE
        CIterator() :
          B( null, null )
        {}

    };

    /**
     * %Iterator with non-constant access to container elements.
     */
    class Iterator : public oz::Iterator<Elem>
    {
      friend class Array;

      private:

        /// Base class type, convenience definition to make code cleaner.
        typedef oz::Iterator<Elem> B;

        /**
         * %Iterator for the given container, points to its first element.
         */
        OZ_ALWAYS_INLINE
        explicit Iterator( Array& a ) :
          B( a.data, a.data + SIZE )
        {}

      public:

        /**
         * Default constructor, creates an invalid iterator.
         */
        OZ_ALWAYS_INLINE
        Iterator() :
          B( null, null )
        {}

    };

  private:

    Elem data[SIZE]; ///< %Array of elements.

  public:

    /**
     * Create uninitialised array.
     */
    Array() = default;

    /**
     * Initialise form a C++ array.
     */
    explicit Array( const Elem* array )
    {
      aCopy<Elem>( data, array, SIZE );
    }

    /**
     * True iff respective elements are equal.
     */
    bool operator == ( const Array& a ) const
    {
      return aEquals<Elem>( data, a.data, SIZE );
    }

    /**
     * False iff respective elements are equal.
     */
    bool operator != ( const Array& a ) const
    {
      return !aEquals<Elem>( data, a.data, SIZE );
    }

    /**
     * %Iterator with constant access, initially points to the first element.
     */
    OZ_ALWAYS_INLINE
    CIterator citer() const
    {
      return CIterator( *this );
    }

    /**
     * %Iterator with non-constant access, initially points to the first element.
     */
    OZ_ALWAYS_INLINE
    Iterator iter()
    {
      return Iterator( *this );
    }

    /**
     * Constant pointer to the first element.
     */
    OZ_ALWAYS_INLINE
    operator const Elem* () const
    {
      return data;
    }

    /**
     * Pointer to the first element.
     */
    OZ_ALWAYS_INLINE
    operator Elem* ()
    {
      return data;
    }

    /**
     * Number of elements.
     */
    OZ_ALWAYS_INLINE
    int length() const
    {
      return SIZE;
    }

    /**
     * Always false since static array cannot have zero size.
     */
    OZ_ALWAYS_INLINE
    bool isEmpty() const
    {
      return false;
    }

    /**
     * Constant reference to the i-th element.
     */
    OZ_ALWAYS_INLINE
    const Elem& operator [] ( int i ) const
    {
      hard_assert( uint( i ) < uint( SIZE ) );

      return data[i];
    }

    /**
     * Reference the i-th element.
     */
    OZ_ALWAYS_INLINE
    Elem& operator [] ( int i )
    {
      hard_assert( uint( i ) < uint( SIZE ) );

      return data[i];
    }

    /**
     * Constant reference to the first element.
     */
    OZ_ALWAYS_INLINE
    const Elem& first() const
    {
      return data[0];
    }

    /**
     * Reference to the first element.
     */
    OZ_ALWAYS_INLINE
    Elem& first()
    {
      return data[0];
    }

    /**
     * Constant reference to the last element.
     */
    OZ_ALWAYS_INLINE
    const Elem& last() const
    {
      return data[SIZE - 1];
    }

    /**
     * Reference to the last element.
     */
    OZ_ALWAYS_INLINE
    Elem& last()
    {
      return data[SIZE - 1];
    }

    /**
     * True iff the given value is found in the array.
     */
    bool contains( const Elem& e ) const
    {
      return aContains<Elem>( data, e, SIZE );
    }

    /**
     * Index of the first occurrence of the value or -1 if not found.
     */
    int index( const Elem& e ) const
    {
      return aIndex<Elem>( data, e, SIZE );
    }

    /**
     * Index of the last occurrence of the value or -1 if not found.
     */
    int lastIndex( const Elem& e ) const
    {
      return aLastIndex<Elem>( data, e, SIZE );
    }

    /**
     * Sort elements with quicksort.
     */
    void sort()
    {
      aSort<Elem>( data, SIZE );
    }

    /**
     * Delete objects referenced by elements and set all elements to <tt>null</tt>.
     */
    void free()
    {
      aFree<Elem>( data, SIZE );
    }

};

}
