#include <iostream>
#include "funset.hpp"

int main(int argc, char* argv[])
{
	int ret = test_gdal_support_chinese_path(argc, argv);
	if (ret == 0)
		std::cout << "ok" << std::endl;
	else
		std::cout << "fail" << std::endl;
}
