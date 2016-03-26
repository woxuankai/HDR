#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdint.h>


using namespace std;
using namespace cv;


/*
#include <math.h>
Mat cal_sigma(const Mat& src, float alpha, float beta)
{
    CV_Assert(src.depth() == CV_32F);
    CV_Assert(src.channels() == 1);
    Mat I = src.clone();
    //int channels = I.channels();
    int channels = 1;
    int nRows = I.rows;
    int nCols = I.cols * channels;

    if (I.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    int i,j;
    float* p;
    for( i = 0; i < nRows; ++i)
    {
        p = I.ptr<float>(i);
        for ( j = 0; j < nCols; ++j)
        {
        	if(p[j] <= 0)
        		p[j] = 0;
        	else
        		p[j] = pow(p[j],alpha)*beta;
        }
    }
    return I;
}
Mat cal_R(const Mat& src, float Rmax, float n, const Mat& sigma)
{
    CV_Assert(src.depth() == CV_32F);
    CV_Assert(src.channels() == 1);
    CV_Assert(sigma.depth() == CV_32F);
    CV_Assert(sigma.channels() == 1);

    Mat I = src.clone();
    int channels = 1;
    int nRows = I.rows;
    int nCols = I.cols * channels;

    if (I.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    int i,j;
    float* p;
    float* psigma;
    for( i = 0; i < nRows; ++i)
    {
        p = I.ptr<float>(i);
        psigma = sigma.ptr<float>(i);
        for ( j = 0; j < nCols; ++j)
        {
        	if(p[j] <= 0)
        		p[j] = 0;
        	else
        	{
        		float temppow = pow(p[j], n);
				p[j] = Rmax*temppow/(temppow + pow(psigma[j], n));
        	}
        }
    }
    return I;
}
*/

const float srcgain =  (1./(256.0*256.0));

void cal_Lcone_Lrod(const Mat& srcBGR, Mat& Lcone, Mat& Lrod);




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

	if(argc != 2)
	{
		cout << "usage:" << endl;
		cout << "./hdr imgfile" << endl;
		return -1;
	}
	Mat origImg = imread(argv[1],CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR );
	if (!origImg.data)
	{
		cout << "Unable to load image: " << argv[1] << endl;
		return -1;
	}


	////////single time work//////////
	Mat Lcone = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat Lrod = Mat::zeros(origImg.rows, origImg.cols, CV_32FC1);
	Mat hh = getGaussianKernel(21,1,CV_32F) - getGaussianKernel(21,4,CV_32F);
	/////////////end//////////////////

	////////temp///////////
	Mat imgout, imgin;
	origImg.convertTo(imgin, CV_32FC3, srcgain);
	////////end/////////

//int rpt = 5;
//while(rpt-- > 0)
//{

	double timestart = (double)getTickCount();
	double lasttime = timestart;

	//processImage
	//Mat imginXYZ;
	//cvtColor(imgin, imginXYZ, CV_BGR2XYZ);
	//Mat imgxyzs[3];
	//split(imginXYZ, imgxyzs);
	//see also mixChannels
	//Mat Lcone1,Lrod1;
	//Lcone1 = imgxyzs[1];
	//Lrod1 = -0.702*imgxyzs[0]\
	//		+1.039*imgxyzs[1]\
	//		+0.433*imgxyzs[2];
	cal_Lcone_Lrod(origImg, Lcone, Lrod);

TIMESTAMP(rod_cone);

	float arfa = 0.67;
	float beita = 4;
	float beita2 = 2;
	Mat sigma_cone;
	pow(Lcone, arfa, sigma_cone);
	sigma_cone = beita * sigma_cone;
	Mat sigma_rod;
	pow(Lrod, arfa, sigma_rod);
	sigma_rod = beita * sigma_rod;


	float n=0.8;
	float Rmax=2.5;
	Mat R_cone;
	pow(Lcone, n, R_cone);
	pow(sigma_cone, n, sigma_cone);
	R_cone = R_cone/(R_cone + sigma_cone);
	sigma_cone.release();//sigma is bad now, must release

TIMESTAMP(gensigma);

	Mat R_rod;
	pow(Lrod, n, R_rod);
	pow(sigma_rod, n, sigma_rod);
	R_rod = R_rod/(R_rod + sigma_rod);
	sigma_rod.release();//sigma is bad now, must release


	Mat DOG_cone, DOG_rod;
	filter2D(R_cone, DOG_cone, CV_32F, hh/*, borderType = BORDER_REFLECT_101*/);
	filter2D(R_rod, DOG_rod, CV_32F, hh/*, borderType = BORDER_REFLECT_101*/);

TIMESTAMP(dofilt);


	float KK=2.5;
	DOG_cone = R_cone + KK * DOG_cone;
	DOG_rod = R_rod + KK * DOG_rod;

	float t=0.1;
	Mat a;
	pow(Lcone, -t, a);
	double minvalue, maxvalue;
	minMaxLoc(a, &minvalue, &maxvalue);
	float minVal = minvalue;
	Mat w=1/(1 - minVal +a);

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

	multiply(w , DOG_cone, DOG_cone);
	multiply(1-w , DOG_rod, DOG_rod);
	Mat Lout =  DOG_cone + DOG_rod;

TIMESTAMP(Lout);

	Mat Lcone3c,Lout3c;
	cvtColor(Lcone, Lcone3c, CV_GRAY2BGR);
	cvtColor(Lout, Lout3c, CV_GRAY2BGR);

TIMESTAMP(meaningless);

	float s=0.8;
	divide(imgin, Lcone3c, imgout);
TIMESTAMP(div);
	pow(imgout , s, imgout);
TIMESTAMP(pow);
	multiply(imgout , Lout3c, imgout);
TIMESTAMP(mul);

	timestart  = ((double)getTickCount() - timestart ) / getTickFrequency() * 1000;
	cout << "time cost : "<< timestart << endl;

//}

	//namedWindow("origin image", WINDOW_AUTOSIZE);
	//imshow("origin image", imgin );
	//namedWindow("Output", WINDOW_AUTOSIZE);
	//imshow("Output", imgout);
	//waitKey(0);
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
        	//Lcone = 0.2127*LinR + 0.7152*LinG + 0.0722*LinB;
        	//Lrod = -0.0602*LinR + 0.5436*LinG + 0.3598*LinB;
        	pcone[j] = srcgain*0.0722*psrc[srcj]\
        			+ srcgain*0.7152*psrc[srcj+1]\
					+ srcgain*0.2127*psrc[srcj+2];
        	prod[j] = srcgain*0.3598*psrc[srcj]\
        			+ srcgain*0.5436*psrc[srcj+1]\
					- srcgain*0.0602*psrc[srcj+2];
        }
    }
}



