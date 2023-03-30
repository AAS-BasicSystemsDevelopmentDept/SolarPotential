#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "CalcSolarPotentialMng.h"


// 1���b�V�����Ƃ̔��d�|�e���V�������v�f�[�^
class CCalcSolarPower
{
public:
	CCalcSolarPower();
	~CCalcSolarPower(void);

public:
	// �N�ԗ\�����d��(EPY)�̎Z�o [kWh/�N]
	bool CalcEPY(CBuildingDataMap& dataMap);
	void SetPperUnit(double d) { m_dPperUnit = d; };

private:
	double m_dPperUnit;
};

