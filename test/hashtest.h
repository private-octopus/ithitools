#ifndef HASHTEST_H
#define HASHTEST_H
class hashtest
{
public:
    hashtest();
    ~hashtest();

    bool DoTest();
private:
    bool DoBinHashTest(char const ** hash_input, size_t nb_input);
    bool DoLruHashTest(char const ** hash_input, size_t nb_input);
    bool LruCheck(void * vtable);
};

#endif /* HASHTEST_H */