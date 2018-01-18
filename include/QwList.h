/*
    Queue World is copyright (c) 2014 Ross Bencina

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#ifndef INCLUDED_QWLIST_H
#define INCLUDED_QWLIST_H

#include <algorithm>
#include <cassert>

#include "qw_remove_pointer.h"

#include "QwConfig.h"


/*
    QwList is a single-threaded doubly linked list.

    Has bidirectional iterator supporting operator-- on end().
    This implies that the iterator internally points to the node before *i.
*/

template<typename NodePtrT, int NEXT_LINK_INDEX, int PREVIOUS_LINK_INDEX>
class QwList{
    typedef QwList<NodePtrT, NEXT_LINK_INDEX, PREVIOUS_LINK_INDEX> this_list_t;

public:
    typedef typename qw_remove_pointer<NodePtrT>::type node_type;

    typedef NodePtrT node_ptr_type;
    typedef const node_type* const_node_ptr_type;

private:
    static const node_ptr_type& NEXT_PTR( const node_ptr_type& n )
    {
        return n->links_[ NEXT_LINK_INDEX ];
    }

    static node_ptr_type& NEXT_PTR( node_ptr_type& n )
    {
        return n->links_[ NEXT_LINK_INDEX ];
    }

    static const_node_ptr_type NEXT_PTR( const_node_ptr_type& n )
    {
        return n->links_[ NEXT_LINK_INDEX ];
    }

    static size_t OFFSETOF_NEXT_PTR()
    {
        return reinterpret_cast<size_t>(&(static_cast<node_ptr_type>(0)->links_[ NEXT_LINK_INDEX ]));
    }

    static const node_ptr_type& PREVIOUS_PTR( const node_ptr_type& n )
    {
        return n->links_[ PREVIOUS_LINK_INDEX ];
    }

    static node_ptr_type& PREVIOUS_PTR( node_ptr_type& n )
    {
        return n->links_[ PREVIOUS_LINK_INDEX ];
    }

    static const_node_ptr_type PREVIOUS_PTR( const_node_ptr_type& n )
    {
        return n->links_[ PREVIOUS_LINK_INDEX ];
    }

    static size_t OFFSETOF_PREVIOUS_PTR()
    {
        return reinterpret_cast<size_t>(&(static_cast<node_ptr_type>(0)->links_[ PREVIOUS_LINK_INDEX ]));
    }

    bool NODE_IS_LINKED( const_node_ptr_type n )
    {
        return( NEXT_PTR(n) != 0 || PREVIOUS_PTR(n) != 0 ); // allow for node being at start or end
    }

#ifdef QW_VALIDATE_NODE_LINKS
    void CHECK_NODE_IS_UNLINKED( const_node_ptr_type n )
    {
        assert( NEXT_PTR(n) == 0 );
        assert( PREVIOUS_PTR(n) == 0 );

        // if the list has only one element n->next will be 0 but could
        // still be in our list (or another list, but we can only check our list)
        assert( n != front_ );
    }

    void CLEAR_NODE_LINKS( node_ptr_type n )
    {
        NEXT_PTR(n) = 0;
        PREVIOUS_PTR(n) = 0;
    }
#else
#define CHECK_NODE_IS_UNLINKED(n)
#define CLEAR_NODE_LINKS(n)
#endif

    node_ptr_type front_; // aka head. first link in list
    node_ptr_type back_; // last link in list

public: /// ONLY PUBLIC FOR TESTING

    // internal use. exposed for testing only
    node_ptr_type before_front_()
    {
        // pretend our front_ field is actually the next link field in a node struct
        // offset backwards from front_ then cast to a node ptr and wrap in an iterator
        // this is probably not strictly portable but it allows us to insert at the beginning.
        return reinterpret_cast<node_ptr_type>(reinterpret_cast<char*>(&front_) - OFFSETOF_NEXT_PTR());
    }

    const node_type* before_front_() const
    {
        // pretend our front_ field is actually the next link field in a node struct
        // offset backwards from front_ then cast to a node ptr and wrap in an iterator
        // this is probably not strictly portable but it allows us to insert at the beginning.
        //return reinterpret_cast<const_node_ptr_type>(reinterpret_cast<const char*>(&front_) - OFFSETOF_NEXT_PTR());

        return reinterpret_cast<const node_type*>(reinterpret_cast<const char*>(&front_) - OFFSETOF_NEXT_PTR());
    }

public:

    class iterator{
        friend this_list_t;
        node_ptr_type p_;
    public:
#ifdef QW_VALIDATE_NODE_LINKS
        iterator() : p_( 0 ) {}
#else
        iterator() {}
#endif

        // FIXME make ctor take the pointed-to node so clients can construct their own iterators.
        // end() can be constructed with a private ctor with a dummy parameter

        explicit iterator( node_ptr_type p ) : p_( p ) {} // an iterator pointing to p->next

        iterator& operator++ ()     // prefix ++
        {
            p_ = NEXT_PTR(p_);
            return *this;
        }

        iterator operator++ (int)  // postfix ++
        {
            iterator result(*this);
            ++(*this);
            return result;
        }

        iterator& operator-- ()     // prefix --
        {
            p_ = PREVIOUS_PTR(p_);
            return *this;
        }

        iterator operator-- (int)  // postfix --
        {
            iterator result(*this);
            --(*this);
            return result;
        }

        // it's a container of pointers so dereferencing the iterator gives a pointer
        node_ptr_type operator*() const { return NEXT_PTR(p_); }
        const node_ptr_type* operator->() const { return &NEXT_PTR(p_); }

        bool operator!=(const iterator& rhs) const { return rhs.p_ != p_; }
        bool operator==(const iterator& rhs) const { return rhs.p_ == p_; }
    };

    class const_iterator{
        friend this_list_t;
        const node_type *p_;
    public:
#ifdef QW_VALIDATE_NODE_LINKS
        const_iterator() : p_( 0 ) {}
#else
        const_iterator() {}
#endif

        explicit const_iterator( const node_type *p ) : p_( p ) {} // an iterator pointing to p->next
        explicit const_iterator( const iterator& i ) : p_( i.p_ ) {} // an iterator pointing to p->next

        const_iterator& operator++ ()     // prefix ++
        {
            p_ = NEXT_PTR(p_);
            return *this;
        }

        const_iterator operator++ (int)  // postfix ++
        {
            iterator result(*this);
            ++(*this);
            return result;
        }

        const_iterator& operator-- ()     // prefix --
        {
            p_ = PREVIOUS_PTR(p_);
            return *this;
        }

        const_iterator operator-- (int)  // postfix --
        {
            iterator result(*this);
            --(*this);
            return result;
        }

        // it's a container of pointers so dereferencing the iterator gives a pointer
        const node_type* operator*() const { return NEXT_PTR(p_); }
        const node_type** operator->() const { return &NEXT_PTR(p_); }

        bool operator!=(const const_iterator& rhs) const { return rhs.p_ != p_; }
        bool operator==(const const_iterator& rhs) const { return rhs.p_ == p_; }
    };

    QwList() : front_( before_front_() ), back_( before_front_() ) {}

    void clear() {
#ifdef QW_VALIDATE_NODE_LINKS
        while( !empty() ) pop_front();
#else
        // this doesn't mark nodes as unlinked
        front_ = back_ = before_front_();
#endif
    }

    void swap( QwList& other ) {
        std::swap( front_, other.front_ );
        std::swap( back_, other.back_ );

        if( front_ == other.before_front_() ){ // empty
            front_ = back_ = before_front_();
        }else{
            PREVIOUS_PTR(front_) = before_front_();
        }

        if( other.front_ == before_front_() ){ // empty
            other.front_ = other.back_ = other.before_front_();
        }else{
            PREVIOUS_PTR(other.front_) = other.before_front_();
        }
    }
    //see also void swap( QwList& a, QwList &b );

    bool empty() const
    {
        return (front_ == before_front_());
    }

    bool size_is_1() const
    {
        return (front_ != before_front_() && front_ == back_ );
    }

    bool size_is_greater_than_1() const
    {
        return (front_ != before_front_() && front_ != back_ );
    }

    node_ptr_type front() { assert(!empty()); return front_; }
    const_node_ptr_type front() const { assert(!empty()); return front_; }

    node_ptr_type back() { assert(!empty()); return back_; }
    const_node_ptr_type back() const { assert(!empty()); return back_; }

    void push_front( node_ptr_type n )
    {
        CHECK_NODE_IS_UNLINKED( n );

        if( empty() ){
            NEXT_PTR(n) = 0;
            back_ = n;
        }else{
            NEXT_PTR(n) = front_;
            PREVIOUS_PTR(front_) = n;
        }

        PREVIOUS_PTR(n) = before_front_();
        front_ = n;
    }

    node_ptr_type pop_front()
    {
        assert( !empty() ); // this version of pop_front doesn't work on an empty list.
							// caller should check is_empty() first.

        node_ptr_type result = front_;

        front_ = NEXT_PTR(front_);
        if( front_ ){
            PREVIOUS_PTR(front_) = before_front_();
        }else{
            front_ = back_ = before_front_();
        }

        CLEAR_NODE_LINKS( result );
        return result;
    }

    void push_back( node_ptr_type n )
    {
        CHECK_NODE_IS_UNLINKED( n );

        NEXT_PTR(n) = 0;

		if( empty() ){
            PREVIOUS_PTR(n) = before_front_();
            front_ = n;
        }else{
            PREVIOUS_PTR(n) = back_;
            NEXT_PTR(back_) = n;
        }

        back_ = n;
    }

    void insert_after( node_ptr_type before, node_ptr_type n ) // insert n after node before
    {
        assert( before != 0 );
        assert( n != 0 );
        CHECK_NODE_IS_UNLINKED( n );

        if( empty() ){
            assert( before == before_front_() );

            PREVIOUS_PTR(n) = before_front_();
            NEXT_PTR(n) = 0;
            back_ = front_ = n;
        }else{
            node_ptr_type after = NEXT_PTR(before);

            NEXT_PTR(n) = after;
            if( after )
                PREVIOUS_PTR(after) = n;
            else
                back_ = n;

            // if before is before_front_() then this will update front_:
            NEXT_PTR(before) = n;
            PREVIOUS_PTR(n) = before;
        }
    }

    node_ptr_type remove_after( node_ptr_type before ) // returns the removed node
    {
        assert( NEXT_PTR(before) != 0 ); // can't remove an item after the last item

        node_ptr_type result = NEXT_PTR(before);
        node_ptr_type after = NEXT_PTR(result);

        NEXT_PTR(before) = after; // (NEXT_PTR(before) aliases front when using before_front_())
        // so this will correctly zero front_ if list is empty

        if( after ){
            PREVIOUS_PTR(after) = before;
        }else{
            if( front_ == 0 ) // zeroed above in assignment to NEXT_PTR(before)
                front_ = back_ = before_front_();
            else
                back_ = before;
        }

        CLEAR_NODE_LINKS( result );
        return result;
    }

    node_ptr_type pop_back()
    {
        assert( !empty() ); // this version of pop_back doesn't work on an empty list.
							// caller should check is_empty() first.

        node_ptr_type result = back_;

        back_ = PREVIOUS_PTR(back_);
        if( back_ == before_front_() ){
            front_ = before_front_();
        }else{
            NEXT_PTR(back_) = 0;
        }

        CLEAR_NODE_LINKS( result );
        return result;
    }

    void insert( node_ptr_type at, node_ptr_type n ) // insert n before node at
    {
        assert( at != 0 );
        assert( at != before_front_() );
        CHECK_NODE_IS_UNLINKED( n );

        NEXT_PTR(n) = at;
        PREVIOUS_PTR(n) = PREVIOUS_PTR(at);

        PREVIOUS_PTR(at) = n;

        // this will correctly update front_ if at is at the front
        NEXT_PTR( PREVIOUS_PTR(n) ) = n;
    }

    void insert( iterator at, node_ptr_type n ) // insert n before node at
    {
        insert_after( at.p_, n ); // use insert_after because iterator.p_ points to the previous item
    }

    // forward_list provides remove() and remove_if()

    void remove( node_ptr_type at ) // remove node at
    {
        assert( NODE_IS_LINKED( at ) );

        node_ptr_type before = PREVIOUS_PTR(at);
        node_ptr_type after = NEXT_PTR(at);

        // this also covers the case where at == front_, where it updates front_
        NEXT_PTR(before) = after;

        if( after ){
            // at wasn't the last (or only) element
            PREVIOUS_PTR(after) = before;
        }else{
            // at was back
            if( front_ == 0 ) // updated in assignment from NEXT_PTR(before) = after
                front_ = back_ = before_front_();
            else
                back_ = before;
        }

        CLEAR_NODE_LINKS( at );
    }

    void erase( iterator at ) // remove node at at
    {
        remove_after( at.p_ );
    }

    iterator begin() { return iterator(before_front_()); }
    iterator end() { return iterator(back_); }

    const_iterator begin() const { return const_iterator(before_front_()); }
    const_iterator end() const { return const_iterator(back_); }

    // forward_list also provides const iterator and const iterator accessors

    static node_ptr_type next( node_ptr_type n ) { return NEXT_PTR(n); }
    static node_ptr_type previous( node_ptr_type n ) { return PREVIOUS_PTR(n); }
};

template<typename NodePtrT, int NEXT_LINK_INDEX, int PREVIOUS_LINK_INDEX>
inline void swap( QwList<NodePtrT,NEXT_LINK_INDEX,PREVIOUS_LINK_INDEX>& a, QwList<NodePtrT,NEXT_LINK_INDEX,PREVIOUS_LINK_INDEX>& b )
{
    a.swap(b);
}

#endif /* INCLUDED_QWLIST_H */

/* -----------------------------------------------------------------------
Last reviewed: June 30, 2013
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
