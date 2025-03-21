#pragma once
#include <string>
#include <vector>

// ÎÛ
enum class eTarget
{
	TARGET_NONE = 0,		// s¾(G[)
	TARGET_BUILD = 1,		// ¨
	TARGET_LAND = 2,		// yn
};


// z\¢ÌíÞ
enum class ePriority
{
	PRIORITY_RANK_UNKNOWN = 0,		// DæxNÈµ
	PRIORITY_RANK_1 = 1,			// DæxN1
	PRIORITY_RANK_2 = 2,			// DæxN2
	PRIORITY_RANK_3 = 3,			// DæxN3
	PRIORITY_RANK_4 = 4,			// DæxN4
	PRIORITY_RANK_5 = 5,			// DæxN5
};

struct ResultJudgment
{
	// GAID
	std::string m_strAreaId;
	// bVID
	int m_iMeshId;
	// ¨ID
	std::string m_strBuildingId;
	// Dæx
	ePriority m_ePriority;
	// e»èðÌKEsK
	std::string m_strSuitable1_1_1;			// »èð1_1_1
	std::string m_strSuitable1_1_2;			// »èð1_1_2
	std::string m_strSuitable1_2;			// »èð1_2
	std::string m_strSuitable1_3;			// »èð1_3
	std::string m_strSuitable2_1;			// »èð2_1
	std::string m_strSuitable2_2;			// »èð2_2
	std::string m_strSuitable2_3;			// »èð2_3
	std::string m_strSuitable2_4;			// »èð2_4
	std::string m_strSuitable3_1;			// »èð3_1
	std::string m_strSuitable3_2;			// »èð3_2
	std::string m_strSuitable3_3;			// »èð3_3

	// RXgN^
	ResultJudgment()
		: m_strAreaId("")
		, m_iMeshId(0)
		, m_strBuildingId("")
		, m_ePriority(ePriority::PRIORITY_RANK_5)
		, m_strSuitable1_1_1("-")
		, m_strSuitable1_1_2("-")
		, m_strSuitable1_2("-")
		, m_strSuitable1_3("-")
		, m_strSuitable2_1("-")
		, m_strSuitable2_2("-")
		, m_strSuitable2_3("-")
		, m_strSuitable2_4("-")
		, m_strSuitable3_1("-")
		, m_strSuitable3_2("-")
		, m_strSuitable3_3("-")
	{
	};
};

class CResultJudgment
{
public:
	CResultJudgment(void);
	~CResultJudgment(void);

	// Êf[^ÌÇÁ
	void Add(const ResultJudgment& result)
	{
		m_result.push_back(result);
	}
	// Êf[^
	size_t GetSize()
	{
		return m_result.size();
	};

	// DæxðÝè·é
	void Prioritization();

	// Dæxðæ¾·é
	ePriority GetPriority(std::string strBuildingId);

	// »èÊCSVoÍ
	bool OutputResultCSV(const std::wstring& filepath);

	// ÎÛðÝè·é
	void SetTarget(eTarget target) { m_eTarget = target; };

private:
	std::vector<ResultJudgment> m_result;
	eTarget m_eTarget;

};




