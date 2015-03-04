#!/bin/bash

set -e

cd `dirname "$0"`

[[ -d ./build ]] && rm -rf ./build/*

mkdir -p ./build
touch ./build/cflare.md
cp -r ../images ./build/images

VERSION=`git describe --tags --always | egrep -o "[0-9\.]+(\-[0-9]+)?" | head -1 | tr "-" "."`

make_ () {
	echo "$0: error: build target not specified." > /dev/stderr
	exit 1
}

make_md () {
	DOCS=`find ./src -type f | sort --field-separator="/" --key=3n`
	for DOC in $DOCS; do
		if [[ -x "$DOC" ]]; then
			echo "exec $DOC ..."
			"./$DOC" >> ./build/cflare.md
		else
			echo "copy $DOC ..."
			cat $DOC >> ./build/cflare.md
		fi
	
		# ensure a newline is placed between the files...
		echo >> ./build/cflare.md
	done
}

make_tex () {
	make_md
	set -x # show the commands ran
	pandoc -s -t latex ./build/cflare.md -o ./build/cflare.tex
	./fix-tex ./build/cflare.tex $VERSION
}

make_pdf () {
	make_tex
	cd ./build
	latexmk -pdf -synctex=1 ./cflare.tex
	cd -
}

make_$1
