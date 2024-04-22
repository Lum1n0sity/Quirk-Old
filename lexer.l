%token KEYWORD IDENTIFIER INTEGER
%token STRING PLUS MINUS STAR SLASH
%token EQUAL SEMICOLON LPAREN RPAREN

%%
[ \t\n\r] {  }

"int"       { return KEYWORD; }
"if"        { return KEYWORD; }
"else"      { return KEYWORD; }
"while"     { return KEYWORD; }

[a-zA-Z_][a-zA-Z0-9_]* { return IDENTIFIER; }

[0-9]+  { return INTEGER; }
\"([^\"\\\\]|\\.)*\" { return STRING; }

\+  { return PLUS; }
\-  { return MINUS; }
\*  { return STAR; }
\/  { return SLASH; }

=   { return EQUAL; }
;   { return SEMICOLON; }
\ ( { return LPAREN; }
\ ) { return RPAREN;}

%%