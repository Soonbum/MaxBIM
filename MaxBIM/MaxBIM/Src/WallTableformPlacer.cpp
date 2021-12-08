#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "WallTableformPlacer.hpp"

using namespace wallTableformPlacerDG;

static WallTableformPlacingZone		placingZone;		// �⺻ ���� ���� ����
static InfoWall						infoWall;			// �� ��ü ����
API_Guid		structuralObject_forTableformWall;		// ���� ��ü�� GUID

static short	layerInd_Euroform;			// ���̾� ��ȣ: ������ (����)
static short	layerInd_RectPipe;			// ���̾� ��ȣ: ��� ������ (����)
static short	layerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ (A,CŸ��)
static short	layerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ�� (AŸ��, �� �̻� ������� ����)
static short	layerInd_Clamp;				// ���̾� ��ȣ: ���� Ŭ���� (�� �̻� ������� ����)
static short	layerInd_HeadPiece;			// ���̾� ��ȣ: ����ǽ� (A,B,CŸ��, B,CŸ�Կ����� ������Ʈ�� Push-Pull Props ����ǽ�)
static short	layerInd_Join;				// ���̾� ��ȣ: ����ö�� (A,B,CŸ��, B,CŸ�Կ����� �簢������ ����ö��)

static short	layerInd_Plywood;			// ���̾� ��ȣ: ���� (����)
static short	layerInd_Timber;			// ���̾� ��ȣ: ���� (����)
static short	layerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ (BŸ��)
static short	layerInd_BlueClamp;			// ���̾� ��ȣ: ��� Ŭ����
static short	layerInd_BlueTimberRail;	// ���̾� ��ȣ: ��� ���
static short	layerInd_Hidden;			// ���̾� ��ȣ: ���� (B,CŸ��, ��������� Ÿ���� ���� �ָ��� ���� ����� ��ü)

// Ŀ���� ����
static bool		bLayerInd_Euroform;			// ���̾� ��ȣ: ������
static bool		bLayerInd_RectPipe;			// ���̾� ��ȣ: ��� ������
static bool		bLayerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
static bool		bLayerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ��
static bool		bLayerInd_HeadPiece;		// ���̾� ��ȣ: ����ǽ�
static bool		bLayerInd_Join;				// ���̾� ��ȣ: ����ö��

static bool		bLayerInd_SlabTableform;	// ���̾� ��ȣ: ������ ���̺���
static bool		bLayerInd_Profile;			// ���̾� ��ȣ: KS��������

static bool		bLayerInd_Steelform;		// ���̾� ��ȣ: ��ƿ��
static bool		bLayerInd_Plywood;			// ���̾� ��ȣ: ����
static bool		bLayerInd_Timber;			// ���̾� ��ȣ: ����
static bool		bLayerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static bool		bLayerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static bool		bLayerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳ��ǳ�
static bool		bLayerInd_IncornerPanel;	// ���̾� ��ȣ: ���ڳ��ǳ�
static bool		bLayerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���
static bool		bLayerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
static bool		bLayerInd_Hidden;			// ���̾� ��ȣ: ����

static short	layerInd_SlabTableform;		// ���̾� ��ȣ: ������ ���̺���
static short	layerInd_Profile;			// ���̾� ��ȣ: KS��������

static short	layerInd_Steelform;			// ���̾� ��ȣ: ��ƿ��
static short	layerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static short	layerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳ��ǳ�
static short	layerInd_IncornerPanel;		// ���̾� ��ȣ: ���ڳ��ǳ�
static short	layerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���

static GS::Array<API_Guid>	elemList_Front;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������ (�ո�)
static GS::Array<API_Guid>	elemList_Back;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������ (�޸�)

static int	clickedIndex;	// Ŭ���� ��ư�� �ε���
static int	iMarginSide;	// �� ����� ä����� �ϴµ� ���� ����Ǵ� ���� ��� �� ���ΰ�? (1-������, 2-������)


// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeTableformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx;
	double		dx, dy;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>		walls;
	GS::Array<API_Guid>		morphs;
	long					nWalls = 0;
	long					nMorphs = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// ���� ��ü ����
	InfoMorphForWallTableform	infoMorph [2];
	InfoMorphForWallTableform	infoMorph_Basic;
	InfoMorphForWallTableform	infoMorph_Extra;

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_wall;		// ���� �۾� �� ����


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: �� (1��), ���� ���� ���� (1��)\n�ɼ� ����: ���� ���� ����(�޸� - 1�� ������ ���̰� �ٸ�) (1��)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// �� 1��, ���� 1~2�� �����ؾ� ��
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_WallID)		// ���ΰ�?
				walls.Push (tElem.header.guid);

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nWalls = walls.GetSize ();
	nMorphs = morphs.GetSize ();

	// ���� 1���ΰ�?
	if (nWalls != 1) {
		ACAPI_WriteReport ("���� 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1�� �Ǵ� 2���ΰ�?
	if ((nMorphs < 1) || (nMorphs > 2)) {
		ACAPI_WriteReport ("���� ���� ������ 1�� �Ǵ� 2���� �����ϼž� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// (1) �� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = walls.Pop ();
	structuralObject_forTableformWall = elem.header.guid;
	err = ACAPI_Element_Get (&elem);						// elem.wall.poly.nCoords : ������ ���� ������ �� ����
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);	// memo.coords : ������ ��ǥ�� ������ �� ����
	
	if (elem.wall.thickness != elem.wall.thickness1) {
		ACAPI_WriteReport ("���� �β��� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}
	infoWall.wallThk		= elem.wall.thickness;
	infoWall.floorInd		= elem.header.floorInd;
	infoWall.bottomOffset	= elem.wall.bottomOffset;
	infoWall.begX			= elem.wall.begC.x;
	infoWall.begY			= elem.wall.begC.y;
	infoWall.endX			= elem.wall.endC.x;
	infoWall.endY			= elem.wall.endC.y;

	ACAPI_DisposeElemMemoHdls (&memo);

	// (2) ���� ������ ������
	for (xx = 0 ; xx < nMorphs ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = morphs.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

		// ���� ������ ���� ������(������ ���� ������) �ߴ�
		if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
			ACAPI_WriteReport ("������ ������ ���� �ʽ��ϴ�.", true);
			err = APIERR_GENERAL;
			return err;
		}

		// ������ GUID ����
		infoMorph [xx].guid = elem.header.guid;

		// ������ ���ϴ�, ���� �� ����
		if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
			// ���ϴ� ��ǥ ����
			infoMorph [xx].leftBottomX = elem.morph.tranmat.tmx [3];
			infoMorph [xx].leftBottomY = elem.morph.tranmat.tmx [7];
			infoMorph [xx].leftBottomZ = elem.morph.tranmat.tmx [11];

			// ���� ��ǥ��?
			if (abs (infoMorph [xx].leftBottomX - info3D.bounds.xMin) < EPS)
				infoMorph [xx].rightTopX = info3D.bounds.xMax;
			else
				infoMorph [xx].rightTopX = info3D.bounds.xMin;
			if (abs (infoMorph [xx].leftBottomY - info3D.bounds.yMin) < EPS)
				infoMorph [xx].rightTopY = info3D.bounds.yMax;
			else
				infoMorph [xx].rightTopY = info3D.bounds.yMin;
			if (abs (infoMorph [xx].leftBottomZ - info3D.bounds.zMin) < EPS)
				infoMorph [xx].rightTopZ = info3D.bounds.zMax;
			else
				infoMorph [xx].rightTopZ = info3D.bounds.zMin;
		} else {
			// ���� ��ǥ ����
			infoMorph [xx].rightTopX = elem.morph.tranmat.tmx [3];
			infoMorph [xx].rightTopY = elem.morph.tranmat.tmx [7];
			infoMorph [xx].rightTopZ = elem.morph.tranmat.tmx [11];

			// ���ϴ� ��ǥ��?
			if (abs (infoMorph [xx].rightTopX - info3D.bounds.xMin) < EPS)
				infoMorph [xx].leftBottomX = info3D.bounds.xMax;
			else
				infoMorph [xx].leftBottomX = info3D.bounds.xMin;
			if (abs (infoMorph [xx].rightTopY - info3D.bounds.yMin) < EPS)
				infoMorph [xx].leftBottomY = info3D.bounds.yMax;
			else
				infoMorph [xx].leftBottomY = info3D.bounds.yMin;
			if (abs (infoMorph [xx].rightTopZ - info3D.bounds.zMin) < EPS)
				infoMorph [xx].leftBottomZ = info3D.bounds.zMax;
			else
				infoMorph [xx].leftBottomZ = info3D.bounds.zMin;
		}

		// ������ Z�� ȸ�� ���� (���� ��ġ ����)
		dx = infoMorph [xx].rightTopX - infoMorph [xx].leftBottomX;
		dy = infoMorph [xx].rightTopY - infoMorph [xx].leftBottomY;
		infoMorph [xx].ang = RadToDegree (atan2 (dy, dx));

		// ������ ���� ����
		infoMorph [xx].horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);

		// ������ ���� ����
		infoMorph [xx].verLen = abs (info3D.bounds.zMax - info3D.bounds.zMin);

		// ���� ���� ����
		API_Elem_Head* headList = new API_Elem_Head [1];
		headList [0] = elem.header;
		err = ACAPI_Element_Delete (&headList, 1);
		delete headList;
	}

	if (nMorphs == 1) {
		placingZone.bExtra = false;

		// ������ 1���� ���, �⺻ ������ ����
		infoMorph_Basic = infoMorph [0];
		infoMorph_Extra = infoMorph [0];
	} else {
		placingZone.bExtra = true;

		// ������ 2���� ���, ���� ���̰� ���� ���� �⺻ ������
		if (infoMorph [0].verLen > infoMorph [1].verLen) {
			infoMorph_Basic = infoMorph [1];
			infoMorph_Extra = infoMorph [0];
		} else {
			infoMorph_Basic = infoMorph [0];
			infoMorph_Extra = infoMorph [1];
		}
	}

	// ���� ������ ���� ���� ���� ������Ʈ
	placingZone.leftBottomX		= infoMorph_Basic.leftBottomX;
	placingZone.leftBottomY		= infoMorph_Basic.leftBottomY;
	placingZone.leftBottomZ		= infoMorph_Basic.leftBottomZ;
	placingZone.horLen			= infoMorph_Basic.horLen;
	placingZone.verLenBasic		= infoMorph_Basic.verLen;
	placingZone.ang				= DegreeToRad (infoMorph_Basic.ang);
	if (placingZone.bExtra == true)
		placingZone.verLenExtra		= infoMorph_Extra.verLen;
	
	// �۾� �� ���� �ݿ� -- ����
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_wall = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoWall.floorInd) {
			workLevel_wall = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ���� ������ �� ������ ����
	placingZone.leftBottomZ = infoWall.bottomOffset;

	// �ʱ� �� ���� ���
	placingZone.nCellsInHor = (short)floor (placingZone.horLen / 2.300);
	placingZone.nCellsInVerBasic = (short)floor (placingZone.verLenBasic / 1.200);
	placingZone.nCellsInVerExtra = (short)floor (placingZone.verLenExtra / 1.200);

FIRST:

	// [DIALOG] 1��° ���̾�α׿��� ���ڳ� ���� �� ����, ���̺����� ����� ����/���� ���� �������� ������ ������ ���̸� ������
	result = DGBlankModalDialog (550, 950, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler1, 0);

	if (result != DG_OK)
		return err;

	// [DIALOG] 2��° ���̾�α׿��� ���纰 ���̾ ������
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32519, ACAPI_GetOwnResModule (), wallTableformPlacerHandler2, 0);

	if (result != DG_OK)
		goto FIRST;

	// ��ü ��ġ�ϱ�
	placingZone.placeObjects (&placingZone);

	// [DIALOG] 4��° ���̾�α׿��� �� ����� ������ ������ �ٸ� �԰��� ���������� ��ü�� ������ �����ϴ�.
	iMarginSide = 1;
	result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler4, 0);
	if (placingZone.bExtra == true) {
		iMarginSide = 2;
		result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler4, 0);
	}

	// ��� ���� ä���
	placingZone.fillRestAreas (&placingZone);

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// �⺻ ������
WallTableformPlacingZone::WallTableformPlacingZone ()
{
	this->presetWidth_tableform [0]		= 2300;
	this->presetWidth_tableform [1]		= 2250;
	this->presetWidth_tableform [2]		= 2200;
	this->presetWidth_tableform [3]		= 2150;
	this->presetWidth_tableform [4]		= 2100;
	this->presetWidth_tableform [5]		= 2050;
	this->presetWidth_tableform [6]		= 2000;
	this->presetWidth_tableform [7]		= 1950;
	this->presetWidth_tableform [8]		= 1900;
	this->presetWidth_tableform [9]		= 1850;
	this->presetWidth_tableform [10]	= 1800;
	this->presetWidth_tableform [11]	= 1750;
	this->presetWidth_tableform [12]	= 1700;
	this->presetWidth_tableform [13]	= 1650;
	this->presetWidth_tableform [14]	= 1600;
	this->presetWidth_tableform [15]	= 1550;
	this->presetWidth_tableform [16]	= 1500;
	this->presetWidth_tableform [17]	= 1450;
	this->presetWidth_tableform [18]	= 1400;
	this->presetWidth_tableform [19]	= 1350;
	this->presetWidth_tableform [20]	= 1300;
	this->presetWidth_tableform [21]	= 1250;
	this->presetWidth_tableform [22]	= 1200;
	this->presetWidth_tableform [23]	= 1150;
	this->presetWidth_tableform [24]	= 1100;
	this->presetWidth_tableform [25]	= 1050;
	this->presetWidth_tableform [26]	= 1000;
	this->presetWidth_tableform [27]	= 950;
	this->presetWidth_tableform [28]	= 900;
	this->presetWidth_tableform [29]	= 850;
	this->presetWidth_tableform [30]	= 800;
	this->presetWidth_tableform [31]	= 750;
	this->presetWidth_tableform [32]	= 700;
	this->presetWidth_tableform [33]	= 650;
	this->presetWidth_tableform [34]	= 600;
	this->presetWidth_tableform [35]	= 500;
	this->presetWidth_tableform [36]	= 450;
	this->presetWidth_tableform [37]	= 400;
	this->presetWidth_tableform [38]	= 300;
	this->presetWidth_tableform [39]	= 200;

	this->presetHeight_tableform [0]	= 6000;
	this->presetHeight_tableform [1]	= 5700;
	this->presetHeight_tableform [2]	= 5400;
	this->presetHeight_tableform [3]	= 5100;
	this->presetHeight_tableform [4]	= 4800;
	this->presetHeight_tableform [5]	= 4500;
	this->presetHeight_tableform [6]	= 4200;
	this->presetHeight_tableform [7]	= 3900;
	this->presetHeight_tableform [8]	= 3600;
	this->presetHeight_tableform [9]	= 3300;
	this->presetHeight_tableform [10]	= 3000;
	this->presetHeight_tableform [11]	= 2700;
	this->presetHeight_tableform [12]	= 2400;
	this->presetHeight_tableform [13]	= 2100;
	this->presetHeight_tableform [14]	= 1800;
	this->presetHeight_tableform [15]	= 1500;

	this->presetWidth_euroform [0]		= 600;
	this->presetWidth_euroform [1]		= 500;
	this->presetWidth_euroform [2]		= 450;
	this->presetWidth_euroform [3]		= 400;
	this->presetWidth_euroform [4]		= 300;
	this->presetWidth_euroform [5]		= 200;
	this->presetWidth_euroform [6]		= 0;

	this->presetHeight_euroform [0]		= 1200;
	this->presetHeight_euroform [1]		= 900;
	this->presetHeight_euroform [2]		= 600;
	this->presetHeight_euroform [3]		= 0;

	this->presetWidth_config_vertical [0][0] = 4;	this->presetWidth_config_vertical [0][1] = 600;		this->presetWidth_config_vertical [0][2] = 600;		this->presetWidth_config_vertical [0][3] = 500;		this->presetWidth_config_vertical [0][4] = 600;		// 2300
	this->presetWidth_config_vertical [1][0] = 4;	this->presetWidth_config_vertical [1][1] = 600;		this->presetWidth_config_vertical [1][2] = 600;		this->presetWidth_config_vertical [1][3] = 450;		this->presetWidth_config_vertical [1][4] = 600;		// 2250
	this->presetWidth_config_vertical [2][0] = 4;	this->presetWidth_config_vertical [2][1] = 600;		this->presetWidth_config_vertical [2][2] = 600;		this->presetWidth_config_vertical [2][3] = 400;		this->presetWidth_config_vertical [2][4] = 600;		// 2200
	this->presetWidth_config_vertical [3][0] = 4;	this->presetWidth_config_vertical [3][1] = 600;		this->presetWidth_config_vertical [3][2] = 500;		this->presetWidth_config_vertical [3][3] = 450;		this->presetWidth_config_vertical [3][4] = 600;		// 2150
	this->presetWidth_config_vertical [4][0] = 4;	this->presetWidth_config_vertical [4][1] = 600;		this->presetWidth_config_vertical [4][2] = 600;		this->presetWidth_config_vertical [4][3] = 300;		this->presetWidth_config_vertical [4][4] = 600;		// 2100
	this->presetWidth_config_vertical [5][0] = 4;	this->presetWidth_config_vertical [5][1] = 600;		this->presetWidth_config_vertical [5][2] = 450;		this->presetWidth_config_vertical [5][3] = 400;		this->presetWidth_config_vertical [5][4] = 600;		// 2050
	this->presetWidth_config_vertical [6][0] = 4;	this->presetWidth_config_vertical [6][1] = 600;		this->presetWidth_config_vertical [6][2] = 600;		this->presetWidth_config_vertical [6][3] = 200;		this->presetWidth_config_vertical [6][4] = 600;		// 2000
	this->presetWidth_config_vertical [7][0] = 4;	this->presetWidth_config_vertical [7][1] = 600;		this->presetWidth_config_vertical [7][2] = 450;		this->presetWidth_config_vertical [7][3] = 300;		this->presetWidth_config_vertical [7][4] = 600;		// 1950
	this->presetWidth_config_vertical [8][0] = 4;	this->presetWidth_config_vertical [8][1] = 600;		this->presetWidth_config_vertical [8][2] = 500;		this->presetWidth_config_vertical [8][3] = 200;		this->presetWidth_config_vertical [8][4] = 600;		// 1900
	this->presetWidth_config_vertical [9][0] = 4;	this->presetWidth_config_vertical [9][1] = 600;		this->presetWidth_config_vertical [9][2] = 450;		this->presetWidth_config_vertical [9][3] = 200;		this->presetWidth_config_vertical [9][4] = 600;		// 1850
	this->presetWidth_config_vertical [10][0] = 3;	this->presetWidth_config_vertical [10][1] = 600;	this->presetWidth_config_vertical [10][2] = 600;	this->presetWidth_config_vertical [10][3] = 600;	this->presetWidth_config_vertical [10][4] = 0;		// 1800
	this->presetWidth_config_vertical [11][0] = 4;	this->presetWidth_config_vertical [11][1] = 600;	this->presetWidth_config_vertical [11][2] = 200;	this->presetWidth_config_vertical [11][3] = 450;	this->presetWidth_config_vertical [11][4] = 500;	// 1750
	this->presetWidth_config_vertical [12][0] = 3;	this->presetWidth_config_vertical [12][1] = 600;	this->presetWidth_config_vertical [12][2] = 500;	this->presetWidth_config_vertical [12][3] = 600;	this->presetWidth_config_vertical [12][4] = 0;		// 1700
	this->presetWidth_config_vertical [13][0] = 3;	this->presetWidth_config_vertical [13][1] = 600;	this->presetWidth_config_vertical [13][2] = 450;	this->presetWidth_config_vertical [13][3] = 600;	this->presetWidth_config_vertical [13][4] = 0;		// 1650
	this->presetWidth_config_vertical [14][0] = 3;	this->presetWidth_config_vertical [14][1] = 600;	this->presetWidth_config_vertical [14][2] = 400;	this->presetWidth_config_vertical [14][3] = 600;	this->presetWidth_config_vertical [14][4] = 0;		// 1600
	this->presetWidth_config_vertical [15][0] = 3;	this->presetWidth_config_vertical [15][1] = 600;	this->presetWidth_config_vertical [15][2] = 450;	this->presetWidth_config_vertical [15][3] = 500;	this->presetWidth_config_vertical [15][4] = 0;		// 1550
	this->presetWidth_config_vertical [16][0] = 3;	this->presetWidth_config_vertical [16][1] = 600;	this->presetWidth_config_vertical [16][2] = 300;	this->presetWidth_config_vertical [16][3] = 600;	this->presetWidth_config_vertical [16][4] = 0;		// 1500
	this->presetWidth_config_vertical [17][0] = 3;	this->presetWidth_config_vertical [17][1] = 500;	this->presetWidth_config_vertical [17][2] = 450;	this->presetWidth_config_vertical [17][3] = 500;	this->presetWidth_config_vertical [17][4] = 0;		// 1450
	this->presetWidth_config_vertical [18][0] = 3;	this->presetWidth_config_vertical [18][1] = 500;	this->presetWidth_config_vertical [18][2] = 400;	this->presetWidth_config_vertical [18][3] = 500;	this->presetWidth_config_vertical [18][4] = 0;		// 1400
	this->presetWidth_config_vertical [19][0] = 3;	this->presetWidth_config_vertical [19][1] = 600;	this->presetWidth_config_vertical [19][2] = 300;	this->presetWidth_config_vertical [19][3] = 450;	this->presetWidth_config_vertical [19][4] = 0;		// 1350
	this->presetWidth_config_vertical [20][0] = 3;	this->presetWidth_config_vertical [20][1] = 600;	this->presetWidth_config_vertical [20][2] = 200;	this->presetWidth_config_vertical [20][3] = 500;	this->presetWidth_config_vertical [20][4] = 0;		// 1300
	this->presetWidth_config_vertical [21][0] = 3;	this->presetWidth_config_vertical [21][1] = 600;	this->presetWidth_config_vertical [21][2] = 200;	this->presetWidth_config_vertical [21][3] = 450;	this->presetWidth_config_vertical [21][4] = 0;		// 1250
	this->presetWidth_config_vertical [22][0] = 2;	this->presetWidth_config_vertical [22][1] = 600;	this->presetWidth_config_vertical [22][2] = 600;	this->presetWidth_config_vertical [22][3] = 0;		this->presetWidth_config_vertical [22][4] = 0;		// 1200
	this->presetWidth_config_vertical [23][0] = 3;	this->presetWidth_config_vertical [23][1] = 450;	this->presetWidth_config_vertical [23][2] = 300;	this->presetWidth_config_vertical [23][3] = 400;	this->presetWidth_config_vertical [23][4] = 0;		// 1150
	this->presetWidth_config_vertical [24][0] = 3;	this->presetWidth_config_vertical [24][1] = 400;	this->presetWidth_config_vertical [24][2] = 300;	this->presetWidth_config_vertical [24][3] = 400;	this->presetWidth_config_vertical [24][4] = 0;		// 1100
	this->presetWidth_config_vertical [25][0] = 3;	this->presetWidth_config_vertical [25][1] = 450;	this->presetWidth_config_vertical [25][2] = 300;	this->presetWidth_config_vertical [25][3] = 300;	this->presetWidth_config_vertical [25][4] = 0;		// 1050
	this->presetWidth_config_vertical [26][0] = 2;	this->presetWidth_config_vertical [26][1] = 600;	this->presetWidth_config_vertical [26][2] = 400;	this->presetWidth_config_vertical [26][3] = 0;		this->presetWidth_config_vertical [26][4] = 0;		// 1000
	this->presetWidth_config_vertical [27][0] = 2;	this->presetWidth_config_vertical [27][1] = 450;	this->presetWidth_config_vertical [27][2] = 500;	this->presetWidth_config_vertical [27][3] = 0;		this->presetWidth_config_vertical [27][4] = 0;		// 950
	this->presetWidth_config_vertical [28][0] = 2;	this->presetWidth_config_vertical [28][1] = 600;	this->presetWidth_config_vertical [28][2] = 300;	this->presetWidth_config_vertical [28][3] = 0;		this->presetWidth_config_vertical [28][4] = 0;		// 900
	this->presetWidth_config_vertical [29][0] = 2;	this->presetWidth_config_vertical [29][1] = 400;	this->presetWidth_config_vertical [29][2] = 450;	this->presetWidth_config_vertical [29][3] = 0;		this->presetWidth_config_vertical [29][4] = 0;		// 850
	this->presetWidth_config_vertical [30][0] = 2;	this->presetWidth_config_vertical [30][1] = 400;	this->presetWidth_config_vertical [30][2] = 400;	this->presetWidth_config_vertical [30][3] = 0;		this->presetWidth_config_vertical [30][4] = 0;		// 800
	this->presetWidth_config_vertical [31][0] = 2;	this->presetWidth_config_vertical [31][1] = 450;	this->presetWidth_config_vertical [31][2] = 300;	this->presetWidth_config_vertical [31][3] = 0;		this->presetWidth_config_vertical [31][4] = 0;		// 750
	this->presetWidth_config_vertical [32][0] = 2;	this->presetWidth_config_vertical [32][1] = 400;	this->presetWidth_config_vertical [32][2] = 300;	this->presetWidth_config_vertical [32][3] = 0;		this->presetWidth_config_vertical [32][4] = 0;		// 700
	this->presetWidth_config_vertical [33][0] = 2;	this->presetWidth_config_vertical [33][1] = 450;	this->presetWidth_config_vertical [33][2] = 200;	this->presetWidth_config_vertical [33][3] = 0;		this->presetWidth_config_vertical [33][4] = 0;		// 650
	this->presetWidth_config_vertical [34][0] = 1;	this->presetWidth_config_vertical [34][1] = 600;	this->presetWidth_config_vertical [34][2] = 0;		this->presetWidth_config_vertical [34][3] = 0;		this->presetWidth_config_vertical [34][4] = 0;		// 600
	this->presetWidth_config_vertical [35][0] = 1;	this->presetWidth_config_vertical [35][1] = 500;	this->presetWidth_config_vertical [35][2] = 0;		this->presetWidth_config_vertical [35][3] = 0;		this->presetWidth_config_vertical [35][4] = 0;		// 500
	this->presetWidth_config_vertical [36][0] = 1;	this->presetWidth_config_vertical [36][1] = 450;	this->presetWidth_config_vertical [36][2] = 0;		this->presetWidth_config_vertical [36][3] = 0;		this->presetWidth_config_vertical [36][4] = 0;		// 450
	this->presetWidth_config_vertical [37][0] = 1;	this->presetWidth_config_vertical [37][1] = 400;	this->presetWidth_config_vertical [37][2] = 0;		this->presetWidth_config_vertical [37][3] = 0;		this->presetWidth_config_vertical [37][4] = 0;		// 400
	this->presetWidth_config_vertical [38][0] = 1;	this->presetWidth_config_vertical [38][1] = 300;	this->presetWidth_config_vertical [38][2] = 0;		this->presetWidth_config_vertical [38][3] = 0;		this->presetWidth_config_vertical [38][4] = 0;		// 300
	this->presetWidth_config_vertical [39][0] = 1;	this->presetWidth_config_vertical [39][1] = 200;	this->presetWidth_config_vertical [39][2] = 0;		this->presetWidth_config_vertical [39][3] = 0;		this->presetWidth_config_vertical [39][4] = 0;		// 200

	this->presetHeight_config_vertical [0][0] = 5;	this->presetHeight_config_vertical [0][1] = 1200;	this->presetHeight_config_vertical [0][2] = 1200;	this->presetHeight_config_vertical [0][3] = 1200;	this->presetHeight_config_vertical [0][4] = 1200;	this->presetHeight_config_vertical [0][5] = 1200;	// 6000
	this->presetHeight_config_vertical [1][0] = 5;	this->presetHeight_config_vertical [1][1] = 1200;	this->presetHeight_config_vertical [1][2] = 1200;	this->presetHeight_config_vertical [1][3] = 1200;	this->presetHeight_config_vertical [1][4] = 1200;	this->presetHeight_config_vertical [1][5] = 900;	// 5700
	this->presetHeight_config_vertical [2][0] = 5;	this->presetHeight_config_vertical [2][1] = 1200;	this->presetHeight_config_vertical [2][2] = 1200;	this->presetHeight_config_vertical [2][3] = 1200;	this->presetHeight_config_vertical [2][4] = 900;	this->presetHeight_config_vertical [2][5] = 900;	// 5400
	this->presetHeight_config_vertical [3][0] = 5;	this->presetHeight_config_vertical [3][1] = 1200;	this->presetHeight_config_vertical [3][2] = 1200;	this->presetHeight_config_vertical [3][3] = 1200;	this->presetHeight_config_vertical [3][4] = 900;	this->presetHeight_config_vertical [3][5] = 600;	// 5100
	this->presetHeight_config_vertical [4][0] = 4;	this->presetHeight_config_vertical [4][1] = 1200;	this->presetHeight_config_vertical [4][2] = 1200;	this->presetHeight_config_vertical [4][3] = 1200;	this->presetHeight_config_vertical [4][4] = 1200;	this->presetHeight_config_vertical [4][5] = 0;		// 4800
	this->presetHeight_config_vertical [5][0] = 4;	this->presetHeight_config_vertical [5][1] = 1200;	this->presetHeight_config_vertical [5][2] = 1200;	this->presetHeight_config_vertical [5][3] = 1200;	this->presetHeight_config_vertical [5][4] = 900;	this->presetHeight_config_vertical [5][5] = 0;		// 4500
	this->presetHeight_config_vertical [6][0] = 4;	this->presetHeight_config_vertical [6][1] = 1200;	this->presetHeight_config_vertical [6][2] = 1200;	this->presetHeight_config_vertical [6][3] = 900;	this->presetHeight_config_vertical [6][4] = 900;	this->presetHeight_config_vertical [6][5] = 0;		// 4200
	this->presetHeight_config_vertical [7][0] = 4;	this->presetHeight_config_vertical [7][1] = 1200;	this->presetHeight_config_vertical [7][2] = 1200;	this->presetHeight_config_vertical [7][3] = 900;	this->presetHeight_config_vertical [7][4] = 600;	this->presetHeight_config_vertical [7][5] = 0;		// 3900
	this->presetHeight_config_vertical [8][0] = 3;	this->presetHeight_config_vertical [8][1] = 1200;	this->presetHeight_config_vertical [8][2] = 1200;	this->presetHeight_config_vertical [8][3] = 1200;	this->presetHeight_config_vertical [8][4] = 0;		this->presetHeight_config_vertical [8][5] = 0;		// 3600
	this->presetHeight_config_vertical [9][0] = 3;	this->presetHeight_config_vertical [9][1] = 1200;	this->presetHeight_config_vertical [9][2] = 1200;	this->presetHeight_config_vertical [9][3] = 900;	this->presetHeight_config_vertical [9][4] = 0;		this->presetHeight_config_vertical [9][5] = 0;		// 3300
	this->presetHeight_config_vertical [10][0] = 3;	this->presetHeight_config_vertical [10][1] = 1200;	this->presetHeight_config_vertical [10][2] = 1200;	this->presetHeight_config_vertical [10][3] = 600;	this->presetHeight_config_vertical [10][4] = 0;		this->presetHeight_config_vertical [10][5] = 0;		// 3000
	this->presetHeight_config_vertical [11][0] = 3;	this->presetHeight_config_vertical [11][1] = 1200;	this->presetHeight_config_vertical [11][2] = 900;	this->presetHeight_config_vertical [11][3] = 600;	this->presetHeight_config_vertical [11][4] = 0;		this->presetHeight_config_vertical [11][5] = 0;		// 2700
	this->presetHeight_config_vertical [12][0] = 2;	this->presetHeight_config_vertical [12][1] = 1200;	this->presetHeight_config_vertical [12][2] = 1200;	this->presetHeight_config_vertical [12][3] = 0;		this->presetHeight_config_vertical [12][4] = 0;		this->presetHeight_config_vertical [12][5] = 0;		// 2400
	this->presetHeight_config_vertical [13][0] = 2;	this->presetHeight_config_vertical [13][1] = 1200;	this->presetHeight_config_vertical [13][2] = 900;	this->presetHeight_config_vertical [13][3] = 0;		this->presetHeight_config_vertical [13][4] = 0;		this->presetHeight_config_vertical [13][5] = 0;		// 2100
	this->presetHeight_config_vertical [14][0] = 2;	this->presetHeight_config_vertical [14][1] = 900;	this->presetHeight_config_vertical [14][2] = 900;	this->presetHeight_config_vertical [14][3] = 0;		this->presetHeight_config_vertical [14][4] = 0;		this->presetHeight_config_vertical [14][5] = 0;		// 1800
	this->presetHeight_config_vertical [15][0] = 2;	this->presetHeight_config_vertical [15][1] = 900;	this->presetHeight_config_vertical [15][2] = 600;	this->presetHeight_config_vertical [15][3] = 0;		this->presetHeight_config_vertical [15][4] = 0;		this->presetHeight_config_vertical [15][5] = 0;		// 1500

	this->presetWidth_config_horizontal [0][0] = 5;	this->presetWidth_config_horizontal [0][1] = 1200;	this->presetWidth_config_horizontal [0][2] = 1200;	this->presetWidth_config_horizontal [0][3] = 1200;	this->presetWidth_config_horizontal [0][4] = 1200;	this->presetWidth_config_horizontal [0][5] = 1200;	// 6000
	this->presetWidth_config_horizontal [1][0] = 5;	this->presetWidth_config_horizontal [1][1] = 1200;	this->presetWidth_config_horizontal [1][2] = 1200;	this->presetWidth_config_horizontal [1][3] = 1200;	this->presetWidth_config_horizontal [1][4] = 1200;	this->presetWidth_config_horizontal [1][5] = 900;	// 5700
	this->presetWidth_config_horizontal [2][0] = 5;	this->presetWidth_config_horizontal [2][1] = 1200;	this->presetWidth_config_horizontal [2][2] = 1200;	this->presetWidth_config_horizontal [2][3] = 1200;	this->presetWidth_config_horizontal [2][4] = 900;	this->presetWidth_config_horizontal [2][5] = 900;	// 5400
	this->presetWidth_config_horizontal [3][0] = 5;	this->presetWidth_config_horizontal [3][1] = 1200;	this->presetWidth_config_horizontal [3][2] = 1200;	this->presetWidth_config_horizontal [3][3] = 1200;	this->presetWidth_config_horizontal [3][4] = 900;	this->presetWidth_config_horizontal [3][5] = 600;	// 5100
	this->presetWidth_config_horizontal [4][0] = 4;	this->presetWidth_config_horizontal [4][1] = 1200;	this->presetWidth_config_horizontal [4][2] = 1200;	this->presetWidth_config_horizontal [4][3] = 1200;	this->presetWidth_config_horizontal [4][4] = 1200;	this->presetWidth_config_horizontal [4][5] = 0;		// 4800
	this->presetWidth_config_horizontal [5][0] = 4;	this->presetWidth_config_horizontal [5][1] = 1200;	this->presetWidth_config_horizontal [5][2] = 1200;	this->presetWidth_config_horizontal [5][3] = 1200;	this->presetWidth_config_horizontal [5][4] = 900;	this->presetWidth_config_horizontal [5][5] = 0;		// 4500
	this->presetWidth_config_horizontal [6][0] = 4;	this->presetWidth_config_horizontal [6][1] = 1200;	this->presetWidth_config_horizontal [6][2] = 1200;	this->presetWidth_config_horizontal [6][3] = 900;	this->presetWidth_config_horizontal [6][4] = 900;	this->presetWidth_config_horizontal [6][5] = 0;		// 4200
	this->presetWidth_config_horizontal [7][0] = 4;	this->presetWidth_config_horizontal [7][1] = 1200;	this->presetWidth_config_horizontal [7][2] = 1200;	this->presetWidth_config_horizontal [7][3] = 900;	this->presetWidth_config_horizontal [7][4] = 600;	this->presetWidth_config_horizontal [7][5] = 0;		// 3900
	this->presetWidth_config_horizontal [8][0] = 3;	this->presetWidth_config_horizontal [8][1] = 1200;	this->presetWidth_config_horizontal [8][2] = 1200;	this->presetWidth_config_horizontal [8][3] = 1200;	this->presetWidth_config_horizontal [8][4] = 0;		this->presetWidth_config_horizontal [8][5] = 0;		// 3600
	this->presetWidth_config_horizontal [9][0] = 3;	this->presetWidth_config_horizontal [9][1] = 1200;	this->presetWidth_config_horizontal [9][2] = 1200;	this->presetWidth_config_horizontal [9][3] = 900;	this->presetWidth_config_horizontal [9][4] = 0;		this->presetWidth_config_horizontal [9][5] = 0;		// 3300
	this->presetWidth_config_horizontal [10][0] = 3;this->presetWidth_config_horizontal [10][1] = 1200;	this->presetWidth_config_horizontal [10][2] = 1200;	this->presetWidth_config_horizontal [10][3] = 600;	this->presetWidth_config_horizontal [10][4] = 0;	this->presetWidth_config_horizontal [10][5] = 0;	// 3000
	this->presetWidth_config_horizontal [11][0] = 3;this->presetWidth_config_horizontal [11][1] = 1200;	this->presetWidth_config_horizontal [11][2] = 900;	this->presetWidth_config_horizontal [11][3] = 600;	this->presetWidth_config_horizontal [11][4] = 0;	this->presetWidth_config_horizontal [11][5] = 0;	// 2700
	this->presetWidth_config_horizontal [12][0] = 2;this->presetWidth_config_horizontal [12][1] = 1200;	this->presetWidth_config_horizontal [12][2] = 1200;	this->presetWidth_config_horizontal [12][3] = 0;	this->presetWidth_config_horizontal [12][4] = 0;	this->presetWidth_config_horizontal [12][5] = 0;	// 2400
	this->presetWidth_config_horizontal [13][0] = 2;this->presetWidth_config_horizontal [13][1] = 1200;	this->presetWidth_config_horizontal [13][2] = 900;	this->presetWidth_config_horizontal [13][3] = 0;	this->presetWidth_config_horizontal [13][4] = 0;	this->presetWidth_config_horizontal [13][5] = 0;	// 2100
	this->presetWidth_config_horizontal [14][0] = 2;this->presetWidth_config_horizontal [14][1] = 900;	this->presetWidth_config_horizontal [14][2] = 900;	this->presetWidth_config_horizontal [14][3] = 0;	this->presetWidth_config_horizontal [14][4] = 0;	this->presetWidth_config_horizontal [14][5] = 0;	// 1800
	this->presetWidth_config_horizontal [15][0] = 2;this->presetWidth_config_horizontal [15][1] = 900;	this->presetWidth_config_horizontal [15][2] = 600;	this->presetWidth_config_horizontal [15][3] = 0;	this->presetWidth_config_horizontal [15][4] = 0;	this->presetWidth_config_horizontal [15][5] = 0;	// 1500

	this->presetHeight_config_horizontal [0][0] = 4;	this->presetHeight_config_horizontal [0][1] = 600;	this->presetHeight_config_horizontal [0][2] = 600;	this->presetHeight_config_horizontal [0][3] = 600;	this->presetHeight_config_horizontal [0][4] = 500;		// 2300
	this->presetHeight_config_horizontal [1][0] = 4;	this->presetHeight_config_horizontal [1][1] = 600;	this->presetHeight_config_horizontal [1][2] = 600;	this->presetHeight_config_horizontal [1][3] = 450;	this->presetHeight_config_horizontal [1][4] = 600;		// 2250
	this->presetHeight_config_horizontal [2][0] = 4;	this->presetHeight_config_horizontal [2][1] = 600;	this->presetHeight_config_horizontal [2][2] = 600;	this->presetHeight_config_horizontal [2][3] = 600;	this->presetHeight_config_horizontal [2][4] = 400;		// 2200
	this->presetHeight_config_horizontal [3][0] = 4;	this->presetHeight_config_horizontal [3][1] = 600;	this->presetHeight_config_horizontal [3][2] = 450;	this->presetHeight_config_horizontal [3][3] = 600;	this->presetHeight_config_horizontal [3][4] = 500;		// 2150
	this->presetHeight_config_horizontal [4][0] = 4;	this->presetHeight_config_horizontal [4][1] = 600;	this->presetHeight_config_horizontal [4][2] = 300;	this->presetHeight_config_horizontal [4][3] = 600;	this->presetHeight_config_horizontal [4][4] = 600;		// 2100
	this->presetHeight_config_horizontal [5][0] = 4;	this->presetHeight_config_horizontal [5][1] = 600;	this->presetHeight_config_horizontal [5][2] = 600;	this->presetHeight_config_horizontal [5][3] = 450;	this->presetHeight_config_horizontal [5][4] = 400;		// 2050
	this->presetHeight_config_horizontal [6][0] = 4;	this->presetHeight_config_horizontal [6][1] = 600;	this->presetHeight_config_horizontal [6][2] = 600;	this->presetHeight_config_horizontal [6][3] = 600;	this->presetHeight_config_horizontal [6][4] = 200;		// 2000
	this->presetHeight_config_horizontal [7][0] = 4;	this->presetHeight_config_horizontal [7][1] = 600;	this->presetHeight_config_horizontal [7][2] = 300;	this->presetHeight_config_horizontal [7][3] = 450;	this->presetHeight_config_horizontal [7][4] = 600;		// 1950
	this->presetHeight_config_horizontal [8][0] = 4;	this->presetHeight_config_horizontal [8][1] = 600;	this->presetHeight_config_horizontal [8][2] = 600;	this->presetHeight_config_horizontal [8][3] = 200;	this->presetHeight_config_horizontal [8][4] = 500;		// 1900
	this->presetHeight_config_horizontal [9][0] = 4;	this->presetHeight_config_horizontal [9][1] = 600;	this->presetHeight_config_horizontal [9][2] = 600;	this->presetHeight_config_horizontal [9][3] = 450;	this->presetHeight_config_horizontal [9][4] = 200;		// 1850
	this->presetHeight_config_horizontal [10][0] = 3;	this->presetHeight_config_horizontal [10][1] = 600;	this->presetHeight_config_horizontal [10][2] = 600;	this->presetHeight_config_horizontal [10][3] = 600;	this->presetHeight_config_horizontal [10][4] = 0;		// 1800
	this->presetHeight_config_horizontal [11][0] = 4;	this->presetHeight_config_horizontal [11][1] = 600;	this->presetHeight_config_horizontal [11][2] = 450;	this->presetHeight_config_horizontal [11][3] = 200;	this->presetHeight_config_horizontal [11][4] = 500;		// 1750
	this->presetHeight_config_horizontal [12][0] = 3;	this->presetHeight_config_horizontal [12][1] = 600;	this->presetHeight_config_horizontal [12][2] = 600;	this->presetHeight_config_horizontal [12][3] = 500;	this->presetHeight_config_horizontal [12][4] = 0;		// 1700
	this->presetHeight_config_horizontal [13][0] = 3;	this->presetHeight_config_horizontal [13][1] = 600;	this->presetHeight_config_horizontal [13][2] = 450;	this->presetHeight_config_horizontal [13][3] = 600;	this->presetHeight_config_horizontal [13][4] = 0;		// 1650
	this->presetHeight_config_horizontal [14][0] = 3;	this->presetHeight_config_horizontal [14][1] = 600;	this->presetHeight_config_horizontal [14][2] = 600;	this->presetHeight_config_horizontal [14][3] = 400;	this->presetHeight_config_horizontal [14][4] = 0;		// 1600
	this->presetHeight_config_horizontal [15][0] = 3;	this->presetHeight_config_horizontal [15][1] = 600;	this->presetHeight_config_horizontal [15][2] = 450;	this->presetHeight_config_horizontal [15][3] = 500;	this->presetHeight_config_horizontal [15][4] = 0;		// 1550
	this->presetHeight_config_horizontal [16][0] = 3;	this->presetHeight_config_horizontal [16][1] = 600;	this->presetHeight_config_horizontal [16][2] = 300;	this->presetHeight_config_horizontal [16][3] = 600;	this->presetHeight_config_horizontal [16][4] = 0;		// 1500
	this->presetHeight_config_horizontal [17][0] = 3;	this->presetHeight_config_horizontal [17][1] = 600;	this->presetHeight_config_horizontal [17][2] = 450;	this->presetHeight_config_horizontal [17][3] = 400;	this->presetHeight_config_horizontal [17][4] = 0;		// 1450
	this->presetHeight_config_horizontal [18][0] = 3;	this->presetHeight_config_horizontal [18][1] = 600;	this->presetHeight_config_horizontal [18][2] = 300;	this->presetHeight_config_horizontal [18][3] = 500;	this->presetHeight_config_horizontal [18][4] = 0;		// 1400
	this->presetHeight_config_horizontal [19][0] = 3;	this->presetHeight_config_horizontal [19][1] = 450;	this->presetHeight_config_horizontal [19][2] = 300;	this->presetHeight_config_horizontal [19][3] = 600;	this->presetHeight_config_horizontal [19][4] = 0;		// 1350
	this->presetHeight_config_horizontal [20][0] = 3;	this->presetHeight_config_horizontal [20][1] = 600;	this->presetHeight_config_horizontal [20][2] = 200;	this->presetHeight_config_horizontal [20][3] = 500;	this->presetHeight_config_horizontal [20][4] = 0;		// 1300
	this->presetHeight_config_horizontal [21][0] = 3;	this->presetHeight_config_horizontal [21][1] = 450;	this->presetHeight_config_horizontal [21][2] = 600;	this->presetHeight_config_horizontal [21][3] = 200;	this->presetHeight_config_horizontal [21][4] = 0;		// 1250
	this->presetHeight_config_horizontal [22][0] = 2;	this->presetHeight_config_horizontal [22][1] = 600;	this->presetHeight_config_horizontal [22][2] = 600;	this->presetHeight_config_horizontal [22][3] = 0;	this->presetHeight_config_horizontal [22][4] = 0;		// 1200
	this->presetHeight_config_horizontal [23][0] = 3;	this->presetHeight_config_horizontal [23][1] = 450;	this->presetHeight_config_horizontal [23][2] = 300;	this->presetHeight_config_horizontal [23][3] = 400;	this->presetHeight_config_horizontal [23][4] = 0;		// 1150
	this->presetHeight_config_horizontal [24][0] = 2;	this->presetHeight_config_horizontal [24][1] = 600;	this->presetHeight_config_horizontal [24][2] = 500;	this->presetHeight_config_horizontal [24][3] = 0;	this->presetHeight_config_horizontal [24][4] = 0;		// 1100
	this->presetHeight_config_horizontal [25][0] = 3;	this->presetHeight_config_horizontal [25][1] = 300;	this->presetHeight_config_horizontal [25][2] = 300;	this->presetHeight_config_horizontal [25][3] = 450;	this->presetHeight_config_horizontal [25][4] = 0;		// 1050
	this->presetHeight_config_horizontal [26][0] = 2;	this->presetHeight_config_horizontal [26][1] = 600;	this->presetHeight_config_horizontal [26][2] = 400;	this->presetHeight_config_horizontal [26][3] = 0;	this->presetHeight_config_horizontal [26][4] = 0;		// 1000
	this->presetHeight_config_horizontal [27][0] = 2;	this->presetHeight_config_horizontal [27][1] = 450;	this->presetHeight_config_horizontal [27][2] = 500;	this->presetHeight_config_horizontal [27][3] = 0;	this->presetHeight_config_horizontal [27][4] = 0;		// 950
	this->presetHeight_config_horizontal [28][0] = 2;	this->presetHeight_config_horizontal [28][1] = 600;	this->presetHeight_config_horizontal [28][2] = 300;	this->presetHeight_config_horizontal [28][3] = 0;	this->presetHeight_config_horizontal [28][4] = 0;		// 900
	this->presetHeight_config_horizontal [29][0] = 2;	this->presetHeight_config_horizontal [29][1] = 450;	this->presetHeight_config_horizontal [29][2] = 400;	this->presetHeight_config_horizontal [29][3] = 0;	this->presetHeight_config_horizontal [29][4] = 0;		// 850
	this->presetHeight_config_horizontal [30][0] = 2;	this->presetHeight_config_horizontal [30][1] = 400;	this->presetHeight_config_horizontal [30][2] = 400;	this->presetHeight_config_horizontal [30][3] = 0;	this->presetHeight_config_horizontal [30][4] = 0;		// 800
	this->presetHeight_config_horizontal [31][0] = 2;	this->presetHeight_config_horizontal [31][1] = 300;	this->presetHeight_config_horizontal [31][2] = 450;	this->presetHeight_config_horizontal [31][3] = 0;	this->presetHeight_config_horizontal [31][4] = 0;		// 750
	this->presetHeight_config_horizontal [32][0] = 2;	this->presetHeight_config_horizontal [32][1] = 300;	this->presetHeight_config_horizontal [32][2] = 400;	this->presetHeight_config_horizontal [32][3] = 0;	this->presetHeight_config_horizontal [32][4] = 0;		// 700
	this->presetHeight_config_horizontal [33][0] = 2;	this->presetHeight_config_horizontal [33][1] = 450;	this->presetHeight_config_horizontal [33][2] = 200;	this->presetHeight_config_horizontal [33][3] = 0;	this->presetHeight_config_horizontal [33][4] = 0;		// 650
	this->presetHeight_config_horizontal [34][0] = 1;	this->presetHeight_config_horizontal [34][1] = 600;	this->presetHeight_config_horizontal [34][2] = 0;	this->presetHeight_config_horizontal [34][3] = 0;	this->presetHeight_config_horizontal [34][4] = 0;		// 600
	this->presetHeight_config_horizontal [35][0] = 1;	this->presetHeight_config_horizontal [35][1] = 500;	this->presetHeight_config_horizontal [35][2] = 0;	this->presetHeight_config_horizontal [35][3] = 0;	this->presetHeight_config_horizontal [35][4] = 0;		// 500
	this->presetHeight_config_horizontal [36][0] = 1;	this->presetHeight_config_horizontal [36][1] = 450;	this->presetHeight_config_horizontal [36][2] = 0;	this->presetHeight_config_horizontal [36][3] = 0;	this->presetHeight_config_horizontal [36][4] = 0;		// 450
	this->presetHeight_config_horizontal [37][0] = 1;	this->presetHeight_config_horizontal [37][1] = 400;	this->presetHeight_config_horizontal [37][2] = 0;	this->presetHeight_config_horizontal [37][3] = 0;	this->presetHeight_config_horizontal [37][4] = 0;		// 400
	this->presetHeight_config_horizontal [38][0] = 1;	this->presetHeight_config_horizontal [38][1] = 300;	this->presetHeight_config_horizontal [38][2] = 0;	this->presetHeight_config_horizontal [38][3] = 0;	this->presetHeight_config_horizontal [38][4] = 0;		// 300
	this->presetHeight_config_horizontal [39][0] = 1;	this->presetHeight_config_horizontal [39][1] = 200;	this->presetHeight_config_horizontal [39][2] = 0;	this->presetHeight_config_horizontal [39][3] = 0;	this->presetHeight_config_horizontal [39][4] = 0;		// 200
}

// �� ���� �ʱ�ȭ
void	WallTableformPlacingZone::initCells (WallTableformPlacingZone* placingZone, bool bVertical)
{
	short	xx;

	// �� ���� ä��� (�� �ʺ� ������ �̸� ä��)
	// ���ι����̸�
	if (bVertical == true) {
		for (xx = 0 ; xx < sizeof (placingZone->cells) / sizeof (CellForWallTableform) ; ++xx) {
			placingZone->cells [xx].objType = TABLEFORM;
			placingZone->cells [xx].horLen = 2300;
			placingZone->cells [xx].tableInHor [0] = 600;
			placingZone->cells [xx].tableInHor [1] = 600;
			placingZone->cells [xx].tableInHor [2] = 500;
			placingZone->cells [xx].tableInHor [3] = 600;
		}

	// ���ι����̸�
	} else {
		for (xx = 0 ; xx < sizeof (placingZone->cells) / sizeof (CellForWallTableform) ; ++xx) {
			placingZone->cells [xx].objType = TABLEFORM;
			placingZone->cells [xx].horLen = 6000;
			placingZone->cells [xx].tableInHor [0] = 1200;
			placingZone->cells [xx].tableInHor [1] = 1200;
			placingZone->cells [xx].tableInHor [2] = 1200;
			placingZone->cells [xx].tableInHor [3] = 1200;
			placingZone->cells [xx].tableInHor [4] = 1200;
		}
	}
}

// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ����
double	WallTableformPlacingZone::getCellPositionLeftBottomX (WallTableformPlacingZone* placingZone, short idx)
{
	double	distance = (placingZone->bLincorner == true) ? placingZone->lenLincorner : 0;

	for (short xx = 0 ; xx < idx ; ++xx)
		distance += (double)placingZone->cells [xx].horLen / 1000.0;

	return distance;
}

// �� ��ġ�� �ٸ��� ������
void	WallTableformPlacingZone::adjustCellsPosition (WallTableformPlacingZone* placingZone)
{
	for (short xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->cells [xx].ang = placingZone->ang;
		placingZone->cells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cells [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cells [xx].leftBottomZ = placingZone->leftBottomZ;
	}
}

// �� ������ ������� ��ü���� ��ġ��
GSErrCode	WallTableformPlacingZone::placeObjects (WallTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy, varEnd;
	double	accumDist;
	double	lengthDouble;
	int		lengthInt;
	int		remainLengthInt;

	// ================================================== ���ڳ� ��ġ
	// ���� ���ڳ� ��ġ
	if (placingZone->bLincorner == true) {
		// �ո�
		EasyObjectPlacement incorner;
		incorner.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (90.0));

		moveIn3D ('y', incorner.radAng + DegreeToRad (90.0), -placingZone->gap, &incorner.posX, &incorner.posY, &incorner.posZ);	// ������ ���ݸ�ŭ �̵�

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
			elemList_Front.Push (incorner.placeObject (5,
				"in_comp", APIParT_CString, "���ڳ��ǳ�",
				"wid_s", APIParT_Length, format_string ("%.3f", 0.100),
				"leng_s", APIParT_Length, format_string ("%.3f", placingZone->lenLincorner),
				"hei_s", APIParT_Length, format_string ("%.3f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
				"dir_s", APIParT_CString, "�����"));

			moveIn3D ('z', incorner.radAng + DegreeToRad (90.0), (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &incorner.posX, &incorner.posY, &incorner.posZ);
		}

		// �޸�
		incorner.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

		moveIn3D ('y', incorner.radAng, infoWall.wallThk + placingZone->gap, &incorner.posX, &incorner.posY, &incorner.posZ);		// ������ ���ݸ�ŭ �̵�

		varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

		for (xx = 0 ; xx < varEnd ; ++xx) {
			if (placingZone->bExtra == true)
				lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
			else
				lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

			elemList_Back.Push (incorner.placeObject (5,
				"in_comp", APIParT_CString, "���ڳ��ǳ�",
				"wid_s", APIParT_Length, format_string ("%.3f", placingZone->lenLincorner),
				"leng_s", APIParT_Length, format_string ("%.3f", 0.100),
				"hei_s", APIParT_Length, format_string ("%.3f", lengthDouble),
				"dir_s", APIParT_CString, "�����"));

			moveIn3D ('z', incorner.radAng, lengthDouble, &incorner.posX, &incorner.posY, &incorner.posZ);
		}
	}
	
	// ����� ��ü �׷�ȭ (�ո�)
	if (!elemList_Front.IsEmpty ()) {
		GSSize nElems = elemList_Front.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList_Front [i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
		elemList_Front.Clear ();
	}

	// ����� ��ü �׷�ȭ (�޸�)
	if (!elemList_Back.IsEmpty ()) {
		GSSize nElems = elemList_Back.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList_Back [i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
		elemList_Back.Clear ();
	}

	// ���� ���ڳ� ��ġ
	if (placingZone->bRincorner == true) {
		// �ո�
		EasyObjectPlacement incorner;
		incorner.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (180.0));

		moveIn3D ('y', incorner.radAng + DegreeToRad (180.0), -placingZone->gap, &incorner.posX, &incorner.posY, &incorner.posZ);		// ������ ���ݸ�ŭ �̵�
		moveIn3D ('x', incorner.radAng + DegreeToRad (180.0), placingZone->horLen, &incorner.posX, &incorner.posY, &incorner.posZ);		// ���� �������� �̵�

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
			elemList_Front.Push (incorner.placeObject (5,
				"in_comp", APIParT_CString, "���ڳ��ǳ�",
				"wid_s", APIParT_Length, format_string ("%.3f", placingZone->lenRincorner),
				"leng_s", APIParT_Length, format_string ("%.3f", 0.100),
				"hei_s", APIParT_Length, format_string ("%.3f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
				"dir_s", APIParT_CString, "�����"));

			moveIn3D ('z', incorner.radAng + DegreeToRad (180.0), (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &incorner.posX, &incorner.posY, &incorner.posZ);
		}

		// �޸�
		incorner.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (90.0));

		moveIn3D ('y', incorner.radAng - DegreeToRad (90.0), infoWall.wallThk + placingZone->gap, &incorner.posX, &incorner.posY, &incorner.posZ);	// ������ ���ݸ�ŭ �̵�
		moveIn3D ('x', incorner.radAng - DegreeToRad (90.0), placingZone->horLen, &incorner.posX, &incorner.posY, &incorner.posZ);					// ���� �������� �̵�

		varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

		for (xx = 0 ; xx < varEnd ; ++xx) {
			if (placingZone->bExtra == true)
				lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
			else
				lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

			elemList_Back.Push (incorner.placeObject (5,
				"in_comp", APIParT_CString, "���ڳ��ǳ�",
				"wid_s", APIParT_Length, format_string ("%.3f", 0.100),
				"leng_s", APIParT_Length, format_string ("%.3f", placingZone->lenRincorner),
				"hei_s", APIParT_Length, format_string ("%.3f", lengthDouble),
				"dir_s", APIParT_CString, "�����"));

			moveIn3D ('z', incorner.radAng, lengthDouble, &incorner.posX, &incorner.posY, &incorner.posZ);
		}
	}

	// ����� ��ü �׷�ȭ (�ո�)
	if (!elemList_Front.IsEmpty ()) {
		GSSize nElems = elemList_Front.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList_Front [i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
		elemList_Front.Clear ();
	}

	// ����� ��ü �׷�ȭ (�޸�)
	if (!elemList_Back.IsEmpty ()) {
		GSSize nElems = elemList_Back.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList_Back [i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
		elemList_Back.Clear ();
	}

	// ================================================== ������ ��ġ
	// �ո� ��ġ
	EasyObjectPlacement euroform;
	euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	if (placingZone->bLincorner == true)	moveIn3D ('x', euroform.radAng, placingZone->lenLincorner, &euroform.posX, &euroform.posY, &euroform.posZ);		// ���� ���ڳ� ������ x �̵�
	moveIn3D ('y', euroform.radAng, -placingZone->gap, &euroform.posX, &euroform.posY, &euroform.posZ);														// ������ ���ݸ�ŭ �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == EUROFORM) {
			accumDist = 0.0;

			for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
				if (placingZone->bVertical == true) {
					// ���ι���
					elemList_Front.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].tableInVerBasic [yy]),
						"u_ins", APIParT_CString, "�������",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
				} else {
					// ���ι���
					moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					elemList_Front.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].tableInVerBasic [yy]),
						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
						"u_ins", APIParT_CString, "��������",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
					moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
				}
				accumDist += (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0;
				moveIn3D ('z', euroform.radAng, (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}
			moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);

			// ����� ��ü �׷�ȭ (�ո�)
			if (!elemList_Front.IsEmpty ()) {
				GSSize nElems = elemList_Front.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList_Front [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList_Front.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	}

	// �޸� ��ġ
	euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

	if (placingZone->bLincorner == true)	moveIn3D ('x', euroform.radAng - DegreeToRad (180.0), placingZone->lenLincorner, &euroform.posX, &euroform.posY, &euroform.posZ);	// ���� ���ڳ� ������ x �̵�
	moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &euroform.posX, &euroform.posY, &euroform.posZ);									// ������ ���ݸ�ŭ �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == EUROFORM) {
			accumDist = 0.0;

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (yy = 0 ; yy < varEnd ; ++yy) {
				if (placingZone->bExtra == true)
					lengthInt = placingZone->cells [xx].tableInVerExtra [yy];
				else
					lengthInt = placingZone->cells [xx].tableInVerBasic [yy];

				if (placingZone->bVertical == true) {
					// ���ι���
					moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					elemList_Back.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
						"eu_hei", APIParT_CString, format_string ("%d", lengthInt),
						"u_ins", APIParT_CString, "�������",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
					moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
				} else {
					// ���ι���
					elemList_Back.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", lengthInt),
						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
						"u_ins", APIParT_CString, "��������",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
				}
				accumDist += (double)lengthInt / 1000.0;
				moveIn3D ('z', euroform.radAng, (double)lengthInt / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}
			moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);

			// ����� ��ü �׷�ȭ (�޸�)
			if (!elemList_Back.IsEmpty ()) {
				GSSize nElems = elemList_Back.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList_Back [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList_Back.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	}

	// ================================================== �ٷ������̼� ��ġ (�׻� ���ι���)
	// �ո� ��ġ
	EasyObjectPlacement fillersp;
	fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillersp, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	if (placingZone->bLincorner == true)	moveIn3D ('x', fillersp.radAng, placingZone->lenLincorner, &fillersp.posX, &fillersp.posY, &fillersp.posZ);		// ���� ���ڳ� ������ x �̵�
	moveIn3D ('y', fillersp.radAng, -placingZone->gap, &fillersp.posX, &fillersp.posY, &fillersp.posZ);														// ������ ���ݸ�ŭ �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == FILLERSP) {
			accumDist = 0.0;
			remainLengthInt = 0;
			for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
				remainLengthInt += placingZone->cells [xx].tableInVerBasic [yy];
				accumDist += (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0;
			}

			moveIn3D ('x', fillersp.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
			while (remainLengthInt > 0) {
				if (remainLengthInt >= 2400)
					lengthInt = 2400;
				else
					lengthInt = remainLengthInt;

				elemList_Front.Push (fillersp.placeObject (4,
					"f_thk", APIParT_Length, format_string ("%.3f", (double)placingZone->cells [xx].horLen / 1000.0),
					"f_leng", APIParT_Length, format_string ("%.3f", (double)lengthInt / 1000.0),
					"f_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)),
					"f_rota", APIParT_Angle, format_string ("%.3f", 0.0)));
				moveIn3D ('z', fillersp.radAng, (double)lengthInt / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

				remainLengthInt -= 2400;
			}
			moveIn3D ('x', fillersp.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
			moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

			// ����� ��ü �׷�ȭ (�ո�)
			if (!elemList_Front.IsEmpty ()) {
				GSSize nElems = elemList_Front.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList_Front [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList_Front.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', fillersp.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	}

	// �޸� ��ġ
	fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillersp, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

	if (placingZone->bLincorner == true)	moveIn3D ('x', fillersp.radAng - DegreeToRad (180.0), placingZone->lenLincorner, &fillersp.posX, &fillersp.posY, &fillersp.posZ);	// ���� ���ڳ� ������ x �̵�
	moveIn3D ('y', fillersp.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &fillersp.posX, &fillersp.posY, &fillersp.posZ);									// ������ ���ݸ�ŭ �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == FILLERSP) {
			accumDist = 0.0;
			remainLengthInt = 0;

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (yy = 0 ; yy < varEnd ; ++yy) {
				if (placingZone->bExtra == true)
					lengthInt = placingZone->cells [xx].tableInVerExtra [yy];
				else
					lengthInt = placingZone->cells [xx].tableInVerBasic [yy];

				remainLengthInt += lengthInt;
				accumDist += (double)lengthInt / 1000.0;
			}

			while (remainLengthInt > 0) {
				if (remainLengthInt >= 2400)
					lengthInt = 2400;
				else
					lengthInt = remainLengthInt;

				elemList_Back.Push (fillersp.placeObject (4,
					"f_thk", APIParT_Length, format_string ("%.3f", (double)placingZone->cells [xx].horLen / 1000.0),
					"f_leng", APIParT_Length, format_string ("%.3f", (double)lengthInt / 1000.0),
					"f_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)),
					"f_rota", APIParT_Angle, format_string ("%.3f", 0.0)));
				moveIn3D ('z', fillersp.radAng, (double)lengthInt / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

				remainLengthInt -= 2400;
			}
			moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

			// ����� ��ü �׷�ȭ (�޸�)
			if (!elemList_Back.IsEmpty ()) {
				GSSize nElems = elemList_Back.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList_Back [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList_Back.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', fillersp.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	}

	// ================================================== ���� ��ġ (�׻� ���ι���)
	// �ո� ��ġ
	EasyObjectPlacement plywood;
	plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	if (placingZone->bLincorner == true)	moveIn3D ('x', plywood.radAng, placingZone->lenLincorner, &plywood.posX, &plywood.posY, &plywood.posZ);		// ���� ���ڳ� ������ x �̵�
	moveIn3D ('y', plywood.radAng, -placingZone->gap, &plywood.posX, &plywood.posY, &plywood.posZ);														// ������ ���ݸ�ŭ �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == PLYWOOD) {
			accumDist = 0.0;
			remainLengthInt = 0;
			for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
				remainLengthInt += placingZone->cells [xx].tableInVerBasic [yy];
				accumDist += (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0;
			}

			while (remainLengthInt > 0) {
				if (remainLengthInt >= 2400)
					lengthInt = 2400;
				else
					lengthInt = remainLengthInt;

				elemList_Front.Push (plywood.placeObject (13,
					"p_stan", APIParT_CString, "��԰�",
					"w_dir", APIParT_CString, "�������",
					"p_thk", APIParT_CString, "11.5T",
					"p_wid", APIParT_Length, format_string ("%.3f", (double)placingZone->cells [xx].horLen / 1000.0),
					"p_leng", APIParT_Length, format_string ("%.3f", (double)lengthInt / 1000.0),
					"p_ang", APIParT_Angle, format_string ("%.3f", 0.0),
					"sogak", APIParT_Boolean, "1.0",
					"bInverseSogak", APIParT_Boolean, "1.0",
					"prof", APIParT_CString, "�Ұ�",
					"gap_a", APIParT_Length, format_string ("%.3f", 0.0),
					"gap_b", APIParT_Length, format_string ("%.3f", 0.0),
					"gap_c", APIParT_Length, format_string ("%.3f", 0.0),
					"gap_d", APIParT_Length, format_string ("%.3f", 0.0)));
				moveIn3D ('z', plywood.radAng, (double)lengthInt / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);

				remainLengthInt -= 2400;
			}
			moveIn3D ('z', plywood.radAng, -accumDist, &plywood.posX, &plywood.posY, &plywood.posZ);

			// ����� ��ü �׷�ȭ (�ո�)
			if (!elemList_Front.IsEmpty ()) {
				GSSize nElems = elemList_Front.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList_Front [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList_Front.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', plywood.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
	}

	// �޸� ��ġ
	plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

	if (placingZone->bLincorner == true)	moveIn3D ('x', plywood.radAng - DegreeToRad (180.0), placingZone->lenLincorner, &plywood.posX, &plywood.posY, &plywood.posZ);		// ���� ���ڳ� ������ x �̵�
	moveIn3D ('y', plywood.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &plywood.posX, &plywood.posY, &plywood.posZ);										// ������ ���ݸ�ŭ �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == PLYWOOD) {
			accumDist = 0.0;
			remainLengthInt = 0;

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (yy = 0 ; yy < varEnd ; ++yy) {
				if (placingZone->bExtra == true)
					lengthInt = placingZone->cells [xx].tableInVerExtra [yy];
				else
					lengthInt = placingZone->cells [xx].tableInVerBasic [yy];

				remainLengthInt += lengthInt;
				accumDist += (double)lengthInt / 1000.0;
			}

			moveIn3D ('x', plywood.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
			while (remainLengthInt > 0) {
				if (remainLengthInt >= 2400)
					lengthInt = 2400;
				else
					lengthInt = remainLengthInt;

				elemList_Back.Push (plywood.placeObject (13,
					"p_stan", APIParT_CString, "��԰�",
					"w_dir", APIParT_CString, "�������",
					"p_thk", APIParT_CString, "11.5T",
					"p_wid", APIParT_Length, format_string ("%.3f", (double)placingZone->cells [xx].horLen / 1000.0),
					"p_leng", APIParT_Length, format_string ("%.3f", (double)lengthInt / 1000.0),
					"p_ang", APIParT_Angle, format_string ("%.3f", 0.0),
					"sogak", APIParT_Boolean, "1.0",
					"bInverseSogak", APIParT_Boolean, "1.0",
					"prof", APIParT_CString, "�Ұ�",
					"gap_a", APIParT_Length, format_string ("%.3f", 0.0),
					"gap_b", APIParT_Length, format_string ("%.3f", 0.0),
					"gap_c", APIParT_Length, format_string ("%.3f", 0.0),
					"gap_d", APIParT_Length, format_string ("%.3f", 0.0)));
				moveIn3D ('z', plywood.radAng, (double)lengthInt / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);

				remainLengthInt -= 2400;
			}
			moveIn3D ('x', plywood.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3D ('z', plywood.radAng, -accumDist, &plywood.posX, &plywood.posY, &plywood.posZ);

			// ����� ��ü �׷�ȭ (�޸�)
			if (!elemList_Back.IsEmpty ()) {
				GSSize nElems = elemList_Back.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList_Back [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList_Back.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', plywood.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
	}

	// ================================================== ���� ��ġ (�׻� ���ι���)
	// �ո� ��ġ
	EasyObjectPlacement timber;
	timber.init (L("����v1.0.gsm"), layerInd_Timber, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	if (placingZone->bLincorner == true)	moveIn3D ('x', timber.radAng, placingZone->lenLincorner, &timber.posX, &timber.posY, &timber.posZ);		// ���� ���ڳ� ������ x �̵�
	moveIn3D ('y', timber.radAng, -placingZone->gap, &timber.posX, &timber.posY, &timber.posZ);														// ������ ���ݸ�ŭ �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == TIMBER) {
			accumDist = 0.0;
			remainLengthInt = 0;
			for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
				remainLengthInt += placingZone->cells [xx].tableInVerBasic [yy];
				accumDist += (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0;
			}

			moveIn3D ('x', timber.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
			while (remainLengthInt > 0) {
				if (remainLengthInt >= 3600)
					lengthInt = 3600;
				else
					lengthInt = remainLengthInt;

				elemList_Front.Push (timber.placeObject (6,
					"w_ins", APIParT_CString, "�������",
					"w_w", APIParT_Length, format_string ("%.3f", 0.050),
					"w_h", APIParT_Length, format_string ("%.3f", (double)placingZone->cells [xx].horLen / 1000.0),
					"w_leng", APIParT_Length, format_string ("%.3f", (double)lengthInt / 1000.0),
					"w_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)),
					"torsion_ang", APIParT_Angle, format_string ("%.3f", 0.0)));
				moveIn3D ('z', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

				remainLengthInt -= 3600;
			}
			moveIn3D ('x', timber.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
			moveIn3D ('z', timber.radAng, -accumDist, &timber.posX, &timber.posY, &timber.posZ);
			
			// ����� ��ü �׷�ȭ (�ո�)
			if (!elemList_Front.IsEmpty ()) {
				GSSize nElems = elemList_Front.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList_Front [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList_Front.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', timber.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
	}

	// �޸� ��ġ
	timber.init (L("����v1.0.gsm"), layerInd_Timber, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

	if (placingZone->bLincorner == true)	moveIn3D ('x', timber.radAng - DegreeToRad (180.0), placingZone->lenLincorner, &timber.posX, &timber.posY, &timber.posZ);		// ���� ���ڳ� ������ x �̵�
	moveIn3D ('y', timber.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &timber.posX, &timber.posY, &timber.posZ);										// ������ ���ݸ�ŭ �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == TIMBER) {
			accumDist = 0.0;
			remainLengthInt = 0;

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (yy = 0 ; yy < varEnd ; ++yy) {
				if (placingZone->bExtra == true)
					lengthInt = placingZone->cells [xx].tableInVerExtra [yy];
				else
					lengthInt = placingZone->cells [xx].tableInVerBasic [yy];

				remainLengthInt += lengthInt;
				accumDist += (double)lengthInt / 1000.0;
			}

			while (remainLengthInt > 0) {
				if (remainLengthInt >= 3600)
					lengthInt = 3600;
				else
					lengthInt = remainLengthInt;

				elemList_Back.Push (timber.placeObject (6,
					"w_ins", APIParT_CString, "�������",
					"w_w", APIParT_Length, format_string ("%.3f", 0.050),
					"w_h", APIParT_Length, format_string ("%.3f", (double)placingZone->cells [xx].horLen / 1000.0),
					"w_leng", APIParT_Length, format_string ("%.3f", (double)lengthInt / 1000.0),
					"w_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)),
					"torsion_ang", APIParT_Angle, format_string ("%.3f", 0.0)));
				moveIn3D ('z', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

				remainLengthInt -= 3600;
			}
			moveIn3D ('z', timber.radAng, -accumDist, &timber.posX, &timber.posY, &timber.posZ);

			// ����� ��ü �׷�ȭ (�޸�)
			if (!elemList_Back.IsEmpty ()) {
				GSSize nElems = elemList_Back.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList_Back [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList_Back.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', timber.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
	}
	
	// ================================================== ���̺��� ��ġ
	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->placeEuroformsOfTableform (this, xx);		// ���̺����� ������ ��ġ (Ÿ�� �ҹ� ����)
		
		if (placingZone->tableformType == 1)		placingZone->placeTableformA (this, xx);
		else if (placingZone->tableformType == 2)	placingZone->placeTableformB (this, xx);
	}
	
	return err;
}

// ��� ������ ������ �Ǵ� ����, ���� ������ ä��
GSErrCode	WallTableformPlacingZone::fillRestAreas (WallTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;

	// !!!

	return err;
}

// ���̺��� �� ������ ��ġ (����)
void	WallTableformPlacingZone::placeEuroformsOfTableform (WallTableformPlacingZone* placingZone, short idxCell)
{
	short	xx, yy, varEnd;
	double	accumDist;
	int		lengthInt;

	EasyObjectPlacement euroform;

	if (placingZone->cells [idxCell].objType == TABLEFORM) {
		if (placingZone->bVertical == true) {
			// ���ι���
			// �ո�
			euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				accumDist = 0.0;
				for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
					if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->cells [idxCell].tableInVerBasic [yy] > 0)) {
						elemList_Front.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
							"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInVerBasic [yy]),
							"u_ins", APIParT_CString, "�������",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						accumDist += (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0;
						moveIn3D ('z', euroform.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					}
				}
				moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}

			// �޸�
			euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
			moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &euroform.posX, &euroform.posY, &euroform.posZ);		// ������ ���ݸ�ŭ �̵�

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				accumDist = 0.0;
				for (yy = 0 ; yy < varEnd ; ++yy) {
					if (placingZone->bExtra == true)
						lengthInt = placingZone->cells [idxCell].tableInVerExtra [yy];
					else
						lengthInt = placingZone->cells [idxCell].tableInVerBasic [yy];

					if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (lengthInt > 0)) {
						moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
						elemList_Back.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
							"eu_hei", APIParT_CString, format_string ("%d", lengthInt),
							"u_ins", APIParT_CString, "�������",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);

						accumDist += (double)lengthInt / 1000.0;
						moveIn3D ('z', euroform.radAng, (double)lengthInt / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					}
				}
				moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
				moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}
		} else {
			// ���ι���
			// �ո�
			euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				accumDist = 0.0;
				for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
					if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->cells [idxCell].tableInVerBasic [yy] > 0)) {
						moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
						elemList_Front.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInVerBasic [yy]),
							"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
							"u_ins", APIParT_CString, "��������",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);

						accumDist += (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0;
						moveIn3D ('z', euroform.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					}
				}
				moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}

			// �޸�
			euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
			moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &euroform.posX, &euroform.posY, &euroform.posZ);		// ������ ���ݸ�ŭ �̵�

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				accumDist = 0.0;
				for (yy = 0 ; yy < varEnd ; ++yy) {
					if (placingZone->bExtra == true)
						lengthInt = placingZone->cells [idxCell].tableInVerExtra [yy];
					else
						lengthInt = placingZone->cells [idxCell].tableInVerBasic [yy];

					if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (lengthInt > 0)) {
						elemList_Back.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", lengthInt),
							"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
							"u_ins", APIParT_CString, "��������",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));

						accumDist += (double)lengthInt / 1000.0;
						moveIn3D ('z', euroform.radAng, (double)lengthInt / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					}
				}
				moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
				moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}
		}
	}
}

// ���̺��� Ÿ��A ��ġ (������ ����) - �������� 2��
void	WallTableformPlacingZone::placeTableformA (WallTableformPlacingZone* placingZone, short idxCell)
{
	short	xx, yy, varEnd;
	int		pipeLength;
	double	sideMargin;
	int*	intPointer;
	
	int		firstWidth, lastWidth;
	bool	bFoundFirstWidth;
	short	realWidthCount, count;

	if (placingZone->bExtra == true) {
		varEnd = placingZone->nCellsInVerExtra;
		intPointer = placingZone->cells [idxCell].tableInVerExtra;
	} else {
		varEnd = placingZone->nCellsInVerBasic;
		intPointer = placingZone->cells [idxCell].tableInVerBasic;
	}

	// ���̺��� �� 1��°�� ������ �������� ������ �ʺ� ������ (���̰� 0�� �������� ���)
	realWidthCount = 0;
	bFoundFirstWidth = false;
	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
		if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
			if (bFoundFirstWidth == false) {
				firstWidth = placingZone->cells [idxCell].tableInHor [xx];
				bFoundFirstWidth = true;
			}
			lastWidth = placingZone->cells [idxCell].tableInHor [xx];
			++realWidthCount;
		}
	}

	if (placingZone->bVertical == true) {
		// ================================================== ���ι���
		// ���� ������ ��ġ - �ո�
		if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].horLen - 50;
			sideMargin = 0.025;
		}

		EasyObjectPlacement rectPipe;
		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// �Ϻ�
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {								// �߰�
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

				moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			}
		}
		moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ���
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// ���� ������ ��ġ - �޸�
		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// �Ϻ�
		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		for (xx = 0 ; xx < varEnd - 1 ; ++xx) {														// �߰�
			if (intPointer [xx] > 0) {
				moveIn3D ('z', rectPipe.radAng, (double)intPointer [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

				moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			}
		}
		moveIn3D ('z', rectPipe.radAng, (double)intPointer [varEnd - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ���
		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// ���� ������ ��ġ - �ո�
		if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
			sideMargin = 0.025;
		}

		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, (double)firstWidth / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ����
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)firstWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ������
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)firstWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// ���� ������ ��ġ - �޸�
		if (placingZone->bExtra == true) {
			if (placingZone->cells [idxCell].verLenExtra % 100 == 0) {
				pipeLength = placingZone->cells [idxCell].verLenExtra - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = placingZone->cells [idxCell].verLenExtra - 50;
				sideMargin = 0.025;
			}
		} else {
			if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
				pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
				sideMargin = 0.025;
			}
		}

		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ����
		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)lastWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - firstWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ������
		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%.3f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)firstWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// �ɺ�Ʈ��Ʈ - �ո�
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// �Ϻ�
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount;
		for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {										// �߰�
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
					if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
						if (count > 0) {
							if (count == realWidthCount) {
								// ����
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
									moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else if (count == 1) {
								// ����
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.150), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else {
								// ������
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%.3f", 0.100), "bolt_dia", APIParT_Length, format_string ("%.3f", 0.010), "washer_pos", APIParT_Length, format_string ("%.3f", 0.050), "washer_size", APIParT_Length, format_string ("%.3f", 0.100), "angX", APIParT_Angle, format_string ("%.3f", DegreeToRad (-90.0)), "angY", APIParT_Angle, format_string ("%.3f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							}

							--count;
						}
					}
				}
			}
		}





		// �ɺ�Ʈ��Ʈ - �޸�

		// ����ö�� - �ո�
		// ... ����ö�� (�簢�ͼ�Ȱ��) v1.0
		// ����ö�� - �޸�

		// ����ǽ� - �ո�
		// ... RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����)
		// ����ǽ� - �޸�
	} else {
		// ================================================== ���ι���
		// ���� ������ ��ġ - �ո�
		// ���� ������ ��ġ - �޸�

		// ���� ������ ��ġ - �ո�
		// ���� ������ ��ġ - �޸�

		// �ɺ�Ʈ��Ʈ - �ո�
		// �ɺ�Ʈ��Ʈ - �޸�

		// ����ö�� - �ո�
		// ����ö�� - �޸�

		// ����ǽ� - �ո�
		// ����ǽ� - �޸�
	}

	// ����� ��ü �׷�ȭ (�ո�)
	if (!elemList_Front.IsEmpty ()) {
		GSSize nElems = elemList_Front.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList_Front [i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
		elemList_Front.Clear ();
	}

	// ����� ��ü �׷�ȭ (�޸�)
	if (!elemList_Back.IsEmpty ()) {
		GSSize nElems = elemList_Back.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList_Back [i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
		elemList_Back.Clear ();
	}
}

// ���̺��� Ÿ��B ��ġ (������ ����) - �������� 1��
void	WallTableformPlacingZone::placeTableformB (WallTableformPlacingZone* placingZone, short idxCell)
{
	// !!!

	// ���� ������
	// ���������v1.0

	// ���� ������
	// ���������v1.0

	// �ɺ�Ʈ��Ʈ
	// �ɺ�Ʈ��Ʈv1.0

	// ����ǽ�
	// ������Ʈ�� Push-Pull Props ����ǽ� v1.0

	// ... ������ ���� ����ϱ� (gap)
	// ... ����/���ι��� ��� (bVertical)
	// ... ��Ī/���Ī ��� (bExtra)
}

// ���̺���/������/�ٷ������̼�/����/���� ��ġ�� ���� ���̾�α� (���̺��� ����, ��� ����, ���� �� ����)
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy, zz;
	char	buffer [256];
	char	numbuf [32];
	int		presetValue, cellHeightValue, cellWidthValue, accumLength;
	const short		maxCol = 50;		// �� �ִ� ����
	const short		maxRow = 10;		// �� �ִ� ����
	double			totalWidth, totalHeight;
	static short	dialogSizeX = 550;			// ���� ���̾�α� ũ�� X
	static short	dialogSizeY = 950;			// ���� ���̾�α� ũ�� Y

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̺���/���ڳ�/������/�ٷ������̼�/����/���� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 200, 900, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 300, 900, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// ������ ��ġ (��: ����, ��ư: �߰�, ����, �׿�: ���� �ʺ�)
			// ��: ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 17, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "����");
			DGShowItem (dialogID, itmIdx);

			// ��ư: �߰�
			placingZone.BUTTON_ADD_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 10, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_ADD_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_ADD_HOR, "�߰�");
			DGShowItem (dialogID, placingZone.BUTTON_ADD_HOR);

			// ��ư: ����
			placingZone.BUTTON_DEL_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 10, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_DEL_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_DEL_HOR, "����");
			DGShowItem (dialogID, placingZone.BUTTON_DEL_HOR);

			// ��: ���� �ʺ�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 290, 17, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� �ʺ�");
			DGShowItem (dialogID, itmIdx);

			// Edit��Ʈ��: ���� �ʺ�
			placingZone.EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 375, 10, 80, 25);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH);

			// ��: ������ ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 50, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "������ ����");
			DGShowItem (dialogID, itmIdx);

			// Edit��Ʈ��: ������ ����
			placingZone.EDITCONTROL_GAP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 105, 45, 70, 23);
			DGShowItem (dialogID, placingZone.EDITCONTROL_GAP);

			// ��: ���̺��� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 80, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���̺��� ����");
			DGShowItem (dialogID, itmIdx);

			// �˾���Ʈ��: ���̺��� ����
			placingZone.POPUP_DIRECTION = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 105, 75, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_DIRECTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM, "����");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_DIRECTION);

			// ��: ���̺��� Ÿ��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 220, 80, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���̺��� Ÿ��");
			DGShowItem (dialogID, itmIdx);

			// �˾���Ʈ��: ���̺��� Ÿ��
			placingZone.POPUP_TABLEFORM_TYPE = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 305, 75, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "Ÿ��A");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "Ÿ��B");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "Ÿ��C");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "Ÿ��D");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE);

			//////////////////////////////////////////////////////////// �� ���� �ʱ�ȭ
			placingZone.initCells (&placingZone, true);

			//////////////////////////////////////////////////////////// ������ ��ġ (���� ���� ��ư)
			// ���� ���ڳ� ���� (üũ��ư)
			placingZone.CHECKBOX_LINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 20, 135, 70, 70);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_LINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_LINCORNER, "���ڳ�");
			DGShowItem (dialogID, placingZone.CHECKBOX_LINCORNER);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_LINCORNER, TRUE);
			// ���� ���ڳ� ���� (Edit��Ʈ��)
			placingZone.EDITCONTROL_LINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 20, 205, 70, 25);
			DGShowItem (dialogID, placingZone.EDITCONTROL_LINCORNER);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_LINCORNER, 0.100);

			// �Ϲ� ��: �⺻���� ���̺���
			itmPosX = 90;
			itmPosY = 137;
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				// ��ư
				placingZone.BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
				DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], "���̺���");
				DGShowItem (dialogID, placingZone.BUTTON_OBJ [xx]);

				// ��ü Ÿ�� (�˾���Ʈ��)
				placingZone.POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "���̺���");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "������");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "�ٷ������̼�");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

				// �ʺ� (�˾���Ʈ��)
				placingZone.POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
				}
				DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

				// �ʺ� (�˾���Ʈ��) - ó������ ����
				placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
				DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);

				itmPosX += 70;
			}

			// ���� ���ڳ� ���� (üũ��ư)
			placingZone.CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_RINCORNER, "���ڳ�");
			DGShowItem (dialogID, placingZone.CHECKBOX_RINCORNER);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER, TRUE);
			// ���� ���ڳ� ���� (Edit��Ʈ��)
			placingZone.EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
			DGShowItem (dialogID, placingZone.EDITCONTROL_RINCORNER);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER, 0.100);

			//////////////////////////////////////////////////////////// ������ ��ġ (���� ���� ��ư)
			// ��: ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 257, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "����");
			DGShowItem (dialogID, itmIdx);

			// ��ư: �߰� (������)
			placingZone.BUTTON_ADD_VER_BASIC = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 250, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_ADD_VER_BASIC, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_ADD_VER_BASIC, "�߰�(L)");
			DGShowItem (dialogID, placingZone.BUTTON_ADD_VER_BASIC);

			// ��ư: ���� (������)
			placingZone.BUTTON_DEL_VER_BASIC = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 250 + 30, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_DEL_VER_BASIC, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_DEL_VER_BASIC, "����(L)");
			DGShowItem (dialogID, placingZone.BUTTON_DEL_VER_BASIC);

			// ��: ���� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 317, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ����");
			DGShowItem (dialogID, itmIdx);

			// Edit��Ʈ��: ���� ���� (������)
			placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 105, 310, 70, 25);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC);
			DGShowItem (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC);

			if (placingZone.bExtra == true) {
				// ��ư: �߰� (������)
				placingZone.BUTTON_ADD_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250, 70, 25);
				DGSetItemFont (dialogID, placingZone.BUTTON_ADD_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_ADD_VER_EXTRA, "�߰�(H)");
				DGShowItem (dialogID, placingZone.BUTTON_ADD_VER_EXTRA);

				// ��ư: ���� (������)
				placingZone.BUTTON_DEL_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250 + 30, 70, 25);
				DGSetItemFont (dialogID, placingZone.BUTTON_DEL_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_DEL_VER_EXTRA, "����(H)");
				DGShowItem (dialogID, placingZone.BUTTON_DEL_VER_EXTRA);

				// Edit��Ʈ��: ���� ���� (������)
				placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 185, 310, 70, 25);
				DGDisableItem (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA);
				DGShowItem (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA);
			}

			// �˾���Ʈ��: ���̺��� ���ι��� ������
			placingZone.POPUP_HEIGHT_PRESET = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 20, 835, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_IS_LARGE | DG_IS_PLAIN);
			for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
				DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
				_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, numbuf);
			}
			DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, "Free");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
			DGShowItem (dialogID, placingZone.POPUP_HEIGHT_PRESET);

			// ���ʿ� ������
			itmPosX = 105;
			itmPosY = 820;
			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
				DGShowItem (dialogID, itmIdx);

				placingZone.POPUP_HEIGHT_BASIC [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
				}
				DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx]);

				itmPosY -= 50;
			}

			if (placingZone.bExtra == true) {
				// �����ʿ� ������
				itmPosX = 185;
				itmPosY = 820;
				if (placingZone.bExtra == true) {
					for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
						DGShowItem (dialogID, itmIdx);

						placingZone.POPUP_HEIGHT_EXTRA [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
						DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_IS_LARGE | DG_IS_PLAIN);
						for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
							DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
							_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
						}
						DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);
						DGShowItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx]);

						itmPosY -= 50;
					}
				}
			}

			//////////////////////////////////////////////////////////// ���̾�α� ũ�� ����, ���� �ʺ� �� ���� ���
			// ���̾�α� ũ�� ����
			dialogSizeX = 550;
			dialogSizeY = 950;
			if (placingZone.nCellsInHor >= 5) {
				DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
			}

			// ���� �ʺ� ���
			totalWidth = 0.0;
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_LINCORNER);
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			// ���� ���� ��� (������)
			totalHeight = 0.0;
			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
				totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx])).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

			if (placingZone.bExtra == true) {
				// ���� ���� ��� (������)
				totalHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
					totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx])).ToCStr ().Get ()) / 1000;
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
			}

			// �ʱⰪ�� ���ι���
			placingZone.bVertical = true;

			break;

		case DG_MSG_CHANGE:
			// ����/���� ������ ��
			if (item == placingZone.POPUP_DIRECTION) {
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_DIRECTION, DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION)).ToCStr ().Get ());

				// ������ ���
				if (my_strcmp (buffer, "����") == 0) {
					// �� ���� �ʱ�ȭ
					placingZone.initCells (&placingZone, false);
					placingZone.bVertical = false;

					// ����
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

					// ����
					for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
						// �˾� ���� ����
						DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_ALL_ITEMS);

						// �˾� ���� �ٽ� ä���
						for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
							DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
							_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
						}
						DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);
					}

					if (placingZone.bExtra == true) {
						for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);
						}
					}

					// ������
					DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_ALL_ITEMS);

					for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
						DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
						_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, numbuf);
					}
					DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, "Free");
					DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);

				// ������ ���
				} else {
					// �� ���� �ʱ�ȭ
					placingZone.initCells (&placingZone, true);
					placingZone.bVertical = true;

					// ����
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

					// ����
					for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
						// �˾� ���� ����
						DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_ALL_ITEMS);

						// �˾� ���� �ٽ� ä���
						for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
							DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
							_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
						}
						DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);
					}

					if (placingZone.bExtra == true) {
						for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);
						}
					}

					// ������
					DGInvalidateItem (dialogID, placingZone.POPUP_HEIGHT_PRESET);

					for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
						DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
						_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, numbuf);
					}
					DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, "Free");
					DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
				}
			}

			// ��ü Ÿ�� ������ ��
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if (item == placingZone.POPUP_OBJ_TYPE [xx]) {
					// �ش� ��ư�� �̸� ����
					DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], DGPopUpGetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx])));
					
					// ����/���� ���� ���ο� ���� �˾���Ʈ���� ���빰�� �ٲ�
					if (placingZone.bVertical == false) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					} else {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

					// ���̺��� Ÿ���� �ƴϸ� ��ư�� ���
					if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) != TABLEFORM)
						DGDisableItem (dialogID, placingZone.BUTTON_OBJ [xx]);
					else
						DGEnableItem (dialogID, placingZone.BUTTON_OBJ [xx]);

					// ���̺���/�������̸� �ʺ� �˾���Ʈ��, �� �ܿ��� Edit��Ʈ��, ������ ��� ����
					if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM)) {
						if (!DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE) {
						if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);
					} else {
						if (!DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))	DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);

						// �ٷ������̼�, ����, ������ ���� ���� ������ �ּ�, �ִ밪
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == FILLERSP) {
							DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.010);
							DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.050);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD) {
							DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.090);
							DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 1.220);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TIMBER) {
							DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.005);
							DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 1.000);
						}
					}
				}
			}

			// ������ �����
			if (item == placingZone.POPUP_HEIGHT_PRESET) {
				// ������ ��
				if (placingZone.bVertical == false) {
					presetValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_PRESET)).ToCStr ().Get ());
					for (xx = 0 ; xx < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++xx) {
						if (presetValue == placingZone.presetWidth_tableform [xx]) {
							for (yy = 0 ; yy < placingZone.nCellsInVerBasic ; ++yy) {
								DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_PLAIN);
								for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy]) ; ++zz) {
									cellHeightValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], zz).ToCStr ().Get ());
									if (cellHeightValue == placingZone.presetHeight_config_horizontal [xx][yy+1]) {
										DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], zz);
										DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_BOLD);
									}
								}
							}
						}
					}
					if (placingZone.bExtra == true) {
						presetValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_PRESET)).ToCStr ().Get ());
						for (xx = 0 ; xx < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++xx) {
							if (presetValue == placingZone.presetWidth_tableform [xx]) {
								for (yy = 0 ; yy < placingZone.nCellsInVerExtra ; ++yy) {
									DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_PLAIN);
									for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy]) ; ++zz) {
										cellHeightValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], zz).ToCStr ().Get ());
										if (cellHeightValue == placingZone.presetHeight_config_horizontal [xx][yy+1]) {
											DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], zz);
											DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_BOLD);
										}
									}
								}
							}
						}
					}

				// ������ ��
				} else {
					presetValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_PRESET)).ToCStr ().Get ());
					for (xx = 0 ; xx < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++xx) {
						if (presetValue == placingZone.presetHeight_tableform [xx]) {
							for (yy = 0 ; yy < placingZone.nCellsInVerBasic ; ++yy) {
								DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_PLAIN);
								for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy]) ; ++zz) {
									cellHeightValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], zz).ToCStr ().Get ());
									if (cellHeightValue == placingZone.presetHeight_config_vertical [xx][yy+1]) {
										DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], zz);
										DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_BOLD);
									}
								}
							}
						}
					}
					if (placingZone.bExtra == true) {
						presetValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_PRESET)).ToCStr ().Get ());
						for (xx = 0 ; xx < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++xx) {
							if (presetValue == placingZone.presetHeight_tableform [xx]) {
								for (yy = 0 ; yy < placingZone.nCellsInVerExtra ; ++yy) {
									DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_PLAIN);
									for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy]) ; ++zz) {
										cellHeightValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], zz).ToCStr ().Get ());
										if (cellHeightValue == placingZone.presetHeight_config_vertical [xx][yy+1]) {
											DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], zz);
											DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_BOLD);
										}
									}
								}
							}
						}
					}
				}
			}

			// ���̺��� �ʺ� ������ ��
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if (item == placingZone.POPUP_WIDTH [xx]) {
					if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
						cellWidthValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], (DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx]))).ToCStr ().Get ());
						if (placingZone.bVertical == true) {
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
								if (cellWidthValue == placingZone.presetWidth_tableform [yy]) {
									for (zz = 0 ; zz < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++zz) {
										if ((zz >= 0) && (zz < placingZone.presetWidth_config_vertical [yy][0]))
											placingZone.cells [xx].tableInHor [zz] = placingZone.presetWidth_config_vertical [yy][zz+1];
										else
											placingZone.cells [xx].tableInHor [zz] = 0;
									}
								}
							}
						} else {
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
								if (cellWidthValue == placingZone.presetHeight_tableform [yy]) {
									for (zz = 0 ; zz < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++zz) {
										if ((zz >= 0) && (zz < placingZone.presetWidth_config_horizontal [yy][0]))
											placingZone.cells [xx].tableInHor [zz] = placingZone.presetWidth_config_horizontal [yy][zz+1];
										else
											placingZone.cells [xx].tableInHor [zz] = 0;
									}
								}
							}
						}
						placingZone.cells [xx].horLen = cellWidthValue;
					}
				}
			}

			// ���� �ʺ� ���
			totalWidth = 0.0;
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_LINCORNER);
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			// ���� ���� ��� (������)
			totalHeight = 0.0;
			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
				totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx])).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

			if (placingZone.bExtra == true) {
				// ���� ���� ��� (������)
				totalHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
					totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx])).ToCStr ().Get ()) / 1000;
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
			}

		case DG_MSG_CLICK:
			// Ȯ�� ��ư
			if (item == DG_OK) {

				// ������ ����
				placingZone.gap = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_GAP);

				// ���̺��� ����
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_DIRECTION, DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION)).ToCStr ().Get ());
				if (my_strcmp (buffer, "����") == 0)
					placingZone.bVertical = true;
				else
					placingZone.bVertical = false;

				// ���̺��� Ÿ��
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DGPopUpGetSelected (dialogID, placingZone.POPUP_TABLEFORM_TYPE)).ToCStr ().Get ());
				if (my_strcmp (buffer, "Ÿ��A") == 0)
					placingZone.tableformType = 1;
				else if (my_strcmp (buffer, "Ÿ��B") == 0)
					placingZone.tableformType = 2;
				else if (my_strcmp (buffer, "Ÿ��C") == 0)
					placingZone.tableformType = 3;
				else if (my_strcmp (buffer, "Ÿ��D") == 0)
					placingZone.tableformType = 4;

				// ���ڳ� ���� �� ����
				placingZone.bLincorner = (DGGetItemValLong (dialogID, placingZone.CHECKBOX_LINCORNER) == TRUE) ? true : false;
				placingZone.bRincorner = (DGGetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER) == TRUE) ? true : false;
				placingZone.lenLincorner = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_LINCORNER);
				placingZone.lenRincorner = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER);

				// ���� ����, ���̺� �� ���� ���� ��� 0���� �ʱ�ȭ
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					placingZone.cells [xx].verLenBasic = 0;
					placingZone.cells [xx].verLenExtra = 0;

					for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInVerBasic) / sizeof (int) ; ++yy)
						placingZone.cells [xx].tableInVerBasic [yy] = 0;
					for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInVerExtra) / sizeof (int) ; ++yy)
						placingZone.cells [xx].tableInVerExtra [yy] = 0;
				}

				// �� ���� ������Ʈ
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					// ��ü Ÿ��
					placingZone.cells [xx].objType = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

					// ���� ���� ���� ����
					if (placingZone.cells [xx].objType == NONE) {
						// ���� ���� 0
						placingZone.cells [xx].horLen = 0;

						// ���̺� �� ���� ���� ��� 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;

					} else if (placingZone.cells [xx].objType == EUROFORM) {
						// ������ �ʺ�
						placingZone.cells [xx].horLen = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ());

						// ���̺� �� ���� ���� ��� 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;

					} else if ((placingZone.cells [xx].objType == FILLERSP) || (placingZone.cells [xx].objType == PLYWOOD) || (placingZone.cells [xx].objType == TIMBER)) {
						// �ٷ������̼�, ����, ������ �ʺ�
						placingZone.cells [xx].horLen = (int)(DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]) * 1000);

						// ���̺� �� ���� ���� ��� 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;
					}

					// ���� ���� ���� ���� (������)
					accumLength = 0;
					for (yy = 0 ; yy < placingZone.nCellsInVerBasic ; ++yy) {
						// ���̺� �� ���� ���� ����
						placingZone.cells [xx].tableInVerBasic [yy] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy])).ToCStr ().Get ());
						accumLength += placingZone.cells [xx].tableInVerBasic [yy];
					}
					placingZone.cells [xx].verLenBasic = accumLength;
				
					// ���� ���� ���� ���� (������)
					accumLength = 0;
					for (yy = 0 ; yy < placingZone.nCellsInVerExtra ; ++yy) {
						// ���̺� �� ���� ���� ����
						placingZone.cells [xx].tableInVerExtra [yy] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy])).ToCStr ().Get ());
						accumLength += placingZone.cells [xx].tableInVerExtra [yy];
					}
					placingZone.cells [xx].verLenExtra = accumLength;
				}

				// ���̾� ����
				bLayerInd_Euroform = true;		// ������ �׻� On
				bLayerInd_RectPipe = false;
				bLayerInd_PinBolt = false;
				bLayerInd_WallTie = false;
				bLayerInd_HeadPiece = false;
				bLayerInd_Join = false;

				bLayerInd_SlabTableform = false;
				bLayerInd_Profile = false;

				bLayerInd_Steelform = false;
				bLayerInd_Plywood = true;		// ���� �׻� On
				bLayerInd_Timber = true;		// ���� �׻� On
				bLayerInd_OutcornerAngle = false;
				bLayerInd_OutcornerPanel = false;
				bLayerInd_RectpipeHanger = false;
				bLayerInd_EuroformHook = false;
				bLayerInd_Hidden = false;

				bLayerInd_Fillersp = false;
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					if (placingZone.cells [xx].objType == FILLERSP) {
						bLayerInd_Fillersp = true;
						break;
					}
				}

				bLayerInd_IncornerPanel = false;
				if ((placingZone.bLincorner == true) || (placingZone.bRincorner == true))
					bLayerInd_IncornerPanel = true;

				if (placingZone.tableformType == 1) {
					bLayerInd_Euroform = true;
					bLayerInd_RectPipe = true;
					bLayerInd_PinBolt = true;
					bLayerInd_HeadPiece = true;
					bLayerInd_Join = true;

				} else if (placingZone.tableformType == 2) {
					bLayerInd_Euroform = true;
					bLayerInd_RectPipe = true;
					bLayerInd_RectpipeHanger = true;
					bLayerInd_EuroformHook = true;
					bLayerInd_HeadPiece = true;
					bLayerInd_Join = true;
					bLayerInd_Hidden = false;

				} else if ((placingZone.tableformType == 3) || (placingZone.tableformType == 4)) {
					bLayerInd_Euroform = true;
					bLayerInd_RectPipe = true;
					bLayerInd_PinBolt = true;
					bLayerInd_HeadPiece = true;
					bLayerInd_Join = true;
					bLayerInd_Hidden = false;
				}

				placingZone.marginTopBasic = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC);		// ��� ���� (������)
				placingZone.marginTopExtra = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA);		// ��� ���� (������)

				placingZone.adjustCellsPosition (&placingZone);		// ���� ��ġ ����

			} else if (item == DG_CANCEL) {
				// �ƹ� �۾��� ���� ����
			} else {
				if ((item == placingZone.BUTTON_ADD_HOR) || (item == placingZone.BUTTON_DEL_HOR)) {
					// ���� - �߰� ��ư Ŭ��
					if (item == placingZone.BUTTON_ADD_HOR) {
						if (placingZone.nCellsInHor < maxCol) {
							// ���� ���ڳ� ��ư�� �����
							DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_RINCORNER);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_RINCORNER);

							// ������ �� ��ư �����ʿ� ���ο� �� ��ư�� �߰��ϰ�
							itmPosX = 90 + (70 * placingZone.nCellsInHor);
							itmPosY = 137;
							// ��ư
							placingZone.BUTTON_OBJ [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], "���̺���");
							DGShowItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor]);

							// ��ü Ÿ�� (�˾���Ʈ��)
							placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "����");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "���̺���");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "������");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "�ٷ������̼�");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "����");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "����");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_TOP+1);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor]);

							// �ʺ� (�˾���Ʈ��)
							placingZone.POPUP_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
							if (placingZone.bVertical == true) {
								for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM, numbuf);
								}
							} else {
								for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM, numbuf);
								}
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor]);

							// �ʺ� (�˾���Ʈ��) - ó������ ����
							placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);

							itmPosX += 70;

							// ���� ���ڳ� ��ư�� ������ ���� ����
							// ���� ���ڳ� ���� (üũ��ư)
							placingZone.CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
							DGSetItemFont (dialogID, placingZone.CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.CHECKBOX_RINCORNER, "���ڳ�");
							DGShowItem (dialogID, placingZone.CHECKBOX_RINCORNER);
							DGSetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER, TRUE);
							// ���� ���ڳ� ���� (Edit��Ʈ��)
							placingZone.EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
							DGShowItem (dialogID, placingZone.EDITCONTROL_RINCORNER);
							DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER, 0.100);

							++placingZone.nCellsInHor;
						}
					}

					// ���� - ���� ��ư Ŭ��
					else if (item == placingZone.BUTTON_DEL_HOR) {
						if (placingZone.nCellsInHor > 1) {
							// ���� ���ڳ� ��ư�� �����
							DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_RINCORNER);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_RINCORNER);

							// ������ �� ��ư�� �����
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor - 1]);

							// 3. ���� ���ڳ� ��ư�� ������ ���� ����
							itmPosX = 90 + (70 * (placingZone.nCellsInHor - 1));
							itmPosY = 137;
							// ���� ���ڳ� ���� (üũ��ư)
							placingZone.CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
							DGSetItemFont (dialogID, placingZone.CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.CHECKBOX_RINCORNER, "���ڳ�");
							DGShowItem (dialogID, placingZone.CHECKBOX_RINCORNER);
							DGSetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER, TRUE);
							// ���� ���ڳ� ���� (Edit��Ʈ��)
							placingZone.EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
							DGShowItem (dialogID, placingZone.EDITCONTROL_RINCORNER);
							DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER, 0.100);

							--placingZone.nCellsInHor;
						}
					}

				} else if ((item == placingZone.BUTTON_ADD_VER_BASIC) || (item == placingZone.BUTTON_DEL_VER_BASIC)) {
					// ���� - �߰�(L) ��ư Ŭ��
					if (item == placingZone.BUTTON_ADD_VER_BASIC) {
						if (placingZone.nCellsInVerBasic < maxRow) {
							// �� ���� �� �ϳ� �߰�
							itmPosX = 105;
							itmPosY = 820 - (50 * placingZone.nCellsInVerBasic);

							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
							DGShowItem (dialogID, itmIdx);

							placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_IS_LARGE | DG_IS_PLAIN);
							if (placingZone.bVertical == true) {
								for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM, numbuf);
								}
							} else {
								for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM, numbuf);
								}
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic]);

							++placingZone.nCellsInVerBasic;
						}						
					}

					// ���� - ����(L) ��ư Ŭ��
					else if (item == placingZone.BUTTON_DEL_VER_BASIC) {
						if (placingZone.nCellsInVerBasic > 1) {
							// �� ���� �ִ� �� �ϳ� ����
							DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic - 1] - 1);

							--placingZone.nCellsInVerBasic;
						}
					}

				} else if ((item == placingZone.BUTTON_ADD_VER_EXTRA) || (item == placingZone.BUTTON_DEL_VER_EXTRA)) {
					// ���� - �߰�(H) ��ư Ŭ��
					if (item == placingZone.BUTTON_ADD_VER_EXTRA) {
						if (placingZone.nCellsInVerExtra < maxRow) {
							// �� ���� �� �ϳ� �߰�
							itmPosX = 185;
							itmPosY = 820 - (50 * placingZone.nCellsInVerExtra);

							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
							DGShowItem (dialogID, itmIdx);

							placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_IS_LARGE | DG_IS_PLAIN);
							if (placingZone.bVertical == true) {
								for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM, numbuf);
								}
							} else {
								for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM, numbuf);
								}
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra]);

							++placingZone.nCellsInVerExtra;
						}
					}

					// ���� - ����(H) ��ư Ŭ��
					else if (item == placingZone.BUTTON_DEL_VER_EXTRA) {
						if (placingZone.nCellsInVerExtra > 1) {
							// �� ���� �ִ� �� �ϳ� ����
							DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra - 1] - 1);

							--placingZone.nCellsInVerExtra;
						}
					}

				} else {
					// ��ü ��ư Ŭ�� (���̺����� ��쿡�� ��ȿ��)
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if ((item == placingZone.BUTTON_OBJ [xx]) && (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM)) {
							clickedIndex = xx;
							if (placingZone.bVertical == true) {
								// ���̺��� Ÿ�� (���� ����)�� ���, 3��° ���̾�α�(���ι���) ����
								result = DGBlankModalDialog (350, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3_Vertical, (short) 0);
							} else {
								// ���̺��� Ÿ�� (���� ����)�� ���, 3��° ���̾�α�(���ι���) ����
								result = DGBlankModalDialog (420, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3_Horizontal, (short) 0);
							}

							// �޺��ڽ��� ��ü �ʺ� �� ����
							accumLength = 0;

							for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
								accumLength += placingZone.cells [xx].tableInHor [yy];

							for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH [xx]) ; ++yy) {
								if (accumLength == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], yy).ToCStr ().Get ())) {
									DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], yy);
									break;
								}
							}
						}
					}
				}

				// ���̾�α� ũ�� ����
				dialogSizeX = 550;
				dialogSizeY = 950;
				if (placingZone.nCellsInHor >= 5) {
					DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
				}

				// ���� �ʺ� ���
				totalWidth = 0.0;
				if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_LINCORNER);
				if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER);
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
						totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
					else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
						totalWidth += 0.0;
					else
						totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

				// ���� ���� ��� (������)
				totalHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
					totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx])).ToCStr ().Get ()) / 1000;
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

				if (placingZone.bExtra == true) {
					// ���� ���� ��� (������)
					totalHeight = 0.0;
					for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
						totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx])).ToCStr ().Get ()) / 1000;
					}
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
				}

				item = 0;
			}

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// ��ü�� ���̾ �����ϱ� ���� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "������ ���̾� �����ϱ�");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// ���̾� ���� ��
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "������ ���̺���");
			DGSetItemText (dialogID, LABEL_LAYER_PROFILE, "C����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "��� ������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "��ü Ÿ��");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "����ö��");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "����ǽ�");
			DGSetItemText (dialogID, LABEL_LAYER_STEELFORM, "��ƿ��");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "����");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, "�ٷ������̼�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "�ƿ��ڳʾޱ�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, "�ƿ��ڳ��ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL, "���ڳ��ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "�����������");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "������ ��ũ");
			DGSetItemText (dialogID, LABEL_LAYER_HIDDEN, "����");

			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 580, 160, 25);
			DGSetItemFont (dialogID, BUTTON_AUTOSET, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_AUTOSET, "���̾� �ڵ� ����");
			DGShowItem (dialogID, BUTTON_AUTOSET);

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_SLABTABLEFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, 1);
			if (bLayerInd_SlabTableform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PROFILE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, 1);
			if (bLayerInd_Profile == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PROFILE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PROFILE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);
			if (bLayerInd_Euroform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);
			if (bLayerInd_RectPipe == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);
			if (bLayerInd_PinBolt == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			}

			ucb.itemID	 = USERCONTROL_LAYER_WALLTIE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, 1);
			if (bLayerInd_WallTie == true) {
				DGEnableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_JOIN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, 1);
			if (bLayerInd_Join == true) {
				DGEnableItem (dialogID, LABEL_LAYER_JOIN);
				DGEnableItem (dialogID, USERCONTROL_LAYER_JOIN);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_JOIN);
				DGDisableItem (dialogID, USERCONTROL_LAYER_JOIN);
			}

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);
			if (bLayerInd_HeadPiece == true) {
				DGEnableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_STEELFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, 1);
			if (bLayerInd_Steelform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_STEELFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_STEELFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);
			if (bLayerInd_Plywood == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			}

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);
			if (bLayerInd_Plywood == true) {
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_TIMBER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, 1);
			if (bLayerInd_Fillersp == true) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);
			if (bLayerInd_OutcornerAngle == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, 1);
			if (bLayerInd_OutcornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_INCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, 1);
			if (bLayerInd_IncornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, 1);
			if (bLayerInd_RectpipeHanger == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			}

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, 1);
			if (bLayerInd_EuroformHook == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			}

			ucb.itemID	 = USERCONTROL_LAYER_HIDDEN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN, 1);
			if (bLayerInd_Hidden == true) {
				DGEnableItem (dialogID, LABEL_LAYER_HIDDEN);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_HIDDEN);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			}
			break;

		case DG_MSG_CHANGE:
			// ���̾� ���� �ٲ�
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_SLABTABLEFORM:
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						break;
					case USERCONTROL_LAYER_PROFILE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						break;
					case USERCONTROL_LAYER_EUROFORM:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						break;
					case USERCONTROL_LAYER_RECTPIPE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						break;
					case USERCONTROL_LAYER_PINBOLT:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						break;
					case USERCONTROL_LAYER_WALLTIE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						break;
					case USERCONTROL_LAYER_JOIN:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						break;
					case USERCONTROL_LAYER_HEADPIECE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						break;
					case USERCONTROL_LAYER_STEELFORM:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						break;
					case USERCONTROL_LAYER_PLYWOOD:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						break;
					case USERCONTROL_LAYER_TIMBER:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						break;
					case USERCONTROL_LAYER_FILLERSP:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						break;
					case USERCONTROL_LAYER_OUTCORNER_ANGLE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						break;
					case USERCONTROL_LAYER_OUTCORNER_PANEL:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						break;
					case USERCONTROL_LAYER_INCORNER_PANEL:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						break;
					case USERCONTROL_LAYER_RECTPIPE_HANGER:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						break;
					case USERCONTROL_LAYER_EUROFORM_HOOK:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						break;
				}
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���̾� ��ȣ ����
					if (bLayerInd_SlabTableform == true)	layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					if (bLayerInd_Profile == true)			layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE);
					if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					if (bLayerInd_WallTie == true)			layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					if (bLayerInd_Steelform == true)		layerInd_Steelform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM);
					if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					if (bLayerInd_Timber == true)			layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
					if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
					if (bLayerInd_RectpipeHanger == true)	layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
					if (bLayerInd_EuroformHook == true)		layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
					if (bLayerInd_Hidden == true)			layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					if (placingZone.tableformType == 1) {
						layerInd_Euroform	= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
						layerInd_RectPipe	= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
						layerInd_PinBolt	= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
						layerInd_Join		= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
						layerInd_HeadPiece	= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
						layerInd_Plywood	= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);

					} else if (placingZone.tableformType == 2) {
						layerInd_Euroform	= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
						layerInd_RectPipe	= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
						layerInd_RectpipeHanger	= makeTemporaryLayer (structuralObject_forTableformWall, "JOIB", NULL);
						layerInd_EuroformHook	= makeTemporaryLayer (structuralObject_forTableformWall, "HOOK", NULL);
						layerInd_Join		= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
						layerInd_HeadPiece	= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
						layerInd_Plywood	= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, layerInd_RectpipeHanger);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, layerInd_EuroformHook);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);

					} else if ((placingZone.tableformType == 3) || (placingZone.tableformType == 4)) {
						layerInd_Euroform	= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
						layerInd_RectPipe	= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
						layerInd_PinBolt	= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
						layerInd_Join		= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
						layerInd_HeadPiece	= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
						layerInd_Plywood	= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
					}

					break;

				case DG_CANCEL:
					break;
			}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	// clickedIndex: ���� ���̾�α׿��� ���� ��ư�� 0-��� �ε��� ��ȣ (BUTTON_OBJ [xx])

	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	int		accumLength;
	char	buffer [256];
	char	numbuf [32];

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̺��� (���ι���) �迭 ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 100, 140, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 180, 140, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");
			DGShowItem (dialogID, DG_CANCEL);

			// ���� �ʺ� (��)
			accumLength = 0;
			for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
				accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			sprintf (buffer, "���� �ʺ�: %d", accumLength);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 20, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, buffer);
			DGShowItem (dialogID, itmIdx);

			// ����� �ʺ� (��)
			sprintf (buffer, "����� �ʺ�: %d", 0);
			placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 20, 100, 23);
			DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			itmPosX = 35;
			itmPosY = 55;

			for (xx = 0 ; xx < 4 ; ++xx) {
				// ������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
				DGShowItem (dialogID, itmIdx);

				// �ؽ�Ʈ(������)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				// �޺��ڽ�
				placingZone.POPUP_WIDTH_IN_TABLE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 5, itmPosY + 40, 60, 25);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM, numbuf);
				}
				for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]) ; ++yy) {
					if (placingZone.cells [clickedIndex].tableInHor [xx] == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy).ToCStr ().Get ())) {
						DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy);
						break;
					}
				}
				DGShowItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]);

				itmPosX += 70;
			}

			// ����� �ʺ� (��) ������Ʈ
			accumLength = 0;
			for (xx = 0 ; xx < 4 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "����� �ʺ�: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// ����� �ʺ� (��) ������Ʈ
			accumLength = 0;
			for (xx = 0 ; xx < 4 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "����� �ʺ�: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ������ �޺��ڽ����� ���� ������� ����ü ���� ������
					for (xx = 0 ; xx < 4 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}
					for (xx = 4 ; xx < 10 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = 0;
					}

					accumLength = 0;
					for (xx = 0 ; xx < 5 ; ++xx) {
						accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}
					placingZone.cells [clickedIndex].horLen = accumLength;
					break;

				case DG_CANCEL:
					break;
			}

			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	// clickedIndex: ���� ���̾�α׿��� ���� ��ư�� 0-��� �ε��� ��ȣ (BUTTON_OBJ [xx])

	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	int		accumLength;
	char	buffer [256];
	char	numbuf [32];

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̺��� (���ι���) �迭 ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 140, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 200, 140, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");
			DGShowItem (dialogID, DG_CANCEL);

			// ���� �ʺ� (��)
			accumLength = 0;
			for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
				accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			sprintf (buffer, "���� �ʺ�: %d", accumLength);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 20, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, buffer);
			DGShowItem (dialogID, itmIdx);

			// ����� �ʺ� (��)
			sprintf (buffer, "����� �ʺ�: %d", 0);
			placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 20, 100, 23);
			DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			itmPosX = 35;
			itmPosY = 55;

			for (xx = 0 ; xx < 5 ; ++xx) {
				// ������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
				DGShowItem (dialogID, itmIdx);

				// �ؽ�Ʈ(������)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				// �޺��ڽ�
				placingZone.POPUP_WIDTH_IN_TABLE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 5, itmPosY + 40, 60, 25);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM, numbuf);
				}
				for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]) ; ++yy) {
					if (placingZone.cells [clickedIndex].tableInHor [xx] == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy).ToCStr ().Get ())) {
						DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy);
						break;
					}
				}
				DGShowItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]);

				itmPosX += 70;
			}

			// ����� �ʺ� (��) ������Ʈ
			accumLength = 0;
			for (xx = 0 ; xx < 5 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "����� �ʺ�: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// ����� �ʺ� (��) ������Ʈ
			accumLength = 0;
			for (xx = 0 ; xx < 5 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "����� �ʺ�: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ������ �޺��ڽ����� ���� ������� ����ü ���� ������
					for (xx = 0 ; xx < 5 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}
					for (xx = 5 ; xx < 10 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = 0;
					}

					accumLength = 0;
					for (xx = 0 ; xx < 5 ; ++xx) {
						accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}
					placingZone.cells [clickedIndex].horLen = accumLength;
					break;

				case DG_CANCEL:
					break;
			}

			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// �� ����� ������ ������ ������ �Ǵ� ����/����� ä���� ����� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler4 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	short	xx;

	double	initPlywoodHeight = 0.0;
	double	changedPlywoodHeight = 0.0;

	// ���̾�α׿��� ������ ������ ����
	bool	bEuroform1, bEuroform2;
	bool	bEuroformStandard1, bEuroformStandard2;
	double	euroformWidth1 = 0.0, euroformWidth2 = 0.0;


	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			if (iMarginSide == 1)
				DGSetDialogTitle (dialogID, "�� ��� ä��� (������)");
			else
				DGSetDialogTitle (dialogID, "�� ��� ä��� (������)");

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 70, 240, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "��");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 160, 240, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "�ƴϿ�");
			DGShowItem (dialogID, DG_CANCEL);

			// ��: ����
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 10, 260, 23);
			DGSetItemFont (dialogID, LABEL_DESC1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DESC1_TOPREST, "�� ��ο� ���� ���� ��ŭ�� ������ �ֽ��ϴ�.");
			DGShowItem (dialogID, LABEL_DESC1_TOPREST);

			// ��: ����
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 40, 50, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT_TOPREST, "����");
			DGShowItem (dialogID, LABEL_HEIGHT_TOPREST);

			// Edit ��Ʈ��: ����
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 140, 40-6, 70, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_HEIGHT_TOPREST);
			if (iMarginSide == 1)
				DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_TOPREST, placingZone.marginTopBasic);
			else
				DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_TOPREST, placingZone.marginTopExtra);
			DGDisableItem (dialogID, EDITCONTROL_HEIGHT_TOPREST);

			// ��: ����
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 50, 80, 200, 23);
			DGSetItemFont (dialogID, LABEL_DESC2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DESC2_TOPREST, "���������� ä��ðڽ��ϱ�?");
			DGShowItem (dialogID, LABEL_DESC2_TOPREST);

			// ��: '��'
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 20, 120, 30, 23);
			DGSetItemFont (dialogID, LABEL_UP_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_UP_TOPREST, "��");
			DGShowItem (dialogID, LABEL_UP_TOPREST);

			// ��: '��'
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 20, 150, 30, 23);
			DGSetItemFont (dialogID, LABEL_ARROWUP_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ARROWUP_TOPREST, "��");
			DGShowItem (dialogID, LABEL_ARROWUP_TOPREST);

			// ��: '�Ʒ�'
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 20, 180, 30, 23);
			DGSetItemFont (dialogID, LABEL_DOWN_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DOWN_TOPREST, "�Ʒ�");
			DGShowItem (dialogID, LABEL_DOWN_TOPREST);

			// üũ�ڽ�: �� On/Off (1�� - �� �Ʒ�)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 70, 180-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST, "������");
			DGShowItem (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST);

			// üũ�ڽ�: �� On/Off (2�� - �߰�)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 70, 150-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST, "������");
			DGShowItem (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST);

			// ��: ����
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 120, 70, 23);
			DGSetItemFont (dialogID, LABEL_PLYWOOD_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
			DGShowItem (dialogID, LABEL_PLYWOOD_TOPREST);

			// üũ�ڽ�: �԰��� (1�� - �� �Ʒ�)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 150, 180-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, "�԰���");
			DGShowItem (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST);
			DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, TRUE);

			// üũ�ڽ�: �԰��� (2�� - �߰�)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 150, 150-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, "�԰���");
			DGShowItem (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST);
			DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, TRUE);

			// �˾� ��Ʈ��: ������ (1��) �ʺ�
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 220, 180-6, 70, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "200");
			DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);

			// �˾� ��Ʈ��: ������ (2��) �ʺ�
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 220, 150-6, 70, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "200");
			DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);

			// Edit ��Ʈ��: ������ (1��) �ʺ�
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 180-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);

			// Edit ��Ʈ��: ������ (2��) �ʺ�
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 150-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);

			// Edit ��Ʈ��: ���� �Ǵ� ���� �ʺ�
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 120-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_PLYWOOD_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_PLYWOOD_TOPREST);
			DGDisableItem (dialogID, EDITCONTROL_PLYWOOD_TOPREST);

			if (iMarginSide == 1) {
				if (placingZone.marginTopBasic < EPS) {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
				} else if (placingZone.marginTopBasic < 0.110) {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
				} else {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
				}
				DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, placingZone.marginTopBasic);
			} else {
				if (placingZone.marginTopExtra < EPS) {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
				} else if (placingZone.marginTopExtra < 0.110) {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
				} else {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
				}
				DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, placingZone.marginTopExtra);
			}

			// üũ�ڽ� ���� ���� �׸� Ȱ��ȭ/��Ȱ��ȭ
			if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST) == TRUE) {
				DGEnableItem (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST);
				DGEnableItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
				DGEnableItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
			} else {
				DGDisableItem (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST);
				DGDisableItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
				DGDisableItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
			}

			if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST) == TRUE) {
				DGEnableItem (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST);
				DGEnableItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
				DGEnableItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
			} else {
				DGDisableItem (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST);
				DGDisableItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
				DGDisableItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
			}

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case CHECKBOX_SET_STANDARD_1_TOPREST:
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST) == TRUE) {
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
					} else {
						DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
					}
					break;
				
				case CHECKBOX_SET_STANDARD_2_TOPREST:
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST) == TRUE) {
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
					} else {
						DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
					}
					break;
			}

			// ���� �ʺ� ���� ����/���� ������ ���̰� �޶���
			if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST) == TRUE)
				bEuroform1 = true;
			else
				bEuroform1 = false;
			if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST) == TRUE)
				bEuroform2 = true;
			else
				bEuroform2 = false;
			if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST) == TRUE)
				bEuroformStandard1 = true;
			else
				bEuroformStandard1 = false;
			if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST) == TRUE)
				bEuroformStandard2 = true;
			else
				bEuroformStandard2 = false;

			if (iMarginSide == 1)
				initPlywoodHeight = placingZone.marginTopBasic;
			else
				initPlywoodHeight = placingZone.marginTopExtra;

			changedPlywoodHeight = initPlywoodHeight;
			euroformWidth1 = 0.0;
			euroformWidth2 = 0.0;

			if (bEuroform1) {
				if (bEuroformStandard1)
					euroformWidth1 = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST)).ToCStr ()) / 1000.0;
				else
					euroformWidth1 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
			}

			if (bEuroform2) {
				if (bEuroformStandard2)
					euroformWidth2 = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST)).ToCStr ()) / 1000.0;
				else
					euroformWidth2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
			}

			changedPlywoodHeight -= (euroformWidth1 + euroformWidth2);

			if (changedPlywoodHeight < EPS) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
			} else if (changedPlywoodHeight < 0.110) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
			} else {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "����");
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, changedPlywoodHeight);

			// üũ�ڽ� ���� ���� �׸� Ȱ��ȭ/��Ȱ��ȭ
			if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST) == TRUE) {
				DGEnableItem (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST);
				DGEnableItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
				DGEnableItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
			} else {
				DGDisableItem (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST);
				DGDisableItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
				DGDisableItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
			}

			if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST) == TRUE) {
				DGEnableItem (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST);
				DGEnableItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
				DGEnableItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
			} else {
				DGDisableItem (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST);
				DGDisableItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
				DGDisableItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// �� ������ �� ���� ������ ���̸� �����
					if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST) == TRUE)
						bEuroform1 = true;
					else
						bEuroform1 = false;
					if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST) == TRUE)
						bEuroform2 = true;
					else
						bEuroform2 = false;
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST) == TRUE)
						bEuroformStandard1 = true;
					else
						bEuroformStandard1 = false;
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST) == TRUE)
						bEuroformStandard2 = true;
					else
						bEuroformStandard2 = false;

					if (iMarginSide == 1)
						initPlywoodHeight = placingZone.marginTopBasic;
					else
						initPlywoodHeight = placingZone.marginTopExtra;
			
					changedPlywoodHeight = initPlywoodHeight;
					euroformWidth1 = 0.0;
					euroformWidth2 = 0.0;

					if (bEuroform1) {
						if (bEuroformStandard1)
							euroformWidth1 = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST)).ToCStr ()) / 1000.0;
						else
							euroformWidth1 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
					}

					if (bEuroform2) {
						if (bEuroformStandard2)
							euroformWidth2 = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST)).ToCStr ()) / 1000.0;
						else
							euroformWidth2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
					}

					changedPlywoodHeight -= (euroformWidth1 + euroformWidth2);

					if (iMarginSide == 1) {
						for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
							placingZone.marginCellsBasic [xx].bFill = true;

							placingZone.marginCellsBasic [xx].bEuroform1 = bEuroform1;
							placingZone.marginCellsBasic [xx].bEuroformStandard1 = bEuroformStandard1;
							placingZone.marginCellsBasic [xx].formWidth1 = euroformWidth1;

							placingZone.marginCellsBasic [xx].bEuroform2 = bEuroform2;
							placingZone.marginCellsBasic [xx].bEuroformStandard2 = bEuroformStandard2;
							placingZone.marginCellsBasic [xx].formWidth2 = euroformWidth2;
						}
					} else {
						for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
							placingZone.marginCellsExtra [xx].bFill = true;

							placingZone.marginCellsExtra [xx].bEuroform1 = bEuroform1;
							placingZone.marginCellsExtra [xx].bEuroformStandard1 = bEuroformStandard1;
							placingZone.marginCellsExtra [xx].formWidth1 = euroformWidth1;

							placingZone.marginCellsExtra [xx].bEuroform2 = bEuroform2;
							placingZone.marginCellsExtra [xx].bEuroformStandard2 = bEuroformStandard2;
							placingZone.marginCellsExtra [xx].formWidth2 = euroformWidth2;
						}
					}

					break;
				case DG_CANCEL:
					break;
			}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}