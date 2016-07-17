all:
	mkdir -p html
	pandoc -f markdown -t html markdown/index.md -o html/index.html

run: all
	python -m SimpleHTTPServer
