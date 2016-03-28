#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdint.h>
#include <limits.h>

using namespace std;
using namespace cv;


void cal_Lcone_Lrod(const Mat& srcBGR, Mat& Lcone, Mat& Lrod);
void cal_R(const Mat& L, Mat& R, float* table);
void cal_L2R_table(float* table,float Rmax, float n, float alpha, float beta);
void correct_dog(const Mat& R, Mat& DOG, float KK);




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
	Mat origImg = imread(argv[1],CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR );
	if (!origImg.data)
	{
		cout << "Unable to load image: " << argv[1] << endl;
		return -1;
	}

	/////////parameters///////////////
	float alpha = 0.67;
	float beta_cone = 4;
	float beta_rod = 2;
	float n=0.8;
	float R_max=2.5;
	float KK=2.5;
	float t=0.1;
	///////////end///////////////////
	////////one-time work//////////
	Mat L_cone = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat L_rod = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat R_cone = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat R_rod = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat hh = getGaussianKernel(21,1,CV_32F) - getGaussianKernel(21,4,CV_32F);
	Mat a = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat Lout = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);

	float table_L2R_cone[UINT16_MAX+1] = {0};
	float table_L2R_rod[UINT16_MAX+1] = {0};
	cal_L2R_table(table_L2R_cone, R_max, n, alpha, beta_cone);
	cal_L2R_table(table_L2R_rod, R_max, n, alpha, beta_rod);

	/////////////end//////////////////


	////////temporary///////////
	Mat imgout, imgin;
	const float srcgain =  (1./(UINT16_MAX));
	origImg.convertTo(imgin, CV_32FC3, srcgain);
	////////end/////////


	////////////////////////////infinite loop///////////////////////////////////
	double timestart = (double)getTickCount();
	double lasttime = timestart;

	cal_Lcone_Lrod(origImg, L_cone, L_rod);
TIMESTAMP(rod_cone);

	cal_R(L_cone, R_cone, table_L2R_cone);
	cal_R(L_rod, R_rod, table_L2R_rod);
TIMESTAMP(genR);

	Mat DOG_cone, DOG_rod;
	filter2D(R_cone, DOG_cone, CV_32F, hh);
	filter2D(R_rod, DOG_rod, CV_32F, hh);
TIMESTAMP(dofilt);

	//correct_dog(R_cone, DOG_cone, KK);
	//correct_dog(R_rod, DOG_rod, KK);
	DOG_cone = R_cone + KK * DOG_cone;
	DOG_rod = R_rod + KK * DOG_rod;
	double minvalue, maxvalue;
	minMaxLoc(DOG_rod, &minvalue, &maxvalue);
	float maxd = maxvalue;
	if (maxd<=1)
	{
		float minDOG_rod = minvalue,
				maxDOG_rod = maxvalue;
		minMaxLoc(DOG_cone, &minvalue, &maxvalue);
		float minDOG_cone = minvalue,
				maxDOG_cone = maxvalue;
	    DOG_cone =
	    		(DOG_cone-minDOG_cone)/
				(maxDOG_cone - minDOG_cone);
	    DOG_rod =
	    		(DOG_rod-minDOG_rod)/
	    		(maxDOG_rod-minDOG_rod);
	}
TIMESTAMP(correctDOG);

	pow(L_cone, -t, a);
	minMaxLoc(a, &minvalue, &maxvalue);
	float minVal = minvalue;
	Mat w=1/(1 - minVal +a);
	multiply(w , DOG_cone, DOG_cone);
	multiply(1-w , DOG_rod, DOG_rod);
	Lout =  DOG_cone + DOG_rod;

TIMESTAMP(weightedadd);

	Mat Lcone3c,Lout3c;
	cvtColor(L_cone, Lcone3c, CV_GRAY2BGR);
	cvtColor(Lout, Lout3c, CV_GRAY2BGR);
TIMESTAMP(meaningless);

	float s=0.8;
	divide(imgin, Lcone3c, imgout);
//TIMESTAMP(div);
	pow(imgout , s, imgout);
//TIMESTAMP(pow);
	multiply(imgout , Lout3c, imgout);
//TIMESTAMP(mul);
TIMESTAMP(RGB);

	timestart  = ((double)getTickCount() - timestart ) / getTickFrequency() * 1000;
	cout << "time cost : "<< timestart << endl;
	////////////////////////////end infinite loop///////////////////////////////////

	if((argc == 3)&&(*(argv[2]) == 'd'))
	{
		namedWindow("origin image", WINDOW_AUTOSIZE);
		imshow("origin image", imgin );
		namedWindow("Output", WINDOW_AUTOSIZE);
		imshow("Output", imgout);
		waitKey(0);
	}
	return 0;
}


void cal_Lcone_Lrod(const Mat& srcBGR, Mat& Lcone, Mat& Lrod)
{
    CV_Assert(srcBGR.depth() == CV_16U);
    CV_Assert(srcBGR.channels() == 3);
    CV_Assert(srcBGR.data != NULL);

    CV_Assert(Lcone.depth() == CV_32F);
    CV_Assert(Lcone.channels() == 1);
    CV_Assert(Lcone.data != srcBGR.data);
    CV_Assert(Lcone.size == srcBGR.size);

    CV_Assert(Lrod.depth() == CV_32F);
    CV_Assert(Lrod.channels() == 1);
    CV_Assert(Lrod.data != srcBGR.data);
    CV_Assert(Lrod.size == srcBGR.size);

    const float srcgain = 1./(UINT16_MAX);
	//Lcone = 0.2127*LinR + 0.7152*LinG + 0.0722*LinB;
	//Lrod = -0.0602*LinR + 0.5436*LinG + 0.3598*LinB;
    const float bgr2cone[3] = {srcgain*0.0722,\
    		srcgain*0.7152,\
			srcgain*0.2127};
    const float bgr2rod[3] = {srcgain*0.3598,\
    		srcgain*0.5436,\
			-srcgain*0.0602};

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
	float *pcone, *prod;

    for( i = 0; i < nRows; ++i)
    {
    	psrc = srcBGR.ptr<uint16_t>(i);
        pcone = Lcone.ptr<float>(i);
    	prod = Lrod.ptr<float>(i);
        for ( j = 0; j < nCols; ++j)
        {
        	int srcj = j * channels;

        	pcone[j] = bgr2cone[0]*psrc[srcj]\
        			+ bgr2cone[1]*psrc[srcj+1]\
					+ bgr2cone[2]*psrc[srcj+2];
        	//if(pcone[j] <= 0)
        	//	pcone[j] = 0;
        	if(pcone[j] > 1)
        		pcone[j] = 1;
        	prod[j] = bgr2rod[0]*psrc[srcj]\
        			+ bgr2rod[1]*psrc[srcj+1]\
					+ bgr2rod[2]*psrc[srcj+2];
        	if(prod[j] < 0)
        		//prod[j] = FLT_MIN;
        		prod[j] = 0;
        	if(prod[j] > 1)
        		prod[j] = 1;
        }
    }
}






void cal_R(const Mat& L, Mat& R, float* table)
{
    CV_Assert(L.depth() == CV_32F);
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
    const float *pL;
    float *pR;

    for( i = 0; i < nRows; ++i)
    {
    	pL = L.ptr<float>(i);
        pR = R.ptr<float>(i);
        for ( j = 0; j < nCols; ++j)
        {
        	uint16_t index = (uint16_t)(pL[j]*UINT16_MAX+0.5);
        	pR[j] = table[index];
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
		double temp = Lpown + pow(pow(double(index)/UINT16_MAX,alpha)*beta,n)+FLT_MIN;
		table[index] = Rmax*Lpown/temp;
	}
	return;
}

void correct_dog(const Mat& R, Mat& DOG, float KK)
{
	CV_Assert(false);
	return ;
}

//void cal_Lout(const Mat& L_cone,const Mat& DOG_cone, const Mat& DOG_rod,\
		Mat& Lout, )


/*
 	if(1)
	{
		double time = (double)getTickCount();
		time = (double)getTickCount();
		float tempfloat = 1;
		for(int j=0;j<1000;j++)
			for(uint32_t i=1; i <= 1000; i=i+1)
				tempfloat = tempfloat*i;
		time  = ((double)getTickCount() - time ) / getTickFrequency() * 1000;
		cout << tempfloat << "float timecost in ms: " << time << endl;

		uint32_t tempint = 1;
		for(int j=0;j<1000;j++)
			for(uint32_t i=1; i <= 1000000; i=i+1)
				tempint = tempint*i;
		time  = ((double)getTickCount() - time ) / getTickFrequency() * 1000;
		cout <<  tempint <<"int timecost in ms: " << time << endl;

		time = (double)getTickCount();
		tempfloat = 1;
		for(int j=0;j<1000;j++)
			for(uint32_t i=1; i <= 1000000; i=i+1)
				tempfloat = tempfloat*i;
		time  = ((double)getTickCount() - time ) / getTickFrequency() * 1000;
		cout << tempfloat << "float timecost in ms: " << time << endl;

		tempint = 1;
		for(int j=0;j<1000;j++)
			for(uint32_t i=1; i <= 1000000; i=i+1)
				tempint = tempint*i;
		time  = ((double)getTickCount() - time ) / getTickFrequency() * 1000;
		cout <<  tempint <<"int timecost in ms: " << time << endl;
	}
 */

