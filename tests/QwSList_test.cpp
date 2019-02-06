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
#include "QwSList.h"

#include "catch.hpp"

#include "Qw_Lists_adhocTestsShared.h"
#include "Qw_Lists_axiomaticTestsShared.h"
#include "Qw_Lists_randomisedTestShared.h"

namespace{

    struct TestNode{
        TestNode *links_[2];
        enum { LINK_INDEX_1, LINK_INDEX_2, LINK_COUNT };

        int value;

        TestNode()
            : value(0)
        {
            for (int i=0; i < LINK_COUNT; ++i)
                links_[i] = nullptr;
        }
    };

    typedef QwSList<TestNode*, TestNode::LINK_INDEX_1> TestSList;

} // end anonymous namespace


TEST_CASE("qw/slist/empty", "QwSList operations on empty lists") {

    TestSList a, b;
    emptyListTest(a, b);
}

TEST_CASE("qw/slist/one", "QwSList list operations with 1 node/element") {

    TestNode node;
    node.value = 42;

    TestSList a(&node);
    TestSList b;

    singleItemSListTest(a, b, &node);
}

TEST_CASE("qw/slist/two", "QwSList list operations with 2 nodes/elements") {

    TestNode node1;
    node1.value = 0;

    TestNode node2;
    node2.value = 1;

    TestSList a(&node2);
    TestSList b;

    twoItemSListTest(a, b, &node1, &node2);
}

TEST_CASE("qw/slist/many", "QwSList list operations with many nodes/elements") {

    const int NODE_COUNT = 5;
    TestNode nodes[NODE_COUNT];

    manyItemsSListTest<TestSList, NODE_COUNT>(nodes);
}

TEST_CASE("qw/slist/stack", "QwSList front stack test") {

    TestSList a;

    const int NODE_COUNT = 10;
    TestNode nodes[NODE_COUNT];
    for (int i=0; i < NODE_COUNT; ++i)
        nodes[i].value = i;

    frontStackTest(a, nodes, NODE_COUNT);
}

TEST_CASE("qw/slist/remove", "QwSList remove free function") {

    TestNode node0;
    node0.value = 0;

    TestNode node1;
    node1.value = 1;

    TestNode node2;
    node2.value = 2;

    TestSList a;
    // 0, 1, 2
    a.push_front(&node2);
    a.push_front(&node1);
    a.push_front(&node0);

    SECTION("first-node", "remove first node") {
        remove(a, &node0);
        REQUIRE(a.front() == &node1);
        a.pop_front();
        REQUIRE(a.front() == &node2);
        a.pop_front();
        REQUIRE(a.empty());
    }

    SECTION("middle-node", "remove middle node") {
        remove(a, &node1);
        REQUIRE(a.front() == &node0);
        a.pop_front();
        REQUIRE(a.front() == &node2);
        a.pop_front();
        REQUIRE(a.empty());
    }

    SECTION("last-node", "remove last node") {
        remove(a, &node2);
        REQUIRE(a.front() == &node0);
        a.pop_front();
        REQUIRE(a.front() == &node1);
        a.pop_front();
        REQUIRE(a.empty());
    }
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

static void requireEmptyInvariants(TestSList& a)
{
    // size related attributes

    REQUIRE(a.empty() == true);
    REQUIRE(a.size_is_1() == false);
    REQUIRE(a.size_is_greater_than_1() == false);

    // front()

    REQUIRE(a.front() == (TestNode*)nullptr);

    // iterator sequence invariants

    REQUIRE(a.begin() == a.end());

    { // begin comes directly after before_begin (pre-increment)
        TestSList::iterator i = a.before_begin();
        ++i;
        REQUIRE(i == a.begin());
    }

    { // begin comes directly after before_begin (post-increment)
        TestSList::iterator i = a.before_begin();
        i++;
        REQUIRE(i == a.begin());
    }
}

static void requireSingleNodeInvariants(TestSList& a, TestNode *node)
{
    // size related attributes

    REQUIRE(a.empty() == false);
    REQUIRE(a.size_is_1() == true);
    REQUIRE(a.size_is_greater_than_1() == false);

    // front() invariants

    REQUIRE(a.front() != (TestNode*)nullptr);
    REQUIRE(a.front() == node);

    REQUIRE(*a.begin() == node);

    // next() of front() is nullptr in a one item list
    REQUIRE(a.next(a.front()) == (TestNode*)nullptr);

    // iterator sequence invariants

    REQUIRE(a.begin() != a.end());

    { // begin comes directly after before_begin (pre-increment)
        TestSList::iterator i = a.before_begin();
        ++i;
        REQUIRE(i == a.begin());
    }

    { // begin comes directly after before_begin (post-increment)
        TestSList::iterator i = a.before_begin();
        i++;
        REQUIRE(i == a.begin());
    }

    { // end comes directly after begin in a one item list (pre-increment)
        TestSList::iterator i = a.begin();
        REQUIRE(*i == node);
        ++i;
        REQUIRE(i == a.end());
    }

    { // end comes directly after begin in a one item list (post-increment)
        TestSList::iterator i = a.begin();
        REQUIRE(*i == node);
        i++;
        REQUIRE(i == a.end());
    }
}

static void requireMoreThanOneNodeInvariants(TestSList& a, TestNode *nodes, int nodeCount)
{
    // size related attributes

    REQUIRE(a.empty() == false);
    REQUIRE(a.size_is_1() == false);
    REQUIRE(a.size_is_greater_than_1() == true);

    // front() invariants

    REQUIRE(a.front() != (TestNode*)nullptr);
    REQUIRE(a.front() == &nodes[0]);

    REQUIRE(*a.begin() == &nodes[0]);

    // iterator sequence invariants

    REQUIRE(a.begin() != a.end());

    { // begin comes directly after before_begin (pre-increment)
        TestSList::iterator i = a.before_begin();
        ++i;
        REQUIRE(i == a.begin());
    }

    { // begin comes directly after before_begin (post-increment)
        TestSList::iterator i = a.before_begin();
        i++;
        REQUIRE(i == a.begin());
    }

    // nodes are in expected sequence according to iterators
    // and also according to a.next()

    // walk forwards through the list using both an iterator and a.next(n)
    // in each case check that the sequence matches the array passed in
    // and that the list is terminated correctly.

    TestNode *n = a.front();
    TestSList::iterator i_pre = a.begin();
    TestSList::iterator i_post = a.begin();
    for (int j=0; j < nodeCount; ++j) {

        REQUIRE(n == &nodes[j]);
        n = a.next(n);

        REQUIRE(*i_pre == &nodes[j]);
        ++i_pre;

        REQUIRE(*i_post == &nodes[j]);
        i_post++;
    }

    REQUIRE(n == (TestNode*)nullptr);
    REQUIRE(i_pre == a.end());
    REQUIRE(i_post == a.end());
}

TEST_CASE("qw/slist/axiomatic/baseline", "QwSList baseline axiomatic tests") {
    axiomaticBaselineTest<TestSList>(requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants);
}

TEST_CASE("qw/slist/axiomatic/node-ptr-ctor", "QwSList construct with raw node ptr") {

    SECTION("one", "init with front node ptr")
    {
        TestNode node;
        node.links_[0] = nullptr;
        TestSList a(&node);
        requireSingleNodeInvariants(a, &node);
        a.clear();
        requireEmptyInvariants(a);
    }

    SECTION("many", "init with front node ptr")
    {
        for (int count = 2; count <= 5; ++count) {
            TestNode nodes[5];
            for (int i=1; i < count; ++i) {
                nodes[i-1].links_[0] = &nodes[i];
                nodes[i].links_[0] = nullptr;
            }
            TestSList a(&nodes[0]);
            requireMoreThanOneNodeInvariants(a, nodes, count);
            a.clear();
            requireEmptyInvariants(a);
        }
    }
}

TEST_CASE("qw/slist/axiomatic/swap", "QwSList axiomatic test of swap") {
    axiomaticSwapTest<TestSList>(requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants);
}

TEST_CASE("qw/slist/axiomatic/pop_front", "QwSList axiomatic test of pop_front") {
    axiomaticPopFrontTest<TestSList>(requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants);
}

TEST_CASE("qw/slist/axiomatic/insert_after/node_ptr/empty", "QwSList axiomatic test of insert_after(node_ptr, n) for empty list") {

    SECTION("empty", "initially empty list, insert after before_begin")
    { // empty
        TestSList a;
        TestNode node;
        a.insert_after(*a.before_begin(), &node); // strictly you shouldn't de-ref the before_begin iterator, it doesn't point to a node
        requireSingleNodeInvariants(a, &node);
    }
}

TEST_CASE("qw/slist/axiomatic/insert_after/node_ptr", "QwSList axiomatic test of insert_after(node_ptr, n)") {
    axiomaticInsertAfterNodePtrTest<TestSList>(requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants);
}

TEST_CASE("qw/slist/axiomatic/insert_after/iter", "QwSList axiomatic test of insert_after(iter, n)") {
    axiomaticInsertAfterIterTest<TestSList>(requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants);
}

TEST_CASE("qw/slist/axiomatic/remove_after/before_begin/node_ptr", "QwSList axiomatic test of remove_after(*before_begin())") {
    axiomaticRemoveAfterBeforeBeginTest<TestSList>(requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants);
}

TEST_CASE("qw/slist/axiomatic/remove_after/2/node_ptr", "QwSList axiomatic test of remove_after(node_ptr) using front") {
    axiomaticRemoveAfter2NodePtrTest<TestSList>(requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants);
}

TEST_CASE("qw/slist/axiomatic/remove_after/iter", "QwSList axiomatic test of remove_after(iter)") {
    axiomaticRemoveAfterIterTest<TestSList>(requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants);
}

TEST_CASE("qw/slist/axiomatic/erase_after/iter", "QwSList axiomatic test of erase_after(iter)") {
    axiomaticEraseAfterIterTest<TestSList>(requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants);
}

/* fuzz test */

static void verify(TestSList& list, int expectedCount)
{
    verifyForwards(list, expectedCount);
}

static void randomisedInsert(TestSList& list, TestNode* node, int currentCount)
{
    switch (list.empty() ? 0 : rand() % 2) {
    case 0:
        list.push_front(node);
        break;
    case 1:
    default:
        {
            int atj = rand() % currentCount;
            TestSList::node_ptr_type at = list.front();
            for (int i=0; i<atj; ++i)
                at = list.next(at);
            list.insert_after(at, node); // insert n after node before
            break;
        }
    }
}

static TestNode* randomisedRemove(TestSList& list, int currentCount)
{
    switch (currentCount > 1 ? rand() % 3 : 0) {
    case 0:
        return list.pop_front();
    case 1:
        {
            int atj = rand() % currentCount;
            TestSList::node_ptr_type at = list.front();
            for (int i=0; i<atj; ++i)
                at = list.next(at);
            remove(list, at);
            return at;
        }
    case 2:
        // falls through
    default:
        {
            int atj = rand() % (currentCount - 1); // -1 because we can't remove after the last item
            TestSList::node_ptr_type at = list.front();
            for (int i=0; i<atj; ++i)
                at = list.next(at);
            return list.remove_after(at); // returns the removed node
        }
    }
}

TEST_CASE("qw/slist/fuzz", "[fuzz] QwSList fuzz test") {
    fuzzTest<TestSList>(randomisedInsert, randomisedRemove, verify);
}

/* -----------------------------------------------------------------------
Last reviewed: June 30, 2013
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
