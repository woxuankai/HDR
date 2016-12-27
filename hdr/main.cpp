#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdint.h>
#include <limits.h>
#include <semaphore.h>
#include <pthread.h>
using namespace std;
using namespace cv;


int threadnum = 1;


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

#define TIMESTAMP(id) \
{\
	double temptime = (double)getTickCount();\
	cout << #id << ": "\
		<< (temptime - lasttime ) \
			/ getTickFrequency() * 1000 << endl;\
	lasttime = temptime ;\
}\


int main(int argc, char* argv[])
{


	if(argc < 2)
	{
		cout << "usage:" << endl;
		cout << "./hdr imgfile [d]" << endl;
		return -1;
	}
	// read as 8-bit unsigned
	Mat origImg = imread(argv[1],CV_LOAD_IMAGE_COLOR );
	//Mat origImg = imread(argv[1],CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR );
	if (!origImg.data)
	{
		cout << "Unable to load image: " << argv[1] << endl;
		return -1;
	}
	origImg.convertTo(origImg, CV_16U, 255);

	/////////parameters///////////////
	float alpha = 0.67;
	float beta_cone = 4;
	float beta_rod = 2;
	float n=0.8;
	float R_max=2.5;
	float KK=2.5;
	float t=0.1;
	float s=0.8;
	///////////end///////////////////
	////////one-time work//////////
	Mat L_cone = Mat::zeros(origImg.rows, origImg.cols, CV_16UC1);
	Mat L_rod = Mat::zeros(origImg.rows, origImg.cols, CV_16UC1);
	Mat R_cone = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat R_rod = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat DOG_cone = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat DOG_rod = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat hh = (getGaussianKernel(21,1,CV_32F) - getGaussianKernel(21,4,CV_32F))*KK;
	hh.at<float>(1,10) += 1;


	float table_L2R_cone[UINT16_MAX+1] = {0};
	float table_L2R_rod[UINT16_MAX+1] = {0};
	float table_Lcone2a[UINT16_MAX+1] = {0};
	float table_Lconepownegatives[UINT16_MAX+1] = {0};

	cal_L2R_table(table_L2R_cone, R_max, n, alpha, beta_cone);
	cal_L2R_table(table_L2R_rod, R_max, n, alpha, beta_rod);
	cal_Lcone2a_table(table_Lcone2a, t);
	cal_Lconepownegatives_table(table_Lconepownegatives, s);
	/////////////end//////////////////


	////////////////////////////infinite loop///////////////////////////////////
	double timestart = (double)getTickCount();
	double lasttime = timestart;

	cal_Lcone_Lrod(origImg, L_cone, L_rod);
TIMESTAMP(rod_cone);

	cal_R(L_cone, R_cone, table_L2R_cone);
	cal_R(L_rod, R_rod, table_L2R_rod);
TIMESTAMP(genR);


	filter2D(R_cone, DOG_cone, CV_32F, hh);
	filter2D(R_rod, DOG_rod, CV_32F, hh);
TIMESTAMP(dofilt);


	cal_BGR(L_cone, DOG_cone, DOG_rod, origImg,\
			table_Lcone2a,table_Lconepownegatives);
	TIMESTAMP(calBGR);

	timestart  = ((double)getTickCount() - timestart ) / getTickFrequency() * 1000;
	cout << "time cost : "<< timestart << endl;
	////////////////////////////end infinite loop///////////////////////////////////


	if((argc == 3)&&(*(argv[2]) == 'd'))
	{
		//namedWindow("origin image", WINDOW_AUTOSIZE);
		//imshow("origin image", imgin );
		namedWindow("Output", WINDOW_NORMAL);
		imshow("Output", origImg);
		waitKey(0);
	}
	return 0;
}


void cal_Lcone_Lrod(const Mat& srcBGR, Mat& Lcone, Mat& Lrod)
{
    CV_Assert(srcBGR.depth() == CV_16U);
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

	//Lcone = 0.2127*LinR + 0.7152*LinG + 0.0722*LinB;
	//Lrod = -0.0602*LinR + 0.5436*LinG + 0.3598*LinB;
    const float bgr2cone[3] = {0.072169, 0.715160, 0.212671};
    const float bgr2rod[3] = {0.359774936, 0.543640649, -0.060205215};
    //const float bgr2rod[3] = {0.359775, 0.543641, -0.060205};

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
    int i,j;
    const uint16_t *psrc;
    uint16_t *pcone, *prod;

    for( i = 0; i < nRows; ++i)
    {
    	psrc = srcBGR.ptr<uint16_t>(i);
        pcone = Lcone.ptr<uint16_t>(i);
    	prod = Lrod.ptr<uint16_t>(i);
    	float temp;
        for ( j = 0; j < nCols; ++j)
        {
        	int srcj = j * channels;
        	temp = bgr2cone[0]*psrc[srcj]\
        			+ bgr2cone[1]*psrc[srcj+1]\
					+ bgr2cone[2]*psrc[srcj+2] + 0.5;
        	if(temp < 1)
        		temp = 1;
        	if(temp > UINT16_MAX)
        		temp = UINT16_MAX;
        	pcone[j] = temp;

        	temp = bgr2rod[0]*psrc[srcj]\
        			+ bgr2rod[1]*psrc[srcj+1]\
					+ bgr2rod[2]*psrc[srcj+2] + 0.5;
        	if(temp < 1)
        		temp = 1;
        	if(temp > UINT16_MAX)
        		temp = UINT16_MAX;
        	prod[j] = temp;
        }
    }
}

void cal_R(const Mat& L, Mat& R, float* table)
{
    CV_Assert(L.depth() == CV_16U);
    CV_Assert(L.channels() == 1);
    CV_Assert(L.data != NULL);
    CV_Assert(R.depth() == CV_32F);
    CV_Assert(R.channels() == 1);
    CV_Assert(R.data != L.data);
    CV_Assert(R.size == L.size);
    CV_Assert(table != NULL);

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
    CV_Assert(BGR.depth() == CV_16U);
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
    int i,j;
    uint16_t *pBGR;
    const uint16_t *pLcone;
    const float *pDOGcone, *pDOGrod;
    float tempbgr;

    for( i = 0; i < nRows; ++i)
    {
    	pBGR = BGR.ptr<uint16_t>(i);
        pLcone = Lcone.ptr<uint16_t>(i);
    	pDOGcone = DOGcone.ptr<float>(i);
    	pDOGrod = DOGrod.ptr<float>(i);

        for ( j = 0; j < nCols; ++j)
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
        	if(tempbgr > UINT16_MAX)
        		tempbgr = UINT16_MAX;
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
        	if(tempbgr > UINT16_MAX)
        		tempbgr = UINT16_MAX;
        	pBGR[BGRj+2] = tempbgr;
        }
    }
    return;
}

