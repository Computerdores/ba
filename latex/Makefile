.PHONY: default build clean format open

default: build

build: src.tar.gz
	mkdir -p build
	find . -mindepth 1 -maxdepth 1 -not -path './build' -not -path './.git' -exec cp -r {} build/ \; # copy everything except for .git/, build/ into build/
	cd build; latexmk main.tex

open: build
	xdg-open build/main.pdf

# Create archive of source code, minus some builds to minimise archive size
src.tar.gz: $(shell find ../src -type f)
	tar -czf $@ -C .. \
		--exclude=src/todo.md \
		--exclude=src/build \
		--exclude=src/cmake-build-debug \
		--exclude=src/cmake-build-release \
		src

clean:
	rm -rf build/*
	rm src.tar.gz

format:
	tex-fmt $(shell find . -not -path './build/*' -a -name '*.tex')
