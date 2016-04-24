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

typedef struct {
    uint8_t bit;
    cpuid_feature_t feature;
} cpuid_feature_map_t;

void set_feature_bits(cpuid_data_t *data, const cpuid_feature_map_t *feature,
                      const unsigned int array_size, const uint32_t reg);
void set_common_features(const cpuid_raw_data_t *raw, cpuid_data_t *data);
void set_common_xfeatures(cpuid_data_t *data, const uint64_t xcr0);
