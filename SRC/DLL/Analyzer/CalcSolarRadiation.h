#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "CalcSolarPotentialMng.h"

// �������Ƃ̓��˗ʂ��v�Z����
class CCalcSolarRadiation
{
public:
	CCalcSolarRadiation(CCalcSolarPotentialMng* mng);
	~CCalcSolarRadiation(void);

	// ���˗ʐ��v
	bool Exec(
		CBuildingDataMap& dataMap,				// �v�Z���ʊi�[�p�f�[�^�}�b�v
		const bool& bModDegree,					// true:�␳�p�x�f�[�^���g�p
		const std::wstring& wstrPath,			// ����CSV�o�̓t�@�C����
		const std::string Lv3meshId				// 3�����b�V��ID
	);

	// ���Ɨ��ɂ��␳
	bool ModifySunRate(
		CBuildingDataMap& dataMap,				// �v�Z���ʊi�[�p�f�[�^�}�b�v
		const std::wstring& wstrPath			// ����CSV�o�̓t�@�C����
	);

	// ��������1m^2������̔N�ԓ��˗ʂ��Z�o
	bool CalcBuildSolarRadiation(CBuildingDataMap& dataMap, const std::wstring& wstrOutDir);

private:
	// �����ʔN�ԓ��˗�
	// �����ʖ���1m^2������̓��˗ʂ��W�v
	void calcTotalSolarRadiation(CBuildingDataMap& dataMap, const std::wstring& wstrOutDir);

	double calcSlopeDif(
		const double& surfaceAngle,	// �X�Ίp
		const double& angIn,		// ���ˊp
		const double& sunAngle,		// ���z���x
		const double& refRate,		// ���˗�
		const bool& bDirect,		// ���B�����̗L��
		const double& pdif			// ���ߗ�
	);

	// ���˗ʂ̊e�v�Z
	// ���ˊp�Z�o
	static double calcAngleIn(double sunAngle, double surfaceAngle, double surfaceAz, double alpha);
	// �@���ʒ��B���˗�(W/m2)
	static double calcDirectNormal(double sunAngle, double pdif);
	// �����ʓV����˗�(W/m2)
	static double calcSkyHorizon(double sunAngle, double pdif);
	// �Ζʒ��B���˗�
	static double calcDirectSlope(double dDN, double angIn);
	// �ΖʓV����˗�
	static double calcSkySlope(double dSH, double surfaceAngle);
	// �����ʑS�V���˗�
	static double calcSolarHorizon(double dDN, double dSH, double sunAngle);
	// �Ζʂɓ��˂��锽�˓��˗�
	static double calcRefrectSlope(double dTH, double surfaceAngle, double refRate);

private:
	CCalcSolarPotentialMng*		m_pMng;

};
