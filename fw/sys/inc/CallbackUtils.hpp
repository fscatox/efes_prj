/**
 * @file     CallbackUtils.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     23.01.2025
 * @brief    Non-owning wrapper to (member) function pointer
 */

#ifndef CALLBACKUTILS_HPP
#define CALLBACKUTILS_HPP

template<typename Fn>
struct ICallback;

template<typename R, typename ...Args>
struct ICallback<R(Args...)> {
  using FnType = R(Args...);

  virtual ~ICallback() = default;

  virtual bool empty() const = 0;
  explicit operator bool() const { return !empty(); }
  virtual R operator()(Args... args) const = 0;
};

template<typename Fn>
struct FnCallback;

template<typename R, typename ...Args>
struct FnCallback<R(Args...)> : ICallback<R(Args...)> {
  using Fn = R(*)(Args...);
  explicit FnCallback(Fn fn = nullptr) : _fn(fn) {}
  bool empty() const override { return !_fn; }
  R operator()(Args... args) const override { return (*_fn)(args...); }

private:
  Fn _fn;
};

template<typename R, typename ...Args>
FnCallback(R(*)(Args...)) -> FnCallback<R(Args...)>;

template<typename C, typename MemFn>
struct MemFnCallback;

template<typename C, typename R, typename ...Args>
struct MemFnCallback<C, R(Args...)> : ICallback<R(Args...)> {
  using MemFn = R(C::*)(Args...);
  explicit MemFnCallback(C *that = nullptr, MemFn mem_fn = nullptr)
    : _that(that), _mem_fn(mem_fn) {}
  bool empty() const override { return !_that || !_mem_fn; }
  R operator()(Args... args) const override { return (_that->*_mem_fn)(args...); }

private:
  C *_that;
  MemFn _mem_fn;
};

template<typename C, typename R, typename ...Args>
MemFnCallback(C*, R(C::*)(Args...)) -> MemFnCallback<C, R(Args...)>;

#endif // CALLBACKUTILS_HPP
