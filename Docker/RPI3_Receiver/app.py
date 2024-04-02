#!/usr/bin/python
from influxdb import InfluxDBClient
import signal
import time
import serial_port_reader

shutdown = False
DEVICE_NAME = 'RaspberryPi3Receiver'
BAUDRATE = 115200
field_list = ['DHT11_Temperature_Celsius', 'DHT11_Temperature_Fahrenheit', 'DHT11_Humidity', 'DHT11_Feels_Like', 'MQ135', 'PMS_PM1', 'PMS_PM2.5', 'PMS_PM10', 'PMS_N0.3', 'PMS_N0.5', 'PMS_N1.0', 'PMS_N2.5', 'PMS_N5.0', 'PMS_N10', 'eco2', 'etvoc']

def signal_handler(signal, frame):
	global shutdown
	shutdown = True
	print('Initialising shutdown procedure...\nSayonara')

def AddToDatabase(measurement):
	timestamp = int(time.time())

	data = [
		{
			"measurement": measurement[0],
			"tags":{
				"device": DEVICE_NAME
			},
			"time": timestamp,
			"fields": {"value":measurement[1]}
		}
	]

	#print('\n\n--- Assembled data:\n')
	#print(data)
	#print('\n---\n\n')
	client.write_points(data, database = 'aqstation', time_precision = 's', protocol = 'json')

def VerifyLine(line):
	components = line.split(':')
	if components[0] in field_list:
		print(components[0] + ' has value ' + components[1] + '.\n')
		AddToDatabase(components)

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

print('Press Ctrl+C to stop and exit!')

client = InfluxDBClient(
	host = 'influxdb',
	port = 8086,
	username = 'raspberry',
	password = 'raspberry'
)

port = '/dev/ttyUSB0'
serial_port = serial_port_reader.serial_port_reader(DEVICE_NAME, port, BAUDRATE)
serial_port.open_connection()

#stringa tipo: campo:valore
while not shutdown:
	line = serial_port.read_line()
	VerifyLine(line)

serial_port.close_connection()
