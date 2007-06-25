#SWIG extensions
		  
swig: _eibclient.so 

eibclient_wrap.o: eibclient_wrap.c 
	$(CC) -fPIC -I/usr/include/python2.4 -I../../eibd/include $< -c -o $@	

eibclient_wrap.c: eibclient.i
	swig -python $(AM_CPPFLAGS) -I../../eibd/include eibclient.i
	ln eibclient.py eibclient

_eibclient.so: eibclient_wrap.o
	$(CC)  -fPIC -shared $(AM_CPPFLAGS) -I../../eibd/include $(SOURCES) $< -o $@ 

swig_clean:
	-rm -f _eibclient.so eibclient.pyc \
		eibclient.py eibclient_wrap.c \
		eibclient_wrap.o eibclient/eibclient.py
