#include "thread_functions.hxx"
#include "hdr.hxx"
#include <iostream>
#include <thread>
#include <chrono>

blocking_queue<cv::Mat>::size_type critical_queue_size = 30;

void thread_display(bool &exitflag, blocking_queue<cv::Mat> &image_queue_in, \
    std::string windowname){
  namedWindow(windowname, cv::WINDOW_NORMAL);
  cv::Mat image;
  while(!exitflag){
    image_queue_in.get(image);
    imshow("Output", image);
    if(cv::waitKey(1) >= 0){ //key pressed, exit
      exitflag = true;
      break;
    }
  }
  std::cout << "display thread exiting..." << \
      "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
  return;
}

void thread_process(bool &exitflag, blocking_queue<cv::Mat> &image_queue_in, \
    blocking_queue<cv::Mat> &image_queue_out){
  cv::Mat image;
  image_queue_in.get(image);
  auto imgsize = image.size();
  hdr image_processor(imgsize);
  image_queue_out.put(image);
  while(!exitflag){
    image_queue_in.get(image);
    image_processor.process(image,image);
    if(image_queue_out.put(image) > critical_queue_size){
      std::cout << "image queue out full in thread_process!" << std::endl;
      exitflag=true;
      break;
    }
  }
  std::cout << "processor thread exiting..." << \
      "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
  return;
}

//capture from video or camera
void thread_capture(bool &exitflag, blocking_queue<cv::Mat> &image_queue_out, \
    cv::VideoCapture &cap){
  cv::Mat image;
  while(!exitflag){
    cap.read(image);
    if(image_queue_out.put(image.clone()) > critical_queue_size){
      std::cout << "image_queue_out full in thread_capture!" << std::endl;
      exitflag=true;
      break;
    }
  }
  std::cout << "capture thread exiting..." << \
      "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
}


// capture image as video
void thread_capture_img(bool &exitflag, \
    blocking_queue<cv::Mat> &image_queue_out, cv::Mat &image){
    while(!exitflag){
      if(image_queue_out.put(image.clone()) > critical_queue_size){
        std::cout << "image_queue_out full in thread_capture_img" << std::endl;
	exitflag=true;
	break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
      std::cout << "capture_img thread exiting..." << \
	  "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
}