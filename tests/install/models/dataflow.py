class Source:
	def __init__(self, R, log=None):
		self.R = R

		self.log = log

	def cycle(self):
		self.R.send(0)

class Sink:
	def __init__(self, L, log=None):
		self.L = L

		self.log = log

	def cycle(self):
		while True:
			if self.L.isValid():
				self.L.recv()
			else:
				return


class Buffer:
	def __init__(self, L, R, log=None):
		self.L = L
		self.R = R

		self.log = log

	def cycle(self):
		while True:
			if self.L.isValid():
				self.R.send(self.L.recv())
			else:
				return

class Copy:
	def __init__(self, L, R0, R1, log=None):
		self.L = L
		self.R0 = R0
		self.R1 = R1

		self.log = log

	def cycle(self):
		while True:
			if self.L.isValid():
				l = self.L.recv()
				self.R0.send(l)
				self.R1.send(l)
			else:
				return

class Add:
	def __init__(self, width, A, B, S, signed=False, log=None):
		self.A = A
		self.B = B
		self.S = S
		self.lo = -(2**(width-1)) if signed else 0
		self.hi = (2**(width-1)) if signed else (2**width)

		self.log = log

	def cycle(self):
		while True:
			if not self.A.isValid() or not self.B.isValid():
				return

			s = self.A.recv()+self.B.recv()
			while s < self.lo:
				s -= self.lo
			while s >= self.hi:
				s -= self.hi
			self.S.send(s)

class Split:
	def __init__(self, Cc, L, R0, R1, log=None):
		self.Cc = Cc
		self.L = L
		self.R0 = R0
		self.R1 = R1

		self.log = log

	def cycle(self):
		while True:
			if self.Cc.isValid() and self.L.isValid() and self.Cc.probe() == 0:
				c = self.Cc.recv()
				self.R0.send(self.L.recv())
			elif self.Cc.isValid() and self.L.isValid() and self.Cc.probe() == 1:
				c = self.Cc.recv()
				self.R1.send(self.L.recv())
			else:
				return

class Merge:
	def __init__(self, Cc, L0, L1, R, log=None):
		self.Cc = Cc
		self.L0 = L0
		self.L1 = L1
		self.R = R

		self.log = log

	def cycle(self):
		while True:
			if self.Cc.isValid() and self.L0.isValid() and self.Cc.probe() == 0:
				self.Cc.recv()
				self.R.send(self.L0.recv())
			elif self.Cc.isValid() and self.L1.isValid() and self.Cc.probe() == 1:
				self.Cc.recv()
				self.R.send(self.L1.recv())
			else:
				return
