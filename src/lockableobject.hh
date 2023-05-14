#pragma once

#include <mutex>
#include <vector>
#include <set>
#include <map>
#include <functional>

namespace sforce
{
  template<class T>
  class LockableObject
  {
  public:
    class WithLock
    {
    public:
      WithLock(LockableObject<T> *lkobj)
        : lkobj_(lkobj)
      {
        lkobj_->mx_.lock();
      }

      ~WithLock()
      {
        lkobj_->mx_.unlock();
      }

      T& operator*() { return lkobj_->obj_; }
      T* operator->() { return &lkobj_->obj_; }

      WithLock(const WithLock &) = delete;
      WithLock(WithLock &&) = delete;

      WithLock& operator=(const WithLock &) = delete;
      WithLock& operator=(WithLock &&) = delete;
    
    private:
      LockableObject<T> *lkobj_;
    };

    WithLock withLock()
    {
      return WithLock(this);
    }

    T& operator*() { return obj_; }
    T* operator->() { return &obj_; }

    const T& operator*() const { return obj_; }
    const T* operator->() const { return &obj_; }

    std::mutex& mx() { return mx_; }

  private:
    T obj_;
    std::mutex mx_;
  };

  template<typename T>
  using LockableVector = LockableObject<std::vector<T>>;

  template<typename Key, typename Cmp = std::less<Key>>
  using LockableSet = LockableObject<std::set<Key, Cmp>>;

  template<typename Key, typename Val, typename Cmp = std::less<Key>>
  using LockableMap = LockableObject<std::map<Key, Val, Cmp>>;
}
