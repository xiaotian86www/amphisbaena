#pragma once

#include "schedule.hpp"

namespace translator {
template<typename Tp_>
class Promise;

template<typename Tp_>
class Future
{
  friend class Promise<Tp_>;

public:
  Future() = default;
  Future(const Future<Tp_>&) = delete;
  Future<Tp_>& operator=(const Future<Tp_>&) = delete;

public:
  Tp_&& get()
  {
    Schedule::yield();
    return std::move(value_);
  }

private:
  Tp_ value_;
  Schedule* sch_ = Schedule::this_sch();
  std::weak_ptr<Schedule::Coroutine> co_{ Schedule::this_co() };
};

template<typename Tp_>
class Promise
{
public:
  Promise() = default;
  Promise(const Promise<Tp_>&) = delete;
  Promise<Tp_>& operator=(const Promise<Tp_>&) = delete;

public:
  template<typename ValueTp_>
  void set(ValueTp_&& value)
  {
    ft_.value_ = std::forward<ValueTp_>(value);

    ft_.sch_->resume(ft_.co_);
  }

  Future<Tp_>& future() { return ft_; }

private:
  Future<Tp_> ft_;
};

} // namespace translator
