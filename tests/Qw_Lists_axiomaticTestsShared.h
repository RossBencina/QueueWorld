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
#ifndef INCLUDED_QWLISTSAXIOMATICTESTSSHARED_H
#define INCLUDED_QWLISTSAXIOMATICTESTSSHARED_H

#include "QwConfig.h"

/*
    The following "axiomatic" tests aim to build base invariants
    for a list constructed with push_back: size functions operate correctly,
    constructed list contains exactly the expected items in the expected order.
    we then test each other function separately by constructing all of
    the different scenarios, which amount to treating lists of
    zero, one or many items, and inserting at the beginning, middle or end
    of the list.
*/

// push nodeCount nodes from the start of nodes[] onto a
// such that the order from front is the same as the order in the array
template< typename list_type >
static void pushFrontN( list_type& a, typename list_type::node_ptr_type nodes, int nodeCount )
{
    for( int i=0; i < nodeCount; ++i ){
        a.push_front( &nodes[(nodeCount-1)-i] ); // push in reverse order
        REQUIRE( a.front() == &nodes[(nodeCount-1)-i] );
    }
}

template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticBaselineTest( requireEmptyInvariantsT& requireEmptyInvariants,
    requireSingleNodeInvariantsT& requireSingleNodeInvariants,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    /*
        The main purpose of this test is to prove that we can use push_front
        to insert a sequence of 1 to 5 items into the list and that all of
        the expected invariants hold (see above for invariant checks).

        push_front is used as the basis for the rest of the tests.

        We also show that clear can return the list to empty in each case.

        tests:

        constructors: default and head node ptr constructors

        push_front
    */

    SECTION( "empty", "test empty list")
    {
        list_type a;
        requireEmptyInvariants( a );
        a.clear();
        requireEmptyInvariants( a );
    }

    SECTION( "one", "push one item, clear")
    {
        list_type a;
        typename list_type::node_type node;
        a.push_front( &node );
        REQUIRE( a.front() == &node );
        requireSingleNodeInvariants( a, &node );
        a.clear();
        requireEmptyInvariants( a );
    }

    SECTION( "many", "push many items, clear")
    {
        for( int count = 2; count <= 5; ++count ){
            typename list_type::node_type nodes[5];
            list_type a;
            pushFrontN( a, nodes, count );
            requireMoreThanOneNodeInvariants( a, nodes, count );
            a.clear();
            requireEmptyInvariants( a );
        }
    }
}

template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticPushBackTest( requireEmptyInvariantsT& requireEmptyInvariants,
    requireSingleNodeInvariantsT& requireSingleNodeInvariants,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    SECTION( "one", "push one item")
    {
        list_type a;
        typename list_type::node_type node;
        a.push_back( &node );
        requireSingleNodeInvariants( a, &node );
        a.clear();
        requireEmptyInvariants( a );
    }

    SECTION( "many", "push many items")
    {
        for( int count = 2; count <= 5; ++count ){
            typename list_type::node_type nodes[5];
            list_type a;
            for( int i=0; i < count; ++i )
                a.push_back( &nodes[i] ); // push in forward order
            requireMoreThanOneNodeInvariants( a, nodes, count );
            a.clear();
            requireEmptyInvariants( a );
        }
    }
}

template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticSwapTest( requireEmptyInvariantsT& requireEmptyInvariants,
    requireSingleNodeInvariantsT& requireSingleNodeInvariants,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    // we conclude by inspection that swap(a, b) also tests a.swap(b)

    SECTION( "empty", "swap empty lists")
    {
        list_type a, b;
        requireEmptyInvariants( a );
        requireEmptyInvariants( b );
        swap( a, b );
        requireEmptyInvariants( a );
        requireEmptyInvariants( b );
    }

    SECTION( "one", "swap one item lists")
    {
        list_type a, b;
        typename list_type::node_type node;
        a.push_front( &node );
        requireSingleNodeInvariants( a, &node );
        requireEmptyInvariants( b );
        swap( a, b );
        requireEmptyInvariants( a );
        requireSingleNodeInvariants( b, &node );
        swap( a, b );
        requireEmptyInvariants( b );
        requireSingleNodeInvariants( a, &node );
    }

    SECTION( "many", "swap many item lists")
    {
        for( int count = 2; count <= 5; ++count ){
            typename list_type::node_type nodes[5];
            list_type a, b;
            pushFrontN( a, nodes, count );
            requireMoreThanOneNodeInvariants( a, nodes, count );
            requireEmptyInvariants( b );
            swap( a, b );
            requireEmptyInvariants( a );
            requireMoreThanOneNodeInvariants( b, nodes, count );
            swap( a, b );
            requireMoreThanOneNodeInvariants( a, nodes, count );
            requireEmptyInvariants( b );
        }
    }
}

template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticPopFrontTest( requireEmptyInvariantsT& requireEmptyInvariants,
    requireSingleNodeInvariantsT& requireSingleNodeInvariants,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    SECTION( "one", "init with one item, pop front")
    {
        list_type a;
        typename list_type::node_type node;
        a.push_front( &node );
        requireSingleNodeInvariants( a, &node );
        REQUIRE( a.pop_front() == &node );
        requireEmptyInvariants( a );
    }

    SECTION( "many", "init with many items, pop them from front")
    {
        for( int count = 2; count <= 5; ++count ){
            typename list_type::node_type nodes[5];
            list_type a;
            pushFrontN( a, nodes, count );
            requireMoreThanOneNodeInvariants( a, nodes, count );

            // pop nodes off one by one checking that invariants are correct at each step
            for( int i=0; i < count; ++i ){
                if( (count - i) > 1 ){
                    requireMoreThanOneNodeInvariants( a, &nodes[i], count-i );
                }else if( (count - i) == 1 ){
                    requireSingleNodeInvariants( a, &nodes[i] );
                }
                REQUIRE( a.pop_front() == &nodes[i] );
            }
            requireEmptyInvariants( a );
        }
    }
}

/*
    insert_after tests target the following cases:

    list is empty:
        insert_after( before_begin(), n ) // only possible with the iter version

    list is non-empty (1 item):
        insert_after( begin(), n )
        insert_after( front(), n )

    list is non-empty (many, say 5 items):
        insert_after( begin(), n )
        insert_after( ++begin(), n )
        insert_after( front(), n )
        insert_after( a.next(front()), n )
        insert_after( last element iter )
        insert_after( last element ptr )

*/

// empty case is treated separately for each concrete list type

template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticInsertAfterNodePtrTest( requireEmptyInvariantsT& /*requireEmptyInvariants*/,
    requireSingleNodeInvariantsT& /*requireSingleNodeInvariants*/,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    SECTION( "one", "init with one item, insert after front")
    {
        list_type a;
        typename list_type::node_type nodes[2];
        a.push_front( &nodes[0] );
        a.insert_after( a.front(), &nodes[1] );
        requireMoreThanOneNodeInvariants( a, nodes, 2 );
    }

    SECTION( "many/front", "init with many items, insert after front")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[0] );
        a.insert_after( a.front(), &nodes[1] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/next-to-front", "init with many items, insert after next(front)")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.insert_after( a.next(a.front()), &nodes[2] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/last", "init with many items, insert after last")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.insert_after( &nodes[3], &nodes[4] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }
}

template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticInsertAfterIterTest( requireEmptyInvariantsT& /*requireEmptyInvariants*/,
    requireSingleNodeInvariantsT& requireSingleNodeInvariants,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    SECTION( "empty", "init empty, insert after before_begin")
    {
        list_type a;
        typename list_type::node_type node;
        a.insert_after( a.before_begin(), &node );
        requireSingleNodeInvariants( a, &node );
    }

    SECTION( "one", "init with one item, insert_after begin")
    {
        list_type a;
        typename list_type::node_type nodes[2];
        a.push_front( &nodes[0] );
        a.insert_after( a.begin(), &nodes[1] );
        requireMoreThanOneNodeInvariants( a, nodes, 2 );
    }

    SECTION( "many/begin", "init with many items, insert_after begin")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[0] );
        a.insert_after( a.begin(), &nodes[1] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/inc-begin", "init with many items, insert_after next to begin")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.insert_after( ++a.begin(), &nodes[2] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }

    SECTION( "many/last", "init with many items, insert_after last")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        typename list_type::iterator i = a.begin();
        ++i;
        ++i;
        ++i;
        a.insert_after( i, &nodes[4] );
        requireMoreThanOneNodeInvariants( a, nodes, 5 );
    }
}

/*
    remove_after tests target the following cases:

    list has one item:

        remove_after( before_begin() )

    list 2 items
        remove_after( before_begin() )
        remove_after( begin() )
        remove_after( front() )

    list has 5 items:
        remove_after( before_begin() )
        remove_after( begin() )
        remove_after( front() )
        remove_after( ++begin() )
        remove_after( next(front()) )
        remove_after( next-to-last item iter )
        remove_after( next-to-last item ptr )
*/


template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticRemoveAfterBeforeBeginTest( requireEmptyInvariantsT& requireEmptyInvariants,
    requireSingleNodeInvariantsT& requireSingleNodeInvariants,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    SECTION( "one", "initially one item, remove after before_begin")
    { // one
        list_type a;
        typename list_type::node_type node;
        a.push_front( &node );
        REQUIRE( a.remove_after( *a.before_begin() ) == &node ); // strictly you shouldn't de-ref the before_begin iterator, it doesn't point to a node
        requireEmptyInvariants( a );
    }

    SECTION( "two/before-begin", "initially two items, remove after before_begin")
    { // two, before_begin
        list_type a;
        typename list_type::node_type nodes[2];
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        REQUIRE( a.remove_after( *a.before_begin() ) == &nodes[0] ); // strictly you shouldn't de-ref the before_begin iterator, it doesn't point to a node
        requireSingleNodeInvariants( a, &nodes[1] );
    }

    SECTION( "many/before_begin", "initially many items, remove after before_begin")
    { // many items, before_begin
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        REQUIRE( a.remove_after( *a.before_begin() ) == &nodes[0] ); // strictly you shouldn't de-ref the before_begin iterator, it doesn't point to a node
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }
}


template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticRemoveAfter2NodePtrTest( requireEmptyInvariantsT& /*requireEmptyInvariants*/,
    requireSingleNodeInvariantsT& requireSingleNodeInvariants,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    SECTION( "two/front", "init with two items, remove after front")
    {
        list_type a;
        typename list_type::node_type nodes[2];
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        REQUIRE( a.remove_after( a.front() ) == &nodes[1] );
        requireSingleNodeInvariants( a, &nodes[0] );
    }

    SECTION( "many/front", "init with many items, remove after front")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[0] );
        a.push_front( &nodes[1] );
        REQUIRE( a.remove_after( a.front() ) == &nodes[0] );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/next-to-front", "init with many items, remove after next(front)")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[0] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        REQUIRE( a.remove_after( a.next(a.front()) ) == &nodes[0] );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/previous-to-last", "init with many items, remove after previous to last")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[0] );
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        REQUIRE( a.remove_after( &nodes[4] ) == &nodes[0] );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }
}

template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticRemoveAfterIterTest( requireEmptyInvariantsT& requireEmptyInvariants,
    requireSingleNodeInvariantsT& requireSingleNodeInvariants,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    SECTION( "one/before_begin", "init with one item, remove_after before_begin")
    {
        list_type a;
        typename list_type::node_type node;
        a.push_front( &node );
        a.remove_after( a.before_begin() );
        requireEmptyInvariants( a );
    }

    SECTION( "two/before_begin", "init with two items, remove_after before_begin")
    {
        list_type a;
        typename list_type::node_type nodes[2];
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.remove_after( a.before_begin() );
        requireSingleNodeInvariants( a, &nodes[1] );
    }

    SECTION( "two/begin", "init with two items, remove_after begin")
    {
        list_type a;
        typename list_type::node_type nodes[2];
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.remove_after( a.begin() );
        requireSingleNodeInvariants( a, &nodes[0] );
    }

    SECTION( "many/before_begin", "init with many items, remove_after before_begin")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        a.remove_after( a.before_begin() );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/begin", "init with many items, remove_after begin")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[0] );
        a.push_front( &nodes[1] );
        a.remove_after( a.begin() );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/inc-begin", "init with many items, remove_after ++begin")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[0] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.remove_after( ++a.begin() );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/previous-to-last", "init with many items, remove_after previous to last")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[0] );
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        typename list_type::iterator i = a.begin();
        ++i;
        ++i;
        ++i;
        a.remove_after( i );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }
}

template<typename list_type,
    typename requireEmptyInvariantsT,
    typename requireSingleNodeInvariantsT,
    typename requireMoreThanOneNodeInvariantsT >
void axiomaticEraseAfterIterTest( requireEmptyInvariantsT& requireEmptyInvariants,
    requireSingleNodeInvariantsT& requireSingleNodeInvariants,
    requireMoreThanOneNodeInvariantsT& requireMoreThanOneNodeInvariants )
{
    // similar tests to remove_after except test the return value of erase_after

    SECTION( "one", "init with one item, erase_after before_begin")
    {
        list_type a;
        typename list_type::node_type node;
        a.push_front( &node );
        typename list_type::iterator i = a.erase_after( a.before_begin() );
        REQUIRE( i == a.end() );
        requireEmptyInvariants( a );
    }

    SECTION( "two/before-begin", "init with two items, erase_after before_begin")
    {
        list_type a;
        typename list_type::node_type nodes[2];
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        typename list_type::iterator i = a.erase_after( a.before_begin() );
        REQUIRE( i == a.begin() );
        requireSingleNodeInvariants( a, &nodes[1] );
    }

    SECTION( "two/begin", "init with two items, erase_after begin")
    {
        list_type a;
        typename list_type::node_type nodes[2];
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        typename list_type::iterator i = a.erase_after( a.begin() );
        REQUIRE( i == a.end() );
        requireSingleNodeInvariants( a, &nodes[0] );
    }

    SECTION( "many/before-begin", "init with many items, erase_after before_begin")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        a.push_front( &nodes[0] );
        typename list_type::iterator i = a.erase_after( a.before_begin() );
        REQUIRE( i == a.begin() );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/begin", "init with many items, erase_after begin")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[0] );
        a.push_front( &nodes[1] );
        typename list_type::iterator i = a.erase_after( a.begin() );
        typename list_type::iterator j = a.begin();
        ++j;
        REQUIRE( i == j );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/inc-begin", "init with many items, erase_after ++begin")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[0] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        typename list_type::iterator i = a.erase_after( ++a.begin() );
        typename list_type::iterator j = a.begin();
        ++j;
        ++j;
        REQUIRE( i == j );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }

    SECTION( "many/previous-to-last", "init with many items, erase_after previous to last")
    {
        list_type a;
        typename list_type::node_type nodes[5];
        a.push_front( &nodes[0] );
        a.push_front( &nodes[4] );
        a.push_front( &nodes[3] );
        a.push_front( &nodes[2] );
        a.push_front( &nodes[1] );
        typename list_type::iterator i = a.begin();
        ++i;
        ++i;
        ++i;
        typename list_type::iterator j = a.erase_after( i );
        REQUIRE( j == a.end() );
        requireMoreThanOneNodeInvariants( a, &nodes[1], 4 );
    }
}

#endif /* INCLUDED_QWLISTSAXIOMATICTESTSSHARED_H */

/* -----------------------------------------------------------------------
Last reviewed: June 30, 2013
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
