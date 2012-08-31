#!/bin/bash
cd "`dirname \"$0\"`"

if [ -x /opt/local/bin/ctags ]; then
    CTAGS=/opt/local/bin/ctags
elif [ `uname` = Linux ]; then
    CTAGS=ctags
else
    CTAGS=sandbox-bin/ctags
fi

/usr/bin/find * \( \
    -iname '*.fail.html' -o \
    -false \
\) -prune \
-o \
\( \
    -iname '*.cpp' -o  \
    -iname '*.h' -o    \
    -iname '*.js' -o   \
    -iname '*.html' -o \
    -iname '*.py' -o   \
    -false             \
\) -print | $CTAGS -e -L- 2>&1 | grep -v "ignoring.null.tag"

TAGS=`pwd`/TAGS
echo "Updated $TAGS"
