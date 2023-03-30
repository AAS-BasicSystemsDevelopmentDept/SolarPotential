#include "pch.h"
#include "ImportWeatherData.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "shapefil.h"

#ifdef _DEBUG
#pragma comment(lib,"shapelib_i.lib")
#else
#pragma comment(lib,"shapelib_i.lib")
#endif

CImportWeatherData::CImportWeatherData(void)
{

}

CImportWeatherData::~CImportWeatherData(void)
{
	Initialize();
}

void CImportWeatherData::Initialize()
{
	if (m_arySnowDepthData.size() > 0)
	{
		for (SnowDepth* pRestrictArea : m_arySnowDepthData)
		{
			delete pRestrictArea;
		}
	}
	m_arySnowDepthData.clear();
}

bool CImportWeatherData::ReadData()
{
	if (!GetFUtil()->IsExistPath(m_strFilePath))
	{
		return	false;
	}

	std::string strFilePath = CStringEx::ToString(m_strFilePath);

	// Dbf�t�@�C�����擾
	std::string strDbfPath = CFileUtil::ChangeFileNameExt(strFilePath, ".dbf");
	if (!GetFUtil()->IsExistPath(strDbfPath))
	{
		return false;
	}

	// �V�F�[�v�t�@�C�����I�[�v��
	SHPHandle hSHP;
	hSHP = SHPOpen(strFilePath.c_str(), "r");
	if (hSHP == NULL)
	{
		// �t�@�C���̃I�[�v���Ɏ��s
		return false;
	}

	// Dbf�t�@�C�����I�[�v��
	DBFHandle hDBF;
	hDBF = DBFOpen(strDbfPath.c_str(), "rb");
	if (hDBF == NULL)
	{
		// �t�@�C���̃I�[�v���Ɏ��s
		SHPClose(hSHP);
		return false;
	}

	// ���
	int nShapeType = SHPT_NULL;
	// �v�f��
	int nEntities = 0;
	//�@�o�E���f�B���O
	double adfMinBound[4], adfMaxBound[4];
	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);

	if (nEntities <= 0)
	{
		// �V�F�[�v�t�@�C�����R�[�h�̎擾�Ɏ��s
		SHPClose(hSHP);
		DBFClose(hDBF);
		return false;
	}
	// ��ʂ̃`�F�b�N
	if (nShapeType != SHPT_POLYGON &&
		nShapeType != SHPT_POLYGONZ &&
		nShapeType != SHPT_POLYGONM)
	{
		// �V�F�[�v�^�C�v���ΏۊO
		SHPClose(hSHP);
		DBFClose(hDBF);
		return false;
	}

	// �t�B�[���h��
	int nField = DBFGetFieldCount(hDBF);

	// 3�����b�V���R�[�h�t�B�[���h
	const char* pMeshField = "G02_001";
	// �N�Ő[�ϐ�t�B�[���h
	const char* pSnowDepthField = "G02_058";

	// �Ώۂ̑����t�B�[���hIndex
	int meshFieldIdx = -1;
	int snowDepthFieldIdx = -1;

	char szTitle[20];
	int nWidth, nDecimals;
	for (int ic = 0; ic < nField; ic++)
	{
		DBFFieldType eType = DBFGetFieldInfo(hDBF, ic, szTitle, &nWidth, &nDecimals);
		switch (eType)
		{
		case FTString:
			if (strcmp(szTitle, pMeshField) == 0)
			{
				meshFieldIdx = ic;
			}
			break;
		case FTDouble:
			if (strcmp(szTitle, pSnowDepthField) == 0)
			{
				snowDepthFieldIdx = ic;
			}
			break;
		}
	}
	if (meshFieldIdx < 0 || snowDepthFieldIdx < 0)
	{
		// ���b�V��ID����ѐϐ�[�̑������Ȃ��ꍇ�͎��s
		SHPClose(hSHP);
		DBFClose(hDBF);
		return false;
	}

	// �t�@�C������f�[�^��ǂݍ���Œ��_�z��ɒǉ�
	SHPObject* psElem;
	for (int ic = 0; ic < nEntities; ic++)
	{
		psElem = SHPReadObject(hSHP, ic);

		if (psElem)
		{
			SnowDepth* pSnowDepth = new SnowDepth;

			// ���_��̎擾
			for (int jc = 0; jc < psElem->nVertices; jc++)
			{	// �ܓx�o�x�@���@���ʒ��p���W�n�ɕϊ�
				int JPZONE = GetINIParam()->GetJPZone();
				double dEast, dNorth;
				CGeoUtil::LonLatToXY(psElem->padfX[jc], psElem->padfY[jc], JPZONE, dEast, dNorth);

				// �ϊ��������W�̒ǉ�
				CPointBase pt(dEast, dNorth, psElem->padfZ[jc]);
				pSnowDepth->vecPolyline.push_back(pt);
			}

			// �����̎擾
			pSnowDepth->iMeshID = DBFReadIntegerAttribute(hDBF, ic, meshFieldIdx);
			pSnowDepth->iSnowDepth = DBFReadIntegerAttribute(hDBF, ic, snowDepthFieldIdx);
			if (pSnowDepth->iMeshID >= 0 && pSnowDepth->iSnowDepth >= 0 && pSnowDepth->vecPolyline.size() >= 0)
			{
				m_arySnowDepthData.push_back(pSnowDepth);
			}
		}
		SHPDestroyObject(psElem);
	}

	SHPClose(hSHP);
	DBFClose(hDBF);

	return true;
}


// �ϐ�[���擾
int CImportWeatherData::GetSnowDepth(const CPoint2D& pointTarget)
{
	// ���b�V����
	size_t iAreaNum = m_arySnowDepthData.size();

	// �ϐ�[(cm)
	int iSnowDepth = -1;

	for (SnowDepth* pSnowDepth : m_arySnowDepthData)
	{
		int iCountPoint = (int)pSnowDepth->vecPolyline.size();
		CPoint2D* pPoint = new CPoint2D[iCountPoint];
		for (int n = 0; n < iCountPoint; n++)
		{
			pPoint[n] = CPoint2D(pSnowDepth->vecPolyline[n].x, pSnowDepth->vecPolyline[n].y);
		}

		// ���O����
		bool bRet = CGeoUtil::IsPointInPolygon(pointTarget, (int)pSnowDepth->vecPolyline.size(), pPoint);

		delete[] pPoint;

		if (bRet)
		{	// �Y�����b�V���̏ꍇ
			iSnowDepth = pSnowDepth->iSnowDepth;
			break;
		}
	}
	return iSnowDepth;

}

// �ϐ�׏d�v�Z
double CImportWeatherData::CalSnowLoad(const CPoint2D& pointTarget, const double dp)
{
	// �ϐ�[(cm)���擾
	int iSnowDepth = GetSnowDepth(pointTarget);

	if (iSnowDepth >= 0)
	{
		// �ϐ�׏d���v�Z����
		// �ϐ�׏d(kgf/�u) = �N�Ő[�ϐ��cm �~ ��N/�u 
		double dS = iSnowDepth * dp / 10.0; // 10N/�u = 1kgf/�u

		return dS;
	}

	return -1;
}
