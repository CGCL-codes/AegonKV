import os
import sys
import time
import psutil


def main():
    if len(sys.argv) < 2:
        print("missing pid arg")
        sys.exit()

    pid = int(sys.argv[1])

    interval = 30
    with open("SIZE", "a+") as f:
        f.write("time,size\n")
        while True:
            time.sleep(interval)
            current_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))
            db_size = os.popen('du -sh ./')
            line = current_time + ',' + db_size.read().split('\t')[0] + '\n'
            f.write(line)
            try:
                os.kill(pid, 0)
            except OSError as err:
                raise


if __name__ == '__main__':
    main()
