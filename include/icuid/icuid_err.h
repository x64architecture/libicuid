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

#ifndef __LIBICUID_ERR_H__
#define __LIBICUID_ERR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ICUID_OK              0  /*!< No error */
#define ICUID_NO_CPUID        1  /*!< The cpuid instruction is not supported */
#define ICUID_PASSED_NULL     2  /*!< Passed a NULL parameter when you shouldn't have */
#define ICUID_ERROR_OPEN      3  /*!< Error opening file */
#define ICUID_ERROR_PARSING   4  /*!< Error parsing cpuid data from input */

const char *icuid_errorstr(int err);

#ifdef __cplusplus
}
#endif

#endif /* __LIBICUID_ERR_H__ */
