// once this works, move it to:
//    <libpi/include/vector-base.h>
// and make sure it still works.
#ifndef __VECTOR_BASE_SET_H__
#define __VECTOR_BASE_SET_H__
#include "asm-helpers.h"
#include "libc/bit-support.h"

/*
 * vector base address register:
 *   arm1176.pdf:3-121 --- lets us control where the
 *   exception jump table is!  makes it easy to switch
 *   tables and also make exceptions faster.
 *
 * defines:
 *  - vector_base_set
 *  - vector_base_get
 */

cp_asm_get(arm_vector_base, p15, 0, c12, c0, 0);
cp_asm_set(arm_vector_base, p15, 0, c12, c0, 0);

// return the current value vector base is set to.
static inline void *vector_base_get(void) {
  // Use inline assembly to get the vector base register
  uint32_t vec_base = arm_vector_base_get();
  return (void *)vec_base;
}

// check that not null and alignment is good.
static inline int vector_base_chk(void *vector_base) {
  if (!vector_base) {
    return 0;
  }

  // Check if the vector base is 32-bit aligned
  if (((unsigned) vector_base & 0b11111) != 0) {
    return 0;
  }

  return 1;
}

// set vector base: must not have been set already.
static inline void vector_base_set(void *vec) {
  if (!vector_base_chk(vec))
    panic("illegal vector base %p\n", vec);

  void *v = vector_base_get();
  // if already set to the same vector, just return.
  if (v == vec)
    return;

  if (v)
    panic("vector base register already set=%p\n", v);

  // Set the vector base register
  uint32_t vec_base_32bit = (uint32_t)vec;
  arm_vector_base_set(vec_base_32bit);

  // double check that what we set is what we have.
  v = vector_base_get();
  if (v != vec)
    panic("set vector=%p, but have %p\n", vec, v);
}

// set vector base to <vec> and return old value: could have
// been previously set (i.e., non-null).
static inline void *vector_base_reset(void *vec) {
  void *old_vec = 0;

  if (!vector_base_chk(vec))
    panic("illegal vector base %p\n", vec);

  // Get the old vector base and set the new one
  old_vec = vector_base_get();
  uint32_t vec_base_32bit = (uint32_t)vec;
  arm_vector_base_set(vec_base_32bit);

  // double check that what we set is what we have.
  assert(vector_base_get() == vec);
  return old_vec;
}
#endif
