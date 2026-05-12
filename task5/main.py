import time
import argparse
from multiprocessing import Process, Queue, set_start_method
import sys

from ultralytics import YOLO
import cv2


class ReadVideo:
    def __init__(self, path):
        self.cap = cv2.VideoCapture(path)

        if not self.cap.isOpened():
            raise RuntimeError(f"Не удалось открыть видео: {path}")

    def read_frame(self):
        frames = []
        while True:
            ret, frame = self.cap.read()

            if not ret:
                break

            frames.append(frame)
        return frames

    def clean(self):
        self.cap.release()


class Frames2Video():
    def __init__(self, filename, first_frame, fps):
        fourcc = cv2.VideoWriter.fourcc(*"mp4v")
        height, width = first_frame.shape[:2]

        self.writer = cv2.VideoWriter(
            filename,
            fourcc,
            fps,
            (width, height)
        )

        if not self.writer.isOpened():
            raise RuntimeError("Не удалось создать видео")
    
    def write_video(self, frames):
        for frame in frames:
            self.writer.write(frame)
    
    def clean(self):
        self.writer.release()


def split_frames(frames, num_proces):
    frames_size = len(frames)
    chunk_size = frames_size // num_proces
    chunks = []

    for i in range(num_proces):
        start = i * chunk_size
        end = (i + 1) * chunk_size if i != num_proces - 1 else frames_size
        chunks.append((start, frames[start:end]))
    
    return chunks

def worker(start_index, frames, queue):
    model = YOLO("yolov8s-pose.pt")

    for local_index, frame in enumerate(frames):
        result = model.predict(source=frame, conf=0.25, verbose=False)[0]
        annotated_frame = result.plot()
        global_index = start_index + local_index
        queue.put((global_index, annotated_frame))
      

def main():
    parser = argparse.ArgumentParser(
        prog='main.py',
        description='',
        epilog='Пример ввода в консоль: python task5/main.py --path task5/example_in.MP4 --mode 1 --name example_out'
    )
    parser.add_argument(
        '--path',
        type=str,
        default='task5/example_in.MP4',
        help='Путь к видео'
    )
    parser.add_argument(
        '--mode',
        type=int,
        default=1,
        help='Режим выполнения - колличество потоков'
    )
    parser.add_argument(
        '--name',
        type=str,
        default='example_out',
        help='Имя выходного видеофайла'
    )
    args = parser.parse_args()

    try:
        reader = None
        writer = None

        try:
            reader = ReadVideo(args.path)
        except Exception as e:
            print(e)
            sys.exit(1)

        fps = reader.cap.get(cv2.CAP_PROP_FPS)

        frames = reader.read_frame()
        
        if len(frames) == 0:
            print("Не получены кадры из видео")
            sys.exit(1)

        chunks = split_frames(frames, args.mode)

        processes = []
        queue = Queue()

        for start_index, chunk in chunks:
                processes.append(
                    Process(
                        target=worker,
                        args=(start_index, chunk, queue)
                    )
                )
        
        results = []

        start_time = time.perf_counter()

        for process in processes:
            process.start()

        for _ in range(len(frames)):
            frame_index, annotated_frame = queue.get()
            results.append((frame_index, annotated_frame))
        
        for process in processes:
            process.join()
        
        runtime = time.perf_counter() - start_time

        results.sort(key=lambda x: x[0])

        output_frames = []
        for _, frame in results:
            output_frames.append(frame)

        try:
            writer = Frames2Video(args.name + ".mp4", frames[0], fps)
        except Exception as e:
            print(e)
            sys.exit(1)

        writer.write_video(output_frames)

        print(f"Видео {args.path} обработано за {runtime}с на {args.mode} процессах и сохранено под названием {args.name + '.mp4'}")

    finally:
        if reader is not None:
            reader.clean() 

        if writer is not None:
            writer.clean()

if __name__ == "__main__":
    set_start_method("spawn", force=True)
    main()