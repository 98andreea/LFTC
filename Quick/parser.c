// Andreea Rosu
// Informatica
// Anul III, 2025
// ACTIVITATE 3 --> NOTA 9
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include "lexer.h"
#include "ad.h"

//lab 6
#include "gen.h"

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

int iTk;    // the iterator in tokens
Token* consumed;    // the last consumed token

// same as err, but also prints the line of the current token
void tkerr(const char* fmt, ...) {
    fprintf(stderr, "error in line %d: ", tokens[iTk].line);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool consume(int code) {
    if (tokens[iTk].code == code) {
        consumed = &tokens[iTk++];
        return true;
    }
    return false;
}

// lab 5
//void setRet(int type, bool lval) {
//    ret.type = type;
//    ret.lval = lval;
//}

// baseType ::= TYPE_INT | TYPE_REAL | TYPE_STR
bool baseType() {
    if (consume(TYPE_INT)) {
        ret.type = TYPE_INT;
        return true;
    }
    if (consume(TYPE_REAL)) {
        ret.type = TYPE_REAL;
        return true;
    }
    if (consume(TYPE_STR)) {
        ret.type = TYPE_STR;
        return true;
    }
    return false;
}

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
    int star = iTk;
    Symbol* s = NULL;

    const char* name = NULL;

    if (!consume(VAR)) {
        iTk = star;
        return false;
    }
    else {
        if (!consume(ID)) {
            tkerr("Eroare: Lipseste identificator dupa variabila var");
        }

        {
            name = consumed->text;
            s = searchInCurrentDomain(name);
            if (s) tkerr("symbol redefinition: %s", name);
            s = addSymbol(name, KIND_VAR);
            s->local = (crtFn != NULL);
        }

        if (!consume(COLON)) {
            tkerr("Eroare: Lipseste ':' dupa numele variabilei");
        }
        if (!baseType()) {
            tkerr("Eroare: Tip invalid pentru variabila");
        }

        if (s) {
            s->type = ret.type;
        }

        if (!consume(SEMICOLON)) {
            tkerr("Eroare: Lipseste ';' dupa numele variabilei");
        }
    }

    //lab 6
    Text_write(crtVar, "%s %s;\n", cType(ret.type), name);

    return true;
}

// defFunc ::= FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar* block END
bool defFunc() {
    int star = iTk;
    if (!consume(FUNCTION)) {
        iTk = star;
        return false;
    }
    else {
        if (!consume(ID)) {
            tkerr("Eroare: Lipseste denumirea functiei");
        }

        const char* name = consumed->text;

        //lab 6
        crtCode = &tFunctions;
        crtVar = &tFunctions;
        Text_clear(&tFnHeader);
        Text_write(&tFnHeader, "%s(", name);

        Symbol* s = searchInCurrentDomain(name);
        if (s) tkerr("Simbol redefinit: %s", name);
        crtFn = addSymbol(name, KIND_FN);
        crtFn->args = NULL;
        addDomain();

        if (!consume(LPAR)) {
            tkerr("Eroare: Lipseste '(' dupa numele functiei");
        }
        funcParams();
        if (!consume(RPAR)) {
            tkerr("Eroare: Lipseste ')' dupa parametrii functiei");
        }
        if (!consume(COLON)) {
            tkerr("Eroare: Lipseste ':' dupa parametrii functiei");
        }
        if (!baseType()) {
            tkerr("Eroare: Tipul functiei invalid");
        }

        crtFn->type = ret.type;

        //lab 6
        Text_write(&tFunctions, "\n%s %s){\n", cType(ret.type), tFnHeader.buf);

        while (defVar()) {}
        if (!block()) {
            tkerr("Eroare: Blocul functiei lipseste");
        }
        if (!consume(END)) {
            tkerr("Eroare: Lipseste 'END' la finalul functiei");
        }
    }

    //lab 6
    Text_write(&tFunctions, "}\n");
    crtCode = &tMain;

    crtVar = &tBegin;
    delDomain();
    crtFn = NULL;
    return true;
}

bool block() {
    int star = iTk;
    if (!instr()) {
        iTk = star;
        return false;
    }
    while (instr()) {}
    return true;
}

bool funcParams() {
    int star = iTk;
    if (!funcParam()) {
        iTk = star;
        return false;
    }
    while (consume(COMMA)) {
        if (!funcParam()) {
            //lab 6
            Text_write(&tFnHeader, ", ");

            tkerr("Eroare: Lipseste parametru dupa ','");
        }
    }
    return true;
}

bool funcParam() {
    int star = iTk;
    if (!consume(ID)) {
        iTk = star;
        return false;
    }

    const char* name = consumed->text;
    Symbol* s = searchInCurrentDomain(name);
    if (s) tkerr("Simbol redefinit: %s", name);
    s = addSymbol(name, KIND_ARG);
    Symbol* sFnParam = addFnArg(crtFn, name);

    if (!consume(COLON)) {
        tkerr("Eroare: Lipseste ':' dupa numele parametrului");
    }

    if (!baseType()) {
        tkerr("Eroare: Tip invalid pentru parametrul functiei");
    }
    s->type = ret.type;
    sFnParam->type = ret.type;


    //lab 6
    Text_write(&tFnHeader, "%s %s", cType(ret.type), name);

    return true;
}

bool instr() {
    int star = iTk;

    if (consume(IF)) {
        //lab 6
        Text_write(crtCode, "if(");

        if (!consume(LPAR)) tkerr("Eroare: Lipseste '(' dupa IF");
        if (!expr()) tkerr("Eroare: Lipseste expresia dupa IF");

        // lab 5
        if (ret.type == TYPE_STR) tkerr("the if condition must have type int or real");

        if (!consume(RPAR)) tkerr("Eroare: Lipseste ')' dupa IF");
        
        //lab 6
        Text_write(crtCode, "){\n");

        if (!block()) tkerr("Eroare: Lipseste blocul dupa IF");

        //lab 6
        Text_write(crtCode, ")}\n");

        if (consume(ELSE)) {
            //lab 6
            Text_write(crtCode, "){\n");

            if (!block()) tkerr("Eroare: Lipseste blocul dupa ELSE");

            //lab 6
            Text_write(crtCode, ")}\n");
        }
        if (!consume(END)) tkerr("Eroare: Lipseste END dupa IF");
        return true;
    }

    if (consume(RETURN)) {
        //lab 6
        Text_write(crtCode, "return ");

        if (!expr()) tkerr("Eroare: Lipseste expresie dupa RETURN");

        // lab 5
        if (!crtFn) tkerr("return can be used only in a function");
        if (ret.type != crtFn->type) tkerr("the return type must be the same as function return type");

        if (!consume(SEMICOLON)) tkerr("Eroare: Lipseste ';' dupa RETURN");

        //lab 6 
        Text_write(crtCode, ";\n");

        return true;
    }

    if (consume(WHILE)) {
        //lab 6
        Text_write(crtCode, "while(");

        if (!consume(LPAR)) tkerr("Eroare: Lipseste '(' dupa WHILE");
        if (!expr()) tkerr("Eroare: Lipseste expresia dupa WHILE");

        // lab 5
        if (ret.type == TYPE_STR) tkerr("the while condition must have type int or real");

        if (!consume(RPAR)) tkerr("Eroare: Lipseste ')' dupa WHILE");

        //lab 6
        Text_write(crtCode, "){\n");

        if (!block()) tkerr("Eroare: Lipseste blocul dupa WHILE");
        if (!consume(END)) tkerr("Eroare: Lipseste 'END' dupa WHILE");

        //lab 6
        Text_write(crtCode, ")}\n");

        return true;
    }

    if (expr()) {
        if (!consume(SEMICOLON)) tkerr("Eroare: Lipseste ';' dupa expresie");

        //lab 6
        Text_write(crtCode, ";\n");

        return true;
    }

    iTk = star;
    return false;
}

bool expr() {
    return exprLogic();
}

bool exprLogic() {
    if (!exprAssign()) return false;
    while (1) {

        // lab 5
        Ret leftType = ret;

        if (consume(AND) || consume(OR)) {
            //lab 6
            int op = consumed->code;
            Text_write(crtCode, (op == AND) ? " && " : " || ");

            //lab 5
            if (leftType.type == TYPE_STR) tkerr("the left operand of && or || cannot be str");

            if (!exprAssign()) tkerr("Lipseste expresia dupa operatorul logic");

            // lab 5
            if (ret.type == TYPE_STR) tkerr("the right operand of && or || cannot be str");
            setRet(TYPE_INT, false);
        }
        else break;
    }
    return true;
}

bool exprAssign() {
    int star = iTk;
    if (consume(ID)) {
        const char* name = consumed->text;
        if (consume(ASSIGN)) {

            //lab 6
            Text_write(crtCode, "%s = ", name);

            if (!exprComp()) {
                tkerr("Eroare: Lipseste expresia dupa '='");
            }

            // lab 5
            Symbol* s = searchSymbol(name);
            if (!s) tkerr("undefined symbol: %s", name);
            if (s->kind == KIND_FN) tkerr("a function cannot be used as destination for assignment");
            if (s->type != ret.type) tkerr("source and destination for assignment must have same type");

            setRet(ret.type, false);
            return true;
        }
        else iTk = star;
    }
    return exprComp();
}

bool exprComp() {
    if (!exprAdd()) return false;
    if (consume(LESS) || consume(EQUAL)) {
        //lab 6
        int op = consumed->code;
        Text_write(crtCode, (op == LESS) ? " < " : " == ");

        // lab 5
        Ret leftType = ret;

        if (!exprAdd()) tkerr("Lipseste operandul dupa operatorul de comparatie");

        // lab 5
        if (leftType.type != ret.type) tkerr("different types for comparison");
        setRet(TYPE_INT, false);
    }
    return true;
}

bool exprAdd() {
    if (!exprMul()) return false;
    while (1) {
        if (consume(ADD) || consume(SUB)) {
            //lab 6
            int op = consumed->code;
            Text_write(crtCode, (op == ADD) ? " + " : " - ");

            // lab 5
            Ret leftType = ret;
            if (leftType.type == TYPE_STR) tkerr("operands of + or - cannot be str");

            if (!exprMul()) tkerr("Lipseste operandul dupa '+' sau '-'");

            // lab 5
            if (leftType.type != ret.type) tkerr("different types for + or -");
            setRet(leftType.type, false);
        }
        else break;
    }
    return true;
}

bool exprMul() {
    if (!exprPrefix()) return false;
    while (1) {
        if (consume(MUL) || consume(DIV)) {
            //lab 6
            int op = consumed->code;
            Text_write(crtCode, (op == MUL) ? " * " : " / ");

            // lab 5
            Ret leftType = ret;
            if (leftType.type == TYPE_STR) tkerr("operands of * or / cannot be str");

            if (!exprPrefix()) tkerr("Lipseste operandul dupa '*' sau '/'");

            // lab 5
            if (leftType.type != ret.type) tkerr("different types for * or /");
            setRet(leftType.type, false);
        }
        else break;
    }
    return true;
}

bool exprPrefix() {
    if (consume(SUB)) {
        //lab 6
        Text_write(crtCode, "-");

        if (!factor()) tkerr("Factor invalid");

        // lab 5
        if (ret.type == TYPE_STR) tkerr("unary - must be int or real");
        setRet(ret.type, false);

        return true;
    }
    if (consume(NOT)) {
        //lab 6
        Text_write(crtCode, "!");

        if (!factor()) tkerr("Factor invalid");

        // lab 5
        if (ret.type == TYPE_STR) tkerr("! must be int or real");
        setRet(TYPE_INT, false);

        return true;
    }
    return factor();
}

bool factor() {
    int star = iTk;
    // lab 5
    if (consume(INT)) {
        //lab 6
        Text_write(crtCode, "%d", consumed->i);

        setRet(TYPE_INT, false); 
        return true; 
    }
    if (consume(REAL)) { 
        //lab 6
        Text_write(crtCode, "%g", consumed->r);

        setRet(TYPE_REAL, false); 
        return true; 
    }
    if (consume(STR)) { 
        //lab 6
        Text_write(crtCode, "\"%s\"", consumed->text);

        setRet(TYPE_STR, false); 
        return true; 
    }

    if (consume(LPAR)) {
        //lab 6
        Text_write(crtCode, "(");

        if (!expr()) tkerr("Expresie lipsa dupa '('");
        if (!consume(RPAR)) tkerr("Lipseste ')'");

        //lab 6
        Text_write(crtCode, ")");

        return true;
    }

    if (consume(ID)) {
        //lab 6
        const char* name = consumed->text;

        // lab 5
        Symbol* s = searchSymbol(consumed->text);
        if (!s) tkerr("undefined symbol: %s", consumed->text);

        //lab 6
        Text_write(crtCode, "%s", name);

        if (consume(LPAR)) {
            //lab 6
            Text_write(crtCode, "(");

            // lab 5
            if (s->kind != KIND_FN) tkerr("%s is not a function", s->name);
            Symbol* argDef = s->args;

            if (expr()) {
                // lab 5
                if (!argDef) tkerr("too many arguments for %s", s->name);
                if (argDef->type != ret.type) tkerr("argument type mismatch");
                argDef = argDef->next;

                while (consume(COMMA)) {
                    //lab 6
                    Text_write(crtCode, ", ");

                    if (!expr()) tkerr("Lipseste expresie dupa ','");

                    // lab 5
                    if (!argDef) tkerr("too many arguments");
                    if (argDef->type != ret.type) tkerr("argument type mismatch");
                    argDef = argDef->next;
                }
            }

            // lab 5
            if (argDef) tkerr("too few arguments for %s", s->name);

            if (!consume(RPAR)) tkerr("Lipseste ')' la apelul functiei");

            //lab 6
            Text_write(crtCode, ")");

            // lab 5
            setRet(s->type, false);
        }
        // lab 5
        else {
            if (s->kind == KIND_FN) tkerr("function %s can only be called", s->name);
            setRet(s->type, true);
        }
        return true;
    }

    iTk = star;
    return false;
}

bool program() {
    //lab 6
    crtCode = &tMain;
    crtVar = &tBegin;
    Text_write(&tBegin, "#include \"quick.h\"\n\n");
    Text_write(&tMain, "\nint main(){\n");

    addDomain();

    // lab 5
    addPredefinedFns();

    while (1) {
        if (defVar()) {}
        else if (defFunc()) {}
        else if (block()) {}
        else break;
    }
    if (!consume(FINISH)) tkerr("syntax error");
    delDomain();
    return true;
}

void parse() {
    iTk = 0;
    program();
}