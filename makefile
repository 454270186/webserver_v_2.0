.PHONY: all
all: build

.PHONY: build
build:
	mkdir -p build
	cd build && \
	cmake .. && \
	make

.PHONY: run
run:
	./build/http_server

.PHONY: clean
clean:
	rm -rf build