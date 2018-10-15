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
#ifndef INCLUDED_QWCONFIG_H
#define INCLUDED_QWCONFIG_H

/*
    QueueWorld configuration #defines

    Mostly these control validation checks used for debugging.
*/

// Cache line size. Used for rounding. TODO: provide runtime cache line size query.

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE (static_cast<size_t>(64))
#endif


// QW_VALIDATE_NODE_LINKS switches on the following behavior:
//  - Node links are set to nullptr after use.
//  - Node links are verified as zero before collections insert them.
//
// The client should ensure that node links are correctly set to nullptr
// before trying to add nodes to queues.
//
// To explicitly enable/disable define QW_VALIDATE_NODE_LINKS to 0 or 1
// with a compiler -D flag, otherwise a default value will be used.

#ifndef QW_VALIDATE_NODE_LINKS

    // If QW_VALIDATE_NODE_LINKS is not defined, use a computed default value
    #ifdef NDEBUG
        #define QW_VALIDATE_NODE_LINKS_DEFAULT 0
    #else
        #define QW_VALIDATE_NODE_LINKS_DEFAULT 1
    #endif

    #define QW_VALIDATE_NODE_LINKS QW_VALIDATE_NODE_LINKS_DEFAULT

#elif (QW_VALIDATE_NODE_LINKS != 0) && (QW_VALIDATE_NODE_LINKS != 1)

    #if defined(__GNUC__) || defined(__clang__)
        #warning "QW_VALIDATE_NODE_LINKS was defined but not 0 or 1. defaulting to 1."
    #else
        #pragma message "warning: QW_VALIDATE_NODE_LINKS was defined but not 0 or 1. defaulting to 1."
    #endif

    // If QW_VALIDATE_NODE_LINKS is defined, but is neither 0 nor 1, set it to 1
    #define QW_VALIDATE_NODE_LINKS 1

#endif

// QW_DEBUG_COUNT_NODE_ALLOCATIONS causes QwNodePool to track the number
// of node allocations and deallocations. When enabled, QwNodePool's
// dtor will assert that all nodes have been deallocated.
//
// To explicitly enable/disable define QW_VALIDATE_NODE_LINKS to 0 or 1
// with a compiler -D flag, otherwise a default value will be used.

#ifndef QW_DEBUG_COUNT_NODE_ALLOCATIONS

    // If QW_DEBUG_COUNT_NODE_ALLOCATIONS is not defined, use a computed default value
    #ifdef NDEBUG
        #define QW_DEBUG_COUNT_NODE_ALLOCATIONS_DEFAULT 0
    #else
        #define QW_DEBUG_COUNT_NODE_ALLOCATIONS_DEFAULT 1
    #endif

    #define QW_DEBUG_COUNT_NODE_ALLOCATIONS QW_DEBUG_COUNT_NODE_ALLOCATIONS_DEFAULT

#elif (QW_DEBUG_COUNT_NODE_ALLOCATIONS != 0) && (QW_DEBUG_COUNT_NODE_ALLOCATIONS != 1)

    #if defined(__GNUC__) || defined(__clang__)
        #warning "QW_DEBUG_COUNT_NODE_ALLOCATIONS was defined but not 0 or 1. defaulting to 1."
    #else
        #pragma message "warning: QW_DEBUG_COUNT_NODE_ALLOCATIONS was defined but not 0 or 1. defaulting to 1."
    #endif

    // If QW_DEBUG_COUNT_NODE_ALLOCATIONS is defined, but is neither 0 nor 1, set it to 1
    #define QW_DEBUG_COUNT_NODE_ALLOCATIONS 1

#endif

#endif /* INCLUDED_QWCONFIG_H */

/* -----------------------------------------------------------------------
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
Comments:
- cache line size needs to be determined at runtime
- consider tying debug checks to whether NDEBUG is define
-------------------------------------------------------------------------- */
