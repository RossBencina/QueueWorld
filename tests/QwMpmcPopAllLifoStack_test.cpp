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
#include "QwMpmcPopAllLifoStack.h"

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

    typedef QwMPMCPopAllLifoStack<TestNode*, TestNode::LINK_INDEX_1> mpmc_pop_all_lifo_stack_t;

} // end anonymous namespace

TEST_CASE( "qw/mpmc_pop_all_lifo_stack", "QwMPMCPopAllLifoStack single threaded test" ) {

    TestNode nodes[10];
    TestNode *a = &nodes[0];
    TestNode *b = &nodes[1];
    TestNode *c = &nodes[2];

    mpmc_pop_all_lifo_stack_t stack;

    REQUIRE( stack.empty() );
    REQUIRE( stack.pop_all() == 0 );

    // void push( node_ptr_type node )

    stack.push(a);
    REQUIRE( !stack.empty() );
    REQUIRE( stack.pop_all() == a );
    REQUIRE( stack.empty() );
    REQUIRE( stack.pop_all() == 0 );
    
    for (int i=0; i < 10; ++i)
        stack.push(&nodes[i]);

    REQUIRE( !stack.empty() );

    { // verify that pop-all returns items in LIFO order:
        TestNode *xs = stack.pop_all();
        REQUIRE( stack.empty() );
        
        for (int i=9; i >=0; --i) {
            REQUIRE( xs == &nodes[i] );
            TestNode *next = xs->links_[TestNode::LINK_INDEX_1];
            // zero links as we go. in test mode the data structure requires all links to be zeroed
            xs->links_[TestNode::LINK_INDEX_1] = 0;
            xs = next;
        }
        REQUIRE( xs == 0 );
    }

    // void push( node_ptr_type node, bool& wasEmpty )

    bool wasEmpty=false;
    stack.push(a,wasEmpty);
    REQUIRE( wasEmpty == true );
    stack.push(b,wasEmpty);
    REQUIRE( wasEmpty == false );

    REQUIRE( stack.pop_all() == b );
    REQUIRE( stack.empty() );
    a->links_[TestNode::LINK_INDEX_1] = 0;
    b->links_[TestNode::LINK_INDEX_1] = 0;

    // void push_multiple( node_ptr_type front, node_ptr_type back )

    a->links_[TestNode::LINK_INDEX_1] = b;
    b->links_[TestNode::LINK_INDEX_1] = c;
    c->links_[TestNode::LINK_INDEX_1] = 0;

    stack.push_multiple(a, c);
    REQUIRE( !stack.empty() );

    { // verify that pop-all returns items in LIFO order:
        TestNode *xs = stack.pop_all();
        REQUIRE( stack.empty() );

        REQUIRE( xs == a );
        REQUIRE( xs->links_[TestNode::LINK_INDEX_1] == b );
        REQUIRE( xs->links_[TestNode::LINK_INDEX_1]->links_[TestNode::LINK_INDEX_1] == c );

        a->links_[TestNode::LINK_INDEX_1] = 0;
        b->links_[TestNode::LINK_INDEX_1] = 0;
        c->links_[TestNode::LINK_INDEX_1] = 0;
    }
}
