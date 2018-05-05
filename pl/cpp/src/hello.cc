#include <iostream>
#include <vector>
#include <string>

typedef std::vector<int> int_vector;

int test_vector() {
    std::cout << "== TEST VECTOR ==\n";

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
    std::cout << "== TEST CLASS ==\n";

    Person *p = new Student();
    p->aboutMe();
    p->addCourse("Math");

    delete p;
}


int main(int argc, char **argv) {
    for (int i=0; i < argc; i++) {
        std::cout << i << ": " << argv[i] << std::endl;
    }

    test_vector();
    test_class();

    return 0;
}
