# Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src tests docs

EXTRA_DIST = LICENSE README.md

# Travis CI uses `make test` by default
test: check

man:
	$(MAKE) -C docs man
