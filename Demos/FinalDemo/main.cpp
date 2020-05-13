#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <stack>
#include <map>

enum operator_type : unsigned {
	eLeft = 0,
	eOr = 1,
	eAnd = 2,
	eNot = 3,
	eRight = 4
};

using variable = std::pair<char, bool>;

bool is_alpha(char c) { return c >= 'A' && c <= 'Z'; }

bool is_operator(char c) { return c == '&' || c == '|' || c == '~'; }

operator_type convert_char_to_operator(char c) {
	if (c == '|') return eOr;
	if (c == '&') return eAnd;
	if (c == '~') return eNot;

	throw ">..<";
}

class expression {
public:
	explicit expression(const std::string& text);

	bool evaluate(const std::vector<int>& variables) const;

	std::string instance(const std::vector<int>& variables) const;
	
	char variable(size_t index) const;
	
	size_t count() const noexcept;

	const std::map<char, std::vector<size_t>>& variables() const noexcept;
private:
	void calculate(std::stack<operator_type>& op, std::stack<int>& values) const;

	std::map<char, std::vector<size_t>> mVariables;
	std::string mText;
};

expression::expression(const std::string& text) : mText(text)
{
	for (size_t index = 0; index < mText.size(); index++) {
		if (!is_alpha(mText[index])) continue;

		mVariables[mText[index]].push_back(index);
	}
}

bool expression::evaluate(const std::vector<int>& variables) const
{
	auto expression = instance(variables);
	
	std::stack<operator_type> operators;
	std::stack<int> values;

	for (size_t index = 0; index < expression.size(); index++) {
		const auto character = expression[index];
		
		if (character == '1' || character == '0') { values.push(character - '0'); continue; }
		if (character == ' ') continue;

		if (character == '(') { operators.push(eLeft); continue; }

		if (character == ')') {
			while (!operators.empty() && operators.top() != eLeft)
				calculate(operators, values);

			if (!operators.empty()) operators.pop();

			continue;
		}
		
		if (is_operator(character)) {
			const auto type = convert_char_to_operator(character);
			
			while (!operators.empty() && operators.top() > type)
				calculate(operators, values);

			operators.push(type);
		}
	}

	while (!operators.empty()) calculate(operators, values);

	return values.top();
}

std::string expression::instance(const std::vector<int>& variables) const
{
	auto expression = mText;

	auto it = mVariables.begin();

	for (size_t index = 0; index < variables.size(); index++) {
		for (const auto& position : it->second)
			expression[position] = variables[index] + '0';

		++it;
	}

	return expression;
}

char expression::variable(size_t index) const
{
	auto it = mVariables.begin();

	for (size_t i = 0; i < index; i++) ++it;
	
	return it->first;
}

size_t expression::count() const noexcept
{
	return mVariables.size();
}

const std::map<char, std::vector<size_t>>& expression::variables() const noexcept
{
	return mVariables;
}

void expression::calculate(std::stack<operator_type>& op, std::stack<int>& values) const
{
	const auto type = op.top(); op.pop();

	if (type == eNot) {
		const auto value = values.top(); values.pop();

		values.push(!value);

		return;
	}

	const auto value0 = values.top(); values.pop();
	const auto value1 = values.top(); values.pop();

	if (type == eAnd) values.push(value0 & value1);
	if (type == eOr) values.push(value0 | value1);
}

void evaluate_command(const expression& expression)
{
	std::vector<int> values;

	std::string value;

	std::cout << std::endl;
	
	while (std::cin >> value) {
		values.push_back(std::stoi(value));

		if (values.back() != 0 && values.back() != 1) {
			std::cout << "wrong value of variables." << std::endl;

			return;
		}

		if (values.size() == expression.count()) break;
	}

	std::cout << "the instance of expression is " << expression.instance(values) << std::endl;
	
	const auto result = expression.evaluate(values);

	std::cout << "the value of expression is " << result << std::endl;
}

void enum_variables(std::vector<int>& values, const expression& expression)
{
	if (values.size() == expression.count()) {

		for (size_t index = 0; index < values.size(); index++)
			std::cout << values[index] << " ";

		std::cout << expression.evaluate(values) << std::endl;
		
		return;
	}

	values.push_back(0); enum_variables(values, expression); values.pop_back();
	values.push_back(1); enum_variables(values, expression); values.pop_back();
}

void list_command(const expression& expression)
{
	std::vector<int> values;

	std::cout << std::endl;
	
	for (size_t index = 0; index < expression.count(); index++)
		std::cout << expression.variable(index) << " ";

	std::cout << "ANS" << std::endl;
	
	enum_variables(values, expression);
}

/*bool test_0_function(int A, int B, int C, int D, int E) {
	return (A & B) | (!!!!!!(!!!C & A | !B) & ((!D) | !(D & E)));
}

void enum_test_0(std::vector<int>& values, expression& expression) {
	if (values.size() == expression.count()) {

		int exp_value = expression.evaluate(values);
		int fun_value = test_0_function(values[0], values[1], values[2], values[3], values[4]);
		
		if (exp_value != fun_value) {
			int x = 2;
		}
		return;
	}

	values.push_back(0); enum_test_0(values, expression); values.pop_back();
	values.push_back(1); enum_test_0(values, expression); values.pop_back();
}

void build_test_0() {
	std::string test = "(A & B) | (~~~~~~(~~~C & A | ~B) & ((~D) | ~(D & E)))";

	expression expression(test);
	std::vector<int> values;
	
	enum_test_0(values, expression);
}*/

int main() {
	std::string text;
	
	std::cout << "please input the expression." << std::endl;
	std::getline(std::cin, text);

	expression expression(text);

	std::cout << "the expression you input is " << text << std::endl;

	std::cout << std::endl;
	std::cout << "----------------------------" << std::endl;
	std::cout << "evaluate first second ... : give the value of variables, evaluate the value of expression." << std::endl;
	std::cout << "list                      : list all input and value of expression." << std::endl;

	std::string command;
	
	while (std::cin >> command) {

		if (command == "evaluate")
			evaluate_command(expression);

		if (command == "list") list_command(expression);

		if (command != "list" && command != "evaluate") 
			std::cout << "unknown command." << std::endl;
		
		std::cout << std::endl;
		std::cout << "----------------------------" << std::endl;
		std::cout << "evaluate first second ... : give the value of variables, evaluate the value of expression." << std::endl;
		std::cout << "list                      : list all input and value of expression." << std::endl;
	}
}
