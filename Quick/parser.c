#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"

bool block();
bool expr();
bool exprAdd();
bool exprAssign();
bool exprComp();
bool exprLogic();
bool exprMul();
bool exprPefix();
bool factor();
bool funcParam();
bool funcParams();
bool instr();
bool exprPrefix();


int iTk;	// the iterator in tokens
Token *consumed;	// the last consumed token

// same as err, but also prints the line of the current token
void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",tokens[iTk].line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
	}

bool consume(int code){
	if(tokens[iTk].code==code){
		consumed=&tokens[iTk++];
		return true;
		}
	return false;
	}



//Laborator activitate sapt 4 ---> 
//baseType :: = TYPE_INT | TYPE_REAL | TYPE_STR --> chestiile de baza, tipurile
bool baseType() {
	printf("#baseType %d\n", tokens[iTk].code);
	if (consume(TYPE_INT) || consume(TYPE_REAL) || consume(TYPE_STR)) {
		return true;
	}
	return false;
}

//defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
	printf("#defVar %d\n", tokens[iTk].code);
	//totul sau nimic
	// -daca se indeplinese regula, isi consuma toti atomii si returneaza true
	// -altfel, daca nu se indeplinese regla, reurneaza false
	int star = iTk; //salvare pozitie initiala
	if (consume(VAR)) {
		if (consume(ID)) {
			if (consume(COLON)) {
				if (baseType()) {
					if (consume(SEMICOLON)) {
						return true;
					}
				}
			}
		}
		return false;
	}
	iTk = star; //restaurare
	return false;
}

//defFunc :: = FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar * block END
bool defFunc() {
	printf("#defFunc %d\n", tokens[iTk].code);
	int star = iTk;
	if (consume(FUNCTION)) {
		if (consume(ID)) {
			if(consume(LPAR)) {
				if(funcParams()){}
				if (consume(RPAR)) {
					if (consume(COLON)) {
						if (baseType()) {
							while (defVar()) {}
							if(block()){
								if (consume(END)) {
									return true;
								}
							}
						}
					}
				}
			}
		}
	}
	iTk = star; //restaurare
	return false;
}

//block :: = instr +
bool block() {
	printf("#block %d\n", tokens[iTk].code);
	int star = iTk;

	if (!instr()) { //daca nu exisa prima instructiune
		iTk = star; //restaurare
		return false;
	}

	while(instr()){}

	return true;	
}	


//funcParams :: = funcParam(COMMA funcParam)*
bool funcParams() {
	printf("#funcParams %d\n", tokens[iTk].code);
	int star = iTk;
	if (funcParam()) {
			while(consume(COMMA)) {
				if (funcParam()) {}
			}
			return true;
		}
	iTk = star; //restaurare
	return false;
}

//funcParam :: = ID COLON baseType
bool funcParam() {
	printf("#funcParam %d\n", tokens[iTk].code);
	int star = iTk;
	if (consume(ID)) {
		if (consume(COLON)) {
			if (baseType()) {
				return true;
			}
		}
	}
	iTk = star; //restaurare
	return false;
}

//instr :: = expr ? SEMICOLON
//| IF LPAR expr RPAR block(ELSE block) ? END
//| RETURN expr SEMICOLON
//| WHILE LPAR expr RPAR block END
bool instr() {
	printf("#instr %d\n", tokens[iTk].code);
	int star = iTk;
	if(expr() || 1){         //optionala                           
		if (consume(SEMICOLON)) {} 
		 else if (consume(IF)) {
			if (consume(LPAR)) {
				if (expr()) {
					if (consume(RPAR)) {
						if (block()) {
							if (((consume(ELSE)) && (block())) || 1) {} //optionala
							if (consume(END)) {
							}
						}
					}
				}
			}
			iTk = star; //restaurare
			return false;
		}
		 else if (consume(RETURN)) {
			if (expr()) {
				if (consume(SEMICOLON)) {

				}
			}
			iTk = star; //restaurare
			return false;
		}
		 else if (consume(WHILE)) {
			if (consume(LPAR)) {
				if (expr()) {
					if (consume(RPAR)) {
						if (block()) {
							if (consume(END)) {
							}
						}
					}
				}
			}
			iTk = star; //restaurare
			return false;
		}
		return false;
	}
	iTk = star; //restaurare
	return false;
}	

//expr ::= exprLogic
bool expr() {
	printf("#expr %d\n", tokens[iTk].code);
	int star;
	if (exprLogic()) {
		return true;
	}

	iTk = star; //restaurare
	return false;
}

//exprLogic :: = exprAssign((AND | OR) exprAssign) *
bool exprLogic() {
	printf("#exprLogic %d\n", tokens[iTk].code);
	int star;

	if (exprAssign()) {
		while ((consume(AND)) || (consume(OR))) {
			if (exprAsssign()) {
			}
		}
		return true;
	}
	iTk = star; //restaurare
	return false;
}


//exprAssign :: = (ID ASSIGN) ? exprComp
bool exprAssign() {
	printf("#exprAssign %d\n", tokens[iTk].code);
	int star;
	
	if (consume(ID) || 1) {
		if (exprComp()) {
		}
		return true;
	}

	iTk = star; //restaurare
	return false;
}


//exprComp :: = exprAdd((LESS | EQUAL) exprAdd) ?
bool exprComp() {
	printf("#exprComp %d", tokens[iTk].code);
	int star;

	if (exprAdd()) {
		if(consume(LESS) || consume(EQUAL)){}
		if (exprAdd()) {}
		return true;
	}

	iTk = star; //restaurare
	return false;
}

//exprAdd :: = exprMul((ADD | SUB) exprMul) *
bool exprAdd() {
	printf("#exprAdd %d", tokens[iTk].code);
	int star;

	if (exprMul()) {
		if(consume(ADD) || consume(SUB)){}
		if(exprMul()){}
		return true;
	}

	iTk = star; //restaurare
	return false;
}

//exprMul :: = exprPrefix((MUL | DIV) exprPrefix) *
bool exprMul() {
	printf("#exprMul %d", tokens[iTk].code);
	int star;

	if (exprPrefix()) {
		if(consume(MUL) || consume(DIV)){}
		if (exprPrefix){}
		return true;
	}

	iTk = star; //restaurare
	return false;
}

//exprPrefix :: = (SUB | NOT) ? factor
bool exprPrefix() {
	printf("#exprPrefix %d", tokens[iTk].code);
	int star;

	if (consume(SUB) || consume(NOT)){
		if (factor() || 1) {}
		return true;
	}

	iTk = star; //restaurare
	return false;
}

//factor :: = INT
//| REAL
//| STR
//| LPAR expr RPAR
//| ID(LPAR(expr(COMMA expr)*) ? RPAR) ?
bool factor() {
	printf("#factor %d", tokens[iTk].code);
	int star;

	if(consume(INT)){}
	else if(consume(REAL)){}
	else if(consume(STR)){}
	else if (consume(LPAR)) {
		if (expr()) {
			if(consume(RPAR)){}
		}
	}
	else if (consume(ID) && consume(LPAR)) {
		if (expr()) {
			if (consume(COMMA)) {
				if(expr()){}
			}
		}
		if(consume(RPAR)){}
	}
	return true;
}



// program ::= ( defVar | defFunc | block )* FINISH
bool program(){
	for(;;){
		if(defVar()){}
		else if(defFunc()){}
		else if(block()){}
		else break;
		}
	if(consume(FINISH)){
		return true;
		}else tkerr("syntax error");
	return false;
	}

void parse(){
	iTk=0;
	program();
	}
