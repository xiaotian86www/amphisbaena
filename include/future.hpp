#pragma once

#include <memory>
#include <mutex>
#include <optional>

#include "schedule.hpp"

namespace translator {
namespace detail {
template<typename Tp_>
class Future
{
public:
  Future(ScheduleRef sch, CoroutineRef co)
    : sch_(sch)
    , co_(co)
  {
  }

  Future(const Future<Tp_>&) = delete;
  Future<Tp_>& operator=(const Future<Tp_>&) = delete;

public:
  Tp_&& get()
  {
    co_.yield();
    std::lock_guard<std::mutex> lg(mtx_);
    return std::move(value_.value());
  }

  Tp_&& get_for(int milli, Tp_&& default_value)
  {
    co_.yield_for(milli);
    std::lock_guard<std::mutex> lg(mtx_);
    return std::move(value_.value_or(std::move(default_value)));
  }

  template<typename ValueTp_>
  void set(ValueTp_&& value)
  {
    {
      std::lock_guard<std::mutex> lg(mtx_);
      value_ = std::forward<ValueTp_>(value);
    }

    sch_.resume(co_);
  }

private:
  ScheduleRef sch_;
  CoroutineRef co_;
  std::mutex mtx_;
  std::optional<Tp_> value_;
};
}

template<typename Tp_>
class Future
{
public:
  Future(std::shared_ptr<detail::Future<Tp_>> ftr)
    : ftr_(ftr)
  {
  }

  Future(const Future<Tp_>&) = delete;
  Future<Tp_>& operator=(const Future<Tp_>&) = delete;

public:
  Tp_&& get() { return std::move(ftr_->get()); }

  Tp_&& get_for(int milli, Tp_&& default_value)
  {
    return std::move(ftr_->get_for(milli, std::move(default_value)));
  }

private:
  std::shared_ptr<detail::Future<Tp_>> ftr_;
};

template<typename Tp_>
class Promise
{
public:
  Promise(ScheduleRef sch, CoroutineRef co)
  {
    ftr_ = std::make_shared<detail::Future<Tp_>>(sch, co);
  }

  Promise(Promise<Tp_>&& other)
    : ftr_(std::move(other.ftr_))
  {
  }

  Promise<Tp_>& operator=(Promise<Tp_>&& other)
  {
    if (this == &other)
      return *this;

    ftr_ = std::move(other.ftr_);

    return *this;
  }

  Promise(const Promise<Tp_>&) = delete;
  Promise<Tp_>& operator=(const Promise<Tp_>&) = delete;

public:
  template<typename ValueTp_>
  void set(ValueTp_&& value)
  {
    ftr_->set(std::move(value));
  }

  Future<Tp_> future() { return Future<Tp_>(ftr_); }

private:
  std::shared_ptr<detail::Future<Tp_>> ftr_;
};

} // namespace translator
