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

#include "mintomic/mintomic.h"
#include "qw_atomic.h"

#include "QwConfig.h"
#include "QwSingleLinkNodeInfo.h"

/*
    QwMPMCPopAllLifoStack is a lock-free concurrent LIFO stack that
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
class QwMPMCPopAllLifoStack{
    typedef QwSingleLinkNodeInfo<NodePtrT,NEXT_LINK_INDEX> nodeinfo;

    mint_atomicPtr_t top_;

public:
    typedef typename nodeinfo::node_type node_type;
    typedef typename nodeinfo::node_ptr_type node_ptr_type;
    typedef typename nodeinfo::const_node_ptr_type const_node_ptr_type;
    
    QwMPMCPopAllLifoStack()
    {
        top_._nonatomic = 0;
    }

    void push( node_ptr_type node )
    {
        nodeinfo::check_node_is_unlinked( node );

        node_ptr_type top;
        do{
            top = static_cast<node_ptr_type>(mint_load_ptr_relaxed(&top_));
            nodeinfo::next_ptr(node) = top;
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to client
            //   2. ensure that node->next <-- top is written before top <-- node
            mint_thread_fence_release();
        } while (mint_compare_exchange_strong_ptr_relaxed(&top_, top, node)!=top);
    }

    void push( node_ptr_type node, bool& wasEmpty )
    {
        nodeinfo::check_node_is_unlinked( node );

        node_ptr_type top;
        do{
            top = static_cast<node_ptr_type>(mint_load_ptr_relaxed(&top_));
            nodeinfo::next_ptr(node) = top;
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
        nodeinfo::check_node_is_unlinked( back );

        node_ptr_type top;
        do{
            top = static_cast<node_ptr_type>(mint_load_ptr_relaxed(&top_));
            nodeinfo::next_ptr(back) = top;
            // A fence is needed here for two reasons:
            //   1. so that node's payload gets written before node becomes visible to client
            //   2. ensure that node->next <-- top is written before top <-- node
            mint_thread_fence_release();
        } while (mint_compare_exchange_strong_ptr_relaxed(&top_, top, front)!=top);
    }
    
    bool empty() const
    {
        return (mint_load_ptr_relaxed(const_cast<mint_atomicPtr_t*>(&top_)) == 0);
    }

    node_ptr_type pop_all()
    {
        node_ptr_type result = (node_ptr_type)qw_mint_exchange_ptr_relaxed( &top_, 0 ); // we'll return the first item
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
