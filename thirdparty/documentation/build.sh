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
	pushd ./build
	latexmk -pdf -synctex=1 ./cflare.tex
	popd
}

make_epub () {
	make_md
	set -x # show the commands ran
	pushd ./build
	
	ebook-convert ./cflare.md ./cflare.epub \
		--title="CFlare Documentation" \
		--authors="Kate Adams; Victor Meriqui" \
		--cover images/cover.png --preserve-cover-aspect-ratio \
		--chapter-mark=none \
		--use-auto-toc \
		--chapter="//*[((name()='h1' or name()='h2'or name()='h3')]" \
		--page-breaks-before="//*[name()='h1']" \
		--change-justification=justify \
		--epub-inline-toc \
		--level1-toc="//*[name()='h1']" \
		--level2-toc="//*[name()='h2']" \
		--level3-toc="//*[name()='h3']" \
	#	--verbose
	set +x
	
	# we need to fix up some stuff that ebook-convert does wrong/we don't like.
	echo "unpacking epub..."
	unzip ./cflare.epub -d ./cflare > /dev/null && rm ./cflare.epub
		
	# ids may not start with a hyphen, only alpha
	# ebook-convert will only remove the first hyphen, so we need to fix it.
	echo "fixing hyperlinks..."
	# id="-abc" -> id="abc"
	# then href="#-abc" -> href="#abc"
	sed -i "s/\"-/\"/g" ./cflare/*.*html
	sed -i "s/#-/#/g"   ./cflare/*.*html
	
	sed -i "s/\"-/\"/g" ./cflare/*.ncx
	sed -i "s/#-/#/g"   ./cflare/*.ncx
	
	sed -i "s/\"-/\"/g" ./cflare/*.opf
	sed -i "s/#-/#/g"   ./cflare/*.opf
	
	# modify the stylesheet
	echo "updating stylesheet..."
	echo "
h1, h2, h3, h4, h5, h6 {
	text-align: left;
	text-indent: -1em;
	padding-left: 1em;
}
code {
	text-align: left;
}
p {
	text-align: justify;
}" >> ./cflare/stylesheet.css
	
	# now rebuild the epub
	echo "repacking epub..."
	pushd ./cflare
		zip -X   ../cflare.epub mimetype      > /dev/null
		zip -grX ../cflare.epub META-INF/ *.* > /dev/null
	popd
	
	# now check that it's sane
	epubcheck ./cflare.epub
	
	popd
}

make_$1
