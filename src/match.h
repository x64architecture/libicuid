/*
 * Copyright (c) 2016, Kurt Cancemi (kurt@x64architecture.com)
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

typedef struct {
    uint32_t family, model, stepping, ext_family, ext_model;
    uint32_t cores, l2_cache, l3_cache, brand_code;
    const char *uarch;
} match_uarch_t;

int match_pattern(const char *haystack, const char *needle);
void match_cpu_uarch(cpuid_data_t *data, const match_uarch_t *matchtable,
                     const unsigned int array_size, const uint32_t brand_code);
