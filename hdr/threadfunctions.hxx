#include "blocking_queue.hxx"
#include <opencv2/opencv>


int critical_queue_size = 100;

void thread_display(bool &exitflag, blocking_queue &image_queue_in){
  namedWindow("Output", WINDOW_NORMAL);

void thread_process(bool &exitflag, blocking_queue &image_queue_in, \
    blocking_queue &image_queue_out);

// capture from video or camera
void thread_capture(bool &exitflag, blocking_queue &image_queue_out, \
    cv::VideoCapture &cap);


// capture image as video
void thread_capture_img(bool &exitflag, blocking_queue &image_queue_out, \
    cv::Mat &image);
