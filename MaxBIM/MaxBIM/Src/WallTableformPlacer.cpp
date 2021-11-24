#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "WallTableformPlacer.hpp"

using namespace wallTableformPlacerDG;

static WallTableformPlacingZone		placingZone;		// 기본 벽면 영역 정보
static InfoWall						infoWall;			// 벽 객체 정보
API_Guid		structuralObject_forTableformWall;		// 구조 객체의 GUID

static short	layerInd_Euroform;			// 레이어 번호: 유로폼 (공통)
static short	layerInd_RectPipe;			// 레이어 번호: 비계 파이프 (공통)
static short	layerInd_PinBolt;			// 레이어 번호: 핀볼트 세트 (A,C타입)
static short	layerInd_WallTie;			// 레이어 번호: 빅체 타이 (A타입, 더 이상 사용하지 않음)
static short	layerInd_Clamp;				// 레이어 번호: 직교 클램프 (더 이상 사용하지 않음)
static short	layerInd_HeadPiece;			// 레이어 번호: 헤드피스 (A,B,C타입, B,C타입에서는 빔조인트용 Push-Pull Props 헤드피스)
static short	layerInd_Join;				// 레이어 번호: 결합철물 (A,B,C타입, B,C타입에서는 사각파이프 연결철물)
static short	layerInd_Plywood;			// 레이어 번호: 합판 (공통)
static short	layerInd_Wood;				// 레이어 번호: 목재 (공통)
static short	layerInd_EuroformHook;		// 레이어 번호: 유로폼 후크 (B타입)
static short	layerInd_BlueClamp;			// 레이어 번호: 블루 클램프
static short	layerInd_BlueTimberRail;	// 레이어 번호: 블루 목심
static short	layerInd_Hidden;			// 레이어 번호: 숨김 (B,C타입, 비계파이프 타공을 위한 솔리드 빼기 연산용 객체)

// 커스텀 전용
static bool		bLayerInd_Euroform;			// 레이어 번호: 유로폼
static bool		bLayerInd_RectPipe;			// 레이어 번호: 비계 파이프
static bool		bLayerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
static bool		bLayerInd_WallTie;			// 레이어 번호: 벽체 타이
static bool		bLayerInd_HeadPiece;		// 레이어 번호: 헤드피스
static bool		bLayerInd_Join;				// 레이어 번호: 결합철물

static bool		bLayerInd_SlabTableform;	// 레이어 번호: 슬래브 테이블폼
static bool		bLayerInd_Profile;			// 레이어 번호: KS프로파일

static bool		bLayerInd_Steelform;		// 레이어 번호: 스틸폼
static bool		bLayerInd_Plywood;			// 레이어 번호: 합판
static bool		bLayerInd_Fillersp;			// 레이어 번호: 휠러스페이서
static bool		bLayerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static bool		bLayerInd_OutcornerPanel;	// 레이어 번호: 아웃코너앵글
static bool		bLayerInd_IncornerPanel;	// 레이어 번호: 인코너앵글
static bool		bLayerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거
static bool		bLayerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
static bool		bLayerInd_Hidden;			// 레이어 번호: 숨김

static short	layerInd_SlabTableform;		// 레이어 번호: 슬래브 테이블폼
static short	layerInd_Profile;			// 레이어 번호: KS프로파일

static short	layerInd_Steelform;			// 레이어 번호: 스틸폼
static short	layerInd_Fillersp;			// 레이어 번호: 휠러스페이서
static short	layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static short	layerInd_OutcornerPanel;	// 레이어 번호: 아웃코너앵글
static short	layerInd_IncornerPanel;		// 레이어 번호: 인코너앵글
static short	layerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거

static GS::Array<API_Guid>	elemList;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함

// 다이얼로그 동적 요소 인덱스 번호 저장
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

static short	BUTTON_OBJ [10];
static short	POPUP_OBJ_TYPE [10];
static short	POPUP_TABLEFORM_TYPE [10];
static short	POPUP_DIRECTION [10];
static short	POPUP_WIDTH [10];
static short	EDITCONTROL_WIDTH [10];

//static double	preferWidth;
static bool		clickedPrevButton;		// 이전 버튼을 눌렀습니까?


// 벽에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx;
	double		dx, dy;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>		walls;
	GS::Array<API_Guid>		morphs;
	long					nWalls = 0;
	long					nMorphs = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 객체 정보
	InfoMorphForWallTableform	infoMorph [2];
	InfoMorphForWallTableform	infoMorph_Basic;
	InfoMorphForWallTableform	infoMorph_Extra;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_wall;		// 벽의 작업 층 높이


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 벽 (1개), 벽을 덮는 모프 (1개)\n옵션 선택: 벽을 덮는 모프(뒷면 - 1차 모프와 높이가 다름) (1개)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 벽 1개, 모프 1~2개 선택해야 함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_WallID)		// 벽인가?
				walls.Push (tElem.header.guid);

			if (tElem.header.typeID == API_MorphID)		// 모프인가?
				morphs.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nWalls = walls.GetSize ();
	nMorphs = morphs.GetSize ();

	// 벽이 1개인가?
	if (nWalls != 1) {
		ACAPI_WriteReport ("벽을 1개 선택해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개 또는 2개인가?
	if ((nMorphs < 1) || (nMorphs > 2)) {
		ACAPI_WriteReport ("벽을 덮는 모프를 1개 또는 2개를 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// (1) 벽 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = walls.Pop ();
	structuralObject_forTableformWall = elem.header.guid;
	err = ACAPI_Element_Get (&elem);						// elem.wall.poly.nCoords : 폴리곤 수를 가져올 수 있음
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);	// memo.coords : 폴리곤 좌표를 가져올 수 있음
	
	if (elem.wall.thickness != elem.wall.thickness1) {
		ACAPI_WriteReport ("벽의 두께는 균일해야 합니다.", true);
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

	// (2) 모프 정보를 가져옴
	for (xx = 0 ; xx < nMorphs ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = morphs.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

		// 만약 모프가 누워 있으면(세워져 있지 않으면) 중단
		if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
			ACAPI_WriteReport ("모프가 세워져 있지 않습니다.", true);
			err = APIERR_GENERAL;
			return err;
		}

		// 모프의 GUID 저장
		infoMorph [xx].guid = elem.header.guid;

		// 모프의 좌하단, 우상단 점 지정
		if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
			// 좌하단 좌표 결정
			infoMorph [xx].leftBottomX = elem.morph.tranmat.tmx [3];
			infoMorph [xx].leftBottomY = elem.morph.tranmat.tmx [7];
			infoMorph [xx].leftBottomZ = elem.morph.tranmat.tmx [11];

			// 우상단 좌표는?
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
			// 우상단 좌표 결정
			infoMorph [xx].rightTopX = elem.morph.tranmat.tmx [3];
			infoMorph [xx].rightTopY = elem.morph.tranmat.tmx [7];
			infoMorph [xx].rightTopZ = elem.morph.tranmat.tmx [11];

			// 좌하단 좌표는?
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

		// 모프의 Z축 회전 각도 (벽의 설치 각도)
		dx = infoMorph [xx].rightTopX - infoMorph [xx].leftBottomX;
		dy = infoMorph [xx].rightTopY - infoMorph [xx].leftBottomY;
		infoMorph [xx].ang = RadToDegree (atan2 (dy, dx));

		// 모프의 가로 길이
		infoMorph [xx].horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);

		// 모프의 세로 길이
		infoMorph [xx].verLen = abs (info3D.bounds.zMax - info3D.bounds.zMin);

		// 영역 모프 제거
		API_Elem_Head* headList = new API_Elem_Head [1];
		headList [0] = elem.header;
		err = ACAPI_Element_Delete (&headList, 1);
		delete headList;
	}

	if (nMorphs == 1) {
		// 모프가 1개일 경우, 기본 모프만 설정
		infoMorph_Basic = infoMorph [0];
		infoMorph_Extra = infoMorph [0];
	} else {
		// 모프가 2개일 경우, 세로 길이가 낮은 것이 기본 모프임
		if (infoMorph [0].verLen > infoMorph [1].verLen) {
			infoMorph_Basic = infoMorph [1];
			infoMorph_Extra = infoMorph [0];
		} else {
			infoMorph_Basic = infoMorph [0];
			infoMorph_Extra = infoMorph [1];
		}
	}

	// 벽면 모프를 통해 영역 정보 업데이트
	placingZone.leftBottomX		= infoMorph_Basic.leftBottomX;
	placingZone.leftBottomY		= infoMorph_Basic.leftBottomY;
	placingZone.leftBottomZ		= infoMorph_Basic.leftBottomZ;
	placingZone.horLen			= infoMorph_Basic.horLen;
	placingZone.verLen			= infoMorph_Basic.verLen;
	placingZone.ang				= DegreeToRad (infoMorph_Basic.ang);
	
	// 작업 층 높이 반영 -- 모프
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

	// 영역 정보의 고도 정보를 수정
	placingZone.leftBottomZ = infoWall.bottomOffset;

	clickedPrevButton = false;

	// ... 초기 셀 개수 계산
	placingZone.nCellsInHor = ceil (placingZone.horLen / 2.300);
	placingZone.nCellsInVerBasic = ceil (placingZone.verLen / 1.200);
	placingZone.nCellsInVerExtra = placingZone.nCellsInVerBasic + 1;

	// [DIALOG] 1번째 다이얼로그에서 인코너 유무 및 길이, 테이블폼의 방향과 가로/세로 방향 유로폼의 개수와 각각의 길이를 선택함
	result = DGBlankModalDialog (550, 650, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler1, 0);

	if (result != DG_OK)
		return err;

	// [DIALOG] 2번째 다이얼로그에서 부재별 레이어를 지정함
	//result = DGModalDialog (ACAPI_GetOwnResModule (), 32519, ACAPI_GetOwnResModule (), wallTableformPlacerHandler2, 0);

	if (result != DG_OK)
		return err;

	// 셀 정보 초기화
	// ...

	// 테이블폼 배치하기
	// ...

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// 셀 정보 초기화
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

// 테이블폼/유로폼/휠러스페이서/합판/목재 배치를 위한 다이얼로그 (테이블폼 구성, 요소 방향, 개수 및 길이)
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy, zz;
	//char	buffer [256];
	//short	boxSizeX = 120, boxSizeY = 120;
	//double	totalWidth, totalHeight;
	//bool	bChanged;						// 화면이 변경되는가?
	//static short	customTableRow = 1;		// 행 현재 개수
	//static short	customTableCol = 1;		// 열 현재 개수
	//const short		maxRow = 5;				// 행 최대 개수
	//const short		maxCol = 5;				// 열 최대 개수
	//short			widthInd, heightInd;	// 너비, 높이를 가리키는 팝업컨트롤 인덱스
	//double			accX, accZ;				// 유로폼을 설치하기 위한 거리를 계산하기 위한 변수
	//static short	dialogSizeX;			// 현재 다이얼로그 크기 X
	//static short	dialogSizeY;			// 현재 다이얼로그 크기 Y
	//static short	itmIdx_width [10][10];
	//static short	itmIdx_height [10][10];

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "테이블폼/인코너/유로폼/휠러스페이서/합판/목재 구성");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 200, 600, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확 인");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 300, 600, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취 소");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// 아이템 배치 (라벨: 정면, 버튼: 추가, 삭제, 그외: 남은 너비)
			// 라벨: 정면
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 17, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "정면");
			DGShowItem (dialogID, itmIdx);

			// 버튼: 추가
			BUTTON_ADD_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 10, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_HOR, "추가");
			DGShowItem (dialogID, BUTTON_ADD_HOR);

			// 버튼: 삭제
			BUTTON_DEL_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 10, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_HOR, "삭제");
			DGShowItem (dialogID, BUTTON_DEL_HOR);

			// 라벨: 남은 너비
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 290, 17, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "남은 너비");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 남은 너비
			EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 375, 10, 80, 25);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_WIDTH);

			//////////////////////////////////////////////////////////// 아이템 배치 (정면 관련 버튼)
			// 좌측 인코너 유무 (체크버튼)
			CHECKBOX_LINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 20, 135, 70, 70);
			DGSetItemFont (dialogID, CHECKBOX_LINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_LINCORNER, "인코너");
			DGShowItem (dialogID, CHECKBOX_LINCORNER);
			// 좌측 인코너 길이 (Edit컨트롤)
			EDITCONTROL_LINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 20, 205, 70, 25);
			DGShowItem (dialogID, EDITCONTROL_LINCORNER);

			// 일반 셀: 기본값은 테이블폼
			itmPosX = 90;
			itmPosY = 137;
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				// 버튼
				BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
				DGSetItemFont (dialogID, BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, BUTTON_OBJ [xx], "테이블폼");
				DGShowItem (dialogID, BUTTON_OBJ [xx]);

				// 객체 타입 (팝업컨트롤)
				POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 75, 70, 23);
				DGSetItemFont (dialogID, POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "없음");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "테이블폼");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "유로폼");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "휠러스페이서");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "합판");
				DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "각재");
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, POPUP_OBJ_TYPE [xx]);

				// 테이블폼 타입 (팝업컨트롤)
				POPUP_TABLEFORM_TYPE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 50, 70, 23);
				DGSetItemFont (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_POPUP_BOTTOM, "타입A");
				DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_POPUP_BOTTOM, "타입B");
				DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_POPUP_BOTTOM, "타입C");
				DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_POPUP_BOTTOM, "타입D");
				DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_TYPE [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, POPUP_TABLEFORM_TYPE [xx]);

				// 방향 (팝업컨트롤)
				POPUP_DIRECTION [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
				DGSetItemFont (dialogID, POPUP_DIRECTION [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, POPUP_DIRECTION [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_DIRECTION [xx], DG_POPUP_BOTTOM, "가로");
				DGPopUpInsertItem (dialogID, POPUP_DIRECTION [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_DIRECTION [xx], DG_POPUP_BOTTOM, "세로");
				DGPopUpSelectItem (dialogID, POPUP_DIRECTION [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, POPUP_DIRECTION [xx]);

				// 너비 (팝업컨트롤)
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

				// 너비 (팝업컨트롤) - 처음에는 숨김
				EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);

				itmPosX += 70;
			}

			// 우측 인코너 유무 (체크버튼)
			CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
			DGSetItemFont (dialogID, CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_RINCORNER, "인코너");
			DGShowItem (dialogID, CHECKBOX_RINCORNER);
			// 우측 인코너 길이 (Edit컨트롤)
			EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
			DGShowItem (dialogID, EDITCONTROL_RINCORNER);

			//////////////////////////////////////////////////////////// 아이템 배치 (측면 관련 버튼)
			// 구분자
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 5, 240, 550-10, 1);
			DGShowItem (dialogID, itmIdx);

			// 라벨: 측면
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 257, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "측면");
			DGShowItem (dialogID, itmIdx);

			// 버튼: 추가 (낮은쪽)
			BUTTON_ADD_VER_BASIC = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 250, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_VER_BASIC, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_VER_BASIC, "추가(L)");
			DGShowItem (dialogID, BUTTON_ADD_VER_BASIC);

			// 버튼: 추가 (높은쪽)
			BUTTON_ADD_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_VER_EXTRA, "추가(H)");
			DGShowItem (dialogID, BUTTON_ADD_VER_EXTRA);

			// 버튼: 삭제 (낮은쪽)
			BUTTON_DEL_VER_BASIC = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 250 + 30, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_VER_BASIC, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_VER_BASIC, "삭제(L)");
			DGShowItem (dialogID, BUTTON_DEL_VER_BASIC);

			// 버튼: 삭제 (높은쪽)
			BUTTON_DEL_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250 + 30, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_VER_EXTRA, "삭제(H)");
			DGShowItem (dialogID, BUTTON_DEL_VER_EXTRA);

			// 라벨: 남은 높이 (낮은쪽)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 317, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "남은 높이");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 남은 높이 (낮은쪽)
			EDITCONTROL_REMAIN_HEIGHT_BASIC = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 105, 310, 70, 25);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC);

			// Edit컨트롤: 남은 높이 (높은쪽)
			EDITCONTROL_REMAIN_HEIGHT_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 185, 310, 70, 25);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA);

			// ... 테이블폼은 한 셀마다 여러 개의 수직 방향 유로폼이 있음을 고려할 것!

			//placingZone.nCellsInVerBasic
			// 왼쪽에 낮은쪽
			//placingZone.nCellsInVerExtra
			// 오른쪽에 높은쪽

			//////////////////////////////////////////////////////////// 아이템 배치 (벽과의 간격, 양면/단면)

			//////////////////////////////////////////////////////////// 기타
			// 다이얼로그 크기 설정
			// ...

			// 남은 너비 계산
			// ...

			// 남은 높이 계산
			// ...





	//		//////////////////////////////////////////////////////////// 아이템 배치 (나머지)
	//		// 팝업컨트롤: 타입
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 1, 95, 10, 70, 25);
	//		DGSetItemFont (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM);
	//		DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM, "타입A");
	//		DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM);
	//		DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM, "타입B");
	//		DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM);
	//		DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM, "타입C");
	//		DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM);
	//		DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM, "타입D");
	//		DGShowItem (dialogID, POPUP_TYPE_SELECTOR_CUSTOM);

	//		// 라벨: 테이블폼 방향
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 20, 80, 23);
	//		DGSetItemFont (dialogID, LABEL_TABLEFORM_ORIENTATION_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, LABEL_TABLEFORM_ORIENTATION_CUSTOM, "테이블폼 방향");
	//		DGShowItem (dialogID, LABEL_TABLEFORM_ORIENTATION_CUSTOM);

	//		// 팝업컨트롤: 테이블폼 방향
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 10, 300, 15, 100, 23);
	//		DGSetItemFont (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_BOTTOM);
	//		DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_BOTTOM, "세로방향");
	//		DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_BOTTOM);
	//		DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_BOTTOM, "가로방향");
	//		DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_TOP);
	//		DGShowItem (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM);

	//		// 라벨: 총 너비 ("min ~ max 가능")
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 120, 60, 150, 25);
	//		if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION)
	//			strcpy (buffer, "총 너비\n(400 ~ 2300 가능)");
	//		else
	//			strcpy (buffer, "총 너비\n(1500 ~ 6000 가능)");
	//		DGSetItemFont (dialogID, LABEL_TOTAL_WIDTH_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, LABEL_TOTAL_WIDTH_CUSTOM, buffer);
	//		DGShowItem (dialogID, LABEL_TOTAL_WIDTH_CUSTOM);
	//		
	//		// Edit컨트롤: 총 너비
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 160, 90, 70, 25);
	//		DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH_CUSTOM);
	//		DGShowItem (dialogID, EDITCONTROL_TOTAL_WIDTH_CUSTOM);
	//		DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH_CUSTOM, placingZone.horLen);

	//		// 라벨: 남은 너비
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 250, 60, 100, 23);
	//		DGSetItemFont (dialogID, LABEL_REMAIN_WIDTH_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, LABEL_REMAIN_WIDTH_CUSTOM, "남은 너비");
	//		DGShowItem (dialogID, LABEL_REMAIN_WIDTH_CUSTOM);
	//		
	//		// Edit컨트롤: 남은 너비
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 265, 90, 70, 25);
	//		DGDisableItem (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM);
	//		DGShowItem (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM);

	//		// 라벨: 총 높이 ("min ~ max 가능")
	//		// 가로방향, 세로방향에 따라 뒤의 텍스트가 달라짐
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 140, 150, 25);
	//		if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION)
	//			strcpy (buffer, "총 높이\n(1500 ~ 6000 가능)");
	//		else
	//			strcpy (buffer, "총 높이\n(400 ~ 2300 가능)");
	//		DGSetItemFont (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM, buffer);
	//		DGShowItem (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM);

	//		// Edit컨트롤: 총 높이
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 50, 170, 70, 25);
	//		DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT_CUSTOM);
	//		DGShowItem (dialogID, EDITCONTROL_TOTAL_HEIGHT_CUSTOM);
	//		DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT_CUSTOM, placingZone.verLen);

	//		// 라벨: 남은 높이
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 35, 220, 100, 23);
	//		DGSetItemFont (dialogID, LABEL_REMAIN_HEIGHT_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, LABEL_REMAIN_HEIGHT_CUSTOM, "남은 높이");
	//		DGShowItem (dialogID, LABEL_REMAIN_HEIGHT_CUSTOM);

	//		// Edit컨트롤: 남은 높이
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 50, 240, 70, 25);
	//		DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM);
	//		DGShowItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM);

	//		// 버튼: 열 추가
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 435, 90, 70, 25);
	//		DGSetItemFont (dialogID, BUTTON_ADD_COL_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_ADD_COL_CUSTOM, "열 추가");
	//		DGShowItem (dialogID, BUTTON_ADD_COL_CUSTOM);

	//		// 버튼: 열 삭제
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 360, 90, 70, 25);
	//		DGSetItemFont (dialogID, BUTTON_DEL_COL_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_DEL_COL_CUSTOM, "열 삭제");
	//		DGShowItem (dialogID, BUTTON_DEL_COL_CUSTOM);

	//		// 버튼: 행 추가
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 50, 330, 70, 25);
	//		DGSetItemFont (dialogID, BUTTON_ADD_ROW_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_ADD_ROW_CUSTOM, "행 추가");
	//		DGShowItem (dialogID, BUTTON_ADD_ROW_CUSTOM);

	//		// 버튼: 행 삭제
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 50, 300, 70, 25);
	//		DGSetItemFont (dialogID, BUTTON_DEL_ROW_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_DEL_ROW_CUSTOM, "행 삭제");
	//		DGShowItem (dialogID, BUTTON_DEL_ROW_CUSTOM);

	//		// 다이얼로그 크기 초기화
	//		dialogSizeX = 550;
	//		dialogSizeY = 450;

	//		// 처음에는 행 개수 2개, 열 개수를 2개로 시작함
	//		customTableRow = 2;
	//		customTableCol = 2;

	//		// 여기부터는 itmIdx의 개수가 동적: REST_ITEM_START_CUSTOM부터
	//		itmPosX = 170;
	//		itmPosY = 150 + (customTableRow-1) * boxSizeY;

	//		for (xx = 0 ; xx < customTableRow ; ++xx) {
	//			for (yy = 0 ; yy < customTableCol ; ++yy) {
	//				// 구분자
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, boxSizeX, boxSizeY);
	//				DGShowItem (dialogID, itmIdx);

	//				// 팝업컨트롤: 너비
	//				itmIdx_width [xx][yy] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, itmPosX + boxSizeX/2 - 30, itmPosY + boxSizeY - 30, 60, 23);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//				DGPopUpDisableDraw (dialogID, itmIdx);
	//				if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
	//				} else {
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//				}
	//				DGPopUpEnableDraw (dialogID, itmIdx);
	//				DGShowItem (dialogID, itmIdx);
	//		
	//				// 팝업컨트롤: 높이
	//				itmIdx_height [xx][yy] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, itmPosX + 5, itmPosY + boxSizeY/2 - 15, 60, 23);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//				DGPopUpDisableDraw (dialogID, itmIdx);
	//				if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//				} else {
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
	//					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
	//				}
	//				DGPopUpEnableDraw (dialogID, itmIdx);
	//				DGShowItem (dialogID, itmIdx);

	//				itmPosX += boxSizeX;
	//			}
	//			itmPosX = 170;
	//			itmPosY -= boxSizeY;
	//		}

	//		// 남은 너비 표시
	//		totalWidth = 0.0;
	//		for (xx = 0 ; xx < customTableCol ; ++xx) {
	//			totalWidth += atof (DGPopUpGetItemText (dialogID, itmIdx_width [0][xx], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_width [0][xx]))).ToCStr ().Get ()) / 1000;
	//		}
	//		DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM, placingZone.horLen - totalWidth);

	//		// 남은 높이 표시
	//		totalHeight = 0.0;
	//		for (xx = 0 ; xx < customTableRow ; ++xx) {
	//			totalHeight += atof (DGPopUpGetItemText (dialogID, itmIdx_height [xx][0], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_height [xx][0]))).ToCStr ().Get ()) / 1000;
	//		}
	//		DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM, placingZone.verLen - totalHeight);
			
			break;

	//	case DG_MSG_CHANGE:
	//		switch (item) {
	//			case POPUP_TABLEFORM_ORIENTATION_CUSTOM:
	//				// 테이블폼 방향이 바뀌면?
	//				if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
	//					DGSetItemText (dialogID, LABEL_TOTAL_WIDTH_CUSTOM, "총 너비\n(400 ~ 2300 가능)");
	//					DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM, "총 높이\n(1500 ~ 6000 가능)");

	//					// 테이블폼 너비/높이 팝업컨트롤의 값이 바뀌어야 함
	//					for (xx = 0 ; xx <= (customTableCol * customTableRow) ; ++xx) {
	//						// 팝업컨트롤: 너비
	//						itmIdx = REST_ITEM_START_CUSTOM + xx*3 + 1;
	//						while (DGPopUpGetItemCount (dialogID, itmIdx) > 0)
	//						DGPopUpDeleteItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
	//		
	//						// 팝업컨트롤: 높이
	//						itmIdx = REST_ITEM_START_CUSTOM + xx*3 + 2;
	//						while (DGPopUpGetItemCount (dialogID, itmIdx) > 0)
	//							DGPopUpDeleteItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//					}
	//				} else {
	//					DGSetItemText (dialogID, LABEL_TOTAL_WIDTH_CUSTOM, "총 너비\n(1500 ~ 6000 가능)");
	//					DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM, "총 높이\n(400 ~ 2300 가능)");

	//					// 테이블폼 너비/높이 팝업컨트롤의 값이 바뀌어야 함
	//					for (xx = 0 ; xx <= (customTableCol * customTableRow) ; ++xx) {
	//						// 팝업컨트롤: 너비
	//						itmIdx = REST_ITEM_START_CUSTOM + xx*3 + 1;
	//						while (DGPopUpGetItemCount (dialogID, itmIdx) > 0)
	//							DGPopUpDeleteItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//		
	//						// 팝업컨트롤: 높이
	//						itmIdx = REST_ITEM_START_CUSTOM + xx*3 + 2;
	//						while (DGPopUpGetItemCount (dialogID, itmIdx) > 0)
	//							DGPopUpDeleteItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
	//						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
	//					}
	//				}

	//				// 남은 너비 표시
	//				totalWidth = 0.0;
	//				for (xx = 0 ; xx < customTableCol ; ++xx) {
	//					totalWidth += atof (DGPopUpGetItemText (dialogID, itmIdx_width [0][xx], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_width [0][xx]))).ToCStr ().Get ()) / 1000;
	//				}
	//				DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM, placingZone.horLen - totalWidth);

	//				// 남은 높이 표시
	//				totalHeight = 0.0;
	//				for (xx = 0 ; xx < customTableRow ; ++xx) {
	//					totalHeight += atof (DGPopUpGetItemText (dialogID, itmIdx_height [xx][0], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_height [xx][0]))).ToCStr ().Get ()) / 1000;
	//				}
	//				DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM, placingZone.verLen - totalHeight);

	//				break;

	//			default:
	//				for (xx = 0 ; xx < customTableRow ; ++xx) {
	//					for (yy = 0 ; yy < customTableCol ; ++yy) {
	//						if (item == itmIdx_width [xx][yy]) {
	//							// 어떤 유로폼의 너비를 변경하면, 같은 열의 모든 행 유로폼 너비도 같이 변경됨
	//							for (zz = 0 ; zz < customTableRow ; ++zz) {
	//								if (xx != zz) {
	//									DGPopUpSelectItem (dialogID, itmIdx_width [zz][yy], DGPopUpGetSelected (dialogID, itmIdx_width [xx][yy]));
	//								}
	//							}
	//						}

	//						if (item == itmIdx_height [xx][yy]) {
	//							// 어떤 유로폼의 높이를 변경하면, 같은 행의 모든 열 유로폼 너비도 같이 변경됨
	//							for (zz = 0 ; zz < customTableCol ; ++zz) {
	//								if (yy != zz) {
	//									DGPopUpSelectItem (dialogID, itmIdx_height [xx][zz], DGPopUpGetSelected (dialogID, itmIdx_height [xx][yy]));
	//								}
	//							}
	//						}
	//					}
	//				}

	//				// 남은 너비 표시
	//				totalWidth = 0.0;
	//				for (xx = 0 ; xx < customTableCol ; ++xx) {
	//					totalWidth += atof (DGPopUpGetItemText (dialogID, itmIdx_width [0][xx], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_width [0][xx]))).ToCStr ().Get ()) / 1000;
	//				}
	//				DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM, placingZone.horLen - totalWidth);

	//				// 남은 높이 표시
	//				totalHeight = 0.0;
	//				for (xx = 0 ; xx < customTableRow ; ++xx) {
	//					totalHeight += atof (DGPopUpGetItemText (dialogID, itmIdx_height [xx][0], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_height [xx][0]))).ToCStr ().Get ()) / 1000;
	//				}
	//				DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM, placingZone.verLen - totalHeight);

	//				break;
	//		}

	//		break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

	//				placingZone.bDoubleSide = false;	// 단면 전용
	//				placingZone.gap = 0.0;				// 벽과의 간격은 0.0
	//				
	//				if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION)
	//					placingZone.orientation = VERTICAL_DIRECTION;
	//				else
	//					placingZone.orientation = HORIZONTAL_DIRECTION;

	//				// 타입 지정 (타입A, 타입B)
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

	//				// 유로폼 너비, 높이 값 저장
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

					break;

				case DG_CANCEL:
					break;

	//			case BUTTON_ADD_COL_CUSTOM:
	//			case BUTTON_DEL_COL_CUSTOM:
	//			case BUTTON_ADD_ROW_CUSTOM:
	//			case BUTTON_DEL_ROW_CUSTOM:
	//				bChanged = false;

	//				if (item == BUTTON_ADD_COL_CUSTOM) {
	//					if (customTableCol < maxCol) {
	//						customTableCol ++;
	//						bChanged = true;
	//					}
	//				}
	//				if (item == BUTTON_DEL_COL_CUSTOM) {
	//					if (customTableCol >= 2) {
	//						customTableCol --;
	//						bChanged = true;
	//					}
	//				}
	//				if (item == BUTTON_ADD_ROW_CUSTOM) {
	//					if (customTableRow < maxRow) {
	//						customTableRow ++;
	//						bChanged = true;
	//					}
	//				}
	//				if (item == BUTTON_DEL_ROW_CUSTOM) {
	//					if (customTableRow >= 2) {
	//						customTableRow --;
	//						bChanged = true;
	//					}
	//				}

	//				item = 0;

	//				if (bChanged == true) {
	//					DGRemoveDialogItems (dialogID, REST_ITEM_START_CUSTOM);

	//					// 테이블폼 관련 항목 재배치
	//					itmPosX = 170;
	//					itmPosY = 150 + (customTableRow-1) * boxSizeY;

	//					for (xx = 0 ; xx < customTableRow ; ++xx) {
	//						for (yy = 0 ; yy < customTableCol ; ++yy) {
	//							// 구분자
	//							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, boxSizeX, boxSizeY);
	//							DGShowItem (dialogID, itmIdx);

	//							// 팝업컨트롤: 너비
	//							itmIdx_width [xx][yy] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, itmPosX + boxSizeX/2 - 30, itmPosY + boxSizeY - 30, 60, 23);
	//							DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//							//DGPopUpDisableDraw (dialogID, itmIdx);
	//							if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
	//							} else {
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//							}
	//							//DGPopUpEnableDraw (dialogID, itmIdx);
	//							DGShowItem (dialogID, itmIdx);
	//		
	//							// 팝업컨트롤: 높이
	//							itmIdx_height [xx][yy] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, itmPosX + 5, itmPosY + boxSizeY/2 - 15, 60, 23);
	//							DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//							//DGPopUpDisableDraw (dialogID, itmIdx);
	//							if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//							} else {
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
	//								DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
	//								DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
	//							}
	//							//DGPopUpEnableDraw (dialogID, itmIdx);
	//							DGShowItem (dialogID, itmIdx);

	//							itmPosX += boxSizeX;
	//						}
	//						itmPosX = 170;
	//						itmPosY -= boxSizeY;
	//					}

	//					// 남은 너비 표시
	//					totalWidth = 0.0;
	//					for (xx = 0 ; xx < customTableCol ; ++xx) {
	//						totalWidth += atof (DGPopUpGetItemText (dialogID, REST_ITEM_START_CUSTOM + xx*3 + 1, static_cast<short>(DGGetItemValLong (dialogID, REST_ITEM_START_CUSTOM + xx*3 + 1))).ToCStr ().Get ()) / 1000;
	//					}
	//					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM, placingZone.horLen - totalWidth);

	//					// 남은 높이 표시
	//					totalHeight = 0.0;
	//					for (xx = 0 ; xx < customTableRow ; ++xx) {
	//						totalHeight += atof (DGPopUpGetItemText (dialogID, REST_ITEM_START_CUSTOM + customTableCol*3*xx + 2, static_cast<short>(DGGetItemValLong (dialogID, REST_ITEM_START_CUSTOM + customTableCol*3*xx + 2))).ToCStr ().Get ()) / 1000;
	//					}
	//					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM, placingZone.verLen - totalHeight);

	//					// 다이얼로그 크기 변경
	//					dialogSizeX = 550;
	//					if (customTableCol > 2)	dialogSizeX += boxSizeX * (customTableCol-2);
	//					dialogSizeY = 450;
	//					if (customTableRow > 2)	dialogSizeY += boxSizeY * (customTableRow-2);
	//					DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
	//				}

	//				break;
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

// 객체의 레이어를 선택하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	//API_UCCallbackType	ucb;

	//switch (message) {
	//	case DG_MSG_INIT:
	//		// 다이얼로그 타이틀
	//		DGSetDialogTitle (dialogID, "가설재 레이어 선택하기");

	//		//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
	//		// 적용 버튼
	//		DGSetItemText (dialogID, DG_OK, "확 인");

	//		// 종료 버튼
	//		DGSetItemText (dialogID, DG_CANCEL, "취 소");

	//		// 체크박스: 레이어 묶음
	//		DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING_CUSTOM, "레이어 묶음");
	//		DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_CUSTOM, TRUE);

	//		// 레이어 관련 라벨
	//		DGSetItemText (dialogID, LABEL_LAYER_SETTINGS_CUSTOM, "부재별 레이어 설정");
	//		DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM_CUSTOM, "슬래브 테이블폼");
	//		DGSetItemText (dialogID, LABEL_LAYER_PROFILE_CUSTOM, "C형강");
	//		DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_CUSTOM, "유로폼");
	//		DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_CUSTOM, "비계 파이프");
	//		DGSetItemText (dialogID, LABEL_LAYER_PINBOLT_CUSTOM, "핀볼트 세트");
	//		DGSetItemText (dialogID, LABEL_LAYER_WALLTIE_CUSTOM, "벽체 타이");
	//		DGSetItemText (dialogID, LABEL_LAYER_JOIN_CUSTOM, "결합철물");
	//		DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE_CUSTOM, "헤드피스");
	//		DGSetItemText (dialogID, LABEL_LAYER_STEELFORM_CUSTOM, "스틸폼");
	//		DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD_CUSTOM, "합판");
	//		DGSetItemText (dialogID, LABEL_LAYER_FILLERSP_CUSTOM, "휠러스페이서");
	//		DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE_CUSTOM, "아웃코너앵글");
	//		DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL_CUSTOM, "아웃코너판넬");
	//		DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL_CUSTOM, "인코너판넬");
	//		DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER_CUSTOM, "각파이프행거");
	//		DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK_CUSTOM, "유로폼 후크");
	//		DGSetItemText (dialogID, LABEL_LAYER_HIDDEN_CUSTOM, "숨김");

	//		DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 580, 160, 25);
	//		DGSetItemFont (dialogID, BUTTON_AUTOSET_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_AUTOSET_CUSTOM, "레이어 자동 설정");
	//		DGShowItem (dialogID, BUTTON_AUTOSET_CUSTOM);

	//		// 유저 컨트롤 초기화
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
	//		// 레이어 같이 바뀜
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
	//				// 레이어 번호 저장
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
