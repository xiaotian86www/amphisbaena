#include "schedule.hpp"

#include "detail/schedule_impl.hpp"

namespace translator {

Schedule::Worker::Worker(Schedule& sch)
  : sch_(sch)
{
  sch_.impl_->increase();
}

Schedule::Worker::~Worker()
{
  sch_.impl_->decrease();
}

Schedule::Schedule()
  : impl_(std::make_shared<Impl>(this))
{
}

Schedule::~Schedule() {}

void
Schedule::run()
{
  impl_->run();
}

void
Schedule::stop()
{
  impl_->stop();
}

void
Schedule::post(task&& func)
{
  impl_->post(std::move(func));
}

void
Schedule::yield()
{
  impl_->yield();
}

void
Schedule::resume(CoroutinePtr co)
{
  impl_->resume(co);
}

void
Schedule::yield_for_(const timespec& rtime)
{
  impl_->yield_for(rtime);
}

Schedule::CoroutinePtr
Schedule::this_co()
{
  return impl_->this_co();
}

ScheduleRef::ScheduleRef(std::weak_ptr<Schedule::Impl> impl)
  : ptr_(impl)
{
}

void
ScheduleRef::stop()
{
  auto impl = ptr_.lock();
  if (impl)
    impl->stop();
}

void
ScheduleRef::post(task&& func)
{
  auto impl = ptr_.lock();
  if (impl)
    impl->post(std::move(func));
}

void
ScheduleRef::resume(Schedule::CoroutinePtr co)
{
  auto impl = ptr_.lock();
  if (impl)
    impl->resume(co);
}

void
ScheduleRef::yield()
{
  auto impl = ptr_.lock();
  if (impl)
    impl->yield();
}

void
ScheduleRef::yield_for_(const timespec& rtime)
{
  auto impl = ptr_.lock();
  if (impl)
    impl->yield_for(rtime);
}

Schedule::CoroutinePtr
ScheduleRef::this_co()
{
  auto impl = ptr_.lock();
  if (impl)
    return impl->this_co();
  else
    return Schedule::CoroutinePtr();
}

} // namespace translator
