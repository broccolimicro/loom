all:
	$(MAKE) -s $(MAKE_FLAGS) -C hseenc
	$(MAKE) -s $(MAKE_FLAGS) -C hsesim
	$(MAKE) -s $(MAKE_FLAGS) -C chpsim
	$(MAKE) -s $(MAKE_FLAGS) -C hseplot nogv
	$(MAKE) -s $(MAKE_FLAGS) -C prsim
	$(MAKE) -s $(MAKE_FLAGS) -C bubble
	$(MAKE) -s $(MAKE_FLAGS) -C gated
	$(MAKE) -s $(MAKE_FLAGS) -C prsize

test:
	$(MAKE) -s $(MAKE_FLAGS) -C hseenc test
	$(MAKE) -s $(MAKE_FLAGS) -C hsesim test
	$(MAKE) -s $(MAKE_FLAGS) -C chpsim test
	$(MAKE) -s $(MAKE_FLAGS) -C hseplot test
	$(MAKE) -s $(MAKE_FLAGS) -C prsim test
	$(MAKE) -s $(MAKE_FLAGS) -C bubble test
	$(MAKE) -s $(MAKE_FLAGS) -C gated test
	$(MAKE) -s $(MAKE_FLAGS) -C prsize test

check:
	./hseenc/test
	./hsesim/test
	./chpsim/test
	./hseplot/test
	./prsim/test
	./bubble/test
	./gated/test
	./prsize/test

install:
	cp hseenc/hseenc ${ACT_HOME}/bin/hseenc
	cp hsesim/hsesim ${ACT_HOME}/bin/hsesim
	cp hseplot/plot ${ACT_HOME}/bin/plot
	cp prsim/prsim ${ACT_HOME}/bin/hprsim
	cp bubble/bubble ${ACT_HOME}/bin/bubble
	cp gated/gated ${ACT_HOME}/bin/gated
	cp prsize/size ${ACT_HOME}/bin/size

clean:
	$(MAKE) -s $(MAKE_FLAGS) -C hseenc clean
	$(MAKE) -s $(MAKE_FLAGS) -C hsesim clean
	$(MAKE) -s $(MAKE_FLAGS) -C chpsim clean
	$(MAKE) -s $(MAKE_FLAGS) -C hseplot clean
	$(MAKE) -s $(MAKE_FLAGS) -C prsim clean
	$(MAKE) -s $(MAKE_FLAGS) -C bubble clean
	$(MAKE) -s $(MAKE_FLAGS) -C gated clean
	$(MAKE) -s $(MAKE_FLAGS) -C prsize clean


