#ifndef PLUGIN_TEST_H
#define PLUGIN_TEST_H

class PluginTest
{
public:
    PluginTest();
    ~PluginTest();

    bool DoTest();
private:
    bool LoadPcapFile(char const * fileName);
    void LoadOpt(int argc, char * argv[]);
};

#endif /* PLUGIN_TEST_H */

