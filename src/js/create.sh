#!/bin/sh

# simple test for top level directory of a pebble project.
if [[ ! -e readable.js ]]; then
	cd src/js/
fi

if [[ ! -e readable.js ]]; then
	exit 1
fi

# read js.html, remove indents, remove newlines, and replace "'" with "\'".
html=$(sed -re 's/^[ \t]*//' js.html)
html=$(echo "$html" | sed -re ':a;N;$!ba;s/\n//g')
html=${html//"'"/"\\'"}

# read readable.js, replace $REPLACEHTML with html from above.
js=$(cat readable.js)
js=${js//"\$REPLACEHTML"/"'$html'"}

# store in the appropriate file.
echo "$js" > pebble-js-app.js
