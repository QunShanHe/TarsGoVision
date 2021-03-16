/*
*	@Author: Qunshan He,mountain.he@qq.com
*	@Date:	 2020.06.15
*	@Brief:  This header file declares all the classes and params used to solve eular angle
*/

#ifndef ANGLE_SOLVER
#define ANGLE_SOLVER

#include "General/General.h"

class AngleSolver
{
public:
	AngleSolver();
	~AngleSolver();

	/**
	* @brief Set camera params
	* @param camera_matrix: camera IntrinsicMatrix
	* @param distortion_coeff: camera DistortionCoefficients
	*/
	void setCameraParam(const cv::Mat & camera_matrix, const cv::Mat & distortion_coeff);
	//overload function. Params set by xml file
	int setCameraParam(const char* filePath, int camId);

	/**
	* @brief Set armor size
	* @param type: input target type small/big
	* @param width: the width of armor (mm)
	* @param height: the height of armor (mm)
	*/
	void setArmorSize(ArmorType type, double width, double height);

	/**
	* @brief Set bullet speed
	* @param bulletSpeed: the speed of bullet(mm/s)
	*/
	void setBulletSpeed(int bulletSpeed);

	/**
	* @brief set the target armor contour points and centerPoint
	* @param points2d image points set with following order : left_up, right_up, left_down, right_down
	* @param type target armor type
	*/
	void setTarget(std::vector<cv::Point2f> contoursPoints, cv::Point2f centerPoint, ArmorType type);


	/**
	* @brief solve the angles using P4P or PinHole according to the distance
	*/
	void solveAngles();

	/**
	* @brief solve the angles using P4P method
	*/
	void P4P_solver();

	/**
	* @brief solve the angles using PinHole method
	*/
	void PinHole_solver();

	/**
	* @brief compensation of pitch
	*/
	void compensateAngle();

	/**
	* @brief compensation of pitch for y_offset between barrel and camera
	*/
	void compensateOffset();

	/**
	* @brief compensation of pitch for gravity
	*/
	void compensateGravity();

	/**
	* @brief according to the target2D points to get the yaw and pitch and distance towards the certain type target using solvePnP
	* @param inputArrayOfPoints contourPoints, the vertices of target armor
	* @param inputPoint centerPoint the center point of target armor
	* @param input type the type of armor BIG_ARMOR or SMALL_ARMOR
	* @param output y_yaw angle     the angle that yaw revolve     '<-' left is minus-       '->' right is positive+
	* @param output x_pitch angle   the angle that pitch revolve   '下' down is minus-       '上' up is positive+ 
	* @param output distance  unit is mm
	*/
	void getAngle(std::vector<cv::Point2f> & contourPoints, cv::Point2f centerPoint, ArmorType type, double & yaw, double & pitch, double & evaluateDistance);

    /**
    * @brief show debug information
    */
    void showDebugInfo(bool showCurrentResult, bool showTVec, bool showP4P, bool showPinHole, bool showCompensation, bool showCameraParams);

private:

	//Camera params
	cv::Mat CAMERA_MATRIX;		//IntrinsicMatrix		  fx,fy,cx,cy
	cv::Mat DISTORTION_COEFF;	//DistortionCoefficients k1,k2,p1,p2

	//Object points in world coordinate
	std::vector<cv::Point3f> SMALL_ARMOR_POINTS_3D;
	std::vector<cv::Point3f> BIG_ARMOR_POINTS_3D;

	//speed of bullet (compensation for gravity and air fru)
	double BULLET_SPEED;

	//distance between camera and barrel in y axis(positive when camera is under barrel)  barrel_y = camera_y + barrel_camera_y
	double GUN_CAM_DISTANCE_Y;

	//Targets
	std::vector<cv::Point2f> targetContour;
	cv::Point2f targetCenter;
	ArmorType targetType;

	// calculated by solvePnP
	//s[R|t]=s'  s->world coordinate;s`->camera coordinate
	cv::Mat rVec;		//rot rotation between camera and target center
	cv::Mat tVec;		//trans tanslation between camera and target center

	//Results
	float y_yaw;
	float x_pitch;
	double distance;
};


#endif // !ANGLE_SOLVER

