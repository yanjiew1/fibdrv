#include "bignum.h"

void bn_add(struct bignum *c, struct bignum *a, struct bignum *b)
{
    int i;
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
        c->digits[i] = a->digits[i] + carry;
        carry = c->digits[i] < a->digits[i];
    }

    if (i < c->capacity && carry) {
        c->digits[i] = carry;
        i++;
    }

    c->size = i;

    for (; i < c->size; i++)
        c->digits[i] = 0;
}

void bn_sub(struct bignum *c, struct bignum *a, struct bignum *b)
{
    int i;
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
        c->digits[i] = a->digits[i] - borrow;
        borrow = c->digits[i] > a->digits[i];
    }

    if (borrow) {
        for (; i < c->capacity; i++)
            c->digits[i] = -1;
        c->size = c->capacity;
    } else {
        c->size = i;
        for (; i < c->size; i++)
            c->digits[i] = 0;

        while (c->size > 0 && c->digits[c->size - 1] == 0)
            c->size--;
    }
}
