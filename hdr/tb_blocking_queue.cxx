#include <iostream>
#include <chrono>
#include "blocking_queue.hxx"


const int maxrepeat=1000;
const int maxlength=30;
cv::Size imagesize(640,480);
auto image = cv::Mat::ones(Size,cv::CV_8UC3)

int main(int argc, char* argv[]){
  blocking_queue<cv::Mat> q;
  for(int repeatcnt = 0; repeatcnt < maxrepeat; repeatcnt++)
    for(int lengthcnt = 0; lengthcnt < maxlength; lengthcnt++)
      clone
      q.put(imagecopy);
  q.put(123214);
  q.put(123214);
  int out;
  while(q.get(out)){
    std::cout << out << std::endl;
  };
  std::cout << "end" << std::endl;
  return 0;
}


