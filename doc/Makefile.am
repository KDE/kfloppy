
docs_DATA = index.html index-1.html index-2.html index-3.html index-4.html \
	logotp3.png

docsdir = $(kde_htmldir)/en/kfloppy

EXTRA_DIST = $(docs_DATA) index.sgml index.nif

doc: index.sgml index.nif
	ksgml2html index en
