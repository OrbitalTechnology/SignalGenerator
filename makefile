NAME := siggen
ARCH := x64
ID := debug

OUTPUT := $(NAME)-$(ARCH)-$(ID)

build :
	mkdir -p bin/
	clang++ src/*.cpp -I include/ -l ao -o bin/$(OUTPUT)

clean :	
	rm -rf bin/* || true