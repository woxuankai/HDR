#ifndef __MY_HDR_HXX
#define __MY_HDR_HXX

#include<opencv2/opencv.hpp>
#include<cstdint>

class hdr;

class hdr{

protected:
  // image size
  cv::Size imgsize;
  
  // paramaters
  float alpha = 0.67;
  float beta_cone = 4;
  float beta_rod = 2;
  float n = 0.8;
  float R_max = 2.5;
  float KK = 2.5;
  float t = 0.1;
  float s = 0.8;

  // calculate tables
  void cal_L2R_table(float* table,float Rmax, float n, float alpha, float beta);
  void cal_Lcone2a_table(float* table, float t);
  void cal_Lconepownegatives_table(float* table, float s);

  // calculate temp result, used in function process
  void cal_Lcone_Lrod(const cv::Mat& srcBGR, cv::Mat& Lcone, cv::Mat& Lrod);
  void cal_R(const cv::Mat& L, cv::Mat& R, float* table);
  void cal_BGR(const cv::Mat& Lcone,\
      const cv::Mat& DOGcone, const cv::Mat& DOGrod,\
      cv::Mat& BGR,\
      float *table_Lcone2a,\
      float *table_Lconepownegatives);

  // temporary variables
  cv::Mat L_cone = cv::Mat::zeros(imgsize, CV_16UC1);
  cv::Mat L_rod = cv::Mat::zeros(imgsize, CV_16UC1);
  cv::Mat R_cone = cv::Mat::zeros(imgsize, CV_32FC1);
  cv::Mat R_rod = cv::Mat::zeros(imgsize, CV_32FC1);
  cv::Mat DOG_cone = cv::Mat::zeros(imgsize, CV_32FC1);
  cv::Mat DOG_rod = cv::Mat::zeros(imgsize, CV_32FC1);
  cv::Mat hh;

  float table_L2R_cone[UINT16_MAX+1] = {0};
  float table_L2R_rod[UINT16_MAX+1] = {0};
  float table_Lcone2a[UINT16_MAX+1] = {0};
  float table_Lconepownegatives[UINT16_MAX+1] = {0};

public:
  hdr(cv::Size imgsize);
  ~hdr();
  void process(cv::Mat &in, cv::Mat &out);
};

#endif //__MY_HDR_HXX
