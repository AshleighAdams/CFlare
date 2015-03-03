#!/bin/bash

set -e

cd `dirname "$0"`

[[ -f cflare.md ]] && rm cflare.md
touch cflare.md

DOCS=`find ./src -type f | sort --field-separator="/" --key=3n`
for DOC in $DOCS; do
	if [[ -x "$DOC" ]]; then
		echo "exec $DOC ..."
		"./$DOC" >> cflare.md
	else
		echo "copy $DOC ..."
		cat $DOC >> cflare.md
	fi
	
	# ensure a newline is placed between the files...
	echo >> cflare.md
done
