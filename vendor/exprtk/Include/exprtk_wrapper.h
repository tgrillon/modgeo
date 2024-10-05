#ifndef EXPRTKWRAPPER_HPP
#define EXPRTKWRAPPER_HPP

#ifdef __cplusplus
extern "C" {
#endif

typedef void* exprtkWrapper;

exprtkWrapper exprtk_wrapper_init(void);

void set_expression_count(exprtkWrapper, unsigned int count);

void set_expression_string(exprtkWrapper, const char* expr, unsigned int idx);

const char* get_expression_string(exprtkWrapper, unsigned int idx);

void add_double_variable(exprtkWrapper, const char* var);

void add_string_variable(exprtkWrapper, const char* var);

void add_vector_variable(exprtkWrapper, const char* var);

int compile_expression(exprtkWrapper);

void set_double_variable_value(exprtkWrapper, const char* var, double val);

void set_string_variable_value(exprtkWrapper, const char* var, const char* val);

void set_vector_variable_value(exprtkWrapper, const char* var, double val[], int len);

double get_evaluated_value(exprtkWrapper, unsigned int idx);

void delete_exprtk(exprtkWrapper);


#ifdef __cplusplus
}
#endif

#endif // EXPRTKWRAPPER_HPP
