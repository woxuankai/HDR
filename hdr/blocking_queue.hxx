#ifndef __MY_BLOCKING_QUEUE
#define __MY_BLOCKING_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

template<class Alloc>
class blocking_queue{//: protected std::queue{//too many functions to implement
public:
  typedef typename std::queue<Alloc>::size_type size_type;
  blocking_queue(){};
  ~blocking_queue(){};
  size_type put(const Alloc &obj){ // return queue length after put
    std::unique_lock<std::mutex> lck(mtx_);
    queue_.push(obj);
    cond_var_.notify_all();
    return queue_.size(); // the mutex is still locked now
  }
  size_type get(Alloc &obj){ // return queue length after get
    std::unique_lock<std::mutex> lck(mtx_);
    while(queue_.empty()) cond_var_.wait(lck);
    obj = queue_.front();
    queue_.pop();
    return queue_.size();
  }
protected:
  std::mutex mtx_; // queue mutex
  std::queue<Alloc> queue_;
  std::condition_variable cond_var_;
};


#endif //__MY_BLOCKING_QUEUE
