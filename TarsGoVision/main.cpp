#include"GxCamera.h"

int main(int argc, char* argv[]) {

	GX_STATUS status = GX_STATUS_SUCCESS;

	/*
	 *Preparation: CvMat image content
	*/
	Mat frame;

	/*
	 *First init: Implementation of GxCamera and init it
	*/
	GxCamera gxCam;
	status = gxCam.initLib();
	GX_VERIFY(status);

	/*
	 *Second init: Open Camera by SN/Index
	*/
	status = gxCam.openDeviceBySN("NV0200090084");	//By SN
	status = gxCam.openDeviceByIndex("1");			//By Index
	GX_VERIFY(status);
	
	/*
	 *Third init: Set Camera Params: ROI, Exposure, Gain, WhiteBalance
	*/
	gxCam.setRoiParam(1024, 1024, 0, 0);				// ROI
	gxCam.setExposureParam(3000, true, 1000, 10000);	// Exposure
	gxCam.setGainParam(0, true, 0, 10);					// Gain
	gxCam.setWhiteBalanceOn(true);						// WhiteBalance

	/*
	 *Before acq: Send Acquisition Start Command
	*/
	status = gxCam.startAcquiring();					// Send Start Acquisition Command
	GX_VERIFY(status);
	
	while (true)
	{
		/*
		 *In acq: Snap a CvMat Image and store it in CvMat Content
		*/
		status = gxCam.snapCvMat(frame);				// Snap an image and return as CvMat Foramt
		GX_VERIFY(status);
		imshow("frame", frame);
		char chKey = waitKey(1);
		if (chKey == 27) break;
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

	system("pause");
	return 0;
}