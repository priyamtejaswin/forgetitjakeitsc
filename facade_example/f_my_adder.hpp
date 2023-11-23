#ifndef F_MY_ADDER
#define F_MY_ADDER

typedef struct FMyAdder FMyAdder;

FMyAdder* createMyAdder(const int val);
void destroyMyAdder(FMyAdder *fma);
void do_add(FMyAdder *fma, const int val);

#endif