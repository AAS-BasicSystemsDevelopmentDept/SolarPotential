#include "pch.h"
#include <math.h>
#include <random>
#include <iostream>
#include <unordered_map>
#include "CalcSolarRadiation.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CSunVector.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"

// ���˗ʌv�Z�p�����[�^
// ���z�萔
const double def_Sconst = 1367.0;

CCalcSolarRadiation::CCalcSolarRadiation
(
	CCalcSolarPotentialMng* pMng
)
	: m_pMng(pMng)
{

}

CCalcSolarRadiation::~CCalcSolarRadiation(void)
{

}

// 3.���z�O�������Ƃɂ������˗ʂ̎Z�o
bool CCalcSolarRadiation::Exec(
	CBuildingDataMap& dataMap,				// �v�Z���ʊi�[�p�f�[�^�}�b�v
	const bool& bModDegree,					// true:�␳�p�x�f�[�^���g�p
	const std::wstring& wstrPath,			// ����CSV�o�̓t�@�C����
	const std::string Lv3meshId				// 3�����b�V��ID
)
{
	if (!m_pMng)	return false;

	int JPZONE = GetINIParam()->GetJPZone();

	int iYear = m_pMng->GetYear();

	for (auto& bld : dataMap)
	{
		// �L�����Z�����m
		if (m_pMng->IsCancel())
		{
			return false;
		};

		CBuildingData& bldData = bld.second;
		const CVector3D bldCenter = bldData.center;

		// �אڂ��錚�����擾
		std::vector<BLDGLIST> neighborBuildings;
		m_pMng->GetNeighborBuildings(Lv3meshId, bldCenter, neighborBuildings);

		// �אڂ���DEM���擾
		std::vector<DEMLIST> neighborDems;
		if (m_pMng->IsEnableDEMData())
		{
			m_pMng->GetNeighborDems(Lv3meshId, bldCenter, neighborDems);
		}

		for (auto& surface : bldData.mapRoofSurface)
		{
			std::string surfaceId = surface.first;
			CRoofSurfaceData& surfaceData = surface.second;

			// ���B�����̗L���𔻒�
			std::unordered_map<int, bool> mapSunDir;
			double dLat = 0.0, dLon = 0.0;
			CGeoUtil::XYToLatLon(JPZONE, surfaceData.center.y, surfaceData.center.x, dLat, dLon);

			for (auto& mesh : surfaceData.vecRoofMesh)
			{
				double slopeDegree = 0.0, azDegree = 0.0;	// �X�Ίp�A���ʊp

				if (!bModDegree)
				{	// �␳���Ȃ�
					slopeDegree = surfaceData.slopeDegreeAve;
					azDegree = surfaceData.azDegreeAve;
				}
				else
				{	// �␳����
					slopeDegree = surfaceData.slopeModDegree;
					azDegree = surfaceData.azModDegree;
				}

				double surfaceAngle = slopeDegree * _COEF_DEG_TO_RAD;	// �Ζʂ̌X�Ίp
				double surfaceAz = azDegree * _COEF_DEG_TO_RAD;			// �Ζʂ̕��ʊp

				for (int month = 1; month <= 12; month++)
				{
					double dSunnyVal = 0.0; double dCloudVal = 0.0;

					double p = GetINIParam()->GetTransmissivity(month);		// ��C���ߗ�
					double pdifCloud = 1.0 - p;								// ���ߗ�(�ܓV��)

					int dnum = CTime::GetDayNum(month);
					for (int day = 1; day <= dnum; day++)
					{
						CTime date(iYear, month, day, 0, 0, 0);
						const CSunVector outSun(dLat, dLon, date);

						for (int hour = 0; hour < 24; hour++)
						{
							HorizontalCoordinate pos;
							outSun.GetPos(hour, pos);
							double sunAngle = pos.altitude;	// ���z�V���p(���z���x)	
							if (sunAngle < 0)
							{
								//���z���x��0����
								continue;
							}
							double alpha = pos.azimuth + _PI;		// ���z����

							int idx = date.iYDayCnt * 24 + hour;

							// ���˗�
							double refRate = 0.0;	// ���˗�
							date.iHour = hour;
							double depth = this->m_pMng->GetMetpvData()->GetSnowDepth(date);
							if (depth > 10.0)		// 10cm�ȏ�
							{
								refRate = GetINIParam()->GetReflectivitySnow();
							}
							else
							{
								refRate = GetINIParam()->GetReflectivity();
							}

							// ���ˊp
							double angIn = calcAngleIn(sunAngle, surfaceAngle, surfaceAz, alpha);
							if (angIn < 0.0)	angIn = 0.0;

							CVector3D sunVector;
							outSun.GetVector(hour, sunVector);

							// ���B�����̗L���𔻒�
							if (mapSunDir.count(idx) == 0)
							{
								mapSunDir[idx] = m_pMng->IntersectRoofSurfaceCenter(sunVector, surfaceData.bbPos, surfaceId, neighborBuildings, neighborDems);
							}

							bool bDirect = mapSunDir.at(idx);

							dSunnyVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, p);
							dCloudVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, pdifCloud);

						}
					}

					// �Ђƌ����Ƃɒl��K�p
					mesh.solarRadiationSunny[month - 1] = dSunnyVal;
					mesh.solarRadiationCloud[month - 1] = dCloudVal;
				}

			}
			mapSunDir.clear();
		}
	}

	return true;
}

bool CCalcSolarRadiation::ModifySunRate
(
	CBuildingDataMap& dataMap,				// �v�Z���ʊi�[�p�f�[�^�}�b�v
	const std::wstring& wstrPath			// ����CSV�o�̓t�@�C����
)
{
	if (!m_pMng)	return false;

	for (auto& bld : dataMap)
	{
		// �L�����Z�����m
		if (m_pMng->IsCancel())
		{
			return false;
		};

		std::string buildId = bld.first;
		CBuildingData& bldData = bld.second;

		for (auto& surface : bldData.mapRoofSurface)
		{
			std::string surfaceId = surface.first;
			CRoofSurfaceData& surfaceData = surface.second;

			for (auto& mesh : surfaceData.vecRoofMesh)
			{
				for (int month = 1; month <= 12; month++)
				{
					// ���V���̓��˗� �~ ���V���̓��Ɨ�(�T)
					double sunnyVal = mesh.solarRadiationSunny[month - 1] * m_pMng->GetRadiationData()->sunnyRate[month - 1];
					// �ܓV���̓��˗� �~ �ܓV���̓��Ɨ�(�U)
					double cloudVal = mesh.solarRadiationCloud[month - 1] * m_pMng->GetRadiationData()->cloudRate[month - 1];
					double val = sunnyVal + cloudVal;
					mesh.solarRadiation[month - 1] = val;	// �␳�������˗ʂ�K�p
				}
			}
		}
	}

	return true;
}

// �Ζʓ��˗ʂ̎Z�o
double CCalcSolarRadiation::calcSlopeDif(
	const double& surfaceAngle,	// �X�Ίp
	const double& angIn,		// ���ˊp
	const double& sunAngle,		// ���z���x
	const double& refRate,		// ���˗�
	const bool& bDirect,		// ���B�����̗L��
	const double& pdif			// ���ߗ�
)
{
	// �@���ʒ��B���˗�(W/m2)
	double dDN = 0.0;
	if (bDirect)	dDN = calcDirectNormal(sunAngle, pdif);

	// �����ʓV����˗�(W/m2)
	double dSH = calcSkyHorizon(sunAngle, pdif);

	// �Ζʒ��B���˗�
	double dDT = 0.0;
	if (bDirect)	dDT = calcDirectSlope(dDN, angIn);

	// �ΖʓV����˗�
	double dST = calcSkySlope(dSH, surfaceAngle);

	// �����ʑS�V���˗�
	double dTH = calcSolarHorizon(dDN, dSH, sunAngle);

	// �Ζʂɓ��˂��锽�˓��˗�
	double dRT = calcRefrectSlope(dTH, surfaceAngle, refRate);

	// �ΖʑS�V���˗�
	double dTT = dDT + dST + dRT;

	return dTT;
}

// ���ˊp�Z�o
inline double CCalcSolarRadiation::calcAngleIn(double sunAngle, double surfaceAngle, double surfaceAz, double alpha)
{
	return sin(sunAngle) * cos(surfaceAngle) + cos(sunAngle) * sin(surfaceAngle) * cos(alpha - surfaceAz);
}

// �@���ʒ��B���˗�(W/m2)
inline double CCalcSolarRadiation::calcDirectNormal(double sunAngle, double pdif)
{
	double m = 1 / sin(sunAngle);
	double betaM = pow(pdif, m);
	return def_Sconst * betaM;
}

// �����ʓV����˗�(W/m2)
inline double CCalcSolarRadiation::calcSkyHorizon(double sunAngle, double pdif)
{
	double m = 1 / sin(sunAngle);
	double betaM = pow(pdif, m);
	double dN = 1.0 - betaM;
	double dD = 1.0 - 1.4 * log(pdif);
	return def_Sconst * sin(sunAngle) * (dN / dD) * 0.5;
}

// �Ζʒ��B���˗�
inline double CCalcSolarRadiation::calcDirectSlope(double dDN, double angIn)
{
	return dDN * angIn;
}

// �ΖʓV����˗�
inline double CCalcSolarRadiation::calcSkySlope(double dSH, double surfaceAngle)
{
	return dSH * (1 + cos(surfaceAngle)) * 0.5;
}

// �����ʑS�V���˗�
inline double CCalcSolarRadiation::calcSolarHorizon(double dDN, double dSH, double sunAngle)
{
	return dDN * sin(sunAngle) + dSH;
}

// �Ζʂɓ��˂��锽�˓��˗�
inline double CCalcSolarRadiation::calcRefrectSlope(double dTH, double surfaceAngle, double refRate)
{
	return dTH * (1 - cos(surfaceAngle)) * 0.5 * refRate;
}

// �����ʔN�ԓ��˗�
bool CCalcSolarRadiation::CalcBuildSolarRadiation(CBuildingDataMap& dataMap, const std::wstring& wstrOutDir)
{
	if (!m_pMng)	return false;

	// �W�v
	calcTotalSolarRadiation(dataMap, wstrOutDir);

	return true;
}


// �W�v
void CCalcSolarRadiation::calcTotalSolarRadiation(CBuildingDataMap& dataMap, const std::wstring& wstrOutDir)
{
	for (auto& bld : dataMap)
	{
		std::string buildId = bld.first;
		CBuildingData& build = bld.second;

		for ( auto& surface : build.mapRoofSurface)
		{
			// �L�����Z�����m
			if (m_pMng->IsCancel())
			{
				return;
			};

			std::string surfaceId = surface.first;
			CRoofSurfaceData& surfaceData = surface.second;

			double sumVal = 0.0;

			for (const auto& mesh : surfaceData.vecRoofMesh)
			{
				for (int month = 0; month < 12; month++)
				{
					sumVal += mesh.solarRadiation[month];
				}
			}

			int meshsize = (int)surfaceData.vecRoofMesh.size();

			// 1m2������̓��˗�
			surfaceData.solarRadiationUnit = sumVal / meshsize;	// ����
			surfaceData.solarRadiationUnit *= 0.001;			// kWh�ɕϊ�

			// �����ʑS�̂̓��˗�(kWh)
			surfaceData.solarRadiation = surfaceData.solarRadiationUnit * surfaceData.GetRoofArea();

			// ���v(kWh)
			build.solarRadiationTotal += surfaceData.solarRadiation;
		}

		// ��������1m2������̔N�ԓ��˗ʂ��Z�o
		double area = build.GetAllRoofArea();
		if (area > 0.0)
		{
			build.solarRadiationUnit = build.solarRadiationTotal / build.GetAllRoofArea();
		}

	}

}

