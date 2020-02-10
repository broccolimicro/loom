.PHONY: libs parse interpret

all: libs parse interpret

libs:
	$(MAKE) -s $(MAKE_FLAGS) -C common
	$(MAKE) -s $(MAKE_FLAGS) -C boolean
	$(MAKE) -s $(MAKE_FLAGS) -C arithmetic
	$(MAKE) -s $(MAKE_FLAGS) -C prs
	$(MAKE) -s $(MAKE_FLAGS) -C petri
	$(MAKE) -s $(MAKE_FLAGS) -C hse
	#$(MAKE) -s $(MAKE_FLAGS) -C chp
	$(MAKE) -s $(MAKE_FLAGS) -C ucs

parse:
	$(MAKE) -s $(MAKE_FLAGS) -C parse
	$(MAKE) -s $(MAKE_FLAGS) -C parse_expression
	$(MAKE) -s $(MAKE_FLAGS) -C parse_prs
	$(MAKE) -s $(MAKE_FLAGS) -C parse_astg
	$(MAKE) -s $(MAKE_FLAGS) -C parse_dot
	$(MAKE) -s $(MAKE_FLAGS) -C parse_chp
	$(MAKE) -s $(MAKE_FLAGS) -C parse_ucs

interpret:
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_boolean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_arithmetic
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_prs
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_hse
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_ucs
	
