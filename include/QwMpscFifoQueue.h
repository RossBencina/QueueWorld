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
#ifndef INCLUDED_QWMPSCFIFOQUEUE_H
#define INCLUDED_QWMPSCFIFOQUEUE_H

#include "QwLinkTraits.h"
#include "QwMpmcPopAllLifoStack.h"
#include "QwSTailList.h"


/*
    QwMpscFifoQueue is a lock-free concurrent, multiple-producer single-consumer FIFO queue.

    Producer(s) operations: push()
    Consumer operations: consumer_empty(), pop().

    There may be multiple producers, but only one consumer.

    All operations may be invoked concurrently.

    Implemented using the "Reversed IBM Freelist" technique.
    See ALGORITHMS.txt
*/

template<typename NodePtrT, int NEXT_LINK_INDEX>
class QwMpscFifoQueue {
    typedef QwLinkTraits<NodePtrT, NEXT_LINK_INDEX> nextlink;

public:
    typedef typename nextlink::node_type node_type;
    typedef typename nextlink::node_ptr_type node_ptr_type;
    typedef typename nextlink::const_node_ptr_type const_node_ptr_type;

private:
    QwMpmcPopAllLifoStack<NodePtrT, NEXT_LINK_INDEX> mpscLifo_;
    QwSTailList<NodePtrT, NEXT_LINK_INDEX> consumerLocalReversingQueue_;

#if (QW_VALIDATE_NODE_LINKS == 1)
    void CLEAR_NODE_LINKS_FOR_VALIDATION(node_ptr_type n) const
    {
        nextlink::store(n, nullptr);
    }
#else
    void CLEAR_NODE_LINKS_FOR_VALIDATION(node_ptr_type) const {}
#endif

public:
    void push(node_ptr_type n)
    {
        return mpscLifo_.push(n);
    }

    // KNOWNBUG: push and push_multiple indicates wasEmpty
    // even if the consumer local-queue is non-empty.
    // not sure that will be fixed.
    void push(node_ptr_type n, bool& wasEmpty)
    {
        return mpscLifo_.push(n, wasEmpty);
    }

    // NOTE: back will be the first item to be dequeued
    void push_multiple(node_ptr_type front, node_ptr_type back, bool& wasEmpty)
    {
        return mpscLifo_.push_multiple(front, back, wasEmpty);
    }

    bool consumer_empty() const
    {
        return (consumerLocalReversingQueue_.empty() && mpscLifo_.empty());
    }

    node_ptr_type pop()
    {
        if (consumerLocalReversingQueue_.empty()) {
            if (mpscLifo_.empty()) {
                return nullptr;
            } else {
                node_ptr_type n = mpscLifo_.pop_all();
                if (n) {
                    // push all but the last node popped from the lifo into consumerLocalReversingQueue_
                    // this reverses their order, putting them into fifo order.
                    node_ptr_type next = nextlink::load(n);
                    while (next != nullptr) {
                        CLEAR_NODE_LINKS_FOR_VALIDATION(n);
                        consumerLocalReversingQueue_.push_front(n);
                        n = next;
                        next = nextlink::load(n);
                    }
                    // return the last request, which is the next in fifo order
                    return n; // n->next is always zero here.
                } else {
                    return nullptr;
                }
            }

        } else {
            // (know consumerLocalReversingQueue_ is not empty here)
            return consumerLocalReversingQueue_.pop_front();
        }
    }
};

#endif /* INCLUDED_QWMPSCFIFOQUEUE_H */

/* -----------------------------------------------------------------------
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
Comments:
- push(n, wasEmpty) has a knownbug: wasEmpty is not accurate
-------------------------------------------------------------------------- */
