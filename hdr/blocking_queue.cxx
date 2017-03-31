#include "blocking_queue.hxx"

template<typename Alloc>
blocking_queue<Alloc>::blocking_queue(){
}

template<typename Alloc>
blocking_queue<Alloc>::~blocking_queue(){
}

template<typename Alloc>
typename blocking_queue<Alloc>::size_type \
    blocking_queue<Alloc>::put(const Alloc &obj){
  std::unique_lock<std::mutex> lck(mtx_);
  queue_.push(obj);
  cond_var_.notify_all();
  return queue_.size(); // the mutex is still locked now
} 

template<typename Alloc>
typename blocking_queue<Alloc>::size_type \
    blocking_queue<Alloc>::get(Alloc &obj){
  std::unique_lock<std::mutex> lck(mtx_);
  while(queue_.empty()) cond_var_.wait(lck);
  obj = queue_.front();
  queue_.pop();
  return queue_.size();
}
