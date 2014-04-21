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

#include "mintomic/mintomic.h"
#include "qw_atomic.h"

#include "QwSingleLinkNodeInfo.h"


// Single-producer single-consumer (SPSC) wait-free relaxed-order queue
// Used for returning results from a server to a client. 
// We usually instantiate result queues inside Nodes, which is why this is a POD.

template<typename NodePtrT, int NEXT_LINK_INDEX>
class QwSpscUnorderedResultQueue{
    typedef QwSingleLinkNodeInfo<NodePtrT,NEXT_LINK_INDEX> nodeinfo;

public:
    typedef typename nodeinfo::node_type node_type;
    typedef typename nodeinfo::node_ptr_type node_ptr_type;
    typedef typename nodeinfo::const_node_ptr_type const_node_ptr_type;

private:
    mint_atomicPtr_t atomicLifoTop_; // LIFO. same algorithm as QwMpmcPopAllLifoStack. shared by producer and consumer
    node_ptr_type consumerLocalHead_; // LIFO order reader queue. only referenced by the consumer
    int expectedResultCount_; // consumer increments this when making a request, pop() decrements it

public:
    void init()
    {
        atomicLifoTop_._nonatomic = 0;
        consumerLocalHead_ = 0;
        expectedResultCount_ = 0;
    }

    void push( node_ptr_type node ) // called by producer
    {
        // Single producer, push one item onto the atomic lifo.

        // link node to point to current atomicLifoTop_
        nodeinfo::next_ptr(node) = static_cast<node_ptr_type>(mint_load_ptr_relaxed(&atomicLifoTop_));

        mint_thread_fence_release(); // fence for next ptr and item data of node
        mint_store_ptr_relaxed(&atomicLifoTop_, node); // push node onto head of atomic lifo
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
                node_ptr_type result = (node_ptr_type)qw_mint_exchange_ptr_relaxed( &atomicLifoTop_, 0 ); // we'll return the first item
                mint_thread_fence_acquire(); // fence for all captured node data

                // retain all but the first item for future pops
                consumerLocalHead_ = nodeinfo::next_ptr(result);

                nodeinfo::clear_node_link_for_validation(result);
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
            consumerLocalHead_ = nodeinfo::next_ptr(result);

            nodeinfo::clear_node_link_for_validation(result);
            assert( expectedResultCount_ > 0 );
            --expectedResultCount_;
            return result;
        }
    }

    // expectedResultCount accessor and manipulator to be called on consumer side only:

    int expectedResultCount() const { return expectedResultCount_; }

    void incrementExpectedResultCount() { ++expectedResultCount_; }
    void incrementExpectedResultCount( int k ) { expectedResultCount_ += k; }
};

#endif /* INCLUDED_QWSPSCUNORDEREDRESULTQUEUE_H */
