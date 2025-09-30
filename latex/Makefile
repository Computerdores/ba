.PHONY: default build clean format open

default: build

build: build/src.tar.gz
	mkdir -p build
	find . -mindepth 1 -maxdepth 1 -not -path './build' -not -path './.git' -exec cp -r {} build/ \; # copy everything except for .git/, build/ into build/
	cd build; latexmk thesis.tex

open: build
	xdg-open build/main.pdf

# Create archive of source code, minus some builds to minimise archive size
build/src.tar.gz: $(shell find .. -type f)
	tar -czf $@ \
		--exclude=todo.md \
		--exclude=latex/build \
		--exclude=src/build \
		--exclude=src/cmake-build-debug \
		--exclude=src/cmake-build-release \
		../src ../latex ../plots ../*.md ../flake.* ../LICENSE ../.gitignore

clean:
	rm -rf build/*

format:
	tex-fmt $(shell find . -not -path './build/*' -a -name '*.tex')
