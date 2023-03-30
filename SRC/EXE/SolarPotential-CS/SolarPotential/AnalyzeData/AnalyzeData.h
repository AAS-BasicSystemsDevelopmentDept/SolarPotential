#pragma once
#include "stdafx.h"
#include "../../../../LIB/CommonUtil/CGeoUtil.h"


#include <Windows.h>
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
#include <fstream>
#include <iomanip>

#include <assert.h>
#include <cmath>

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")


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

#define NAME_SPACE _T("xmlns:core='http://www.opengis.net/citygml/2.0' xmlns:bldg='http://www.opengis.net/citygml/building/2.0' xmlns:gen='http://www.opengis.net/citygml/generics/2.0' xmlns:gml='http://www.opengis.net/gml' xmlns:app='http://www.opengis.net/citygml/appearance/2.0' xmlns:uro='https://www.geospatial.jp/iur/uro/2.0'")
#define DEM_NAME_SPACE _T("xmlns:core='http://www.opengis.net/citygml/2.0' xmlns:dem='http://www.opengis.net/citygml/relief/2.0' xmlns:gml='http://www.opengis.net/gml'")

#define XPATH1 _T("bldg:Building/bldg:lod2Solid")
#define XPATH2 _T("core:CityModel/core:cityObjectMember")
#define XPATH3 _T("bldg:Building")
#define XPATH4 _T("bldg:Building/bldg:boundedBy")
#define XPATH5 _T("bldg:RoofSurface")
#define XPATH6 _T("bldg:RoofSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")
#define XPATH7 _T("gml:Polygon")
#define XPATH8 _T("gml:Polygon/gml:exterior/gml:LinearRing")
#define XPATH9 _T("gml:Polygon/gml:exterior/gml:LinearRing/gml:posList")

#define XPATH10 _T("bldg:WallSurface")
#define XPATH11 _T("bldg:WallSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")

#define XPATH12 _T("bldg:Building/bldg:lod1Solid/gml:Solid")
#define XPATH13 _T("gml:exterior/gml:CompositeSurface/gml:surfaceMember")

// ����ID
#define XPATH_stringAttribute1 _T("bldg:Building/gen:stringAttribute[@name='����ID']")
#define XPATH_stringAttribute2 _T("gen:value")
#define XPATH_stringAttribute1_2 _T("bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute")
#define XPATH_stringAttribute2_2 _T("uro:buildingID")

#define BOUND_XPATH1 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:lowerCorner")
#define BOUND_XPATH2 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:upperCorner")


#define DEM_XPATH1 _T("dem:ReliefFeature/dem:reliefComponent/dem:TINRelief/dem:tin/gml:TriangulatedSurface/gml:trianglePatches/gml:Triangle")
#define DEM_XPATH2 _T("gml:exterior/gml:LinearRing/gml:posList")
#define DEM_XPATH3 _T("dem:ReliefFeature/gml:name")

#define TEX_XPATH1 _T("core:CityModel")
#define TEX_XPATH2 _T("core:CityModel/app:appearanceMember/app:Appearance")
#define TEX_XPATH3 _T("core:CityModel/app:appearanceMember")
#define OUTPUTFILE _T("initFile_Coordinates.txt")
#define CANCELFILE _T("cancel.txt")

#define INPUTFILE1 _T("���������Q��������.csv")         // �\�����Q��������
#define INPUTFILE2 _T("�������N�ԗ\�����d��.csv")       // �N�ԗ\�����˗ʁE�N�ԗ\�����d��

// �n�ԍ�
int JPZONE;

// �ő�ܓx�o�x
CPoint2D maxPosition;
// �ŏ��ܓx�o�x
CPoint2D minPosition;

// �S��jpg�T�C�Y
int jpgWidth = 0;
int jpgHeight = 0;

// �؂�o��jpg�T�C�Y
int trimWidth = 0;
int trimHeight = 0;

// jpg�������W
double tfw_x = 0.0;;
double tfw_y = 0.0;;
double tfw_meshSize = 0.0;


// 1m���b�V�����W
typedef struct meshPositionXY
{
    double leftTopX;                        // ����X
    double leftTopY;                        // ����Y
    double leftDownX;                       // ����X
    double leftDownY;                       // ����Y
    double rightTopX;                       // �E��X
    double rightTopY;                       // �E��Y
    double rightDownX;                      // �E��X
    double rightDownY;                      // �E��Y

} MESHPOSITION_XY;

// DEM���W
typedef struct demPosition
{
    double lon;		                        // �o�x
    double lat;		                        // �ܓx
    double ht;		                        // ����

} DEMPOSITION;

// DEM���b�V��
typedef struct demList
{
    std::string meshID;		                    // ���b�V��ID
    std::vector<CTriangle> posTriangleList;     // �O�p�`�̍��W���X�g

} DEMLIST;

// ���W
typedef struct position
{
    double lon;		                        // �o�x
    double lat;		                        // �ܓx
    double ht;		                        // ����


} POSITION;

// �����ڍ�
typedef struct surfaceMembers
{
    std::string polygon;		                    // �|���S��ID
    std::string linearRing;		                // ���C��ID
    std::vector<CPointBase> posList;               // ���W���X�g

} SURFACEMEMBERS;

// ����
typedef struct roofSurfaces
{
    std::string roofSurfaceId;		            // ����ID
    std::vector<SURFACEMEMBERS> roofSurfaceList; // �����ڍ׃��X�g
    std::vector<MESHPOSITION_XY> meshPosList;       // 1m���b�V�����W���X�g

} ROOFSURFACES;

// ��
typedef struct wallSurfaces
{
    std::string wallSurfaceId;		            // ��ID
    std::vector<SURFACEMEMBERS> wallSurfaceList; // �Ǐڍ׃��X�g

} WALLSURFACES;

// ����LOD2
typedef struct buildings
{
    std::string building;		                // ����ID
    std::vector<ROOFSURFACES> roofSurfaceList;   // �������X�g
    std::vector<WALLSURFACES> wallSurfaceList;   // �ǃ��X�g

} BUILDINGS;

// ����LOD1(LOD1�����Ȃ�����)
typedef struct buildingsLOD1
{
    std::string building;
    std::vector<WALLSURFACES> wallSurfaceList;   // �ǃ��X�g�i�S�Ă̖ʁj
}BUILDINGSLOD1;

// �����A���b�V�����W
typedef struct buildingsInfo
{
    std::vector<BUILDINGS> buildingList;        // �������X�g
    std::vector<BUILDINGSLOD1> buildingListLOD1; // �������X�g(LOD1)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
} BUILDINGSINFO;

// ���b�V��
typedef struct bldgList
{
    std::string meshID;		                    // ���b�V��ID
    std::vector<BUILDINGS> buildingList;        // �������X�g
    std::vector<BUILDINGSLOD1> buildingListLOD1; // �������X�g(LOD1)
    // ���b�V�����W��MIN,MAX(���ʒ��p���W)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
} BLDGLIST;

// LOD2�o�͍���
typedef struct lod2outList
{
    std::string meshID;		                    // ���b�V��ID
    std::string building;		                // ����ID
    std::string solarInsolation;                // �N�ԓ��˗�
    std::string solarPowerGeneration;           // �N�Ԕ��d��
    std::string lightPollutionTimeSpring;		// ���Q�������Ԑ�_�t��
    std::string lightPollutionTimeSummer;		// ���Q�������Ԑ�_�Ď�
    std::string lightPollutionTimeWinter;		// ���Q�������Ԑ�_�~��

    bool operator<(const lod2outList& right) const {
        return meshID == right.meshID ? building < right.building : meshID < right.meshID;
    }

} LOD2OUTLIST;

// �iCSV�ǂݎ��p�j�N�ԗ\�����˗ʁA���d��
enum struct yearPrediction
{
    meshID = 0,                 // ���b�V��ID
    building = 1,               // ����ID
    solarInsolation = 2,        // ���˗�
    solarPowerGeneration = 3,   // ���d��
};

// �iCSV�ǂݎ��p�j���Q�������ԑ���
enum struct lightPollution
{
    meshID = 0,                 // ���b�V��ID
    building = 1,               // ����ID
    summer = 2,                 // �Ď�
    spling = 3,                 // �t��
    winter = 4,                 // �~��
};

// �������X�g���擾
std::vector<BLDGLIST> allList{};
void* __cdecl GetAllList()
{
    return (void*)(&allList);
}

// DEM���X�g���擾
std::vector<DEMLIST> allDemList{};
void* __cdecl GetAllDemList()
{
    return (void*)(&allDemList);
}

// �G���R�[�_�[�̎擾
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

// src�Ŏw�肷��jpeg�t�@�C���̋�`�̈�(����sx,sy �� width ���� height)�Ńg���~���O��dtc�Ŏw�肷��jpeg�t�@�C���ɕۑ�
int imgcut(TCHAR* dtc, TCHAR* src, int sx, int sy, int wdith, int height);

class AnalyzeData
{
public:
	AnalyzeData(void);
	~AnalyzeData(void);

};
