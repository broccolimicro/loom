////// DSA
//func decomp_dsa_before(chan A, B) {
//	var fixed x, y, z;
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		x = 3;
//		z = 5 + x;
//		x = A.recv();
//		B.send(x)
//	}
//}
//
//
//func decomp_dsa_after(chan A, B) {
//	var fixed x0, x1, y, z;
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		x0 = 3;
//		z = 5 + x0;
//		x1 = A.recv();
//		B.send(x1)
//	}
//}
//
//
//// Sequential Buffers
//func decomp_first_before(chan A, B, C, D, E) {
//	var fixed x, y, z;
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		x = A.recv();
//		y = B.recv();
//		C.send(x + 1);
//		z = D.recv();
//		E.send(y + z)
//	}
//}
//
//
//func decomp_first_after1(chan B, D, E) {
//	var fixed y, z;
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		y = B.recv();
//		z = D.recv();
//		E.send(y + z)
//	}
//}
//
//
//func decomp_first_after2(chan A, C) {
//	var fixed x;
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		x = A.recv();
//		C.send(x + 1)
//	}
//}


//// Linear Subprograms
//func decomp_second_before(chan A, P, Q, X, Y) {
//	var fixed p, x, v;
//	v = 3;
//	x = 5;
//	p = A.recv()
//
//	while {
//		v = 0;  //TODO: temp debug reset
//		v = v + 1;  //TODO: temp debug reset
//		v = 2 * v  //TODO: temp debug reset
//
//		if v == 0 {
//			v = A.recv()
//
//		} or if !(v == 0) {
//			v = v - 1;
//			v = v - 1;
//			v = v - 1
//		}
//
//		if v > 2 {
//			x = X.recv();
//			Y.send(x / v)
//
//		} or if !(v > 2) {
//			p = P.recv();
//			Q.send(p * v)
//		}
//	}
//}


//func decomp_P1(chan A, B, X, Y) {
//	var fixed a,b,_
//	while {
//		_=0;_=0;
//		a = A.recv();
//		b = B.recv();
//		X.send(a+2);
//		Y.send(3 * b)
//	}
//}


func decomp_Q1(chan A, B, C, X) {
	var fixed a,b,c,d,y,_

	while {
		//_=0;_=0;
		a = A.recv() and
			b = B.recv() and
			c = C.recv();
		X.send(a + b)

		if c {
			d = D.recv();
			y = b + c + d

		} or if !c {
			skip
		}
	}
}


//func decomp_Q2(chan A, B, C, X) {
//	var fixed a,b,bf,b0,b1,c,cf,c0,c1,d,y,_
//
//	while {
//		//_=0;_=0;
//		a = A.recv() and
//			b = B.recv() and
//			c = C.recv();
//		bf = b;
//		b0 = bf;
//		b1 = bf;
//		cf = c;
//		c0 = cf;
//		c1 = cf;
//		X.send(a + b0)
//		
//		if c0 & c1 {
//			d = D.recv();
//			y = b1 + c0 + d
//
//		} or if !c0 & !c1 {
//			skip
//		}
//	}
//}


//func decomp_Q3(chan A, A_cp_X, B, B_fork, B_cp_X, C, C_fork, C_cp_d, C_cp_y, D, D_cp_y, X) {
//	var fixed a
//
//	while {
//		a = A.recv() and
//			b = B.recv() and
//			c = C.recv();
//
//		B_fork.send(b) and
//			B_fork.recv(b_fork) and
//			C_fork.send(c) and
//			C_fork.recv(c_fork);
//
//		A_cp_X.send(a) and
//			a_cp_X = A_cp_X.recv() and
//			B_cp_X.send(b_fork) and
//			b_cp_X = B_cp_X.recv() and
//			C_cp_d.send(c_fork) and
//			c_cp_d = C_cp_d.recv() and
//			C_cp_y.send(c_fork) and
//			c_cp_y = C_cp_y.recv();
//
//		X.send(a_cp_X + b_cp_X)
//
//		if c_cp_d and c_cp_y {
//			d = D.recv();
//			D_cp_y.send(d) and
//				d_cp_y = D_cp_y.recv();
//			y = b_cp_y + c_cp_y + d_cp_y
//
//		} or if !(c_cp_d and c_cp_y) {
//			skip
//		}
//	}
//}


//func decomp_second_mid(chan A, P, Q, X, Y, V) {
//	var fixed p, x, v, v1;
//	v = 0;
//
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		if v == 0 {
//			v = A.recv()
//
//		} or if !(v == 0) {
//			v = v - 0
//		}
//
//		V.send(v)
//		v1 = V.recv()
//
//		if v1 > 2 {
//			x = X.recv();
//			Y.send(x / v1)
//
//		} or if !(v1 > 2) {
//			p = P.recv();
//			Q.send(p * v1)
//		}
//	}
//}
//
//
//func decomp_second_after1(chan A, P, Q, X, Y, V) {
//	var fixed p, x, v, v1;
//	v = 0;
//
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		if v == 0 {
//			v = A.recv()
//
//		} or if !(v == 0) {
//			v = v - 0
//		}
//
//		V.send(v)
//	}
//}
//
//
//func decomp_second_after2(chan A, P, Q, Y, V) {
//	var fixed v1;
//
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		v1 = V.recv();
//
//		if v1 > 2 {
//			x = X.recv();
//			Y.send(x / v1)
//
//		} or if !(v1 > 2) {
//			p = P.recv();
//			Q.send(p * v1)
//		}
//	}
//}
//
//
//// Conditional Router
//// Simplifiying initial assumption: Start at T0 from program start
////digraph G {
////  c -> c0, c1, c2, c3;
////  z -> øz;
////  y -> øy;
////  x -> øx;
////  c0 -> øx;
////  c1 -> øy;
////  c2 -> øy, øz;
////  øx -> X;
////  øy -> Y, YZ;
////  øz -> YZ;
////}
//func decomp_third_before(chan A, B, C, D, X, Y, Z) {
//	var fixed c, x, y, z;
//
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		x = A.recv();
//		y = B.recv();
//		z = D.recv();
//		c = C.recv();
//
//		if c == 0 {
//			X.send(x)
//
//		} or if c == 1 {
//			Y.send(y)
//
//		} or if c == 2 {
//			Z.send(y + z)
//
//		} or if c == 3 {
//			skip
//		}
//	}
//}
//
//func decomp_third_after1(chan A, C1, X) {
//	var fixed c1, x;
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		x = A.recv();
//		c1 = C1.recv();
//
//		if c1 == 0 {
//			X.send(x)
//
//		} or if !(c1 == 0) {  // else
//			skip
//		}
//	}
//}
//
//
//func decomp_third_after2(chan B, C, C1, D, Y, Z) {
//	var fixed c, y, z;
//	while {
//		v=0;  //TODO: temp debug reset
//		v=0;  //TODO: temp debug reset
//		y = B.recv();
//		z = D.recv();
//		c = C.recv();
//		C1.send(c);
//
//		if c == 1 {
//			Y.send(y)
//
//		} or if c == 2 {
//			Z.send(y + z)
//
//		} or if c == 3 {
//			skip
//
//		} or if !(c == 0) & !(c == 2) & !(c == 3) {  // else
//			skip
//		}
//	}
//}
