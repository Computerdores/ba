.PHONY: default build clean format open

default: build

build:
	mkdir -p build
	find . -mindepth 1 -maxdepth 1 -not -path './build' -not -path './.git' -exec cp -r {} build/ \; # copy everything except for .git/, build/ into build/
	cd build && latexmk main.tex; exit_code=$$?; chown --reference=. -R .; exit $$exit_code

open: build
	xdg-open build/main.pdf

clean:
	rm -rf build/*

format:
	tex-fmt $(shell find . -not -path './build/*' -a -name '*.tex')
