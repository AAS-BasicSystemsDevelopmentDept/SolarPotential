#pragma once
#include "pch.h"
#include "../../LIB/CommonUtil/CINIFileIO.h"

/*! �D��x�ݒ�t�@�C���ǂݍ��݃N���X
	�ǂݎ���p
*/
class CJudgeSuitablePlacePriorityIniFile : public CINIFileIO
{
public:
	CJudgeSuitablePlacePriorityIniFile(std::string strFilePath)
	{
		// ini�t�@�C��OPEN
		this->Open(strFilePath);
	}
	~CJudgeSuitablePlacePriorityIniFile(void)
	{
		this->Close();
	}

	// �D��x�̔���
	/*! �D��x�����N5
	*/
	int GetCriterion_5()
	{
		return this->GetInt("Criterion", "JudgementCriterion_5", 0);
	}
	/*! �D��x�����N4
	*/
	int GetCriterion_4()
	{
		return this->GetInt("Criterion", "JudgementCriterion_4", -5);
	}
	/*! �D��x�����N3
	*/
	int GetCriterion_3()
	{
		return this->GetInt("Criterion", "JudgementCriterion_3", -15);
	}
	/*! �D��x�����N2
	*/
	int GetCriterion_2()
	{
		return this->GetInt("Criterion", "JudgementCriterion_2", -25);
	}
	// �����ɕt����������̃|�C���g�̐ݒ�
	/*! ���˗ʂ����Ȃ��{��
	*/
	int GetJudgementCondition_1_1()
	{
		return this->GetInt("Building", "JudgementCondition_1_1", -1);
	}
	/*! �����\���ɂ�鏜�O
	*/
	int GetJudgementCondition_1_2()
	{
		return this->GetInt("Building", "JudgementCondition_1_2", -1);
	}
	/*! ����̊K�w�̎{��
	*/
	int GetJudgementCondition_1_3()
	{
		return this->GetInt("Building", "JudgementCondition_1_3", -1);
	}
	// �ЊQ���̃��X�N�ɂ������̃|�C���g�̐ݒ�
	/*! �������z�肳���ő�Ôg����������錚��
	*/
	int GetJudgementCondition_2_1()
	{
		return this->GetInt("Hazard", "JudgementCondition_2_1", -1);
	}
	/*! �����������z�肳���͐�Z���z��Z���[������錚��
	*/
	int GetJudgementCondition_2_2()
	{
		return this->GetInt("Hazard", "JudgementCondition_2_2", -1);
	}
	/*! �y���ЊQ�x�������ɑ��݂��錚��
	*/
	int GetJudgementCondition_2_3()
	{
		return this->GetInt("Hazard", "JudgementCondition_2_3", -1);
	}
	/*! �ϐႪ�����n��̌���
	*/
	int GetJudgementCondition_2_4()
	{
		return this->GetInt("Hazard", "JudgementCondition_2_4", -1);
	}
	// ���ɂ������̃|�C���g�̐ݒ�
	/*! �ݒu�ɐ�����������1
	*/
	int GetJudgementCondition_3_1()
	{
		return this->GetInt("Restrict", "JudgementCondition_3_1", -1);
	}
	/*! �ݒu�ɐ�����������2
	*/
	int GetJudgementCondition_3_2()
	{
		return this->GetInt("Restrict", "JudgementCondition_3_2", -1);
	}
	/*! �ݒu�ɐ�����������3
	*/
	int GetJudgementCondition_3_3()
	{
		return this->GetInt("Restrict", "JudgementCondition_3_3", -1);
	}
};


