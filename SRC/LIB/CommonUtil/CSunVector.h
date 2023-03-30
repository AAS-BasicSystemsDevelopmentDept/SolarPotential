#pragma once

#include <vector>
#include "CGeoUtil.h"
#include "CTime.h"

// ���n���W
struct HorizontalCoordinate
{
	double altitude;	// ���x[rad]

	// ����[rad]
	// �^���0�Ƃ���
	// �������}�C�i�X�����A�������v���X����
	double azimuth;
};


// ���z���x�N�g��
class CSunVector
{
public:
	CSunVector() = delete;
	CSunVector(double lat, double lon, const CTime& date);

	// �w�莞�Ԃ̑��z���̃x�N�g���擾
	bool GetVector(uint8_t hour, CVector3D& vec) const;

	// �w�莞�Ԃ̍��x�ƕ��ʂ��擾
	bool GetPos(uint8_t hour, HorizontalCoordinate& pos) const;

	// ���x�ƕ��ʂ�24���ԃf�[�^�擾
	std::vector<HorizontalCoordinate>& GetPosAry();
	

private:
	// ���z�̈ܓx�o�x
	double m_lat;
	double m_lon;

	// ���t
	CTime m_date;

	// 24���Ԃ̑��z�̍��x�ƕ���
	std::vector<HorizontalCoordinate> m_position;


	// �ܓx�o�x�Ɠ��t���瑾�z�̍��x�ƕ��ʂ��Z�o
	void CalcHorizontalCoordinates();
};
