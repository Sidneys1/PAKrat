all: pakrat2 pakratj

pakrat2.cpp: pakrat2.cpp2
	cppfront -a -verb "$<"

pakrat2: pakrat2.cpp cpp2util.h
	clang++-16 -lfmt -std=c++2b "$<" -o pakrat2

pakratj: pakratj.jakt
	jakt -J6 -d -C /usr/bin/clang++-15 -lfmt "$<"
	cp build/pakratj .
