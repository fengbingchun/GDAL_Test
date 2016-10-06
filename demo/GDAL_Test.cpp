#include <iostream>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <gdal_alg.h>

int main()
{
	const char* image_name = "E:/GitCode/GDAL_Test/test_images/a.tif";
	GDALAllRegister();
	GDALDataset* poSrc = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);
	if (poSrc == nullptr) {
		std::cout << "input image error" << std::endl;
		return -1;
	}

	int width_src = poSrc->GetRasterXSize();
	int height_src = poSrc->GetRasterYSize();
	int band_count_src = poSrc->GetRasterCount();
	fprintf(stderr, "width: %d, height: %d, bandCount: %d\n", width_src, height_src, band_count_src);
	GDALDataType  gdal_data_type = poSrc->GetRasterBand(1)->GetRasterDataType();
	int depth = GDALGetDataTypeSize((GDALDataType)gdal_data_type);
	fprintf(stderr, "depth: %d\n", depth);

	GDALClose((GDALDatasetH)poSrc);

	std::cout << "ok" << std::endl;

	return 0;
}
