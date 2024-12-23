def main():
    path = '/home/hfeng/data/cluster10.sort'
    new_file = '/home/hfeng/data/cluster10.load'
    showed = {}
    with open(path) as fd, open(new_file, 'wt') as out:
        line = fd.readline()
        while line:
            split = line.split(',')
            if split[5] == 'add':
                showed[split[1]] = True
            elif split[5] == 'get':
                if not split[1] in showed:
                    out.write(split[1])
                    out.write('\n')
                    showed[split[1]] = True
            line = fd.readline()


def check():
    path = '/home/hfeng/data/cluster10.sort'
    showed = {}
    res = 0
    with open(path) as fd:
        line = fd.readline()
        while line:
            split = line.split(',')
            if split[5] == 'get':
                showed[split[1]] = True
            elif split[5] == 'add':
                if not split[1] in showed:
                    res += 1
            line = fd.readline()
    print(res)


if __name__ == '__main__':
    main()
    # check()
