#include <iostream>
#include <memory>

void func()
{
    std::unique_ptr<int> some_int;
    std::cout << "Plugin will cause segfault now!" << std::endl;
    *some_int = 10;
}

extern "C" int function(const char* /*some_string*/)
{
    int some_int {};
    func();
    return some_int;
}
