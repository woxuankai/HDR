#include "thread_functions.hxx"


int critical_queue_size = 100;

void thread_display(bool &exitflag, blocking_queue &image_queue_in){
  namedWindow("Output", WINDOW_NORMAL);
  cv::Mat image;
  auto queue_size = image_queue_in.get(image);
  while(!exitflag){
    queue_size = image_queue_in.get(image);
    imshow("Output", image);
    if(waitKey(1) >= 0) //key pressed, exit
      exitflag = true;
  }
  std::cout << "display thread exiting..." << \
      "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
  return;
}

void thread_process(bool &exitflag, blocking_queue &image_queue_in, \
    blocking_queue &image_queue_out){
  cv::Mat image;
  image_queue_in.get(image);
  auto imgsize = image.size();
  hdr image_processor(imagesize);
  image_queue_out.put(image);
  while(!exitflag){
    image_queue_in.get(image);
    image_processor.process(image,image);
    if(image_queue_out.put(image);
  }
  std::cout << "processor thread exiting..." << \
      "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
  return;
}

//capture from video or camera
void thread_capture(bool &exitflag, blocking_queue &image_queue_out, \
    cv::VideoCapture &cap){
  while(!exitflag){
  }
  std::cout << "capture thread exiting..." << \
      "(thread id: " << std::this_thread::get_id() << ")" << std::endl;
}


// capture image as video
void thread_capture_img(bool &exitflag, blocking_queue &image_queue_out, \
    cv::Mat &image){
    key_wait(33)
}
