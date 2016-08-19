.PHONY: pdf

all: pdf

html:
	mkdir -p html
	pandoc -f markdown -t html markdown/index.md -o html/index.html

pdf:
	mkdir -p pdf
	pandoc markdown/index.md -o pdf/hds.pdf -t latex --latex-engine=xelatex --number-sections --smart --template=template/pm-template -V mainfont="Microsoft YaHei" --trace
	@#pandoc markdown/index.md -o out/bootstrap.pdf -t latex --latex-engine=xelatex --toc --smart --template=template -V mainfont="Microsoft YaHei" --trace
	@#pandoc markdown/index.md -o out/bootstrap.pdf -t latex --latex-engine=xelatex --smart --template=template-letter.tex -V mainfont="Microsoft YaHei" --trace

run: all
	python -m SimpleHTTPServer
