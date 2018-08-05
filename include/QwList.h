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
#ifdef NDEBUG
#include <cstdlib> // abort
#endif

#include "qw_remove_pointer.h"

#include "QwConfig.h"
#include "QwLinkTraits.h"


template<typename NodePtrT, int NEXT_LINK_INDEX, int PREVIOUS_LINK_INDEX>
struct QwDoubleLinkNodeInfo {
    typedef std::size_t size_t;

    typedef typename qw_remove_pointer<NodePtrT>::type node_type;
    typedef NodePtrT node_ptr_type;
    typedef const node_type* const_node_ptr_type;

    typedef QwLinkTraits<NodePtrT,NEXT_LINK_INDEX> nextlink;
    typedef QwLinkTraits<NodePtrT,PREVIOUS_LINK_INDEX> prevlink;

    static node_ptr_type load_next( const_node_ptr_type n ) { return nextlink::load(n); }
    static void store_next( node_ptr_type n, node_ptr_type x ) { nextlink::store(n, x); }
    static size_t offsetof_next_link() { return nextlink::offsetof_link(); }

    static node_ptr_type load_prev( const_node_ptr_type n ) { return prevlink::load(n); }
    static void store_prev( node_ptr_type n, node_ptr_type x ) { prevlink::store(n, x); }
    static size_t offsetof_prev_link() { return prevlink::offsetof_link(); }

    static bool is_linked( const_node_ptr_type n )
    {
        // allow for node being at start or end: only one of {next, prev} needs to be linked.
        return ((nextlink::load(n) != 0) || (prevlink::load(n) != 0));
    }

    static bool is_unlinked( const_node_ptr_type n )
    {
        return ((nextlink::load(n) == 0) && (prevlink::load(n) == 0));
    }

    static void clear( node_ptr_type n )
    {
        nextlink::store(n, 0);
        prevlink::store(n, 0);
    }
};

/*
    QwList is a single-threaded doubly linked list.

    Has bidirectional iterator supporting operator-- on end().
    This implies that the iterator internally points to the node before *i.
*/

template<typename NodePtrT, int NEXT_LINK_INDEX, int PREVIOUS_LINK_INDEX>
class QwList{
    typedef QwList<NodePtrT, NEXT_LINK_INDEX, PREVIOUS_LINK_INDEX> list_type;

    typedef QwDoubleLinkNodeInfo<NodePtrT, NEXT_LINK_INDEX, PREVIOUS_LINK_INDEX> links;

public:
    typedef typename links::node_type node_type;
    typedef typename links::node_ptr_type node_ptr_type;
    typedef typename links::const_node_ptr_type const_node_ptr_type;

private:

#if (QW_VALIDATE_NODE_LINKS == 1)
    void CHECK_NODE_IS_LINKED( const_node_ptr_type n ) const
    {
#ifndef NDEBUG
        assert( links::is_linked(n) == true );
#else
        if(!( links::is_linked(n) == true )) { std::abort(); }
#endif
    }

    void CHECK_NODE_IS_UNLINKED( const_node_ptr_type n ) const
    {
#ifndef NDEBUG
        assert( links::is_unlinked(n) == true );
        assert( n != front_ );
        assert( n != back_ );
        // Note: we can't check that the node is not referenced by some other list
#else
        if(!( links::is_unlinked(n) == true )) { std::abort(); }
        if(!( n != front_ )) { std::abort(); }
        if(!( n != back_ )) { std::abort(); }
#endif
    }

    void CLEAR_NODE_LINKS_FOR_VALIDATION( node_ptr_type n ) const
    {
        links::clear(n);
    }
#else
    void CHECK_NODE_IS_LINKED( const_node_ptr_type ) const {}
    void CHECK_NODE_IS_UNLINKED( const_node_ptr_type ) const {}
    void CLEAR_NODE_LINKS_FOR_VALIDATION( node_ptr_type ) const {}
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
        return reinterpret_cast<node_ptr_type>(reinterpret_cast<char*>(&front_) - links::offsetof_next_link());
    }

    const_node_ptr_type before_front_() const
    {
        // pretend our front_ field is actually the next link field in a node struct
        // offset backwards from front_ then cast to a node ptr and wrap in an iterator
        // this is probably not strictly portable but it allows us to insert at the beginning.
        return reinterpret_cast<const_node_ptr_type>(reinterpret_cast<const char*>(&front_) - links::offsetof_next_link());
    }

public:

    class iterator{
        friend list_type;
        node_ptr_type p_;
    public:
#if (QW_VALIDATE_NODE_LINKS == 1)
        iterator() : p_( 0 ) {}
#else
        iterator() {}
#endif

        // FIXME make ctor take the pointed-to node so clients can construct their own iterators.
        // end() can be constructed with a private ctor with a dummy parameter

        explicit iterator( node_ptr_type p ) : p_( p ) {} // an iterator pointing to p->next

        iterator& operator++ ()     // prefix ++
        {
            p_ = links::load_next(p_);
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
            p_ = links::load_prev(p_);
            return *this;
        }

        iterator operator-- (int)  // postfix --
        {
            iterator result(*this);
            --(*this);
            return result;
        }

        // list is a container of pointers so dereferencing the iterator gives a pointer
        node_ptr_type operator*() const { return links::load_next(p_); }
        const node_ptr_type* operator->() const { return &links::load_next(p_); }

        bool operator!=(const iterator& rhs) const { return rhs.p_ != p_; }
        bool operator==(const iterator& rhs) const { return rhs.p_ == p_; }
    };

    class const_iterator{
        friend list_type;
        const_node_ptr_type p_;
    public:
#if (QW_VALIDATE_NODE_LINKS == 1)
        const_iterator() : p_( 0 ) {}
#else
        const_iterator() {}
#endif

        explicit const_iterator( const_node_ptr_type p ) : p_( p ) {} // an iterator pointing to p->next
        explicit const_iterator( const iterator& i ) : p_( i.p_ ) {} // an iterator pointing to p->next

        const_iterator& operator++ ()     // prefix ++
        {
            p_ = links::load_next(p_);
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
            p_ = links::load_prev(p_);
            return *this;
        }

        const_iterator operator-- (int)  // postfix --
        {
            iterator result(*this);
            --(*this);
            return result;
        }

        // list is a container of pointers so dereferencing the iterator gives a pointer
        const_node_ptr_type operator*() const { return links::load_next(p_); }
        const const_node_ptr_type* operator->() const { return &links::load_next(p_); }

        bool operator!=(const const_iterator& rhs) const { return rhs.p_ != p_; }
        bool operator==(const const_iterator& rhs) const { return rhs.p_ == p_; }
    };

    QwList() : front_( before_front_() ), back_( before_front_() ) {}

    void clear() {
#if (QW_VALIDATE_NODE_LINKS == 1)
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
            links::store_prev(front_, before_front_());
        }

        if( other.front_ == before_front_() ){ // empty
            other.front_ = other.back_ = other.before_front_();
        }else{
            links::store_prev(other.front_, other.before_front_());
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
            links::store_next(n, 0);
            back_ = n;
        }else{
            links::store_next(n, front_);
            links::store_prev(front_, n);
        }

        links::store_prev(n, before_front_());
        front_ = n;
    }

    node_ptr_type pop_front()
    {
        assert( !empty() ); // this version of pop_front doesn't work on an empty list.
							// caller should check is_empty() first.

        node_ptr_type result = front_;

        front_ = links::load_next(front_);
        if( front_ ){
            links::store_prev(front_, before_front_());
        }else{
            front_ = back_ = before_front_();
        }

        CLEAR_NODE_LINKS_FOR_VALIDATION( result );
        return result;
    }

    void push_back( node_ptr_type n )
    {
        CHECK_NODE_IS_UNLINKED( n );

        links::store_next(n, 0);

		if( empty() ){
            links::store_prev(n, before_front_());
            front_ = n;
        }else{
            links::store_prev(n, back_);
            links::store_next(back_, n);
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

            links::store_prev(n, before_front_());
            links::store_next(n, 0);
            back_ = front_ = n;
        }else{
            node_ptr_type after = links::load_next(before);

            links::store_next(n, after);
            if( after )
                links::store_prev(after, n);
            else
                back_ = n;

            // if before is before_front_() then this will update front_:
            links::store_next(before, n);
            links::store_prev(n, before);
        }
    }

    node_ptr_type remove_after( node_ptr_type before ) // returns the removed node
    {
        assert( links::load_next(before) != 0 ); // can't remove an item after the last item

        node_ptr_type result = links::load_next(before);
        node_ptr_type after = links::load_next(result);

        links::store_next(before, after); // (links::load_next(before) aliases front when using before_front_())
        // so this will correctly zero front_ if list is empty

        if( after ){
            links::store_prev(after, before);
        }else{
            if( front_ == 0 ) // zeroed above in assignment to links::load_next(before)
                front_ = back_ = before_front_();
            else
                back_ = before;
        }

        CLEAR_NODE_LINKS_FOR_VALIDATION( result );
        return result;
    }

    node_ptr_type pop_back()
    {
        assert( !empty() ); // this version of pop_back doesn't work on an empty list.
							// caller should check is_empty() first.

        node_ptr_type result = back_;

        back_ = links::load_prev(back_);
        if( back_ == before_front_() ){
            front_ = before_front_();
        }else{
            links::store_next(back_, 0);
        }

        CLEAR_NODE_LINKS_FOR_VALIDATION( result );
        return result;
    }

    void insert( node_ptr_type at, node_ptr_type n ) // insert n before node at
    {
        assert( at != 0 );
        assert( at != before_front_() );
        CHECK_NODE_IS_UNLINKED( n );

        links::store_next(n, at);
        links::store_prev(n, links::load_prev(at));

        links::store_prev(at, n);

        // this will correctly update front_ if at is at the front
        links::store_next( links::load_prev(n), n);
    }

    void insert( iterator at, node_ptr_type n ) // insert n before node at
    {
        insert_after( at.p_, n ); // use insert_after because iterator.p_ points to the previous item
    }

    // forward_list provides remove() and remove_if()

    void remove( node_ptr_type at ) // remove node at
    {
        CHECK_NODE_IS_LINKED( at );

        node_ptr_type before = links::load_prev(at);
        node_ptr_type after = links::load_next(at);

        // this also covers the case where at == front_, where it updates front_
        links::store_next(before, after);

        if( after ){
            // at wasn't the last (or only) element
            links::store_prev(after, before);
        }else{
            // at was back
            if( front_ == 0 ) // updated in assignment links::store_next(before, after)
                front_ = back_ = before_front_();
            else
                back_ = before;
        }

        CLEAR_NODE_LINKS_FOR_VALIDATION( at );
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

    static node_ptr_type next( node_ptr_type n ) { return links::load_next(n); }
    static node_ptr_type previous( node_ptr_type n ) { return links::load_prev(n); }
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
