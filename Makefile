# Compatibility wrapper. Prefer using ./run.sh directly.

.PHONY: all build run dist configure clean

all: build

build:
	./run.sh build

run:
	./run.sh run

dist:
	./run.sh dist

configure:
	./run.sh configure

clean:
	./run.sh clean

push:
	git remote add origin git@github.com:kwn/miami-quack-82.git
	git push origin main
	git remote remove origin