import threading
import argparse
import logging
from queue import Queue, Empty
import time
import os
import sys

import cv2


os.makedirs("task4/log", exist_ok=True)

logging.basicConfig(
    filename='task4/log/app.log',
    filemode="w",
    format="[%(relativeCreated)d ms] [%(levelname)s] %(message)s",
    level=logging.INFO
)

parser = argparse.ArgumentParser(
    prog='main.py',
    description='Чтение данных с камеры и трёх датчиков в отдельных потоках с последующим отображением в окне',
    epilog='Пример ввода в консоль: python task4/main.py --camera 0 --resolution 1440x960 --fps 30'
)
parser.add_argument(
    '--camera',
    type=str,
    default='0',
    help='Имя камеры в системе'
)
parser.add_argument(
    '--resolution',
    type=str,
    default='1440x960',
    help='Желаемое разрешение камеры'
)
parser.add_argument(
    '--fps',
    type=int,
    default=30,
    help='Частота отображения'
)
args = parser.parse_args()


class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")
    

class SensorX(Sensor):
    def __init__(self, delay: float):
        self._delay = delay
        self._data = 0
    
    def get(self) -> int:
        time.sleep(self._delay)
        self._data += 1
        return self._data


class SensorCam(Sensor):
    def __init__(self, camera, resolution):
        camera = int(camera) if camera.isdigit() else camera
        self.camera = cv2.VideoCapture(camera)

        if not self.camera.isOpened():
            logging.error(f"Камера {camera} не найдена или не открылась")
            raise RuntimeError(f"Камера {camera} не найдена или не открылась")
        
        logging.info(f"Камера {camera} установлена")
        
        size = resolution.lower().split('x')
        if len(size) == 2:
            width, height = size
        else:
            logging.error(f"Неправильно введено разрешение. Пример: 1440x960")
            raise ValueError("Неправильно введено разрешение.")

        if not width.isdigit() or not height.isdigit():
            logging.error(f"Ширина и высота - целые числа.")
            raise ValueError("Ширина и высота - целые числа.")
        
        width, height = int(width), int(height)
        self.camera.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        self.camera.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

        real_width = self.camera.get(cv2.CAP_PROP_FRAME_WIDTH)
        real_height = self.camera.get(cv2.CAP_PROP_FRAME_HEIGHT)

        if int(real_width) != width or int(real_height) != height:
            logging.warning(f"Камера не поддерживает разрешение: {width}x{height}")

        logging.info(f"Установлено разрешение: {real_width}x{real_height}")

    def get(self):
        ret, frame = self.camera.read()

        if not ret:
            logging.error(f"Не удалось получить кадр")
            raise RuntimeError(f"Не удалось получить кадр")

        return frame
            
    def __del__(self):
        self.camera.release()
        logging.info("Камера освобождена")


class WindowImage():
    def __init__(self, fps):
        if fps <= 0:
            logging.error("fps должно быть больше 0")
            raise ValueError("fps должно быть больше 0")
        
        self.fps = fps
        logging.info("Окно создано")

    def show(self, img):
        try:
            cv2.imshow('Window', img)
            return cv2.waitKey(int(1000 / self.fps)) & 0xFF
        except Exception as e:
            logging.error(f"Не удалось отобразить изображение: {e}")
            raise RuntimeError(f"Не удалось отобразить изображение: {e}")

    def __del__(self):
        cv2.destroyAllWindows()
        logging.info("Окно закрыто")


def worker(sensor, q, stop_event):
    while not stop_event.is_set():
        try:
            data = sensor.get()
        except Exception as e:
            print(e)

        if data is None:
            continue

        try:
            q.get_nowait()
        except Empty:
            pass
        
        q.put(data)

try:
    camera_sensor = SensorCam(args.camera, args.resolution)
except Exception as e:
    print(e)
    sys.exit(1)

sensor_100 = SensorX(1 / 100)
sensor_10 = SensorX(1 / 10)
sensor_1 = SensorX(1 / 1)

try:
    window = WindowImage(args.fps)
except Exception as e:
    print(e)
    sys.exit(1)

cam_queue = Queue(maxsize=1)
q100 = Queue(maxsize=1)
q10 = Queue(maxsize=1)
q1 = Queue(maxsize=1)

stop_event = threading.Event()

threads = [
    threading.Thread(
        target=worker,
        args=(camera_sensor, cam_queue, stop_event),
        name='camera_sensor'
    ),
    threading.Thread(
        target=worker,
        args=(sensor_100, q100, stop_event),
        name='sensor_100'
    ),
    threading.Thread(
        target=worker,
        args=(sensor_10, q10, stop_event),
        name='sensor_10'
    ),
    threading.Thread(
        target=worker,
        args=(sensor_1, q1, stop_event),
        name='sensor_1'
    )
]

cam_frame, frame_100, frame_10, frame_1 = None, 0, 0, 0

for thread in threads:
    thread.start()
    logging.info(f"Поток {thread.name} запущен")

try:
    while True:
        try:
            while True:
                cam_frame = cam_queue.get_nowait()
        except Empty:
            pass

        try:
            while True:
                frame_100 = q100.get_nowait()
        except Empty:
            pass

        try:
            while True:
                frame_10 = q10.get_nowait()
        except Empty:
            pass

        try:
            while True:
                frame_1 = q1.get_nowait()
        except Empty:
            pass

        if cam_frame is not None:
            img = cam_frame.copy()

            cv2.putText(img, f"Sensor100: {frame_100}", (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.putText(img, f"Sensor10: {frame_10}", (20, 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.putText(img, f"Sensor1: {frame_1}", (20, 120), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

            key = window.show(img)

            if key == ord('q'):
                logging.info("Нажата клавиша q")
                break
finally:
    stop_event.set()

    for thread in threads:
        thread.join()
        logging.info(f"Поток {thread.name} остановился")

    del camera_sensor
    del window
