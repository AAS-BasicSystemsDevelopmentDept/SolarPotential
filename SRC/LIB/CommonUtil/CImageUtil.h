#pragma once
#include "StringEx.h"

class CImageUtil
{
public:
	CImageUtil();
	~CImageUtil();

	// GeoTIFF->Jpeg�ϊ�
	static bool ConvertTiffToJpeg(const std::wstring& strTiffPath);

	// �}��摜�̍쐬
	static bool CreateLegendImage(const std::wstring& strColorSetting, const std::wstring& strHeader);

private:

};

