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

#include <ctype.h>
#include <string.h>

#include <icuid/icuid.h>

#include "match.h"

static size_t regex_match(const char c, const char *needle)
{
    size_t j;

    if (c == '\0')
        return 0;
    if (c == needle[0])
        return 1;
    if (needle[0] == '.')
        return 1;
    if (needle[0] == '#' && isdigit(c))
        return 1;
    /* needle can be multiple values */
    if (needle[0] == '[') {
        j = 1;
        while (needle[j] != '\0' && needle[j] != ']')
            j++;
        if (needle[j] != '\0')
            return 0;
        while (*needle++ != '\0')
            if (needle[0] == c)
                return j + 1;
    }
    return 0;
}

int match_pattern(const char *haystack, const char *needle)
{
    size_t j, dj, m = strlen(needle);

    while (*haystack++ != '\0') {
        j = 0;
        while (j < m && (dj = regex_match(haystack[0], needle + j) != 0)) {
            haystack++;
            j += dj;
        }
        if (j == m)
            return 1;
    }
    return 0;
}

static unsigned int score(const cpuid_data_t *data, const match_uarch_t *entry,
                          const uint32_t brand_code)
{
    unsigned int rv = 0;

    if (entry->family     == data->family)
        rv += 2;
    if (entry->model      == data->model)
        rv += 2;
    if (entry->stepping   == data->stepping)
        rv += 2;
    if (entry->ext_family == data->ext_family)
        rv += 2;
    if (entry->ext_model  == data->ext_model)
        rv += 2;
    if (entry->cores      == data->cores)
        rv += 2;
    if (entry->l2_cache   == data->l2_cache)
        rv += 1;
    if (entry->l3_cache   == data->l3_cache)
        rv += 1;
    if (entry->brand_code == brand_code)
        rv += 2;

    return rv;
}

void match_cpu_uarch(cpuid_data_t *data, const match_uarch_t *matchtable,
                     const unsigned int array_size, const uint32_t brand_code)
{
    unsigned int bestscore = 0;
    unsigned int bestindex = 0;
    unsigned int i, t;

    for (i = 0; i < array_size; i++) {
        t = score(data, &matchtable[i], brand_code);
        if (t > bestscore) {
            bestscore = t;
            bestindex = i;
        }
    }
    data->codename = matchtable[bestindex].uarch;
}
