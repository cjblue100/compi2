const char *intTemps[] = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9"};

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

#define INT_TEMP_COUNT 10
#define FLOAT_TEMP_COUNT 32
set<string> intTempMap;
set<string> floatTempMap;

string getIntTemp()
{
    for (int i = 0; i < INT_TEMP_COUNT; i++)
    {
        if (intTempMap.find(intTemps[i]) == intTempMap.end())
        {
            intTempMap.insert(intTemps[i]);
            return string(intTemps[i]);
        }
    }
    return "";
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
    return "";
}

void releaseRegister(string temp)
{
    intTempMap.erase(temp);
    floatTempMap.erase(temp);
}

void toFloat(Code &context)
{
    if (context.type == INT)
    {
        string floatTemp = getFloatTemp();
        stringstream code;
        code << context.code
             << "mtc1 " << context.place << ", " << floatTemp << endl
             << "cvt.s.w " << floatTemp << ", " << floatTemp << endl;
        releaseRegister(context.place);
        context.place = floatTemp;
        context.type = FLOAT;
        context.code = code.str();
    }
}

int labelCounter = 0;
string newLabel(string prefix)
{
    stringstream ss;
    ss << prefix << "_" << labelCounter;
    labelCounter++;
    return ss.str();
}

map<string, CodeGenerationVariableInfo *> codeGenerationVars = {};
class CodeGenerationVariableInfo
{
public:
    CodeGenerationVariableInfo(int offset, bool isArray, bool isParameter, Type type)
    {
        this->offset = offset;
        this->isArray = isArray;
        this->isParameter = isParameter;
        this->type = type;
    }
    int offset;
    bool isArray;
    bool isParameter;
    Type type;
};