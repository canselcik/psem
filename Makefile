all: build

build:
	@rake compile

clean:
	@rm -rvf tmp

test: build
	@irb -Ilib -rpsem
