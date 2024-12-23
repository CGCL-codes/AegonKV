# method one
import binascii

res = '7573657236383239333638333435353336303332393931'
data = binascii.unhexlify(res)
print(data, type(data))

with open('/Users/fenghao/Desktop/000190.blob', 'rb') as f:
    datas = f.read()
    start_char = datas.find(data)
    print(start_char)
