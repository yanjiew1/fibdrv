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

int bn_init(struct bignum *bn, int capacity);
void bn_free(struct bignum *bn);
void bn_set_ul(struct bignum *c, unsigned long a);
void bn_set(struct bignum *c, struct bignum *a);
void bn_swap(struct bignum *a, struct bignum *b);

#endif
