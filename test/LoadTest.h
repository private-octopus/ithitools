#ifndef LOAD_TEST_H
#define LOAD_TEST_H

class LoadTest
{
public:
    LoadTest();
    ~LoadTest();

    bool DoTest();
private:
    bool DoGoodTest();
    bool DoNoSuchTest();
};

#endif