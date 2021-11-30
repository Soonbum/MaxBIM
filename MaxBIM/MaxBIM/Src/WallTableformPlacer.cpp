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
static short	POPUP_HEIGHT_PRESET;
static short	POPUP_HEIGHT_BASIC [10];
static short	POPUP_HEIGHT_EXTRA [10];

static short	LABEL_TOTAL_WIDTH;
static short	POPUP_WIDTH_IN_TABLE [4];

//static double	preferWidth;
static bool		clickedPrevButton;		// 이전 버튼을 눌렀습니까?
static short	clickedIndex;			// 클릭한 버튼의 인덱스


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
		placingZone.bExtra = false;

		// 모프가 1개일 경우, 기본 모프만 설정
		infoMorph_Basic = infoMorph [0];
		infoMorph_Extra = infoMorph [0];
	} else {
		placingZone.bExtra = true;

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
	placingZone.verLenBasic		= infoMorph_Basic.verLen;
	placingZone.ang				= DegreeToRad (infoMorph_Basic.ang);
	if (placingZone.bExtra == true)
		placingZone.verLenExtra		= infoMorph_Extra.verLen;
	
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

	// 초기 셀 개수 계산
	placingZone.nCellsInHor = (short)floor (placingZone.horLen / 2.300);
	placingZone.nCellsInVerBasic = (short)floor (placingZone.verLenBasic / 1.200);
	placingZone.nCellsInVerExtra = (short)floor (placingZone.verLenExtra / 1.200);

	// [DIALOG] 1번째 다이얼로그에서 인코너 유무 및 길이, 테이블폼의 방향과 가로/세로 방향 유로폼의 개수와 각각의 길이를 선택함
	result = DGBlankModalDialog (550, 950, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler1, 0);

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

	// 테이블폼 상단 여백 채우기
	// ...

	// 테이블폼 하단 여백 채우기
	// ???

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// 기본 생성자
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

	this->presetHeight_euroform [0]		= 1200;
	this->presetHeight_euroform [1]		= 900;
	this->presetHeight_euroform [2]		= 600;

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
}

// 셀 정보 초기화
void	WallTableformPlacingZone::initCells (WallTableformPlacingZone* placingZone, bool bVertical)
{
	short	xx;

	// 셀 정보 채우기 (셀 너비 정보만 미리 채움)
	// 세로방향이면
	if (bVertical == true) {
		for (xx = 0 ; xx < sizeof (placingZone->cells) / sizeof (CellForWallTableform) ; ++xx) {
			placingZone->cells [xx].objType = TABLEFORM;
			placingZone->cells [xx].horLen = 2300;
			placingZone->cells [xx].tableInHor [0] = 600;
			placingZone->cells [xx].tableInHor [1] = 600;
			placingZone->cells [xx].tableInHor [2] = 500;
			placingZone->cells [xx].tableInHor [3] = 600;
		}

	// 가로방향이면
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
//	}
}

// 테이블폼/유로폼/휠러스페이서/합판/목재 배치를 위한 다이얼로그 (테이블폼 구성, 요소 방향, 개수 및 길이)
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy, zz;
	char	buffer [256];
	char	numbuf [32];
	int		presetValue, cellHeightValue, cellWidthValue, accumLength;
	const short		maxCol = 50;		// 열 최대 개수
	const short		maxRow = 10;		// 행 최대 개수
	double			totalWidth, totalHeight;
	static short	dialogSizeX = 550;			// 현재 다이얼로그 크기 X
	static short	dialogSizeY = 950;			// 현재 다이얼로그 크기 Y

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "테이블폼/인코너/유로폼/휠러스페이서/합판/목재 구성");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 200, 900, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확 인");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 300, 900, 70, 25);
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

			// 라벨: 벽과의 간격
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 50, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "벽과의 간격");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 벽과의 간격
			EDITCONTROL_GAP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 105, 45, 70, 23);
			DGShowItem (dialogID, EDITCONTROL_GAP);

			// 라벨: 테이블폼 방향
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 80, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "테이블폼 방향");
			DGShowItem (dialogID, itmIdx);

			// 팝업컨트롤: 테이블폼 방향
			POPUP_DIRECTION = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 105, 75, 70, 23);
			DGSetItemFont (dialogID, POPUP_DIRECTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_DIRECTION, DG_POPUP_BOTTOM, "세로");
			DGPopUpInsertItem (dialogID, POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_DIRECTION, DG_POPUP_BOTTOM, "가로");
			DGPopUpSelectItem (dialogID, POPUP_DIRECTION, DG_POPUP_TOP);
			DGShowItem (dialogID, POPUP_DIRECTION);

			// 라벨: 테이블폼 타입
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 220, 80, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "테이블폼 타입");
			DGShowItem (dialogID, itmIdx);

			// 팝업컨트롤: 테이블폼 타입
			POPUP_TABLEFORM_TYPE = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 305, 75, 70, 23);
			DGSetItemFont (dialogID, POPUP_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "타입A");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "타입B");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "타입C");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "타입D");
			DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_TOP);
			DGShowItem (dialogID, POPUP_TABLEFORM_TYPE);

			//////////////////////////////////////////////////////////// 셀 정보 초기화
			placingZone.initCells (&placingZone, true);

			//////////////////////////////////////////////////////////// 아이템 배치 (정면 관련 버튼)
			// 좌측 인코너 유무 (체크버튼)
			CHECKBOX_LINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 20, 135, 70, 70);
			DGSetItemFont (dialogID, CHECKBOX_LINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_LINCORNER, "인코너");
			DGShowItem (dialogID, CHECKBOX_LINCORNER);
			DGSetItemValLong (dialogID, CHECKBOX_LINCORNER, TRUE);
			// 좌측 인코너 길이 (Edit컨트롤)
			EDITCONTROL_LINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 20, 205, 70, 25);
			DGShowItem (dialogID, EDITCONTROL_LINCORNER);
			DGSetItemValDouble (dialogID, EDITCONTROL_LINCORNER, 0.100);

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
				POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
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

				// 너비 (팝업컨트롤)
				POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
				DGSetItemFont (dialogID, POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
				}
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
			DGSetItemValLong (dialogID, CHECKBOX_RINCORNER, TRUE);
			// 우측 인코너 길이 (Edit컨트롤)
			EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
			DGShowItem (dialogID, EDITCONTROL_RINCORNER);
			DGSetItemValDouble (dialogID, EDITCONTROL_RINCORNER, 0.100);

			//////////////////////////////////////////////////////////// 아이템 배치 (측면 관련 버튼)
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

			// 버튼: 삭제 (낮은쪽)
			BUTTON_DEL_VER_BASIC = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 250 + 30, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_VER_BASIC, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_VER_BASIC, "삭제(L)");
			DGShowItem (dialogID, BUTTON_DEL_VER_BASIC);

			// 라벨: 남은 높이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 317, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "남은 높이");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 남은 높이 (낮은쪽)
			EDITCONTROL_REMAIN_HEIGHT_BASIC = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 105, 310, 70, 25);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC);

			if (placingZone.bExtra == true) {
				// 버튼: 추가 (높은쪽)
				BUTTON_ADD_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250, 70, 25);
				DGSetItemFont (dialogID, BUTTON_ADD_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, BUTTON_ADD_VER_EXTRA, "추가(H)");
				DGShowItem (dialogID, BUTTON_ADD_VER_EXTRA);

				// 버튼: 삭제 (높은쪽)
				BUTTON_DEL_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250 + 30, 70, 25);
				DGSetItemFont (dialogID, BUTTON_DEL_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, BUTTON_DEL_VER_EXTRA, "삭제(H)");
				DGShowItem (dialogID, BUTTON_DEL_VER_EXTRA);

				// Edit컨트롤: 남은 높이 (높은쪽)
				EDITCONTROL_REMAIN_HEIGHT_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 185, 310, 70, 25);
				DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA);
				DGShowItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA);
			}

			// 팝업컨트롤: 테이블폼 세로방향 프리셋
			POPUP_HEIGHT_PRESET = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 20, 835, 70, 23);
			DGSetItemFont (dialogID, POPUP_HEIGHT_PRESET, DG_IS_LARGE | DG_IS_PLAIN);
			for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
				_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, numbuf);
			}
			DGPopUpInsertItem (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, "Free");
			DGPopUpSelectItem (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
			DGShowItem (dialogID, POPUP_HEIGHT_PRESET);

			// 왼쪽에 낮은쪽
			itmPosX = 105;
			itmPosY = 820;
			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
				DGShowItem (dialogID, itmIdx);

				POPUP_HEIGHT_BASIC [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
				DGSetItemFont (dialogID, POPUP_HEIGHT_BASIC [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
				}
				DGPopUpSelectItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, POPUP_HEIGHT_BASIC [xx]);

				itmPosY -= 50;
			}

			if (placingZone.bExtra == true) {
				// 오른쪽에 높은쪽
				itmPosX = 185;
				itmPosY = 820;
				if (placingZone.bExtra == true) {
					for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
						DGShowItem (dialogID, itmIdx);

						POPUP_HEIGHT_EXTRA [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
						DGSetItemFont (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_IS_LARGE | DG_IS_PLAIN);
						for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
							_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
						}
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);
						DGShowItem (dialogID, POPUP_HEIGHT_EXTRA [xx]);

						itmPosY -= 50;
					}
				}
			}

			//////////////////////////////////////////////////////////// 다이얼로그 크기 변경, 남은 너비 및 높이 계산
			// 다이얼로그 크기 설정
			dialogSizeX = 550;
			dialogSizeY = 950;
			if (placingZone.nCellsInHor >= 5) {
				DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
			}

			// 남은 너비 계산
			totalWidth = 0.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_LINCORNER);
			if (DGGetItemValLong (dialogID, CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_RINCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_WIDTH [xx]))).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			// 남은 높이 계산 (낮은쪽)
			totalHeight = 0.0;
			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
				totalHeight += atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_BASIC [xx]))).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

			if (placingZone.bExtra == true) {
				// 남은 높이 계산 (높은쪽)
				totalHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
					totalHeight += atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_EXTRA [xx]))).ToCStr ().Get ()) / 1000;
				}
				DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
			}

			break;

		case DG_MSG_CHANGE:
			// 가로/세로 변경할 때
			if (item == POPUP_DIRECTION) {
				strcpy (buffer, DGPopUpGetItemText (dialogID, POPUP_DIRECTION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_DIRECTION))).ToCStr ().Get ());

				// 가로일 경우
				if (my_strcmp (buffer, "가로") == 0) {
					// 셀 정보 초기화
					placingZone.initCells (&placingZone, false);

					// 정면
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_WIDTH [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_WIDTH [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_TOP);

							// 팝업 보이고, Edit컨트롤 숨김
							DGShowItem (dialogID, POPUP_WIDTH [xx]);
							DGHideItem (dialogID, EDITCONTROL_WIDTH [xx]);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_WIDTH [xx]);
							DGRedrawItem (dialogID, POPUP_OBJ_TYPE [xx]);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_WIDTH [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_WIDTH [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_TOP);

							// 팝업 보이고, Edit컨트롤 숨김
							DGShowItem (dialogID, POPUP_WIDTH [xx]);
							DGHideItem (dialogID, EDITCONTROL_WIDTH [xx]);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_WIDTH [xx]);
							DGRedrawItem (dialogID, POPUP_OBJ_TYPE [xx]);
						}
					}

					// 측면
					for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
						// 그리기 비활성화
						DGInvalidateItem (dialogID, POPUP_HEIGHT_BASIC [xx]);
						DGPopUpDisableDraw (dialogID, POPUP_HEIGHT_BASIC [xx]);

						// 팝업 전부 비우고
						DGPopUpDeleteItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_ALL_ITEMS);

						// 팝업 내용 다시 채우고
						for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
							_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
						}
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);

						// 그리기 활성화
						DGPopUpEnableDraw (dialogID, POPUP_HEIGHT_BASIC [xx]);
						DGRedrawItem (dialogID, POPUP_HEIGHT_BASIC [xx]);
					}

					if (placingZone.bExtra == true) {
						for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_HEIGHT_EXTRA [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_HEIGHT_EXTRA [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_HEIGHT_EXTRA [xx]);
							DGRedrawItem (dialogID, POPUP_HEIGHT_EXTRA [xx]);
						}
					}

					// 프리셋
					DGInvalidateItem (dialogID, POPUP_HEIGHT_PRESET);
					DGPopUpDisableDraw (dialogID, POPUP_HEIGHT_PRESET);
					DGPopUpDeleteItem (dialogID, POPUP_HEIGHT_PRESET, DG_ALL_ITEMS);

					for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
						DGPopUpInsertItem (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
						_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
						DGPopUpSetItemText (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, numbuf);
					}
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, "Free");
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);

					DGPopUpEnableDraw (dialogID, POPUP_HEIGHT_PRESET);
					DGRedrawItem (dialogID, POPUP_HEIGHT_PRESET);

				// 세로일 경우
				} else {
					// 셀 정보 초기화
					placingZone.initCells (&placingZone, true);

					// 정면
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_WIDTH [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_WIDTH [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_TOP);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_WIDTH [xx]);
							DGRedrawItem (dialogID, POPUP_OBJ_TYPE [xx]);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_WIDTH [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_WIDTH [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_TOP);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_WIDTH [xx]);
							DGRedrawItem (dialogID, POPUP_OBJ_TYPE [xx]);
						}
					}

					// 측면
					for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
						// 그리기 비활성화
						DGInvalidateItem (dialogID, POPUP_HEIGHT_BASIC [xx]);
						DGPopUpDisableDraw (dialogID, POPUP_HEIGHT_BASIC [xx]);

						// 팝업 전부 비우고
						DGPopUpDeleteItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_ALL_ITEMS);

						// 팝업 내용 다시 채우고
						for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
							_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
						}
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);

						// 그리기 활성화
						DGPopUpEnableDraw (dialogID, POPUP_HEIGHT_BASIC [xx]);
						DGRedrawItem (dialogID, POPUP_HEIGHT_BASIC [xx]);
					}

					if (placingZone.bExtra == true) {
						for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_HEIGHT_EXTRA [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_HEIGHT_EXTRA [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_HEIGHT_EXTRA [xx]);
							DGRedrawItem (dialogID, POPUP_HEIGHT_EXTRA [xx]);
						}
					}

					// 프리셋
					DGInvalidateItem (dialogID, POPUP_HEIGHT_PRESET);
					DGPopUpDisableDraw (dialogID, POPUP_HEIGHT_PRESET);
					DGPopUpDeleteItem (dialogID, POPUP_HEIGHT_PRESET, DG_ALL_ITEMS);

					for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
						DGPopUpInsertItem (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
						_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
						DGPopUpSetItemText (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, numbuf);
					}
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, "Free");
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);

					DGPopUpEnableDraw (dialogID, POPUP_HEIGHT_PRESET);
					DGRedrawItem (dialogID, POPUP_HEIGHT_PRESET);
				}
			}

			// 객체 타입 변경할 때
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if (item == POPUP_OBJ_TYPE [xx]) {
					// 해당 버튼의 이름 변경
					DGSetItemText (dialogID, BUTTON_OBJ [xx], DGPopUpGetItemText (dialogID, POPUP_OBJ_TYPE [xx], DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx])));
					
					// 가로/세로 방향 여부에 따라 팝업컨트롤의 내용물이 바뀜
					strcpy (buffer, DGPopUpGetItemText (dialogID, POPUP_DIRECTION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_DIRECTION))).ToCStr ().Get ());
					if (my_strcmp (buffer, "가로") == 0) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_WIDTH [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_WIDTH [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_TOP);

							// 팝업 보이고, Edit컨트롤 숨김
							DGShowItem (dialogID, POPUP_WIDTH [xx]);
							DGHideItem (dialogID, EDITCONTROL_WIDTH [xx]);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_WIDTH [xx]);
							DGRedrawItem (dialogID, POPUP_OBJ_TYPE [xx]);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_WIDTH [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_WIDTH [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_TOP);

							// 팝업 보이고, Edit컨트롤 숨김
							DGShowItem (dialogID, POPUP_WIDTH [xx]);
							DGHideItem (dialogID, EDITCONTROL_WIDTH [xx]);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_WIDTH [xx]);
							DGRedrawItem (dialogID, POPUP_OBJ_TYPE [xx]);
						}
					} else {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_WIDTH [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_WIDTH [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_TOP);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_WIDTH [xx]);
							DGRedrawItem (dialogID, POPUP_OBJ_TYPE [xx]);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 그리기 비활성화
							DGInvalidateItem (dialogID, POPUP_WIDTH [xx]);
							DGPopUpDisableDraw (dialogID, POPUP_WIDTH [xx]);

							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_TOP);

							// 그리기 활성화
							DGPopUpEnableDraw (dialogID, POPUP_WIDTH [xx]);
							DGRedrawItem (dialogID, POPUP_OBJ_TYPE [xx]);
						}
					}

					// 테이블폼/유로폼이면 너비가 팝업컨트롤, 그 외에는 Edit컨트롤, 없으면 모두 숨김
					if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == EUROFORM)) {
						DGShowItem (dialogID, POPUP_WIDTH [xx]);
						DGHideItem (dialogID, EDITCONTROL_WIDTH [xx]);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == NONE) {
						DGHideItem (dialogID, EDITCONTROL_WIDTH [xx]);
						DGHideItem (dialogID, POPUP_WIDTH [xx]);
					} else {
						DGShowItem (dialogID, EDITCONTROL_WIDTH [xx]);
						DGHideItem (dialogID, POPUP_WIDTH [xx]);
					}
				}
			}

			// 프리셋 변경시
			if (item == POPUP_HEIGHT_PRESET) {
				// 가로일 때
				strcpy (buffer, DGPopUpGetItemText (dialogID, POPUP_DIRECTION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_DIRECTION))).ToCStr ().Get ());
				if (my_strcmp (buffer, "가로") == 0) {
					presetValue = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_PRESET, static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_PRESET))).ToCStr ().Get ());
					for (xx = 0 ; xx < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++xx) {
						if (presetValue == placingZone.presetWidth_tableform [xx]) {
							for (yy = 0 ; yy < placingZone.nCellsInVerBasic ; ++yy) {
								DGSetItemFont (dialogID, POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_PLAIN);
								for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, POPUP_HEIGHT_BASIC [yy]) ; ++zz) {
									cellHeightValue = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_BASIC [yy], zz).ToCStr ().Get ());
									if (cellHeightValue == placingZone.presetHeight_config_horizontal [xx][yy+1]) {
										DGPopUpSelectItem (dialogID, POPUP_HEIGHT_BASIC [yy], zz);
										DGSetItemFont (dialogID, POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_BOLD);
									}
								}
							}
						}
					}
					if (placingZone.bExtra == true) {
						presetValue = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_PRESET, static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_PRESET))).ToCStr ().Get ());
						for (xx = 0 ; xx < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++xx) {
							if (presetValue == placingZone.presetWidth_tableform [xx]) {
								for (yy = 0 ; yy < placingZone.nCellsInVerExtra ; ++yy) {
									DGSetItemFont (dialogID, POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_PLAIN);
									for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, POPUP_HEIGHT_EXTRA [yy]) ; ++zz) {
										cellHeightValue = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_EXTRA [yy], zz).ToCStr ().Get ());
										if (cellHeightValue == placingZone.presetHeight_config_horizontal [xx][yy+1]) {
											DGPopUpSelectItem (dialogID, POPUP_HEIGHT_EXTRA [yy], zz);
											DGSetItemFont (dialogID, POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_BOLD);
										}
									}
								}
							}
						}
					}

				// 세로일 때
				} else {
					presetValue = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_PRESET, static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_PRESET))).ToCStr ().Get ());
					for (xx = 0 ; xx < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++xx) {
						if (presetValue == placingZone.presetHeight_tableform [xx]) {
							for (yy = 0 ; yy < placingZone.nCellsInVerBasic ; ++yy) {
								DGSetItemFont (dialogID, POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_PLAIN);
								for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, POPUP_HEIGHT_BASIC [yy]) ; ++zz) {
									cellHeightValue = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_BASIC [yy], zz).ToCStr ().Get ());
									if (cellHeightValue == placingZone.presetHeight_config_vertical [xx][yy+1]) {
										DGPopUpSelectItem (dialogID, POPUP_HEIGHT_BASIC [yy], zz);
										DGSetItemFont (dialogID, POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_BOLD);
									}
								}
							}
						}
					}
					if (placingZone.bExtra == true) {
						presetValue = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_PRESET, static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_PRESET))).ToCStr ().Get ());
						for (xx = 0 ; xx < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++xx) {
							if (presetValue == placingZone.presetHeight_tableform [xx]) {
								for (yy = 0 ; yy < placingZone.nCellsInVerExtra ; ++yy) {
									DGSetItemFont (dialogID, POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_PLAIN);
									for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, POPUP_HEIGHT_EXTRA [yy]) ; ++zz) {
										cellHeightValue = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_EXTRA [yy], zz).ToCStr ().Get ());
										if (cellHeightValue == placingZone.presetHeight_config_vertical [xx][yy+1]) {
											DGPopUpSelectItem (dialogID, POPUP_HEIGHT_EXTRA [yy], zz);
											DGSetItemFont (dialogID, POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_BOLD);
										}
									}
								}
							}
						}
					}
				}
			}

			// 테이블폼 너비를 변경할 때 !!!
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				// 테이블폼 너비를 변경할 때
				if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
					strcpy (buffer, DGPopUpGetItemText (dialogID, POPUP_DIRECTION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_DIRECTION))).ToCStr ().Get ());
					// 세로방향일 경우에만 각 셀의 너비를 설정함
					if (my_strcmp (buffer, "세로") == 0) {
						if (item == POPUP_WIDTH [xx]) {
							cellWidthValue = atoi (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_WIDTH [xx]))).ToCStr ().Get ());
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
								if (cellWidthValue == placingZone.presetWidth_tableform [yy]) {
									for (zz = 0 ; zz < placingZone.presetWidth_config_vertical [yy][0] ; ++zz) {
										if ((zz >= 0) || (zz < placingZone.presetWidth_config_vertical [yy][0]))
											placingZone.cells [xx].tableInHor [zz] = placingZone.presetWidth_config_vertical [yy][zz+1];
										else
											placingZone.cells [xx].tableInHor [zz] = 0;
									}
								}
							}
						}
					}
				}
			}

			// 남은 너비 계산
			totalWidth = 0.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_LINCORNER);
			if (DGGetItemValLong (dialogID, CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_RINCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_WIDTH [xx]))).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			// 남은 높이 계산 (낮은쪽)
			totalHeight = 0.0;
			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
				totalHeight += atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_BASIC [xx]))).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

			if (placingZone.bExtra == true) {
				// 남은 높이 계산 (높은쪽)
				totalHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
					totalHeight += atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_EXTRA [xx]))).ToCStr ().Get ()) / 1000;
				}
				DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
			}

		case DG_MSG_CLICK:
			// 확인 버튼
			if (item == DG_OK) {
				// ...
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

			} else if (item == DG_CANCEL) {
				// 처리할 내용 없음

			} else {
				if ((item == BUTTON_ADD_HOR) || (item == BUTTON_DEL_HOR)) {
					// 정면 - 추가 버튼 클릭
					if (item == BUTTON_ADD_HOR) {
						if (placingZone.nCellsInHor < maxCol) {
							// 우측 인코너 버튼을 지우고
							DGRemoveDialogItem (dialogID, CHECKBOX_RINCORNER);
							DGRemoveDialogItem (dialogID, EDITCONTROL_RINCORNER);

							// 마지막 셀 버튼 오른쪽에 새로운 셀 버튼을 추가하고
							itmPosX = 90 + (70 * placingZone.nCellsInHor);
							itmPosY = 137;
							// 버튼
							BUTTON_OBJ [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
							DGSetItemFont (dialogID, BUTTON_OBJ [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, BUTTON_OBJ [placingZone.nCellsInHor], "테이블폼");
							DGShowItem (dialogID, BUTTON_OBJ [placingZone.nCellsInHor]);

							// 객체 타입 (팝업컨트롤)
							POPUP_OBJ_TYPE [placingZone.nCellsInHor] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
							DGSetItemFont (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "없음");
							DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "테이블폼");
							DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "유로폼");
							DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "휠러스페이서");
							DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "합판");
							DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "각재");
							DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_TOP+1);
							DGShowItem (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor]);

							// 너비 (팝업컨트롤)
							POPUP_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
							DGSetItemFont (dialogID, POPUP_WIDTH [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_TOP);
							DGShowItem (dialogID, POPUP_WIDTH [placingZone.nCellsInHor]);

							// 너비 (팝업컨트롤) - 처음에는 숨김
							EDITCONTROL_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);

							itmPosX += 70;

							// 우측 인코너 버튼을 오른쪽 끝에 붙임
							// 우측 인코너 유무 (체크버튼)
							CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
							DGSetItemFont (dialogID, CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, CHECKBOX_RINCORNER, "인코너");
							DGShowItem (dialogID, CHECKBOX_RINCORNER);
							DGSetItemValLong (dialogID, CHECKBOX_RINCORNER, TRUE);
							// 우측 인코너 길이 (Edit컨트롤)
							EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
							DGShowItem (dialogID, EDITCONTROL_RINCORNER);
							DGSetItemValDouble (dialogID, EDITCONTROL_RINCORNER, 0.100);

							++placingZone.nCellsInHor;
						}
					}

					// 정면 - 삭제 버튼 클릭
					else if (item == BUTTON_DEL_HOR) {
						if (placingZone.nCellsInHor > 1) {
							// 우측 인코너 버튼을 지우고
							DGRemoveDialogItem (dialogID, CHECKBOX_RINCORNER);
							DGRemoveDialogItem (dialogID, EDITCONTROL_RINCORNER);

							// 마지막 셀 버튼을 지우고
							DGRemoveDialogItem (dialogID, BUTTON_OBJ [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, POPUP_OBJ_TYPE [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, POPUP_WIDTH [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, EDITCONTROL_WIDTH [placingZone.nCellsInHor - 1]);

							// 3. 우측 인코너 버튼을 오른쪽 끝에 붙임
							itmPosX = 90 + (70 * (placingZone.nCellsInHor - 1));
							itmPosY = 137;
							// 우측 인코너 유무 (체크버튼)
							CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
							DGSetItemFont (dialogID, CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, CHECKBOX_RINCORNER, "인코너");
							DGShowItem (dialogID, CHECKBOX_RINCORNER);
							DGSetItemValLong (dialogID, CHECKBOX_RINCORNER, TRUE);
							// 우측 인코너 길이 (Edit컨트롤)
							EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
							DGShowItem (dialogID, EDITCONTROL_RINCORNER);
							DGSetItemValDouble (dialogID, EDITCONTROL_RINCORNER, 0.100);

							--placingZone.nCellsInHor;
						}
					}

				} else if ((item == BUTTON_ADD_VER_BASIC) || (item == BUTTON_DEL_VER_BASIC)) {
					// 측면 - 추가(L) 버튼 클릭
					if (item == BUTTON_ADD_VER_BASIC) {
						if (placingZone.nCellsInVerBasic < maxRow) {
							// 맨 위에 셀 하나 추가
							itmPosX = 105;
							itmPosY = 820 - (50 * placingZone.nCellsInVerBasic);

							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
							DGShowItem (dialogID, itmIdx);

							POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
							DGSetItemFont (dialogID, POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_IS_LARGE | DG_IS_PLAIN);
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_TOP);
							DGShowItem (dialogID, POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic]);

							++placingZone.nCellsInVerBasic;
						}						
					}

					// 측면 - 삭제(L) 버튼 클릭
					else if (item == BUTTON_DEL_VER_BASIC) {
						if (placingZone.nCellsInVerBasic > 1) {
							// 맨 위에 있던 셀 하나 제거
							DGRemoveDialogItem (dialogID, POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic - 1]);
							DGRemoveDialogItem (dialogID, POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic - 1] - 1);

							--placingZone.nCellsInVerBasic;
						}
					}

				} else if ((item == BUTTON_ADD_VER_EXTRA) || (item == BUTTON_DEL_VER_EXTRA)) {
					// 측면 - 추가(H) 버튼 클릭
					if (item == BUTTON_ADD_VER_EXTRA) {
						if (placingZone.nCellsInVerExtra < maxRow) {
							// 맨 위에 셀 하나 추가
							itmPosX = 185;
							itmPosY = 820 - (50 * placingZone.nCellsInVerExtra);

							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
							DGShowItem (dialogID, itmIdx);

							POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
							DGSetItemFont (dialogID, POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_IS_LARGE | DG_IS_PLAIN);
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_TOP);
							DGShowItem (dialogID, POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra]);

							++placingZone.nCellsInVerExtra;
						}
					}

					// 측면 - 삭제(H) 버튼 클릭
					else if (item == BUTTON_DEL_VER_EXTRA) {
						if (placingZone.nCellsInVerExtra > 1) {
							// 맨 위에 있던 셀 하나 제거
							DGRemoveDialogItem (dialogID, POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra - 1]);
							DGRemoveDialogItem (dialogID, POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra - 1] - 1);

							--placingZone.nCellsInVerExtra;
						}
					}

				} else {
					// 객체 버튼 클릭
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (item == BUTTON_OBJ [xx]) {
							// 테이블폼 타입 (세로 방향)일 경우에만 3번째 다이얼로그 열기
							clickedIndex = xx;
							result = DGBlankModalDialog (350, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3, (short) 0);

							accumLength = 0;
							for (yy = 0 ; yy < 4 ; ++yy) {
								accumLength += placingZone.cells [xx].tableInHor [yy];
								// !!! 안 바뀌나?
								for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, POPUP_WIDTH [xx]) ; ++zz) {
									if (accumLength == atoi (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, POPUP_WIDTH [xx])).ToCStr ().Get ())) {
										DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], zz);
									}
								}
							}
						}
					}
				}

				// 다이얼로그 크기 설정
				dialogSizeX = 550;
				dialogSizeY = 950;
				if (placingZone.nCellsInHor >= 5) {
					DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
				}

				// 남은 너비 계산
				totalWidth = 0.0;
				if (DGGetItemValLong (dialogID, CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_LINCORNER);
				if (DGGetItemValLong (dialogID, CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_RINCORNER);
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == EUROFORM))
						totalWidth += atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_WIDTH [xx]))).ToCStr ().Get ()) / 1000;
					else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE [xx]) == NONE)
						totalWidth += 0.0;
					else
						totalWidth += DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH [xx]);
				}
				DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

				// 남은 높이 계산 (낮은쪽)
				totalHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
					totalHeight += atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_BASIC [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_BASIC [xx]))).ToCStr ().Get ()) / 1000;
				}
				DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

				if (placingZone.bExtra == true) {
					// 남은 높이 계산 (높은쪽)
					totalHeight = 0.0;
					for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
						totalHeight += atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT_EXTRA [xx], static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT_EXTRA [xx]))).ToCStr ().Get ()) / 1000;
					}
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
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

// 테이블폼 세로방향에 대하여 유로폼의 수평 배열을 변경하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	// clickedIndex: 이전 다이얼로그에서 눌린 버튼의 인덱스 번호 (BUTTON_OBJ [xx])

	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	int		accumLength;
	char	buffer [256];
	char	numbuf [32];

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "테이블폼 (세로방향) 배열 설정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 100, 140, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확 인");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 180, 140, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취 소");
			DGShowItem (dialogID, DG_CANCEL);

			// 기존 너비 (라벨)
			accumLength = 0;
			for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
				accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			sprintf (buffer, "기존 너비: %d", accumLength);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 20, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, buffer);
			DGShowItem (dialogID, itmIdx);

			// 변경된 너비 (라벨)
			sprintf (buffer, "변경된 너비: %d", 0);
			LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 20, 100, 23);
			DGSetItemFont (dialogID, LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, buffer);
			DGShowItem (dialogID, LABEL_TOTAL_WIDTH);

			itmPosX = 35;
			itmPosY = 55;

			for (xx = 0 ; xx < 4 ; ++xx) {
				// 구분자
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
				DGShowItem (dialogID, itmIdx);

				// 텍스트(유로폼)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "유로폼");
				DGShowItem (dialogID, itmIdx);

				// 콤보박스
				POPUP_WIDTH_IN_TABLE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 5, itmPosY + 40, 60, 25);
				DGSetItemFont (dialogID, POPUP_WIDTH_IN_TABLE [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM, numbuf);
				}
				for (yy = 0 ; yy < DGPopUpGetItemCount (dialogID, POPUP_WIDTH_IN_TABLE [xx]) ; ++yy) {
					if (placingZone.cells [clickedIndex].tableInHor [xx] == atoi (DGPopUpGetItemText (dialogID, POPUP_WIDTH_IN_TABLE [xx], yy).ToCStr ().Get ())) {
						DGPopUpSelectItem (dialogID, POPUP_WIDTH_IN_TABLE [xx], yy);
						break;
					}
				}
				DGShowItem (dialogID, POPUP_WIDTH_IN_TABLE [xx]);

				itmPosX += 70;
			}

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 4 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 4 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 선택한 콤보박스들의 값을 기반으로 구조체 값을 갱신함
					for (xx = 0 ; xx < 4 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}
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