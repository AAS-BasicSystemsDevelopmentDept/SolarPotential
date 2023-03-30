#pragma once
#include <vector>
#include "CGeoUtil.h"
#include "StringEx.h"

// TIFF(�摜)�o�͗p�f�[�^�N���X

class CTiffData
{
public:
	CTiffData();
	CTiffData(std::vector<CPointBase>* pData);
	~CTiffData();

private:
	// �o�̓f�[�^
	std::vector<CPointBase>* m_pData;
	// �_�Q�f�[�^�̊e���W�ŏ��l
	CPointBase m_minPoint;
	// �_�Q�f�[�^�̊e���W�ő�l
	CPointBase m_maxPoint;

	// �t�@�C����
	std::wstring m_fileName;

public:
	// ������
	void Initialize();
	// �f�[�^���
	void Analysis();

	bool CalcMaxMinPoint();

	const CPointBase GetPointMin();
	const CPointBase GetPointMax();

	void SetData(std::vector<CPointBase>* pData) { m_pData = pData; };
	const std::vector<CPointBase>* GetData() { return m_pData; };

	void SetFileName(const std::wstring& str) { m_fileName = str; };
	const std::wstring GetFileName() { return m_fileName; };

};

