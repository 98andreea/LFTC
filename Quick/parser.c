// Andreea Rosu
// Informatica
// Anul III, 2025
// ACTIVITATE 3 --> NOTA 9
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"
#include "ad.h"

Ret ret; 
Symbol* crtFn = NULL;

bool block();
bool expr();
bool exprAdd();
bool exprAssign();
bool exprComp();
bool exprLogic();
bool exprMul();
bool exprPrefix();
bool factor();
bool funcParam();
bool funcParams();
bool instr();



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



//Laborator activitate 3 ---> 
//baseType :: = TYPE_INT | TYPE_REAL | TYPE_STR --> chestiile de baza, tipurile
bool baseType() {
	//printf("#baseType %d\n", tokens[iTk].code);
	if (consume(TYPE_INT)){
		ret.type = TYPE_INT;
		return true;
	}
	
	if (consume(TYPE_REAL)) {
		ret.type = TYPE_REAL;
		return true;
	}
	
	if(consume(TYPE_STR)) {
		ret.type = TYPE_STR;
		return true;
	}

	return false;
}

//defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
	//printf("#defVar %d\n", tokens[iTk].code);
	//totul sau nimic
	// -daca se indeplinese regula, isi consuma toti atomii si returneaza true
	// -altfel, daca nu se indeplinese regla, reurneaza false
	int star = iTk; //salvare pozitie initiala
	Symbol* s = NULL;

	if (!consume(VAR)) {
		iTk = star; //restaurare
		return false;
	}
	else {
		if (!consume(ID)) {
			tkerr("Eroare: Lipseste identificator dupa variabila var");
		}
	
		////analiza de domeniu
		{
			const char* name = consumed->text;
			Symbol* s = searchInCurrentDomain(name);
			if (s)tkerr("symbol redefinition: %s", name);
			s = addSymbol(name, KIND_VAR);
			s->local = crtFn != NULL;
		}
	
		if (!consume(COLON)) {
			tkerr("Eroare: Lipseste ':' dupa numele variabilei");
		}
		if (!baseType()) {
			tkerr("Eroare: Tip invalid pentru variabila");
		}

		////analiza de domeniu
		if (s) {
			s->type = ret.type;
		}

		if (!consume(SEMICOLON)) {
			tkerr("Eroare: Lipseste ';' dupa numele variabilei");
		}
	}
	return true;
}

//defFunc :: = FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar * block END
bool defFunc() {
	//printf("#defFunc %d\n", tokens[iTk].code);
	int star = iTk;
	if (!consume(FUNCTION)) {
		iTk = star; //restaurare
		return false;
	}
	else {
		if (!consume(ID)) {
			tkerr("Eroare: Lipseste denumirea functiei numele 'function'");
		}

		////analiza de domeniu
		const char* name = consumed->text;
		Symbol* s = searchInCurrentDomain(name);
		if (s)tkerr("Simbol redefinit: %s", name);
		crtFn = addSymbol(name, KIND_FN);
		crtFn->args = NULL;
		addDomain();

		if (!consume(LPAR)) {
			tkerr("Eroare: Lipseste '(' dupa numele functiei");
		}
		funcParams(); //optional ?
		if (!consume(RPAR)) {
			tkerr("Eroare: Lipseste ')' dupa parametrii functiei");
		}
		if (!consume(COLON)) {
			tkerr("Eroare: Lipseste ':' dupa parametrii functiei");
		}
		if (!baseType()) {
			tkerr("Eroare: Tipul functiei invalid, rebuie sa fie int, real sau str");
		}

		////analiza de domeniu
		crtFn->type = ret.type;

		while (defVar()) {} //repetam de 0 sau mai multe ori
		if (!block()) {
			tkerr("Eroare: Blocul functiei lipseste");
		}
		if (!consume(END)) {
			tkerr("Eroare: Lipseste 'END' la finalul functiei");
		}
	}

	////analiza de domeniu
	delDomain();
	crtFn = NULL;

	return true;
}

//block :: = instr +
bool block() {
	//printf("#block %d\n", tokens[iTk].code);
	int star = iTk;

	if (!instr()) { 
		iTk = star; //restaurare
		return false;
	}

	while(instr()){}

	return true;	
}	


//funcParams :: = funcParam(COMMA funcParam)* * --> SE REPETA DE LA 0 LA oo
bool funcParams() {
	//printf("#funcParams %d\n", tokens[iTk].code);
	int star = iTk;

	if (!funcParam()) {
		tkerr("Eroare: Trebuie sa existe cel putin 1 parametru in functie\n");
		iTk = star;
		return false;
	}

	while (consume(COMMA)) {
		if (!funcParam()) {
			tkerr("Eroare: Lipseste parametru dupa ','");
			iTk = star; //restaurare
			return false;
		}
	}

	return true;
}

//funcParam :: = ID COLON baseType
bool funcParam() {
	//printf("#funcParam %d\n", tokens[iTk].code);
	int star = iTk;
	if (!consume(ID)) {
		tkerr("Eroare: Lipseste identificatorul parametrului");
		iTk = star; //restaurare
		return false;
	}

	////analiza de domeniu
	const char* name = consumed->text;
	Symbol* s = searchInCurrentDomain(name);
	if (s)tkerr("Simbol redefinit: %s", name);
	s = addSymbol(name, KIND_ARG);
	Symbol* sFnParam = addFnArg(crtFn, name);

	if (!consume(COLON)) {
		tkerr("Eroare: Lipseste ':' dupa numele parametrului");
		iTk = star; //restaurare
		return false;
	}
	

	if (!baseType()) {
		tkerr("Eroare: Tip invalid pentru parametrul functiei");
		iTk = star; //restaurare
		return false;
	}
	////analiza de domeniu
	s->type = ret.type;
	sFnParam->type = ret.type;

	return true;
}



//instr :: = expr ? SEMICOLON
//| IF LPAR expr RPAR block(ELSE block) ? END
//| RETURN expr SEMICOLON
//| WHILE LPAR expr RPAR block END
bool instr() {
	//printf("#instr %d\n", tokens[iTk].code);
	int star = iTk;

	//| IF LPAR expr RPAR block(ELSE block) ? END
	if (consume(IF)) {
		if (!consume(LPAR)) {
			tkerr("Eroare: Lipseste '(' dupa IF");
		}

		if (!expr()) {
			tkerr("Eroare: Lipseste expresia dupa IF");
		}

		if (!consume(RPAR)) {
			tkerr("Eroare: Lipseste ')' dupa IF");
		}

		if (!block()) {
			tkerr("Eroare: Lipseste blocul de instructiune dupa dupa IF");
		}


		//optional -> (ELSE block) ?
		if (consume(ELSE)) {
			if (!block()) {
				tkerr("Eroare: Lipseste blocul de instructiune dupa IF");
			}
		}

		if (!consume(END)) {
			tkerr("Eroare: Lipseste END dupa IF");
		}
		return true;
	}

	//| RETURN expr SEMICOLON
	if (consume(RETURN)) {
		if (!expr()) {
			tkerr("Eroare: Lipseste expresie dupa RETURN");
		}
		if (!consume(SEMICOLON)) {
			tkerr("Eroare: Lipseste ';' dupa RETURN");
		}
		return true;
	}


	//| WHILE LPAR expr RPAR block END
	else if (consume(WHILE)) {
		if (!consume(LPAR)) {
			tkerr("Eroare: Lipseste '(' dupa WHILE");
		}
		if (!expr()) {
			tkerr("Eroare: Lipseste expresia dupa WHILE");
		}
		if (!consume(RPAR)) {
			tkerr("Eroare: Lipseste ')' dupa WHILE");
		}
		if (!block()) {
			tkerr("Eroare: Lipseste blocul de instructiune dupa WHILE");
		}
		if (!consume(END)) {
			tkerr("Eroare: Lipseste 'END' dupa WHILE");
		}
		return true;
	}


	//instr :: = expr ? SEMICOLON (ultimul caz, optional)
	if (expr()) {
		if (!consume(SEMICOLON)) {
			tkerr("Eroare: Lipseste ';' dupa expresie");
		}
		
		return true;
	}

	iTk = star; //restaurare finala
	return false;
}


//expr ::= exprLogic
bool expr() {
	//printf("#expr %d\n", tokens[iTk].code);
	int star = iTk; //salvare pozitie initiala

	if (!exprLogic()) {
		iTk = star; //restaurare finala
		return false;
	}
	
	return true;
}

//exprLogic :: = exprAssign((AND | OR) exprAssign) * --> de 0 sau mai mule ori
bool exprLogic() {
	//printf("#exprLogic %d\n", tokens[iTk].code);
	int star = iTk; //salvare pozitie initiala

	if (!exprAssign()) {
		iTk = star; //restaurare finala
		return false;
	}
		while (1) {
			if (consume(AND)) {
				if (!exprAssign()) {
					tkerr("ELipseste expresia dupa operatorul AND");
					//iTk = star; //restaurare finala
					//return false;
				}
			}
			else if (consume(OR)) {
				if (!exprAssign()) {
					tkerr("ELipseste expresia dupa operatorul OR");
					//iTk = star; //restaurare
					//return false;
				}
			}
			else {
				break;
			}
		}
	return true;
}


//exprAssign :: = (ID ASSIGN) ? exprComp
bool exprAssign() {
	//printf("#exprAssign %d\n", tokens[iTk].code);
	int star = iTk;
	
	//(ID ASSIGN) ? --> asta este optionala
	if (consume(ID)) {
		if (consume(ASSIGN)) {
			if (!exprComp()) {
				tkerr("Eroare: Lipseste expresia dupa '='");
				iTk = star;  // restaurăm poziția
				return false;
			}
			return true;
		}
		else {
			iTk = star; //restaurare		
		}
	}

	if (!exprComp()) {
		return false;
	}

	return true;
}


//exprComp :: = exprAdd((LESS | EQUAL) exprAdd) ?
bool exprComp() {
	//printf("#exprComp %d\n", tokens[iTk].code);
	int star = iTk; //salvare pozitie initiala

	if (!exprAdd()) {
		iTk = star; //restaurare
		return false;
	}

	if (consume(LESS) || consume(EQUAL)) {
		if (!exprAdd()) {
			tkerr("Lipseste operandul dupa operatorul de comparatie");
			iTk = star; //restaurare
			return false;
		}
	}

	return true;
}

//exprAdd :: = exprMul((ADD | SUB) exprMul) * --> 0 sau mai multe ori
bool exprAdd() {
	//printf("#exprAdd %d\n", tokens[iTk].code);
	int star = iTk; //salvare pozitie initiala

	if (!exprMul()) {
		iTk = star; //restaurare
		return false;
	}

	while (1) {
		if ((consume(ADD)) || consume(SUB)) {
			if (!exprMul()) {
				tkerr("Lipseste operandul dupa operatorul '+' sau '-'");
				iTk = star; //restaurare
				return false;
			}
		}
		else {
			break;
		}
	}
	return true;
}

//exprMul :: = exprPrefix((MUL | DIV) exprPrefix) *
bool exprMul() {
	//printf("#exprMul %d\n", tokens[iTk].code);
	int star = iTk; //salvare pozitie initiala

	if (!exprPrefix()) {
		iTk = star; //restaurare
		return false;
	}

	while (1) {
		if (consume(MUL) || consume(DIV)) {
			if (!exprPrefix()) {
				tkerr("Lipseste operandul dupa operatorul '*' sau '/'");
				iTk = star; //restaurare
				return false;
			}
		}
		else {
			break;
		}
	}

	return true;
}

//exprPrefix :: = (SUB | NOT) ? factor
bool exprPrefix() {
	//printf("#exprPrefix %d\n", tokens[iTk].code);
	int star = iTk; //salvare pozitie initiala

	if (consume(SUB) || consume(NOT)){
		if (!factor()) {
			tkerr("Eroare: Factor invalid dupa SUB sau NOT");
			iTk = star; //restaurare
			return false;
		}
		return true;
	}

	if (!factor()) {
		return false;
	}

	return true;
}

//factor :: = INT
//| REAL
//| STR
//| LPAR expr RPAR
//| ID(LPAR(expr(COMMA expr)*) ? RPAR) ?
bool factor() {
	//printf("#factor %d\n", tokens[iTk].code);
	int star = iTk; //salvare pozitie initiala

	if (consume(INT) || consume(REAL) || consume(STR)) {
		return true;
	}

	
	if (consume(LPAR)) {
		if (!expr()) {
			tkerr("Expresie lipsa dupa '('");
		}
		if (!consume(RPAR)) {
			tkerr("Expresie lipsa dupa ')'");
		}
		return true;
	}
	
	if (consume(ID)) {
		if (consume(LPAR)) {
			if (expr()) {
				while (consume(COMMA)) {
					if (!expr()) {
						tkerr("Lipseste expresie dupa ',' in apelul de functie");
					}
				}
			}

			if (!consume(RPAR)) {
				tkerr("Lipseste ')' la sfarsitul apelului de functie");
			}
		}
		return true;
	}

	iTk = star; //restaurare
	return false;
}



// program ::= ( defVar | defFunc | block )* FINISH
bool program(){
	////analiza de domeniu
	addDomain(); // creates the global domain
	

	for(;;){
		
		if(defVar()){}
		else if(defFunc()){}
		else if(block()){}
		else break;
		}
	if(consume(FINISH)){
	
		}else tkerr("syntax error");

	////analiza de domeniu
	delDomain();

	return true;

	}

void parse(){
	iTk=0;
	program();
	}
