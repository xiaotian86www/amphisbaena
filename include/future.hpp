#pragma once

#include <memory>
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
  Future(Promise<Tp_>& pms)
    : pms_(pms)
  {
  }

  Future(const Future<Tp_>&) = delete;
  Future<Tp_>& operator=(const Future<Tp_>&) = delete;

public:
  Tp_&& get() &&
  {
    pms_.co_.yield();
    return std::move(pms_.value().value());
  }

  const Tp_& get() &
  {
    if (!has_get_) {
      pms_.co_.yield();
      has_get_ = true;
    }
    return pms_.value().value();
  }

  Tp_&& get_for(int milli,
                Tp_&& default_value) &&
  {
    pms_.co_.yield_for(milli);
    return std::move(pms_.value().value_or(std::move(default_value)));
  }

  const Tp_& get_for(int milli,
                     const Tp_& default_value) &
  {
    if (!has_get_) {
      pms_.co_.yield_for(milli);
      has_get_ = true;
    }

    return pms_.value().value_or(default_value);
  }

private:
  Promise<Tp_>& pms_;
  bool has_get_ = false;
};

template<typename Tp_>
class Promise
{
  friend class Future<Tp_>;

public:
  Promise(CoroutineRef co)
    : co_(co)
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

    co_.resume();
  }

  Future<Tp_> future() { return Future<Tp_>(*this); }

private:
  std::optional<Tp_>&& value()
  {
    std::lock_guard<std::mutex> lg(mtx_);
    return std::move(value_);
  }

private:
  // std::weak_ptr<Schedule> sch_;
  CoroutineRef co_;
  std::mutex mtx_;
  std::optional<Tp_> value_;
};

} // namespace translator
