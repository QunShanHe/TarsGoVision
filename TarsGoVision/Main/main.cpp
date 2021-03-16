/*
*	@Author: Qunshan He,mountain.he@qq.com
*	@Date:	 2021.03.16
*	@Brief:  multi-thread starts
*/

#include "AngleSolver/AngleSolver.h"
#include "Armor/Armor.h"
#include "General/General.h"
#include "GxCamera/GxCamera.h"

using namespace cv;
using namespace std;

// muti-threads control variables
mutex   Globalmutex;                        // C++11 mutex
condition_variable GlobalCondCV;            // C++11 condition variable
bool    imageReadable = false;              // threads conflict due to image-updating
Mat     src = Mat::zeros(480,640,CV_8UC3);  // Transfering buffer


int main(int argc, char** argv)
{
    // camera image updating thread
    thread(imageUpdatingThread).detach();
    // armor detecting thread
    thread(armorDetectingThread).detach();
    // main thread
    while (true)
    {
        char chKey = getchar();
        if (chKey == 'Q' || chKey == 'q')
            break;
    }
    return 0;
}
