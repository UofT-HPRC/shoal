#include <iostream>

#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

#pragma message(VAR_NAME_VALUE(MACRO_VALUE))

int main(){
	std::cout << MACRO_VALUE << "\n";
	return 0;
}
