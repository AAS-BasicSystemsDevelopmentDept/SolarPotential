#include "pch.h"
#include "ReflectionSimulateMng.h"
#include <fstream>
#include <thread>
#include <vector>
#include <filesystem>
#include <iomanip>

#include <CommonUtil/CTime.h>
#include "AnalyzeData.h"

using namespace std;

void CReflectionSimulateMng::Exec(const std::string& outDir, UIParam* pUIParam, int year)
{
	m_outDir = outDir;
	m_year = year;
	m_pParam = pUIParam;

	// �Ď��A�t���A�~���̏��Ŕ��˃V�~�����[�V�������ʂ��擾����
	vector<CAnalysisReflectionOneDay> result;
	bool res = ReflectionSim(result);
	if (!res)
	{
		return;
	}

	// ���Q�v�Z
	ReflectionEffect(result);
}

// ���˃V�~�����[�V������͎��{
bool CReflectionSimulateMng::ReflectionSim(
	vector<CAnalysisReflectionOneDay>& result
)
{
	// �Ď��E�~���E�t���̓��t�擾
	CTime summer = CTime::GetSummerSolstice(m_year);
	CTime winter = CTime::GetWinterSolstice(m_year);
	CTime sprint = CTime::GetVernalEquinox(m_year);

	CReflectionSimulator summerSim(summer, m_pParam);
	CReflectionSimulator winterSim(winter, m_pParam);
	CReflectionSimulator springSim(sprint, m_pParam);


	// ���������擾
	vector<BLDGLIST>* allList;
	allList = reinterpret_cast<vector<BLDGLIST>*>(GetAllList());


#	// �X���b�h
	std::thread threadSummer(&CReflectionSimulator::Exec, &summerSim, *allList, *allList);
	std::thread threadSpring(&CReflectionSimulator::Exec, &springSim, *allList, *allList);
	std::thread threadWinter(&CReflectionSimulator::Exec, &winterSim, *allList, *allList);
	threadSummer.join();
	threadSpring.join();
	threadWinter.join();

	// ��͎��s���ʃ`�F�b�N
	if (!summerSim.GetExecResult() ||
		!springSim.GetExecResult() ||
		!winterSim.GetExecResult())
	{
		return false;
	}


	// ��͌��ʏo��
	filesystem::path filepath;

	filepath = m_outDir;
	filepath /= "1_summer.csv";
	summerSim.OutResult(filepath.string());
	CAnalysisReflectionOneDay summerResult = summerSim.GetResult();
	result.push_back(summerResult);

	filepath = m_outDir;
	filepath /= "2_spring.csv";
	springSim.OutResult(filepath.string());
	CAnalysisReflectionOneDay springResult = springSim.GetResult();
	result.push_back(springResult);

	filepath = m_outDir;
	filepath /= "3_winter.csv";
	winterSim.OutResult(filepath.string());
	CAnalysisReflectionOneDay winterResult = winterSim.GetResult();
	result.push_back(winterResult);


	// CZML�t�@�C���o��
	filesystem::path czmlfilepath;

	czmlfilepath = m_outDir;
	czmlfilepath /= "summer.czml";
	summerSim.OutResultCZML(czmlfilepath.string());

	czmlfilepath = m_outDir;
	czmlfilepath /= "spring.czml";
	springSim.OutResultCZML(czmlfilepath.string());

	czmlfilepath = m_outDir;
	czmlfilepath /= "winter.czml";
	winterSim.OutResultCZML(czmlfilepath.string());

	return true;
}

// ���Q��͎��{
bool CReflectionSimulateMng::ReflectionEffect(
	const vector<CAnalysisReflectionOneDay>& result
)
{
	// ���Q��͌��ʂ��i�[����
	// key�͌���ID�Avalue�͉Ď��E�t���E�~�����Ƃ̌��Q����
	map<CResultKeyData, ReflectionEffectTime> effectResult;
	// �����̌���
	// �Ď��A�t���A�~���̏��ɓ����Ă���
	int dateCount = 0;
	for (const auto& dateResult : result)
	{
		// 1���Ԃ��Ƃ̌���
		for (const auto& oneHourResult : dateResult)
		{
			map<CResultKeyData, ReflectionEffectTime> tmpEffect;
			// 1���b�V����
			for (const auto& meshResult : oneHourResult)
			{
				// ���ː�ID
				string targetBuildingId = meshResult.reflectionTarget.buildingId;
				// ���ˌ�ID
				string reflectionBuildingId	= meshResult.reflectionRoof.buildingId;
				int index					= meshResult.reflectionRoof.buildingIndex;

				// ���������͌��Q����̃J�E���g���Ȃ�
				if (targetBuildingId == reflectionBuildingId)
					continue;

				// ���ˌ�ID���J�E���g����
				CResultKeyData keyData(reflectionBuildingId, index);
				if (dateCount == 0)
					tmpEffect[keyData].summer++;
				else if (dateCount == 1)
					tmpEffect[keyData].spring++;
				else if (dateCount == 2)
					tmpEffect[keyData].winter++;
			}
			// 1������1����1�J�E���g�ɂ���
			for (const auto& effect : tmpEffect)
			{
				if (dateCount == 0)
					effectResult[effect.first].summer++;
				else if (dateCount == 1)
					effectResult[effect.first].spring++;
				else if (dateCount == 2)
					effectResult[effect.first].winter++;
			}
		}
		dateCount++;
	}

	// ���ʂ��t�@�C���ɏo�͂���
	filesystem::path filepath = m_outDir;
	filepath /= "���������Q��������.csv";
	OutReflectionEffect(filepath.string(), effectResult);

	return true;
}

bool CReflectionSimulateMng::OutReflectionEffect(
	const std::string csvfile,
	const map<CResultKeyData, ReflectionEffectTime>& effectResult
)
{
	ofstream ofs;
	ofs.open(csvfile);
	if (!ofs.is_open())
		return false;

	// �w�b�_�[��
	ofs << "���b�V��ID,����ID,�Ď�,�t��,�~��" << endl;

	// �f�[�^��
	for (const auto& [key, value] : effectResult)
	{
		// ���b�V��ID�����擾
		string meshID;
		GetMeshID(key.buildingId, meshID);

		ofs << meshID << ","
			<< key.buildingId << ","
			<< value.summer << ","
			<< value.spring << ","
			<< value.winter << endl;
	}

	ofs.close();

	return true;
}

// ���������烁�b�V��ID���擾
bool CReflectionSimulateMng::GetMeshID(const string& buildingID, string& meshID) const
{
	// ���������擾
	vector<BLDGLIST>* allList;
	allList = reinterpret_cast<vector<BLDGLIST>*>(GetAllList());
	if (!allList)
		return false;

	bool ret = false;

	for (const auto& mesh : *allList)
	{
		for (const auto& building : mesh.buildingList)
		{
			if (building.building == buildingID)
			{
				ret = true;
				meshID = mesh.meshID;
				break;
			}
		}
		// �������Ă���̂Ń��[�v�𔲂���
		if (ret)
			break;

		for (const auto& building : mesh.buildingListLOD1)
		{
			if (building.building == buildingID)
			{
				ret = true;
				meshID = mesh.meshID;
				break;
			}
		}
		// �������Ă���̂Ń��[�v�𔲂���
		if (ret)
			break;
	}

	return ret;
}
