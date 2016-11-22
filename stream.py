import wave
import time
import socket
import sys

ip = sys.argv[1]
waveFile = wave.open(sys.argv[2], 'r')
t = 1./waveFile.getframerate()
length = waveFile.getnframes()
sampwid = waveFile.getsampwidth()

print(t,1/t,length,sampwid)

s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
s.connect((ip,80))

s.send("Play song %d\r" % length)

bytes_sent = 0

while bytes_sent < length:
    req = s.recv(16).strip()

    if len(req) > 0:
        if req == 'Done!':
            print('Yay! Done!')
            exit()
        else:
            print(req, req[5:])
            bytes_req = int(req[5:])
            print("Sending %d bytes" % bytes_req)
            waveData = waveFile.readframes(bytes_req)
            n = 0
            while n < bytes_req:
                # print '%d,' % ord(waveData[n][0]),
                sent = s.send(waveData[n][0])
                n+=sent

s.close()
