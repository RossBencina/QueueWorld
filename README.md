QUEUE WORLD IS UNDER CONSTRUCTION – please check back soon : )

Queue World
===========

_A light-weight C++ toolkit for building concurrent, lock-free, message-passing programs._

Queue World is a little collection of light-weight C++ data structures for lock-free multi-threaded programming. The focus is on providing simple primitives using well-known algorithms. Queue World makes heavy use of endogenous linking (links embedded in client structures). This avoids allocator overhead, which is especially important in time-critical code. In addition to concurrent queues, a variety of non-re-entrant endogenous linked-lists are also provided.


Concurrent data structures
--------------------------

**QwMpmcPopAllLifoStack** -- a multiple producer multiple consumer LIFO stack that supports push() and pop_all() operations, but not pop().

**QwMpscFifoQueue** -- a multiple producer single consumer FIFO stack. Useful for a server thread that receives requests sent from many client threads.

**QwSpscUnorderedResultQueue** -- a single producer single consumer "relaxed order" queue for returning results from a server thread to a client. Includes a client-side counter for tracking expected vs. received results.

**QwNodePool** -- a concurrent freelist that allocates and frees fixed-size nodes from a fixed-size node pool. Guarantees cache-line alignment of each node to avoid false sharing.


Single threaded (non-reentrant) data structures
-----------------------------------------------

**QwSList** -- a singly linked list

**QwSTailList** -- a singly linked tail list (supports constant-time insertion at back)

**QwList** -- a doubly linked list

The single threaded data structures provide an STL-like interface.


Philosophy
----------

Queue World is envisaged as an inter-thread communication library for real-time audio applications, where mutexes are not an option due to the risk of priority inversion. The typical use-cases involve relatively low queue contention. So far, the main goals have been to keep things simple and to provide infrastructure for avoiding priority inversion.

If you're looking for water-tight abstractions you might be in the wrong place. The goal here is simple and efficient implementations, even if that means having a few "pipes on the outside of the building". This is most strongly reflected in the use of endogenous linking.


Implementation Details
----------------------

For simplicity of implementation, the lock-free data structures are currently built out of variations on the well-known "IBM Freelist" (see ALGORITHMS.txt for details). The main advantage of this approach is that it avoids the need to manage additional link nodes.

In the future we plan to experiment with other algorithms and to evaluate performance. In particular, an MS-queue is desirable for use as an MPMC FIFO.

Queue World uses Mintomic (https://github.com/mintomic) for atomic operations and memory barriers. Queue world does not require the availability of C++11 atomics.

All Queue World classes are provided with unit tests written using Catch (https://github.com/philsquared/Catch).


Licence
-------

Queue World is copyright (c) 2014 Ross Bencina.

Queue World is licensed under The MIT Licence: http://opensource.org/licenses/MIT
