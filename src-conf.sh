#! /bin/sh
# generate src/src-conf.mk
TEXT=
case "$1" in
def) TEXT="D$2 = -D$2=1" ;;
obj) TEXT="$2_obj = $2.o" ;;
lib) TEXT="$2_lib = $2.a" ;;
set) TEXT="$2 = $3"
     shift
esac
if [ -n "$TEXT" ]
then
	[ -z "$2$3" ] && exit
	if [ -n "$3" ] && [ "x$4" = x-a ]
	then shift; shift
	fi
	if [ "x$3" = x-z ] && [ -n "$4" ]
	then TEXT="#$TEXT"
	elif [ -z "$3" ]
	then TEXT="#$TEXT"
	fi
	echo $TEXT >> src/src-conf.mk
	exit
fi
cd src
if [ -n "$1" ]
then echo CC = $1 > src-conf.mk
else echo > src-conf.mk	
fi
echo 'CCFLAGS = $(CFLAGS) $(CPPFLAGS)' >> src-conf.mk
echo '#CFLAGS = -O2 -Wall -pedantic -Wno-parentheses' >> src-conf.mk
if [ -n "$2" ]
then echo CFLAGS = $2 >> src-conf.mk
else echo CFLAGS = -O2 >> src-conf.mk
fi
[ -n "$3" ] && echo CPPFLAGS = $3 >> src-conf.mk
cd ..
