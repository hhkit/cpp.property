#include <iostream>

#include "property.hpp"

class A {
 private:
  int i_ = 5;

  int get_i() const { return i_; }

  void set_i(int new_i) {
    if (new_i < 0 || new_i > 10) return;
    i_ = new_i;
  }

  float get_f() const { return 50.f; }

 public:
  A(){};  // note: default constructor required for MSVC
  // A() = default; // will fail to link under MSVC

  PROPERTIES_BEGIN()
  fcp::property<int, &A::get_i, &A::set_i> i;
  fcp::property<float, &A::get_f> f;
  PROPERTIES_END()
};

int main() {
  A a{};

  auto a2 = a;

  a.i = 6;
  std::cout << "a.i: " << a.i << "\n";  // prints 6

  a.i = 20;
  std::cout << "a.i: " << a.i << "\n";  // prints 6

  // auto error = a.i;                  // will not compile
  // a.f = 7.f;                         // fails to compile
  std::cout << "a.f: " << a.f << "\n";  // prints 50
}