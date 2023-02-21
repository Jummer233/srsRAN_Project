/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "detail/base_task.h"

namespace srsran {

/// Lazy awaitable coroutine type that outputs a result of type R when completed.
/// \tparam R Result of the task
template <typename R>
class async_task : public detail::common_task_crtp<async_task<R>, R>
{
public:
  using result_type = R;

  struct promise_type : public detail::promise_data<result_type, detail::task_promise_base> {
    struct final_awaiter {
      /// Lifetime of coroutine is bounded to the lazy_task object.
      bool await_ready() const { return false; }

      /// \brief Tail-resumes suspending/awaiting coroutine continuation.
      /// Lazy tasks always have a continuation, if they went beyond the initial suspension point.
      coro_handle<> await_suspend(coro_handle<promise_type> cb) { return cb.promise().continuation; }

      void await_resume() {}

      /// Points to itself as an awaiter
      final_awaiter& get_awaiter() { return *this; }
    };

    /// Initial suspension awaiter. Lazy_tasks always suspend at initial suspension point.
    suspend_always initial_suspend() { return {}; }

    /// Final suspension awaiter. Tail-resumes continuation.
    final_awaiter final_suspend() { return {}; }

    async_task<R> get_return_object()
    {
      auto corohandle = coro_handle<promise_type>::from_promise(this);
      corohandle.resume();
      return async_task<R>{corohandle};
    }
  };

  async_task() = default;
  explicit async_task(coro_handle<promise_type> cb) : handle(cb) {}

  /// Retrieve awaiter interface.
  async_task<R>& get_awaiter() { return *this; }

  /// \brief Register suspending coroutine as continuation of the current lazy_task. Tail-resumes "this" lazy_task.
  /// Called solely when "this" lazy_task is at initial suspension point.
  /// \param h suspending coroutine that is calling await_suspend.
  /// \return Coroutine handle to tail-resume.
  coro_handle<> await_suspend(coro_handle<> h) noexcept
  {
    srsgnb_sanity_check(not this->empty(), "Awaiting an empty task");
    srsgnb_sanity_check(handle.promise().continuation.empty(), "Lazy task can only be awaited once.");

    // Store continuation in promise, so that it gets called at "this" coroutine final suspension point.
    handle.promise().continuation = h;

    // We tail-resume the current awaiter task's coroutine, which is currently suspended at initial suspension point.
    return *handle;
  }

private:
  friend class detail::common_task_crtp<async_task<R>, R>;

  unique_coroutine<promise_type> handle;
};

} // namespace srsran