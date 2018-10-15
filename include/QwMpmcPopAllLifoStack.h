/*
    Queue World is copyright (c) 2014-2018 Ross Bencina

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

#include <atomic>
#include <cassert>
#ifdef NDEBUG
#include <cstdlib> // abort
#endif

#include "QwConfig.h"
#include "QwLinkTraits.h"

/*
    QwMpmcPopAllLifoStack is a lock-free concurrent LIFO stack that
    provides push() and pop_all()  operations. No single-node pop()
    operation is provided.

    All operations may be invoked concurrently.

    Implemented using the "IBM Freelist" LIFO algorithm.

    The algorithm doesn't need ABA protection because it does not provide a pop() operation.
    pop_all() is not subject to the ABA problem because it swaps in a nullptr value and never
    requires comparison to a non-nullptr value.
    See ALGORITHMS.txt
*/

template<typename NodePtrT, int NEXT_LINK_INDEX>
class QwMpmcPopAllLifoStack{
    typedef QwLinkTraits<NodePtrT,NEXT_LINK_INDEX> nextlink;
    // Note: there is no requirement for nextlink to be atomic, since
    // it is never accessed from multiple threads simultaneously, and
    // transmission between threads is always mediated by an atomic
    // compare exchange with appropriate memory barrier.

public:
    typedef typename nextlink::node_type node_type;
    typedef typename nextlink::node_ptr_type node_ptr_type;
    typedef typename nextlink::const_node_ptr_type const_node_ptr_type;

private:
    std::atomic<node_ptr_type> top_;

#if (QW_VALIDATE_NODE_LINKS == 1)
    void CHECK_NODE_IS_UNLINKED( const_node_ptr_type n ) const
    {
#ifndef NDEBUG
        assert( nextlink::load(n) == nullptr ); // (require unlinked)
        // Node could be unlinked (nullptr next ptr) but still at top of stack; check that:
        assert( n != top_.load(std::memory_order_relaxed) );
        // Note: we can't check that the node is not referenced by some other list
#else
        if(!( nextlink::load(n) == nullptr )) { std::abort(); } // (require unlinked)
        if(!( n != top_.load(std::memory_order_relaxed) )) { std::abort(); }
#endif
    }

    void CLEAR_NODE_LINKS_FOR_VALIDATION( node_ptr_type n ) const
    {
        nextlink::store(n, nullptr);
    }
#else
    void CHECK_NODE_IS_UNLINKED( const_node_ptr_type ) const {}
    void CLEAR_NODE_LINKS_FOR_VALIDATION( node_ptr_type ) const {}
#endif

public:
    QwMpmcPopAllLifoStack()
        : top_(nullptr)
    {}

    // The following four push variants use the same algorithm. Theh differ
    // only in whether one or multiple items are pushed, and whether they
    // provide the was-empty check.
    // push(n), push(n,&wasEmpty), push_multiple(a,b) and push_multiple(a,b,&wasEmpty)

    void push( node_ptr_type node )
    {
        CHECK_NODE_IS_UNLINKED( node );

        node_ptr_type top = top_.load(std::memory_order_relaxed);
        do{
            nextlink::store(node, top);
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to consumer
            //   2. ensure that node->next <-- top is written before top <-- node
        } while (top_.compare_exchange_strong(top, node,
                /*success:*/ std::memory_order_release,
                /*failure:*/ std::memory_order_relaxed) == false);
    }

    void push( node_ptr_type node, bool& wasEmpty )
    {
        CHECK_NODE_IS_UNLINKED( node );

        node_ptr_type top = top_.load(std::memory_order_relaxed);
        do{
            nextlink::store(node, top);
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to consumer
            //   2. ensure that node->next <-- top is written before top <-- node
        } while (top_.compare_exchange_strong(top, node,
                /*success:*/ std::memory_order_release,
                /*failure:*/ std::memory_order_relaxed) == false);

        wasEmpty = (top==nullptr);
    }

    // push linked list link from front through to back
    void push_multiple( node_ptr_type front, node_ptr_type back )
    {
        CHECK_NODE_IS_UNLINKED( back );

        node_ptr_type top = top_.load(std::memory_order_relaxed);
        do{
            nextlink::store(back, top);
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to consumer
            //   2. ensure that back->next <-- top is written before top <-- front
        } while (top_.compare_exchange_strong(top, front,
                /*success:*/ std::memory_order_release,
                /*failure:*/ std::memory_order_relaxed) == false);
    }

    void push_multiple( node_ptr_type front, node_ptr_type back, bool& wasEmpty )
    {
        CHECK_NODE_IS_UNLINKED( back );

        node_ptr_type top = top_.load(std::memory_order_relaxed);
        do{
            nextlink::store(back, top);
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to consumer
            //   2. ensure that back->next <-- top is written before top <-- front
        } while (top_.compare_exchange_strong(top, front,
                /*success:*/ std::memory_order_release,
                /*failure:*/ std::memory_order_relaxed) == false);

        wasEmpty = (top==nullptr);
    }

    bool empty() const
    {
        return (top_.load(std::memory_order_relaxed) == nullptr);
    }

    node_ptr_type pop_all()
    {
        // acquire fence for all captured node data
        return top_.exchange(nullptr, std::memory_order_acquire); // we'll return the first item
    }
};

#endif /* INCLUDED_QWMPMCPOPALLLIFOSTACK_H */

/* -----------------------------------------------------------------------
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
