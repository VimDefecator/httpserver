#pragma once

#include <memory>
#include <functional>

class TaskPool
{
public:
  TaskPool(int numThreads);
  ~TaskPool();

  void push(std::function<void()> task);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

