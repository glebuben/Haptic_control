#include <iostream>

#include <HD/hd.h>
#include <HDU/hduError.h>
#include <HDU/hduVector.h>

// Функция обратного вызова, которая будет вызвана планировщиком
HDCallbackCode HDCALLBACK positionCallback(void *userData)
{
    // Получаем текущее положение стилуса
    hduVector3Dd position;
    hdGetDoublev(HD_CURRENT_POSITION, position);

    // Выводим положение стилуса
    std::cout << "Current position: ("
              << position[0] << ", "
              << position[1] << ", "
              << position[2] << ")"
              << std::endl;

    // Эта функция должна возвращать HD_CALLBACK_CONTINUE, чтобы быть вызванной снова,
    // или HD_CALLBACK_DONE, чтобы остановить ее вызов.
    return HD_CALLBACK_CONTINUE;
}
int main() {
    HDErrorInfo error;
    HHD hHD = hdInitDevice(HD_DEFAULT_DEVICE);
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        fprintf(stderr, "Failed to initialize haptic device: %s\n", hdGetErrorString(error.errorCode));
        return 1;
    }

    hdEnable(HD_FORCE_OUTPUT);
    hdStartScheduler();
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        fprintf(stderr, "Failed to start the scheduler: %s\n", hdGetErrorString(error.errorCode));
        hdDisableDevice(hHD);
        return 1;
    }

//    while (true) // В реальном приложении здесь должно быть условие выхода
//    {
//        hdBeginFrame(hHD);
//
//        hduVector3Dd position;
//        hdGetDoublev(HD_CURRENT_POSITION, position);
//
//        // Вычисление силы сопротивления как функции от положения
//        // В этом примере просто используем пропорциональное отрицательное значение позиции для создания силы
////        hduVector3Dd force = {-10, 0, 0};
//
//        // Установка силы сопротивления
////        hdSetDoublev(HD_CURRENT_FORCE, force);
//
//        hdEndFrame(hHD);
//
//        printf("Stylus Position: (%f, %f, %f)\n", position[0], position[1], position[2]);
//
//        // В реальном приложении здесь может быть задержка или обработка событий GUI
//    }

    hdStopScheduler();
    hdDisableDevice(hHD);
    return 0;
}
