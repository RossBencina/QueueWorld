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
#ifndef INCLUDED_QWSPSCUNORDEREDRESULTQUEUE_H
#define INCLUDED_QWSPSCUNORDEREDRESULTQUEUE_H

#include <cassert>
#ifdef NDEBUG
#include <cstdlib> // abort
#endif

#include "mintomic/mintomic.h"
#include "qw_atomic.h"

#include "QwLinkTraits.h"


/*
    QwSpscUnorderedResultQueue is a lock-free concurrent, single-producer
    single-consumer (SPSC) wait-free relaxed-order queue.

    Used for returning results from a server to a client.
    We usually instantiate result queues inside Nodes/messages, which is why this is a POD.

    Producer operations: push()
    Consumer operations: pop(), expectedResultCount(), incrementExpectedResultCount()

    There may be only one producer and one consumer.

    All operations may be invoked concurrently.
*/

template<typename NodePtrT, int NEXT_LINK_INDEX>
class QwSpscUnorderedResultQueue{
    typedef QwLinkTraits<NodePtrT,NEXT_LINK_INDEX> nextlink;

public:
    typedef typename nextlink::node_type node_type;
    typedef typename nextlink::node_ptr_type node_ptr_type;
    typedef typename nextlink::const_node_ptr_type const_node_ptr_type;

private:
    mint_atomicPtr_t atomicLifoTop_; // LIFO. same algorithm as QwMpmcPopAllLifoStack. shared by producer and consumer
    node_ptr_type consumerLocalHead_; // LIFO order reader queue. only referenced by the consumer
    size_t expectedResultCount_; // consumer increments this when making a request, pop() decrements it

#if (QW_VALIDATE_NODE_LINKS == 1)
    void CHECK_NODE_IS_UNLINKED( const_node_ptr_type n ) const
    {
#ifndef NDEBUG
        assert( nextlink::is_unlinked(n) == true );
        assert( n != static_cast<node_ptr_type>(mint_load_ptr_relaxed(&atomicLifoTop_)) );
        assert( n != static_cast<node_ptr_type>(consumerLocalHead_) );
        // Note: we can't check that the node is not referenced by some other list
#else
        if(!( nextlink::is_unlinked(n) == true )) { std::abort(); }
        if(!( n != static_cast<node_ptr_type>(mint_load_ptr_relaxed(&atomicLifoTop_)) )) { std::abort(); }
        if(!( n != static_cast<node_ptr_type>(consumerLocalHead_) )) { std::abort(); }
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
    void init()
    {
        atomicLifoTop_._nonatomic = 0;
        consumerLocalHead_ = 0;
        expectedResultCount_ = 0;
    }

    void push( node_ptr_type node ) // called by producer
    {
        CHECK_NODE_IS_UNLINKED(node);

        // Single producer, push one item onto the atomic LIFO.

        // link node to point to current atomicLifoTop_
        nextlink::store(node, static_cast<node_ptr_type>(mint_load_ptr_relaxed(&atomicLifoTop_)));

        mint_thread_fence_release(); // fence for next ptr and item data of node
        mint_store_ptr_relaxed(&atomicLifoTop_, node); // push node onto head of atomic LIFO
    }

    node_ptr_type pop() // called by consumer
    {
        // Single consumer, pop one item: either from the consumer-local queue,
        // or by capturing a new segment from the atomic LIFO.

        if (consumerLocalHead_ == 0) {
            // consumer-local reader queue is empty, try to refresh it from the atomic LIFO
            if( mint_load_ptr_relaxed(&atomicLifoTop_) != 0 ){ // poll passively first to avoid unnecessarily locking the bus
                // there are new items in the atomic LIFO

                // capture all nodes from the atomic LIFO
                node_ptr_type result = static_cast<node_ptr_type>(qw_mint_exchange_ptr_relaxed( &atomicLifoTop_, 0 )); // we'll return the first item
                mint_thread_fence_acquire(); // fence for all captured node data

                // retain all but the first item for future pops
                consumerLocalHead_ = nextlink::load(result);

                CLEAR_NODE_LINKS_FOR_VALIDATION(result);
                assert( expectedResultCount_ > 0 );
                --expectedResultCount_;
                return result;
            }
            else
            {
                return 0; // no items available
            }
        } else {
            // consumer-local reader queue is non-empty. pop one item from consumerLocalHead_
            node_ptr_type result = consumerLocalHead_;
            consumerLocalHead_ = nextlink::load(result);

            CLEAR_NODE_LINKS_FOR_VALIDATION(result);
            assert( expectedResultCount_ > 0 );
            --expectedResultCount_;
            return result;
        }
    }

    // expectedResultCount getter and mutator to be called on consumer side only:

    size_t expectedResultCount() const { return expectedResultCount_; }

    void incrementExpectedResultCount() { ++expectedResultCount_; }
    void incrementExpectedResultCount( size_t k ) { expectedResultCount_ += k; }
};

#endif /* INCLUDED_QWSPSCUNORDEREDRESULTQUEUE_H */

/* -----------------------------------------------------------------------
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
