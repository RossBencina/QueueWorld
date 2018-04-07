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
#ifndef INCLUDED_QWLISTSADHOCTESTSSHARED_H
#define INCLUDED_QWLISTSADHOCTESTSSHARED_H

#include "QwConfig.h"


template< typename ListT >
inline void emptyListTest( ListT& a, ListT& b )
{
    // operations on empty lists

    REQUIRE( a.empty() == true );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == false );

    REQUIRE( a.begin() == a.end() );

    REQUIRE( b.empty() == true );
    a.swap( b );
    REQUIRE( a.empty() == true );
    REQUIRE( b.empty() == true );
    swap( a, b );
    REQUIRE( a.empty() == true );
    REQUIRE( b.empty() == true );

    a.clear();
    REQUIRE( a.empty() == true );
}

template< typename ListT >
inline void singleItemSListTest( ListT& a, ListT& b, typename ListT::node_ptr_type nodePtr )
{
    // slist test. tests before_begin()
    // list operations with 1 node/element

    REQUIRE( a.empty() == false );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.size_is_greater_than_1() == false );

    REQUIRE( a.front() == nodePtr );
    REQUIRE( ListT::next(nodePtr) == (TestNode*)NULL );

    REQUIRE( a.begin() != a.end() );
    REQUIRE( *(a.begin()) == nodePtr );

    {
        typename ListT::iterator i = a.before_begin();
        ++i;
        REQUIRE( i == a.begin() );

        typename ListT::iterator j = a.before_begin();
        j++;
        REQUIRE( j == a.begin() );
    }

    a.swap( b );
    REQUIRE( a.empty() == true );
    REQUIRE( b.empty() == false );
    REQUIRE( b.front() == nodePtr );
    swap( a, b );
    REQUIRE( a.empty() == false );
    REQUIRE( b.empty() == true );

    a.clear();
    REQUIRE( a.empty() == true );

    // push front / pop front
    a.push_front( nodePtr );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.pop_front() == nodePtr );
    REQUIRE( a.empty() == true );

    // insert_after/remove_after using before_begin iterator
    a.insert_after( a.before_begin(), nodePtr );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.front() == nodePtr );
    a.remove_after( a.before_begin() );
    REQUIRE( a.empty() == true );

    // insert_after/remove after using before_begin pseudo-node
    // we shouldn't really allow to de-reference the before_begin iterator
    typename ListT::node_type *bb = *(a.before_begin());
    a.insert_after( bb, nodePtr );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.front() == nodePtr );
    a.remove_after( bb );
    REQUIRE( a.empty() == true );

    // test erase_after iterator
    a.push_front( nodePtr );
    a.erase_after( a.before_begin() );
    REQUIRE( a.empty() );
}

template< typename ListT >
inline void singleItemListTest( ListT& a, ListT& b, typename ListT::node_ptr_type nodePtr )
{
    // list test, doesn't test before_begin()
    // list operations with 1 node/element

    REQUIRE( a.empty() == false );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.size_is_greater_than_1() == false );

    REQUIRE( a.front() == nodePtr );
    REQUIRE( ListT::next(nodePtr) == (ListT::node_ptr_type)NULL );

    REQUIRE( a.begin() != a.end() );
    REQUIRE( *(a.begin()) == nodePtr );

    a.swap( b );
    REQUIRE( a.empty() == true );
    REQUIRE( b.empty() == false );
    REQUIRE( b.front() == nodePtr );
    swap( a, b );
    REQUIRE( a.empty() == false );
    REQUIRE( b.empty() == true );

    a.clear();
    REQUIRE( a.empty() == true );

    // push front / pop front
    a.push_front( nodePtr );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.pop_front() == nodePtr );
    REQUIRE( a.empty() == true );

    // push back / pop back
    a.push_back( nodePtr );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.pop_back() == nodePtr );
    REQUIRE( a.empty() == true );

    // push front / pop back
    a.push_front( nodePtr );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.pop_back() == nodePtr );
    REQUIRE( a.empty() == true );

    // push back / pop front
    a.push_back( nodePtr );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.pop_front() == nodePtr );
    REQUIRE( a.empty() == true );

    // insert at end/remove front
    a.insert( a.end(), nodePtr );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.front() == nodePtr );
    a.remove( a.front() );
    REQUIRE( a.empty() == true );

    // insert at end/erase at begin
    a.insert( a.end(), nodePtr );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.front() == nodePtr );
    a.erase( a.begin() );
    REQUIRE( a.empty() == true );
}


template< typename ListT >
inline void twoItemSListTest( ListT& a, ListT& b,
        typename ListT::node_ptr_type node1Ptr,
        typename ListT::node_ptr_type node2Ptr )
{
    // slist operations with 2 nodes/elements

    REQUIRE( a.empty() == false );
    REQUIRE( b.empty() == true );

    a.push_front( node1Ptr );
    REQUIRE( a.empty() == false );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );

    REQUIRE( a.front() == node1Ptr );
    REQUIRE( ListT::next(node1Ptr) == node2Ptr );
    REQUIRE( ListT::next(node2Ptr) == (ListT::node_ptr_type)NULL );

    REQUIRE( a.begin() != a.end() );
    REQUIRE( *(a.begin()) == node1Ptr );

    {
        typename ListT::iterator i = a.before_begin();
        ++i;
        REQUIRE( i == a.begin() );

        typename ListT::iterator j = a.before_begin();
        j++;
        REQUIRE( j == a.begin() );
    }

    // iterator pre-increment
    int j=0;
    for( typename ListT::iterator i=a.begin(); i != a.end(); ++i, ++j )
        REQUIRE( (*i)->value == j );

    // iterator post-increment
    j=0;
    for( typename ListT::iterator i=a.begin(); i != a.end(); i++, ++j )
        REQUIRE( (*i)->value == j );

    a.swap( b );
    REQUIRE( a.empty() == true );
    REQUIRE( b.empty() == false );
    REQUIRE( b.front() == node1Ptr );
    swap( a, b );
    REQUIRE( a.empty() == false );
    REQUIRE( b.empty() == true );

    a.clear();
    REQUIRE( a.empty() == true );

    // push front / pop front
    REQUIRE( a.front() == (ListT::node_ptr_type)NULL );
    a.push_front( node2Ptr );
    a.push_front( node1Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.pop_front() == node1Ptr );
    REQUIRE( a.pop_front() == node2Ptr );
    REQUIRE( a.empty() == true );

    // insert_after/remove_after using before_begin iterator
    a.insert_after( a.before_begin(), node2Ptr );
    a.insert_after( a.before_begin(), node1Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.front() == node1Ptr );
    a.remove_after( a.before_begin() );
    REQUIRE( a.front() == node2Ptr );
    a.remove_after( a.before_begin() );
    REQUIRE( a.empty() == true );


    // insert_after/remove_after using begin iterator
    a.insert_after( a.before_begin(), node1Ptr );
    a.insert_after( a.begin(), node2Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.front() == node1Ptr );
    REQUIRE( ListT::next(a.front()) == node2Ptr );
    a.remove_after( a.begin() );
    REQUIRE( a.front() == node1Ptr );
    a.pop_front();
    REQUIRE( a.empty() == true );


    // insert_after/remove after using before_begin pseudo-node
    // we shouldn't really allow to de-reference the before_begin iterator
    typename ListT::node_type *bb = *(a.before_begin());
    a.insert_after( bb, node2Ptr );
    a.insert_after( bb, node1Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.front() == node1Ptr );
    a.remove_after( bb );
    REQUIRE( a.front() == node2Ptr );
    a.remove_after( bb );
    REQUIRE( a.empty() == true );

    // test erase_after iterator
    a.push_front( node2Ptr );
    a.push_front( node1Ptr );
    a.erase_after( a.begin() );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.front() == node1Ptr );
    a.clear();
}


template< typename ListT >
inline void twoItemListTest( ListT& a, ListT& b,
    typename ListT::node_ptr_type node1Ptr,
    typename ListT::node_ptr_type node2Ptr )
{
    // list operations with 2 nodes/elements

    REQUIRE( a.empty() == false );
    REQUIRE( b.empty() == true );

    a.push_front( node1Ptr );
    REQUIRE( a.empty() == false );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );

    REQUIRE( a.front() == node1Ptr );
    REQUIRE( ListT::next(node1Ptr) == node2Ptr );
    REQUIRE( ListT::next(node2Ptr) == (ListT::node_ptr_type)NULL );

    REQUIRE( a.begin() != a.end() );
    REQUIRE( *(a.begin()) == node1Ptr );

    // iterator pre-increment
    int j=0;
    for( typename ListT::iterator i=a.begin(); i != a.end(); ++i, ++j )
        REQUIRE( (*i)->value == j );

    // iterator post-increment
    j=0;
    for( typename ListT::iterator i=a.begin(); i != a.end(); i++, ++j )
        REQUIRE( (*i)->value == j );

    // iterator pre-decrement
    {
        typename ListT::iterator i=a.end();
        REQUIRE( (*--i)->value == 1 );
        REQUIRE( (*--i)->value == 0 );
        REQUIRE( i == a.begin() );
    }

    // iterator post-decrement
    {
        typename ListT::iterator i=a.end();
        i--; REQUIRE( (*i--)->value == 1 );
        REQUIRE( (*i)->value == 0 );
        REQUIRE( i == a.begin() );
    }

    a.swap( b );
    REQUIRE( a.empty() == true );
    REQUIRE( b.empty() == false );
    REQUIRE( b.front() == node1Ptr );
    swap( a, b );
    REQUIRE( a.empty() == false );
    REQUIRE( b.empty() == true );

    a.clear();
    REQUIRE( a.empty() == true );

    // push front / pop front
    a.push_front( node2Ptr );
    a.push_front( node1Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.pop_front() == node1Ptr );
    REQUIRE( a.pop_front() == node2Ptr );
    REQUIRE( a.empty() == true );

    // push back / pop back
    a.push_back( node2Ptr );
    a.push_back( node1Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.pop_back() == node1Ptr );
    REQUIRE( a.pop_back() == node2Ptr );
    REQUIRE( a.empty() == true );

    // push front / pop back
    a.push_front( node2Ptr );
    a.push_front( node1Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.pop_back() == node2Ptr );
    REQUIRE( a.pop_back() == node1Ptr );
    REQUIRE( a.empty() == true );

    // push back / pop front
    a.push_back( node2Ptr );
    a.push_back( node1Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.pop_front() == node2Ptr );
    REQUIRE( a.pop_front() == node1Ptr );
    REQUIRE( a.empty() == true );

    // insert at end/remove front
    a.insert( a.end(), node2Ptr );
    a.insert( a.begin(), node1Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.front() == node1Ptr );
    REQUIRE( ListT::next(a.front()) == node2Ptr );
    REQUIRE( ListT::previous(a.back()) == node1Ptr );
    a.remove( a.front() );
    REQUIRE( a.front() == node2Ptr );
    REQUIRE( a.size_is_1() == true );
    a.remove( a.front() );
    REQUIRE( a.empty() == true );

    // insert at begin/remove front
    a.insert( a.begin(), node2Ptr );
    a.insert( a.begin(), node1Ptr );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );
    REQUIRE( a.front() == node1Ptr );
    REQUIRE( ListT::next(a.front()) == node2Ptr );
    REQUIRE( ListT::previous(a.back()) == node1Ptr );
    a.remove( a.front() );
    REQUIRE( a.front() == node2Ptr );
    REQUIRE( a.size_is_1() == true );
    a.remove( a.front() );
    REQUIRE( a.empty() == true );

    // test eraser iterator
    a.push_front( node2Ptr );
    a.push_front( node1Ptr );
    a.erase( a.begin() );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.front() == node2Ptr );
    a.clear();
}


template< typename ListT >
inline void backAndPushBackListTest( ListT& a, ListT& b,
    typename ListT::node_ptr_type node1Ptr,
    typename ListT::node_ptr_type node2Ptr,
    typename ListT::node_ptr_type node3Ptr )
{
    // test that back() and front() are correct when pushing from
    // back or front, one, two or three elements
    // test that back and front are correct after swapping

    //REQUIRE( a.back() == (ListT::node_ptr_type)NULL );

    a.push_back( node1Ptr );
    REQUIRE( a.back() == node1Ptr );
    REQUIRE( a.front() == node1Ptr );

    a.push_back( node2Ptr );
    REQUIRE( a.back() == node2Ptr );
    REQUIRE( a.front() == node1Ptr );

    a.push_back( node3Ptr );
    REQUIRE( a.back() == node3Ptr );
    REQUIRE( a.front() == node1Ptr );

    a.pop_front();
    REQUIRE( a.back() == node3Ptr );
    REQUIRE( a.front() == node2Ptr );

    a.pop_front();
    REQUIRE( a.back() == node3Ptr );
    REQUIRE( a.front() == node3Ptr );

    a.pop_front();
    REQUIRE( a.empty() );
    //REQUIRE( a.back() == (ListT::node_ptr_type)NULL );
    //REQUIRE( a.front() == (ListT::node_ptr_type)NULL );

    a.push_front( node1Ptr );
    REQUIRE( a.back() == node1Ptr );
    REQUIRE( a.front() == node1Ptr );

    a.push_front( node2Ptr );
    REQUIRE( a.back() == node1Ptr );
    REQUIRE( a.front() == node2Ptr );

    a.push_front( node3Ptr );
    REQUIRE( a.back() == node1Ptr );
    REQUIRE( a.front() == node3Ptr );

    a.clear();

    a.push_back( node1Ptr );
    REQUIRE( a.back() == node1Ptr );
    REQUIRE( a.front() == node1Ptr );

    swap( a, b );
    REQUIRE( a.empty() );
    REQUIRE( b.back() == node1Ptr );
    REQUIRE( b.front() == node1Ptr );

    b.push_back( node2Ptr );
    REQUIRE( b.back() == node2Ptr );
    REQUIRE( b.front() == node1Ptr );

    swap( a, b );
    REQUIRE( b.empty() );
    REQUIRE( a.back() == node2Ptr );
    REQUIRE( a.front() == node1Ptr );

    a.push_back( node3Ptr );
    REQUIRE( a.back() == node3Ptr );
    REQUIRE( a.front() == node1Ptr );

    swap( a, b );
    REQUIRE( a.empty() );
    REQUIRE( b.back() == node3Ptr );
    REQUIRE( b.front() == node1Ptr );

    b.pop_front();
    REQUIRE( b.back() == node3Ptr );
    REQUIRE( b.front() == node2Ptr );

    b.clear();
}

template< typename ListT, int MAX_COUNT >
inline void manyItemsSListTest( typename ListT::node_ptr_type nodes )
{
    // slist operations with many nodes/elements

    for( int i=0; i < MAX_COUNT; ++i )
        nodes[i].value = i;

    {
        // push elements on in reverse-value order (so they're in order in the list)
        ListT a;
        for( int i=0; i < MAX_COUNT; ++i )
            a.push_front( &nodes[(MAX_COUNT-1)-i] );

        REQUIRE( a.empty() == false );
        REQUIRE( a.size_is_1() == false );
        REQUIRE( a.size_is_greater_than_1() == true );

        // preincrement iteration
        int j=0;
        for( typename ListT::iterator i = a.begin(); i != a.end(); ++i, ++j )
            REQUIRE( (*i)->value == j );

        // postincrement iteration
        j=0;
        for( typename ListT::iterator i = a.begin(); i != a.end(); i++, ++j )
            REQUIRE( (*i)->value == j );

        for( int i=0; i < MAX_COUNT; ++i )
            REQUIRE( a.pop_front()->value == i );

        REQUIRE( a.empty() );
    }


    {
        ListT a;
        for( int i=0; i < MAX_COUNT; ++i )
            a.insert_after( a.before_begin(), &nodes[4-i] );

        for( int i=0; i < MAX_COUNT; ++i ){
            REQUIRE( a.front()->value == i );
            a.remove_after( a.before_begin() );
        }
    }

    {
        ListT a;
        for( int i=0; i < MAX_COUNT; ++i )
            a.insert_after( a.before_begin(), &nodes[4-i] );

        // test removing after begin keeps first element in place
        for( int i=0; i < (MAX_COUNT-1); ++i ){
            REQUIRE( a.front()->value ==  0 );
            a.remove_after( a.begin() );
        }

        REQUIRE( a.empty() == false );
        REQUIRE( a.size_is_1() == true );
        REQUIRE( a.size_is_greater_than_1() == false );
        REQUIRE( a.front()->value == 0 );
        a.pop_front();
    }

    // in order insert using iterator
    {
        ListT a;
        {
            typename ListT::iterator i=a.before_begin();
            for( int j=0; j < MAX_COUNT; ++j ){
                a.insert_after( i, &nodes[j] );
                ++i;
            }
        }
        {
            int j=0;
            for( typename ListT::iterator i = a.begin(); i != a.end(); ++i, ++j )
                REQUIRE( (*i)->value == j );
        }
        a.clear();
    }

    // in order insert then removal in reverse order using saved iterators
    {
        ListT a;
        typename ListT::iterator i=a.before_begin();
        typename ListT::iterator is[MAX_COUNT];
        for( int j=0; j < MAX_COUNT; ++j ){
            is[j] = i;
            a.insert_after( i, &nodes[j] );
            ++i;
        }

        for( signed int j=(MAX_COUNT-1); j >= 0; --j ){
            REQUIRE( ListT::next((*(is[j])))->value == j );
            a.remove_after( is[j] );
        }

        REQUIRE( a.empty() );
    }
}

template< typename ListT, int MAX_COUNT >
inline void manyItemsListTest( typename ListT::node_ptr_type nodes )
{
    // list operations with many nodes/elements

    for( int i=0; i < MAX_COUNT; ++i )
        nodes[i].value = i;

    {
        // push elements on in reverse-value order (so they're in order in the list)
        ListT a;
        for( int i=0; i < MAX_COUNT; ++i )
            a.push_front( &nodes[(MAX_COUNT-1)-i] );

        REQUIRE( a.empty() == false );
        REQUIRE( a.size_is_1() == false );
        REQUIRE( a.size_is_greater_than_1() == true );

        // preincrement iteration
        int j=0;
        for( typename ListT::iterator i = a.begin(); i != a.end(); ++i, ++j )
            REQUIRE( (*i)->value == j );

        // postincrement iteration
        j=0;
        for( typename ListT::iterator i = a.begin(); i != a.end(); i++, ++j )
            REQUIRE( (*i)->value == j );

        // predecrement iteration
        {
            typename ListT::iterator i = --a.end();
            for( j = MAX_COUNT - 1; j >= 0; --j ){
                REQUIRE( (*i)->value == j );
                --i;
            }
        }
        // postdecrement iteration
        {
            typename ListT::iterator i = a.end();
            i--;
            for( j = MAX_COUNT - 1; j >= 0; --j ){
                REQUIRE( (*i)->value == j );
                i--;
            }
        }

        for( int i=0; i < MAX_COUNT; ++i )
            REQUIRE( a.pop_front()->value == i );

        REQUIRE( a.empty() );
    }


    {
        ListT a;
        for( int i=0; i < MAX_COUNT; ++i )
            a.insert( a.begin(), &nodes[4-i] );

        for( int i=0; i < MAX_COUNT; ++i ){
            REQUIRE( a.front()->value == i );
            a.erase( a.begin() );
        }
    }


    {
        ListT a;
        for( int i=0; i < MAX_COUNT; ++i )
            a.insert( a.begin(), &nodes[4-i] );

        // test removing after begin keeps first element in place
        for( int i=0; i < (MAX_COUNT-1); ++i ){
            REQUIRE( a.front()->value == 0 );
            a.remove( *(++a.begin()) );
        }

        REQUIRE( a.empty() == false );
        REQUIRE( a.size_is_1() == true );
        REQUIRE( a.size_is_greater_than_1() == false );
        REQUIRE( a.front()->value == 0 );
        a.pop_front();
    }


    // in order insert using iterator
    {
        ListT a;
        {
            typename ListT::iterator i=a.begin();
            for( int j=0; j < MAX_COUNT; ++j ){
                a.insert( i, &nodes[j] );
                ++i;
            }
        }
        {
            int j=0;
            for( typename ListT::iterator i = a.begin(); i != a.end(); ++i, ++j )
                REQUIRE( (*i)->value == j );
        }
        a.clear();
    }


    // in order insert then removal in reverse order using saved iterators
    {
        ListT a;
        typename ListT::iterator i=a.begin();
        typename ListT::iterator is[MAX_COUNT];
        for( int j=0; j < MAX_COUNT; ++j ){
            is[j] = i;
            a.insert( i, &nodes[j] );
            ++i;
        }

        for( signed int j=(MAX_COUNT-1); j >= 0; --j ){
            REQUIRE( (*(is[j]))->value == j );
            a.erase( is[j] );
        }

        REQUIRE( a.empty() );
    }
}

template< typename ListT >
inline size_t countElements( const ListT& list )
{
    size_t result = 0;

    for( typename ListT::const_iterator i = list.begin(); i != list.end(); ++i )
        ++result;

    return result;
}

template< typename ListT >
inline void frontStackTest( ListT& list, typename ListT::node_ptr_type testNodes, size_t maxCount )
{
    REQUIRE( list.empty() );
    REQUIRE( countElements( list ) == 0 );

    // push_front + pop_front up to i, checking that correct counts are present

    for( size_t i=1; i < maxCount; ++i ){

        REQUIRE( list.empty() );
        for( size_t j=0; j < i; ++j ){
            REQUIRE( countElements( list ) == j );
            list.push_front( &(testNodes[j]) );
            REQUIRE( countElements( list ) == j + 1 );
        }

        REQUIRE( !list.empty() );
        REQUIRE( list.begin() != list.end() );
        if( i == 1 ){
            REQUIRE( list.size_is_1() == true );
            REQUIRE( list.size_is_greater_than_1() == false );
            REQUIRE( list.front() == *list.begin() );
        }else{
            REQUIRE( list.size_is_1() == false );
            REQUIRE( list.size_is_greater_than_1() == true );
            REQUIRE( list.front() == *list.begin() );
        }

        for( size_t j=i; j > 0; --j ){
            REQUIRE( countElements( list ) == j );
            list.pop_front();
            REQUIRE( countElements( list ) == j - 1 );
        }

        REQUIRE( list.empty() );
    }
}

template< typename ListT >
inline void frontStackTest_withBackChecks( ListT& list, typename ListT::node_ptr_type testNodes, size_t maxCount )
{
    REQUIRE( list.empty() );
    REQUIRE( countElements( list ) == 0 );

    // push_front + pop_front up to i, checking that correct counts are present

    for( size_t i=1; i < maxCount; ++i ){

        REQUIRE( list.empty() );
        for( size_t j=0; j < i; ++j ){
            REQUIRE( countElements( list ) == j );
            list.push_front( &(testNodes[j]) );
            REQUIRE( countElements( list ) == j + 1 );
        }

        REQUIRE( !list.empty() );
        REQUIRE( list.begin() != list.end() );
        if( i == 1 ){
            REQUIRE( list.size_is_1() == true );
            REQUIRE( list.size_is_greater_than_1() == false );
            REQUIRE( list.front() == list.back() );
            REQUIRE( list.front() == *list.begin() );
            REQUIRE( list.back() == *list.begin() );
        }else{
            REQUIRE( list.size_is_1() == false );
            REQUIRE( list.size_is_greater_than_1() == true );
            REQUIRE( list.front() != list.back() );
            REQUIRE( list.front() == *list.begin() );
            REQUIRE( list.back() != *list.begin() );
        }

        for( size_t j=i; j > 0; --j ){
            REQUIRE( countElements( list ) == j );
            list.pop_front();
            REQUIRE( countElements( list ) == j - 1 );
        }

        REQUIRE( list.empty() );
    }
}

template< typename ListT >
inline void backStackTest_withBackChecks( ListT& list, typename ListT::node_ptr_type testNodes, size_t maxCount )
{
    // same as frontStackTest_withBackChecks except front and back are reversed

    REQUIRE( list.empty() );
    REQUIRE( countElements( list ) == 0 );

    // push_back + pop_back up to i, checking that correct counts are present

    for( size_t i=1; i < maxCount; ++i ){

        REQUIRE( list.empty() );
        for( size_t j=0; j < i; ++j ){
            REQUIRE( countElements( list ) == j );
            list.push_back( &(testNodes[j]) );
            REQUIRE( countElements( list ) == j + 1 );
        }

        REQUIRE( !list.empty() );
        REQUIRE( list.begin() != list.end() );
        if( i == 1 ){
            REQUIRE( list.size_is_1() == true );
            REQUIRE( list.size_is_greater_than_1() == false );
            REQUIRE( list.front() == list.back() );
            REQUIRE( list.front() == *list.begin() );
            REQUIRE( list.back() == *list.begin() );
        }else{
            REQUIRE( list.size_is_1() == false );
            REQUIRE( list.size_is_greater_than_1() == true );
            REQUIRE( list.front() != list.back() );
            REQUIRE( list.front() == *list.begin() );
            REQUIRE( list.back() != *list.begin() );
        }

        for( size_t j=i; j > 0; --j ){
            REQUIRE( countElements( list ) == j );
            list.pop_back();
            REQUIRE( countElements( list ) == j - 1 );
        }

        REQUIRE( list.empty() );
    }
}

template< typename ListT >
inline void backQueueTest( ListT& list, typename ListT::node_ptr_type testNodes, size_t maxCount )
{
    REQUIRE( list.empty() );
    REQUIRE( countElements( list ) == 0 );

    // push_back + pop_front up to i, checking that correct counts are present

    for( size_t i=1; i < maxCount; ++i ){

        REQUIRE( list.empty() );
        for( size_t j=0; j < i; ++j ){
            REQUIRE( countElements( list ) == j );
            list.push_back( &(testNodes[j]) );
            REQUIRE( countElements( list ) == j + 1 );
        }

        REQUIRE( !list.empty() );
        REQUIRE( list.begin() != list.end() );
        if( i == 1 ){
            REQUIRE( list.size_is_1() == true );
            REQUIRE( list.size_is_greater_than_1() == false );
            REQUIRE( list.front() == list.back() );
            REQUIRE( list.front() == *list.begin() );
            REQUIRE( list.back() == *list.begin() );
        }else{
            REQUIRE( list.size_is_1() == false );
            REQUIRE( list.size_is_greater_than_1() == true );
            REQUIRE( list.front() != list.back() );
            REQUIRE( list.front() == *list.begin() );
            REQUIRE( list.back() != *list.begin() );
        }

        for( size_t j=i; j > 0; --j ){
            REQUIRE( countElements( list ) == j );
            list.pop_front();
            REQUIRE( countElements( list ) == j - 1 );
        }

        REQUIRE( list.empty() );
    }
}

template< typename ListT >
inline void frontQueueTest( ListT& list, typename ListT::node_ptr_type testNodes, size_t maxCount )
{
    REQUIRE( list.empty() );
    REQUIRE( countElements( list ) == 0 );

    // push_front + pop_back up to i, checking that correct counts are present

    for( size_t i=1; i < maxCount; ++i ){

        REQUIRE( list.empty() );
        for( size_t j=0; j < i; ++j ){
            REQUIRE( countElements( list ) == j );
            list.push_front( &(testNodes[j]) );
            REQUIRE( countElements( list ) == j + 1 );
        }

        REQUIRE( !list.empty() );
        REQUIRE( list.begin() != list.end() );
        if( i == 1 ){
            REQUIRE( list.size_is_1() == true );
            REQUIRE( list.size_is_greater_than_1() == false );
            REQUIRE( list.front() == list.back() );
            REQUIRE( list.front() == *list.begin() );
            REQUIRE( list.back() == *list.begin() );
        }else{
            REQUIRE( list.size_is_1() == false );
            REQUIRE( list.size_is_greater_than_1() == true );
            REQUIRE( list.front() != list.back() );
            REQUIRE( list.front() == *list.begin() );
            REQUIRE( list.back() != *list.begin() );
        }

        for( size_t j=i; j > 0; --j ){
            REQUIRE( countElements( list ) == j );
            list.pop_back();
            REQUIRE( countElements( list ) == j - 1 );
        }

        REQUIRE( list.empty() );
    }
}

#endif /* INCLUDED_QWLISTSADHOCTESTSSHARED_H */

/* -----------------------------------------------------------------------
Last reviewed: June 30, 2013
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
