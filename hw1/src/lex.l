%{
#define LIST               strcat(buf, yytext)
#define token(t)           {LIST; if (Opt_T) printf("<%s>\n", #t);}
#define tokenChar(t)       {LIST; if (Opt_T) printf("<%c>\n", (t));}
#define tokenInteger(t, i) {LIST; if (Opt_T) printf("<%s: %d>\n", #t, (i));}
#define tokenOctInteger(t, i) {LIST; if (Opt_T) printf("<%s: %s>\n", #t, (i));}
#define tokenString(t, s)  {if (Opt_T) printf("<%s: %s>\n", #t, (s));}
#define tokenId(t, s)  {LIST;if (Opt_T) printf("<%s: %s>\n", #t, (s));}
#define tokenScientific(t, s)  {LIST;if (Opt_T) printf("<%s: %s>\n", #t, (s));}
#define tokenFloat(t, s)  {LIST;if (Opt_T) printf("<%s: %s>\n", #t, (s));}
#define tokenOther(t, s)  {LIST;if (Opt_T) printf("<%s: %s>\n", #t, (s));}

// Define your token macros here
//  for example :
//    #define tokenScientific(t, s)   <definition>
//    #define tokenFloat(ts)          <definition>
//    #define <any other macros you needed>

#define MAX_LINE_LENG      256

int Opt_S = 1;
int Opt_T = 1;
int linenum = 1;
char buf[MAX_LINE_LENG];
%}


digit [0-9]
letter [a-zA-Z_]
float (0|[1-9][0-9]*)\.(0|[0-9]*[1-9])

%x C_COMMENT
%%

    /* Pseudocomments */
"//&S+".* {Opt_S = 1;LIST;}
"//&S-".* {Opt_S = 0;LIST;}
"//&T+".* {Opt_T = 1;LIST;}
"//&T-".* {Opt_T = 0;LIST;}

    /* c style */
"/*" {LIST;BEGIN C_COMMENT;}
<C_COMMENT>.  {LIST;}
<C_COMMENT>"*/" {LIST;BEGIN INITIAL;}

    /* C++ style */
"//".* {LIST;}

    /* space and tab */
[\t ]   {LIST;}

    /* address string */
    /* ignore start and end ( " ) */
\"([^"]|\"{2})*\"  {LIST;
    int len = yyleng, cnt = 0;
    char new_str[len];
    for(int idx = 1; idx < len;) {
        if(yytext[idx] == '\"' && yytext[idx + 1] == '\"') {
            idx++;
        }
        new_str[cnt++] = yytext[idx++];
    }
    new_str[cnt] = 0;
    new_str[strlen(new_str) - 1] = '\0';

    tokenString(string, new_str);
}

    /*Delimiters*/
","        {tokenChar(',');}
";"        {tokenChar(';');}
":"        {tokenChar(':');}
"("        {tokenChar('(');}
")"        {tokenChar(')');}
"["        {tokenChar('[');}
"]"        {tokenChar(']');}

    /*Arithmetic, Relational, and Logical Operators*/
"+"        {token(+);}
"-"        {token(-);}
"*"        {token(*);}
"/"        {token(/);}
"mod"      {token(mod);}
":="       {token(:=);}
"<"        {token(<);}
"<="       {token(<=);}
"<>"       {token(<>);}
">="       {token(>=);}
">"        {token(>);}
"="        {token(=);}
"and"      {token(and);}
"or"       {token(or);} 
"not"      {token(not);}

    /*keyword*/
"array"    {token(KWarray);}
"begin"    {token(KWbegin);}
"boolean"  {token(KWboolean);}
"def"      {token(KWdef);}
"do"       {token(KWdo);}
"else"     {token(KWelse);}
"end"      {token(KWend);}
"false"    {token(KWfalse);}
"for"      {token(KWfor);}
"integer"  {token(KWinteger);}
"if"       {token(KWif);}
"of"       {token(KWof);}
"print"    {token(KWprint);}
"read"     {token(KWread);}
"real"     {token(KWreal);}
"string"   {token(KWstring);}
"then"     {token(KWthen);}
"to"       {token(KWto);}
"true"     {token(KWtrue);}
"return"   {token(KWreturn);}
"var"      {token(KWvar);}
"while"    {token(KWwhile);}

0|([1-9][0-9]*) {tokenInteger(integer, atoi(yytext));}
0[0-7]+  {tokenOctInteger(oct_integer, yytext);}

{letter}({letter}|{digit})*  {tokenId(id, yytext);}
(0|[1-9][0-9]*)\.(0|[0-9]*[1-9])  {tokenFloat(float, yytext);}
({float}|([1-9][0-9]*))?[eE][+-]?(([1-9][0-9]*)|0)  {tokenScientific(scientific, yytext);}

    /* \n */
<*>\n      {
          LIST;
          if (Opt_S)
            printf("%d: %s", linenum, buf);
          linenum++;
          buf[0] = '\0';
        }

    /*Unrecognized character*/
.       {
          /* error */
          printf("Error at line %d: bad character \"%s\"\n", linenum, yytext );
          exit(-1);
        }


%%
int main( int argc, char **argv )
{
  if( argc != 2 ) {
    fprintf( stderr, "Usage: ./scanner [filename]\n" );
    exit(0);
  }
  
  FILE *fp = fopen( argv[1], "r" );
  if( fp == NULL ) {
    fprintf( stderr, "Open file error\n" );
    exit(-1);
  }
  
  yyin = fp;

  yylex();

  exit(0);
}