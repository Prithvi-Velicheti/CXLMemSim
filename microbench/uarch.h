/*
 * Microbench testies for MLP and memory latency in CXLMS
 *
 *  By: Andrew Quinn
 *      Yiwei Yang
 *
 *  Copyright 2023 Regents of the Univeristy of California
 *  UC Santa Cruz Sluglab.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

uint32_t *lfs_random_array;
#define KERNEL_BEGIN                                                                                                   \
    do {                                                                                                               \
    } while (0);
#define KERNEL_END                                                                                                     \
    do {                                                                                                               \
    } while (0);
#define CACHELINE_SIZE 64

#define SIZEBTNT_64_AVX512                                                                                             \
    "vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                           \
    "add $0x40, %%r10 \n"

#define SIZEBTNT_128_AVX512                                                                                            \
    "vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                           \
    "vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                          \
    "add $0x80, %%r10 \n"

#define SIZEBTNT_256_AVX512                                                                                            \
    "vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                           \
    "vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                          \
    "vmovntdq  %%zmm0,  0x80(%%r9, %%r10) \n"                                                                          \
    "vmovntdq  %%zmm0,  0xc0(%%r9, %%r10) \n"                                                                          \
    "add $0x100, %%r10 \n"

#define SIZEBTNT_512_AVX512                                                                                            \
    "vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                           \
    "vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                          \
    "vmovntdq  %%zmm0,  0x80(%%r9, %%r10) \n"                                                                          \
    "vmovntdq  %%zmm0,  0xc0(%%r9, %%r10) \n"                                                                          \
    "vmovntdq  %%zmm0,  0x100(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x140(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x180(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x1c0(%%r9, %%r10) \n"                                                                         \
    "add $0x200, %%r10 \n"

#define SIZEBTNT_1024_AVX512                                                                                           \
    "vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                           \
    "vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                          \
    "vmovntdq  %%zmm0,  0x80(%%r9, %%r10) \n"                                                                          \
    "vmovntdq  %%zmm0,  0xc0(%%r9, %%r10) \n"                                                                          \
    "vmovntdq  %%zmm0,  0x100(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x140(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x180(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x1c0(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x200(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x240(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x280(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x2c0(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x300(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x340(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x380(%%r9, %%r10) \n"                                                                         \
    "vmovntdq  %%zmm0,  0x3c0(%%r9, %%r10) \n"                                                                         \
    "add $0x400, %%r10 \n"

#define SIZEBTSTFLUSH_64_AVX512                                                                                        \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "clwb  0x0(%%r9, %%r10) \n"                                                                                        \
    "add $0x40, %%r10 \n"

#define SIZEBTSTFLUSH_128_AVX512                                                                                       \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "clwb  0x0(%%r9, %%r10) \n"                                                                                        \
    "vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                         \
    "clwb  0x40(%%r9, %%r10) \n"                                                                                       \
    "add $0x80, %%r10 \n"

#define SIZEBTSTFLUSH_256_AVX512                                                                                       \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "clwb  0x0(%%r9, %%r10) \n"                                                                                        \
    "vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                         \
    "clwb  0x40(%%r9, %%r10) \n"                                                                                       \
    "vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n"                                                                         \
    "clwb  0x80(%%r9, %%r10) \n"                                                                                       \
    "vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n"                                                                         \
    "clwb  0xc0(%%r9, %%r10) \n"                                                                                       \
    "add $0x100, %%r10 \n"

#define SIZEBTSTFLUSH_512_AVX512                                                                                       \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "clwb  0x0(%%r9, %%r10) \n"                                                                                        \
    "vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                         \
    "clwb  0x40(%%r9, %%r10) \n"                                                                                       \
    "vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n"                                                                         \
    "clwb  0x80(%%r9, %%r10) \n"                                                                                       \
    "vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n"                                                                         \
    "clwb  0xc0(%%r9, %%r10) \n"                                                                                       \
    "vmovdqa64  %%zmm0,  0x100(%%r9, %%r10) \n"                                                                        \
    "clwb  0x100(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x140(%%r9, %%r10) \n"                                                                        \
    "clwb  0x140(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x180(%%r9, %%r10) \n"                                                                        \
    "clwb  0x180(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x1c0(%%r9, %%r10) \n"                                                                        \
    "clwb  0x1c0(%%r9, %%r10) \n"                                                                                      \
    "add $0x200, %%r10 \n"

#define SIZEBTSTFLUSH_1024_AVX512                                                                                      \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "clwb  0x0(%%r9, %%r10) \n"                                                                                        \
    "vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                         \
    "clwb  0x40(%%r9, %%r10) \n"                                                                                       \
    "vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n"                                                                         \
    "clwb  0x80(%%r9, %%r10) \n"                                                                                       \
    "vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n"                                                                         \
    "clwb  0xc0(%%r9, %%r10) \n"                                                                                       \
    "vmovdqa64  %%zmm0,  0x100(%%r9, %%r10) \n"                                                                        \
    "clwb  0x100(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x140(%%r9, %%r10) \n"                                                                        \
    "clwb  0x140(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x180(%%r9, %%r10) \n"                                                                        \
    "clwb  0x180(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x1c0(%%r9, %%r10) \n"                                                                        \
    "clwb  0x1c0(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x200(%%r9, %%r10) \n"                                                                        \
    "clwb  0x200(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x240(%%r9, %%r10) \n"                                                                        \
    "clwb  0x240(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x280(%%r9, %%r10) \n"                                                                        \
    "clwb  0x280(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x2c0(%%r9, %%r10) \n"                                                                        \
    "clwb  0x2c0(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x300(%%r9, %%r10) \n"                                                                        \
    "clwb  0x300(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x340(%%r9, %%r10) \n"                                                                        \
    "clwb  0x340(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x380(%%r9, %%r10) \n"                                                                        \
    "clwb  0x380(%%r9, %%r10) \n"                                                                                      \
    "vmovdqa64  %%zmm0,  0x3c0(%%r9, %%r10) \n"                                                                        \
    "clwb  0x3c0(%%r9, %%r10) \n"                                                                                      \
    "add $0x400, %%r10 \n"

#define SIZEBTST_64_AVX512                                                                                             \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "add $0x40, %%r10 \n"

#define SIZEBTST_128_AVX512                                                                                            \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                         \
    "add $0x80, %%r10 \n"

#define SIZEBTST_256_AVX512                                                                                            \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                         \
    "vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n"                                                                         \
    "vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n"                                                                         \
    "add $0x100, %%r10 \n"

#define SIZEBTST_512_AVX512                                                                                            \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                         \
    "vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n"                                                                         \
    "vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n"                                                                         \
    "vmovdqa64  %%zmm0,  0x100(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x140(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x180(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x1c0(%%r9, %%r10) \n"                                                                        \
    "add $0x200, %%r10 \n"

#define SIZEBTST_1024_AVX512                                                                                           \
    "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n"                                                                          \
    "vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n"                                                                         \
    "vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n"                                                                         \
    "vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n"                                                                         \
    "vmovdqa64  %%zmm0,  0x100(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x140(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x180(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x1c0(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x200(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x240(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x280(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x2c0(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x300(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x340(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x380(%%r9, %%r10) \n"                                                                        \
    "vmovdqa64  %%zmm0,  0x3c0(%%r9, %%r10) \n"                                                                        \
    "add $0x400, %%r10 \n"

#define SIZEBTLD_64_AVX512                                                                                             \
    "vmovntdqa 0x0(%%r9, %%r10), %%zmm0 \n"                                                                            \
    "add $0x40, %%r10 \n"

#define SIZEBTLD_128_AVX512                                                                                            \
    "vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n"                                                                           \
    "vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n"                                                                          \
    "add $0x80, %%r10 \n"

#define SIZEBTLD_256_AVX512                                                                                            \
    "vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n"                                                                           \
    "vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n"                                                                          \
    "vmovntdqa  0x80(%%r9, %%r10), %%zmm2 \n"                                                                          \
    "vmovntdqa  0xc0(%%r9, %%r10), %%zmm3 \n"                                                                          \
    "add $0x100, %%r10 \n"

#define SIZEBTLD_512_AVX512                                                                                            \
    "vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n"                                                                           \
    "vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n"                                                                          \
    "vmovntdqa  0x80(%%r9, %%r10), %%zmm2 \n"                                                                          \
    "vmovntdqa  0xc0(%%r9, %%r10), %%zmm3 \n"                                                                          \
    "vmovntdqa  0x100(%%r9, %%r10), %%zmm4 \n"                                                                         \
    "vmovntdqa  0x140(%%r9, %%r10), %%zmm5 \n"                                                                         \
    "vmovntdqa  0x180(%%r9, %%r10), %%zmm6 \n"                                                                         \
    "vmovntdqa  0x1c0(%%r9, %%r10), %%zmm7 \n"                                                                         \
    "add $0x200, %%r10 \n"

#define SIZEBTLD_1024_AVX512                                                                                           \
    "vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n"                                                                           \
    "vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n"                                                                          \
    "vmovntdqa  0x80(%%r9, %%r10), %%zmm2 \n"                                                                          \
    "vmovntdqa  0xc0(%%r9, %%r10), %%zmm3 \n"                                                                          \
    "vmovntdqa  0x100(%%r9, %%r10), %%zmm4 \n"                                                                         \
    "vmovntdqa  0x140(%%r9, %%r10), %%zmm5 \n"                                                                         \
    "vmovntdqa  0x180(%%r9, %%r10), %%zmm6 \n"                                                                         \
    "vmovntdqa  0x1c0(%%r9, %%r10), %%zmm7 \n"                                                                         \
    "vmovntdqa  0x200(%%r9, %%r10), %%zmm8 \n"                                                                         \
    "vmovntdqa  0x240(%%r9, %%r10), %%zmm9 \n"                                                                         \
    "vmovntdqa  0x280(%%r9, %%r10), %%zmm10 \n"                                                                        \
    "vmovntdqa  0x2c0(%%r9, %%r10), %%zmm11 \n"                                                                        \
    "vmovntdqa  0x300(%%r9, %%r10), %%zmm12 \n"                                                                        \
    "vmovntdqa  0x340(%%r9, %%r10), %%zmm13 \n"                                                                        \
    "vmovntdqa  0x380(%%r9, %%r10), %%zmm14 \n"                                                                        \
    "vmovntdqa  0x3c0(%%r9, %%r10), %%zmm15 \n"                                                                        \
    "add $0x400, %%r10 \n"

#define SIZEBT_NT_64                                                                                                   \
    "movnti %[random], 0x0(%%r9, %%r10) \n"                                                                            \
    "movnti %[random], 0x8(%%r9, %%r10) \n"                                                                            \
    "movnti %[random], 0x10(%%r9, %%r10) \n"                                                                           \
    "movnti %[random], 0x18(%%r9, %%r10) \n"                                                                           \
    "movnti %[random], 0x20(%%r9, %%r10) \n"                                                                           \
    "movnti %[random], 0x28(%%r9, %%r10) \n"                                                                           \
    "movnti %[random], 0x30(%%r9, %%r10) \n"                                                                           \
    "movnti %[random], 0x38(%%r9, %%r10) \n"                                                                           \
    "add $0x40, %%r10 \n"

#define SIZEBT_LOAD_64                                                                                                 \
    "mov 0x0(%%r9, %%r10),  %%r13  \n"                                                                                 \
    "mov 0x8(%%r9, %%r10),  %%r13  \n"                                                                                 \
    "mov 0x10(%%r9, %%r10), %%r13  \n"                                                                                 \
    "mov 0x18(%%r9, %%r10), %%r13  \n"                                                                                 \
    "mov 0x20(%%r9, %%r10), %%r13  \n"                                                                                 \
    "mov 0x28(%%r9, %%r10), %%r13  \n"                                                                                 \
    "mov 0x30(%%r9, %%r10), %%r13  \n"                                                                                 \
    "mov 0x38(%%r9, %%r10), %%r13  \n"

/* Arbitrary sizes w/o clearing pipeline */

#define SIZEBTNT_MACRO SIZEBTNT_512_AVX512
#define SIZEBTST_MACRO SIZEBTST_512_AVX512
#define SIZEBTLD_MACRO SIZEBT_LOAD_64
#define SIZEBTSTFLUSH_MACRO SIZEBTSTFLUSH_512_AVX512

// #define SIZEBTST_FENCE	"mfence \n"
// #define SIZEBTLD_FENCE	"mfence \n"
#define SIZEBTST_FENCE ""
#define SIZEBTLD_FENCE ""

#define CACHEFENCE_FENCE "sfence \n"
// #define CACHEFENCE_FENCE	"mfence \n"

#define RandLFSR64_NEW(rand, accessmask, addr)				\
  "mov    (%[" #rand "]), %%r9 \n"					\
  "mov    %%r9, %%r12 \n"						\
  "shr    %%r9 \n"							\
  "and    $0x1, %%r12d \n"						\
  "neg    %%r12 \n"							\
  "and    %%rcx, %%r12 \n"						\
  "xor    %%r9, %%r12 \n"						\
  "mov    %%r12, (%[" #rand "]) \n"					\
  "mov    %%r12, %%r8 \n"						\
  "and    %[" #accessmask "], %%r8 \n"					\
  "lea (%[" #addr "], %%r8), %%r9 \n"					

#define RandLFSR64							\
  "mov    (%[random]), %%r9 \n"						\
  "mov    %%r9, %%r12 \n"						\
  "shr    %%r9 \n"							\
  "and    $0x1, %%r12d \n"						\
  "neg    %%r12 \n"							\
  "and    %%rcx, %%r12 \n"						\
  "xor    %%r9, %%r12 \n"						\
  "mov    %%r12, (%[random]) \n"					\
  "mov    %%r12, %%r8 \n"						\
  "and    %[accessmask], %%r8 \n"


void sizebw_load(char *start_addr, long size, long count, long *rand_seed, long access_mask) {
    KERNEL_BEGIN
    asm volatile("movabs $0xd800000000000000, %%rcx \n" /* rcx: bitmask used in LFSR */
                 "xor %%r8, %%r8 \n" /* r8: access offset */
                 "xor %%r11, %%r11 \n" /* r11: access counter */
                 // 1
                 "1: \n" /* outer (counter) loop */
                 RandLFSR64 /* LFSR: uses r9, r12 (reusable), rcx (above), fill r8 */
                 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n" /* r10: accessed size */
                 "2: \n" /* inner (access) loop, unroll 8 times */
                 SIZEBTLD_MACRO /* Access: uses r8[rand_base], r10[size_accessed], r9 */
                 "cmp %[accesssize], %%r10 \n"
                 "jl 2b \n" SIZEBTLD_FENCE

                     // 2
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "3: \n" SIZEBTLD_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 3b \n" SIZEBTLD_FENCE
                     // 3
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "4: \n" SIZEBTLD_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 4b \n" SIZEBTLD_FENCE
                     // 4
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "5: \n" SIZEBTLD_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 5b \n" SIZEBTLD_FENCE

                 "add $4, %%r11 \n"
                 "cmp %[count], %%r11\n"
                 "jl 1b \n"

                 : [random] "=r"(rand_seed)
                 : [start_addr] "r"(start_addr), [accesssize] "r"(size), [count] "r"(count),
                   "0"(rand_seed), [accessmask] "r"(access_mask)
                 : "%rcx", "%r12", "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

void sizebw_load_new(char *start_addr, long count, long *rand_seed, uint64_t access_mask) {
    KERNEL_BEGIN
    asm volatile("movabs $0xd800000000000000, %%rcx \n" /* rcx: bitmask used in LFSR */
                 "xor %%r8, %%r8 \n" /* r8: access offset */
                 "xor %%r11, %%r11 \n" /* r11: access counter */
                 // 1
                 "LD_LOOP_NEW: \n" 
                 RandLFSR64 /* LFSR: uses r9, r12 (reusable), rcx (above), fill r8 */
                 "lea (%[start_addr], %%r8), %%r9 \n"
		 "mov 0x0(%%r9), %%r13 \n"
                 "add $1, %%r11 \n"
                 "cmp %[count], %%r11\n"
                 "jl LD_LOOP_NEW \n"
                 : [random] "=r"(rand_seed)
                 : [start_addr] "r"(start_addr), [count] "r"(count),
		 "0"(rand_seed), [accessmask] "r"(access_mask)
		 : "%rcx", "%r13", "%r12", "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}


#define OPERATION


#define RANDOM_OPER(rand, mask, buf)		\
  RandLFSR64_NEW(rand, mask, buf)		\
    OPERATION							


#define UNROLL_4(rand, mask, buf)			\
  RANDOM_OPER(rand, mask, buf)				\
    RANDOM_OPER(rand, mask, buf)			\
    RANDOM_OPER(rand, mask, buf)			\
    RANDOM_OPER(rand, mask, buf)

#define UNROLL_16(rand, mask, buf)		\
  UNROLL_4(rand, mask, buf)			\
    UNROLL_4(rand, mask, buf)			\
    UNROLL_4(rand, mask, buf)			\
    UNROLL_4(rand, mask, buf)			


#define LOAD_NEW(start_addr, rand_seed, access_mask)			\
  do {									\
    /*r8: rand number, r9: computed addr, r13: dest, r12: temp in lfsr, */ \
    /*rcx: bitmask for lfsr */						\
    asm volatile("movabs $0xd800000000000000, %%rcx \n" /*  bitmask for LFSR */ \
		 "xor %%r8, %%r8 \n" /* r8: access offset */		\
		 UNROLL_16(random, accessmask, buf)			\
		 : [random] "=r"(rand_seed)				\
		 : [buf] "r"(start_addr), "0"(rand_seed), [accessmask] "r"(access_mask) \
		 : "%rcx", "%r13", "%r12", "%r9", "%r8"); \
  } while(0);
    

void sizebw_nt(char *start_addr, long size, long count, long *rand_seed, long access_mask) {
    KERNEL_BEGIN
    asm volatile("movabs $0xd800000000000000, %%rcx \n"
                 "xor %%r11, %%r11 \n"
                 "movq %[start_addr], %%xmm0 \n" /* zmm0: read/write register */
                 // 1
                 "6: \n" RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "7: \n" SIZEBTNT_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 7b \n" SIZEBTST_FENCE

                     // 2
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "8: \n" SIZEBTNT_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 8b \n" SIZEBTST_FENCE
                     // 3
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "9: \n" SIZEBTNT_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 9b \n" SIZEBTST_FENCE
                     // 4
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "10: \n" SIZEBTNT_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 10b \n" SIZEBTST_FENCE

                 "add $4, %%r11 \n"
                 "cmp %[count], %%r11\n"
                 "jl 6b \n"

                 : [random] "=r"(rand_seed)
                 : [start_addr] "r"(start_addr), [accesssize] "r"(size), [count] "r"(count),
                   "0"(rand_seed), [accessmask] "r"(access_mask)
                 : "%rcx", "%r12", "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

void sizebw_store(char *start_addr, long size, long count, long *rand_seed, long access_mask) {
    KERNEL_BEGIN
    asm volatile("movabs $0xd800000000000000, %%rcx \n"
                 "xor %%r11, %%r11 \n"
                 "movq %[start_addr], %%xmm0 \n" /* zmm0: read/write register */
                 // 1
                 "11: \n" RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "12: \n" SIZEBTST_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 12b \n" SIZEBTST_FENCE

                     // 2
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "13: \n" SIZEBTST_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 13b \n" SIZEBTST_FENCE
                     // 3
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "14: \n" SIZEBTST_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 14b \n" SIZEBTST_FENCE
                     // 4
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "15: \n" SIZEBTST_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 15b \n" SIZEBTST_FENCE

                 "add $4, %%r11 \n"
                 "cmp %[count], %%r11\n"
                 "jl 11b \n"

                 : [random] "=r"(rand_seed)
                 : [start_addr] "r"(start_addr), [accesssize] "r"(size), [count] "r"(count),
                   "0"(rand_seed), [accessmask] "r"(access_mask)
                 : "%rcx", "%r12", "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

void sizebw_storeclwb(char *start_addr, long size, long count, long *rand_seed, long access_mask) {
    KERNEL_BEGIN
    asm volatile("movabs $0xd800000000000000, %%rcx \n"
                 "xor %%r11, %%r11 \n"
                 "movq %[start_addr], %%xmm0 \n" /* zmm0: read/write register */
                 // 1
                 "16: \n" RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "17: \n" SIZEBTSTFLUSH_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 17b \n" SIZEBTST_FENCE

                     // 2
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "18: \n" SIZEBTSTFLUSH_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 18b \n" SIZEBTST_FENCE
                     // 3
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "19: \n" SIZEBTSTFLUSH_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 19b \n" SIZEBTST_FENCE
                     // 4
                     RandLFSR64 "lea (%[start_addr], %%r8), %%r9 \n"
                 "xor %%r10, %%r10 \n"
                 "20: \n" SIZEBTSTFLUSH_MACRO "cmp %[accesssize], %%r10 \n"
                 "jl 20b \n" SIZEBTST_FENCE

                 "add $4, %%r11 \n"
                 "cmp %[count], %%r11\n"
                 "jl 16b \n"

                 : [random] "=r"(rand_seed)
                 : [start_addr] "r"(start_addr), [accesssize] "r"(size), [count] "r"(count),
                   "0"(rand_seed), [accessmask] "r"(access_mask)
                 : "%rcx", "%r12", "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

void stride_load(char *start_addr, long size, long skip, long delay, long count) {
    KERNEL_BEGIN
    asm volatile("xor %%r8, %%r8 \n" /* r8: access offset */
                 "xor %%r11, %%r11 \n" /* r11: counter */

                 // 1
                 "21: \n" /* outer (counter) loop */
                 "lea (%[start_addr], %%r8), %%r9 \n" /* r9: access loc */
                 "xor %%r10, %%r10 \n" /* r10: accessed size */
                 "22: \n" /* inner (access) loop, unroll 8 times */
                 SIZEBTLD_64_AVX512 /* Access: uses r10[size_accessed], r9 */
                 "cmp %[accesssize], %%r10 \n"
                 "jl 22b \n" SIZEBTLD_FENCE

                 "xor %%r10, %%r10 \n"
                 "23: \n" /* delay <delay> cycles */
                 "inc %%r10 \n"
                 "cmp %[delay], %%r10 \n"
                 "jl 23b \n"

                 "add %[skip], %%r8 \n"
                 "inc %%r11 \n"
                 "cmp %[count], %%r11 \n"

                 "jl 21b \n"

                 ::[start_addr] "r"(start_addr),
                 [accesssize] "r"(size), [count] "r"(count), [skip] "r"(skip), [delay] "r"(delay)
                 : "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

void stride_nt(char *start_addr, long size, long skip, long delay, long count) {
    KERNEL_BEGIN
    asm volatile("xor %%r8, %%r8 \n" /* r8: access offset */
                 "xor %%r11, %%r11 \n" /* r11: counter */
                 "movq %[start_addr], %%xmm0 \n" /* zmm0: read/write register */
                 // 1
                 "24: \n" /* outer (counter) loop */
                 "lea (%[start_addr], %%r8), %%r9 \n" /* r9: access loc */
                 "xor %%r10, %%r10 \n" /* r10: accessed size */
                 "25: \n" /* inner (access) loop, unroll 8 times */
                 SIZEBTNT_64_AVX512 /* Access: uses r10[size_accessed], r9 */
                 "cmp %[accesssize], %%r10 \n"
                 "jl 25b \n" SIZEBTLD_FENCE

                 "xor %%r10, %%r10 \n"
                 "26: \n" /* delay <delay> cycles */
                 "inc %%r10 \n"
                 "cmp %[delay], %%r10 \n"
                 "jl 26b \n"

                 "add %[skip], %%r8 \n"
                 "inc %%r11 \n"
                 "cmp %[count], %%r11 \n"

                 "jl 24b \n"

                 ::[start_addr] "r"(start_addr),
                 [accesssize] "r"(size), [count] "r"(count), [skip] "r"(skip), [delay] "r"(delay)
                 : "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

void stride_store(char *start_addr, long size, long skip, long delay, long count) {
    KERNEL_BEGIN
    asm volatile("xor %%r8, %%r8 \n" /* r8: access offset */
                 "xor %%r11, %%r11 \n" /* r11: counter */
                 "movq %[start_addr], %%xmm0 \n" /* zmm0: read/write register */
                 // 1
                 "27: \n" /* outer (counter) loop */
                 "lea (%[start_addr], %%r8), %%r9 \n" /* r9: access loc */
                 "xor %%r10, %%r10 \n" /* r10: accessed size */
                 "28: \n" /* inner (access) loop, unroll 8 times */
                 SIZEBTST_64_AVX512 /* Access: uses r10[size_accessed], r9 */
                 "cmp %[accesssize], %%r10 \n"
                 "jl 28b \n" SIZEBTST_FENCE

                 "xor %%r10, %%r10 \n"
                 "29: \n" /* delay <delay> cycles */
                 "inc %%r10 \n"
                 "cmp %[delay], %%r10 \n"
                 "jl 29b \n"

                 "add %[skip], %%r8 \n"
                 "inc %%r11 \n"
                 "cmp %[count], %%r11 \n"

                 "jl 27b \n"

                 ::[start_addr] "r"(start_addr),
                 [accesssize] "r"(size), [count] "r"(count), [skip] "r"(skip), [delay] "r"(delay)
                 : "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

void stride_storeclwb(char *start_addr, long size, long skip, long delay, long count) {
    KERNEL_BEGIN
    asm volatile("xor %%r8, %%r8 \n" /* r8: access offset */
                 "xor %%r11, %%r11 \n" /* r11: counter */
                 "movq %[start_addr], %%xmm0 \n" /* zmm0: read/write register */
                 // 1
                 "30: \n" /* outer (counter) loop */
                 "lea (%[start_addr], %%r8), %%r9 \n" /* r9: access loc */
                 "xor %%r10, %%r10 \n" /* r10: accessed size */
                 "31: \n" /* inner (access) loop, unroll 8 times */
                 SIZEBTSTFLUSH_64_AVX512 /* Access: uses r10[size_accessed], r9 */
                 "cmp %[accesssize], %%r10 \n"
                 "jl 31b \n" SIZEBTST_FENCE

                 "xor %%r10, %%r10 \n"
                 "32: \n" /* delay <delay> cycles */
                 "inc %%r10 \n"
                 "cmp %[delay], %%r10 \n"
                 "jl 32b \n"

                 "add %[skip], %%r8 \n"
                 "inc %%r11 \n"
                 "cmp %[count], %%r11 \n"

                 "jl 30b \n"

                 ::[start_addr] "r"(start_addr),
                 [accesssize] "r"(size), [count] "r"(count), [skip] "r"(skip), [delay] "r"(delay)
                 : "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

#define RDRAND_MAX_RETRY 32

/*
 * Generate random number to [rd] within [range], return 0 if success, 1 if fail.
 */
void stride_read_after_write(char *start_addr, long size, long skip, long delay, long count) {
    KERNEL_BEGIN
    asm volatile(
        "xor    %%r8, %%r8\n"
        "xor    %%r11, %%r11\n"
        "movq   %[start_addr], %%xmm0\n"
        "1:\n"
        "lea    (%[start_addr], %%r8), %%r9\n"
        "xor    %%r10, %%r10\n"
        "2:\n"
        "vmovdqa64  %%zmm0,  0x0(%%r9, %%r10)\n"
        "clwb  0x0(%%r9, %%r10)\n"
        "add $0x40, %%r10\n"
        "cmp    %[accesssize], %%r10\n"
        "jl     2b\n"
        "mfence\n"
        "xor    %%r10, %%r10\n"
        "3:\n"
        "vmovntdq  %%zmm0,  0x0(%%r9, %%r10)\n"
        "add $0x40, %%r10\n"
        "cmp    %[accesssize], %%r10\n"
        "jl     3b\n"
        "mfence\n"
        "xor    %%r10, %%r10\n"
        "4:\n"
        "inc    %%r10\n"
        "cmp    %[delay], %%r10\n"
        "jl     4b\n"
        "add    %[skip], %%r8\n"
        "inc    %%r11\n"
        "cmp    %[count], %%r11\n"
        "jl     1b\n"
        :: [start_addr] "r"(start_addr),
           [accesssize] "r"(size), [count] "r"(count), [skip] "r"(skip), [delay] "r"(delay)
        : "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

static inline int get_rand(uint64_t *rd, uint64_t range) {
    uint8_t ok;
    int i = 0;
    for (i = 0; i < RDRAND_MAX_RETRY; i++) {
        asm volatile("rdrand %0; setc %1\n\t" : "=r"(*rd), "=qm"(ok));

        if (ok) {
            *rd = *rd % range;
            return 0;
        }
    }

    return 1;
}

int init_chasing_index(uint64_t *cindex, uint64_t csize) {
    uint64_t curr_pos = 0;
    uint64_t next_pos = 0;
    uint64_t i = 0;
    int ret = 0;

    memset(cindex, 0, sizeof(uint64_t) * csize);

    for (i = 0; i < csize - 1; i++) {
        do {
            ret = get_rand(&next_pos, csize);
            if (ret != 0)
                return 1;
        } while ((cindex[next_pos] != 0) || (next_pos == curr_pos));

        cindex[curr_pos] = next_pos;
        curr_pos = next_pos;
    }

    return 0;
}

void chasing_storeclwb(char *start_addr, long size, long skip, long count, uint64_t *cindex) {
    KERNEL_BEGIN
    asm volatile("xor	%%r8, %%r8 \n" /* r8: access offset */
                 "xor	%%r11, %%r11 \n" /* r11: counter */
                 "33: \n" /* outer (counter) loop */
                 "lea	(%[start_addr], %%r8), %%r9 \n" /* r9: access loc */
                 "xor	%%r10, %%r10 \n" /* r10: accessed size */
                 "xor	%%r12, %%r12 \n" /* r12: chasing index addr */
                 "34: \n"
                 "movq	(%[cindex], %%r12, 8), %%xmm0\n"
                 "shl    $0x06, %%r12\n"
                 "vmovdqa64	%%zmm0,  0x0(%%r9, %%r12) \n"
                 "clwb	0x0(%%r9, %%r12) \n"
                 "add	$0x40, %%r10\n"
                 "movq	%%xmm0, %%r12\n" /* Update to next chasing element */

                 "cmp	%[accesssize], %%r10 \n"
                 "jl		34b \n" SIZEBTST_FENCE

                 "xor	%%r10, %%r10 \n"

                 "add	%[skip], %%r8 \n"
                 "inc	%%r11 \n"
                 "cmp	%[count], %%r11 \n"

                 "jl		33b \n"

                 :
                 : [start_addr] "r"(start_addr), [accesssize] "r"(size), [count] "r"(count), [skip] "r"(skip),
                   [cindex] "r"(cindex)
                 : "%r12", "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

void chasing_loadnt(char *start_addr, long size, long skip, long count, uint64_t *cindex) {
    KERNEL_BEGIN
    asm volatile("xor    %%r8, %%r8 \n" /* r8: access offset */
                 "xor    %%r11, %%r11 \n" /* r11: counter */
                 "35: \n" /* outer (counter) loop */
                 "lea    (%[start_addr], %%r8), %%r9 \n" /* r9: access loc */
                 "xor    %%r10, %%r10 \n" /* r10: accessed size */
                 "xor	%%r12, %%r12 \n" /* r12: chasing index addr */
                 "36: \n"
                 "shl    $0x06, %%r12\n"
                 "vmovntdqa 0x0(%%r9, %%r12), %%zmm0\n"
                 "movq   %%xmm0, %%r12\n" /* Update to next chasing element */
                 "add    $0x40, %%r10 \n"

                 "cmp    %[accesssize], %%r10 \n"
                 "jl     36b \n" SIZEBTLD_FENCE

                 //"mfence \n"  /* !!!! */
                 "add    %[skip], %%r8 \n"
                 "inc    %%r11 \n"
                 "cmp    %[count], %%r11 \n"

                 "jl     35b \n"

                 :
                 : [start_addr] "r"(start_addr), [accesssize] "r"(size), [count] "r"(count), [skip] "r"(skip),
                   [cindex] "r"(cindex)
                 : "%r11", "%r10", "%r9", "%r8");
    KERNEL_END
}

void cachefence(char *start_addr, long size, long cache, long fence) {
    KERNEL_BEGIN
    asm volatile("movq %[start_addr], %%xmm0 \n"
                 "xor %%r9, %%r9 \n" /* r9: offset of write */
                 "CACHEFENCE_FENCEBEGIN: \n"
                 "xor %%r11, %%r11 \n" /* r11: fence counter */
                 "CACHEFENCE_FLUSHBEGIN: \n"
                 "xor %%r10, %%r10 \n" /* r10: clwb counter */
                 //		"movq %%r9, %%rdx \n"				/* rdx: flush start offset */
                 "leaq (%[start_addr], %%r9), %%rdx \n"
                 "CACHEFENCE_WRITEONE: \n"
                 "vmovdqa64  %%zmm0, 0x0(%[start_addr], %%r9) \n" /* Write one addr */
                 "add $0x40, %%r9 \n"
                 "add $0x40, %%r10 \n"
                 "add $0x40, %%r11 \n"
                 "cmp %[cache], %%r10 \n" /* check clwb */
                 "jl CACHEFENCE_WRITEONE \n"

                 "leaq (%[start_addr], %%r9), %%rcx \n" /* rcx: flush end offset, rdx->rcx */
                 //		"add %[start_addr], %%rcx"
                 "CACHEFENCE_FLUSHONE: \n"
                 "clwb (%%rdx) \n" /* Flush from rdx to rcx */
                 "add $0x40, %%rdx \n"
                 "cmp %%rcx, %%rdx \n"
                 "jl CACHEFENCE_FLUSHONE \n"

                 "cmp %[fence], %%r11 \n"
                 "jl CACHEFENCE_FLUSHBEGIN \n" CACHEFENCE_FENCE

                 "cmp %[accesssize], %%r9 \n"
                 "jl CACHEFENCE_FENCEBEGIN \n"

                 ::[start_addr] "r"(start_addr),
                 [accesssize] "r"(size), [cache] "r"(cache), [fence] "r"(fence)
                 : "%rdx", "%rcx", "%r11", "%r10", "%r9");
    KERNEL_END
    return;
}

void cacheprobe(char *start_addr, char *end_addr, long stride) {
    KERNEL_BEGIN
    asm volatile("mov %[start_addr], %%r8 \n"
                 "movq %[start_addr], %%xmm0 \n"
                 "37: \n"
                 "vmovdqa64 %%zmm0, 0x0(%%r8) \n"
                 "clflush (%%r8) \n"
                 "vmovdqa64 %%zmm0, 0x40(%%r8) \n"
                 "clflush 0x40(%%r8) \n"
                 "add %[stride], %%r8 \n"
                 "cmp %[end_addr], %%r8 \n"
                 "jl 37b \n"
                 "mfence \n"

                 ::[start_addr] "r"(start_addr),
                 [end_addr] "r"(end_addr), [stride] "r"(stride)
                 : "%r8");
    KERNEL_END
    return;
}

void imcprobe(char *start_addr, char *end_addr, long loop) {
    KERNEL_BEGIN
    asm volatile("xor %%r9, %%r9 \n"
                 "movq %[start_addr], %%xmm0 \n"

                 "LOOP1_IMCPROBE: \n"
                 "mov %[start_addr], %%r8 \n"
                 "LOOP2_IMCPROBE: \n"
                 "vmovntdq %%zmm0, 0x0(%%r8) \n"
                 "add $0x40, %%r8 \n"
                 "cmp %[end_addr], %%r8 \n"
                 "jl LOOP2_IMCPROBE \n"

                 "add $1, %%r9 \n"
                 "cmp %[loop], %%r9 \n"
                 "jl LOOP1_IMCPROBE \n"

                 ::[start_addr] "r"(start_addr),
                 [end_addr] "r"(end_addr), [loop] "r"(loop)
                 : "%r8", "%r9");
    KERNEL_END
    return;
}

void seq_load(char *start_addr, char *end_addr, long size) {
    KERNEL_BEGIN
    asm volatile("mov %[start_addr], %%r9 \n"

                 "38: \n"
                 "xor %%r8, %%r8 \n"
                 "39: \n"
                 "vmovntdqa 0x0(%%r9, %%r8), %%zmm0 \n"
                 "add $0x40, %%r8 \n"
                 "cmp %[size], %%r8 \n"
                 "jl 39b \n"

                 "add %[size], %%r9 \n"
                 "cmp %[end_addr], %%r9 \n"
                 "jl 38b \n"

                 ::[start_addr] "r"(start_addr),
                 [end_addr] "r"(end_addr), [size] "r"(size)
                 : "%r8", "%r9");
    KERNEL_END
    return;
}
void seq_store(char *start_addr, char *end_addr, long size) {
    KERNEL_BEGIN
    asm volatile("mov %[start_addr], %%r9 \n"
                 "movq %[start_addr], %%xmm0 \n"

                 "40: \n"
                 "xor %%r8, %%r8 \n"
                 "41: \n"
                 "vmovdqa64  %%zmm0,  0x0(%%r9, %%r8) \n"
                 "clwb  (%%r9, %%r8) \n"
                 "add $0x40, %%r8 \n"
                 "cmp %[size], %%r8 \n"
                 "jl 41b \n"

                 "add %[size], %%r9 \n"
                 "cmp %[end_addr], %%r9 \n"
                 "jl 40b \n"

                 ::[start_addr] "r"(start_addr),
                 [end_addr] "r"(end_addr), [size] "r"(size)
                 : "%r8", "%r9");
    KERNEL_END
    return;
}

void seq_clwb(char *start_addr, char *end_addr, long size) {
    KERNEL_BEGIN
    asm volatile("mov %[start_addr], %%r9 \n"
                 "movq %[start_addr], %%xmm0 \n"

                 "42: \n"
                 "xor %%r8, %%r8 \n"
                 "43: \n"
                 "vmovdqa64  %%zmm0,  0x0(%%r9, %%r8) \n"
                 "clwb  (%%r9, %%r8) \n"
                 "add $0x40, %%r8 \n"
                 "cmp %[size], %%r8 \n"
                 "jl 43b \n"

                 "add %[size], %%r9 \n"
                 "cmp %[end_addr], %%r9 \n"
                 "jl 42b \n"

                 ::[start_addr] "r"(start_addr),
                 [end_addr] "r"(end_addr), [size] "r"(size)
                 : "%r8", "%r9");
    KERNEL_END
}

void seq_nt(char *start_addr, char *end_addr, long size) {
    KERNEL_BEGIN
    asm volatile("mov %[start_addr], %%r9 \n"
                 "movq %[start_addr], %%xmm0 \n"

                 "44: \n"
                 "xor %%r8, %%r8 \n"
                 "45: \n"
                 "vmovntdq %%zmm0, 0x0(%%r9, %%r8) \n"
                 "add $0x40, %%r8 \n"
                 "cmp %[size], %%r8 \n"
                 "jl 45b \n"

                 "add %[size], %%r9 \n"
                 "cmp %[end_addr], %%r9 \n"
                 "jl 44b \n"

                 ::[start_addr] "r"(start_addr),
                 [end_addr] "r"(end_addr), [size] "r"(size)
                 : "%r8", "%r9");
    KERNEL_END
}

struct timespec tstart, tend;
unsigned int c_store_start_hi, c_store_start_lo;
unsigned int c_ntload_start_hi, c_ntload_start_lo;
unsigned int c_ntload_end_hi, c_ntload_end_lo;
unsigned long c_store_start;
unsigned long c_ntload_start, c_ntload_end;
long pages, diff;

		 
#define BEFORE(buf, size, name)						\
  asm volatile("xor %%r8, %%r8 \n" /* r8: counter */			\
	       "FLUSH_LOOP" #name ": \n"				\
	       "lea (%[buf], %%r8), %%r9 \n"				\
  	       "clflush (%%r9) \n"					\
	       "add $1, %%r8 \n"					\
	       "cmp %[size], %%r8 \n"					\
	       "jl FLUSH_LOOP" #name " \n"				\
	       "mfence \n"						\
	       :: [buf] "r" (buf), [size] "r"(size)			\
	       : "%r8", "%r9");						\
  clock_gettime(CLOCK_MONOTONIC_RAW, &tstart);				\
  asm volatile("mfence \n\t"						\
	       "rdtscp \n\t"						\
	       "mfence \n\t"						\
	       "mov %%edx, %[hi]\n\t"					\
	       "mov %%eax, %[lo]\n\t"					\
	       : [hi] "=r"(c_store_start_hi), [lo] "=r"(c_store_start_lo) \
	       :							\
	       : "rdx", "rax", "rcx");


#define AFTER								\
  asm volatile("mfence \n\t"						\
	       "rdtscp \n\t"						\
	       "mfence \n\t"						\
	       "mov %%edx, %[hi]\n\t"					\
	       "mov %%eax, %[lo]\n\t"					\
	       : [hi] "=r"(c_ntload_end_hi), [lo] "=r"(c_ntload_end_lo)	\
	       :							\
	       : "rdx", "rax", "rcx");					\
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &tend) == 0) {			\
    diff = (tend.tv_sec - tstart.tv_sec) * 1e9 + tend.tv_nsec - tstart.tv_nsec; \
  }									\
  c_store_start = (((unsigned long)c_store_start_hi) << 32) | c_store_start_lo; \
  c_ntload_start = (((unsigned long)c_ntload_start_hi) << 32) | c_ntload_start_lo; \
  c_ntload_end = (((unsigned long)c_ntload_end_hi) << 32) | c_ntload_end_lo;



#define LFS_PERMRAND_ENTRIES 0x1000
#define RAW_BEFORE_WRITE                                                                                               \
    clock_gettime(CLOCK_MONOTONIC_RAW, &tstart);                                                                       \
    asm volatile("rdtscp \n\t"                                                                                         \
                 "lfence \n\t"                                                                                         \
                 "mov %%edx, %[hi]\n\t"                                                                                \
                 "mov %%eax, %[lo]\n\t"                                                                                \
                 : [hi] "=r"(c_store_start_hi), [lo] "=r"(c_store_start_lo)                                            \
                 :                                                                                                     \
                 : "rdx", "rax", "rcx");
#define RAW_BEFORE_READ                                                                                                \
    asm volatile("rdtscp \n\t"                                                                                         \
                 "lfence \n\t"                                                                                         \
                 "mov %%edx, %[hi]\n\t"                                                                                \
                 "mov %%eax, %[lo]\n\t"                                                                                \
                 : [hi] "=r"(c_ntload_start_hi), [lo] "=r"(c_ntload_start_lo)                                          \
                 :                                                                                                     \
                 : "rdx", "rax", "rcx");
#define RAW_FINAL(job_name)						\
  asm volatile("lfence \n\t"						\
	       "rdtscp \n\t"						\
	       "lfence \n\t"						\
	       "mov %%edx, %[hi]\n\t"					\
	       "mov %%eax, %[lo]\n\t"					\
	       : [hi] "=r"(c_ntload_end_hi), [lo] "=r"(c_ntload_end_lo)	\
	       :							\
	       : "rdx", "rax", "rcx");					\
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &tend) == 0) {			\
    diff = (tend.tv_sec - tstart.tv_sec) * 1e9 + tend.tv_nsec - tstart.tv_nsec; \
  }									\
  c_store_start = (((unsigned long)c_store_start_hi) << 32) | c_store_start_lo; \
  c_ntload_start = (((unsigned long)c_ntload_start_hi) << 32) | c_ntload_start_lo; \
  c_ntload_end = (((unsigned long)c_ntload_end_hi) << 32) | c_ntload_end_lo;
