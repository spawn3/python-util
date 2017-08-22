% class file for ``LaTeX for Complete Novices''
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{novices}

\newif\ifscreen
\screenfalse

\newcommand*{\extra@stuff}{}

\newcommand{\dosummarymark}[1]{%
  \def\@summarymarker{#1}%
  \@onelevel@sanitize\@summarymarker
  \edef\do@writeout{%
     \noexpand\protected@write\noexpand\@auxout{}{%
       \string\summarymarker{\noexpand\thepage}{\@summarymarker}%
     }%
  }%
  \do@writeout
}

% Default is report
\def\novices@loadsty{novices-a4paper}
\def\novices@ptsize{12pt}
\newcount\novicemedia
\def\NOVICEreport{0}
\def\NOVICEscreen{1}
\def\NOVICEbook{2}
\novicemedia=\NOVICEreport\relax
\PassOptionsToClass{chapterprefix=true,oneside,captions=tableheading,numbers=noenddot}{scrbook}%

\DeclareOption{report}{%
  \screenfalse
  \def\novices@loadsty{novices-a4paper}%
  \def\novices@ptsize{12pt}%
  \novicemedia=\NOVICEreport\relax
  \PassOptionsToClass{chapterprefix=true,oneside,captions=tableheading,numbers=noenddot}{scrbook}%
}
\DeclareOption{html}{%
  \screenfalse
  \def\novices@loadsty{novices-html}%
  \def\novices@ptsize{12pt}%
  \novicemedia=\NOVICEreport\relax
  \PassOptionsToClass{chapterprefix=true,oneside,captions=tableheading,numbers=noenddot}{scrbook}%
  \renewcommand*{\extra@stuff}{%
    \hypersetup{linkcolor=black,citecolor=black,urlcolor=black}%
    \renewcommand{\tableref}[1]{\tablename~\ref{##1}}%
    \renewcommand{\vfigureref}[1]{\figurename~\ref{##1}}%
    \renewcommand{\figureref}[1]{\figurename~\ref{##1}}%
  }%
  \renewcommand{\dosummarymark}[1]{}%
}
\DeclareOption{screen}{%
  \screentrue
  \def\novices@loadsty{novices-screen}%
  \def\novices@ptsize{8pt}
  \novicemedia=\NOVICEscreen\relax
  \PassOptionsToClass{chapterprefix=false,oneside,headings=small,captions=tableheading,numbers=noenddot}{scrbook}%
}
\DeclareOption{book}{%
  \screenfalse
  \def\novices@loadsty{novices-book}%
  \def\novices@ptsize{10pt}
  \novicemedia=\NOVICEbook\relax
  \PassOptionsToClass{chapterprefix=true,twoside,headings=normal,captions=tableheading,numbers=noenddot}{scrbook}%
}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{scrbook}}

\ProcessOptions

\LoadClass{scrbook}

\expandafter\RequirePackage\expandafter[\novices@ptsize]{extsizes}

\RequirePackage{graphicx}
\RequirePackage{amsmath}

% Requires at least version 1.08 of the flowfram package
\RequirePackage{\novices@loadsty}

\ifnum\novicemedia=\NOVICEbook
 \setlength{\marginparwidth}{0.7in}
 \newlength\@even@offset
 \setlength{\@even@offset}{0.3in}

 \setflowframe*{main,header,left,leftshort}{margin=outer}

 \@flowframeid{right}
 \expandafter\addtolength\csname col@\romannumeral\ff@id @evenx\endcsname{\@even@offset}
 \@flowframeid{rightshort}
 \expandafter\addtolength\csname col@\romannumeral\ff@id @evenx\endcsname{\@even@offset}

 \@flowframeid{left}
 \expandafter\addtolength\csname col@\romannumeral\ff@id @evenx\endcsname{\@even@offset}
 \@flowframeid{leftshort}
 \expandafter\addtolength\csname col@\romannumeral\ff@id @evenx\endcsname{\@even@offset}

 \@flowframeid{main}
 \expandafter\addtolength\csname col@\romannumeral\ff@id @evenx\endcsname{\@even@offset}

 \@dynamicframeid{header}
 \expandafter\addtolength\csname @df@\romannumeral\ff@id @evenx\endcsname{\@even@offset}

 \@dynamicframeid{footer}
 \expandafter\addtolength\csname @df@\romannumeral\ff@id @evenx\endcsname{\@even@offset}

 \@dynamicframeid{indextab}
 \expandafter\computeleftedgeeven
   \csname @df@\romannumeral\ff@id @evenx\endcsname
% \expandafter\addtolength
%   \csname @df@\romannumeral\ff@id @evenx\endcsname
%   {0mm}%
 \expandafter\addtolength
   \csname @df@\romannumeral\ff@id @posx\endcsname
   {3mm}%

 \setdynamiccontents{\ff@id}{%
   \color{white}\LARGE\bfseries\scshape\centering
   \ifodd\c@page\relax
     \hspace*{-1ex}\rotatebox[origin=cb]{-90}{Index}%
   \else
     \hspace*{1ex}\rotatebox[origin=cb]{90}{Index}%
   \fi
   \par
 }

 \@dynamicframeid{summarytab}
 \expandafter\computeleftedgeeven
   \csname @df@\romannumeral\ff@id @evenx\endcsname
% \expandafter\addtolength
%   \csname @df@\romannumeral\ff@id @evenx\endcsname
%   {0mm}%
 \expandafter\addtolength
   \csname @df@\romannumeral\ff@id @posx\endcsname
   {3mm}%

 \setdynamiccontents{\ff@id}{%
   \color{white}\LARGE\bfseries\scshape\centering
   \ifodd\c@page\relax
     \hspace*{-1ex}\rotatebox[origin=cb]{-90}{Summary}%
   \else
     \hspace*{1ex}\rotatebox[origin=cb]{90}{Summary}%
   \fi
   \par
 }

\fi

\RequirePackage[T1]{fontenc}
\RequirePackage{pifont}
\RequirePackage{ifthen}
\RequirePackage{ifpdf}
\RequirePackage{url}
\RequirePackage[french]{datetime}
\RequirePackage{etoolbox}
\RequirePackage{makeidx}

% html package conflicts with varioref+hyperref, so emulate it:
\RequirePackage{comment}
\specialcomment{htmlonly}{}{}
\excludecomment{htmlonly}
\newenvironment{latexonly}{\ignorespaces}{\ignorespacesafterend}
\newenvironment{makeimage}{}{}
\newcommand{\latex}[1]{#1}
\newcommand{\latexhtml}[2]{#1}
\newcommand{\html}[1]{}
\newcommand{\htmlimage}[2]{}
\newcommand{\mathorit}[1]{\ensuremath{#1}}

\RequirePackage{varioref}
\RequirePackage[hyperindex=false]{hyperref}
\RequirePackage[toc,subentrycounter,
   sanitize={sort=false,description=true},
   acronym,indexonlyfirst,
   counter=section,numberedsection=nolabel,savewrites]{glossaries}
\usepackage{glossary-inline}

% don't check for makeindex special characters, as they're dealt
% with explicitly
\let\@gls@checkmkidxchars\@gobble

\longdate

\ifnum\novicemedia=\NOVICEbook
  \hypersetup{draft}
\else
  \hypersetup{colorlinks,plainpages=false,
  linkcolor=blue,menucolor=blue,citecolor=blue,
  urlcolor=magenta,filecolor=magenta}
\fi

\newcommand{\vfigureref}[1]{\figurename~\vref{#1}}

\ifnum\novicemedia=\NOVICEbook
 \newcommand{\figureref}[1]{\figurename~\vref{#1}}
 \newcommand{\tableref}[1]{\tablename~\vref{#1}}
\else
  \newcommand{\figureref}[1]{\hyperlink{#1top}{\figurename~\ref*{#1}}}
  \newcommand{\tableref}[1]{\htmlref{\tablename~\ref*{#1}}{#1}}
\fi

\extra@stuff

\robustify\htmlref

\renewcommand*{\headfont}{\em}

\ifnum\novicemedia=\NOVICEscreen\relax
  \renewcommand*{\footfont}{\small\scshape}
\else
  \renewcommand*{\footfont}{\em}
\fi

\newcommand*{\leftfootmark}{}
\newcommand*{\rightfootmark}{}
\renewcommand*{\ps@headings}{%
  \let\@mkboth\markboth
  \renewcommand*{\@evenhead}{\normalfont\headfont\thepage\hfill\leftmark}%
  \renewcommand*{\@oddhead}{\normalfont\headfont\rightmark\hfill\thepage}%
  \renewcommand*{\@evenfoot}{\normalfont\footfont\leftfootmark\@leftfoothook\hfill}%
  \renewcommand*{\@oddfoot}{\hfill\normalfont\footfont\rightfootmark\@rightfoothook}%
}

\renewcommand*{\ps@plain}{%
  \renewcommand*{\@oddhead}{}%
  \renewcommand*{\@evenhead}{}%
  \renewcommand*{\@oddfoot}{\normalfont\footfont\mbox{}\hfill\thepage\@rightfoothook}%
  \renewcommand*{\@evenfoot}{\normalfont\footfont\thepage\hfill\mbox{}\@leftfoothook}%
}

\pagestyle{headings}

\newcommand*{\@leftfoothook}{}
\newcommand*{\@rightfoothook}{}

\newcommand*{\setleftfootmark}[1]{}

\newcommand*{\@setleftfootmark}[1]{%
  \gdef\leftfootmark{#1}%
  \global\let\setleftfootmark\@gobble
}

\newcommand*{\setrightfootmark}[1]{}

\newcommand*{\@setrightfootmark}[1]{%
  \gdef\rightfootmark{#1}%
}

\ifnum\novicemedia=\NOVICEbook
  \newcommand*{\enablefoothooks}{%
    \let\setrightfootmark\@setrightfootmark
    \let\setleftfootmark\@setleftfootmark
    \renewcommand*{\@rightfoothook}{%
      \global\let\setleftfootmark\@setleftfootmark
    }%
    \renewcommand*{\@leftfoothook}{%
      \global\let\setleftfootmark\@setleftfootmark
    }%
  }
\else
  \newcommand*{\enablefoothooks}{}
\fi

\newcommand*{\disablefoothooks}{%
  \global\let\leftfootmark\@empty
  \global\let\rightfootmark\@empty
  \global\let\setrightfootmark\@gobble
  \global\let\setleftfootmark\@gobble
  \global\let\@leftfoothook\@empty
  \global\let\@rightfoothook\@empty
}

\providecommand{\abstractname}{Abstract}

\newcommand*{\@edition}{}
\newcommand*{\edition}[1]{\renewcommand*{\@edition}{#1}}
\newcommand*{\theversion}{}
\newcommand*{\version}[1]{\renewcommand*{\theversion}{#1}}
\newcommand*{\thevolume}{}
\newcommand*{\volume}[1]{\renewcommand*{\thevolume}{#1}}
\newcommand*{\theseries}{}
\newcommand*{\series}[1]{\renewcommand*{\theseries}{#1}}

\newcommand*{\theseriesurl}{}
\newcommand*{\seriesurl}[1]{\renewcommand*{\theseriesurl}{#1}}

\newcommand{\navigationbar}{{%
\ifthenelse{\equal{\thepage}{a}}{\textcolor[gray]{.75}{Previous}}{%
\Acrobatmenu{PrevPage}{Previous}}\enspace
\ifthenelse{\boolean{lastpg}}{\textcolor[gray]{.75}{Next}}{%
\Acrobatmenu{NextPage}{Next}}\enspace
\Acrobatmenu{FirstPage}{First}\enspace
\Acrobatmenu{LastPage}{Last}\enspace
\Acrobatmenu{GoBack}{Back}\enspace
\Acrobatmenu{GoForward}{Forward}\enspace
\htmlref{Summary}{ch:glossary}\enspace
\htmlref{Index}{ch:index}}}

\newcommand{\ps@online}{%
\renewcommand{\@oddhead}{{\normalfont\headfont\rightmark}\hfil}%
\renewcommand{\@evenhead}{{\normalfont\hfil\headfont\leftmark}}%
\renewcommand{\@oddfoot}{\normalfont\footfont\hfill
\navigationbar
\hfill\thepage}
\renewcommand{\@evenfoot}{\@oddfoot}}


\ifscreen
\pagestyle{online}

\renewcommand{\ps@plain}{%
\renewcommand{\@oddhead}{}%
\renewcommand{\@evenhead}{}%
\renewcommand{\@oddfoot}{\normalfont\footfont\hfill
\navigationbar
\hfill\thepage}
\renewcommand{\@evenfoot}{\@oddfoot}}

\renewcommand{\ps@empty}{%
\renewcommand{\@oddhead}{}%
\renewcommand{\@evenhead}{}%
\renewcommand{\@oddfoot}{\normalfont\footfont\hfill
\navigationbar \hfill\mbox{}}
\renewcommand{\@evenfoot}{\@oddfoot}}

\fi

\ifnum\novicemedia=\NOVICEscreen\relax
  \renewcommand*{\chapterformat}{%
  \chapappifchapterprefix{\ }\textup{\thechapter}.\enspace}
  \addtokomafont{chapter}{\centering\rmfamily\scshape}
\else
  \renewcommand*{\chapterformat}{%
  \chapappifchapterprefix{\ }\thechapter}
  \addtokomafont{chapter}{\raggedleft\rmfamily\scshape}
\fi

\addtokomafont{chapterentry}{\large\rmfamily\scshape}
\addtokomafont{chapterentrypagenumber}{\large\rmfamily\upshape}
\addtokomafont{section}{\rmfamily\bfseries}
\addtokomafont{subsection}{\rmfamily\bfseries}
\addtokomafont{subsubsection}{\rmfamily\bfseries}
\addtokomafont{minisec}{\rmfamily\bfseries\scshape}

\ifnum\novicemedia=\NOVICEreport
  \renewcommand{\chapterheadstartvskip}{%
    \vspace*{1.5\baselineskip}%
  }
\fi

\renewcommand*\l@chapter[2]{%
  \ifnum\c@tocdepth>\m@ne
    \ifnum\lastpenalty<20010
      \addpenalty{-\@highpenalty}%
    \fi
    \vskip 1.0em \@plus \p@ 
    \setlength \@tempdima{2em}%
    \if@tocleft
      \ifx\toc@l@number\@empty
      \else
        \setlength \@tempdima{0\toc@l@number}%
      \fi
    \fi 
    \begingroup
      \raggedchapterentry
      \parindent\z@
      \advance\rightskip\@pnumwidth 
      \parfillskip -\@pnumwidth
      \interlinepenalty\@M
      \leavevmode
      \advance\leftskip \@tempdima
      \null\nobreak
      \hskip -\leftskip
      \usekomafont{chapterentry}%
      {%
        #1\nobreak
        \usekomafont{chapterentrypagenumber}%
        {%
          \hfill
          \nobreak
          \hb@xt@\@pnumwidth{\hss #2}%
        }%
      }%
      \par
      \ifnum\scr@compatibility>\@nameuse{scr@v@2.96}%
       \relax
       \endgroup
       \penalty 20009
     \else
       \penalty\@highpenalty
       \endgroup
     \fi
   \fi
}

\renewcommand*{\l@section}{%
  \bprot@dottedtocline {1}{1.5em}{3em}%
}

\renewcommand*{\captionformat}{\space}

\renewcommand{\title}[2][]{\ifthenelse{\equal{#1}{}}{%
\renewcommand{\@pdftitle}{#2}}{
\ifthenelse{\equal{\@pdftitle}{}}{\renewcommand{\@pdftitle}{#1}}{}}%
\renewcommand{\@title}{#2}}

\renewcommand{\author}[2][]{\ifthenelse{\equal{#1}{}}{%
\ifthenelse{\equal{\@pdfauthor}{}}{%
\renewcommand{\@pdfauthor}{#2}}{}}{\renewcommand{\@pdfauthor}{#1}}%
\renewcommand{\@author}{#2}}

\newcommand{\keywords}[1]{\renewcommand{\@pdfkeywords}{#1}}

\providecommand{\subject}[1]{%
\renewcommand{\@pdfsubject}{#1}}

\newcommand*{\@affiliation}{}
\newcommand*{\thepublisher}{\@affiliation}
\newcommand*{\@affiliationurl}{}
\newcommand*{\@affiliationaddress}{}

\newcommand*{\affiliation}[3]{%
  \renewcommand{\@affiliation}{#1}%
  \renewcommand{\@affiliationurl}{\url{#2}}%
  \renewcommand{\@affiliationaddress}{#3}%
}

\ifpdf
 \DeclareGraphicsExtensions{.pdf,.png,.jpg}
\else
 \DeclareGraphicsExtensions{.ps,.eps}
\fi

\ifnum\novicemedia=\NOVICEbook
  % book doesn't have sidepanel
\else
  % get the dimensions of sidepanel (values are stored in \ffareax, \ffareay, \ffareawidth and \ffareaheight)
  \getdynamicbounds*{sidepanel}

  % set the margin width to be the same as the width of sidepanel
  \setlength{\marginparwidth}{\ffareawidth}
\fi

\newcommand*{\index@preamble}{}
\newcommand{\indexpreamble}[1]{%
  \renewcommand{\index@preamble}{#1}%
}

% renew theindex environment so that it appears in the table of contents
% and set up index frames

\renewenvironment{theindex}
{%
  \clearpage
  \begingroup
   \normalfont\sectfont
   \chapterheadstartvskip
   \phantomsection
   \size@chapter{\indexname}%
   \thispagestyle{plain}%
   \label{ch:index}%
   \par
  \endgroup
  \markboth{\indexname}{\indexname}%
  \addcontentsline{toc}{chapter}{\indexname}%
  \parindent\z@
  \index@preamble\par
  \enablefoothooks
%  \parskip\z@ \@plus .3\p@\relax
  \let\item\@idxitem
  \ifnum\novicemedia=\NOVICEbook\relax
    \setleftfootmark{\doindextopmark}%
    \setrightfootmark{\doindexbottommark}%
  \else
    \setdynamiccontents*{sidepanel}{%
      \hfil\ifscreen\normalsize\else\large\fi\indexbar\hfil}%
  \fi
  \ifscreen\small\fi
  \newpage
  \ignorespacesafterend
}
{%
  \protected@write\@auxout{}{\string\@flow@endindex{\thepage}}%
  \protected@write\@auxout{}{\string\@flow@old@last@idx@page{\number\@last@idx@page}}%
  \setrightfootmark{\doindexbottommark\disablefoothooks}%
  \setleftfootmark{\doindextopmark\disablefoothooks}%
}

\newcount\@first@idx@page
\newcount\@last@idx@page
\newcommand*{\@flow@endindex}[1]{\global\@last@idx@page=#1\relax}

\newcommand*{\@old@last@idx@page}{-1}
\newcommand*{\@flow@old@last@idx@page}[1]{%
  \gdef\@old@last@idx@page{#1}%
}

\newcount\@last@summary@page
\newcommand*{\@flow@endsummary}[1]{%
  \global\@last@summary@page=#1\relax
  \global\@first@idx@page=\@last@summary@page
  \global\advance\@first@idx@page by 1\relax
  \ifnum\novicemedia=\NOVICEbook\relax
    \ifodd\@first@idx@page
      \relax
    \else
      \global\advance\@first@idx@page by 1\relax
    \fi
  \fi
}

\newcount\@first@summary@page

\newcount\@last@acr@page
\newcommand*{\@flow@endacr}[1]{\global\@last@acr@page=#1\relax}

\newcommand*{\@flow@startsummary}[1]{%
  \global\@first@summary@page=#1\relax
   \ifnum\novicemedia=\NOVICEbook\relax
     \ifodd\@first@summary@page
       \relax
     \else
       \global\advance\@first@summary@page by 1\relax
    \fi
  \fi
}


% Put glossary in double-column

\newcommand*{\setdoublecolumnglo}{%
  \let\glossarysection\dblglossarysection
}

\newcount\tmp@count

\newcommand{\@setup@pages}{%
  \ifnum\novicemedia=\NOVICEbook\relax
    \setdynamicframe*{summarytab}{pages={\number\@first@summary@page-\number\@last@summary@page}}%
    \setdynamicframe*{indextab}{pages={\number\@first@idx@page-\number\@last@idx@page}}%
  \else
    \setdynamicframe*{sidepanel}%
      {pages={\number\@first@summary@page-\number\@last@summary@page,%
              \number\@first@idx@page-\number\@last@idx@page}}%
  \fi
  \ifnum\@first@summary@page>0\relax
    \@ff@tmpN=\@first@summary@page
    \advance\@ff@tmpN by 1\relax
    \tmp@count=\@first@idx@page
    \advance\tmp@count by 1\relax
    \edef\@nv@set@flf{\noexpand\setflowframe*{left,right}%
      {pages={\number\@ff@tmpN-\number\@last@summary@page
             ,\number\tmp@count-\number\@last@idx@page}}}%
    \@nv@set@flf
    \setflowframe*{main}%
      {pages={<\number\@last@acr@page,\number\@last@acr@page,>\number\@last@idx@page}}
    \setflowframe*{header,leftshort,rightshort}%
      {pages={\number\@first@summary@page,\number\@first@idx@page}}
  \fi
}

\AtBeginDocument{\@setup@pages}

\newcommand*{\dblglossarysection}[2][\@gls@title]{%
  \doifnotbook
  {%
    \setdynamiccontents*{sidepanel}{%
      \hfil\ifscreen\normalsize\else\large\fi\summarybar\hfil}%
  }%
  \clearpage
  \protected@write\@auxout{}{\string\@flow@startsummary{\thepage}}%
  \begingroup
  \normalfont\sectfont
  \chapterheadstartvskip
  \phantomsection
  \size@chapter{#2}%
  \thispagestyle{plain}%
  \label{ch:glossary}%
  \par
  \endgroup
  \markboth{\glossarytoctitle}{\glossarytoctitle}%
  \addcontentsline{toc}{chapter}{#1}%
  \vskip\baselineskip
  \noindent
  \emph{\summarypreamble}\par
  \newpage
}

\def\summarypreamble{}

\newif\iflastpg
\AtEndDocument
{%
  \lastpgtrue
  \ifnum\@last@idx@page=\@old@last@idx@page
    \relax
  \else
    \ClassWarningNoLine{novices}{Rerun required to get frames up-to-date}%
  \fi
}

\renewcommand{\maketitle}{%
  \pagenumbering{alph}% for PDF bookmarks
  \ifpdf
    \pdfinfo
    {%
       /Author (\@pdfauthor)
       /Title  (\@pdftitle)
       /Keywords (\@pdfkeywords)
       /Subject  (\@pdfsubject)
       /ModDate (D:\pdfdate)
       /CreationDate (D:20040706000000)
    }%
  \fi
  \begin{titlepage}%
   \@maketitle
  \end{titlepage}%
  \thispagestyle{empty}%
  \newpage
  \bgroup
    \thecopyrighttext
    \par
  \egroup
  \setcounter{footnote}{0}%
  \global\let\thetitle\@title
  \global\let\theauthor\@author
  \global\let\thanks\relax
  \global\let\maketitle\relax
  \global\let\@thanks\@empty
  \global\let\@author\@empty
  \global\let\@date\@empty
  \global\let\@title\@empty
  \global\let\title\relax
  \global\let\author\relax
  \global\let\date\relax
  \global\let\and\relax
  \normalfont
}

\providecommand{\theauthoryear}{}
\providecommand{\theauthorsurname}{}
\providecommand{\theauthorforenames}{}

\newcommand*{\authordetails}[3]{%
  \renewcommand*{\theauthoryear}{#1}%
  \renewcommand*{\theauthorsurname}{#2}%
  \renewcommand*{\theauthorforenames}{#3}%
  \renewcommand*{\@author}{#3\ #2}%
}

\newcommand\thetitle{\@title}

\providecommand*{\maketitlehook}{}

\providecommand{\thecopyrighttext}{}

\newcommand{\copyrighttext}[1]{%
  \appto\thecopyrighttext{#1}%
}

\ifcase\novicemedia

  % report version
  \newcommand{\indexbar}{%
    \begin{tabular}[t]{|c}
      \idxheaderlink{Symbols} \\
      \idxheaderlink{A} \\
      \idxheaderlink{B} \\
      \idxheaderlink{C} \\
      \idxheaderlink{D} \\
      \idxheaderlink{E} \\
      \idxheaderlink{F} \\
      \idxheaderlink{G} \\
      \idxheaderlink{H} \\
      \idxheaderlink{I} \\
      \idxheaderlink{J} \\
      \idxheaderlink{K} \\
      \idxheaderlink{L} \\
      \idxheaderlink{M} \\
      \idxheaderlink{N} \\
      \idxheaderlink{O} \\
      \idxheaderlink{P} \\
      \idxheaderlink{Q} \\
      \idxheaderlink{R} \\
      \idxheaderlink{S} \\
      \idxheaderlink{T} \\
      \idxheaderlink{U} \\
      \idxheaderlink{V} \\
      \idxheaderlink{W} \\
      \idxheaderlink{X} \\
      \idxheaderlink{Y} \\
      \idxheaderlink{Z}
    \end{tabular}%
  }

  \newcommand{\summarybar}{%
    \begin{tabular}[t]{|c}
      \glsheaderlink{glssymbols} \\
      \glsheaderlink{A} \\
      \glsheaderlink{B} \\
      \glsheaderlink{C} \\
      \glsheaderlink{D} \\
      \glsheaderlink{E} \\
      \glsheaderlink{F} \\
      \glsheaderlink{G} \\
      \glsheaderlink{H} \\
      \glsheaderlink{I} \\
      \glsheaderlink{J} \\
      \glsheaderlink{K} \\
      \glsheaderlink{L} \\
      \glsheaderlink{M} \\
      \glsheaderlink{N} \\
      \glsheaderlink{O} \\
      \glsheaderlink{P} \\
      \glsheaderlink{Q} \\
      \glsheaderlink{R} \\
      \glsheaderlink{S} \\
      \glsheaderlink{T} \\
      \glsheaderlink{U} \\
      \glsheaderlink{V} \\
      \glsheaderlink{W} \\
      \glsheaderlink{X} \\
      \glsheaderlink{Y} \\
      \glsheaderlink{Z}
    \end{tabular}%
  }

  \renewcommand{\@maketitle}{%
  \let\footnotesize\small
  \let\footnoterule\relax
  \let \footnote \thanks
  \null\vfil
  \vskip 60\p@
  \begin{center}%
    {\LARGE \@title \par}%
    \vskip 2ex%
    {\large Version \theversion\par}%
    \vskip 3em%
    {\large
     \lineskip .75em%
      \begin{tabular}[t]{c}%
        \@author
      \end{tabular}\par}%
      \vskip 1.5em%
    {\large 
       \begin{tabular}[t]{c}
       \@affiliation\\
       \@affiliationurl
       \end{tabular}\par}%
      \vskip 1.5em%
    {\large \@date \par}%       % Set date in \large size.
  \end{center}\par
  \@thanks
  \vfil\null
}

\or
  % screen version
  \newcommand{\indexbar}{%
    \begin{tabular}[t]{|cc}
    \multicolumn{2}{|c}{\idxheaderlink{Symbols}}\\
    \idxheaderlink{A} &
    \idxheaderlink{N} \\
    \idxheaderlink{B} &
    \idxheaderlink{O} \\
    \idxheaderlink{C} &
    \idxheaderlink{P} \\
    \idxheaderlink{D} &
    \idxheaderlink{Q} \\
    \idxheaderlink{E} &
    \idxheaderlink{R} \\
    \idxheaderlink{F} &
    \idxheaderlink{S} \\
    \idxheaderlink{G} &
    \idxheaderlink{T} \\
    \idxheaderlink{H} &
    \idxheaderlink{U} \\
    \idxheaderlink{I} &
    \idxheaderlink{V} \\
    \idxheaderlink{J} &
    \idxheaderlink{W} \\
    \idxheaderlink{K} &
    \idxheaderlink{X} \\
    \idxheaderlink{L} &
    \idxheaderlink{Y} \\
    \idxheaderlink{M} &
    \idxheaderlink{Z}
    \end{tabular}%
  }

  \newcommand{\summarybar}{%
    \begin{tabular}[t]{|cc}
    \multicolumn{2}{|c}{\glsheaderlink{glssymbols}}\\
    \glsheaderlink{A} &
    \glsheaderlink{N} \\
    \glsheaderlink{B} &
    \glsheaderlink{O} \\
    \glsheaderlink{C} &
    \glsheaderlink{P} \\
    \glsheaderlink{D} &
    \glsheaderlink{Q} \\
    \glsheaderlink{E} &
    \glsheaderlink{R} \\
    \glsheaderlink{F} &
    \glsheaderlink{S} \\
    \glsheaderlink{G} &
    \glsheaderlink{T} \\
    \glsheaderlink{H} &
    \glsheaderlink{U} \\
    \glsheaderlink{I} &
    \glsheaderlink{V} \\
    \glsheaderlink{J} &
    \glsheaderlink{W} \\
    \glsheaderlink{K} &
    \glsheaderlink{X} \\
    \glsheaderlink{L} &
    \glsheaderlink{Y} \\
    \glsheaderlink{M} &
    \glsheaderlink{Z}
    \end{tabular}%
  }

  \renewcommand\@maketitle{%
  \let\footnotesize\small
  \let\footnoterule\relax
  \let \footnote \thanks
  \null\vfil
  \begin{center}%
    {\LARGE \@title \par}%
    \vskip 2ex%
    {\large Version \theversion\par}%
    \vskip 3em%
    {\large
     \lineskip .75em%
      \begin{tabular}[t]{c}%
        \@author
      \end{tabular}\par}%
      \vskip 1.5em%
      {\large
       \begin{tabular}[t]{c}
       \@affiliation\\
       \@affiliationurl
       \end{tabular}\par}%
      \vskip 1.5em%
    {\large \@date \par}%       % Set date in \large size.
  \end{center}\par
  \@thanks
  \vfil\null
}

\or
 % book version

 \let\indexbar\relax
 \let\summarybar\relax

 \renewcommand\@maketitle{%
   \null\par
   \vskip4ex
   \centering
   \bfseries\scshape
   \Huge
   \@title
   \vskip4ex
   \Large
   \@author
   \par
   \vfill
   \normalsize
   Volume~\thevolume~of\par
   \theseries
   \par
   \vfill
   \maketitlehook
   \Large
   \mdseries
   \vskip2ex
   \@affiliation
   \par
   \vskip1ex
   \small
   \@affiliationaddress
   \par
   \vskip4ex
   \par\null
 }

\fi

\newlength\indexheaderskip

\setlength{\indexheaderskip}{2ex plus .5ex minus .5ex}

\newcommand{\indexheader}[1]{%
 \par\noindent
 \vskip\indexheaderskip
 \makebox[\linewidth][c]%
 {%
   \ifnum\novicemedia=\NOVICEbook\relax
     \textbf{#1}%
   \else
     \hypertarget{idx:#1}{\textbf{#1}}%
     \protected@write\@auxout{}{\string\idx@header{#1}}%
   \fi
 }%
 \\*[\indexheaderskip]%
}


\newcommand*{\idx@header}[1]{%
  \csgdef{idx@header@#1}{\hyperlink{idx:#1}{\textbf{#1}}}%
}

\newcommand*{\gls@header}[1]{%
  \csgdef{gls@header@#1}{\hyperlink{gls:#1}{\textbf{\glsgetgrouptitle{#1}}}}%
}

\newcommand*{\idxheaderlink}[1]{%
  \ifcsdef{idx@header@#1}%
  {\csname idx@header@#1\endcsname}%
  {\textbf{\color{gray}#1}}%
}

\newcommand*{\glsheaderlink}[1]{%
  \ifcsdef{gls@header@#1}%
  {\csname gls@header@#1\endcsname}%
  {\textbf{\color{gray}\glsgetgrouptitle{#1}}}%
}

\newtoks\indexmarktok

\newcommand{\doindexmark}[1]{%
  \def\@indexmarker{#1}%
  \@onelevel@sanitize\@indexmarker
  \edef\do@writeout{%
     \noexpand\protected@write\noexpand\@auxout{}{%
       \string\indexmarker{\noexpand\thepage}{\@indexmarker}%
     }%
  }%
  \do@writeout
}

\newcommand{\indexmarker}[2]{%
  \ifcsundef{@index@top@marker@\romannumeral#1}%
  {\csgdef{@index@top@marker@\romannumeral#1}{#2}}%
  {}%
  \csgdef{@index@bottom@marker@\romannumeral#1}{#2}%
}

\newcommand*{\doindextopmark}{%
  \csname @index@top@marker@\romannumeral\c@page\endcsname
}

\newcommand*{\doindexbottommark}{%
  \csname @index@bottom@marker@\romannumeral\c@page\endcsname
}

\newcommand{\summarymarker}[2]{%
  \ifcsundef{@summary@top@marker@\romannumeral#1}%
  {\csgdef{@summary@top@marker@\romannumeral#1}{#2}}%
  {}%
  \csgdef{@summary@bottom@marker@\romannumeral#1}{#2}%
}

\newcommand*{\dosummarytopmark}{%
  \csname @summary@top@marker@\romannumeral\c@page\endcsname
}

\newcommand*{\dosummarybottommark}{%
  \csname @summary@bottom@marker@\romannumeral\c@page\endcsname
}

\newlength\indexitemskip
\setlength\indexitemskip{1pt plus .5pt}

\ifnum\novicemedia=\NOVICEbook

  \newcommand{\mainindexitem}[2]{%
    \vbox{\item #1\doindexmark{#1}\space\hfill#2}%
    \par\vskip\indexitemskip
  }

\else
  \newcommand{\mainindexitem}[2]{%
    \vbox{\item #1\space\hfill#2}\par
    \par\vskip\indexitemskip
  }

\fi

\newcommand{\subindexitem}[2]{\vbox{\subitem #1\space\hfill #2}\par}
\newcommand{\subsubindexitem}[2]{\vbox{\subsubitem #1\space\hfill #2}\par}

% put headers and footers into the dynamic frames labelled
% 'header' and 'footer'
\renewcommand{\@dothehead}{}%
\renewcommand{\@dothefoot}{}%
\renewcommand{\@dodynamicthehead}{}
\renewcommand{\@dodynamicthefoot}{}

\ifnum\novicemedia=\NOVICEbook\relax
  \setdynamiccontents*{header}{\vfill\ifodd\c@page\relax\@oddhead\else\@evenhead\fi\vfill}
  \setdynamiccontents*{footer}{\vfill\ifodd\c@page\relax\@oddfoot\else\@evenfoot\fi\vfill}
\else
  \setdynamiccontents*{header}{\vfill\@oddhead\vfill}
  \setdynamiccontents*{footer}{\vfill\@oddfoot\vfill}
\fi

\newcommand{\@frontmatter}{\mbox{}\par\vfill\framebreak\clearpage
\pagenumbering{roman}%
\setflowframe*{title}{pages=none}}

\@ifundefined{frontmatter}
 {%
   \let\frontmatter\@frontmatter
 }
 {%
   \let\orgfrontmatter\frontmatter
   \def\frontmatter{\@frontmatter\orgfrontmatter}
 }

\newwrite\tf@hloe

\newcommand{\listofexercises}{%
  \chapter*{List of Exercises}
  \@starttoc{loe}
  \immediate\openout\tf@hloe \jobname.hloe\relax
  \immediate\write\tf@hloe{\string\begin{exerciselist}}%
}

\AtEndDocument{\immediate\write\tf@hloe{\string\end{exerciselist}}}

\newcommand{\l@exercise}{\@dottedtocline{1}{1.5em}{2.3em}}

\newcommand*{\htmlexerciseref}[4]{\htmlref{#1}{#2}}
\newcommand*{\htmlexerciseitem}{\item \htmlexerciseref}

\newenvironment{exerciselist}{\begin{enumerate}}{\end{enumerate}}

\newcommand*{\@html@nodename}{\jobname}
\newcommand*{\setnode}[1]{\renewcommand*{\@html@nodename}{#1}}

\newcounter{exercise}
%\begin{exercise}[html backref]{title}{label}
\newenvironment{exercise}[3][\@htmlex@backref]{%
  \def\@htmlex@backref{\@html@nodename}%
  \vskip10pt plus 5pt minus 1pt\relax
  \par\noindent
  \refstepcounter{exercise}\label{#3}%
  \textbf{\large Exercise \theexercise\ (#2)}%
  \par\nopagebreak
  \vspace{10pt}\nopagebreak
  \def\@the@exercisetitle{#2}%
  \@onelevel@sanitize\@the@exercisetitle
  \immediate\write\tf@hloe{\string\htmlexerciseitem{\@the@exercisetitle}{#3}{#1}{\theexercise}}%
  \addcontentsline{loe}{exercise}{\protect\numberline{\theexercise}{\@the@exercisetitle}}\nopagebreak
}%
{%
  \vskip5pt plus 2pt minus 1pt\relax
  \par\noindent\null\hfill
  \rule{1in}{1pt}%
  \hfill\null\par
  \vskip5pt plus 5pt minus 1pt\relax
}

\newcommand{\faqlink}[2]{%
  \href{http://www.tex.ac.uk/cgi-bin/texfaq2html?label=#2}{#1}%
}

\newcommand{\faq}[2]{%
  \marginpar
  {%
    \parbox[t]{\marginparwidth}{%
      \raggedright
      \ifcase\novicemedia
        \small
      \or
        \footnotesize
      \or
       \footnotesize\em
      \fi
      [FAQ: \faqlink{#1}{#2}]%
    }\par
  }%
}

\newcommand{\ukenglishdate}{%
  \renewcommand{\fmtord}[1]{##1}%
  \renewcommand*{\formatdate}[3]{%
  \@day=##1\relax\@month=##2\relax\@year=##3\relax
  \ordinalnum{\the\@day}\ \monthname[\@month] \the\@year}%
}

\DeclareMathOperator{\card}{card}
\DeclareMathOperator{\diag}{diag}
\DeclareMathOperator*{\mode}{mode}

\newcommand*{\refname}{References}

\newcommand*{\backslashcircumsort}{%
  \string\~%
}

\newcommand*{\backslashtildesort}{%
  \string\~%
}

% option format
\newcommand*{\optfmt}[1]{\texttt{#1}}

% display and index class files:

\DeclareRobustCommand*{\icls}[1]{%
  \cls{#1}%
  \index{class files (.cls)@class files (\texttt{.cls})!#1@\cls{#1}|hyperpage}%
  \index{#1@\cls{#1} class|hyperpage}%
}

\DeclareRobustCommand*{\clsopt}[1]{%
 \optfmt{#1}%
 \index{class file options!#1@\optfmt{#1}|hyperpage}%
}

\let\nxclsopt\clsopt


% display and index package:
\DeclareRobustCommand*{\isty}[1]{\sty{#1}\index{#1 package@\sty{#1}
package|hyperpage}}

\DeclareRobustCommand*{\istystart}[1]{\sty{#1}\index{#1
package@\sty{#1} package|(hyperpage}}
\DeclareRobustCommand*{\istyend}[1]{\sty{#1}\index{#1
package@\sty{#1} package|)hyperpage}}

% display option, index package and option
% \istyopt{package}{option}
\DeclareRobustCommand*{\istyopt}[2]{\optfmt{#2}%
\index{#1 package@\sty{#1} package!#2@\optfmt{#2}|hyperpage}}

% KOMA Script class option
\DeclareRobustCommand*{\scrclsopt}[2][]{%
 \ifstrempty{#1}%
 {%
   \optfmt{#2}%
 }%
 {%
   \optfmt{#2\discretionary{}{}{}=\discretionary{}{}{}#1}%
 }%
 \index{KOMA Script class options!#2@\optfmt{#2}|hyperpage}%
 \index{class file options!#2@\optfmt{#2} (KOMA)|hyperpage}%
}

\let\nxscrclsopt\scrclsopt

% Command key-val option
% \ikeyvalopt{csname}{key}
\DeclareRobustCommand*{\ikeyvalopt}[2]{\index{#1@\protect\cmdname{#1}!#2@\texttt{#2}|hyperpage}\texttt{#2}}
\DeclareRobustCommand*{\keyvalopt}[1]{\texttt{#1}}

% index page style
\newcommand{\idxpagestyle}[1]{\index{page
style!#1@\texttt{#1}|hyperpage}}
% display and index page style
\DeclareRobustCommand*{\ipagestyle}[1]{\pagestylefmt{#1}\idxpagestyle{#1}}

\newcommand{\pagestylefmt}[1]{\texttt{#1}}

\newcommand{\pagenumberingfmt}[1]{\texttt{#1}}
% index page numbering
\newcommand{\indexpagenumbering}[1]{%
  \index{page numbering!#1@\pagenumberingfmt{#1}|hyperpage}%
}
% display page numbering
\newcommand{\ipagenumbering}[1]{%
  \pagenumberingfmt{#1}\indexpagenumbering{#1}%
}

% display and index unit of measurement
\newcommand*{\iunit}[2]{\texttt{#1}\index{#1 (#2)@\texttt{#1}
(#2)|hyperpage}\index{units!#1 (#2)@\texttt{#1} (#2)|hyperpage}}

\renewcommand{\see}[2]{\par\qquad\emph{\seename} #1}

\newcommand{\ibackslashcircumdef}{\cmdname{\textasciicircum}%
\index{\string\^@\protect\cmdname{\protect\textasciicircum}|indexdef}}

\newcommand{\ibackslashtildedef}{\cmdname{\textasciitilde}%
\index{\string\~@\protect\cmdname{\protect\textasciitilde}|indexdef}}

\newcommand{\iquestiondowncmd}{\glsi{questiondown}}
\newcommand{\iexclamdowncmd}{\glsi{exclamdown}}

\newcommand*{\boldc}{\ensuremath{\boldsymbol{c}}}

\newcommand*{\supdag}{\textsuperscript{\dag}}
\newcommand*{\supddag}{\textsuperscript{\ddag}}
\newcommand*{\supdagddag}{\textsuperscript{\dag\ddag}}

\edef\percentsym{\expandafter\@gobble\string\%}
\edef\hashsym{\expandafter\@gobble\string\#}
\edef\ampsym{\expandafter\@gobble\string\&}
\edef\dollarsym{\expandafter\@gobble\string\$}
\edef\underscoresym{\expandafter\@gobble\string\_}
\edef\circumsym{\expandafter\@gobble\string\^}
\edef\tildesym{\expandafter\@gobble\string\~}
\edef\leftbracesym{\expandafter\@gobble\string\{}
\edef\rightbracesym{\expandafter\@gobble\string\}}
\edef\dbbackslashsym{\string\\}
\edef\backslashsym{\expandafter\@gobble\dbbackslashsym}

% protected makeindex special characters
\newcommand*{\pleftbracesym}{"\leftbracesym}
\newcommand*{\prightbracesym}{"\rightbracesym}

\DeclareRobustCommand*{\quotecs}{\cmdname{"}}
\DeclareRobustCommand*{\umlautaccent}[1]{\"{#1}}

\DeclareRobustCommand*{\doublebar}[1]{\ensuremath{\|}}

\newcommand{\exclamsym}{!}
\newcommand{\questionsym}{?}
\newcommand{\atsym}{@}
\newcommand{\xatsym}{"@}
\newcommand{\vbarsym}{|}
\newcommand{\doublequotesym}{"}
\newcommand{\slashsym}{/}
\newcommand{\emDashcs}{-{}-{}-}
\newcommand{\enDashcs}{-{}-}
\newcommand{\dashcs}{-}

\newcommand{\quoteleftcs}{`}
\newcommand{\quotedblleftcs}{`{}`}
\newcommand{\quoterightcs}{'}
\newcommand{\quotedblrightcs}{'{}'}
\newcommand{\questiondowncs}{?{}`}
\newcommand{\exclamdowncs}{!{}`}
\newcommand{\opensqsym}{[}
\newcommand{\closesqsym}{]}
\newcommand{\openparensym}{(}
\newcommand{\closeparensym}{)}
\newcommand{\commasym}{,}
\newcommand{\colonsym}{:}
\newcommand{\semicolonsym}{;}
\newcommand{\lesssym}{<}
\newcommand{\greatersym}{>}
\newcommand{\periodsym}{.}
\newcommand{\equalsym}{=}

\newcommand{\dash}{\,---\,}

\newcommand{\doublequestionmark}{%
  ??\index{"?"? (undefined reference)}%
}

\DeclareRobustCommand{\symbolcmddef}[1]{%
 \cmdname{#1}%
 \indexsymbolcmddef{#1}%
}

\DeclareRobustCommand{\indexsymbolcmddef}[1]{%
 \index{\backslashsym#1@\protect\cmdname{#1}|indexdef}%
}

\DeclareRobustCommand{\indexspsymbolcmddef}[1]{%
{\edef\sortkey{\expandafter\string\csname#1\endcsname}%
\index{\sortkey @\protect\cmdname{"#1}|indexdef}}}

\DeclareRobustCommand{\indexspsymbolcmd}[1]{%
{\edef\sortkey{\expandafter\string\csname#1\endcsname}%
\index{\sortkey @\protect\cmdname{"#1}|hyperpage}}}

\DeclareRobustCommand{\symbolcmd}[1]{%
 \cmdname{#1}%
 \indexsymbolcmd{#1}%
}

\DeclareRobustCommand{\indexsymbolcmd}[1]{%
  \index{\backslashsym#1@\protect\cmdname{#1}|hyperpage}%
}

% index a command definition, but don't display it in the text
\newcommand{\indexComdef}[1]{\index{#1@\protect\cmdname{#1}|indexdef}}

% index a command name, but don't display it in the text
\newcommand{\indexCom}[1]{\index{#1@\protect\cmdname{#1}|hyperpage}}

\DeclareRobustCommand{\indexspsymbol}[1]{%
{\edef\sortkey{\expandafter\string\csname#1\endcsname}%
\index{\sortkey @\protect\texttt{"#1}|hyperpage}}}

\newcommand*{\keywordfmt}[1]{\emph{#1}}

\newcommand*{\keyword}[2][\keywordtext]{%
 \def\keywordtext{#2}%
 \keywordfmt{#1}\index{#2|hyperpage}%
}

\renewcommand{\glossarysection}[2][\@gls@title]{%
  \def\@gls@title{#2}%
  \protected@edef\do@def@glo@section{\noexpand\chapter[#1]{#2}}%
  \do@def@glo@section
}

\newcommand{\enter}{\scalebox{0.8}{\input{pictures/entersymbol}}}

\DeclareRobustCommand{\cmdname}[1]{\texttt{\char`\\#1}}
\DeclareRobustCommand{\parg}[1]{%
  \discretionary{}{}{}\texttt(#1\texttt)}
\DeclareRobustCommand{\meta}[1]{\textnormal{\ensuremath{\langle}\emph{#1}\ensuremath{\rangle}}}

\DeclareRobustCommand{\marg}{%
  \@ifstar\@smeta\@meta
}

\DeclareRobustCommand{\@meta}[1]{%
  \discretionary{}{}{}\glsni{leftbracechar}#1\glsni{rightbracechar}%
}

\DeclareRobustCommand{\@smeta}[1]{%
  \discretionary{}{}{}\texttt{\leftbracesym}#1\texttt{\rightbracesym}%
}

\DeclareRobustCommand{\oarg}{%
  \@ifstar\@soarg\@oarg
}

\DeclareRobustCommand{\@oarg}[1]{%
  \discretionary{}{}{}\glsni{opt.opensq}#1\glsni{opt.closesq}}

\DeclareRobustCommand{\@soarg}[1]{%
  \discretionary{}{}{}\texttt[#1\texttt]}

\DeclareRobustCommand{\comma}{,\discretionary{}{}{}}

\DeclareRobustCommand{\envname}[1]{\textsf{#1}}

\DeclareRobustCommand{\cls}[1]{\textsf{#1}}
\DeclareRobustCommand{\sty}[1]{\textsf{#1}}
\newcommand{\counter}[1]{\textsf{#1}}

\newcommand{\appname}[1]{\texttt{#1}}
\newcommand{\iappname}[2][\appnamesort]{%
{\def\appnamesort{#2}%
\appname{#2}%
\index{#1@\appname{#2}|hyperpage}}}

\newcommand{\texdistro}[1]{\texttt{#1}}
\newcommand{\itexdistro}[2][\distrosort]{%
{%
  \def\distrosort{#2}%
  \texdistro{#2}%
  \index{#1@\texdistro{#2}|hyperpage}%
  \index{TeX Distributions@\protect\TeX\ Distributions!#1@\protect\texdistro{#2}|hyperpage}%
}%
}

\newcommand{\perldistro}[1]{\texttt{#1}}

\newcommand{\menuto}{\discretionary{}{}{}$\rightarrow$\discretionary{}{}{}}

\newcommand{\startmenu}[1]{\begin{center}\sffamily Start$\rightarrow$ Programs $\rightarrow$ #1\end{center}}

\newcommand{\menu}[1]{\textsf{#1}}

\DeclareRobustCommand{\BiBTeX}{\textsc{Bib}\TeX}
\newcommand{\PDFLaTeX}{PDF\LaTeX}

\newcommand{\dq}[1]{``#1''}
\newcommand{\sq}[1]{`#1'}

\newcommand{\footnotesymbols}{%
\@fnsymbol1 \@fnsymbol2 \@fnsymbol3 \@fnsymbol4 \@fnsymbol5
\@fnsymbol6 \@fnsymbol7 \@fnsymbol8 \@fnsymbol9}

\newlength\imgHeight
\newlength\maxImageHeight

\setlength{\maxImageHeight}{0.8\textheight}

\newlength\imgWidth
\newlength\maxImageWidth
\setlength{\maxImageWidth}{\textwidth}

\define@key{Gin}{alt}{}

\newcommand{\incGraphics}[2][]{%
  \setlength{\maxImageWidth}{\linewidth}%
  \settoheight{\imgHeight}{\includegraphics[#1]{#2}}%
  \ifdim\imgHeight>\maxImageHeight
    \settowidth{\imgWidth}{\includegraphics[#1,height=\maxImageHeight]{#2}}%
    \ifdim\imgWidth>\maxImageWidth
      \includegraphics[#1,width=\maxImageWidth]{#2}%
    \else
      \includegraphics[#1,height=\maxImageHeight]{#2}%
    \fi
  \else
    \settowidth{\imgWidth}{\includegraphics[#1]{#2}}%
    \ifdim\imgWidth>\maxImageWidth
      \includegraphics[#1,width=\maxImageWidth]{#2}%
    \else
     \includegraphics[#1]{#2}%
    \fi
  \fi
}

\newsavebox{\boxcontents}

\newcommand*{\includeteximage}[2][]{%
  \setlength{\maxImageWidth}{\linewidth}%
  \def\Gin@req@sizes{%
    \Gin@req@height\Gin@nat@height
    \Gin@req@width\Gin@nat@width}%
  \begingroup
    \@tempswafalse
    \let\input@path\Ginput@path
    \toks@{\InputIfFileExists{#2}{}{\@warning{File `#1' not found}}}%
    \setkeys{Gin}{#1}%
    \Gin@esetsize
    \sbox\boxcontents{\the\toks@}%
    \settoheight{\imgHeight}{\usebox\boxcontents}%
    \settodepth{\@ff@tmp@y}{\usebox\boxcontents}%
    \addtolength{\imgHeight}{\@ff@tmp@y}%
    \ifdim\imgHeight>\maxImageHeight
      \ifdim\maxImageHeight<\maxImageWidth\relax
        \resizebox{!}{\maxImageHeight}{\usebox\boxcontents}%
      \else
        \settowidth{\imgWidth}{\usebox\boxcontents}%
        \ifdim\imgWidth>\maxImageWidth
          \resizebox{\maxImageWidth}{!}{\usebox\boxcontents}%
        \else
          \resizebox{!}{\maxImageHeight}{\usebox\boxcontents}%
        \fi
      \fi
    \else
      \settowidth{\imgWidth}{\usebox\boxcontents}%
      \ifdim\imgWidth>\maxImageWidth
       \resizebox{\maxImageWidth}{!}{\usebox\boxcontents}%
      \else
        \usebox\boxcontents
      \fi
    \fi
  \endgroup
}

\newcommand*{\incPgfOrGraphics}[2][]{%
  \IfFileExists{#2}{\includeteximage[#1]{#2}}{\incGraphics[#1]{#2}}%
}

\newcommand{\indexdef}[1]{\textit{\hyperpage{#1}}}

% footnotes for minipage examples
\newcommand{\mpexfootnote}[1]{%
\renewcommand{\thempfootnote}{\textcolor{black}{\alph{mpfootnote}}}%
\footnote{#1}}

\newcommand{\labelledbox}[2]{\begin{tabular}{r}\doublebox{#1}\\\scriptsize\textsf{#2}\end{tabular}}
\newcommand{\labelled}[2]{\begin{tabular}{r}#1\\\scriptsize\textsf{#2}\end{tabular}}

\ifnum\novicemedia=\NOVICEbook
  \definecolor{inputcol}{gray}{0.6}
  \definecolor{outputcol}{gray}{0.4}
\else
  \definecolor{inputcol}{cmyk}{1,0,0,0}
  \definecolor{outputcol}{rgb}{0.6,0,1}
\fi

\newcommand{\inputlabelformat}[1]{\textcolor{inputcol}{\scriptsize\sffamily #1}}
\newcommand{\outputlabelformat}[1]{\textcolor{outputcol}{\scriptsize\sffamily #1}}

\newcommand{\toparrow}{\ensuremath{\overline\uparrow}}
\newcommand{\botarrow}{\ensuremath{\underline\downarrow}}

\newlength\minremainspace

\ifnum\novicemedia=\NOVICEbook

  \newcommand*{\marginblocklabel}[2]{%
    \ifthispageodd
    {%
      \makebox[\marginparwidth][l]{#1 #2}%
    }%
    {%
      \makebox[\marginparwidth][r]{#2 #1}%
    }%
  }

 \newcommand*{\marginlabel}[1]{%
    \ifthispageodd
    {%
      \makebox[\marginparwidth][l]{#1}%
    }%
    {%
      \makebox[\marginparwidth][r]{#1}%
    }%
 }

 \setlength\minremainspace{50pt}

\else
 \newcommand*{\marginblocklabel}[2]{#1 #2}
 \newcommand*{\marginlabel}[1]{#1}
 \setlength\minremainspace{42pt}
\fi

% Changing the colour in the margin occasionally causes problems
% so store the coloured marginal text in save boxes.

\newsavebox\Input
\newsavebox\Output
\newsavebox\InputT
\newsavebox\OutputT
\newsavebox\CodeT
\newsavebox\InputB
\newsavebox\OutputB
\newsavebox\CodeB
\newsavebox\Code

\sbox\Input{\inputlabelformat{Input}}
\sbox\Output{\outputlabelformat{Output}}
\sbox\Code{\inputlabelformat{Code}}

\sbox\InputT{\inputlabelformat{%
   \toparrow
}}

\sbox\OutputT{\outputlabelformat{%
  \toparrow
 }}

\sbox\CodeT{\inputlabelformat{%
  \toparrow
}}

\sbox\InputB{\inputlabelformat{%
  \botarrow
}}

\sbox\OutputB{\outputlabelformat{%
  \botarrow
}}

\sbox\CodeB{\inputlabelformat{%
  \botarrow
}}

\newcommand{\inputruleB}{%
 \vspace*{-5pt}\par\noindent
 \textcolor{inputcol}{\rule{0.4pt}{5pt}\leaders\hrule\hfill\rule{0.4pt}{5pt}}}
\newcommand{\outputruleB}{\par\noindent
 \textcolor{outputcol}{\rule{0.4pt}{5pt}\leaders\hrule\hfill\rule{0.4pt}{5pt}}}
\newcommand{\coderuleB}{\par\noindent
 \textcolor{inputcol}{\rule{0.4pt}{5pt}\leaders\hrule\hfill\rule{0.4pt}{5pt}}}
\newcommand{\inputruleT}{\par\noindent
 \textcolor{inputcol}{\rule[-5pt]{0.4pt}{5pt}\leaders\hrule\hfill\rule[-5pt]{0.4pt}{5pt}}}
\newcommand{\outputruleT}{\par\noindent
 \textcolor{outputcol}{\rule[-5pt]{0.4pt}{5pt}\leaders\hrule\hfill\rule[-5pt]{0.4pt}{5pt}}}
\newcommand{\coderuleT}{\par\noindent
 \textcolor{inputcol}{\rule[-5pt]{0.4pt}{5pt}\leaders\hrule\hfill\rule[-5pt]{0.4pt}{5pt}}}

\newcommand*{\xminisec}[1]{%
 \@check@remainingspace[3.5\baselineskip]%
 \minisec{#1}%
}

\newcommand*{\@check@remainingspace}[1][\minremainspace]{%
  \par
  \mbox{}%
  \@tempdimc\pagegoal
  \advance\@tempdimc-\pagetotal
  \penalty -100\noindent
  \setlength{\@tempdimb}{#1}%
  \ifdim\@tempdimb>\@tempdimc\relax
   \pagebreak
  \else
   \vskip-\baselineskip
  \fi
}

% define commands governing how input code and 
% corresponding output should be displayed

\newenvironment{bcode}%
 {%
   \@check@remainingspace
   \par\vspace{10pt plus 2pt minus 1pt}%
   \coderuleT
   \marginpar{\raisebox{-8pt}{%
     \marginblocklabel{\usebox\CodeT}{\usebox\Code}}}%
   \ttfamily\frenchspacing\obeyspaces\par\raggedright\noindent\ignorespaces
 }%
 {%
   \par\coderuleB
   \marginpar{\raisebox{4pt}{%
     \marginblocklabel{\usebox\CodeB}{\usebox\Code}}}%
   \par\vspace{10pt plus 2pt minus 1pt}%
   \noindent\ignorespacesafterend
 }

%Single line of input
\newenvironment{codeS}[1][0.9\linewidth]%
{%
  \par
  \vspace{5pt plus 4pt minus 2pt}%
  \noindent
  \marginpar{\mbox{\marginlabel{\usebox\Input}}}%
  \begin{minipage}{#1}\ttfamily\raggedright\frenchspacing
}%
{%
 \par\end{minipage}%
 \par\vspace{5pt plus 4pt minus 2pt}%
 \noindent\ignorespacesafterend
}

%Multiple lines of input
\newenvironment{code}[1][0.9\linewidth]%
{%
  \@check@remainingspace
  \vskip5pt plus 4pt minus 2pt\relax
  \inputruleT
  \marginpar{\raisebox{-8pt}{%
    \marginblocklabel{\usebox\InputT}{\usebox\Input}}}%
  \nopagebreak
  \begin{ttfamily}\nopagebreak\par\noindent\raggedright\frenchspacing
}%
{%
  \par\end{ttfamily}\nopagebreak
  \inputruleB
  \marginpar{\raisebox{4pt}{%
    \marginblocklabel{\usebox\InputB}{\usebox\Input}}}%
  \par\vspace{5pt plus 4pt minus 2pt}%
  \noindent\ignorespacesafterend
}

\newcommand*{\ltxresultwidth}{0.9\linewidth}

%Single line of output
\newenvironment{ltxresultS}[1][]%
{%
  \vskip5pt plus 4pt minus 2pt\relax
  \par\noindent
  \marginpar{\mbox{\marginlabel{\usebox\Output}}}%
  \begin{minipage}{\ltxresultwidth}%
}%
{%
  \end{minipage}%
  \vskip5pt plus 4pt minus 2pt\relax
  \noindent\ignorespacesafterend
}

%Multiple lines of output
\newenvironment{ltxresultM}[1][]%
{%
  \@check@remainingspace
  \vskip5pt plus 4pt minus 2pt\relax
  \outputruleT
  \nopagebreak
  \marginpar{\raisebox{-8pt}{%
    \marginblocklabel{\usebox\OutputT}{\usebox\Output}}}%
  \nopagebreak
  \par\nopagebreak
}%
{%
  \nopagebreak\outputruleB
  \marginpar{\raisebox{4pt}{%
    \marginblocklabel{\usebox\OutputB}{\usebox\Output}}}%
  \par\vspace{5pt plus 4pt minus 2pt}%
  \noindent\ignorespacesafterend
}

\newenvironment{htmlresult}[1]{\ignorespaces}{\ignorespacesafterend}

\newenvironment{definition}[1][\linewidth]%
{%
  \par\vspace{10pt plus 1pt minus 1pt}%
  \noindent
  \marginpar{\mbox{\marginlabel{\scriptsize\sffamily Definition}}}%
  \begin{lrbox}{\boxcontents}
  \begin{minipage}{#1}\ttfamily\raggedright\frenchspacing
}%
{%
  \par
  \end{minipage}\end{lrbox}%
  \usebox{\boxcontents}%
  \par\vspace{10pt plus 1pt minus 1pt}%
  \noindent\ignorespacesafterend
}

\DeclareRobustCommand{\csentryargitem}[1]{\item{\meta{#1}}}

\DeclareRobustCommand{\BeginArgList}{\begin{description}}
\DeclareRobustCommand{\EndArgList}{\end{description}}

\protected@write\@auxout{}{%
\string\html{\string\gdef\string\newlabelxx
\string#1\string#2\string#3\string#4\string#5\string#6{%
\string\oldnewlabel{\string#1}{{\string#2}{\string#3}}%
\string\sectionlabel{\string#1}{\string#4}}}}

\ifnum\novicemedia=\NOVICEbook
  \DeclareRobustCommand*{\appendixref}[2][]{Appendix~\ref{#2}
    (\protect\nameref{#2})}
  \DeclareRobustCommand*{\chapterref}[2][]{Chapter~\ref{#2}
    (\protect\nameref{#2})}
  \DeclareRobustCommand*{\sectionref}[2][]{Section~\ref*{#2}}
\else
  \DeclareRobustCommand*{\appendixref}[2][]{\htmlref{Appendix~\ref*{#2}}{#2}
    (\protect\nameref*{#2})}
  \DeclareRobustCommand*{\chapterref}[2][]{\htmlref{Chapter~\ref*{#2}}{#2}
    (\protect\nameref*{#2})}
  \DeclareRobustCommand*{\sectionref}[2][]{\htmlref{Section~\ref*{#2}}{#2}}
\fi

\renewcommand*{\thefootnote}{\thechapter.\arabic{footnote}}

\let\@mkboth\markboth

\newcommand*{\warning}{%
 \marginpar
 {%
  \marginlabel
  {%
    {%
      \LARGE
      \fontencoding {U}%
      \fontfamily{futs}\selectfont
      \char 66\relax
    }%
  }%
 }%
}
\newcommand*{\wrong}{\marginpar{\marginlabel{\LARGE\color{red}\ding{56}}}}
\newcommand*{\correct}{\marginpar{\marginlabel{\LARGE\color{green}\ding{52}}}}

% Computer Modern maths fonts
\DeclareMathAlphabet{\cmmathrm}{T1}{cmr}{m}{n}
\DeclareMathAlphabet{\cmmathit}{T1}{cmr}{m}{it}
\DeclareMathAlphabet{\cmmathtt}{T1}{cmtt}{m}{n}
\DeclareMathAlphabet{\cmmathsf}{T1}{cmss}{m}{n}
\DeclareMathAlphabet{\cmmathbf}{T1}{cmr}{bx}{n}
\DeclareSymbolFontAlphabet{\cmmathcal}{symbols}
\DeclareSymbolFontAlphabet{\cmmathnormal}{letters}

\newcommand*{\cmr}[1]{{\fontfamily{cmr}\selectfont #1}}

% keywords stuff

\newglossarystyle{keywords}%
{%
  \glossarystyle{inline}%
  \renewcommand{\glossaryheader}{%
    \keywordpreamble
    \renewcommand{\glssubentryitem}[1]{}%
  }%
  \renewcommand{\glsinlinenameformat}[2]{%
    \setnode{##1}%
    \section[\glstoc{##1}]{\glstarget{##1}{##2}}%
    \label{sec:##1}%
    \index{\glsindex{##1}|indexdef}%
    \glsextra{##1}%
    \glsreset{##1}%
  }%
  \renewcommand{\glsinlinesubnameformat}[2]{%
    \setnode{##1}%
    \subsection[\glstoc{##1}]{\glstarget{##1}{##2}}%
    \label{sec:##1}%
    \glsreset{##1}%
  }%
  \renewcommand*{\glsinlineseparator}{}%
  \renewcommand*{\glsinlinesubseparator}{}%
  \renewcommand*{\glsinlineparentchildseparator}{}%
  \renewcommand*{\glspostinline}{}%
  \renewcommand{\glsinlinedescformat}[3]{##1}%
  \renewcommand{\glsinlinesubdescformat}[3]{##1}%
}

\newcommand{\keywordpreamble}{\input{term-defs/introduction}}

\newcommand*{\glsd}[2][]{%
  \ifglsused{#2}%
  {%
    \glsuseri[#1]{#2}\indexglsindex{#2}%
  }%
  {%
    \emph{\glsuseri[hyper=false,#1]{#2}}%
    \glsunset{#2}%
  }%
}

\newcommand*{\glsing}[2][]{%
  \ifglsused{#2}%
  {%
    \glsuserii[#1]{#2}%
    \indexglsindex{#2}%
  }%
  {%
    \emph{\glsuserii[hyper=false,#1]{#2}}%
    \glsunset{#2}%
  }%
}

\newcommand*{\glstoc}[1]{\glsentryuseriii{#1}}
\newcommand*{\glsterm}[2][]{\glsuseriv[#1]{#2}}
\newcommand*{\glsindex}[1]{\glsentryuserv{#1}}
\newcommand*{\glsextra}[1]{\glsdoifexists{#1}{\glsentryuservi{#1}}}

\def\glsdkey{user1}
\def\glsingkey{user2}
\def\glstockey{user3}
\def\glstermkey{user4}
\def\glsindexkey{user5}
\def\glsextrakey{user6}

\newcounter{keyword}

% \newkeyword[options][plural]{label}{name}

\newcommand*{\newkeyword}[1][]{%
  \def\keyword@options{#1}%
  \@newkeyword
}

\newcommand*{\@newkeyword}[3][\keyword@plural]{%
  \def\keyword@plural{#3s}%
  \protected@edef\@do@newkeyword{\noexpand\@@newkeyword
    {\keyword@options}%
    {#1}%
    {#2}%
    {#3}%
  }%
  \@do@newkeyword
}

\DeclareRobustCommand*{\indexglsindex}[1]{%
  \index{\glsindex{#1}|hyperpage}%
}

\newcommand{\four@digits}[1]{%
 \ifnum #1<1000 0\fi\ifnum #1<100 0\fi\ifnum #1<10 0\fi \number #1%
}

\newcommand*{\@@newkeyword}[4]{%
 \stepcounter{keyword}%
  \newglossaryentry{#3}{%
    sort={\four@digits{\c@keyword}},
    type=keywords,
    name={\protect\capitalisewords{#4}},
    \glsindexkey={#4},
    \glsextrakey={},
    first={\protect\keywordfmt{#4}},
    firstplural={\protect\keywordfmt{#2}},
    text={#4\indexglsindex{#3}},
    plural={#2\indexglsindex{#3}},
    \glsdkey={#4ed},
    \glsingkey={#4ing},
    description={\protect\input{term-defs/#3}},
    \glstockey={#4},
    \glstermkey={#4},
    #1
  }%
}

\newglossary[klg]{keywords}{kls}{klo}{Some Definitions}

\newcommand{\printkeywords}{%
  \printglossary[type=keywords,style=keywords]
}

% \newacr{label}{short}{long}{url}

\newcommand*{\newacr}[4]{%
  \newglossaryentry{#1}{%
    type=acronym,
    sort={#2},
    name={#2\gloindex{#2}},
    text={#2\protect\index{#2|hyperpage}\latex{~\cite{#1}}},
    first={#3\protect\footnote{\protect\url{#4}}\protect\index{#2|hyperpage}
(#2)},
    description={\makefirstuc{#3}. \protect\url{#4}}
  }%
}

% \newacrx{label}{short}{long}

\newcommand*{\newacrx}[3]{%
  \newglossaryentry{#1}{%
    type=acronym,
    sort={#2},
    name={#2\gloindex{#2}},
    text={#2\protect\index{#2|hyperpage}\latex{~\cite{#1}}},
    first={#3\protect\index{#2|hyperpage} (#2)},
    description={\makefirstuc{#3}.}
  }%
}

% \newacrnocite{label}{short}{long}

\newcommand*{\newacrnocite}[3]{%
  \newglossaryentry{#1}{%
    type=acronym,
    sort={#2},
    name={#2\gloindex{#2}},
    text={#2\protect\index{#2|hyperpage}},
    first={#3\protect\index{#2|hyperpage} (#2)},
    description={\makefirstuc{#3}.}
  }%
}

\newglossarystyle{acronyms}{%
  \glossarystyle{list}%
  \renewcommand*{\glsgroupskip}{}%
  \renewenvironment{theglossary}%
    {\begin{description}}
    {%
      \end{description}%
      \protected@write\@auxout{}{\string\@flow@endacr{\thepage}}%
    }%
}

% Code optimized for book, but some tweaking may be required
\ifnum\novicemedia=\NOVICEbook
  \DeclareRobustCommand{\bookpagebreak}{\par\pagebreak}
  \DeclareRobustCommand{\booklinebreak}{\linebreak}
\else
  \newcommand*{\bookpagebreak}{}
  \newcommand*{\booklinebreak}{}
\fi
\ifnum\novicemedia=\NOVICEreport
  \DeclareRobustCommand{\reportpagebreak}{\par\pagebreak}
  \DeclareRobustCommand*{\reportlinebreak}{\linebreak}
\else
  \newcommand*{\reportpagebreak}{}
  \newcommand*{\reportlinebreak}{}
\fi
\ifnum\novicemedia=\NOVICEscreen
  \newcommand*{\screenpagebreak}{\par\pagebreak}
  \newcommand*{\screenlinebreak}{\linebreak}
\else
  \newcommand*{\screenpagebreak}{}
  \newcommand*{\screenlinebreak}{}
\fi

% index and display application name with hyperlink to application's
% home page
 \ifnum\novicemedia=\NOVICEbook

  \newcommand{\iappnamelink}[3][]{%
   \appname{#2}#1\footnote{\url{#3}}%
   \index{#2@\appname{#2}}}

  \newcommand{\iperldistro}[3][]{%
    \perldistro{#2}#1\footnote{\url{#3}}%
    \index{#2@\protect\perldistro{#2}|hyperpage}%
  }

  \newcommand{\itexdistrolink}[3][]{%
  {%
    \texdistro{#2}#3\footnote{\url{#3}}%
    \index{#2@\protect\texdistro{#2}|hyperpage}%
    \index{TeX Distributions@\protect\TeX\ Distributions!#2@\protect\texdistro{#2}|hyperpage}%
  }%
 }

 \else
  \newcommand{\iappnamelink}[3][]{%
   \htmladdnormallink{\appname{#2}}{#3}#1%
   \index{#2@\appname{#2}|hyperpage}}

  \newcommand{\iperldistro}[3][]{%
   \htmladdnormallink{\perldistro{#2}}{#3}#1%
   \index{#2@\protect\perldistro{#2}|hyperpage}}

  \newcommand{\itexdistrolink}[3][]{%
  {%
    \htmladdnormallink{\texdistro{#2}}{#3}#1%
    \index{#2@\texdistro{#2}|hyperpage}%
    \index{TeX Distributions@\protect\TeX\ Distributions!#2@\protect\texdistro{#2}|hyperpage}%
  }%
 }

 \fi

\newcommand{\suppmaterial}[2][]{#2#1\footnote{\url{\baseurl/latex/novices/html/supplemental.html}}}
\newcommand{\suppref}[3][]{#2#1\footnote{\url{\baseurl/latex/novices/html/#3.html}}}

\newcommand{\objectref}[2]{\htmlref{#1~\ref*{#2}}{#2}}

\newcommand{\xfigureref}[1]{\figurename~\ref{#1}}

\newcommand{\xpageref}[2]{\vpageref{#2}}

\newcommand{\figuretop}[1]{\hypertarget{#1top}{}}
\newcommand{\makeimg}[2]{{#2}}
\let\oelig\oe

\newlength\rightmargini
\newlength\rightmarginii
\newlength\rightmarginiii
\newlength\rightmarginiv

\def\itemize{%
  \ifnum\@itemdepth>\thr@@
    \@toodeep
  \else
    \setlength{\rightmargini}{5em}%
    \advance\@itemdepth\@ne
    \edef\@itemitem{labelitem\romannumeral\the\@itemdepth}%
    \expandafter\list\csname\@itemitem\endcsname
     {\def\makelabel##1{\hss\llap{##1}}}%
  \fi
}

\def\enumerate{%
  \ifnum\@enumdepth>\thr@@
    \@toodeep
  \else
    \advance \@enumdepth\@ne
    \edef\@enumctr{enum\romannumeral\the\@enumdepth}%
    \expandafter\list\csname label\@enumctr\endcsname
      {% 
         \usecounter\@enumctr
         \setlength{\rightmargin}%
            {\csname rightmargin\romannumeral\the\@enumdepth\endcsname}%
         \def\makelabel##1{\hss\llap{##1}}%
      }%
  \fi
}

\newcommand{\fwitem}[1]{\item[{\makebox[\labelwidth][l]{#1}}]}

\newenvironment{fwlist}[1]{%
\begin{list}{}{%
\settowidth{\labelwidth}{#1}\setlength{\itemindent}{0pt}
\setlength{\leftmargin}{\labelwidth}
\addtolength{\leftmargin}{\labelsep}}
}{
\end{list}}

\newcommand{\exerciseref}[1]{\objectref{Exercise}{#1}%
  \ifnum\novicemedia=\NOVICEbook\relax
   \space (page~\pageref{#1})%
  \fi
}

\newcommand{\modification}[1]{\textbf{\ttfamily #1}}

\newcommand{\comptexttex}{\texttt{comp.text.tex}~\cite{comptexttex}}
\newcommand{\texhax}{\texttt{texhax}~\cite{texhax}}

 \ifcase\novicemedia
 % report
   \newcommand{\latexthesis}{%
     \htmladdnormallink{\emph{Using \LaTeX\ to Write a PhD Thesis}}%
       {\baseurl/latex/thesis/index.html}~\cite{thesis}%
   }
 \or
 % screen

   \newcommand{\latexthesis}{%
     \htmladdnormallink{\emph{Using \LaTeX\ to Write a PhD Thesis}}%
       {\baseurl/latex/thesis/index.html}~\cite{thesis}%
   }%
 \or
 % book

   \newcommand{\latexthesis}{%
     \emph{Using \LaTeX\ to Write a PhD Thesis}~\cite{thesis}%
   }
 \fi

\newcommand{\latexpdfdoc}{%
 \begin{em}%
  \htmladdnormallink{Creating}{\baseurl/latex/pdfdoc/}
  \htmladdnormallink{a}{\baseurl/latex/pdfdoc/}
  \htmladdnormallink{PDF}{\baseurl/latex/pdfdoc/}
  \htmladdnormallink{Document}{\baseurl/latex/pdfdoc/}
  \htmladdnormallink{using}{\baseurl/latex/pdfdoc/}
  \htmladdnormallink{PDFLaTeX}{\baseurl/latex/pdfdoc/}%
 \end{em}%
 \ifscreen\else \footnote{\url{\baseurl/latex/pdfdoc/}}\fi
}

\newenvironment{resultS*}[1][]%
{\begin{ltxresultS}}%
{\end{ltxresultS}\ignorespacesafterend}

\newenvironment{resultS}[1][]%
{\begin{ltxresultS}}%
{\end{ltxresultS}\ignorespacesafterend}

\newenvironment{result}[1][]%
{\begin{ltxresultM}}%
{\end{ltxresultM}\ignorespacesafterend}

\newcounter{object}

\newenvironment{inlinedef}{\begin{ttfamily}}{\end{ttfamily}}

% \casemedia{A4}{screen}{book}{html}
\newcommand{\casemedia}[4]{%
  \ifcase\novicemedia#1\or#2\or#3\fi%
}

\newcommand{\download}[1]{%
\casemedia
 {\htmladdnormallink{download}{\exerciseurl/#1.tex}}% report
 {\htmladdnormallink{download}{\exerciseurl/#1.tex}}% screen
 {download\footnote{\url{\exerciseurl/#1.tex}}}% book
 {\htmladdnormallink{download}{\exerciseurl/#1.tex}}% html
}

\newcommand{\downloadorview}[1]{%
\casemedia
 {\htmladdnormallink{download}{\exerciseurl/#1.tex} or
\htmladdnormallink{view}{\exerciseurl/#1.html}}% report
 {\htmladdnormallink{download}{\exerciseurl/#1.tex} or
\htmladdnormallink{view}{\exerciseurl/#1.html}}% screen
 {download\footnote{\url{\exerciseurl/#1.tex}}}% book
 {\htmladdnormallink{download}{\exerciseurl/#1.tex} or
\htmladdnormallink{view}{\exerciseurl/#1.html}}% html
}

\newcommand*{\ifbookorother}[2]{%
  \ifnum\novicemedia=\NOVICEbook\relax
   #1%
  \else
   #2%
  \fi
}

\newcommand*{\ifscreenorother}[2]{%
  \ifnum\novicemedia=\NOVICEscreen\relax
   #1%
  \else
   \ifpdf
    #2%
   \else
    #1%
   \fi
  \fi
}

\newcommand*{\doifreport}[1]{%
  \ifnum\novicemedia=\NOVICEreport\relax
   #1%
  \fi
}

\newcommand*{\doifbook}[1]{%
  \ifnum\novicemedia=\NOVICEbook\relax
   #1%
  \fi
}

\newcommand*{\doifnotbook}[1]{%
  \ifnum\novicemedia=\NOVICEbook
  \else
   #1%
  \fi
}

\newcommand*{\breakenumi}{%
  \xdef\currentenumi{\arabic{enumi}}
  \end{enumerate}
  \begin{enumerate}
  \setcounter{enumi}{\currentenumi}
}

%\dosidewaysfigure{label}{image}{caption}
\newcommand*{\dosidewaysfigure}[3]{%
  \ifscreenorother
  {%
    \begin{figure}
      \figuretop{#1}
      \rotatebox{90}%
      {%
        \begin{minipage}[c][\textwidth]{0.75\textheight}%
         \centering
         #2% image
         \caption{#3}% caption
         \label{#1}%
        \end{minipage}
      }
    \end{figure}
  }%
  {%
    \begin{sidewaysfigure}
      \figuretop{#1}
      \centering
      #2% image
      \caption{#3}% caption
      \label{#1}%
    \end{sidewaysfigure}
  }%
}

\newcommand*{\degrees}[1]{\ensuremath{#1^{\circ}}}

\newcommand*{\idxcrossref}[2]{\see{#1}{#2}}

% display and index a counter:
\newcommand{\icounter}[1]{%
\counter{#1}\index{counters!#1@\counter{#1}|hyperpage}}

% display and index BibTeX:
\newcommand{\iBiBTeX}{\BiBTeX\index{BibTeX@\BiBTeX|hyperpage}}

% display and index PDFLaTeX:
\newcommand{\iPDFLaTeX}{\PDFLaTeX\index{PDFLaTeX@\PDFLaTeX|hyperpage}}

% display and index text:
\newcommand{\Index}[1]{#1\index{#1|hyperpage}}

% index monospaced text:
\newcommand{\indextt}[1]{\index{#1@\texttt{#1}|hyperpage}}

% display and index monospaced text:
\newcommand{\Indextt}[1]{\texttt{#1}\indextt{#1}}

% command definition: display and index
\newcommand*{\cmddef}[1]{\cmdname{#1}\indexComdef{#1}}

% index and display a command name
\newcommand*{\icmdname}[1]{\cmdname{#1}\indexCom{#1}}

% index an environment definition, but don't display it
\newcommand*{\indexEnvdef}[1]{\index{#1@\envname{#1} environment|indexdef}}

% index an environment, but don't display it
\newcommand*{\indexEnv}[1]{\index{#1@\envname{#1}
environment|hyperpage}}

% environment definition: display and index
\newcommand*{\envdef}[1]{\envname{#1}\indexEnvdef{#1}}

% index and display environment
\newcommand*{\ienvname}[1]{\envname{#1}\index{#1@\envname{#1} environment|hyperpage}}

\newtoks\argsummarytok

\newcommand*{\indexglo}[1]{\textbf{\hyperpage{#1}}}

\DeclareRobustCommand*{\gloindex}[1]{\index{#1|indexglo}}

\newcommand*{\glsi}[1]{%
 \glsdoifexists{#1}{\glshyperlink[\glsentrytext{#1}]{#1}}%
}

\newcommand*{\glsni}[1]{%
 \glsdoifexists{#1}{\glshyperlink[\glsentryuseriv{#1}]{#1}}%
}

\newcommand*{\glsnl}[1]{%
 \glsdoifexists{#1}{\glsentryuseriv{#1}}%
}

\DeclareRobustCommand*{\indexuservi}[1]{%
  \expandafter\index\expandafter{\glsentryuservi{#1}|hyperpage}%
}

\DeclareRobustCommand*{\indexdefuservi}[1]{%
  \expandafter\index\expandafter{\glsentryuservi{#1}|indexdef}%
}

% \defgcs[extra][label]{name}{arg syntax}{location of definition}{summary}{arg
% summary}
\newcommand{\defgcs}[1][]{%
 \def\@gcs@extra{#1}%
 \@defgcs
}

\newcommand{\@defgcs}[6][\gcslabel]{%
 \def\gcslabel{#2}%
 \argsummarytok{#6}%
 \expandafter\newglossaryentry\expandafter
   {#1}%
   {%
     sort={#2},
     name={\cmdname{#2}},
     first={\protect\cmdname{#2}\indexdefuservi{#1}},
     text={\protect\cmdname{#2}\indexuservi{#1}},
     user1={#3},
     user2={#4},
     description={#5},
     user3={\the\argsummarytok},
     user4={\protect\cmdname{#2}},
     user5={\@gcs@extra},
     user6={#2@\protect\cmdname{#2}}
   }%
}

% \defgchar[extra]{label}{char}{arg syntax}{location}{summary}{arg summary}
\newcommand{\defgchar}[7][]{%
  \newglossaryentry{#2}%
   {
     name={\texttt{#3}},
     sort={#3},
     first={\texttt{#3}%
      \indexdefuservi{#2}},
     text={\protect\texttt{#3}%
      \indexuservi{#2}},
     user1={#4},
     user2={#5},
     user3={#7},
     user4={\texttt{#3}},
     description={#6},
     user5={#1},
     user6={#3@\texttt{#3}}
   }%
}

% \defgchildchar{label}{parent}{char}{summary}
\newcommand{\defgchildchar}[4]{%
  \newglossaryentry{#1}%
  {%
     parent={#2},
     description={#4},
     first={\texttt{#3}%
      \indexuservi{#1}},
     text={\protect\texttt{#3}%
      \indexuservi{#1}},
     user4={\texttt{#3}},
     user6={#3@\texttt{#3}}
  }%
}

% \defgxchildchar{label}{parent}{char}{summary}{extra}
\newcommand{\defgxchildchar}[5]{%
  \newglossaryentry{#1}%
  {%
     parent={#2},
     description={#4},
     first={\texttt{#3}%
      \indexuservi{#1}%
      \protect\index{#5|hyperpage}%
     },
     text={\protect\texttt{#3}%
      \indexuservi{#1}%
      \protect\index{#5|hyperpage}%
     },
     user4={\texttt{#3}},
     user6={#3@\texttt{#3}}
  }%
}

% \defgactivechar[sort]{label}{cs}{arg syntax}{location}{summary}{arg summary}
\newcommand{\defgactivechar}[7][]{%
  \ifstrempty{#1}
  {%
    \def\@gcs@sort{#3}%
  }%
  {%
    \def\@gcs@sort{#1}%
  }%
  \protected@edef\@do@newglossaryentry{%
     \newglossaryentry{#2}%
     {%
        name={\noexpand\texttt{\noexpand\protect\noexpand#3}},
        sort={\@gcs@sort},
        first={\noexpand\texttt{#3}%
         \indexdefuservi{#2}},
        text={\noexpand\protect\noexpand\texttt{#3}%
         \indexuservi{#2}},
        user1={#4},
        user2={#5},
        user3={#7},
        user4={\noexpand\texttt{#3}},
        description={#6},
        user6={\expandonce\@gcs@sort @\texttt{\protect#3}}
      }%
   }%
   \@do@newglossaryentry
}

% \defgchildactivechar{label}{parent}{cs}{summary}
\newcommand{\defgchildactivechar}[4]{%
  \newglossaryentry{#1}%
  {%
     parent={#2},
     first={\texttt{#3}%
      \indexuservi{#1}},
     text={\protect\texttt{#3}%
      \indexuservi{#1}},
     user4={\texttt{#3}},
     description={#4},
     user6={#3@\texttt{#3}}
   }%
}

% \defgactivecharcs[sort]{label}{cs}{arg syntax}{location}{summary}{arg summary}
\newcommand{\defgactivecharcs}[7][]{%
  \ifstrempty{#1}
  {%
    \def\@gcs@sort{#3}%
  }%
  {%
    \def\@gcs@sort{#1}%
  }%
  \protected@edef\@do@newglossaryentry{%
     \newglossaryentry{#2}%
     {%
        name={\noexpand\cmdname{\noexpand\protect\noexpand#3}},
        sort={\backslashsym#1},
        first={\noexpand\cmdname{#3}%
         \indexdefuservi{#2}},
        text={\noexpand\protect\noexpand\cmdname{#3}%
         \indexuservi{#2}},
        user1={#4},
        user2={#5},
        user3={#7},
        user4={\noexpand\cmdname{#3}},
        description={#6},
        user6={\backslashsym#1@\protect\cmdname{\string#3}}
      }%
   }%
   \@do@newglossaryentry
}

% \defgidxactivechar[sort]{label}{cs}{arg syntax}{location}{summary}{arg summary}
\newcommand{\defgidxactivechar}[7][]{%
  \ifstrempty{#1}
  {%
    \def\@gcs@sort{#3}%
  }%
  {%
    \def\@gcs@sort{#1}%
  }%
  \toks@{#6}%
  \protected@edef\@do@newglossaryentry{%
    \newglossaryentry{#2}%
     {%
       name={\noexpand\texttt{\noexpand#3}},
       sort={"\expandonce\@gcs@sort},
       first={\noexpand\protect\noexpand\texttt{\noexpand#3}%
        \indexdefuservi{#2}},
       text={\noexpand\protect\noexpand\texttt{\noexpand#3}%
        \indexuservi{#2}},
       user1={#4},
       user2={#5},
       user3={#7},
       user4={\noexpand\texttt{\noexpand#3}},
       description={\the\toks@},
       user6={"\expandonce\@gcs@sort @\texttt{\string#3}}
   }%
  }%
  \@do@newglossaryentry
}

% \defgchildidxactivechar{label}{parent}{cs}{summary}
\newcommand{\defgchildidxactivechar}[4]{%
  \newglossaryentry{#1}%
  {%
     parent={#2},
     name={\noexpand\texttt{\noexpand#3}},
     first={\noexpand\texttt{\noexpand#3}%
      \indexuservi{#1}},
     text={\protect\texttt{\noexpand#3}%
      \indexuservi{#1}},
     user4={\texttt{#3}},
     description={#4},
     user6={"#3@\texttt{\string#3}}
   }%
}

% \defgidxactivecharcs{label}{charcs}{arg syntax}{location}{summary}{arg summary}
\newcommand{\defgidxactivecharcs}[6]{%
  \newglossaryentry{#1}%
   {%
     name={\cmdname{\protect#2}},
     sort={"\backslashsym"#2},
     first={\cmdname{#2}%
      \indexdefuservi{#1}},
     text={\protect\cmdname{#2}%
      \indexuservi{#1}},
     user1={#3},
     user2={#4},
     user3={#6},
     user4={\cmdname{#2}},
     description={#5},
     user6={"\backslashsym"#2@\protect\cmdname{\string#2}}
   }%
}

% \defgsymcs[label]{sym}{syntax}{location}{summary}{arg summary}
\newcommand{\defgsymcs}[6][\gcslabel]{%
 \def\gcslabel{#2}%
 \argsummarytok{#6}%
 \expandafter\newglossaryentry\expandafter
   {#1}%
   {%
     name={\cmdname{#2}},
     sort={\backslashsym#2},
     first={\protect\cmdname{#2}%
       \indexdefuservi{#1}},
     text={\protect\cmdname{#2}%
       \indexuservi{#1}},
     user1={#3},
     user2={#4},
     description={#5},
     user3={\the\argsummarytok},
     user4={\protect\cmdname{#2}},
     user5={},
     user6={\backslashsym#2@\protect\cmdname{#2}}
   }%
}

% \defgenv[label]{env name}{arg syntax}{location}{summary}{arg
% summary}
\newcommand{\defgenv}[6][\gcslabel]{%
 \def\gcslabel{env-#2}%
 \argsummarytok{#6}%
 \expandafter\newglossaryentry\expandafter
   {#1}%
   {%
     sort={#2},
     name={\protect\cmdname{begin}\protect\texttt{\protect\marg{#2}}},
     first={\protect\envname{#2}%
        \indexdefuservi{#1}},
     text={\protect\envname{#2}%
        \indexuservi{#1}},
     user1={#3},
     user2={#4},
     description={#5},
     user3={\the\argsummarytok},
     user4={\protect\envname{#2}},
     user5={},
     user6={#2@\envname{#2} environment}
   }%
}

\newlength\summaryskip
\newlength\summarypostskip
\newlength\summarypredescskip
\newlength\summarypostnameskip

\setlength{\summaryskip}{2ex plus .5ex minus .5ex}
\setlength{\summarypostskip}{.5ex plus .1ex minus .1ex}
\setlength{\summarypredescskip}{1ex plus .5ex minus .5ex}
\setlength{\summarypostnameskip}{1ex plus .25ex minus .25ex}

\newglossarystyle{summary}% Summary glossary style
{%
  \glossarystyle{inline}%
    \renewcommand{\glossaryheader}{\raggedright\enablefoothooks
      \ifnum\novicemedia=\NOVICEbook\relax
        \setleftfootmark{\dosummarytopmark}%
        \setrightfootmark{\dosummarybottommark}%
      \fi
    }%
    \renewcommand{\glsgroupheading}[1]{%
      \gls@inlinesep\def\gls@inlinesep{}%
      \noindent\makebox[\linewidth][c]%
      {%
        \ifnum\novicemedia=\NOVICEbook
          \textbf{\glsgetgrouptitle{##1}}%
        \else
          \protected@write\@auxout{}{\string\gls@header{##1}}%
          \hypertarget{gls:##1}{\textbf{\glsgetgrouptitle{##1}}}%
        \fi
      }%
      \\*[\summarypostskip]%
    }%
  \renewcommand{\glsinlinenameformat}[2]{%
    \def\glsentrylabel{##1}%
      \par\noindent
      \glstarget{##1}{##2\glsentryuseri{##1}}%
      \gloindex{\glsentryuservi{##1}}%
      \dosummarymark{\glsuseriv{##1}}%
      \ifcsempty{glo@##1@userii}%
      {%
        \\*[\summarypredescskip]%
      }%
      {%
        \\*[\summarypostnameskip]%
        \textbf{Defined in: }\glsentryuserii{##1}.%
        \par
        \vskip\summarypredescskip
      }%
  }%
  \renewcommand*{\glsinlinesubnameformat}[2]{\glstarget{##1}{\strut}}%
  \renewcommand{\glsinlinedescformat}[3]{##1\space[\S##3]\glsentryuserv{\glsentrylabel}}%
  \renewcommand{\glsinlineseparator}{\par\vskip\summaryskip\noindent}%
  \renewcommand{\glsinlinesubseparator}{;\space}%
  \renewcommand{\glsinlinesubdescformat}[3]{##1\space[\S##3]}%
  \renewcommand{\glsinlineparentchildseparator}{}%
  \renewcommand{\glsinlinepostchild}{.}%
  \renewcommand{\glspostinline}{%
%    \gappto\rightfootmark{\disablefoothooks}%
%    \gappto\leftfootmark{\disablefoothooks}%
    \protected@write\@auxout{}{\string\@flow@endsummary{\thepage}}%
  }%
}

\newcommand*{\nxipagestyle}[1]{\protect\ipagestyle{#1}}

\newcommand*{\nxisty}[1]{\protect\isty{#1}}
\newcommand*{\nxicls}[1]{\protect\icls{#1}}
\newcommand*{\nxiappname}[1]{\protect\iappname{#1}}

\newcommand*{\nxglsi}{\protect\glsi}
\newcommand*{\nxglsni}{\protect\glsni}

\newcommand*{\nxglslink}[2]{\protect\glslink{#1}{#2}}

\newcommand*{\nxGls}{\protect\Gls}
\newcommand*{\nxgls}{\protect\gls}

\newcommand*{\nxicounter}{\protect\icounter}

\newcommand{\dohtmlcolorbox}[2][]{#2}

\newcommand*{\ocaron}{\v{o}}
\newcommand*{\ocedilla}{\c{o}}
\newcommand*{\obarunder}{\b{o}}
\newcommand*{\odotunder}{\d{o}}
\newcommand*{\omacron}{\={o}}
\newcommand*{\odoubleacute}{\H{o}}
\newcommand*{\odotover}{\.{o}}
\newcommand*{\otilde}{\~{o}}
\newcommand*{\ocircum}{\^{o}}
\newcommand*{\obreve}{\u{o}}
\DeclareRobustCommand*{\oumlaut}{\"{o}}
\newcommand*{\uring}{\r{u}}
\newcommand*{\rbarunder}{\b{r}}
\newcommand*{\xytie}{\t{xy}}
\newcommand*{\dotlessj}{\j}

\newcommand{\indexEPS}{%
 \index{Encapsulated PostScript (EPS) file|hyperpage}%
 \index{file formats!eps@\texttt{.eps}|hyperpage}%
}

\newcommand{\indexDVI}{%
  \index{Device Independent (DVI) file|hyperpage}%
  \index{file formats!dvi@\texttt{.dvi}|hyperpage}%
}

\newcommand{\indexPDF}{%
  \index{Portable Document Format (PDF) file|hyperpage}%
  \index{file formats!pdf@\texttt{.pdf}|hyperpage}%
}

\newcommand{\indexTOC}{%
  \index{table of contents file (.toc)@table of contents file (\texttt{.toc})|hyperpage}%
  \index{file formats!toc@\texttt{.toc}|hyperpage}%
}

\newcommand{\indexLOF}{%
  \index{list of figures file (.lof)@list of figures file (\texttt{.lof})|hyperpage}%
  \index{file formats!lof@\texttt{.lof}|hyperpage}%
}

\newcommand{\indexLOT}{%
  \index{list of tables file (.lot)@list of tables file (\texttt{.lot})|hyperpage}%
  \index{file formats!lot@\texttt{.lot}|hyperpage}%
}

\newcommand{\indexLOG}{%
  \index{log file (.log)@log file (\texttt{.log})|hyperpage}%
  \index{file formats!log@\texttt{.log}|hyperpage}%
}

\newcommand{\indexAUX}{%
  \index{auxiliary file (.aux)@auxiliary file (\texttt{.aux})|hyperpage}%
  \index{file formats!aux@\texttt{.aux}|hyperpage}%
}

\newcommand{\iauxfile}{auxiliary (\texttt{.aux}) file\indexAUX}

\newcommand{\indexSYNCTEX}{%
  \index{synctex file (.synctex.gz)@synctex file (\texttt{.synctex.gz})|hyperpage}%
  \index{file formats!synctex.gz@\texttt{.synctex.gz}|hyperpage}%
}

\newcommand{\indexCLS}{%
  \index{class files (.cls)@class files (\texttt{.cls})|hyperpage}%
  \index{file formats!cls@\texttt{.cls}|hyperpage}%
}

\newcommand{\indexSTY}{%
  \index{packages files (.sty)@package files (\texttt{.sty})|hyperpage}%
  \index{file formats!sty@\texttt{.sty}|hyperpage}%
}

\newcommand*{\TO}{\ensuremath{\Rightarrow}}

\DeclareRobustCommand*{\extraindexaux}{%
 \index{file formats!aux@\texttt{.aux}|indexdef}%
}


\DeclareRobustCommand*{\extraindexsentencespacing}{%
 \index{spacing!French|indexdef}%
 \index{spacing!English|indexdef}%
 \index{French spacing|idxcrossref{spacing, French}}%
 \index{English spacing|idxcrossref{spacing, English}}%
 \index{inter-sentence spacing|idxcrossref{spacing, inter-sentence}}%
}

\DeclareRobustCommand*{\extraindexshortlong}{%
  \index{short command|idxcrossref{command, short}}%
  \index{long command|idxcrossref{command, long}}%
  \index{command!long|indexdef}%
}

\DeclareRobustCommand*{\extraindexfragile}{%
 \index{moving argument|idxcrossref{argument, moving}}%
 \index{argument!moving|indexdef}%
 \index{fragile command|idxcrossref{command, fragile}}%
}

\DeclareRobustCommand*{\extraindexoptional}{%
  \index{optional argument|idxcrossref{argument, optional}}%
}

\DeclareRobustCommand*{\extraindexmandatory}{%
  \index{mandatory argument|idxcrossref{argument, mandatory}}%
}

\DeclareRobustCommand*{\extraindexrobust}{%
  \index{robust command|idxcrossref{command, robust}}%
}

\DeclareRobustCommand*{\extraindexarg}{%
  \index{parameter|idxcrossref{argument}}%
}

\DeclareRobustCommand*{\extraindexgroup}{%
  \index{scope|idxcrossref{group}}%
}

\DeclareRobustCommand*{\extraindexterminal}{%
  \index{command prompt|idxcrossref{terminal}}%
}

\newcommand*{\preambleillustration}{%
\begin{ttfamily}
\begin{tabbing}
\glsni{documentclass}\marg{\ldots}\=\\
\>\rmfamily $\longleftarrow$ This bit in here is the preamble.\\
\glsni{begin}\marg{document}
\end{tabbing}
\end{ttfamily}
}

\newcommand{\dobibexamplenumbered}{%
\noindent\textbf{\Large Bibliography}\relax % simulate thebibliography
\begin{fwlist}{[3]}
\fwitem{[1]} ``\LaTeX: a document preparation
system'', Leslie Lamport, 2nd edition (updated for
\LaTeXe), Addison-Wesley (1994).

\fwitem{[2]} ``A Guide to \LaTeX: document
preparation for beginners and advanced users'',
Helmut Kopka and Patrick W. Daly, Addison-Wesley (1995).

\fwitem{[3]} ``The \LaTeX\ Companion'',
Michel Goossens, Frank Mittelbach and
Alexander Samarin, Addison-Wesley, (1994).
\end{fwlist}
}

\newcommand{\dobibexamplelabelled}{%
\par\noindent
{\Large\bfseries \bibname}%
\begin{fwlist}{[Goossens 1994]}
\fwitem{[Lamport 1994]} ``\LaTeX\ : a document preparation
system'', Leslie Lamport, 2nd edition (updated for
\LaTeXe), Addison-Wesley (1994).

\fwitem{[Kopka 1995]} ``A Guide to \LaTeX: document
preparation for beginners and advanced users'',
Helmut Kopka and Patrick W. Daly, Addison-Wesley (1995).

\fwitem{[Goossens 1994]} ``The \LaTeX\ Companion'',
Michel Goossens, Frank Mittelbach and
Alexander Samarin, Addison-Wesley, (1994).
\end{fwlist}
}

\newcommand{\dodescriptionexample}{%
\begin{description}
\item[Animal] Living being

\item[Vegetable] Plant

\item[Mineral] Natural inorganic substance
\end{description}
}

\newcommand{\dodescriptionkomaexample}{%
\addtokomafont{descriptionlabel}{\normalfont\scshape}
\begin{description}
\item[Animal] Living being

\item[Vegetable] Plant

\item[Mineral] Natural inorganic substance
\end{description}
}

\newcommand{\doexerciseexample}{%
\begin{result}
\setlength{\parindent}{1.5em}%
\par\textbf{Exercise}
\begin{itshape}\par\vspace{\baselineskip}
This is a sample.
\end{itshape}
\end{result}}

\newcommand*{\historyitem}[1]{\section*{#1}}

\ifnum\novicemedia=\NOVICEbook
  \newcommand*{\backcoverheading}{%
    \newpage\thispagestyle{empty}%
    \section*{Back Cover Text}%
  }
\else
  \newcommand*{\backcoverheading}{\chapter*{Back Cover Text}}
\fi

\endinput
