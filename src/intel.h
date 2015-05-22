/*
 * Copyright (c) 2015, Kurt Cancemi (kurt@x64architecture.com)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Extended Topology */
#define INVALID 0x0
#define THREAD  0x1
#define CORE    0x2

/* Model Codes */
#define CPU_MODEL_HASWELL       0x306C
#define CPU_MODEL_HASWELL_E     0x306F
#define CPU_MODEL_HASWELL_ULT   0x4065
#define CPU_MODEL_IVYBRIDGE     0x306A
#define CPU_MODEL_IVYBRIDGE_EP  0x306E
#define CPU_MODEL_SANDYBRIDGE   0x206A
#define CPU_MODEL_SANDYBRIDGE_E 0x206D
#define CPU_MODEL_WESTMERE      0x2065
#define CPU_MODEL_WESTMERE_EP   0x206C
#define CPU_MODEL_WESTMERE_EX   0x206F
#define CPU_MODEL_NEHALEM       0x106E
#define CPU_MODEL_NEHALEM_EP    0x106A
#define CPU_MODEL_NEHALEM_EX    0x206E
#define CPU_MODEL_PENRYN        0x1067
#define CPU_MODEL_PENRYN_E      0x106D
#define CPU_MODEL_MEROM         0x006F


void read_intel_data(cpuid_raw_data_t *raw, cpuid_data_t *data);
