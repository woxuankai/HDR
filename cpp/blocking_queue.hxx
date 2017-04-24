#ifndef __MY_BLOCKING_QUEUE
#define __MY_BLOCKING_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

template<class T>
class blocking_queue{//: protected std::queue{//too many functions to implement
public:
  typedef typename std::queue<T>::size_type size_type;
  blocking_queue();
  ~blocking_queue();
  size_type put(const T &obj); // return queue length after put
  size_type get(T &obj); // return queue length after get
protected:
  std::mutex mtx_; // queue mutex
  std::queue<T> queue_;
  std::condition_variable cond_var_;
};

template<class T>
inline blocking_queue<T>::~blocking_queue()
{
}

template<class T>
inline blocking_queue<T>::blocking_queue()
{
};

template<class T>
inline typename blocking_queue<T>::size_type blocking_queue<T>::get(T &obj)
{ // return queue length after get
  std::unique_lock<std::mutex> lck(mtx_);
  //while(queue_.empty()) cond_var_.wait(lck);
  cond_var_.wait(lck,[this]()->bool {return !queue_.empty(); });
  obj = queue_.front();
  queue_.pop();
  return queue_.size();
}

template<class T>
inline typename blocking_queue<T>::size_type \
    blocking_queue<T>::put(const T &obj)
{ // return queue length after put
  std::unique_lock<std::mutex> lck(mtx_);
  queue_.push(obj);
  cond_var_.notify_all();
  return queue_.size(); // the mutex is still locked now
}

#endif //__MY_BLOCKING_QUEUE
