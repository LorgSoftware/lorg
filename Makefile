release:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
	mv build/lorg lorg

debug:
	mkdir -p build-debug
	cd build-debug && cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build .
	mv build-debug/lorg-debug lorg-debug

clean:
	rm -rf build lorg
	rm -rf build-debug lorg-debug

.PHONY: release debug clean
