.SUFFIXES : # Delete the default suffixes

BASE	= rapport.tex
SOURCES = $(wildcard *.tex)


all:	mermaid	docker

pdf:	$(SOURCES)
	pdflatex -output-format pdf $(BASE)
	pdflatex -output-format pdf $(BASE)

# --- Compilation via Docker ---
docker: 
	@echo -e "\033[1;33mCompilation via Docker...\033[0m"
	@mkdir -p build
	@docker run -i --rm --user $$(id -u):$$(id -g) --name latex \
		-v "$$(pwd)/..":/usr/src/app \
		-w /usr/src/app/rapport \
		texlive/texlive:latest \
		latexmk -pdf --file-line-error --synctex=1 --shell-escape \
		-output-directory=build \
		-interaction=nonstopmode \
		$(BASE)
	@echo -e "\033[1;32mCompilation via Docker terminée.\033[0m"
	@cp build/rapport.pdf rapport.pdf
# ------------------------------------------------

# compiler mermaid diagrams :  docker run --rm -u `id -u`:`id -g` -v ./:/data  minlag/mermaid-cli -i diagramm.mmd -o diagramm.pdf
# mermaid diagramm are in images/ directory
mermaid: 
	@echo -e "\033[1;33mCompiling Mermaid diagrams...\033[0m"
	@for file in images/*.mmd ; do \
		docker run --rm -u $$(id -u):$$(id -g) -v "$$(pwd)/..":/data minlag/mermaid-cli -i /data/rapport/$$file -o /data/rapport/$${file%.mmd}.pdf -f ; \
	done
	@echo -e "\033[1;32mMermaid diagrams compilation finished.\033[0m"


clean:
	rm -f *.bak *~
	rm -f *.log *.aux *.bbl *.blg *~ *.bak *.dvi *.out \
	*.toc *.lof *.lot *.brf
	rm -rf build/

