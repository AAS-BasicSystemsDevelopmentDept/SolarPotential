#include "pch.h"
#include "CLightRay.h"
#include <algorithm>

using namespace std;

CLightRay::CLightRay(const CVector3D& pos, const CVector3D& vec)
	: m_pos(pos), m_vec(vec)
{
}

// ���ˌ����擾
// ���ˌ��̒����͓��ˌ��Ɠ����ɂ���
CLightRay CLightRay::Reflect(const CVector3D& reflectPos, const CVector3D& normal) const
{
	if ((m_vec.Length() < DBL_EPSILON) ||
		(normal.Length() < DBL_EPSILON))
		return CLightRay(reflectPos, CVector3D());

	CVector3D inVec = CGeoUtil::Normalize(m_vec);
	CVector3D nVec = CGeoUtil::Normalize(normal);

	// ��������̂Ƃ��͖@�����t�ɂ��ė��ʂ�\�ɂ���
	double dot = CGeoUtil::InnerProduct(inVec, nVec);
	if (dot > 0.0f)
		nVec *= -1;

	// ���˃x�N�g�� = inVec * 2 * a * normal (a : ���˃x�N�g���t�����Ɩ@���̓���)
	CVector3D reflectVec = inVec + nVec * 2 * CGeoUtil::InnerProduct(inVec * (-1), nVec);
	reflectVec.Normalize();
	reflectVec *= m_vec.Length();

	return CLightRay(reflectPos, reflectVec);
}

// ���������ʂƌ������Ă��邩
bool CLightRay::Intersect(
	const std::vector<CVector3D>& polygon,
	CVector3D* point,
	double* dist
) const
{
	if (polygon.size() < 3)
		return false;

	// ���ʂ̖@��
	CVector3D n;
	{
		CVector3D vec1;
		CVector3D vec2;
		// [0]����̊e�_�̃x�N�g��
		vector<CVector3D> vecPolyList;
		for (int i = 1; i < polygon.size(); ++i)
		{
			CVector3D vec(polygon[i], polygon[0]);
			vecPolyList.push_back(vec);
		}
		sort(
			vecPolyList.begin(),
			vecPolyList.end(),
			[](const CVector3D& x, const CVector3D& y) { return x.Length() > y.Length(); }
		);
		vec1 = vecPolyList[0];
		vec1.Normalize();
		for (const auto& pos : vecPolyList)
		{
			CVector3D tempVec = pos;
			tempVec.Normalize();
			// �����������t�����̂Ƃ��͖@�����܂�Ȃ�
			if (abs(CGeoUtil::InnerProduct(vec1, tempVec)) > 0.999)
				continue;
			vec2 = tempVec;
			break;
		}
		CGeoUtil::OuterProduct(vec1, vec2, n);
	}

	// ���ʂɑ΂�������̕������`�F�b�N
	CVector3D inVec = CGeoUtil::Normalize(m_vec);
	double dot = CGeoUtil::InnerProduct(n, inVec);
	// ���ʂƕ��s�̂Ƃ��͌������Ȃ�
	if (abs(dot) < DBL_EPSILON)
		return false;
	// ��������̂Ƃ��͖@�����t�ɂ��ė��ʂ�\�ɂ���
	if (dot > 0.0)
	{
		n   *= -1;
		dot *= -1;
	}


	// ���ʂ̎�
	// ax + by + cz = d ����
	// p �E n = d (p:���ʏ�̑S�Ă̓_, n:�@��[a,b,c]) �ƕ\����
	CVector3D p(polygon[0].x, polygon[0].y, polygon[0].z);
	double d = CGeoUtil::InnerProduct(p, n);

	// ���ʂƌ����̌�_�̎�
	// p(t) = p0 + t * v (p0:�����̌��_�At:���_�����_�܂ł̋����Av:�����̒P�ʃx�N�g��)
	// ��_�̎��ƕ��ʂ̎�����t�̎��ɂ���
	CVector3D p0(m_pos.x, m_pos.y, m_pos.z);
	double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
	
	if ((t > m_vec.Length()) ||	// ���ꂷ���Ă���Ƃ��͌������Ă��Ȃ��Ƃ���
		(t <= 0.0))				// ���ʏ�܂��͋t�����̎��͌������Ă��Ȃ��Ƃ���
		return false;
	double tempDist = t;

	// ��_
	CVector3D tempPoint = p0 + t * inVec;

	// ��_���|���S�����ʓ��ɂ��邩
	bool result = IsPointInPolygon(tempPoint, polygon);
	if (result)
	{
		*point = tempPoint;
		*dist = tempDist;
	}

	return result;
}

// ���O����
bool CLightRay::IsPointInPolygon(const CVector3D& point, const vector<CVector3D>& polygon) const
{
	// �܂��͑�܂��Ƀo�E���f�B���O�{�b�N�X���ɂ��邩�łӂ邢�ɂ�����
	if (!IsPointInBB(point, polygon))
		return false;

	// ���ʂ̖@��
	CVector3D n;
	CGeoUtil::OuterProduct(CVector3D(polygon[1], polygon[0]), CVector3D(polygon[2], polygon[1]), n);

	// ���ʂɓ��e����
	// ���e��������͖@���̕�������������
	vector<CVector2D> plane2d;
	CVector2D targetPoint;
	// x�����ɐL�тĂ��鎞yz���ʂɓ��e
	if (abs(n.x) > abs(n.y) && abs(n.x) > abs(n.z))
	{
		for (const auto& vertex : polygon)
		{
			plane2d.push_back(CVector2D(vertex.y, vertex.z));
		}
		targetPoint.x = point.y;
		targetPoint.y = point.z;
	}
	// y�����ɐL�тĂ��鎞xz���ʂɓ��e
	else if (abs(n.y) > abs(n.x) && abs(n.y) > abs(n.z))
	{
		for (const auto& vertex : polygon)
		{
			plane2d.push_back(CVector2D(vertex.x, vertex.z));
		}
		targetPoint.x = point.x;
		targetPoint.y = point.z;
	}
	// z�����ɐL�тĂ��鎞xy���ʂɓ��e
	else
	{
		for (const auto& vertex : polygon)
		{
			plane2d.push_back(CVector2D(vertex.x, vertex.y));
		}
		targetPoint.x = point.x;
		targetPoint.y = point.y;
	}

	// ���ʏ�̓��O����
	return CGeoUtil::IsPointInPolygon(targetPoint, static_cast<unsigned int>(polygon.size()), plane2d.data());
}


// �|���S���̃o�E���f�B���O�{�b�N�X���ɓ_�����݂��邩����
// ��polygon�͕��ʃf�[�^�Ŏg�p����z��̂��߁ABB�͏����]�T���������Ĕ��肷��
bool CLightRay::IsPointInBB(const CVector3D& pointTarget, const std::vector<CVector3D>& polygon) const
{
	constexpr double margine = 0.1;	// BB�̗]�T

	CVector3D bbMin(DBL_MAX, DBL_MAX, DBL_MAX);
	CVector3D bbMax(-DBL_MAX, -DBL_MAX, -DBL_MAX);

	for (const auto& vertex : polygon)
	{
		if (vertex.x < bbMin.x) bbMin.x = vertex.x;
		if (vertex.y < bbMin.y) bbMin.y = vertex.y;
		if (vertex.z < bbMin.z) bbMin.z = vertex.z;
		if (vertex.x > bbMax.x) bbMax.x = vertex.x;
		if (vertex.y > bbMax.y) bbMax.y = vertex.y;
		if (vertex.z > bbMax.z) bbMax.z = vertex.z;
	}

	bbMin -= CVector3D(margine, margine, margine);
	bbMax += CVector3D(margine, margine, margine);

	// �͈͓�
	if ((pointTarget.x >= bbMin.x && pointTarget.x <= bbMax.x) &&
		(pointTarget.y >= bbMin.y && pointTarget.y <= bbMax.y) &&
		(pointTarget.z >= bbMin.z && pointTarget.z <= bbMax.z))
		return true;

	return false;
}
