#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "blocking_queue.hxx"



const int maxrepeat=100;
const int maxlength=30;
cv::Size imagesize(640,480);
cv::Mat image = cv::Mat::ones(imagesize,CV_8UC3);
cv::Mat tmpimage = image;

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
  blocking_queue<cv::Mat> q;
  for(int repeatcnt = 0; repeatcnt < maxrepeat; repeatcnt++){
    for(int lengthcnt = 0; lengthcnt < maxlength; lengthcnt++){
      start = clocktype::now();
      tmpimage = image.clone();
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
      " times, in microseconds" << std::endl;
  std::cout << "clone: " << \
      std::chrono::duration_cast<disptype>(time_clone).count() << std::endl;
  std::cout << "put  : " << \
      std::chrono::duration_cast<disptype>(time_put).count() << std::endl;
  std::cout << "get  : " << \
      std::chrono::duration_cast<disptype>(time_get).count() << std::endl;
  return 0;
}


