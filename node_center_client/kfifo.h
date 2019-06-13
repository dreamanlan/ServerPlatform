#ifndef __KFIFO_H__
#define __KFIFO_H__

#include <assert.h>
#include <memory.h>

//注意：T需要是可以直接进行内存拷贝的类型，不能是带有虚函数的类（可以是这样的类的指针）
template <typename T>
class kfifo
{
public:
  inline kfifo(unsigned int size);
  inline ~kfifo();

  inline void push(const T&);
  inline const T& pop();
  inline bool full() const;
  inline bool empty() const;

  inline bool put(const T* ptr, unsigned int count);
  inline bool get(T* ptr, unsigned int count);
  inline const T* in_place_get(const T*& start, unsigned int& sizeToWrapAround) const;
  inline void in_place_get_advance(unsigned int count);
  inline unsigned int length() const { return in_ - out_; }
  inline unsigned int space() const { return size_ - in_ + out_; }
private:
  kfifo(const kfifo& rhs);
  kfifo& operator=(const kfifo& rhs);

  T *elements_;
  unsigned int size_;
  unsigned int in_, out_;
};

static inline void roundup_power_2(unsigned int& size)
{
  unsigned int n = 2;
  while (n < size) n *= 2;
  size = n;
}

template <typename T> inline
kfifo<T>::kfifo(unsigned int size)
  : elements_(0), size_(size), in_(0), out_(0)
{
  if (size & (size - 1))
    roundup_power_2(size_);
  elements_ = new T[size_];
}

template <typename T> inline
kfifo<T>::~kfifo()
{
  delete[] elements_;
}

template <typename T> inline
void kfifo<T>::push(const T& t)
{
  elements_[in_++ % size_] = t;
}

template <typename T> inline
const T& kfifo<T>::pop()
{
  const T& t = elements_[out_++ % size_];
  if (out_ == in_) in_ = out_ = 0;
  return t;
}

template <typename T> inline
bool kfifo<T>::full() const
{
  return length() >= size_;
}

template <typename T> inline
bool kfifo<T>::empty() const
{
  return length() == 0;
}

template <typename T> inline
bool kfifo<T>::put(const T* ptr, unsigned int count)
{
  if (count <= space()){
    unsigned int ix = (in_ & (size_ - 1));
    unsigned int leftSpace = size_ - ix;
    if (count <= leftSpace){
      memcpy(reinterpret_cast<void*>(elements_ + ix), reinterpret_cast<const void*>(ptr), count*sizeof(T));
    } else {
      memcpy(reinterpret_cast<void*>(elements_ + ix), reinterpret_cast<const void*>(ptr), leftSpace*sizeof(T));
      memcpy(reinterpret_cast<void*>(elements_), reinterpret_cast<const void*>(ptr + leftSpace), (count - leftSpace)*sizeof(T));
    }
    in_ += count;
    return true;
  }
  return false;
}

template <typename T> inline
bool kfifo<T>::get(T* ptr, unsigned int count)
{
  if (count <= length()){
    unsigned int ix = (out_ & (size_ - 1));
    unsigned int leftSpace = size_ - ix;
    if (count <= leftSpace){
      memcpy(reinterpret_cast<void*>(ptr), reinterpret_cast<const void*>(elements_ + ix), count*sizeof(T));
    } else {
      memcpy(reinterpret_cast<void*>(ptr), reinterpret_cast<const void*>(elements_ + ix), leftSpace*sizeof(T));
      memcpy(reinterpret_cast<void*>(ptr + leftSpace), reinterpret_cast<const void*>(elements_), (count - leftSpace)*sizeof(T));
    }
    out_ += count;
    return true;
  }
  return false;
}

template <typename T> inline
const T* kfifo<T>::in_place_get(const T*& start, unsigned int& sizeToWrapAround) const
{
  unsigned int ix = (out_ & (size_ - 1));
  sizeToWrapAround = size_ - ix;
  start = elements_ + ix;
  return elements_;
}

template <typename T> inline
void kfifo<T>::in_place_get_advance(unsigned int count)
{
  out_ += count;
}

#endif //__KFIFO_H__