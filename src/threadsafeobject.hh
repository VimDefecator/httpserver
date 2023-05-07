#pragma once

#include <mutex>
#include <shared_mutex>

template<class T>
class ThreadSafeObject
{
public:
  template<bool Shared>
  class WithLock
  {
  public:
    WithLock(ThreadSafeObject<T> *tsobj)
      : tsobj_(tsobj)
    {
      if(Shared)
        tsobj_->mx_.lock_shared();
      else
        tsobj_->mx_.lock();
    }

    ~WithLock()
    {
      if(Shared)
        tsobj_->mx_.unlock_shared();
      else
        tsobj_->mx_.unlock();
    }

    T& operator*() { return tsobj_->obj_; }
    T* operator->() { return &tsobj_->obj_; }

    WithLock(const WithLock &other) = delete;
    WithLock(WithLock &&other) = delete;

    WithLock& operator=(const WithLock &other) = delete;
    WithLock& operator=(WithLock &&other) = delete;
  
  private:
    ThreadSafeObject<T> *tsobj_;
  };

  using WithUniqueLock = WithLock<false>;
  using WithSharedLock = WithLock<true>;

  WithUniqueLock withLock()
  {
    return WithUniqueLock(this);
  }

  WithSharedLock withSharedLock()
  {
    return WithSharedLock(this);
  }

  T& operator*() { return obj_; }
  T* operator->() { return &obj_; }

  std::shared_mutex& mx() { return mx_; }

private:
  T obj_;
  std::shared_mutex mx_;
};

