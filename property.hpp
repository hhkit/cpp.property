//
// Copyright (c) Ho Han Kit Ivan. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for
// details.
//

#ifndef FCP_PROPERTY_HPP_
#define FCP_PROPERTY_HPP_

#include <cstddef>
#include <iosfwd>

namespace fcp {
namespace prop_detail {
template <typename T> struct extract_memfn_types;
} // namespace prop_detail

template <typename T, auto GetFn, auto SetFn = nullptr> class property;

template <typename T, auto GetFn, auto SetFn>
std::ostream &operator<<(std::ostream &os, const property<T, GetFn, SetFn> &p);

template <typename T, auto GetFn, auto SetFn> class property {
private:
  using GetFnInfo = prop_detail::extract_memfn_types<decltype(GetFn)>;
  using SetFnInfo = prop_detail::extract_memfn_types<decltype(SetFn)>;

public:
  explicit property() = default;
  explicit property(const property &) = default;
  explicit property(property &&) = default;

  T get() const noexcept(GetFnInfo::is_noexcept_v);
  void set(const T &val) noexcept(SetFnInfo::is_noexcept_v);

  operator T() const noexcept(GetFnInfo::is_noexcept_v);
  property &operator=(const T &rhs) noexcept(SetFnInfo::is_noexcept_v);
  template <typename T2, auto GetFn2, auto SetFn2>
  property &operator=(const property<T2, GetFn2, SetFn2> &rhs) noexcept(
      SetFnInfo::is_noexcept_v &&noexcept(rhs.get()));

  friend std::ostream &operator<<<>(std::ostream &os, const property &p);

private:
  using GetT = typename GetFnInfo::type;
  using SetT = typename SetFnInfo::type;

  int offset_;

  const GetT *getter_this() const noexcept;
  SetT *setter_this() noexcept;
};

template <typename T, auto GetFn> class property<T, GetFn, nullptr> {
private:
  using GetFnInfo = prop_detail::extract_memfn_types<decltype(GetFn)>;

public:
  explicit property() = default;
  explicit property(const property &) = default;
  explicit property(property &&) = default;

  T get() const noexcept(GetFnInfo::is_noexcept_v);
  operator T() const noexcept(noexcept(get()));

  friend std::ostream &operator<<<>(std::ostream &os, const property &p);

private:
  using GetT = typename prop_detail::extract_memfn_types<decltype(GetFn)>::type;

  int offset_;

  const GetT *getter_this() const noexcept;
};
} // namespace fcp

#define PROPERTIES_BEGIN()                                                     \
  union {                                                                      \
    ::fcp::prop_detail::property_offset PROPERTY_OFFSET{this};

#define PROPERTIES_END()                                                       \
  }                                                                            \
  ;

/* Implementation follows below. */

namespace fcp {
namespace prop_detail {
template <typename Mem, typename Ret, typename... Args>
struct extract_memfn_types<Ret (Mem::*)(Args...)> {
  using type = Mem;
  using ret = Ret;
  static constexpr bool is_noexcept_v = false;
};

template <typename Mem, typename Ret, typename... Args>
struct extract_memfn_types<Ret (Mem::*)(Args...) const> {
  using type = Mem;
  using ret = Ret;
  static constexpr bool is_noexcept_v = false;
};

template <typename Mem, typename Ret, typename... Args>
struct extract_memfn_types<Ret (Mem::*)(Args...) noexcept> {
  using type = Mem;
  using ret = Ret;
  static constexpr bool is_noexcept_v = true;
};

template <typename Mem, typename Ret, typename... Args>
struct extract_memfn_types<Ret (Mem::*)(Args...) const noexcept> {
  using type = Mem;
  using ret = Ret;
  static constexpr bool is_noexcept_v = true;
};

class property_offset {
public:
  explicit property_offset(const void *obj) {
    // rather than store the "this" pointer, we store the offset
    // this allows us to copy/move the property_offset class trivially
    // without invoking copy/move semantics
    const auto member_ptr = reinterpret_cast<const std::byte *>(this);
    const auto obj_ptr = reinterpret_cast<const std::byte *>(obj);
    offset_ = member_ptr - obj_ptr;
  }

private:
  int offset_;
};
} // namespace prop_detail

/* Property implementation */
template <typename T, auto GetFn, auto SetFn>
T property<T, GetFn, SetFn>::get() const noexcept(GetFnInfo::is_noexcept_v) {
  return (getter_this()->*GetFn)();
}

template <typename T, auto GetFn, auto SetFn>
property<T, GetFn, SetFn>::operator T() const
    noexcept(GetFnInfo::is_noexcept_v) {
  return get();
}

template <typename T, auto GetFn, auto SetFn>
void property<T, GetFn, SetFn>::set(const T &val) noexcept(
    SetFnInfo::is_noexcept_v) {
  (setter_this()->*SetFn)(val);
}

template <typename T, auto GetFn, auto SetFn>
property<T, GetFn, SetFn> &property<T, GetFn, SetFn>::operator=(
    const T &val) noexcept(SetFnInfo::is_noexcept_v) {
  set(val);
  return *this;
}

template <typename T, auto GetFn, auto SetFn>
template <typename T2, auto GetFn2, auto SetFn2>
property<T, GetFn, SetFn> &property<T, GetFn, SetFn>::operator=(
    const property<T2, GetFn2, SetFn2>
        &rhs) noexcept(SetFnInfo::is_noexcept_v &&noexcept(rhs.get())) {
  (setter_this()->*SetFn)(rhs.get());
  return *this;
}

template <typename T, auto GetFn, auto SetFn>
const typename property<T, GetFn, SetFn>::GetT *
property<T, GetFn, SetFn>::getter_this() const noexcept {
  const auto member_ptr = reinterpret_cast<const std::byte *>(this);
  return reinterpret_cast<const GetT *>(member_ptr - offset_);
}

template <typename T, auto GetFn, auto SetFn>
typename property<T, GetFn, SetFn>::SetT *
property<T, GetFn, SetFn>::setter_this() noexcept {
  const auto member_ptr = reinterpret_cast<std::byte *>(this);
  return reinterpret_cast<SetT *>(member_ptr - offset_);
}

template <typename T, auto GetFn, auto SetFn>
std::ostream &operator<<(std::ostream &os, const property<T, GetFn, SetFn> &p) {
  return os << p.get();
}

/* Implementation for property with no setter */
template <typename T, auto GetFn>
property<T, GetFn, nullptr>::operator T() const noexcept(noexcept(get())) {
  return get();
}

template <typename T, auto GetFn>
T property<T, GetFn, nullptr>::get() const noexcept(GetFnInfo::is_noexcept_v) {
  return (getter_this()->*GetFn)();
}

template <typename T, auto GetFn>
const typename property<T, GetFn, nullptr>::GetT *
property<T, GetFn, nullptr>::getter_this() const noexcept {
  const auto member_ptr = reinterpret_cast<const std::byte *>(this);
  return reinterpret_cast<const GetT *>(member_ptr - offset_);
}
} // namespace fcp

#endif