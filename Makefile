#
# HALos Makefile
#
# Copyright (c) Robert J. Hilderman.
# All rights reserved.
#

COMPILER=g++
OPTS=-c -g
DEFS=
OBJS=
INCS=
LIBS=

all:		HAL9000 HALos HALkeyboardDriver HALdisplayDriver HALdiskDriver HALshell

HAL9000:	HAL9000.o HALmemory.o
		$(COMPILER) HAL9000.o HALmemory.o -o HAL9000

HALos:		HALos.o HALqueue.o HALmemory.o HALfileTable.o
		$(COMPILER) HALos.o HALqueue.o HALmemory.o HALfileTable.o -o HALos

HALkeyboardDriver: HALkeyboardDriver.o
		$(COMPILER) HALkeyboardDriver.o -o HALkeyboardDriver

HALdisplayDriver: HALdisplayDriver.o
		$(COMPILER) HALdisplayDriver.o -o HALdisplayDriver

HALdiskDriver:	HALdiskDriver.o
		$(COMPILER) HALdiskDriver.o -o HALdiskDriver

HALshell:	HALshell.o dataStructures.o
		$(COMPILER) dataStructures.o HALshell.o -o HALshell

HAL9000.o:	HAL9000.cpp HAL9000.h
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) HAL9000.cpp $(LIBS)

HALos.o: 	HALos.cpp HALos.h HALosInit.h
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) HALos.cpp $(LIBS)

HALkeyboardDriver.o: HALkeyboardDriver.cpp HALkeyboardDriver.h
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) HALkeyboardDriver.cpp $(LIBS)

HALdisplayDriver.o: HALdisplayDriver.cpp HALdisplayDriver.h
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) HALdisplayDriver.cpp $(LIBS)

HALdiskDriver.o: HALdiskDriver.cpp HALdiskDriver.h
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) HALdiskDriver.cpp $(LIBS)

HALshell.o:	HALshell.cpp HALshell.h
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) HALshell.cpp $(LIBS)

HALmemory.o:	HALmemory.cpp HALmemory.h
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) HALmemory.cpp $(LIBS)

HALqueue.o:	HALqueue.cpp HALqueue.h
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) HALqueue.cpp $(LIBS)

HALfileTable.o:	HALfileTable.cpp HALfileTable.h
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) HALfileTable.cpp $(LIBS)

dataStructures.o:	dataStructures.cpp dataStructures.cpp
		$(COMPILER) $(OPTS) $(DEFS) $(INCS) dataStructures.cpp $(LIBS)

clean:
		rm -f HAL9000
		rm -f HALos
		rm -f HALkeyboardDriver
		rm -f HALdisplayDriver
		rm -f HALdiskDriver
		rm -f HALshell
		rm -f *.o

##install:
##		cp -f HAL9000 ../bin
##		cp -f HALos ../bin
##		cp -f HALkeyboardDriver ../bin
##		cp -f HALdisplayDriver ../bin
##		cp -f HALdiskDriver ../bin
##		cp -f HALshell ../bin
