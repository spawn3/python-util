.PHONY: pdf

all: pdf

pdf:
	mkdir -p pdf
	pandoc markdown/remotecopy.md -o pdf/remotecopy.pdf -t latex --latex-engine=xelatex --number-sections --smart --template=template/pm-template -V mainfont="Microsoft YaHei" --trace

run:
	python -m SimpleHTTPServer
