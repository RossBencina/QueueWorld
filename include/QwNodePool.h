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
#ifndef INCLUDED_QWNODEPOOL_H
#define INCLUDED_QWNODEPOOL_H

#include <cassert>

#include "mintomic/mintomic.h"

#include "QwConfig.h"

/*
    QwNodePool provides a thread-safe, lock-free fixed-size pool of
    fixed-size "nodes" -- memory blocks that are usually linked into
    Queue World data structures such as lists and queues.

    Nodes can be allocated and deallocated from any thread. Even
    real-time audio threads.

    QwNodePool ensures that all nodes are aligned to cache line boundaries
    to avoid false sharing.

    The implementation uses the "IBM Freelist" lock-free stack algorithm.
    See ALGORITHMS.txt

    This implementation may not be the most efficient. However, it is
    portable to 64-bit systems that lack 128-bit CAS. Tagged pointers
    are packed into 64 bit words as (count,index), where index can be
    converted to a node pointer into the nodeArrayBase_ array.
    More efficient alternative implementations may be provided later.

    IDEA: if we wanted to support an expandable request pool,
    the index space could be partitioned across multiple base arrays.
    Additional arrays need only be allocated on demand.
*/

class QwRawNodePool {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
    int8_t padding1_[CACHE_LINE_SIZE]; // avoid false sharing. TODO FIXME: give this more thought
#pragma GCC diagnostic pop

    int8_t *nodeStorage_;       // The raw memory buffer that is allocated and freed

    enum { NULL_NODE_INDEX=0 };
    int8_t *nodeArrayBase_;     // base ptr indexed by the packed pointer indexes. 1-based. nodeArrayBase_[0] should not be dereferenced
    size_t nodeSize_;           // nodes are allocated on cache-line boundaries. in this impl they also have power-of-two size
    int8_t nodeBitShift_;       // index=(ptr-nodeArrayBase_)>>nodeBitShift_; (nodeArrayBase_+(index<<nodeBitShift_)) == ptr

    //////////////////////////////////////////////////////////////////////
    // Packed pointer representation with ABA-prevention count.

    // important: must use uint to get correct wrap-around behavior on count,
    // because signed int overflow is undefined in C and C++
    typedef uint64_t abapointer_t; // (node-index, aba-count)
    typedef size_t nodeindex_t;
    typedef abapointer_t abacount_t;

    abapointer_t indexMask_;
    abapointer_t countMask_;
    abacount_t countIncrement_;

    nodeindex_t ap_index( abapointer_t ptr ) const {
        // index is in low bits, no shift needed
        return static_cast<nodeindex_t>(ptr & indexMask_);
    }

    abacount_t ap_count( abapointer_t ptr ) const {
        // count is in high bits, no shift needed because we always add countIncrement_, which is also shifted
        return static_cast<abacount_t>(ptr & countMask_);
    }

    abapointer_t make_abapointer( nodeindex_t index, abacount_t count ) const {
        return static_cast<abapointer_t>(index) | (static_cast<abapointer_t>(count)&countMask_);
    }

    // end packed pointer representation.
    //////////////////////////////////////////////////////////////////////

    mint_atomic64_t top_; // must be large enough to store abapointer_t

#ifdef QW_DEBUG_COUNT_NODE_ALLOCATIONS
    mint_atomic32_t allocCount_;
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
    int8_t padding2_[CACHE_LINE_SIZE]; // avoid false sharing
#pragma GCC diagnostic pop

    // Node representation. Since this is a freelist, there is no node content.
    // When stored on the stack, each node contains a next index at the start:
    //
    //  Node {
    //     nodeindex_t next;
    //  }

    // node->next = x; --> node_next_lvalue(node) = x
    nodeindex_t& node_next_lvalue(void *node) const
    {
        return *static_cast<nodeindex_t*>(node);
    }

    // x = node->next; --> x = node_next(node)
    nodeindex_t node_next(void *node) const // when stored on the stack, each node contains a next index at the start
    {
        return *static_cast<nodeindex_t*>(node);
    }

    // convert a node pointer to an array index
    nodeindex_t index_of_node(void *node)
    {
        ptrdiff_t i = (static_cast<int8_t*>(node) - nodeArrayBase_) >> nodeBitShift_;
        return static_cast<nodeindex_t>(i);
    }

    // convert array index to a pointer
    void *node_at_index(nodeindex_t index)
    {
        int8_t *p = nodeArrayBase_ + (static_cast<ptrdiff_t>(index) << nodeBitShift_);
        return p;
    }

    // init, push and pop follow the pseudocode of Michael and Scott 97
    // see ALGORITHMS.txt for details
    //
    // The implementation is slightly obscured by the indirection via indices. Here's a decoder from MS97 to our code:
    //
    //  top.ptr     --> node_pointer(ap_index(top))
    //  top.next    --> node_next(node_pointer(ap_index(top)))
    //  top.count   --> ap_count(top)
    //
    //  We directly push and pop nodes with embedded next links. We don't separately allocate nodes.

    void stack_init()
    {
        top_._nonatomic = make_abapointer(NULL_NODE_INDEX, 0);
    }

    // thread unsafe non-atomic version for construction time
    void stack_push_nonatomic( void *node )
    {
        assert( node != 0 );
        nodeindex_t nodeIndex = index_of_node(node);
        node_next_lvalue(node) = ap_index(top_._nonatomic); // Link new node to head of list (node.next <- top.ptr)
        top_._nonatomic = make_abapointer(nodeIndex,0); // Set top to new node. no need for ABA counter during thread-unsafe code
    }

    void stack_push( void *node )
    {
        assert( node != 0 );
        nodeindex_t nodeIndex = index_of_node(node);

        abapointer_t top;
        do {                                        // Keep trying until push is done
            top = mint_load_64_relaxed(&top_);      // Read top.ptr and top.count together
            node_next_lvalue(node) = ap_index(top); // Link new node to head of list (node.next <- top.ptr)
            mint_thread_fence_release();            // (Ensure node.next is visible to consumers)
            // Try to swing top to the new node:
        } while (mint_compare_exchange_strong_64_relaxed(&top_, top, make_abapointer(nodeIndex,ap_count(top)+countIncrement_))!=top);
    }

    void *stack_pop()
    {
        abapointer_t top;
        void *node;
        do {                                        // Keep trying until pop is done
            top = mint_load_64_relaxed(&top_);      // Read top
            mint_thread_fence_acquire();            // (Acquire top.next)
            nodeindex_t nodeIndex = ap_index(top);
            if (nodeIndex==NULL_NODE_INDEX)         // Is the stack empty?
                return 0;                           // The stack was empty, couldn't pop
            // Try to swing top to the next node:
            node = node_at_index(nodeIndex);
        } while (mint_compare_exchange_strong_64_relaxed(&top_, top, make_abapointer(node_next(node),ap_count(top)+countIncrement_))!=top);

        return node;
    }

public:
    QwRawNodePool( size_t nodeSize, size_t maxNodes );
    ~QwRawNodePool();

    void *allocate()
    {
        void *result = stack_pop();

#ifdef QW_DEBUG_COUNT_NODE_ALLOCATIONS
        if (result)
            mint_fetch_add_32_relaxed(&allocCount_,1);
#endif
        return result;
    }

    void deallocate( void *node )
    {
#ifdef QW_DEBUG_COUNT_NODE_ALLOCATIONS
        mint_fetch_add_32_relaxed(&allocCount_,-1);
#endif
        stack_push(node);
    }
};


template<typename NodeT>
class QwNodePool{
    QwRawNodePool rawPool_;
public:

    typedef NodeT node_type;

    QwNodePool( size_t maxNodes )
        : rawPool_( sizeof(NodeT), maxNodes )
    {}

    node_type *allocate()
    {
        void *p = rawPool_.allocate();
        if (!p)
            return 0;
        return new (p) node_type();
    }

    void deallocate( node_type *p )
    {
        p->~node_type();
        rawPool_.deallocate(p);
    }
};

#endif /* INCLUDED_QWNODEPOOL_H */

/* -----------------------------------------------------------------------
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
