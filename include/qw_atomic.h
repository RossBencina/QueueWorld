#ifndef INCLUDED_QW_ATOMIC_H
#define INCLUDED_QW_ATOMIC_H

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

#define NOMINMAX // suppress windows.h min/max
#include <intrin.h>

MINT_C_INLINE uint32_t qw_mint_exchange_32_relaxed(mint_atomic32_t *object, uint32_t operand)
{
    return _InterlockedExchange((volatile long*)object, operand);
}

MINT_C_INLINE uint64_t qw_mint_exchange_64_relaxed(mint_atomic64_t *object, uint64_t operand)
{
    return InterlockedExchange64((volatile LONG64*)object, operand);
}

#elif MINT_COMPILER_GCC && (MINT_CPU_X86 || MINT_CPU_X64)

// FIXME TODO
// see http://stackoverflow.com/questions/8268243/porting-interlockedexchange-using-gcc-intrinsics-only
// apparently for gcc we should use the __sync_lock_test_and_set  (which is supposedly just an atomic exchange)
// http://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html#_005f_005fsync-Builtins

#else

#error /* no support */

#endif


//--------------------------------------------------------------
//  Pointer-sized atomic RMW operation wrappers
//--------------------------------------------------------------
#if MINT_PTR_SIZE == 4

MINT_C_INLINE void* qw_mint_exchange_ptr_relaxed(mint_atomicPtr_t *object, void *operand)
{
    return (void*) qw_mint_exchange_32_relaxed((mint_atomic32_t *) object, (uint32_t)operand);
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
Last reviewed: April 22, 2014
Last reviewed by: Ross B.
Status: OK
Comments:
- no gcc or OS X support
-------------------------------------------------------------------------- */
