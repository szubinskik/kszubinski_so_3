all:
	+$(MAKE) -C src

clean:
	+$(MAKE) clean -C src

ARGS = --help
run:
	bin/main ${ARGS}
