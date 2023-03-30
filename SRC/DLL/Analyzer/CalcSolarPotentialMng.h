#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CSunVector.h"
#include "../../LIB/CommonUtil/TiffDataManager.h"
#include "../../LIB/CommonUtil/CLightRay.h"
#include "AnalysisRadiationData.h"
#include "ImportMetpvData.h"
#include "ImportPossibleSunshineData.h"
#include "ImportAverageSunshineData.h"
#include "UIParam.h"

#include "AnalyzeData.h"


class CCalcSolarPotentialMng
{
public:
	// ���ʔ͈͂������l(����16���ʂ�22�x����)
	const double AZ_RANGE_JUDGE_DEGREE = 22.0;

	enum class eOutputImageTarget
	{
		SOLAR_RAD = 0,
		SOLAR_POWER,
	};

	CCalcSolarPotentialMng(
		CImportPossibleSunshineData* pSunshineData,
		CImportAverageSunshineData* pPointData,
		CImportMetpvData* pMetpvData,
		UIParam* m_pUIParam,
		const int& iYear
	);
	~CCalcSolarPotentialMng(void);


public:
	bool AnalyzeSolarPotential();	// ���d�|�e���V�������v���C������

	CAnalysisRadiationCommon*	GetRadiationData()	{ return m_pRadiationData; };
	UIParam*					GetUIParam()		{ return m_pUIParam; };
	CImportPossibleSunshineData*	GetSunshineData()	{ return m_pSunshineData; };
	CImportAverageSunshineData*		GetPointData()		{ return m_pPointData; };
	CImportMetpvData*				GetMetpvData()		{ return m_pMetpvData; };
	
	// ���ӂ̒n���Ɏז����ꂸ�����ʂɌ����������邩�ǂ����̔���
	bool IntersectRoofSurfaceCenter(
		const CVector3D& inputVec,						// ���ˌ�
		const std::vector<CVector3D>& roofMesh,			// �Ώۂ̉���BB
		const std::string& strId,						// �Ώۂ̉���ID
		const vector<BLDGLIST>& neighborBuildings,		// ���ӂ̌������X�g
		const vector<DEMLIST>& neighborDems				// ���ӂ̒n�`DEM���X�g
	);

	// �Ώۃ��b�V���̗אڂ��郁�b�V�����擾
	void GetNeighborBuildings(
		const std::string& targetMeshId,				// �Ώۂ�3�����b�V��ID
		const CVector3D& bldCenter,						// �Ώۂ̌������S
		std::vector<BLDGLIST>& neighborBuildings		// �ߗ׃��b�V��
	);

	// �Ώۂ̌����ɗאڂ���DEM���擾
	void GetNeighborDems(
		const std::string& targetMeshId,				// �Ώۂ�3�����b�V��ID
		const CVector3D& bldCenter,						// �Ώۂ̌������S
		std::vector<DEMLIST>& neighborDems				// �ߗ�DEM
	);

	const int GetYear() { return m_iYear; };

	// �L�����Z������
	bool IsCancel();

	// DEM�f�[�^�̎g�p�L��
	bool IsEnableDEMData();

private:
	double calcLength(double dx, double dy, double dz)
	{
		return dx * dx + dy * dy + dz * dz;
	}

	void initialize();			// �v�Z�p�f�[�^���̏�����
	// ���˗ʐ��v
	bool calcSolarRadiation(const std::string& Lv3meshId, double bbMinX, double bbMinY, double bbMaxX, double bbMaxY, CBuildingDataMap& bldDataMap);
	// ���d�ʐ��v
	bool calcSolarPower(const std::string& Lv3meshId, double bbMinX, double bbMinY, double bbMaxX, double bbMaxY, CBuildingDataMap& bldDataMap);

	// �X�Ίp�A���ʊp���Z�o����
	void calcRoofAspect(const BLDGLIST& bldList, CBuildingDataMap& bldDataMap);
	bool calcRansacPlane(const std::vector<CPointBase>& vecAry, CVector3D& vNormal);

	// �����Ƃ̓��Ɨ����v�Z
	void calcMonthlyRate();

	// �o�͏���
	// �o�͗p��3D�|�C���g�f�[�^���쐬
	bool createPointData(
		std::vector<CPointBase>& vecPoint3d,
		const std::string& Lv3meshId,
		double bbMinX,
		double bbMinY,
		double bbMaxX,
		double bbMaxY,
		double outMeshsize,
		const CBuildingDataMap& bldDataMap,
		const eOutputImageTarget& eTarget
	);
	// ���˗ʂ̒l�ɉ����Ē��F�����摜���o��
	bool outputImage(
		const std::wstring strFilePath,
		const std::string& Lv3meshId,
		double bbMinX, double bbMinY, double bbMaxX, double bbMaxY,
		const CBuildingDataMap& bldDataMap,
		const eOutputImageTarget& eTarget
	);
	bool outputLegendImage();	// �}��摜���o��
	bool outputResultCSV();														// �N�ԓ��˗ʁE���d�ʂ��o��
	bool outputAzimuthDataCSV();												// �K�n����p ���ʊp���ԃt�@�C���o��
	bool outputMonthlyRadCSV(const std::string& Lv3meshId, const CBuildingDataMap& dataMap, const std::wstring& wstrOutDir);	// ���ʓ��˗�CSV�o��
	bool outputRoofRadCSV(const std::string& Lv3meshId, const CBuildingDataMap& dataMap, const std::wstring& wstrOutDir);		// �����ʕʔN�ԓ��˗�CSV�o��

	bool setTotalSolarRadiationToSHP();

	void finalize();

	double calcArea(const std::vector<CPointBase>& vecPos);
	
	// �����������Q�ɂ������Ă��邩�ǂ���
	bool intersectBuildings(
		const CLightRay& lightRay,					// ����
		const std::string& strId,					// �Ώۂ̉���ID
		const std::vector<BLDGLIST>& buildingsList	// �������������Ă��邩�`�F�b�N���錚���Q
	);
	// �����������ɂ������Ă��邩�ǂ���
	bool intersectBuilding(
		const CLightRay& lightRay,					// ����
		const vector<WALLSURFACES>& wallSurfaceList	// �������������Ă��邩�`�F�b�N���錚���̕�
	);
	bool intersectBuilding(
		const CLightRay& lightRay,					// ����
		const std::string& strId,					// �Ώۂ̉���ID
		const BUILDINGS& buildings					// �������������Ă��邩�`�F�b�N���錚���Q
	);
	// �����������͈͓̔���
	bool checkDistance(const CLightRay& lightRay, const vector<WALLSURFACES>& wallSurfaceList);

	// �������n�`�ɂ������Ă��邩�ǂ���
	bool intersectLandDEM(
		const CLightRay& lightRay,					// ����
		const vector<DEMLIST>& demList				// �������������Ă��邩�`�F�b�N����n�`��DEM
	);

	// 3�����b�V�����א�(����8����)���Ă��邩
	bool isNeighborMesh(const std::string& meshId1, const std::string& meshId2);

private:
	// ���̓f�[�^
	CImportPossibleSunshineData* m_pSunshineData;
	CImportAverageSunshineData* m_pPointData;
	CImportMetpvData* m_pMetpvData;
	UIParam* m_pUIParam;

	std::vector<BLDGLIST>*		m_pvecAllBuildList;		// CityGML����擾�����S�����f�[�^���
	std::vector<DEMLIST>*		m_pvecAllDemList;		// CityGML����擾����DEM�f�[�^���

	CAnalysisRadiationCommon*	m_pRadiationData;		// ���ʂ̌v�Z�p�����[�^
	CResultDataMap*				m_pmapResultData;		// �v�Z���ʃf�[�^�}�b�v

	int m_iYear;

	std::wstring m_strCancelFilePath;

};
