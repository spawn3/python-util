#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>
#include <algorithm>
#include <functional>

#include "hello.h"

typedef std::vector<int> int_vector;

static inline void test_header(const char *tip) {
    std::cout << "\n== TEST " << tip << " ==\n";
}

void test_hello() {
    assert(hello_sum(1, 1) == 2);

    std::string a = "aaa";
    std::string&& b = std::move(a);
    std::string c = std::move(b);
    // std::string&& b = static_cast<std::string&&>(a);

    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "c = " << c << std::endl;
}

void test_constexpr() {
    const int n = 5;
    std::array<int, n> v;

    (void) v;

}

void test_lambda() {
    test_header(__FUNCTION__);

    std::vector<int> c = {1, 2, 3, 4, 5, 6, 7};
    int x = 5;
    c.erase(std::remove_if(c.begin(), c.end(), [x](int n) { return n < x;  }), c.end());

    std::cout << "c: ";
    std::for_each(c.begin(), c.end(), [](int i){ std::cout << i << ' ';  });
    std::cout << '\n';

    // the type of a closure cannot be named, but can be inferred with auto
    // since C++14, lambda could own default arguments
    auto func1 = [](int i = 6) { return i + 4;  };
    std::cout << "func1: " << func1() << '\n';

    // like all callable objects, closures can be captured in std::function
    // (this may incur unnecessary overhead)
    std::function<int(int)> func2 = [](int i) { return i + 4;  };
    std::cout << "func2: " << func2(6) << '\n';
}

int test_vector() {
    test_header(__FUNCTION__);

    // const int ci = 1;

    // ci = 2;

    int_vector v;
    for (int i=0; i < 10; i++) {
        v.push_back(i);
    }

    for (int i=0; i < 10; i++) {
        std::cout << i << ": " << v[i] << std::endl;
    }

    for (int_vector::iterator it = v.begin(); it != v.end(); it++) {
        std::cout << ": " << *it << std::endl;
    }

    return 0;
}

class Person {
public:
    virtual ~Person() {
        std::cout << "deleting a person\n";
    }

    virtual void aboutMe() {
        std::cout << "I am a person." << std::endl;
    }

    virtual void addCourse(const std::string &course) = 0;

private:
    int id;
};

class Student: public Person {
public:
    ~Student() {
        std::cout << "deleting a student\n";
    }

    void aboutMe() {
        std::cout << "I am a student." << std::endl;
    }

    virtual void addCourse(const std::string &course) {
        std::cout << "add course " << course << " to student\n";
    }
};

void test_class() {
    test_header(__FUNCTION__);

    Person *p = new Student();
    p->aboutMe();
    p->addCourse("Math");

    delete p;
}

void test_except() {
    test_header(__FUNCTION__);

    try {
        std::vector<int> v{3, 4, 3, 1};
        int i{v.at(4)};
        (void) i;
    } catch (std::out_of_range &e) {
        std::cerr << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Some fatal error\n";
    }
}

void test_memory() {
    test_header(__FUNCTION__);

    int ret;
    void *ptr;

    ret = posix_memalign(&ptr, 4096, 1048576 * 2);
    assert(ret == 0);

    printf("ret %d ptr %p\n", ret, ptr);
}

int func_args_stack(int a, int b) {
    printf("a = %d, b = %d\n", a, b);
    return 0;
}

int f() {
    printf("f called\n");
    return 0;
}

int g() {
    printf("g called\n");
    return 0;
}

/**
 * argv is a pointer array
 */
int main(int argc, char *argv[]) {
    printf("enter main\n");

    for (int i=0; i < argc; i++) {
        std::cout << i << ": " << &argv[i] << " " << argv[i] << std::endl;
    }

    int a[] = {1, 2, 3, 4, 5};

    for (auto& i : a) {
        std::cout << i << std::endl;
    }

    test_hello();
    test_vector();
    test_class();
    test_except();
    test_memory();
    test_lambda();
    test_constexpr();

    {
        func_args_stack(f(), g());
    }

    return 0;
}

/**
 * @see spdk
 */
__attribute__((constructor(102))) static void register_trace_log_flag() {
    printf("%s\n", __FUNCTION__);
}

__attribute__((constructor(101))) static void register_trace_log_flag_2() {
    printf("%s\n", __FUNCTION__);
}
