#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

template<class Alloc>
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







template<typename T>
blocking_queue<T>::blocking_queue(){
}

template<typename T>
blocking_queue<T>::~blocking_queue(){
}

template<typename T>
typename blocking_queue<T>::size_type     blocking_queue<T>::put(const T &obj){
  std::unique_lock<std::mutex> lck(mtx_);
  queue_.push(obj);
  cond_var_.notify_all();
  return queue_.size(); // the mutex is still locked now
}


template<typename T>
typename blocking_queue<T>::size_type     blocking_queue<T>::get(T &obj){
  std::unique_lock<std::mutex> lck(mtx_);
  while(queue_.empty()) cond_var_.wait(lck);
  obj = queue_.front();
  queue_.pop();
  return queue_.size();
}



int main(int argc, char* argv[]){
  blocking_queue<int> q;
  q.put(1);
  q.put(1222);
  q.put(123214);
  int out;
  while(q.get(out)){
    std::cout << out << std::endl;
  };
  std::cout << "end" << std::endl;
  return 0;
}


