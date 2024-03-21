import struct
import time
from struct import *
import socket

import numpy

# type_operations:
# 0 - запрос состояния
# 1 - запуск сканирования
# 2 - запуск сканирования и детектирования верхней рессоры
# 3 - получение последней задетектированной рессоры, центр и углы
# второй аргумент - тип рессоры 3 - маленький, 1 - все большие
# 4 - сброс ошибки по пустой рессоре
# TODO: получение снаружи габаритных размеров рессор


# ответы сервера
# 0 - не инициализированно
# 1 - библиотека инициализированна, идeт homing
# 2 - homing сделан
# 3 - сканирование
# 4 - ожидание команды на сканирование
# 5 - детектирование рессоры
# 6 - ошибка распознавания
# 7 - тара не пустая
# 8 - тара пустая
# 9 - в процессе детектирования
host = 'localhost'
port = 65000
addr = (host, port)
pa = pack("hhhh",2,1,700,20)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
s.sendall(pa)
data = s.recv(1024)
print(data)
s1 = struct.unpack('h', data[0:2])[0]
if s1 == 1:
    print("библиотека инициализированна, идeт homing")
elif s1 == 2:
    print("homing сделан")
elif s1 == 3:
    print("сканирование")
elif s1 == 4:
    print("ожидание команды")
elif s1 == 5:
    print("принят ответ")
    print("Центр х:", struct.unpack('f', data[2:6])[0])
    print("Центр y:", struct.unpack('f', data[6:10])[0])
    print("Центр z:", struct.unpack('f', data[10:14])[0])
    print("Угол 1:", struct.unpack('f', data[14:18])[0])
    print("Угол 3:", struct.unpack('f', data[22:26])[0])
    print("Угол 3:", struct.unpack('f', data[26:30])[0])
elif s1 == 6:
    print("ошибка обработки рессор")
elif s1 == 7:
    print("тара не пустая сбросьте ошибку")
elif s1 == 8:
    print("тара пустая, сбросьте ошибку")
elif s1 == 9:
    print("в процессе детектирования")
s.close()
print("Finished")