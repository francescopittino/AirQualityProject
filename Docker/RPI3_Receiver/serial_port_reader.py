#/usr/bin/python3
import serial

class serial_port_reader:
	def __init__(self, label, port_number, baudrate):
		self.label = label
		self.port = serial.Serial()
		self.port.port = port_number
		self.port.baudrate = baudrate
		self.open = False
		self.port.bytesize = serial.EIGHTBITS
		self.port.parity = serial.PARITY_NONE
		self.port.stopbits = serial.STOPBITS_ONE
		self.port.timeout = 30
		self.port.xonxoff = False
		self.port.rtscts = False
		self.port.dsrdtr = False

	def open_connection(self):
		if not self.open:
			print('('+self.label+') opening connection')
			self.port.open()
			self.open = True

	def read_line(self):
		try: 
			line = str(self.port.readline().decode('ascii')).strip()
		except:
			line = ''
		return line

	def close_connection(self):
		if self.open:
			print('('+self.label+') closing connection')
			self.port.close()
			self.open = False

	def __del__(self):
		if self.open:
			self.port.close()
			self.open = False
