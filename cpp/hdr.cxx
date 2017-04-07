#include <cstdint>
#include <opencv2/opencv.hpp>
#include "hdr.hxx"

#define HDR_USE_ASSERT

hdr::hdr(cv::Size _imgsize): imgsize(_imgsize) {
  cal_L2R_table(table_L2R_cone, R_max, n, alpha, beta_cone);
  cal_L2R_table(table_L2R_rod, R_max, n, alpha, beta_rod);
  cal_Lcone2a_table(table_Lcone2a, t);
  cal_Lconepownegatives_table(table_Lconepownegatives, s);

  hh = (cv::getGaussianKernel(5,0.25,CV_32F) - \
      cv::getGaussianKernel(5,1,CV_32F));
  hh = hh.t() * hh * KK;
  hh.at<float>(2,2) += 1;
  return;
}

hdr::~hdr(){
};

void hdr::process(cv::Mat &imgin, cv::Mat &imgout) {
  CV_Assert(imgin.ptr() == imgout.ptr());// in cal_BGR, input image is used
  CV_Assert(imgout.size() == imgsize);
  CV_Assert(imgin.size() == imgsize);
  CV_Assert(imgout.ptr() != nullptr);
  CV_Assert(imgin.ptr() != nullptr);
  CV_Assert(imgin.channels() == 3);
  CV_Assert(imgout.channels() == 3);
  cal_Lcone_Lrod(imgin, L_cone, L_rod);
  cal_R(L_cone, R_cone, table_L2R_cone);
  cal_R(L_rod, R_rod, table_L2R_rod);
  filter2D(R_cone, DOG_cone, CV_32F, hh);
  filter2D(R_rod, DOG_rod, CV_32F, hh);
  cal_BGR(L_cone, DOG_cone, DOG_rod, imgout,\
	  table_Lcone2a,table_Lconepownegatives);
  return;
}




void hdr::cal_Lcone_Lrod(const cv::Mat& srcBGR, cv::Mat& Lcone, cv::Mat& Lrod)
{
#ifdef HDR_USE_ASSERT
  CV_Assert(srcBGR.depth() == CV_8U);
  CV_Assert(srcBGR.channels() == 3);
  CV_Assert(srcBGR.ptr() != NULL);

  CV_Assert(Lcone.depth() == CV_16U);
  CV_Assert(Lcone.channels() == 1);
  CV_Assert(Lcone.ptr() != srcBGR.ptr());
  CV_Assert(Lcone.size == srcBGR.size);

  CV_Assert(Lrod.depth() == CV_16U);
  CV_Assert(Lrod.channels() == 1);
  CV_Assert(Lrod.ptr() != srcBGR.ptr());
  CV_Assert(Lrod.size == srcBGR.size);
#endif
  
  const float bgr2cone[3] = {0.072169, 0.715160, 0.212671};
  const float bgr2rod[3] = {0.359774936, 0.543640649, -0.060205215};


  int channels = 3;
  int nRows = srcBGR.rows;
  int nCols = srcBGR.cols;

  if ((srcBGR.isContinuous())&&\
      (Lcone.isContinuous())&&\
      (Lrod.isContinuous()))
    {
      nCols *= nRows;
      nRows = 1;
    }

  for(int i = 0; i < nRows; ++i)
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
    }
}

void hdr::cal_R(const cv::Mat& L, cv::Mat& R, float* table)
{
#ifdef HDR_USE_ASSERT
  CV_Assert(L.depth() == CV_16U);
  CV_Assert(L.channels() == 1);
  CV_Assert(L.ptr() != NULL);
  CV_Assert(R.depth() == CV_32F);
  CV_Assert(R.channels() == 1);
  CV_Assert(R.ptr() != L.ptr());
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

void hdr::cal_L2R_table(float* table,float Rmax, float n, float alpha, float beta)
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

void hdr::cal_Lcone2a_table(float* table, float t)
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

void hdr::cal_Lconepownegatives_table(float* table, float s)
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

void hdr::cal_BGR(const cv::Mat& Lcone,\
	     const cv::Mat& DOGcone, const cv::Mat& DOGrod,\
	     cv::Mat& BGR,\
	     float *table_Lcone2a,\
	     float *table_Lconepownegatives)
{
#ifdef HDR_USE_ASSERT
  CV_Assert(BGR.depth() == CV_8U);
  CV_Assert(BGR.channels() == 3);
  CV_Assert(BGR.ptr() != NULL);
  CV_Assert(Lcone.depth() == CV_16U);
  CV_Assert(Lcone.channels() == 1);
  CV_Assert(Lcone.ptr() != BGR.ptr());
  CV_Assert(Lcone.size == BGR.size);
  CV_Assert(DOGcone.depth() == CV_32F);
  CV_Assert(DOGcone.channels() == 1);
  CV_Assert(DOGcone.ptr() != BGR.ptr());
  CV_Assert(DOGcone.size == BGR.size);
  CV_Assert(DOGrod.depth() == CV_32F);
  CV_Assert(DOGrod.channels() == 1);
  CV_Assert(DOGrod.ptr() != BGR.ptr());
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

  if ((BGR.isContinuous())&&\
      (Lcone.isContinuous())&&\
      (DOGcone.isContinuous())&&\
      (DOGrod.isContinuous()))
    {
      nCols *= nRows;
      nRows = 1;
    }

  for(int i = 0; i < nRows; ++i)
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
    }
  return;
}

