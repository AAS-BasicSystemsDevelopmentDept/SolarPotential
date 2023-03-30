#pragma once

#define WIN32_LEAN_AND_MEAN             // Windows �w�b�_�[����قƂ�ǎg�p����Ă��Ȃ����������O����

// �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă�������
#include "../../../../LIB/CommonUtil/CGeoUtil.h"

#include <Windows.h>
#include <string.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <tchar.h>


#include <algorithm>
#include <iterator> 

#import "msxml6.dll" named_guids raw_interfaces_only
#include <atlbase.h>	// CComVariant, CComBSTR
#include <conio.h>
#include <iomanip>
#include <fstream>
#include <sstream>


// CityGML�̃o�[�W����
enum class eCityGMLVersion
{
	VERSION_1,          // 1: 2021�N�x��
	VERSION_2,          // 2: 2022�N�x��
	End
};
// operator ++
eCityGMLVersion& operator ++ (eCityGMLVersion& ver)
{
	if (ver == eCityGMLVersion::End) {
		throw std::out_of_range("for eCityGMLVersion& operator ++ (eCityGMLVersion&)");
	}
	ver = eCityGMLVersion(static_cast<std::underlying_type<eCityGMLVersion>::type>(ver) + 1);
	return ver;
}

const std::wstring uroNamespace1 = L"https://www.chisou.go.jp/tiiki/toshisaisei/itoshisaisei/iur/uro/1.5";
const std::wstring uroNamespace2 = L"https://www.geospatial.jp/iur/uro/2.0";

#define NAME_SPACE _T("xmlns:core='http://www.opengis.net/citygml/2.0' xmlns:bldg='http://www.opengis.net/citygml/building/2.0'  xmlns:uro='https://www.chisou.go.jp/tiiki/toshisaisei/itoshisaisei/iur/uro/1.5' xmlns:gen='http://www.opengis.net/citygml/generics/2.0' xmlns:gml='http://www.opengis.net/gml'")
#define NAME_SPACE2 _T("xmlns:core='http://www.opengis.net/citygml/2.0' xmlns:bldg='http://www.opengis.net/citygml/building/2.0'  xmlns:uro='https://www.geospatial.jp/iur/uro/2.0' xmlns:gen='http://www.opengis.net/citygml/generics/2.0' xmlns:gml='http://www.opengis.net/gml'")

#define XPATH1 _T("core:CityModel/core:cityObjectMember/bldg:Building/bldg:boundedBy/bldg:RoofSurface")
#define XPATH2 _T("core:CityModel/core:cityObjectMember")
#define XPATH3 _T("bldg:Building")
#define XPATH4 _T("bldg:Building/bldg:boundedBy")
#define XPATH5 _T("bldg:RoofSurface")
#define XPATH6 _T("bldg:RoofSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")
#define XPATH7 _T("gml:Polygon/gml:exterior/gml:LinearRing/gml:posList")

#define XPATH_measureAttribute1 _T("bldg:Building/gen:measureAttribute[@name='�N�ԗ\�����˗�']")
#define XPATH_measureAttribute2 _T("gen:value")
#define XPATH_measuredHeight1 _T("bldg:Building/bldg:measuredHeight")

// 2021
#define XPATH_genericAttributeSet1 _T("bldg:Building/gen:genericAttributeSet[contains(@name,'�^���Z��')]")
#define XPATH_genericAttributeSet2 _T("gen:measureAttribute[@name='�Z���[']")
#define XPATH_genericAttributeSet3 _T("gen:value")
#define XPATH_genericAttributeSet4 _T("bldg:Building/gen:genericAttributeSet[contains(@name,'�Ôg�Z��')]")
#define XPATH_genericAttributeSet5 _T("bldg:Building/gen:genericAttributeSet[@name='�y���ЊQ�x�����']")
// 2022
#define XPATH_genericAttributeSet1_2 _T("bldg:Building/uro:buildingDisasterRiskAttribute/uro:BuildingRiverFloodingRiskAttribute") // �^���Z��
#define XPATH_genericAttributeSet2_2 _T("uro:depth")
#define XPATH_genericAttributeSet3_2 _T("bldg:Building/uro:buildingDisasterRiskAttribute/uro:BuildingTsunamiRiskAttribute") // �Ôg�Z��
#define XPATH_genericAttributeSet4_2 _T("bldg:Building/uro:buildingDisasterRiskAttribute/uro:BuildingLandSlideRiskAttribute") // �y���ЊQ
#define XPATH_genericAttributeSet5_2 _T("uro:description")

// 2021
#define XPATH_buildingStructureType _T("bldg:Building/uro:buildingDetails/uro:BuildingDetails/uro:buildingStructureType")
// 2022
#define XPATH_buildingStructureType_2 _T("bldg:Building/uro:buildingDetailAttribute/uro:BuildingDetailAttribute/uro:buildingStructureType")

#define XPATH_extendedAttribute1 _T("bldg:Building/uro:extendedAttribute")
#define XPATH_extendedAttribute2 _T("uro:KeyValuePair/uro:key")
#define XPATH_extendedAttribute3 _T("uro:KeyValuePair/uro:codeValue")

#define XPATH_aggregateData1 _T("gen:measureAttribute[@name='�N�ԗ\�����˗�']")
#define XPATH_aggregateData2 _T("gen:value")
#define XPATH_aggregateData3 _T("gen:measureAttribute[@name='�N�ԗ\�����d��']")
#define XPATH_aggregateData4 _T("gen:measureAttribute[@name='���Q�������ԁi�Ď��j']")
#define XPATH_aggregateData5 _T("gen:measureAttribute[@name='���Q�������ԁi�t���j']")
#define XPATH_aggregateData6 _T("gen:measureAttribute[@name='���Q�������ԁi�~���j']")

// ����ID
#define XPATH_stringAttribute1 _T("bldg:Building/gen:stringAttribute[@name='����ID']")
#define XPATH_stringAttribute2 _T("gen:value")
#define XPATH_stringAttribute1_2 _T("bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute")
#define XPATH_stringAttribute2_2 _T("uro:buildingID")

#define CANCELFILE _T("cancel.txt")

const std::string judgeFile = "�����ʓK�n���茋��.csv";
const std::string outputFile = "�W�v����.csv";
const std::string extension_csv = ".csv";
const std::string extension_gml = ".gml";
const std::string outputHeader = "�͈͓�������,�N�ԗ\�����˗ʑ��v,�N�ԗ\�����d�ʑ��v,���Q�𔭐������錚����,���Q�������ԑ��v�i�Ď��j,���Q�������ԑ��v�i�t���j,���Q�������ԑ��v�i�~���j,�͈͓��D��x5������,�͈͓��D��x4������,�͈͓��D��x3������,�͈͓��D��x2������,�͈͓��D��x1������";
const int priorityLevel1 = 1;
const int priorityLevel2 = 2;
const int priorityLevel3 = 3;
const int priorityLevel4 = 4;
const int priorityLevel5 = 5;

// �n�ԍ�
int JPZONE;


// �G�N�X�|�[�g�ƃC���|�[�g�̐؂�ւ�
#ifdef AGGREGATEDATA_EXPORTS
#define VC_DLL_EXPORTS extern "C" __declspec(dllexport)
#else
#define VC_DLL_EXPORTS extern "C" __declspec(dllimport)
#endif

VC_DLL_EXPORTS int __cdecl AggregateBldgFiles(const char* str, const char* strOut);
VC_DLL_EXPORTS int __cdecl AggregateAllData(const char* str, const char* strOut);
VC_DLL_EXPORTS void __cdecl SetJPZone();

VC_DLL_EXPORTS void* __cdecl GetAllList();

// �K�n���茋��
typedef struct judgeSuitablePlace
{
	std::string strBuildingId;           // ����ID
	int priority;						 // �D��x
	std::string condition_1_1_1;		 // �������_1_1_1
	std::string condition_1_1_2;		 // �������_1_1_2
	std::string condition_1_2;			 // �������_1_2
	std::string condition_1_3;			 // �������_1_3
	std::string condition_2_1;			 // �������_2_1
	std::string condition_2_2;			 // �������_2_2
	std::string condition_2_3;			 // �������_2_3
	std::string condition_2_4;			 // �������_2_4
	std::string condition_3_1;			 // �������_3_1
	std::string condition_3_2;			 // �������_3_2
	std::string condition_3_3;			 // �������_3_3

} JUDGESUITABLEPLACE;

// �K�n���胊�X�g
typedef struct judgeList
{
	int meshID;                          // ���b�V��ID
	std::vector<JUDGESUITABLEPLACE> judgeSuitablePlaceList;  // �K�n���茋�ʃ��X�g

} JUDGELIST;

// �����ڍ�
typedef struct surfaceMembers
{
	std::vector<CPointBase> posList;               // ���W���X�g
} SURFACEMEMBERS;

// ����
typedef struct roofSurfaces
{
	std::string roofSurfaceId;                     // ����ID
	std::vector<SURFACEMEMBERS> roofSurfaceList;   // �����ڍ׃��X�g
} ROOFSURFACES;

// ����
typedef struct building
{

	std::string strBuildingId;           // ����ID
	double dBuildingHeight;		         // ����
	std::vector<ROOFSURFACES> roofSurfaceList;   // �������X�g
	double dSolorRadiation;              // �N�ԗ\�����˗�(kWh/m2)
	int iBldStructureType;               // �\�����
	double dFloodDepth;                  // �^���Z���z��̐Z���[(���[�g��)
	double dTsunamiHeight;               // �Ôg�Z���z��(���[�g��)
	bool bLandslideArea;                 // �y���ЊQ�x�����(�^�O�������true)
	int iBldStructureType2;              // �s�s���Ƃ̓Ǝ��敪�Ɋ�Â����z�\���̎��
	int iFloorType;                      // �s�s���Ƃ̓Ǝ��敪�Ɋ�Â��n��K���͈̔�

} BUILDING;


// �������X�g
typedef struct bldgList
{
	int meshID;                          // ���b�V��ID
	std::vector<BUILDING> buildingList;  // �������X�g

} BLDGLIST;

// �W�v���������
typedef struct agtBuilding
{
	std::string strBuildingId;           // ����ID
	double dSolorRadiation;              // �N�ԗ\�����˗�(kWh/m2)
	double dElectricGeneration;          // �N�ԗ\�����d��(kWh/�N)
	double dLightPollutionSummer;        // ���Q��������(�Ď�)
	double dLightPollutionSpling;        // ���Q��������(�t��)
	double dLightPollutionWinter;        // ���Q��������(�~��)

} AGTBUILDING;


// �W�v���������X�g
typedef struct agtBldgList
{
	int meshID;                           // ���b�V��ID
	std::vector<AGTBUILDING> buildingList;// �������X�g

} AGTBLDGLIST;

// �S�̃��X�g
std::vector<BLDGLIST> allList{};
void* GetAllList()
{
	return (void*)(&allList);
}

class AggregateData
{
public:
	AggregateData(void);
	~AggregateData(void);

};

