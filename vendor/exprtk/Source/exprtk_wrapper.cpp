#include "exprtk_wrapper.h"
#include "exprtk.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>

struct DoubleValue{
    std::string varName;
    double value;
};

struct StringValue{
    std::string varName;
    std::string value;
};

struct VectorValue{
    std::string varName;
    std::vector<double> value;
};

struct  ExprtkStruct
{
    std::vector<std::string> exprString;
    std::vector<DoubleValue*> doubleValue;
    std::vector<StringValue*> stringValue;
    std::vector<VectorValue*> vectorValue;

    exprtk::symbol_table<double> symbolTable;
    std::vector<exprtk::expression<double>> expression;
    exprtk::parser<double> parser;
};

exprtkWrapper exprtk_wrapper_init()
{
    ExprtkStruct* obj = new ExprtkStruct();
    return (void*)obj;
}

void set_expression_count(exprtkWrapper obj, unsigned int count)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    exprtkStruct->exprString.resize(count);
    exprtkStruct->expression.resize(count);
}

void set_expression_string(exprtkWrapper obj, const char* expr, unsigned int idx)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    assert(idx < exprtkStruct->exprString.size());
    exprtkStruct->exprString[idx] = std::string(expr);
}

const char* get_expression_string(exprtkWrapper obj, unsigned int idx)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    assert(idx < exprtkStruct->exprString.size());
    return exprtkStruct->exprString[idx].c_str();
}

int compile_expression(exprtkWrapper obj)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;

    exprtkStruct->symbolTable.add_constants();

    for (int i= 0; i < exprtkStruct->expression.size(); ++i)
    {
        exprtkStruct->expression[i].register_symbol_table(exprtkStruct->symbolTable);
        if (!exprtkStruct->parser.compile(exprtkStruct->exprString[i], exprtkStruct->expression[i]))
        {
            return 0;
        }
    }

    return 1;
}

void set_double_variable_value(exprtkWrapper obj, const char* var, double val)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    std::string varName = std::string(var);
    for (int i = 0; i < exprtkStruct->doubleValue.size();i++)
    {
        if (varName.compare(exprtkStruct->doubleValue[i]->varName) == 0)
        {
            exprtkStruct->doubleValue[i]->value = val;
            break;
        }
    }
}

void set_string_variable_value(exprtkWrapper obj, const char* var, const char* val)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    std::string varName = std::string(var);
    std::string value = std::string(val);
     for (int i = 0; i < exprtkStruct->stringValue.size();i++)
     {
        if (varName.compare(exprtkStruct->stringValue[i]->varName) == 0)
        {
            exprtkStruct->stringValue[i]->value = value;
            break;
        }
    }
}

void set_vector_variable_value(exprtkWrapper obj, const char* var, double val[], int len)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    std::string varName = std::string(var);
    for (int i = 0; i < exprtkStruct->vectorValue.size();i++)
    {
        if (varName.compare(exprtkStruct->vectorValue[i]->varName) == 0)
        {
            exprtkStruct->vectorValue[i]->value.clear();
            for (int j = 0; j < len; j++)
            {
                exprtkStruct->vectorValue[i]->value.push_back(double(val[j]));
            }
            break;
        }
    }
}
  

double get_evaluated_value(exprtkWrapper obj, unsigned int idx)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    assert(idx < exprtkStruct->expression.size());
    return exprtkStruct->expression[idx].value();
}

void add_double_variable(exprtkWrapper obj, const char* var)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    DoubleValue *d = new(DoubleValue);
    d->varName = std::string(var);
    d->value = 0;
    exprtkStruct->doubleValue.push_back(d);
    exprtkStruct->symbolTable.add_variable(d->varName,d->value);
}

void add_string_variable(exprtkWrapper obj, const char* var)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    StringValue *s = new(StringValue);
    s->varName = std::string(var);
    s->value = "";
    exprtkStruct->stringValue.push_back(s);
    exprtkStruct->symbolTable.add_stringvar(s->varName,s->value);
}

void add_vector_variable(exprtkWrapper obj, const char* var)
{
    ExprtkStruct* exprtkStruct = (ExprtkStruct*)obj;
    VectorValue *v = new(VectorValue);
    v->varName = std::string(var);
    v->value.push_back(double(0));
    exprtkStruct->vectorValue.push_back(v);
    exprtkStruct->symbolTable.add_vector(v->varName,v->value);
}

void delete_exprtk(exprtkWrapper obj)
{
    delete (ExprtkStruct*)obj;
}
