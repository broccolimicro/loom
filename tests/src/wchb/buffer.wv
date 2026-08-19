func buffer_e(fixed Lr; wire Le; fixed Rr; wire Re) {
	region 1 {
		Lr- and i = 0
		await Le
		while {
			Lr = i
			i = i + 1
			await ~Le
			Lr-
			await Le
		}
	} and {
		Rr- and Le+
		await Re & ~Lr
		while {
			Rr = Lr
			Le-
			await ~Re & ~Lr
			Rr-
			Le+
			await Re
		}
	} and region 1 {
		Re+
		await ~Rr
		while {
			await Rr
			Re-
			await ~Rr
			Re+
		}
	}
}

proto buffer_e1of2(wire Lrf, Lrt, Le, Rrf, Rrt, Re) : buffer_e {
	region 1 {
		Lrf- and Lrt-
		await Le
		while {
			Lrf+ xor Lrt+
			await ~Le
			Lrf- and Lrt-
			await Le
		}
	} and {
		Le+ and Rrf- and Rrt-
		await Re & ~Lrf & ~Lrt
		while {
			await Re & Lrf {
				Rrf+
			} or await Re & Lrt {
				Rrt+
			}
			Le-
			await ~Re & ~Lrf & ~Lrt
			Rrf- and Rrt-
			Le+
		}
	} and region 1 {
		Re+
		await ~Rrf & ~Rrt
		while {
			await Rrf | Rrt
			Re-
			await ~Rrf & ~Rrt
			Re+
		}
	}
}
