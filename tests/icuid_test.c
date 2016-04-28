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

#define BUF_SIZE 512

int run_test(cpuid_data_t *data, const char *file)
{
    FILE *fp;
    char line[BUF_SIZE], tmp[BUF_SIZE];
    char tmp_features[BUF_SIZE];
    int i, errors = 0;
    uint32_t tmpuint;

    fp = fopen(file, "rt");
    if (fp == NULL)
        return -1;

    while (fgets(line, sizeof(line), fp)) {
        /* Check vendor name */
        if (sscanf(line, "vendor_str=%s", tmp) == 1) {
            if (strcmp(data->vendor_str, tmp) != 0) {
                _eprintf("ERROR: got %s instead of %s\n", data->vendor_str, tmp);
                errors++;
            }
            continue;
        }
        /* Check vendor id */
        if (sscanf(line, "vendor_id=%u", &tmpuint) == 1) {
            if (data->vendor != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->vendor, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check CPU name */
        if (sscanf(line, "cpu_name=%[^\n]", tmp) == 1) {
            if (memcmp(data->brand_str, tmp, strlen(tmp) - 1) != 0) {
                _eprintf("ERROR: got %s instead of %s", data->brand_str, tmp);
                errors++;
            }
            continue;
        }
        /* Check number of cores */
        if (sscanf(line, "cores=%u", &tmpuint) == 1) {
            if (data->cores != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->cores, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check number of logical cores */
        if (sscanf(line, "logical=%u", &tmpuint) == 1) {
            if (data->logical_cpus != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->logical_cpus, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check CPU codename */
        if (sscanf(line, "codename=%[^\n]", tmp) == 1) {
            if (memcmp(data->codename, tmp, strlen(tmp) - 1) != 0) {
                _eprintf("ERROR: got %s instead of %s\n", data->codename, tmp);
                errors++;
            }
            continue;
        }
        /* Check CPU family */
        if (sscanf(line, "family=%u", &tmpuint) == 1) {
            if (data->family != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->family, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check CPU model */
        if (sscanf(line, "model=%u", &tmpuint) == 1) {
            if (data->model != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->model, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check CPU stepping */
        if (sscanf(line, "stepping=%u", &tmpuint) == 1) {
            if (data->stepping != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->stepping, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check CPU type */
        if (sscanf(line, "type=%u", &tmpuint) == 1) {
            if (data->type != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->type, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check CPU extended family */
        if (sscanf(line, "ext_family=%u", &tmpuint) == 1) {
            if (data->ext_family != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->ext_family, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check CPU extended model */
        if (sscanf(line, "ext_model=%u", &tmpuint) == 1) {
            if (data->ext_model != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->ext_model, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check CPU signature */
        if (sscanf(line, "signature=%u", &tmpuint) == 1) {
            if (data->signature != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->signature, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check L1 data cache */
        if (sscanf(line, "l1d_cache=%u", &tmpuint) == 1) {
            if (data->l1_data_cache != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->l1_data_cache, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check L2 cache */
        if (sscanf(line, "l2_cache=%u", &tmpuint) == 1) {
            if (data->l2_cache != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->l2_cache, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check L1 instruction cache */
        if (sscanf(line, "l3_cache=%u", &tmpuint) == 1) {
            if (data->l3_cache != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->l3_cache, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check L1 associativity */
        if (sscanf(line, "l1_assoc=%u", &tmpuint) == 1) {
            if (data->l1_associativity != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n",
                         data->l1_associativity, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check L2 associativity */
        if (sscanf(line, "l2_assoc=%u", &tmpuint) == 1) {
            if (data->l2_associativity != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n",
                         data->l2_associativity, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check L3 associativity */
        if (sscanf(line, "l3_assoc=%u", &tmpuint) == 1) {
            if (data->l3_associativity != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n",
                         data->l3_associativity, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check L1 cache line size */
        if (sscanf(line, "l1_linesz=%u", &tmpuint) == 1) {
            if (data->l1_cacheline != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->l1_cacheline, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check L2 cache line size */
        if (sscanf(line, "l2_linesz=%u", &tmpuint) == 1) {
            if (data->l2_cacheline != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->l2_cacheline, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check L3 cache line size */
        if (sscanf(line, "l3_linesz=%u", &tmpuint) == 1) {
            if (data->l3_cacheline != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n", data->l3_cacheline, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check physical address size */
        if (sscanf(line, "physical_addrsz=%u", &tmpuint) == 1) {
            if (data->physical_address_bits != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n",
                    data->physical_address_bits, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check virtual address size */
        if (sscanf(line, "virtual_addrsz=%u", &tmpuint) == 1) {
            if (data->virtual_address_bits != tmpuint) {
                _eprintf("ERROR: got %u instead of %u\n",
                         data->virtual_address_bits, tmpuint);
                errors++;
            }
            continue;
        }
        /* Check CPU features with some regex magic */
        if (sscanf(line, "features=%[^\n]", tmp) == 1) {
            tmp_features[0] = '\0';
            for (i = 0; i < NUM_CPU_FEATURES; i++) {
                if (data->flags[i]) {
                    strcat(tmp_features, cpu_feature_str(i));
                    if (i == (NUM_CPU_FEATURES - 1))
                        break;
                    strcat(tmp_features, " ");
                }
            }
            if (memcmp(tmp_features, tmp, strlen(tmp) - 1) != 0) {
                _eprintf("ERROR: got: %s\ninstead of: %s\n", tmp_features, tmp);
                errors++;
            }
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
