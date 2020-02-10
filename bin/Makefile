all:
	$(MAKE) -s $(MAKE_FLAGS) -C hseenc
	$(MAKE) -s $(MAKE_FLAGS) -C hsesim
	#$(MAKE) -s $(MAKE_FLAGS) -C chpsim
	$(MAKE) -s $(MAKE_FLAGS) -C hseplot nogv
	$(MAKE) -s $(MAKE_FLAGS) -C prsim
	$(MAKE) -s $(MAKE_FLAGS) -C prsize

clean:
	$(MAKE) -s $(MAKE_FLAGS) -C hseenc clean
	$(MAKE) -s $(MAKE_FLAGS) -C hsesim clean
	#$(MAKE) -s $(MAKE_FLAGS) -C chpsim clean
	$(MAKE) -s $(MAKE_FLAGS) -C hseplot clean
	$(MAKE) -s $(MAKE_FLAGS) -C prsim clean
	$(MAKE) -s $(MAKE_FLAGS) -C prsize clean


