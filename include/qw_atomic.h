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
#ifndef INCLUDED_QW_ATOMIC_H
#define INCLUDED_QW_ATOMIC_H

#define NOMINMAX // suppress windows.h min/max

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
// Workaround for a bug in some older Windows SDKs where the following errors were triggered
// when winnt.h or intrin.h was included:
//   error C2733: second C linkage of overloaded function '_interlockedbittestandset' not allowed
//   error C2733: second C linkage of overloaded function '_interlockedbittestandreset' not allowed
// Note: Make sure the workaround is used prior to including intrin.h or windows.h elsewhere (e.g. before including mintomic.h)
// See: https://stackoverflow.com/questions/18253129/error-c2733-second-c-linkage-of-overloaded-function
#   pragma push_macro("_interlockedbittestandset")
#   pragma push_macro("_interlockedbittestandreset")
#   pragma push_macro("_interlockedbittestandset64")
#   pragma push_macro("_interlockedbittestandreset64")
#   define _interlockedbittestandset _local_interlockedbittestandset
#   define _interlockedbittestandreset _local_interlockedbittestandreset
#   define _interlockedbittestandset64 _local_interlockedbittestandset64
#   define _interlockedbittestandreset64 _local_interlockedbittestandreset64
#   include <intrin.h> // to force the header not to be included elsewhere
#   pragma pop_macro("_interlockedbittestandreset64")
#   pragma pop_macro("_interlockedbittestandset64")
#   pragma pop_macro("_interlockedbittestandreset")
#   pragma pop_macro("_interlockedbittestandset")
#endif

#include "mintomic/mintomic.h"

/*
    Additional atomic functions:

        qw_mint_exchange_32_relaxed
        qw_mint_exchange_64_relaxed
        qw_mint_exchange_ptr_relaxed

    Mintomic lacks atomic swap. I've posted an issue about this:
        https://github.com/mintomic/mintomic/issues/7

    For now we provide our own versions here. Perhaps we'll patch Mintomic later.
*/

#if MINT_COMPILER_MSVC

MINT_C_INLINE uint32_t qw_mint_exchange_32_relaxed(mint_atomic32_t *object, uint32_t operand)
{
    return _InterlockedExchange((volatile long*)object, operand);
}

MINT_C_INLINE uint64_t qw_mint_exchange_64_relaxed(mint_atomic64_t *object, uint64_t operand)
{
    return InterlockedExchange64((volatile LONG64*)object, operand);
}

#elif MINT_COMPILER_GCC && (MINT_CPU_X86 || MINT_CPU_X64)

MINT_C_INLINE uint32_t qw_mint_exchange_32_relaxed(mint_atomic32_t *object, uint32_t operand)
{
    // __sync_lock_test_and_set is equivalent to LOCK XCHG on Intel
    // "This built-in function, as described by Intel, is not a traditional test-and-set operation,"
    // "but rather an atomic exchange operation. It writes value into *ptr, and returns the"
    // "previous contents of *ptr."
    // http://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html#_005f_005fsync-Builtins
    // see also http://stackoverflow.com/questions/8268243/porting-interlockedexchange-using-gcc-intrinsics-only

    return __sync_lock_test_and_set(&(object->_nonatomic), operand);
}

MINT_C_INLINE uint64_t qw_mint_exchange_64_relaxed(mint_atomic64_t *object, uint64_t operand)
{
    return __sync_lock_test_and_set(&(object->_nonatomic), operand);
}

#else

// ARM has atomic SWAP:
// http://gcc.gnu.org/ml/gcc-help/2007-09/msg00111.html
// in all architectures: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489c/Chdbbbai.html
// but it is deprecated since ARM 7 and can be disable by the OS:
//      http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dht0008a/CJHBGBBJ.html
// should probably use LDREXD and STREXD
// http://infocenter.arm.com/help/topic/com.arm.doc.dht0008a/DHT0008A_arm_synchronization_primitives.pdf
// See e.g. NoBarrier_AtomicExchange here:
// https://chromium.googlesource.com/chromium/src/base/+/master/atomicops_internals_arm_gcc.h
// https://gitorious.org/0xdroid/system_core/commit/275a98353e1263b8cb32c2d6ebf61a6e45ff43d6
// https://groups.google.com/forum/#!topic/golang-dev/s0mj3RoAO9A
// https://codereview.appspot.com/12670045/
// see bionic swap:
// https://android.googlesource.com/platform/bionic/+/android-4.4_r1.1/libc/private/bionic_atomic_arm.h

#error /* no support */

#endif


//--------------------------------------------------------------
//  Pointer-sized atomic RMW operation wrappers
//--------------------------------------------------------------
#if MINT_PTR_SIZE == 4

MINT_C_INLINE void* qw_mint_exchange_ptr_relaxed(mint_atomicPtr_t *object, void *operand)
{
#if defined(_MSC_VER)
// suppress MSVC 2005 erroneous warnings
#pragma warning(push)
#pragma warning(disable: 4311) //warning C4311: 'type cast' : pointer truncation from 'void *' to 'uint32_t'
#pragma warning(disable: 4312) //warning C4312: 'type cast' : conversion from 'uint32_t' to 'void *' of greater size
#endif
    return (void*) qw_mint_exchange_32_relaxed((mint_atomic32_t *) object, (uint32_t)operand);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}

#elif MINT_PTR_SIZE == 8

MINT_C_INLINE void* qw_mint_exchange_ptr_relaxed(mint_atomicPtr_t *object, void *operand)
{
    return (void*) qw_mint_exchange_64_relaxed((mint_atomic64_t *) object, (uint64_t)operand);
}

#else
#error MINT_PTR_SIZE not set!
#endif

#endif /* INCLUDED_QW_ATOMIC_H */

/* -----------------------------------------------------------------------
Last reviewed: May 5, 2014
Last reviewed by: Ross B.
Status: OK
Comments:
- no ARM or PPC support for atomic exchange
-------------------------------------------------------------------------- */
