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
#ifndef INCLUDED_QWLINKTRAITS_H
#define INCLUDED_QWLINKTRAITS_H

#include "QwConfig.h"

#include <atomic>
#include <cstddef> // std::size_t
#include <memory> // std::addressof
#include <type_traits> // std::remove_ptr

/*
    QwLinkTraits is an adapter that linked list and node-based queue classes
    use to access the "next link" pointer (or sometimes, "previous link" pointer)
    stored in each node.

    QwLinkTraits is parameterised by Node type and link index. An instantiation
    of QwLinkTraits thus provides access to a single link field of a particular
    node type using QwLinkTraits::load() and QwLinkTraits::store() functions.
    If the link supports atomic operations, QwLinkTraits::is_atomic == true
    and QwLinkTraits also provides atomic_load() and atomic_store() functions.

    The default implementation of QwLinkTraits requires that the Node type has
    a publicly accessible array of link pointers named `links_`.
    The zero-based LINK_INDEX template parameter determines the element of
    the `links_` array that is operated upon.

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

    This link array representation makes it easy to declare Nodes that provide
    multiple links for simultaneous use by multiple lists and queues. Perhaps
    more importantly, it also allows the same link to be "overloaded" for
    multiple uses, and when this is done, it is clear from the enum values that
    the index is being overloaded (see link index 0 in the example above).

    Clients can specialize QwLinkTraits to support alternative link
    representations, or to customize link access.

    The QwLinkTraits template defined in this file is the default/base-case
    template. The base-case template delegates to either QwDefaultAtomicLinkTraitsImpl or
    QwDefaultNonAtomicLinkTraitsImpl depending on whether or not the Node's links_ are
    declared as std::atomic.
*/

// -----------------------------------------------------------------------

namespace Qw {
namespace impl {

template<typename node_type, int LINK_INDEX>
inline static std::size_t offsetof_link()
{
#if __cplusplus >= 201103L // C++11 version.
    // The following is well defined according to discussion and links here:
    // https://stackoverflow.com/questions/49775980/is-it-well-defined-to-use-stdaddressof-on-non-active-union-members
    // I prefer this to offsetof() because offsetof() gives warnings in C++11 for non-standard-layout types.
    // Arguably should use offsetof() in C++17 and later where it is conditionally supported for other types.
    union U {
        char c;
        node_type n;
        constexpr U() : c(0) {}
    };
    static const U u; // REVIEW: do we get better compiler output by making this non-static?
    return static_cast<std::size_t>(
            reinterpret_cast<const char*>(std::addressof(u.n.links_[LINK_INDEX])) -
            reinterpret_cast<const char*>(&u));
    // notes:
    // - the union layout rules guarantee that &u == &u.n (use &u because we know operator& isn't defined)
    // - use std::addressof to avoid problems with pointer-like objects that define operator&
#else
    return offsetof(node_type, links_[LINK_INDEX]);
#endif
}

} } // end namespace Qw::impl

template<typename NodePtrT, int LINK_INDEX>
struct QwDefaultNonAtomicLinkTraitsImpl {
    typedef typename std::remove_pointer<NodePtrT>::type node_type;
    typedef NodePtrT node_ptr_type;
    typedef const node_type* const_node_ptr_type;

    static constexpr bool is_atomic = false;

    static node_ptr_type load( const_node_ptr_type n )
    {
        return static_cast<node_ptr_type>(n->links_[ LINK_INDEX ]); // downcast to node_ptr_type in case links_[n] is a ptr to base class type
    }

    static void store( node_ptr_type n, node_ptr_type x ) // n->link = x
    {
        n->links_[ LINK_INDEX ] = x;
    }

    static std::size_t offsetof_link()
    {
        return Qw::impl::offsetof_link<node_type, LINK_INDEX>();
    }
};

// -----------------------------------------------------------------------

template<typename NodePtrT, int LINK_INDEX>
struct QwDefaultAtomicLinkTraitsImpl {
    typedef typename std::remove_pointer<NodePtrT>::type node_type;
    typedef NodePtrT node_ptr_type;
    typedef const node_type* const_node_ptr_type;

    // atomic accessors

    // is_atomic=true indicates that the link can be loaded and stored atomically
    // using the atomic_load() and atomic_store() functions.
    // Containers that require atomic link access check this tag at compile time.
    static constexpr bool is_atomic = true;

    static node_ptr_type atomic_load( const_node_ptr_type n, std::memory_order order ) // downcast to node_ptr_type in case links_[n] is a ptr to base class type
    {
        return static_cast<node_ptr_type>(n->links_[LINK_INDEX].load(order));
    }

    static void atomic_store( node_ptr_type n, node_ptr_type x, std::memory_order order ) // n->link = x
    {
        n->links_[LINK_INDEX].store(x, order);
    }

    // non-atomic accessors

    // IMPORTANT: if is_atomic == true, then all link accesses must be atomic,
    // not just atomic_load() and atomic_store() (this is because simultaneous
    // atomic and non-atomic access violates strict aliasing rules.)
    // Use memory_order_relaxed for the non-atomic accessors, since these will
    // be invoked by single-threaded containers.

    static node_ptr_type load( const_node_ptr_type n )
    {
        return atomic_load(n, std::memory_order_relaxed);
    }

    static void store( node_ptr_type n, node_ptr_type x ) // n->link = x
    {
        atomic_store(n, x, std::memory_order_relaxed);
    }

    static std::size_t offsetof_link()
    {
        return Qw::impl::offsetof_link<node_type, LINK_INDEX>();
    }
};

// -----------------------------------------------------------------------

namespace Qw {
namespace impl {
    template<typename T>
    struct is_std_atomic_ : std::false_type {};

    template<typename T>
    struct is_std_atomic_<std::atomic<T> > : std::true_type {};
} } // end namespace Qw::impl

template<typename NodePtrT, int LINK_INDEX, typename Enabled=void>
struct QwLinkTraits : std::conditional<
        Qw::impl::is_std_atomic_<typename std::remove_extent<decltype(std::remove_pointer<NodePtrT>::type::links_)>::type>::value,
        QwDefaultAtomicLinkTraitsImpl<NodePtrT, LINK_INDEX>,
        QwDefaultNonAtomicLinkTraitsImpl<NodePtrT, LINK_INDEX>
    >::type {};

#endif /* INCLUDED_QWLINKTRAITS_H */

/* -----------------------------------------------------------------------
Last reviewed: October 13, 2018
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
