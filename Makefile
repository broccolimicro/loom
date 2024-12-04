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

MAINTAINER_NAME = "$(shell git config user.name)"
MAINTAINER_EMAIL = "$(shell git config user.email)"

all: lib

version:
	@git fetch --tags --no-recurse-submodules
	@git fetch origin $(shell git rev-list --tags --max-count=1)
	@git describe --tags --abbrev=0 | cut -c2- > VERSION

linux: lib
	mkdir -p lm-linux/usr/local/bin
	cp bin/ckt/lm lm-linux/usr/local/bin
	mkdir -p lm-linux/usr/local/share/
	cp -r tech lm-linux/usr/local/share
	mkdir -p lm-linux/DEBIAN
	mkdir -p debian
	touch debian/control
	echo "Package: loom" > lm-linux/DEBIAN/control
	cat VERSION | sed 's/\([0-9]\+\)\.\([0-9]\+\)\.\([0-9]\+\)/\1.\2-\3/g' | xargs -I{} echo "Version: {}" >> lm-linux/DEBIAN/control
	echo "Section: base" >> lm-linux/DEBIAN/control
	echo "Priority: optional" >> lm-linux/DEBIAN/control
	echo "Architecture: amd64" >> lm-linux/DEBIAN/control
	dpkg-shlibdeps -O lm-linux/usr/local/bin/lm | sed 's/.*Depends=/Depends: /g' >> lm-linux/DEBIAN/control
	echo "Maintainer: $(MAINTAINER_NAME) <$(MAINTAINER_EMAIL)>" >> lm-linux/DEBIAN/control
	echo "Description: Loom" >> lm-linux/DEBIAN/control
	echo " A programming language for quasi-delay insensitive asynchronous circuits" >> lm-linux/DEBIAN/control
	dpkg-deb --build --root-owner-group lm-linux
	rm -rf debian lm-linux

windows: lib
	mkdir -p Loom/bin
	mkdir -p Loom/share
	cp bin/ckt/lm.exe Loom/bin/lm.exe
	cp -r tech Loom/share/tech
	ldd Loom/bin/lm.exe | grep "mingw64" | sed 's/.*\/mingw64/\/mingw64/g' | sed 's/ (.*$$//g' | xargs -I{} cp {} Loom/bin
	zip -r lm-windows.zip Loom
	rm -rf Loom

macos: lib
	mkdir -p lm-macos/bin
	mkdir -p lm-macos/share
	cp bin/ckt/lm lm-macos/bin/lm
	cp -r tech lm-macos/share/tech
	tar -czvf lm-macos.tar.gz lm-macos

lib: gdstk
	@$(foreach item,$(LIBS),echo "$(subst +, ,$(item))"; $(MAKE) VERSION="v$(shell cat VERSION)" -s $(MAKE_FLAGS) -C $(subst +, ,$(item));)

test: googletest
	@$(foreach item,$(LIBS),echo "$(subst +, ,$(item))"; $(MAKE) -s $(MAKE_FLAGS) -C $(subst +, ,$(item)) test;)

ifeq ($(UNAME_S),Darwin)
gdstk:
	grep "CMAKE_OSX_DEPLOYMENT_TARGET" lib/gdstk/Makefile || ( \
		mv lib/gdstk/Makefile lib/gdstk/Makefile_old; \
		sed 's/Ninja/Ninja \-DCMAKE_OSX_DEPLOYMENT_TARGET\=12\.0/g' lib/gdstk/Makefile_old > lib/gdstk/Makefile)
	$(MAKE) -s $(MAKE_FLAGS) -C lib/gdstk lib
else ifeq ($(UNAME_S),Windows_NT)
gdstk:
	grep "CMAKE_PREFIX_PATH" lib/gdstk/Makefile || ( \
		mv lib/gdstk/Makefile lib/gdstk/Makefile_old; \
		sed 's/Ninja/Ninja \-DCMAKE_PREFIX_PATH\="\/mingw64\/"/g' lib/gdstk/Makefile_old > lib/gdstk/Makefile)
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
	
