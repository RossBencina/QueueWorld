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
#ifndef INCLUDED_QWLISTSRANDOMISEDTESTSSHARED_H
#define INCLUDED_QWLISTSRANDOMISEDTESTSSHARED_H

#include "QwConfig.h"

#include <cstdlib> // rand and srand
#include <ctime> // time()

// minimal
//#define QW_RANDOMISED_TEST_OBJECT_COUNT     5
//#define QW_RANDOMISED_TEST_ITERATION_COUNT  20

// normal
#define QW_RANDOMISED_TEST_OBJECT_COUNT     50
#define QW_RANDOMISED_TEST_ITERATION_COUNT  100

// big
//#define QW_RANDOMISED_TEST_OBJECT_COUNT     50
//#define QW_RANDOMISED_TEST_ITERATION_COUNT  10000


/*
    randomised fuzz test:

    use a pre-allocated stack of NODE_COUNT nodes

    keep track of how many nodes are currently the list under test

    each node has a flag to keep track of whether it's in the list (value= 0/1)

    at each step, choose randomly between available options
    (when full/empty there is only one option):
        a) insert a node (set its list membership flag before adding) :randomisedInsert()
        b) remove a node, check its list membership flag is correct and clear it :randomisedRemove()
    at each step: :verify()
        - validate that the correct number of nodes are in the list
        - validate that the list is well ordered by walking forwards (and backwards) through it)

    insertion is performed as a random choice between all available insertion operations
    removal is performed as a random choice between all available removal operations
    point in the list where the removal/insertion is performed is chosen randomly
*/

// verify that the list is correctly linked in the forwards direction
// by requiring that the traversal from front to back takes exactly
// expected count iterations. i.e. we don't reach the end early,
// and after expectedCount iterations we are at the end.
template<typename list_t>
void verifyForwards( list_t& list, int expectedCount )
{
    if( expectedCount == 0 ){
        REQUIRE( list.empty() );
    }else{
        typename list_t::node_ptr_type n = list.front();
        typename list_t::node_ptr_type past_back = 0;

        typename list_t::iterator i = list.begin();
        typename list_t::iterator end = list.end();

        for( int j=0; j < expectedCount; ++j )
        {
            REQUIRE( n != past_back );
            REQUIRE( i != end );

            n = list.next(n);
            ++i;
        }

        REQUIRE( n == past_back );
        REQUIRE( i == end );
    }
}

template<typename list_t>
void verifyBackwards( list_t& list, int expectedCount )
{
    if( expectedCount == 0 ){
        REQUIRE( list.empty() );
    }else{
        typename list_t::node_ptr_type n = list.back();
        typename list_t::node_ptr_type past_front = list.before_front_();

        typename list_t::iterator i = list.end();
        typename list_t::iterator begin = list.begin();

        for( int j=0; j < expectedCount; ++j )
        {
            REQUIRE( n != past_front );
            REQUIRE( i != begin );

            n = list.previous(n);
            --i;
        }

        REQUIRE( n == past_front );
        REQUIRE( i == begin );
    }
}

template<typename list_t,
    typename randomisedInsertT,
    typename randomisedRemoveT,
    typename verifyT >
void fuzzTest( randomisedInsertT& randomisedInsert,
    randomisedRemoveT& randomisedRemove,
    verifyT& verify )
{
    std::srand(static_cast<unsigned int>(std::time(0)));

    list_t a;
    typedef typename list_t::node_type node_t;


    const int NODE_COUNT = QW_RANDOMISED_TEST_OBJECT_COUNT;
    node_t nodes[NODE_COUNT];

    node_t *nodeStack[NODE_COUNT];
    for( int i=0; i < NODE_COUNT; ++i ){
        nodeStack[i] = &nodes[i];
        nodeStack[i]->value = 0;
    }

    int nodeStackTop = NODE_COUNT;
    int nodesInListCount = 0;

    const int ITERATION_COUNT = QW_RANDOMISED_TEST_ITERATION_COUNT;

    enum { BIAS_INSERTING=0, BIAS_REMOVING=1 };
    int biasState = BIAS_INSERTING;
    int bias[2] = { static_cast<int>(RAND_MAX * 0.55), static_cast<int>(RAND_MAX * 0.45) };

    for( int i=0; i < ITERATION_COUNT; ++i ){

        if( (rand() < bias[biasState] || nodesInListCount == 0) && nodeStackTop != 0 ){
            // insert
            node_t *node = nodeStack[ --nodeStackTop ];

            REQUIRE( node->value == 0 );
            node->value = 1;
            randomisedInsert( a, node, nodesInListCount );
            ++nodesInListCount;

            verify( a, nodesInListCount );

            // when we run out of nodes we bias towards removing until the list is empty
            if( nodeStackTop == 0 )
                biasState = BIAS_REMOVING;
        }else{
            // remove
            node_t *node = randomisedRemove( a, nodesInListCount );
            --nodesInListCount;

            REQUIRE( node->value == 1 );
            node->value = 0;
            nodeStack[ nodeStackTop++ ] = node;

            verify( a, nodesInListCount );

            // when list is empty we bias towards inserting until we run out of nodes
            if( nodesInListCount == 0 )
                biasState = BIAS_INSERTING;
        }
    }
}

#endif /* INCLUDED_QWLISTSRANDOMISEDTESTSSHARED_H */

/* -----------------------------------------------------------------------
Last reviewed: June 30, 2013
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
