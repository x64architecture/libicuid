/*
 * Copyright (c) 2015 - 2016, Kurt Cancemi (kurt@x64architecture.com)
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

typedef enum {
    NA = -1,
    NO_CODE,

    PENTIUM = 10,
    MOBILE_PENTIUM,

    XEON = 20,
    XEON_IRWIN,
    XEONMP,
    XEON_POTOMAC,
    XEON_I7,
    XEON_GAINESTOWN,
    XEON_WESTMERE,

    MOBILE_PENTIUM_M = 30,
    CELERON,
    MOBILE_CELERON,
    NOT_CELERON,

    CORE_SOLO = 40,
    MOBILE_CORE_SOLO,
    CORE_DUO,
    MOBILE_CORE_DUO,

    WOLFDALE = 50,
    MEROM,
    PENRYN,
    QUAD_CORE,
    DUAL_CORE_HT,
    QUAD_CORE_HT,
    PENTIUM_D,

    ATOM = 60,
    ATOM_SILVERTHORNE,
    ATOM_DIAMONDVILLE,
    ATOM_PINEVIEW,
    ATOM_CEDARVIEW,

    CORE_I3 = 70,
    CORE_I5,
    CORE_I7,
    DEVILSCANYON,
} intel_uarch_t;

void read_intel_data(const cpuid_raw_data_t *raw, cpuid_data_t *data);
