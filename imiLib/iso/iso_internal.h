
#pragma once
#include "iso_defs.h"
/**********************************************************************/
// Global Defines
/**********************************************************************/

// Max. number of ECALLs (over all domains)
#define NUM_REGISTERED_ECALLS 64

// Size of trusted exception stack slots
#ifdef RELEASE
// For debug output, give more stack
#define EXCEPTION_STACK_WORDS (1024*(PAGESIZE/WORDSIZE)) // 4MB
#else
#define EXCEPTION_STACK_WORDS (16*(PAGESIZE/WORDSIZE))   // 64KB
#endif // RELEASE

#define GET_STACK_TOP(DOMAIN_THREAD_DATA) ((uintptr_t)(DOMAIN_THREAD_DATA)->user_stack_base + ((DOMAIN_THREAD_DATA)->user_stack_size) - 2*WORDSIZE)

/* Struct which is pushed on caller stack upon dcall, and verified upon dreturn */
typedef struct _expected_return {
    int      dom_num;
    void *   reentry;
    void *   previous;
#ifdef ADDITIONAL_DEBUG_CHECKS
    void *   sp;
    uint64_t cookie;
#endif
#ifdef EXPECTED_RETURN_PADDING
    unsigned char padding[EXPECTED_RETURN_PADDING];
#endif
} _expected_return;
/* Struct which is pushed on target stack upon dcall such that it knows where to return to */
typedef struct _return_dom_num {
#ifdef ADDITIONAL_DEBUG_CHECKS
    uint64_t cookie1;
#endif
    int64_t dom_num;
#ifdef ADDITIONAL_DEBUG_CHECKS
    uint64_t cookie2;
#endif
} _return_dom_num;
//------------------------------------------------------------------------------




typedef struct _iso_thread_domain {
	_expected_return * expected_return; // points to the stack where the struct lives in case a dcall is pending (waiting for return), or null.
    uint64_t *         user_stack_base; // base address of user stack
    size_t             user_stack_size; // size in bytes
} _iso_thread_domain;
