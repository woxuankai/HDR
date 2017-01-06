#include <iostream>
#include <cstdint>
#include <tbb/tbb.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//#define HDR_USE_ASSERT
#define HDR_USE_TBB

using namespace std;
using namespace cv;


void cal_L2R_table(float* table,float Rmax, float n, float alpha, float beta);
void cal_Lcone2a_table(float* table, float t);
void cal_Lconepownegatives_table(float* table, float s);

void cal_Lcone_Lrod(const Mat& srcBGR, Mat& Lcone, Mat& Lrod);
void cal_R(const Mat& L, Mat& R, float* table);
void cal_BGR(const Mat& Lcone,\
	     const Mat& DOGcone, const Mat& DOGrod,\
	     Mat& BGR,\
	     float *table_Lcone2a,\
	     float *table_Lconepownegatives);



#define TIMER_STAMP_I(TIMER, ID)		\
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

float alpha = 0.67;
float beta_cone = 4;
float beta_rod = 2;
float n = 0.8;
float R_max = 2.5;
float KK = 2.5;
float t = 0.1;
float s = 0.8;

#define IMG_WIDTH 640
#define IMG_HEIGHT 480


Mat L_cone = Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_16UC1);
Mat L_rod = Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_16UC1);
Mat R_cone = Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_32FC1);
Mat R_rod = Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_32FC1);
Mat DOG_cone = Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_32FC1);
Mat DOG_rod = Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_32FC1);
Mat hh;



float table_L2R_cone[UINT16_MAX+1] = {0};
float table_L2R_rod[UINT16_MAX+1] = {0};
float table_Lcone2a[UINT16_MAX+1] = {0};
float table_Lconepownegatives[UINT16_MAX+1] = {0};


void init_hdr();
void do_hdr(Mat &origImg);


TIMER_INIT(HDR);
TIMER_STAMP_I(HDR,LL);
TIMER_STAMP_I(HDR,L2R);
TIMER_STAMP_I(HDR,FILT);
TIMER_STAMP_I(HDR,BGR);



int main(int argc, char* argv[])
{
  int ifvideo = 0;
  VideoCapture cap(0);
  Mat origImg;
  if(argc < 2)
    {
      ifvideo = 1;
      cap.set(CAP_PROP_FRAME_WIDTH,640);
      cap.set(CAP_PROP_FRAME_HEIGHT,480);
    }
  else
    {
      // read as 8-bit unsigned
      origImg = imread(argv[1],IMREAD_COLOR);
      //Mat origImg = imread(argv[1],IMREAD__ANYDEPTH | IMREAD_COLOR );
      if (!origImg.data)
	{
	  cout << "Unable to load image: " << argv[1] << endl;
	  return -1;
	}
    }

  init_hdr();

  while(1)
    {
      if(ifvideo)
	{
	  cap >> origImg;
	}
      else
	{
	  origImg = imread(argv[1],IMREAD_COLOR);
	}
      
      CV_Assert(origImg.cols == IMG_WIDTH);
      CV_Assert(origImg.rows == IMG_HEIGHT);
      do_hdr(origImg);

      if(ifvideo)
	{
	  imshow("output",origImg);
	  if(waitKey(1) >= 0)
	    break;
	}
      else if((argc == 3)&&(*(argv[2]) == 'd'))
	{
	  //namedWindow("origin image", WINDOW_AUTOSIZE);
	  //imshow("origin image", imgin );
	  namedWindow("Output", WINDOW_NORMAL);
	  imshow("Output", origImg);
	  waitKey(0);
	  break;
	}
      else
	{
	  static int loadimagecount=0;
	  if(loadimagecount++ >= 100)
	    break;
	}
    }
  TIMER_STAMP_P(HDR,LL);
  TIMER_STAMP_P(HDR,L2R);
  TIMER_STAMP_P(HDR,FILT);
  TIMER_STAMP_P(HDR,BGR);
  TIMER_PRINT(HDR);
  return 0;
}




void init_hdr()
{
  cal_L2R_table(table_L2R_cone, R_max, n, alpha, beta_cone);
  cal_L2R_table(table_L2R_rod, R_max, n, alpha, beta_rod);
  cal_Lcone2a_table(table_Lcone2a, t);
  cal_Lconepownegatives_table(table_Lconepownegatives, s);

  hh = (getGaussianKernel(21,1,CV_32F) - getGaussianKernel(21,4,CV_32F))*KK;
  hh.at<float>(1,10) += 1;
  return;
}

void do_hdr(Mat &origImg)
{
  TIMER_START(HDR);
  
  cal_Lcone_Lrod(origImg, L_cone, L_rod);
  TIMER_STAMP(HDR,LL);

  cal_R(L_cone, R_cone, table_L2R_cone);
  cal_R(L_rod, R_rod, table_L2R_rod);
  TIMER_STAMP(HDR,L2R);

  filter2D(R_cone, DOG_cone, CV_32F, hh);
  filter2D(R_rod, DOG_rod, CV_32F, hh);
  TIMER_STAMP(HDR,FILT);
 
  cal_BGR(L_cone, DOG_cone, DOG_rod, origImg,\
	  table_Lcone2a,table_Lconepownegatives);
  TIMER_STAMP(HDR,BGR);

  TIMER_STOP(HDR);

  return;
}





void cal_Lcone_Lrod(const Mat& srcBGR, Mat& Lcone, Mat& Lrod)
{
#ifdef HDR_USE_ASSERT
  CV_Assert(srcBGR.depth() == CV_8U);
  CV_Assert(srcBGR.channels() == 3);
  CV_Assert(srcBGR.data != NULL);

  CV_Assert(Lcone.depth() == CV_16U);
  CV_Assert(Lcone.channels() == 1);
  CV_Assert(Lcone.data != srcBGR.data);
  CV_Assert(Lcone.size == srcBGR.size);

  CV_Assert(Lrod.depth() == CV_16U);
  CV_Assert(Lrod.channels() == 1);
  CV_Assert(Lrod.data != srcBGR.data);
  CV_Assert(Lrod.size == srcBGR.size);
#endif
  
  const float bgr2cone[3] = {0.072169, 0.715160, 0.212671};
  const float bgr2rod[3] = {0.359774936, 0.543640649, -0.060205215};


  int channels = 3;
  int nRows = srcBGR.rows;
  int nCols = srcBGR.cols;

#ifndef HDR_USE_TBB
  if ((srcBGR.isContinuous())&&\
      (Lcone.isContinuous())&&\
      (Lrod.isContinuous()))
    {
      nCols *= nRows;
      nRows = 1;
    }

  for(int i = 0; i < nRows; ++i)
#else
    tbb::parallel_for(0, nRows, [&](int i)
#endif
    {
      const uint8_t *psrc;
      uint16_t *pcone, *prod;

      psrc = srcBGR.ptr<uint8_t>(i);
      pcone = Lcone.ptr<uint16_t>(i);
      prod = Lrod.ptr<uint16_t>(i);
      float temp;
      for (int j = 0; j < nCols; ++j)
        {
	  int srcj = j * channels;
	  temp = bgr2cone[0]*psrc[srcj]\
	    + bgr2cone[1]*psrc[srcj+1]\
	    + bgr2cone[2]*psrc[srcj+2] + 0.5;
	  temp = temp*255;
	  if(temp < 1)
	    temp = 1;
	  if(temp > UINT16_MAX)
	    temp = UINT16_MAX;
	  pcone[j] = temp;

	  temp = bgr2rod[0]*psrc[srcj]\
	    + bgr2rod[1]*psrc[srcj+1]\
	    + bgr2rod[2]*psrc[srcj+2] + 0.5;
	  temp = temp*255;
	  if(temp < 1)
	    temp = 1;
	  if(temp > UINT16_MAX)
	    temp = UINT16_MAX;
	  prod[j] = temp;
        }
#ifdef HDR_USE_TBB
    });
#else
    }
#endif
}

void cal_R(const Mat& L, Mat& R, float* table)
{
#ifdef HDR_USE_ASSERT
  CV_Assert(L.depth() == CV_16U);
  CV_Assert(L.channels() == 1);
  CV_Assert(L.data != NULL);
  CV_Assert(R.depth() == CV_32F);
  CV_Assert(R.channels() == 1);
  CV_Assert(R.data != L.data);
  CV_Assert(R.size == L.size);
  CV_Assert(table != NULL);
#endif
  
  int nRows = L.rows;
  int nCols = L.cols;

  if ((L.isContinuous())&&\
      (R.isContinuous()))
    {
      nCols *= nRows;
      nRows = 1;
    }
  int i,j;
  const uint16_t *pL;
  float *pR;

  for( i = 0; i < nRows; ++i)
    {
      pL = L.ptr<uint16_t>(i);
      pR = R.ptr<float>(i);
      for ( j = 0; j < nCols; ++j)
        {
	  pR[j] = table[pL[j]];
        }
    }
  return;
}

void cal_L2R_table(float* table,float Rmax, float n, float alpha, float beta)
{
  CV_Assert(table != NULL);

  for(uint32_t index = 0; index <= UINT16_MAX; index++)
    {
      double Lpown = pow(double(index)/UINT16_MAX,n);
      double temp = Lpown + pow(pow(double(index)/UINT16_MAX,alpha)*beta,n);
      if(temp == 0)
	temp = 0;
      else
	temp = Rmax*Lpown/temp;
      table[index] = temp;
    }
  return;
}

void cal_Lcone2a_table(float* table, float t)
{
  CV_Assert(t > 0);
  CV_Assert(table != NULL);
  for(int i=0; i<=UINT16_MAX; i++)
    {
      double temp;
      temp = pow(double(i)/UINT16_MAX, -t);
      if(i==0)
	table[i] = FLT_MAX;
      else
	table[i] = temp;
    }
}

void cal_Lconepownegatives_table(float* table, float s)
{
  CV_Assert(s > 0);
  CV_Assert(table != NULL);
  for(int i=0; i<=UINT16_MAX; i++)
    {
      double temp;
      temp = pow(double(i)/UINT16_MAX, -s);
      if(i==0)
	table[i] = UINT16_MAX;
      table[i] = temp;
    }
}

void cal_BGR(const Mat& Lcone,\
	     const Mat& DOGcone, const Mat& DOGrod,\
	     Mat& BGR,\
	     float *table_Lcone2a,\
	     float *table_Lconepownegatives)
{
#ifdef HDR_USE_ASSERT
  CV_Assert(BGR.depth() == CV_8U);
  CV_Assert(BGR.channels() == 3);
  CV_Assert(BGR.data != NULL);
  CV_Assert(Lcone.depth() == CV_16U);
  CV_Assert(Lcone.channels() == 1);
  CV_Assert(Lcone.data != BGR.data);
  CV_Assert(Lcone.size == BGR.size);
  CV_Assert(DOGcone.depth() == CV_32F);
  CV_Assert(DOGcone.channels() == 1);
  CV_Assert(DOGcone.data != BGR.data);
  CV_Assert(DOGcone.size == BGR.size);
  CV_Assert(DOGrod.depth() == CV_32F);
  CV_Assert(DOGrod.channels() == 1);
  CV_Assert(DOGrod.data != BGR.data);
  CV_Assert(DOGrod.size == BGR.size);
  CV_Assert(table_Lcone2a != NULL);
  CV_Assert(table_Lconepownegatives != NULL);
#endif
  
  double minvalue,maxvalue;
  minMaxLoc(Lcone, &minvalue, &maxvalue);
  float mina;
  mina = table_Lcone2a[(uint16_t)maxvalue];

  int channels = BGR.channels();
  int nRows = BGR.rows;
  int nCols = BGR.cols;

#ifndef HDR_USE_TBB
  if ((BGR.isContinuous())&&\
      (Lcone.isContinuous())&&\
      (DOGcone.isContinuous())&&\
      (DOGrod.isContinuous()))
    {
      nCols *= nRows;
      nRows = 1;
    }

  for(int i = 0; i < nRows; ++i)
#else
  tbb::parallel_for(0, nRows,[&](int i)
#endif
    {
      uint8_t *pBGR;
      const uint16_t *pLcone;
      const float *pDOGcone, *pDOGrod;
      float tempbgr;
      pBGR = BGR.ptr<uint8_t>(i);
      pLcone = Lcone.ptr<uint16_t>(i);
      pDOGcone = DOGcone.ptr<float>(i);
      pDOGrod = DOGrod.ptr<float>(i);

      for (int j = 0; j < nCols; ++j)
        {
	  int BGRj = j * channels;
	  uint16_t Lconevalue;
	  float DOGconevalue,DOGrodvalue;
	  float a,w,Lout;
	  Lconevalue = pLcone[j];
	  DOGconevalue = pDOGcone[j];
	  DOGrodvalue = pDOGrod[j];

	  a = table_Lcone2a[Lconevalue];
	  w = 1/(1-mina+a);
	  Lout = w*DOGconevalue + (1-w)*DOGrodvalue;

	  tempbgr = pBGR[BGRj]*\
	    table_Lconepownegatives[Lconevalue]*\
	    Lout + 0.5;
	  if(tempbgr < 0)
	    tempbgr = 0;
	  if(tempbgr > UINT8_MAX)
	    tempbgr = UINT8_MAX;
	  pBGR[BGRj] = tempbgr;

	  tempbgr = pBGR[BGRj+1]*\
	    table_Lconepownegatives[Lconevalue]*\
	    Lout + 0.5;
	  if(tempbgr < 0)
	    tempbgr = 0;
	  if(tempbgr > UINT16_MAX)
	    tempbgr = UINT16_MAX;
	  pBGR[BGRj+1] = tempbgr;

	  tempbgr = pBGR[BGRj+2]*\
	    table_Lconepownegatives[Lconevalue]*\
	    Lout + 0.5;
	  if(tempbgr < 0)
	    tempbgr = 0;
	  if(tempbgr > UINT8_MAX)
	    tempbgr = UINT8_MAX;
	  pBGR[BGRj+2] = tempbgr;
	}
#ifdef HDR_USE_TBB
    });
#else
    }
#endif
  return;
}

