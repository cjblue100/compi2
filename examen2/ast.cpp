#include "ast.h"
#include <iostream>
#include <sstream>
#include <set>
#include "asm.h"

const char *floatTemps[] = {"$f0",
                            "$f1",
                            "$f2",
                            "$f3",
                            "$f4",
                            "$f5",
                            "$f6",
                            "$f7",
                            "$f8",
                            "$f9",
                            "$f10",
                            "$f11",
                            "$f12",
                            "$f13",
                            "$f14",
                            "$f15",
                            "$f16",
                            "$f17",
                            "$f18",
                            "$f19",
                            "$f20",
                            "$f21",
                            "$f22",
                            "$f23",
                            "$f24",
                            "$f25",
                            "$f26",
                            "$f27",
                            "$f28",
                            "$f29",
                            "$f30",
                            "$f31"};

#define FLOAT_TEMP_COUNT 32
set<string> intTempMap;
set<string> floatTempMap;

class CodeGenerationVariableInfo
{
public:
    CodeGenerationVariableInfo(int offset, bool isParameter)
    {
        this->offset = offset;
        
        this->isParameter = isParameter;
   
    }
    int offset;
    bool isArray;
    bool isParameter;

};
class Parameter{
    public:
        Parameter(string declarator, int line){
            
            this->declarator = declarator;
            this->line = line;
        }
       
        string declarator;
        int line;
        int evaluateSemantic();
};

class MethodInfo
{
public:
    
    list<Parameter *> parameters;
};


map<string, CodeGenerationVariableInfo *> codeGenerationVars = {};

extern Asm assemblyFile;

int globalStackPointer = 0;

int labelCounter = 0;
string newLabel(string prefix)
{
    stringstream ss;
    ss << prefix << "_" << labelCounter;
    labelCounter++;
    return ss.str();
}

string getFloatTemp()
{
    for (int i = 0; i < FLOAT_TEMP_COUNT; i++)
    {
        if (floatTempMap.find(floatTemps[i]) == floatTempMap.end())
        {
            floatTempMap.insert(floatTemps[i]);
            return string(floatTemps[i]);
        }
    }
    cout << "No more float registers!" << endl;
    return "";
}

void releaseFloatTemp(string temp)
{
    floatTempMap.erase(temp);
}

void FloatExpr::genCode(Code &context)
{
    stringstream code;
    string tempReg = getFloatTemp();
    code << "li.s " << tempReg << ", " << this->number << endl;
    context.code = code.str();
    context.place = tempReg;
}

void SubExpr::genCode(Code &code)
{
    stringstream ss;
    string temp = getFloatTemp();
    Code expr1;
    Code expr2;
    this->expr1->genCode(expr1);
    this->expr2->genCode(expr2);

    ss << "sub.s " << temp << ", " << expr1.place << ", " << expr2.place << endl;
    releaseFloatTemp(expr1.place);
    releaseFloatTemp(expr2.place);
    code.code = ss.str();
}

void DivExpr::genCode(Code &code)
{
    stringstream ss;
    string temp = getFloatTemp();
    Code expr1;
    Code expr2;
    this->expr1->genCode(expr1);
    this->expr2->genCode(expr2);

    ss << "div.s " << temp << ", " << expr1.place << ", " << expr2.place<<endl;
    releaseFloatTemp(expr1.place);
    releaseFloatTemp(expr2.place);

    code.code = ss.str();
}

void IdExpr::genCode(Code &code)
{
    if (codeGenerationVars.find(this->id) == codeGenerationVars.end())
    {

        string temp = getFloatTemp();
        code.code = "l.s " + temp + ", " + this->id + "\n";
        code.place = temp;
    }
    else
    {

        string floatTemp = getFloatTemp();
        code.place = floatTemp;
        code.code = "l.s " + floatTemp + ", " + to_string(codeGenerationVars[this->id]->offset) + "($sp)\n";
    }
}

string ExprStatement::genCode()
{
    Code code;
    this->expr->genCode(code);
    releaseFloatTemp(code.place);
    return code.code;
 
}

string IfStatement::genCode()
{
    Code code;
    stringstream codigo;
    this->conditionalExpr->genCode(code);
    releaseFloatTemp(code.place);
    string endLabel = newLabel("else");
    string elseLabel = newLabel("endIf");

    // x>5;
    codigo << code.code << endl
           << "bc1f "
           << ", " << elseLabel << endl;
    // codigo de statment
    // j exit
    list<Statement *>::iterator truestat = this->trueStatement.begin();

    while (truestat != this->trueStatement.end())
    {
        codigo << (*truestat)->genCode() << endl;

        truestat++;
    }
    codigo << " j " << endLabel << endl;
    // else:
    codigo << elseLabel << ": " << endl;
    list<Statement *>::iterator falsestat = this->falseStatement.begin();
    // codigo statement
    while (falsestat != this->falseStatement.end())
    {
        codigo << (*falsestat)->genCode() << endl;

        falsestat++;
    }
    codigo << " j " << endLabel << endl;

    // endlabel:
    codigo << endLabel << ": " << endl;

    return codigo.str();
}

map<string, MethodInfo *> methods = {};

void MethodInvocationExpr::genCode(Code &context)
{
    list<Expr *>::iterator it = this->expressions.begin();
    list<Code> codes;
    stringstream ss;
    Code argCode;
    while (it != this->expressions.end())
    {
        (*it)->genCode(argCode);
        ss << argCode.code << endl;
        codes.push_back(argCode);
        it++;
    }

    int i = 0;
    list<Code>::iterator codeIt = codes.begin();
    while (codeIt != codes.end())
    {
        releaseFloatTemp((*codeIt).place);
       
        ss << "mfc1 $a" << i << ", " << (*codeIt).place << endl;
        
        i++;
        codeIt++;
    }

    ss << "jal " <<this->id << endl;
    string result;
    
    result = getFloatTemp();
    ss << "mtc1 $v0, " << result << endl;
    
   
    context.code = ss.str();
    context.place = result;
   
}

string AssignationStatement::genCode()
{

    Code expr;
    stringstream ss;
    this->value->genCode(expr);
    ss << expr.code << endl;
    string name = ((IdExpr *)this->value)->id;

   
       

       
    releaseFloatTemp(expr.place);
    
    return ss.str();
}

void GteExpr::genCode(Code &code)
{
    stringstream ss;
    Code expr1;
    Code expr2;
    this->expr1->genCode(expr1);
    this->expr2->genCode(expr2);

    ss << expr1.code << endl
       << expr2.code << endl;
    releaseFloatTemp(expr1.place);
    releaseFloatTemp(expr2.place);

    ss << "c.le.s " << expr2.place << ", " << expr1.place << endl;

    code.code = ss.str();
}

void LteExpr::genCode(Code &code)
{

    stringstream ss;

    Code expr1;
    Code expr2;
    this->expr1->genCode(expr1);
    this->expr2->genCode(expr2);

    ss << expr1.code << endl
       << expr2.code << endl;
    releaseFloatTemp(expr1.place);
    releaseFloatTemp(expr2.place);

    ss << "c.le.s " << expr1.place << ", " << expr2.place << endl;

    code.code = ss.str();
}

void EqExpr::genCode(Code &code)
{
    stringstream ss;

    Code expr1;
    Code expr2;
    this->expr1->genCode(expr1);
    this->expr2->genCode(expr2);

    ss << expr1.code << endl
       << expr2.code << endl;
    releaseFloatTemp(expr1.place);
    releaseFloatTemp(expr2.place);

    ss << "c.eq.s " << expr1.place << ", " << expr2.place << endl;

    code.code = ss.str();
}

void ReadFloatExpr::genCode(Code &code)
{
}

string PrintStatement::genCode()
{
    return "Print statement code generation\n";
}

string ReturnStatement::genCode()
{
    Code exprCode;
    this->expr->genCode(exprCode);
    releaseFloatTemp(exprCode.code);
    stringstream code;
    code << exprCode.code << endl;
    
    code << "mfc1 $v0, " << exprCode.place << endl;
    

    return code.str();
}
string saveState()
{
    stringstream ss;
    ss << "sw $ra, " << globalStackPointer << "($sp)" << endl;
    globalStackPointer += 4;
    return ss.str();
}
string retrieveState(string state)
{
    std::string::size_type n = 0;
    string originalStatement = "sw";
    while ((n = state.find(originalStatement, n)) != std::string::npos)
    {
        state.replace(n, originalStatement.size(), "lw");
        n += originalStatement.size();
    }
    return state;
}


string MethodDefinitionStatement::genCode()
{
    

    int stackPointer = 4;
    globalStackPointer = 0;
    stringstream code;
    code << this->id << ": " << endl;
    string state = saveState();
    code << state << endl;

    if (this->params.size() > 0)
    {
        list<string >::iterator it = this->params.begin();
        for (int i = 0; i < this->params.size(); i++)
        {
            code << "sw $a" << i << ", " << stackPointer << "($sp)" << endl;
            codeGenerationVars[(*it)] =
                new CodeGenerationVariableInfo(stackPointer,  true);
            
            stackPointer += 4;
            globalStackPointer += 4;
            it++;
        }
    }
    list<Statement* >::iterator it = this->stmts.begin();
    while(it != this->stmts.end())
    {
        code << (*it)->genCode() << endl;
        it++;
    }
    
    int currentStackPointer = globalStackPointer;
    stringstream sp;
    sp << endl
       << "addiu $sp, $sp, -" << currentStackPointer << endl;
    code << retrieveState(state) << endl
         << "addiu $sp, $sp, " << currentStackPointer << endl
         << "jr $ra" << endl;
    codeGenerationVars.clear();
    string result = code.str();
    result.insert(this->id.size() + 2, sp.str());
    return result;
}



