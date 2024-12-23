import os
import sys
import time


def main():
    if len(sys.argv) < 2:
        print("missing pid arg")
        sys.exit()

    pid = int(sys.argv[1])
    count = 0

    while True:
        file_name = '/home/SmartSSD_data/hfeng/test_' + str(count)
        os.system("dd if=/dev/zero bs=100M count=1000 of=" + file_name)
        count += 1
        try:
            os.kill(pid, 0)
        except OSError as err:
            raise
        time.sleep(1)
        os.system("rm -f " + file_name)

    # for i in range(14):
    #     file_name = '/home/SmartSSD_data/hfeng/test_' + str(count)
    #     os.system("dd if=/dev/zero bs=100M count=1000 of=" + file_name)
    #     count += 1
    #     print(count)


if __name__ == '__main__':
    main()
