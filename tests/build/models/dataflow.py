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
