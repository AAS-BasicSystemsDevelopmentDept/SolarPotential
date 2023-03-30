#pragma once
#include "pch.h"
#include "UIParam.h"
#include "../../LIB/CommonUtil/CINIFileIO.h"

/*! �s�s���Ƃ̓Ǝ��敪�Ɋ�Â��g�������̐ݒ�t�@�C���ǂݍ��݃N���X
	�ǂݎ���p
*/
class CExtendedAttributeIniFile : public CINIFileIO
{
public:
	CExtendedAttributeIniFile(std::string strFilePath)
	{
		// ini�t�@�C��OPEN
		this->Open(strFilePath);

		m_vecWood						= GetWood();
		m_vecSteelReinforcedConcrete	= GetSteelReinforcedConcrete();
		m_vecReinforcedConcrete			= GetReinforcedConcrete();
		m_vecSteel						= GetSteel();
		m_vecLightGaugeSteel			= GetLightGaugeSteel();
		m_vecMasonryConstruction		= GetMasonryConstruction();
		m_vecUnknown					= GetUnknown();
		m_vecNonWood					= GetNonWood();
		m_i1stFloor						= Get1stFloor();
		m_i2ndFloor						= Get2ndFloor();
		m_i3rdFloor						= Get3rdFloor();
		m_i4thFloor						= Get4thFloor();
		m_i5thFloor						= Get5thFloor();
		m_i6thFloor						= Get6thFloor();
		m_i7thFloor						= Get7thFloor();
		m_i8thFloor						= Get8thFloor();
		m_i9thFloor						= Get9thFloor();
		m_vec10thFloorAndAbove			= Get10thFloorAndAbove();

	}
	~CExtendedAttributeIniFile(void)
	{
		this->Close();
	}

	std::vector<int> GetSplitInt(std::string str)
	{
		char del = ',';
		std::vector<std::string> vecStr = CStringEx::split(str, del);
		std::vector<int> vecInt;
		for (std::string str : vecStr)
		{
			vecInt.push_back(atoi(str.c_str()));
		}
		return vecInt;
	}

	/*! �s�s���Ƃ̓Ǝ��敪�Ɋ�Â����z�\���̎��
	*/
	std::string GetBuildingStructureCodelists()
	{
		return this->GetString("ExtendedAttribute", "BuildingStructureCodelists", "-");
	}
	/*! �ؑ��E�y����
	*/
	std::vector<int> GetWood()
	{
		std::string str = this->GetString("ExtendedAttribute", "Wood", "-");
		return GetSplitInt(str);
	}
	/*! �S���S�؃R���N���[�g��
	*/
	std::vector<int> GetSteelReinforcedConcrete()
	{
		std::string str = this->GetString("ExtendedAttribute", "SteelReinforcedConcrete", "-");
		return GetSplitInt(str);
	}
	/*! �S�؃R���N���[�g��
	*/
	std::vector<int> GetReinforcedConcrete()
	{
		std::string str = this->GetString("ExtendedAttribute", "ReinforcedConcrete", "-");
		return GetSplitInt(str);
	}
	/*! �S����
	*/
	std::vector<int> GetSteel()
	{
		std::string str = this->GetString("ExtendedAttribute", "Steel", "-");
		return GetSplitInt(str);
	}
	/*! �y�ʓS����
	*/
	std::vector<int> GetLightGaugeSteel()
	{
		std::string str = this->GetString("ExtendedAttribute", "LightGaugeSteel", "-");
		return GetSplitInt(str);
	}
	/*! ��񂪑�/�R���N���[�g�u���b�N��/�Α���
	*/
	std::vector<int> GetMasonryConstruction()
	{
		std::string str = this->GetString("ExtendedAttribute", "MasonryConstruction", "-");
		return GetSplitInt(str);
	}
	/*! �s��
	*/
	std::vector<int> GetUnknown()
	{
		std::string str = this->GetString("ExtendedAttribute", "Unknown", "-");
		return GetSplitInt(str);
	}
	/*! ��ؑ�
	*/
	std::vector<int> GetNonWood()
	{
		std::string str = this->GetString("ExtendedAttribute", "NonWood", "-");
		return GetSplitInt(str);
	}

	eBuildingStructure GetBuildingStructureType2(int iBldStructureType)
	{
		for (int num : m_vecWood)
		{
			if (num == iBldStructureType) return eBuildingStructure::WOOD;
		}
		for (int num : m_vecSteelReinforcedConcrete)
		{
			if (num == iBldStructureType) return eBuildingStructure::STEEL_REINFORCED_CONCRETE;
		}
		for (int num : m_vecReinforcedConcrete)
		{
			if (num == iBldStructureType) return eBuildingStructure::REINFORCED_CONCRETE;
		}
		for (int num : m_vecSteel)
		{
			if (num == iBldStructureType) return eBuildingStructure::STEEL;
		}
		for (int num : m_vecLightGaugeSteel)
		{
			if (num == iBldStructureType) return eBuildingStructure::LIGHT_GAUGE_STEEL;
		}
		for (int num : m_vecMasonryConstruction)
		{
			if (num == iBldStructureType) return eBuildingStructure::MASONRY_CONSTRUCTION;
		}
		for (int num : m_vecUnknown)
		{
			if (num == iBldStructureType) return eBuildingStructure::UNKNOWN;
		}
		for (int num : m_vecNonWood)
		{
			if (num == iBldStructureType) return eBuildingStructure::NON_WOOD;
		}
		return eBuildingStructure::UNKNOWN;
	}

	/*! �s�s���Ƃ̓Ǝ��敪�Ɋ�Â��n��K���͈̔�
	*/
	std::string GetNumberOfFloorsAboveGroundCodelists()
	{
		return this->GetString("ExtendedAttribute", "NumberOfFloorsAboveGroundCodelists", "-");
	}
	/*! 1�K
		*/
	int Get1stFloor()
	{
		return this->GetInt("ExtendedAttribute", "1stFloor", -1);
	}
	/*! 2�K
	*/
	int Get2ndFloor()
	{
		return this->GetInt("ExtendedAttribute", "2ndFloor", -1);
	}
	/*! 3�K
	*/
	int Get3rdFloor()
	{
		return this->GetInt("ExtendedAttribute", "3rdFloor", -1);
	}
	/*! 4�K
	*/
	int Get4thFloor()
	{
		return this->GetInt("ExtendedAttribute", "4thFloor", -1);
	}
	/*! 5�K
	*/
	int Get5thFloor()
	{
		return this->GetInt("ExtendedAttribute", "5thFloor", -1);
	}
	/*! 6�K
	*/
	int Get6thFloor()
	{
		return this->GetInt("ExtendedAttribute", "6thFloor", -1);
	}
	/*! 7�K
	*/
	int Get7thFloor()
	{
		return this->GetInt("ExtendedAttribute", "7thFloor", -1);
	}
	/*! 8�K
	*/
	int Get8thFloor()
	{
		return this->GetInt("ExtendedAttribute", "8thFloor", -1);
	}
	/*! 9�K
	*/
	int Get9thFloor()
	{
		return this->GetInt("ExtendedAttribute", "9thFloor", -1);
	}
	/*! 10�K�ȏ�
	*/
	std::vector<int> Get10thFloorAndAbove()
	{
		std::string str = this->GetString("ExtendedAttribute", "10thFloorAndAbove", "-");
		return GetSplitInt(str);
	}
	int GetFloorType(int iFloorType)
	{
		if (m_i1stFloor == iFloorType) return 1;
		if (m_i2ndFloor == iFloorType) return 2;
		if (m_i3rdFloor == iFloorType) return 3;
		if (m_i4thFloor == iFloorType) return 4;
		if (m_i5thFloor == iFloorType) return 5;
		if (m_i6thFloor == iFloorType) return 6;
		if (m_i7thFloor == iFloorType) return 7;
		if (m_i8thFloor == iFloorType) return 8;
		if (m_i9thFloor == iFloorType) return 9;
		for (int num : m_vec10thFloorAndAbove)
		{
			if (num == iFloorType) return 10;
		}

		// �s���Ȃ̂ł��̂܂ܕԂ�
		return iFloorType;
	}
private:
	std::vector<int> m_vecWood;
	std::vector<int> m_vecSteelReinforcedConcrete;
	std::vector<int> m_vecReinforcedConcrete;
	std::vector<int> m_vecSteel;
	std::vector<int> m_vecLightGaugeSteel;
	std::vector<int> m_vecMasonryConstruction;
	std::vector<int> m_vecUnknown;
	std::vector<int> m_vecNonWood;
	int m_i1stFloor;
	int m_i2ndFloor;
	int m_i3rdFloor;
	int m_i4thFloor;
	int m_i5thFloor;
	int m_i6thFloor;
	int m_i7thFloor;
	int m_i8thFloor;
	int m_i9thFloor;
	std::vector<int> m_vec10thFloorAndAbove;
};


