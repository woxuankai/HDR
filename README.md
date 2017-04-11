#	HDR
基于人视觉构造高性能实现(移植)HDR算法

##	file layout
*	cpp (my main work)  
	C++ implementation of hdr algorithm, with parallel computing using producer–consumer synchronization
*	matlab  
	matlab version of hdr algorithm, from PuXuan's master's thesis
*	python  
	python implementation of hdr algorithm, just for transformation from matlab functions to opencv, quiet slow
*	reference  
	references, including PuXuan's master's thesis
*	testiamges  
	well, test images

## dependency
*	CMake
*	C++11 compatible compiler
*	OpenCV
