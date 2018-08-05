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
#ifndef INCLUDED_QWSINGLELINKNODEINFO_H
#define INCLUDED_QWSINGLELINKNODEINFO_H

#include "qw_remove_pointer.h"

#include "QwConfig.h"

#include <cstddef> // std::size_t
#include <memory> // std::memory

/*
    QwLinkTraits is an adapter that queue and list classes
    use to access the "next link" stored in each node. It requires that
    the Node type has a links_[] array, and uses the LINK_INDEX
    template parameter to determine which link to use.

    The intent is that clients will declare an enum indicating the role(s)
    of each link. e.g.:

        struct Node {

            enum LinkIndices {
                ACTOR_MESSAGE_QUEUE_LINK = 0,
                CLIENT_WHEN_NOT_SENT_LINK_INDEX = 0,
                SERVER_INTERNAL_QUEUEING_LINK_INDEX = 0,
                REQUEST_CHAINING_LINK_WHEN_SENT = 1,
                CLIENT_EXCLUSIVE_LINK_INDEX = 2,
                LINK_COUNT = 3
            };

            Node *links_[LINK_COUNT];

            ...
        };

    Although it is not perfect for all uses, we chose this
    link array representation because it makes it easy to declare
    Nodes that use multiple links. Perhaps more importantly: it allows
    the same link to be "overloaded" for multiple uses, and when this is done,
    it is clear from the enum values that the index is being overloaded
    (see link index 0 in the above example).

    Clients can specialize QwLinkTraits to support alternative link representations.
*/
template<typename NodePtrT, int LINK_INDEX>
struct QwLinkTraits {
    typedef std::size_t size_t;

    typedef typename qw_remove_pointer<NodePtrT>::type node_type;
    typedef NodePtrT node_ptr_type;
    typedef const node_type* const_node_ptr_type;

    static node_ptr_type load( const_node_ptr_type n )
    {
        return static_cast<node_ptr_type>(n->links_[ LINK_INDEX ]); // downcast to node_ptr_type in case links_[n] is a ptr to base class type
    }

    static void store( node_ptr_type n, node_ptr_type x ) // n->link = x
    {
        n->links_[ LINK_INDEX ] = x;
    }

    static size_t offsetof_link()
    {
        // The following is well defined according to discussion and links here:
        // https://stackoverflow.com/questions/49775980/is-it-well-defined-to-use-stdaddressof-on-non-active-union-members
        // I prefer this to offsetof() because offsetof() gives warnings in C++11 for non-standard-layout types
        // Arguably should use offsetof() in C++17 and later where it is conditionally supported for other types.
        union U {
            char c;
            node_type n;
            constexpr U() : c(0) {}
        };
        static const U u;
        return static_cast<size_t>(
                reinterpret_cast<const char*>(std::addressof(u.n.links_[LINK_INDEX])) -
                reinterpret_cast<const char*>(&u));
        // notes:
        // - the union layout rules guarantee that &u == &u.n (use &u because we know operator& isn't defined)
        // - use std::addressof to avoid problems with pointer-like objects that define operator&
    }
};

#endif /* INCLUDED_SINGLELINKNODEINFO_H */

/* -----------------------------------------------------------------------
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
