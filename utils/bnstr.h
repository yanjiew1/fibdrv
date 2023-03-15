#ifndef __BNSTR_H_
#define __BNSTR_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* estimate required space */
static size_t bn_str_len(int nlimbs)
{
    return 20 * nlimbs + 1;
}

static void bn_div_ul(uint64_t *q,
                      uint64_t *r,
                      const uint64_t *n,
                      int nlimbs,
                      int *qlimbs,
                      uint64_t d)
{
    uint64_t rem = 0;
    int i;
    *qlimbs = 0;

    for (i = nlimbs - 1; i >= 0; i--) {
        __uint128_t num = ((__uint128_t) rem << 64) | n[i];
        uint64_t quot;

        quot = num / d;
        rem = num % d;
        q[i] = quot;

        if (quot != 0 && i + 1 > *qlimbs)
            *qlimbs = i + 1;
    }

    *r = rem;
}

static char *bn_to_str(char *buf, uint64_t *bn, int nlimbs)
{
    char *p = buf;
    uint64_t *q = (uint64_t *) malloc(nlimbs * sizeof(uint64_t));
    int qlimbs = nlimbs;

    if (!q)
        return NULL;

    memcpy(q, bn, nlimbs * sizeof(uint64_t));

    while (qlimbs > 0) {
        uint64_t rem;
        bn_div_ul(q, &rem, q, qlimbs, &qlimbs, 10000000000000000000ULL);
        uint64_t tmp = rem;
        for (int i = 0; i < 19; i++) {
            *p++ = '0' + tmp % 10;
            tmp /= 10;
            if (!tmp && !qlimbs)
                break;
        }
    }

    /* swap buffer */
    size_t bufsize = p - buf;
    for (size_t i = 0; i < bufsize / 2; i++) {
        char tmp = buf[i];
        buf[i] = buf[bufsize - i - 1];
        buf[bufsize - i - 1] = tmp;
    }

    *p = '\0';
    free(q);

    return buf;
}

#endif
