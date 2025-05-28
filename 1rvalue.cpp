#include <iostream>

int main()
{
	auto x{ 10};
	const decltype(x)& y = x+1;	
	auto&& z = x+2;
	z += 30;
	std::cout <<"y = " << y << "\nz = " << z << "\n\n";
	//	int && v = x;
	int && r = std::move(x);	

}