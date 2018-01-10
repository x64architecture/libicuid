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

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <icuid/icuid.h>

#include "match.h"
#include "intel.h"

static unsigned int score(const cpuid_data_t *data, const match_codename_t *entry)
{
    unsigned int rv = 0;

    if (entry->family != NA && entry->family == data->family)
        rv += 2;
    if (entry->ext_family != NA && entry->ext_family == data->ext_family)
        rv += 2;
    if (entry->model != NA && entry->model == data->model)
        rv += 2;
    if (entry->ext_model != NA && entry->ext_model == data->ext_model)
        rv += 2;
    if (entry->stepping != NA && entry->stepping == data->stepping)
        rv += 2;

    return rv;
}

void cpu_to_codename(cpuid_data_t *data, const match_codename_t *matchtable,
                     const unsigned int array_size)
{
    unsigned int bestscore = 0;
    unsigned int bestindex = 0;
    unsigned int i, t;

    for (i = 0; i < array_size; i++) {
        t = score(data, &matchtable[i]);
        if (t > bestscore) {
            bestscore = t;
            bestindex = i;
        }
    }

    data->codename = matchtable[bestindex].codename;
}
