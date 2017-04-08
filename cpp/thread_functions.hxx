#ifndef __MY_THREAD_FUNCTIONS_HXX
#define __MY_THREAD_FUNCTIONS_HXX

#include <opencv2/opencv.hpp>
#include <string>
#include <memory>
#include "blocking_queue.hxx"

typedef std::shared_ptr<cv::Mat> mat_ptr;

// defined in thread_functions.cxx
extern blocking_queue<mat_ptr>::size_type critical_queue_size;

// display images from queue
void thread_display(bool &exitflag, blocking_queue<mat_ptr> &image_queue_in, \
    std::string windowname);

void thread_process(bool &exitflag, blocking_queue<mat_ptr> &image_queue_in, \
    blocking_queue<mat_ptr> &image_queue_out);

// capture from video or camera
void thread_capture(bool &exitflag, blocking_queue<mat_ptr> &image_queue_out, \
    cv::VideoCapture &cap);

// capture image as video
void thread_capture_img(bool &exitflag, \
    blocking_queue<mat_ptr> &image_queue_out, cv::Mat &image);

#endif //__MY_THREAD_FUNCTIONS_HXX
