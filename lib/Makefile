.PHONY: libs parse interpret

all: libs parse interpret

libs:
	$(MAKE) -s $(MAKE_FLAGS) -C common
	$(MAKE) -s $(MAKE_FLAGS) -C boolean
	$(MAKE) -s $(MAKE_FLAGS) -C arithmetic
	$(MAKE) -s $(MAKE_FLAGS) -C phy
	$(MAKE) -s $(MAKE_FLAGS) -C sch
	$(MAKE) -s $(MAKE_FLAGS) -C prs
	$(MAKE) -s $(MAKE_FLAGS) -C petri
	$(MAKE) -s $(MAKE_FLAGS) -C hse
	$(MAKE) -s $(MAKE_FLAGS) -C chp
	$(MAKE) -s $(MAKE_FLAGS) -C ucs

parse:
	$(MAKE) -s $(MAKE_FLAGS) -C parse
	$(MAKE) -s $(MAKE_FLAGS) -C parse_expression
	$(MAKE) -s $(MAKE_FLAGS) -C parse_prs
	$(MAKE) -s $(MAKE_FLAGS) -C parse_spice
	$(MAKE) -s $(MAKE_FLAGS) -C parse_astg
	$(MAKE) -s $(MAKE_FLAGS) -C parse_dot
	$(MAKE) -s $(MAKE_FLAGS) -C parse_chp
	$(MAKE) -s $(MAKE_FLAGS) -C parse_ucs
	$(MAKE) -s $(MAKE_FLAGS) -C gdstk lib

interpret:
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_boolean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_arithmetic
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_phy
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_sch
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_prs
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_hse
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_ucs
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_chp

test:
	$(MAKE) -s $(MAKE_FLAGS) -C common test
	$(MAKE) -s $(MAKE_FLAGS) -C boolean test
	$(MAKE) -s $(MAKE_FLAGS) -C arithmetic test
	$(MAKE) -s $(MAKE_FLAGS) -C prs test
	$(MAKE) -s $(MAKE_FLAGS) -C petri test
	$(MAKE) -s $(MAKE_FLAGS) -C hse test
	$(MAKE) -s $(MAKE_FLAGS) -C chp test
	$(MAKE) -s $(MAKE_FLAGS) -C ucs test
	$(MAKE) -s $(MAKE_FLAGS) -C parse test
	$(MAKE) -s $(MAKE_FLAGS) -C parse_expression test
	$(MAKE) -s $(MAKE_FLAGS) -C parse_prs test
	$(MAKE) -s $(MAKE_FLAGS) -C parse_spice test
	$(MAKE) -s $(MAKE_FLAGS) -C parse_astg test
	$(MAKE) -s $(MAKE_FLAGS) -C parse_dot test
	$(MAKE) -s $(MAKE_FLAGS) -C parse_chp test
	$(MAKE) -s $(MAKE_FLAGS) -C parse_ucs test
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_boolean test
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_arithmetic test
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_prs test
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_hse test
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_ucs test
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_chp test

check:	
	./common/test
	./boolean/test
	./arithmetic/test
	./prs/test
	./petri/test
	./hse/test
	./chp/test
	./ucs/test
	./parse/test
	./parse_expression/test
	./parse_prs/test
	./parse_spice/test
	./parse_astg/test
	./parse_dot/test
	./parse_chp/test
	./parse_ucs/test
	./interpret_boolean/test
	./interpret_arithmetic/test
	./interpret_prs/test
	./interpret_hse/test
	./interpret_ucs/test
	./interpret_chp/test

clean:
	$(MAKE) -s $(MAKE_FLAGS) -C common clean
	$(MAKE) -s $(MAKE_FLAGS) -C boolean clean
	$(MAKE) -s $(MAKE_FLAGS) -C arithmetic clean
	$(MAKE) -s $(MAKE_FLAGS) -C phy clean
	$(MAKE) -s $(MAKE_FLAGS) -C sch clean
	$(MAKE) -s $(MAKE_FLAGS) -C prs clean
	$(MAKE) -s $(MAKE_FLAGS) -C petri clean
	$(MAKE) -s $(MAKE_FLAGS) -C hse clean
	$(MAKE) -s $(MAKE_FLAGS) -C chp clean
	$(MAKE) -s $(MAKE_FLAGS) -C ucs clean
	$(MAKE) -s $(MAKE_FLAGS) -C parse clean
	$(MAKE) -s $(MAKE_FLAGS) -C parse_expression clean
	$(MAKE) -s $(MAKE_FLAGS) -C parse_prs clean
	$(MAKE) -s $(MAKE_FLAGS) -C parse_spice clean
	$(MAKE) -s $(MAKE_FLAGS) -C parse_astg clean
	$(MAKE) -s $(MAKE_FLAGS) -C parse_dot clean
	$(MAKE) -s $(MAKE_FLAGS) -C parse_chp clean
	$(MAKE) -s $(MAKE_FLAGS) -C parse_ucs clean
	$(MAKE) -s $(MAKE_FLAGS) -C gdstk clean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_boolean clean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_arithmetic clean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_phy clean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_sch clean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_prs clean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_hse clean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_ucs clean
	$(MAKE) -s $(MAKE_FLAGS) -C interpret_chp clean
	
