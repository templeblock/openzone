/*
 *  DList.h
 *
 *  Double-linked list
 *  The Type should provide the "prev[INDEX]" and "next[INDEX]" pointers.
 *
 *  Copyright (C) 2002-2010, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU General Public License v3. See COPYING for details.
 */

#pragma once

namespace oz
{

  /**
   * Double-linked list
   *
   * It can only be applied on classes that have <code>next[]</code> and <code>prev[]</code>
   * members.
   * Example:
   * <pre>struct C
   * {
   *   C* prev[2];
   *   C* next[2];
   *   int value;
   * };
   * ...
   * DList&lt;C, 0&gt; list1;
   * DList&lt;C, 1&gt; list2;</pre>
   * That way the objects of the same class can be in two separate lists at once.
   * <code>prev[0]</code> and <code>next[0]</code> point to previous and next element respectively
   * in <code>list1</code> and
   * <code>prev[1]</code> and <code>next[1]</code> point to previous and next element respectively
   * in <code>list2</code>.
   *
   * <code>prev[INDEX]</code> and <code>next[INDEX]</code> pointers are not cleared when element is
   * removed from the list, they may still point to elements in the list or to invalid locations!
   *
   * <code>DList</code> class doesn't take care of memory management except for the
   * <code>free()</code> method.
   *
   * In general all operations are O(1) except <code>contains()</code>, <code>length()</code> and
   * <code>free()</code> are O(n).
   */


  template <class Type, int INDEX = 0>
  class DList
  {
    public:

      /**
       * DList iterator.
       */
      class Iterator : public IteratorBase<Type>
      {
        private:

          typedef IteratorBase<Type> B;

        public:

          /**
           * Default constructor returns a dummy passed iterator
           * @return
           */
          explicit Iterator() : B( null )
          {}

          /**
           * Make iterator for given list. After creation it points to first element.
           * @param l
           */
          explicit Iterator( const DList& l ) : B( l.firstElem )
          {}

          /**
           * When iterator advances beyond last element, it becomes passed. It points to an invalid
           * location.
           * @return true if iterator is passed
           */
          bool isPast() const
          {
            return B::elem == null;
          }

          /**
           * Advance to next element.
           */
          Iterator& operator ++ ()
          {
            assert( B::elem != null );

            B::elem = B::elem->next[INDEX];
            return *this;
          }

      };

    private:

      // First element in list.
      Type* firstElem;
      // Last element in list.
      Type* lastElem;

    public:

      /**
       * Create an empty list.
       */
      explicit DList() : firstElem( null ), lastElem( null )
      {}

      /**
       * Copy constructor
       * Allocate copies of all elements of the original list. Type should implement copy
       * constructor and for sake of performance the order of elements in the new list is reversed.
       * @param l the original list
       */
      DList( const DList& l ) : firstElem( null ), lastElem( null )
      {
        foreach( e, Iterator( l ) ) {
          pushFirst( new Type( *e ) );
        }
      }

      /**
       * Create a list with only one element.
       * @param e the element
       */
      explicit DList( Type* e ) : firstElem( e ), lastElem( e )
      {
        e->prev[INDEX] = null;
        e->next[INDEX] = null;
      }

      /**
       * Clone list.
       * Allocate copies of all elements of the original list. Type should implement copy
       * constructor and for sake of performance the order of elements in the new list is reversed.
       * @param l
       * @return
       */
      DList clone() const
      {
        DList clone;

        foreach( e, Iterator( *this ) ) {
          clone.pushFirst( new Type( *e ) );
        }
        return clone;
      }

      /**
       * Compare all elements in two lists.
       * Type should implement operator =, otherwise comparison doesn't make sense (two copies
       * always differ on next[INDEX] members).
       * @param l
       * @return
       */
      bool operator == ( const DList& l ) const
      {
        Type* e1 = firstElem;
        Type* e2 = l.firstElem;

        while( e1 != null && e2 != null ) {
          if( *e1 != *e2 ) {
            return false;
          }
          e1 = e1->next[INDEX];
          e2 = e2->next[INDEX];
        }
        // at least one is null, so (e1 == e2) <=> (e1 == null && e2 == null)
        return e1 == e2;
      }

      /**
       * Compare all elements in two lists.
       * Type should implement operator =, otherwise comparison doesn't make sense (two copies
       * always differ on next[INDEX] members).
       * @param l
       * @return
       */
      bool operator != ( const DList& l ) const
      {
        Type* e1 = firstElem;
        Type* e2 = l.firstElem;

        while( e1 != null && e2 != null ) {
          if( *e1 != *e2 ) {
            return true;
          }
          e1 = e1->next[INDEX];
          e2 = e2->next[INDEX];
        }
        // at least one is null, so (e1 == e2) <=> (e1 == null && e2 == null)
        return e1 != e2;
      }

      /**
       * @return iterator for this list
       */
      Iterator begin() const
      {
        return Iterator( *this );
      }

      /**
       * @return pointer that matches the passed iterator
       */
      const Type* end() const
      {
        return null;
      }

      /**
       * Count the elements in the list.
       * @return size of the list
       */
      int length() const
      {
        int i = 0;
        Type* p = firstElem;

        while( p != null ) {
          p = p->next[INDEX];
          ++i;
        }
        return i;
      }

      /**
       * @return true if the list has no elements
       */
      bool isEmpty() const
      {
        assert( ( firstElem == null ) == ( lastElem == null ) );

        return firstElem == null;
      }

      /**
       * @param e requested element
       * @return true if some element in the list points to the requested element
       */
      bool contains( const Type* e ) const
      {
        assert( e != null );

        Type* p = firstElem;

        while( p != null ) {
          if( p == e ) {
            return true;
          }
          p = p->next[INDEX];
        }
        return false;
      }

      /**
       * @return pointer to first element in the list
       */
      Type* first()
      {
        return firstElem;
      }

      /**
       * @return constant pointer to first element in the list
       */
      const Type* first() const
      {
        return firstElem;
      }

      /**
       * @return pointer to last element in the list
       */
      Type* last()
      {
        return lastElem;
      }

      /**
       * @return constant pointer to last element in the list
       */
      const Type* last() const
      {
        return lastElem;
      }

      /**
       * Add element to the beginning of the list.
       * @param e element to be added
       */
      void operator << ( Type* e )
      {
        pushFirst( e );
      }

      /**
       * Add element to the beginning of the list.
       * @param e element to be added
       */
      void add( Type* e )
      {
        pushFirst( e );
      }

      /**
       * Add element to the beginning of the list.
       * @param e element to be added
       */
      void pushFirst( Type* e )
      {
        assert( e != null );

        e->prev[INDEX] = null;
        e->next[INDEX] = firstElem;

        if( firstElem == null ) {
          firstElem = e;
          lastElem = e;
        }
        else {
          firstElem->prev[INDEX] = e;
          firstElem = e;
        }
      }

      /**
       * Add element to the end of the list.
       * @param e element to be added
       */
      void pushLast( Type* e )
      {
        assert( e != null );

        e->prev[INDEX] = lastElem;
        e->next[INDEX] = null;

        if( lastElem == null ) {
          firstElem = e;
          lastElem = e;
        }
        else {
          lastElem->next[INDEX] = e;
          lastElem = e;
        }
      }

      /**
       * Pop first element from the list.
       * @param e reference to pointer where the pointer to the first element is to be saved
       */
      void operator >> ( Type*& e )
      {
        e = popFirst();
      }

      /**
       * Pop first element from the list.
       * @param e pointer to the first element
       */
      Type* popFirst()
      {
        assert( firstElem != null );

        Type* p = firstElem;

        firstElem = p->next[INDEX];

        if( firstElem == null ) {
          lastElem = null;
        }
        else {
          firstElem->prev[INDEX] = null;
        }
        return p;
      }

      /**
       * Pop last element from the list.
       * @param e pointer to the last element
       */
      Type* popLast()
      {
        assert( lastElem != null );

        Type* p = lastElem;

        lastElem = p->prev[INDEX];

        if( lastElem == null ) {
          firstElem = null;
        }
        else {
          lastElem->next[INDEX] = null;
        }
        return p;
      }

      /**
       * Insert an element after an element in the list.
       * @param e element to be inserted
       * @param p pointer to element after which we want to insert
       */
      void insertAfter( Type* e, Type* p )
      {
        assert( e != null );
        assert( p != null );

        e->prev[INDEX] = p;

        if( p == lastElem ) {
          e->next[INDEX] = null;
          p->next[INDEX] = e;
          lastElem = e;
        }
        else {
          Type* next = p->next[INDEX];

          next->prev[INDEX] = e;
          e->next[INDEX] = next;
          p->next[INDEX] = e;
        }
      }

      /**
       * Insert an element before an element in the list.
       * @param e element to be inserted
       * @param p pointer to element before which we want to insert
       */
      void insertBefore( Type* e, Type* p )
      {
        assert( e != null );
        assert( p != null );

        e->next[INDEX] = p;

        if( p == firstElem ) {
          e->prev[INDEX] = null;
          p->prev[INDEX] = e;
          firstElem = e;
        }
        else {
          Type* prev = p->prev[INDEX];

          prev->next[INDEX] = e;
          e->prev[INDEX] = prev;
          p->prev[INDEX] = e;
        }
      }

      /**
       * Remove an element from the list.
       * @param e element to be removed
       */
      void remove( Type* e )
      {
        if( e == firstElem ) {
          firstElem = e->next[INDEX];
        }
        else {
          e->prev[INDEX]->next[INDEX] = e->next[INDEX];
        }
        if( e == lastElem ) {
          lastElem = e->prev[INDEX];
        }
        else {
          e->next[INDEX]->prev[INDEX] = e->prev[INDEX];
        }
      }

      /**
       * Transfer elements from given list. The given list is cleared after the operation.
       * @param l
       */
      void transfer( DList& l )
      {
        if( l.isEmpty() ) {
          return;
        }

        assert( l.firstElem->prev[INDEX] == null );
        assert( l.lastElem->next[INDEX] == null );

        l.lastElem->next[INDEX] = firstElem;

        if( firstElem == null ) {
          firstElem = l.firstElem;
          lastElem = l.lastElem;
        }
        else {
          firstElem->prev[INDEX] = l.lastElem;
          firstElem = l.firstElem;
        }

        firstElem = null;
        lastElem = null;
      }

      /**
       * Empty the list but don't delete the elements.
       */
      void clear()
      {
        firstElem = null;
        lastElem = null;
      }

      /**
       * Empty the list and delete all elements - take care of memory management.
       */
      void free()
      {
        Type* p = firstElem;

        while( p != null ) {
          Type* next = p->next[INDEX];

          delete p;
          p = next;
        }

        firstElem = null;
        lastElem = null;
      }

  };

}
