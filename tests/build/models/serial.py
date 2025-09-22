class Add:
	def __init__(self, Ac, Ad, Bc, Bd, Sc, Sd, log=None):
		self.Ac = Ac
		self.Ad = Ad
		self.Bc = Bc
		self.Bd = Bd
		self.Sc = Sc
		self.Sd = Sd

		self.ci = 0

		self.log = log

	def cycle(self):
		while True:
			# Wait until all inputs have a token
			if not self.Ac.isValid() or not self.Ad.isValid() or not self.Bc.isValid() or not self.Bd.isValid():
				return

			s = self.Ad.probe() + self.Bd.probe() + self.ci;
			if self.Ac.probe()==0 and self.Bc.probe()==0:
				self.Ac.recv()
				self.Ad.recv()
				self.Bc.recv()
				self.Bd.recv()
				self.Sc.send(0)
				self.Sd.send(int(s)&15)
				self.ci = int(s) >> 4
			elif self.Ac.probe()==0 and self.Bc.probe()==1:
				self.Ac.recv()
				self.Ad.recv()
				self.Sc.send(0)
				self.Sd.send(int(s)&15)
				self.ci = int(s) >> 4
			elif self.Ac.probe()==1 and self.Bc.probe()==0:
				self.Bc.recv()
				self.Bd.recv()
				self.Sc.send(0)
				self.Sd.send(int(s)&15)
				self.ci = int(s) >> 4
			elif self.Ac.probe()==1 and self.Bc.probe()==1 and (int(s)>>4) != self.ci:
				self.Sc.send(0)
				self.Sd.send(int(s)&15)
				self.ci = int(s) >> 4
			elif self.Ac.probe()==1 and self.Bc.probe()==1 and (int(s)>>4) == self.ci:
				self.Ac.recv()
				self.Ad.recv()
				self.Bc.recv()
				self.Bd.recv()
				self.Sc.send(1)
				self.Sd.send(int(s)&15)
				self.ci = 0

