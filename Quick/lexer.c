// Andreea Rosu
// Informatica
// Anul III, 2025

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "utils.h"

// variabile globale
Token tokens[MAX_TOKENS]; // lista
int nTokens;
int line = 1; // the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token* addTk(int code) {
    if (nTokens == MAX_TOKENS) err("too many tokens");
    Token* tk = &tokens[nTokens];
    tk->code = code;
    tk->line = line;
    nTokens++;
    return tk;
}

// copy in the dst buffer the string between [begin,end)
char* copyn(char* dst, const char* begin, const char* end) {
    char* p = dst;
    if (end - begin > MAX_STR) err("string too long");
    while (begin != end) *p++ = *begin++;
    *p = '\0';
    return dst;
}

void tokenize(const char* pch) {
    const char* start;
    Token* tk;
    char buf[MAX_STR + 1];

    for (;;) { // se poate folosi si while
        switch (*pch) { // asta e pch[0]
        case ' ': 
        case '\t': 
            pch++; 
            break;

        case '\r': // handles different kinds of newlines
            if (pch[1] == '\n') pch++;
            // fallthrough to \n
        case '\n':
            line++;
            pch++;
            break;

            // delimitatori
        case '\0': 
            addTk(FINISH); 
            return;

        case ',': 
            addTk(COMMA); 
            pch++; 
            break;

        case '(': 
            addTk(LPAR); 
            pch++; 
            break;

        case ')': 
            addTk(RPAR); 
            pch++; 
            break;

        case ';': 
            addTk(SEMICOLON); 
            pch++; 
            break;

        case ':': 
            addTk(COLON); 
            pch++; 
            break;

            // cazuri operatori:
        case '=':
            if (pch[1] == '=') { 
                addTk(EQUAL); 
                pch += 2; 
            }
            else { 
                addTk(ASSIGN); 
                pch++; 
            }
            break;

        case '+': 
            addTk(ADD); 
            pch++; 
            break;

        case '-': 
            addTk(SUB); 
            pch++; 
            break;

        case '*': 
            addTk(MUL); 
            pch++; 
            break;

        case '&':
            if (pch[1] == '&') { 
                addTk(AND); 
                pch += 2; 
            }
            break;

        case '|':
            if (pch[1] == '|') { 
                addTk(OR); 
                pch += 2; 
            }
            break;

        case '!':
            if (pch[1] == '=') { 
                addTk(NOTEQ); 
                pch += 2; 
            }
            else { 
                addTk(NOT); 
                pch++; 
            }
            break;

        case '<': 
            addTk(LESS); 
            pch++; 
            break;

        case '>':
            if (pch[1] == '=') { 
                addTk(GREATERQ); 
                pch += 2; 
            }
            else { 
                addTk(GREATER); 
                pch++; 
            }
            break;

            // sarim peste partea de comentariu si ""
        case '/':
            if (pch[1] == '/') {
                pch += 2;
                while (*pch != '\n' && *pch != '\0') pch++;
            }
            else { 
                addTk(DIV); 
                pch++; 
            }
            break;

        case '"':
            pch++;
            start = pch;
            while (*pch != '"' && *pch != '\0')
                pch++;
            tk = addTk(STR); // Schimba TYPE_STR in STR
            copyn(tk->text, start, pch);
            pch++;
            break;

        default:
            if (isalpha(*pch) || *pch == '_') {
                for (start = pch++; isalnum(*pch) || *pch == '_'; pch++) {}
                char* text = copyn(buf, start, pch); // COPIE DOAR O DATĂ

                // verificare daca este vre-un keyword
                if (strcmp(text, "int") == 0) addTk(TYPE_INT);
                else if (strcmp(text, "function") == 0) addTk(FUNCTION);
                else if (strcmp(text, "var") == 0) addTk(VAR);
                else if (strcmp(text, "if") == 0) addTk(IF);
                else if (strcmp(text, "else") == 0) addTk(ELSE);
                else if (strcmp(text, "while") == 0) addTk(WHILE);
                else if (strcmp(text, "end") == 0) addTk(END);
                else if (strcmp(text, "return") == 0) addTk(RETURN);
                else if (strcmp(text, "real") == 0) addTk(TYPE_REAL);
                else if (strcmp(text, "str") == 0) addTk(TYPE_STR);
                else {
                    tk = addTk(ID);
                    strcpy(tk->text, text); // strcpy pt siruri
                }
            }
            else if (isdigit(*pch)) {
                start = pch;

                // 1. Cautam partea intreaga
                while (isdigit(*pch)) {
                    pch++;
                }

                int is_real = 0;
                // 2. Verificam si cautam partea zecimana
                if (*pch == '.') {
                    is_real = 1;
                    pch++; // Sare peste '.'
                    while (isdigit(*pch)) {
                        pch++;
                    }
                }

                // 3. Copiem nr inttren in buf
                copyn(buf, start, pch);

                // 4. Se creeaza tokenul
                if (is_real) {
                    tk = addTk(REAL);
                    tk->r = atof(buf);
                }
                else {
                    tk = addTk(INT);
                    tk->i = atoi(buf);
                }
            }
            else err("invalid char: %c (%d)", *pch, *pch);
        }
    }
}
  
void showTokens() {
    for (int i = 0; i < nTokens; i++) {
        Token* tk = &tokens[i];
        printf("%d ", tk->line);

        switch (tk->code) {
            case ID: printf("ID:%s\n", tk->text); break;
            case TYPE_INT: printf("TYPE_INT\n"); break;
            case VAR: printf("VAR\n"); break;
            case FUNCTION: printf("FUNCTION\n"); break;
            case IF: printf("IF\n"); break;
            case ELSE: printf("ELSE\n"); break;
            case WHILE: printf("WHILE\n"); break;
            case END: printf("END\n"); break;
            case RETURN: printf("RETURN\n"); break;
            case TYPE_REAL: printf("TYPE_REAL\n"); break;
            case TYPE_STR: printf("TYPE_STR:\n"); break;
            case COMMA: printf("COMMA\n"); break;
            case COLON: printf("COLON\n"); break;
            case SEMICOLON: printf("SEMICOLON\n"); break;
            case LPAR: printf("LPAR\n"); break;
            case RPAR: printf("RPAR\n"); break;
            case FINISH: printf("FINISH\n"); break;
            case ASSIGN: printf("ASSIGN\n"); break;
            case EQUAL: printf("EQUAL\n"); break;
            case ADD: printf("ADD\n"); break;
            case SUB: printf("SUB\n"); break;
            case DIV: printf("MUL\n"); break;
            case MUL: printf("AND\n"); break;
            case OR: printf("OR\n"); break;
            case NOT: printf("NOT\n"); break;
            case NOTEQ: printf("NOTEQ\n"); break;
            case LESS: printf("LESS\n"); break;
            case GREATER: printf("GREATER\n"); break;
            case GREATERQ: printf("GREATERQ\n"); break;
            case INT: printf("INT:%d\n", tk->i); break;
            case REAL: printf("REAL:%f\n", tk->r); break;
            case STR: printf("STR:%s\n", tk->text); break;
        }
    }
}


//ID
//// keywords
//, TYPE_INT, VAR, FUNCTION, IF, ELSE, WHILE, END, RETURN, TYPE_REAL, TYPE_STR
//// delimiters
//, COMMA, COLON, SEMICOLON, LPAR, RPAR, FINISH
//// operators
//, ASSIGN, EQUAL, ADD, SUB, DIV, MUL, AND, OR, NOT, NOTEQ, LESS, GREATER, GREATERQ
////const
//, INT, REAL, STR
//	};