#include "pch.h"
#include "CSunVector.h"

CSunVector::CSunVector(double lat, double lon, const CTime& date)
	:m_lat(lat), m_lon(lon), m_date(date)
{
	// 24���Ԃ̃f�[�^
	m_position.resize(24);

	// �����̍��x�A���ʂ̎Z�o
	CalcHorizontalCoordinates();
}

// �x�N�g���擾
bool CSunVector::GetVector(uint8_t hour, CVector3D& vec) const
{
	// ���x�E���ʂ��擾
	HorizontalCoordinate pos;
	if (!GetPos(hour, pos))
		return false;

	// ���x�ƕ��ʂ̃x�N�g���v�Z
	// ���ʂ͐^�삪0, x�v���X�����͓�, y�v���X�����͖k
	vec.x = cos(-pos.azimuth - _PI * 0.5);
	vec.y = sin(-pos.azimuth - _PI * 0.5);
	vec.z = sin(pos.altitude);
	// ���x�ƕ��ʂ̈ʒu����̌��̕����̂��߁A�x�N�g���͋t����
	vec *= -1;

	return true;
}

// ���x�E���ʂ��擾
bool CSunVector::GetPos(uint8_t hour, HorizontalCoordinate& pos) const
{
	if (hour >= 24)
		return false;

	pos = m_position[hour];

	return true;
}

// ���x�E���ʂ�24���ԃf�[�^���擾
std::vector<HorizontalCoordinate>& CSunVector::GetPosAry()
{
	return m_position;
}

// ���z��0���`23����1���ԒP�ʂ̈ʒu(���x�E���ʁj���Z�o
void CSunVector::CalcHorizontalCoordinates()
{
	// ���z���x�N�g���Z�o�p
	constexpr double def_SunVecA[] = { 0.006918, 0.399912, 0.070257, 0.006758, 0.000907, 0.002697, 0.001480 };
	constexpr double def_SunVecB[] = { 0.000075, 0.001868, 0.032077, 0.014615, 0.040849 };

	// ���W�A���ɕϊ�
	double dLat = m_lat * _COEF_DEG_TO_RAD;
	double dLon = m_lon * _COEF_DEG_TO_RAD;

	double dTheta = 2 * _PI * ((int64_t)m_date.iYDayCnt - 1) / 365.0;

	// ���z�Ԉ�
	double dSekii = def_SunVecA[0] - def_SunVecA[1] * cos(dTheta) + def_SunVecA[2] * sin(dTheta)
		- def_SunVecA[3] * cos(2 * dTheta) + def_SunVecA[4] * sin(2 * dTheta)
		- def_SunVecA[5] * cos(3 * dTheta) + def_SunVecA[6] * sin(3 * dTheta);
	// �ώ���
	double dEq = def_SunVecB[0] + def_SunVecB[1] * cos(dTheta)
		- def_SunVecB[2] * sin(dTheta)
		- def_SunVecB[3] * cos(2 * dTheta)
		- def_SunVecB[4] * sin(2 * dTheta);

	// �����̌v�Z
	for (short hour = 0; hour < 24; ++hour)
	{
		// ���p[���W�A��]
		int diffHour = hour - 12;
		double dH = diffHour * _PI / 12 + (m_lon - 135) * _COEF_DEG_TO_RAD + dEq;

		// ���x[���W�A��]
		double dAlpha = asin(sin(dLat) * sin(dSekii) + cos(dLat) * cos(dSekii) * cos(dH));

		// ���z����[���W�A��]
		double dPsi = atan2(cos(dLat) * cos(dSekii) * sin(dH), sin(dLat) * sin(dAlpha) - sin(dSekii));

		m_position[hour].altitude = dAlpha;	// ���x
		m_position[hour].azimuth = dPsi;	// ����
	}
}
