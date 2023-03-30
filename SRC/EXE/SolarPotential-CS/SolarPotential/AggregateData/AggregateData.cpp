#include "pch.h"
#include "AggregateData.h"
#include "../../../../LIB/CommonUtil/CGeoUtil.h"
#include "../../../../LIB/CommonUtil/ReadINIParam.h"
#include "../../../../LIB/CommonUtil/CFileIO.h"

#ifdef _DEBUG
#include <sys/timeb.h>
#include <time.h>
#include <psapi.h>
#endif

using namespace std;

AggregateData::AggregateData(void)
{
}

AggregateData::~AggregateData(void)
{
}

// split�֐�
vector<string> split(string str, string separator) {
    if (separator == "") return { str };
    vector<string> result;
    string tstr = str + separator;
    long l = (long)tstr.length(), sl = (long)separator.length();
    string::size_type pos = 0, prev = 0;

    for (;pos < l && (pos = tstr.find(separator, pos)) != string::npos; prev = (pos += sl)) {
        result.emplace_back(tstr, prev, pos - prev);
    }
    return result;
}

void SetJPZone()
{
    GetINIParam()->Initialize();
    JPZONE = GetINIParam()->GetJPZone();
}

// BSTR��std::string�ϊ�����
std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen)
{
    int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

    std::string dblstr(len, '\0');
    len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */,
        pstr, wslen /* not necessary NULL-terminated */,
        &dblstr[0], len,
        NULL, NULL /* no default char */);

    return dblstr;
}
// BSTR��std::string�ϊ�
std::string ConvertBSTRToMBS(BSTR bstr)
{
    int wslen = ::SysStringLen(bstr);
    return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}
/*
    string��wstring�֕ϊ�����
*/
std::wstring StringToWString
(
    std::string oString
)
{
    // SJIS �� wstring
    int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str()
        , -1, (wchar_t*)NULL, 0);

    // �o�b�t�@�̎擾
    wchar_t* cpUCS2 = new wchar_t[iBufferSize];

    // SJIS �� wstring
    MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2
        , iBufferSize);

    // string�̐���
    std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

    // �o�b�t�@�̔j��
    delete[] cpUCS2;

    // �ϊ����ʂ�Ԃ�
    return(oRet);
}
// std::string��wchar_t�ϊ�����
wchar_t* ConvertStringTowchar(std::string str)
{
    const size_t newsizew = str.size() + 1;
    size_t convertedChars = 0;
    wchar_t* wcstring = new wchar_t[newsizew];
    mbstowcs_s(&convertedChars, wcstring, newsizew, str.c_str(), _TRUNCATE);

    return wcstring;
}
/**
* @brief �t�H���_�ȉ��̃t�@�C���ꗗ���擾����֐�
* @param[in]    folderPath  �t�H���_�p�X
* @param[out]   file_names  �t�@�C�����ꗗ
* return        true:����, false:���s
*/
bool getFileNames(std::string folderPath, std::string ext, std::vector<std::string>& file_names)
{
    using namespace std::filesystem;
    directory_iterator iter(folderPath), end;
    std::error_code err;

    for (; iter != end && !err; iter.increment(err)) {
        const directory_entry entry = *iter;

        if (entry.path().extension() == ext) {
            file_names.push_back(entry.path().string());
            printf("%s\n", file_names.back().c_str());
        }
    }

    /* �G���[���� */
    if (err) {
        std::cout << err.value() << std::endl;
        std::cout << err.message() << std::endl;
        return false;
    }
    return true;
}
// CSV�ǂݍ���
vector<vector<string> >csv2vector(string filename, int ignore_line_num = 0) {
    //csv�t�@�C���̓ǂݍ���
    ifstream reading_file;
    reading_file.open(filename, ios::in);
    if (!reading_file) {
        vector<vector<string> > data;
        return data;
    }
    string reading_line_buffer;
    //�ŏ���ignore_line_num�s����ǂ݂���
    for (int line = 0; line < ignore_line_num; line++) {
        getline(reading_file, reading_line_buffer);
        if (reading_file.eof()) break;
    }

    //�񎟌���vector���쐬
    vector<vector<string> > data;
    while (getline(reading_file, reading_line_buffer)) {
        if (reading_line_buffer.size() == 0) break;
        vector<string> temp_data;
        temp_data = split(reading_line_buffer, ",");
        data.push_back(temp_data);
    }
    return data;
}

// ����������(YYYYMMDDhhmmss)�擾
string getDatetimeStr() {
    time_t t = time(nullptr);
    struct tm nowTime;
    errno_t error;
    error = localtime_s(&nowTime, &t);

    std::stringstream s;
    s << nowTime.tm_year + 1900;
    // setw(),setfill()��0�l��
    s << setw(2) << setfill('0') << nowTime.tm_mon + 1;
    s << setw(2) << setfill('0') << nowTime.tm_mday;
    s << setw(2) << setfill('0') << nowTime.tm_hour;
    s << setw(2) << setfill('0') << nowTime.tm_min;
    s << setw(2) << setfill('0') << nowTime.tm_sec;
    // std::string�ɂ��Ēl��Ԃ�
    return s.str();
}

// ����ID���擾����
bool getBuildId(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, std::string& strid)
{
    bool bret = false;
    HRESULT hResult;

    // ����ID�̃m�[�h���擾
    MSXML2::IXMLDOMNodePtr building = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR build = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            build = SysAllocString(XPATH_stringAttribute1);
            break;

        case eCityGMLVersion::VERSION_2:
            build = SysAllocString(XPATH_stringAttribute1_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectSingleNode(build, &building);
        if (building != NULL)   break;

    }

    if (version == eCityGMLVersion::End)
    {   // �擾���s
        return false;
    }

    // �l���擾
    MSXML2::IXMLDOMNodePtr buildingValue = 0;
    BSTR val = _bstr_t("");
    switch (version)
    {
    case eCityGMLVersion::VERSION_1:
        val = SysAllocString(XPATH_stringAttribute2);
        break;

    case eCityGMLVersion::VERSION_2:
        val = SysAllocString(XPATH_stringAttribute2_2);
        break;

    default:
        break;
    }
    building->selectSingleNode(val, &buildingValue);
    if (NULL != buildingValue)
    {
        // �m�[�h�^�C�v�擾
        MSXML2::DOMNodeType eMemberNodeType;
        hResult = buildingValue->get_nodeType(&eMemberNodeType);
        if (FAILED(hResult))
        {
            assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
            return false;
        }

        // �G�������g�^�ւ̕ϊ�
        MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
        hResult = buildingValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
        if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
        {
            assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
            return false;
        }

        // �l�e�L�X�g�擾
        BSTR valueText;
        hResult = pXMLDOMMemberElement->get_text(&valueText);
        if (SUCCEEDED(hResult))
        {
            // BSTR��std::string�ϊ�
            strid = ConvertBSTRToMBS(valueText);
            bret = true;
        }
    }

    return bret;
}

// �^���Z�����X�N�̐Z���[���擾����
bool getRiverFloodingRisk(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, double& depth)
{
    bool bret = false;
    HRESULT hResult;

    // �^���Z���z��̃m�[�h���擾
    MSXML2::IXMLDOMNodeListPtr genericAttributeSetList = NULL;
    long lGenericAttributeCountNode = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR xpGeneric1 = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            xpGeneric1 = SysAllocString(XPATH_genericAttributeSet1);
            break;

        case eCityGMLVersion::VERSION_2:
            xpGeneric1 = SysAllocString(XPATH_genericAttributeSet1_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectNodes(xpGeneric1, &genericAttributeSetList);
        if (genericAttributeSetList == NULL) continue;

        // �m�[�h����
        hResult = genericAttributeSetList->get_length(&lGenericAttributeCountNode);
        if ( 0 != lGenericAttributeCountNode) {
            break;
        }
    }

    if (version == eCityGMLVersion::End)
    {   // �擾���s
        return false;
    }

    // �^���Z�������J��Ԃ�
    for (int j = 0; j < lGenericAttributeCountNode; j++) {

        // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
        MSXML2::IXMLDOMNodePtr pXMLDOMGenericNode = NULL;
        hResult = genericAttributeSetList->get_item(j, &pXMLDOMGenericNode);
        if (FAILED(hResult))
        {
            assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
            continue;
        }

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
        {
            MSXML2::IXMLDOMNodePtr floodDepth = 0;
            BSTR flood = SysAllocString(XPATH_genericAttributeSet2);
            pXMLDOMGenericNode->selectSingleNode(flood, &floodDepth);
            if (NULL != floodDepth)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr floodDepthValue = 0;
                BSTR val = SysAllocString(XPATH_genericAttributeSet3);
                floodDepth->selectSingleNode(val, &floodDepthValue);
                if (NULL != floodDepthValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = floodDepthValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = floodDepthValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        if (depth < stod(valueStr)) {
                            depth = stod(valueStr);
                        }
                        bret = true;
                    }
                }
            }
            break;
        }

        case eCityGMLVersion::VERSION_2:
        {
            // �l���擾
            MSXML2::IXMLDOMNodePtr floodDepthValue = 0;
            BSTR val = SysAllocString(XPATH_genericAttributeSet2_2);
            pXMLDOMGenericNode->selectSingleNode(val, &floodDepthValue);
            if (NULL != floodDepthValue)
            {
                // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType eMemberNodeType;
                hResult = floodDepthValue->get_nodeType(&eMemberNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                hResult = floodDepthValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    continue;
                }

                // �l�e�L�X�g�擾
                BSTR valueText;
                hResult = pXMLDOMMemberElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR��std::string�ϊ�
                    std::string valueStr = ConvertBSTRToMBS(valueText);
                    if (depth < stod(valueStr)) {
                        depth = stod(valueStr);
                    }
                    bret = true;
                }
            }
            break;
        }

        default:
            break;
        }
    }

    return true;
}

// �Ôg�Z�����X�N�̐Z���[���擾����
bool getTsunamiRisk(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, double& height)
{
    bool bret = false;
    HRESULT hResult;

    // �Ôg�Z���z��̃m�[�h���擾
    MSXML2::IXMLDOMNodePtr tsunamiHeightNode = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR tsunami = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            tsunami = SysAllocString(XPATH_genericAttributeSet4);
            break;

        case eCityGMLVersion::VERSION_2:
            tsunami = SysAllocString(XPATH_genericAttributeSet3_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectSingleNode(tsunami, &tsunamiHeightNode);
        if (tsunamiHeightNode != NULL)   break;

    }

    if (version == eCityGMLVersion::End)
    {   // �擾���s
        return false;
    }

    switch (version)
    {
    case eCityGMLVersion::VERSION_1:
    {
        // �Z���[
        MSXML2::IXMLDOMNodePtr tsunamiHeight = 0;
        BSTR flood = SysAllocString(XPATH_genericAttributeSet2);
        tsunamiHeightNode->selectSingleNode(flood, &tsunamiHeight);
        if (NULL != tsunamiHeight)
        {
            // �l���擾
            MSXML2::IXMLDOMNodePtr tsunamiHeightValue = 0;
            BSTR val = SysAllocString(XPATH_genericAttributeSet3);
            tsunamiHeight->selectSingleNode(val, &tsunamiHeightValue);
            if (NULL != tsunamiHeightValue)
            {
                // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType eMemberNodeType;
                hResult = tsunamiHeight->get_nodeType(&eMemberNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    return false;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                hResult = tsunamiHeight->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    return false;
                }

                // �l�e�L�X�g�擾
                BSTR valueText;
                hResult = pXMLDOMMemberElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR��std::string�ϊ�
                    std::string valueStr = ConvertBSTRToMBS(valueText);
                    height = stod(valueStr);
                    bret = true;
                }
            }
        }
        break;
    }

    case eCityGMLVersion::VERSION_2:
    {
        // �l���擾
        MSXML2::IXMLDOMNodePtr tsunamiHeightValue = 0;
        BSTR val = SysAllocString(XPATH_genericAttributeSet2_2);
        tsunamiHeightNode->selectSingleNode(val, &tsunamiHeightValue);
        if (NULL != tsunamiHeightValue)
        {
            // �m�[�h�^�C�v�擾
            MSXML2::DOMNodeType eMemberNodeType;
            hResult = tsunamiHeightValue->get_nodeType(&eMemberNodeType);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                return false;
            }

            // �G�������g�^�ւ̕ϊ�
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hResult = tsunamiHeightValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                return false;
            }

            // �l�e�L�X�g�擾
            BSTR valueText;
            hResult = pXMLDOMMemberElement->get_text(&valueText);
            if (SUCCEEDED(hResult))
            {
                // BSTR��std::string�ϊ�
                std::string valueStr = ConvertBSTRToMBS(valueText);
                height = stod(valueStr);
                bret = true;
            }
        }
        break;
    }

    default:
        break;
    }

    return true;
}

// �y���ЊQ���X�N���擾����
bool getLandSlideRisk(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode)
{
    // �y���ЊQ���X�N�̃m�[�h���擾
    MSXML2::IXMLDOMNodePtr landslideArea = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR landslide = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            landslide = SysAllocString(XPATH_genericAttributeSet5);
            break;

        case eCityGMLVersion::VERSION_2:
            landslide = SysAllocString(XPATH_genericAttributeSet4_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectSingleNode(landslide, &landslideArea);
        if (landslideArea != NULL)   break;

    }

    if (version == eCityGMLVersion::End)
    {   // �擾���s
        return false;
    }

    return true;
}

// �\������ʂ��擾����
bool getBuildStructureType(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, int& iBldStructureType)
{
    bool bret = false;
    HRESULT hResult;

    MSXML2::IXMLDOMNodePtr structureType = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR structure = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            structure = SysAllocString(XPATH_buildingStructureType);
            break;

        case eCityGMLVersion::VERSION_2:
            structure = SysAllocString(XPATH_buildingStructureType_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectSingleNode(structure, &structureType);
        if (structureType != NULL)   break;

    }

    if (version == eCityGMLVersion::End)
    {   // �擾���s
        return false;
    }

    // �m�[�h�^�C�v�擾
    MSXML2::DOMNodeType eMemberNodeType;
    hResult = structureType->get_nodeType(&eMemberNodeType);
    if (FAILED(hResult))
    {
        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
        return false;
    }

    // �G�������g�^�ւ̕ϊ�
    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
    hResult = structureType->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
    {
        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
        return false;
    }

    // �l�e�L�X�g�擾
    BSTR valueText;
    hResult = pXMLDOMMemberElement->get_text(&valueText);
    if (SUCCEEDED(hResult))
    {
        // BSTR��std::string�ϊ�
        std::string valueStr = ConvertBSTRToMBS(valueText);
        iBldStructureType = stoi(valueStr);
        bret = true;
    }

    return bret;
}

// ���������擾
vector<BUILDING> GetBldgAttribute(wstring xmldata)
{
    //xml�I�u�W�F�N�g����
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpath���T�|�[�g����悤�ɐݒ�
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));

    // uro�^�O�̖��O���URI���`�F�b�N
    CFileIO fio;
    if (fio.Open(xmldata, L"rt"))
    {
        std::wstring strLine;
        int lineCnt = 0;
        wchar_t cBuff[1024];

        while (fio.ReadLineW(cBuff, 1024) != NULL)
        {
            strLine = cBuff;
            if (strLine.find(L"core:CityModel") != std::string::npos)
            {
                if (strLine.find(uroNamespace1) != std::string::npos)    // ���o�[�W����
                {
                    //namespace���T�|�[�g����悤�ɐݒ�
                    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE));
                }
                else if (strLine.find(uroNamespace2) != std::string::npos)    // �V�o�[�W����
                {
                    //namespace���T�|�[�g����悤�ɐݒ�
                    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE2));
                }
                break;
            }
        }
    }
    else
    {
        //namespace���T�|�[�g����悤�ɐݒ�
        reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE));
    }
    fio.Close();

    //���[�h
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    // �������X�g
    vector<BUILDING> allBuildingList{};

    HRESULT hResult;

    BSTR xp2 = SysAllocString(XPATH2);
    MSXML2::IXMLDOMNodeListPtr buildingList = NULL;
    reader->selectNodes(xp2, &buildingList);

    // �m�[�h�����擾
    long lCountNode = 0;
    hResult = buildingList->get_length(&lCountNode);

    // ���������J��Ԃ�
    for (int i = 0; i < lCountNode; i++) {
        // ������񏉊���
        BUILDING buildingInfo{};

        // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
        MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
        hResult = buildingList->get_item(i, &pXMLDOMNode);
        if (FAILED(hResult))
        {
            assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
            continue;
        }

        // ����ID
        std::string buildId = "";
        if (!getBuildId(pXMLDOMNode, buildId))
        {
            assert(!"����ID�̎擾�Ɏ��s");
            continue;
        }
        buildingInfo.strBuildingId = buildId;

        // �N�ԗ\�����˗�
        MSXML2::IXMLDOMNodePtr solorRadiation = 0;
        BSTR solor = SysAllocString(XPATH_measureAttribute1);
        pXMLDOMNode->selectSingleNode(solor, &solorRadiation);
        if (NULL != solorRadiation)
        {
            // �l���擾
            MSXML2::IXMLDOMNodePtr solorRadiationValue = 0;
            BSTR val = SysAllocString(XPATH_measureAttribute2);
            solorRadiation->selectSingleNode(val, &solorRadiationValue);
            if (NULL != solorRadiationValue)
            {
                // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType eMemberNodeType;
                hResult = solorRadiationValue->get_nodeType(&eMemberNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                hResult = solorRadiationValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    continue;
                }

                // �l�e�L�X�g�擾
                BSTR valueText;
                hResult = pXMLDOMMemberElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR��std::string�ϊ�
                    std::string valueStr = ConvertBSTRToMBS(valueText);
                    buildingInfo.dSolorRadiation = stod(valueStr);

                }
            }
        }

        // ����
        MSXML2::IXMLDOMNodePtr height = 0;
        BSTR ht = SysAllocString(XPATH_measuredHeight1);
        pXMLDOMNode->selectSingleNode(ht, &height);
        if (NULL != height)
        {
            // �m�[�h�^�C�v�擾
            MSXML2::DOMNodeType eMemberNodeType;
            hResult = height->get_nodeType(&eMemberNodeType);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                continue;
            }

            // �G�������g�^�ւ̕ϊ�
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hResult = height->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                continue;
            }

            // �l�e�L�X�g�擾
            BSTR valueText;
            hResult = pXMLDOMMemberElement->get_text(&valueText);
            if (SUCCEEDED(hResult))
            {
                // BSTR��std::string�ϊ�
                std::string valueStr = ConvertBSTRToMBS(valueText);
                buildingInfo.dBuildingHeight = stod(valueStr);

            }
                
        }

        // �^���Z���z��̐Z���[
        double wFloodDepth = 0.0;
        // �l������ΐݒ�(�ő�l)
        if (getRiverFloodingRisk(pXMLDOMNode, wFloodDepth))
        {
            buildingInfo.dFloodDepth = wFloodDepth;
        }

        // �Ôg�Z���z��̐Z���[
        double tsunamiHeight = 0.0;
        if (getTsunamiRisk(pXMLDOMNode, tsunamiHeight))
        {
            buildingInfo.dTsunamiHeight = tsunamiHeight;
        }

        // �y���ЊQ�x�����
        if (getLandSlideRisk(pXMLDOMNode))
        {
            buildingInfo.bLandslideArea = true;
        }
        else
        {
            buildingInfo.bLandslideArea = false;
        }

        // �\�����
        int iBldStructureType = 0;
        if (getBuildStructureType(pXMLDOMNode, iBldStructureType))
        {
            buildingInfo.iBldStructureType = iBldStructureType;
        }

        // �s�s���Ƃ̓Ǝ��敪�Ɋ�Â����z�\���̎��
        // �s�s���Ƃ̓Ǝ��敪�Ɋ�Â��n��K���͈̔�
        BSTR xpExtended1 = SysAllocString(XPATH_extendedAttribute1);
        MSXML2::IXMLDOMNodeListPtr extendedAttributeList = NULL;
        pXMLDOMNode->selectNodes(xpExtended1, &extendedAttributeList);

        // �m�[�h�����擾
        long lExtendedAttributeCountNode = 0;
        hResult = extendedAttributeList->get_length(&lExtendedAttributeCountNode);

        // �s�s���Ƃ̓Ǝ��敪�����J��Ԃ�
        for (int j = 0; j < lExtendedAttributeCountNode; j++) {

            // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
            MSXML2::IXMLDOMNodePtr pXMLDOMExtendedNode = NULL;
            hResult = extendedAttributeList->get_item(j, &pXMLDOMExtendedNode);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                continue;
            }

            MSXML2::IXMLDOMNodePtr keyValuePair = 0;
            BSTR key = SysAllocString(XPATH_extendedAttribute2);
            pXMLDOMExtendedNode->selectSingleNode(key, &keyValuePair);
            if (NULL != keyValuePair)
            {
                // �l���擾
                    // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType ekeyValuePairNodeType;
                hResult = keyValuePair->get_nodeType(&ekeyValuePairNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMkeyValuePairElement = NULL;
                hResult = keyValuePair->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMkeyValuePairElement);
                if (FAILED(hResult) || NULL == pXMLDOMkeyValuePairElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    continue;
                }

                // �l�e�L�X�g�擾
                BSTR valueText;
                std::string keyValueStr;
                hResult = pXMLDOMkeyValuePairElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR��std::string�ϊ�
                    keyValueStr = ConvertBSTRToMBS(valueText);
                }

                // ���z�\���̎��or�n��K���͈̔�
                if (keyValueStr == "10" || keyValueStr == "100")
                {
                    // �l���擾
                    MSXML2::IXMLDOMNodePtr codeValue = 0;
                    BSTR val = SysAllocString(XPATH_extendedAttribute3);
                    pXMLDOMExtendedNode->selectSingleNode(val, &codeValue);
                    if (NULL != codeValue)
                    {
                        // �m�[�h�^�C�v�擾
                        MSXML2::DOMNodeType eCodeValueNodeType;
                        hResult = codeValue->get_nodeType(&eCodeValueNodeType);
                        if (FAILED(hResult))
                        {
                            assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                            continue;
                        }

                        // �G�������g�^�ւ̕ϊ�
                        MSXML2::IXMLDOMElementPtr pXMLDOMCodeValueElement = NULL;
                        hResult = codeValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMCodeValueElement);
                        if (FAILED(hResult) || NULL == pXMLDOMCodeValueElement)
                        {
                            assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                            continue;
                        }

                        // �l�e�L�X�g�擾
                        BSTR valueText;
                        hResult = pXMLDOMCodeValueElement->get_text(&valueText);
                        if (SUCCEEDED(hResult))
                        {
                            // BSTR��std::string�ϊ�
                            std::string valueStr = ConvertBSTRToMBS(valueText);
                            if (keyValueStr == "10")
                            {
                                buildingInfo.iBldStructureType2 = stoi(valueStr);
                            }
                            else if(keyValueStr == "100")
                            {
                                buildingInfo.iFloorType = stoi(valueStr);
                            }
                        }
                    }
                }
            }

        }

        // �����ʍ��W
        BSTR xp4 = SysAllocString(XPATH4);
        MSXML2::IXMLDOMNodeListPtr roofSurfaceList = NULL;
        pXMLDOMNode->selectNodes(xp4, &roofSurfaceList);

        // �m�[�h�����擾
        long lRoofSurfaceCountNode = 0;
        hResult = roofSurfaceList->get_length(&lRoofSurfaceCountNode);

        // ���������J��Ԃ�
        for (int j = 0; j < lRoofSurfaceCountNode; j++) {

            // ������񏉊���
            ROOFSURFACES roofSurfaces{};

            // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
            MSXML2::IXMLDOMNodePtr pXMLDOMRoofNode = NULL;
            hResult = roofSurfaceList->get_item(j, &pXMLDOMRoofNode);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                continue;
            }

            // �������^�O�I��
            MSXML2::IXMLDOMNodePtr roofSurface = 0;
            BSTR roof = SysAllocString(XPATH5);
            pXMLDOMRoofNode->selectSingleNode(roof, &roofSurface);
            // ������񂪂���Ώ��������s
            if (NULL != roofSurface) {
                // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType eRoofNodeType;
                hResult = roofSurface->get_nodeType(&eRoofNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMRoofElement = NULL;
                hResult = roofSurface->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMRoofElement);
                if (FAILED(hResult) || NULL == pXMLDOMRoofElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    continue;
                }

                // ����ID���擾
                MSXML2::IXMLDOMAttribute* pAttributeRoofNode = NULL;
                CComVariant varValue;
                BSTR id = SysAllocString(L"gml:id");
                hResult = pXMLDOMRoofElement->getAttribute(id, &varValue);
                if (SUCCEEDED(hResult) && VT_BSTR == varValue.vt)
                {
                    // BSTR��std::string�ϊ�
                    std::string roofSurfaceId = ConvertBSTRToMBS(varValue.bstrVal);
                    roofSurfaces.roofSurfaceId = roofSurfaceId;

                }

                // �����ڍ׏��擾(�m�[�h)
                BSTR xp6 = SysAllocString(XPATH6);
                MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                pXMLDOMRoofNode->selectNodes(xp6, &surfaceMemberList);

                // �m�[�h�����擾
                long lSurfaceMemberCountNode = 0;
                hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                // �����ڍא����J��Ԃ�
                for (int k = 0; k < lSurfaceMemberCountNode; k++) {

                    // ���[�N�����ڍ׏�����
                    SURFACEMEMBERS wSurfaceMembers{};

                    // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                    MSXML2::IXMLDOMNodePtr pXMLDOMSurfaceNode = NULL;
                    hResult = surfaceMemberList->get_item(k, &pXMLDOMSurfaceNode);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                        continue;
                    }


                    // ���W�^�O�I��
                    MSXML2::IXMLDOMNodePtr Position = 0;
                    BSTR pos = SysAllocString(XPATH7);
                    pXMLDOMSurfaceNode->selectSingleNode(pos, &Position);

                    if (NULL != Position) {
                        // �m�[�h�^�C�v�擾
                        MSXML2::DOMNodeType eMemberNodeType;
                        hResult = Position->get_nodeType(&eMemberNodeType);
                        if (FAILED(hResult))
                        {
                            assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                            continue;
                        }

                        // �G�������g�^�ւ̕ϊ�
                        MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                        hResult = Position->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                        if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                        {
                            assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                            continue;
                        }

                        // ���W�e�L�X�g�擾
                        BSTR positionText;
                        hResult = pXMLDOMMemberElement->get_text(&positionText);
                        if (SUCCEEDED(hResult))
                        {
                            // BSTR��std::string�ϊ�
                            std::string posStr = ConvertBSTRToMBS(positionText);

                            // ���W����
                            vector<std::string> posAry = split(posStr, " ");

                            // ���W�i�[
                            int posCnt = 0;
                            // �ꎞ���W���X�g������
                            CPointBase wPosition{};

                            // ���ʒ��p�ϊ��p
                            double dEast, dNorth;

                            for (int x = 0; x < posAry.size(); x++) {

                                if (posCnt == 0) {
                                    // double�ɕϊ�
                                    wPosition.y = std::stod(posAry[x]);

                                    posCnt++;
                                }
                                else if (posCnt == 1) {
                                    // double�ɕϊ�
                                    wPosition.x = std::stod(posAry[x]);

                                    posCnt++;
                                }
                                else if (posCnt == 2) {
                                    // double�ɕϊ�
                                    wPosition.z = std::stod(posAry[x]);

                                    // ���ʒ��p���W�ɕϊ�

                                    CGeoUtil::LonLatToXY(wPosition.x, wPosition.y, JPZONE, dEast, dNorth);
                                    CPointBase pt(dEast, dNorth, wPosition.z);

                                    // �����ڍ׃��X�g�����W���X�g�ɒǉ�                                                
                                    wSurfaceMembers.posList.push_back(pt);

                                    // �ꎞ���W���X�g������
                                    CPointBase wPosition{};
                                    posCnt = 0;
                                }

                            }
                        }
                    }
                    else {
                        continue;
                    }
                    // �����ڍ׃��X�g�ɒǉ�
                    roofSurfaces.roofSurfaceList.push_back(wSurfaceMembers);
                }
                // �������X�g�ɒǉ�
                buildingInfo.roofSurfaceList.push_back(roofSurfaces);
            }

        }

        // �������X�g�ɒǉ�
        allBuildingList.push_back(buildingInfo);

    }


    //xml�I�u�W�F�N�g���
    reader.Release();

    return allBuildingList;

}

// �W�v���f�[�^�擾
vector<AGTBUILDING> GetBldgAggregateData(wstring xmldata, int meshid, vector<JUDGESUITABLEPLACE> judge)
{
    //xml�I�u�W�F�N�g����
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpath���T�|�[�g����悤�ɐݒ�
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
    //namespace���T�|�[�g����悤�ɐݒ�
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE2));

    //���[�h
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    // �������X�g
    vector<AGTBUILDING> AgtBuildingList{};

    HRESULT hResult;

    // ���������J��Ԃ�
    for (int i = 0; i < judge.size(); i++) {
        // ������񏉊���
        AGTBUILDING buildingInfo{};

        // ����ID������
        MSXML2::IXMLDOMNodeListPtr buildingList = NULL;
        long lCountNode = 0;
        eCityGMLVersion version;
        for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version)
        {
            BSTR result = _bstr_t("");

            switch (version)
            {
            case eCityGMLVersion::VERSION_1:
            {
                const std::string bldgPath = "core:CityModel/core:cityObjectMember/bldg:Building/gen:stringAttribute/gen:value";
                string path = bldgPath + "[contains(text(),'" + judge[i].strBuildingId + "')]";
                wstring wide_string = wstring(path.begin(), path.end());
                result = _bstr_t(wide_string.c_str());
                break;
            }

            case eCityGMLVersion::VERSION_2:
            {
                const std::string bldgPath = "core:CityModel/core:cityObjectMember/bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute/uro:buildingID";
                string path = bldgPath + "[contains(text(),'" + judge[i].strBuildingId + "')]";
                wstring wide_string = wstring(path.begin(), path.end());
                result = _bstr_t(wide_string.c_str());
                break;
            }

            default:
                break;
            }

            // ����ID�w��̃m�[�h��I��
            BSTR xp2 = SysAllocString(result);
            reader->selectNodes(xp2, &buildingList);
            if (buildingList == NULL)   continue;

            // �m�[�h�����擾
            hResult = buildingList->get_length(&lCountNode);
            if (0 != lCountNode) {
                break;
            }

        }

        if (version == eCityGMLVersion::End)
        {   // �擾���s
            continue;
        }
                
        for (int j = 0; j < lCountNode; j++) {

            // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
            MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
            hResult = buildingList->get_item(j, &pXMLDOMNode);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                continue;
            }

            // ����ID���擾
            buildingInfo.strBuildingId = judge[i].strBuildingId;

            // �e�̐e�Ɉړ��@
            MSXML2::IXMLDOMNodePtr pPearentNode = NULL;
            hResult = pXMLDOMNode->get_parentNode(&pPearentNode);
            if (FAILED(hResult))
            {
                assert(!"�e�m�[�h�^�C�v�̎擾�Ɏ��s");
                continue;
            }
            hResult = pPearentNode->get_parentNode(&pXMLDOMNode);
            if (FAILED(hResult))
            {
                assert(!"�e�m�[�h�^�C�v�̎擾�Ɏ��s");
                continue;
            }
            if (version == eCityGMLVersion::VERSION_2)
            {
                MSXML2::IXMLDOMNodePtr pPearentNode2 = pXMLDOMNode;
                hResult = pPearentNode2->get_parentNode(&pXMLDOMNode);
                if (FAILED(hResult))
                {
                    assert(!"�e�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }
            }
            

            // �N�ԗ\�����˗�
            MSXML2::IXMLDOMNodePtr solorRadiation = 0;
            BSTR solor = SysAllocString(XPATH_aggregateData1);
            pXMLDOMNode->selectSingleNode(solor, &solorRadiation);
            if (NULL != solorRadiation)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr solorRadiationValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                solorRadiation->selectSingleNode(val, &solorRadiationValue);
                if (NULL != solorRadiationValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = solorRadiationValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = solorRadiationValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dSolorRadiation = stod(valueStr);

                    }
                }
            }

            // �N�ԗ\�����d��
            MSXML2::IXMLDOMNodePtr electricGeneration = 0;
            BSTR electric = SysAllocString(XPATH_aggregateData3);
            pXMLDOMNode->selectSingleNode(electric, &electricGeneration);
            if (NULL != electricGeneration)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr electricGenerationValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                electricGeneration->selectSingleNode(val, &electricGenerationValue);
                if (NULL != electricGenerationValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = electricGenerationValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = electricGenerationValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dElectricGeneration = stod(valueStr);

                    }
                }
            }

            // ���Q��������(�Ď�)
            MSXML2::IXMLDOMNodePtr lightPollutionSummer = 0;
            BSTR summer = SysAllocString(XPATH_aggregateData4);
            pXMLDOMNode->selectSingleNode(summer, &lightPollutionSummer);
            if (NULL != lightPollutionSummer)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr lightPollutionSummerValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionSummer->selectSingleNode(val, &lightPollutionSummerValue);
                if (NULL != lightPollutionSummerValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionSummerValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionSummerValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionSummer = stod(valueStr);

                    }
                }
            }

            // ���Q��������(�t��)
            MSXML2::IXMLDOMNodePtr lightPollutionSpling = 0;
            BSTR spling = SysAllocString(XPATH_aggregateData5);
            pXMLDOMNode->selectSingleNode(spling, &lightPollutionSpling);
            if (NULL != lightPollutionSpling)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr lightPollutionSplingValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionSpling->selectSingleNode(val, &lightPollutionSplingValue);
                if (NULL != lightPollutionSplingValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionSplingValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionSplingValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionSpling = stod(valueStr);

                    }
                }
            }

            // ���Q��������(�~��)
            MSXML2::IXMLDOMNodePtr lightPollutionWinter = 0;
            BSTR winter = SysAllocString(XPATH_aggregateData6);
            pXMLDOMNode->selectSingleNode(winter, &lightPollutionWinter);
            if (NULL != lightPollutionWinter)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr lightPollutionWinterValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionWinter->selectSingleNode(val, &lightPollutionWinterValue);
                if (NULL != lightPollutionWinterValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionWinterValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionWinterValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionWinter = stod(valueStr);

                    }
                }
            }

            // �������X�g�ɒǉ�
            AgtBuildingList.push_back(buildingInfo);
        }
    }


    //xml�I�u�W�F�N�g���
    reader.Release();
    //COM�̉��
    CoUninitialize();

    return AgtBuildingList;

}

// �W�v�f�[�^�쐬
vector<string> GetAggregateData(string folderPath, string csvfile){

    // CSV�t�@�C���ǂݍ���
    vector<vector<string> > data = csv2vector(csvfile, 1);
    // �K�n���茋�ʔz��
    vector<JUDGELIST> judgeList{};
    // �K�n���茋�ʔz��
    vector<JUDGESUITABLEPLACE> judgeRsultList{};
    // �K�n����
    JUDGELIST tmpJudgeList{};
    // �W�v����
    vector<string> result{};


    // �f�[�^�ݒ�
    int tmpMeshID = 0;
    for (int i = 0; i < data.size(); i++) {

        if (i == 0) {
            // ����͒l�ǉ�
            tmpJudgeList={};
            tmpMeshID = stoi(data[i][0]);
            vector<JUDGESUITABLEPLACE> judgeRsultList{};
        }
        else if (tmpMeshID != stoi(data[i][0])) {
            // ���b�V��ID���ύX���ꂽ�烊�X�g�ǉ�
            tmpJudgeList.meshID = tmpMeshID;
            tmpJudgeList.judgeSuitablePlaceList = judgeRsultList;
            judgeList.push_back(tmpJudgeList);
            JUDGELIST tmpJudgeList{};
            tmpMeshID = stoi(data[i][0]);
            judgeRsultList={};
        }

        if (data[i].size() == 14) {
            JUDGESUITABLEPLACE tmpJudge{};

            tmpJudge.strBuildingId = data[i][1];
            tmpJudge.priority = stoi(data[i][2]);
            tmpJudge.condition_1_1_1 = data[i][3];
            tmpJudge.condition_1_1_2 = data[i][4];
            tmpJudge.condition_1_2 = data[i][5];
            tmpJudge.condition_1_3 = data[i][6];
            tmpJudge.condition_2_1 = data[i][7];
            tmpJudge.condition_2_2 = data[i][8];
            tmpJudge.condition_2_3 = data[i][9];
            tmpJudge.condition_2_4 = data[i][10];
            tmpJudge.condition_3_1 = data[i][11];
            tmpJudge.condition_3_2 = data[i][12];
            tmpJudge.condition_3_3 = data[i][13];

            judgeRsultList.push_back(tmpJudge);

        }

    }
    // �Ō�̃f�[�^��ݒ�
    if (judgeRsultList.size() > 0) {
        tmpJudgeList.meshID = tmpMeshID;
        tmpJudgeList.judgeSuitablePlaceList = judgeRsultList;
        judgeList.push_back(tmpJudgeList);

    }

    // �f�[�^�擾
    std::vector<std::string> filePath;
    vector<AGTBLDGLIST> allBuiBldgAggregateList{};

    if (getFileNames(folderPath, extension_gml, filePath) == true) {

        // ���b�V��ID���ƂɏW�v���f�[�^�쐬
        for (int i = 0; i < judgeList.size(); i++) {
            // �t�@�C�����J��Ԃ�
            for (auto& p : filePath) {
                vector<AGTBUILDING> result{};
                AGTBLDGLIST wbldgList{};

                // �t�@�C�������烁�b�V��ID���擾
                std::string fullpath = p.c_str();
                int path_i = (int)fullpath.find_last_of("\\") + 1;
                int ext_i = (int)fullpath.find_last_of(".");
                std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
                int meshId = stoi(filename.substr(0, 8));

                // ���b�V��ID����v������f�[�^�쐬
                if (meshId == judgeList[i].meshID) {
                    // ���b�V��ID�Z�b�g
                    wbldgList.meshID = meshId;

                    //  Reader�̍쐬
                    std::wstring oWString = StringToWString(p.c_str());

                    // XML�f�[�^����Œ�XPath�̃f�[�^���擾
                    result = GetBldgAggregateData(oWString, meshId, judgeList[i].judgeSuitablePlaceList);

                    // �f�[�^���Ȃ��ꍇ�͎��̃t�@�C���ֈړ�
                    if (result.empty()) {
                        continue;
                    }
                    wbldgList.buildingList = result;

                    // �f�[�^�ǉ�
                    allBuiBldgAggregateList.push_back(wbldgList);

                    // ���̃f�[�^�֕ύX
                    break;
                }

            }

        }

    }

    // �͈͓�������
    int building = 0;
    // �N�ԗ\�����˗ʑ��v
    double solorRadiationTotal = 0.0;
    // �N�ԗ\�����d�ʑ��v
    double electricGenerationTotal = 0.0;
    // ���Q�𔭐������錚����
    int lightPollutionBuilding = 0;
    // ���Q�������ԑ��v�i�Ď��j
    int lightPollutionSummerTotal = 0;
    // ���Q�������ԑ��v�i�t���j
    int lightPollutionSplingTotal = 0;
    // ���Q�������ԑ��v�i�~���j
    int lightPollutionWinterTotal = 0;
    // �͈͓��D��x1������
    int priorityLevel1Count = 0;
    // �͈͓��D��x2������
    int priorityLevel2Count = 0;
    // �͈͓��D��x3������
    int priorityLevel3Count = 0;
    // �͈͓��D��x4������
    int priorityLevel4Count = 0;
    // �͈͓��D��x5������
    int priorityLevel5Count = 0;
 

    // �W�v
    if (allBuiBldgAggregateList.size() > 0) {
        // �w�b�_�[�ݒ�
        result.push_back(outputHeader);
        for (int i = 0; i < judgeList.size(); i++) {
            for (const auto& item : allBuiBldgAggregateList) {
            
                if (item.meshID == judgeList[i].meshID) {

                    for (const auto& item2 : item.buildingList) {
                        for (const auto& item3 : judgeList[i].judgeSuitablePlaceList) {
                            if (item2.strBuildingId == item3.strBuildingId) {
                                building++;
                                solorRadiationTotal = solorRadiationTotal + item2.dSolorRadiation;
                                electricGenerationTotal = electricGenerationTotal + item2.dElectricGeneration;
                                lightPollutionSummerTotal = lightPollutionSummerTotal + (int)item2.dLightPollutionSummer;
                                lightPollutionSplingTotal = lightPollutionSplingTotal + (int)item2.dLightPollutionSpling;
                                lightPollutionWinterTotal = lightPollutionWinterTotal + (int)item2.dLightPollutionWinter;
                                // ���Q������
                                if (item2.dLightPollutionSummer + item2.dLightPollutionSpling + item2.dLightPollutionWinter > 0) {
                                    lightPollutionBuilding++;
                                }
                                // �D��x
                                if (item3.priority == priorityLevel1) {
                                    priorityLevel1Count++;
                                }
                                else if (item3.priority == priorityLevel2) {
                                    priorityLevel2Count++;
                                }
                                else if (item3.priority == priorityLevel3) {
                                    priorityLevel3Count++;
                                }
                                else if (item3.priority == priorityLevel4) {
                                    priorityLevel4Count++;
                                }
                                else if (item3.priority == priorityLevel5) {
                                    priorityLevel5Count++;
                                }

                                // ��v�����甲����
                                break;
                            }

                        }
                    }
                }
            }
        }
        // �W�v�l�ݒ�
        std::ostringstream ss;
        ss << building << "," << solorRadiationTotal << "," << electricGenerationTotal << "," 
            << lightPollutionBuilding << "," << lightPollutionSummerTotal << "," << lightPollutionSplingTotal << "," 
            << lightPollutionWinterTotal << "," << priorityLevel1Count << "," << priorityLevel2Count << ","
            << priorityLevel3Count << "," << priorityLevel4Count << "," << priorityLevel5Count;
        std::string s = ss.str();
        result.push_back(s);
    }

    return result;

}

int __cdecl AggregateBldgFiles(const char* str, const char* strOut)
{
#ifdef _DEBUG
    std::cout << "AggregateBldgFiles Start " << std::endl;
    struct __timeb32 timebuffer;
    _ftime32_s(&timebuffer);
    double dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return 1;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return 1;
    }
    std::vector<std::string> fileName;

    allList.clear();

    // �L�����Z���t�@�C���p�X
    std::filesystem::path path = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path.string();

    //�t�@�C�����擾
    if (getFileNames(str, extension_gml, fileName) == true) {
    
        // �t�@�C�����J��Ԃ�
        for (auto& p : fileName) {
            vector<BUILDING> result{};
            BLDGLIST wbldgList{};

            // �t�@�C�������烁�b�V��ID���擾
            std::string fullpath = p.c_str();
            int path_i = (int)fullpath.find_last_of("\\") + 1;
            int ext_i = (int)fullpath.find_last_of(".");
            std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
            int meshId = stoi(filename.substr(0, 8));

            // ���b�V��ID�Z�b�g
            wbldgList.meshID = meshId;

            //  Reader�̍쐬
            std::wstring oWString = StringToWString(p.c_str());

            // XML�f�[�^����Œ�XPath�̃f�[�^���擾
            result = GetBldgAttribute(oWString);

            // �f�[�^���Ȃ��ꍇ�͎��̃t�@�C���ֈړ�
            if (result.empty()) {
                continue;
            }
            wbldgList.buildingList = result;

            // �f�[�^�ǉ�
            allList.push_back(wbldgList);

            // �L�����Z���t�@�C���`�F�b�N
            if (std::filesystem::exists(cancelPath)) {
                return 2;
            }
        }
    }
    else {
        return 1;
    }
#ifdef _DEBUG
    _ftime32_s(&timebuffer);
    double dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "AggregateBldgFiles Time: " << dEndStart << " sec" << std::endl;
    dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    return 0;
}

// �W�v����
int __cdecl AggregateAllData(const char* str, const char* strOut) {


    // �p�X�`�F�b�N
    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return 1;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return 1;
    }

    std::vector<std::string> filePath;

    std::vector<std::string> result{};

    // �L�����Z���t�@�C���p�X
    std::filesystem::path path = std::filesystem::path(strOut).parent_path() / CANCELFILE;
    std::string cancelPath = path.string();

    // �Q�ƃp�X�ݒ�
    std::filesystem::path path1 = std::filesystem::path(str) / "output" / "bldg" ;      // 3D�s�s�f�[�^
    std::filesystem::path path2 = std::filesystem::path(strOut).parent_path() / "data"; // �K�n����
    std::string bldgPath = path1.string();
    std::string csvPath = path2.string();

    // �t�@�C�����擾
    if (getFileNames(csvPath, extension_csv, filePath) == true) {
    
        if (filePath.size() > 0) {
            for (auto& p : filePath) {

                // �K�n����t�@�C��������Ώ��������s
                std::string strfile = p.c_str();
                std::filesystem::path filePath = strfile;
                if (filePath.filename() == judgeFile) {

                    result = GetAggregateData(bldgPath, p.c_str());

                    // csv�o�͂���
                    // �t�@�C�����ɓ��t������t�^
                    string dateStr = getDatetimeStr();
                    std::filesystem::path outPath = std::filesystem::path(strOut) / outputFile;
                    ofstream ofs(outPath);
                    for (int i = 0; i < result.size(); i++) {
                        ofs << result[i] << endl;
                    }
                }

                // �L�����Z���t�@�C���`�F�b�N
                if (std::filesystem::exists(cancelPath)) {
                    return 2;
                }
            }
        }
        else {
            return 1;
        }
    }
    else {
        return 1;
    }

    return 0;
}