/******************************************************************************
 * $Id: AWXDataset.cpp 34610 2018-01-04 11:45:38Z liml $
 *
 * Project:  GDAL
 * Purpose:  Implements Advanced Weather-satellite eXchange format.
 * Author:   Li Minlu, liminlu0314@163.com
 *
 ******************************************************************************
 * Copyright (c) 2018, Li Minlu, liminlu0314@163.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

 //���ڽ����������Ƿַ���Ʒ���ݸ�ʽ����ʽ��ϸ˵���ο�
 //���ģ�http://satellite.nsmc.org.cn/PortalSite/StaticContent/FileDownload.aspx?CategoryID=1&LinkID=86
 //Ӣ�ģ�http://satellite.nsmc.org.cn/PortalSite/StaticContent/FileDownload.aspx?CategoryID=1&LinkID=28

#include "cpl_conv.h"
#include "cpl_string.h"
#include "gdal_frmts.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "rawdataset.h"

CPL_C_START
void CPL_DLL GDALRegister_AWX(void);
CPL_C_END

typedef struct {
	char fileName[12];          //1-12   Sat96�ļ���
	short byteOrder;            //13_14  ���������ֽ�˳��
	short sizeHead1;            //15-16  ��һ���ļ�ͷ����
	short sizeHead2;            //17-18  �ڶ����ļ�ͷ����
	short sizeFilled;           //19-20  �������ݳ���
	short sizeRecord;           //21-22  ��¼����
	short numHead;              //23-24   �ļ�ͷռ�ü�¼��
	short numData;              //25-26   ��Ʒ����ռ�ü�¼��
	short typeProduct;          //27-28   ��Ʒ���
	short compressMode;         //29-30   ѹ����ʽ
	char fmtFlag[8];            //31-38   ��ʽ˵���ַ���
	short qualityFlag;          //39-40   ��Ʒ�����������
}AWX_HEAD1; //��һ���ļ�ͷ

typedef struct {
	char satName[8];        //41-48  ������
	short year;             //49-50   ʱ�䣨�꣩
	short month;            //51-52   ʱ�䣨�£�
	short day;              //53-54   ʱ�䣨�գ�
	short hour;             //55-56   ʱ�䣨ʱ��
	short minute;           //57-58   ʱ�䣨�֣�
	short channel;          //59-60   ͨ����
	short projection;       //61-62   ͶӰ��ʽ
	short width;            //63-64   ͼ�ο��
	short height;           //65-66   ͼ��߶�
	short leftUpLine;       //67-68   ͼ�����Ͻ�ɨ���ߺ�
	short leftUpPixel;      //69-70   ͼ�����Ͻ���Ԫ��
	short sample;           //71-72   ������
	short boundN;           //73-74   ����Χ����γ��
	short boundS;           //75-76   ����Χ����γ��
	short boundW;           //77-78   ����Χ��������
	short boundE;           //79-80   ����Χ��������
	short centerLat;        //81-82   ͶӰ����γ�ȣ���*100��
	short centerLon;        //83-84   ͶӰ���ľ��ȣ���*100��
	short standard1;        //85-86   ͶӰ��׼γ��1�����׼���ȣ�����*100��
	short standard2;        //87-88   ��׼ͶӰγ��2
	short resolutionH;      //89-90   ͶӰˮƽ�ֱ���
	short resolutionV;      //91-92   ͶӰ��ֱ�ֱ���
	short gridflag;         //93-94   ����������ӱ�־
	short gridvalue;        //95-96   �����������ֵ
	short sizePalette;      //97-98   ��ɫ�����ݳ���
	short sizeCalibration;  //99-100  �������ݿ鳤��
	short sizeNavigation;   //101-102 ��λ���ݿ鳤��
	short reserved;         //103-104 ����
}AWX_HEAD2_P1; //��ֹ��������ͼ���Ʒ�ڶ����ļ�ͷ��¼��ʽ

typedef struct {
	char satName[8];            //41-48    ������
	short bYear;                //49-50    ��ʼʱ�䣨�꣩
	short bMonth;               //51-52    ��ʼʱ��(��) 
	short bDay;                 //53-54    ��ʼʱ�䣨�գ�
	short bHour;                //55-56    ��ʼʱ��(ʱ)
	short bMinute;              //57-58    ��ʼʱ�䣨�֣�
	short eYear;                //59-60    ����ʱ�䣨�꣩
	short eMonth;               //61-62    ����ʱ�䣨�£�
	short eDay;                 //63-64    ����ʱ�䣨�գ�
	short eHour;                //65-66    ����ʱ�䣨ʱ��
	short eMinute;              //67-68    ����ʱ�䣨�֣�
	short channel;              //69-70    ͨ����
	short channelR;             //71-72    Rͨ����
	short channelG;             //73-74    Gͨ���� 
	short channelB;             //75-76    Bͨ����
	short orbitFlag;            //77-78    �������־
	short orbitNum;             //79-80    �����  
	short sizeElement;          //81-82    һ����Ԫռ�ֽ���
	short projection;           //83-84    ͶӰ��ʽ   
	short productType;          //85-86    ��Ʒ����    
	short width;                //87-88    ͼ����   
	short height;               //89-90    ͼ��߶�    
	short leftUpLine;           //91-92    ͼ�����Ͻ�ɨ���ߺ�
	short leftUpPixel;          //93-94    ͼ�����Ͻ���Ԫ�� 
	short sample;               //95-96    ������  
	short boundN;               //97-98    ����Χ����γ��
	short boundS;               //99-100   ����Χ����γ��
	short boundW;               //101-102  ����Χ��������
	short boundE;               //103-104  ����Χ��������
	short centerLat;            //105-106  ͶӰ����γ�� ��*100 
	short centerLon;            //107-108  ͶӰ���ľ��� ��*100  
	short standard1;            //109-110  ͶӰ��׼γ��1�����׼���ȣ� ��*100 
	short standard2;            //111-112  ��׼ͶӰγ��2  
	short resolutionH;          //113-114  ͶӰˮƽ�ֱ��� 
	short resolutionV;          //115-116  ͶӰ���ͷֱ���   
	short gridFlag;             //117-118  ����������ӱ�־  
	short gridValue;            //119-120  �����������ֵ    
	short sizePalette;          //121-122  ��ɫ�����ݿ鳤�� 
	short sizeCalibration;      //123-124 �������ݿ鳤��
	short sizeNavigation;       //125-126 ��λ���ݿ鳤�� 
	short reserved;             //127-128 ����
}AWX_HEAD2_P2; //������������ͼ���Ʒ�ڶ����ļ�ͷ��¼��ʽ

typedef struct {
	char satName[8];        //41-48  ������
	short element;          //49-50  ��㳡Ҫ��
	short sizeElement;      //51-52  ��������ֽ�
	short valueBase;        //53-54  ������ݻ�׼ֵ
	short valueRate;        //55-56  ������ݱ�������
	short timeRange;        //57-58  ʱ�䷶Χ����
	short bYear;            //59-60  ��ʼ��
	short bMonth;           //61-62  ��ʼ��
	short bDay;             //63-64  ��ʼ��
	short bHour;            //65-66  ��ʼʱ
	short bMinute;          //67-68  ��ʼ��
	short eYear;            //69-70  ������
	short eMonth;           //71-72  ������
	short eDay;             //73-74  ������
	short eHour;            //75-76  ����ʱ
	short eMinute;          //77-78  ������
	short latLeftUp;        //79-80  �������Ͻ�γ��
	short lonLeftUp;        //81-82  �������ϽǾ���
	short latRightDown;     //83-84  �������½�γ��
	short lonRightDown;     //85-86  �������½Ǿ���
	short gridUnit;         //87-88  ��൥λ
	short gridSpaceH;       //89-90  ������
	short gridSpaceV;       //91-92  ������
	short gridCountH;       //93-94  ��������
	short gridCountV;       //95-96  ��������
	short landFlag;         //97-98  ����½������ֵ
	short landValue;        //99-100 ½�ؾ�������ֵ
	short cloudFlag;        //101-102 ����������ֵ
	short cloudValue;       //103-104 �ƾ�������ֵ
	short waterFlag;        //105-106 ����ˮ������ֵ 
	short waterValue;       //107-108 ˮ���������ֵ
	short iceFlag;          //109-110 ���ޱ�������ֵ
	short iceValue;         //111-112 �����������ֵ
	short flagQuality;      //113-114 �Ƿ�����������ֵ
	short qualityUp;        //115-116  ��������ֵ����
	short qualityDown;      //117-118  ��������ֵ����
	short reserved;         //119-120  ����
}AWX_HEAD2_P3;  //��㳡������Ʒ��ʽ

typedef struct {
	char satName[8];        //41-48   ������
	short element;          //49-50   Ҫ��
	short sizeElement;		//51-52   ÿ����¼���ٸ���
	short countElement;     //53-54   ̽�������
	short bYear;            //55-56   ��ʼ��
	short bMonth;           //57-58   ��ʼ��
	short bDay;             //59-60   ��ʼ��
	short bHour;            //61-62   ��ʼʱ
	short bMinute;          //63-64   ��ʼ��
	short eYear;            //65-66   ������
	short eMonth;           //67-68   ������
	short eDay;             //69-70   ������
	short eHour;            //71-72   ����ʱ
	short eMinute;          //73-74   ������
	short algorithm;        //75-76   ���ݷ�������
	short initType;         //77-78   ����������
	short reserved;         //79-80   ȱʡֵ
}AWX_HEAD2_P4;  //��ɢ���ڶ����ļ�ͷ��¼��ʽ

typedef struct {
	short nProbeLatitude;		//1-2  ̽����γ��
	short nProbeLongitude;		//3_4  ̽���ľ���
	short nProbeElevation;
	short nSurPressure;
	short nClearSkyMark;
	short GH15SL[15];
	short AT15SL[15];
	short ADPT6SL[6];
	short GWS9SL[9];
	short nAtmStabIndex;
	short nAtmColTOC;
	short CSACTWVC;
	short OLR;
	short nCloudTopTemperature;
	short nCloudTopPressure;
	short nCloudiness;
	short nVisibleChannelAlbedo;
	short LI500H;
	short nLocalZenith;
	short nSolarZenith;
	short nIET10SL[10];
	short nIEDPT5SL[5];
	short nBTF19H2C[19];
	short BTF4MC[4];
	char cReserved[24];
} AWX_ATOVS;  //��������ATOVS��ɢ�����ݸ�ʽ

typedef struct {
	short nProbeLatitude;		//1-2  ̽����γ��
	short nProbeLongitude;		//3_4  ̽���ľ���
	short nProbeLevels;			//5-6  ̽����
	short nWindDirection;		//7-8  ����
	short nWindVelocity;		//9-10 ����
	short nUnkonwn;				//11-12 //TODO ˵���ĵ���û������
	short nTemperature;			//13-14 �¶�
	char cReserved[26];			//15-40 �ڲ�����
} AWX_GDCMW;  //��ֹ�����Ƽ�����ɢ�����ݸ�ʽ

enum AWXProduct  // Product
{
	AS_UNKNOWN = 0,				//δ�������͵Ĳ�Ʒ
	AS_GEOSTATIONARY = 1,       // ��ֹ��������ͼ���Ʒ
	AS_POLAR_ORBITING = 2,      // ������������ͼ���Ʒ
	AS_GRID_FIELD = 3,			// ��㳡������Ʒ
	AS_DISCRETE_FIELD = 4,      // ��ɢ��������Ʒ
	AS_GRAPHICAL_ANALYSIS = 5   // ͼ�κͷ�����Ʒ
};

/************************************************************************/
/* ==================================================================== */
/*                             AWXDataset                               */
/* ==================================================================== */
/************************************************************************/

class AWXRasterBand;
class OGRAWXLayer;

class CPL_DLL AWXDataset : public GDALPamDataset
{
	friend class AWXRasterBand;

	double       adfGeoTransform[6];
	char        *pszProjection;

	VSILFILE    *fp;
	const char  *pszFilename;
	AWXProduct iProduct;
	bool bRaster;

	bool bHaveOffsetScale;
	double dfScale;
	double dfOffset;

	GDALDataType eDataType;
	GDALColorTable *poColorTable;

	std::vector<OGRAWXLayer*> apoLayers;

	void ParseSRS(int nType, int nS1, int nS2);

	void ParseColorTable(GByte *pData, int nSize);
	void ParseCalibration(GByte *pData, int nSize);
	void ParseNavigation(GByte *pData, int nSize);

	void ParseGeotransform(double dfULX, double dfULY,
		double dfURX, double dfURY,
		double dfLLX, double dfLLY,
		double dfLRX, double dfLRY);

	bool OpenP1();
	bool OpenP2();
	bool OpenP3();
	bool OpenP4(vsi_l_offset nHeaderSize);
	bool OpenP5();

	void GetHeaderSize(vsi_l_offset &nHeaderSize);

public:
	AWXDataset();
	~AWXDataset() override;

	static GDALDataset *Open(GDALOpenInfo *);

	CPLErr      GetGeoTransform(double *) override;
	const char  *GetProjectionRef() override;

	virtual int      GetLayerCount() override { return (int)apoLayers.size(); }
	virtual OGRLayer *GetLayer(int) override;
};

/************************************************************************/
/* ==================================================================== */
/*                           AWXRasterBand                              */
/* ==================================================================== */
/************************************************************************/

class AWXRasterBand : public RawRasterBand
{
	friend class AWXDataset;

public:

	AWXRasterBand(AWXDataset *, int, VSILFILE *, vsi_l_offset,
		int, int, GDALDataType, int, GDALColorTable *);
};

/************************************************************************/
/*                             OGRAWXLayer                              */
/************************************************************************/

class OGRAWXLayer : public OGRLayer
{
	OGRFeatureDefn     *poFeatureDefn;
	VSILFILE           *fp;
	AWX_HEAD2_P4       *pHeader2;
	vsi_l_offset        nHeaderSize;
	bool			    bATOVS;

	OGRSpatialReference *poSRS;
	int                 nNextFID;

public:
	OGRAWXLayer(VSILFILE *, vsi_l_offset, AWX_HEAD2_P4 *, const char*, bool);
	virtual ~OGRAWXLayer();

	void                ResetReading() override;
	OGRFeature *        GetNextFeature() override;
	OGRFeature *        GetAtovsNextFeature();
	OGRFeature *        GetGdcmwNextFeature();

	OGRFeatureDefn *    GetLayerDefn() override { return poFeatureDefn; }
	int                 TestCapability(const char *) { return FALSE; }
};

/************************************************************************/
/*                       AWXRasterBand()                                */
/************************************************************************/

AWXRasterBand::AWXRasterBand(AWXDataset *poDSIn, int nBandIn, VSILFILE * fpRawIn,
	vsi_l_offset nImgOffsetIn, int nPixelOffsetIn,
	int nLineOffsetIn,
	GDALDataType eDataTypeIn, int bNativeOrderIn, GDALColorTable *poColorTable) :
	RawRasterBand(poDSIn, nBandIn, fpRawIn, nImgOffsetIn, nPixelOffsetIn,
		nLineOffsetIn, eDataTypeIn, bNativeOrderIn, RawRasterBand::OwnFP::NO)
{
	if (poColorTable != NULL)
	{
		SetColorTable(poColorTable);
		SetColorInterpretation(GCI_PaletteIndex);
	}
}

/************************************************************************/
/*                        OGRAWXLayer()                                 */
/************************************************************************/
OGRAWXLayer::OGRAWXLayer(VSILFILE *pFp, vsi_l_offset nHeaderByte,
	AWX_HEAD2_P4 *pHeader, const char* pszLayerName, bool bATOVS)
	:fp(pFp),
	nHeaderSize(nHeaderByte),
	pHeader2(pHeader)
{
	this->poSRS = NULL;
	this->nNextFID = 0;
	this->bATOVS = bATOVS;

	poFeatureDefn = new OGRFeatureDefn(pszLayerName);
	SetDescription(poFeatureDefn->GetName());
	poFeatureDefn->Reference();

	poFeatureDefn->SetGeomType(wkbPoint25D);

	std::string strATOVSField[] = {
		"ProbePointLongitude",
		"ProbePointLatitude",
		"ProbePointElevation",
		"ProbePointSurfacePressure",
		"ProbePointClearSkyMark",
		"GH15SL",
		"AT15SL",
		"ADPT6SL",
		"GWS9SL",
		"AtmStabIndex",
		"AtmColTOC",
		"CSACTWVC",
		"OutgoingLongwaveRadiation",
		"CloudTopPressure",
		"CloudTopTemperature",
		"Cloudiness",
		"VisibleChannelAlbedo",
		"500HLI",
		"LocalZenith",
		"SolarZenith",
		"IET10SL",
		"IEDPT5SL",
		"BTF19H2C",
		"BTF4MC",
		"Reserved"
	};

	std::string strGDCMWField[7] = {
		"ProbePointLongitude",
		"ProbePointLatitude",
		"ProbeLevels",
		"WindDirection",
		"WindVeloctity",
		"Temperature",
		"Reserved"
	};

	std::string *pstrField = strGDCMWField;
	int nFieldCount = 7;
	if (bATOVS)
	{
		pstrField = strATOVSField;
		nFieldCount = 24;
	}

	/* -------------------------------------------------------------------- */
	/*      Build field definitions.                                        */
	/* -------------------------------------------------------------------- */
	for (int iField = 0; iField < nFieldCount; iField++)
	{
		OGRFieldDefn oField(pstrField[iField].c_str(), OFTReal);
		poFeatureDefn->AddFieldDefn(&oField);
	}

	poSRS = new OGRSpatialReference();
	poSRS->SetWellKnownGeogCS("WGS84");

	if (poFeatureDefn->GetGeomFieldCount() > 0)
		poFeatureDefn->GetGeomFieldDefn(0)->SetSpatialRef(poSRS);

	ResetReading();
}

OGRAWXLayer::~OGRAWXLayer()
{
	poFeatureDefn->Release();

	if (poSRS)
		poSRS->Release();
}

/************************************************************************/
/*                            ResetReading()                            */
/************************************************************************/

void OGRAWXLayer::ResetReading()
{
	VSIFSeekL(fp, nHeaderSize, SEEK_SET);
	nNextFID = 0;
}

/************************************************************************/
/*                           GetNextFeature()                           */
/************************************************************************/

OGRFeature *OGRAWXLayer::GetNextFeature()
{
	if (bATOVS)
		return GetAtovsNextFeature();
	else
		return GetGdcmwNextFeature();
}

inline bool AllZero(GByte *pData, int nLength)
{
	for (int i = 0; i < nLength; i++)
	{
		if (pData[i] != 0x00)
			return false;
	}

	return true;
}

OGRFeature *OGRAWXLayer::GetAtovsNextFeature()
{
	while (true)
	{
		AWX_ATOVS *pPoint = static_cast<AWX_ATOVS *>(CPLMalloc(sizeof(AWX_ATOVS)));
		int nBytesRead = VSIFReadL(pPoint, 1, sizeof(AWX_ATOVS), this->fp);
		if (nBytesRead < sizeof(AWX_ATOVS))
		{
			CPLDebug("AWX", "AWX_ATOVS file too short. Reading failed");
			CPLFree(pPoint);
			return NULL;
		}

		if (AllZero((GByte*)pPoint, sizeof(AWX_ATOVS)))
		{
			CPLDebug("AWX", "End of AWX_ATOVS record");
			CPLFree(pPoint);
			return NULL;
		}

		double dfX = pPoint->nProbeLongitude * 0.01;
		double dfY = pPoint->nProbeLatitude * 0.01;
		double dfZ = pPoint->nProbeElevation;

		if (dfX > 180)
			dfX -= 360;

		OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
		poFeature->SetGeometryDirectly(new OGRPoint(dfX, dfY, dfZ));

		poFeature->SetField(0, dfX);
		poFeature->SetField(1, dfY);
		poFeature->SetField(2, dfZ);
		poFeature->SetField(3, pPoint->nSurPressure);
		poFeature->SetField(4, pPoint->nClearSkyMark);

		//poFeature->SetField(5, pPoint->nTemperature);
		//poFeature->SetField(6, pPoint->nWindDirection);
		//poFeature->SetField(7, pPoint->nWindVelocity);
		//poFeature->SetField(8, pPoint->nTemperature);

		poFeature->SetField(9, pPoint->nAtmStabIndex * 0.01);
		poFeature->SetField(10, pPoint->nAtmColTOC*0.015625);
		poFeature->SetField(11, pPoint->CSACTWVC * 0.01);
		poFeature->SetField(12, pPoint->OLR*0.015625);
		poFeature->SetField(13, pPoint->nCloudTopPressure);
		poFeature->SetField(14, pPoint->nCloudTopTemperature*0.015625);
		poFeature->SetField(15, pPoint->nCloudiness);
		poFeature->SetField(16, pPoint->nVisibleChannelAlbedo * 0.01);
		poFeature->SetField(17, pPoint->LI500H * 0.01);
		poFeature->SetField(18, pPoint->nLocalZenith);
		poFeature->SetField(19, pPoint->nSolarZenith);
		//poFeature->SetField(20, pPoint->nAtmColTOC);
		//poFeature->SetField(21, pPoint->nAtmColTOC);
		//poFeature->SetField(22, pPoint->nAtmColTOC);
		//poFeature->SetField(23, pPoint->nAtmColTOC);
		//poFeature->SetField(24, pPoint->nAtmColTOC);

		poFeature->SetFID(nNextFID++);

		CPLFree(pPoint);

		if ((m_poFilterGeom == NULL || FilterGeometry(poFeature->GetGeometryRef())) &&
			(m_poAttrQuery == NULL || m_poAttrQuery->Evaluate(poFeature)))
			return poFeature;

		delete poFeature;
	}
}

OGRFeature *OGRAWXLayer::GetGdcmwNextFeature()
{
	while (true)
	{
		AWX_GDCMW *pPoint = static_cast<AWX_GDCMW *>(CPLMalloc(sizeof(AWX_GDCMW)));
		int nBytesRead = VSIFReadL(pPoint, 1, sizeof(AWX_GDCMW), this->fp);
		if (nBytesRead < sizeof(AWX_GDCMW))
		{
			CPLDebug("AWX", "AWX_GDCMW file too short. Reading failed");
			CPLFree(pPoint);
			return NULL;
		}

		if (AllZero((GByte*)pPoint, sizeof(AWX_GDCMW)))
		{
			CPLDebug("AWX", "End of AWX_GDCMW record");
			CPLFree(pPoint);
			return NULL;
		}

		double dfX = pPoint->nProbeLongitude * 0.01;
		double dfY = pPoint->nProbeLatitude * 0.01;
		double dfZ = pPoint->nProbeLevels;

		if (dfX > 180)
			dfX -= 360;

		OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
		poFeature->SetGeometryDirectly(new OGRPoint(dfX, dfY, dfZ));

		poFeature->SetField(0, dfX);
		poFeature->SetField(1, dfY);
		poFeature->SetField(2, dfZ);
		poFeature->SetField(3, pPoint->nWindDirection);
		poFeature->SetField(4, pPoint->nWindVelocity);
		poFeature->SetField(5, pPoint->nTemperature);
		poFeature->SetFID(nNextFID++);

		CPLFree(pPoint);

		if ((m_poFilterGeom == NULL || FilterGeometry(poFeature->GetGeometryRef())) &&
			(m_poAttrQuery == NULL || m_poAttrQuery->Evaluate(poFeature)))
			return poFeature;

		delete poFeature;
	}
}

/************************************************************************/
/*                            AWXDataset()                              */
/************************************************************************/

AWXDataset::AWXDataset() :
	pszProjection(CPLStrdup("")),
	fp(NULL),
	pszFilename(NULL),
	bRaster(true),
	bHaveOffsetScale(false),
	dfOffset(0.0),
	dfScale(1.0),
	eDataType(GDT_Unknown),
	iProduct(AS_UNKNOWN),
	poColorTable(NULL)
{
	adfGeoTransform[0] = 0.0;
	adfGeoTransform[1] = 1.0;
	adfGeoTransform[2] = 0.0;
	adfGeoTransform[3] = 0.0;
	adfGeoTransform[4] = 0.0;
	adfGeoTransform[5] = 1.0;
	nBands = 0;
	apoLayers.clear();
}

/************************************************************************/
/*                           ~AWXDataset()                              */
/************************************************************************/

AWXDataset::~AWXDataset()

{
	FlushCache();

	while (!apoLayers.empty())
	{
		delete apoLayers.back();
		apoLayers.pop_back();
	}

	if (fp != NULL)
	{
		if (VSIFCloseL(fp) != 0)
		{
			CPLError(CE_Failure, CPLE_FileIO, "I/O error");
		}
	}

	if (poColorTable != NULL)
		delete poColorTable;

	CPLFree(pszProjection);
}

OGRLayer *AWXDataset::GetLayer(int iLayer)
{
	if (iLayer < 0 || iLayer >= apoLayers.size())
		return NULL;
	return apoLayers[iLayer];
}

/************************************************************************/
/*                          GetGeoTransform()                           */
/************************************************************************/

CPLErr AWXDataset::GetGeoTransform(double * padfTransform)
{
	memcpy(padfTransform, adfGeoTransform, sizeof(double) * 6);
	return CE_None;
}

/************************************************************************/
/*                          GetProjectionRef()                          */
/************************************************************************/

const char *AWXDataset::GetProjectionRef()
{
	if (pszProjection)
		return pszProjection;

	return "";
}

inline std::string ToString(int nValue)
{
	char szTmp[32] = { 0 };
	sprintf(szTmp, "%d", nValue);
	return std::string(szTmp);
}

inline std::string ToString(double dfValue)
{
	char szTmp[32] = { 0 };
	sprintf(szTmp, "%.6f", dfValue);
	return std::string(szTmp);
}

inline std::string ProductToStirng(AWXProduct iProduct)
{
	std::string strId = ToString((short)iProduct) + ": ";
	switch (iProduct)
	{
	case AS_UNKNOWN:
		return strId + "Undefined product type";
		break;
	case AS_GEOSTATIONARY:
		return strId + "Geostationary imagery products";
		break;
	case AS_POLAR_ORBITING:
		return strId + "Polar-orbiting imagery products";
		break;
	case AS_GRID_FIELD:
		return strId + "Grid field quantitative products";
		break;
	case AS_DISCRETE_FIELD:
		return strId + "Discrete field quantitative products";
		break;
	case AS_GRAPHICAL_ANALYSIS:
		return strId + "Graphical analysis products";
		break;
	default:
		return strId;
		break;
	}
}

inline std::string CompressToString(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	std::string strString[4] = {
		"Uncompressed",
		"Compressed length codes",
		"LZW compression",
		"Special compression mode"
	};

	if (nValue >= 0 && nValue <= 3)
		return strId + strString[nValue];
	else
		return strId;
}

inline std::string QualityToString(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	std::string strString[6] = {
		"Without quality control.",
		"(<1e-6,<0.05%)Reliable data quality without missing values or errors.",
		"(1e-4~1e-6,0.05%~0.2%)Basically reliable quality, with the missing values or errors within the allowable range. ",
		"(1e-3~1e-4,0.2%~2%)With missing values or errors, though still usable.",
		"(1e-2~1e-3,2%~20%)With missing values or errors, literally unusable, though can serve as reference. ",
		"(>1e-2,>20%)Unreliable data, unusable."
	};

	if (nValue >= 0 && nValue <= 5)
		return strId + strString[nValue];
	else
		return strId;
}

inline std::string ChannelToString(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	if (nValue >= 6 && nValue <= 100)
		return strId + "Reserved";

	std::string strString[5] = {
		"Infrared channel (10.3-11.3)",
		"Water vapor channel (6.3-7.6)",
		"IR split window channel (11.5-12.5)",
		"Visible channel (0.5-0.9)",
		"Mid-infrared channel (3.5-4.0)"
	};

	if (nValue >= 1 && nValue <= 5)
		return strId + strString[nValue - 1];
	else
		return strId;
}

inline std::string ChannelToString2(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	if (nValue == 0)
		return strId + "RGB";
	if (nValue >= 6 && nValue <= 100)
		return strId + "Reserved";
	if (nValue >= 120 && nValue <= 200)
		return strId + "Reserved";
	if (nValue >= 250)
		return strId + "Reserved";
	if (nValue >= 101 && nValue <= 119)
		return strId + "TOVS HIRS channel";
	if (nValue >= 201 && nValue <= 204)
		return strId + "TOVS MSU channel";

	std::string strString[5] = {
		"1 channel",
		"2 channel",
		"3 channel",
		"4 channel",
		"5 channel"
	};

	if (nValue >= 1 && nValue <= 5)
		return strId + strString[nValue - 1];
	else
		return strId;
}

inline std::string ProjectionToString(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	std::string strString[5] = {
		"Lambert projection",
		"Mercator projection",
		"Stereographic projection",
		"Latitude/longitude projection",
		"Equal area projection"
	};

	if (nValue >= 1 && nValue <= 5)
		return strId + strString[nValue - 1];
	else
		return strId + "Without projection (satellite projection)";
}

inline std::string ProductToString(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	if (nValue >= 14 && nValue <= 99)
		return strId + "Reserved";

	if (nValue >= 100)
		return strId + "TOVS imageries";

	std::string strString[14] = {
		"Average imagery",
		"Fire",
		"Water",
		"Drought",
		"Snow",
		"Vegetation",
		"Ice",
		"Sea surface temperature ",
		"Surface temperature",
		"Cloud height",
		"Soil moisture",
		"Estuarine sediment ",
		"Urban Heat Island",
		"Ocean color"
	};

	if (nValue >= 0 && nValue <= 13)
		return strId + strString[nValue];
	else
		return strId;
}

void AWXDataset::ParseSRS(int nType, int nS1, int nS2)
{
	if (nType == 0 || nType > 5)	//δͶӰ
		return;

	OGRSpatialReference oSRS;

	//TODO ��Ҫ�Ը���ͶӰ��ʽ����ȷ��

	if (nType == 1)
		oSRS.SetLCC(nS1, nS2, 0, 0, 0, 0);//Lambert 
	else if (nType == 2)
		oSRS.SetMercator(nS1, 0, 1, 0, 0);//Mercator
	else if (nType == 3)
		oSRS.SetPS(0, nS1, 1, 0, 0);//����ͶӰ
	else if (nType == 4)
		oSRS.SetWellKnownGeogCS("WGS84");//�Ⱦ�γ��ͶӰ
	else if (nType == 5)//�����ͶӰ
	{
	}

	if (this->pszProjection)
		CPLFree(this->pszProjection);

	OGRErr eErr = oSRS.exportToWkt(&this->pszProjection);
	if (eErr != OGRERR_NONE)
		CPLDebug("AWX", "Export projection to WKT failed: %d", eErr);
}

void AWXDataset::ParseColorTable(GByte *pData, int nSize)
{
	poColorTable = new GDALColorTable;
	for (int n = 0; n < 256; n++)
	{
		GDALColorEntry oCE;
		oCE.c1 = pData[n];
		oCE.c2 = pData[n + 256];
		oCE.c3 = pData[n + 512];
		oCE.c4 = 255;
		poColorTable->SetColorEntry(n, &oCE);
	}
}

void AWXDataset::ParseCalibration(GByte *pData, int nSize)
{
	//TODO �������ݿ�
}

void AWXDataset::ParseNavigation(GByte *pData, int nSize)
{
	//TODO ��λ���ݿ�
}

void AWXDataset::ParseGeotransform(double dfULX, double dfULY,
	double dfURX, double dfURY,
	double dfLLX, double dfLLY,
	double dfLRX, double dfLRY)
{
	// Generate GCPs
	GDAL_GCP *pasGCPList = static_cast<GDAL_GCP *>(CPLCalloc(sizeof(GDAL_GCP), 4));
	GDALInitGCPs(4, pasGCPList);
	CPLFree(pasGCPList[0].pszId);
	CPLFree(pasGCPList[1].pszId);
	CPLFree(pasGCPList[2].pszId);
	CPLFree(pasGCPList[3].pszId);

	/* Let's order the GCP in TL, TR, BR, BL order to benefit from the */
	/* GDALGCPsToGeoTransform optimization */
	pasGCPList[0].pszId = CPLStrdup("UPPER_LEFT");
	pasGCPList[0].dfGCPX = dfULX;
	pasGCPList[0].dfGCPY = dfULY;
	pasGCPList[0].dfGCPZ = 0.0;
	pasGCPList[0].dfGCPPixel = 0.5;
	pasGCPList[0].dfGCPLine = 0.5;
	pasGCPList[1].pszId = CPLStrdup("UPPER_RIGHT");
	pasGCPList[1].dfGCPX = dfURX;
	pasGCPList[1].dfGCPY = dfURY;
	pasGCPList[1].dfGCPZ = 0.0;
	pasGCPList[1].dfGCPPixel = this->nRasterXSize - 0.5;
	pasGCPList[1].dfGCPLine = 0.5;
	pasGCPList[2].pszId = CPLStrdup("LOWER_RIGHT");
	pasGCPList[2].dfGCPX = dfLRX;
	pasGCPList[2].dfGCPY = dfLRY;
	pasGCPList[2].dfGCPZ = 0.0;
	pasGCPList[2].dfGCPPixel = this->nRasterXSize - 0.5;
	pasGCPList[2].dfGCPLine = this->nRasterYSize - 0.5;
	pasGCPList[3].pszId = CPLStrdup("LOWER_LEFT");
	pasGCPList[3].dfGCPX = dfLLX;
	pasGCPList[3].dfGCPY = dfLLY;
	pasGCPList[3].dfGCPZ = 0.0;
	pasGCPList[3].dfGCPPixel = 0.5;
	pasGCPList[3].dfGCPLine = this->nRasterYSize - 0.5;

	// Calculate transformation matrix, if accurate
	const bool transform_ok
		= CPL_TO_BOOL(GDALGCPsToGeoTransform(4, pasGCPList, this->adfGeoTransform, 0));

	if (!transform_ok)
	{
		this->adfGeoTransform[0] = 0.0;
		this->adfGeoTransform[1] = 1.0;
		this->adfGeoTransform[2] = 0.0;
		this->adfGeoTransform[3] = 0.0;
		this->adfGeoTransform[4] = 0.0;
		this->adfGeoTransform[5] = 1.0;
		if (this->pszProjection)
			CPLFree(this->pszProjection);
		this->pszProjection = CPLStrdup("");
	}

	GDALDeinitGCPs(4, pasGCPList);
		CPLFree(pasGCPList);
}

bool AWXDataset::OpenP1()
{
	AWX_HEAD2_P1 *pHeader2P1 = static_cast<AWX_HEAD2_P1 *>(CPLMalloc(sizeof(AWX_HEAD2_P1)));
	int nBytesRead = VSIFReadL(pHeader2P1, 1, sizeof(AWX_HEAD2_P1), this->fp);
	if (nBytesRead < sizeof(AWX_HEAD2_P1))
	{
		CPLDebug("AWX", "Header2P1 file too short. Reading failed");
		CPLFree(pHeader2P1);
		return false;
	}

	char szDateTime[17] = { 0 };
	sprintf(szDateTime, "%04d-%02d-%02dT%02d:%02d", pHeader2P1->year, pHeader2P1->month, pHeader2P1->day, pHeader2P1->hour, pHeader2P1->minute);
	//�����ڶ����ļ�ͷ;
	this->SetMetadataItem("HEADER2_SAT_NAME", pHeader2P1->satName);
	this->SetMetadataItem("HEADER2_RECEIVE_DATETIME", szDateTime);
	this->SetMetadataItem("HEADER2_CHANNEL_NUM", ChannelToString(pHeader2P1->channel).c_str());
	this->SetMetadataItem("HEADER2_PROJECTION", ProjectionToString(pHeader2P1->projection).c_str());
	this->SetMetadataItem("HEADER2_IMAGE_WIDTH", ToString(pHeader2P1->width).c_str());
	this->SetMetadataItem("HEADER2_IMAGE_HEIGHT", ToString(pHeader2P1->height).c_str());
	this->SetMetadataItem("HEADER2_LEFTUP_LINE", ToString(pHeader2P1->leftUpLine).c_str());
	this->SetMetadataItem("HEADER2_LEFTUP_PIXEL", ToString(pHeader2P1->leftUpPixel).c_str());
	this->SetMetadataItem("HEADER2_SAMPLE", ToString(pHeader2P1->sample).c_str());
	this->SetMetadataItem("HEADER2_BOUND_NORTH", ToString(pHeader2P1->boundN*0.01).c_str());
	this->SetMetadataItem("HEADER2_BOUND_SOUTH", ToString(pHeader2P1->boundS*0.01).c_str());
	this->SetMetadataItem("HEADER2_BOUND_WEST", ToString(pHeader2P1->boundW*0.01).c_str());
	this->SetMetadataItem("HEADER2_BOUND_EAST", ToString(pHeader2P1->boundE*0.01).c_str());
	this->SetMetadataItem("HEADER2_CENTER_LAT", ToString(pHeader2P1->centerLat*0.01).c_str());
	this->SetMetadataItem("HEADER2_CENTER_LON", ToString(pHeader2P1->centerLon*0.01).c_str());
	this->SetMetadataItem("HEADER2_STANDARD_1", ToString(pHeader2P1->standard1*0.01).c_str());
	this->SetMetadataItem("HEADER2_STANDARD_2", ToString(pHeader2P1->standard2*0.01).c_str());
	this->SetMetadataItem("HEADER2_RESOLUTION_H", ToString(pHeader2P1->resolutionH*0.01).c_str());
	this->SetMetadataItem("HEADER2_RESOLUTION_V", ToString(pHeader2P1->resolutionV*0.01).c_str());
	this->SetMetadataItem("HEADER2_GRID_FLAG", (pHeader2P1->gridflag == 0) ? "Un-superimposed geographic grids" : "Superimposed geographic grids");
	this->SetMetadataItem("HEADER2_GRID_VALUE", ToString(pHeader2P1->gridvalue).c_str());
	this->SetMetadataItem("HEADER2_PALETEE_SIZE", ToString(pHeader2P1->sizePalette).c_str());
	this->SetMetadataItem("HEADER2_CALIBRATION_SIZE", ToString(pHeader2P1->sizeCalibration).c_str());
	this->SetMetadataItem("HEADER2_NAVGATION_SIZE", ToString(pHeader2P1->sizeNavigation).c_str());
	this->SetMetadataItem("HEADER2_RESERVED", ToString(pHeader2P1->reserved).c_str());

	this->nRasterXSize = pHeader2P1->width;
	this->nRasterYSize = pHeader2P1->height;

	ParseSRS(pHeader2P1->projection, pHeader2P1->standard1, pHeader2P1->standard2);

	ParseGeotransform(pHeader2P1->boundW*0.01, pHeader2P1->boundN*0.01,
		pHeader2P1->boundE*0.01, pHeader2P1->boundN*0.01,
		pHeader2P1->boundW*0.01, pHeader2P1->boundS*0.01,
		pHeader2P1->boundE*0.01, pHeader2P1->boundS*0.01);

	if (pHeader2P1->sizePalette > 0)
	{
		GByte *pData = static_cast<GByte *>(CPLMalloc(pHeader2P1->sizePalette));
		nBytesRead = VSIFReadL(pData, 1, pHeader2P1->sizePalette, this->fp);
		ParseColorTable(pData, pHeader2P1->sizePalette);
		CPLFree(pData);
	}

	if (pHeader2P1->sizeCalibration > 0)
	{
		GByte *pData = static_cast<GByte *>(CPLMalloc(pHeader2P1->sizeCalibration));
		nBytesRead = VSIFReadL(pData, 1, pHeader2P1->sizeCalibration, this->fp);
		ParseCalibration(pData, pHeader2P1->sizeCalibration);
		CPLFree(pData);
	}

	if (pHeader2P1->sizeNavigation > 0)
	{
		GByte *pData = static_cast<GByte *>(CPLMalloc(pHeader2P1->sizeNavigation));
		nBytesRead = VSIFReadL(pData, 1, pHeader2P1->sizeNavigation, this->fp);
		ParseNavigation(pData, pHeader2P1->sizeNavigation);
		CPLFree(pData);
	}

	//TODO ������չ���ݿ�

	CPLFree(pHeader2P1);
	return true;
}

bool AWXDataset::OpenP2()
{
	AWX_HEAD2_P2 *pHeader2P2 = static_cast<AWX_HEAD2_P2 *>(CPLMalloc(sizeof(AWX_HEAD2_P2)));
	int nBytesRead = VSIFReadL(pHeader2P2, 1, sizeof(AWX_HEAD2_P2), this->fp);
	if (nBytesRead < sizeof(AWX_HEAD2_P2))
	{
		CPLDebug("AWX", "Header2P2 file too short. Reading failed");
		CPLFree(pHeader2P2);
		return false;
	}

	char szBegDateTime[17] = { 0 };
	char szEndDateTime[17] = { 0 };
	sprintf(szBegDateTime, "%04d-%02d-%02dT%02d:%02d", pHeader2P2->bYear, pHeader2P2->bMonth, pHeader2P2->bDay, pHeader2P2->bHour, pHeader2P2->bMinute);
	sprintf(szEndDateTime, "%04d-%02d-%02dT%02d:%02d", pHeader2P2->eYear, pHeader2P2->eMonth, pHeader2P2->eDay, pHeader2P2->eHour, pHeader2P2->eMinute);
	//�����ڶ����ļ�ͷ;
	this->SetMetadataItem("HEADER2_SAT_NAME", pHeader2P2->satName);
	this->SetMetadataItem("HEADER2_START_DATETIME", szBegDateTime);
	this->SetMetadataItem("HEADER2_END_DATETIME", szEndDateTime);
	this->SetMetadataItem("HEADER2_CHANNEL_NUM", ChannelToString2(pHeader2P2->channel).c_str());
	this->SetMetadataItem("HEADER2_CHANNEL_R", ToString(pHeader2P2->channelR).c_str());
	this->SetMetadataItem("HEADER2_CHANNEL_G", ToString(pHeader2P2->channelG).c_str());
	this->SetMetadataItem("HEADER2_CHANNEL_B", ToString(pHeader2P2->channelB).c_str());
	this->SetMetadataItem("HEADER2_ORBIT_FLAG", (pHeader2P2->orbitFlag == 0) ? "Lower track" : "Higher track");
	this->SetMetadataItem("HEADER2_ELEMENT_SIZE", ToString(pHeader2P2->sizeElement).c_str());
	this->SetMetadataItem("HEADER2_PROJECTION", ProjectionToString(pHeader2P2->projection).c_str());
	this->SetMetadataItem("HEADER2_PRODUCT_TYPE", ProductToString(pHeader2P2->productType).c_str());
	this->SetMetadataItem("HEADER2_IMAGE_WIDTH", ToString(pHeader2P2->width).c_str());
	this->SetMetadataItem("HEADER2_IMAGE_HEIGHT", ToString(pHeader2P2->height).c_str());
	this->SetMetadataItem("HEADER2_LEFTUP_LINE", ToString(pHeader2P2->leftUpLine).c_str());
	this->SetMetadataItem("HEADER2_LEFTUP_PIXEL", ToString(pHeader2P2->leftUpPixel).c_str());
	this->SetMetadataItem("HEADER2_SAMPLE", ToString(pHeader2P2->sample).c_str());
	this->SetMetadataItem("HEADER2_BOUND_NORTH", ToString(pHeader2P2->boundN*0.01).c_str());
	this->SetMetadataItem("HEADER2_BOUND_SOUTH", ToString(pHeader2P2->boundS*0.01).c_str());
	this->SetMetadataItem("HEADER2_BOUND_WEST", ToString(pHeader2P2->boundW*0.01).c_str());
	this->SetMetadataItem("HEADER2_BOUND_EAST", ToString(pHeader2P2->boundE*0.01).c_str());
	this->SetMetadataItem("HEADER2_CENTER_LAT", ToString(pHeader2P2->centerLat*0.01).c_str());
	this->SetMetadataItem("HEADER2_CENTER_LON", ToString(pHeader2P2->centerLon*0.01).c_str());
	this->SetMetadataItem("HEADER2_STANDARD_1", ToString(pHeader2P2->standard1*0.01).c_str());
	this->SetMetadataItem("HEADER2_STANDARD_2", ToString(pHeader2P2->standard2*0.01).c_str());
	this->SetMetadataItem("HEADER2_RESOLUTION_H", ToString(pHeader2P2->resolutionH*0.01).c_str());
	this->SetMetadataItem("HEADER2_RESOLUTION_V", ToString(pHeader2P2->resolutionV*0.01).c_str());
	this->SetMetadataItem("HEADER2_GRID_FLAG", (pHeader2P2->gridFlag == 0) ? "Un-superimposed geographic grids" : "Superimposed geographic grids");
	this->SetMetadataItem("HEADER2_GRID_VALUE", ToString(pHeader2P2->gridValue).c_str());
	this->SetMetadataItem("HEADER2_PALETEE_SIZE", ToString(pHeader2P2->sizePalette).c_str());
	this->SetMetadataItem("HEADER2_CALIBRATION_SIZE", ToString(pHeader2P2->sizeCalibration).c_str());
	this->SetMetadataItem("HEADER2_NAVGATION_SIZE", ToString(pHeader2P2->sizeNavigation).c_str());
	this->SetMetadataItem("HEADER2_RESERVED", ToString(pHeader2P2->reserved).c_str());

	this->nRasterXSize = pHeader2P2->width;
	this->nRasterYSize = pHeader2P2->height;

	ParseSRS(pHeader2P2->projection, pHeader2P2->standard1, pHeader2P2->standard2);

	ParseGeotransform(pHeader2P2->boundW*0.01, pHeader2P2->boundN*0.01,
		pHeader2P2->boundE*0.01, pHeader2P2->boundN*0.01,
		pHeader2P2->boundW*0.01, pHeader2P2->boundS*0.01,
		pHeader2P2->boundE*0.01, pHeader2P2->boundS*0.01);

	if (pHeader2P2->sizePalette > 0)
	{
		GByte *pData = static_cast<GByte *>(CPLMalloc(pHeader2P2->sizePalette));
		nBytesRead = VSIFReadL(pData, 1, pHeader2P2->sizePalette, this->fp);
		ParseColorTable(pData, pHeader2P2->sizePalette);
		CPLFree(pData);
	}

	if (pHeader2P2->sizeCalibration > 0)
	{
		GByte *pData = static_cast<GByte *>(CPLMalloc(pHeader2P2->sizeCalibration));
		nBytesRead = VSIFReadL(pData, 1, pHeader2P2->sizeCalibration, this->fp);
		ParseCalibration(pData, pHeader2P2->sizeCalibration);
		CPLFree(pData);
	}

	if (pHeader2P2->sizeNavigation > 0)
	{
		GByte *pData = static_cast<GByte *>(CPLMalloc(pHeader2P2->sizeNavigation));
		nBytesRead = VSIFReadL(pData, 1, pHeader2P2->sizeNavigation, this->fp);
		ParseNavigation(pData, pHeader2P2->sizeNavigation);
		CPLFree(pData);
	}

	//TODO ������չ���ݿ�

	CPLFree(pHeader2P2);
	return true;
}

inline std::string ElementToString(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	std::string strString[27] = {
		"Numerical weather prediction",
		"Sea surface temperature (K) ",
		"Sea ice distribution (dimensionless) ",
		"Sea ice density (dimensionless) ",
		"Outgoing longwave radiation (W/m^2) ",
		"Normalized vegetation index (dimensionless) ",
		"Vegetation index ratio (dimensionless) ",
		"Snow distribution (dimensionless)",
		"Soil moisture (kg/m^3) ",
		"Sunshine (hour) ",
		"Cloud top height (hPa)",
		"Cloud top temperature (K)",
		"Low cirrus (dimensionless) ",
		"High cirrus (dimensionless)",
		"Precipitation index (mm/1h) ",
		"Precipitation index (mm/6h) ",
		"Precipitation index (mm/12h) ",
		"Precipitation index (mm/24h) ",
		"Upper tropospheric water vapor (relative humidity) (dimensionless)",
		"Brightness temperature",
		"Cloud amount (percentage)",
		"Cloud classification (dimensionless)",
		"Precipitation estimates (mm/6h)",
		"Precipitation estimates (mm/24h)",
		"Clear sky atmospheric precipitation (mm)",
		"Reserved",
		"Ground incident solar radiation (W/m^2)"
	};

	if (nValue >= 0 && nValue <= 26)
		return strId + strString[nValue];

	if (nValue >= 27 && nValue <= 30)
		return strId + "Reserved";

	if (nValue >= 31 && nValue <= 37)
		return strId + "Cloud humidity profiles (1000-300hPa), relative humidity fields of 7 standard levels(dimensionless)";

	if (nValue >= 38 && nValue <= 100)
		return strId + "Reserved";

	if (nValue == 101)
		return strId + "Clear sky environment monitoring datasets, synthesized data from channels 1, 2, and 4 (32 Bit) ";

	if (nValue >= 102 && nValue <= 200)
		return strId + "Reserved";

	if (nValue >= 201 && nValue <= 215)
		return "ATOVS (1000-10 hPa) temperature fields (K) of 15 standard levels";

	if (nValue >= 216 && nValue <= 300)
		return strId + "Reserved";

	if (nValue >= 301 && nValue <= 314)
		return strId + "ATOVS (850 ~ 10 hPa) thickness fields (m) of 14 standard levels";

	if (nValue >= 315 && nValue <= 400)
		return strId + "Reserved";

	if (nValue >= 401 && nValue <= 406)
		return strId + "ATOVS (1000 ~ 300 hPa) dew point temperature fields (K)of 6 standard levels";

	if (nValue >= 407 && nValue <= 500)
		return strId + "Reserved";

	switch (nValue)
	{
	case 501:
		return strId + "ATOVS atmospheric stability index (dimensionless)";
	case 502:
		return strId + "ATOVS clear sky total atmospheric column water vapor content(mm) ";
	case 503:
		return strId + "ATOVS total atmospheric column ozone content (Db) ";
	case 504:
		return strId + "ATOVS outgoing longwave radiation (W/m^2) ";
	case 505:
		return strId + "ATOVS cloud top height (hPa)";
	case 506:
		return strId + "ATOVS cloud top temperature (K) ";
	case 507:
		return strId + "ATOVS cloudiness (dimensionless)(ZK)";
	default:
		return strId;
		break;
	}
}

inline std::string TimeRangeToString(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	std::string strString[11] = {
		"Real time",
		"Daily mean",
		"5-day mean",
		"10-day mean",
		"Monthly mean",
		"Annual mean",
		"Daily accumulation",
		"5-day accumulation",
		"10-day accumulation",
		"Monthly accumulation",
		"Annual accumulation"
	};

	if (nValue >= 0 && nValue <= 10)
		return strId + strString[nValue];
	else
		return strId;
}

inline std::string QualityToString3(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	std::string strString[4] = {
		"No quality control value",
		"Upper limit quality control value only",
		"Lower limit quality control value only",
		"Having both lower and upper limit quality control values"
	};

	if (nValue >= 0 && nValue <= 3)
		return strId + strString[nValue];
	else
		return strId;
}

bool AWXDataset::OpenP3()
{
	AWX_HEAD2_P3 *pHeader2P3 = static_cast<AWX_HEAD2_P3 *>(CPLMalloc(sizeof(AWX_HEAD2_P3)));
	int nBytesRead = VSIFReadL(pHeader2P3, 1, sizeof(AWX_HEAD2_P3), this->fp);
	if (nBytesRead < sizeof(AWX_HEAD2_P3))
	{
		CPLDebug("AWX", "Header2P3 file too short. Reading failed");
		CPLFree(pHeader2P3);
		return false;
	}

	char szBeginDateTime[17] = { 0 };
	char szEndDateTime[17] = { 0 };
	sprintf(szBeginDateTime, "%04d-%02d-%02dT%02d:%02d", pHeader2P3->bYear, pHeader2P3->bMonth, pHeader2P3->bDay, pHeader2P3->bHour, pHeader2P3->bMinute);
	sprintf(szEndDateTime, "%04d-%02d-%02dT%02d:%02d", pHeader2P3->eYear, pHeader2P3->eMonth, pHeader2P3->eDay, pHeader2P3->eHour, pHeader2P3->eMinute);
	//�����ڶ����ļ�ͷ;
	this->SetMetadataItem("HEADER2_SAT_NAME", pHeader2P3->satName);
	this->SetMetadataItem("HEADER2_ELEMENT", ElementToString(pHeader2P3->element).c_str());
	this->SetMetadataItem("HEADER2_ELEMENT_SIZE", ToString(pHeader2P3->sizeElement).c_str());
	this->SetMetadataItem("HEADER2_VALUE_BASE", ToString(pHeader2P3->valueBase).c_str());
	this->SetMetadataItem("HEADER2_VALUE_RATE", ToString(pHeader2P3->valueRate).c_str());
	this->SetMetadataItem("HEADER2_TIME_RANGE", TimeRangeToString(pHeader2P3->timeRange).c_str());
	this->SetMetadataItem("HEADER2_START_DATATIME", szBeginDateTime);
	this->SetMetadataItem("HEADER2_END_DATATIME", szEndDateTime);
	this->SetMetadataItem("HEADER2_LEFTUP_LAT", ToString(pHeader2P3->latLeftUp*0.01).c_str());
	this->SetMetadataItem("HEADER2_LEFTUP_LON", ToString(pHeader2P3->lonLeftUp*0.01).c_str());
	this->SetMetadataItem("HEADER2_RIGHTDOWN_LAT", ToString(pHeader2P3->latRightDown*0.01).c_str());
	this->SetMetadataItem("HEADER2_RIGHTDOWN_LON", ToString(pHeader2P3->lonRightDown*0.01).c_str());
	this->SetMetadataItem("HEADER2_GRID_UNIT", ToString(pHeader2P3->gridUnit).c_str());
	this->SetMetadataItem("HEADER2_SPACE_H", ToString(pHeader2P3->gridSpaceH).c_str());
	this->SetMetadataItem("HEADER2_SPACE_V", ToString(pHeader2P3->gridSpaceV).c_str());
	this->SetMetadataItem("HEADER2_GRID_COUNT_H", ToString(pHeader2P3->gridCountH).c_str());
	this->SetMetadataItem("HEADER2_GRID_COUNT_V", ToString(pHeader2P3->gridCountV).c_str());
	this->SetMetadataItem("HEADER2_LAND_FLAG", (pHeader2P3->landFlag == 0) ? "Without judging value" : "With judging value");
	this->SetMetadataItem("HEADER2_LAND_VALUE", ToString(pHeader2P3->landValue).c_str());
	this->SetMetadataItem("HEADER2_CLOUD_FLAG", (pHeader2P3->cloudFlag == 0) ? "Without judging value" : "With judging value");
	this->SetMetadataItem("HEADER2_CLOUD_VALUE", ToString(pHeader2P3->cloudValue).c_str());
	this->SetMetadataItem("HEADER2_WATER_FLAG", (pHeader2P3->waterFlag == 0) ? "Without judging value" : "With judging value");
	this->SetMetadataItem("HEADER2_WATER_VALUE", ToString(pHeader2P3->waterValue).c_str());
	this->SetMetadataItem("HEADER2_QUALITY_FLAG", QualityToString3(pHeader2P3->flagQuality).c_str());
	this->SetMetadataItem("HEADER2_QUALITY_UP", ToString(pHeader2P3->qualityUp).c_str());
	this->SetMetadataItem("HEADER2_QUALITY_DOWN", ToString(pHeader2P3->qualityDown).c_str());
	this->SetMetadataItem("HEADER2_RESERVED", ToString(pHeader2P3->reserved).c_str());

	switch (pHeader2P3->sizeElement)
	{
	case 1:
	default:
		this->eDataType = GDT_Byte;
		break;
	case 2:
		this->eDataType = GDT_UInt16;
		break;
	case 4:
		this->eDataType = GDT_UInt32;
		break;
	}

	this->nRasterXSize = pHeader2P3->gridCountH;
	this->nRasterYSize = pHeader2P3->gridCountV;

	this->bHaveOffsetScale = true;

	//awx  units_value = (raw_value + base ) / rate = raw_value * (1.0/rate) + (base / rate)
	//gdal units_value = (raw_value * scale ) + offset
	double dfGdalScale = 1.0f / pHeader2P3->valueRate;
	double dfGdalOffset = pHeader2P3->valueBase * 1.0 / pHeader2P3->valueRate;

	this->dfScale = dfGdalScale;
	this->dfOffset = dfGdalOffset;

	this->pszProjection = CPLStrdup(SRS_WKT_WGS84);

	ParseGeotransform(pHeader2P3->lonLeftUp*0.01, pHeader2P3->latLeftUp*0.01,
		pHeader2P3->lonRightDown*0.01, pHeader2P3->latLeftUp*0.01,
		pHeader2P3->lonLeftUp*0.01, pHeader2P3->latRightDown*0.01,
		pHeader2P3->lonRightDown*0.01, pHeader2P3->latRightDown*0.01);

	//TODO ������չ���ݿ�

	CPLFree(pHeader2P3);
	return true;
}

inline std::string ElementToString4(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	if (nValue == 1)
		return (strId + "Polar-orbiting satellite data (ATOVS)");
	else if (nValue == 101)
		return (strId + "Geostationary atmospheric motion vectors");
	else
		return (strId + "Reserved");
}

inline std::string AlgorithmToString(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	std::string strString[3] = {
		strId + "Statistical regression inversion",
		strId + "Physical inversion",
		strId + "Maximum correlation method"
	};

	if (nValue >= 1 && nValue <= 3)
		return strString[nValue - 1];
	else
		return strId;
}

inline std::string InitTypeToString(short nValue)
{
	std::string strId = ToString(nValue) + ": ";
	std::string strString[5] = {
		strId + "Climate data",
		strId + "Conventional sounding data analysis fields ",
		strId + "NWP field",
		strId + "Regression inversion results",
		strId + "T213 report"
	};

	if (nValue >= 1 && nValue <= 5)
		return strString[nValue - 1];
	else
		return strId;
}

bool AWXDataset::OpenP4(vsi_l_offset nHeaderSize)
{
	AWX_HEAD2_P4 *pHeader2P4 = static_cast<AWX_HEAD2_P4 *>(CPLMalloc(sizeof(AWX_HEAD2_P4)));
	int nBytesRead = VSIFReadL(pHeader2P4, 1, sizeof(AWX_HEAD2_P4), this->fp);
	if (nBytesRead < sizeof(AWX_HEAD2_P4))
	{
		CPLDebug("AWX", "Header2P4 file too short. Reading failed");
		CPLFree(pHeader2P4);
		return false;
	}

	char szBeginDateTime[17] = { 0 };
	char szEndDateTime[17] = { 0 };
	sprintf(szBeginDateTime, "%04d-%02d-%02dT%02d:%02d", pHeader2P4->bYear, pHeader2P4->bMonth, pHeader2P4->bDay, pHeader2P4->bHour, pHeader2P4->bMinute);
	sprintf(szEndDateTime, "%04d-%02d-%02dT%02d:%02d", pHeader2P4->eYear, pHeader2P4->eMonth, pHeader2P4->eDay, pHeader2P4->eHour, pHeader2P4->eMinute);
	//�����ڶ����ļ�ͷ;
	this->SetMetadataItem("HEADER2_SAT_NAME", pHeader2P4->satName);
	this->SetMetadataItem("HEADER2_ELEMENT", ElementToString4(pHeader2P4->element).c_str());
	this->SetMetadataItem("HEADER2_ELEMENT_SIZE", ToString(pHeader2P4->sizeElement).c_str());
	this->SetMetadataItem("HEADER2_ELEMENT_COUNT", ToString(pHeader2P4->countElement).c_str());
	this->SetMetadataItem("HEADER2_START_DATATIME", szBeginDateTime);
	this->SetMetadataItem("HEADER2_END_DATATIME", szEndDateTime);
	this->SetMetadataItem("HEADER2_ALGORITHM", AlgorithmToString(pHeader2P4->algorithm).c_str());
	this->SetMetadataItem("HEADER2_INIT_TYPE", InitTypeToString(pHeader2P4->initType).c_str());
	this->SetMetadataItem("HEADER2_RESERVED", ToString(pHeader2P4->reserved).c_str());

	this->bRaster = false;

	if (pHeader2P4->element == 1)  //��������ATOVS��ɢ����Ҫ��
	{
		this->apoLayers.push_back(new OGRAWXLayer(this->fp, nHeaderSize, pHeader2P4, "Polar-orbiting satellite data (ATOVS)", true));
	}
	else if (pHeader2P4->element == 101)  //��ֹ�����Ƽ�����ɢ����Ҫ��
	{
		this->apoLayers.push_back(new OGRAWXLayer(this->fp, nHeaderSize, pHeader2P4, "Geostationary atmospheric motion vectors", false));
	}
	else
	{
		CPLDebug("AWX", "Header2P4 element = %d. current unsupport.");
		CPLFree(pHeader2P4);
		return false;
	}

	//TODO ������չ���ݿ�

	CPLFree(pHeader2P4);
	return true;
}

bool AWXDataset::OpenP5()
{
	//TODO ����

	CPLDebug("AWX", "Unsupport analysis product. Reading failed");
	return false;
}

void AWXDataset::GetHeaderSize(vsi_l_offset &nHeaderSize)
{
	vsi_l_offset size = nHeaderSize;
	struct stat statbuf;

	int err = stat(this->pszFilename, &statbuf);
	if (err != 0)
	{
		nHeaderSize = VSIFTellL(this->fp);
		VSIFSeekL(this->fp, 0, SEEK_END);
		size = VSIFTellL(this->fp);
		VSIFSeekL(this->fp, nHeaderSize, SEEK_SET);
	}
	else
	{
		size = statbuf.st_size;
	}

	int nPixelSize = GDALGetDataTypeSize(eDataType) / 8;
	//��ȡ�ļ���С����ȥͼ���С��ʣ���Ϊ�ļ�ͷ
	nHeaderSize = size - ((vsi_l_offset)nRasterXSize * nRasterYSize * nPixelSize);

	CPLDebug("AWX", "Header size = %d", nHeaderSize);
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

GDALDataset *AWXDataset::Open(GDALOpenInfo * poOpenInfo)
{
	if (poOpenInfo->nHeaderBytes < 128)
		return NULL;
	/* -------------------------------------------------------------------- */
	/*  Create a corresponding GDALDataset.                                 */
	/* -------------------------------------------------------------------- */
	AWXDataset *poDS = new AWXDataset();

	poDS->fp = VSIFOpenL(poOpenInfo->pszFilename, "rb");
	if (poDS->fp == NULL)
	{
		delete poDS;
		return NULL;
	}

	poDS->pszFilename = poOpenInfo->pszFilename;

	/* -------------------------------------------------------------------- */
	/*  ��ȡ��һ���ļ�ͷ.                                                   */
	/* -------------------------------------------------------------------- */
	AWX_HEAD1 *pHeader1 = static_cast<AWX_HEAD1 *>(CPLMalloc(sizeof(AWX_HEAD1)));

	size_t nBytesRead = 0;
	if (VSIFSeekL(poDS->fp, 0, SEEK_SET) >= 0)
		nBytesRead = VSIFReadL(pHeader1, 1, sizeof(AWX_HEAD1), poDS->fp);

	if (nBytesRead < sizeof(AWX_HEAD1))
	{
		CPLDebug("AWX", "Header file too short. Reading failed");
		CPLFree(pHeader1);
		delete poDS;
		return NULL;
	}

	if (!EQUALN(pHeader1->fileName + 8, ".AWX", 4))
	{
		CPLDebug("AWX", "Unsupported format.");
		CPLFree(pHeader1);
		delete poDS;
		return NULL;
	}

	//������һ���ļ�ͷ
	poDS->iProduct = (AWXProduct)pHeader1->typeProduct;

	poDS->SetMetadataItem("HEADER1_SAT96_NAME", pHeader1->fileName);
	poDS->SetMetadataItem("HEADER1_BYTE_ORDER", pHeader1->byteOrder == 0 ? "INTEL(LSB)" : "MOTOROLA(MSB)");
	poDS->SetMetadataItem("HEADER1_FIRST_HEADER_SIZE", ToString(pHeader1->sizeHead1).c_str());
	poDS->SetMetadataItem("HEADER1_SECOND_HEADER_SIZE", ToString(pHeader1->sizeHead2).c_str());
	poDS->SetMetadataItem("HEADER1_FILL_SIZE", ToString(pHeader1->sizeFilled).c_str());
	poDS->SetMetadataItem("HEADER1_RECORD_SIZE", ToString(pHeader1->sizeRecord).c_str());
	poDS->SetMetadataItem("HEADER1_NUM_HEAD", ToString(pHeader1->numHead).c_str());
	poDS->SetMetadataItem("HEADER1_NUM_DATA", ToString(pHeader1->numData).c_str());
	poDS->SetMetadataItem("HEADER1_TYPE_PRODUCT", ProductToStirng(poDS->iProduct).c_str());
	poDS->SetMetadataItem("HEADER1_COMPRESS_MODE", ToString(pHeader1->compressMode).c_str());
	poDS->SetMetadataItem("HEADER1_FORMAT_FLAG", pHeader1->fmtFlag);
	poDS->SetMetadataItem("HEADER1_QUALITY_FLAG", QualityToString(pHeader1->qualityFlag).c_str());

	/* -------------------------------------------------------------------- */
	/*  ��ȡ�ڶ����ļ�ͷ.                                                   */
	/* -------------------------------------------------------------------- */
	poDS->nBands = 1;
	poDS->eDataType = GDT_Byte;

	vsi_l_offset nHeaderSize = pHeader1->sizeRecord * pHeader1->numHead;

	switch (poDS->iProduct)
	{
	case AS_GEOSTATIONARY: // ��ֹ��������ͼ���Ʒ
	{
		if (!poDS->OpenP1())
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_POLAR_ORBITING:// ������������ͼ���Ʒ
	{
		if (!poDS->OpenP2())
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_GRID_FIELD:// ��㳡������Ʒ
	{
		if (!poDS->OpenP3())
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_DISCRETE_FIELD: // ��ɢ��������Ʒ
	{
		if (!poDS->OpenP4(nHeaderSize))
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_GRAPHICAL_ANALYSIS:// ͼ�κͷ�����Ʒ
	{
		if (!poDS->OpenP5())
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_UNKNOWN://δ�������͵Ĳ�Ʒ
	default:
	{
		CPLDebug("AWX", "Header2 file too short. Reading failed");
		CPLFree(pHeader1);
		delete poDS;
		return NULL;
	}
	}

	if (poDS->bRaster)
	{
		if (!GDALCheckDatasetDimensions(poDS->nRasterXSize, poDS->nRasterYSize))
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}

		/* -------------------------------------------------------------------- */
		/*      Create band information objects.                                */
		/* -------------------------------------------------------------------- */
		const int nPixelOffset = GDALGetDataTypeSize(poDS->eDataType) / 8;
		const int nLineOffset = poDS->nRasterXSize * nPixelOffset;

		for (int i = 1; i <= poDS->nBands; i++)
		{
			AWXRasterBand *pBand = new AWXRasterBand(poDS, i, poDS->fp,
				nHeaderSize, nPixelOffset, nLineOffset, poDS->eDataType,
				pHeader1->byteOrder, poDS->poColorTable);

			if (poDS->bHaveOffsetScale)
			{
				pBand->SetScale(poDS->dfScale);
				pBand->SetOffset(poDS->dfOffset);
			}

			poDS->SetBand(i, pBand);
		}
	}

	CPLFree(pHeader1);

	/* -------------------------------------------------------------------- */
	/*      Initialize any PAM information.                                 */
	/* -------------------------------------------------------------------- */
	poDS->SetDescription(poOpenInfo->pszFilename);
	poDS->TryLoadXML();

	// opens overviews.
	poDS->oOvManager.Initialize(poDS, poDS->pszFilename);

	/* -------------------------------------------------------------------- */
	/*      Confirm the requested access is supported.                      */
	/* -------------------------------------------------------------------- */
	if (poOpenInfo->eAccess == GA_Update)
	{
		delete poDS;
		CPLError(CE_Failure, CPLE_NotSupported,
			"The AWX driver does not support update access to existing"
			" datasets.");
		return NULL;
	}

	return poDS;
}

/************************************************************************/
/*                        GDALRegister_AWX()                            */
/************************************************************************/

void GDALRegister_AWX()
{
	if (GDALGetDriverByName("AWX") != NULL)
		return;

	GDALDriver *poDriver = new GDALDriver();

	poDriver->SetDescription("AWX");
	poDriver->SetMetadataItem(GDAL_DCAP_RASTER, "YES");
	poDriver->SetMetadataItem(GDAL_DCAP_VECTOR, "YES");
	poDriver->SetMetadataItem(GDAL_DMD_LONGNAME, "Advanced Weather-satellite eXchange format");
	poDriver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "frmt_awx.html");
	poDriver->SetMetadataItem(GDAL_DMD_EXTENSION, "awx");
	poDriver->SetMetadataItem(GDAL_DCAP_VIRTUALIO, "YES");

	poDriver->pfnOpen = AWXDataset::Open;

	GetGDALDriverManager()->RegisterDriver(poDriver);
}