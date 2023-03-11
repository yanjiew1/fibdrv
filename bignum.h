#ifndef __BIGNUM_H_
#define __BIGNUM_H_

struct bignum {
    unsigned long *digits;
    int capacity;
    int size;
};

void bn_add(struct bignum *c, struct bignum *a, struct bignum *b);
void bn_sub(struct bignum *c, struct bignum *a, struct bignum *b);
void bn_mul(struct bignum *c, struct bignum *a, struct bignum *b);
void bn_lshift1(struct bignum *c);

#endif
