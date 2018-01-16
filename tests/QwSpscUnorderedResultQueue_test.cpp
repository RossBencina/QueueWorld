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
#include "QwSpscUnorderedResultQueue.h"

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
                links_[i] = 0;
        }
    };

    typedef QwSpscUnorderedResultQueue<TestNode*, TestNode::LINK_INDEX_1> spsc_undordered_result_queue_t;

} // end anonymous namespace


TEST_CASE( "qw/spsc_undordered_result_queue", "QwSpscUnorderedResultQueue single threaded test" ) {

    TestNode nodes[3];
    TestNode *a = &nodes[0];
    TestNode *b = &nodes[1];
    TestNode *c = &nodes[2];

    spsc_undordered_result_queue_t q;
    q.init();

    REQUIRE( q.expectedResultCount() == 0 );
    REQUIRE( q.pop() == 0 );

    q.incrementExpectedResultCount();
    q.push( a );
    REQUIRE( q.expectedResultCount() == 1 );
    REQUIRE( q.pop() == a );
    REQUIRE( q.expectedResultCount() == 0 );
    REQUIRE( q.pop() == 0 );

    q.incrementExpectedResultCount(3);
    q.push( a );
    q.push( b );
    q.push( c );

    REQUIRE( q.expectedResultCount() == 3 );
    // the queue doesn't guarantee a result order
    {
        TestNode *x = q.pop();
        REQUIRE( (x==a || x==b || x==c) );
    }
    REQUIRE( q.expectedResultCount() == 2 );
    {
        TestNode *x = q.pop();
        REQUIRE( (x==a || x==b || x==c) );
    }
    REQUIRE( q.expectedResultCount() == 1 );
    {
        TestNode *x = q.pop();
        REQUIRE( (x==a || x==b || x==c) );
    }
    REQUIRE( q.expectedResultCount() == 0 );
    REQUIRE( q.pop() == 0 );
}
