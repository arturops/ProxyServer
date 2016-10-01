
CC=g++
RM=rm
SRC=main.cpp HttpPkt.cpp HttpHdr.cpp HttpGetForwarder.cpp FilterHelper.cpp CacheStorageHelper.cpp CacheSearchHelper.cpp CacheManagement.cpp
CFLAGS=-std=c++11 -lpthread -g  

all : ${SRC}
	${CC} $^ -o artoz ${CFLAGS}

clean :
	${RM} artoz  
	${RM} -rf *.dSYM	
