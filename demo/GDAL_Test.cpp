#include <iostream>
#include "funset.hpp"

int main()
{
	int ret = test_gadl_GDALDataset_write();
	if (ret == 0)
		std::cout << "ok" << std::endl;
	else
		std::cout << "fail" << std::endl;
}
