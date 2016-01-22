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

/* Get number of elements in an array (STACK ONLY!) */
#define NELEMS(x) (sizeof(x) / sizeof(x[0]))

typedef struct {
    uint32_t family, model, stepping, ext_family, ext_model;
    uint32_t cores, l2_cache, l3_cache, brand_code;
    char uarch[32];
} match_uarch_t;

void set_common_features(cpuid_raw_data_t *raw, cpuid_data_t *data);
void set_common_xfeatures(uint32_t eax, cpuid_data_t *data);
int match_pattern(const char *haystack, const char *needle);
void match_cpu_uarch(const match_uarch_t *matchtable, int count,
                     cpuid_data_t *data, uint32_t brand_code);
