#pragma once

#include <string>
#include <vector>
using namespace std;

#define CANCELFILE "cancel.txt"

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

// vector<BLDGLIST>���擾����
extern "C" __declspec(dllimport) void* __cdecl GetAllList();

// DEM���X�g���擾
extern "C" __declspec(dllimport) void* __cdecl GetAllDemList();
