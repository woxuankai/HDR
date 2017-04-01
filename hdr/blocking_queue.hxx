#ifndef __MY_BLOCKING_QUEUE
#define __MY_BLOCKING_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

template<typename Alloc>
class blocking_queue{//: protected std::queue{//too many functions to implement
public:
  typedef typename std::queue<Alloc>::size_type size_type;
  blocking_queue();
  ~blocking_queue();
  size_type put(const Alloc &); // return queue length after put
  size_type get(Alloc &); // return queue length after get
protected:
  std::mutex mtx_; // queue mutex
  std::queue<Alloc> queue_;
  std::condition_variable cond_var_;
};


#endif //__MY_BLOCKING_QUEUE
