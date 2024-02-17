#pragma once

template<class Derived>
class EnableMoveGetter
{
public:
  Derived &&move() { return std::move(*(Derived *)this); }
};

template<class Derived>
class EnableCopyGetter
{
public:
  const Derived &copy() { return *(Derived *)this; }
};

#define CHAIN_METHOD(thisType, chainMethodName, methodName) \
  template<typename... Args>                                \
  thisType &chainMethodName(Args&&... args) & {             \
    methodName(std::forward<Args>(args)...);                \
    return *(thisType *)this;                               \
  }                                                         \
  template<typename... Args>                                \
  thisType &&chainMethodName(Args&&... args) && {           \
    methodName(std::forward<Args>(args)...);                \
    return std::move(*(thisType *)this);                    \
  }

#define CHAIN_METHOD_AUTO(chainMethodName, methodName)  \
  template<typename... Args>                            \
  auto &chainMethodName(Args&&... args) & {             \
    methodName(std::forward<Args>(args)...);            \
    return *this;                                       \
  }                                                     \
  template<typename... Args>                            \
  auto &&chainMethodName(Args&&... args) && {           \
    methodName(std::forward<Args>(args)...);            \
    return std::move(*this);                            \
  }

#define CHAIN_METHOD_FOR_TYPE(thisType, chainMethodName, methodName, type)  \
  thisType &chainMethodName(const type &arg) & {                            \
    methodName(arg);                                                        \
    return *(thisType *)this;                                               \
  }                                                                         \
  thisType &chainMethodName(type &&arg) & {                                 \
    methodName(std::move(arg));                                             \
    return *(thisType *)this;                                               \
  }                                                                         \
  thisType &&chainMethodName(const type &arg) && {                          \
    methodName(arg);                                                        \
    return std::move(*(thisType *)this);                                    \
  }                                                                         \
  thisType &&chainMethodName(type &&arg) && {                               \
    methodName(std::move(arg));                                             \
    return std::move(*(thisType *)this);                                    \
  }

#define CHAIN_METHOD_FOR_TYPE_AUTO(chainMethodName, methodName, type) \
  auto &chainMethodName(const type &arg) & {                          \
    methodName(arg);                                                  \
    return *this;                                                     \
  }                                                                   \
  auto &chainMethodName(type &&arg) & {                               \
    methodName(std::move(arg));                                       \
    return *this;                                                     \
  }                                                                   \
  auto &&chainMethodName(const type &arg) && {                        \
    methodName(arg);                                                  \
    return std::move(*this);                                          \
  }                                                                   \
  auto &&chainMethodName(type &&arg) && {                             \
    methodName(std::move(arg));                                       \
    return std::move(*this);                                          \
  }

#define CHAIN_METHOD_FOR_TYPE_NO_ARG(thisType, chainMethodName, methodName, type) \
  struct type {};                                                                 \
  void methodName(type) { methodName(); }                                         \
  CHAIN_METHOD_FOR_TYPE(thisType, chainMethodName, methodName, type)

#define CHAIN_METHOD_FOR_TYPE_NO_ARG_AUTO(chainMethodName, methodName, type)  \
  struct type {};                                                             \
  void methodName(type) { methodName(); }                                     \
  CHAIN_METHOD_FOR_TYPE_AUTO(chainMethodName, methodName, type)

