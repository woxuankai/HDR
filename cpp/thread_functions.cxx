#include "thread_functions.hxx"
#include "hdr.hxx"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>


blocking_queue<cv::Mat>::size_type critical_queue_size = 30;

void thread_display(bool &exitflag, blocking_queue<mat_ptr> &image_queue_in, \
    std::string windowname){
  double fps=0;
  std::string fpsstr="0";
  std::stringstream ss;
  double fontscale=1;
  cv::Scalar textcolor(0,0,255);
  namedWindow(windowname, cv::WINDOW_AUTOSIZE);
  mat_ptr imageptr;
  while(!exitflag){
    if(image_queue_in.get(imageptr) >= 1){
      std::cout << "Warning: thread_display ignored one frame" << std::endl;
      continue;
    }
    /*ss << std::setprecision(4) << fps << std::endl;
    ss >> fpsstr;
    cv::putText(*imageptr, std::string("fps:")+fpsstr, \
        cv::Point(0, (imageptr->rows)*0.95), \
        cv::FONT_HERSHEY_SIMPLEX, fontscale, textcolor);*/
    imshow(windowname, *imageptr);
    if(cv::waitKey(1) >= 0){ //key pressed, exit
      exitflag = true;
      break;
    }
    /*static const int updatecnt_max = 10;//update fps every 10 times
    static int updatecnt = 0;
    static double timeperframe=1/30.0*updatecnt_max;
    static double thistime=cv::getTickCount()/(double)cv::getTickFrequency();
    static double lasttime=thistime - timeperframe;
    if(++updatecnt >= updatecnt_max){
      updatecnt = 0;
      thistime=cv::getTickCount()/(double)cv::getTickFrequency();
      timeperframe = thistime-lasttime;
      lasttime=thistime;
      fps=1/timeperframe*updatecnt_max;
      //std::cout << "\rfps: " << fps << std::endl;
    }*/
  }
  std::cout << "display thread exiting..." << \
      "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
  return;
}

void thread_process(bool &exitflag, blocking_queue<mat_ptr> &image_queue_in, \
    blocking_queue<mat_ptr> &image_queue_out){
  mat_ptr imageptr;
  image_queue_in.get(imageptr);
  auto imgsize = imageptr->size();
  hdr image_processor(imgsize);
  image_queue_out.put(imageptr);
  while(!exitflag){
    image_queue_in.get(imageptr);
    cv::Mat imgout(imageptr->size(), imageptr->type());
    image_processor.process(*imageptr, imgout);
    cv::hconcat(*imageptr, imgout, *imageptr);
    if(image_queue_out.put(imageptr) > critical_queue_size){
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
void thread_capture(bool &exitflag, blocking_queue<mat_ptr> &image_queue_out, \
    cv::VideoCapture &cap){
  mat_ptr imageptr;
  void* lastimagedataptr = nullptr;
  cap.grab();
  while(!exitflag){
    imageptr = mat_ptr(new cv::Mat());
    if(!cap.retrieve(*imageptr)){
      std::cout << "failed to grab!" << std::endl;
      break;
    }
    cap.grab();
    if(lastimagedataptr == (void*)imageptr->ptr()){
      // Mat data pointor should not be the same
      // in fact, I donnot think this check could not ensure that
      // allocated data space is the same addr of any existing images
      std::cout << "cap.read not allocating new space!" << std::endl;
      break;
    }
    lastimagedataptr = (void*)imageptr->ptr();
    if(image_queue_out.put(imageptr) > critical_queue_size){
      std::cout << "image_queue_out full in thread_capture!" << std::endl;
      break;
    }
    //imageptr->release();
  }
  exitflag=true;
  std::cout << "capture thread exiting..." << \
      "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
}


// capture image as video
void thread_capture_img(bool &exitflag, \
    blocking_queue<mat_ptr> &image_queue_out, cv::Mat &image){
    auto wakeuptime = std::chrono::system_clock::now();
    mat_ptr imageptr;
    while(!exitflag){
      wakeuptime = wakeuptime + std::chrono::microseconds(33333);
      std::this_thread::sleep_until(wakeuptime);
      imageptr = mat_ptr(new cv::Mat());
      *imageptr = image.clone();
      if(image_queue_out.put(imageptr) > critical_queue_size){
        std::cout << "image_queue_out full in thread_capture_img" << std::endl;
	exitflag=true;
	break;
      }
      //std::cout << std::chrono::duration_cast<std::chrono::microseconds>(\
          std::chrono::system_clock::now() - wakeuptime).count() << std::endl;
    }
      std::cout << "capture_img thread exiting..." << \
	  "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
}
