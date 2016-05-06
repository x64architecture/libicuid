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

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <icuid/icuid.h>

#define _eprintf(format, ...) fprintf(stdout, format, __VA_ARGS__)

int generate_test(cpuid_raw_data_t *raw, cpuid_data_t *data, const char *file)
{
    int i, ret = 0;
    FILE *fp;

    fp = fopen(file, "a+");
    if (fp == NULL)
        return -1;

    ret = cpuid_deserialize_raw_data(raw, file);
    if (ret != ICUID_OK) {
        _eprintf("%s\n", icuid_errorstr(ret));
        fclose(fp);
        return -1;
    }

    fprintf(fp, "################EXPECTED RESULTS###############\n");
    fprintf(fp, "vendor_str=%s\n", data->vendor_str);
    fprintf(fp, "vendor_id=%u\n", data->vendor);
    fprintf(fp, "cpu_name=%s\n", data->brand_str);
    fprintf(fp, "cores=%u\n", data->cores);
    fprintf(fp, "logical=%u\n", data->logical_cpus);
    fprintf(fp, "codename=%s\n", data->codename);
    fprintf(fp, "family=%u\n", data->family);
    fprintf(fp, "model=%u\n", data->model);
    fprintf(fp, "stepping=%u\n", data->stepping);
    fprintf(fp, "type=%u\n", data->type);
    fprintf(fp, "ext_family=%u\n", data->ext_family);
    fprintf(fp, "ext_model=%u\n", data->ext_model);
    fprintf(fp, "signature=%u\n", data->signature);
    fprintf(fp, "l1d_cache=%u\n", data->l1_data_cache);
    fprintf(fp, "l1i_cache=%u\n", data->l1_instruction_cache);
    fprintf(fp, "l2_cache=%u\n", data->l2_cache);
    fprintf(fp, "l3_cache=%u\n", data->l3_cache);
    fprintf(fp, "l1_assoc=%u\n", data->l1_associativity);
    fprintf(fp, "l2_assoc=%u\n", data->l2_associativity);
    fprintf(fp, "l3_assoc=%u\n", data->l3_associativity);
    fprintf(fp, "l1_linesz=%u\n", data->l1_cacheline);
    fprintf(fp, "l2_linesz=%u\n", data->l2_cacheline);
    fprintf(fp, "l3_linesz=%u\n", data->l3_cacheline);
    fprintf(fp, "physical_addrsz=%u\n", data->physical_address_bits);
    fprintf(fp, "virtual_addrsz=%u\n", data->virtual_address_bits);
    fprintf(fp, "features=");
    for (i = 0; i < NUM_CPU_FEATURES; i++) {
        if (data->flags[i]) {
            if (i != 0)
                fprintf(fp, " %s", cpu_feature_str(i));
            else
                fprintf(fp, "%s", cpu_feature_str(i));
        }
    }
    fprintf(fp, "\n");

    fclose(fp);

    return 0;
}

int icuid_compare_string(const char *line, const char *name, const char *str)
{
    char *p = NULL;

    if (strncmp(line, name, strlen(name)) != 0)
        return 1;

    p = strchr(line, '\n');
    if (p != NULL) {
        *p = '\0';
        if (*(--p) == '\r')
            *p = '\0';
    }

    p = strchr(line, '=');
    if (p == NULL)
        goto err;
    p += 1;
    if (strcmp(str, p) != 0)
        goto err;

    return 1;

err:
    _eprintf("ERROR: got '%s' instead of '%s'\n", str, p);
    return 0;
}

int icuid_compare_uint(const char *line, const char *name, uint32_t value)
{
    char *p = NULL, *ex;
    unsigned long tmp_uint;

    if (strncmp(line, name, strlen(name)) != 0)
        return 1;

    p = strchr(line, '\n');
    if (p != NULL) {
        *p = '\0';
        if (*(--p) == '\r')
            *p = '\0';
    }

    p = strchr(line, '=');
    if (p == NULL)
        goto err;
    p += 1;

    errno = 0;
    tmp_uint = strtoul(p, &ex, 10);
    if (ex == p) {
        return 0;
    } else if (tmp_uint == ULONG_MAX && errno == ERANGE) {
        return 0;
    } else if (tmp_uint > 0xFFFFFFFF /* 2^32-1 */) {
        return 0;
    } else if ((tmp_uint == 0 && errno == EINVAL)) {
        return 0;
    }

    if (value != tmp_uint)
        goto err;

    return 1;

err:
    _eprintf("ERROR: got '%u' instead of '%lu'\n", value, tmp_uint);
    return 0;
}

#define BUF_SIZE 512

int run_test(cpuid_data_t *data, const char *file)
{
    FILE *fp;
    char line[BUF_SIZE];
    char tmp_features[BUF_SIZE];
    unsigned int i, errors = 0;

    fp = fopen(file, "rt");
    if (fp == NULL)
        return -1;

    /* Convert features to string */
    tmp_features[0] = '\0';
    for (i = 0; i < NUM_CPU_FEATURES; i++) {
        if (data->flags[i]) {
            strcat(tmp_features, cpu_feature_str(i));
            strcat(tmp_features, " ");
        }
    }
    tmp_features[strlen(tmp_features) - 1] = '\0';

    while (fgets(line, sizeof(line), fp)) {
        /* Check vendor name */
        if (!icuid_compare_string(line, "vendor_str", data->vendor_str)) {
            errors++;
            continue;
        }
        /* Check vendor id */
        if (!icuid_compare_uint(line, "vendor_id", data->vendor)) {
            errors++;
            continue;
        }
        /* Check CPU name */
        if (!icuid_compare_string(line, "cpu_name", data->brand_str)) {
            errors++;
            continue;
        }
        /* Check number of cores */
        if (!icuid_compare_uint(line, "cores", data->cores)) {
            errors++;
            continue;
        }
        /* Check number of logical cores */
        if (!icuid_compare_uint(line, "logical", data->logical_cpus)) {
            errors++;
            continue;
        }
        /* Check CPU codename */
        if (!icuid_compare_string(line, "codename", data->codename)) {
            errors++;
            continue;
        }
        /* Check CPU family */
        if (!icuid_compare_uint(line, "family", data->family)) {
            errors++;
            continue;
        }
        /* Check CPU model */
        if (!icuid_compare_uint(line, "model", data->model)) {
            errors++;
            continue;
        }
        /* Check CPU stepping */
        if (!icuid_compare_uint(line, "stepping", data->stepping)) {
            errors++;
            continue;
        }
        /* Check CPU type */
        if (!icuid_compare_uint(line, "type", data->type)) {
            errors++;
            continue;
        }
        /* Check CPU extended family */
        if (!icuid_compare_uint(line, "ext_family", data->ext_family)) {
            errors++;
            continue;
        }
        /* Check CPU extended model */
        if (!icuid_compare_uint(line, "ext_model", data->ext_model)) {
            errors++;
            continue;
        }
        /* Check CPU signature */
        if (!icuid_compare_uint(line, "signature", data->signature)) {
            errors++;
            continue;
        }
        /* Check L1 data cache */
        if (!icuid_compare_uint(line, "l1d_cache", data->l1_data_cache)) {
            errors++;
            continue;
        }
        /* Check L1 instruction cache */
        if (!icuid_compare_uint(line, "l1i_cache", data->l1_instruction_cache)) {
            errors++;
            continue;
        }
        /* Check L2 cache */
        if (!icuid_compare_uint(line, "l2_cache", data->l2_cache)) {
            errors++;
            continue;
        }
        /* Check L3 cache */
        if (!icuid_compare_uint(line, "l3_cache", data->l3_cache)) {
            errors++;
            continue;
        }
        /* Check L1 associativity */
        if (!icuid_compare_uint(line, "l1_assoc", data->l1_associativity)) {
            errors++;
            continue;
        }
        /* Check L2 associativity */
        if (!icuid_compare_uint(line, "l2_assoc", data->l2_associativity)) {
            errors++;
            continue;
        }
        /* Check L3 associativity */
        if (!icuid_compare_uint(line, "l3_assoc", data->l3_associativity)) {
            errors++;
            continue;
        }
        /* Check L1 cache line size */
        if (!icuid_compare_uint(line, "l1_linesz", data->l1_cacheline)) {
            errors++;
            continue;
        }
        /* Check L2 cache line size */
        if (!icuid_compare_uint(line, "l2_linesz", data->l2_cacheline)) {
            errors++;
            continue;
        }
        /* Check L3 cache line size */
        if (!icuid_compare_uint(line, "l3_linesz", data->l3_cacheline)) {
            errors++;
            continue;
        }
        /* Check physical address size */
        if (!icuid_compare_uint(line, "physical_addrsz", data->physical_address_bits)) {
            errors++;
            continue;
        }
        /* Check virtual address size */
        if (!icuid_compare_uint(line, "virtual_addrsz", data->virtual_address_bits)) {
            errors++;
            continue;
        }
        /* Check CPU features */
        if (!icuid_compare_string(line, "features", tmp_features)) {
            errors++;
            continue;
        }
    }
    fclose(fp);

    return errors;
}

static void usage(void)
{
    printf("usage: icuid_test [option]\n");
    printf(" --generate_test <file>\n");
    printf(" --run_test <file>\n");
}

int main(int argc, char **argv)
{
    int ret = -1;
    cpuid_raw_data_t raw;
    cpuid_data_t data;

    if (argc < 3) {
        usage();
        return ret;
    }

    if (strcmp("--generate_test", argv[1]) == 0) {
        ret = cpuid_deserialize_raw_data(&raw, argv[2]);
        if (ret != ICUID_OK) {
            _eprintf("%s\n", icuid_errorstr(ret));
            return ret;
        }
        ret = icuid_identify(&raw, &data);
        if (ret != ICUID_OK) {
            _eprintf("%s\n", icuid_errorstr(ret));
            return ret;
        }
        ret = generate_test(&raw, &data, argv[2]);
    } else if (strcmp("--run_test", argv[1]) == 0) {
        ret = cpuid_serialize_raw_data(&raw, argv[2]);
        if (ret != ICUID_OK) {
            _eprintf("%s\n", icuid_errorstr(ret));
            return ret;
        }
        ret = icuid_identify(&raw, &data);
        if (ret != ICUID_OK) {
            _eprintf("%s\n", icuid_errorstr(ret));
            return ret;
        }
        ret = run_test(&data, argv[2]);
    } else {
        _eprintf("Invalid option %s\n", argv[1]);
        usage();
        return ret;
    }

    return ret;
}
