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
static short	layerInd_Wood;				// ���̾� ��ȣ: ���� (����)
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
static bool		bLayerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static bool		bLayerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static bool		bLayerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static bool		bLayerInd_IncornerPanel;	// ���̾� ��ȣ: ���ڳʾޱ�
static bool		bLayerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���
static bool		bLayerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
static bool		bLayerInd_Hidden;			// ���̾� ��ȣ: ����

static short	layerInd_SlabTableform;		// ���̾� ��ȣ: ������ ���̺���
static short	layerInd_Profile;			// ���̾� ��ȣ: KS��������

static short	layerInd_Steelform;			// ���̾� ��ȣ: ��ƿ��
static short	layerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static short	layerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_IncornerPanel;		// ���̾� ��ȣ: ���ڳʾޱ�
static short	layerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���

static GS::Array<API_Guid>	elemList;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

// ���̾�α� ���� ��� �ε��� ��ȣ ����
static short	EDITCONTROL_GAP;
static short	POPUP_DIRECTION;
static short	POPUP_TABLEFORM_TYPE;
static short	EDITCONTROL_REMAIN_WIDTH;
static short	EDITCONTROL_REMAIN_HEIGHT_BASIC;
static short	EDITCONTROL_REMAIN_HEIGHT_EXTRA;
static short	BUTTON_ADD_HOR;
static short	BUTTON_DEL_HOR;
static short	CHECKBOX_LINCORNER;
static short	EDITCONTROL_LINCORNER;
static short	CHECKBOX_RINCORNER;
static short	EDITCONTROL_RINCORNER;
static short	BUTTON_ADD_VER_BASIC;
static short	BUTTON_DEL_VER_BASIC;
static short	BUTTON_ADD_VER_EXTRA;
static short	BUTTON_DEL_VER_EXTRA;

static short	BUTTON_OBJ [50];
static short	POPUP_OBJ_TYPE [50];
static short	POPUP_WIDTH [50];
static short	EDITCONTROL_WIDTH [50];
static short	POPUP_HEIGHT_BASIC [10];
static short	POPUP_HEIGHT_EXTRA [10];

//static double	preferWidth;
static bool		clickedPrevButton;		// ���� ��ư�� �������ϱ�?


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

	clickedPrevButton = false;

	// �ʱ� �� ���� ���
	placingZone.nCellsInHor = (short)floor (placingZone.horLen / 2.300);
	placingZone.nCellsInVerBasic = (short)floor (placingZone.verLenBasic / 1.200);
	placingZone.nCellsInVerExtra = (short)floor (placingZone.verLenExtra / 1.200);

	// [DIALOG] 1��° ���̾�α׿��� ���ڳ� ���� �� ����, ���̺����� ����� ����/���� ���� �������� ������ ������ ���̸� ������
	result = DGBlankModalDialog (550, 950, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler1, 0);

	if (result != DG_OK)
		return err;

	// [DIALOG] 2��° ���̾�α׿��� ���纰 ���̾ ������
	//result = DGModalDialog (ACAPI_GetOwnResModule (), 32519, ACAPI_GetOwnResModule (), wallTableformPlacerHandler2, 0);

	if (result != DG_OK)
		return err;

	// �� ���� �ʱ�ȭ
	// ...

	// ���̺��� ��ġ�ϱ�
	// ...

	// ���̺��� ��� ���� ä���
	// ...

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// �� ���� �ʱ�ȭ
void	WallTableformPlacingZone::initCells (WallTableformPlacingZone* placingZone)
{
//	short	xx;
//
//	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
//		placingZone->cells [xx].ang = placingZone->ang;
//		placingZone->cells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
//		placingZone->cells [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
//		placingZone->cells [xx].leftBottomZ = placingZone->leftBottomZ;
//
//		placingZone->upperCells [xx].ang = placingZone->ang;
//		placingZone->upperCells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
//		placingZone->upperCells [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
//		placingZone->upperCells [xx].leftBottomZ = placingZone->leftBottomZ;
//		
//		placingZone->upperCells [xx].bFill = false;
//		placingZone->upperCells [xx].bEuroform1 = false;
//		placingZone->upperCells [xx].bEuroformStandard1 = false;
//		placingZone->upperCells [xx].formWidth1 = 0.0;
//		placingZone->upperCells [xx].bEuroform2 = false;
//		placingZone->upperCells [xx].bEuroformStandard2 = false;
//		placingZone->upperCells [xx].formWidth2 = 0.0;
//	}
}

// ���̺���/������/�ٷ������̼�/����/���� ��ġ�� ���� ���̾�α� (���̺��� ����, ��� ����, ���� �� ����)
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx;
	//char	buffer [256];
	//bool	bChanged;					// ȭ���� ����Ǵ°�?
	static short	currentCol;			// �� ���� ����
	static short	currentRowBasic;	// �� ���� ���� (������)
	static short	currentRowExtra;	// �� ���� ���� (������)
	const short		maxCol = 50;		// �� �ִ� ����
	const short		maxRow = 10;		// �� �ִ� ����
	double			totalWidth, totalHeight;
	//short			widthInd, heightInd;	// �ʺ�, ���̸� ����Ű�� �˾���Ʈ�� �ε���
	//double			accX, accZ;				// �������� ��ġ�ϱ� ���� �Ÿ��� ����ϱ� ���� ����
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
			BUTTON_ADD_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 10, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_HOR, "�߰�");
			DGShowItem (dialogID, BUTTON_ADD_HOR);

			// ��ư: ����
			BUTTON_DEL_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 10, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_HOR, "����");
			DGShowItem (dialogID, BUTTON_DEL_HOR);

			// ��: ���� �ʺ�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 290, 17, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� �ʺ�");
			DGShowItem (dialogID, itmIdx);

			// Edit��Ʈ��: ���� �ʺ�
			EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 375, 10, 80, 25);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_WIDTH);

			// ��: ������ ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 50, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "������ ����");
			DGShowItem (dialogID, itmIdx);

			// Edit��Ʈ��: ������ ����
			EDITCONTROL_GAP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 105, 45, 70, 23);
			DGShowItem (dialogID, EDITCONTROL_GAP);

			// ��: ���̺��� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 80, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���̺��� ����");
			DGShowItem (dialogID, itmIdx);

			// �˾���Ʈ��: ���̺��� ����
			POPUP_DIRECTION = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 105, 75, 70, 23);
			DGSetItemFont (dialogID, POPUP_DIRECTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_DIRECTION, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_DIRECTION, DG_POPUP_BOTTOM, "����");
			DGPopUpSelectItem (dialogID, POPUP_DIRECTION, DG_POPUP_TOP);
			DGShowItem (dialogID, POPUP_DIRECTION);

			// ��: ���̺��� Ÿ��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 220, 80, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���̺��� Ÿ��");
			DGShowItem (dialogID, itmIdx);

			// �˾���Ʈ��: ���̺��� Ÿ��
			POPUP_TABLEFORM_TYPE = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 305, 75, 70, 23);
			DGSetItemFont (dialogID, POPUP_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "Ÿ��A");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "Ÿ��B");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "Ÿ��C");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "Ÿ��D");
			DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_TOP);
			DGShowItem (dialogID, POPUP_TABLEFORM_TYPE);

			//////////////////////////////////////////////////////////// ������ ��ġ (���� ���� ��ư)
			// ���� ���ڳ� ���� (üũ��ư)
			CHECKBOX_LINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 20, 135, 70, 70);
			DGSetItemFont (dialogID, CHECKBOX_LINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_LINCORNER, "���ڳ�");
			DGShowItem (dialogID, CHECKBOX_LINCORNER);
			DGSetItemValLong (dialogID, CHECKBOX_LINCORNER, TRUE);
			// ���� ���ڳ� ���� (Edit��Ʈ��)
			EDITCONTROL_LINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 20, 205, 70, 25);
			DGShowItem (dialogID, EDITCONTROL_LINCORNER);
			DGSetItemValDouble (dialogID, EDITCONTROL_LINCORNER, 0.100);

			// �Ϲ� ��: �⺻���� ���̺���
			itmPosX = 90;
			itmPosY = 137;
			currentCol = placingZone.nCellsInHor;
			for (xx = 0 ; xx < currentCol ; ++xx) {
				// ��ư
				BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
				DGSetItemFont (dialogID, BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, BUTTON_OBJ [xx], "���̺���");
				DGShowItem (dialogID, BUTTON_OBJ [xx]);

				// ��ü Ÿ�� (�˾���Ʈ��)
				POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
				DGSetItemFont (dialogID, POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "���̺���");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "������");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "�ٷ������̼�");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, POPUP_OBJ_TYPE [xx]);

				// �ʺ� (�˾���Ʈ��)
				POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
				DGSetItemFont (dialogID, POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2300");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2250");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2200");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2150");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2100");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2050");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2000");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1950");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1900");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1850");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1800");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1750");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1700");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1650");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1600");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1550");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1500");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1450");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1400");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1350");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1300");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1250");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1200");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1150");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1100");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1050");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1000");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "950");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "900");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "850");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "800");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "750");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "700");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "650");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "600");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "500");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "450");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "400");
				DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, POPUP_WIDTH [xx]);

				// �ʺ� (�˾���Ʈ��) - ó������ ����
				EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);

				itmPosX += 70;
			}

			// ���� ���ڳ� ���� (üũ��ư)
			CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
			DGSetItemFont (dialogID, CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_RINCORNER, "���ڳ�");
			DGShowItem (dialogID, CHECKBOX_RINCORNER);
			DGSetItemValLong (dialogID, CHECKBOX_RINCORNER, TRUE);
			// ���� ���ڳ� ���� (Edit��Ʈ��)
			EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
			DGShowItem (dialogID, EDITCONTROL_RINCORNER);
			DGSetItemValDouble (dialogID, EDITCONTROL_RINCORNER, 0.100);

			//////////////////////////////////////////////////////////// ������ ��ġ (���� ���� ��ư)
			// ��: ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 257, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "����");
			DGShowItem (dialogID, itmIdx);

			// ��ư: �߰� (������)
			BUTTON_ADD_VER_BASIC = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 250, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_VER_BASIC, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_VER_BASIC, "�߰�(L)");
			DGShowItem (dialogID, BUTTON_ADD_VER_BASIC);

			// ��ư: �߰� (������)
			BUTTON_ADD_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_VER_EXTRA, "�߰�(H)");
			DGShowItem (dialogID, BUTTON_ADD_VER_EXTRA);

			// ��ư: ���� (������)
			BUTTON_DEL_VER_BASIC = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 250 + 30, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_VER_BASIC, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_VER_BASIC, "����(L)");
			DGShowItem (dialogID, BUTTON_DEL_VER_BASIC);

			// ��ư: ���� (������)
			BUTTON_DEL_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250 + 30, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_VER_EXTRA, "����(H)");
			DGShowItem (dialogID, BUTTON_DEL_VER_EXTRA);

			// ��: ���� ���� (������)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 317, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ����");
			DGShowItem (dialogID, itmIdx);

			// Edit��Ʈ��: ���� ���� (������)
			EDITCONTROL_REMAIN_HEIGHT_BASIC = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 105, 310, 70, 25);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC);

			// Edit��Ʈ��: ���� ���� (������)
			EDITCONTROL_REMAIN_HEIGHT_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 185, 310, 70, 25);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA);

			// ���ʿ� ������
			itmPosX = 105;
			itmPosY = 820;
			currentRowBasic = placingZone.nCellsInVerBasic;
			currentRowExtra = placingZone.nCellsInVerExtra;
			for (xx = 0 ; xx < currentRowBasic ; ++xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
				DGShowItem (dialogID, itmIdx);

				POPUP_HEIGHT_BASIC [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
				DGSetItemFont (dialogID, POPUP_HEIGHT_BASIC [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, "1200");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, "900");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, "600");
				DGPopUpSelectItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, POPUP_HEIGHT_BASIC [xx]);

				itmPosY -= 50;
			}

			// �����ʿ� ������
			itmPosX = 185;
			itmPosY = 820;
			if (placingZone.bExtra == true) {
				for (xx = 0 ; xx < currentRowExtra ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
					DGShowItem (dialogID, itmIdx);

					POPUP_HEIGHT_EXTRA [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
					DGSetItemFont (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_IS_LARGE | DG_IS_PLAIN);
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, "1200");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, "900");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, "600");
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);
					DGShowItem (dialogID, POPUP_HEIGHT_EXTRA [xx]);

					itmPosY -= 50;
				}
			}

			//////////////////////////////////////////////////////////// ���̾�α� ũ�� ����, ���� �ʺ� �� ���� ���
			// ���̾�α� ũ�� ����
			dialogSizeX = 550;
			dialogSizeY = 950;
			if (currentCol >= 5) {
				DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (currentCol - 5), dialogSizeY, DG_TOPLEFT, true);
			}

			// ���� �ʺ� ���
			totalWidth = 0.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_LINCORNER);
			if (DGGetItemValLong (dialogID, CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_RINCORNER);
			for (xx = 0 ; xx < currentCol ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_WIDTH [xx]))).ToCStr ().Get ()) / 1000;
				else
					totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			// ���� ���� ��� (������)
			totalHeight = 0.0;
			for (xx = 0 ; xx < currentRowBasic ; ++xx) {
				totalHeight += atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_BASIC [xx]))).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

			// ���� ���� ��� (������)
			totalHeight = 0.0;
			for (xx = 0 ; xx < currentRowExtra ; ++xx) {
				totalHeight += atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_EXTRA [xx]))).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);

			break;

		case DG_MSG_CHANGE:
			// ����/���� ������ ��
			if (item == POPUP_DIRECTION) {
				// ...
			}

			// ���ڳ� ��ư Ŭ���� ��
			else if (item == CHECKBOX_LINCORNER) {
				// ...
			}
			else if (item == CHECKBOX_RINCORNER) {
				// ...
			}

			// ���ڳ� �ʺ� ����� ��
			else if (item == EDITCONTROL_LINCORNER) {
				// ...
			}
			else if (item == EDITCONTROL_RINCORNER) {
				// ...
			}
			
			else {
				for (xx = 0 ; xx < currentCol ; ++xx) {
					// ��ü Ÿ�� ������ ��
					if (item == POPUP_OBJ_TYPE [xx]) {
						// ...
					}
					// �� �ʺ� ����� �� (���̺���, ������)
					else if (item == POPUP_WIDTH [xx]) {
						// ...
					}
					// �� �ʺ� ����� �� (�ٷ������̼�, ����, ����)
					else if (item == EDITCONTROL_WIDTH [xx]) {
						// ...
					}
				}

				for (xx = 0 ; xx < currentRowBasic ; ++xx) {
					// �� ���� ����� �� (������)
					if (item == POPUP_HEIGHT_BASIC [xx]) {
						// ...
					}
					// �� ���� ����� �� (������)
					else if (item == POPUP_HEIGHT_EXTRA [xx]) {
						// ...
					}
				}
			}

		case DG_MSG_CLICK:
			// Ȯ�� ��ư
			if (item == DG_OK) {
				// ...
//				placingZone.gap = 0.0;				// ������ ������ 0.0
//				
//				if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION)
//					placingZone.orientation = VERTICAL_DIRECTION;
//				else
//					placingZone.orientation = HORIZONTAL_DIRECTION;

//				// Ÿ�� ���� (Ÿ��A, Ÿ��B)
//				bLayerInd_Euroform = false;
//				bLayerInd_RectPipe = false;
//				bLayerInd_PinBolt = false;
//				bLayerInd_WallTie = false;
//				bLayerInd_HeadPiece = false;
//				bLayerInd_Join = false;

//				bLayerInd_SlabTableform = false;
//				bLayerInd_Profile = false;

//				bLayerInd_Steelform = false;
//				bLayerInd_Plywood = false;
//				bLayerInd_Fillersp = false;
//				bLayerInd_OutcornerAngle = false;
//				bLayerInd_OutcornerPanel = false;
//				bLayerInd_IncornerPanel = false;
//				bLayerInd_RectpipeHanger = false;
//				bLayerInd_EuroformHook = false;
//				bLayerInd_Hidden = false;

//				if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR_CUSTOM) == 1) {
//					placingZone.type = 1;

//					bLayerInd_Euroform = true;
//					bLayerInd_RectPipe = true;
//					bLayerInd_PinBolt = true;
//					bLayerInd_HeadPiece = true;
//					bLayerInd_Join = true;
//				} else if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR_CUSTOM) == 2) {
//					placingZone.type = 2;

//					bLayerInd_Euroform = true;
//					bLayerInd_RectPipe = true;
//					bLayerInd_RectpipeHanger = true;
//					bLayerInd_EuroformHook = true;
//					bLayerInd_HeadPiece = true;
//					bLayerInd_Join = true;
//					bLayerInd_Hidden = false;

//				} else if ((DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR_CUSTOM) == 3) || (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR_CUSTOM) == 4)) {
//					if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR_CUSTOM) == 3)
//						placingZone.type = 3;
//					else if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR_CUSTOM) == 4)
//						placingZone.type = 4;

//					bLayerInd_Euroform = true;
//					bLayerInd_RectPipe = true;
//					bLayerInd_PinBolt = true;
//					bLayerInd_HeadPiece = true;
//					bLayerInd_Join = true;
//					bLayerInd_Hidden = false;
//				}

//				placingZone.nCells = customTableCol;
//				placingZone.nCells_vertical = customTableRow;

//				accX = 0.0;
//				accZ = 0.0;

//				// ������ �ʺ�, ���� �� ����
//				for (xx = 1 ; xx <= customTableRow ; ++xx) {
//					for (yy = 1 ; yy <= customTableCol ; ++yy) {
//						widthInd = REST_ITEM_START_CUSTOM + (xx-1)*3*customTableCol + 3*(yy-1) + 1;
//						heightInd = REST_ITEM_START_CUSTOM + (xx-1)*3*customTableCol + 3*(yy-1) + 2;

//						placingZone.customCells [xx-1][yy-1].ang = placingZone.ang;
//						placingZone.customCells [xx-1][yy-1].horLen = atof (DGPopUpGetItemText (dialogID, widthInd, static_cast<short>(DGGetItemValLong (dialogID, widthInd))).ToCStr ().Get ()) / 1000;
//						placingZone.customCells [xx-1][yy-1].verLen = atof (DGPopUpGetItemText (dialogID, heightInd, static_cast<short>(DGGetItemValLong (dialogID, heightInd))).ToCStr ().Get ()) / 1000;
//						placingZone.customCells [xx-1][yy-1].leftBottomX = placingZone.leftBottomX;
//						placingZone.customCells [xx-1][yy-1].leftBottomY = placingZone.leftBottomY;
//						placingZone.customCells [xx-1][yy-1].leftBottomZ = placingZone.leftBottomZ;

//						moveIn3D ('x', placingZone.customCells [xx-1][yy-1].ang, accX, &placingZone.customCells [xx-1][yy-1].leftBottomX, &placingZone.customCells [xx-1][yy-1].leftBottomY, &placingZone.customCells [xx-1][yy-1].leftBottomZ);
//						moveIn3D ('z', placingZone.customCells [xx-1][yy-1].ang, accZ, &placingZone.customCells [xx-1][yy-1].leftBottomX, &placingZone.customCells [xx-1][yy-1].leftBottomY, &placingZone.customCells [xx-1][yy-1].leftBottomZ);

//						if (yy < customTableCol)
//							accX += placingZone.customCells [xx-1][yy-1].horLen;
//						else
//							accX = 0.0;
//							
//						if (yy == customTableCol)
//							accZ += placingZone.customCells [xx-1][yy-1].verLen;
//					}
//				}

//				placingZone.marginTop = placingZone.verLen - accZ;
			} else if (item == DG_CANCEL) {
				// ó���� ���� ����
			} else {


				// ���� - �߰� ��ư Ŭ��
				if (item == BUTTON_ADD_HOR) {
					// ...
				}

				// ���� - ���� ��ư Ŭ��
				else if (item == BUTTON_DEL_HOR) {
					// ...
				}

				// ���� - �߰�(L) ��ư Ŭ��
				else if (item == BUTTON_ADD_VER_BASIC) {
					// ...
				}

				// ���� - �߰�(H) ��ư Ŭ��
				else if (item == BUTTON_ADD_VER_EXTRA) {
					// ...
				}

				// ���� - ����(L) ��ư Ŭ��
				else if (item == BUTTON_DEL_VER_BASIC) {
					// ...
				}

				// ���� - ����(H) ��ư Ŭ��
				else if (item == BUTTON_DEL_VER_EXTRA) {
					// ...
				}

				else {
					// ��ü ��ư Ŭ��
					for (xx = 0 ; xx < currentCol ; ++xx) {
						if (item == BUTTON_OBJ [xx]) {
							// ...
						}
					}
				}
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
	//API_UCCallbackType	ucb;

	//switch (message) {
	//	case DG_MSG_INIT:
	//		// ���̾�α� Ÿ��Ʋ
	//		DGSetDialogTitle (dialogID, "������ ���̾� �����ϱ�");

	//		//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
	//		// ���� ��ư
	//		DGSetItemText (dialogID, DG_OK, "Ȯ ��");

	//		// ���� ��ư
	//		DGSetItemText (dialogID, DG_CANCEL, "�� ��");

	//		// üũ�ڽ�: ���̾� ����
	//		DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING_CUSTOM, "���̾� ����");
	//		DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_CUSTOM, TRUE);

	//		// ���̾� ���� ��
	//		DGSetItemText (dialogID, LABEL_LAYER_SETTINGS_CUSTOM, "���纰 ���̾� ����");
	//		DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM_CUSTOM, "������ ���̺���");
	//		DGSetItemText (dialogID, LABEL_LAYER_PROFILE_CUSTOM, "C����");
	//		DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_CUSTOM, "������");
	//		DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_CUSTOM, "��� ������");
	//		DGSetItemText (dialogID, LABEL_LAYER_PINBOLT_CUSTOM, "�ɺ�Ʈ ��Ʈ");
	//		DGSetItemText (dialogID, LABEL_LAYER_WALLTIE_CUSTOM, "��ü Ÿ��");
	//		DGSetItemText (dialogID, LABEL_LAYER_JOIN_CUSTOM, "����ö��");
	//		DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE_CUSTOM, "����ǽ�");
	//		DGSetItemText (dialogID, LABEL_LAYER_STEELFORM_CUSTOM, "��ƿ��");
	//		DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD_CUSTOM, "����");
	//		DGSetItemText (dialogID, LABEL_LAYER_FILLERSP_CUSTOM, "�ٷ������̼�");
	//		DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE_CUSTOM, "�ƿ��ڳʾޱ�");
	//		DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL_CUSTOM, "�ƿ��ڳ��ǳ�");
	//		DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL_CUSTOM, "���ڳ��ǳ�");
	//		DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER_CUSTOM, "�����������");
	//		DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK_CUSTOM, "������ ��ũ");
	//		DGSetItemText (dialogID, LABEL_LAYER_HIDDEN_CUSTOM, "����");

	//		DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 580, 160, 25);
	//		DGSetItemFont (dialogID, BUTTON_AUTOSET_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_AUTOSET_CUSTOM, "���̾� �ڵ� ����");
	//		DGShowItem (dialogID, BUTTON_AUTOSET_CUSTOM);

	//		// ���� ��Ʈ�� �ʱ�ȭ
	//		BNZeroMemory (&ucb, sizeof (ucb));
	//		ucb.dialogID = dialogID;
	//		ucb.type	 = APIUserControlType_Layer;
	//		ucb.itemID	 = USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, 1);
	//		if (bLayerInd_SlabTableform == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_SLABTABLEFORM_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_PROFILE_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, 1);
	//		if (bLayerInd_Profile == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_PROFILE_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_PROFILE_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, 1);
	//		if (bLayerInd_Euroform == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_EUROFORM_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, 1);
	//		if (bLayerInd_RectPipe == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_PINBOLT_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, 1);
	//		if (bLayerInd_PinBolt == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_PINBOLT_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_PINBOLT_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_WALLTIE_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, 1);
	//		if (bLayerInd_WallTie == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_WALLTIE_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_WALLTIE_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_JOIN_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, 1);
	//		if (bLayerInd_Join == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_JOIN_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_JOIN_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, 1);
	//		if (bLayerInd_HeadPiece == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_HEADPIECE_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_HEADPIECE_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_STEELFORM_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, 1);
	//		if (bLayerInd_Steelform == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_STEELFORM_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_STEELFORM_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, 1);
	//		if (bLayerInd_Plywood == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_PLYWOOD_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_FILLERSP_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, 1);
	//		if (bLayerInd_Fillersp == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_FILLERSP_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_FILLERSP_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, 1);
	//		if (bLayerInd_OutcornerAngle == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, 1);
	//		if (bLayerInd_OutcornerPanel == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, 1);
	//		if (bLayerInd_IncornerPanel == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_INCORNER_PANEL_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_INCORNER_PANEL_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, 1);
	//		if (bLayerInd_RectpipeHanger == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, 1);
	//		if (bLayerInd_EuroformHook == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM);
	//		}

	//		ucb.itemID	 = USERCONTROL_LAYER_HIDDEN_CUSTOM;
	//		ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
	//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN_CUSTOM, 1);
	//		if (bLayerInd_Hidden == true) {
	//			DGEnableItem (dialogID, LABEL_LAYER_HIDDEN_CUSTOM);
	//			DGEnableItem (dialogID, USERCONTROL_LAYER_HIDDEN_CUSTOM);
	//		} else {
	//			DGDisableItem (dialogID, LABEL_LAYER_HIDDEN_CUSTOM);
	//			DGDisableItem (dialogID, USERCONTROL_LAYER_HIDDEN_CUSTOM);
	//		}
	//		break;

	//	case DG_MSG_CHANGE:
	//		// ���̾� ���� �ٲ�
	//		if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_CUSTOM) == 1) {
	//			switch (item) {
	//				case USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM:
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_PROFILE_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_EUROFORM_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_RECTPIPE_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_PINBOLT_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_WALLTIE_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_JOIN_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_HEADPIECE_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_STEELFORM_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_PLYWOOD_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_FILLERSP_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM));
	//					break;
	//				case USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM:
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM));
	//					break;
	//			}
	//		}

	//		break;

	//	case DG_MSG_CLICK:
	//		switch (item) {
	//			case DG_OK:
	//				// ���̾� ��ȣ ����
	//				if (bLayerInd_SlabTableform == true)	layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM);
	//				if (bLayerInd_Profile == true)			layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE_CUSTOM);
	//				if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM);
	//				if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM);
	//				if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM);
	//				if (bLayerInd_WallTie == true)			layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE_CUSTOM);
	//				if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM);
	//				if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM);
	//				if (bLayerInd_Steelform == true)		layerInd_Steelform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM_CUSTOM);
	//				if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM);
	//				if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP_CUSTOM);
	//				if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM);
	//				if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM);
	//				if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM);
	//				if (bLayerInd_RectpipeHanger == true)	layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM);
	//				if (bLayerInd_EuroformHook == true)		layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM);
	//				if (bLayerInd_Hidden == true)			layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN_CUSTOM);

	//				break;

	//			case BUTTON_AUTOSET_CUSTOM:
	//				item = 0;

	//				DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_CUSTOM, FALSE);

	//				if (placingZone.type == 1) {
	//					layerInd_Euroform	= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
	//					layerInd_RectPipe	= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
	//					layerInd_PinBolt	= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
	//					layerInd_Join		= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
	//					layerInd_HeadPiece	= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
	//					layerInd_Plywood	= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);

	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, layerInd_Euroform);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, layerInd_RectPipe);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, layerInd_PinBolt);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, layerInd_Join);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, layerInd_HeadPiece);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, layerInd_Plywood);

	//				} else if (placingZone.type == 2) {
	//					layerInd_Euroform	= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
	//					layerInd_RectPipe	= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
	//					layerInd_RectpipeHanger	= makeTemporaryLayer (structuralObject_forTableformWall, "JOIB", NULL);
	//					layerInd_EuroformHook	= makeTemporaryLayer (structuralObject_forTableformWall, "HOOK", NULL);
	//					layerInd_Join		= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
	//					layerInd_HeadPiece	= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
	//					layerInd_Plywood	= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);

	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, layerInd_Euroform);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, layerInd_RectPipe);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM, layerInd_RectpipeHanger);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM, layerInd_EuroformHook);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, layerInd_Join);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, layerInd_HeadPiece);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, layerInd_Plywood);

	//				} else if ((placingZone.type == 3) || (placingZone.type == 4)) {
	//					layerInd_Euroform	= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
	//					layerInd_RectPipe	= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
	//					layerInd_PinBolt	= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
	//					layerInd_Join		= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
	//					layerInd_HeadPiece	= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
	//					layerInd_Plywood	= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);

	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_CUSTOM, layerInd_Euroform);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_CUSTOM, layerInd_RectPipe);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT_CUSTOM, layerInd_PinBolt);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN_CUSTOM, layerInd_Join);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE_CUSTOM, layerInd_HeadPiece);
	//					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_CUSTOM, layerInd_Plywood);
	//				}

	//				break;

	//			case DG_CANCEL:
	//				break;
	//		}
	//	case DG_MSG_CLOSE:
	//		switch (item) {
	//			case DG_CLOSEBOX:
	//				break;
	//		}
	//}

	//result = item;

	return	result;
}
