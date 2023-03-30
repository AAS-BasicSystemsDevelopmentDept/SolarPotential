#pragma once
#include "../../LIB/CommonUtil/StringEx.h"

// ���p
enum class eDirections
{
	NONE = 0,
	NORTH = 1,		// �k����
	EAST = 2,		// ������
	SOUTH = 3,		// �����
	WEST = 4		// ������
};

// ���d�|�e���V�������v
class CElecPotential
{
public:
	double dArea2D;			// �ʐ�
	eDirections eAzimuth;	// ����
	double dAzimuthAngle;	// ���ʌX��
	double dSlopeAngle;		// �X��

	CElecPotential(const double& d1, const eDirections& e, const double& d2, const double& d3)
	{
		dArea2D = d1; eAzimuth = e; dAzimuthAngle = d2; dSlopeAngle = d3;
	}
	CElecPotential()
	{
		dArea2D = 0.0; eAzimuth = eDirections::NONE; dAzimuthAngle = 0.0; dSlopeAngle = 0.0;
	}

	CElecPotential& operator = (const CElecPotential& v) {
		if (&v != this) { dArea2D = v.dArea2D; eAzimuth = v.eAzimuth; dAzimuthAngle = v.dAzimuthAngle; dSlopeAngle = v.dSlopeAngle;	}
		return *this;
	}
};

// �����ʕ␳
class CRoofSurfaceCorrect
{
public:
	double dLowerAngle;			// �X��(�)
	double dTargetAngle;		// �X��(�␳�l)

	CRoofSurfaceCorrect(const double& d1, const double& d2)
	{
		dLowerAngle = d1; dTargetAngle = d2;
	}
	CRoofSurfaceCorrect()
	{
		dLowerAngle = 0.0; dTargetAngle = 0.0;
	}

	CRoofSurfaceCorrect& operator = (const CRoofSurfaceCorrect& v) {
		if (&v != this) { dLowerAngle = v.dLowerAngle; dTargetAngle = v.dTargetAngle; }
		return *this;
	}
};

// ���˃V�~�����[�V�������̉����ʂ̌����E�X���␳
class CReflectionRoofCorrect
{
public:
	bool bRoofSurface;
	bool bSpecify;
	eDirections eAzimuth;
	double dSlopeAngle;

	CReflectionRoofCorrect(const bool& b1, const bool& b2, const eDirections& e, const double& d)
	{
		bRoofSurface = b1; bSpecify = b2; eAzimuth = e; dSlopeAngle = d;
	}
	CReflectionRoofCorrect()
	{
		bRoofSurface = false; bSpecify = false; eAzimuth = eDirections::NONE; dSlopeAngle = 0.0;
	}

	CReflectionRoofCorrect& operator = (const CReflectionRoofCorrect& v) {
		if (&v != this) { bRoofSurface = v.bRoofSurface; bSpecify = v.bSpecify; eAzimuth = v.eAzimuth; dSlopeAngle = v.dSlopeAngle; }
		return *this;
	}
};


class UIParam
{
public:
	CElecPotential			_elecPotential;
	CRoofSurfaceCorrect		_roofSurfaceCorrect;
	double					_dAreaSolarPower;			// ���z���p�l���P�ʖʐϓ�����̔��d�e��
	CReflectionRoofCorrect	_reflectRoofCorrect_Lower;	// 3�x����
	CReflectionRoofCorrect	_reflectRoofCorrect_Upper;	// 3�x�ȏ�

	std::wstring			strOutputDirPath;

	bool					bEnableDEMData;				// DEM�f�[�^���g�p���邩

	bool					bExecSolarPotantial;		// ���d�|�e���V�������v���s�t���O
	bool					bExecReflection;			// ���˃V�~�����[�V�������s�t���O

	UIParam()
	{
		_dAreaSolarPower = 0.0;	strOutputDirPath = L"";
		bEnableDEMData = false;
		bExecSolarPotantial = true; bExecReflection = true;
	}

public:
	void SetElecPotential(const double& d1, const eDirections& e, const double& d2, const double& d3)
	{
		_elecPotential = CElecPotential(d1, e, d2, d3);
	};

	void SetRoofSurfaceCorrect(const double& d1, const double& d2)
	{
		_roofSurfaceCorrect = CRoofSurfaceCorrect(d1, d2);
	};

	void SetAreaSolarPower(const double& d){ _dAreaSolarPower = d; };

	void SetReflectionRoofCorrect_Lower(const bool& b1, const bool& b2, const eDirections& e, const double& d3)
	{
		_reflectRoofCorrect_Lower = CReflectionRoofCorrect(b1, b2, e, d3);
	};

	void SetReflectionRoofCorrect_Upper(const bool& b1, const bool& b2, const eDirections& e, const double& d3)
	{
		_reflectRoofCorrect_Upper = CReflectionRoofCorrect(b1, b2, e, d3);
	};

	void SetOutputDirPath(std::string str) { strOutputDirPath = CStringEx::ToWString(str); };

	void SetEnableDEMData(bool b) { bEnableDEMData = b; };

	void SetExecSolarPotantial(bool b) { bExecSolarPotantial = b; };
	void SetExecReflection(bool b) { bExecReflection = b; };

};