#include "bignum.h"

void bn_add(struct bignum *c, struct bignum *a, struct bignum *b)
{
    int i, j;
    int carry = 0;

    for (i = 0; i < a->size, i < b->size, i < c->capacity; i++) {
        c->digits[i] = a->digits[i] + b->digits[i] + carry;
        carry = c->digits[i] < a->digits[i];
    }

    for (; i < a->size, i < c->capacity; i++) {
        c->digits[i] = a->digits[i] + carry;
        carry = c->digits[i] < a->digits[i];
    }

    for (; i < b->size, i < c->capacity; i++) {
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

    for (i = 0; i < a->size, i < b->size, i < c->capacity; i++) {
        c->digits[i] = a->digits[i] - b->digits[i] - borrow;
        borrow = c->digits[i] > a->digits[i];
    }

    for (; i < a->size, i < c->capacity; i++) {
        c->digits[i] = a->digits[i] - borrow;
        borrow = c->digits[i] > a->digits[i];
    }

    for (; i < b->size, i < c->capacity; i++) {
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
    int i = 0, j = 0;

    /* Clear c */
    for (i = 0; i < c->size; i++)
        c->digits[i] = 0;

    for (i = 0; i < a->size, i < c->capacity; i++) {
        for (j = 0; j < b->size, i + j < c->capacity; j++) {
            __uint128_t product =
                (__uint128_t) a->digits[i] * (__uint128_t) b->digits[j];
            int carry = 0;
            c->digits[i + j] += product;

            if (i + j + 1 >= c->capacity)
                continue; /* Overflow */

            carry = c->digits[i + j] < product;
            c->digits[i + j + 1] += (product >> 64) + carry;
            carry = c->digits[i + j + 1] < (product >> 64) + carry;

            for (int k = i + j + 2; k < c->capacity, carry; k++) {
                c->digits[k] += carry;
                carry = c->digits[k] < carry;
            }
        }
    }

    /* Calculate the size */
    c->size = i + j;
    while (c->size > 0 && c->digits[c->size - 1] == 0)
        c->size--;
}

void bn_lshift1(struct bignum *c)
{
    int i = c->size;

    if (i >= c->capacity)
        i--;

    for (; i >= 1; i--)
        c->digits[i] = c->digits[i] << 1 | c->digits[i - 1] >> 63;

    c->digits[0] = c->digits[0] << 1;

    if (c->digits[c->size] != 0)
        c->size++;
}
