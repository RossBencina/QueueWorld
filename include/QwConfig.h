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
#ifndef INCLUDED_QWDEBUGCONFIGURATION_H
#define INCLUDED_QWDEBUGCONFIGURATION_H

// QW_VALIDATE_NODE_LINKS switches on the following behavior:
//  - Node links are zeroed after use.
//
//  - The client should ensure that node links are correctly zeroed
// 

#define QW_VALIDATE_NODE_LINKS

#ifdef QW_VALIDATE_NODE_LINKS
#define QW_CLEAR_LINK_FOR_VALIDATION(x) (x) = 0
#else
#define QW_CLEAR_LINK_FOR_VALIDATION(x)
#endif


// QW_DEBUG_COUNT_NODE_ALLOCATIONS causes NodePool to track the number
// of node allocations and deallocations. When enabled, NodePool's
// dtor will assert that all nodes have been deallocated.

#define QW_DEBUG_COUNT_NODE_ALLOCATIONS


// QW_DEBUG_COUNT_SHARED_BUFFER_ALLOCATIONS causes QwSharedBufferAllocator
// to track the number of shared buffer allocations and deallocations.
// You can call QwSharedBufferAllocator::checkForLeaks() at exit to see
// whether any buffers have leaked.

#define QW_DEBUG_COUNT_SHARED_BUFFER_ALLOCATIONS


#endif /* INCLUDED_QWDEBUGCONFIGURATION_H */