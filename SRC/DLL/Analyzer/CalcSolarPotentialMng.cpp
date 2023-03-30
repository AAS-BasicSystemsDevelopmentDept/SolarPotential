#include "pch.h"
#include <math.h>
#include <random>
#include <iostream>
#include "CalcSolarPotentialMng.h"
#include "CalcSolarRadiation.h"
#include "CalcSolarPower.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "../../LIB/CommonUtil/CImageUtil.h"
#include "../../LIB/CommonUtil/CEpsUtil.h"

#define DEF_IMG_NODATA -9999
#define DEF_EPSGCODE 6675

CCalcSolarPotentialMng::CCalcSolarPotentialMng
(
	CImportPossibleSunshineData* pSunshineData,
	CImportAverageSunshineData* pPointData,
	CImportMetpvData* pMetpvData,
	UIParam* pParam,
	const int& iYear
)
	: m_pSunshineData(pSunshineData)
	, m_pPointData(pPointData)
	, m_pMetpvData(pMetpvData)
	, m_pUIParam(pParam)
	, m_pRadiationData(NULL)
	, m_pvecAllBuildList(NULL)
	, m_pmapResultData(NULL)
	, m_iYear(iYear)
	, m_strCancelFilePath(L"")
	, m_pvecAllDemList(NULL)
{

}

CCalcSolarPotentialMng::~CCalcSolarPotentialMng(void)
{
}


// �v�Z�p�f�[�^���̏�����
void CCalcSolarPotentialMng::initialize()
{
	// �f�[�^�擾
	m_pvecAllBuildList = reinterpret_cast<std::vector<BLDGLIST>*>(GetAllList());
	m_pvecAllDemList = reinterpret_cast<std::vector<DEMLIST>*>(GetAllDemList());

	m_pmapResultData = new CResultDataMap;
	m_pRadiationData = new CAnalysisRadiationCommon;

	// �L�����Z��
	std::wstring strDir = GetFUtil()->GetParentDir(m_pUIParam->strOutputDirPath);
	m_strCancelFilePath = GetFUtil()->Combine(strDir, CStringEx::ToWString(CANCELFILE));

}

// �������
void CCalcSolarPotentialMng::finalize()
{
	if (m_pRadiationData)
	{
		delete m_pRadiationData;
		m_pRadiationData = NULL;
	}

	if (m_pmapResultData)
	{
		m_pmapResultData->clear();
		delete m_pmapResultData;
		m_pmapResultData = NULL;
	}
}


// ���d�|�e���V�������v(���C������)
bool CCalcSolarPotentialMng::AnalyzeSolarPotential()
{
	setlocale(LC_ALL, "");

	if (!m_pSunshineData) return false;
	if (!m_pPointData) return false;
	if (!m_pMetpvData) return false;
	if (!m_pUIParam) return false;

	// ������
	initialize();
	if (!m_pRadiationData) return false;
	if (!m_pvecAllBuildList) return false;
	if (m_pUIParam->bEnableDEMData && !m_pvecAllDemList) return false;

	bool ret = false;

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		if (IsCancel())
		{
			finalize();
			return false;
		}

		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);

		// �������Ƃ̌��ʃf�[�^
		CBuildingDataMap buildDataMap;

		// �X�Ίp�E���ʊp�̎Z�o(�Ώۃf�[�^�擾)
		calcRoofAspect(bldList, buildDataMap);
		if (buildDataMap.empty())	continue;

		// ���˗ʐ��v
		ret = calcSolarRadiation(bldList.meshID, bldList.bbMinX, bldList.bbMinY, bldList.bbMaxX, bldList.bbMaxY, buildDataMap);
		if (!ret)
		{
			finalize();
			return false;
		}

		// ���d�ʐ��v
		ret &= calcSolarPower(bldList.meshID, bldList.bbMinX, bldList.bbMinY, bldList.bbMaxX, bldList.bbMaxY, buildDataMap);
		if (!ret)
		{
			finalize();
			return false;
		}

		(*m_pmapResultData)[bldList.meshID] = buildDataMap;

	}

	if (m_pmapResultData->empty())	ret = false;	// �Ώۃf�[�^������

	if (ret && !IsCancel())
	{
		// �o�͏���(�S�f�[�^)
		ret &= outputAzimuthDataCSV();		// �K�n����p�̒��ԃt�@�C������(CSV)
		ret &= outputResultCSV();			// �N�ԗ\�����˗ʁE���d�ʂ��o��

		// �}��o��
		ret &= outputLegendImage();
	}

	// �������
	finalize();

	return ret;
}

// ���˗ʐ��v(3�����b�V������)
bool CCalcSolarPotentialMng::calcSolarRadiation
(
	const std::string&	Lv3meshId,			// 3�����b�V��ID
	double bbMinX,
	double bbMinY,
	double bbMaxX,
	double bbMaxY,
	CBuildingDataMap&	bldDataMap			// �Ώۂ̌����f�[�^���X�g
)
{
	if (bldDataMap.empty())		return false;

	if (IsCancel())	return false;

	bool ret = false;

	CCalcSolarRadiation calcSolarRad(this);

	// 3�����b�V��ID
	std::wstring meshId = CStringEx::ToWString(Lv3meshId);

	// ����CSV�̏o�̓t�H���_���쐬
	std::wstring strOutDir = m_pUIParam->strOutputDirPath;	// �o�̓t�H���_
	if (!GetFUtil()->IsExistPath(strOutDir))
	{
		if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}
	// 3�����b�V��ID���ƂɃT�u�t�H���_���쐬
	std::wstring strOutSubDir = GetFUtil()->Combine(strOutDir, CStringEx::Format(L"%s", meshId.c_str()));
	if (!GetFUtil()->IsExistPath(strOutSubDir))
	{
		if (CreateDirectory(strOutSubDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}

	// ����CSV�t�@�C���p�X
	std::wstring strCsvPath;

	// �����Ƃ̓��Ɨ����v�Z
	calcMonthlyRate();

	// ���˗ʂ̎Z�o
	{
		// ���z�O�������Ƃɂ������˗ʂ̎Z�o
		strCsvPath = GetFUtil()->Combine(strOutSubDir, L"���V�ܓV�����˗�_����_�p�x�␳.csv");
		ret = calcSolarRad.Exec(bldDataMap, true, strCsvPath, Lv3meshId);
		if (!ret)	return false;

		// ���Ɨ��ɂ��␳	
		ret &= calcSolarRad.ModifySunRate(bldDataMap, strCsvPath);
		ret &= outputMonthlyRadCSV(Lv3meshId, bldDataMap, strOutSubDir);
		if (!ret)	return false;
	}

	// �����ʕʔN�ԓ��˗�
	ret &= calcSolarRad.CalcBuildSolarRadiation(bldDataMap, strOutSubDir);
	ret &= outputRoofRadCSV(Lv3meshId, bldDataMap, strOutSubDir);

	// ���˗ʃe�N�X�`���o��
	std::wstring strFileName = CStringEx::Format(L"���˗�%s.tif", meshId.c_str());
	std::wstring strTiffPath = GetFUtil()->Combine(strOutSubDir, strFileName);
	ret &= outputImage(strTiffPath, Lv3meshId, bbMinX, bbMinY, bbMaxX, bbMaxY, bldDataMap, eOutputImageTarget::SOLAR_RAD);

	if (!ret)
	{
		return false;
	}
	else
	{
		// �o��TIFF�摜���R�s�[
		std::wstring strCopyDir = GetFUtil()->Combine(strOutDir, L"���˗ʉ摜");
		if (!GetFUtil()->IsExistPath(strCopyDir))
		{
			if (CreateDirectory(strCopyDir.c_str(), NULL) == FALSE)
			{
				return false;
			}
		}
		std::wstring dstPath = GetFUtil()->Combine(strCopyDir, strFileName);
		CopyFile(strTiffPath.c_str(), dstPath.c_str(), FALSE);
		std::wstring strWldPath = GetFUtil()->ChangeFileNameExt(strTiffPath, L".tfw");
		std::wstring dstPath2 = GetFUtil()->ChangeFileNameExt(dstPath, L".tfw");
		CopyFile(strWldPath.c_str(), dstPath2.c_str(), FALSE);
	}

	return ret;

}

// ���d�ʐ��v
bool CCalcSolarPotentialMng::calcSolarPower
(
	const std::string& Lv3meshId,			// 3�����b�V��ID
	double bbMinX,
	double bbMinY,
	double bbMaxX,
	double bbMaxY,
	CBuildingDataMap& bldDataMap			// �Ώۂ̌����f�[�^���X�g
)
{
	if (IsCancel())	return false;

	CCalcSolarPower calcSolarPower;
	calcSolarPower.SetPperUnit(m_pUIParam->_dAreaSolarPower);
	bool ret = calcSolarPower.CalcEPY(bldDataMap);

	// �e�N�X�`���o��
	// �o�̓t�H���_���쐬
	std::wstring strOutDir = GetFUtil()->Combine(m_pUIParam->strOutputDirPath, L"���d�|�e���V�����摜");
	if (!GetFUtil()->IsExistPath(strOutDir))
	{
		if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}
	// 3�����b�V��ID
	std::wstring meshId = CStringEx::ToWString(Lv3meshId);
	// �e�N�X�`���o��
	std::wstring strFileName = CStringEx::Format(L"%s.tif", meshId.c_str());
	std::wstring strTiffPath = GetFUtil()->Combine(strOutDir, strFileName);
	ret &= outputImage(strTiffPath, Lv3meshId, bbMinX, bbMinY, bbMaxX, bbMaxY, bldDataMap, eOutputImageTarget::SOLAR_POWER);

	return ret;
}


// �X�Ίp�ƕ��ʊp���Z�o
// 
void CCalcSolarPotentialMng::calcRoofAspect(const BLDGLIST& bldList, CBuildingDataMap& bldDataMap)
{
	int bldsize = (int)bldList.buildingList.size();

	for (int ic = 0; ic < bldsize; ic++)
	{	// ����
		BUILDINGS build = bldList.buildingList[ic];
		int surfacesize = (int)build.roofSurfaceList.size();
		if (surfacesize == 0)
		{
			continue;
		}

		CBuildingData bldData;

		// �����S�̂�BB
		double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
		double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;

		std::vector<CPointBase> vecAllRoofPos;

		for (int jc = 0; jc < surfacesize; jc++)
		{	// ������
			ROOFSURFACES surface = build.roofSurfaceList[jc];
			int roofsize = (int)surface.roofSurfaceList.size();
			if (roofsize == 0)
			{
				continue;
			}

			int meshsize = (int)surface.meshPosList.size();
			if (meshsize == 0)
			{
				continue;
			}

			CRoofSurfaceData roofData;

			// �����ʑS�̂�BB
			double bbRoofMinX = DBL_MAX, bbRoofMinY = DBL_MAX;
			double bbRoofMaxX = -DBL_MAX, bbRoofMaxY = -DBL_MAX;

			std::vector<CPointBase> vecRoofPos;
			
			// ���όX�Ίp�v�Z�p
			double slopSum = 0;

			// ���ϕ��ʊp�v�Z�p
			double azSum = 0;

			// �ʐϘa
			double areaSum = 0;

			std::vector<CMeshData> tempRoofMesh;

			for (int kc = 0; kc < roofsize; kc++)
			{	// �����ʏڍ�
				SURFACEMEMBERS member = surface.roofSurfaceList[kc];
				int polygonsize = (int)member.posList.size();
				if (polygonsize == 0)
				{
					continue;
				}
				const std::vector<CPointBase> vecAry(member.posList.begin(), member.posList.end() - 1);	// �\���_�̎n�_�ƏI�_�͓����_�Ȃ̂ŏ��O����
				vecAllRoofPos.insert(vecAllRoofPos.end(), vecAry.begin(), vecAry.end());
				vecRoofPos.insert(vecRoofPos.end(), vecAry.begin(), vecAry.end());
				
				// �����S�̂�BB�����߂�(���O�������܂�)
				for (const auto& pos : vecAry)
				{
					if (pos.x < bbBldMinX)	bbBldMinX = pos.x;
					if (pos.y < bbBldMinY)	bbBldMinY = pos.y;
					if (pos.x > bbBldMaxX)	bbBldMaxX = pos.x;
					if (pos.y > bbBldMaxY)	bbBldMaxY = pos.y;
				}

				// �ʐώZ�o
				double area = calcArea(vecAry);

				// �@���Z�o
				CVector3D normal;
				if (!calcRansacPlane(vecAry, normal))
				{
					continue;
				}

				// �X�Ίp
				double slopeDeg;
				CGeoUtil::CalcSlope(normal, slopeDeg);

				// �X�����O����
				if (slopeDeg > m_pUIParam->_elecPotential.dSlopeAngle)
				{
					continue;
				}

				// ���ʊp
				double azDeg;
				int JPZONE = GetINIParam()->GetJPZone();
				CGeoUtil::CalcAzimuth(normal, azDeg, JPZONE);

				// ���ʏ��O����
				bool bExclusion = false;
				switch (m_pUIParam->_elecPotential.eAzimuth)
				{
					case eDirections::EAST:
					{
						bExclusion = (abs(azDeg) < 90.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 90.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->_elecPotential.dAzimuthAngle) ? true : false;
						break;
					}
					case eDirections::WEST:
					{
						bExclusion = (abs(azDeg) < 270.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 270.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->_elecPotential.dAzimuthAngle) ? true : false;
						break;
					}
					case eDirections::SOUTH:
					{
						bExclusion = (abs(azDeg) < 180.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 180.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->_elecPotential.dAzimuthAngle) ? true : false;
						break;
					}
					case eDirections::NORTH:
					{
						bExclusion = (abs(azDeg) < 0.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 360.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->_elecPotential.dAzimuthAngle) ? true : false;
						break;
					}
				}
				if (bExclusion)
				{
					continue;
				}

				// �����ʂ�BB�����߂�(���O�������܂܂Ȃ�)
				for (const auto& pos : vecAry)
				{
					if (pos.x < bbRoofMinX)	bbRoofMinX = pos.x;
					if (pos.y < bbRoofMinY)	bbRoofMinY = pos.y;
					if (pos.x > bbRoofMaxX)	bbRoofMaxX = pos.x;
					if (pos.y > bbRoofMaxY)	bbRoofMaxY = pos.y;
				}

				areaSum += area;
				slopSum += slopeDeg * area;
				azSum += azDeg * area;

			}
			
			if (areaSum < _MAX_TOL) { continue; }

			// �ʐς��������ꍇ���O
			if (areaSum < m_pUIParam->_elecPotential.dArea2D)
			{
				continue;
			}

			// ������BB�ɍ�����t�^
			std::vector<CVector3D> bbPos{
				{bbRoofMinX, bbRoofMinY, 0.0},
				{bbRoofMinX, bbRoofMaxY, 0.0},
				{bbRoofMaxX, bbRoofMinY, 0.0},
				{bbRoofMaxX, bbRoofMaxY, 0.0}
			};
			roofData.bbPos = bbPos;

			// ������BB�̖@��
			CVector3D n;
			CGeoUtil::OuterProduct(
				CVector3D(bbPos[1], bbPos[0]),
				CVector3D(bbPos[2], bbPos[1]), n);
			if (n.z < 0) n *= -1;

			CVector3D p(vecRoofPos[0].x, vecRoofPos[0].y, vecRoofPos[0].z);
			double d = CGeoUtil::InnerProduct(p, n);
			CVector3D inVec(0.0, 0.0, 1.0);
			double dot = CGeoUtil::InnerProduct(n, inVec);

			CVector3D center;		// ���S�̍��W
			for (auto& pos : roofData.bbPos)
			{
				// ���ʂƐ����̌�_
				CVector3D p0(pos.x, pos.y, 0.0);
				double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
				// ��_
				CVector3D tempPoint = p0 + t * inVec;
				pos.z = tempPoint.z;
				// ���S
				center += pos;
			}
			center *= 0.25;
			roofData.center = center;


			// ���b�V�����Ƃ̌v�Z���ʊi�[�p�f�[�^��ǉ�
			for (int lc = 0; lc < meshsize; lc++)
			{
				MESHPOSITION_XY meshXY = surface.meshPosList[lc];

				CMeshData mesh;

				std::vector<CVector3D> meshXYZ{
					{meshXY.leftDownX, meshXY.leftDownY, 0.0},
					{meshXY.leftTopX, meshXY.leftTopY, 0.0},
					{meshXY.rightDownX, meshXY.rightDownY, 0.0},
					{meshXY.rightTopX, meshXY.rightTopY, 0.0}
				};
				mesh.meshPos = meshXYZ;

				CVector3D center;		// ���S�̍��W
				for (auto& meshPos : mesh.meshPos)
				{
					// ���ʂƐ����̌�_
					CVector3D p0(meshPos.x, meshPos.y, 0.0);
					double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
					// ��_
					CVector3D tempPoint = p0 + t * inVec;
					meshPos.z = tempPoint.z;
					// ���S
					center += meshPos;
				}
				center *= 0.25;

				mesh.meshId = CStringEx::Format("%s_%d", surface.roofSurfaceId.c_str(), jc);
				mesh.center = center;
				mesh.centerMod = center;

				tempRoofMesh.emplace_back(mesh);

			}

			// �Ώۃ��b�V�����Ȃ�
			if (tempRoofMesh.size() == 0) { continue; }

			// �����ʂ��Ƃ̌v�Z���ʊi�[�p�f�[�^���쐬
			{
				roofData.area = areaSum;

				// �X�Ίp�ƕ��ʊp�̕��ϐݒ�
				roofData.slopeDegreeAve = slopSum / areaSum;
				roofData.azDegreeAve = azSum / areaSum;

				double slopeMdDeg = roofData.slopeDegreeAve;
				double azMdDeg = roofData.azDegreeAve;

				// �␳
				if (slopeMdDeg < m_pUIParam->_roofSurfaceCorrect.dLowerAngle)
				{
					slopeMdDeg = m_pUIParam->_roofSurfaceCorrect.dTargetAngle;
					azMdDeg = 180.0;	// �����

					// ���b�V�����W��␳
					for (auto& mesh : tempRoofMesh)
					{
						CVector3D centerMd;

						for (auto& meshPos : mesh.meshPos)
						{
							CVector3D orgPos(meshPos - mesh.center);
							orgPos.z = 0.0;
							CVector3D rotPos;
							double theta = slopeMdDeg * _COEF_DEG_TO_RAD;

							// ������ɕϊ�
							rotPos.x = orgPos.x;
							rotPos.y = orgPos.y * cos(theta);
							rotPos.z = orgPos.y * sin(theta);

							rotPos += mesh.center;
							meshPos = rotPos;

							// ���S
							centerMd += meshPos;
						}
						centerMd *= 0.25;
						mesh.centerMod = centerMd;
					}
				}

				roofData.slopeModDegree = slopeMdDeg;
				roofData.azModDegree = azMdDeg;

				// ���b�V�����X�g��ǉ�
				roofData.vecRoofMesh = tempRoofMesh;

				// �Ώۉ����ʂɒǉ�
				bldData.mapRoofSurface[surface.roofSurfaceId] = roofData;

			}
		}

		// �Ώۉ����ʂ��Ȃ�
		if (bldData.mapRoofSurface.size() == 0) { continue; }

		// ����BB�ɍ�����t�^
		std::vector<CVector3D> bbPos{
			{bbBldMinX, bbBldMinY, 0.0},
			{bbBldMinX, bbBldMaxY, 0.0},
			{bbBldMaxX, bbBldMinY, 0.0},
			{bbBldMaxX, bbBldMaxY, 0.0}
		};
		bldData.bbPos = bbPos;

		// �����S��BB�̖@��
		CVector3D n;
		CGeoUtil::OuterProduct(
			CVector3D(bbPos[1], bbPos[0]),
			CVector3D(bbPos[2], bbPos[1]), n);
		if (n.z < 0) n *= -1;

		CVector3D p(vecAllRoofPos[0].x, vecAllRoofPos[0].y, vecAllRoofPos[0].z);
		double d = CGeoUtil::InnerProduct(p, n);
		CVector3D inVec(0.0, 0.0, 1.0);
		double dot = CGeoUtil::InnerProduct(n, inVec);

		CVector3D center;		// ���S�̍��W
		for (auto& pos : bldData.bbPos)
		{
			// ���ʂƐ����̌�_
			CVector3D p0(pos.x, pos.y, 0.0);
			double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
			// ��_
			CVector3D tempPoint = p0 + t * inVec;
			pos.z = tempPoint.z;
			// ���S
			center += pos;
		}
		center *= 0.25;
		bldData.center = center;

		bldDataMap[build.building] = bldData;

	}
}


/*!	���_�Q����ߎ����ʎZ�o
@note   �����ʂ̌X�Ίp�E���ʊp
*/
bool CCalcSolarPotentialMng::calcRansacPlane(
	const std::vector<CPointBase>& vecAry,		//!< in		���_�z��
	CVector3D& vNormal							//!< out	�@��
)
{
	bool	bRet = false;

	CVector3D vec1;
	CVector3D vec2;
	// [0]����̊e�_�̃x�N�g��
	vector<CVector3D> vecPolyList;
	for (int i = 1; i < vecAry.size(); ++i)
	{
		CVector3D vec(vecAry[i], vecAry[0]);
		vecPolyList.emplace_back(vec);
	}
	sort(
		vecPolyList.begin(),
		vecPolyList.end(),
		[](const CVector3D& x, const CVector3D& y) { return x.Length() > y.Length(); }
	);
	vec1 = vecPolyList[0];
	vec1.Normalize();
	for (const auto& pos : vecPolyList)
	{
		CVector3D tempVec = pos;
		tempVec.Normalize();
		// �����������t�����̂Ƃ��͖@�����܂�Ȃ�
		if (abs(CGeoUtil::InnerProduct(vec1, tempVec)) > 0.999)
			continue;
		vec2 = tempVec;
		break;
	}
	CGeoUtil::OuterProduct(vec1, vec2, vNormal);

	if (vNormal.z < 0.0)
	{
		// ������ɔ��]
		vNormal.Inverse();
	}

	return	true;
}

// �ʐς��Z�o
double CCalcSolarPotentialMng::calcArea(const std::vector<CPointBase>& vecPos)
{
	double area = 0.0;

	CVector3D v1, v2, v3;

	if (vecPos.size() < 3)
		return area;

	for (int i = 0; i < vecPos.size() - 2; i++)
	{
		v1 = CVector3D(vecPos.at((int64_t)i + 1).x, vecPos.at((int64_t)i + 1).y, 0.0) - CVector3D(vecPos.at(0).x, vecPos.at(0).y, 0.0);
		v2 = CVector3D(vecPos.at((int64_t)i + 2).x, vecPos.at((int64_t)i + 2).y, 0.0) - CVector3D(vecPos.at(0).x, vecPos.at(0).y, 0.0);
		CGeoUtil::OuterProduct(v1, v2, v3);

		area += v3.Length() / 2.0;
	}

	return area;
}

void CCalcSolarPotentialMng::calcMonthlyRate()
{
	for (int i = 0; i < 12; i++)
	{
		int month = i + 1;
		CTime time(m_iYear, month, 0, 0, 0, 0);
	
		// �����Ƃ̉Ǝ���
		double dPossibleSunshineDuration = m_pSunshineData->GetPossibleSunshineDuration(time);

		// �����Ƃ̓��Ǝ���
		double dSunshineTime = m_pPointData->GetAverageSunshineTime(time);

		// ���V/�ܓV���̌����Ƃ̓��Ɨ����Z�o
		// ���V���̓��Ɨ�(�T)��[���Ǝ���] / [�Ǝ���]
		m_pRadiationData->sunnyRate[i] = dSunshineTime / dPossibleSunshineDuration;
		// �ܓV���̓��Ɨ�(�U)��(1 - ���V���̓��Ɨ�(�T))
		m_pRadiationData->cloudRate[i] = 1 - m_pRadiationData->sunnyRate[i];
	}
}

// ���˗ʎZ�o���ʂ̃|�C���g�f�[�^���쐬
bool CCalcSolarPotentialMng::createPointData(
	std::vector<CPointBase>& vecPoint3d,
	const std::string& Lv3meshId,
	double bbMinX,
	double bbMinY,
	double bbMaxX,
	double bbMaxY,
	double outMeshsize,		// �o�̓��b�V���T�C�Y(1.0m�ȉ�)
	const CBuildingDataMap& bldDataMap,
	const eOutputImageTarget& eTarget
)
{
	// 1m���b�V���̒������W(x,y)�Əo�͂��������˗ʎZ�o����(z)
	bool ret = false;

	vecPoint3d.clear();

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);
		if (bldList.meshID != Lv3meshId)	continue;

		int bldCount = (int)bldList.buildingList.size();
		for (int jc = 0; jc < bldCount; jc++)
		{
			BUILDINGS build = bldList.buildingList[jc];
			std::string buildId = build.building;
			if (bldDataMap.count(buildId) == 0)		continue;

			const CBuildingData& bldData = bldDataMap.at(buildId);

			int surfaceCount = (int)build.roofSurfaceList.size();
			for (int kc = 0; kc < surfaceCount; kc++)
			{
				ROOFSURFACES surface = build.roofSurfaceList[kc];
				std::string surfaceId = surface.roofSurfaceId;
				if (bldData.mapRoofSurface.count(surfaceId) == 0)	continue;

				const CRoofSurfaceData& surfaceData = bldData.mapRoofSurface.at(surfaceId);
				double dMinX = surfaceData.bbPos[0].x, dMaxX = surfaceData.bbPos[3].x;
				double dMinY = surfaceData.bbPos[0].y, dMaxY = surfaceData.bbPos[3].y;
				int iH = (int)std::ceil((dMaxY - dMinY) / outMeshsize);
				int iW = (int)std::ceil((dMaxX - dMinX) / outMeshsize);

				for (int h = 0; h < iH; h++)
				{
					double curtY = dMinY + h * (double)outMeshsize;
					if (CEpsUtil::Less(dMaxY, curtY)) curtY = dMaxY;

					for (int w = 0; w < iW; w++)
					{
						double curtX = dMinX + w * (double)outMeshsize;
						if (CEpsUtil::Less(dMaxX, curtX)) curtX = dMaxX;

						// �������Ƃɓ��O���肷��
						for (const auto& roofSurfaces : surface.roofSurfaceList)
						{
							// ���O����p
							int iCountPoint = (int)roofSurfaces.posList.size();
							CPoint2D* ppoint = new CPoint2D[iCountPoint];
							for (int n = 0; n < iCountPoint; n++)
							{
								ppoint[n] = CPoint2D(roofSurfaces.posList[n].x, roofSurfaces.posList[n].y);
							}
							CPoint2D target2d(curtX, curtY);
							bool bRet = CGeoUtil::IsPointInPolygon(target2d, iCountPoint, ppoint);

							// Z�l�ɓ��˗ʎZ�o���ʂ�ݒ肷��
							if (bRet)
							{
								switch (eTarget)
								{
								case eOutputImageTarget::SOLAR_RAD:
									vecPoint3d.emplace_back(CPointBase(curtX, curtY, surfaceData.solarRadiationUnit));
									break;

								case eOutputImageTarget::SOLAR_POWER:
									vecPoint3d.emplace_back(CPointBase(curtX, curtY, bldData.solarPowerUnit));
									break;

								default:
									return false;
								}
							}
							delete[] ppoint;
						}
					}
				}
			}
		}
	}

	double tmpMinX = DBL_MAX, tmpMaxX = -DBL_MAX, tmpMinY = DBL_MAX, tmpMaxY = -DBL_MAX;
	for (const auto& point : vecPoint3d)
	{
		tmpMinX = (tmpMinX > point.x) ? point.x : tmpMinX;
		tmpMaxX = (tmpMaxX < point.x) ? point.x : tmpMaxX;
		tmpMinY = (tmpMinY > point.y) ? point.y : tmpMinY;
		tmpMaxY = (tmpMaxY < point.y) ? point.y : tmpMaxY;
	}

	// 3�����b�V���̃T�C�Y�ŉ摜�o�͂������̂Ŏl���̍��W��ǉ�
	bool addLB, addLT, addRB, addRT;
	addLB = ((tmpMinX > bbMinX && tmpMinX < (bbMinX + 1.0)) && (tmpMinY > bbMinY && tmpMinY < (bbMinY + 1.0))) ? false : true;
	addLT = ((tmpMinX > bbMinX && tmpMinX < (bbMinX + 1.0)) && (tmpMaxY < bbMaxY && tmpMaxY > (bbMaxY - 1.0))) ? false : true;
	addRB = ((tmpMaxX < bbMaxX && tmpMaxX > (bbMaxX - 1.0)) && (tmpMinY > bbMinY && tmpMinY < (bbMinY + 1.0))) ? false : true;
	addRT = ((tmpMaxX < bbMaxX && tmpMaxX > (bbMaxX - 1.0)) && (tmpMaxY < bbMaxY && tmpMaxY > (bbMaxY - 1.0))) ? false : true;
	// ����
	if (addLB)	vecPoint3d.push_back(CPointBase(bbMinX + 0.5, bbMinY + 0.5, DEF_IMG_NODATA));
	if (addLT)	vecPoint3d.push_back(CPointBase(bbMinX + 0.5, bbMaxY - 0.5, DEF_IMG_NODATA));
	if (addRB)	vecPoint3d.push_back(CPointBase(bbMaxX - 0.5, bbMinY + 0.5, DEF_IMG_NODATA));
	if (addRT)	vecPoint3d.push_back(CPointBase(bbMaxX - 0.5, bbMaxY - 0.5, DEF_IMG_NODATA));

	if (vecPoint3d.size() > 0)	ret = true;

	return ret;
}

// �摜�o��
bool CCalcSolarPotentialMng::outputImage(
	const std::wstring strFilePath,
	const std::string& Lv3meshId,
	double bbMinX,
	double bbMinY,
	double bbMaxX,
	double bbMaxY,
	const CBuildingDataMap& bldDataMap,
	const eOutputImageTarget& eTarget
)
{
	if (IsCancel())		return false;

	bool ret = false;

	double outMeshsize = 1.0;

	// �o�͗p�f�[�^���쐬
	std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>;
	ret = createPointData(*pvecPoint3d, Lv3meshId, bbMinX, bbMinY, bbMaxX, bbMaxY, outMeshsize, bldDataMap, eTarget);

	std::wstring strColorSettingPath = L"";
	if (eTarget == eOutputImageTarget::SOLAR_RAD)
	{
		strColorSettingPath = L"colorSetting_SolarRad.txt";
	}
	else if (eTarget == eOutputImageTarget::SOLAR_POWER)
	{
		strColorSettingPath = L"colorSetting_SolarPower.txt";
	}

	CTiffDataManager tiffDataMng;
	tiffDataMng.SetColorSetting(strColorSettingPath);
	tiffDataMng.SetMeshSize((float)outMeshsize);
	tiffDataMng.SetNoDataVal(DEF_IMG_NODATA);
	tiffDataMng.SetEPSGCode(DEF_EPSGCODE);
	tiffDataMng.SetFilePath(strFilePath);
	ret &= tiffDataMng.AddTiffData(pvecPoint3d);

	// TIFF�摜�쐬
	if (ret)
	{
		ret &= tiffDataMng.Create();
	}

	// JPEG�o��
	if (ret && eTarget == eOutputImageTarget::SOLAR_RAD)
	{
		ret &= CImageUtil::ConvertTiffToJpeg(strFilePath);
	}

	return ret;
}

bool CCalcSolarPotentialMng::outputLegendImage()
{
	bool ret = false;

	std::wstring strColorSettingPath = L"";
	std::wstring strMdlPath = CFileUtil::GetModulePathW();
	std::wstring strOutDir = m_pUIParam->strOutputDirPath;

	// ���˗�
	strColorSettingPath = L"colorSetting_SolarRad.txt";
	ret = CImageUtil::CreateLegendImage(strColorSettingPath, L"���˗�(kWh/m2)");
	if (ret)
	{
		std::wstring colorSetting = CFileUtil::Combine(strMdlPath, strColorSettingPath);
		std::wstring srcPath = GetFUtil()->ChangeFileNameExt(colorSetting, L".jpg");

		std::wstring tmpPath = CFileUtil::Combine(strOutDir, strColorSettingPath);
		std::wstring dstPath = GetFUtil()->ChangeFileNameExt(tmpPath, L".jpg");

		if (MoveFile(srcPath.c_str(), dstPath.c_str()) == FALSE)
		{
			return false;
		}
	}

	// ���d��
	strColorSettingPath = L"colorSetting_SolarPower.txt";
	ret &= CImageUtil::CreateLegendImage(strColorSettingPath, L"���d��(kWh/m2)");
	if (ret)
	{
		std::wstring colorSetting = CFileUtil::Combine(strMdlPath, strColorSettingPath);
		std::wstring srcPath = GetFUtil()->ChangeFileNameExt(colorSetting, L".jpg");

		std::wstring tmpPath = CFileUtil::Combine(strOutDir, strColorSettingPath);
		std::wstring dstPath = GetFUtil()->ChangeFileNameExt(tmpPath, L".jpg");

		if (MoveFile(srcPath.c_str(), dstPath.c_str()) == FALSE)
		{
			return false;
		}
	}

	return ret;
}


// �N�ԓ��˗ʁE���d�ʂ��o��
bool CCalcSolarPotentialMng::outputResultCSV()
{
	if (m_pmapResultData->empty()) return false;

	std::wstring strOutDir = m_pUIParam->strOutputDirPath;
	if (!GetFUtil()->IsExistPath(strOutDir))
	{
		if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}

	CFileIO file;
	std::wstring strPath = GetFUtil()->Combine(strOutDir, L"�������N�ԗ\�����d��.csv");
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}

	// �w�b�_��
	if (!file.WriteLineA("3�����b�V��ID,����ID,�N�ԗ\�����˗�(kWh/m2),�N�ԗ\�����d��(kWh),�p�l���ʐ�,�N�ԗ\�����d��(kWh/m2),X,Y"))
	{
		return false;
	}

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);
		std::string meshId = bldList.meshID;
		if (m_pmapResultData->count(meshId) == 0)	continue;
		const CBuildingDataMap& bldDataMap = m_pmapResultData->at(meshId);

		int bldCount = (int)bldList.buildingList.size();
		for (int jc = 0; jc < bldCount; jc++)
		{
			BUILDINGS build = bldList.buildingList[jc];
			std::string buildId = build.building;
			if (bldDataMap.count(buildId) == 0)	continue;

			const CBuildingData& bldData = bldDataMap.at(buildId);
			std::string strLine = CStringEx::Format("%s,%s,%f,%f,%f,%f,%f,%f",
				meshId.c_str(),
				buildId.c_str(),
				bldData.solarRadiationUnit,
				bldData.solarPower,
				bldData.panelArea,
				bldData.solarPowerUnit,
				bldData.center.x,
				bldData.center.y
			);
			file.WriteLineA(strLine);
		}
	}

	file.Close();

	return true;
}

// ���ʓ��˗�CSV�o��
bool CCalcSolarPotentialMng::outputMonthlyRadCSV(
	const std::string& Lv3meshId,
	const CBuildingDataMap& dataMap, 
	const std::wstring& wstrOutDir
)
{
	if (IsCancel())	return false;

	if (!GetFUtil()->IsExistPath(wstrOutDir))
	{
		if (CreateDirectory(wstrOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}
	// �f�o�b�O�p�o��
	std::wstring strPath = GetFUtil()->Combine(wstrOutDir, L"���ʓ��˗�_�p�x�␳.csv");

	CFileIO file;
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}

	// �w�b�_��
	if (!file.WriteLineA("����ID,������ID,MeshId,�N,��,���˗�(Wh/m2),���˗�(MJ/m2)"))
	{
		return false;
	}

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);
		if (bldList.meshID != Lv3meshId)	continue;

		int bldCount = (int)bldList.buildingList.size();
		for (int jc = 0; jc < bldCount; jc++)
		{
			BUILDINGS build = bldList.buildingList[jc];
			std::string buildId = build.building;
			if (dataMap.count(buildId) == 0)	continue;

			const CBuildingData& bldData = dataMap.at(buildId);

			int surfaceCount = (int)build.roofSurfaceList.size();
			for (int kc = 0; kc < surfaceCount; kc++)
			{
				ROOFSURFACES surface = build.roofSurfaceList[kc];
				std::string surfaceId = surface.roofSurfaceId;
				if (bldData.mapRoofSurface.count(surfaceId) == 0)	continue;

				const CRoofSurfaceData& surfaceData = bldData.mapRoofSurface.at(surfaceId);

				for (auto& mesh : surfaceData.vecRoofMesh)
				{
					for (int month = 1; month <= 12; month++)
					{
						double val = mesh.solarRadiation[month - 1];

						std::string strLine = CStringEx::Format("%s,%s,%s,%d,%d,%f,%f",
							buildId.c_str(),
							surfaceId.c_str(),
							mesh.meshId.c_str(),
							GetYear(),
							month,
							val,
							val / 1000 * 3.6
						);
						file.WriteLineA(strLine);
					}
				}

			}
		}
	}
	file.Close();

	return true;
}


// ���ʓ��˗�CSV�o��
bool CCalcSolarPotentialMng::outputRoofRadCSV(
	const std::string& Lv3meshId,			// 3�����b�V��ID
	const CBuildingDataMap& dataMap,
	const std::wstring& wstrOutDir			// �o�̓t�H���_
)
{
	if (IsCancel())	return false;

	if (!GetFUtil()->IsExistPath(wstrOutDir))
	{
		if (CreateDirectory(wstrOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}
	// �f�o�b�O�p�o��
	std::wstring strPath = GetFUtil()->Combine(wstrOutDir, L"�����ʕʔN�ԓ��˗�.csv");

	CFileIO file;
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}
	// �w�b�_��
	if (!file.WriteLineA("����ID,������ID,�N,1m2������̓��˗�(kWh/m2),��(MJ/m2),�����ʑS�̂̓��˗�(kWh),��(MJ)"))
	{
		return false;
	}

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);
		if (bldList.meshID != Lv3meshId)	continue;

		int bldCount = (int)bldList.buildingList.size();
		for (int jc = 0; jc < bldCount; jc++)
		{
			BUILDINGS build = bldList.buildingList[jc];
			std::string buildId = build.building;
			if (dataMap.count(buildId) == 0)	continue;

			const CBuildingData& bldData = dataMap.at(buildId);

			int surfaceCount = (int)build.roofSurfaceList.size();
			for (int kc = 0; kc < surfaceCount; kc++)
			{
				ROOFSURFACES surface = build.roofSurfaceList[kc];
				std::string surfaceId = surface.roofSurfaceId;
				if (bldData.mapRoofSurface.count(surfaceId) == 0)	continue;

				const CRoofSurfaceData& surfaceData = bldData.mapRoofSurface.at(surfaceId);

				// �f�o�b�O�p�o��
				std::string strLine = CStringEx::Format("%s,%s,%d,%f,%f,%f,%f",
					buildId.c_str(),
					surfaceId.c_str(),
					GetYear(),
					surfaceData.solarRadiationUnit,
					surfaceData.solarRadiationUnit * 3.6,
					surfaceData.solarRadiation,
					surfaceData.solarRadiation * 3.6
				);
				file.WriteLineA(strLine);
			}
		}
	}
	file.Close();

	return true;
}


// �������Ƃ̕��ʊp���ԃf�[�^�o��
bool CCalcSolarPotentialMng::outputAzimuthDataCSV()
{
	if (m_pmapResultData->empty())	return false;

	bool ret = false;

	// �o�̓p�X
	std::wstring strCsvPath = GetINIParam()->GetAzimuthCSVPath();
	std::wstring strTempDir = GetFUtil()->GetParentDir(m_pUIParam->strOutputDirPath) + L"output";
	std::wstring strPath = GetFUtil()->Combine(strTempDir, strCsvPath);

	// �o�͗p�t�H���_�쐬
	std::wstring strParentDir = GetFUtil()->GetParentDir(strPath);
	if (!GetFUtil()->IsExistPath(strParentDir))
	{
		if (CreateDirectory(strParentDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}

	CFileIO file;
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}

	// �w�b�_��
	if (!file.WriteLineA("3�����b�V��ID,����ID,���ꉮ���ʐ�,���ʊp(���ϒl)"))
	{
		return false;
	}

	// �������Ƃ̕��ʊp�f�[�^��������
	for (const auto& result : *m_pmapResultData)
	{
		std::string meshId = result.first;
		const CBuildingDataMap& bldDataMap = result.second;

		for (const auto& bldMap : bldDataMap)
		{
			std::string buildId = bldMap.first;
			const CBuildingData& bldData = bldMap.second;

			int roofsize = (int)bldData.mapRoofSurface.size();
			if (roofsize == 0)	continue;

			std::string strAzimuths = "";
			for (auto val : bldData.mapRoofSurface)
			{
				CRoofSurfaceData roofData = val.second;
				std::string strTemp = CStringEx::Format("%f,", roofData.azModDegree);
				strAzimuths += strTemp;
			}

			// ����ID, ���ꉮ���ʐ�, ���ʊp(���ϒl)�E�E�E"
			std::string strLine = CStringEx::Format("%s,%s,%d,%s", meshId.c_str(), buildId.c_str(), roofsize, strAzimuths.c_str());

			file.WriteLineA(strLine);
		}
	}

	file.Close();

	return true;

}


// SHP�ɕt�^
bool CCalcSolarPotentialMng::setTotalSolarRadiationToSHP()
{
	return true;

}

// �����̒��S�ɓ��ˌ����������Ă��邩
bool CCalcSolarPotentialMng::IntersectRoofSurfaceCenter(
	const CVector3D& inputVec,						// ���ˌ�
	const std::vector<CVector3D>& roofMesh,			// �Ώۂ̉���BB
	const std::string& strId,						// �Ώۂ̉���ID
	const vector<BLDGLIST>& neighborBuildings,		// ���ӂ̌������X�g
	const vector<DEMLIST>& neighborDems				// ���ӂ̒n�`DEM���X�g
)
{
	// �����̗L������
	constexpr double LIGHT_LENGTH = 500.0;

	// �������b�V���̍��W
	CVector3D roofMeshPos;
	for (const auto& mesh : roofMesh)
		roofMeshPos += mesh;
	roofMeshPos *= 0.25;	// 4�_�̕���
	// �������b�V���̖@��
	CVector3D n;
	CGeoUtil::OuterProduct(
		CVector3D(roofMesh[1], roofMesh[0]),
		CVector3D(roofMesh[2], roofMesh[1]), n);
	if (n.z < 0) n *= -1;

	// �����ʃ��b�V���̗���������ˌ����������Ă���Ƃ��͔��˂��Ȃ��̂ŉ�͏I��
	if (CGeoUtil::InnerProduct(n, inputVec) >= 0.0)
		return false;

	// ���ˌ��̌������Z�o
	// �������b�V�����W�̉�������500m�ɐݒ肷��
	CVector3D inputInverseVec = CGeoUtil::Normalize(inputVec) * ((-1) * LIGHT_LENGTH);
	CVector3D sunPos = roofMeshPos + inputInverseVec;
	CLightRay lightRay(sunPos, CGeoUtil::Normalize(inputVec) * LIGHT_LENGTH);

	// ���ˌ�������̌����Ɏז����ꂸ�ɉ����ʂɓ����邩�`�F�b�N
	if (intersectBuildings(lightRay, strId, neighborBuildings))
	{
		// �����ɂ������Ă���ꍇ�͉������b�V���Ɍ������������Ă��Ȃ��̂ŉ�͏I��
		return false;
	}

	// ���ˌ�������̒n�`�Ɏז����ꂸ�Ɍ����ɓ����邩�`�F�b�N
	if (IsEnableDEMData() && roofMeshPos.z > GetINIParam()->GetDemHeight())
	{
		if (intersectLandDEM(lightRay, neighborDems))
		{
			return false;
		}
	}

	return true;
}

// �����Q�Ɍ������������Ă��邩
bool CCalcSolarPotentialMng::intersectBuildings(
	const CLightRay& lightRay,
	const std::string& strId,
	const std::vector<BLDGLIST>& buildingsList
)
{
	for (const auto& bldglist : buildingsList)
	{
		// LOD2
		const vector<BUILDINGS>& buildings = bldglist.buildingList;
		for (const auto& building : buildings)
		{
			if (intersectBuilding(lightRay, strId, building))
			{
				return true;
			}
		}

		// LOD1
		const vector<BUILDINGSLOD1>& buildingsLOD1 = bldglist.buildingListLOD1;
		for (const auto& building : buildingsLOD1)
		{
			if (intersectBuilding(lightRay, building.wallSurfaceList))
			{
				return true;
			}
		}
	}

	return false;
}

// �����Ɍ������������Ă��邩�ǂ���
bool CCalcSolarPotentialMng::intersectBuilding(
	const CLightRay& lightRay,
	const vector<WALLSURFACES>& wallSurfaceList
)
{
	// ������������艓�����Ȃ������ׂ�
	if (!checkDistance(lightRay, wallSurfaceList))
		return false;

	double tempDist;
	CVector3D tempTargetPos;
	for (const auto& wall : wallSurfaceList)
	{
		for (const auto& polygon : wall.wallSurfaceList)
		{
			vector<CVector3D> posList(polygon.posList.size());
			int i = 0;
			for (const auto& pos : polygon.posList)
			{
				posList[i] = CVector3D(pos.x, pos.y, pos.z);
				++i;
			}

			// �����ƃ|���S���̌�_��T��
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}
		}
	}

	return false;
}

// �����Ɍ������������Ă��邩�ǂ���
bool CCalcSolarPotentialMng::intersectBuilding(
	const CLightRay& lightRay,
	const std::string& strId,
	const BUILDINGS& buildiings
)
{
	// ������������艓�����Ȃ������ׂ�
	if (!checkDistance(lightRay, buildiings.wallSurfaceList))
		return false;

	double tempDist;
	CVector3D tempTargetPos;

	// ����
	for (const auto& roof : buildiings.roofSurfaceList)
	{
		if (strId == roof.roofSurfaceId)	continue;	// ���g�̉����͏��O

		for (const auto& polygon : roof.roofSurfaceList)
		{
			vector<CVector3D> posList(polygon.posList.size());
			int i = 0;
			for (const auto& pos : polygon.posList)
			{
				posList[i] = CVector3D(pos.x, pos.y, pos.z);
				++i;
			}

			// �����ƃ|���S���̌�_��T��
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}
		}
	}

	// ��
	for (const auto& wall : buildiings.wallSurfaceList)
	{
		for (const auto& polygon : wall.wallSurfaceList)
		{
			vector<CVector3D> posList(polygon.posList.size());
			int i = 0;
			for (const auto& pos : polygon.posList)
			{
				posList[i] = CVector3D(pos.x, pos.y, pos.z);
				++i;
			}

			// �����ƃ|���S���̌�_��T��
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}
		}
	}

	return false;
}

// �����������͈͓̔�����܂��ɂ܂��͔��肷��
bool CCalcSolarPotentialMng::checkDistance(const CLightRay& lightRay, const vector<WALLSURFACES>& wallSurfaceList)
{
	// �������͈͓������肷�鋗���͈�
	constexpr double LIGHT_LENGTH = 550.0; //�]�T����������
	constexpr double SQUARE_LINGHT_LENGTH = LIGHT_LENGTH * LIGHT_LENGTH;

	const CVector3D lightRayPos = lightRay.GetPos();
	const CVector3D lightRayVec = lightRay.GetVector();

	bool bDist = false;
	bool bDirect = false;
	for (const auto& wall : wallSurfaceList)
	{
		for (const auto& polygon : wall.wallSurfaceList)
		{
			for (const auto& pos : polygon.posList)
			{
				double dx = pos.x - lightRayPos.x;
				double dy = pos.y - lightRayPos.y;
				double dz = pos.z - lightRayPos.z;

				// ���ʂ̒��_���t�����ɂ���Ƃ��͔͈͊O�Ƃ���
				double dot = CGeoUtil::InnerProduct(lightRayVec, CVector3D(dx, dy, dz));
				if (dot < 0.0)	continue;

				// �������������Ȃ����`�F�b�N
				double len = calcLength(dx, dy, dz);
				if (len > SQUARE_LINGHT_LENGTH)	continue;

				return true;
			}
		}
	}

	return false;
}

// �Ώی����ɗאڂ��郁�b�V�����擾
void CCalcSolarPotentialMng::GetNeighborBuildings(
	const std::string& targetMeshId,
	const CVector3D& bldCenter,
	std::vector<BLDGLIST>& neighborBuildings
)
{
	const double DIST = GetINIParam()->GetNeighborBuildDist_SolarRad();	// �אڂ���BBox�͈̔�[m]

	// �������SXY
	CVector2D bldCenterXY(bldCenter.x, bldCenter.y);

	for (const auto& building : *m_pvecAllBuildList)
	{
		if (!isNeighborMesh(targetMeshId, building.meshID))	continue;

		BLDGLIST tmpBldList = building;
		tmpBldList.buildingList.clear();
		tmpBldList.buildingListLOD1.clear();

		// �͈͓��ɂ��邩
		// LOD2
		for (const auto& build : building.buildingList)
		{
			double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
			double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;

			// �����S�̂�BB�����߂�
			for (const auto& surface : build.roofSurfaceList)
			{
				for (const auto& member : surface.roofSurfaceList)
				{
					for (const auto& pos : member.posList)
					{
						if (pos.x < bbBldMinX)	bbBldMinX = pos.x;
						if (pos.y < bbBldMinY)	bbBldMinY = pos.y;
						if (pos.x > bbBldMaxX)	bbBldMaxX = pos.x;
						if (pos.y > bbBldMaxY)	bbBldMaxY = pos.y;
					}
				}
			}

			double buildCenterX = ((int64_t)bbBldMaxX + (int64_t)bbBldMinX) * 0.5;
			double buildCenterY = ((int64_t)bbBldMaxY + (int64_t)bbBldMinY) * 0.5;
			// ���S���m�̋���
			double tmpdist = bldCenterXY.Distance(buildCenterX, buildCenterY);
			// DIST�ȓ��̋����̂Ƃ��ߗׂƂ���
			if (tmpdist <= DIST)
			{
				tmpBldList.buildingList.emplace_back(build);
				continue;
			}
		}

		// LOD1
		for (const auto& build : building.buildingListLOD1)
		{
			double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
			double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;

			// �����S�̂�BB�����߂�
			for (const auto& wall : build.wallSurfaceList)
			{
				for (const auto& member : wall.wallSurfaceList)
				{
					for (const auto& pos : member.posList)
					{
						if (pos.x < bbBldMinX)	bbBldMinX = pos.x;
						if (pos.y < bbBldMinY)	bbBldMinY = pos.y;
						if (pos.x > bbBldMaxX)	bbBldMaxX = pos.x;
						if (pos.y > bbBldMaxY)	bbBldMaxY = pos.y;
					}
				}
			}

			double buildCenterX = ((int64_t)bbBldMaxX + (int64_t)bbBldMinX) * 0.5;
			double buildCenterY = ((int64_t)bbBldMaxY + (int64_t)bbBldMinY) * 0.5;
			// ���S���m�̋���
			double tmpdist = bldCenterXY.Distance(buildCenterX, buildCenterY);
			// DIST�ȓ��̋����̂Ƃ��ߗׂƂ���
			if (tmpdist <= DIST)
			{
				tmpBldList.buildingListLOD1.emplace_back(build);
				continue;
			}
		}

		if (tmpBldList.buildingList.empty() && tmpBldList.buildingListLOD1.empty())	continue;

		neighborBuildings.emplace_back(tmpBldList);
	}

}

// �Ώی����ɗאڂ���DEM���擾
void CCalcSolarPotentialMng::GetNeighborDems(
	const std::string& targetMeshId,
	const CVector3D& bldCenter,
	std::vector<DEMLIST>& neighborDems
)
{
	// ���O����
	if (bldCenter.z < GetINIParam()->GetDemHeight())	return;

	const double DIST = GetINIParam()->GetDemDist();	// �Ώ۔͈�[m]

	// �������SXY
	CVector2D bldCenterXY(bldCenter.x, bldCenter.y);

	for (const auto& dem : *m_pvecAllDemList)
	{
		if (isNeighborMesh(targetMeshId, dem.meshID))
		{
			DEMLIST targetDem;
			targetDem.meshID = dem.meshID;
			targetDem.posTriangleList.clear();

			for (const auto& triangle : dem.posTriangleList)
			{
				// XY���ʂ̏d�S�����߂�
				double x = (triangle.posTriangle[0].x + triangle.posTriangle[1].x + triangle.posTriangle[2].x) / 3.0;
				double y = (triangle.posTriangle[0].y + triangle.posTriangle[1].y + triangle.posTriangle[2].y) / 3.0;
				double z = (triangle.posTriangle[0].z + triangle.posTriangle[1].z + triangle.posTriangle[2].z) / 3.0;

				double tmpdist = bldCenterXY.Distance(x, y);

				bool bAddList = false;

				// ����
				if (tmpdist <= DIST &&	// �Ώۂ̌���(���S)�Ƃ̋������Ώ۔͈͓�
					bldCenter.z < z)	// �Ώۂ̌���(���S)��荂���ʒu�ɂ���
				{
					targetDem.posTriangleList.emplace_back(triangle);
				}
			}

			// �Ώ۔͈͂ŊԈ�����DEM��ǉ�����
			neighborDems.emplace_back(targetDem);
		}

	}
}


// �n�`�Ɍ������������Ă��邩�ǂ���
bool CCalcSolarPotentialMng::intersectLandDEM(
	const CLightRay& lightRay,					// ����
	//const double& height,						// �����W��
	const vector<DEMLIST>& demList				// �������������Ă��邩�`�F�b�N����n�`��DEM
)
{
	double tempDist;
	CVector3D tempTargetPos;

	const CVector3D lightRayPos = lightRay.GetPos();
	const CVector3D lightRayVec = lightRay.GetVector();

	for (const auto& dem : demList)
	{
		for (const auto& triangle : dem.posTriangleList)
		{
			if (IsCancel())		return false;

			vector<CVector3D> posList;

			posList.emplace_back(CVector3D(triangle.posTriangle[0].x, triangle.posTriangle[0].y, triangle.posTriangle[0].z));
			posList.emplace_back(CVector3D(triangle.posTriangle[1].x, triangle.posTriangle[1].y, triangle.posTriangle[1].z));
			posList.emplace_back(CVector3D(triangle.posTriangle[2].x, triangle.posTriangle[2].y, triangle.posTriangle[2].z));

			bool bDirect = false;
			for (const auto& pos : triangle.posTriangle)
			{
				double dx = pos.x - lightRayPos.x;
				double dy = pos.y - lightRayPos.y;
				double dz = pos.z - lightRayPos.z;

				// ���ʂ̒��_���t�����ɂ���Ƃ��͔͈͊O�Ƃ���
				double dot = CGeoUtil::InnerProduct(lightRayVec, CVector3D(dx, dy, dz));
				if (dot > 0.0)
				{
					bDirect = true;
					break;
				}
			}
			if (!bDirect)	continue;

			// �����ƃ|���S���̌�_��T��
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}

		}
	}

	return false;
}

bool CCalcSolarPotentialMng::IsCancel()
{
	return GetFUtil()->IsExistPath(m_strCancelFilePath);
}

bool CCalcSolarPotentialMng::IsEnableDEMData()
{
	return m_pUIParam->bEnableDEMData;
}

// 3�����b�V�����א�(����8����)���Ă��邩
bool CCalcSolarPotentialMng::isNeighborMesh(const std::string& meshId1, const std::string& meshId2)
{
	bool bret = false;

	// �������b�V��
	if (meshId1 == meshId2)
	{
		return true;
	}

	// ��2���̐������烁�b�V���̈ʒu�p�^�[���𔻒�
	bool bTop = (meshId1.substr(6, 1) == "9") ? true : false;		// 7���ڂ�9�Ȃ�k��
	bool bBottom = (meshId1.substr(6, 1) == "0") ? true : false;	// 7���ڂ�0�Ȃ�쑤
	bool bLeft = (meshId1.substr(7, 1) == "0") ? true : false;		// 8���ڂ�9�Ȃ琼��
	bool bRight = (meshId1.substr(7, 1) == "9") ? true : false;		// 8���ڂ�9�Ȃ瓌��

	int nRet = 0;

	//�k
	if (bTop) nRet = (stoi(meshId1) + 1000 - 90);
	else      nRet = (stoi(meshId1) + 10);
	std::string strId_N = CStringEx::Format("%d", nRet);

	//��
	if (bBottom) nRet = (stoi(meshId1) - 1000 + 90);
	else      nRet = (stoi(meshId1) - 10);
	std::string strId_S = CStringEx::Format("%d", nRet);

	//��
	if (bRight) nRet = (stoi(meshId1) + 100 - 9);
	else      nRet = (stoi(meshId1) + 1);
	std::string strId_E = CStringEx::Format("%d", nRet);

	//��
	if (bLeft) nRet = (stoi(meshId1) - 100 + 9);
	else      nRet = (stoi(meshId1) - 1);
	std::string strId_W = CStringEx::Format("%d", nRet);

	//�k��
	if (bTop && bRight) nRet = (stoi(meshId1) + 1000 + 1);
	else if (bRight) nRet = (stoi(strId_E) + 10);
	else      nRet = (stoi(strId_N) + 1);
	std::string strId_NE = CStringEx::Format("%d", nRet);

	//�쓌
	if (bBottom && bRight) nRet = (stoi(strId_S) + 100 - 9);
	else if (bRight) nRet = (stoi(strId_E) - 10);
	else      nRet = (stoi(strId_S) + 1);
	std::string strId_SE = CStringEx::Format("%d", nRet);

	//�쐼
	if (bBottom && bLeft) nRet = (stoi(meshId1) - 1000 - 1);
	else if (bLeft) nRet = (stoi(strId_W) - 10);
	else      nRet = (stoi(strId_S) - 1);
	std::string strId_SW = CStringEx::Format("%d", nRet);

	//�k��
	if (bTop && bLeft) nRet = (stoi(strId_N) - 100 + 9);
	else if (bLeft) nRet = (stoi(strId_W) + 10);
	else      nRet = (stoi(strId_N) - 1);
	std::string strId_NW = CStringEx::Format("%d", nRet);

	if (strId_N == meshId2)			return true;
	else if (strId_S == meshId2)	return true;
	else if (strId_E == meshId2)	return true;
	else if (strId_W == meshId2)	return true;
	else if (strId_NE == meshId2)	return true;
	else if (strId_SE == meshId2)	return true;
	else if (strId_SW == meshId2)	return true;
	else if (strId_NW == meshId2)	return true;

	return false;
}
