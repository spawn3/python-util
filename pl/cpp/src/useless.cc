#include <iostream>

#include "useless.h"

// using namespace std;

int Useless::ct = 0;

Useless::Useless() {
    ++ct;
    std::cout << "default constructor called; number of objects: " << ct << std::endl;

    n = 0;
    pc = nullptr;
    ShowObject();
}

Useless::~Useless() {
    std::cout << "destructor called; object left: " << --ct << std::endl;
    std::cout << "deleted object:\n";
    ShowObject();
    delete []pc;
}

Useless::Useless(int k): n(k) {
    ++ct;
    std::cout << "int constructor called; number of objects: " << ct << std::endl;

    pc = new char[n];
    ShowObject();
}

Useless::Useless(int k, char ch): n(k) {
    ++ct;
    std::cout << "int, char constructor called; number of objects: " << ct << std::endl;

    pc = new char[n];
    for (int i=0; i < n; i++) {
        pc[i] = ch;
    }
    ShowObject();
}

Useless::Useless(const Useless& f): n(f.n) {
    ++ct;
    std::cout << "copy constructor called; number of objects: " << ct << std::endl;

    pc = new char[n];
    for (int i=0; i < n; i++) {
        pc[i] = f.pc[i];
    }
    ShowObject();
}

Useless::Useless(Useless&& f): n(f.n) {
    ++ct;
    std::cout << "move constructor called; number of objects: " << ct << std::endl;

    pc = f.pc;

    f.n = 0;
    f.pc = nullptr;

    ShowObject();
}

void Useless::ShowObject() const {
    std::cout << "\tNumber of elements: " << n;
    std::cout << " Data address: " << (void *)pc << std::endl;
}

Useless Useless::operator+(const Useless& f) const {
    std::cout << "Entering operator+()\n";
    Useless temp = Useless(n + f.n);

    for (int i=0; i<n; i++) {
        temp.pc[i] = pc[i];
    }

    for (int i=n; i < temp.n; i++) {
        temp.pc[i] = f.pc[i-n];
    }

    std::cout << "temp object:\n";
    temp.ShowObject();
    std::cout << "Leaving operator+()\n";

    return temp;
}
