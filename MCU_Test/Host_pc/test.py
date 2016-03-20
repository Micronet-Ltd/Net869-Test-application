#! python
import sys
import time, serial
import datetime
import binascii
from time import gmtime, strftime

now = datetime.datetime.now()
port = sys.argv[1]
# configure the serial connections (the parameters differs on the device you are connecting to)
ser = serial.Serial(
    port,
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=0,              #set a timeout value, None for waiting forever
    xonxoff=0,              #enable software flow control
    rtscts=0               #enable RTS/CTS flow control
)


cmd = "test_" + str(sys.argv[1]) + chr(13)

time_now = str(sys.argv[0])
f = open( 'test_id_' + str(sys.argv[2]) +  strftime("_%d_%b_%Y_%H_%M", gmtime()) + '.txt','w')


string_end_of_test = b'end_test\r\n'
ser.write('test_0x1234\r\n'.encode())
end_of_test = True

while end_of_test:
    out = ser.readlines()
    for i in out:
        f.write(i.decode())
        #print(i)
        if (i == string_end_of_test):
            end_of_test = False
            break

ser.close()
f.close()