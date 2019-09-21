/*
 * Copyright (c) 2015 - 2019, Kurt Cancemi (kurt@x64architecture.com)
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

#ifndef __LIBICUID_TYPES_H__
#define __LIBICUID_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_MSC_VER) || _MSC_VER >= 1700
 #include <stdint.h>
#else
 typedef signed __int8     int8_t;
 typedef signed __int16    int16_t;
 typedef signed __int32    int32_t;
 typedef unsigned __int8   uint8_t;
 typedef unsigned __int16  uint16_t;
 typedef unsigned __int32  uint32_t;
 typedef signed __int64    int64_t;
 typedef unsigned __int64  uint64_t;
#endif /* _MSC_VER */

#ifdef __cplusplus
}
#endif

#endif /* __LIBICUID_TYPES_H__ */
