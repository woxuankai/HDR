#ifndef __MY_BLOCKING_QUEUE
#define __MY_BLOCKING_QUEUE

#include <boost/circular_buffer.hpp>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

template<class T>
class blocking_queue{//: protected std::queue{//too many functions to implement
public:
  typedef typename boost::circular_buffer<T>::size_type size_type;
  blocking_queue(){}
  blocking_queue(int capacity){
    queue_.set_capacity(capacity);
  }
  ~blocking_queue(){};
  size_type put(const T &obj){ // return queue length after put
    std::unique_lock<std::mutex> lck(mtx_);
    queue_.push_back(obj);
    cond_var_.notify_all();
    return queue_.size(); // the mutex is still locked now
  }
  size_type get(T &obj){ // return queue length after get
    std::unique_lock<std::mutex> lck(mtx_);
    //while(queue_.empty()) cond_var_.wait(lck);
    cond_var_.wait(lck,[this]()->bool {return !queue_.empty(); });
    obj = queue_.front();
    queue_.pop_front();
    return queue_.size();
  }
protected:
  std::mutex mtx_; // queue mutex
  boost::circular_buffer<T> queue_;
  std::condition_variable cond_var_;
};


#endif //__MY_BLOCKING_QUEUE
