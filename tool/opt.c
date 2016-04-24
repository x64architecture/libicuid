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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "internal/stdcompat.h"
#include "opt.h"

#define OPT_WIDTH 5

void options_usage(struct OPTION *opts)
{
    const char *p, *q;
    char optstr[36];
    int i;

    for (i = 0; opts[i].name != NULL; i++) {
        if (opts[i].desc == NULL)
            continue;

        assert(strlen(opts[i].name) < sizeof(optstr));
        sprintf(optstr, "--%s %s", opts[i].name,
                (opts[i].argname != NULL) ? opts[i].argname : "");
        fprintf(stderr, " %-*s", OPT_WIDTH, optstr);
        if (strlen(optstr) > OPT_WIDTH)
            fprintf(stderr, "\n %-*s", OPT_WIDTH, "");

        p = opts[i].desc;
        for (;;) {
            q = strchr(p, '\n');
            if (q == NULL)
                break;
            fprintf(stderr, " %.*s", (int)(q - p), p);
            fprintf(stderr, "\n %-*s", OPT_WIDTH, "");
            p = q + 1;
        }
        fprintf(stderr, " %s\n", p);
    }
}

int options_parse(int argc, char **argv, struct OPTION *opts, char **unnamed)
{
    struct OPTION *opt;
    char *arg, *p;
    int i, j;

    for (i = 1; i < argc; i++) {
        p = arg = argv[i];

        /* Handle arguments with a leading double dash */
        p++;
        if (*p++ != '-') {
            if (unnamed == NULL)
                goto unknown;
            *unnamed = arg;
            continue;
        }

        for (j = 0; opts[j].name != NULL; j++) {
            opt = &opts[j];
            if (strcmp(p, opt->name) != 0)
                continue;

            if (opt->type == OPTION_ARG) {
                if (++i >= argc) {
                    fprintf(stderr, "missing %s argument for --%s\n",
                            opt->argname, opt->name);
                    return 1;
                }
            } else if (opt->type == OPTION_ARG_NR) {
                if (++i >= argc) {
                    *opt->arg = "";
                    return 0;
                }
            }

            switch (opt->type) {
                case OPTION_ARG:
                    *opt->arg = argv[i];
                    break;
                case OPTION_ARG_NR:
                    *opt->arg = argv[i];
                    break;
                case OPTION_FLAG:
                    *opt->flag = 1;
                    break;

                default: /* invalid type */
                    fprintf(stderr, "option %s - unknown type %i\n",
                            opt->name, opt->type);
                    return 1;
            }

            break;
        }

        if (opts[j].name == NULL)
            goto unknown;
    }

    return 0;

unknown:
    fprintf(stderr, "unknown option '%s'\n", arg);
    return (1);
}
