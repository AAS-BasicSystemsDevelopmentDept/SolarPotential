#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <locale.h>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/StringEx.h"

class CImportWeatherData
{

public:
	CImportWeatherData(void);
	~CImportWeatherData(void);

	struct SnowDepth
	{
		int						iMeshID;		// ���b�V��ID
		std::vector<CPointBase> vecPolyline;	// �|�����C���̓_��
		int						iSnowDepth;		// �N�Ԑϐ��(cm)

		/* �R���X�g���N�^
		*/
		SnowDepth()
		{
			iMeshID = -1;
			iSnowDepth = -1;
		}
		/* �f�X�g���N�^
		*/
		virtual ~SnowDepth()
		{
		}

		/* �R�s�[�R���X�g���N�^
		*/
		SnowDepth(const SnowDepth& x) { *this = x; }
		/* ������Z�q
		*/
		SnowDepth& operator=(const SnowDepth& x)
		{
			if (this != &x)
			{
				iMeshID = x.iMeshID;
				copy(x.vecPolyline.begin(), x.vecPolyline.end(), back_inserter(vecPolyline));
				iSnowDepth = x.iSnowDepth;
			}
			return *this;
		}

	};

	void Initialize();
	void SetReadFilePath(std::string path)
	{
		setlocale(LC_ALL, "");
		m_strFilePath = CStringEx::ToWString(path);
	};
	bool ReadData();
	int GetSnowDepth(const CPoint2D& pointTarget);
	double CalSnowLoad(const CPoint2D& pointTarget, const double dp);

private:
	std::wstring			m_strFilePath;			// ���̓t�@�C���p�X
	std::vector<SnowDepth*>	m_arySnowDepthData;		// �ϐ�f�[�^
};