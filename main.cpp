#include <iostream>

#include <HD/hd.h>
#include <Cursor.h>
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


};
msg mymsg;
int command = mymsg.command;
HDCallbackCode HDCALLBACK informationCallback(void *userData)
{
    hdBeginFrame(hdGetCurrentDevice());

    // Текущее положение стилуса
    HDint buttonState;
    HDboolean safetySwitch;
    HDboolean inkwellSwitch;
    hduVector3Dd position; // позиция
    hduVector3Dd joint_angles; // углы шарниров основания
    hduVector3Dd joint_gimbal_angles; // углы шарниров карданового подвеса (последних трёх шариниров)
    HDdouble transform[16]; // матрица преобразования
    HDlong encoder_values[6];

//    std::cout << "Current buttons state:" << transform << std::endl;

    // Получение данных
    hdGetDoublev(HD_CURRENT_POSITION, position);
    hdGetDoublev(HD_CURRENT_JOINT_ANGLES, joint_angles);
    hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, joint_gimbal_angles);
    hdGetDoublev(HD_CURRENT_TRANSFORM, transform);
    hdGetIntegerv(HD_CURRENT_BUTTONS, &buttonState);
    hdGetBooleanv(HD_CURRENT_SAFETY_SWITCH, &safetySwitch);
    hdGetBooleanv(HD_CURRENT_INKWELL_SWITCH, &inkwellSwitch);
    hdGetLongv(HD_CURRENT_ENCODER_VALUES, encoder_values);

    // Состояние кнопок. Каждый бит соответствует определенной кнопке.
    std::cout << "Current buttons state:" << buttonState <<  std::endl;
    // Чтобы получать значение с определенной кнопки можно использовать побитовое 'и' с нужной кнопкой.
    // Значение для каждой кнопки - 2 в какой-то степени, в нашем случае это просто '1' и '2'
    HDint button1 = buttonState & 1;
    std::cout  << "Current button 1 state:"<< button1 <<  std::endl;
    HDint button2 = buttonState & 2;
    std::cout  << "Current button 2 state:"<< button2 <<  std::endl;

//    // Проверка предохранительный выключатель (safety switch)
//    // У нашего устройства его либо нет, либо я не разобрался, где его найти. Всегда '0'
//    std::cout << static_cast<int>(safetySwitch) <<  std::endl;

    // Проверка, что стилус находится в чернильнице. Если внутри, возвращает '0', в обратном случае '1'
    std::cout << static_cast<int>(inkwellSwitch) <<  std::endl;

//    std::cout << "Current Encoder values:" <<  std::endl
//    << encoder_values[0] << ", "
//    << encoder_values[1] << ", "
//    << encoder_values[2] << ", "
//    << encoder_values[3] << ", "
//    << encoder_values[4] << ", "
//    << encoder_values[5] << std::endl;

//    // Матрица преобразования. Не уверен, в каком порядке нужно выводить значения
//    std::cout << "Current Transformation matrix:" <<  std::endl
//    << transform[0] << ", " << transform[4] << ", " << transform[8] << ", " << transform[12] << std::endl
//    << transform[1] << ", " << transform[5] << ", " << transform[9] << ", " << transform[13] << std::endl
//    << transform[2] << ", " << transform[6] << ", " << transform[10] << ", " << transform[14] << std::endl
//    << transform[3] << ", " << transform[7] << ", " << transform[11] << ", " << transform[15] << std::endl;


//    // Углы первых трёх шарниров
//    std::cout << "Current base angles: ("
//              << joint_angles << ")"
//              << std::endl;

//    // Углы последних трёх шарниров
//    std::cout << "Current position: ("
//            << joint_gimbal_angles << ")"
//            << std::endl << std::endl;
//

    hdEndFrame(hdGetCurrentDevice());

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
    HDulong callbackHandle = hdScheduleAsynchronous(informationCallback, NULL, HD_DEFAULT_SCHEDULER_PRIORITY);

    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();

    hdUnschedule(callbackHandle);
    hdStopScheduler();
    hdDisableDevice(hHD);

    return 0;
}