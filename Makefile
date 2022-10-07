build:
	mkdir -p build
	cd build && cmake .. && cmake --build .
	mv build/lorg lorg

clean:
	rm -rf build lorg

.PHONY: build clean
