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
#ifndef INCLUDED_QWMPMCPOPALLLIFOSTACK_H
#define INCLUDED_QWMPMCPOPALLLIFOSTACK_H

#include <cassert>
#ifdef NDEBUG
#include <cstdlib> // abort
#endif

#include "mintomic/mintomic.h"
#include "qw_atomic.h"

#include "QwConfig.h"
#include "QwLinkTraits.h"

/*
    QwMpmcPopAllLifoStack is a lock-free concurrent LIFO stack that
    provides push() and pop_all()  operations. No single-node pop()
    operation is provided.

    All operations may be invoked concurrently.

    Implemented using the "IBM Freelist" LIFO algorithm.

    The algorithm doesn't need ABA protection because it does not provide a pop() operation.
    pop_all() is not subject to the ABA problem because it swaps in a 0 value and never
    requires comparison to a non-0 value.
    See ALGORITHMS.txt
*/

template<typename NodePtrT, int NEXT_LINK_INDEX>
class QwMpmcPopAllLifoStack{
    typedef QwLinkTraits<NodePtrT,NEXT_LINK_INDEX> nextlink;

public:
    typedef typename nextlink::node_type node_type;
    typedef typename nextlink::node_ptr_type node_ptr_type;
    typedef typename nextlink::const_node_ptr_type const_node_ptr_type;

private:
    mint_atomicPtr_t top_;

#if (QW_VALIDATE_NODE_LINKS == 1)
    void CHECK_NODE_IS_UNLINKED( const_node_ptr_type n ) const
    {
#ifndef NDEBUG
        assert( nextlink::is_unlinked(n) == true );
        assert( n != static_cast<node_ptr_type>(mint_load_ptr_relaxed(&top_)) );
        // Note: we can't check that the node is not referenced by some other list
#else
        if(!( nextlink::is_unlinked(n) == true )) { std::abort(); }
        if(!( n != static_cast<node_ptr_type>(mint_load_ptr_relaxed(&top_)) )) { std::abort(); }
#endif
    }

    void CLEAR_NODE_LINKS_FOR_VALIDATION( node_ptr_type n ) const
    {
        nextlink::clear(n);
    }
#else
    void CHECK_NODE_IS_UNLINKED( const_node_ptr_type ) const {}
    void CLEAR_NODE_LINKS_FOR_VALIDATION( node_ptr_type ) const {}
#endif

public:
    QwMpmcPopAllLifoStack()
    {
        top_._nonatomic = 0;
    }

    // The following four push variants use the same algorithm. Theh differ
    // only in whether one or multiple items are pushed, and whether they
    // provide the was-empty check.
    // push(n), push(n,&wasEmpty), push_multiple(a,b) and push_multiple(a,b,&wasEmpty)

    void push( node_ptr_type node )
    {
        CHECK_NODE_IS_UNLINKED( node );

        node_ptr_type top;
        do{
            top = static_cast<node_ptr_type>(mint_load_ptr_relaxed(&top_));
            nextlink::store(node, top);
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to client
            //   2. ensure that node->next <-- top is written before top <-- node
            mint_thread_fence_release();
        } while (mint_compare_exchange_strong_ptr_relaxed(&top_, top, node)!=top);
    }

    void push( node_ptr_type node, bool& wasEmpty )
    {
        CHECK_NODE_IS_UNLINKED( node );

        node_ptr_type top;
        do{
            top = static_cast<node_ptr_type>(mint_load_ptr_relaxed(&top_));
            nextlink::store(node, top);
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to client
            //   2. ensure that node->next <-- top is written before top <-- node
            mint_thread_fence_release();
        } while (mint_compare_exchange_strong_ptr_relaxed(&top_, top, node)!=top);

        wasEmpty = (top==0);
    }

    // push linked list link from front through to back
    void push_multiple( node_ptr_type front, node_ptr_type back )
    {
        CHECK_NODE_IS_UNLINKED( back );

        node_ptr_type top;
        do{
            top = static_cast<node_ptr_type>(mint_load_ptr_relaxed(&top_));
            nextlink::store(back, top);
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to client
            //   2. ensure that back->next <-- top is written before top <-- front
            mint_thread_fence_release();
        } while (mint_compare_exchange_strong_ptr_relaxed(&top_, top, front)!=top);
    }

    void push_multiple( node_ptr_type front, node_ptr_type back, bool& wasEmpty )
    {
        CHECK_NODE_IS_UNLINKED( back );

        node_ptr_type top;
        do{
            top = static_cast<node_ptr_type>(mint_load_ptr_relaxed(&top_));
            nextlink::store(back, top);
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to client
            //   2. ensure that back->next <-- top is written before top <-- front
            mint_thread_fence_release();
        } while (mint_compare_exchange_strong_ptr_relaxed(&top_, top, front)!=top);

        wasEmpty = (top==0);
    }

    bool empty() const
    {
        return (mint_load_ptr_relaxed(const_cast<mint_atomicPtr_t*>(&top_)) == 0);
    }

    node_ptr_type pop_all()
    {
        node_ptr_type result = static_cast<node_ptr_type>(qw_mint_exchange_ptr_relaxed( &top_, 0 )); // we'll return the first item
        mint_thread_fence_acquire(); // fence for all captured node data
        return result;
    }
};

#endif /* INCLUDED_QWMPMCPOPALLLIFOSTACK_H */

/* -----------------------------------------------------------------------
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
