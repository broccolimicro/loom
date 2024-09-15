.PHONY: gdstk googletest lib install

# Use + instead of spaces in the below list
LIBS = \
	lib/common \
	lib/boolean \
	lib/arithmetic \
	lib/phy \
	lib/sch \
	lib/prs \
	lib/petri \
	lib/hse \
	lib/chp \
	lib/ucs \
	lib/parse \
	lib/parse_expression \
	lib/parse_prs \
	lib/parse_spice \
	lib/parse_astg \
	lib/parse_dot \
	lib/parse_chp \
	lib/parse_ucs \
	lib/interpret_boolean \
	lib/interpret_arithmetic \
	lib/interpret_phy \
	lib/interpret_sch \
	lib/interpret_prs \
	lib/interpret_hse \
	lib/interpret_ucs \
	lib/interpret_chp \
	bin/ckt

all: lib install

install:
	cp bin/ckt/lm .

lib: gdstk
	@$(foreach item,$(LIBS),echo "$(subst +, ,$(item))"; $(MAKE) -s $(MAKE_FLAGS) -C $(subst +, ,$(item));)

test: googletest
	@$(foreach item,$(LIBS),echo "$(subst +, ,$(item))"; $(MAKE) -s $(MAKE_FLAGS) -C $(subst +, ,$(item)) test;)

gdstk:
	$(MAKE) -s $(MAKE_FLAGS) -C lib/gdstk lib

googletest:
	mkdir -p googletest/build; cd googletest/build; cmake .. -DBUILD_GMOCK=OFF
	$(MAKE) -s $(MAKE_FLAGS) -C googletest/build

check:	
	@$(foreach item,$(LIBS),echo "$(subst +, ,$(item))"; ./$(subst +, ,$(item))/test;)

clean:
	@$(foreach item,$(LIBS),echo "$(subst +, ,$(item))"; $(MAKE) -s $(MAKE_FLAGS) -C $(subst +, ,$(item)) clean;)
	
