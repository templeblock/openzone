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
 * @file oz/HashIndex.hh
 *
 * HashIndex template class.
 */

#pragma once

#include "arrays.hh"
#include "Pool.hh"

namespace oz
{

/**
 * Chaining hashtable implementation with integer key.
 *
 * A prime number is recommended as size unless key distribution is truly "random".
 *
 * Memory is allocated when the first element is added.
 *
 * @ingroup oz
 */
template <typename Value = nullptr_t, int SIZE = 253>
class HashIndex
{
  static_assert( SIZE > 0, "HashIndex size must be at least 1" );

  private:

    /**
     * Internal class for key/value elements.
     */
    struct Elem
    {
      Elem*     next;  ///< Next element in a slot.
      const int key;   ///< Key.
      Value     value; ///< Value.

      /**
       * Initialise a new element.
       */
      template <typename Value_ = Value>
      OZ_ALWAYS_INLINE
      explicit Elem( Elem* next_, int key_, Value_&& value_ ) :
        next( next_ ), key( key_ ), value( static_cast<Value_&&>( value_ ) )
      {}

      OZ_PLACEMENT_POOL_ALLOC( Elem, SIZE )
    };

  public:

    /**
     * %Iterator with constant access to container elements.
     */
    class CIterator : public CIteratorBase<Elem>
    {
      friend class HashIndex;

      private:

        using CIteratorBase<Elem>::elem;

        const Elem* const* data;  ///< Pointer to hashtable slots.
        int                index; ///< Index of the current slot.

        /**
         * %Iterator for the given container, points to its first element.
         */
        explicit CIterator( const HashIndex& t ) :
          CIteratorBase<Elem>( t.data[0] ), data( t.data ), index( 0 )
        {
          while( elem == null && index < SIZE - 1 ) {
            ++index;
            elem = data[index];
          }
        }

      public:

        /**
         * Default constructor, creates an invalid iterator.
         */
        OZ_ALWAYS_INLINE
        CIterator() :
          CIteratorBase<Elem>( null ), data( null ), index( 0 )
        {}

        /**
         * Constant pointer to the current element's key.
         */
        OZ_ALWAYS_INLINE
        operator const int* () const
        {
          return &elem->key;
        }

        /**
         * Constant reference to the current element's key.
         */
        OZ_ALWAYS_INLINE
        const int& operator * () const
        {
          return elem->key;
        }

        /**
         * Constant reference to the current element's key.
         */
        OZ_ALWAYS_INLINE
        const int& key() const
        {
          return elem->key;
        }

        /**
         * Constant reference to the current element's value.
         */
        OZ_ALWAYS_INLINE
        const Value& value() const
        {
          return elem->value;
        }

        /**
         * Advance to the next element.
         */
        CIterator& operator ++ ()
        {
          hard_assert( elem != null );

          if( elem->next != null ) {
            elem = elem->next;
          }
          else if( index == SIZE - 1 ) {
            elem = null;
          }
          else {
            do {
              ++index;
              elem = data[index];
            }
            while( elem == null && index < SIZE - 1 );
          }
          return *this;
        }

    };

    /**
     * %Iterator with non-constant access to container elements.
     */
    class Iterator : public IteratorBase<Elem>
    {
      friend class HashIndex;

      private:

        using IteratorBase<Elem>::elem;

        Elem* const* data;  ///< Pointer to hashtable slots.
        int          index; ///< Index of the current slot.

        /**
         * %Iterator for the given container, points to its first element.
         */
        explicit Iterator( const HashIndex& t ) :
          IteratorBase<Elem>( t.data[0] ), data( t.data ), index( 0 )
        {
          while( elem == null && index < SIZE - 1 ) {
            ++index;
            elem = data[index];
          }
        }

      public:

        /**
         * Default constructor, creates an invalid iterator.
         */
        OZ_ALWAYS_INLINE
        Iterator() :
          IteratorBase<Elem>( null ), data( null ), index( 0 )
        {}

        /**
         * Constant pointer to the current element's key.
         */
        OZ_ALWAYS_INLINE
        operator const int* () const
        {
          return &elem->key;
        }

        /**
         * Constant reference to the current element's key.
         */
        OZ_ALWAYS_INLINE
        const int& operator * () const
        {
          return elem->key;
        }

        /**
         * Constant reference to the current element's key.
         */
        OZ_ALWAYS_INLINE
        const int& key() const
        {
          return elem->key;
        }

        /**
         * Constant reference to the current element's value.
         */
        OZ_ALWAYS_INLINE
        const Value& value() const
        {
          return elem->value;
        }

        /**
         * Reference to the current element's value.
         */
        OZ_ALWAYS_INLINE
        Value& value()
        {
          return elem->value;
        }

        /**
         * Advance to the next element.
         */
        Iterator& operator ++ ()
        {
          hard_assert( elem != null );

          if( elem->next != null ) {
            elem = elem->next;
          }
          else if( index == SIZE - 1 ) {
            elem = null;
          }
          else {
            do {
              ++index;
              elem = data[index];
            }
            while( elem == null && index < SIZE - 1 );
          }
          return *this;
        }

    };

  private:

    Elem*            data[SIZE]; ///< %Array of linked lists.
    Pool<Elem, SIZE> pool;       ///< Memory pool for elements.

    /**
     * True iff chains have same length and respective elements are equal.
     */
    static bool areChainsEqual( const Elem* chainA, const Elem* chainB )
    {
      while( chainA != null && chainB != null ) {
        if( chainA->key != chainB->key || chainA->value != chainB->value ) {
          return false;
        }

        chainA = chainA->next;
        chainB = chainB->next;
      }
      // At least one is null.
      return chainA == chainB;
    }

    /**
     * Allocate and make a copy of the given chain.
     */
    Elem* cloneChain( const Elem* chain )
    {
      Elem* newChain = null;

      while( chain != null ) {
        newChain = new( pool ) Elem( newChain, chain->key, chain->value );
        chain = chain->next;
      }
      return newChain;
    }

    /**
     * Delete all elements in the given chain.
     */
    void clearChain( Elem* chain )
    {
      while( chain != null ) {
        Elem* next = chain->next;

        chain->~Elem();
        pool.dealloc( chain );

        chain = next;
      }
    }

    /**
     * Delete all elements and referenced objects in the given chain.
     */
    void freeChain( Elem* chain )
    {
      while( chain != null ) {
        Elem* next = chain->next;

        delete chain->value;
        chain->~Elem();
        pool.dealloc( chain );

        chain = next;
      }
    }

  public:

    /**
     * Create an empty hashtable.
     */
    HashIndex()
    {
      aSet<Elem*, Elem*>( data, null, SIZE );
    }

    /**
     * Destructor.
     */
    ~HashIndex()
    {
      clear();
      dealloc();
    }

    /**
     * Copy constructor, copies elements and storage.
     */
    HashIndex( const HashIndex& t )
    {
      for( int i = 0; i < SIZE; ++i ) {
        data[i] = cloneChain( t.data[i] );
      }
    }

    /**
     * Move constructor, moves storage.
     */
    HashIndex( HashIndex&& t ) :
      pool( static_cast< Pool<Elem, SIZE>&& >( t.pool ) )
    {
      aCopy<Elem*>( data, t.data, SIZE );
      aSet<Elem*, Elem*>( t.data, null, SIZE );
    }

    /**
     * Copy operator, copies elements and storage.
     */
    HashIndex& operator = ( const HashIndex& t )
    {
      if( &t == this ) {
        return *this;
      }

      for( int i = 0; i < SIZE; ++i ) {
        clearChain( data[i] );
        data[i] = cloneChain( t.data[i] );
      }
      return *this;
    }

    /**
     * Move operator, moves storage.
     */
    HashIndex& operator = ( HashIndex&& t )
    {
      if( &t == this ) {
        return *this;
      }

      clear();

      aCopy<Elem*>( data, t.data, SIZE );
      pool = static_cast< Pool<Elem, SIZE>&& >( t.pool );

      aSet<Elem*, Elem*>( t.data, null, SIZE );
      return *this;
    }

    /**
     * True iff respective elements are equal (including chain order).
     */
    bool operator == ( const HashIndex& t ) const
    {
      if( pool.length() != t.pool.length() ) {
        return false;
      }

      for( int i = 0; i < SIZE; ++i ) {
        if( !areChainsEqual( data[i], t.data[i] ) ) {
          return false;
        }
      }
      return true;
    }

    /**
     * False iff respective elements are equal (including chain order!).
     */
    bool operator != ( const HashIndex& t ) const
    {
      if( pool.length() != t.pool.length() ) {
        return true;
      }

      for( int i = 0; i < SIZE; ++i ) {
        if( !areChainsEqual( data[i], t.data[i] ) ) {
          return true;
        }
      }
      return false;
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
    Iterator iter() const
    {
      return Iterator( *this );
    }

    /**
     * Number of elements.
     */
    OZ_ALWAYS_INLINE
    int length() const
    {
      return pool.length();
    }

    /**
     * True iff empty.
     */
    OZ_ALWAYS_INLINE
    bool isEmpty() const
    {
      return pool.isEmpty();
    }

    /**
     * Current size of memory pool for elements.
     */
    OZ_ALWAYS_INLINE
    int capacity() const
    {
      return pool.capacity();
    }

    /**
     * Number of elements divided by hashtable index size.
     */
    float loadFactor() const
    {
      return float( pool.length() ) / float( SIZE );
    }

    /**
     * True iff the given key is found in the hashtable.
     */
    bool contains( int key ) const
    {
      uint  i = uint( key ) % uint( SIZE );
      Elem* p = data[i];

      while( p != null ) {
        if( p->key == key ) {
          return true;
        }

        p = p->next;
      }
      return false;
    }

    /**
     * If the key exists, return constant pointer to its value, otherwise return <tt>null</tt>.
     */
    const Value* find( int key ) const
    {
      uint  i = uint( key ) % uint( SIZE );
      Elem* p = data[i];

      while( p != null ) {
        if( p->key == key ) {
          return &p->value;
        }

        p = p->next;
      }
      return null;
    }

    /**
     * If the key exists, return pointer to its value, otherwise return <tt>null</tt>.
     */
    Value* find( int key )
    {
      uint  i = uint( key ) % uint( SIZE );
      Elem* p = data[i];

      while( p != null ) {
        if( p->key == key ) {
          return &p->value;
        }

        p = p->next;
      }
      return null;
    }

    /**
     * Add a new element, if the key already exists in the hashtable overwrite existing element.
     *
     * @return Pointer to the value of the inserted element.
     */
    template <typename Value_ = Value>
    Value* add( int key, Value_&& value = Value() )
    {
      uint  i = uint( key ) % uint( SIZE );
      Elem* p = data[i];

      while( p != null ) {
        if( p->key == key ) {
          p->value = static_cast<Value_&&>( value );
          return &p->value;
        }

        p = p->next;
      }

      data[i] = new( pool ) Elem( data[i], key, static_cast<Value_&&>( value ) );

      soft_assert( loadFactor() < 0.75f );

      return &data[i]->value;
    }

    /**
     * Add a new element if the key does not exist in the hashtable.
     *
     * @return Pointer to the value of the inserted or the existing element with the same key.
     */
    template <typename Value_ = Value>
    Value* include( int key, Value_&& value = Value() )
    {
      uint  i = uint( key ) % uint( SIZE );
      Elem* p = data[i];

      while( p != null ) {
        if( p->key == key ) {
          return &p->value;
        }

        p = p->next;
      }

      data[i] = new( pool ) Elem( data[i], key, static_cast<Value_&&>( value ) );

      soft_assert( loadFactor() < 0.75f );

      return &data[i]->value;
    }

    /**
     * Remove element with the given key.
     *
     * @return True iff the key was found (and removed).
     */
    bool exclude( int key )
    {
      uint   i    = uint( key ) % uint( SIZE );
      Elem*  p    = data[i];
      Elem** prev = &data[i];

      while( p != null ) {
        if( p->key == key ) {
          *prev = p->next;

          p->~Elem();
          pool.dealloc( p );

          return true;
        }

        prev = &p->next;
        p = p->next;
      }
      return false;
    }

    /**
     * Empty the hashtable.
     */
    void clear()
    {
      for( int i = 0; i < SIZE; ++i ) {
        clearChain( data[i] );
        data[i] = null;
      }
    }

    /**
     * Delete all objects referenced by element values and empty the hashtable.
     */
    void free()
    {
      for( int i = 0; i < SIZE; ++i ) {
        freeChain( data[i] );
        data[i] = null;
      }
    }

    /**
     * Deallocate pool memory of an empty hastable.
     */
    void dealloc()
    {
      hard_assert( pool.isEmpty() );

      pool.free();
    }

};

}
