#! /bin/sh

user=`whoami | tr '/' ' '`
host=`hostname | tr '/' ' '`
uname=`uname -a | tr '/' ' '`
date=`date | tr '/' ' '`
compiler=`gcc -v 2>&1 | tail -1 | tr '/' ' '`
eval `cat VERSION`
version="$VERSION"
year=`date | awk '{print $(NF)}' | tr '/' ' '`

sed "s/%USER%/$user/g;s/%HOST%/$host/g;s/%UNAME%/$uname/g;
     s/%DATE%/$date/g;s/%COMPILER%/$compiler/g;
     s/%VERSION%/$version/g;s/%YEAR%/$year/g;"

exit 0

