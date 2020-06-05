# cpp.property

A single-header property class written in C++17 that adds property functionality
to C++.

Note that C++ was not built to support this for a reason. Enabling properties in
your class is not a zero-cost abstraction. In particular, this system has the
following weaknesses:
* Properties will set your class's minimum alignment to `alignof(unsigned)`.
* Properties will add a minimum of `sizeof(unsigned)` bytes to your class, 
  padded to `alignof(unsigned)`.
* Getters and setters used in the properties must be declared before the 
  properties themselves or the symbol will not be unrecognized by the compiler.

## Installation
* Copy `property.hpp` into your project.
* `#include "property.hpp"`
* ???
* Profit

## Usage
A property is constructed using the `fcp::property` template, which takes in
three arguments:
1. `T`, the type of the property.
2. `GetFn`, a pointer to the getter member function for the property
   The member function pointed to by GetFn must be const-qualified.
3. `SetFn`, an optional setter for the property.

The properties must be guarded by the `PROPERTIES_BEGIN()` and 
`PROPERTIES_END()` macros, which hide a union and a property_offset object that
serves as a helper to determine the `this` pointer of the object. Changing the
values in the union and adding non-property variables to the guard may result in
undefined behavior.

Example:
```cpp
#include "property.hpp"
#include <iostream>

class A {
private:
  int i_ = 5;

  int get_i() const {
    return i_;
  }

  void set_i(int new_i) {
    if (new_i < 0 || new_i > 10)
      return;
    i_ = new_i;
  }

  float get_f() const {
    return 50.f;
  }
public:
  PROPERTIES_BEGIN()
    fcp::property<int, &A::get_i, &A::set_i> i;
    fcp::property<int, &A::get_f> f;
  PROPERTIES_END()
};

int main()
{
  A a{};

  a.i = 6;
  std::cout << "a.i: " << a.i << "\n"; // prints 6

  a.i = 20;
  std::cout << "a.i: " << a.i << "\n"; // prints 6
  
  // a.f = 7.f;                        // fails to compile
  std::cout << "a.f: " << a.f << "\n"; // prints 50
}
```

## Known Issues
* Properties may be copied out of their respective classes, which may result in
  undefined behavior or segmentation faults. For example, this is an error that
  is impossible to detect at compile time.
  ```cpp
  int main()
  {
    A a{};
    auto runtime_error = a.i; // will cause runtime error
    runtime_error = 5;
  }
  ```

* MSVC fails to link when default constructing a value without arguments.
  This is due to an abuse in the language where the this pointer is passed in
  during aggregate initialization, which MSVC does not invoke when no arguments
  are provided. To fix this issue, simply call the constructor by using an empty
  pair of braces.

  ```cpp
  int main()
  {
    A a; // fails to compile
    A a{}; // compiles
  }
  ```
