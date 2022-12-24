from socket import*
import RPi.GPIO as GPIO
import time
import pickle

servo_pin = 33

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(servo_pin, GPIO.OUT)

clientSock=socket(AF_INET,SOCK_STREAM)
clientSock.connect(('IP',PORT))

print("Success to connect")

pwm = GPIO.PWM(servo_pin, 50)
pwm.start(1)
print("start")

while True:
	data=clientSock.recv(1024)
	cmd = pickle.loads(data)
	print("Received Data :", cmd)

	#Control Servo Motor
	if cmd < 2200:
		pwm.ChangeDutyCycle(6)
	if cmd >= 2200:
		pwm.ChangeDutyCycle(1)

print("Connection Ending")
pwm.ChangeDutyCycle(1)
print("end")

pwm.stop()
GPIO.cleanup()
