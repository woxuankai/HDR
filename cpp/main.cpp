#include <iostream>
#include <cstdint>
#include <thread>
#include <deque>
#include <functional>
#include <opencv2/opencv.hpp>
#include "hdr.hxx"
#include "blocking_queue.hxx"
#include "thread_functions.hxx"


void fun(blocking_queue<cv::Mat> &q){
  std::cout << "papa" << std::endl;
  return;
}


int main(int argc, char* argv[])
{
  // init threads
  bool exitflag=false;
  blocking_queue<mat_ptr> q_orig;
  blocking_queue<mat_ptr> q_disp;
  auto exitflag_ref = std::ref<bool>(exitflag);
  auto q_orig_ref = std::ref(q_orig);
  auto q_disp_ref = std::ref(q_disp);
  const int num_process_threads = 4;
  std::deque<std::thread> threads;
  // init disp thread
  threads.push_front(std::thread(thread_display, \
      exitflag_ref, q_disp_ref, std::string("output")));
  std::cout << "thread display started" << std::endl;
  // init work thread
  for (int i=0; i<num_process_threads; i++){
    threads.push_front(std::thread(thread_process, \
        exitflag_ref, q_orig_ref, q_disp_ref));
    std::cout << "thread process " << i << " started" << std::endl;
  }
  // init capture
  bool ifvideo;
  std::string path;
  if(argc < 2){
    ifvideo = true;
    path = "/dev/video0";
  }else{
    ifvideo = false;
    path = std::string(argv[1]);
  }
  cv::Size imgsize(640,480);
  cv::Mat origImg;
  cv::VideoCapture cap;
  if(ifvideo){
    cap = cv::VideoCapture(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, imgsize.width);
    // cv::CAP_PROP_FRAME_WIDTH won't work in opencv 2.x version
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, imgsize.height);
    if(!cap.isOpened())
      cap.open(0);
    if(!cap.isOpened()){
      std::cout << "Can not open video_capture device : " << path << std::endl;
      return -1;
    }
    cap.read(origImg);
    CV_Assert(origImg.size() == imgsize);
    threads.push_front(std::thread(thread_capture, \
        exitflag_ref, q_orig_ref, std::ref(cap)));
    std::cout << "thread capture started" << std::endl;
  }else{
    // read as 8-bit unsigned
    origImg = imread(argv[1],cv::IMREAD_COLOR);
    //Mat origImg = imread(argv[1],IMREAD__ANYDEPTH | IMREAD_COLOR );
    if (!origImg.ptr()){
      std::cout << "Unable to load image: " << argv[1] << std::endl;
      return -1;
    }
    origImg=cv::Mat(origImg, cv::Rect(0, 0, 640, 480));
    threads.push_front(std::thread(thread_capture_img, \
        exitflag_ref, q_orig_ref, std::ref(origImg)));
    std::cout << "thread capture_img started" << std::endl;
  }
  
  int check_interval_ms = 500;
  int join_interval_ms = 100;
  // what to do during wait?
  // 1.wait some ms
  // 2.check if there is any thread joinable, if true, set exit flag
  // 3.check if exit flag is set, if set, try join threads, else go to 1
  while(!exitflag){
    std::this_thread::sleep_for(std::chrono::milliseconds(check_interval_ms));
    //for(auto & onethread: threads){
    //  if(onethread.joinable()){
    //    exitflag=true;
    //    break;
    //  }
    //}
  }

  //std::this_thread::sleep_for (std::chrono::seconds(60)); 
// how to join a thread?
// 1.wait 33ms, test if joinable;
// 2.if not joinable, insert black image to a queue, goto 1; else join it
  cv::Mat imageblack = cv::Mat::zeros(imgsize, CV_8UC3);
  for (auto& onethread: threads){
    //while(!onethread.joinable()){
      mat_ptr ptr1, ptr2;
      ptr1 = mat_ptr(new cv::Mat());
      ptr2 = mat_ptr(new cv::Mat());
      *ptr1 = imageblack.clone();
      *ptr2 = imageblack.clone();
      q_orig.put(ptr1); // Now we donnot care if images share same
      q_disp.put(ptr2); // data address
      // In fact, pushing images to queue can not guarantee that next thread
      // to be joined can fetch from queue when more than one consumers are
      // waiting from the same queue.
      std::this_thread::sleep_for(std::chrono::milliseconds(join_interval_ms));
    //}
    onethread.join();
  }

  return 0;
}


