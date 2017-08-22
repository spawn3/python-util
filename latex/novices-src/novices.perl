#!/usr/bin/perl
package main;

&do_require_package("report");
&do_require_package("makeidx");
&do_require_package("alltt");
&do_require_package("amsmath");
&do_require_package("color");
&do_require_package("html");
&do_require_package("datetime");

&do_require_package('inputenc');
&do_inputenc_utf8;

sub do_novices_html{}

%section_commands = 
(
  'printkeywords', '2',
  'listofexercises', '2',
  'backcoverheading', '2',
   %section_commands
);

&do_require_package("glossaries");

&make_newglossarytype("acronym", "\\acronymname");
$acronymtype = 'acronym';

&make_newglossarytype("keywords", "Some Definitions");

$gls_title{'main'} = "Summary";
$gls_toctitle{'main'} = "Summary";

$SUMMARYTYPE='main';
$KEYWORDTYPE='keywords';

$sidepanelcontents = '';

$INDEXONLYFIRST=1;

$LISTOFEXERCISESTITLE = 'List of Exercises';
$BACKCOVERTITLE = 'Back Cover Text';

sub do_cmd_printkeywords{
  local($after,$ot) = @_;

  local($id) = ++$global{'max_id'};

  $after = "$O$id$C$gls_title{keywords}$O$id$C $after";

  local($open_tags_R) = defined $ot ? $ot : $open_tags_R;
  &do_cmd_section_helper('H1', 'chapter');
}

sub do_cmd_listofexercises{
  local($after,$ot) = @_;

  local($id) = ++$global{'max_id'};

  $after = "$O$id$C$LISTOFEXERCISESTITLE$O$id$C\\deferredlistofexercises $after";

  local($open_tags_R) = defined $ot ? $ot : $open_tags_R;

  &do_cmd_section_helper('H1', 'chapterstar');
}

sub do_cmd_backcoverheading{
  local($after,$ot) = @_;

  local($id) = ++$global{'max_id'};

  $after = "$O$id$C$BACKCOVERTITLE$O$id$C $after";

  local($open_tags_R) = defined $ot ? $ot : $open_tags_R;

  &do_cmd_section_helper('H1', 'chapterstar');
}

$ADDRESS = '';

$MAX_LINK_DEPTH=1;
$child_name='';
$NUMBERED_FOOTNOTES=1;
$TOP_NAVIGATION=1;
$BOTTOM_NAVIGATION=1;

$CHILDLINE = "<BR>";
sub upper_child_line{ '<BR>'; };

$UPCHAR = '<font style="font-size: xx-large; font-weight: bold;">&#x21E7;</font>';
$PREVCHAR = '<font style="font-size: xx-large; font-weight: bold;">&#x21E6;</font>';
$NEXTCHAR = '<font style="font-size: xx-large; font-weight: bold;">&#x21E8;</font>';

sub top_navigation_panel{
  local($gonext) = $NEXT;
  local($goprev) = $PREVIOUS;
  local($goup) = $UP;

  if ($gonext=~s/<tex2html_next_page_visible_mark>/$NEXTCHAR<BR>Next/)
  {
     $gonext = "<div class=\"navigatenext\">$gonext</div>";
  }
  else
  {
    $gonext=~s/<tex2html_next_page_inactive_visible_mark>/<div class="disnavigatenext">$NEXTCHAR<BR>Next<\/div>/;
  }

  if ($goprev=~s/<tex2html_previous_page_visible_mark>/$PREVCHAR<BR>Previous/)
  {
     $goprev = "<div class=\"navigateprev\">$goprev</div>";
  }
  else
  {
    $goprev=~s/<tex2html_previous_page_inactive_visible_mark>/<div class="disnavigateprev">$PREVCHAR<BR>Previous<\/div>/;
  }

  if ($goup=~s/<tex2html_up_visible_mark>/$UPCHAR<BR>Up/)
  {
     $goup = "<div class=\"navigateup\">$goup</div>";
  }
  else
  {
    $goup=~s/<tex2html_up_inactive_visible_mark>/<div class="disnavigateup">$UPCHAR<BR>Up<\/div>/;
  }

  $MAX_LINK_DEPTH=1;
  local ($nav) = &add_child_links('', $file, $depth, $child_star, $key, @keys);
  $MAX_LINK_DEPTH=-1;

<<_END_HEADER_TEXT;
 <div id="banner">Dickimaw Books</DIV>
    <div id="toolbar">
      <div class="tool">
      <a href="http://www.dickimaw-books.com/">About</a>
      </div>
      <div class="tool">
      <a href="http://www.dickimaw-books.com/latexresources.html">LaTeX</a>
      </div>
      <div class="tool">
      <a href="http://www.dickimaw-books.com/software.html">Free Software</a>
      </div>
      <div class="tool">
      <a href="http://www.dickimaw-books.com/books.html">Books</a>
      </div>
      <div class="tool">
      <a href="http://www.dickimaw-books.com/consultancy.html">Consultancy</a>
      </div>
      <div class="tool">
      <a href="http://www.dickimaw-books.com/contact.html">Contact</a>
      </div>
    </div>
    <div id="sidebar">
     <div style="text-align: center;">
     <A HREF="http://www.dickimaw-books.com/latex/novices/index.html">Book Home Page</A><P>
     <A HREF="contents.html">Contents</A>
     <A HREF="summary.html">Summary</A>
     <A HREF="bookindex.html">Index</A>
     </div>
    <P>
    <DIV class="navigate">
    $goprev $goup $gonext
    <BR>
    </DIV>
    <P>
    $nav
    <div class="idxnav"></div>
    </div>
    <div id="main">
_END_HEADER_TEXT
}

sub bot_navigation_panel{
<<'_END_TEXT';
    </div>
    <div id="boilerplate">
    &copy; 2012 Dickimaw Books.
    "Dickimaw", "Dickimaw Books" and the Dickimaw parrot logo are
    trademarks. The logo was derived from a painting by Magdalene
Pritchett.
    </div>
_END_TEXT
}

eval(<<'_END_EVAL');
sub replace_markers {
    &find_quote_ligatures;
    &replace_general_markers;
    &replace_extra_markers;
    &text_cleanup;
    &replace_sensitive_markers;
    &replace_init_file_mark if (/$init_file_mark/);
    &replace_file_marks;
    &post_replace_markers;
}

sub add_idx{
   local($sidx_style, $eidx_style) =("","");
   &add_book_idx(@_);
}

sub named_index_entry{
   &book_named_index_entry(@_);
}

sub do_env_alltt {
    local ($_) = @_;
    local($closures,$reopens,$alltt_start,$alltt_end,@open_block_tags);

    $alltt_start = "\n<DIV CLASS=\"alltt\" ALIGN=\"LEFT\">\n";
    $alltt_end = "\n</DIV>\n";

    # get the tag-strings for all open tags
    local(@keep_open_tags) = @$open_tags_R;
    ($closures,$reopens) = &preserve_open_tags() if (@$open_tags_R);

    # get the tags for text-level tags only
    $open_tags_R = [ @keep_open_tags ];
    local($local_closures, $local_reopens);
    ($local_closures, $local_reopens,@open_block_tags) = &preserve_open_block_tags
	if (@$open_tags_R);

    $open_tags_R = [ @open_block_tags ];

    do {
	local($open_tags_R) = [ @open_block_tags ];
	local(@save_open_tags) = ();

	local($cnt) = ++$global{'max_id'};
	$_ = join('',"$O$cnt$C\\tt$O", ++$global{'max_id'}, $C
		, $_ , $O, $global{'max_id'}, "$C$O$cnt$C");

	$_ = &translate_environments($_);
	$_ = &translate_commands($_) if (/\\/);

	# preserve space-runs, using &nbsp;
	while (s/(\S) ( +)/$1$2;SPMnbsp;/g){};
	s/(<BR>) /$1;SPMnbsp;/g;

	$_ = join('', $closures, $alltt_start , $local_reopens
		, $_
		, &balance_tags() #, $local_closures
		, $alltt_end, $reopens);
	undef $open_tags_R; undef @save_open_tags;
    };

    $open_tags_R = [ @keep_open_tags ];
    $_;
}
_END_EVAL

sub replace_extra_markers{
  s/<tex2html_glstoc_deferred>/\\deferredglstoc /eo;
  s/<div class="idxnav"><\/div>/$sidepanelcontents/m;
  $sidepanelcontents = '';
}

$global{'exercise'} = 0;

sub do_cmd_theexercise{
   local($_) = @_;

   $global{'exercise'}.$_;
}

$global{'keyword'} = 0;

sub do_cmd_thekeyword{
   local($_) = @_;

   $global{'keyword'}.$_;
}

sub do_env_fwlist{
   local($_) = @_;
   local($maxtag);

   $maxtag = &missing_braces
              unless (s/$next_pair_rx/$maxtag=$2,''/eo);

  "<DL>$_</DL>";
}

sub do_cmd_fwitem{
   local($_) = @_;
   local($text);

   $text = &missing_braces
              unless (s/$next_pair_pr_rx/$text=$2,''/eo);

   "<DT>$text</DT><DD>".$_;
}

sub do_env_exercise{
   local($_) = @_;
   local($title, $label,$id1, $id2);

   local($backref,$pat) = &get_next_optional_argument;

   $title = &missing_braces
              unless (s/$next_pair_rx/$id1=$1,$title=$2,''/eo);

   $label = &missing_braces
              unless (s/$next_pair_rx/$id2=$1,$label=$2,''/eo);

   &translate_commands(
      "\\refstepcounter$OP$id1${CP}exercise$OP$id1$CP"
     ."\\label$OP$id2$CP$label$OP$id2$CP").

   "<BLOCKQUOTE><B>Exercise \\theexercise: $title</B><P>" . $_ . "</BLOCKQUOTE>";
}

sub do_cmd_deferredlistofexercises{
   local($_) = @_;

   local($ext) = "hloe";

   unless (&process_ext_file($ext))
   {
      &write_warnings("\nThe '$FILE.$ext' was not found.");
   }

   $_;
}

sub do_env_exerciselist{
  local($_) = @_;

  "<OL>$_</OL>\n<P>\n<A HREF=\"exercises/index.html\">Solutions to Exercises</A>";
}

sub do_cmd_htmlexerciseitem{
   local($_) = @_;

   local($id, $title, $label, $node, $num);

   $title = &missing_braces
              unless (s/$next_pair_pr_rx/$id1=$1,$title=$2,''/eo);

   $label = &missing_braces
              unless (s/$next_pair_pr_rx/$id2=$1,$label=$2,''/eo);

   $node = &missing_braces
              unless (s/$next_pair_pr_rx/$id2=$1,$node=$2,''/eo);

   $num = &missing_braces
              unless (s/$next_pair_pr_rx/$id2=$1,$num=$2,''/eo);

   "<LI VALUE=\"$num\"> <A HREF=\"$node.html#$label\">$title</A>$_";
}

$EXTRA_IMAGE_SCALE=2;

sub do_env_htmlresult{
   local($_) = @_;
   local($desc, $id);

   $desc = &missing_braces
              unless (s/$next_pair_rx/$id=$1,$desc=$2,''/eo);

   local ($id2) = ++$global{'max_id'};

   local($code) = $_;

   unless ($desc)
   {
     $desc = "Image showing typeset output";
   }

   local($text);

   if ($desc=~/^(.+)\.html$/)
   {
     local($base) = $1;

     $alttext = "Image showing typeset output (click here for a more detailed description).";

     $code=~s/\\begin$O(\d+)${C}makeimage$O\1$C/$&\\htmlimage$O$id${C}alt=$alttext$O$id$C$O$id2$C$O$id2$C/;

     # remove any labels

     $code=~s/\\label\s*$O(\d+)$C(.*?)$O\1$C//g;

     $text = "<P><DIV CLASS=\"result\"><A NAME=\"$base\" HREF=\"imagedesc/$desc\">$code</A></DIV><P>";
   }
   else
   {
     $desc=~s/\\n/ /g;

     $desc=~s/\\tildesym /~/g;

     $code=~s/\\begin$O(\d+)${C}makeimage$O\1$C/$&\\htmlimage$O$id${C}alt=$desc$O$id$C$O$id2$C$O$id2$C/;

     $text = "<P><DIV CLASS=\"result\">$code</DIV></P>";
   }

   $text
}

# don't convert contents of starred form into an image
sub do_env_resultSstar{
   local($_) = @_;

   local ($contents) = &translate_commands($_);

   join('', "<P><DIV CLASS=\"result\">", $contents, "</DIV><P>");
}

sub do_env_bcode{
   local($_) = @_;
   "<P><HR><TT>$_</TT><HR><P>";
}

sub do_env_code{
   local($_) = @_;
   local($width,$pat) = &get_next_optional_argument;
   "<P><DIV CLASS=\"code\">$_</DIV><P>";
}

sub do_env_codeS{
   local($_) = @_;
   local($width,$pat) = &get_next_optional_argument;
   "<P><DIV CLASS=\"code\">$_</DIV><P>";
}

sub do_env_definition{
   local($_) = @_;
   local($width,$pat) = &get_next_optional_argument;
   "<P><DIV CLASS=\"definition\">$_</DIV><P>";
}

sub do_cmd_keywords{
   local($_) = @_;
   local($keywords);

   $keywords = &missing_braces unless
      s/$next_pair_pr_rx/$keywords=$2;''/eo;

   "<META NAME=\"Keywords\" CONTEXT=\"$keywords\">".$_;
}

sub do_cmd_optfmt{
   local($_) = @_;
   local($opt);

   $opt = &missing_braces unless
      s/$next_pair_pr_rx/$opt=$2;''/eo;

   "<TT>$opt</TT>".$_;
}

sub do_faqlink{
   local($title, $label) = @_;

   "<A HREF=\"http://www.tex.ac.uk/cgi-bin/texfaq2html?label=$label\">$title</A>"
}

sub do_cmd_faqlink{
   local($_) = @_;
   local($title,$label);

   $title = &missing_braces unless
      s/$next_pair_pr_rx/$title=$2;''/eo;

   $label = &missing_braces unless
      s/$next_pair_pr_rx/$label=$2;''/eo;

   &do_faqlink($title, $label).$_;
}

sub do_cmd_faq{
   local($_) = @_;
   local($title,$label);

   $title = &missing_braces unless
      s/$next_pair_pr_rx/$id1=$1,$title=$2;''/eo;

   $label = &missing_braces unless
      s/$next_pair_pr_rx/$id2=$1,$label=$2;''/eo;

   "[".&do_faqlink($title,$label)."]".$_;
}

sub do_cmd_meta{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<EM>&lt;$text&gt;</EM>" . $_;
}

sub do_cmd_dq{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "&#147;$text&#148;" . $_;
}

sub do_cmd_sq{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "&#145;$text&#146;" . $_;
}

sub do_cmd_hyperpage{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   $text . $_;
}

sub do_cmd_indexdef{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<EM>$text</EM>" . $_;
}

sub do_cmd_indexglo{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<B>$text</B>" . $_;
}

sub do_cmd_ttbackslash{
   local($_) = @_;

   "<TT>&#092;</TT>" . $_;
}

sub do_cmd_marg{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT><A CLASS=\"gls\" HREF=\"$gls_file_mark{main}#gls:leftbracechar\">{</A>"
    .$text
    ."<A CLASS=\"gls\" HREF=\"$gls_file_mark{main}#gls:rightbracechar\">}</A></TT>" . $_;
}

sub do_cmd_margstar{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>{$text}</TT>" . $_;
}

sub do_cmd_oarg{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT><A CLASS=\"gls\" HREF=\"$gls_file_mark{main}#gls:opt.opensq\">[</A>"
    .$text
    ."<A CLASS=\"gls\" HREF=\"$gls_file_mark{main}#gls:opt.closesq\">]</A></TT>" . $_;
}

sub do_cmd_oargstar{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>[$text]</TT>" . $_;
}

sub do_cmd_parg{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>($text)</TT>" . $_;
}

sub do_cmd_leftbrace{
   local($_) = @_;
   "&#123;".$_;
}

sub do_cmd_rightbrace{
   local($_) = @_;
   "&#125;".$_;
}

sub do_cmd_leftbracesym{
   local($_) = @_;
   "&#123;".$_;
}

sub do_cmd_rightbracesym{
   local($_) = @_;
   "&#125;".$_;
}

sub do_cmd_leftsqbrace{
   local($_) = @_;
   "&#091;".$_;
}

sub do_cmd_rightsqbrace{
   local($_) = @_;
   "&#093;".$_;
}

sub do_cmd_percentsym{
   local($_) = @_;
   "&#037;".$_;
}

sub do_cmd_hashsym{
   local($_) = @_;
   "&#035;".$_;
}

sub do_cmd_atsym{
   local($_) = @_;
   "&#064;".$_;
}

sub do_cmd_xatsym{
   local($_) = @_;
   "&#064;".$_;
}

sub do_cmd_ampsym{
   local($_) = @_;
   "&#038;".$_;
}

sub do_cmd_vbarsym{
   local($_) = @_;
   "&#124;".$_;
}

sub do_cmd_questionsym{
   local($_) = @_;
   "&#063;".$_;
}

sub do_cmd_dollarsym{
   local($_) = @_;
   "&#036;".$_;
}

sub do_cmd_keyword{
   local($_) = @_;
   local($text,$indextext,$pat,$id);

   ($indextext,$pat) = &get_next_optional_argument;

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   if ($indextext eq '')
   {
      $indextext = $text
   }

   &translate_commands("\\keywordfmt $OP$id$CP$text$OP$id$CP")
    . "\\index$OP$id$CP$indextext$OP$id$CP" . $_;
}

sub do_cmd_symbolcmddef{
   local($_) = @_;
   local($symbol,$id);

   $symbol = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$symbol=$2;''/eo;

  "<TT>&#092;$symbol</TT>\\indexsymbolcmddef$OP$id$CP$symbol$OP$id$CP".$_;
}

sub do_cmd_indexsymbolcmddef{
   local($_) = @_;
   local($symbol,$id);

   $symbol = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$symbol=$2;''/eo;

  "\\index$OP$id$CP\\$symbol@<TT>&#092;$symbol</TT>|indexdef$OP$id$CP".$_;
}

sub do_cmd_symbolcmd{
   local($_) = @_;
   local($symbol,$id);

   $symbol = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$symbol=$2;''/eo;

  "<TT>&#092;$symbol</TT>\\indexsymbolcmd$OP$id$CP$symbol$OP$id$CP".$_;
}

sub do_cmd_indexsymbolcmd{
   local($_) = @_;
   local($symbol,$id);

   $symbol = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$symbol=$2;''/eo;

  "\\index$OP$id$CP\\$symbol@<TT>&#092;$symbol</TT>$OP$id$CP".$_;
}

sub do_cmd_ibackslashcircumdef{
   local($_)=@_;

   local($id) = ++$global{'max_id'};

   "<TT>&#092;&#094;</TT>\\index$OP$id$CP\\^ @<TT>&#092;&#094;</TT>$OP$id$CP".$_;
}

sub do_cmd_backslashcircumsort{
   local($_)=@_;

   "\\^ " . $_;
}

sub do_cmd_ibackslashtildedef{
   local($_)=@_;

   local($id) = ++$global{'max_id'};

   "<TT>&#092;&#126;</TT>\\index$OP$id$CP\\~ @<TT>&#092;&#126;</TT>$OP$id$CP".$_;
}

sub do_cmd_backslashtildesort{
   local($_)=@_;

   "\\~ " . $_;
}

sub indexfileformat{
   local($ext) = @_;

   local($id) = ++$global{'max_id'};

   &named_index_entry($id, "file formats!.$ext@<TT>.$ext</TT>"),
}

sub do_cmd_indexEPS{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "Encapsulated PostScript (EPS) file"),
      &indexfileformat('eps'),
      $_);
}

sub do_cmd_indexPDF{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "Portable Document Format (PDF) file"),
      &indexfileformat('pdf'),
      $_);
}

sub do_cmd_indexDVI{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "Device Independent (DVI) file"),
      &indexfileformat('dvi'),
      $_);
}

sub do_cmd_indexSYNCTEX{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "synctex file"),
      &indexfileformat('synctex.gz'),
      $_);
}

sub do_cmd_indexCLS{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "class files"),
      &indexfileformat('cls'),
      $_);
}

sub do_cmd_indexSTY{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "package files"),
      &indexfileformat('sty'),
      $_);
}

sub do_cmd_indexTOC{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "table of contents (toc) file"),
      &indexfileformat('toc'),
      $_);
}

sub do_cmd_indexLOF{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "list of figures (lof) file"),
      &indexfileformat('lof'),
      $_);
}

sub do_cmd_indexLOT{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "list of tables (lot) file"),
      &indexfileformat('lot'),
      $_);
}

sub do_cmd_indexLOG{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "log file"),
      &indexfileformat('log'),
      $_);
}

sub do_cmd_indexAUX{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "auxiliary file"),
      &indexfileformat('aux'),
      $_);
}

sub do_cmd_iauxfile{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('', 
      'auxiliary (<tt>.aux</tt>) file',
      &named_index_entry($id, "auxiliary file"),
      &indexfileformat('aux'),
      $_);
}

sub do_cmd_iBiBTeX{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('', 
      'BibTeX',
      &named_index_entry($id, "BibTeX"),
      $_);
}

sub do_cmd_iPDFLaTeX{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   join('', 
      'PDFLaTeX',
      &named_index_entry($id, "PDFLaTeX"),
      $_);
}

sub do_cmd_extraindexaux{
   "<tex2html_extraindexaux_deferred>".$_[0];
}

sub do_cmd_deferredextraindexaux{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   join('', 
      &named_index_entry(++$global{'max_id'}, "file formats!aux@<TT>.aux</TT>|indexdef"),
      $_);
}

sub do_cmd_extraindexsentencespacing{
   "<tex2html_extraindexsentencespacing_deferred>".$_[0];
}

sub do_cmd_deferredextraindexsentencespacing{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my($id1) = ++$global{'max_id'};
   my($id2) = ++$global{'max_id'};
   my($id3) = ++$global{'max_id'};
   my($id4) = ++$global{'max_id'};
   my($id5) = ++$global{'max_id'};

   join('', 
      &named_index_entry($id1, "spacing!French|indexdef"),
      &named_index_entry($id2, "spacing!English|indexdef"),
      &named_index_entry($id3, "French spacing|idxcrossref$OP$id1${CP}spacing, French$OP$id1$CP"),
      &named_index_entry($id4, "English spacing|idxcrossref$OP$id2${CP}spacing, English$OP$id2$CP"),
      &named_index_entry($id5, "inter-sentence spacing|idxcrossref$OP$id3${CP}spacing, inter-sentence$OP$id3$CP"),
      $_);
}

sub do_cmd_extraindexshortlong{
   "<tex2html_extraindexshortlong_deferred>".$_[0];
}

sub do_cmd_deferredextraindexshortlong{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my($id1) = ++$global{'max_id'};
   my($id2) = ++$global{'max_id'};
   my($id3) = ++$global{'max_id'};

   join('', 
      &named_index_entry($id1, "short command|idxcrossref$OP$id2${CP}command, short$OP$id1$CP"),
      &named_index_entry($id2, "long command|idxcrossref$OP$id2${CP}command, long$OP$id2$CP"),
      &named_index_entry($id3, "command!long|indexdef"),
      $_);
}

sub do_cmd_extraindexfragile{
   "<tex2html_extraindexfragile_deferred>".$_[0];
}

sub do_cmd_deferredextraindexfragile{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my($id) = ++$global{'max_id'};

   join('', 
      &named_index_entry($id, "moving argument|idxcrossref$OP$id1${CP}argument, moving$OP$id1$CP"),
      &named_index_entry($id, "argument!moving|indexdef"),
      &named_index_entry($id, "fragile command|idxcrossref$OP$id2${CP}command, fragile$OP$id2$CP"),
      $_);
}

sub do_cmd_extraindexoptional{
   "<tex2html_extraindexoptional_deferred>".$_[0];
}

sub do_cmd_deferredextraindexoptional{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my($id) = ++$global{'max_id'};

   join('', 
      &named_index_entry($id, "optional argument|idxcrossref$OP$id1${CP}argument, optional$OP$id1$CP"),
      $_);
}

sub do_cmd_extraindexmandatory{
   "<tex2html_extraindexmandatory_deferred>".$_[0];
}

sub do_cmd_deferredextraindexmandatory{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my($id) = ++$global{'max_id'};

   join('', 
      &named_index_entry($id, "mandatory argument|idxcrossref$OP$id1${CP}argument, mandatory$OP$id1$CP"),
      $_);
}

sub do_cmd_extraindexrobust{
   "<tex2html_extraindexrobust_deferred>".$_[0];
}

sub do_cmd_deferredextraindexrobust{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my($id) = ++$global{'max_id'};

   join('', 
      &named_index_entry($id, "robust command|idxcrossref$OP$id1${CP}command, robust$OP$id1$CP"),
      $_);
}

sub do_cmd_extraindexarg{
   "<tex2html_extraindexarg_deferred>".$_[0];
}

sub do_cmd_deferredextraindexarg{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my($id) = ++$global{'max_id'};

   join('', 
      &named_index_entry($id, "parameter|idxcrossref$OP$id1${CP}argument$OP$id1$CP"),
      $_);
}

sub do_cmd_extraindexgroup{
   "<tex2html_extraindexgroup_deferred>".$_[0];
}

sub do_cmd_deferredextraindexgroup{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my($id) = ++$global{'max_id'};

   join('', 
      &named_index_entry($id, "scope|idxcrossref$OP$id1${CP}group$OP$id1$CP"),
      $_);
}

sub do_cmd_extraindexterminal{
   "<tex2html_extraindexterminal_deferred>".$_[0];
}

sub do_cmd_deferredextraindexterminal{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my($id) = ++$global{'max_id'};
   my($id1) = ++$global{'max_id'};

   join('', 
      &named_index_entry($id, "command prompt|idxcrossref$OP$id1${CP}terminal$OP$id1$CP"),
      $_);
}

sub do_cmd_doublequestionmark{
   local($_) = @_;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
      '&#063;&#063;',
      &named_index_entry($id, "&#063;&#063; (undefined reference)"),
      $_);
}

sub do_cmd_Index{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     $text,
      &named_index_entry($id, $text),
      $_);
}

sub do_cmd_Indextt{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text</TT>",
      &named_index_entry($id, "$text@<TT>$text</TT>"),
      $_);
}

sub do_cmd_indextt{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "$text@<TT>$text</TT>"),
      $_);
}

sub do_cmd_indexttdef{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "$text@<TT>$text</TT>|indexdef"),
      $_);
}

sub do_cmd_envname{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>$text</TT>" . $_;
}

sub do_cmd_envdef{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   "<TT>$text</TT>\\indexEnvdef $OP$id$CP$text$OP$id$CP" . $_;
}

sub do_cmd_ienvname{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   "<TT>$text</TT>\\indexEnv $OP$id$CP$text$OP$id$CP" . $_;
}

sub do_cmd_cmdname{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<CODE>&#092;$text</CODE>" . $_;
}

sub do_cmd_ipagestyle{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($fmttext) = &translate_commands("\\pagestylefmt $OP$id$CP$text$OP$id$CP");

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
      $fmttext,
      &named_index_entry($id, "page style!$text"),
      $_);
}

sub do_cmd_pagestylefmt{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>$text</TT>" . $_;
}

sub do_cmd_icounter{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text</TT>",
      &named_index_entry($id, "counters!$text"),
      $_);
}

sub do_cmd_cmddef{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<CODE>&#092;$text</CODE>",
      &named_index_entry($id, "$text@<TT>&#092;$text</TT>|indexdef"),
      $_);
}

sub do_cmd_icmdname{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<CODE>&#092;$text</CODE>",
      &named_index_entry($id, "$text@<TT>&#092;$text</TT>"),
      $_);
}

sub do_cmd_indexCom{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "$text@<TT>&#092;$text</TT>"),
      $_);
}

sub do_cmd_indexEnv{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "$text environment@<TT>$text</TT> environment"),
      $_);
}

sub do_cmd_indexEnvdef{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "$text environment@<TT>$text</TT> environment|indexdef"),
      $_);
}

sub do_cmd_glocsindex{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "$text@<TT>&#092;$text</TT>|indexglo"),
      $_);
}

sub do_cmd_isty{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text</TT>",
      &named_index_entry($id, "$text package@<TT>$text</TT> package"),
      &named_index_entry($id, "packages!$text@<TT>$text</TT>"),
      $_);
}

sub do_cmd_icls{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text</TT>",
      &named_index_entry($id, "$text class@<TT>$text</TT> class"),
      &named_index_entry($id, "class files!$text@<TT>$text</TT>"),
      $_);
}

sub do_cmd_nxscrclsopt{
  "<tex2html_scrclsopt_deferred>".$_[0];
}

sub do_cmd_deferredscrclsopt{
   local($_) = @_;
   local($text,$id);

   local($opt,$pat) = &get_next_optional_argument;

   $opt = '='.$opt if ($opt);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text$opt</TT>",
      &named_index_entry($id, "KOMA Script class options!$text@<TT>$text</TT>"),
      &named_index_entry($id, "class file options!$text@<TT>$text</TT>"),
      $_);
}

sub do_cmd_scrclsopt{
   local($_) = @_;
   local($text,$id);

   local($opt,$pat) = &get_next_optional_argument;

   $opt = '='.$opt if ($opt);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text$opt</TT>",
      &named_index_entry($id, "KOMA Script class options!$text@<TT>$text</TT>"),
      &named_index_entry($id, "class file options!$text@<TT>$text</TT>"),
      $_);
}

sub do_cmd_nxclsopt{
  "<tex2html_clsopt_deferred>".$_[0];
}

sub do_cmd_clsopt{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text</TT>",
      &named_index_entry($id, "class file options!$text@<TT>$text</TT>"),
      $_);
}

sub do_cmd_istyopt{
   local($_) = @_;
   local($text,$sty,$id);

   $sty = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$sty=$2;''/eo;

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text</TT>",
      &named_index_entry($id, "$sty package!$text@<TT>$text</TT>"),
      $_);
}

sub do_cmd_deferredclsopt{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text</TT>",
      &named_index_entry($id, "class file options!$text@<TT>$text</TT>"),
      $_);
}

sub do_cmd_ikeyvalopt{
   local($_) = @_;
   local($text, $opt,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   $opt = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$opt=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$opt</TT>",
      &named_index_entry($id, "$text@&#092;$text!$opt@<TT>$opt</TT>"),
      $_);
}

sub do_cmd_appname{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>$text</TT>" . $_;
}

sub do_cmd_iappname{
   local($_) = @_;
   local($text,$indextext,$pat,$id);

   ($indextext,$pat) = &get_next_optional_argument;

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   if ($indextext eq '')
   {
      $indextext = $text
   }

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text</TT>",
      &named_index_entry($id, "$indextext application@<TT>$indextext</TT> application"),
      $_);
}

sub do_cmd_iappnamelink{
   local($_) = @_;
   local($text,$url, $insert, $pat);

   ($insert,$pat) = &get_next_optional_argument;

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   $url = &missing_braces unless
      s/$next_pair_pr_rx/$url=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<A HREF=\"$url\"><TT>$text</TT></A>",
      &named_index_entry(++$global{'max_id'},
        "$text application@<TT>$text</TT> application"),
       $insert,
      $_);
}

sub do_cmd_texdistro{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>$text</TT>" . $_;
}

sub do_cmd_itexdistro{
   local($_) = @_;
   local($text,$indextext,$pat,$id);

   ($indextext,$pat) = &get_next_optional_argument;

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   if ($indextext eq '')
   {
      $indextext = $text
   }

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('',
     "<TT>$text</TT>",
      &named_index_entry($id, "$indextext@<TT>$indextext</TT>"),
      &named_index_entry(++$global{'max_id'},
        "TeX Distributions!$indextext@<TT>$indextext</TT>"),
      $_);
}

sub do_cmd_itexdistrolink{
   local($_) = @_;
   local($text, $indextext,$url, $insert, $pat);

   ($insert,$pat) = &get_next_optional_argument;

   $indextext = &missing_braces unless
      s/$next_pair_pr_rx/$indextext=$2;''/eo;

   $url = &missing_braces unless
      s/$next_pair_pr_rx/$url=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my $id = ++$global{'max_id'};

   my $text = &do_cmd_texdistro("$OP$id$CP$indextext$OP$id$CP");

   join('',
     "<A HREF=\"$url\">$text</A>",
      &named_index_entry($id, "$indextext\@$text"),
      &named_index_entry(++$global{'max_id'},
        "TeX Distributions!$indextext\@$text"),
      $insert,
      $_);
}

sub do_cmd_perldistro{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>$text</TT>" . $_;
}

sub do_cmd_iperldistro{
   local($_) = @_;
   local($text,$url, $insert, $pat);

   ($insert,$pat) = &get_next_optional_argument;

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   $url = &missing_braces unless
      s/$next_pair_pr_rx/$url=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   my $indextext = &do_cmd_perldistro("$OP$id$CP$text$OP$id$CP");

   join('',
     "<A HREF=\"$url\"><TT>$text</TT></A>",
      &named_index_entry(++$global{'max_id'},
        "$text\@$indextext"),
      $insert,
      $_);
}

sub do_cmd_sty{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>$text</TT>" . $_;
}

sub do_cmd_cls{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>$text</TT>" . $_;
}

sub do_cmd_counter{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>$text</TT>" . $_;
}

sub do_cmd_menuto{
   local($_) = @_;
   "&#x2192;".$_;
}

sub do_cmd_menu{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>$text</TT>" . $_;
}

sub do_cmd_startmenu{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   "<TT>Start &#x2192; Programs &#x2192; $text</TT>" . $_;
}

sub do_cmd_incPgfOrGraphics{
  &do_cmd_incGraphics(@_);
}

sub do_cmd_incGraphics{
   local($_) = @_;
   local($img, $alt);

   local($opt,$pat) = &get_next_optional_argument;

   if ($opt=~/\balt\s*=\s*([^,]+)/i)
   {
      $alt = "ALT=\"$1\"";
   }
   else
   {
      $alt = '';
   }

   $img = &missing_braces unless
      s/$next_pair_pr_rx/$img=$2;''/eo;

   "<IMG SRC=\"$img.png\" $alt>";
}

sub do_cmd_frontmatter{

  join("\n",
  "For the document source and alternative formats of this book, go to",
  "<A HREF=\"../index.html\">this book's home page</A>.",
  '<P>',
  $copyrighttext,
  '<HR>',
  $_[0]); 
}


# override default interpretation of \LaTeX (looks a bit ugly in HTML!)

sub do_cmd_LaTeX{
   local($_)=@_;

   "LaTeX".$_;
}

sub do_cmd_LaTeXe{
   local($_)=@_;

   "LaTeX2e".$_;
}

sub do_cmd_TeX{
   local($_)=@_;

   "TeX".$_;
}

sub do_cmd_BiBTeX{
   local($_)=@_;

   "BibTeX".$_;
}

sub do_cmd_PDFLaTeX{
   local($_)=@_;

   "PDFLaTeX".$_;
}

sub do_cmd_subject{
   local($_) = @_;
   local($text);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$text=$2;''/eo;

   $_;
}

sub do_cmd_enter{
   local($_) = @_;
   "<IMG SRC=\"pictures/entersymbol.png\" ALIGN=\"CENTER\" ALT=\"return key symbol\">".$_;
}

sub do_cmd_degrees{
   local($_) = @_;

   local($value);

   $value = &missing_braces unless
     s/$next_pair_pr_rx/$value=$2;''/eo;

   "$value&nbsp;degrees$_";
}

sub do_cmd_yen{
   local($_)=@_;

   "&yen;".$_;
}

sub do_cmd_textregistered{
   local($_)=@_;

   "&#174;".$_;
}

sub do_cmd_textcopyright{
   local($_)=@_;

   "&#169;".$_;
}

sub do_cmd_textquoteleft{
   local($_)=@_;

   "&#145;".$_;
}

sub do_cmd_textquoteright{
   local($_)=@_;

   "&#146;".$_;
}

sub do_cmd_textquotedblleft{
   local($_)=@_;

   "&#147;".$_;
}

sub do_cmd_textquotedblright{
   local($_)=@_;

   "&#148;".$_;
}

sub do_cmd_dash{
   local($_)=@_;

   "&#151;".$_;
}

sub do_cmd_textemdash{
   local($_)=@_;

   "&#151;".$_;
}

sub do_cmd_textendash{
   local($_)=@_;

   "&#150;".$_;
}

sub do_cmd_dag{
   local($_)=@_;

   "&#134;".$_;
}

sub do_cmd_ddag{
   local($_)=@_;

   "&#135;".$_;
}

sub do_cmd_TO{
   local($_)=@_;

   "&#x21D2;".$_;
}

sub do_cmd_warning{
  "<div class=\"warning\">&#x26A0;</div>".$_[0];
}

sub do_cmd_wrong{
   local($_)=@_;

  "<div class=\"incorrect\">&#x2718;</div>".$_[0];
}

sub do_cmd_correct{
   local($_)=@_;

  "<div class=\"correct\">&#x2714;</div>".$_[0];
}

sub do_cmd_comma{
   local($_)=@_;

   ",".$_;
}

sub do_cmd_middot{
   local($_)=@_;

   "&#183;".$_;
}

sub do_cmd_oelig{
   local($_)=@_;

   "&#156;".$_;
}

sub do_cmd_iquestiondowncmd{
   local($_)=@_;

   local($id) = ++$global{'max_id'};

   "&#063;`".&translate_commands("\\index$OP$id$CP?`$OP$id$CP").$_;
}

sub do_cmd_iexclamdowncmd{
   local($_)=@_;

   local($id) = ++$global{'max_id'};

   "&#033;`".&translate_commands("\\index$OP$id$CP!`$OP$id$CP").$_;
}

sub do_cmd_refstar{
   local($_)=@_;
   local($label,$id);

   $label = &missing_braces unless
              s/$next_pair_pr_rx/($id,$label)=($1,$2);''/eo;

   local($ref_mark) = &get_ref_mark($label,$id);

   $ref_mark.$_;
}

$nodename='';

sub custom_title_hook{
   local($title) = @_;

   local($name) = $nodename;

   unless ($name)
   {
      $name = lc($title);
      $name=~s/[^\w]//g;

      $name=~s/^\d+//;
   }

   $nodename = '';

   $name
}

sub make_name {
    local($sec_name, $packed_curr_sec_id) = @_;
    local($title,$making_name,$saved) = ('',1,'');
    if ($LONG_TITLES) {
        $saved = $_;
        &set_sectioning_title($_) if /^$sections_rx/; # modified
        $title = &make_long_title($TITLE)
            unless ((! $TITLE) || ($TITLE eq $default_title));
        $_ = $saved;
    } elsif ($CUSTOM_TITLES) {
        $saved = $_;
        &set_sectioning_title($_) if /^$sections_rx/; # modified
        $title = &custom_title_hook($TITLE)
            unless ((! $TITLE) || ($TITLE eq $default_title));
        $_ = $saved;
    }
    if ($title) {
        #ensure no more than 32 characters, including .html extension
        $title =~ s/^(.{1,27}).*$/$1/;
        ++$OUT_NODE;
        join("", ${PREFIX}, $title, $EXTN);
    } else {
    # Remove 0's from the end of $packed_curr_sec_id
        $packed_curr_sec_id =~ s/(_0)*$//;
        $packed_curr_sec_id =~ s/^\d+$//o; # Top level file
        join("",($packed_curr_sec_id ?
            "${PREFIX}$NODE_NAME". ++$OUT_NODE : $sec_name), $EXTN);
    }
}

sub set_sectioning_title{
  local($_) = @_;

  if (/$section_rx/)
  {
    ($before, $cmd, $after) = ($`, $1.$2, $4.$');

    if (defined($unnumbered_section_commands{$cmd}))
    {
       &process_command($section_rx, $_[0]);
    }
    elsif ($cmd eq 'printkeywords')
    {
       $TITLE = "Some Definitions";
    }
    elsif ($cmd eq 'listofexercises')
    {
       $TITLE = "List of Exercises";
    }
    else
    {
       local($align, $dummy) = &get_next_optional_argument;

       $TITLE = &missing_braces
           unless $after=~s/$next_pair_rx/$TITLE=$2;''/eo;
    }

    $br_id = ++$global{'max_id'};
    $TITLE = &translate_environments("$O$br_id$C$TITLE$O$br_id$C");
    $TITLE = &simplify($TITLE);
  }
}


%sectiontitles = ();


sub do_cmd_sectionlabel{
    local($_) = @_;
    local($label, $title);

    $label = &missing_braces unless (
        (s/$next_pair_pr_rx/$label = $2;''/eo)
        ||(s/$next_pair_rx/$label = $2;''/eo));

    $title = &missing_braces unless (
        (s/$next_pair_pr_rx/$title = $2;''/eo)
        ||(s/$next_pair_rx/$title = $2;''/eo));

    $sectiontitles{$label}=$title;

    &anchor_label($label,$CURRENT_FILE,$_);
}

sub do_cmd_sectionref{
   local($_) = @_;

   local($text,$pat) = &get_next_optional_argument;

   local($label, $id);

   $label = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$label = $2;''/eo;

   local($id2) = ++$global{'max_id'};

   unless ($text)
   {
      local ($num) = $symbolic_labels{$label};

      $text = $sectiontitles{$label} ? 
              $sectiontitles{$label} :
              $label;

      $text=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

      if ($num)
      {
         $text = "\\S " . $num. ". " . $text;
      }
   }

   "\\htmlref$OP$id$CP$text$OP$id$CP$OP$id2$CP$label$OP$id2$CP".$_;
}

sub do_cmd_chapterref{
  &do_cmd_sectionref(@_);
}

sub do_cmd_appendixref{
   local($_) = @_;

   local($text,$pat) = &get_next_optional_argument;

   local($label, $id);

   $label = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$label = $2;''/eo;

   local($id2) = ++$global{'max_id'};

   unless ($text)
   {
      local ($num) = $symbolic_labels{$label};

      $text = $sectiontitles{$label} ? 
              $sectiontitles{$label} :
              $label;

      $text=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

      if ($num)
      {
         $text = "Appendix " . $num. ". " . $text;
      }
   }

   "\\htmlref$OP$id$CP$text$OP$id$CP$OP$id2$CP$label$OP$id2$CP".$_;
}

sub do_cmd_authordetails{
   local($_) = @_;

   local($dob, $surname, $forenames, $id);

   $dob = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$dob = $2;''/eo;

   $surname = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$surname = $2;''/eo;

   $forenames = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$forenames = $2;''/eo;

   $t_author = join(' ', $forenames, $surname);

   push @authors, $t_author;

   $_
}

sub do_cmd_affiliation{
   local($_) = @_;

   local($url, $address, $id);

   $t_affil = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$t_affil = $2;''/eo;

   $url = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$url = $2;''/eo;

   $address = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$address = $2;''/eo;

   $t_address = "<A HREF=\"$url\"><TT>$url</TT></A>";

   push @affils, $t_affil;
   push @addresses, $t_address;

   $_
}

sub do_cmd_version{
   local($_) = @_;

   local($theversion, $id);

   $theversion = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$theversion = $2;''/eo;

   $t_date = "Version $theversion";

   $_
}

my($t_series, $t_volume, $t_edition) = ();

sub do_cmd_edition{
   local($_) = @_;

   local($id);

   $t_edition = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$t_edition = $2;''/eo;

   $_
}

sub do_cmd_volume{
   local($_) = @_;

   local($id);

   $t_volume = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$t_volume = $2;''/eo;

   $_
}

sub do_cmd_series{
   local($_) = @_;

   local($id);

   $t_series = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$t_series = $2;''/eo;

   $_
}

sub do_cmd_seriesurl{
   local($_) = @_;

   local($id);

   $t_seriesurl = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$t_seriesurl = $2;''/eo;

   $_
}

$copyrighttext = '';

sub do_cmd_copyrighttext{
   local($_) = @_;

   $copyrighttext = &missing_braces unless
     s/$next_pair_pr_rx/$copyrighttext=$2;''/eo;

   $_;
}

eval(<<'_END_EVAL');
sub do_cmd_maketitle{
   local($after) = @_;
   local($the_title) = '';

   if ($t_title)
   {
     $the_title = "<H1 ALIGN=CENTER>$t_title</H1>\n";
   }
   else
   {
      &write_warnings("This document has no title.");
   }

   if (($#authors > 0)||$MULTIPLE_AUTHOR_TABLE)
   {
        $the_title .= &make_multipleauthors_title($alignc,$alignl);
   }
   else
   {
     $the_title .= &make_singleauthor_title($alignc,$alignl,$t_author,
       $t_affil,$t_institute,$t_date,$t_address,$t_email,$t_authorURL);
   }

   if (($t_translator)&&!($t_translator=~/^\s*(($O|$OP)\d+($C|$CP))\s*\1\s*$/))
   {
      $the_title .= "<BR><P ALIGN=CENTER>Translated by $t_translator</P>\n";
   }

   if (($t_version)&&!($t_version=~/^\s*(($O|$OP)\d+($C|$CP))\s*\1\s*$/))
   {
      $the_title .= "<BR><P ALIGN=CENTER>$t_version</P>\n";
   }

   if (($t_date)&&!($t_date=~/^\s*(($O|$OP)\d+($C|$CP))\s*\1\s*$/))
   {
      $the_title .= "<BR><P ALIGN=CENTER>$t_date</P>\n";
   }

   if ($t_volume && $t_series)
   {
      local($seriestext) = "<EM>$t_series</EM>";

      if ($t_seriesurl)
      {
         $seriestext = "<A HREF=\"$t_seriesurl\">$seriestext</A>";
      }

      $the_title .= "<BR><P ALIGN=CENTER>Volume $t_volume of $seriestext</P>\n";
   }

   if ($t_keywords)
   {
     $the_title .= "<BR><P><P ALIGN=LEFT><FONT SIZE=-1>".
            "Key words and phrases: $t_keywords</FONT></P>\n";
   }

    join("\n", $the_title, '<P>',
        $after);
}

sub make_real_glossary_entry {
    local($entry,$text,$type) = @_;
    local($this_file) = $CURRENT_FILE;

    $TITLE = $saved_title
       if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

    # Save the reference
    local($str) = "$label###" . ++$global{'max_id'}; # Make unique

    local($id) = ++$glsentry{$entry}{'maxid'};
    local($glsanchor)="gls:$entry$id";

    local($target) = $frame_body_name;

    if (defined($frame_main_suffix))
    {
       $this_file=~s/$frame_main_suffix/$frame_body_suffix/;
    }

    $glossary{$type}{$str} .= &make_half_href($this_file."#$glsanchor");
    $glossary_format{$type}{$str} = $GLSCURRENTFORMAT;
    $glossary_entry{$type}{$str} = $entry;
    $glossary_linktext{$type}{$str} = $TITLE;

    local($mark) = $gls_file_mark{$type};

    $mark = &get_gls_file_mark($type, $entry) if (defined(&get_gls_file_mark));

    $text = &translate_commands($text);

    if (defined($frame_foot_name))
    {
       "<A CLASS=\"gls\" HREF=\"$mark#gls:$entry\" NAME=\"$glsanchor\" TARGET=\"$frame_foot_name\">$text<\/A>";
    }
    else
    {
       "<A CLASS=\"gls\" HREF=\"$mark#gls:$entry\" NAME=\"$glsanchor\">$text<\/A>";
    }
}

sub make_real_glossary_entry_no_backlink {
    local($entry,$text,$type) = @_;
    local($this_file) = $CURRENT_FILE;

    local($target) = $frame_body_name;

    if (defined($frame_main_suffix))
    {
       $this_file=~s/$frame_main_suffix/$frame_body_suffix/;
    }

    local($mark) = $gls_file_mark{$type};

    $mark = &get_gls_file_mark($type, $entry) if (defined(&get_gls_file_mark));

    $text = &translate_commands($text);

    if (defined($frame_foot_name))
    {
       "<A CLASS=\"gls\" HREF=\"$mark#gls:$entry\" NAME=\"$glsanchor\" TARGET=\"$frame_foot_name\">$text<\/A>";
    }
    else
    {
       "<A CLASS=\"gls\" HREF=\"$mark#gls:$entry\" >$text<\/A>";
    }
}

sub do_cmd_glshyperlink{
   local($_) = @_;

   local($text,$pat) = &get_next_optional_argument;

   local($id, $label);

   $label = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $text = &gls_get_text($label) unless $text;

   local($type) = &gls_get_type($label);

   "<A CLASS=\"gls\" HREF=\"$gls_file_mark{$type}#gls:$label\">$text<\/A>"
   .$_;
}

_END_EVAL

sub do_cmd_boldc{
  join('', '<B><EM>c</EM></B>', @_[0]);
}

sub do_cmd_dag{
  join('', '&#134;', @_[0]);
}

sub do_cmd_ddag{
  join('', '&#135;', @_[0]);
}

sub do_cmd_supdag{
  join('', '<SUP>&#134;</SUP>', @_[0]);
}

sub do_cmd_supddag{
  join('', '<SUP>&#135;</SUP>', @_[0]);
}

sub do_cmd_supdagddag{
  join('', '<SUP>&#134;&#135;</SUP>', @_[0]);
}

# Helps with the sort if symbols are indexed as the HTML numeric
# entities

sub do_cmd_backslashsym{

  join('', '&#092;', @_[0]);
}

sub do_cmd_dbbackslashsym{
  join('', '&#092;&#092;', @_[0]);
}

sub do_cmd_emDashcs{
  join('', '&#045;&#045;&#045;', @_[0]);
}

sub do_cmd_enDashcs{
  join('', '&#045;&#045;', @_[0]);
}

sub do_cmd_dashcs{
  join('', '&#045;', @_[0]);
}

sub do_cmd_quoteleftcs{
  join('', '&#096;', @_[0]);
}

sub do_cmd_quotedblleftcs{
  join('', '&#096;&#096;', @_[0]);
}

sub do_cmd_quoterightcs{
  join('', '&#039;', @_[0]);
}

sub do_cmd_quotedblrightcs{
  join('', '&#039;&#039;', @_[0]);
}

sub do_cmd_questiondowncs{
  join('', '&#063;&#096;', @_[0]);
}

sub do_cmd_exclamdowncs{
  join('', '&#033;&#096;', @_[0]);
}

sub do_cmd_exclamsym{
  join('', '&#033;', @_[0]);
}

sub do_cmd_slash{
  join('', '&#047;', @_[0]);
}

sub do_cmd_slashsym{
  join('', '&#047;', @_[0]);
}

sub do_cmd_openparensym{
  join('', '&#040;', @_[0]);
}

sub do_cmd_closeparensym{
  join('', '&#041;', @_[0]);
}

sub do_cmd_opensqsym{
  join('', '&#091;', @_[0]);
}

sub do_cmd_closesqsym{
  join('', '&#093;', @_[0]);
}

sub do_cmd_commasym{
  join('', '&#044;', @_[0]);
}

sub do_cmd_colonsym{
  join('', '&#058;', @_[0]);
}

sub do_cmd_semicolonsym{
  join('', '&#059;', @_[0]);
}

sub do_cmd_lesssym{
  join('', '&#060;', @_[0]);
}

sub do_cmd_greatersym{
  join('', '&#062;', @_[0]);
}

sub do_cmd_periodsym{
  join('', '&#046;', @_[0]);
}

sub do_cmd_equalsym{
  join('', '&#061;', @_[0]);
}

sub do_cmd_underscoresym{
  join('', '&#095;', @_[0]);
}

sub do_cmd_textunderscore{
  join('', '&#095;', @_[0]);
}

sub do_cmd_tildesym{
  join('', '&#126;', @_[0]);
}

sub do_cmd_circumsym{
  join('', '&#136;', @_[0]);
}

sub do_cmd_quotecs{
  join('', '&#092;&#034;', @_[0]);
}

sub do_cmd_doublequotesym{
  join('', '&#034;', @_[0]);
}

sub do_cmd_ocaron{
  join('', '&#x01D2;', @_[0]);
}

sub do_cmd_ocedilla{
  join('', '&#x01EB;', @_[0]);
}

sub do_cmd_odotunder{
  join('', '&#x1ECD;', @_[0]);
}

sub do_cmd_omacron{
  join('', '&#x014D;', @_[0]);
}

sub do_cmd_odoubleacute{
  join('', '&#x0150;', @_[0]);
}

sub do_cmd_odotover{
  join('', '&#x022F;', @_[0]);
}

sub do_cmd_oumlaut{
  join('', '&#x00F6;', @_[0]);
}

sub do_cmd_otilde{
  join('', '&#x00F5;', @_[0]);
}

sub do_cmd_ocircum{
  join('', '&#x00F4;', @_[0]);
}

sub do_cmd_obreve{
  join('', '&#x014F;', @_[0]);
}

sub do_cmd_uring{
  join('', '&#x016F;', @_[0]);
}

sub do_cmd_rbarunder{
  join('', '&#x1E5F;', @_[0]);
}

sub do_cmd_xytie{
  join('', 'x&#x0361;y', @_[0]);
}

sub do_cmd_dotlessj{ "&#x0237;".$_[0]; }

sub do_cmd_textvisiblespace{
  join('', '&#x2423;', @_[0]);
}

sub do_cmd_preambleillustration{
<<_END_TEXT;
<PRE>
&#092;documentclass{...}

                    &#x2B05; <EM>This bit here is the preamble</EM>

&#092;begin{document}
</PRE>
   $_[0]
_END_TEXT
}

#sub do_cmd_doexerciseexample{
   #join('',
        #'<DIV CLASS="result">',
        #'<IMG ',
        #' ALT="Image: similar to the original but with a paragraph indent at the start of each line." ',
        #'SRC="pictures/exsamp.png"></DIV>', $_[0]);
#}

sub do_cmd_xminisec{
   local($_) = @_;

   local($id,$title);

   $title = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$title = $2;''/eo;

   "<P><B>$title</B><P>$_";
}

sub do_cmd_refstar{
  local($_) = @_;
  local($label, $id);

  $label = &missing_braces unless
    s/$next_pair_pr_rx/($id,$label);''/eo;

  local($ref_mark) = &get_ref_mark($label, $id);

  $ref_mark.$_;
}

sub meta_information{
  local($_) = @_;

  local($keywords) = (defined($t_keywords) ? $t_keywords : $FILE);

  # Can't have nested HTML tags...

  do
  {
    s/<[^>]*>//g;
    "<META NAME=\"description\" CONTENT=\"$_\">\n" .
    "<META NAME=\"keywords\" CONTENT=\"$keywords\">\n" .
    "<META NAME=\"resource-type\" CONTENT=\"document\">\n" .
    "<META NAME=\"distribution\" CONTENT=\"global\">\n"
  }
  if $_;
}

sub do_cmd_historyitem{
   local($_) = @_;

   local($text);

   $text = &missing_braces unless 
    s/$next_pair_pr_rx/$text=$2;''/eo;

   "<H2>$text</H2>$_";
}

sub do_cmd_setnode{
  local($_) = @_;

  local($id);

  $nodename = &missing_braces unless
    s/$next_pair_pr_rx/$id=$1,$nodename=$2;''/eo;

  $_;
}

sub do_cmd_keywordfmt{
   local($_) = @_;

   local($text);

   $text = &missing_braces unless
     s/$next_pair_pr_rx/$text=$2;''/eo;

   "<EM>$text</EM>".$_;
}

eval(<<'_END_EVAL');
sub do_cmd_glsdisplay{
   local($_) = @_;
   local($text,$description,$symbol,$insert);

   $text = &missing_braces unless
        (s/$next_pair_pr_rx/$text=$2;''/eo);

   $description = &missing_braces unless
        (s/$next_pair_pr_rx/$description=$2;''/eo);

   $symbol = &missing_braces unless
        (s/$next_pair_pr_rx/$symbol=$2;''/eo);

   $insert = &missing_braces unless
        (s/$next_pair_pr_rx/$insert=$2;''/eo);

   $text = "$text$insert";

   $text = &translate_commands($text);

   $text =~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

   $text = &translate_commands($text);

   $text . $_;
}

sub do_cmd_glsdisplayfirst{
   local($_) = @_;
   local($text,$description,$symbol,$insert);

   $text = &missing_braces unless
        (s/$next_pair_pr_rx/$text=$2;''/eo);

   $description = &missing_braces unless
        (s/$next_pair_pr_rx/$description=$2;''/eo);

   $symbol = &missing_braces unless
        (s/$next_pair_pr_rx/$symbol=$2;''/eo);

   $insert = &missing_braces unless
        (s/$next_pair_pr_rx/$insert=$2;''/eo);

   $text = "$text$insert";

   $text = &translate_commands($text);

   $text =~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

   $text = &translate_commands($text);

   $text . $_;
}
_END_EVAL

sub do_cmd_glsdkey{ "user1".$_[0]; }
sub do_cmd_glsingkey{ "user2".$_[0]; }
sub do_cmd_glstockey{ "user3".$_[0]; }
sub do_cmd_glstermkey{ "user4".$_[0]; }
sub do_cmd_glsindexkey{ "user5".$_[0]; }
sub do_cmd_glsextrakey{ "user6".$_[0]; }

sub do_cmd_glstoc{
   "<tex2html_glstoc_deferred>".$_[0];
}

sub do_cmd_deferredglstoc{
   &do_cmd_glsentryuseriii(@_);
}

sub do_cmd_glsindex{
   local($_) = @_;

   local($id, $label);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$label=$2;''/eo;

   my ($text) = &gls_get_user($label, 5);

   join('', $text, $_);
}

sub do_cmd_indexglsindex{
  "<tex2html_indexglsindex_deferred>".$_[0];
}

sub do_cmd_deferredindexglsindex{
   local($_) = @_;

   local($id, $label);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$label=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   my ($idxtext) = &gls_get_user($label, 5);

   $id = ++$global{'max_id'};

   join('',
        &named_index_entry($id, $idxtext),
        $_);
}

sub do_cmd_glsextra{
  &do_cmd_glsentryuservi(@_);
}

sub do_cmd_glsterm{
  &do_cmd_glsuseriv(@_);
}

sub do_cmd_glsd{
   local($_) = @_;

   local($id, $label);

   local($keyval,$pat) = &get_next_optional_argument;

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$label=$2;''/eo;

   local($text) = '';

   if (&gls_used($label))
   {
      local($this_file) = $CURRENT_FILE;

      $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

      my ($idxtext) = &gls_get_user($label, 5);

      $id = ++$global{'max_id'};

      $text = join('',
                &do_cmd_glsuseri("[$keyval]$OP$id$CP$label$OP$id$CP"),
                &named_index_entry($id, $idxtext)
                );
   }
   else
   {
      $text = join('', "<EM>", &gls_get_user($label, 1), "</EM>");
      &unset_entry($label);
   }

   $text.$_;
}

sub do_cmd_glsing{
   local($_) = @_;

   local($id, $label);

   local($keyval,$pat) = &get_next_optional_argument;

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$label=$2;''/eo;

   local($text) = '';

   if (&gls_used($label))
   {
      local($this_file) = $CURRENT_FILE;

      $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

      my ($idxtext) = &gls_get_user($label, 5);

      $id = ++$global{'max_id'};

      $text = join('',
                &do_cmd_glsuserii("[$keyval]$OP$id$CP$label$OP$id$CP"),
                &named_index_entry($id, $idxtext)
             );
   }
   else
   {
      $text = join('', "<EM>", &gls_get_user($label, 2), "</EM>");
      &unset_entry($label);
   }

   $text.$_;
}

sub do_cmd_kglossaryentryfield{
   local($_) = @_;

   local($id, $label, $name, $desc, $symbol, $backlink);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$label=$2;''/eo;

   $name = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$name=$2;''/eo;

   $desc = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$desc=$2;''/eo;

   $symbol = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$symbol=$2;''/eo;

   $backlink = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$backlink=$2;''/eo;

   local($idx) = &translate_commands("\\glsindex $OP$id$CP$label$OP$id$CP");

   if ($idx)
   {
      $id = ++$global{'max_id'};

      $idx = &named_index_entry($id, $idx);
   }

   local($userkey) = &do_cmd_glsextrakey('');

   local($extra) = $glsentry{$label}{$userkey};

   $extra=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

   join('', $idx, $extra, $desc, "\n", $_);
}

sub do_cmd_kglossarysubentryfield{
   local($_) = @_;

   local($id, $level, $label, $name, $desc, $symbol, $backlink);

   $level = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$level=$2;''/eo;

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$label=$2;''/eo;

   $name = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$name=$2;''/eo;

   $desc = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$desc=$2;''/eo;

   $symbol = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$symbol=$2;''/eo;

   $backlink = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1;$backlink=$2;''/eo;

   local($idx) = &translate_commands("\\glsindex $OP$id$CP$label$OP$id$CP");

   if ($idx)
   {
      $id = ++$global{'max_id'};

      $idx = &named_index_entry($id, $idx);
   }

   my $key = &do_cmd_glsextrakey('');

   local($extra) = $glsentry{$label}{$key};

   $extra=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

   join('', $idx, $extra, $desc, "\n", $_);
}

sub get_gls_file_mark{
   local($type, $entry) = @_;

   ($type eq 'keywords' ? "$entry.html" : $gls_file_mark{$type});
}

sub do_cmd_newacr{
   local($_) = @_;

   local($label, $abbrv, $long, $url, $id);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $abbrv = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$abbrv=$2;''/eo;

   $long = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$long=$2;''/eo;

   $url = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$url=$2;''/eo;

   local($name) = "$abbrv<tex2html_gloindex_deferred> $OP$id$CP$abbrv$OP$id$CP";

   local($desc) = &do_real_makefirstuc($long).". <A HREF=\"$url\"><TT>$url</TT></A>.";

   local($first) = "$long ($abbrv)\\deferredindex $OP$id$CP$abbrv$OP$id$CP";
   local($firstplural) = "${long}s (${abbrv}s)\\deferredindex $OP$id$CP$abbrv$OP$id$CP";

   local($text) = "$abbrv\\deferredindex $OP$id$CP$abbrv$OP$id$CP";
   local($plural) = "${abbrv}s\\deferredindex $OP$id$CP$abbrv$OP$id$CP";

   &gls_entry_init($label, $acronymtype, $name, $desc);

   &gls_set_first($label, $first);
   &gls_set_firstplural($label, $firstplural);

   &gls_set_text($label, $text);
   &gls_set_plural($label, $plural);

   push @{$gls_entries{$acronymtype}}, $label;

   $_;
}

sub do_cmd_newacrx{
   local($_) = @_;

   local($label, $abbrv, $long, $id);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $abbrv = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$abbrv=$2;''/eo;

   $long = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$long=$2;''/eo;

   local($name) = "$abbrv<tex2html_gloindex_deferred> $OP$id$CP$abbrv$OP$id$CP";

   local($desc) = &do_real_makefirstuc($long);

   local($first) = "$long ($abbrv)\\deferredindex $OP$id$CP$abbrv$OP$id$CP";
   local($firstplural) = "${long}s (${abbrv}s)\\deferredindex $OP$id$CP$abbrv$OP$id$CP";

   local($text) = "$abbrv\\deferredindex $OP$id$CP$abbrv$OP$id$CP";
   local($plural) = "${abbrv}s\\deferredindex $OP$id$CP$abbrv$OP$id$CP";

   &gls_entry_init($label, $acronymtype, $name, $desc);

   &gls_set_first($label, $first);
   &gls_set_firstplural($label, $firstplural);

   &gls_set_text($label, $text);
   &gls_set_plural($label, $plural);

   push @{$gls_entries{$acronymtype}}, $label;

   $_;
}

sub do_cmd_newacrnocite{
   local($_) = @_;

   local($label, $abbrv, $long, $id);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $abbrv = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$abbrv=$2;''/eo;

   $long = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$long=$2;''/eo;

   local($name) = "$abbrv<tex2html_gloindex_deferred> $OP$id$CP$abbrv$OP$id$CP";

   local($desc) = &do_real_makefirstuc($long);

   local($first) = "$long ($abbrv)\\deferredindex $OP$id$CP$abbrv$OP$id$CP";
   local($firstplural) = "${long}s (${abbrv}s)\\deferredindex $OP$id$CP$abbrv$OP$id$CP";

   local($text) = "$abbrv\\deferredindex $OP$id$CP$abbrv$OP$id$CP";
   local($plural) = "${abbrv}s\\deferredindex $OP$id$CP$abbrv$OP$id$CP";

   &gls_entry_init($label, $acronymtype, $name, $desc);

   &gls_set_first($label, $first);
   &gls_set_firstplural($label, $firstplural);

   &gls_set_text($label, $text);
   &gls_set_plural($label, $plural);

   push @{$gls_entries{$acronymtype}}, $label;

   $_;
}

sub do_cmd_newkeyword{
   local($_) = @_;

   local($label,$name,$description,$symbol,$sort,$text,$first,
     $firstplural,$keyval,$parent,@user);

   local($idx) = ++$global{'keyword'};

   local($id,$id2,$label,$name);

   local($keyval, $plural,$pat);

   ($keyval,$pat) = &get_next_optional_argument;

   $keyval = &translate_commands($keyval);

   ($name,$keyval) = &get_keyval('name', $keyval);
   ($text,$keyval) = &get_keyval('text', $keyval);
   ($first,$keyval) = &get_keyval('first', $keyval);
   ($firstplural,$keyval) = &get_keyval('firstplural', $keyval);
   ($plural,$keyval) = &get_keyval('plural', $keyval);
   ($parent,$keyval) = &get_keyval('parent', $keyval);

   unless ($plural)
   {
      ($plural,$pat) = &get_next_optional_argument;
   }

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   if ($text)
   {
      unless ($name)
      {
         $name = &missing_braces unless
           s/$next_pair_pr_rx/$id2=$1,$name=$2;''/eo;
      }
   }
   else
   {
      $text = &missing_braces unless
        s/$next_pair_pr_rx/$id2=$1,$text=$2;''/eo;
   }

   unless ($name)
   {
      $name = $text;
   }

   &gls_entry_init($label, $KEYWORDTYPE,
    "\\capitalisewords $OP$id$CP$name$OP$id$CP", '');

   @user = ();

   for (my $idx = 0; $idx < 6; $idx++)
   {
      ($user[$idx],$keyval) = &get_keyval("user".($idx+1), $keyval);
   }

   &gls_set_userkeys($label, @user);

   unless ($plural)
   {
      $plural = $text.'s';
   }

   unless ($first)
   {
      $first = "<EM>$text</EM>";
   }

   unless ($firstplural)
   {
      $firstplural = "<EM>${plural}</EM>";
   }

   &gls_set_first($label, $first);
   &gls_set_firstplural($label, $firstplural);

   $id = ++$global{'max_id'};

   &gls_set_text($label, "$text\\deferredindexglsindex $OP$id$CP$label$OP$id$CP");

   $id = ++$global{'max_id'};
   $plural = "$plural\\deferredindexglsindex $OP$id$CP$label$OP$id$CP";

   &gls_set_plural($label, $plural);

   my $key = &do_cmd_glsindexkey('');
   $glsentry{$label}{$key} = $text unless ($glsentry{$label}{$key});

   $key = &do_cmd_glsdkey('');
   $glsentry{$label}{$key} = "${text}ed" unless ($glsentry{$label}{$key});

   $key = &do_cmd_glsingkey('');
   $glsentry{$label}{$key} = "${text}ing" unless ($glsentry{$label}{$key});

   $key = &do_cmd_glstockey('');
   $glsentry{$label}{$key} = $name unless ($glsentry{$label}{$key});

   $key = &do_cmd_glstermkey('');
   $glsentry{$label}{$key} = $text unless ($glsentry{$label}{$key});

   push @{$gls_entries{$KEYWORDTYPE}}, $label;

   $_;
}

sub do_cmd_defgcs{
   local($_) = @_;

   local($extra,$label,$pat);
   local($id,$id2,$id3,$id4,$id5,$name,$syntax,$location,$summary,$argsummary);

   ($extra,$pat) = &get_next_optional_argument;
   ($label,$pat) = &get_next_optional_argument;

   $name = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$name=$2;''/eo;

   $label = $name unless ($label);

   $syntax = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$syntax=$2;''/eo;

   $location = &missing_braces unless
     s/$next_pair_pr_rx/$id3=$1,$location=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id4=$1,$summary=$2;''/eo;

   $argsummary = &missing_braces unless
     s/$next_pair_pr_rx/$id5=$1,$argsummary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE,
     "\\cmdname $OP$id$CP$name$OP$id$CP<tex2html_glocsindex_deferred>$OP$id2$CP$name$OP$id2$CP",
      $summary);

   &gls_set_sort($label, $name);

   &gls_set_first($label, "\\cmddef $OP$id3$CP$name$OP$id3$CP");

   &gls_set_text($label, "\\icmdname $OP$id4$CP$name$OP$id4$CP");

   &gls_set_userkeys($label, $syntax, $location, $argsummary,
     "\\cmdname $OP$id5$CP$name$OP$id5$CP", $extra, '');

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgenv{
   local($_) = @_;


   local($id, $id2, $id3, $id4, $id5, $name,$syntax,$location,$summary,$argsummary);

   local($label,$pat) = &get_next_optional_argument;

   $name = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$name=$2;''/eo;

   $label = "env-$name" unless ($label);

   $syntax = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$syntax=$2;''/eo;

   $location = &missing_braces unless
     s/$next_pair_pr_rx/$id3=$1,$location=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id4=$1,$summary=$2;''/eo;

   $argsummary = &missing_braces unless
     s/$next_pair_pr_rx/$id5=$1,$argsummary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE,
     "<TT>&#092;begin&#123;$name&#125;</TT><tex2html_gloindex_deferred>$OP$id2$CP$name environment@<TT>$name</TT> environment$OP$id2$CP",
     $summary);

   &gls_set_sort($label, $name);

   &gls_set_first($label, "\\envdef $OP$id3$CP$name$OP$id3$CP");

   &gls_set_text($label, "\\ienvname $OP$id4$CP$name$OP$id4$CP");

   &gls_set_userkeys($label, $syntax, $location, $argsummary,
     "\\envname $OP$id5$CP$name$OP$id5$CP", '', '');

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgidxactivechar{
   local($_) = @_;


   my($sort,$pat) = &get_next_optional_argument;

   my($id,$id2,$label,$char,$syntax,$location,$summary,$argsummary);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$char=$2;''/eo;

   $sort = $char unless $sort;

   $syntax = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$syntax=$2;''/eo;

   $location = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$location=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$summary=$2;''/eo;

   $argsummary = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$argsummary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE, 
     "<TT>$char</TT><tex2html_gloindex_deferred>$OP$id$CP$char$OP$id$CP",
      $summary);

   &gls_set_sort($label, $sort);

   &gls_set_first($label, "<TT>$char</TT>\\indexttdef $OP$id$CP$char$OP$id$CP");

   &gls_set_text($label, "<TT>$char</TT>\\indextt $OP$id2$CP$char$OP$id2$CP");

   &gls_set_userkeys($label, $syntax, $location, $argsummary,
     "<TT>$char</TT>", '', '');

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgchildidxactivechar{
   local($_) = @_;


   local($id,$id2,$label,$parent,$char,$summary);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $parent = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$parent=$2;''/eo;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$char=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$summary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE, 
     "<TT>$char</TT><tex2html_gloindex_deferred>$OP$id$CP$char$OP$id$CP", 
     $summary);

   local($sort) = join(' ', &gls_get_sort($parent),
                       (&gls_get_childcount($parent)+1));

   &gls_set_sort($label, $sort);

   &gls_set_first($label, "<TT>$char</TT>\\indexttdef $OP$id$CP$char$OP$id$CP");

   &gls_set_text($label, "<TT>$char</TT>\\indextt $OP$id2$CP$char$OP$id2$CP");

   &gls_set_userkeys($label, '', '', '', "<TT>$char</TT>", '', '');

   &gls_set_parent($label, $parent);

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgidxactivecharcs{
   local($_) = @_;


   local($id,$id2,$label,$char,$syntax,$location,$summary,$argsummary);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$char=$2;''/eo;

   local($sort) = "&#092;$char";

   $syntax = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$syntax=$2;''/eo;

   $location = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$location=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$summary=$2;''/eo;

   $argsummary = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$argsummary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE,
    "<TT>&#092;$char</TT><tex2html_gloindex_deferred>$OP$id$CP&#092;$char$OP$id$CP",
    $summary);

   &gls_set_first($label, "<TT>&#092;$char</TT>\\indexttdef $OP$id$CP&#092;$char$OP$id$CP");

   &gls_set_text($label, "<TT>&#092;$char</TT>\\indextt $OP$id2$CP&#092;$char$OP$id2$CP");

   &gls_set_sort($label, $sort);

   &gls_set_userkeys($label, $syntax, $location, $argsummary,
     "<TT>&#092;$char</TT>", '', '');

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgchar{
   local($_) = @_;


   local($id,$id2,$label,$char,$syntax,$location,$summary,$argsummary);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$char=$2;''/eo;

   local($sort) = $char;

   $syntax = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$syntax=$2;''/eo;

   $location = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$location=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$summary=$2;''/eo;

   $argsummary = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$argsummary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE, 
      "<TT>$char</TT><tex2html_gloindex_deferred>$OP$id$CP$char$OP$id$CP",
      $summary);

   &gls_set_sort($label, $sort);

   &gls_set_first($label, "<TT>$char</TT>\\indexttdef $OP$id$CP$char$OP$id$CP");

   &gls_set_text($label, "<TT>$char</TT>\\indextt $OP$id2$CP$char$OP$id2$CP");

   &gls_set_userkeys($label, $syntax, $location, $argsummary,
     "<TT>$char</TT>", '', '');

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgchildchar{
   local($_) = @_;


   local($id,$id2,$label,$parent,$char,$summary);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $parent = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$parent=$2;''/eo;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$char=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$summary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE,
     "<TT>$char</TT><tex2html_gloindex_deferred>$OP$id$CP$char$OP$id$CP",
     $summary);

   local($sort) = join(' ', &gls_get_sort($parent),
                       (&gls_get_childcount($parent)+1));

   &gls_set_sort($label, $sort);

   &gls_set_first($label, "<TT>$char</TT>\\indexttdef $OP$id$CP$char$OP$id$CP");

   &gls_set_text($label, "<TT>$char</TT>\\indextt $OP$id2$CP$char$OP$id2$CP");

   &gls_set_parent($label, $parent);

   &gls_set_userkeys($label, '', '', '', "<TT>$char</TT>", '', '');

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgxchildchar{
   local($_) = @_;


   local($id,$id2,$label,$parent,$char,$summary,$extra);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $parent = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$parent=$2;''/eo;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$char=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$summary=$2;''/eo;

   $extra = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$extra=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE,
      "<TT>$char</TT><tex2html_gloindex_deferred>$OP$id$CP$char$OP$id$CP",
      $summary);

   local($sort) = join(' ', &gls_get_sort($parent),
                       (&gls_get_childcount($parent)+1));

   &gls_set_sort($label, $sort);

   $id = ++$global{'max_id'};

   local($first) = "<TT>$char</TT>\\indexttdef $OP$id$CP$char$OP$id$CP";

   if ($extra)
   {
      $id2 = ++$global{'max_id'};
      $first .= "\\index $OP$id2$CP$extra$OP$id2$CP";
   }

   &gls_set_first($label, $first);

   $id = ++$global{'max_id'};

   local($text) = "<TT>$char</TT>\\indextt $OP$id2$CP$char$OP$id2$CP";

   if ($extra)
   {
      $id2 = ++$global{'max_id'};
      $text .= "\\index $OP$id2$CP$extra$OP$id2$CP";
   }

   &gls_set_text($label, $text);

   &gls_set_userkeys($label, '', '', '', "<TT>$char</TT>", '', '');

   &gls_set_parent($label, $parent);

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgactivechar{
   local($_) = @_;


   local($id,$id2,$label,$char,$syntax,$location,$summary,$argsummary);

   local($sort,$pat) = &get_next_optional_argument;

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$char=$2;''/eo;

   $sort = $char unless ($sort);

   $sort = &translate_commands($sort);

   $syntax = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$syntax=$2;''/eo;

   $location = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$location=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$summary=$2;''/eo;

   $argsummary = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$argsummary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE, 
     "<TT>$char</TT><tex2html_gloindex_deferred>$OP$id$CP$char$OP$id$CP",
      $summary);

   &gls_set_sort($label, $sort);

   &gls_set_first($label, "<TT>$char</TT>\\indexttdef $OP$id$CP$char$OP$id$CP");

   &gls_set_text($label, "<TT>$char</TT>\\indextt $OP$id$CP$char$OP$id$CP");

   &gls_set_userkeys($label, $syntax, $location, $argsummary,
     "<TT>$char</TT>", '', '');

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgchildactivechar{
   local($_) = @_;


   local($id,$id2,$id3,$id4,$label,$parent,$char,$summary);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $parent = &missing_braces unless
     s/$next_pair_pr_rx/$id2=$1,$parent=$2;''/eo;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id3=$1,$char=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id4=$1,$summary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE,
      "<TT>$char</TT><tex2html_gloindex_deferred>$OP$id$CP$char$OP$id$CP",
      $summary);

   local($sort) = join(' ', &gls_get_sort($parent),
                       (&gls_get_childcount($parent)+1));

   &gls_set_sort($label, $sort);

   &gls_set_first($label,
     "<TT>$char</TT>\\indexttdef $OP$id$CP$char$OP$id$CP");

   &gls_set_text($label,
     "<TT>$char</TT>\\indextt $OP$id$CP$char$OP$id$CP");

   &gls_set_user($label, 4, "<TT>$char</TT>");

   &gls_set_parent($label, $parent);

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgactivecharcs{
   local($_) = @_;


   local($id,$label,$char,$syntax,$location,$summary,$argsummary);

   local($sort,$pat) = &get_next_optional_argument;

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$char=$2;''/eo;

   $sort = $char unless ($sort);

   $sort = &translate_commands($sort);

   $syntax = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$syntax=$2;''/eo;

   $location = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$location=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$summary=$2;''/eo;

   $argsummary = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$argsummary=$2;''/eo;

   &gls_entry_init($label, $SUMMARYTYPE,
     "<TT>&#092;$char</TT><tex2html_gloindex_deferred>$OP$id$CP&#092;$char$OP$id$CP",
      $summary);

   &gls_set_sort($label, "&#092;$sort");

   &gls_set_first($label, "<TT>&#092;$char</TT>\\indexttdef $OP$id$CP&#092;$char$OP$id$CP");

   &gls_set_text($label, "<TT>&#092;$char</TT>\\indextt $OP$id$CP&#092;$char$OP$id$CP");

   &gls_set_userkeys($label, $syntax, $location, $argsummary,
     "<TT>&#092;$char</TT>", '', '');

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_defgsymcs{
   local($_) = @_;

   local($id,$char,$syntax,$location,$summary,$argsummary);

   local($label,$pat) = &get_next_optional_argument;

   $char = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$char=$2;''/eo;

   local($label) = $char unless ($label);

   local($sort) = "&#092;$char";

   $syntax = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$syntax=$2;''/eo;

   $location = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$location=$2;''/eo;

   $summary = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$summary=$2;''/eo;

   $argsummary = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$argsummary=$2;''/eo;

   $id = ++$global{'max_id'};

   &gls_entry_init($label, $SUMMARYTYPE,
     "<TT>&#092;$char</TT><tex2html_gloindex_deferred>$OP$id$CP&#092;$char$OP$id$CP",
      $summary);

   &gls_set_sort($label, $sort);

   $id = ++$global{'max_id'};

   &gls_set_first($label, "<TT>&#092;$char</TT>\\indexttdef $OP$id$CP$char$OP$id$CP");

   $id = ++$global{'max_id'};

   &gls_set_text($label, "<TT>&#092;$char</TT>\\indextt $OP$id$CP$char$OP$id$CP");

   &gls_set_userkeys($label, $syntax, $location, $argsummary,
     "<TT>&#092;$char</TT>", '', '');

   push @{$gls_entries{$SUMMARYTYPE}}, $label;

   $_;
}

sub do_cmd_BeginArgList{
  local($_) = @_;

  "<UL>$_";
}

sub do_cmd_EndArgList{
  local($_) = @_;

  "</UL>$_";
}

sub do_cmd_csentryargitem{
  "<LI>" . &do_cmd_meta(@_);
}

sub do_cmd_nxipagestyle{
   local($_) = @_;

   "<tex2html_ipagestyle_deferred>$_";
}

sub do_cmd_nxgls{
   local($_) = @_;

   "<tex2html_gls_deferred>$_";
}

sub do_cmd_nxGls{
   local($_) = @_;

   "<tex2html_Gls_deferred>$_";
}

sub do_cmd_nxglsi{
   local($_) = @_;

   "<tex2html_glsi_deferred>$_";
}

sub do_cmd_nxglsni{
   local($_) = @_;

   "<tex2html_glsni_deferred>$_";
}

sub do_cmd_nxglslink{
   local($_) = @_;

   "<tex2html_glslink_deferred>$_";
}

sub do_cmd_nxGlsi{
   local($_) = @_;

   "<tex2html_Glsi_deferred>$_";
}

sub do_cmd_nxGlsni{
   local($_) = @_;

   "<tex2html_Glsni_deferred>$_";
}

sub do_cmd_nxisty{
   local($_) = @_;

   "<tex2html_isty_deferred>$_";
}

sub do_cmd_nxicls{
   local($_) = @_;

   "<tex2html_icls_deferred>$_";
}

sub do_cmd_nxiappname{
   local($_) = @_;

   "<tex2html_iappname_deferred>$_";
}

sub do_cmd_nxicounter{
   local($_) = @_;

   "<tex2html_icounter_deferred>$_";
}

sub do_cmd_nxikeyvalopt{
   local($_) = @_;

   "<tex2html_ikeyvalopt_deferred>$_";
}

sub do_cmd_deferredipagestyle{
   &do_cmd_ipagestyle(@_);
}

sub do_cmd_deferredisty{
   &do_cmd_isty(@_);
}

sub do_cmd_deferredicls{
   &do_cmd_icls(@_);
}

sub do_cmd_deferrediappname{
   &do_cmd_iappname(@_);
}

sub do_cmd_deferredicounter{
   local($_) = @_;

   local($id, $name);

   $name = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$name=$2;''/eo;

   "<TT>$name</TT>$_";
}

sub do_cmd_deferredgls{
  &do_cmd_gls(@_);
}

sub do_cmd_deferredGls{
  &do_cmd_Gls(@_);
}

sub do_cmd_deferredglsi{
   local($_) = @_;

   local($id, $label);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   local($text) = &translate_commands(&gls_get_text($label));

   "<A CLASS=\"gls\" HREF=\"$gls_file_mark{$SUMMARYTYPE}#gls:$label\">$text</A>$_";
}

sub do_cmd_glsi{
   local($_) = @_;

   local($id, $label);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   local($text) = &translate_commands(&gls_get_text($label));

   "<A CLASS=\"gls\" HREF=\"$gls_file_mark{$SUMMARYTYPE}#gls:$label\">$text</A>$_";
}

sub do_cmd_deferredglsni{
   local($_) = @_;

   local($id, $label);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   local($text) = &translate_commands(&gls_get_user($label, 4));

   "<A CLASS=\"gls\" HREF=\"$gls_file_mark{$SUMMARYTYPE}#gls:$label\">$text</A>$_";
}

sub do_cmd_glsni{
   local($_) = @_;

   local($id, $label);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   local($text) = &translate_commands(&gls_get_user($label, 4));

   "<A CLASS=\"gls\" HREF=\"$gls_file_mark{$SUMMARYTYPE}#gls:$label\">$text</A>$_";
}

sub do_cmd_glsnl{
   local($_) = @_;

   local($id, $label);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   local($text) = &gls_get_user($label, 4);

   "$text$_";
}

sub do_cmd_deferredGlsi{
   local($_) = @_;

   local($id, $label);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   local($text) = ucfirst(&gls_get_text($label));

   "<A CLASS=\"gls\" HREF=\"$gls_file_mark{$SUMMARYTYPE}#gls:$label\">$text</A>$_";
}

sub do_cmd_deferredglsni{
   local($_) = @_;

   local($id, $label);

   $label = &missing_braces unless
     s/$next_pair_pr_rx/$id=$1,$label=$2;''/eo;

   local($text) = ucfirst(&gls_get_text($label));

   "<A CLASS=\"gls\" HREF=\"$gls_file_mark{$SUMMARYTYPE}#gls:$label\">$text</A>$_";
}

sub do_cmd_deferredglslink{
  &do_cmd_glslink(@_);
}

sub do_cmd_deferredikeyvalopt{
  &do_cmd_ikeyvalopt(@_);
}

sub do_cmd_deferredindex{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $id = ++$global{'max_id'};

   join('', &named_index_entry($id, $text), $_);
}

sub do_cmd_gloindex{
   local($_) = @_;
   local($text,$id);

   $text = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$text=$2;''/eo;

   local($this_file) = $CURRENT_FILE;

   $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));

   $text = "$text@<TT>$text</TT>" unless $text=~/<TT>/;

   $id = ++$global{'max_id'};

   join('',
      &named_index_entry($id, "$text|indexglo"),
      $_);
}

sub do_cmd_deferredgloindex{
  &do_cmd_gloindex(@_);
}

sub do_cmd_deferredglocsindex{
  &do_cmd_glocsindex(@_);
}

sub set_glossarystyle_acronyms{
  eval(<<'END_STYLE'); 
  sub do_cmd_glossaryheader{ 
    local($_) = @_;
    $_
  }

  sub do_cmd_glsgroupheading{ 
     local($_) = @_;

     local($id, $heading);

     $heading = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$heading=$2;''/eo;

    $_
  }

  sub do_cmd_glossaryentryfield{
     local($_) = @_;

     local($id, $id2, $id3, $id4, $id5, $label, $name, $desc, $symbol, $backlink);

     $label = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$label=$2;''/eo;

     $name = &missing_braces unless
       s/$next_pair_pr_rx/$id2=$1;$name=$2;''/eo;

     $name=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

     $desc = &missing_braces unless
       s/$next_pair_pr_rx/$id3=$1;$desc=$2;''/eo;

     $symbol = &missing_braces unless
       s/$next_pair_pr_rx/$id4=$1;$symbol=$2;''/eo;

     $backlink = &missing_braces unless
       s/$next_pair_pr_rx/$id5=$1;$backlink=$2;''/eo;

     "<DT>\\glstarget $OP$id$CP$label$OP$id$CP$OP$id2$CP$name$OP$id2$CP\n<DD>"
     . "$desc$_";
  }

  sub do_cmd_glossarysubentryfield{
     local($_) = @_;

     local($id, $id2, $id3, $id4, $id5, $id6, $level, 
       $label, $name, $desc, $symbol, $backlink);

     $level = &missing_braces unless
       s/$next_pair_pr_rx/$id=$1;$level=$2;''/eo;

     $label = &missing_braces unless
       s/$next_pair_pr_rx/$id2=$1;$label=$2;''/eo;

     $name = &missing_braces unless
       s/$next_pair_pr_rx/$id3=$1;$name=$2;''/eo;

     $name=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

     $desc = &missing_braces unless
       s/$next_pair_pr_rx/$id4=$1;$desc=$2;''/eo;

     $symbol = &missing_braces unless
       s/$next_pair_pr_rx/$id5=$1;$symbol=$2;''/eo;

     $backlink = &missing_braces unless
       s/$next_pair_pr_rx/$id6=$1;$backlink=$2;''/eo;
  
     "<P>\\glssubentryitem $OP$id$CP$label$OP$id$CP"
     ."\\glstarget $OP$id2$CP$label$OP$id2$CP$OP$id3$CP$name$OP$id3$CP"
     ."$desc\n$_";
  }

  sub do_env_theglossary{
    local($_) = @_;

     "<DL>".&translate_commands("\\glossaryheader $_")."</DL>";
  }
END_STYLE
}

sub set_glossarystyle_summary{
  eval(<<'END_STYLE');
   sub do_cmd_glossaryheader{
      local($_)=@_;

      $sidepanelcontents = '';

      join('', 
           "<A NAME=\"top\"></A><P>", 
            &translate_commands("\\summarypreamble "),
            $_);
   }

   sub do_cmd_glsgroupheading{
     local($_) = @_;

     local($id, $heading);

     $heading = &missing_braces unless
      s/$next_pair_pr_rx/$id=$1,$heading=$2;''/eo;

     local($back) = '';

     unless ($headings=~/symbol/i)
     {
        $back = "<A HREF=\"#top\">Top</A>";
     }

     $heading=~s/gls(symbols|numbers)/ucfirst($1)/e;

     if ($sidepanelcontents=~/<A HREF="#$heading">/)
     {
        &write_warnings("Duplicate heading '$heading' found in summary\n");
        return $_;
     }

     $sidepanelcontents .= " |\n" if ($sidepanelcontents);

     $sidepanelcontents .= "<A HREF=\"#$heading\">$heading</A>";
   
     "<P>$back<DIV CLASS=\"summaryheading\"><A NAME=\"$heading\">$heading</A></DIV>".$_;
   }

   sub do_cmd_glossaryentryfield{
      local($_) = @_;

      local($id, $label, $name, $desc, $symbol, $backlink);

      $label = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$label=$2;''/eo;

      $name = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$name=$2;''/eo;

      $name=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

      $desc = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$desc=$2;''/eo;

      $desc=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

      $symbol = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$symbol=$2;''/eo;

      $backlink = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$backlink=$2;''/eo;

      local($syntax) = &gls_get_user($label, 1);

      $syntax=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

      $syntax = &translate_commands($syntax);

      if ($name=~/<TT>&#092;begin&#123;([a-zA-Z]+\*?)&#125;<\/TT>/)
      {
         $syntax .= "<TT><EM>&lt;body&gt;</EM><B>&#092;end&#123;$1&#125;</B></TT>";
      }

      local($location) = &gls_get_user($label, 2);

      $location=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

      local($info) = "<DIV CLASS=\"summary-desc\">"
                    . ($location ? "<B>Defined in:</B> ".&translate_commands($location).".<P>" : '')
                    . &translate_commands($desc);

      local($argsummary) = &gls_get_user($label, 3);

      # strip comments from $argsummary

      $argsummary=~s/<tex2html_comment_mark>\d+[^\n]*\n//s;

      if ($argsummary)
      {
         $argsummary = "<H4>Argument Summary:</H4>$argsummary</DIV>";
      }
      else
      {
         $argsummary = "</DIV>\n";
      }

      if (&gls_get_referenced_childcount($label) > 0)
      {
         $argsummary = '';
         $backlink = "\\glsresetsubentrycounter ";
      }
      else
      {
         $argsummary=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

         $argsummary = &translate_commands($argsummary);

         $backlink = "&#167;$backlink";
      }

      local($text) = "$syntax</DIV>\n<P>$info $backlink$argsummary";

      local($gloindex) = '';

      if ($name=~s/\\deferredgloindex\s*$OP(\d+)$CP(.*)$OP\1$CP//)
      {
         $gloindex = &do_cmd_gloindex("$OP$1$CP$2$OP$1$CP");
      }

      $name = &translate_commands($name);

      $global{'subentrycounter'} = 0;

      $id = ++$global{'max_id'};
      local($id2) = ++$global{'max_id'};

      "<DIV CLASS=\"summary-name\">\\glstarget $OP$id$CP$label$OP$id$CP$OP$id2$CP$name$OP$id2$CP$gloindex$text<P>$_";
   }

   sub do_cmd_glossarysubentryfield{
      local($_) = @_;

      local($id, $level, $label, $name, $desc, $symbol, $backlink);

      $level = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$level=$2;''/eo;

      $label = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$label=$2;''/eo;

      $name = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$name=$2;''/eo;

      $name=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

      $desc = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$desc=$2;''/eo;

      $symbol = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$symbol=$2;''/eo;

      $backlink = &missing_braces unless
        s/$next_pair_pr_rx/$id=$1;$backlink=$2;''/eo;

      # child entries all dealt with by parent entry.

      local($count) = ++$global{'subentrycounter'};

      local($parent) = &gls_get_parent($label);

      unless (&gls_entry_defined($parent))
      {
         &write_warnings("do_cmd_glossarysubentryfield: parent `$parent' doesn't exist for child `$label'");
      }

      local($pre) = '';

      local($id2);

      local($argsummary) = &gls_get_user($parent, 3);

      local($sep);

      if ($count eq &gls_get_referenced_childcount($parent))
      {
         if ($argsummary)
         {
            $argsummary=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

            $argsummary = &translate_commands($argsummary);

            $argsummary = "<H4>Argument Summary:</H4>\n<P>$argsummary\n<P>";
         }

         $sep = ".\n</DIV>";
      }
      else
      {
         $argsummary = '';
         $sep = ";\n";
      }

      $desc=~s/<tex2html_([a-zA-Z]+)_deferred>/\\deferred$1 /g;

      $desc = &translate_commands($desc);

      $id = ++$global{'max_id'};
      $id2 = ++$global{'max_id'};

      local($text) = "$desc &#167;$backlink$sep$argsummary";

      "$pre\\glstarget $OP$id$CP$label$OP$id$CP$OP$id2$CP$count)$OP$id2$CP\n$text$_";
   }

  sub do_env_theglossary{
    local($_) = @_;

    $glsinlinesep = '';

     $TITLE = $gls_toctitle{$SUMMARYTYPE} unless $TITLE;

     &translate_commands("\\glossaryheader $_")."<P><A HREF=\"#top\">Top</A>";
  }

END_STYLE
}

sub do_cmd_idxcrossref{
   local($_) = @_;
   local($text,$loc);

   $text = &missing_braces unless
     s/$next_pair_pr_rx/$text=$2;''/eo;

   $loc = &missing_braces unless
     s/$next_pair_pr_rx/$loc=$2;''/eo;

   join(&do_real_idxcrossref($text), $_);
}

sub do_real_idxcrossref{
   local($text) = @_;

   local(@split) = split /\s*,\s*/, $text;
   local($level) = $#split+1;

   local($anchor) = $split[$#split];

   $anchor=~s/<[^>]*>//g;

   $anchor=~s/([^a-zA-Z0-9])/_/g;

   join('', "<EM>see</EM> <A HREF=\"#$level-$anchor\">",
     $text, "</A>");
}

sub do_cmd_ifscreenorother{
   local($_) = @_;

   local($screen, $other);

   $screen = &missing_braces unless
     s/$next_pair_pr_rx/$screen=$2;''/eo;

   $other = &missing_braces unless
     s/$next_pair_pr_rx/$other=$2;''/eo;

   $screen.$_;
}

sub do_cmd_mbox{$_[0]; }

&ignore_commands( <<_IGNORED_CMDS_);
texorpdfstring # {}
ifpdf
else
fi
pdfinfo # {}
phantomsection
usetikzlibrary # {}
glsnumberformat
setentrycounter # [] # {}
inputlabelformat
outputlabelformat
nopostdesc
spacefactor
obeyspaces
ifbookorother # {}
setdoublecolumnglo
null
breakenumi
indexmarker # {} # {}
_IGNORED_CMDS_

local ($indexpreamble) = '';

sub do_cmd_indexpreamble{
   local($_) = @_;

   $indexpreamble = &missing_braces unless
     s/$next_pair_pr_rx/$indexpreamble=$2;''/eo;

   $_;
}

# This stuff is adapted from makeidx.perl:

sub add_book_idx {
    print "\nDoing the index ...";
    local($key, @keys, $next, $index, $old_key, $old_html);

    local($indexnav) = "";

    @keys = keys %printable_key;

    $currentindexlevel = 1;

    # include non- makeidx  index-entries

    foreach $key (keys %index)
    {
	next if $printable_key{$key};
	$old_key = $key;

	if ($key =~ s/###(.*)$//)
        {
	    next if $printable_key{$key};

	    push (@keys, $key);
	    $printable_key{$key} = $key;

	    if ($index{$old_key} =~ /HREF="([^"]*)"/i)
            {
		$old_html = $1;
		$old_html =~ /$dd?([^#\Q$dd\E]*)#/;
		$old_html = $1;
	    }
            else
            { 
               $old_html = ''
            }
	    $index{$key} = $index{$old_key} . $old_html."</A>\n |\n";
	};
    }

    @keys = sort book_makeidx_keysort @keys;
    @keys = grep(!/\001/, @keys);

    local($previous) = '';

    foreach $key (@keys)
    {
       local($initial) = uc(substr($key, 0, 1));

       $initial = '!' unless ($initial=~/[A-Z]/);

       unless ($initial eq $previous)
       {
          local ($tag) = $initial eq '!' ? "Symbols" : $initial;

          unless ($initial eq '!' or not $previous)
          {
             $index .= "<P><DIV CLASS=\"indextotop\"><A HREF=\"#top\">Top</A></DIV><P>";
          }

          $index .= join('', "<P><DIV CLASS=\"indexheading\"><B><A NAME=\"idx:$tag\">", 
            $tag, "</A></B></DIV><P>");

          $indexnav .= "\n| " if ($indexnav);

          $indexnav .= "<A HREF=\"#idx:$tag\"><B>$tag</B></A>";
       }

       $index .= &add_book_idx_key($key);

       $previous = $initial;
    }
    $index = '<DIV CLASS=\"indexentry\">'.$index unless ($index =~ /^\s*<DIV>/);

    $sidepanelcontents = $indexnav;

    s/$idx_mark/<A NAME="top"><\/A>$indexpreamble<DIV CLASS=\"index\">\n$index\n<\/DIV><P><DIV CLASS=\"indextotop\"><A HREF="#top">Top<\/A><\/DIV>/o;
}

sub book_makeidx_keysort {
  local($x, $y) = ($a,$b);

  &gls_compare($x, $y);
}
 
sub add_book_idx_key {
    local($key) = @_;
    local($index, $next);

    if (($index{$key} eq '@' )&&(!($index_printed{$key})))
    { 
	$index .= "\n<DIV CLASS=\"indexloc\">".&book_print_key."</DIV>\n<BR>"; 
    }
    elsif (($index{$key})&&(!($index_printed{$key})))
    {
	$next = "<DIV CLASS=\"indexentry\">".&book_print_key."</DIV>\n<DIV CLASS=\"indexloc\">"
          . &print_book_idx_links;

	$index .= $next."</DIV>\n<BR>";
	$index_printed{$key} = 1;
    }

    if ($sub_index{$key})
    {
	local($subkey, @subkeys, $subnext, $subindex);
	@subkeys = sort(split("\004", $sub_index{$key}));

	$index .= "<DIV CLASS=\"indexentry\">".&book_print_key."</DIV>\n" 
           unless $index_printed{$key};
	$index .= "<DIV CLASS=\"index\">\n"; 

        $currentindexlevel++;

	foreach $subkey (@subkeys)
        {
	    $index .= &add_book_sub_idx_key($subkey) unless ($index_printed{$subkey});
	}

        $currentindexlevel--;

	$index .= "</DIV>\n";
    }
    return $index;
}

local($currentindexlevel) = 0;

sub book_print_key {
    local($text) = $printable_key{$key};

    local($anchor) = $text;

    $anchor=~s/<[^>]*>//g;

    $anchor=~s/([^a-zA-Z0-9])/_/g;

    "$sidx_style<A NAME=\"$currentindexlevel-$anchor\">$text</A>$eidx_style"
}

sub print_book_idx_links {
    local($links) = $index{$key};

    local(@array) = split /\s+\|\s+/, $links;

    foreach $elem (@array)
    {
       if ($elem=~/idxcrossref(.*)$/)
       {
          local($text) = $1;
          $elem = &do_real_idxcrossref($text);
       }
    }

    $links = join(" |\n", @array);

    $links =~ s/(\n )?\| $//;

    $links 
}

sub add_book_sub_idx_key {
   local($key) = @_;
   local($index, $next);

   if ($sub_index{$key})
   {
      local($subkey, @subkeys, $subnext, $subindex);
      @subkeys = sort(split("\004", $sub_index{$key}));

      if ($index{$key})
      {
         $next = "<DIV CLASS=\"indexentry\">".&book_print_key."</DIV>\n<DIV CLASS=\"indexloc\">". &print_book_idx_links;
         $index .= $next."\n";
      }
      else
      {
         $index .= "<DIV CLASS=\"indexentry\">".&book_print_key  unless $index_printed{$key}
      }

      $index .= "</DIV>\n<DIV CLASS=\"index\">\n";

      $currentindexlevel++;

      foreach $subkey (@subkeys)
      {
         $index .= &add_book_sub_idx_key($subkey) unless ($index_printed{$subkey})
      }

      $index .= "</DIV>\n";

      $currentindexlevel--;
   }
   elsif ($index{$key})
   {
      $next = "<DIV CLASS=\"indexentry\">".&book_print_key."</DIV>\n<DIV CLASS=\"indexloc\">". &print_book_idx_links;
      $index .= $next."</DIV>\n<BR>";
   }

   $index_printed{$key} = 1;
   return $index;
}

sub book_named_index_entry {
    local($br_id, $str) = @_;
    # escape the quoting etc characters
    # ! -> \001
    # @ -> \002
    # | -> \003
    $* = 1; $str =~ s/\n\s*/ /g; $* = 0; # remove any newlines
    # protect \001 occurring with images
    $str =~ s/\001/\016/g;

    $str =~ s/\\\\/\011/g;
    $str =~ s/\\;SPMquot;/\012/g;
    $str =~ s/;SPMquot;!/\013/g;
    $str =~ s/!/\001/g;
    $str =~ s/\013/!/g;
    $str =~ s/;SPMquot;@/\015/g;
    $str =~ s/@/\002/g;
    $str =~ s/\015/@/g;
    $str =~ s/;SPMquot;\|/\017/g;
    $str =~ s/\|/\003/g;
    $str =~ s/\017/|/g;
    $str =~ s/;SPMquot;(.)/\1/g;
    $str =~ s/\012/;SPMquot;/g;
    $str =~ s/\011/\\\\/g;

    local($key_part, $pageref) = split("\003", $str, 2);
    local(@keys) = split("\001", $key_part);

    $TITLE = $saved_title if (($saved_title)&&(!($TITLE)||($TITLE eq $default_title)));
    $TITLE = $before unless $TITLE;

    local($words) = '';
    if ($SHOW_SECTION_NUMBERS) { $words = &make_idxnum; }
    elsif ($SHORT_INDEX) { $words = &make_shortidxname; }
    else { $words = &make_idxname; }
    local($super_key) = '';
    local($sort_key, $printable_key, $cur_key);

    foreach $key (@keys)
    {
	$key =~ s/\016/\001/g; # revert protected \001s

	($sort_key, $printable_key) = split("\002", $key);

	if ($printable_key =~ /tex2html_anchor_mark/ )
        {
	    $printable_key =~ s/><tex2html_anchor_mark><\/A><A//g;
	    local($tmpA,$tmpB) = split("NAME=\"", $printable_key);
	    ($tmpA,$tmpB) = split("\"", $tmpB);
	    $ref_files{$tmpA}='';
	    $index_labels{$tmpA} = 1;
	}

	if ($printable_key =~ /$cross_ref_mark/ )
        {
	    local($label,$id,$ref_label);
	    $printable_key =~ s/$cross_ref_mark#([^#]+)#([^>]+)>$cross_ref_mark/
	        do { ($label,$id) = ($1,$2);
		    $ref_label = $external_labels{$label} unless
			($ref_label = $ref_files{$label});
		    '"' . "$ref_label#$label" . '">' .
		    &get_ref_mark($label,$id)}/geo;
	}

	$printable_key =~ s/<\#[^\#>]*\#>//go;

	$printable_key =~ s/\&\#;\'134/\\/g;
	$printable_key =~ s/\&\#;\`<BR> /\\/g;
	$printable_key =~ s/\&\#;*SPMquot;92/\\/g;

	$sort_key .= "@$printable_key" if !($sort_key);

	if ($super_key)
        {
	    $cur_key = $super_key . "\001" . $sort_key;
	    $sub_index{$super_key} .= $cur_key . "\004";
	}
        else
        {
	    $cur_key = $sort_key;
	}

	$index{$cur_key} .= ""; 

	if (!($printable_key) && ($printable_key{$cur_key}))
	{
           $printable_key = $printable_key{$cur_key};
        } 

	if (!($printable_key{$cur_key} =~ /tex2html_anchor_mark/ ))
	{
           $printable_key{$cur_key} = $printable_key || $key;
        }

	$super_key = $cur_key;
    }

    if ($pageref) {
	if ($pageref eq "\(" ) { 
	    $pageref = ''; 
	    $next .= " from ";
	} elsif ($pageref eq "\)" ) { 
	    $pageref = ''; 
	    local($next) = $index{$cur_key};
	    $next =~ s/(\n )?\| $//;
	    $index{$cur_key} = "$next to ";
	}
    }

    if ($pageref) {
	$pageref =~ s/\s*$//g;	# remove trailing spaces
	if (!$pageref) { $pageref = ' ' }
	$pageref =~ s/see/<i>see <\/i> /g;

	local($tmp) = "do_cmd_$pageref";
	if (defined &$tmp) {
	    $words = &$tmp("<#0#>$words<#0#>");
	    $words =~ s/<\#[^\#]*\#>//go;
	    $pageref = '';
	}
    }
    if ($pageref) {
	if ($pageref =~ /tex2html_anchor_mark/ ) {
	    $pageref =~ s/><tex2html_anchor_mark><\/A><A//g;
	    local($tmpA,$tmpB) = split("NAME=\"", $pageref);
	    ($tmpA,$tmpB) = split("\"", $tmpB);
	    $ref_files{$tmpA}='';
	    $index_labels{$tmpA} = 1;
	}

	if ($pageref =~ /$cross_ref_mark/ ) {
	    local($label,$id,$ref_label);
	    $pageref =~ s/$cross_ref_mark#([^#]+)#([^>]+)>$cross_ref_mark/
	      do { ($label,$id) = ($1,$2);
		$ref_files{$label} = ''; # ???? RRM
		if ($index_labels{$label}) { $ref_label = ''; } 
		else { $ref_label = $external_labels{$label} 
		    unless ($ref_label = $ref_files{$label});
		}
		'"' . "$ref_label#$label" . '">' .
		  &get_ref_mark($label,$id)}/geo;
	}
	$pageref =~ s/<\#[^\#>]*\#>//go;

	if ($pageref eq ' ') { $index{$cur_key}='@'; }
	else { $index{$cur_key} .= $pageref . "\n | "; }
    } else {
	local($thisref) = &make_named_href('',"$CURRENT_FILE#$br_id",$words);
	$thisref =~ s/\n//g;
	$index{$cur_key} .= $thisref."\n | ";
    }
 
    "<A NAME=\"$br_id\">$anchor_invisible_mark<\/A>";
}


1;	# Must be last line
