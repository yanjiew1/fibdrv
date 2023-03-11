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
