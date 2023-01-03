#include "taskpool.hh"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

struct TaskPool::Impl
{
  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mx_;
  std::condition_variable cond_;
  bool stop_ = false;

  void push(std::function<void()> task);
  std::function<void()> pop();
};

void TaskPool::Impl::push(std::function<void()> task)
{
  {
    std::lock_guard _(mx_);
    tasks_.push(std::move(task));
  }
  cond_.notify_one();
}

std::function<void()> TaskPool::Impl::pop()
{
  std::unique_lock lk(mx_);

  while(!stop_ && tasks_.empty())
    cond_.wait(lk);

  if(stop_)
    return {};

  auto task = std::move(tasks_.front());
  tasks_.pop();
  return task;
}

TaskPool::TaskPool(int numThreads)
{
  impl_.reset(new Impl);

  for(int i = 0; i < numThreads; i++)
  {
    impl_->threads_.emplace_back([impl = impl_.get()]
    {
      for(;;)
      {
        if(auto task = impl->pop())
          task();
        else
          break;
      }
    });
  }
}

TaskPool::~TaskPool()
{
  impl_->stop_ = true;
  impl_->cond_.notify_all();
  for(auto &t : impl_->threads_)
    t.join();
}

void TaskPool::push(std::function<void()> task)
{
  impl_->push(std::move(task));
}
