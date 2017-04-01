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
  blocking_queue<cv::Mat> q_orig;
  blocking_queue<cv::Mat> q_disp;
  auto exitflag_ref = std::ref<bool>(exitflag);
  auto q_orig_ref = std::ref(q_orig);
  auto q_disp_ref = std::ref(q_disp);
  const int num_process_threads = 4;
  std::deque<std::thread> threads;
  // init disp thread
  threads.push_back(std::thread(thread_display, \
      exitflag_ref, q_disp_ref, std::string("output")));
  // init work thread
  for (int i=0; i<num_process_threads; i++)
    threads.push_back(std::thread(thread_process, \
        exitflag_ref, q_orig_ref, q_disp_ref));
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
  std::thread threadvideo;
  if(ifvideo){
    cv::VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, imgsize.width);
    // cv::CAP_PROP_FRAME_WIDTH won't work in opencv 2.x version
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, imgsize.height);
    cap >> origImg;
    CV_Assert(origImg.size() == imgsize);
    threads.push_back(std::thread(thread_capture, \
        exitflag_ref, q_orig_ref, std::ref(cap)));
  }else{
    // read as 8-bit unsigned
    origImg = imread(argv[1],cv::IMREAD_COLOR);
    //Mat origImg = imread(argv[1],IMREAD__ANYDEPTH | IMREAD_COLOR );
    if (!origImg.data){
      std::cout << "Unable to load image: " << argv[1] << std::endl;
      return -1;
    }
    origImg=cv::Mat(origImg, cv::Rect(0, 0, 640, 480));
    threads.push_back(std::thread(thread_capture_img, \
        exitflag_ref, q_orig_ref, std::ref(origImg)));
  }
  std::this_thread::sleep_for (std::chrono::seconds(60)); 
  // what to do during wait?
  // 1.wait 100ms
  // 2.check if there is any thread joinable, if true, set exit flag
  // 3.check if exit flag is set, if set, try join threads, else go to 1
  //while(

// how to join a thread?
// 1.wait 33ms, test if joinable;
// 2.if not joinable, insert black image to a queue, goto 1; else join it

  //for (auto& th: threads) th.join();
  return 0;
}

  //Mat imageblack = cv::Mat::zeros(imgsize, CV_8UC3);

