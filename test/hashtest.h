#ifndef HASHTEST_H
#define HASHTEST_H
class hashtest
{
public:
    hashtest();
    ~hashtest();

    bool DoTest();
private:
    bool DoBinHashTest();
    bool DoLruHashTest();
    bool LruCheck(void * vtable);
};

#endif /* HASHTEST_H */