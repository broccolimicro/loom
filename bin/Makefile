all:
	$(MAKE) -s $(MAKE_FLAGS) -C ckt
	$(MAKE) -s $(MAKE_FLAGS) -C gated
	$(MAKE) -s $(MAKE_FLAGS) -C prsize

test:
	$(MAKE) -s $(MAKE_FLAGS) -C ckt test
	$(MAKE) -s $(MAKE_FLAGS) -C gated test
	$(MAKE) -s $(MAKE_FLAGS) -C prsize test

check:
	./ckt/test
	./gated/test
	./prsize/test

install:
	cp ckt/ckt ${ACT_HOME}/bin/ckt
	cp gated/gated ${ACT_HOME}/bin/gated
	cp prsize/size ${ACT_HOME}/bin/size

clean:
	$(MAKE) -s $(MAKE_FLAGS) -C ckt clean
	$(MAKE) -s $(MAKE_FLAGS) -C gated clean
	$(MAKE) -s $(MAKE_FLAGS) -C prsize clean


