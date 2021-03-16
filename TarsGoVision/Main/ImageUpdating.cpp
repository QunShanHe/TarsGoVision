/*
*	@Author: Qunshan He,mountain.he@qq.com
*	@Date:	 2021.03.16
*	@Brief:  Image Acquiring thread function
*/

#include <opencv2/opencv.hpp>
#include <General/General.h>
#include <GxCamera/GxCamera.h>

using namespace cv;

int imageUpdatingThread()
{
	GX_STATUS status = GX_STATUS_SUCCESS;

	/*
	 *Preparation: CvMat image content
	*/
	cv::Mat frame;

	/*
	 *First init: Implementation of GxCamera and init it
	*/
	GxCamera gxCam;
	status = gxCam.initLib();
	GX_VERIFY(status);

	/*
	 *Second init: Open Camera by SN/Index
	*/
	status = gxCam.openDeviceBySN("KJ0180110048");	//By SN
	//status = gxCam.openDeviceByIndex("1");		//By Index
	GX_VERIFY(status);

	/*
	 *Third init: Set Camera Params: ROI, Exposure, Gain, WhiteBalance
	*/
	gxCam.setRoiParam(640, 480, 80, 120);				// ROI
	gxCam.setExposureParam(2000, false, 1000, 3000);	// Exposure
	gxCam.setGainParam(0, false, 0, 10);				// Gain
	gxCam.setWhiteBalanceOn(true);						// WhiteBalance

	/*
	 *Before acq: Send Acquisition Start Command
	*/
	status = gxCam.startAcquiring();					// Send Start Acquisition Command
	GX_VERIFY(status);

	while (true)
	{
		// FPS
		double t = getTickCount();
		/*
		 *In acq: Snap a CvMat Image and store it in CvMat Content
		*/
		status = gxCam.snapCvMat(frame);
		GX_VERIFY(status);
		// Update the image acquired to src Mat content
		if (1) {
			std::unique_lock <std::mutex> lck(Globalmutex);
			frame.copyTo(src);
			Mat show;
			frame.copyTo(show);
            //imshow("frame", show);
			imageReadable = true;
			GlobalCondCV.notify_one();
		}
		char chKey = waitKey(1);
		if (chKey == 'w' || chKey == 'W')
			break;
		//FPS
		double t1 = (getTickCount() - t) / getTickFrequency();
		printf("Image Acquiring FPS: %f\n", 1 / t1);
	}

	/*
	 *After acq: Send Acquisition Stop Command
	*/
	status = gxCam.stopAcquiring();
	GX_VERIFY(status);

	/*
	*Close camera, while you can still open a new different camera
	*/
	gxCam.closeDevice();

	/*
	*Close lib: you can not use any GxCamera device unless you initLib() again
	*/
	gxCam.closeLib();
}
