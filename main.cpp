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
HDCallbackCode HDCALLBACK informationCallback(const HHD *userData)
{
    HHD device = *userData;
    hdMakeCurrentDevice(device);
    hdBeginFrame(device);

    // Текущее положение стилуса
    HDint buttonState;
    HDboolean safetySwitch;
    HDboolean inkwellSwitch;
    hduVector3Dd position; // позиция
    hduVector3Dd velocity;
    hduVector3Dd joint_angles; // углы шарниров основания
    hduVector3Dd joint_gimbal_angles; // углы шарниров карданового подвеса (последних трёх шариниров)
    HDdouble transform[16]; // матрица преобразования
    HDlong encoder_values[6];

    // Получение данных
    hdGetDoublev(HD_CURRENT_POSITION, position);
    hdGetDoublev(HD_CURRENT_VELOCITY, velocity);
    hdGetDoublev(HD_CURRENT_JOINT_ANGLES, joint_angles);
    hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, joint_gimbal_angles);
    hdGetDoublev(HD_CURRENT_TRANSFORM, transform);
    hdGetIntegerv(HD_CURRENT_BUTTONS, &buttonState);
    hdGetBooleanv(HD_CURRENT_SAFETY_SWITCH, &safetySwitch);
    hdGetBooleanv(HD_CURRENT_INKWELL_SWITCH, &inkwellSwitch);
    hdGetLongv(HD_CURRENT_ENCODER_VALUES, encoder_values);

    // Состояние кнопок. Каждый бит соответствует определенной кнопке.
//    std::cout << "Current buttons state:" << buttonState <<  std::endl;
    // Чтобы получать значение с определенной кнопки можно использовать побитовое 'и' с нужной кнопкой.
    // Значение для каждой кнопки - 2 в какой-то степени, в нашем случае это просто '1' и '2'
//    HDint button1 = buttonState & 1;
//    std::cout  << "Current button 1 state:"<< button1 <<  std::endl;
//    HDint button2 = buttonState & 2;
//    std::cout  << "Current button 2 state:"<< button2 <<  std::endl;

//    // Проверка предохранительный выключатель (safety switch)
//    // У нашего устройства его либо нет, либо я не разобрался, где его найти. Всегда '0'
//    std::cout << static_cast<int>(safetySwitch) <<  std::endl;

    // Проверка, что стилус находится в чернильнице. Если внутри, возвращает '0', в обратном случае '1'
//    std::cout << static_cast<int>(inkwellSwitch) <<  std::endl;

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

//    // Скорость
//    std::cout << "Current end-effector velocity: ("
//            << velocity << ")"
//            << std::endl << std::endl;



    hdEndFrame(hdGetCurrentDevice());

    return HD_CALLBACK_CONTINUE;
}

HDCallbackCode HDCALLBACK forceCallback(void *userData)
{
    hdBeginFrame(hdGetCurrentDevice());
    // Текущее положение стилуса
    hduVector3Dd position;
    hduVector3Dd velocity;
    hdGetDoublev(HD_CURRENT_POSITION, position);
    hdGetDoublev(HD_CURRENT_POSITION, velocity);
    // Вычисляем силу как функцию от положения. В этом случае просто
    // притягиваем стилус к началу координат.
    // Пример: простая пропорциональная сила, направленная к центру.
    hduVector3Dd force;

    // Задаем жёсткость (часть, зависящая от позиции) и демпинг (часть, зависящая от скорости)
    double S = 0.01;
    double D = 0.01;
    force[0] = -position[0] * S - velocity[0] * D;
    force[1] = -position[1] * S - velocity[1] * D;
    force[2] = -position[2] * S - velocity[2] * D;

    // Заметил, что при маленьких значениях хаптик не может вернуться в точку (0, 0) - мешает трение.
    // Может быть, просто поигравшись с D, это проблема решается. Надо тестить.
    hdSetDoublev(HD_CURRENT_FORCE, force);

    hdEndFrame(hdGetCurrentDevice());

    return HD_CALLBACK_CONTINUE;
}

HDCallbackCode HDCALLBACK impedanceControlCallback(void *userData)
{
    // Получаем айдишники хаптиков
    HHD **HHDs = (HHD **)userData;
    HHD device1 = *HHDs[0];
    HHD device2 = *HHDs[1];

    // Переключаемся на первый хаптик
    hdMakeCurrentDevice(device1);
    hdBeginFrame(device1);

    // Получаем текущее положение стилуса для первого девайса
    hduVector3Dd position1;
    hduVector3Dd velocity1;
    hdGetDoublev(HD_CURRENT_POSITION, position1);
    hdGetDoublev(HD_CURRENT_POSITION, velocity1);

    // Переключаемся на второй девайс
    hdMakeCurrentDevice(device2);
    hdBeginFrame(device2);

    // Получаем текущее положение стилуса для второго девайса
    hduVector3Dd position2;
    hduVector3Dd velocity2;
    hdGetDoublev(HD_CURRENT_POSITION, position2);
    hdGetDoublev(HD_CURRENT_POSITION, velocity2);
    // Вычисляем силу как функцию от положения. В этом случае просто
    // притягиваем стилус второго к позиции первого.

    hduVector3Dd force;

    // Вычисляем ошибку координат
    hduVector3Dd position_error = position1 - position2;
    // Задаем силу
    force[0] = position_error[0] * 0.1;
    force[1] = position_error[1] * 0.1;
    force[2] = position_error[2] * 0.1;

    // Применяем силу к хаптику
    hdSetDoublev(HD_CURRENT_FORCE, force);
    // Получаем обновленную позицию
    hdGetDoublev(HD_CURRENT_POSITION, position2);

    hdEndFrame(device2);

    // Переключаемся на первый девайс
    hdMakeCurrentDevice(device1);

    // Получаем текущее положение стилуса для первого девайса
    hdGetDoublev(HD_CURRENT_POSITION, position1);
    hdGetDoublev(HD_CURRENT_POSITION, velocity1);

    // Вычисляем ошибку координат
    position_error = position2 - position1;
    // Задаем силу
    force[0] = position_error[0] * 0.1;
    force[1] = position_error[1] * 0.1;
    force[2] = position_error[2] * 0.1;


    // Применяем силу к хаптику
    hdSetDoublev(HD_CURRENT_FORCE, force);


    hdEndFrame(device1);

    return HD_CALLBACK_CONTINUE;
}

int main()
{
    std::thread serverThread(server);
    serverThread.detach();
    HDErrorInfo error;

    //  Инициализируем первый девайс, не забудь настроить конфигурации в TouchDevice
    HHD hHD1 = hdInitDevice("Right Device");
    hdEnable(HD_FORCE_OUTPUT);
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        std::cerr << "Failed to initialize haptic device: " << hdGetErrorString(error.errorCode) << std::endl;
        return -1;
    }
    // Инициализируем второй девайс
    HHD hHD2 = hdInitDevice("Left Device");
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        std::cerr << "Failed to initialize haptic device: " << hdGetErrorString(error.errorCode) << std::endl;
        hdDisableDevice(hHD1);
        return -1;
    }
    hdEnable(HD_FORCE_OUTPUT);
    // Создаем айдишников девайсов, чтобы передавать в callBack
    unsigned int *hhDs[2] = {&hHD1, &hHD2};

    hdStartScheduler();
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        std::cerr << "Failed to start the scheduler: " << hdGetErrorString(error.errorCode) << std::endl;
        hdDisableDevice(hHD1);
        hdDisableDevice(hHD2);
        return -1;
    }

//    HDulong callbackHandle = hdScheduleAsynchronous(positionCallback, NULL, HD_DEFAULT_SCHEDULER_PRIORITY);

    HDulong callbackHandle = hdScheduleAsynchronous(impedanceControlCallback, hhDs, HD_DEFAULT_SCHEDULER_PRIORITY);

    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();

    hdUnschedule(callbackHandle);
    hdStopScheduler();
    hdDisableDevice(hHD1);
    hdDisableDevice(hHD2);

    return 0;
}