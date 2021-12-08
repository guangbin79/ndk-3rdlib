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

 //用于解析气象卫星分发产品数据格式，格式详细说明参考
 //中文：http://satellite.nsmc.org.cn/PortalSite/StaticContent/FileDownload.aspx?CategoryID=1&LinkID=86
 //英文：http://satellite.nsmc.org.cn/PortalSite/StaticContent/FileDownload.aspx?CategoryID=1&LinkID=28

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
	char fileName[12];          //1-12   Sat96文件名
	short byteOrder;            //13_14  整形数的字节顺序
	short sizeHead1;            //15-16  第一级文件头长度
	short sizeHead2;            //17-18  第二级文件头长度
	short sizeFilled;           //19-20  填充段数据长度
	short sizeRecord;           //21-22  记录长度
	short numHead;              //23-24   文件头占用记录数
	short numData;              //25-26   产品数据占用记录数
	short typeProduct;          //27-28   产品类别
	short compressMode;         //29-30   压缩方式
	char fmtFlag[8];            //31-38   格式说明字符串
	short qualityFlag;          //39-40   产品数据质量标记
}AWX_HEAD1; //第一级文件头

typedef struct {
	char satName[8];        //41-48  卫星名
	short year;             //49-50   时间（年）
	short month;            //51-52   时间（月）
	short day;              //53-54   时间（日）
	short hour;             //55-56   时间（时）
	short minute;           //57-58   时间（分）
	short channel;          //59-60   通道号
	short projection;       //61-62   投影方式
	short width;            //63-64   图形宽度
	short height;           //65-66   图像高度
	short leftUpLine;       //67-68   图像左上角扫描线号
	short leftUpPixel;      //69-70   图像左上角象元号
	short sample;           //71-72   抽样率
	short boundN;           //73-74   地理范围（北纬）
	short boundS;           //75-76   地理范围（南纬）
	short boundW;           //77-78   地理范围（西经）
	short boundE;           //79-80   地理范围（东经）
	short centerLat;        //81-82   投影中心纬度（度*100）
	short centerLon;        //83-84   投影中心经度（度*100）
	short standard1;        //85-86   投影标准纬度1（或标准经度）（度*100）
	short standard2;        //87-88   标准投影纬度2
	short resolutionH;      //89-90   投影水平分辨率
	short resolutionV;      //91-92   投影垂直分辨率
	short gridflag;         //93-94   地理网格叠加标志
	short gridvalue;        //95-96   地理网格叠加值
	short sizePalette;      //97-98   雕色表数据长度
	short sizeCalibration;  //99-100  定标数据块长度
	short sizeNavigation;   //101-102 定位数据块长度
	short reserved;         //103-104 保留
}AWX_HEAD2_P1; //静止气象卫星图像产品第二级文件头记录格式

typedef struct {
	char satName[8];            //41-48    卫星名
	short bYear;                //49-50    开始时间（年）
	short bMonth;               //51-52    开始时间(月) 
	short bDay;                 //53-54    开始时间（日）
	short bHour;                //55-56    开始时间(时)
	short bMinute;              //57-58    开始时间（分）
	short eYear;                //59-60    结束时间（年）
	short eMonth;               //61-62    结束时间（月）
	short eDay;                 //63-64    结束时间（日）
	short eHour;                //65-66    结束时间（时）
	short eMinute;              //67-68    结束时间（分）
	short channel;              //69-70    通道号
	short channelR;             //71-72    R通道号
	short channelG;             //73-74    G通道号 
	short channelB;             //75-76    B通道号
	short orbitFlag;            //77-78    升降轨标志
	short orbitNum;             //79-80    轨道号  
	short sizeElement;          //81-82    一个像元占字节数
	short projection;           //83-84    投影方式   
	short productType;          //85-86    产品类型    
	short width;                //87-88    图像宽度   
	short height;               //89-90    图像高度    
	short leftUpLine;           //91-92    图像左上角扫描线号
	short leftUpPixel;          //93-94    图像左上角像元号 
	short sample;               //95-96    抽样率  
	short boundN;               //97-98    地理范围（北纬）
	short boundS;               //99-100   地理范围（南纬）
	short boundW;               //101-102  地理范围（西经）
	short boundE;               //103-104  地理范围（东经）
	short centerLat;            //105-106  投影中心纬度 度*100 
	short centerLon;            //107-108  投影中心经度 度*100  
	short standard1;            //109-110  投影标准纬度1（或标准经度） 度*100 
	short standard2;            //111-112  标准投影纬度2  
	short resolutionH;          //113-114  投影水平分辨率 
	short resolutionV;          //115-116  投影迟滞分辨率   
	short gridFlag;             //117-118  地理网格叠加标志  
	short gridValue;            //119-120  地理网格叠加值    
	short sizePalette;          //121-122  调色表数据块长度 
	short sizeCalibration;      //123-124 定标数据块长度
	short sizeNavigation;       //125-126 定位数据块长度 
	short reserved;             //127-128 保留
}AWX_HEAD2_P2; //极轨气象卫星图像产品第二级文件头记录格式

typedef struct {
	char satName[8];        //41-48  卫星名
	short element;          //49-50  格点场要素
	short sizeElement;      //51-52  格点数据字节
	short valueBase;        //53-54  格点数据基准值
	short valueRate;        //55-56  格点数据比例因子
	short timeRange;        //57-58  时间范围代码
	short bYear;            //59-60  开始年
	short bMonth;           //61-62  开始月
	short bDay;             //63-64  开始日
	short bHour;            //65-66  开始时
	short bMinute;          //67-68  开始分
	short eYear;            //69-70  结束年
	short eMonth;           //71-72  结束月
	short eDay;             //73-74  结束日
	short eHour;            //75-76  结束时
	short eMinute;          //77-78  结束分
	short latLeftUp;        //79-80  网格左上角纬度
	short lonLeftUp;        //81-82  网格左上角经度
	short latRightDown;     //83-84  网格右下角纬度
	short lonRightDown;     //85-86  网格右下角经度
	short gridUnit;         //87-88  格距单位
	short gridSpaceH;       //89-90  横向格距
	short gridSpaceV;       //91-92  纵向格距
	short gridCountH;       //93-94  横向格点数
	short gridCountV;       //95-96  纵向格点数
	short landFlag;         //97-98  有无陆地判释值
	short landValue;        //99-100 陆地具体判释值
	short cloudFlag;        //101-102 有无云判释值
	short cloudValue;       //103-104 云具体判释值
	short waterFlag;        //105-106 有无水体判释值 
	short waterValue;       //107-108 水体具体判释值
	short iceFlag;          //109-110 有无冰体判释值
	short iceValue;         //111-112 冰体具体判释值
	short flagQuality;      //113-114 是否有质量控制值
	short qualityUp;        //115-116  质量控制值上限
	short qualityDown;      //117-118  质量控制值下限
	short reserved;         //119-120  备用
}AWX_HEAD2_P3;  //格点场定量产品格式

typedef struct {
	char satName[8];        //41-48   卫星名
	short element;          //49-50   要素
	short sizeElement;		//51-52   每个记录多少个字
	short countElement;     //53-54   探测点总数
	short bYear;            //55-56   开始年
	short bMonth;           //57-58   开始月
	short bDay;             //59-60   开始日
	short bHour;            //61-62   开始时
	short bMinute;          //63-64   开始分
	short eYear;            //65-66   结束年
	short eMonth;           //67-68   结束月
	short eDay;             //69-70   结束日
	short eHour;            //71-72   结束时
	short eMinute;          //73-74   结束分
	short algorithm;        //75-76   反演方法类型
	short initType;         //77-78   初估场类型
	short reserved;         //79-80   缺省值
}AWX_HEAD2_P4;  //离散场第二级文件头记录格式

typedef struct {
	short nProbeLatitude;		//1-2  探测点的纬度
	short nProbeLongitude;		//3_4  探测点的经度
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
} AWX_ATOVS;  //极轨卫星ATOVS离散场数据格式

typedef struct {
	short nProbeLatitude;		//1-2  探测点的纬度
	short nProbeLongitude;		//3_4  探测点的经度
	short nProbeLevels;			//5-6  探测层次
	short nWindDirection;		//7-8  风向
	short nWindVelocity;		//9-10 风速
	short nUnkonwn;				//11-12 //TODO 说明文档中没有描述
	short nTemperature;			//13-14 温度
	char cReserved[26];			//15-40 内部保留
} AWX_GDCMW;  //静止卫星云迹风离散场数据格式

enum AWXProduct  // Product
{
	AS_UNKNOWN = 0,				//未定义类型的产品
	AS_GEOSTATIONARY = 1,       // 静止气象卫星图象产品
	AS_POLAR_ORBITING = 2,      // 极轨气象卫星图象产品
	AS_GRID_FIELD = 3,			// 格点场定量产品
	AS_DISCRETE_FIELD = 4,      // 离散场定量产品
	AS_GRAPHICAL_ANALYSIS = 5   // 图形和分析产品
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
	if (nType == 0 || nType > 5)	//未投影
		return;

	OGRSpatialReference oSRS;

	//TODO 需要对各种投影方式进行确定

	if (nType == 1)
		oSRS.SetLCC(nS1, nS2, 0, 0, 0, 0);//Lambert 
	else if (nType == 2)
		oSRS.SetMercator(nS1, 0, 1, 0, 0);//Mercator
	else if (nType == 3)
		oSRS.SetPS(0, nS1, 1, 0, 0);//极射投影
	else if (nType == 4)
		oSRS.SetWellKnownGeogCS("WGS84");//等经纬度投影
	else if (nType == 5)//等面积投影
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
	//TODO 定标数据块
}

void AWXDataset::ParseNavigation(GByte *pData, int nSize)
{
	//TODO 定位数据块
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
	//解析第二级文件头;
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

	//TODO 解析扩展数据块

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
	//解析第二级文件头;
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

	//TODO 解析扩展数据块

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
	//解析第二级文件头;
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

	//TODO 解析扩展数据块

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
	//解析第二级文件头;
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

	if (pHeader2P4->element == 1)  //极轨卫星ATOVS离散场的要素
	{
		this->apoLayers.push_back(new OGRAWXLayer(this->fp, nHeaderSize, pHeader2P4, "Polar-orbiting satellite data (ATOVS)", true));
	}
	else if (pHeader2P4->element == 101)  //静止卫星云迹风离散场的要素
	{
		this->apoLayers.push_back(new OGRAWXLayer(this->fp, nHeaderSize, pHeader2P4, "Geostationary atmospheric motion vectors", false));
	}
	else
	{
		CPLDebug("AWX", "Header2P4 element = %d. current unsupport.");
		CPLFree(pHeader2P4);
		return false;
	}

	//TODO 解析扩展数据块

	CPLFree(pHeader2P4);
	return true;
}

bool AWXDataset::OpenP5()
{
	//TODO 解析

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
	//获取文件大小，减去图像大小，剩余的为文件头
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
	/*  读取第一级文件头.                                                   */
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

	//解析第一级文件头
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
	/*  读取第二级文件头.                                                   */
	/* -------------------------------------------------------------------- */
	poDS->nBands = 1;
	poDS->eDataType = GDT_Byte;

	vsi_l_offset nHeaderSize = pHeader1->sizeRecord * pHeader1->numHead;

	switch (poDS->iProduct)
	{
	case AS_GEOSTATIONARY: // 静止气象卫星图象产品
	{
		if (!poDS->OpenP1())
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_POLAR_ORBITING:// 极轨气象卫星图象产品
	{
		if (!poDS->OpenP2())
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_GRID_FIELD:// 格点场定量产品
	{
		if (!poDS->OpenP3())
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_DISCRETE_FIELD: // 离散场定量产品
	{
		if (!poDS->OpenP4(nHeaderSize))
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_GRAPHICAL_ANALYSIS:// 图形和分析产品
	{
		if (!poDS->OpenP5())
		{
			CPLFree(pHeader1);
			delete poDS;
			return NULL;
		}
		break;
	}
	case AS_UNKNOWN://未定义类型的产品
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