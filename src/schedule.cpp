#include "schedule.hpp"

#include "detail/asio_schedule.hpp"
#include <memory>

namespace translator {

Schedule::Schedule()
  : impl_(std::make_shared<Impl>())
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

// void
// Schedule::resume(std::weak_ptr<Coroutine> co)
// {
//   impl_->resume(co);
// }

// void
// Schedule::yield()
// {
//   impl_->yield();
// }

// void
// Schedule::yield_for(int milli)
// {
//   impl_->yield_for(milli);
// }

// std::weak_ptr<Coroutine>
// Schedule::this_co()
// {
//   return impl_->this_co();
// }

// void
// ScheduleRef::stop()
// {
//   auto impl = ptr_.lock();
//   if (impl)
//     impl->stop();
// }

// void
// ScheduleRef::post(task&& func)
// {
//   auto impl = ptr_.lock();
//   if (impl)
//     impl->post(std::move(func));
// }

// void
// ScheduleRef::resume(std::weak_ptr<Coroutine> co)
// {
//   auto impl = ptr_.lock();
//   if (impl)
//     impl->resume(co);
// }

// void
// ScheduleRef::yield()
// {
//   auto impl = ptr_.lock();
//   if (impl)
//     impl->yield();
// }

// void
// ScheduleRef::yield_for(int milli)
// {
//   auto impl = ptr_.lock();
//   if (impl)
//     impl->yield_for(milli);
// }

// std::weak_ptr<Coroutine>
// ScheduleRef::this_co()
// {
//   auto impl = ptr_.lock();
//   if (impl)
//     return impl->this_co();
//   else
//     return std::weak_ptr<Coroutine>();
// }

} // namespace translator
