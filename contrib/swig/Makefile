#SWIG extensions
PYTHON_VERSION=$(shell echo -e "import sys\nprint sys.version[:3]" |python -)

swig: _eibclient.so 

eibclient_wrap.o: eibclient_wrap.c 
	$(CC) -fPIC -I/usr/include/python$(PYTHON_VERSION) -I../../eibd/include $< -c -o $@	

eibclient_wrap.c: eibclient.i
	swig -python $(AM_CPPFLAGS) -I../../eibd/include eibclient.i
	ln eibclient.py eibclient

_eibclient.so: eibclient_wrap.o
	$(CC)  -fPIC -shared $(AM_CPPFLAGS) -I../../eibd/include $(SOURCES) $< -o $@ -L../../eibd/client/c/ -leibclient

swig_clean:
	-rm -f _eibclient.so eibclient.pyc \
		eibclient.py eibclient_wrap.c \
		eibclient_wrap.o eibclient/eibclient.py
