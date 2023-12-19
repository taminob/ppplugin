#include <boost/dll.hpp>
#include <iostream>

void initialize()
{
    std::cout << "C++ initialize" << std::endl;
}

// necessary to circumvent different C++ name mangling schemes
BOOST_DLL_AUTO_ALIAS(initialize);
// this would be the alternative:
//namespace prevent_conflict {
//extern "C" const void* initialize;
//const void* initialize = reinterpret_cast<void*>(&::initialize);
//} // namespace prevent_conflict

class A {
public:
    static int loop(int value)
    {
        std::cout << "C++: " << value++ << std::endl;
        return value;
    }
};
BOOST_DLL_ALIAS(A::loop, loop);
