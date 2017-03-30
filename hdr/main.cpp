#include <iostream>
#include <cstdint>
#include <thread>
//#include <queue> we don't need a queue, as a queue of length=1 is enough
#include <mutex>
#include <opencv2/opencv.hpp>
#include "hdr.hxx"

using namespace std;
using namespace cv;


#define TIMER_STAMP_I(TIMER, ID)			\
  static double __##TIMER##_total_##ID = 0;		\
  static double __##TIMER##_count_##ID = 0;			

#define TIMER_INIT(TIMER) TIMER_STAMP_I(TIMER, timer)

#define TIMER_START(TIMER)				\
  double __##TIMER##_start = (double)getTickCount();	\
  double __##TIMER##_last = __##TIMER##_start;	
  
#define TIMER_STAMP(TIMER, ID)					\
  {								\
    double temptime = (double)getTickCount();			\
    __##TIMER##_total_##ID += temptime - __##TIMER##_last;	\
    __##TIMER##_count_##ID += 1;				\
    __##TIMER##_last = temptime;				\
  }


#define TIMER_STOP(TIMER)					\
  {								\
    double temptime = (double)getTickCount();			\
    __##TIMER##_total_timer += temptime - __##TIMER##_start;	\
    __##TIMER##_count_timer += 1;				\
  }

#define TIMER_STAMP_P(TIMER, ID)				\
  {								\
    cout << #TIMER #ID "("					\
	 <<__##TIMER##_count_##ID				\
	 <<"times) : "						\
	 << __##TIMER##_total_##ID/__##TIMER##_count_##ID	\
      /getTickFrequency()*1000					\
	 << endl;						\
  }


#define TIMER_PRINT(TIMER)			\
  {						\
    TIMER_STAMP_P(TIMER, timer);		\
  }

TIMER_INIT(HDR)




int main(int argc, char* argv[])
{
  Size_<int> imgsize(640,480);
  int ifvideo = 0;
  VideoCapture cap(0);
  Mat origImg;
  // Mat outImg = cv::Mat::zeros(imgsize, CV_8UC3);
  if(argc < 2){
    ifvideo = 1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    // cv::CAP_PROP_FRAME_WIDTH won't work in opencv 2.x version
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    cap >> origImg;
    CV_Assert(origImg.size() == imgsize);
  }
  else{
    // read as 8-bit unsigned
    origImg = imread(argv[1],IMREAD_COLOR);
    //Mat origImg = imread(argv[1],IMREAD__ANYDEPTH | IMREAD_COLOR );
    if (!origImg.data){
      cout << "Unable to load image: " << argv[1] << endl;
      return -1;
    }
    origImg=cv::Mat(origImg, cv::Rect(0, 0, 640, 480));
  }

  hdr process_hdr(imgsize);

  //static const int num_threads = 4;
  //std::thread t[num_threads];
  //for (int i=0; i<num_threads; i++){
  //  t[i]=std::thread(

  while(1){
    // if video, capture one frame
    if(ifvideo){
      cap >> origImg;
    }
    TIMER_START(HDR)
    process_hdr.process(origImg,origImg);
    TIMER_STOP(HDR)

    if(ifvideo){
      imshow("output",origImg);
      if(waitKey(1) >= 0)
        break;
    }else if((argc == 3)&&(*(argv[2]) == 'd')){
      namedWindow("Output", WINDOW_NORMAL);
      imshow("Output", origImg);
      waitKey(0);
      break;
    }else{
      static int loadimagecount=0;
      if(loadimagecount++ >= 99)
      break;
    }
  }
  TIMER_PRINT(HDR)
  return 0;
}

