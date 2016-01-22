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

#include <icuid/icuid_err.h>

const char *icuid_errorstr(int err)
{
    switch (err) {
        case ICUID_OK:
            return "No error";
            break;
        case ICUID_NO_CPUID:
            return "CPUID instruction is not supported";
            break;
        case ICUID_PASSED_NULL:
            return "Passed NULL to a parameter which can't be NULL";
            break;
        case ICUID_ERROR_OPEN:
            return "Error opening file";
            break;
        case ICUID_ERROR_PARSING:
            return "Error parsing cpuid data from input";
            break;
        default:
            return "Unknown error";
            break;
    }
}
