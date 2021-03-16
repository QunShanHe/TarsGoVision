/*
*	@Author: Qunshan He,mountain.he@qq.com
*	@Date:	 2021.03.16
*	@Brief:  This header file include the common head files and define the common structure, function and global variable.
*/

#ifndef GENERAL_H
#define GENERAL_H

#include<thread>
#include<mutex>
#include<condition_variable>
#include<opencv2/opencv.hpp>
#include<iostream>
#include<math.h>

#define DEBUG_MODE
#define RELEASE_MODE

// extern variables
extern std::mutex Globalmutex;            // threads conflict due to image-updating
extern std::condition_variable GlobalCondCV;     // threads conflict due to image-updating
extern bool imageReadable;                  // threads conflict due to image-updating
extern cv::Mat src;                         // Transfering buffer

/**
* @brief: imageUpdating thread
*/
int imageUpdatingThread();

/**
* @brief: armorDetecting thread
*/
void armorDetectingThread();


/**
 *@brief: the types of armor BIG SMALL 大装甲板 小装甲板
 */
enum ArmorType
{
    SMALL_ARMOR = 0,
    BIG_ARMOR = 1
};

/**
* @brief: colors in order B G R 颜色B蓝 G绿 R红
*/
enum Color
{
    BLUE = 0,
    GREEN = 1,
    RED = 2
};

/**
* @brief: get distance between two points
*/
float getPointsDistance(const cv::Point2f& a, const cv::Point2f& b);

#endif // GENERAL_H
