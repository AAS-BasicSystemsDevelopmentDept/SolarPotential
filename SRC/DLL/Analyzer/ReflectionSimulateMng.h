#pragma once
#include <vector>
#include <map>
#include <string>
#include "ReflectionSimulator.h"
#include "UIParam.h"

class CResultKeyData;

class CReflectionSimulateMng
{
public:
	CReflectionSimulateMng()
		: m_outDir("")
		, m_year(2021)
		, m_pParam(nullptr)
	{}

	void Exec(const std::string& outDir, UIParam* pUIParam, int year);

private:
	// ���Q����
	struct ReflectionEffectTime
	{
		int summer{ 0 };	// �Ď�
		int spring{ 0 };	// �t��
		int winter{ 0 };	// �~��
	};

	// ��͌��ʊi�[��
	std::string m_outDir;
	// ��͔N
	int m_year;
	// �ݒ�p�����[�^
	UIParam* m_pParam;

	bool ReflectionSim(std::vector<CAnalysisReflectionOneDay>& result);

	bool ReflectionEffect(const std::vector<CAnalysisReflectionOneDay>& result);

	bool OutReflectionEffect(const std::string csvfile,
		const std::map<CResultKeyData, ReflectionEffectTime>& effectResult);

	// ���������烁�b�V��ID���擾
	bool GetMeshID(const std::string& buildingID, std::string& meshID) const;

};

class CResultKeyData
{
public:
	string buildingId;	// ����ID
	int index;			// ����ID���я�

public:
	CResultKeyData() : buildingId(""), index(0) {}
	CResultKeyData(const string& buildingId, int index)
		: buildingId(buildingId), index(index) {}

	bool operator <(const CResultKeyData& a) const
	{
		// index�Ŕ�r���ē����ł����buildingId�Ŕ�r����
		return tie(index, buildingId) < tie(a.index, a.buildingId);
	}
};
