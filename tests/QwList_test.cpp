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
#include "QwList.h"

#include "catch.hpp"

#include "Qw_Lists_adhocTestsShared.h"
#include "Qw_Lists_axiomaticTestsShared.h"
#include "Qw_Lists_randomisedTestShared.h"

/*
QwList has these functions that QwSTailList doesn't:

    iterators have --

    pop_back()

    insert( at:node, n )
    insert( at:iterator, n )

    remove( at:node )
    erase( at:iterator )

    previous(n)


functions that have more logic
    swap
    push_front
    pop_front
    push_back
    insert_after
    remove_after

functions not present:
	before_begin
	erase_after

*/

namespace {

    struct TestNode{
        TestNode *links_[2];
        enum { LINK_INDEX_1, LINK_INDEX_2, LINK_COUNT };

        int value;

        TestNode()
            : value( 0 )
        {
            for( int i=0; i < LINK_COUNT; ++i )
                links_[i] = nullptr;
        }
    };

    typedef QwList<TestNode*, TestNode::LINK_INDEX_1, TestNode::LINK_INDEX_2> TestList;

} // end anonymous namespace


TEST_CASE( "qw/list/empty", "QwList operations on empty lists" ) {

    TestList a, b;
    emptyListTest( a, b );
}


TEST_CASE( "qw/list/one", "QwList list operations with 1 node/element" ) {

    TestNode node;
    node.value = 42;

    TestList a, b;
    a.push_back( &node );

    REQUIRE( a.empty() == false );
    REQUIRE( a.back() == a.front() );

    singleItemListTest( a, b, &node );
}

TEST_CASE( "qw/list/two", "QwList list operations with 2 nodes/elements" ) {

    TestNode node1;
    node1.value = 0;

    TestNode node2;
    node2.value = 1;

    TestList a;
    a.push_back( &node2 );
    TestList b;

    twoItemListTest( a, b, &node1, &node2 );
}

TEST_CASE( "qw/list/many", "QwList list operations with many nodes/elements" ) {

    const int NODE_COUNT = 5;
    TestNode nodes[NODE_COUNT];

    manyItemsListTest<TestList,NODE_COUNT>( nodes );
}

TEST_CASE( "qw/list/back-and-push_back", "QwList test back() and push_back()" ) {

    TestNode node1;
    node1.value = 0;

    TestNode node2;
    node2.value = 1;

    TestNode node3;
    node3.value = 2;

    TestList a;
    TestList b;

    backAndPushBackListTest( a, b, &node1, &node2, &node3 );
}

TEST_CASE( "qw/list/front-stack", "QwList front stack test" ) {

    TestList a;

    const int NODE_COUNT = 10;

    TestNode nodes[NODE_COUNT];
    for( int i=0; i < NODE_COUNT; ++i )
        nodes[i].value = i;

    frontStackTest_withBackChecks( a, nodes, NODE_COUNT );
}

TEST_CASE( "qw/list/back-stack", "QwList back stack test" ) {

    // pop_back() is unique to QwList. back stack test uses it for dequeue

    TestList a;

    const int NODE_COUNT = 10;

    TestNode nodes[NODE_COUNT];
    for( int i=0; i < NODE_COUNT; ++i )
        nodes[i].value = i;

    backStackTest_withBackChecks( a, nodes, NODE_COUNT );
}

TEST_CASE( "qw/list/back-queue", "QwList back queue test" ) {

    TestList a;

    const int NODE_COUNT = 10;

    TestNode nodes[NODE_COUNT];
    for( int i=0; i < NODE_COUNT; ++i )
        nodes[i].value = i;

    backQueueTest( a, nodes, NODE_COUNT );
}

TEST_CASE( "qw/list/front-queue", "QwList front queue test" ) {

    // pop_back() is unique to QwList. front queue test uses it for dequeue

    TestList a;

    const int NODE_COUNT = 10;

    TestNode nodes[NODE_COUNT];
    for( int i=0; i < NODE_COUNT; ++i )
        nodes[i].value = i;

    frontQueueTest( a, nodes, NODE_COUNT );
}


TEST_CASE( "qw/list/insert-erase", "QwList test insert() and erase()" ) {

    TestNode node1;
    node1.value = 0;

    TestNode node2;
    node2.value = 1;

    TestNode node3;
    node3.value = 2;

    TestList a;

    // we only test iterator insert and remove because these use
    // node based insert and remove under the hood

    // insert using iterators

    a.insert( a.begin(), &node1 );
    a.insert( a.begin(), &node2 );
    REQUIRE( a.front() == &node2 );
    REQUIRE( a.back() == &node1 );
    a.clear();

    a.insert( a.end(), &node1 );
    a.insert( a.end(), &node2 );
    REQUIRE( a.front() == &node1 );
    REQUIRE( a.back() == &node2 );
    a.clear();

    a.insert( a.begin(), &node1 );
    a.insert( a.end(), &node2 );
    REQUIRE( a.front() == &node1 );
    REQUIRE( a.back() == &node2 );

    {
        TestList::iterator i = a.begin();
        ++i;
        a.insert(i, &node3 ); // insert node3 in the middle of the list
    }
    REQUIRE( a.front() == &node1 );
    REQUIRE( a.back() == &node2 );

    REQUIRE( a.pop_front() == &node1 );
    REQUIRE( a.pop_front() == &node3 );
    REQUIRE( a.pop_front() == &node2 );
    a.clear();

    // test erase()

    // one element
    a.push_front( &node1 );
    a.erase( a.begin() );
    REQUIRE( a.empty() == true );

    // two elements, remove first element
    a.push_front( &node2 );
    a.push_front( &node1 );
    a.erase( a.begin() );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.front() == &node2 );
    a.clear();

    // two elements, remove last element
    a.push_front( &node2 );
    a.push_front( &node1 );
    {
        TestList::iterator i = a.begin();
        ++i;
        a.erase(i);
    }
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.front() == &node1 );
    a.clear();

    // three elements, remove first element
    a.push_front( &node3 );
    a.push_front( &node2 );
    a.push_front( &node1 );
    a.erase( a.begin() );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.front() == &node2 );
    REQUIRE( a.back() == &node3 );
    a.clear();

    // three elements, remove last element
    a.push_front( &node3 );
    a.push_front( &node2 );
    a.push_front( &node1 );
    {
        TestList::iterator i = a.begin();
        ++i;
        ++i;
        a.erase(i);
    }
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.front() == &node1 );
    REQUIRE( a.back() == &node2 );
    a.clear();

    // three elements, remove middle element
    a.push_front( &node3 );
    a.push_front( &node2 );
    a.push_front( &node1 );
    {
        TestList::iterator i = a.begin();
        ++i;
        a.erase(i);
    }
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.front() == &node1 );
    REQUIRE( a.back() == &node3 );
    a.clear();
}


/*
    The following "axiomatic" tests aim to build base invariants
    for a list constructed with push_back: size functions operate correctly,
    constructed list contains exactly the expected items in the expected order.
    we then test each other function separately by constructing all of
    the different scenarios, which amount to treating lists of
    zero, one or many items, and inserting at the beginning, middle or end
    of the list.
*/

// requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants
//
// used to test accessors
//
// iterator construction, prefix and postfix ++
//
// empty, size_is_1, size_is_greater_than_1
// front
// before_begin, begin, end
// next

static void requireEmptyInvariants( TestList& a )
{
    // size related attributes

    REQUIRE( a.empty() == true );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == false );

    // iterator sequence invariants

    REQUIRE( a.begin() == a.end() );
}

static void requireSingleNodeInvariants( TestList& a, TestNode *node )
{
    // size related attributes

    REQUIRE( a.empty() == false );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.size_is_greater_than_1() == false );

    // front(), back() invariants

    REQUIRE( a.front() != (TestNode*)nullptr );
    REQUIRE( a.front() == node );

    REQUIRE( a.back() != (TestNode*)nullptr );
    REQUIRE( a.back() == node );

    REQUIRE( *a.begin() == node );

    // next(front), next(back), previous(front), previous(back) invariants

    REQUIRE( a.next(a.front()) == (TestNode*)nullptr );

	REQUIRE( a.next(a.back()) == (TestNode*)nullptr );

	// next() of front() is nullptr in a one item list
    REQUIRE( a.next(a.front()) == (TestNode*)nullptr );

    // iterator sequence invariants

    REQUIRE( a.begin() != a.end() );

    { // end comes directly after begin in a one item list (pre-increment)
        TestList::iterator i = a.begin();
        REQUIRE( *i == node );
        ++i;
        REQUIRE( i == a.end() );
    }

    { // end comes directly after begin in a one item list (post-increment)
        TestList::iterator i = a.begin();
        REQUIRE( *i == node );
        i++;
        REQUIRE( i == a.end() );
    }


    { // begin comes directly before begin (pre-decrement)
        TestList::iterator i = a.end();
        --i;
        REQUIRE( i == a.begin() );
    }

    { // begin comes directly before end (post-decrement)
        TestList::iterator i = a.end();
        i--;
        REQUIRE( i == a.begin() );
    }

    // bidirectional iterator should be able to decrement end():
    // see: http://www.sgi.com/tech/stl/BidirectionalIterator.html

    { // begin comes directly before end (pre-decrement)
        TestList::iterator i = a.end();
        --i;
        REQUIRE( i == a.begin() );
    }

    { // begin comes directly before end (post-decrement)
        TestList::iterator i = a.end();
        i--;
        REQUIRE( i == a.begin() );
    }
}

static void requireMoreThanOneNodeInvariants( TestList& a, TestNode *nodes, int nodeCount )
{
    // size related attributes

    REQUIRE( a.empty() == false );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );

    // front(), back() invariants

    REQUIRE( a.front() != (TestNode*)nullptr );
    REQUIRE( a.front() == &nodes[0] );

    REQUIRE( a.back() != (TestNode*)nullptr );
    REQUIRE( a.back() == &nodes[nodeCount-1] );

    REQUIRE( *a.begin() == &nodes[0] );

    // next(front), next(back), previous(front), previous(back) invariants

    REQUIRE( a.next(a.front()) != (TestNode*)nullptr );
	REQUIRE( a.previous(a.front()) == a.before_front_() );

    REQUIRE( a.previous(a.back()) != (TestNode*)nullptr );
    REQUIRE( a.next(a.back()) == (TestNode*)nullptr );

    // iterator sequence invariants

    REQUIRE( a.begin() != a.end() );

    { // --end references back (pre-decrement)
        TestList::iterator i = a.end();
        --i;
        REQUIRE( i != a.begin() );
        REQUIRE( *i == a.back() );
    }

    { // --end references back (post-decrement)
        TestList::iterator i = a.end();
        i--;
        REQUIRE( i != a.begin() );
        REQUIRE( *i == a.back() );
    }

    // nodes are in expected sequence according to iterators
    // and also according to a.next()

    // walk forwards through the list using both an iterator and a.next(n)
    // in each case check that the sequence matches the array passed in
    // and that the list is terminated correctly.

    // forwards using increment / next:

    TestNode *n = a.front();
    TestList::iterator i_pre = a.begin();
    TestList::iterator i_post = a.begin();
    TestList::iterator i_pre_prev; // iterators with a lag of 1
    TestList::iterator i_post_prev;// so we can perform reverse iteration
    for( int j = 0; j < nodeCount; ++j ){

        i_pre_prev = i_pre;
        i_post_prev = i_post;

        REQUIRE( n == &nodes[j] );
        n = a.next(n);

        REQUIRE( *i_pre == &nodes[j] );
        ++i_pre;

        REQUIRE( *i_post == &nodes[j] );
        i_post++;
    }

    REQUIRE( n == (TestNode*)nullptr );
    REQUIRE( i_pre == a.end() );
    REQUIRE( i_post == a.end() );

    REQUIRE( *i_pre_prev == a.back() );
    REQUIRE( *i_post_prev == a.back() );

    // backwards using decrement / previous:

    n = a.back();
    i_pre = i_pre_prev;
    i_post = i_post_prev;
    for( int j = 0; j < nodeCount; ++j ){

        REQUIRE( n == &nodes[nodeCount - 1 - j] );
        n = a.previous(n);

        REQUIRE( *i_pre == &nodes[nodeCount - 1 - j] );
        --i_pre;

        REQUIRE( *i_post == &nodes[nodeCount - 1 - j] );
        i_post--;
    }

    REQUIRE( n == a.before_front_() );
}

TEST_CASE( "qw/list/axiomatic/baseline", "QwList baseline axiomatic tests" ) {
    axiomaticBaselineTest<TestList>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/list/axiomatic/push_back", "QwList push_back" ) {
    axiomaticPushBackTest<TestList>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/list/axiomatic/swap", "QwList axiomatic test of swap" ) {
    axiomaticSwapTest<TestList>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/list/axiomatic/pop_front", "QwList axiomatic test of pop_front" ) {
    axiomaticPopFrontTest<TestList>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/list/axiomatic/insert_after/node_ptr/empty", "QwList axiomatic test of insert_after(node_ptr,n) for empty list" ) {

    SECTION( "empty", "empty list, insert after before_begin")
    {
        TestList a;
        TestNode node;
        a.insert_after( a.before_front_(), &node ); // strictly you shouldn't de-ref the before_begin iterator, it doesn't point to a node
        requireSingleNodeInvariants( a, &node );
    }
}

TEST_CASE( "qw/list/axiomatic/insert_after/node_ptr", "QwList axiomatic test of insert_after(node_ptr,n)" ) {
    axiomaticInsertAfterNodePtrTest<TestList>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

/*
TEST_CASE( "qw/list/axiomatic/insert_after/iter", "QwList axiomatic test of insert_after(iter,n)" ) {
    axiomaticInsertAfterIterTest<TestList>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}
*/

TEST_CASE( "qw/list/axiomatic/remove_after/before_front_/node_ptr", "QwList axiomatic test of remove_after(a.before_front_())" ) {

    SECTION( "one", "init with one item, remove after before_front_")
    {
        TestList a;
        TestNode node;
        a.push_front( &node );
        REQUIRE( a.remove_after( a.before_front_() ) == &node ); // strictly you shouldn't de-ref the before_front_ iterator, it doesn't point to a node
        requireEmptyInvariants( a );
    }

    SECTION( "two/before-begin", "init with two items, remove after before_front_")
    {
        TestList a;
        TestNode nodes[2];
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        REQUIRE( a.remove_after( a.before_front_() ) == &nodes[0] ); // strictly you shouldn't de-ref the before_begin iterator, it doesn't point to a node
        requireSingleNodeInvariants( a, &nodes[1] );
    }

    SECTION( "many/before_front_", "init with many items, remove after before_front_")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        REQUIRE( a.remove_after( a.before_front_() ) == &nodes[0] ); // strictly you shouldn't de-ref the before_begin iterator, it doesn't point to a node
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

}

TEST_CASE( "qw/list/axiomatic/remove_after/2/node_ptr", "QwList axiomatic test of remove_after(node_ptr) using front" ) {
    axiomaticRemoveAfter2NodePtrTest<TestList>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

/*
TEST_CASE( "qw/list/axiomatic/remove_after/iter", "QwList axiomatic test of remove_after(iter)" ) {
    axiomaticRemoveAfterIterTest<TestList>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}
*/

/*
TEST_CASE( "qw/list/axiomatic/erase_after/iter", "QwList axiomatic test of erase_after(iter)" ) {
    axiomaticEraseAfterIterTest<TestList>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}
*/

TEST_CASE( "qw/list/axiomatic/insert/node_ptr", "QwList axiomatic test of insert(node_ptr,n)" ) {
    // note that you can't use insert(node_ptr,n) on an empty list.
	// you can use the iterator version to insert at end() though.

    SECTION( "one", "init with one item, insert at front (or back, same thing)")
    {
        TestList a;
        TestNode nodes[2];
        a.push_front( &nodes[1] );
        REQUIRE( a.front() == a.back() );
        a.insert( a.front(), &nodes[0] );
        requireMoreThanOneNodeInvariants( a, nodes, 2 );
    }

    SECTION( "many/at-front", "init with many items, insert at front")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.insert( a.front(), &nodes[0] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/at-next-to-front", "init with many items, insert at next(front)")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[0] );
        a.insert( a.next(a.front()), &nodes[1] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/at-back", "init with many items, insert at back")
    { // (back stays at back)
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.insert( &nodes[4], &nodes[3] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/at-previous-to-back", "init with many items, insert at previous(back)")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.insert( a.previous(a.back()), &nodes[2] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }
}

TEST_CASE( "qw/list/axiomatic/insert/iter", "QwList axiomatic test of insert(iter,n)" ) {

    SECTION( "empty", "init empty, insert at begin (or end, same thing)")
    {
        TestList a;
        TestNode node;
        REQUIRE( a.begin() == a.end() );
        a.insert( a.begin(), &node );
        requireSingleNodeInvariants( a, &node );
    }

    SECTION( "one/begin", "init with one item, insert at begin")
    {
        TestList a;
        TestNode nodes[2];
        a.push_front( &nodes[1] );
        a.insert( a.begin(), &nodes[0] );
        requireMoreThanOneNodeInvariants( a, nodes, 2 );
    }

    SECTION( "one/end", "init with one item, insert at end")
    {
        TestList a;
        TestNode nodes[2];
        a.push_front( &nodes[0] );
        a.insert( a.end(), &nodes[1] );
        requireMoreThanOneNodeInvariants( a, nodes, 2 );
    }

    SECTION( "many/at-begin", "init with many items, insert at begin")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.insert( a.begin(), &nodes[0] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/at-inc-begin", "init with many items, insert at ++begin")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[0] );
        a.insert( ++a.begin(), &nodes[1] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/at-last", "init with many items, insert at last")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        TestList::iterator i = a.begin();
        ++i;
        ++i;
        ++i;
        a.insert( i, &nodes[3] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/at-previous-to-last", "init with many items, insert at previous to last")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        TestList::iterator i = a.begin();
        ++i;
        ++i;
        a.insert( i, &nodes[2] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/at-end", "init with many items, insert at end")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.insert( a.end(), &nodes[4] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }
}

TEST_CASE( "qw/list/axiomatic/remove/node_ptr", "QwList axiomatic test of remove(node_ptr)" ) {

    SECTION( "one", "init with one item, remove front (or back, same thing)")
    {
        TestList a;
        TestNode node;
        a.push_front( &node );
        REQUIRE( a.front() == a.back() );
        a.remove( a.front() );
        requireEmptyInvariants( a );
    }

    SECTION( "two/front", "init with two items, remove front")
    {
        TestList a;
        TestNode nodes[2];
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.remove( a.front() );
        requireSingleNodeInvariants( a, &nodes[1] );
    }

    SECTION( "two/back", "init with two items, remove back")
    {
        TestList a;
        TestNode nodes[2];
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.remove( a.back() );
        requireSingleNodeInvariants( a, &nodes[0] );
    }

    SECTION( "many/front", "init with many items, remove front")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.remove( a.front() );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/next-to-front", "init with many items, remove next(front)")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[0] );
        a.push_front( &nodes[1] );
        a.remove( a.next(a.front()) );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/previous-to-back", "init with many items, remove previous(back)")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[0] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.remove( a.previous(a.back()) );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/back", "init with many items, remove back")
    {
        TestList a;
        TestNode nodes[5];
        a.push_front( &nodes[0] );
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.remove( a.back() );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }
}

// no remove(iter) test since it simply forwards to remove(node_ptr)

/* fuzz test */

static void verify( TestList& list, int expectedCount )
{
    verifyForwards( list, expectedCount );
    verifyBackwards( list, expectedCount );
}

static void randomisedInsert( TestList& list, TestNode* node, int currentCount )
{
    switch( list.empty() ? rand() % 2 : rand() % 5 ){
    case 0:
        list.push_front( node );
        break;
    case 1:
        list.push_back( node );
        break;
    case 2:
        {
            int atj = rand() % currentCount;
            TestList::node_ptr_type at = list.front();
            for( int i=0; i<atj; ++i )
                at = list.next(at);
            list.insert( at, node ); // insert n before node at
            break;
        }
    case 3:
        {
            int atj = rand() % currentCount;
            TestList::iterator at = list.begin();
            for( int i=0; i<=atj; ++i ) // list allows inserting at end
                ++at;
            list.insert( at, node ); // insert n before node at
            break;
        }
    case 4:
    default:
        {
            int atj = rand() % currentCount;
            TestList::node_ptr_type at = list.front();
            for( int i=0; i<atj; ++i )
                at = list.next(at);
            list.insert_after( at, node ); // insert n after node before
            break;
        }
    }
}

static TestNode* randomisedRemove( TestList& list, int currentCount )
{
    switch( currentCount > 1 ? rand() % 5 : rand() % 4 ){
    case 0:
        return list.pop_front();
    case 1:
        return list.pop_back();
    case 2:
        {
            int atj = rand() % currentCount;
            TestList::node_ptr_type at = list.front();
            for( int i=0; i<atj; ++i )
                at = list.next(at);
            list.remove( at ); // remove node at
            return at;
        }
    case 3:
        {
            int atj = rand() % currentCount;
            TestList::iterator at = list.begin();
            for( int i=0; i<atj; ++i )
                ++at;
            TestList::node_ptr_type result = *at;
            list.erase( at ); // remove node at at
            return result;
        }
    case 4:
        // falls through
    default:
        {
            int atj = rand() % (currentCount - 1); // -1 because we can't remove after the last item
            TestList::node_ptr_type at = list.front();
            for( int i=0; i<atj; ++i )
                at = list.next(at);
            return list.remove_after( at ); // returns the removed node
        }
    }
}

TEST_CASE( "qw/list/fuzz", "[fuzz] QwList fuzz test" ) {
    fuzzTest<TestList>( randomisedInsert, randomisedRemove, verify );
}

/* -----------------------------------------------------------------------
Last reviewed: June 30, 2013
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
