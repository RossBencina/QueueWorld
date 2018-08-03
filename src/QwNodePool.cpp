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
#include "QwNodePool.h"

#undef max
#undef min

#include <algorithm>
#include <cassert>


// need x to be unsigned.
// "C Standard Section 4.1.5 defines size_t as an unsigned integral type of the result of the sizeof operator"
static size_t roundUpToNextPowerOfTwo(size_t x)
{
    x--;
    x |= x >> 1;  // handle  2 bit numbers
    x |= x >> 2;  // handle  4 bit numbers
    x |= x >> 4;  // handle  8 bit numbers
    x |= x >> 8;  // handle 16 bit numbers
    x |= x >> 16; // handle 32 bit numbers
    x++;

    return x;
}

#ifdef _WIN32

static void *qw_aligned_malloc( size_t size, size_t alignment )
{
    return _aligned_malloc(size, alignment);
}

static void qw_aligned_free( void *memblock )
{
    _aligned_free(memblock);
}

#else

static void *qw_aligned_malloc( size_t size, size_t alignment )
{
    void *result = 0;

    if (posix_memalign(&result, alignment, size)!=0)
        result = 0;

    return result;
}

static void qw_aligned_free( void *memblock )
{
    free(memblock);
}

#endif



QwRawNodePool::QwRawNodePool( size_t nodeSize, size_t maxNodes )
{
#if (QW_DEBUG_COUNT_NODE_ALLOCATIONS == 1)
    allocCount_._nonatomic = 0;
#endif

    assert( sizeof(top_) >= sizeof(abapointer_type) );

    // Align nodes on cache line boundaries to avoid false sharing
    size_t minNodeSize = sizeof(nodeindex_type); // nodes need to be large enough to embed their next ptr
    // Make node size a power of two to allow for using bit shift to convert between pointers and indices
    nodeSize_ = roundUpToNextPowerOfTwo(std::max(nodeSize, std::max(minNodeSize,CACHE_LINE_SIZE)));

    // Aligned allocation
    // TODO: use posix_memalign on unix
    // see also http://cottonvibes.blogspot.com.au/2011/01/dynamically-allocate-aligned-memory.html
    // and http://stackoverflow.com/questions/17378444/stdalign-and-stdaligned-storage-for-aligned-allocation-of-memory-blocks

    nodeStorage_ = (int8_t*)qw_aligned_malloc(nodeSize_*maxNodes,CACHE_LINE_SIZE);
    assert( nodeStorage_ != 0 );

    nodeArrayBase_ = nodeStorage_ - nodeSize_; // node index 0 is the null index, so we want nodeArrayBase_[1] --> nodeStorage_[0]

    nodeBitShift_ = 0;
    size_t x = 1;
    while (x < nodeSize_) {
        x = x << 1;
        ++nodeBitShift_;
    }
    assert( x == nodeSize_ ); // require node size to be a power of two

    size_t maxNodeIndex = maxNodes; // since node indices are 1-based, max index is N, not N-1

    // index is stored in the low bits of the packed pointer.
    // generate a bit mask for it.

    size_t nodeIndexEnd = roundUpToNextPowerOfTwo(maxNodeIndex); // valid node indices are [1,nodeIndexEnd)
    if (nodeIndexEnd==maxNodeIndex) // need an extra bit
        nodeIndexEnd = nodeIndexEnd << 1;
    indexMask_ = nodeIndexEnd-1;

    countMask_ = ~indexMask_; // count is in the high part
    countIncrement_ = nodeIndexEnd;

    // now that everything has been set up, push all nodes onto the stack
    stack_init();

    int8_t *p = nodeStorage_;
    for( size_t i=0; i < maxNodes; ++i ) {
        stack_push_nonatomic(p);
        p += nodeSize_;
    }
}

QwRawNodePool::~QwRawNodePool()
{
#if (QW_DEBUG_COUNT_NODE_ALLOCATIONS == 1)
    assert( allocCount_._nonatomic == 0 );
#endif

    qw_aligned_free(nodeStorage_);
}

/* -----------------------------------------------------------------------
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
Comments:
- constructor is a bit baroque
- factor aligned allocation into a separate module
- implement runtime cache line size query
-------------------------------------------------------------------------- */
