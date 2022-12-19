#ifndef CLASSUTILS_HH
#define CLASSUTILS_HH

template<class Derived>
class EnableMoveGetter
{
public:
  Derived &&move() { return std::move(*(Derived *)this); }
};

#endif
