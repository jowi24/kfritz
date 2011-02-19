#!/bin/sh
$EXTRACTRC `find . -name "*.ui" -o -name "*.kcfg"` >> rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/kfritz.pot
rm -rf rc.cpp
