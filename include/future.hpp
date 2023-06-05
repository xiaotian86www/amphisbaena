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
  Future(Schedule* sch)
    : sch_(sch)
    , co_(sch_->this_co())
  {
  }

  Future(const Future<Tp_>&) = delete;
  Future<Tp_>& operator=(const Future<Tp_>&) = delete;

public:
  Tp_&& get()
  {
    sch_->yield();
    return std::move(value_);
  }

private:
  Tp_ value_;
  Schedule* sch_;
  std::weak_ptr<Schedule::Coroutine> co_;
};

template<typename Tp_>
class Promise
{
public:
  Promise(Schedule* sch)
    : ftr_(sch)
  {
  }

  Promise(const Promise<Tp_>&) = delete;
  Promise<Tp_>& operator=(const Promise<Tp_>&) = delete;

public:
  template<typename ValueTp_>
  void set(ValueTp_&& value)
  {
    ftr_.value_ = std::forward<ValueTp_>(value);

    ftr_.sch_->resume(ftr_.co_);
  }

  Future<Tp_>& future() { return ftr_; }

private:
  Future<Tp_> ftr_;
};

} // namespace translator
