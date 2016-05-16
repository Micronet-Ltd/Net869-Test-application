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
f = open( 'serial_' + str(sys.argv[2]) +  strftime("_%d_%b_%Y_%H_%M", gmtime()) + '.txt','w')

id= str(sys.argv[2])
idnum= id.strip('0x')

string_end_of_test = b'end_test\r\n'
string_a = b'full test pass\r\n'

ser.write(('id:' + idnum + '\r\n').encode()) #"test_" will start auto test running
end_of_test = True

while end_of_test:
    out = ser.readlines()
    for i in out:
        f.write(i.decode())
        if (i == string_end_of_test):
            end_of_test = False
            #print(i)
            break
        if (i == string_a):
            result = 1
        else:
            result = 0
            break
if result == 1:
    print('\n',idnum," ********** Test Pass **********\r")
else:
    #print(i)
    print ("\n************************************************")
    print ('\n',"Board ",idnum," ********** Test FAILED **********\r")
    print ("\n************************************************")
#print (idnum)
ser.close()
f.close()