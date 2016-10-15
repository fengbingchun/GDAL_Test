#include <iostream>
#include "funset.hpp"

int main()
{
	int ret = test_gdal_GDALDataset();
	if (ret == 0)
		std::cout << "ok" << std::endl;
	else
		std::cout << "fail" << std::endl;
}
