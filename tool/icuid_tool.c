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

#include <stdio.h>
#include <string.h>

#include <icuid/icuid.h>
#include <icuid/icuid_ver.h>

#include "opt.h"

static FILE *out;

static struct {
    char *out;
    char *dump;
    char *data;
    int help;
} icuid_opts;

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)
#define HAVE_DESIGNATED_INITIALIZERS
#endif
#ifdef HAVE_DESIGNATED_INITIALIZERS
#define DINIT(x, ...) x = __VA_ARGS__
#else
#define DINIT(x, ...) __VA_ARGS__
#endif

static struct OPTION icuid_options[] = {
    {
      DINIT(.name,      "help"),
      DINIT(.argname,   NULL),
      DINIT(.desc,      "Print this help message"),
      DINIT(.type,      OPTION_FLAG),
      DINIT(.arg,       NULL),
      DINIT(.flag,      &icuid_opts.help),
    },
    {
      DINIT(.name,      "output"),
      DINIT(.argname,   "<file>"),
      DINIT(.desc,      "Redirect output to file"),
      DINIT(.type,      OPTION_ARG),
      DINIT(.arg,       &icuid_opts.out),
      DINIT(.flag,      NULL),
    },
    {
      DINIT(.name,      "dump"),
      DINIT(.argname,   "<file>"),
      DINIT(.desc,      "Dump raw cpuid data to file/stdin"),
      DINIT(.type,      OPTION_ARG_NR),
      DINIT(.arg,       &icuid_opts.dump),
      DINIT(.flag,      NULL),
    },
    {
      DINIT(.name,      "data"),
      DINIT(.argname,   "<file>"),
      DINIT(.desc,      "Read raw cpuid data from file/stdout"),
      DINIT(.type,      OPTION_ARG_NR),
      DINIT(.arg,       &icuid_opts.data),
      DINIT(.flag,      NULL),
    },
    {
      DINIT(.name,      NULL),
      DINIT(.argname,   NULL),
      DINIT(.desc,      NULL),
      DINIT(.type,      0),
      DINIT(.arg,       NULL),
      DINIT(.flag,      NULL),
    },
};

static int usage(void)
{
    fprintf(stderr, "usage: icuid_tool [options]\n");
    options_usage(icuid_options);

    return 0;
}

static int print_summary(cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    int i;

    icuid_identify(raw, data);

    fprintf(out, "ICUID - " LIBICUID_VERSION "\n");
    fprintf(out, "Copyright (c) " LIBICUID_COPYRIGHT "\n");
    fprintf(out, "===============================\n");
    /* fprintf(out, "Vendor Info:\n"); */
    fprintf(out, " Vendor      : %s\n", data->vendor_str);
    fprintf(out, " Vendor ID   : %u\n", data->vendor);
    fprintf(out, " CPU         : %s\n", data->brand_str);

    /* fprintf(out, "Cores Info:\n"); */
    fprintf(out, " Cores       : %u\n", data->cores);
    fprintf(out, " Logical     : %u\n", data->logical_cpus);

    /* fprintf(out, "Proc Info:\n"); */
    fprintf(out, " Codename    : %s\n", data->codename);
    fprintf(out, " Family      : %u\n", data->family);
    fprintf(out, " Model       : %u\n", data->model);
    fprintf(out, " Stepping    : %u\n", data->stepping);
    fprintf(out, " Type        : %u\n", data->type);
    fprintf(out, " Ext Family  : %u\n", data->ext_family);
    fprintf(out, " Ext Model   : %u\n", data->ext_model);
    fprintf(out, " Signature   : 0x%0x\n", data->signature);

    /* fprintf(out, "Cache Info:\n"); */
    fprintf(out, " L1 D Cache  : %ukB\n", data->l1_data_cache);
    fprintf(out, " L1 I Cache  : %ukB\n", data->l1_instruction_cache);
    fprintf(out, " L2 Cache    : %ukB\n", data->l2_cache);
    fprintf(out, " L3 Cache    : %ukB\n", data->l3_cache);
    fprintf(out, " L1 Assoc.   : %u-way\n", data->l1_associativity);
    fprintf(out, " L2 Assoc.   : %u-way\n", data->l2_associativity);
    fprintf(out, " L3 Assoc.   : %u-way\n", data->l3_associativity);
    fprintf(out, " L1 Line sz  : %u bytes\n", data->l1_cacheline);
    fprintf(out, " L2 Line sz  : %u bytes\n", data->l2_cacheline);
    fprintf(out, " L3 Line sz  : %u bytes\n", data->l3_cacheline);

    fprintf(out, " Address szs : %u bits physical, %u bits virtual\n",
            data->physical_address_bits, data->virtual_address_bits);

    fprintf(out, " SSE State   : %s\n", (data->xfeatures[XFEATURE_SSE] == 1 ?
                                                "Enabled" : "Disabled"));
    fprintf(out, " AVX State   : %s\n", (data->xfeatures[XFEATURE_AVX] == 1 ?
                                                "Enabled" : "Disabled"));

    fprintf(out, " Features    :");
    for (i = 0; i < NUM_CPU_FEATURES; i++) {
        if (data->flags[i])
            fprintf(out, " %s", cpu_feature_str(i));
    }
    fprintf(out, "\n");

    return 0;
}

int main(int argc, char **argv)
{
    int ret = -1;
    cpuid_raw_data_t raw;
    cpuid_data_t data;

    memset(&icuid_opts, 0, sizeof(icuid_opts));

    if (options_parse(argc, argv, icuid_options, NULL) != 0) {
        usage();
        return -1;
    }

    if (icuid_opts.help) {
        usage();
        return 0;
    }

    out = stdout;
    if (icuid_opts.out != NULL) {
        if ((out = fopen(icuid_opts.out, "w")) == NULL) {
            fprintf(stderr, "Can't open file %s\n", icuid_opts.out);
            return -1;
        }
    }

    if (icuid_opts.dump != NULL) {
        ret = cpuid_deserialize_raw_data(&raw, icuid_opts.dump);
        if (ret != ICUID_OK) {
            fprintf(out, "%s\n", icuid_errorstr(ret));
            return -1;
        }
        return 0;
    }

    if (icuid_opts.data != NULL) {
        ret = cpuid_serialize_raw_data(&raw, icuid_opts.data);
        if (ret != ICUID_OK) {
            fprintf(out, "%s\n", icuid_errorstr(ret));
            return -1;
        }
    } else {
        ret = cpuid_get_raw_data(&raw);
        if (ret != ICUID_OK) {
            fprintf(out, "%s\n", icuid_errorstr(ret));
            return -1;
        }
    }

    ret = print_summary(&raw, &data);

    if (icuid_opts.out != NULL)
        fclose(out);

    return ret;
}
