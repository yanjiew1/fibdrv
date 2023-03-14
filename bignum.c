#include "bignum.h"
#include <linux/slab.h>

void bn_add(struct bignum *c, struct bignum *a, struct bignum *b)
{
    int i, j;
    int carry = 0;

    for (i = 0; i < a->size && i < b->size && i < c->capacity; i++) {
        c->digits[i] = a->digits[i] + b->digits[i] + carry;
        carry = c->digits[i] < a->digits[i];
    }

    for (; i < a->size && i < c->capacity; i++) {
        c->digits[i] = a->digits[i] + carry;
        carry = c->digits[i] < a->digits[i];
    }

    for (; i < b->size && i < c->capacity; i++) {
        c->digits[i] = b->digits[i] + carry;
        carry = c->digits[i] < a->digits[i];
    }

    if (i < c->capacity && carry) {
        c->digits[i] = carry;
        i++;
    }

    j = i;

    for (; j < c->size; j++)
        c->digits[j] = 0;

    c->size = i;
}

void bn_sub(struct bignum *c, struct bignum *a, struct bignum *b)
{
    int i, j;
    int borrow = 0;

    for (i = 0; i < a->size && i < b->size && i < c->capacity; i++) {
        c->digits[i] = a->digits[i] - b->digits[i] - borrow;
        borrow = c->digits[i] > a->digits[i];
    }

    for (; i < a->size && i < c->capacity; i++) {
        c->digits[i] = a->digits[i] - borrow;
        borrow = c->digits[i] > a->digits[i];
    }

    for (; i < b->size && i < c->capacity; i++) {
        c->digits[i] = b->digits[i] - borrow;
        borrow = c->digits[i] > a->digits[i];
    }

    j = i;
    for (; j < c->size; j++)
        c->digits[i] = 0;
    c->size = i;

    while (c->size > 0 && c->digits[c->size - 1] == 0)
        c->size--;
}

void bn_mul(struct bignum *c, struct bignum *a, struct bignum *b)
{
    int i, j;

    /* Clear c */
    for (i = 0; i < c->size; i++)
        c->digits[i] = 0;

    for (i = 0; i < a->size && i < c->capacity; i++) {
        for (j = 0; j < b->size && i + j < c->capacity; j++) {
            __uint128_t product =
                (__uint128_t) a->digits[i] * (__uint128_t) b->digits[j];

            unsigned long product0 = product;
            unsigned long product1 = product >> 64;

            int carry = 0;
            c->digits[i + j] += product0;

            if (i + j + 1 >= c->capacity)
                continue; /* Overflow */

            carry = c->digits[i + j] < product;
            product1 += carry;
            carry = product1 < carry;
            c->digits[i + j + 1] += product1;
            carry += c->digits[i + j + 1] < product1;

            for (int k = i + j + 2; k < c->capacity && carry; k++) {
                c->digits[k] += carry;
                carry = c->digits[k] < carry;
            }
        }
    }

    /* Calculate the size */
    c->size = a->size + b->size;
    if (c->size > c->capacity)
        c->size = c->capacity;

    while (c->size > 0 && c->digits[c->size - 1] == 0)
        c->size--;
}

void bn_lshift1(struct bignum *c)
{
    int i = c->size;

    if (i >= c->capacity)
        i--;
    int j = i;

    for (; i >= 1; i--)
        c->digits[i] = c->digits[i] << 1 | c->digits[i - 1] >> 63;
    c->digits[0] = c->digits[0] << 1;

    if (c->digits[j] != 0)
        c->size++;
}

int bn_init(struct bignum *bn, int capacity)
{
    bn->capacity = capacity;
    bn->size = 0;
    bn->digits = kmalloc(capacity * sizeof(unsigned long), GFP_KERNEL);

    if (bn->digits == NULL)
        return -ENOMEM;

    return 0;
}

void bn_free(struct bignum *bn)
{
    kfree(bn->digits);
}

void bn_set(struct bignum *c, struct bignum *a)
{
    int i;

    for (i = 0; i < a->size && i < c->capacity; i++)
        c->digits[i] = a->digits[i];

    for (; i < c->size; i++)
        c->digits[i] = 0;

    c->size = a->size;
}

void bn_swap(struct bignum *a, struct bignum *b)
{
    struct bignum tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}
