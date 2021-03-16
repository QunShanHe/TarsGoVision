#ifndef GXCAMERA_H
#define GXCAMERA_H

#include<GxIAPI.h>
#include<DxImageProc.h>
#include<opencv2/opencv.hpp>
#include<iostream>
#include<vector>

using namespace std;

//Show error message
#define GX_VERIFY(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS)     \
    {                                      \
        GetErrorString(emStatus);          \
        return emStatus;                   \
    }

//Get error message and show it
void GetErrorString(GX_STATUS errorStatus);

// ROI Param struct
typedef struct RoiParam {
	int64_t              width;                //< Image roi width
	int64_t              height;               //< Image roi height
	int64_t              offsetX;              //< OffsetX of roi
	int64_t              offsetY;              //< OffsetY of roi
	RoiParam()
	{
		width	= 400;
		height	= 300;
		offsetX = 0;
		offsetY = 0;
	}
}RoiParam;

// Exposure Param struct
typedef struct ExposureParam
{
	double               exposureTimeUs;		//< Exposure Time
	bool                 autoExposure;			//< Exposure is auto mode or not
	double               autoExposureTimeMinUs;	//< Minimum exposure time when using AutoExporsureTime mode
	double               autoExposureTimeMaxUs;	//< Maximum exposure time when using AutoExporsureTime mode
	ExposureParam()
	{
		exposureTimeUs			= 2000;
		autoExposure			= false;
		autoExposureTimeMinUs	= 1000;
		autoExposureTimeMaxUs	= 10000;
	}
}ExposureParam;


// Gain Param struct
typedef struct GainParam
{
	double               gainDb;				//< Gain db
	bool                 autoGain;				//< Gain is auto mode or not
	double               autoGainMinDb;			//< Minimum gain when using AutoGain mode
	double               autoGainMaxDb;			//< Maximum gain when using AutoGain mode
	GainParam()
	{
		gainDb			= 0;
		autoGain		= false;
		autoGainMinDb	= 0;
		autoGainMaxDb	= 10;
	}
}GainParam;

class GxCamera
{
public:
	GxCamera();
	~GxCamera();

	/**
	* @beief: 初始化SDK库，在使用设备之前必须初始化库
	* @return: 获取状态
	*/
	GX_STATUS initLib();

	/**
	* @beief: 关闭SDK库，不再使用任何大恒SDK库之后则关闭Lib释放资源
	* @return: 获取状态
	*/
	GX_STATUS closeLib();

	/**
	* @beief: 根据相机SN号打开对应设备
	* @param: cameraSN  相机SN号
	* @return: 获取状态
	*/
	GX_STATUS openDeviceBySN(string cameraSN);

	/**
	* @beief: 根据相机索引Index打开对应设备
	* @param: camIndex  相机索引Index
	* @return: 获取状态
	*/
	GX_STATUS openDeviceByIndex(string camIndex);

	/**
	* @beief: 关闭当前打开的相机设备
	* @return: 获取状态
	*/
	GX_STATUS closeDevice();

	/**
	* @beief: 开始采集
	* @return: 获取状态
	*/
	GX_STATUS startAcquiring();

	/**
	* @beief: 停止采集
	* @return: 获取状态
	*/
	GX_STATUS stopAcquiring();
	
	/**
	* @beief: 设置白平衡参数
	* @param: whiteBalance 是否开启白平衡
	* @return: 获取状态
	*/
	GX_STATUS setWhiteBalanceOn(bool whiteBalance);

	/**
	* @beief: 采集一张图片(CvMat格式)
	* @param: 目标CvMat图像容器
	* @return: 获取状态
	*/
	GX_STATUS snapCvMat(cv::Mat & dstCvMat);

	/**
	* @beief: 设置ROI参数
	* @param: width  ROI的宽度
	* @param: height  ROI高度
	* @param: offsetX  ROI左上角X坐标
	* @param: offsetY  ROI左上角Y坐标
	* @return: 获取状态
	*/
	void setRoiParam(	int64_t width,
						int64_t height,
						int64_t offsetX,
						int64_t offsetY);

	/**
	* @beief: 设置曝光参数
	* @param: exposureTimeUs  定时曝光值，单位微秒
	* @param: autoExposure  是否开启自动曝光
	* @param: autoExposureTimeMinUs  自动曝光最小值，单位Us
	* @param: AutoExposureTimeMaxUs  自动曝光最大值，单位Us
	* @return: 获取状态
	*/
	void setExposureParam(	double exposureTimeUs,
							bool autoExposure,
							double autoExposureTimeMinUs,
							double autoExposureTimeMaxUs);

	/**
	* @beief: 设置增益参数
	* @param: gainDb  定值增益值，单位db
	* @param: autoGain  是否开启自动增益
	* @param: autoGainMinDb  自动增益最小值，单位db
	* @param: autoGainMaxDb  自动增益最大值，单位db
	* @return: 获取状态
	*/
	void setGainParam(	double gainDb,
						bool autoGain,
						double autoGainMinDb,
						double autoGainMaxDb);
	
private:
	GX_DEV_HANDLE	camHandle_;		// 相机句柄
	RoiParam		roiParam_;		// Roi参数
	ExposureParam	exposureParam_;	// 曝光参数
	GainParam		gainParam_;		// 增益参数
	
	bool			isAcquiring_;	// 是否正在采集
	bool			isColorCam_;	// 是否为彩色相机

	GX_FRAME_DATA	gxFrame_;		// 大恒GxFrame
	cv::Mat			cvImage_;		// CvMat Image

	/**
	* @beief: 载入设置的ROI参数到相机中
	* @return: 获取状态
	*/
	GX_STATUS loadRoi();

	/**
	* @beief: 载入设置的曝光参数到相机中
	* @return: 获取状态
	*/
	GX_STATUS loadExposure();

	/**
	* @beief: 载入设置的增益参数到相机中
	* @return: 获取状态
	*/
	GX_STATUS loadGain();

	/**
	* @beief: 设置相机的数据格式为8bit
	* @return: 获取状态
	*/
	GX_STATUS GxSetPixelFormat8bit();

	/**
	* @beief: 重新申请图像内存空间大小
	* @return: 获取状态
	*/
	GX_STATUS allocateImageBuf();

	/**
	* @beief: 读取开启相机的参数
	* @return: 获取状态
	*/
	GX_STATUS readCameraParams();

	/**
	* @beief: 优化设置的ROI参数，使之能符合相机ROI需求
	* @return: 获取状态
	*/
	GX_STATUS GxOptimizeRoiParam();

	/**
	* @beief: 将GxFrame图像转换为Mat图像
	* @param: srcFrame GxFrame源图像
	* @param: dstMat CvMat目标图像
	* @return: void
	*/
	void cvtGxFrameToCvMat(GX_FRAME_DATA & srcFrame, cv::Mat & dstMat);
};

#endif // !GXCAMERA_H