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
			if not self.Cc.isValid() or not self.L.isValid():
				return

			c = self.Cc.recv()
			if c == 0:
				self.R0.send(self.L.recv())
			elif c == 1:
				self.R1.send(self.L.recv())
