#include "pch.h"
#include "CalcSolarPower.h"
#include "AnalysisRadiationData.h"



// ���d�ʎZ�o�p�����[�^
// ��{�݌v�W��
double def_KPY = 0.88;

// �W�����������ɂ�������ˋ��x
double def_GS = 1;


CCalcSolarPower::CCalcSolarPower
()
	: m_dPperUnit(0.0)
{

}

CCalcSolarPower::~CCalcSolarPower(void)
{

}


// �N�ԗ\�����d��(EPY)�̎Z�o [kWh/�N]
bool CCalcSolarPower::CalcEPY(CBuildingDataMap& dataMap)
{
	double EPY = 0.0;

	for (auto& val1 : dataMap)
	{
		CBuildingData& build = val1.second;

		int meshCount = 0;
		for (auto& val2 : build.mapRoofSurface)
		{
			CRoofSurfaceData& surface = val2.second;
			meshCount += (int)surface.vecRoofMesh.size();
		}

		double panelArea = meshCount * 1.0;		// 1m���b�V���� = �ʐ�(m2)
		build.panelArea = panelArea;
		
		// �ݒu�\�V�X�e���e��(P)
		// �p�l���ʐρ��P�ʖʐϓ�����e��
		double P = panelArea * m_dPperUnit;

		// �N�ԗ\�����˗�(HAY)
		double HAY = build.solarRadiationUnit;

		// �ݒu�\�V�X�e���e��(P) �� �N�ԗ\�����˗�(HAY) �� ��{�݌v�W��(KPY) �� �P �^ �W�����������ɂ�������ˋ��x(GS)
		EPY = P * HAY * def_KPY * 1 / def_GS;

		// �������ƂɎZ�o�������d�ʂ�K�p
		build.solarPower = EPY;

		// 1m2������̔��d��
		build.solarPowerUnit = EPY / panelArea;

	}

	return true;

}

