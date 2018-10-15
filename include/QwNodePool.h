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
#ifndef INCLUDED_QWNODEPOOL_H
#define INCLUDED_QWNODEPOOL_H

#include <atomic>
#include <cassert>
#include <cstddef> // size_t
#include <cstdint>

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
    using size_t = std::size_t;
    using int8_t = std::int8_t;
    using ptrdiff_t = std::ptrdiff_t;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
    int8_t padding1_[CACHE_LINE_SIZE]; // avoid false sharing. TODO FIXME: give this more thought
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    int8_t *nodeStorage_;       // The raw memory buffer that is allocated and freed

    enum { NULL_NODE_INDEX=0 };
    int8_t *nodeArrayBase_;     // base ptr indexed by the packed pointer indexes. 1-based. nodeArrayBase_[0] should not be dereferenced
    size_t nodeSize_;           // nodes are allocated on cache-line boundaries. in this impl they also have power-of-two size
    int8_t nodeBitShift_;       // index=(ptr-nodeArrayBase_)>>nodeBitShift_; (nodeArrayBase_+(index<<nodeBitShift_)) == ptr

    //////////////////////////////////////////////////////////////////////
    // Packed pointer representation with ABA-prevention count.

    // important: must use uint to get correct wrap-around behavior on count,
    // because signed int overflow is undefined in C and C++
    typedef uint64_t abapointer_type; // (node-index, aba-count)
    typedef size_t nodeindex_type;
    typedef abapointer_type abacount_type;

    abapointer_type indexMask_;
    abapointer_type countMask_;
    abacount_type countIncrement_;

    nodeindex_type ap_index( abapointer_type ptr ) const {
        // index is in low bits, no shift needed
        return static_cast<nodeindex_type>(ptr & indexMask_);
    }

    abacount_type ap_count( abapointer_type ptr ) const {
        // count is in high bits, no shift needed because we always add countIncrement_, which is also shifted
        return static_cast<abacount_type>(ptr & countMask_);
    }

    abapointer_type make_abapointer( nodeindex_type index, abacount_type count ) const {
        return static_cast<abapointer_type>(index) | (static_cast<abapointer_type>(count)&countMask_);
    }

    // end packed pointer representation.
    //////////////////////////////////////////////////////////////////////

    std::atomic<std::uint64_t> top_; // must be large enough to store abapointer_type

#if (QW_DEBUG_COUNT_NODE_ALLOCATIONS == 1)
    std::atomic<std::int32_t> allocCount_;
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
    int8_t padding2_[CACHE_LINE_SIZE]; // avoid false sharing
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    // Node representation. Since this is a freelist, there is no node content.
    // When stored on the stack, each node contains a next index at the start:
    //
    //  Node {
    //     nodeindex_type next;
    //  }

    // node->next = x; --> node_next_lvalue(node) = x
    nodeindex_type& node_next_lvalue(void *node) const
    {
        return *static_cast<nodeindex_type*>(node);
    }

    // x = node->next; --> x = node_next(node)
    nodeindex_type node_next(void *node) const // when stored on the stack, each node contains a next index at the start
    {
        return *static_cast<nodeindex_type*>(node);
    }

    // convert a node pointer to an array index
    nodeindex_type index_of_node(void *node)
    {
        ptrdiff_t i = (static_cast<int8_t*>(node) - nodeArrayBase_) >> nodeBitShift_;
        return static_cast<nodeindex_type>(i);
    }

    // convert array index to a pointer
    void *node_at_index(nodeindex_type index)
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
        top_.store(make_abapointer(NULL_NODE_INDEX, 0), std::memory_order_relaxed);
    }

    // thread unsafe non-atomic version for construction time
    void stack_push_nonatomic( void *node )
    {
        assert( node != nullptr );
        nodeindex_type nodeIndex = index_of_node(node);
        node_next_lvalue(node) = ap_index(top_.load(std::memory_order_relaxed)); // Link new node to head of list (node.next <- top.ptr)
        top_.store(make_abapointer(nodeIndex,0), std::memory_order_relaxed); // Set top to new node. no need for ABA counter during thread-unsafe code
    }

    void stack_push( void *node )
    {
        assert( node != nullptr );
        nodeindex_type nodeIndex = index_of_node(node);

        abapointer_type top = top_.load(std::memory_order_relaxed); // Read top.ptr and top.count together (also done by compare_exchange_strong upon failure)
        do {                                            // Keep trying until push is done
            node_next_lvalue(node) = ap_index(top);     // Link new node to head of list (node.next <- top.ptr)
            // Try to swing top to the new node:
        } while (top_.compare_exchange_strong(top, make_abapointer(nodeIndex, ap_count(top)+countIncrement_),
                /*success:*/ std::memory_order_release, // (Ensure node.next is visible to consumers)
                /*failure:*/ std::memory_order_relaxed) == false);
    }

    void *stack_pop()
    {
        abapointer_type top = top_.load(); // Read top (explicitly fenced below)
        void *node;
        do {                                            // Keep trying until pop is done
            std::atomic_thread_fence(std::memory_order_acquire); // Acquire top.next, accessed by node_next(node) below.
            nodeindex_type nodeIndex = ap_index(top);
            if (nodeIndex==NULL_NODE_INDEX)             // Is the stack empty?
                return nullptr;                         // The stack was empty, couldn't pop
            // Try to swing top to the next node:
            node = node_at_index(nodeIndex);
        } while (top_.compare_exchange_strong(top, make_abapointer(node_next(node), ap_count(top)+countIncrement_),
                /*success:*/ std::memory_order_relaxed,
                /*failure:*/ std::memory_order_relaxed) == false); // it would be nice to use std::memory_order_acquire here, but C++11 says we can't.
        // BUG: in C++11, node->next should be an atomic field, but it is not.
        // Under the C++11 memory model, unless node.next is atomic, the read performed by node_next(node) may be a data race (triggers UB)
        // [Consider the following case:
        //      Thread A loads top pointer (top_.load() through to assignment of node in the code above)
        //                                  Thread B loads top pointer (top_.load() through to assignment of node in the code above)
        //      Thread A pops node pointed to by top (stack_pop() pops and returns the node)
        //      Thread A sets node->next
        //                                  Thread B reads node->next (node_next(node) in the code above)
        // The final operations of thread A and B combine to form a data race.]
        // This is discussed further here: https://stackoverflow.com/questions/46415027/c-treiber-stack-and-atomic-next-pointers
        return node;
    }

public:
    QwRawNodePool( size_t nodeSize, size_t maxNodes );
    ~QwRawNodePool();

    void *allocate()
    {
        void *result = stack_pop();

#if (QW_DEBUG_COUNT_NODE_ALLOCATIONS == 1)
        if (result)
            allocCount_.fetch_add(1, std::memory_order_relaxed);
#endif
        return result;
    }

    void deallocate( void *node )
    {
#if (QW_DEBUG_COUNT_NODE_ALLOCATIONS == 1)
        allocCount_.fetch_add(-1, std::memory_order_relaxed);
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
            return nullptr;
        // BUG: don't placement new to re-allocate the node -- in C++11, node objects
        // should be type-stable in order to avoid a strict aliasing violation
        // when QwRawNodePool::stack_pop() reads the next ptr of a node that
        // has already been popped by another thread.
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
