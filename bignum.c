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
