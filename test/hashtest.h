#ifndef HASHTEST_H
#define HASHTEST_H

#include "ithi_test_class.h"

class hashtest:public ithi_test_class
{
public:
    hashtest();
    ~hashtest();

    bool DoTest() override;

private:
    bool DoBinHashTest(char const ** hash_input, size_t nb_input);
    bool DoLruHashTest(char const ** hash_input, size_t nb_input);
    bool LruCheck(void * vtable);
};

#endif /* HASHTEST_H */