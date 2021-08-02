#include <cassert>
#include <type_traits>

#include "fsm.h"

using namespace fsmlite::detail;

void test_concat()
{
    assert((std::is_same<list<>, concat<>::type>::value));
    assert((std::is_same<list<int>, concat<int, list<>>::type>::value));
    assert((std::is_same<list<int, char>, concat<int, list<char>>::type>::value));
}

void test_filter()
{
    assert((std::is_same<list<>, filter<std::is_integral>::type>::value));
    assert((std::is_same<list<>, filter<std::is_integral, float>::type>::value));
    assert((std::is_same<list<int>, filter<std::is_integral, int>::type>::value));
    assert((std::is_same<list<int>, filter<std::is_integral, int, float>::type>::value));
    assert((std::is_same<list<int>, filter<std::is_integral, float, int>::type>::value));
    assert((std::is_same<list<int, long>, filter<std::is_integral, int, long>::type>::value));
    assert((std::is_same<list<long, int>, filter<std::is_integral, long, int>::type>::value));
}

int main()
{
    test_concat();
    test_filter();
    return 0;
}
