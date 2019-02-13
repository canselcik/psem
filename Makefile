all: build

gem:
	@gem build psem.gemspec

build:
	@rake compile

clean:
	@rm -rvf tmp || true
	@rm -rvf *.gem || true

test: build
	@irb -Ilib -rpsem
