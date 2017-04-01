#include "blocking_queue.hxx"

template<typename T>
blocking_queue<T>::blocking_queue(){
}

template<typename T>
blocking_queue<T>::~blocking_queue(){
}

template<typename T>
typename blocking_queue<T>::size_type \
    blocking_queue<T>::put(const T &obj){
  std::unique_lock<std::mutex> lck(mtx_);
  queue_.push(obj);
  cond_var_.notify_all();
  return queue_.size(); // the mutex is still locked now
} 

template<typename T>
typename blocking_queue<T>::size_type \
    blocking_queue<T>::get(T &obj){
  std::unique_lock<std::mutex> lck(mtx_);
  while(queue_.empty()) cond_var_.wait(lck);
  obj = queue_.front();
  queue_.pop();
  return queue_.size();
}


//template<typename T>
//std::reference_wrapper<blocking_queue<cv::Mat>> 
