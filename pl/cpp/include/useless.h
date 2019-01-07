#ifndef __USELESS_H
#define __USELESS_H

class Useless {
private:
    int n;
    char *pc;
    static int ct;
    void ShowObject() const;
public:
    Useless();
    explicit Useless(int k);
    Useless(int k, char ch);
    Useless(const Useless& f);
    ~Useless();

    Useless operator+(const Useless& f) const;
};

#endif
