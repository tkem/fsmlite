# Makefile for Sphinx documentation

EXTRA_DIST = Doxyfile conf.py index.rst requirements.txt

BUILDDIR = build
PAPER =
SPHINXOPTS =

# Internal variables.
PAPEROPT_a4 = -D latex_paper_size=a4
PAPEROPT_letter = -D latex_paper_size=letter
ALLSPHINXOPTS  = -d $(BUILDDIR)/doctrees $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) .
# the i18n builder cannot share the environment and doctrees with the others
I18NSPHINXOPTS = $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) .

clean-local:
	rm -rf $(BUILDDIR) doxyxml

doxygen:
	$(DOXYGEN)

html: doxygen
	$(SPHINXBUILD) -b html $(ALLSPHINXOPTS) $(BUILDDIR)/html

latex: doxygen
	$(SPHINXBUILD) -b latex $(ALLSPHINXOPTS) $(BUILDDIR)/latex

man: doxygen
	$(SPHINXBUILD) -b man $(ALLSPHINXOPTS) $(BUILDDIR)/man

pdf: latex
	$(MAKE) -C $(BUILDDIR)/latex all-pdf
