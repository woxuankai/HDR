#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdint.h>
#include <limits.h>

using namespace std;
using namespace cv;


void cal_Lcone_Lrod(const Mat& srcBGR, Mat& Lcone, Mat& Lrod);
void cal_R_matrix(const Mat& L, Mat& R,\
		float beta_pow_n,\
		uint16_t* table_n, uint16_t *table_alpha_mul_n,\
		float R_max);
void cal_normalized_pow_table(uint16_t* table, double exp);

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
	///////////end///////////////////
	////////one-time work//////////
	Mat L_cone = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat L_rod = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat R_cone = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat R_rod = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);

	Mat hh = getGaussianKernel(21,1,CV_32F) - getGaussianKernel(21,4,CV_32F);

	float beta_cone_pow_n = pow(beta_cone,n);
	float beta_rod_pow_n = pow(beta_rod,n);

	uint16_t table_alpha_mul_n[UINT16_MAX+1] = {0};
	uint16_t table_n[UINT16_MAX+1] = {0};
	cal_normalized_pow_table(table_alpha_mul_n, alpha*n);
	cal_normalized_pow_table(table_n, n);

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

	cal_R_matrix(L_cone, R_cone,\
			beta_cone_pow_n,table_n,table_alpha_mul_n, R_max);
	cal_R_matrix(L_rod, R_rod,\
			beta_rod_pow_n,table_n,table_alpha_mul_n, R_max);
TIMESTAMP(genR);

	Mat DOG_cone, DOG_rod;
	filter2D(R_cone, DOG_cone, CV_32F, hh);
	filter2D(R_rod, DOG_rod, CV_32F, hh);
TIMESTAMP(dofilt);

	double minvalue, maxvalue;
	float KK=2.5;
	DOG_cone = R_cone + KK * DOG_cone;
	DOG_rod = R_rod + KK * DOG_rod;

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
TIMESTAMP(DOG);

	float t=0.1;
	Mat a;
	pow(L_cone, -t, a);
	minMaxLoc(a, &minvalue, &maxvalue);
	float minVal = minvalue;
	Mat w=1/(1 - minVal +a);
TIMESTAMP(cal omega);

	multiply(w , DOG_cone, DOG_cone);
	multiply(1-w , DOG_rod, DOG_rod);
	Mat Lout =  DOG_cone + DOG_rod;
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
	TIMESTAMP(mul);

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



void cal_R_matrix(const Mat& L, Mat& R,\
		float beta_pow_n,\
		uint16_t* table_n, uint16_t *table_alpha_mul_n,\
		float R_max)
{
    CV_Assert(L.depth() == CV_32F);
    CV_Assert(L.channels() == 1);
    CV_Assert(L.data != NULL);

    CV_Assert(R.depth() == CV_32F);
    CV_Assert(R.channels() == 1);
    CV_Assert(R.data != L.data);
    CV_Assert(R.size == L.size);

    CV_Assert(table_n != NULL);
    CV_Assert(table_alpha_mul_n != NULL);

    int channels = 3;
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
        	uint32_t L_pow_n = table_n[index];
        	uint32_t L_pow_alpha_mul_n = table_alpha_mul_n[index];
        	pR[j] = R_max*L_pow_n /\
        			(L_pow_n + \
        			L_pow_alpha_mul_n*beta_pow_n +\
					FLT_MIN);
        }
    }
	return;
}

void cal_normalized_pow_table(uint16_t* table, double exp)
{
	CV_Assert(table != NULL);
	double scale = UINT16_MAX/pow(UINT16_MAX,exp);
	for(uint32_t index = 0; index <= UINT16_MAX; index++)
		table[index] = pow(index, exp)*scale;
	return;
}


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

