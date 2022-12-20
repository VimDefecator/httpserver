#ifndef CLASSUTILS_HH
#define CLASSUTILS_HH

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

#define CHAINMETHOD(chainMethodName, methodName) \
  template<typename... Args> \
  auto &chainMethodName(Args&&... args) & { \
    methodName(std::forward<Args>(args)...); \
    return *this; \
  } \
  template<typename... Args> \
  auto &&chainMethodName(Args&&... args) && { \
    methodName(std::forward<Args>(args)...); \
    return std::move(*this); \
  }

#endif
