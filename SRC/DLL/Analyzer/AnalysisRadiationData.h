#pragma once
#include <map>
#include "..\..\LIB\CommonUtil\CSunVector.h"

// ���d�|�e���V�������v
// ���˗ʋ���
class CAnalysisRadiationCommon
{
public:
	// ���V���̓��Ɨ��i�s�s���Ɓj
	double sunnyRate[12]; // �����Ƃ̓��Ǝ���/�Ǝ���
	// �ܓV���̓��Ɨ��i�s�s���Ɓj
	double cloudRate[12]; // ������ 1 - ���V���̓��Ɨ�


};

// 1���b�V�����Ƃ̔��d�|�e���V�������v�f�[�^
class CMeshData
{
public:
	std::string meshId;		// ID

	std::vector<CVector3D> meshPos;		// ���b�V�����
	CVector3D center;		// ���b�V�����S
	CVector3D centerMod;	// ���b�V�����S(�p�x�␳��)

	// ���ˌv�Z����
	double solarRadiationSunny[12];			// ���˗�(WH/m2) �����Ɓ@���V
	double solarRadiationCloud[12];			// ���˗�(WH/m2) �����Ɓ@�ܓV
	double solarRadiation[12];			// ���Ɨ��ɂ��␳�������˗�(WH/m2)�@������

	CMeshData()
	{
		meshId = "";
		for (int n = 0; n < 12; n++) solarRadiationSunny[n] = 0;
		for (int n = 0; n < 12; n++) solarRadiationCloud[n] = 0;
		for (int n = 0; n < 12; n++) solarRadiation[n] = 0;

	};

};

// �����ʂ��Ƃ̔��d�|�e���V�������v�f�[�^
class CRoofSurfaceData
{
public:
	std::vector<CMeshData> vecRoofMesh;		// ���b�V�����Ƃ̃f�[�^

	std::vector<CVector3D> bbPos;			// �����ʂ�BB
	CVector3D center;						// BB���S

	double slopeDegreeAve;					// �X�Ίp(���ϒl)(�x)
	double azDegreeAve;						// ���ʊp(���ϒl)(�x)
	double slopeModDegree;					// �␳�����X�Ίp(�x)
	double azModDegree;						// �␳�������ʊp(�x)

	double solarRadiation;					// �N�ԗ\�����˗�(kWh)
	double solarRadiationUnit;				// �N�ԗ\�����˗�(kWh/m2)

	double area;							// �����ʂ̖ʐ�(m2)


	// �ʐς��Z�o����
	double GetRoofArea() { return area; };

	CRoofSurfaceData()
	{
		slopeDegreeAve = 0.0; azDegreeAve = 0.0;
		slopeModDegree = 0.0; azModDegree = 0.0;
		solarRadiation = 0.0; solarRadiationUnit = 0.0;
		area = 0.0;
	};

};

typedef std::map<std::string, CRoofSurfaceData> CRoofSurfaceDataMap;


// �������Ƃ̔��d�|�e���V�������v�f�[�^
class CBuildingData
{
public:
	CRoofSurfaceDataMap	mapRoofSurface;	// �����ʂ��Ƃ̃f�[�^�}�b�v

	std::vector<CVector3D> bbPos;		// �����ʑS�̂�BB
	CVector3D center;					// BB���S

	// ��͌���(CityGML�ɑ����t�^)
	double solarRadiationTotal;			// �N�ԗ\�����˗� ���v
	double solarRadiationUnit;			// �N�ԗ\�����˗� 1m2������̓��˗�
	double solarPower;					// �N�ԗ\�����d��
	double solarPowerUnit;				// �N�ԗ\�����d�� 1m2������̔��d��

	double panelArea;					// �p�l���ʐ�(1m���b�V����)

	// �����̑������ʐ�
	double GetAllRoofArea()
	{
		double area = 0.0;
		for (auto val : mapRoofSurface)
		{
			area += val.second.GetRoofArea();
		}
		return area;
	};

	CBuildingData()
	{
		solarRadiationTotal = 0; solarRadiationUnit = 0; solarPower = 0;
		solarPowerUnit = 0; panelArea = 0;
	};
};

typedef std::map<std::string, CBuildingData> CBuildingDataMap;

typedef std::map<std::string, CBuildingDataMap> CResultDataMap;


