#include "GxCamera.h"

using namespace cv;
using namespace std;

GxCamera::GxCamera()
{
	camHandle_		= NULL;				// 相机句柄
	roiParam_		= RoiParam();		// Roi参数
	exposureParam_	= ExposureParam();	// 曝光参数
	gainParam_		= GainParam();		// 增益参数

	isAcquiring_	= false;	// 是否正在采集
	isColorCam_		= false;	// 是否为彩色相机

	gxFrame_		= GX_FRAME_DATA();		// 大恒GxFrame
	cvImage_		= Mat();		// CvMat Image
}

GxCamera::~GxCamera()
{
}

GX_STATUS GxCamera::initLib()
{
	GX_STATUS status = GX_STATUS_SUCCESS;
	printf("\n");
	printf("-------------------------------------------------------------\n");
	printf("Daheng Galaxy Camera C++ drive startup...\n");
	printf("version: 2.0\n");
	printf("-------------------------------------------------------------\n");
	printf("\n");
	printf("Initializing......");
	printf("\n\n");
	//Initialize libary
	status = GXInitLib();
	GX_VERIFY(status);

	return status;
}

GX_STATUS GxCamera::closeLib()
{
	GX_STATUS status = GX_STATUS_SUCCESS;
	printf("\n");
	printf("-------------------------------------------------------------\n");
	printf("Daheng Galaxy Camera C++ drive closing...\n");
	printf("GxCamera Drive Exit!");
	printf("-------------------------------------------------------------\n");
	printf("\n");
	//Close libary
	status = GXCloseLib();
	GX_VERIFY(status);

	return status;
}

GX_STATUS GxCamera::openDeviceBySN(string cameraSN)
{
	GX_STATUS status = GX_STATUS_SUCCESS;

	if (camHandle_ != NULL)
	{
		closeDevice();
		GX_VERIFY(status);
	}

	GX_OPEN_PARAM stOpenParam;
	stOpenParam.accessMode = GX_ACCESS_EXCLUSIVE;
	stOpenParam.openMode = GX_OPEN_SN;
	stOpenParam.pszContent = const_cast<char*>(cameraSN.c_str());

	status = GXOpenDevice(&stOpenParam, &camHandle_);
	GX_VERIFY(status);

	status = readCameraParams();
	GX_VERIFY(status);

	return status;
}

GX_STATUS GxCamera::openDeviceByIndex(string camIndex)
{
	GX_STATUS status = GX_STATUS_SUCCESS;

	if (camHandle_ != NULL)
	{
		status = closeDevice();
		GX_VERIFY(status);
	}

	GX_OPEN_PARAM stOpenParam;
	stOpenParam.accessMode = GX_ACCESS_EXCLUSIVE;
	stOpenParam.openMode = GX_OPEN_INDEX;
	stOpenParam.pszContent = const_cast<char*>(camIndex.c_str());

	status = GXOpenDevice(&stOpenParam, &camHandle_);
	GX_VERIFY(status);

	status = readCameraParams();
	GX_VERIFY(status);

	return status;
}

GX_STATUS GxCamera::closeDevice()
{
	GX_STATUS status = GX_STATUS_SUCCESS;
	bool confSavable = false;
	status = GXIsImplemented(camHandle_, GX_ENUM_USER_SET_DEFAULT, &confSavable);
	GX_VERIFY(status);
	if (confSavable == true)
	{
		status = GXSetEnum(camHandle_, GX_ENUM_USER_SET_SELECTOR, GX_ENUM_USER_SET_SELECTOR_USERSET0);
		GX_VERIFY(status);
		status = GXSendCommand(camHandle_, GX_COMMAND_USER_SET_SAVE);
		GX_VERIFY(status);
		status = GXSetEnum(camHandle_, GX_ENUM_USER_SET_DEFAULT, GX_ENUM_USER_SET_DEFAULT_USERSET0);
		GX_VERIFY(status);
	}
	status = GXCloseDevice(camHandle_);
	GX_VERIFY(status);
	camHandle_ = NULL;
	return status;
}

GX_STATUS GxCamera::startAcquiring()
{
	GX_STATUS status = GX_STATUS_SUCCESS;

	//载入ROI参数到相机中
	status = loadRoi();
	GX_VERIFY(status);
	//载入曝光参数到相机中
	status = loadExposure();
	GX_VERIFY(status);
	//载入增益参数到相机中
	status = loadGain();
	GX_VERIFY(status);
	//申请图像内存空间
	status = allocateImageBuf();
	GX_VERIFY(status);
	//发送开始采集命令
	status = GXSendCommand(camHandle_, GX_COMMAND_ACQUISITION_START);
	GX_VERIFY(status);
	
	//采集开启
	isAcquiring_ = true;

	return status;
}

GX_STATUS GxCamera::stopAcquiring()
{
	GX_STATUS status = GX_STATUS_SUCCESS;

	//发送停止采集命令
	status = GXSendCommand(camHandle_, GX_COMMAND_ACQUISITION_STOP);
	GX_VERIFY(status);

	//释放图像缓冲区buffer
	free(gxFrame_.pImgBuf);
	gxFrame_.pImgBuf = NULL;

	//采集关闭
	isAcquiring_ = false;

	return status;
}

GX_STATUS GxCamera::setWhiteBalanceOn(bool whiteBalance)
{
	GX_STATUS status = GX_STATUS_SUCCESS;

	if (whiteBalance)
	{
		status = GXSetEnum(camHandle_, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_GREEN);

		//设置自动白平衡感兴趣区域(整个roi)
		status = GXSetInt(camHandle_, GX_INT_AWBROI_WIDTH, roiParam_.width);
		status = GXSetInt(camHandle_, GX_INT_AWBROI_HEIGHT, roiParam_.height);
		status = GXSetInt(camHandle_, GX_INT_AWBROI_OFFSETX, roiParam_.offsetX);
		status = GXSetInt(camHandle_, GX_INT_AWBROI_OFFSETY, roiParam_.offsetY);
		GX_VERIFY(status);

		//默认为自适应的光源
		status = GXSetEnum(camHandle_, GX_ENUM_AWB_LAMP_HOUSE, GX_AWB_LAMP_HOUSE_ADAPTIVE);
		GX_VERIFY(status);

		//默认为连续自动白平衡
		status = GXSetEnum(camHandle_, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_CONTINUOUS);
		GX_VERIFY(status);
	}
	else
	{
		//关闭自动白平衡
		status = GXSetEnum(camHandle_, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF);
		GX_VERIFY(status);
	}
	return status;
}

GX_STATUS GxCamera::snapCvMat(Mat & dstCvMat)
{
	GX_STATUS status = GX_STATUS_SUCCESS;
	status = GXGetImage(camHandle_, &gxFrame_, 500);
	GX_VERIFY(status);

	// 将GxFrame转换为CvMat
	cvtGxFrameToCvMat(gxFrame_, dstCvMat);

	return status;
}

void GxCamera::setRoiParam(int64_t width, int64_t height, int64_t offsetX, int64_t offsetY)
{
	roiParam_.width = width;
	roiParam_.height = height;
	roiParam_.offsetX = offsetX;
	roiParam_.offsetY = offsetY;
}

void GxCamera::setExposureParam(double exposureTimeUs, bool autoExposure, double autoExposureTimeMinUs, double autoExposureTimeMaxUs)
{
	exposureParam_.exposureTimeUs = exposureTimeUs;
	exposureParam_.autoExposure = autoExposure;
	exposureParam_.autoExposureTimeMinUs = autoExposureTimeMinUs;
	exposureParam_.autoExposureTimeMaxUs = autoExposureTimeMaxUs;
}

void GxCamera::setGainParam(double gainDb, bool autoGain, double autoGainMinDb, double autoGainMaxDb)
{
	gainParam_.gainDb = gainDb;
	gainParam_.autoGain = autoGain;
	gainParam_.autoGainMinDb = autoGainMinDb;
	gainParam_.autoGainMaxDb = autoGainMaxDb;
}

GX_STATUS GxCamera::loadRoi()
{
	GX_STATUS status = GX_STATUS_SUCCESS;
	// 优化ROI参数
	status = GxOptimizeRoiParam();
	//设 置 一 个 offset 偏 移 为 (X,Y) ,Width * Height 尺 寸 的 区 域
	status = GXSetInt(camHandle_, GX_INT_OFFSET_X, 0);
	GX_VERIFY(status);
	status = GXSetInt(camHandle_, GX_INT_OFFSET_Y, 0);
	GX_VERIFY(status);
	status = GXSetInt(camHandle_, GX_INT_WIDTH, roiParam_.width);
	GX_VERIFY(status);
	status = GXSetInt(camHandle_, GX_INT_HEIGHT, roiParam_.height);
	GX_VERIFY(status);
	status = GXSetInt(camHandle_, GX_INT_OFFSET_X, roiParam_.offsetX);
	GX_VERIFY(status);
	status = GXSetInt(camHandle_, GX_INT_OFFSET_Y, roiParam_.offsetY);
	GX_VERIFY(status);

	return GX_STATUS_SUCCESS;
}

GX_STATUS GxCamera::loadExposure()
{
	GX_STATUS status = GX_STATUS_SUCCESS;
	if (exposureParam_.autoExposure == true)
	{
		status = GXSetEnum(camHandle_, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_CONTINUOUS);
		GX_VERIFY(status);
		status = GXSetFloat(camHandle_, GX_FLOAT_AUTO_EXPOSURE_TIME_MIN, exposureParam_.autoExposureTimeMinUs);
		GX_VERIFY(status);
		status = GXSetFloat(camHandle_, GX_FLOAT_AUTO_EXPOSURE_TIME_MAX, exposureParam_.autoExposureTimeMaxUs);
		GX_VERIFY(status);
	}
	else
	{
		status = GXSetEnum(camHandle_, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);
		GX_VERIFY(status);
		status = GXSetEnum(camHandle_, GX_ENUM_EXPOSURE_MODE, GX_EXPOSURE_MODE_TIMED);
		GX_VERIFY(status);
		status = GXSetFloat(camHandle_, GX_FLOAT_EXPOSURE_TIME, exposureParam_.exposureTimeUs);
		GX_VERIFY(status);
	}
	return status;
}

GX_STATUS GxCamera::loadGain()
{
	GX_STATUS status = GX_STATUS_SUCCESS;
	if (gainParam_.autoGain == true)
	{
		status = GXSetEnum(camHandle_, GX_ENUM_GAIN_AUTO, GX_GAIN_AUTO_CONTINUOUS);
		GX_VERIFY(status);
		status = GXSetFloat(camHandle_, GX_FLOAT_AUTO_GAIN_MIN, gainParam_.autoGainMinDb);
		GX_VERIFY(status);
		status = GXSetFloat(camHandle_, GX_FLOAT_AUTO_GAIN_MAX, gainParam_.autoGainMaxDb);
		GX_VERIFY(status);
	}
	else
	{
		status = GXSetEnum(camHandle_, GX_ENUM_GAIN_AUTO, GX_GAIN_AUTO_OFF);
		GX_VERIFY(status);
		status = GXSetEnum(camHandle_, GX_ENUM_GAIN_SELECTOR, GX_GAIN_SELECTOR_ALL);
		GX_VERIFY(status);
		status = GXSetFloat(camHandle_, GX_FLOAT_GAIN, gainParam_.gainDb);
		GX_VERIFY(status);
	}
	return status;
}

GX_STATUS GxCamera::GxSetPixelFormat8bit()
{
	uint32_t  i = 0;
	int64_t   nPixelSize = 0;
	uint32_t  nEnmuEntry = 0;
	size_t    nBufferSize = 0;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	GX_ENUM_DESCRIPTION  *pEnumDescription = NULL;
	GX_ENUM_DESCRIPTION  *pEnumTemp = NULL;

	// 获取像素位深大小
	emStatus = GXGetEnum(camHandle_, GX_ENUM_PIXEL_SIZE, &nPixelSize);
	if (emStatus != GX_STATUS_SUCCESS)	return emStatus;

	// 判断为8Bit时直接返回
	if (nPixelSize == GX_PIXEL_SIZE_BPP8)
	{
		return GX_STATUS_SUCCESS;
	}

	// 获取设备支持的像素格式的枚举项个数
	emStatus = GXGetEnumEntryNums(camHandle_, GX_ENUM_PIXEL_FORMAT, &nEnmuEntry);
	if (emStatus != GX_STATUS_SUCCESS)	return emStatus;

	// 为获取设备支持的像素格式枚举值准备资源
	nBufferSize = nEnmuEntry * sizeof(GX_ENUM_DESCRIPTION);
	pEnumDescription = new GX_ENUM_DESCRIPTION[nEnmuEntry];

	// 获取支持的枚举值
	emStatus = GXGetEnumDescription(camHandle_, GX_ENUM_PIXEL_FORMAT, pEnumDescription, &nBufferSize);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		// 释放资源
		if (pEnumDescription != NULL)
		{
			delete[]pEnumDescription;
			pEnumDescription = NULL;
		}
		return emStatus;
	}

	// 遍历设备支持的像素格式,设置像素格式为8Bit
	pEnumTemp = pEnumDescription;
	for (i = 0; i < nEnmuEntry; i++)
	{
		if ((pEnumTemp->nValue & GX_PIXEL_8BIT) == GX_PIXEL_8BIT)
		{
			emStatus = GXSetEnum(camHandle_, GX_ENUM_PIXEL_FORMAT, pEnumTemp->nValue);
			break;
		}
		pEnumTemp++;
	}

	// 释放资源
	if (pEnumDescription != NULL)
	{
		delete[]pEnumDescription;
		pEnumDescription = NULL;
	}

	return emStatus;
}

GX_STATUS GxCamera::allocateImageBuf()
{
	GX_STATUS status = GX_STATUS_SUCCESS;

	//为GxFrame的ImgBuf申请内存空间
	int64_t nPayLoadSize = 0;
	status = GXGetInt(camHandle_, GX_INT_PAYLOAD_SIZE, &nPayLoadSize);
	if (status != GX_STATUS_SUCCESS && nPayLoadSize <= 0)	return status;
	gxFrame_.pImgBuf = malloc((size_t)nPayLoadSize);

	//为CvImage预申请内存空间
	cvImage_ = isColorCam_ ? Mat(roiParam_.height, roiParam_.width, CV_8UC3) : Mat(roiParam_.height, roiParam_.width, CV_8UC1);

	return status;
}

GX_STATUS GxCamera::readCameraParams()
{
	GX_STATUS status = GX_STATUS_SUCCESS;

	// Camera color filter whether it is a color camera.
	status = GXIsImplemented(camHandle_, GX_ENUM_PIXEL_COLOR_FILTER, &isColorCam_);
	GX_VERIFY(status);

	// Camera exposure
	status = GXGetFloat(camHandle_, GX_FLOAT_EXPOSURE_TIME, &exposureParam_.exposureTimeUs);
	GX_VERIFY(status);
	int64_t autoExposureValue = 0;
	status = GXGetEnum(camHandle_, GX_ENUM_EXPOSURE_AUTO, &autoExposureValue);
	exposureParam_.autoExposure = autoExposureValue == GX_EXPOSURE_AUTO_OFF ? false : true;
	GX_VERIFY(status);
	status = GXGetFloat(camHandle_, GX_FLOAT_AUTO_EXPOSURE_TIME_MIN, &exposureParam_.autoExposureTimeMinUs);
	GX_VERIFY(status);
	status = GXGetFloat(camHandle_, GX_FLOAT_AUTO_EXPOSURE_TIME_MAX, &exposureParam_.autoExposureTimeMaxUs);
	GX_VERIFY(status);

	// Camera gain
	status = GXGetFloat(camHandle_, GX_FLOAT_GAIN, &gainParam_.gainDb);
	GX_VERIFY(status);
	int64_t autoGainValue = 0;
	status = GXGetEnum(camHandle_, GX_ENUM_GAIN_AUTO, &autoGainValue);
	gainParam_.autoGain = autoGainValue == GX_GAIN_AUTO_OFF ? false : true;
	GX_VERIFY(status);
	status = GXGetFloat(camHandle_, GX_FLOAT_AUTO_GAIN_MIN, &gainParam_.autoGainMinDb);
	GX_VERIFY(status);
	status = GXGetFloat(camHandle_, GX_FLOAT_AUTO_GAIN_MAX, &gainParam_.autoGainMaxDb);
	GX_VERIFY(status);

	// Roi Params
	status = GXGetInt(camHandle_, GX_INT_WIDTH, &roiParam_.width);
	GX_VERIFY(status);
	status = GXGetInt(camHandle_, GX_INT_HEIGHT, &roiParam_.height);
	GX_VERIFY(status);
	status = GXGetInt(camHandle_, GX_INT_OFFSET_X, &roiParam_.offsetX);
	GX_VERIFY(status);
	status = GXGetInt(camHandle_, GX_INT_OFFSET_Y, &roiParam_.offsetY);
	GX_VERIFY(status);

	return status;
}

void GxCamera::cvtGxFrameToCvMat(GX_FRAME_DATA & srcFrame, Mat & dstMat)
{
	int rows = srcFrame.nHeight;
	int cols = srcFrame.nWidth;
	int cvType = CV_8UC1;
	int bayerType = BAYERRG;

	switch (srcFrame.nPixelFormat)
	{
	case GX_PIXEL_FORMAT_MONO8:			cvType = CV_8UC1; break;
	case GX_PIXEL_FORMAT_MONO8_SIGNED:	cvType = CV_8SC1; break;
	case GX_PIXEL_FORMAT_BAYER_GR8:		cvType = CV_8UC3; bayerType = BAYERGR; break;
	case GX_PIXEL_FORMAT_BAYER_RG8:		cvType = CV_8UC3; bayerType = BAYERRG; break;
	case GX_PIXEL_FORMAT_BAYER_GB8:		cvType = CV_8UC3; bayerType = BAYERGB; break;
	case GX_PIXEL_FORMAT_BAYER_BG8:		cvType = CV_8UC3; bayerType = BAYERBG; break;
	default:											  break;
	}

	if (isColorCam_ == true)
	{
		DxRaw8toRGB24(srcFrame.pImgBuf, cvImage_.data,
			srcFrame.nWidth, srcFrame.nHeight,
			RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(bayerType), false);
	}
	else
	{
		//cvImage_.data = (uchar*)srcFrame.pImgBuf;
		memcpy(cvImage_.data, srcFrame.pImgBuf, srcFrame.nImgSize);
	}
	// 将CvMat拷贝到dstMatch中
	cvImage_.copyTo(dstMat);
}

GX_STATUS GxCamera::GxOptimizeRoiParam()
{
	GX_STATUS status;
	GX_INT_RANGE xRange, yRange, widthRange, heightRange;
	int64_t maxWidth, maxHeight;
	status = GXGetIntRange(camHandle_, GX_INT_OFFSET_X, &xRange);
	GX_VERIFY(status);
	status = GXGetIntRange(camHandle_, GX_INT_OFFSET_Y, &yRange);
	GX_VERIFY(status);
	status = GXGetIntRange(camHandle_, GX_INT_WIDTH, &widthRange);
	GX_VERIFY(status);
	status = GXGetIntRange(camHandle_, GX_INT_HEIGHT, &heightRange);
	GX_VERIFY(status);
	status = GXGetInt(camHandle_, GX_INT_WIDTH_MAX, &maxWidth);
	GX_VERIFY(status);
	status = GXGetInt(camHandle_, GX_INT_HEIGHT_MAX, &maxHeight);
	GX_VERIFY(status);

	// Optimize width
	if (roiParam_.width > maxWidth) roiParam_.width = maxWidth;
	if (roiParam_.width < widthRange.nMin) roiParam_.width = widthRange.nMin;
	roiParam_.width = (roiParam_.width / widthRange.nInc)*widthRange.nInc;

	// Optimize height
	if (roiParam_.height > maxHeight) roiParam_.height = maxHeight;
	if (roiParam_.height < heightRange.nMin) roiParam_.height = heightRange.nMin;
	roiParam_.height = (roiParam_.height / heightRange.nInc)*heightRange.nInc;

	// Optimize x
	if (roiParam_.offsetX > maxWidth - roiParam_.width) roiParam_.offsetX = maxWidth - roiParam_.width;
	if (roiParam_.offsetX < 0) roiParam_.offsetX = 0;
	roiParam_.offsetX = (roiParam_.offsetX / xRange.nInc)*xRange.nInc;

	// Optimize y
	if (roiParam_.offsetY > maxHeight - roiParam_.height) roiParam_.offsetY = maxHeight - roiParam_.height;
	if (roiParam_.offsetY < 0) roiParam_.offsetY = 0;
	roiParam_.offsetY = (roiParam_.offsetY / yRange.nInc)*yRange.nInc;
}

void GetErrorString(GX_STATUS errorStatus)
{
	char *error_info = NULL;
	size_t size = 0;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;

	// Get length of error description
	emStatus = GXGetLastError(&errorStatus, NULL, &size);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		printf("<Error when calling GXGetLastError>\n");
		return;
	}

	// Alloc error resources
	error_info = new char[size];
	if (error_info == NULL)
	{
		printf("<Failed to allocate memory>\n");
		return;
	}

	// Get error description
	emStatus = GXGetLastError(&errorStatus, error_info, &size);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		printf("<Error when calling GXGetLastError>\n");
	}
	else
	{
		printf("%s\n", (char*)error_info);
	}

	// Realease error resources
	if (error_info != NULL)
	{
		delete[]error_info;
		error_info = NULL;
	}
}
