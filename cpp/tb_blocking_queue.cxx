#include <iostream>
#include <chrono>
#include <memory>
#include <opencv2/opencv.hpp>
#include "blocking_queue.hxx"


typedef std::shared_ptr<cv::Mat> mat_ptr; 


const int maxrepeat=100;
const int maxlength=100;
cv::Size imagesize(640,480);
cv::Mat image = cv::Mat::ones(imagesize,CV_8UC3);
mat_ptr tmpimage;

typedef typename std::chrono::steady_clock clocktype;
typedef typename clocktype::duration durationtype;
typedef typename clocktype::time_point timepointtype;
durationtype time_clone(0);
durationtype time_put(0);
durationtype time_get(0);
timepointtype start = clocktype::now();
timepointtype stop = start;

//clocktype::time_point start = clocktype::now();


int main(int argc, char* argv[]){
  blocking_queue<mat_ptr> q(maxlength);
  for(int repeatcnt = 0; repeatcnt < maxrepeat; repeatcnt++){
    for(int lengthcnt = 0; lengthcnt < maxlength; lengthcnt++){
      start = clocktype::now();
      tmpimage = mat_ptr(new cv::Mat());
      *tmpimage = image.clone();
      stop = clocktype::now();
      time_clone += stop - start;
      start = clocktype::now();
      q.put(tmpimage);
      stop = clocktype::now();
      time_put += stop - start;
    }
    for(int lengthcnt = 0; lengthcnt < maxlength; lengthcnt++){
      start = clocktype::now();
      q.get(tmpimage);
      stop = clocktype::now();
      time_get += stop - start;
    }
  }
  auto totalrepeats = maxlength * maxrepeat;
  typedef typename std::chrono::microseconds disptype;
  std::cout << "repeat " << totalrepeats << \
      " times, average/total time in microseconds" << std::endl;
  std::cout << "clone: " << \
      std::chrono::duration_cast<disptype>(time_clone).count()/totalrepeats \
      << "/" << std::chrono::duration_cast<disptype>(time_clone).count() \
      << std::endl;
  std::cout << "put  : " << \
      std::chrono::duration_cast<disptype>(time_put).count()/totalrepeats \
      << "/" << std::chrono::duration_cast<disptype>(time_put).count() \
      << std::endl;
  std::cout << "get  : " << \
      std::chrono::duration_cast<disptype>(time_get).count()/totalrepeats \
      << "/" << std::chrono::duration_cast<disptype>(time_get).count() \
      << std::endl;
  return 0;
}


