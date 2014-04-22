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
#ifdef QW_VALIDATE_NODE_LINKS
#include <cassert>
#endif

/*
    QwSingleLinkNodeInfo is an adapter that queue and list classes
    use to access the "next link" stored in each node. It requires that
    the Node type has a links_[] array, and uses the NEXT_LINK_INDEX
    template parameter to determine which link to use.

    Although it is not perfect for all uses, we chose this
    link array representation because it makes it easy to declare
    Nodes that use multiple links. It allows the same link to be
    "overloaded" for multiple uses, and when this is done, it is
    clear from the enum values that the index is being overloaded.

    In the future we'll probably switch to parameterising the
    queue classes by the node info structure to allow alternate
    node info implementations for discontiguous link layouts.
*/
template<typename NodePtrT, int NEXT_LINK_INDEX>
struct QwSingleLinkNodeInfo {

    typedef typename qw_remove_pointer<NodePtrT>::type node_type;

    typedef NodePtrT node_ptr_type;
    typedef const node_type* const_node_ptr_type;

    static node_ptr_type& next_ptr( node_ptr_type& n )
    {
        return n->links_[ NEXT_LINK_INDEX ];
    }

    static const node_ptr_type& next_ptr( const node_ptr_type& n )
    {
        return n->links_[ NEXT_LINK_INDEX ];
    }

    static const size_t offsetof_next_ptr()
    {
        return (size_t)&next_ptr(((node_ptr_type)0));
    }

#ifdef QW_VALIDATE_NODE_LINKS
    static void check_node_is_unlinked( const node_ptr_type n )
    {
        assert( next_ptr(n) == 0 );

        // if the list has only one element n->next will be 0 but could 
        // still be in our list (or another list, but we can only check our list) 
        //assert( n != COUNTPTR_PTR(top_) ); 
    }

    static void clear_node_link_for_validation( node_ptr_type n )
    {
        next_ptr(n) = 0;
    }
#else
    static void check_node_is_unlinked( const node_ptr_type n ) {}
    static void clear_node_link_for_validation( node_ptr_type n ) {}
#endif
};

#endif /* INCLUDED_SINGLELINKNODEINFO_H */

/* -----------------------------------------------------------------------
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
