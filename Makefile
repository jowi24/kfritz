VERSION = $(shell grep 'static const char \*VERSION *=' KFritz.cpp | awk '{ print $$6 }' | sed -e 's/[";]//g')
WDIR    = $(shell pwd)

cmake: LibFritzI18N.cpp
	@if test ! -d build; \
	then mkdir -p build; \
	cd build; cmake ..; \
	fi

cmake-debug: LibFritzI18N.cpp
	@if test ! -d build; \
	then mkdir -p build; \
	cd build; cmake -DCMAKE_BUILD_TYPE:STRING=Debug ..; \
	fi

all: cmake 
	cd build; make 
	
debug: cmake-debug 
	cd build; make 

clean:
	@-make -C libfritz++ clean
	@-rm ../kfritz-${VERSION}.orig.tar.gz
	@-rm -rf build
	
dist: clean update-po
	tar cvz --dereference --exclude=l10n-kde4 \
	        --exclude-vcs --exclude="debian" --exclude=".settings" --exclude=".project" \
	        --exclude=".cproject" --exclude=".cdtproject" --exclude="test" --exclude=".git*" \
	        -f ../kfritz_${VERSION}.orig.tar.gz ../kfritz
	        
fetch-po:
	@if test ! -d l10n-kde4; \
	then svn co --depth=immediates svn+ssh://joachimwilke@svn.kde.org/home/kde/trunk/l10n-kde4/ ; \
	for LCODE in `cat l10n-kde4/subdirs`; do svn up --set-depth=empty l10n-kde4/$$LCODE/messages/ ; done ; \
	for LCODE in `cat l10n-kde4/subdirs`; do svn up --set-depth=files l10n-kde4/$$LCODE/messages/playground-network/ ; done ; \
	fi
	
update-po: fetch-po
	svn up l10n-kde4
	for LCODE in `cat l10n-kde4/subdirs`; do \
	if test -e l10n-kde4/$$LCODE/messages/playground-network/kfritz.po; then \
	cp l10n-kde4/$$LCODE/messages/playground-network/kfritz.po po/$$LCODE.po ; fi ; done
	
LibFritzI18N.cpp: libfritz++/*.cpp
	mv $@ $@.old 
	cat $@.old | grep -v i18n > $@
	rm $@.old
	grep -ir I18N_NOOP libfritz++/*.cpp | sed -e 's/.*I18N_NOOP(\([^)]*\).*/i18n(\1)/' | sort | uniq >> $@

kde-install: all
	cd build; kdesudo make install

kde-install-debug: debug
	cd build; kdesudo make install

install:
	cd build; sudo make install

deb: dist
	debuild -i"(\.svn|\.settings|\.(c|cdt|)project|test)"

deb-src: dist
	debuild -S -i"(\.svn|\.settings|\.(c|cdt|)project|test)"
