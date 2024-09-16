.PHONY: gdstk googletest lib install

ifeq ($(OS),Windows_NT)
    UNAME_S := $(OS)
else
    UNAME_S := $(shell uname -s)
endif


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

ifeq ($(UNAME_S),Darwin)
gdstk:
	grep "CMAKE_OSX_DEPLOYMENT_TARGET" lib/gdstk/Makefile || ( \
		mv lib/gdstk/Makefile lib/gdstk/Makefile_old; \
		sed 's/Ninja/Ninja \-DCMAKE_OSX_DEPLOYMENT_TARGET\=12\.0/g' lib/gdstk/Makefile_old > lib/gdstk/Makefile)
	$(MAKE) -s $(MAKE_FLAGS) -C lib/gdstk lib
else
gdstk:
	$(MAKE) -s $(MAKE_FLAGS) -C lib/gdstk lib
endif

googletest:
	mkdir -p googletest/build; cd googletest/build; cmake .. -DBUILD_GMOCK=OFF
	$(MAKE) -s $(MAKE_FLAGS) -C googletest/build

check:	
	@$(foreach item,$(LIBS),echo "$(subst +, ,$(item))"; ./$(subst +, ,$(item))/test;)

clean:
	@$(foreach item,$(LIBS),echo "$(subst +, ,$(item))"; $(MAKE) -s $(MAKE_FLAGS) -C $(subst +, ,$(item)) clean;)
	
