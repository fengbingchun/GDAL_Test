#include "funset.hpp"
#include <iostream>
#include <assert.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <gdal_alg.h>

int test_gdal_base()
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
	GDALDataType gdal_data_type = poSrc->GetRasterBand(1)->GetRasterDataType();
	int depth = GDALGetDataTypeSize((GDALDataType)gdal_data_type);
	fprintf(stderr, "depth: %d\n", depth);

	GDALClose((GDALDatasetH)poSrc);

	return 0;
}

int test_gdal_GDALRasterBand()
{
	// reference: http://gdal.org/gdal_tutorial.html

	//const char* image_name = "D:/Download/cloud_detection_SDK/test_data/TH01-02_P201603099063743_1B_DGP_08_085_111.TIF";
	const char* image_name = "D:/Download/cloud_detection_SDK/test_data/TH01-01_P201404139099343_1B_DGP_08_274_187.tiff";

	// 1. Opening the File
	GDALAllRegister();

	GDALDataset* poDataset = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);
	if (poDataset == nullptr) {
		std::cout << "input image error" << std::endl;
		return -1;
	}

	// 2. Getting Dataset Information
	fprintf(stderr, "Driver: %s/%s\n", poDataset->GetDriver()->GetDescription(), poDataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME));

	// image size: width * height * band_count
	int width = poDataset->GetRasterXSize();
	int height = poDataset->GetRasterYSize();
	int band_count = poDataset->GetRasterCount(); // band_count == channels num
	fprintf(stderr, "width: %d, height: %d, bandCount: %d\n", width, height, band_count);
	fprintf(stderr, "Size is %dx%dx%d\n", poDataset->GetRasterXSize(), poDataset->GetRasterYSize(), poDataset->GetRasterCount());

	if (poDataset->GetProjectionRef() != nullptr)
		fprintf(stderr, "Projection is `%s'\n", poDataset->GetProjectionRef());

	// 3. Fetching a Raster Band
	for (int i = 1; i <= band_count; i++) {
		fprintf(stderr, "************************************\n");
		int blockXSize{ 0 }, blockYSize{ 0 };
		int gotMin{ -1 }, gotMax{ -1 };
		double adfMinMax[2] {0, 0};

		GDALRasterBand* poBand = poDataset->GetRasterBand(i); // from 1 to GetRasterCount()

		poBand->GetBlockSize(&blockXSize, &blockYSize);
		fprintf(stderr, "Block=%dx%d Type=%s, ColorInterp=%s\n", blockXSize, blockYSize,
			GDALGetDataTypeName(poBand->GetRasterDataType()),
			GDALGetColorInterpretationName(poBand->GetColorInterpretation()));

		adfMinMax[0] = poBand->GetMinimum(&gotMin);
		adfMinMax[1] = poBand->GetMaximum(&gotMax);
		fprintf(stderr, "gotMin: %d, gotMax: %d, adfMin: %f, adfMax: %f\n", gotMin, gotMax, adfMinMax[0], adfMinMax[1]);

		if (!(gotMin && gotMax)) {
			GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);
			fprintf(stderr, "band Min: %f, band Max: %f\n", adfMinMax[0], adfMinMax[1]);
		}

		if (poBand->GetOverviewCount() > 0)
			fprintf(stderr, "Band has %d overviews.\n", poBand->GetOverviewCount());

		if (poBand->GetColorTable() != nullptr)
			fprintf(stderr, "Band has a color table with %d entries.\n", poBand->GetColorTable()->GetColorEntryCount());
	}

	// 4. Reading Raster Data
	float* pafScanline = nullptr;
	GDALRasterBand* poBand = poDataset->GetRasterBand(1);
	int xSize = poBand->GetXSize();
	fprintf(stderr, "xSize: %d\n", xSize);
	pafScanline = (float *)CPLMalloc(sizeof(float) * xSize);
	poBand->RasterIO(GF_Read, 0, 0, xSize, 1, pafScanline, xSize, 1, GDT_Float32, 0, 0);
	CPLFree(pafScanline);

	// 5. Closing the Dataset
	GDALClose((GDALDatasetH)poDataset);

	return 0;
}

int test_gdal_GDALDataset()
{
	// Blog: http://blog.csdn.net/fengbingchun/article/details/52825309
	const char* image_name = "E:/GitCode/GDAL_Test/test_images/3.jpg";

	GDALAllRegister();

	GDALDataset* poDataset = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);
	if (poDataset == nullptr) {
		std::cout << "input image error" << std::endl;
		return -1;
	}

	int width = poDataset->GetRasterXSize();
	int height = poDataset->GetRasterYSize();
	int band_count = poDataset->GetRasterCount();
	size_t length = width * height * band_count;

	GDALDataType gdal_data_type = poDataset->GetRasterBand(1)->GetRasterDataType();
	int depth = GDALGetDataTypeSize((GDALDataType)gdal_data_type);
	//fprintf(stderr, "depth: %d\n", depth);
	assert(depth == 8 || depth == 16);
	int size_byte = 1;
	if (depth == 16) size_byte = 2;

	void* data1 = nullptr;
	void* data2 = nullptr;
	void* data3 = nullptr;

	if (depth == 8) {
		data1 = new unsigned char[length];
		memset(data1, 0, length);
		data2 = new unsigned char[length];
		memset(data2, 0, length);
		data3 = new unsigned char[length];
		memset(data3, 0, length);
	} else {
		data1 = new unsigned short[length];
		memset(data1, 0, length * 2);
		data2 = new unsigned short[length];
		memset(data2, 0, length * 2);
		data3 = new unsigned short[length];
		memset(data3, 0, length * 2);
	}

	GDALClose((GDALDatasetH)poDataset);

	{ // mode1
		GDALDataset* poDataset = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);
		int band_count = poDataset->GetRasterCount();
		fprintf(stderr, "mode=1: band_count = %d\n", band_count);
		int* pBandMap = new int[band_count];
		for (int i = 0; i < band_count; i++) {
			pBandMap[i] = i + 1;
		}
		GDALDataType gdal_data_type = poDataset->GetRasterBand(1)->GetRasterDataType();
		int depth = GDALGetDataTypeSize((GDALDataType)gdal_data_type);
		void* poSrcData = nullptr;
		if (depth == 8)
			poSrcData = new unsigned char[width * height * band_count];
		else
			poSrcData = new unsigned short[width * height * band_count];

		poDataset->RasterIO(GF_Read, 0, 0, width, height,
			poSrcData, width, height, gdal_data_type, band_count, pBandMap, 0, 0, 0);

		if (depth == 8) {
			unsigned char* p1 = (unsigned char*)poSrcData;

			for (int y = 0; y < height; y++) {
				unsigned char* p2 = (unsigned char*)data1 + width * band_count * y;
				for (int x = 0; x < width; x++) {
					for (int band = 0; band < band_count; band++) {
						p2[x * band_count + band] = p1[band * width * height + y * width + x];;
					}
				}
			}
		} else {
			unsigned short* p1 = (unsigned short*)poSrcData;

			for (int y = 0; y < height; y++) {
				unsigned short* p2 = (unsigned short*)data1 + width * band_count * y;
				for (int x = 0; x < width; x++) {
					for (int band = 0; band < band_count; band++) {
						p2[x * band_count + band] = p1[band * width * height + y * width + x];;
					}
				}
			}
		}

		GDALClose((GDALDatasetH)poDataset);
		delete[] pBandMap;
		delete[] poSrcData;
	}

	{ // mode2
		GDALDataset* poDataset = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);

		int crop_width = 200;
		int crop_height = 200;
		int loops_y = height / crop_height;
		int loops_x = width / crop_width;

		for (int y = 0; y < loops_y; y++) {
			for (int x = 0; x < loops_x; x++) {
				int band_count = poDataset->GetRasterCount();
				fprintf(stderr, "mode=2: band_count = %d\n", band_count);
				int* pBandMap = new int[band_count];
				for (int i = 0; i < band_count; i++) {
					pBandMap[i] = i + 1;
				}
				GDALDataType gdal_data_type = poDataset->GetRasterBand(1)->GetRasterDataType();
				int depth = GDALGetDataTypeSize((GDALDataType)gdal_data_type);
				void* poSrcData = nullptr;
				if (depth == 8)
					poSrcData = new unsigned char[crop_width * crop_height * band_count];
				else
					poSrcData = new unsigned short[crop_width * crop_height * band_count];

				int xOff = crop_width * x;
				int yOff = crop_height * y;

				poDataset->RasterIO(GF_Read, xOff, yOff, crop_width, crop_height,
					poSrcData, crop_width, crop_height, gdal_data_type, band_count, pBandMap, 0, 0, 0);

				if (depth == 8) {
					unsigned char* p1 = (unsigned char*)poSrcData;
					unsigned char* p2 = (unsigned char*)data2 + width * band_count * y * crop_height;

					for (int m = 0; m < crop_height; m++) {
						unsigned char* p3 = p2 + width * band_count * m + x * crop_width * band_count;
						for (int n = 0; n < crop_width; n++) {
							for (int band = 0; band < band_count; band++) {
								p3[n * band_count + band] = p1[band * crop_width * crop_height + m * crop_width + n];
							}
						}
					}

				}
				else {
					unsigned short* p1 = (unsigned short*)poSrcData;
					unsigned short* p2 = (unsigned short*)data2 + width * band_count * y * crop_height;

					for (int m = 0; m < crop_height; m++) {
						unsigned short* p3 = p2 + width * band_count * m + x * crop_width * band_count;
						for (int n = 0; n < crop_width; n++) {
							for (int band = 0; band < band_count; band++) {
								p3[n * band_count + band] = p1[band * crop_width * crop_height + m * crop_width + n];
							}
						}
					}
				}

				delete[] pBandMap;
				delete[] poSrcData;
			}
		}

		GDALClose((GDALDatasetH)poDataset);
	}

	{ // mode3
		int crop_width = 200;
		int crop_height = 200;
		int loops_y = height / crop_height;
		int loops_x = width / crop_width;

		for (int y = 0; y < loops_y; y++) {
			for (int x = 0; x < loops_x; x++) {
				GDALDataset* poDataset = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);
				int band_count = poDataset->GetRasterCount();
				fprintf(stderr, "mode=3: band_count = %d\n", band_count);
				int* pBandMap = new int[band_count];
				for (int i = 0; i < band_count; i++) {
					pBandMap[i] = i + 1;
				}
				GDALDataType gdal_data_type = poDataset->GetRasterBand(1)->GetRasterDataType();
				int depth = GDALGetDataTypeSize((GDALDataType)gdal_data_type);
				void* poSrcData = nullptr;
				if (depth == 8)
					poSrcData = new unsigned char[crop_width * crop_height * band_count];
				else
					poSrcData = new unsigned short[crop_width * crop_height * band_count];

				int xOff = crop_width * x;
				int yOff = crop_height * y;

				poDataset->RasterIO(GF_Read, xOff, yOff, crop_width, crop_height,
					poSrcData, crop_width, crop_height, gdal_data_type, band_count, pBandMap, 0, 0, 0);

				if (depth == 8) {
					unsigned char* p1 = (unsigned char*)poSrcData;
					unsigned char* p2 = (unsigned char*)data3 + width * band_count * y * crop_height;

					for (int m = 0; m < crop_height; m++) {
						unsigned char* p3 = p2 + width * band_count * m + x * crop_width * band_count;
						for (int n = 0; n < crop_width; n++) {
							for (int band = 0; band < band_count; band++) {
								p3[n * band_count + band] = p1[band * crop_width * crop_height + m * crop_width + n];
							}
						}
					}

				} else {
					unsigned short* p1 = (unsigned short*)poSrcData;
					unsigned short* p2 = (unsigned short*)data3 + width * band_count * y * crop_height;

					for (int m = 0; m < crop_height; m++) {
						unsigned short* p3 = p2 + width * band_count * m + x * crop_width * band_count;
						for (int n = 0; n < crop_width; n++) {
							for (int band = 0; band < band_count; band++) {
								p3[n * band_count + band] = p1[band * crop_width * crop_height + m * crop_width + n];
							}
						}
					}
				}

				GDALClose((GDALDatasetH)poDataset);
				delete[] pBandMap;
				delete[] poSrcData;
			}
		}
	}

	for (int i = 0; i < length * size_byte; i++) {
		unsigned char* p1 = (unsigned char*)data1;
		unsigned char* p2 = (unsigned char*)data2;
		unsigned char* p3 = (unsigned char*)data3;

		if (p1[i] != p2[i] || p1[i] != p3[i]) {
			fprintf(stderr, "error: data1 != data2 or data1 != data3\n");
			return -1;
		}
	}

	delete[] data1;
	delete[] data2;
	delete[] data3;

	return 0;
}

int test_gdal_GDALDataset_write()
{
	// Blog: http://blog.csdn.net/fengbingchun/article/details/53070583
	const char* image_name = "E:/GitCode/GDAL_Test/test_images/1.jpg";

	{ // write bgr: CreateCopy
		GDALAllRegister();

		const char* pszFormat = "jpeg"; //"bmp";// "png";//"GTiff";
		GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
		if (!poDriver) {
			fprintf(stderr, "get driver by name failed\n");
			return -1;
		}

		char** papszMetadata = poDriver->GetMetadata();;
		if (CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE))
			fprintf(stderr, "Driver %s supports Create() method.\n", pszFormat);
		if (CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATECOPY, FALSE))
			fprintf(stderr, "Driver %s supports CreateCopy() method.\n", pszFormat);
		if (!CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATECOPY, FALSE)) {
			fprintf(stderr, "Driver %s don't supports CreateCopy() method.\n", pszFormat);
			return -1;
		}

		GDALDataset* poDataset = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);
		if (poDataset == nullptr) {
			std::cout << "input image error" << std::endl;
			return -1;
		}

		const char* pszDstFilename = "E:/GitCode/GDAL_Test/test_images/ret_1.jpg";
		// All drivers that support creating new files support the CreateCopy() method, but only a few support the Create() method
		GDALDataset* poDstDS = poDriver->CreateCopy(pszDstFilename, poDataset, FALSE, nullptr, nullptr, nullptr);
		if (!poDstDS) {
			std::cout << "create copy failed" << std::endl;
			return -1;
		}

		GDALClose((GDALDatasetH)poDataset);
		GDALClose((GDALDatasetH)poDstDS);
	}

	{ // write: bgr<->rgb
		GDALAllRegister();

		const char* pszFormat = "GTiff";
		GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
		if (!poDriver) {
			fprintf(stderr, "get driver by name failed\n");
			return -1;
		}

		char** papszMetadata = poDriver->GetMetadata();;
		if (!CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE)) {
			fprintf(stderr, "Driver %s don't supports Create() method.\n", pszFormat);
			return -1;
		}

		GDALDataset* poDataset = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);
		if (poDataset == nullptr) {
			std::cout << "input image error" << std::endl;
			return -1;
		}

		int width = poDataset->GetRasterXSize();
		int height = poDataset->GetRasterYSize();
		int band_count = poDataset->GetRasterCount();
		GDALDataType gdal_data_type = poDataset->GetRasterBand(1)->GetRasterDataType();

		int pBandMap[3] = { 1, 2, 3 };
		unsigned char* pData = new unsigned char[width * height * band_count]; //RRRR...BBBB...GGGG...
		poDataset->RasterIO(GF_Read, 0, 0, width, height, pData, width, height, gdal_data_type, band_count, pBandMap, 0, 0, 0);

		char** papszOptions = nullptr;
		papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
		const char* pszDstFilename = "E:/GitCode/GDAL_Test/test_images/ret_2.jpg";
		GDALDataset* poDstDS = poDriver->Create(pszDstFilename, width, height, band_count, gdal_data_type, papszOptions);
		if (!poDstDS) {
			std::cout << "create copy failed" << std::endl;
			return -1;
		}

		int pBandMap_[3] = { 3, 2, 1 };
		poDstDS->RasterIO(GF_Write, 0, 0, width, height, pData, width, height, gdal_data_type, band_count, pBandMap_, 0, 0, 0);

		GDALClose((GDALDatasetH)poDataset);
		GDALClose((GDALDatasetH)poDstDS);
		delete[] pData;
	}

	{ // write: bgr->b, single channel
		GDALAllRegister();

		const char* pszFormat = "GTiff";
		GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
		if (!poDriver) {
			fprintf(stderr, "get driver by name failed\n");
			return -1;
		}

		char** papszMetadata = poDriver->GetMetadata();;
		if (!CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE)) {
			fprintf(stderr, "Driver %s don't supports Create() method.\n", pszFormat);
			return -1;
		}

		GDALDataset* poDataset = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);
		if (poDataset == nullptr) {
			std::cout << "input image error" << std::endl;
			return -1;
		}

		int width = poDataset->GetRasterXSize();
		int height = poDataset->GetRasterYSize();
		int band_count = poDataset->GetRasterCount();
		GDALDataType gdal_data_type = poDataset->GetRasterBand(1)->GetRasterDataType();

		unsigned char* pData = new unsigned char[width * height];
		GDALRasterBand* poBand = poDataset->GetRasterBand(2); // from 1 to GetRasterCount()
		poBand->RasterIO(GF_Read, 0, 0, width, height, pData, width, height, gdal_data_type, 0, 0, 0);

		char** papszOptions = nullptr;
		papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
		const char* pszDstFilename = "E:/GitCode/GDAL_Test/test_images/ret_3.jpg";
		GDALDataset* poDstDS = poDriver->Create(pszDstFilename, width, height, 1, gdal_data_type, papszOptions);
		if (!poDstDS) {
			std::cout << "create copy failed" << std::endl;
			return -1;
		}

		int bandMap { 1 };
		poDstDS->RasterIO(GF_Write, 0, 0, width, height, pData, width, height, gdal_data_type, 1, &bandMap, 0, 0, 0);

		GDALClose((GDALDatasetH)poDataset);
		GDALClose((GDALDatasetH)poDstDS);
		delete[] pData;
	}

	{ // write: crop image
		GDALAllRegister();

		const char* pszFormat = "GTiff";
		GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
		if (!poDriver) {
			fprintf(stderr, "get driver by name failed\n");
			return -1;
		}

		char** papszMetadata = poDriver->GetMetadata();;
		if (!CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE)) {
			fprintf(stderr, "Driver %s don't supports Create() method.\n", pszFormat);
			return -1;
		}

		GDALDataset* poDataset = (GDALDataset*)GDALOpen(image_name, GA_ReadOnly);
		if (poDataset == nullptr) {
			std::cout << "input image error" << std::endl;
			return -1;
		}

		int width = poDataset->GetRasterXSize() / 2;
		int height = poDataset->GetRasterYSize() / 2;
		int band_count = poDataset->GetRasterCount();
		GDALDataType gdal_data_type = poDataset->GetRasterBand(1)->GetRasterDataType();

		unsigned char* pData = new unsigned char[width  * height * band_count];
		int pBandMap[3] = { 1, 2, 3 };
		poDataset->RasterIO(GF_Read, 0, 0, width, height, pData, width, height, gdal_data_type, band_count, pBandMap, 0, 0, 0);

		const char* pszDstFilename = "E:/GitCode/GDAL_Test/test_images/ret_4.jpg";
		GDALDataset* poDstDS = poDriver->Create(pszDstFilename, width, height, band_count, gdal_data_type, nullptr);
		if (!poDstDS) {
			std::cout << "create copy failed" << std::endl;
			return -1;
		}

		poDstDS->RasterIO(GF_Write, 0, 0, width, height, pData, width, height, gdal_data_type, band_count, pBandMap, 0, 0, 0);

		GDALClose((GDALDatasetH)poDataset);
		GDALClose((GDALDatasetH)poDstDS);
		delete[] pData;
	}

	return 0;
}

int test_gdal_support_chinese_path(int argc, char* argv[])
{
	// Blog: http://blog.csdn.net/fengbingchun/article/details/53185706
	if (argc < 2) {
		fprintf(stderr, "params fail\n");
		return -1;
	}

	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	GDALDataset* poSrc = (GDALDataset*)GDALOpen(argv[1], GA_ReadOnly);
	if (poSrc == nullptr) {
		std::cout << "input image error" << std::endl;
		return -1;
	}

	int width_src = poSrc->GetRasterXSize();
	int height_src = poSrc->GetRasterYSize();
	int band_count_src = poSrc->GetRasterCount();
	fprintf(stderr, "image width: %d, height: %d, bandCount: %d\n", width_src, height_src, band_count_src);
	GDALDataType gdal_data_type = poSrc->GetRasterBand(1)->GetRasterDataType();
	int depth = GDALGetDataTypeSize((GDALDataType)gdal_data_type);
	fprintf(stderr, "depth: %d\n", depth);

	GDALClose((GDALDatasetH)poSrc);

	return 0;
}
