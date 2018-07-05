#!/bin/bash
find . -regextype sed -regex '.*/\(Makefile\|\(.*\.[ch]\)\)' | tar -cf allfiles.tar -T - && scp allfiles.tar $1:allfiles.tar && ssh $1 <<'ENDSSH'
mkdir -p Development/cdp
cd Development/cdp
rm -rf *
tar xf ../../allfiles.tar
rm -f ../../allfiles.tar
cd module
make
ENDSSH
rm -f allfiles.tar