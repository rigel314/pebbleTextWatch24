#!/bin/sh

# simple test for top level directory of a pebble project.
if [[ ! -e script/readable.js ]]; then
	exit 1
fi

# read js.html, remove indents, remove newlines, and replace "'" with "\'".
html=$(sed -re 's/^[ \t]*//' script/js.html)
html=$(echo "$html" | sed -re ':a;N;$!ba;s/\n//g')
html=${html//"'"/"\\'"}

# read readable.js, replace $REPLACEHTML with html from above.
js=$(cat script/readable.js)
js=${js//"\$REPLACEHTML"/"'$html'"}

# store in the appropriate file.
echo "$js" > src/js/pebble-js-app.js
