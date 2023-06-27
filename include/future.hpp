#pragma once

#include <mutex>
#include <optional>

#include "schedule.hpp"

namespace translator {
template<typename Tp_>
class Promise;

template<typename Tp_>
class Future
{
  friend class Promise<Tp_>;

private:
  Future(Promise<Tp_>& pms, ScheduleRef sch)
    : pms_(pms)
    , sch_(sch)
  {
  }

  Future(const Future<Tp_>&) = delete;
  Future<Tp_>& operator=(const Future<Tp_>&) = delete;

public:
  Tp_&& get() &&
  {
    sch_.yield();
    return std::move(pms_.value().value());
  }

  const Tp_& get() const&
  {
    if (!has_get_) {
      sch_.yield();
      value_ = std::move(pms_.value());
      has_get_ = true;
    }
    return value_.value();
  }

  Tp_&& get_for(int milli,
                Tp_&& default_value) &&
  {
    sch_.yield_for(milli);
    return std::move(pms_.value().value_or(std::forward<Tp_>(default_value)));
  }

  const Tp_& get_for(int milli,
                     Tp_&& default_value) const&
  {
    if (!has_get_) {
      sch_.yield_for(milli);
      value_ = std::move(pms_.value());
      has_get_ = true;
    }

    return pms_.value().value_or(std::forward<Tp_>(default_value));
  }

private:
  Promise<Tp_>& pms_;
  mutable ScheduleRef sch_;
  mutable std::optional<Tp_> value_;
  mutable bool has_get_ = false;
};

template<typename Tp_>
class Promise
{
  friend class Future<Tp_>;

public:
  Promise(ScheduleRef sch)
    : sch_(sch)
    , co_(sch_.this_co())
  {
  }

  Promise(const Promise<Tp_>&) = delete;
  Promise<Tp_>& operator=(const Promise<Tp_>&) = delete;

public:
  template<typename ValueTp_>
  void set(ValueTp_&& value)
  {
    std::lock_guard<std::mutex> lg(mtx_);
    value_ = std::forward<ValueTp_>(value);

    sch_.resume(co_);
  }

  Future<Tp_> future() { return Future<Tp_>(*this, sch_); }

private:
  std::optional<Tp_>&& value()
  {
    std::lock_guard<std::mutex> lg(mtx_);
    return std::move(value_);
  }

private:
  ScheduleRef sch_;
  std::weak_ptr<Schedule::Coroutine> co_;
  std::mutex mtx_;
  std::optional<Tp_> value_;
};

} // namespace translator
