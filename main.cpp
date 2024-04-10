#include <iostream>

#include <HD/hd.h>
#include <HDU/hduError.h>
#include <HDU/hduVector.h>
#include "server.h"
#include <thread>
bool terminateThread = false;

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;
int iSendResult = 0;
char recvbuf[1024];
int rcvd = 0;

std::mutex mtx;
struct msg
{
    std::int64_t milliseconds;
    int command;
    double x;
    double y;
    double z;
    double fa1;
    double fa2;
    double fa3;
    double fa4;
    double fa5;
    double fa6;
    double fa7;
    double fx;
    double fy;
    double fz;
    double j1;
    double j2;
    double j3;
    double j4;
    double j5;
    double j6;
    bool button_1;
    bool button_2;
    bool inkwell;


};
msg mymsg;
int command = mymsg.command;
HDCallbackCode HDCALLBACK positionCallback(void *userData)
{
    hdBeginFrame(hdGetCurrentDevice());
    HDint buttonState_1 = 0;
    HDint buttonState_2 = 0;
    hduVector3Dd position;
    HDboolean inkwellSwitch;
    hduVector3Dd joint_angles;
    hdGetDoublev(HD_CURRENT_POSITION, position);
    hdGetIntegerv(HD_CURRENT_BUTTONS, &buttonState_1);
    hdGetIntegerv(HD_CURRENT_BUTTONS, &buttonState_2);
    hdGetBooleanv(HD_CURRENT_INKWELL_SWITCH, &inkwellSwitch);
    hdGetDoublev(HD_CURRENT_JOINT_ANGLES, joint_angles);
    hduVector3Dd joint_gimbal_angles;
    hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, joint_gimbal_angles);
    hdEndFrame(hdGetCurrentDevice());
//    if (buttonState_1 & HD_DEVICE_BUTTON_2) { // Assuming HD_DEVICE_BUTTON_1 corresponds to button 0
//        std::cout << "Button 0 pressed." << std::endl;
//        // Perform your action here
//    }

    std::cout << "Current position: ("
              << joint_angles[0] << ", "
              << joint_angles[1] << ", "
              << joint_angles[2] << ")"
              << std::endl;
    mtx.lock();
    mymsg.x = position[0]/1000;
    mymsg.y = position[1]/1000;
    mymsg.z = position[2]/1000;

    mymsg.j1 = joint_angles[0];
    mymsg.j2 = joint_angles[1];
    mymsg.j3 = joint_angles[2];

    mymsg.j4 = joint_gimbal_angles[0];
    mymsg.j5 = joint_gimbal_angles[1];
    mymsg.j6 = joint_gimbal_angles[2];

    mymsg.inkwell = inkwellSwitch;
    mymsg.button_1 =buttonState_1&1;
    mymsg.button_2 =buttonState_1&2;
    mymsg.inkwell = inkwellSwitch&1;
    std::cout << "Current position angle: ("
              << joint_angles[0] << ", "
              << joint_angles[1] << ", "
              << joint_angles[2] << ")"
              << std::endl;


//        std::cout << (inkwellSwitch&1) <<std::endl;
    mtx.unlock();
    return HD_CALLBACK_CONTINUE;
}

HDCallbackCode HDCALLBACK forceCallback(void *userData)
{
    hdBeginFrame(hdGetCurrentDevice());

    // Текущее положение стилуса
    hduVector3Dd position;
    hdGetDoublev(HD_CURRENT_POSITION, position);

    // Вычисляем силу как функцию от положения. В этом случае просто
    // притягиваем стилус к началу координат.
    // Пример: простая пропорциональная сила, направленная к центру.
    hduVector3Dd force;
//    std::cout << "Current position: ("
//              << position[0] << ", "
//              << position[1] << ", "
//              << position[2] << ")"
//              << std::endl;
    if(mymsg.command == 65538) {
        force[0] = -position[0] * 0.02; // Коэффициент управляет "жесткостью" силы
        force[1] = -position[1] * 0.02;
        force[2] = -position[2] * 0.02;
    }
    hdSetDoublev(HD_CURRENT_FORCE, force);

    hdEndFrame(hdGetCurrentDevice());

    return HD_CALLBACK_CONTINUE;
}

int main()
{
    std::thread serverThread(server);
    serverThread.detach();
    HDErrorInfo error;
    HHD hHD = hdInitDevice(HD_DEFAULT_DEVICE);
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        std::cerr << "Failed to initialize haptic device: " << hdGetErrorString(error.errorCode) << std::endl;
        return -1;
    }

    hdEnable(HD_FORCE_OUTPUT);
    hdStartScheduler();
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        std::cerr << "Failed to start the scheduler: " << hdGetErrorString(error.errorCode) << std::endl;
        hdDisableDevice(hHD);
        return -1;
    }

//    HDulong callbackHandle = hdScheduleAsynchronous(positionCallback, NULL, HD_DEFAULT_SCHEDULER_PRIORITY);
    HDulong callbackHandle = hdScheduleAsynchronous(positionCallback, NULL, HD_DEFAULT_SCHEDULER_PRIORITY);

    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();

    hdUnschedule(callbackHandle);
    hdStopScheduler();
    hdDisableDevice(hHD);

    return 0;
}