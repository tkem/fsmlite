#include <cassert>
#include <type_traits>

#include "fsmlite.h"

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

void test_is_invocable()
{
    struct foo {
        void operator()();
        void operator()(int);
        void operator()(char*);
    };

    assert((is_invocable<foo>::value));
    assert((is_invocable<foo, int>::value));
    assert((is_invocable<foo, int&>::value));
    assert((is_invocable<foo, const int&>::value));
    assert((is_invocable<foo, long>::value));
    assert((is_invocable<foo, char*>::value));

    assert((!is_invocable<foo, int*>::value));
    assert((!is_invocable<foo, const char*>::value));
    assert((!is_invocable<foo, void*>::value));
    assert((!is_invocable<foo, int, char*>::value));
}

void test_is_invocable_fn()
{
    void foo();
    int bar(int);

    assert((is_invocable<decltype(foo)>::value));
    assert((!is_invocable<decltype(foo), int>::value));
    assert((!is_invocable<decltype(foo), int&>::value));
    assert((!is_invocable<decltype(foo), const int&>::value));
    assert((!is_invocable<decltype(foo), long>::value));
    assert((!is_invocable<decltype(foo), int*>::value));
    assert((!is_invocable<decltype(foo), const char*>::value));


    assert((!is_invocable<decltype(bar)>::value));
    assert((is_invocable<decltype(bar), int>::value));
    assert((is_invocable<decltype(bar), int&>::value));
    assert((is_invocable<decltype(bar), const int&>::value));
    assert((is_invocable<decltype(bar), long>::value));
    assert((!is_invocable<decltype(bar), int*>::value));
    assert((!is_invocable<decltype(bar), const char*>::value));
}

int main()
{
    test_concat();
    test_filter();
    test_is_invocable();
    test_is_invocable_fn();
    return 0;
}
