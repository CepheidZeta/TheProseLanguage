%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

extern int line_number;
void yyerror(const char *s);
int yylex();
extern FILE *yyin;

struct symbol {
    char *name;
    int is_int;
    int int_val;
    char *str_val;
} sym[100];
int sym_count = 0;

int find_id(char *n) {
    for(int i=0;i<sym_count;i++)
        if(strcmp(sym[i].name,n)==0) return i;
    return -1;
}

/* ======================
   Code emission helpers
   ====================== */
char data_section[8192] = "";
char code_section[8192] = "";
char binary_section[8192] = "";

void emit_data(const char *fmt, ...) {
    va_list args;
    char buf[256];
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    strcat(data_section, buf);
    strcat(data_section, "\n");
}

void emit_code(const char *fmt, ...) {
    va_list args;
    char buf[256];
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    strcat(code_section, buf);
    strcat(code_section, "\n");
}

int reg_counter = 1;
int checker = 0;
int new_register() { return reg_counter++; }

void emit_data_section() {
    printf(".data\n");
    for(int i = 0; i < sym_count; i++)
        if(sym[i].is_int)
            printf("%s: .dword\n", sym[i].name);
}

void emit_code_section() {
    printf("\n.code\n");
    printf("%s", code_section);
}

unsigned int get_var_address(const char *var) {
    for(int i=0; i<sym_count; i++)
        if(strcmp(sym[i].name, var)==0)
            return i*8; // each variable 8 bytes
    return 0;
}

void emit_binary() {
    char line[256];
    char *next = code_section;

    while(sscanf(next, "%[^\n]\n", line) == 1) {
        next = strchr(next, '\n');
        if(next) next++;
        else next = "";

        unsigned int bin = 0;

        if(strncmp(line, "daddiu", 6) == 0) {
            int rd, imm;
            sscanf(line, "daddiu r%d, r0, #%d", &rd, &imm);
            bin = 0x24000000 | (rd << 16) | (imm & 0xFFFF);
        }
        else if(strncmp(line, "sd", 2) == 0) {
            int rd;
            char var[50];
            sscanf(line, "sd r%d, %[^'(]'(r0)", &rd, var);
            unsigned int addr = get_var_address(var);
            bin = 0xF8000000 | (rd << 16) | (addr & 0xFFFF);
        }
        else if(strncmp(line, "ld", 2) == 0) {
            int rd;
            char var[50];
            sscanf(line, "ld r%d, %[^'(]'(r0)", &rd, var);
            unsigned int addr = get_var_address(var);
            bin = 0xF4000000 | (rd << 16) | (addr & 0xFFFF);
        }
        else if(strncmp(line, "daddu", 5) == 0) {
            int rd, rs, rt;
            sscanf(line, "daddu r%d, r%d, r%d", &rd, &rs, &rt);
            bin = 0x00000021 | (rs << 21) | (rt << 16) | (rd << 11);
        }
        else if(strncmp(line, "dsubu", 5) == 0) {
            int rd, rs, rt;
            sscanf(line, "dsubu r%d, r%d, r%d", &rd, &rs, &rt);
            bin = 0x00000023 | (rs << 21) | (rt << 16) | (rd << 11);
        }
        else if(strncmp(line, "dmult", 5) == 0) {
            int rs, rt;
            sscanf(line, "dmult r%d, r%d", &rs, &rt);
            bin = 0x00000018 | (rs << 21) | (rt << 16);
        }
        else if(strncmp(line, "ddiv", 4) == 0) {
            int rs, rt;
            sscanf(line, "ddiv r%d, r%d", &rs, &rt);
            bin = 0x0000001A | (rs << 21) | (rt << 16);
        }
        else if(strncmp(line, "mflo", 4) == 0) {
            int rd;
            sscanf(line, "mflo r%d", &rd);
            bin = 0x00001012 | (rd << 11);
        }
        else {
            continue; // skip unrecognized lines
        }

        // Print binary and hex
        for(int i=31;i>=0;i--) printf("%d", (bin>>i)&1);
        printf(" | %08X\n", bin);
    }
}

%}


%union {
    int ival;
    char *sval;
    struct {
        int val;
        int reg;
    } expr;
}

%token <sval> START DONE INTEGER CHARACTER DO SHOW ASSIGN PLUS MINUS MULT DIV COMMA DOT RPAREN LPAREN
%token <ival> NUMBER
%token <sval> ID STRING

%type <expr> EXP FXP GXP
%type <sval> IE CE

%left PLUS MINUS
%left MULT DIV
%right UMINUS

%%

program:
      START declaration_list DO execution_list DONE DOT
;

declaration_list:
      /* empty */
    | declaration_list declaration
;

declaration:
      INTEGER IE DOT
    | CHARACTER CE DOT
;

IE:
      ID {
          sym[sym_count].name = strdup($1);
          sym[sym_count].is_int = 1;
          sym[sym_count].int_val = 0;
          sym[sym_count].str_val = NULL;
          sym_count++;
      }
    | IE COMMA ID {
          sym[sym_count].name = strdup($3);
          sym[sym_count].is_int = 1;
          sym[sym_count].int_val = 0;
          sym[sym_count].str_val = NULL;
          sym_count++;
      }
    | ID ASSIGN EXP {
          sym[sym_count].name = strdup($1);
          sym[sym_count].is_int = 1;
          sym[sym_count].int_val = $3.val;
          sym_count++;
          emit_code("daddiu r%d, r0, #%d", $3.reg, $3.val);
          emit_code("sd r%d, %s(r0)", $3.reg, $1);
      }
    | IE COMMA ID ASSIGN EXP {
          sym[sym_count].name = strdup($3);
          sym[sym_count].is_int = 1;
          sym[sym_count].int_val = $5.val;
          sym_count++;
          if(checker != 1){
            emit_code("2daddiu r%d, r0, #%d", $5.reg, $5.val);
            emit_code("sd r%d, %s(r0)", $5.reg, $3);
          }else{
             emit_code("sd r%d, %s(r0)", $5.reg, $3);
             checker = 0;
          }
      }
;

CE:
      ID {
          sym[sym_count].name = strdup($1);
          sym[sym_count].is_int = 0;
          sym[sym_count].str_val = NULL;
          sym_count++;
      }
    | CE COMMA ID {
          sym[sym_count].name = strdup($3);
          sym[sym_count].is_int = 0;
          sym[sym_count].str_val = NULL;
          sym_count++;
      }
    | ID ASSIGN STRING {
          sym[sym_count].name = strdup($1);
          sym[sym_count].is_int = 0;
          sym[sym_count].str_val = strdup($3);
          sym_count++;
      }
    | CE COMMA ID ASSIGN STRING {
          sym[sym_count].name = strdup($3);
          sym[sym_count].is_int = 0;
          sym[sym_count].str_val = strdup($5);
          sym_count++;
      }
;

/* =========================
   EXPRESSIONS (with MIPS)
   ========================= */

EXP:
      EXP PLUS FXP {
          $$.val = $1.val + $3.val;
          int r = new_register();
          emit_code("daddu r%d, r%d, r%d", r, $1.reg, $3.reg);
          $$.reg = r;
      }
    | EXP MINUS FXP {
          $$.val = $1.val - $3.val;
          int r = new_register();
          emit_code("dsubu r%d, r%d, r%d", r, $1.reg, $3.reg);
          $$.reg = r;
      }
    | FXP { $$.val = $1.val; $$.reg = $1.reg; }
;

FXP:
      FXP MULT GXP {
          $$.val = $1.val * $3.val;
          int r = new_register();
          emit_code("dmult r%d, r%d", $1.reg, $3.reg);
          emit_code("mflo r%d", r);
          $$.reg = r;
      }
    | FXP DIV GXP {
          $$.val = $1.val / $3.val;
          int r = new_register();
          emit_code("ddiv r%d, r%d", $1.reg, $3.reg);
          emit_code("mflo r%d", r);
          $$.reg = r;
      }
    | GXP { $$.val = $1.val; $$.reg = $1.reg; }
;

GXP:
      MINUS GXP %prec UMINUS {
          $$.val = -$2.val;
          int r = new_register();
          emit_code("dsubu r%d, r0, r%d", r, $2.reg);
          $$.reg = r;
      }
    | LPAREN EXP RPAREN { $$.val = $2.val; $$.reg = $2.reg; }
    | NUMBER {
          $$.val = $1;
          int r = new_register();
          emit_code("daddiu r%d, r0, #%d", r, $1);
          checker = 1;
          $$.reg = r;
      }
    | ID {
          int idx = find_id($1);
          if(idx==-1) {
              printf("ERROR: undeclared variable %s\n", $1);
              $$.val = 0;
          } else if(!sym[idx].is_int) {
              printf("ERROR: variable %s is not integer\n", $1);
              $$.val = 0;
          } else {
              $$.val = sym[idx].int_val;
              int r = new_register();
              emit_code("ld r%d, %s(r0)", r, $1);
              $$.reg = r;
          }
      }
;

execution_list:
      /* empty */
    | execution_list statement
;

statement:
      PR
    | ID ASSIGN EXP DOT {
          int idx = find_id($1);
          if(idx>=0) sym[idx].int_val = $3.val;
          emit_code("sd r%d, %s(r0)", $3.reg, $1);
      }
    | ID ASSIGN STRING DOT {
          int idx = find_id($1);
          if(idx>=0) sym[idx].str_val = strdup($3);
      }
;

PR:
      SHOW STRING DOT {
          char *src = $2;
          char result[2048] = "";
          char var[100];
          for(int i=0;i<strlen(src);i++) {
              if(src[i]=='[' && src[i+1]=='[') {
                  int j=i+2,k=0;
                  while(!(src[j]==']' && src[j+1]==']') && src[j]!=0) var[k++]=src[j++];
                  var[k]=0; j+=2;
                  int idx = find_id(var);
                  if(idx>=0) {
                      if(sym[idx].is_int) {
                          char tmp[50]; sprintf(tmp,"%d",sym[idx].int_val);
                          strcat(result,tmp);
                      } else strcat(result,sym[idx].str_val);
                  }
                  i = j-1;
              } else if(src[i]=='[') {
                  int j=i+1,k=0;
                  while(src[j]!=']' && src[j]!=0) var[k++]=src[j++];
                  var[k]=0; j++;
                  int idx=find_id(var);
                  if(idx>=0) {
                      if(sym[idx].is_int) {
                          char tmp[50]; sprintf(tmp,"%d",sym[idx].int_val);
                          strcat(result,tmp);
                      } else strcat(result,sym[idx].str_val);
                  }
                  i=j-1;
              } else {
                  int len=strlen(result);
                  result[len]=src[i]; result[len+1]=0;
              }
          }
          printf("%s\n", result);
      }
    | SHOW ID DOT {
          int idx=find_id($2);
          if(idx>=0) {
              if(sym[idx].is_int) printf("%d\n", sym[idx].int_val);
              else printf("%s\n", sym[idx].str_val);
          }
      }
;

%%

int main(int argc,char **argv) {
    if(argc<2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1],"r");
    if(!yyin) { perror("Cannot open file"); return 1; }

    yyparse();

    printf("\n=== EDU-MIPS64 ===\n");
    emit_data_section();
    emit_code_section();

    printf("\n=== BINARY ===\n");
    emit_binary();

    return 0;
}

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d: %s\n", line_number, s);
    printf("Parse error: %s\n", s);
}
