#pragma once

#include <vector>
#include "CGeoUtil.h"

// �����Ɋւ���N���X
// ���ˌ��̌v�Z�╨�̂̓����蔻��ȂǍs��
class CLightRay
{
public:
	CLightRay() = delete;
	CLightRay(const CVector3D& pos, const CVector3D& vec);

	// ���ˌ�̌������Z�o
	CLightRay Reflect(const CVector3D& reflectPos, const CVector3D& normal) const;

	// �����ƕ��ʂ̌����_��T��
	bool Intersect(
		const std::vector<CVector3D>& polygon,	// ����
		CVector3D* point,						// [out]��_
		double* dist							// [out]��_�܂ł̋���
	) const; 

	// �����̃x�N�g���擾
	inline const CVector3D& GetVector() const { return m_vec; }
	// �����擾
	inline const CVector3D& GetPos() const { return m_pos; }

private:
	CVector3D m_vec;	// �����̒����ƌ���
	CVector3D m_pos;	// �����̌��_

private:
	// ���ʏ�̓_�̓��O����
	bool IsPointInPolygon(const CVector3D& pointTarget, const std::vector<CVector3D>& polygon) const;
	// �o�E���f�B���O�{�b�N�X���̔���
	bool IsPointInBB(const CVector3D& pointTarget, const std::vector<CVector3D>& polygon) const;
};
