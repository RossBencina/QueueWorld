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
#include "QwMpscFifoQueue.h"

#include "catch.hpp"


namespace {

    struct TestNode{
        TestNode *links_[2];
        enum { LINK_INDEX_1, LINK_COUNT };

        int value;

        TestNode()
            : value( 0 )
        {
            for( int i=0; i < LINK_COUNT; ++i )
                links_[i] = nullptr;
        }
    };

    typedef QwMpscFifoQueue<TestNode*, TestNode::LINK_INDEX_1> TestMpscFifoQueue;

    TestNode*& next_(TestNode*n) { return n->links_[TestNode::LINK_INDEX_1]; }

} // end anonymous namespace


TEST_CASE( "qw/mpsc_fifo_queue", "QwMpscFifoQueue single threaded test" ) {

    TestNode nodes[4];
    TestNode *a = &nodes[0];
    TestNode *b = &nodes[1];
    TestNode *c = &nodes[2];
    TestNode *d = &nodes[3];

    TestMpscFifoQueue q;

    REQUIRE( q.consumer_empty() == true );
    REQUIRE( q.pop() == (TestNode*)nullptr );

    // void push( node_ptr_type n )
    // bool consumer_empty() const
    // node_ptr_type pop()

    q.push( a );
    REQUIRE( q.consumer_empty() == false );
    REQUIRE( q.pop() == a );
    REQUIRE( q.consumer_empty() == true );

    q.push( a );
    q.push( b );
    q.push( c );

    REQUIRE( q.consumer_empty() == false );
    REQUIRE( q.pop() == a );
    REQUIRE( q.pop() == b );
    REQUIRE( q.pop() == c );
    REQUIRE( q.consumer_empty() == true );

    // void push( node_ptr_type n, bool& wasEmpty )

    bool wasEmpty = false;
    q.push( a, wasEmpty );
    REQUIRE( wasEmpty == true );

    q.push( b, wasEmpty );
    REQUIRE( wasEmpty == false );

    REQUIRE( q.consumer_empty() == false );
    REQUIRE( q.pop() == a );

    q.push( c, wasEmpty );
    // REQUIRE( wasEmpty == false ); // <--- KNOWNBUG this fails.

    REQUIRE( q.pop() == b );
    REQUIRE( q.pop() == c );
    REQUIRE( q.consumer_empty() == true );

    // void push_multiple( node_ptr_type front, node_ptr_type back, bool& wasEmpty )

    // Notice that when using push_multiple,
    // the last item in the list (a) is the first to be popped from the FIFO.
    next_(c) = b;
    next_(b) = a;
    next_(a) = nullptr;
    wasEmpty=false;
    q.push_multiple( c, a, wasEmpty );
    REQUIRE( wasEmpty == true );
    next_(d) = nullptr;
    q.push_multiple( d, d, wasEmpty );
    REQUIRE( wasEmpty == false );

    REQUIRE( q.pop() == a );
    REQUIRE( q.pop() == b );
    REQUIRE( q.pop() == c );
    REQUIRE( q.pop() == d );
}
