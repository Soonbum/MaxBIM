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
static short	layerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
static short	layerInd_WallTie;			// 레이어 번호: 빅체 타이 (더 이상 사용하지 않음)
static short	layerInd_Clamp;				// 레이어 번호: 직교 클램프 (더 이상 사용하지 않음)
static short	layerInd_HeadPiece;			// 레이어 번호: 헤드피스
static short	layerInd_Props;				// 레이어 번호: Push-Pull Props
static short	layerInd_Join;				// 레이어 번호: 결합철물
static short	layerInd_Plywood;			// 레이어 번호: 합판 (공통)
static short	layerInd_Timber;			// 레이어 번호: 각재 (공통)
static short	layerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
static short	layerInd_CrossJointBar;		// 레이어 번호: 십자 조인트 바
static short	layerInd_BlueClamp;			// 레이어 번호: 블루 클램프
static short	layerInd_BlueTimberRail;	// 레이어 번호: 블루 목심
static short	layerInd_Hidden;			// 레이어 번호: 숨김 (더 이상 사용하지 않음)

static short	layerInd_SlabTableform;		// 레이어 번호: 슬래브 테이블폼
static short	layerInd_Profile;			// 레이어 번호: KS프로파일
static short	layerInd_Steelform;			// 레이어 번호: 스틸폼
static short	layerInd_Fillersp;			// 레이어 번호: 휠러스페이서
static short	layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static short	layerInd_OutcornerPanel;	// 레이어 번호: 아웃코너판넬
static short	layerInd_IncornerPanel;		// 레이어 번호: 인코너판넬
static short	layerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거

static bool		bLayerInd_Euroform;			// 레이어 번호: 유로폼
static bool		bLayerInd_RectPipe;			// 레이어 번호: 비계 파이프
static bool		bLayerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
static bool		bLayerInd_WallTie;			// 레이어 번호: 벽체 타이
static bool		bLayerInd_HeadPiece;		// 레이어 번호: 헤드피스
static bool		bLayerInd_Props;			// 레이어 번호: Push-Pull Props
static bool		bLayerInd_Join;				// 레이어 번호: 결합철물
static bool		bLayerInd_Plywood;			// 레이어 번호: 합판
static bool		bLayerInd_Timber;			// 레이어 번호: 각재
static bool		bLayerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
static bool		bLayerInd_CrossJointBar;	// 레이어 번호: 십자 조인트 바
static bool		bLayerInd_BlueClamp;		// 레이어 번호: 블루 클램프
static bool		bLayerInd_BlueTimberRail;	// 레이어 번호: 블루 목심
static bool		bLayerInd_Hidden;			// 레이어 번호: 숨김

static bool		bLayerInd_SlabTableform;	// 레이어 번호: 슬래브 테이블폼
static bool		bLayerInd_Profile;			// 레이어 번호: KS프로파일
static bool		bLayerInd_Steelform;		// 레이어 번호: 스틸폼
static bool		bLayerInd_Fillersp;			// 레이어 번호: 휠러스페이서
static bool		bLayerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static bool		bLayerInd_OutcornerPanel;	// 레이어 번호: 아웃코너판넬
static bool		bLayerInd_IncornerPanel;	// 레이어 번호: 인코너판넬
static bool		bLayerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거

static GS::Array<API_Guid>	elemList_Front;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함 (앞면)
static GS::Array<API_Guid>	elemList_Back;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함 (뒷면)
static GS::Array<API_Guid>	elemList_Front_Add;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함 (앞면)
static GS::Array<API_Guid>	elemList_Back_Add;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함 (뒷면)

static int	clickedIndex;	// 클릭한 버튼의 인덱스
static int	iMarginSide;	// 벽 상단을 채우고자 하는데 현재 진행되는 쪽이 어느 쪽 면인가? (1-낮은쪽, 2-높은쪽)


// 벽에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	short		xx;
	double		dx, dy;

	// Selection Manager 관련 변수
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
	double			workLevel_wall;		// 벽의 작업 층 높이


	// 선택한 요소 가져오기 (벽 1개, 모프 1~2개 선택해야 함)
	err = getGuidsOfSelection (&walls, API_WallID, &nWalls);
	err = getGuidsOfSelection (&morphs, API_MorphID, &nMorphs);
	if (err == APIERR_NOPLAN) {
		WriteReport_Alert ("열린 프로젝트 창이 없습니다.");
	}
	if (err == APIERR_NOSEL) {
		WriteReport_Alert ("아무 것도 선택하지 않았습니다.\n필수 선택: 벽 (1개), 벽을 덮는 모프 (1개)\n옵션 선택: 벽을 덮는 모프(뒷면 - 1차 모프와 높이가 다름) (1개)");
	}

	// 벽이 1개인가?
	if (nWalls != 1) {
		WriteReport_Alert ("벽을 1개 선택해야 합니다.");
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개 또는 2개인가?
	if ((nMorphs < 1) || (nMorphs > 2)) {
		WriteReport_Alert ("벽을 덮는 모프를 1개 또는 2개를 선택하셔야 합니다.");
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
		WriteReport_Alert ("벽의 두께는 균일해야 합니다.");
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
			WriteReport_Alert ("모프가 세워져 있지 않습니다.");
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
		GS::Array<API_Element>	elems;
		elems.Push (elem);
		deleteElements (elems);
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
	workLevel_wall = getWorkLevel (infoWall.floorInd);

	// 영역 정보의 고도 정보를 수정
	placingZone.leftBottomZ = infoWall.bottomOffset;

	// 초기 셀 개수 계산
	placingZone.nCellsInHor = (short)floor (placingZone.horLen / 2.250);
	placingZone.nCellsInVerBasic = (short)floor (placingZone.verLenBasic / 1.200);
	placingZone.nCellsInVerExtra = (short)floor (placingZone.verLenExtra / 1.200);

FIRST:

	// [DIALOG] 1번째 다이얼로그에서 인코너 유무 및 길이, 테이블폼의 방향과 가로/세로 방향 유로폼의 개수와 각각의 길이를 선택함
	result = DGBlankModalDialog (550, 950, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler1, 0);

	if (result != DG_OK)
		return err;

	// [DIALOG] 2번째 다이얼로그에서 부재별 레이어를 지정함
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32503, ACAPI_GetOwnResModule (), wallTableformPlacerHandler2, 0);

	if (result != DG_OK)
		goto FIRST;

	// 객체 배치하기
	placingZone.placeObjects (&placingZone);

	// [DIALOG] 4번째 다이얼로그에서 벽 상단의 자투리 공간을 다른 규격의 유로폼으로 대체할 것인지 묻습니다.
	iMarginSide = 1;
	result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler4, 0);
	if (placingZone.bExtra == true) {
		iMarginSide = 2;
		result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler4, 0);
	}

	// 상단 여백 셀의 위치를 교정함
	placingZone.adjustMarginCellsPosition (&placingZone);

	// 상단 여백 채우기
	for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx)
		placingZone.fillRestAreas (&placingZone, xx);

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

// 셀 정보 초기화
void	WallTableformPlacingZone::initCells (WallTableformPlacingZone* placingZone, bool bVertical)
{
	short	xx;

	// 셀 정보 채우기 (셀 너비 정보만 미리 채움)
	// 세로방향이면
	if (bVertical == true) {
		for (xx = 0 ; xx < sizeof (placingZone->cells) / sizeof (CellForWallTableform) ; ++xx) {
			placingZone->cells [xx].objType = TABLEFORM;
			placingZone->cells [xx].horLen = 2250;
			placingZone->cells [xx].tableInHor [0] = 600;
			placingZone->cells [xx].tableInHor [1] = 600;
			placingZone->cells [xx].tableInHor [2] = 450;
			placingZone->cells [xx].tableInHor [3] = 600;
			placingZone->cells [xx].tableInHor [4] = 0;
			placingZone->cells [xx].tableInHor [5] = 0;
			placingZone->cells [xx].tableInHor [6] = 0;
			placingZone->cells [xx].tableInHor [7] = 0;
			placingZone->cells [xx].tableInHor [8] = 0;
			placingZone->cells [xx].tableInHor [9] = 0;
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
			placingZone->cells [xx].tableInHor [5] = 0;
			placingZone->cells [xx].tableInHor [6] = 0;
			placingZone->cells [xx].tableInHor [7] = 0;
			placingZone->cells [xx].tableInHor [8] = 0;
			placingZone->cells [xx].tableInHor [9] = 0;
		}
	}
}

// 셀(0-기반 인덱스 번호)의 좌하단 점 위치 X 좌표를 구함
double	WallTableformPlacingZone::getCellPositionLeftBottomX (WallTableformPlacingZone* placingZone, short idx)
{
	double	distance = 0.0;

	if ((placingZone->typeLcorner == INCORNER_PANEL) || (placingZone->typeLcorner == OUTCORNER_PANEL))
		distance = placingZone->lenLcorner;

	for (short xx = 0 ; xx < idx ; ++xx)
		distance += (double)placingZone->cells [xx].horLen / 1000.0;

	return distance;
}

// 셀 위치를 바르게 교정함
void	WallTableformPlacingZone::adjustCellsPosition (WallTableformPlacingZone* placingZone)
{
	for (short xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->cells [xx].ang = placingZone->ang;
		placingZone->cells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cells [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cells [xx].leftBottomZ = placingZone->leftBottomZ;
	}
}

// 상단 여백 셀 위치를 바르게 교정함
void	WallTableformPlacingZone::adjustMarginCellsPosition (WallTableformPlacingZone* placingZone)
{
	for (short xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->marginCellsBasic [xx].ang = placingZone->ang;
		placingZone->marginCellsBasic [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->marginCellsBasic [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->marginCellsBasic [xx].leftBottomZ = placingZone->leftBottomZ + placingZone->verLenBasic - placingZone->marginTopBasic;
	}

	// 모프 1개만 있으면 양쪽의 높이와 여백 너비를 동일하게 세트
	if (placingZone->bExtra == false) {
		placingZone->marginTopExtra = placingZone->marginTopBasic;
		placingZone->verLenExtra = placingZone->verLenBasic;
	}

	for (short xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->marginCellsExtra [xx].ang = placingZone->ang;
		placingZone->marginCellsExtra [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->marginCellsExtra [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->marginCellsExtra [xx].leftBottomZ = placingZone->leftBottomZ + placingZone->verLenExtra - placingZone->marginTopExtra;

		// 모프 1개만 있으면 양쪽의 채우기 옵션을 동일하게 세트
		if (placingZone->bExtra == false) {
			placingZone->marginCellsExtra [xx].bFill = placingZone->marginCellsBasic [xx].bFill;
			placingZone->marginCellsExtra [xx].bEuroform1 = placingZone->marginCellsBasic [xx].bEuroform1;
			placingZone->marginCellsExtra [xx].bEuroform2 = placingZone->marginCellsBasic [xx].bEuroform2;
			placingZone->marginCellsExtra [xx].bEuroformStandard1 = placingZone->marginCellsBasic [xx].bEuroformStandard1;
			placingZone->marginCellsExtra [xx].bEuroformStandard2 = placingZone->marginCellsBasic [xx].bEuroformStandard2;
			placingZone->marginCellsExtra [xx].formWidth1 = placingZone->marginCellsBasic [xx].formWidth1;
			placingZone->marginCellsExtra [xx].formWidth2 = placingZone->marginCellsBasic [xx].formWidth2;
		}
	}
}

// 셀 정보를 기반으로 객체들을 배치함
GSErrCode	WallTableformPlacingZone::placeObjects (WallTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy, varEnd;
	double	accumDist;
	double	lengthDouble;
	int		lengthInt;
	int		remainLengthInt;

	// ================================================== 인코너 배치
	// 좌측 인코너/아웃코너 배치
	if (placingZone->typeLcorner == INCORNER_PANEL) {
		// 앞면
		EasyObjectPlacement incornerPanel;
		incornerPanel.init (L("인코너판넬v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (90.0));

		moveIn3D ('y', incornerPanel.radAng + DegreeToRad (90.0), -placingZone->gap, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);	// 벽과의 간격만큼 이동

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
			elemList_Front.Push (incornerPanel.placeObject (5,
				"in_comp", APIParT_CString, "인코너판넬",
				"wid_s", APIParT_Length, format_string ("%f", 0.100),
				"leng_s", APIParT_Length, format_string ("%f", placingZone->lenLcorner),
				"hei_s", APIParT_Length, format_string ("%f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
				"dir_s", APIParT_CString, "세우기"));

			moveIn3D ('z', incornerPanel.radAng + DegreeToRad (90.0), (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);
		}

		// 뒷면
		if (placingZone->bSingleSide == false) {
			incornerPanel.init (L("인코너판넬v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

			moveIn3D ('y', incornerPanel.radAng, infoWall.wallThk + placingZone->gap, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);		// 벽과의 간격만큼 이동

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (xx = 0 ; xx < varEnd ; ++xx) {
				if (placingZone->bExtra == true)
					lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
				else
					lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

				elemList_Back.Push (incornerPanel.placeObject (5,
					"in_comp", APIParT_CString, "인코너판넬",
					"wid_s", APIParT_Length, format_string ("%f", placingZone->lenLcorner),
					"leng_s", APIParT_Length, format_string ("%f", 0.100),
					"hei_s", APIParT_Length, format_string ("%f", lengthDouble),
					"dir_s", APIParT_CString, "세우기"));

				moveIn3D ('z', incornerPanel.radAng, lengthDouble, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);
			}
		}
	} else if (placingZone->typeLcorner == OUTCORNER_PANEL) {
		// 앞면
		EasyObjectPlacement outcornerPanel;
		outcornerPanel.init (L("아웃코너판넬v1.0.gsm"), layerInd_OutcornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

		moveIn3D ('y', outcornerPanel.radAng, -placingZone->gap, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);	// 벽과의 간격만큼 이동

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
			elemList_Front.Push (outcornerPanel.placeObject (4,
				"wid_s", APIParT_Length, format_string ("%f", placingZone->lenLcorner),
				"leng_s", APIParT_Length, format_string ("%f", 0.100),
				"hei_s", APIParT_Length, format_string ("%f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
				"dir_s", APIParT_CString, "세우기"));

			moveIn3D ('z', outcornerPanel.radAng, (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);
		}

		// 뒷면
		if (placingZone->bSingleSide == false) {
			outcornerPanel.init (L("아웃코너판넬v1.0.gsm"), layerInd_OutcornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (90.0));

			moveIn3D ('y', outcornerPanel.radAng + DegreeToRad (90.0), infoWall.wallThk + placingZone->gap, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);		// 벽과의 간격만큼 이동

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (xx = 0 ; xx < varEnd ; ++xx) {
				if (placingZone->bExtra == true)
					lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
				else
					lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

				elemList_Back.Push (outcornerPanel.placeObject (4,
					"wid_s", APIParT_Length, format_string ("%f", 0.100),
					"leng_s", APIParT_Length, format_string ("%f", placingZone->lenLcorner),
					"hei_s", APIParT_Length, format_string ("%f", lengthDouble),
					"dir_s", APIParT_CString, "세우기"));

				moveIn3D ('z', outcornerPanel.radAng, lengthDouble, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);
			}
		}
	} else if (placingZone->typeLcorner == OUTCORNER_ANGLE) {
		// 앞면
		EasyObjectPlacement outcornerAngle;
		outcornerAngle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

		moveIn3D ('y', outcornerAngle.radAng - DegreeToRad (180.0), -placingZone->gap, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);	// 벽과의 간격만큼 이동

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
			elemList_Front.Push (outcornerAngle.placeObject (2,
				"a_leng", APIParT_Length, format_string ("%f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
				"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

			moveIn3D ('z', outcornerAngle.radAng, (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);
		}

		// 뒷면
		if (placingZone->bSingleSide == false) {
			outcornerAngle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (90.0));

			moveIn3D ('y', outcornerAngle.radAng - DegreeToRad (90.0), infoWall.wallThk + placingZone->gap, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);		// 벽과의 간격만큼 이동

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (xx = 0 ; xx < varEnd ; ++xx) {
				if (placingZone->bExtra == true)
					lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
				else
					lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

				elemList_Back.Push (outcornerAngle.placeObject (2,
					"a_leng", APIParT_Length, format_string ("%f", lengthDouble),
					"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

				moveIn3D ('z', outcornerAngle.radAng - DegreeToRad (90.0), lengthDouble, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);
			}
		}
	}
	
	// 결과물 전체 그룹화 (앞면)
	groupElements (elemList_Front);
	elemList_Front.Clear ();

	// 결과물 전체 그룹화 (뒷면)
	groupElements (elemList_Back);
	elemList_Back.Clear ();

	// 우측 인코너/아웃코너 배치
	if (placingZone->typeRcorner == INCORNER_PANEL) {
		// 앞면
		EasyObjectPlacement incornerPanel;
		incornerPanel.init (L("인코너판넬v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

		moveIn3D ('y', incornerPanel.radAng - DegreeToRad (180.0), -placingZone->gap, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);		// 벽과의 간격만큼 이동
		moveIn3D ('x', incornerPanel.radAng - DegreeToRad (180.0), placingZone->horLen, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);		// 영역 우측으로 이동

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
			elemList_Front.Push (incornerPanel.placeObject (5,
				"in_comp", APIParT_CString, "인코너판넬",
				"wid_s", APIParT_Length, format_string ("%f", placingZone->lenRcorner),
				"leng_s", APIParT_Length, format_string ("%f", 0.100),
				"hei_s", APIParT_Length, format_string ("%f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
				"dir_s", APIParT_CString, "세우기"));

			moveIn3D ('z', incornerPanel.radAng - DegreeToRad (180.0), (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);
		}

		// 뒷면
		if (placingZone->bSingleSide == false) {
			incornerPanel.init (L("인코너판넬v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (90.0));

			moveIn3D ('y', incornerPanel.radAng - DegreeToRad (90.0), infoWall.wallThk + placingZone->gap, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);	// 벽과의 간격만큼 이동
			moveIn3D ('x', incornerPanel.radAng - DegreeToRad (90.0), placingZone->horLen, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);					// 영역 우측으로 이동

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (xx = 0 ; xx < varEnd ; ++xx) {
				if (placingZone->bExtra == true)
					lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
				else
					lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

				elemList_Back.Push (incornerPanel.placeObject (5,
					"in_comp", APIParT_CString, "인코너판넬",
					"wid_s", APIParT_Length, format_string ("%f", 0.100),
					"leng_s", APIParT_Length, format_string ("%f", placingZone->lenRcorner),
					"hei_s", APIParT_Length, format_string ("%f", lengthDouble),
					"dir_s", APIParT_CString, "세우기"));

				moveIn3D ('z', incornerPanel.radAng - DegreeToRad (90.0), lengthDouble, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);
			}
		}
	} else if (placingZone->typeRcorner == OUTCORNER_PANEL) {
		// 앞면
		EasyObjectPlacement outcornerPanel;
		outcornerPanel.init (L("아웃코너판넬v1.0.gsm"), layerInd_OutcornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (90.0));

		moveIn3D ('y', outcornerPanel.radAng - DegreeToRad (90.0), -placingZone->gap, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);		// 벽과의 간격만큼 이동
		moveIn3D ('x', outcornerPanel.radAng - DegreeToRad (90.0), placingZone->horLen, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);	// 영역 우측으로 이동

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
			elemList_Front.Push (outcornerPanel.placeObject (4,
				"wid_s", APIParT_Length, format_string ("%f", 0.100),
				"leng_s", APIParT_Length, format_string ("%f", placingZone->lenRcorner),
				"hei_s", APIParT_Length, format_string ("%f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
				"dir_s", APIParT_CString, "세우기"));

			moveIn3D ('z', outcornerPanel.radAng - DegreeToRad (90.0), (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);
		}

		// 뒷면
		if (placingZone->bSingleSide == false) {
			outcornerPanel.init (L("아웃코너판넬v1.0.gsm"), layerInd_OutcornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

			moveIn3D ('y', outcornerPanel.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);		// 벽과의 간격만큼 이동
			moveIn3D ('x', outcornerPanel.radAng - DegreeToRad (180.0), placingZone->horLen, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);						// 영역 우측으로 이동

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (xx = 0 ; xx < varEnd ; ++xx) {
				if (placingZone->bExtra == true)
					lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
				else
					lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

				elemList_Back.Push (outcornerPanel.placeObject (4,
					"wid_s", APIParT_Length, format_string ("%f", placingZone->lenRcorner),
					"leng_s", APIParT_Length, format_string ("%f", 0.100),
					"hei_s", APIParT_Length, format_string ("%f", lengthDouble),
					"dir_s", APIParT_CString, "세우기"));

				moveIn3D ('z', outcornerPanel.radAng - DegreeToRad (180.0), lengthDouble, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);
			}
		}
	} else if (placingZone->typeRcorner == OUTCORNER_ANGLE) {
		// 앞면
		EasyObjectPlacement outcornerAngle;
		outcornerAngle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (270.0));

		moveIn3D ('y', outcornerAngle.radAng - DegreeToRad (270.0), -placingZone->gap, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);	// 벽과의 간격만큼 이동
		moveIn3D ('x', outcornerAngle.radAng - DegreeToRad (270.0), placingZone->horLen, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);	// 영역 우측으로 이동

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
			elemList_Front.Push (outcornerAngle.placeObject (2,
				"a_leng", APIParT_Length, format_string ("%f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
				"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

			moveIn3D ('z', outcornerAngle.radAng - DegreeToRad (270.0), (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);
		}

		// 뒷면
		if (placingZone->bSingleSide == false) {
			outcornerAngle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

			moveIn3D ('y', outcornerAngle.radAng, infoWall.wallThk + placingZone->gap, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);		// 벽과의 간격만큼 이동
			moveIn3D ('x', outcornerAngle.radAng, placingZone->horLen, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);						// 영역 우측으로 이동

			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

			for (xx = 0 ; xx < varEnd ; ++xx) {
				if (placingZone->bExtra == true)
					lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
				else
					lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

				elemList_Back.Push (outcornerAngle.placeObject (2,
					"a_leng", APIParT_Length, format_string ("%f", lengthDouble),
					"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

				moveIn3D ('z', outcornerAngle.radAng, lengthDouble, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);
			}
		}
	}

	// 결과물 전체 그룹화 (앞면)
	groupElements (elemList_Front);
	elemList_Front.Clear ();

	// 결과물 전체 그룹화 (뒷면)
	groupElements (elemList_Back);
	elemList_Back.Clear ();

	// ================================================== 유로폼 배치
	// 앞면 배치
	EasyObjectPlacement euroform;
	euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', euroform.radAng, placingZone->lenLcorner, &euroform.posX, &euroform.posY, &euroform.posZ);		// 좌측 인코너 있으면 x 이동
	moveIn3D ('y', euroform.radAng, -placingZone->gap, &euroform.posX, &euroform.posY, &euroform.posZ);				// 벽과의 간격만큼 이동

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == EUROFORM) {
			accumDist = 0.0;

			for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
				if (placingZone->bVertical == true) {
					// 세로방향
					elemList_Front.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].tableInVerBasic [yy]),
						"u_ins", APIParT_CString, "벽세우기",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
				} else {
					// 가로방향
					moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					elemList_Front.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].tableInVerBasic [yy]),
						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
						"u_ins", APIParT_CString, "벽눕히기",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
					moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
				}
				accumDist += (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0;
				moveIn3D ('z', euroform.radAng, (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}
			moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);

			// 결과물 전체 그룹화 (앞면)
			groupElements (elemList_Front);
			elemList_Front.Clear ();
		}

		// 무조건 가로 방향으로 이동
		moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	}

	// 뒷면 배치
	if (placingZone->bSingleSide == false) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

		moveIn3D ('x', euroform.radAng - DegreeToRad (180.0), placingZone->lenLcorner, &euroform.posX, &euroform.posY, &euroform.posZ);					// 좌측 인코너 있으면 x 이동
		moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &euroform.posX, &euroform.posY, &euroform.posZ);		// 벽과의 간격만큼 이동

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
						// 세로방향
						moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
						elemList_Back.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
							"eu_hei", APIParT_CString, format_string ("%d", lengthInt),
							"u_ins", APIParT_CString, "벽세우기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					} else {
						// 가로방향
						elemList_Back.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", lengthInt),
							"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
							"u_ins", APIParT_CString, "벽눕히기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
					}
					accumDist += (double)lengthInt / 1000.0;
					moveIn3D ('z', euroform.radAng, (double)lengthInt / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
				}
				moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);

				// 결과물 전체 그룹화 (뒷면)
				groupElements (elemList_Back);
				elemList_Back.Clear ();
			}

			// 무조건 가로 방향으로 이동
			moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
		}
	}

	// ================================================== 휠러스페이서 배치 (항상 세로방향)
	// 앞면 배치
	EasyObjectPlacement fillersp;
	fillersp.init (L("휠러스페이서v1.0.gsm"), layerInd_Fillersp, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', fillersp.radAng, placingZone->lenLcorner, &fillersp.posX, &fillersp.posY, &fillersp.posZ);		// 좌측 인코너 있으면 x 이동
	moveIn3D ('y', fillersp.radAng, -placingZone->gap, &fillersp.posX, &fillersp.posY, &fillersp.posZ);				// 벽과의 간격만큼 이동

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
					"f_thk", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
					"f_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
					"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
					"f_rota", APIParT_Angle, format_string ("%f", 0.0)));
				moveIn3D ('z', fillersp.radAng, (double)lengthInt / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

				remainLengthInt -= 2400;
			}
			moveIn3D ('x', fillersp.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
			moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

			// 결과물 전체 그룹화 (앞면)
			groupElements (elemList_Front);
			elemList_Front.Clear ();
		}

		// 무조건 가로 방향으로 이동
		moveIn3D ('x', fillersp.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	}

	// 뒷면 배치
	if (placingZone->bSingleSide == false) {
		fillersp.init (L("휠러스페이서v1.0.gsm"), layerInd_Fillersp, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

		moveIn3D ('x', fillersp.radAng - DegreeToRad (180.0), placingZone->lenLcorner, &fillersp.posX, &fillersp.posY, &fillersp.posZ);					// 좌측 인코너 있으면 x 이동
		moveIn3D ('y', fillersp.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &fillersp.posX, &fillersp.posY, &fillersp.posZ);		// 벽과의 간격만큼 이동

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
						"f_thk", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
						"f_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
						"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
						"f_rota", APIParT_Angle, format_string ("%f", 0.0)));
					moveIn3D ('z', fillersp.radAng, (double)lengthInt / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

					remainLengthInt -= 2400;
				}
				moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

				// 결과물 전체 그룹화 (뒷면)
				groupElements (elemList_Back);
				elemList_Back.Clear ();
			}

			// 무조건 가로 방향으로 이동
			moveIn3D ('x', fillersp.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
		}
	}

	// ================================================== 합판 배치 (항상 세로방향)
	// 앞면 배치
	EasyObjectPlacement plywood;
	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', plywood.radAng, placingZone->lenLcorner, &plywood.posX, &plywood.posY, &plywood.posZ);		// 좌측 인코너 있으면 x 이동
	moveIn3D ('y', plywood.radAng, -placingZone->gap, &plywood.posX, &plywood.posY, &plywood.posZ);				// 벽과의 간격만큼 이동

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
					"p_stan", APIParT_CString, "비규격",
					"w_dir", APIParT_CString, "벽세우기",
					"p_thk", APIParT_CString, "11.5T",
					"p_wid", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
					"p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
					"p_ang", APIParT_Angle, format_string ("%f", 0.0),
					"sogak", APIParT_Boolean, "1.0",
					"bInverseSogak", APIParT_Boolean, "1.0",
					"prof", APIParT_CString, "소각",
					"gap_a", APIParT_Length, format_string ("%f", 0.0),
					"gap_b", APIParT_Length, format_string ("%f", 0.0),
					"gap_c", APIParT_Length, format_string ("%f", 0.0),
					"gap_d", APIParT_Length, format_string ("%f", 0.0)));
				moveIn3D ('z', plywood.radAng, (double)lengthInt / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);

				remainLengthInt -= 2400;
			}
			moveIn3D ('z', plywood.radAng, -accumDist, &plywood.posX, &plywood.posY, &plywood.posZ);

			// 결과물 전체 그룹화 (앞면)
			groupElements (elemList_Front);
			elemList_Front.Clear ();
		}

		// 무조건 가로 방향으로 이동
		moveIn3D ('x', plywood.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
	}

	// 뒷면 배치
	if (placingZone->bSingleSide == false) {
		plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

		moveIn3D ('x', plywood.radAng - DegreeToRad (180.0), placingZone->lenLcorner, &plywood.posX, &plywood.posY, &plywood.posZ);					// 좌측 인코너 있으면 x 이동
		moveIn3D ('y', plywood.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &plywood.posX, &plywood.posY, &plywood.posZ);		// 벽과의 간격만큼 이동

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
						"p_stan", APIParT_CString, "비규격",
						"w_dir", APIParT_CString, "벽세우기",
						"p_thk", APIParT_CString, "11.5T",
						"p_wid", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
						"p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
						"p_ang", APIParT_Angle, format_string ("%f", 0.0),
						"sogak", APIParT_Boolean, "1.0",
						"bInverseSogak", APIParT_Boolean, "1.0",
						"prof", APIParT_CString, "소각",
						"gap_a", APIParT_Length, format_string ("%f", 0.0),
						"gap_b", APIParT_Length, format_string ("%f", 0.0),
						"gap_c", APIParT_Length, format_string ("%f", 0.0),
						"gap_d", APIParT_Length, format_string ("%f", 0.0)));
					moveIn3D ('z', plywood.radAng, (double)lengthInt / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);

					remainLengthInt -= 2400;
				}
				moveIn3D ('x', plywood.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
				moveIn3D ('z', plywood.radAng, -accumDist, &plywood.posX, &plywood.posY, &plywood.posZ);

				// 결과물 전체 그룹화 (뒷면)
				groupElements (elemList_Back);
				elemList_Back.Clear ();
			}

			// 무조건 가로 방향으로 이동
			moveIn3D ('x', plywood.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
		}
	}

	// ================================================== 각재 배치 (항상 세로방향)
	// 앞면 배치
	EasyObjectPlacement timber;
	timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', timber.radAng, placingZone->lenLcorner, &timber.posX, &timber.posY, &timber.posZ);		// 좌측 인코너 있으면 x 이동
	moveIn3D ('y', timber.radAng, -placingZone->gap, &timber.posX, &timber.posY, &timber.posZ);				// 벽과의 간격만큼 이동

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
					"w_ins", APIParT_CString, "벽세우기",
					"w_w", APIParT_Length, format_string ("%f", 0.050),
					"w_h", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
					"w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
					"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
					"torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
				moveIn3D ('z', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

				remainLengthInt -= 3600;
			}
			moveIn3D ('x', timber.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
			moveIn3D ('z', timber.radAng, -accumDist, &timber.posX, &timber.posY, &timber.posZ);
			
			// 결과물 전체 그룹화 (앞면)
			groupElements (elemList_Front);
			elemList_Front.Clear ();
		}

		// 무조건 가로 방향으로 이동
		moveIn3D ('x', timber.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
	}

	// 뒷면 배치
	if (placingZone->bSingleSide == false) {
		timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

		moveIn3D ('x', timber.radAng - DegreeToRad (180.0), placingZone->lenLcorner, &timber.posX, &timber.posY, &timber.posZ);					// 좌측 인코너 있으면 x 이동
		moveIn3D ('y', timber.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &timber.posX, &timber.posY, &timber.posZ);		// 벽과의 간격만큼 이동

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
						"w_ins", APIParT_CString, "벽세우기",
						"w_w", APIParT_Length, format_string ("%f", 0.050),
						"w_h", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
						"w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
						"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
						"torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
					moveIn3D ('z', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

					remainLengthInt -= 3600;
				}
				moveIn3D ('z', timber.radAng, -accumDist, &timber.posX, &timber.posY, &timber.posZ);

				// 결과물 전체 그룹화 (뒷면)
				groupElements (elemList_Back);
				elemList_Back.Clear ();
			}

			// 무조건 가로 방향으로 이동
			moveIn3D ('x', timber.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
		}
	}
	
	// ================================================== 테이블폼 배치
	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->placeEuroformsOfTableform (this, xx);		// 테이블폼의 유로폼 배치 (타입 불문 공통)
		
		if (placingZone->tableformType == 1)		placingZone->placeTableformA (this, xx);
		else if (placingZone->tableformType == 2)	placingZone->placeTableformB (this, xx);
		else if (placingZone->tableformType == 3)	placingZone->placeTableformC (this, xx);

		// 결과물 전체 그룹화 (앞면)
		groupElements (elemList_Front);
		elemList_Front.Clear ();

		// 결과물 전체 그룹화 (뒷면)
		groupElements (elemList_Back);
		elemList_Back.Clear ();
	}

	return err;
}

// 상단 여백을 유로폼 또는 합판, 각재 등으로 채움
GSErrCode	WallTableformPlacingZone::fillRestAreas (WallTableformPlacingZone* placingZone, short idxCell)
{
	GSErrCode	err = NoError;
	short	yy;
	int		remainLengthInt, lengthInt, remainLengthIntStored;

	double	plywoodMarginBasic;
	double	plywoodMarginExtra;

	// 유로폼이 차지하는 부분을 제외한 합판/각재가 덮는 부분의 너비
	plywoodMarginBasic = placingZone->marginTopBasic - (placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2);
	plywoodMarginExtra = placingZone->marginTopExtra - (placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2);
	
	remainLengthIntStored = remainLengthInt = (int)(placingZone->cells [idxCell].horLen * 1000);	// 채워야 할 전체 너비를 계산해야 함

	// 블루클램프 및 블루목심레일 장착 - 앞면
	EasyObjectPlacement blueClamp;
	blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoWall.floorInd, placingZone->marginCellsBasic [idxCell].leftBottomX, placingZone->marginCellsBasic [idxCell].leftBottomY, placingZone->marginCellsBasic [idxCell].leftBottomZ, placingZone->marginCellsBasic [idxCell].ang + DegreeToRad (270.0));
	moveIn3D ('y', blueClamp.radAng - DegreeToRad (270.0), -0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
	moveIn3D ('z', blueClamp.radAng - DegreeToRad (270.0), 0.040 + placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

	EasyObjectPlacement blueTimberRail;
	blueTimberRail.init (L("블루목심v1.0.gsm"), layerInd_BlueTimberRail, infoWall.floorInd, placingZone->marginCellsBasic [idxCell].leftBottomX, placingZone->marginCellsBasic [idxCell].leftBottomY, placingZone->marginCellsBasic [idxCell].leftBottomZ, placingZone->marginCellsBasic [idxCell].ang);
	if (plywoodMarginBasic + EPS > 0.0) {
		if ((plywoodMarginBasic >= 0.010 - EPS) && (plywoodMarginBasic <= 0.020 + EPS)) {
			moveIn3D ('x', blueTimberRail.radAng, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			moveIn3D ('z', blueTimberRail.radAng, -0.003 + placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
		} else if (abs (plywoodMarginBasic - 0.040) < EPS) {
			moveIn3D ('x', blueTimberRail.radAng, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			moveIn3D ('z', blueTimberRail.radAng, -0.003 + placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
		} else if ((plywoodMarginBasic >= 0.050 - EPS) && (plywoodMarginBasic <= 0.070 + EPS)) {
			moveIn3D ('x', blueTimberRail.radAng, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			moveIn3D ('y', blueTimberRail.radAng, -0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			moveIn3D ('z', blueTimberRail.radAng, -0.003 + placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
		} else if (plywoodMarginBasic >= 0.080 - EPS) {
			moveIn3D ('x', blueTimberRail.radAng, -0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			moveIn3D ('y', blueTimberRail.radAng, -0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			moveIn3D ('z', blueTimberRail.radAng, -0.003 + placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
		}
	}

	if (placingZone->marginCellsBasic [idxCell].bFill == true) {
		if (plywoodMarginBasic > 0.100 + EPS) {
			// 합판과 유로폼은 블루 클램프로 고정
			if (placingZone->cells [idxCell].objType == TABLEFORM) {
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
					if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.200, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.100, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.050, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.100, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.050, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					}
				}
			} else if (placingZone->cells [idxCell].objType == EUROFORM) {
				if (placingZone->cells [idxCell].horLen == 600) {
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				} else if (placingZone->cells [idxCell].horLen == 500) {
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.200, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				} else if (placingZone->cells [idxCell].horLen == 450) {
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				} else if (placingZone->cells [idxCell].horLen == 400) {
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.100, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				} else if (placingZone->cells [idxCell].horLen == 300) {
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				} else if (placingZone->cells [idxCell].horLen == 200) {
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.050, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.100, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.050, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				} else if (placingZone->cells [idxCell].horLen == 900) {
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				} else if (placingZone->cells [idxCell].horLen == 1200) {
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					elemList_Front.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
					moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
				}
			}
		} else {
			// 각재와 유로폼은 블루 목심레일로 고정
			if (plywoodMarginBasic + EPS > 0.0) {
				if ((plywoodMarginBasic >= 0.010 - EPS) && (plywoodMarginBasic <= 0.020 + EPS)) {
					if (placingZone->cells [idxCell].objType == TABLEFORM) {
						for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
							if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else {
								moveIn3D ('x', blueTimberRail.radAng, (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							}
						}
					} else if (placingZone->cells [idxCell].objType == EUROFORM) {
						if (placingZone->cells [idxCell].horLen == 600) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 500) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 450) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 400) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 900) {
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 1200) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else  {
							moveIn3D ('x', blueTimberRail.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						}
					}
				} else if (abs (plywoodMarginBasic - 0.040) < EPS) {
					if (placingZone->cells [idxCell].objType == TABLEFORM) {
						for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
							if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else {
								moveIn3D ('x', blueTimberRail.radAng, (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							}
						}
					} else if (placingZone->cells [idxCell].objType == EUROFORM) {
						if (placingZone->cells [idxCell].horLen == 600) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 500) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 450) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 400) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 900) {
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 1200) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else  {
							moveIn3D ('x', blueTimberRail.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						}
					}
				} else if ((plywoodMarginBasic >= 0.050 - EPS) && (plywoodMarginBasic <= 0.070 + EPS)) {
					if (placingZone->cells [idxCell].objType == TABLEFORM) {
						for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
							if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else {
								moveIn3D ('x', blueTimberRail.radAng, (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							}
						}
					} else if (placingZone->cells [idxCell].objType == EUROFORM) {
						if (placingZone->cells [idxCell].horLen == 600) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 500) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 450) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 400) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 900) {
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 1200) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else  {
							moveIn3D ('x', blueTimberRail.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						}
					}
				} else if (plywoodMarginBasic >= 0.080 - EPS) {
					if (placingZone->cells [idxCell].objType == TABLEFORM) {
						for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
							if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
								moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else {
								moveIn3D ('x', blueTimberRail.radAng, (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							}
						}
					} else if (placingZone->cells [idxCell].objType == EUROFORM) {
						if (placingZone->cells [idxCell].horLen == 600) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 500) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 450) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 400) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 900) {
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else if (placingZone->cells [idxCell].horLen == 1200) {
							moveIn3D ('x', blueTimberRail.radAng, 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							elemList_Front.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
							moveIn3D ('x', blueTimberRail.radAng, 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						} else  {
							moveIn3D ('x', blueTimberRail.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
						}
					}
				}
			}
		}
	}

	// 블루클램프 및 블루목심레일 장착 - 뒷면
	if (placingZone->bSingleSide == false) {
		blueClamp.init (L("블루클램프v1.0.gsm"), layerInd_BlueClamp, infoWall.floorInd, placingZone->marginCellsExtra [idxCell].leftBottomX, placingZone->marginCellsExtra [idxCell].leftBottomY, placingZone->marginCellsExtra [idxCell].leftBottomZ, placingZone->marginCellsExtra [idxCell].ang + DegreeToRad (270.0) + DegreeToRad (180.0));
		moveIn3D ('y', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.0659, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
		moveIn3D ('z', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.040 + placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);

		blueTimberRail.init (L("블루목심v1.0.gsm"), layerInd_BlueTimberRail, infoWall.floorInd, placingZone->marginCellsExtra [idxCell].leftBottomX, placingZone->marginCellsExtra [idxCell].leftBottomY, placingZone->marginCellsExtra [idxCell].leftBottomZ, placingZone->marginCellsExtra [idxCell].ang + DegreeToRad (180.0));
		if (plywoodMarginExtra + EPS > 0.0) {
			if ((plywoodMarginExtra >= 0.010 - EPS) && (plywoodMarginExtra <= 0.020 + EPS)) {
				moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.194 - 0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng - DegreeToRad (180.0), -0.003 + placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			} else if (abs (plywoodMarginExtra - 0.040) < EPS) {
				moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.194 - 0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng - DegreeToRad (180.0), -0.003 + placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			} else if ((plywoodMarginExtra >= 0.050 - EPS) && (plywoodMarginExtra <= 0.070 + EPS)) {
				moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.194 - 0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.0525, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng - DegreeToRad (180.0), -0.003 + placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			} else if (plywoodMarginExtra >= 0.080 - EPS) {
				moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.194 - 0.023, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('y', blueTimberRail.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.053, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
				moveIn3D ('z', blueTimberRail.radAng - DegreeToRad (180.0), -0.003 + placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
			}
		}

		if (placingZone->marginCellsExtra [idxCell].bFill == true) {
			// 유로폼을 전혀 체크하지 않은 경우
			if (plywoodMarginExtra > 0.100 + EPS) {
				// 합판과 유로폼은 블루 클램프로 고정
				if (placingZone->cells [idxCell].objType == TABLEFORM) {
					for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
						if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.200, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.100, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.050, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.100, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.050, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
							elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
							moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						}
					}
				} else if (placingZone->cells [idxCell].objType == EUROFORM) {
					if (placingZone->cells [idxCell].horLen == 600) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].horLen == 500) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.200, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].horLen == 450) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].horLen == 400) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.100, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].horLen == 300) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].horLen == 200) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.050, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.100, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.050, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].horLen == 900) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					} else if (placingZone->cells [idxCell].horLen == 1200) {
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
						elemList_Back.Push (blueClamp.placeObject (4, "type", APIParT_CString, "유로목재클램프(제작품v1)", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "openingWidth", APIParT_Length, format_string ("%f", 0.048)));
						moveIn3D ('x', blueClamp.radAng - DegreeToRad (270.0) - DegreeToRad (180.0), 0.300, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
					}
				}

			} else {
				// 각재와 유로폼은 블루 목심레일로 고정
				if (plywoodMarginExtra + EPS > 0.0) {
					if ((plywoodMarginExtra >= 0.010 - EPS) && (plywoodMarginExtra <= 0.020 + EPS)) {
						if (placingZone->cells [idxCell].objType == TABLEFORM) {
							for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								}
							}
						} else if (placingZone->cells [idxCell].objType == EUROFORM) {
							if (placingZone->cells [idxCell].horLen == 600) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 500) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 450) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 400) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 900) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 1200) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 4", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else  {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							}
						}
					} else if (abs (plywoodMarginExtra - 0.040) < EPS) {
						if (placingZone->cells [idxCell].objType == TABLEFORM) {
							for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								}
							}
						} else if (placingZone->cells [idxCell].objType == EUROFORM) {
							if (placingZone->cells [idxCell].horLen == 600) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 500) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 450) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 400) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 900) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 1200) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else  {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							}
						}
					} else if ((plywoodMarginExtra >= 0.050 - EPS) && (plywoodMarginExtra <= 0.070 + EPS)) {
						if (placingZone->cells [idxCell].objType == TABLEFORM) {
							for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								}
							}
						} else if (placingZone->cells [idxCell].objType == EUROFORM) {
							if (placingZone->cells [idxCell].horLen == 600) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 500) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 450) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 400) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 900) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 1200) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 3", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else  {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							}
						}
					} else if (plywoodMarginExtra >= 0.080 - EPS) {
						if (placingZone->cells [idxCell].objType == TABLEFORM) {
							for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 900) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 1200) {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
									elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								} else {
									moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								}
							}
						} else if (placingZone->cells [idxCell].objType == EUROFORM) {
							if (placingZone->cells [idxCell].horLen == 600) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 500) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.350, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 450) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 400) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.250, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 900) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.450, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else if (placingZone->cells [idxCell].horLen == 1200) {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.150, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.750, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
								elemList_Back.Push (blueTimberRail.placeObject (3, "railType", APIParT_CString, "블루목심 1", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), 0.300, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							} else  {
								moveIn3D ('x', blueTimberRail.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0, &blueTimberRail.posX, &blueTimberRail.posY, &blueTimberRail.posZ);
							}
						}
					}
				}
			}
		}
	}

	// 합판 및 각재 채우기
	EasyObjectPlacement	plywood;
	EasyObjectPlacement plywood1, plywood2, plywood3;
	EasyObjectPlacement timber;
	short	addedPlywood;
	double	moveZ;
	bool	bFirstPlywood, bLastPlywood;

	// 채워야 할 전체 너비를 계산해야 함
	remainLengthIntStored = remainLengthInt = placingZone->cells [idxCell].horLen;

	if (placingZone->marginCellsBasic [idxCell].bFill == true) {
		if (plywoodMarginBasic > 0.100 + EPS) {
			bFirstPlywood = true;
			bLastPlywood = false;

			// 앞면 채우기
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->marginCellsBasic [idxCell].leftBottomX, placingZone->marginCellsBasic [idxCell].leftBottomY, placingZone->marginCellsBasic [idxCell].leftBottomZ, placingZone->marginCellsBasic [idxCell].ang);

			moveIn3D ('z', plywood.radAng, placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &plywood.posX, &plywood.posY, &plywood.posZ);

			while (remainLengthInt > 0) {
				if (remainLengthInt <= 2400)
					bLastPlywood = true;

				if (remainLengthInt >= 2400)
					lengthInt = 2400;
				else
					lengthInt = remainLengthInt;

				if ((plywoodMarginBasic > EPS) && (lengthInt > 0)) {
					if (idxCell == 0) {
						if (bFirstPlywood == true)
							elemList_Front.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginBasic), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.070), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.0), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
						else
							elemList_Front.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginBasic), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.0), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.0), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
					} else if (idxCell == this->nCellsInHor-1) {
						if (bLastPlywood == true)
							elemList_Front.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginBasic), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.0), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.070), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
						else
							elemList_Front.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginBasic), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.0), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.0), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
					} else {
						elemList_Front.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginBasic), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.0), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.0), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
					}
				}
				moveIn3D ('x', plywood.radAng, (double)lengthInt / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);

				remainLengthInt -= 2400;
				bFirstPlywood = false;
			}
		} else {
			// 앞면 채우기
			timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoWall.floorInd, placingZone->marginCellsBasic [idxCell].leftBottomX, placingZone->marginCellsBasic [idxCell].leftBottomY, placingZone->marginCellsBasic [idxCell].leftBottomZ, placingZone->marginCellsBasic [idxCell].ang);
			plywood1.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->marginCellsBasic [idxCell].leftBottomX, placingZone->marginCellsBasic [idxCell].leftBottomY, placingZone->marginCellsBasic [idxCell].leftBottomZ, placingZone->marginCellsBasic [idxCell].ang);
			plywood2.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->marginCellsBasic [idxCell].leftBottomX, placingZone->marginCellsBasic [idxCell].leftBottomY, placingZone->marginCellsBasic [idxCell].leftBottomZ + 0.0115, placingZone->marginCellsBasic [idxCell].ang);
			plywood3.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->marginCellsBasic [idxCell].leftBottomX, placingZone->marginCellsBasic [idxCell].leftBottomY, placingZone->marginCellsBasic [idxCell].leftBottomZ + 0.0115*2, placingZone->marginCellsBasic [idxCell].ang);

			moveIn3D ('z', timber.radAng, placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &timber.posX, &timber.posY, &timber.posZ);

			moveIn3D ('z', plywood1.radAng, placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
			moveIn3D ('z', plywood2.radAng, placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
			moveIn3D ('z', plywood3.radAng, placingZone->marginCellsBasic [idxCell].formWidth1 + placingZone->marginCellsBasic [idxCell].formWidth2, &plywood3.posX, &plywood3.posY, &plywood3.posZ);

			if (plywoodMarginBasic + EPS > 0.0) {
				// 유로폼 상단 앞쪽에 투바이(50*80) 배치 - 여백에 부착하지 않음
				if ((plywoodMarginBasic >= 0.010 - EPS) && (plywoodMarginBasic <= 0.030 + EPS)) {
					moveIn3D ('y', timber.radAng, -0.067, &timber.posX, &timber.posY, &timber.posZ);
					moveIn3D ('z', timber.radAng, -0.080, &timber.posX, &timber.posY, &timber.posZ);

					while (remainLengthInt > 0) {
						if (remainLengthInt >= 3600)
							lengthInt = 3600;
						else
							lengthInt = remainLengthInt;

						elemList_Front.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", 0.050), "w_h", APIParT_Length, format_string ("%f", 0.080), "w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
						moveIn3D ('x', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

						remainLengthInt -= 3600;
					}

					if (abs (plywoodMarginBasic - 0.010) < EPS)	addedPlywood = 1;	// 10mm 이면 합판 1장 얹음
					if (abs (plywoodMarginBasic - 0.020) < EPS)	addedPlywood = 2;	// 20mm 이면 합판 2장 얹음
					if (abs (plywoodMarginBasic - 0.030) < EPS)	addedPlywood = 3;	// 30mm 이면 합판 3장 얹음
					moveZ = 0.0;

					remainLengthInt = remainLengthIntStored;

					while (remainLengthInt > 0) {
						if (remainLengthInt >= 2400)
							lengthInt = 2400;
						else
							lengthInt = remainLengthInt;
						
						if (addedPlywood >= 1) {
							moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							elemList_Front.Push (plywood1.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, -moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('x', plywood1.radAng, (double)lengthInt / 1000.0, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						}
						if (addedPlywood >= 2) {
							moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							elemList_Front.Push (plywood2.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, -moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('x', plywood2.radAng, (double)lengthInt / 1000.0, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						}
						if (addedPlywood >= 3) {
							moveIn3D ('y', plywood3.radAng, -0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							moveIn3D ('z', plywood3.radAng, moveZ, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							elemList_Front.Push (plywood3.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood3.radAng, 0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							moveIn3D ('z', plywood3.radAng, -moveZ, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							moveIn3D ('x', plywood3.radAng, (double)lengthInt / 1000.0, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						}

						remainLengthInt -= 2400;
					}

				// 여백에 다루끼(50*40) 배치
				} else if (abs (plywoodMarginBasic - 0.040) < EPS) {
					while (remainLengthInt > 0) {
						if (remainLengthInt >= 3600)
							lengthInt = 3600;
						else
							lengthInt = remainLengthInt;

						elemList_Front.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", 0.050), "w_h", APIParT_Length, format_string ("%f", 0.040), "w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
						moveIn3D ('x', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

						remainLengthInt -= 3600;
					}

				// 여백에 투바이(80*50) 배치
				} else if ((plywoodMarginBasic >= 0.050 - EPS) && (plywoodMarginBasic <= 0.070 + EPS)) {
					while (remainLengthInt > 0) {
						if (remainLengthInt >= 3600)
							lengthInt = 3600;
						else
							lengthInt = remainLengthInt;

						elemList_Front.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", 0.080), "w_h", APIParT_Length, format_string ("%f", 0.050), "w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
						moveIn3D ('x', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

						remainLengthInt -= 3600;
					}

					if (abs (plywoodMarginBasic - 0.060) < EPS)	addedPlywood = 1;	// 60mm 이면 합판 1장 얹음
					if (abs (plywoodMarginBasic - 0.070) < EPS)	addedPlywood = 2;	// 70mm 이면 합판 2장 얹음
					moveZ = 0.050;

					remainLengthInt = remainLengthIntStored;

					while (remainLengthInt > 0) {
						if (remainLengthInt >= 2400)
							lengthInt = 2400;
						else
							lengthInt = remainLengthInt;
						
						if (addedPlywood >= 1) {
							moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							elemList_Front.Push (plywood1.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, -moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('x', plywood1.radAng, (double)lengthInt / 1000.0, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						}
						if (addedPlywood >= 2) {
							moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							elemList_Front.Push (plywood2.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, -moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('x', plywood2.radAng, (double)lengthInt / 1000.0, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						}

						remainLengthInt -= 2400;
					}

				// 여백에 투바이(50*80) 배치
				} else if (plywoodMarginBasic >= 0.080 - EPS) {
					while (remainLengthInt > 0) {
						if (remainLengthInt >= 3600)
							lengthInt = 3600;
						else
							lengthInt = remainLengthInt;

						elemList_Front.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", 0.050), "w_h", APIParT_Length, format_string ("%f", 0.080), "w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
						moveIn3D ('x', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

						remainLengthInt -= 3600;
					}

					if (abs (plywoodMarginBasic - 0.090) < EPS)	addedPlywood = 1;	// 90mm 이면 합판 1장 얹음
					if (abs (plywoodMarginBasic - 0.100) < EPS)	addedPlywood = 2;	// 100mm 이면 합판 2장 얹음
					moveZ = 0.080;

					remainLengthInt = remainLengthIntStored;

					while (remainLengthInt > 0) {
						if (remainLengthInt >= 2400)
							lengthInt = 2400;
						else
							lengthInt = remainLengthInt;
						
						if (addedPlywood >= 1) {
							moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							elemList_Front.Push (plywood1.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, -moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('x', plywood1.radAng, (double)lengthInt / 1000.0, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						}
						if (addedPlywood >= 2) {
							moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							elemList_Front.Push (plywood2.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, -moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('x', plywood2.radAng, (double)lengthInt / 1000.0, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						}

						remainLengthInt -= 2400;
					}
				}
			}
		}
	}

	remainLengthIntStored = remainLengthInt = placingZone->cells [idxCell].horLen;

	if (placingZone->marginCellsExtra [idxCell].bFill == true) {
		if (plywoodMarginExtra > 0.100 + EPS) {
			bFirstPlywood = true;
			bLastPlywood = false;

			// 뒷면 채우기
			if (placingZone->bSingleSide == false) {
				remainLengthInt = remainLengthIntStored;
				plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->marginCellsExtra [idxCell].leftBottomX, placingZone->marginCellsExtra [idxCell].leftBottomY, placingZone->marginCellsExtra [idxCell].leftBottomZ, placingZone->marginCellsExtra [idxCell].ang + DegreeToRad (180.0));

				moveIn3D ('y', plywood.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &plywood.posX, &plywood.posY, &plywood.posZ);
				moveIn3D ('z', plywood.radAng - DegreeToRad (180.0), placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &plywood.posX, &plywood.posY, &plywood.posZ);

				while (remainLengthInt > 0) {
					if (remainLengthInt <= 2400)
						bLastPlywood = true;

					if (remainLengthInt >= 2400)
						lengthInt = 2400;
					else
						lengthInt = remainLengthInt;

					moveIn3D ('x', plywood.radAng, -(double)lengthInt / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
					if ((plywoodMarginExtra > EPS) && (lengthInt > 0)) {
						if (idxCell == 0) {
							if (bFirstPlywood == true)
								elemList_Back.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginExtra), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.0), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.070), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
							else
								elemList_Back.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginExtra), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.0), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.0), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
						} else if (idxCell == this->nCellsInHor-1) {
							if (bLastPlywood == true)
								elemList_Back.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginExtra), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.070), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.0), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
							else
								elemList_Back.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginExtra), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.0), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.0), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
						} else {
							elemList_Back.Push (plywood.placeObject (13, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "벽눕히기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", plywoodMarginExtra), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "1.0", "bInverseSogak", APIParT_Boolean, "1.0", "prof", APIParT_CString, "소각", "gap_a", APIParT_Length, format_string ("%f", 0.0), "gap_b", APIParT_Length, format_string ("%f", 0.0), "gap_c", APIParT_Length, format_string ("%f", 0.0), "gap_d", APIParT_Length, format_string ("%f", 0.0)));
						}
					}

					remainLengthInt -= 2400;
					bFirstPlywood = false;
				}
			}
		} else {
			// 뒷면 채우기
			if (placingZone->bSingleSide == false) {
				remainLengthInt = remainLengthIntStored;
				timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoWall.floorInd, placingZone->marginCellsExtra [idxCell].leftBottomX, placingZone->marginCellsExtra [idxCell].leftBottomY, placingZone->marginCellsExtra [idxCell].leftBottomZ, placingZone->marginCellsExtra [idxCell].ang + DegreeToRad (180.0));
				plywood1.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->marginCellsExtra [idxCell].leftBottomX, placingZone->marginCellsExtra [idxCell].leftBottomY, placingZone->marginCellsExtra [idxCell].leftBottomZ, placingZone->marginCellsExtra [idxCell].ang + DegreeToRad (180.0));
				plywood2.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->marginCellsExtra [idxCell].leftBottomX, placingZone->marginCellsExtra [idxCell].leftBottomY, placingZone->marginCellsExtra [idxCell].leftBottomZ + 0.0115, placingZone->marginCellsExtra [idxCell].ang + DegreeToRad (180.0));
				plywood3.init (L("합판v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->marginCellsExtra [idxCell].leftBottomX, placingZone->marginCellsExtra [idxCell].leftBottomY, placingZone->marginCellsExtra [idxCell].leftBottomZ + 0.0115*2, placingZone->marginCellsExtra [idxCell].ang + DegreeToRad (180.0));

				moveIn3D ('y', timber.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &timber.posX, &timber.posY, &timber.posZ);
				moveIn3D ('z', timber.radAng - DegreeToRad (180.0), placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &timber.posX, &timber.posY, &timber.posZ);

				moveIn3D ('y', plywood1.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
				moveIn3D ('y', plywood2.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
				moveIn3D ('y', plywood3.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
				moveIn3D ('z', plywood1.radAng - DegreeToRad (180.0), placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
				moveIn3D ('z', plywood2.radAng - DegreeToRad (180.0), placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
				moveIn3D ('z', plywood3.radAng - DegreeToRad (180.0), placingZone->marginCellsExtra [idxCell].formWidth1 + placingZone->marginCellsExtra [idxCell].formWidth2, &plywood3.posX, &plywood3.posY, &plywood3.posZ);

				if (plywoodMarginExtra + EPS > 0.0) {
					// 유로폼 상단 앞쪽에 투바이(50*80) 배치 - 여백에 부착하지 않음
					if ((plywoodMarginExtra >= 0.010 - EPS) && (plywoodMarginExtra <= 0.030 + EPS)) {
						moveIn3D ('y', timber.radAng - DegreeToRad (180.0), 0.067, &timber.posX, &timber.posY, &timber.posZ);
						moveIn3D ('z', timber.radAng - DegreeToRad (180.0), -0.080, &timber.posX, &timber.posY, &timber.posZ);

						while (remainLengthInt > 0) {
							if (remainLengthInt >= 3600)
								lengthInt = 3600;
							else
								lengthInt = remainLengthInt;

							moveIn3D ('x', timber.radAng - DegreeToRad (180.0), (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
							elemList_Back.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", 0.050), "w_h", APIParT_Length, format_string ("%f", 0.080), "w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));

							remainLengthInt -= 3600;
						}

						if (abs (plywoodMarginExtra - 0.010) < EPS)	addedPlywood = 1;	// 10mm 이면 합판 1장 얹음
						if (abs (plywoodMarginExtra - 0.020) < EPS)	addedPlywood = 2;	// 20mm 이면 합판 2장 얹음
						if (abs (plywoodMarginExtra - 0.030) < EPS)	addedPlywood = 3;	// 30mm 이면 합판 3장 얹음
						moveZ = 0.0;

						remainLengthInt = remainLengthIntStored;

						while (remainLengthInt > 0) {
							if (remainLengthInt >= 2400)
								lengthInt = 2400;
							else
								lengthInt = remainLengthInt;
						
							if (addedPlywood >= 1) {
								moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
								moveIn3D ('z', plywood1.radAng, moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
								moveIn3D ('x', plywood1.radAng, -(double)lengthInt / 1000.0, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
								elemList_Back.Push (plywood1.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
								moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
								moveIn3D ('z', plywood1.radAng, -moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							}
							if (addedPlywood >= 2) {
								moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
								moveIn3D ('z', plywood2.radAng, moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
								moveIn3D ('x', plywood2.radAng, -(double)lengthInt / 1000.0, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
								elemList_Back.Push (plywood2.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
								moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
								moveIn3D ('z', plywood2.radAng, -moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							}
							if (addedPlywood >= 3) {
								moveIn3D ('y', plywood3.radAng, -0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
								moveIn3D ('z', plywood3.radAng, moveZ, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
								moveIn3D ('x', plywood3.radAng, -(double)lengthInt / 1000.0, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
								elemList_Back.Push (plywood3.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
								moveIn3D ('y', plywood3.radAng, 0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
								moveIn3D ('z', plywood3.radAng, -moveZ, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							}

							remainLengthInt -= 2400;
						}

					// 여백에 다루끼(50*40) 배치
					} else if (abs (plywoodMarginExtra - 0.040) < EPS) {
						while (remainLengthInt > 0) {
							if (remainLengthInt >= 3600)
								lengthInt = 3600;
							else
								lengthInt = remainLengthInt;

							moveIn3D ('x', timber.radAng - DegreeToRad (180.0), (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
							elemList_Back.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", 0.050), "w_h", APIParT_Length, format_string ("%f", 0.040), "w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));

							remainLengthInt -= 3600;
						}
					// 여백에 투바이(80*50) 배치
					} else if ((plywoodMarginExtra >= 0.050 - EPS) && (plywoodMarginExtra <= 0.070 + EPS)) {
						while (remainLengthInt > 0) {
							if (remainLengthInt >= 3600)
								lengthInt = 3600;
							else
								lengthInt = remainLengthInt;

							moveIn3D ('x', timber.radAng - DegreeToRad (180.0), (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
							elemList_Back.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", 0.080), "w_h", APIParT_Length, format_string ("%f", 0.050), "w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));

							remainLengthInt -= 3600;
						}

						if (abs (plywoodMarginExtra - 0.060) < EPS)	addedPlywood = 1;	// 60mm 이면 합판 1장 얹음
						if (abs (plywoodMarginExtra - 0.070) < EPS)	addedPlywood = 2;	// 70mm 이면 합판 2장 얹음
						moveZ = 0.050;

						remainLengthInt = remainLengthIntStored;

						while (remainLengthInt > 0) {
							if (remainLengthInt >= 2400)
								lengthInt = 2400;
							else
								lengthInt = remainLengthInt;
						
							if (addedPlywood >= 1) {
								moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
								moveIn3D ('z', plywood1.radAng, moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
								moveIn3D ('x', plywood1.radAng, -(double)lengthInt / 1000.0, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
								elemList_Back.Push (plywood1.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
								moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
								moveIn3D ('z', plywood1.radAng, -moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							}
							if (addedPlywood >= 2) {
								moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
								moveIn3D ('z', plywood2.radAng, moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
								moveIn3D ('x', plywood2.radAng, -(double)lengthInt / 1000.0, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
								elemList_Back.Push (plywood2.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
								moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
								moveIn3D ('z', plywood2.radAng, -moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							}

							remainLengthInt -= 2400;
						}

					// 여백에 투바이(50*80) 배치
					} else if (plywoodMarginExtra >= 0.080 - EPS) {
						while (remainLengthInt > 0) {
							if (remainLengthInt >= 3600)
								lengthInt = 3600;
							else
								lengthInt = remainLengthInt;

							moveIn3D ('x', timber.radAng - DegreeToRad (180.0), (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
							elemList_Back.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", 0.050), "w_h", APIParT_Length, format_string ("%f", 0.080), "w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));

							remainLengthInt -= 3600;

							if (abs (plywoodMarginExtra - 0.090) < EPS)	addedPlywood = 1;	// 90mm 이면 합판 1장 얹음
							if (abs (plywoodMarginExtra - 0.100) < EPS)	addedPlywood = 2;	// 100mm 이면 합판 2장 얹음
							moveZ = 0.080;

							remainLengthInt = remainLengthIntStored;

							while (remainLengthInt > 0) {
								if (remainLengthInt >= 2400)
									lengthInt = 2400;
								else
									lengthInt = remainLengthInt;
						
								if (addedPlywood >= 1) {
									moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
									moveIn3D ('z', plywood1.radAng, moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
									moveIn3D ('x', plywood1.radAng, -(double)lengthInt / 1000.0, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
									elemList_Back.Push (plywood1.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
									moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
									moveIn3D ('z', plywood1.radAng, -moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
								}
								if (addedPlywood >= 2) {
									moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
									moveIn3D ('z', plywood2.radAng, moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
									moveIn3D ('x', plywood2.radAng, -(double)lengthInt / 1000.0, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
									elemList_Back.Push (plywood2.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
									moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
									moveIn3D ('z', plywood2.radAng, -moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
								}

								remainLengthInt -= 2400;
							}
						}
					}
				}
			}
		}
	}

	// 유로폼 채우기
	EasyObjectPlacement euroform;
	bool	bStandard;

	if (placingZone->marginCellsBasic [idxCell].bFill == true) {
		// 앞면 채우기

		// 1단 유로폼
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->marginCellsBasic [idxCell].leftBottomX, placingZone->marginCellsBasic [idxCell].leftBottomY, placingZone->marginCellsBasic [idxCell].leftBottomZ, placingZone->marginCellsBasic [idxCell].ang);

		if (placingZone->marginCellsBasic [idxCell].bEuroform1 == true) {
			if (placingZone->cells [idxCell].objType == TABLEFORM) {
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
					if ( ((placingZone->cells [idxCell].tableInHor [yy] == 600) || (placingZone->cells [idxCell].tableInHor [yy] == 900) || (placingZone->cells [idxCell].tableInHor [yy] == 1200)) &&
							((abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.600) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.500) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.450) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.400) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.300) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.200) < EPS)) )
						bStandard = true;
					else
						bStandard = false;

					if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
						moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
						if (bStandard == true) {
							elemList_Front.Push (euroform.placeObject (5,
								"eu_stan_onoff", APIParT_Boolean, "1.0",
								"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->marginCellsBasic [idxCell].formWidth1 * 1000)),
								"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [yy]),
								"u_ins", APIParT_CString, "벽눕히기",
								"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						} else {
							elemList_Front.Push (euroform.placeObject (5,
								"eu_stan_onoff", APIParT_Boolean, "0.0",
								"eu_wid2", APIParT_Length, format_string ("%f", placingZone->marginCellsBasic [idxCell].formWidth1),
								"eu_hei2", APIParT_Length, format_string ("%f", (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0),
								"u_ins", APIParT_CString, "벽눕히기",
								"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						}
					}
				}
			} else {
				if ( ((placingZone->cells [idxCell].horLen == 600) || (placingZone->cells [idxCell].horLen == 900) || (placingZone->cells [idxCell].horLen == 1200)) &&
					((abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.600) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.500) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.450) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.400) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.300) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth1 - 0.200) < EPS)) )
					bStandard = true;
				else
					bStandard = false;

				if (placingZone->cells [idxCell].horLen > 0) {
					moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					if (bStandard == true) {
						elemList_Front.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->marginCellsBasic [idxCell].formWidth1 * 1000)),
							"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].horLen),
							"u_ins", APIParT_CString, "벽눕히기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
					} else {
						elemList_Front.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "0.0",
							"eu_wid2", APIParT_Length, format_string ("%f", placingZone->marginCellsBasic [idxCell].formWidth1),
							"eu_hei2", APIParT_Length, format_string ("%f", (double)placingZone->cells [idxCell].horLen / 1000.0),
							"u_ins", APIParT_CString, "벽눕히기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
					}
				}
			}
		}

		// 2단 유로폼
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->marginCellsBasic [idxCell].leftBottomX, placingZone->marginCellsBasic [idxCell].leftBottomY, placingZone->marginCellsBasic [idxCell].leftBottomZ, placingZone->marginCellsBasic [idxCell].ang);

		if (placingZone->marginCellsBasic [idxCell].bEuroform2 == true) {
			moveIn3D ('z', euroform.radAng, placingZone->marginCellsBasic [idxCell].formWidth1, &euroform.posX, &euroform.posY, &euroform.posZ);

			if (placingZone->cells [idxCell].objType == TABLEFORM) {
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
					if ( ((placingZone->cells [idxCell].tableInHor [yy] == 600) || (placingZone->cells [idxCell].tableInHor [yy] == 900) || (placingZone->cells [idxCell].tableInHor [yy] == 1200)) &&
							((abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.600) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.500) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.450) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.400) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.300) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.200) < EPS)) )
						bStandard = true;
					else
						bStandard = false;

					if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
						moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
						if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
							if (bStandard == true) {
								elemList_Front.Push (euroform.placeObject (5,
									"eu_stan_onoff", APIParT_Boolean, "1.0",
									"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->marginCellsBasic [idxCell].formWidth2 * 1000)),
									"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [yy]),
									"u_ins", APIParT_CString, "벽눕히기",
									"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
							} else {
								elemList_Front.Push (euroform.placeObject (5,
									"eu_stan_onoff", APIParT_Boolean, "0.0",
									"eu_wid2", APIParT_Length, format_string ("%f", placingZone->marginCellsBasic [idxCell].formWidth2),
									"eu_hei2", APIParT_Length, format_string ("%f", (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0),
									"u_ins", APIParT_CString, "벽눕히기",
									"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
							}
						}
					}
				}
			} else {
				if ( ((placingZone->cells [idxCell].horLen == 600) || (placingZone->cells [idxCell].horLen == 900) || (placingZone->cells [idxCell].horLen == 1200)) &&
					((abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.600) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.500) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.450) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.400) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.300) < EPS) || (abs (placingZone->marginCellsBasic [idxCell].formWidth2 - 0.200) < EPS)) )
					bStandard = true;
				else
					bStandard = false;

				if (placingZone->cells [idxCell].horLen > 0) {
					moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					if (bStandard == true) {
						elemList_Front.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->marginCellsBasic [idxCell].formWidth2 * 1000)),
							"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].horLen),
							"u_ins", APIParT_CString, "벽눕히기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
					} else {
						elemList_Front.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "0.0",
							"eu_wid2", APIParT_Length, format_string ("%f", placingZone->marginCellsBasic [idxCell].formWidth2),
							"eu_hei2", APIParT_Length, format_string ("%f", (double)placingZone->cells [idxCell].horLen / 1000.0),
							"u_ins", APIParT_CString, "벽눕히기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
					}
				}
			}
		}
	}

	if (placingZone->marginCellsExtra [idxCell].bFill == true) {
		// 뒷면 채우기
		if (placingZone->bSingleSide == false) {
			// 1단 유로폼
			euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->marginCellsExtra [idxCell].leftBottomX, placingZone->marginCellsExtra [idxCell].leftBottomY, placingZone->marginCellsExtra [idxCell].leftBottomZ, placingZone->marginCellsExtra [idxCell].ang + DegreeToRad (180.0));
			moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &euroform.posX, &euroform.posY, &euroform.posZ);

			if (placingZone->marginCellsExtra [idxCell].bEuroform1 == true) {
				if (placingZone->cells [idxCell].objType == TABLEFORM) {
					for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
						if ( ((placingZone->cells [idxCell].tableInHor [yy] == 600) || (placingZone->cells [idxCell].tableInHor [yy] == 900) || (placingZone->cells [idxCell].tableInHor [yy] == 1200)) &&
								((abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.600) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.500) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.450) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.400) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.300) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.200) < EPS)) )
							bStandard = true;
						else
							bStandard = false;

						if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
							if (bStandard == true) {
								elemList_Back.Push (euroform.placeObject (5,
									"eu_stan_onoff", APIParT_Boolean, "1.0",
									"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->marginCellsExtra [idxCell].formWidth1 * 1000)),
									"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [yy]),
									"u_ins", APIParT_CString, "벽눕히기",
									"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
							} else {
								elemList_Back.Push (euroform.placeObject (5,
									"eu_stan_onoff", APIParT_Boolean, "0.0",
									"eu_wid2", APIParT_Length, format_string ("%f", placingZone->marginCellsExtra [idxCell].formWidth1),
									"eu_hei2", APIParT_Length, format_string ("%f", (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0),
									"u_ins", APIParT_CString, "벽눕히기",
									"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
							}
							moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
						}
					}
				} else {
					if ( ((placingZone->cells [idxCell].horLen == 600) || (placingZone->cells [idxCell].horLen == 900) || (placingZone->cells [idxCell].horLen == 1200)) &&
						((abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.600) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.500) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.450) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.400) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.300) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth1 - 0.200) < EPS)) )
						bStandard = true;
					else
						bStandard = false;

					if (placingZone->cells [idxCell].horLen > 0) {
						if (bStandard == true) {
							elemList_Back.Push (euroform.placeObject (5,
								"eu_stan_onoff", APIParT_Boolean, "1.0",
								"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->marginCellsExtra [idxCell].formWidth1 * 1000)),
								"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].horLen),
								"u_ins", APIParT_CString, "벽눕히기",
								"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						} else {
							elemList_Back.Push (euroform.placeObject (5,
								"eu_stan_onoff", APIParT_Boolean, "0.0",
								"eu_wid2", APIParT_Length, format_string ("%f", placingZone->marginCellsExtra [idxCell].formWidth1),
								"eu_hei2", APIParT_Length, format_string ("%f", (double)placingZone->cells [idxCell].horLen / 1000.0),
								"u_ins", APIParT_CString, "벽눕히기",
								"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						}
						moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					}
				}
			}

			// 2단 유로폼
			euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->marginCellsExtra [idxCell].leftBottomX, placingZone->marginCellsExtra [idxCell].leftBottomY, placingZone->marginCellsExtra [idxCell].leftBottomZ, placingZone->marginCellsExtra [idxCell].ang + DegreeToRad (180.0));
			moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &euroform.posX, &euroform.posY, &euroform.posZ);

			if (placingZone->marginCellsExtra [idxCell].bEuroform2 == true) {
				moveIn3D ('z', euroform.radAng, placingZone->marginCellsExtra [idxCell].formWidth1, &euroform.posX, &euroform.posY, &euroform.posZ);

				if (placingZone->cells [idxCell].objType == TABLEFORM) {
					for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
						if ( ((placingZone->cells [idxCell].tableInHor [yy] == 600) || (placingZone->cells [idxCell].tableInHor [yy] == 900) || (placingZone->cells [idxCell].tableInHor [yy] == 1200)) &&
								((abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.600) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.500) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.450) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.400) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.300) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.200) < EPS)) )
							bStandard = true;
						else
							bStandard = false;

						if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
							if (bStandard == true) {
								elemList_Back.Push (euroform.placeObject (5,
									"eu_stan_onoff", APIParT_Boolean, "1.0",
									"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->marginCellsExtra [idxCell].formWidth2 * 1000)),
									"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [yy]),
									"u_ins", APIParT_CString, "벽눕히기",
									"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
							} else {
								elemList_Back.Push (euroform.placeObject (5,
									"eu_stan_onoff", APIParT_Boolean, "0.0",
									"eu_wid2", APIParT_Length, format_string ("%f", placingZone->marginCellsExtra [idxCell].formWidth2),
									"eu_hei2", APIParT_Length, format_string ("%f", (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0),
									"u_ins", APIParT_CString, "벽눕히기",
									"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
							}
							moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
						}
					}
				} else {
					if ( ((placingZone->cells [idxCell].horLen == 600) || (placingZone->cells [idxCell].horLen == 900) || (placingZone->cells [idxCell].horLen == 1200)) &&
						((abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.600) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.500) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.450) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.400) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.300) < EPS) || (abs (placingZone->marginCellsExtra [idxCell].formWidth2 - 0.200) < EPS)) )
						bStandard = true;
					else
						bStandard = false;

					if (placingZone->cells [idxCell].horLen > 0) {
						if (bStandard == true) {
							elemList_Back.Push (euroform.placeObject (5,
								"eu_stan_onoff", APIParT_Boolean, "1.0",
								"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->marginCellsExtra [idxCell].formWidth2 * 1000)),
								"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].horLen),
								"u_ins", APIParT_CString, "벽눕히기",
								"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						} else {
							elemList_Back.Push (euroform.placeObject (5,
								"eu_stan_onoff", APIParT_Boolean, "0.0",
								"eu_wid2", APIParT_Length, format_string ("%f", placingZone->marginCellsExtra [idxCell].formWidth2),
								"eu_hei2", APIParT_Length, format_string ("%f", (double)placingZone->cells [idxCell].horLen / 1000.0),
								"u_ins", APIParT_CString, "벽눕히기",
								"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						}
						moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					}
				}
			}
		}
	}

	// 결과물 전체 그룹화 (앞면)
	groupElements (elemList_Front);
	elemList_Front.Clear ();

	// 결과물 전체 그룹화 (뒷면)
	groupElements (elemList_Back);
	elemList_Back.Clear ();

	return err;
}

// 테이블폼 내 유로폼 배치 (공통)
void	WallTableformPlacingZone::placeEuroformsOfTableform (WallTableformPlacingZone* placingZone, short idxCell)
{
	short	xx, yy, varEnd;
	double	accumDist;
	int		lengthInt;

	EasyObjectPlacement euroform;

	if (placingZone->cells [idxCell].objType == TABLEFORM) {
		if (placingZone->bVertical == true) {
			// 세로방향
			// 앞면
			euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				accumDist = 0.0;
				for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
					if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->cells [idxCell].tableInVerBasic [yy] > 0)) {
						elemList_Front.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
							"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInVerBasic [yy]),
							"u_ins", APIParT_CString, "벽세우기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						accumDist += (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0;
						moveIn3D ('z', euroform.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					}
				}
				moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}

			// 뒷면
			if (placingZone->bSingleSide == false) {
				euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
				moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &euroform.posX, &euroform.posY, &euroform.posZ);		// 벽과의 간격만큼 이동

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
								"u_ins", APIParT_CString, "벽세우기",
								"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
							moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);

							accumDist += (double)lengthInt / 1000.0;
							moveIn3D ('z', euroform.radAng, (double)lengthInt / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
						}
					}
					moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
					moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
				}
			}
		} else {
			// 가로방향
			// 앞면
			euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				accumDist = 0.0;
				for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
					if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->cells [idxCell].tableInVerBasic [yy] > 0)) {
						moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
						elemList_Front.Push (euroform.placeObject (5,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInVerBasic [yy]),
							"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
							"u_ins", APIParT_CString, "벽눕히기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
						moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);

						accumDist += (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0;
						moveIn3D ('z', euroform.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					}
				}
				moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}

			// 뒷면
			if (placingZone->bSingleSide == false) {
				euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
				moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &euroform.posX, &euroform.posY, &euroform.posZ);		// 벽과의 간격만큼 이동

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
								"u_ins", APIParT_CString, "벽눕히기",
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
}

// 테이블폼 타입A 배치 (유로폼 제외) - 각파이프 2줄
void	WallTableformPlacingZone::placeTableformA (WallTableformPlacingZone* placingZone, short idxCell)
{
	short	xx, yy, varEnd;
	int		pipeLength;
	int		headpieceUpPosZ;
	double	sideMargin;
	int*	intPointer;
	int		backHeight;
	
	int		firstWidth, lastWidth;
	bool	bFoundFirstWidth;
	short	realWidthCount, count;

	int		topHeight, bottomHeight;
	bool	bFoundBottomHeight;
	short	realHeightCount;

	if (placingZone->bExtra == true) {
		varEnd = placingZone->nCellsInVerExtra;
		intPointer = placingZone->cells [idxCell].tableInVerExtra;
		backHeight = placingZone->cells [idxCell].verLenExtra;
	} else {
		varEnd = placingZone->nCellsInVerBasic;
		intPointer = placingZone->cells [idxCell].tableInVerBasic;
		backHeight = placingZone->cells [idxCell].verLenBasic;
	}

	// 테이블폼 내 1번째와 마지막 유로폼의 각각의 너비를 가져옴 (길이가 0인 유로폼은 통과) - 세로방향
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

	if (placingZone->cells [idxCell].objType != TABLEFORM)
		return;

	if (placingZone->cells [idxCell].horLen == 0)
		return;

	if (placingZone->bVertical == true) {
		// ================================================== 세로방향
		// 수평 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].horLen - 50;
			sideMargin = 0.025;
		}

		EasyObjectPlacement rectPipe;
		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.031 - 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {								// 중간
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

				moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			}
		}
		moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 상부
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// 수평 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, -0.031 - 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			for (xx = 0 ; xx < varEnd - 1 ; ++xx) {														// 중간
				if (intPointer [xx] > 0) {
					moveIn3D ('z', rectPipe.radAng, (double)intPointer [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

					moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
					elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
					elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				}
			}
			moveIn3D ('z', rectPipe.radAng, (double)intPointer [varEnd - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 상부
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		}

		// 수직 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
			sideMargin = 0.025;
		}

		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, (double)firstWidth / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 왼쪽
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)firstWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 오른쪽
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));

		// 테이블폼 너비가 2300을 초과할 경우
		if (placingZone->cells [idxCell].horLen > 2300) {
			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);
			for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
				moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Front_Add.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Front_Add.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		}

		// 수직 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			if (backHeight % 100 == 0) {
				pipeLength = backHeight - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = backHeight - 50;
				sideMargin = 0.025;
			}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 왼쪽
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)lastWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - firstWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 오른쪽
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));

			// 테이블폼 너비가 2300을 초과할 경우
			if (placingZone->cells [idxCell].horLen > 2300) {
				rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
				for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
					moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

				moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Back_Add.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Back_Add.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
			}
		}

		// 핀볼트 세트 - 앞면
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 하부
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {										// 중간
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

				count = realWidthCount;
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
					if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
						if (count > 0) {
							if (count == realWidthCount) {
								// 좌측
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
									moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else if (count == 1) {
								// 우측
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else {
								// 나머지
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							}

							--count;
						}
					}
				}
				moveIn3D ('x', pinbolt.radAng, -(double)placingZone->cells [idxCell].horLen / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].verLenBasic / 1000.0 - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 상부
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		// 테이블폼 너비가 2300을 초과할 경우
		if (placingZone->cells [idxCell].horLen > 2300) {
			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			moveIn3D ('y', pinbolt.radAng, -0.1635 - 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {
				if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
					moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					elemList_Front_Add.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
				}
			}
		}

		// 핀볼트 세트 - 뒷면
		if (placingZone->bSingleSide == false) {
			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realWidthCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 하부
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			for (xx = 0 ; xx < varEnd - 1 ; ++xx) {																// 중간
				if (intPointer [xx] > 0) {
					moveIn3D ('z', pinbolt.radAng, (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

					count = realWidthCount;
					for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
						if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
							if (count > 0) {
								if (count == realWidthCount) {
									// 좌측
									if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								} else if (count == 1) {
									// 우측
									if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								} else {
									// 나머지
									if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								}

								--count;
							}
						}
					}
					moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), -(double)placingZone->cells [idxCell].horLen / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)backHeight / 1000.0 - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realWidthCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 상부
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}

			// 테이블폼 너비가 2300을 초과할 경우
			if (placingZone->cells [idxCell].horLen > 2300) {
				pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

				for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
					moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

				moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.1635 + 0.050), &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

				for (xx = 0 ; xx < varEnd - 1 ; ++xx) {
					if (intPointer [xx] > 0) {
						moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
						elemList_Back_Add.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
					}
				}
			}
		}
		
		// 결합철물 - 앞면
		EasyObjectPlacement join;
		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('z', join.radAng, 0.450, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('z', join.radAng, (double)placingZone->cells [idxCell].verLenBasic / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		
		// 테이블폼 너비가 2300을 초과할 경우
		if (placingZone->cells [idxCell].horLen > 2300) {
			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);
			moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('z', join.radAng, 0.450, &join.posX, &join.posY, &join.posZ);

			for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
				moveIn3D ('x', join.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Front_Add.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			
			moveIn3D ('z', join.radAng, -0.450, &join.posX, &join.posY, &join.posZ);

			for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx)
				if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0)
					moveIn3D ('z', join.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &join.posX, &join.posY, &join.posZ);

			moveIn3D ('z', join.radAng, -0.150, &join.posX, &join.posY, &join.posZ);
			elemList_Front_Add.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}

		// 결합철물 - 뒷면
		if (placingZone->bSingleSide == false) {
			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('z', join.radAng - DegreeToRad (180.0), 0.450, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', join.radAng, -(double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)backHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', join.radAng, -(double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			// 테이블폼 너비가 2300을 초과할 경우
			if (placingZone->cells [idxCell].horLen > 2300) {
				join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
				moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
				moveIn3D ('z', join.radAng - DegreeToRad (180.0), 0.450, &join.posX, &join.posY, &join.posZ);

				for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
					moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &join.posX, &join.posY, &join.posZ);
				elemList_Back_Add.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			
				moveIn3D ('z', join.radAng - DegreeToRad (180.0), -0.450, &join.posX, &join.posY, &join.posZ);

				for (xx = 0 ; xx < varEnd ; ++xx)
					if (intPointer [xx] > 0)
						moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &join.posX, &join.posY, &join.posZ);

				moveIn3D ('z', join.radAng - DegreeToRad (180.0), -0.150, &join.posX, &join.posY, &join.posZ);
				elemList_Back_Add.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			}
		}

		// 헤드피스 - 앞면
		EasyObjectPlacement headpiece;
		headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.200, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150 - 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 - 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		if (placingZone->cells [idxCell].verLenBasic >= 5300) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 4600) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 3500) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 3000) {
			headpieceUpPosZ = 2200;
		} else if (placingZone->cells [idxCell].verLenBasic >= 2500) {
			headpieceUpPosZ = 1900;
		} else if (placingZone->cells [idxCell].verLenBasic >= 2000) {
			headpieceUpPosZ = 1500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 1500) {
			headpieceUpPosZ = 1100;
		} else if (placingZone->cells [idxCell].verLenBasic >= 1000) {
			headpieceUpPosZ = 800;
		} else {
			headpieceUpPosZ = 150;
		}

		moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150 - 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 - 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// 테이블폼 너비가 2300을 초과할 경우
		if (placingZone->cells [idxCell].horLen > 2300) {
			headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);
			for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
				moveIn3D ('x', headpiece.radAng, (double)(placingZone->cells [idxCell].tableInHor [xx]) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			moveIn3D ('x', headpiece.radAng, -0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('z', headpiece.radAng, 0.200, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Front_Add.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', headpiece.radAng, -0.200 + (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Front_Add.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}

		// Push-Pull Props - 앞면
		EasyObjectPlacement props;
		double	lenV, lenH;
		double	mainLength, mainRadAng;
		double	auxLength, auxRadAng;
		char	mainNom [16], auxNom [16];

		if ((placingZone->propsInstallType == 2) || (placingZone->propsInstallType == 4)) {
			lenH = 1.200 + 0.030 - 0.036;	// 1200: 베이스 플레이트와 헤드피스 플레이트와의 거리, 30: 보조 지지대와 베이스 플레이트 앞쪽과의 거리, 36: 헤드피스 플레이트와 헤드피스 하부 걸쇠와의 거리
			lenV = 0.200 + 0.060 - 0.056;	// 200: 하부 헤드피스 플레이트의 높이, 60: 하부 헤드피스 플레이트와 하부 걸쇠와의 고도차, 56: 베이스 플레이트와 지지대 간의 고도차
			auxLength = sqrt ( (lenH * lenH) + (lenV * lenV) );
			auxRadAng = atan2 (lenV, lenH);

			lenH = 1.200 + 0.090 - 0.036;	// 1200: 베이스 플레이트와 헤드피스 플레이트와의 거리, 90: 메인 지지대와 베이스 플레이트 앞쪽과의 거리, 36: 헤드피스 플레이트와 헤드피스 하부 걸쇠와의 거리
			lenV = (double)headpieceUpPosZ / 1000.0 + 0.060 - 0.056;	// headpieceUpPosZ: 상부 헤드피스 플레이트의 높이, 60: 상부 헤드피스 플레이트와 하부 걸쇠와의 고도차, 56: 베이스 플레이트와 지지대 간의 고도차
			mainLength = sqrt ( (lenH * lenH) + (lenV * lenV) );
			mainRadAng = atan2 (lenV, lenH);

			if (mainLength >= 0.700 && mainLength < 1.100)			strcpy (mainNom, "700~1100");
			else if (mainLength >= 1.100 && mainLength < 1.700)		strcpy (mainNom, "1100~1800");
			else if (mainLength >= 1.700 && mainLength < 2.400)		strcpy (mainNom, "1700~2500");
			else if (mainLength >= 2.400 && mainLength < 3.100)		strcpy (mainNom, "2400~3200");
			else if (mainLength >= 3.100 && mainLength < 3.800)		strcpy (mainNom, "3100~3900");
			else if (mainLength >= 3.800 && mainLength <= 4.600)	strcpy (mainNom, "3800~4600");

			if (auxLength >= 0.700 && auxLength < 1.100)			strcpy (auxNom, "700~1100");
			else if (auxLength >= 1.100 && auxLength < 1.700)		strcpy (auxNom, "1100~1800");
			else if (auxLength >= 1.700 && auxLength < 2.400)		strcpy (auxNom, "1700~2500");
			else if (auxLength >= 2.400 && auxLength < 3.100)		strcpy (auxNom, "2400~3200");
			else if (auxLength >= 3.100 && auxLength < 3.800)		strcpy (auxNom, "3100~3900");
			else if (auxLength >= 3.800 && auxLength <= 4.600)		strcpy (auxNom, "3800~4600");

			props.init (L("Push-Pull Props v1.0 (당사제작품).gsm"), layerInd_Props, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('x', props.radAng, (double)(firstWidth - 150 + 80) / 1000.0, &props.posX, &props.posY, &props.posZ);
			moveIn3D ('y', props.radAng, -(0.2525 + 0.080 + 1.200), &props.posX, &props.posY, &props.posZ);
				props.radAng += DegreeToRad (90.0);
				elemList_Front.Push (props.placeObject (14, "bBasePlate", APIParT_Boolean, "1.0", "typeBasePlate", APIParT_CString, "당사 제작품", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "mainType", APIParT_CString, mainNom, "bMainQuickConnect", APIParT_Boolean, "0.0", "mainLength", APIParT_Length, format_string ("%f", mainLength), "mainAng", APIParT_Angle, format_string ("%f", mainRadAng), "bAuxSupport", APIParT_Boolean, "1.0", "auxType", APIParT_CString, auxNom, "bAuxQuickConnect", APIParT_Boolean, "0.0", "auxLength", APIParT_Length, format_string ("%f", auxLength), "auxAng", APIParT_Angle, format_string ("%f", auxRadAng), "bShowHotspotsOnBase", APIParT_Boolean, "0.0"));
				props.radAng -= DegreeToRad (90.0);
			moveIn3D ('x', props.radAng, (double)(-(firstWidth - 150 + 80) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 + 80)) / 1000.0, &props.posX, &props.posY, &props.posZ);
				props.radAng += DegreeToRad (90.0);
				elemList_Front.Push (props.placeObject (14, "bBasePlate", APIParT_Boolean, "1.0", "typeBasePlate", APIParT_CString, "당사 제작품", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "mainType", APIParT_CString, mainNom, "bMainQuickConnect", APIParT_Boolean, "0.0", "mainLength", APIParT_Length, format_string ("%f", mainLength), "mainAng", APIParT_Angle, format_string ("%f", mainRadAng), "bAuxSupport", APIParT_Boolean, "1.0", "auxType", APIParT_CString, auxNom, "bAuxQuickConnect", APIParT_Boolean, "0.0", "auxLength", APIParT_Length, format_string ("%f", auxLength), "auxAng", APIParT_Angle, format_string ("%f", auxRadAng), "bShowHotspotsOnBase", APIParT_Boolean, "0.0"));
				props.radAng -= DegreeToRad (90.0);

			// 테이블폼 너비가 2300을 초과할 경우
			if (placingZone->cells [idxCell].horLen > 2300) {
				props.init (L("Push-Pull Props v1.0 (당사제작품).gsm"), layerInd_Props, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);
				for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
					moveIn3D ('x', props.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &props.posX, &props.posY, &props.posZ);

				moveIn3D ('x', props.radAng, 0.080, &props.posX, &props.posY, &props.posZ);
				moveIn3D ('y', props.radAng, -(0.2525 + 0.080 + 1.200), &props.posX, &props.posY, &props.posZ);
					props.radAng += DegreeToRad (90.0);
					elemList_Front_Add.Push (props.placeObject (14, "bBasePlate", APIParT_Boolean, "1.0", "typeBasePlate", APIParT_CString, "당사 제작품", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "mainType", APIParT_CString, mainNom, "bMainQuickConnect", APIParT_Boolean, "0.0", "mainLength", APIParT_Length, format_string ("%f", mainLength), "mainAng", APIParT_Angle, format_string ("%f", mainRadAng), "bAuxSupport", APIParT_Boolean, "1.0", "auxType", APIParT_CString, auxNom, "bAuxQuickConnect", APIParT_Boolean, "0.0", "auxLength", APIParT_Length, format_string ("%f", auxLength), "auxAng", APIParT_Angle, format_string ("%f", auxRadAng), "bShowHotspotsOnBase", APIParT_Boolean, "0.0"));
					props.radAng -= DegreeToRad (90.0);
			}
		}

		// 헤드피스 - 뒷면
		if (placingZone->bSingleSide == false) {
			headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(firstWidth - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), 0.200, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(-(firstWidth - 150 + 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 + 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			if (backHeight >= 5300) {
				headpieceUpPosZ = 2500;
			} else if (backHeight >= 4600) {
				headpieceUpPosZ = 2500;
			} else if (backHeight >= 3500) {
				headpieceUpPosZ = 2500;
			} else if (backHeight >= 3000) {
				headpieceUpPosZ = 2200;
			} else if (backHeight >= 2500) {
				headpieceUpPosZ = 1900;
			} else if (backHeight >= 2000) {
				headpieceUpPosZ = 1500;
			} else if (backHeight >= 1500) {
				headpieceUpPosZ = 1100;
			} else if (backHeight >= 1000) {
				headpieceUpPosZ = 800;
			} else {
				headpieceUpPosZ = 150;
			}

			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(firstWidth - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(-(firstWidth - 150 + 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 + 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			// 테이블폼 너비가 2300을 초과할 경우
			if (placingZone->cells [idxCell].horLen > 2300) {
				headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
				for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
					moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(placingZone->cells [idxCell].tableInHor [xx]) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

				moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), 0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
				moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
				moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), 0.200, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

				elemList_Back_Add.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
				moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), -0.200 + (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
				elemList_Back_Add.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			}
		}

		// Push-Pull Props - 뒷면
		if ((placingZone->propsInstallType == 3) || (placingZone->propsInstallType == 4)) {
			lenH = 1.200 + 0.030 - 0.036;	// 1200: 베이스 플레이트와 헤드피스 플레이트와의 거리, 30: 보조 지지대와 베이스 플레이트 앞쪽과의 거리, 36: 헤드피스 플레이트와 헤드피스 하부 걸쇠와의 거리
			lenV = 0.200 + 0.060 - 0.056;	// 200: 하부 헤드피스 플레이트의 높이, 60: 하부 헤드피스 플레이트와 하부 걸쇠와의 고도차, 56: 베이스 플레이트와 지지대 간의 고도차
			auxLength = sqrt ( (lenH * lenH) + (lenV * lenV) );
			auxRadAng = atan2 (lenV, lenH);

			lenH = 1.200 + 0.090 - 0.036;	// 1200: 베이스 플레이트와 헤드피스 플레이트와의 거리, 90: 메인 지지대와 베이스 플레이트 앞쪽과의 거리, 36: 헤드피스 플레이트와 헤드피스 하부 걸쇠와의 거리
			lenV = (double)headpieceUpPosZ / 1000.0 + 0.060 - 0.056;	// headpieceUpPosZ: 상부 헤드피스 플레이트의 높이, 60: 상부 헤드피스 플레이트와 하부 걸쇠와의 고도차, 56: 베이스 플레이트와 지지대 간의 고도차
			mainLength = sqrt ( (lenH * lenH) + (lenV * lenV) );
			mainRadAng = atan2 (lenV, lenH);

			if (mainLength >= 0.700 && mainLength < 1.100)			strcpy (mainNom, "700~1100");
			else if (mainLength >= 1.100 && mainLength < 1.700)		strcpy (mainNom, "1100~1800");
			else if (mainLength >= 1.700 && mainLength < 2.400)		strcpy (mainNom, "1700~2500");
			else if (mainLength >= 2.400 && mainLength < 3.100)		strcpy (mainNom, "2400~3200");
			else if (mainLength >= 3.100 && mainLength < 3.800)		strcpy (mainNom, "3100~3900");
			else if (mainLength >= 3.800 && mainLength <= 4.600)	strcpy (mainNom, "3800~4600");

			if (auxLength >= 0.700 && auxLength < 1.100)			strcpy (auxNom, "700~1100");
			else if (auxLength >= 1.100 && auxLength < 1.700)		strcpy (auxNom, "1100~1800");
			else if (auxLength >= 1.700 && auxLength < 2.400)		strcpy (auxNom, "1700~2500");
			else if (auxLength >= 2.400 && auxLength < 3.100)		strcpy (auxNom, "2400~3200");
			else if (auxLength >= 3.100 && auxLength < 3.800)		strcpy (auxNom, "3100~3900");
			else if (auxLength >= 3.800 && auxLength <= 4.600)		strcpy (auxNom, "3800~4600");

			props.init (L("Push-Pull Props v1.0 (당사제작품).gsm"), layerInd_Props, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', props.radAng - DegreeToRad (180.0), (double)(firstWidth - 150 - 80) / 1000.0, &props.posX, &props.posY, &props.posZ);
			moveIn3D ('y', props.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.2525 + 0.080 + 1.200), &props.posX, &props.posY, &props.posZ);
				props.radAng += DegreeToRad (90.0);
				elemList_Back.Push (props.placeObject (14, "bBasePlate", APIParT_Boolean, "1.0", "typeBasePlate", APIParT_CString, "당사 제작품", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "mainType", APIParT_CString, mainNom, "bMainQuickConnect", APIParT_Boolean, "0.0", "mainLength", APIParT_Length, format_string ("%f", mainLength), "mainAng", APIParT_Angle, format_string ("%f", mainRadAng), "bAuxSupport", APIParT_Boolean, "1.0", "auxType", APIParT_CString, auxNom, "bAuxQuickConnect", APIParT_Boolean, "0.0", "auxLength", APIParT_Length, format_string ("%f", auxLength), "auxAng", APIParT_Angle, format_string ("%f", auxRadAng), "bShowHotspotsOnBase", APIParT_Boolean, "0.0"));
				props.radAng -= DegreeToRad (90.0);
			moveIn3D ('x', props.radAng - DegreeToRad (180.0), (double)(-(firstWidth - 150 - 80) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 - 80)) / 1000.0, &props.posX, &props.posY, &props.posZ);
				props.radAng += DegreeToRad (90.0);
				elemList_Back.Push (props.placeObject (14, "bBasePlate", APIParT_Boolean, "1.0", "typeBasePlate", APIParT_CString, "당사 제작품", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "mainType", APIParT_CString, mainNom, "bMainQuickConnect", APIParT_Boolean, "0.0", "mainLength", APIParT_Length, format_string ("%f", mainLength), "mainAng", APIParT_Angle, format_string ("%f", mainRadAng), "bAuxSupport", APIParT_Boolean, "1.0", "auxType", APIParT_CString, auxNom, "bAuxQuickConnect", APIParT_Boolean, "0.0", "auxLength", APIParT_Length, format_string ("%f", auxLength), "auxAng", APIParT_Angle, format_string ("%f", auxRadAng), "bShowHotspotsOnBase", APIParT_Boolean, "0.0"));
				props.radAng -= DegreeToRad (90.0);

			// 테이블폼 너비가 2300을 초과할 경우
			if (placingZone->cells [idxCell].horLen > 2300) {
				props.init (L("Push-Pull Props v1.0 (당사제작품).gsm"), layerInd_Props, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
				for (xx = 0 ; xx < (short)(realWidthCount / 2) ; ++xx)
					moveIn3D ('x', props.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &props.posX, &props.posY, &props.posZ);

				moveIn3D ('x', props.radAng - DegreeToRad (180.0), -0.080, &props.posX, &props.posY, &props.posZ);
				moveIn3D ('y', props.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.2525 + 0.080 + 1.200), &props.posX, &props.posY, &props.posZ);
					props.radAng += DegreeToRad (90.0);
					elemList_Back_Add.Push (props.placeObject (14, "bBasePlate", APIParT_Boolean, "1.0", "typeBasePlate", APIParT_CString, "당사 제작품", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "mainType", APIParT_CString, mainNom, "bMainQuickConnect", APIParT_Boolean, "0.0", "mainLength", APIParT_Length, format_string ("%f", mainLength), "mainAng", APIParT_Angle, format_string ("%f", mainRadAng), "bAuxSupport", APIParT_Boolean, "1.0", "auxType", APIParT_CString, auxNom, "bAuxQuickConnect", APIParT_Boolean, "0.0", "auxLength", APIParT_Length, format_string ("%f", auxLength), "auxAng", APIParT_Angle, format_string ("%f", auxRadAng), "bShowHotspotsOnBase", APIParT_Boolean, "0.0"));
					props.radAng -= DegreeToRad (90.0);
			}
		}
	} else {
		// ================================================== 가로방향 (세로방향이 오른쪽으로 90도 누웠다고 생각하면 됨)
		// 수평 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
			sideMargin = 0.025;
		}

		EasyObjectPlacement rectPipe;
		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// 중간
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

				moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			}
		}
		moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 상부
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// 수평 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			if (backHeight % 100 == 0) {
				pipeLength = backHeight - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = backHeight - 50;
				sideMargin = 0.025;
			}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// 중간
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

					moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
					elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
					elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
					moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				}
			}
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 상부
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		}

		// 수직 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].horLen - 50;
			sideMargin = 0.025;
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
				++realHeightCount;
			}
		}

		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', rectPipe.radAng, (double)(placingZone->cells [idxCell].verLenBasic - topHeight) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 왼쪽
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.035 - 0.150 + (double)topHeight / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, (double)(-placingZone->cells [idxCell].verLenBasic + bottomHeight) / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 오른쪽
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 수직 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			if (placingZone->cells [idxCell].horLen % 100 == 0) {
				pipeLength = placingZone->cells [idxCell].horLen - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = placingZone->cells [idxCell].horLen - 50;
				sideMargin = 0.025;
			}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('z', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 왼쪽
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, -0.035 + 0.150 - (double)bottomHeight / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('z', rectPipe.radAng, (double)(backHeight - topHeight) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 오른쪽
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));

			// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
			realHeightCount = 0;
			bFoundBottomHeight = false;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
				if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
					if (bFoundBottomHeight == false) {
						bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
						bFoundBottomHeight = true;
					}
					topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					++realHeightCount;
				}
			}
		}

		// 핀볼트 세트 - 앞면
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realHeightCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 하부
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {															// 중간
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

				count = realHeightCount;
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++yy) {
					if (placingZone->cells [idxCell].tableInVerBasic [yy] > 0) {
						if (count > 0) {
							if (count == realHeightCount) {
								// 우측
								if (placingZone->cells [idxCell].tableInVerBasic [yy] == 600) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 500) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 450) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 400) {
									moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 300) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 200) {
									moveIn3D ('z', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else if (count == 1) {
								// 좌측
								if (placingZone->cells [idxCell].tableInVerBasic [yy] == 600) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 500) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 450) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 400) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 300) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 200) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else {
								// 나머지
								if (placingZone->cells [idxCell].tableInVerBasic [yy] == 600) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 500) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 450) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 400) {
									moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 300) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 200) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							}

							--count;
						}
					}
				}
				moveIn3D ('z', pinbolt.radAng, -(double)placingZone->cells [idxCell].verLenBasic / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realHeightCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 상부
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 핀볼트 세트 - 뒷면
		if (placingZone->bSingleSide == false) {
			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realHeightCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 하부
				if (intPointer [xx] > 0) {
					moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {															// 중간
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', pinbolt.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

					count = realHeightCount;
					for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++yy) {
						if (intPointer [yy] > 0) {
							if (count > 0) {
								if (count == realHeightCount) {
									// 우측
									if (intPointer [yy] == 600) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 500) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 450) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 400) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 300) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 200) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								} else if (count == 1) {
									// 좌측
									if (intPointer [yy] == 600) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 500) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 450) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 400) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 300) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 200) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								} else {
									// 나머지
									if (intPointer [yy] == 600) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 500) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 450) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 400) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 300) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 200) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								}

								--count;
							}
						}
					}
					moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), -(double)backHeight / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realHeightCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 상부
				if (intPointer [xx] > 0) {
					moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}
		}
		
		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
				++realHeightCount;
			}
		}

		// 결합철물 - 앞면
		EasyObjectPlacement join;
		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', join.radAng, (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('x', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + placingZone->cells [idxCell].verLenBasic - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', join.radAng, (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('x', join.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + placingZone->cells [idxCell].verLenBasic - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		
		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 결합철물 - 뒷면
		if (placingZone->bSingleSide == false) {
			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('x', join.radAng - DegreeToRad (180.0), 0.150, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + backHeight - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + backHeight - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
				++realHeightCount;
			}
		}

		// 헤드피스 - 앞면
		EasyObjectPlacement headpiece;
		headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', headpiece.radAng, (double)(bottomHeight - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
		moveIn3D ('z', headpiece.radAng, (double)(-(bottomHeight - 150) + placingZone->cells [idxCell].verLenBasic + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

		headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		if (placingZone->cells [idxCell].horLen >= 5300) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].horLen >= 4600) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].horLen >= 3500) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].horLen >= 3000) {
			headpieceUpPosZ = 2200;
		} else if (placingZone->cells [idxCell].horLen >= 2500) {
			headpieceUpPosZ = 1900;
		} else if (placingZone->cells [idxCell].horLen >= 2000) {
			headpieceUpPosZ = 1500;
		} else if (placingZone->cells [idxCell].horLen >= 1500) {
			headpieceUpPosZ = 1100;
		} else if (placingZone->cells [idxCell].horLen >= 1000) {
			headpieceUpPosZ = 800;
		} else {
			headpieceUpPosZ = 150;
		}

		moveIn3D ('z', headpiece.radAng, (double)(bottomHeight - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, (double)(headpieceUpPosZ) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
		moveIn3D ('z', headpiece.radAng, (double)(-(bottomHeight - 150) + placingZone->cells [idxCell].verLenBasic + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 헤드피스 - 뒷면
		if (placingZone->bSingleSide == false) {
			headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(bottomHeight - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), 0.300 + 0.200, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(-(bottomHeight - 150) + backHeight + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

			headpiece.init (L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			if (placingZone->cells [idxCell].horLen >= 5300) {
				headpieceUpPosZ = 2500;
			} else if (placingZone->cells [idxCell].horLen >= 4600) {
				headpieceUpPosZ = 2500;
			} else if (placingZone->cells [idxCell].horLen >= 3500) {
				headpieceUpPosZ = 2500;
			} else if (placingZone->cells [idxCell].horLen >= 3000) {
				headpieceUpPosZ = 2200;
			} else if (placingZone->cells [idxCell].horLen >= 2500) {
				headpieceUpPosZ = 1900;
			} else if (placingZone->cells [idxCell].horLen >= 2000) {
				headpieceUpPosZ = 1500;
			} else if (placingZone->cells [idxCell].horLen >= 1500) {
				headpieceUpPosZ = 1100;
			} else if (placingZone->cells [idxCell].horLen >= 1000) {
				headpieceUpPosZ = 800;
			} else {
				headpieceUpPosZ = 150;
			}

			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(bottomHeight - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(headpieceUpPosZ + 200) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(-(bottomHeight - 150) + backHeight + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
		}
	}

	// 결과물 전체 그룹화 (앞면)
	groupElements (elemList_Front);
	elemList_Front.Clear ();

	// 결과물 전체 그룹화 (뒷면)
	groupElements (elemList_Back);
	elemList_Back.Clear ();

	// 결과물 전체 그룹화 (앞면)
	groupElements (elemList_Front_Add);
	elemList_Front_Add.Clear ();

	// 결과물 전체 그룹화 (뒷면)
	groupElements (elemList_Back_Add);
	elemList_Back_Add.Clear ();
}

// 테이블폼 타입B 배치 (유로폼 제외) - 각파이프 1줄
void	WallTableformPlacingZone::placeTableformB (WallTableformPlacingZone* placingZone, short idxCell)
{
	short	xx, yy, varEnd;
	int		pipeLength;
	int		headpieceUpPosZ;
	double	sideMargin;
	int*	intPointer;
	int		backHeight;
	
	int		firstWidth, lastWidth;
	bool	bFoundFirstWidth;
	short	realWidthCount, count;

	int		topHeight, bottomHeight;
	bool	bFoundBottomHeight;
	short	realHeightCount;

	if (placingZone->bExtra == true) {
		varEnd = placingZone->nCellsInVerExtra;
		intPointer = placingZone->cells [idxCell].tableInVerExtra;
		backHeight = placingZone->cells [idxCell].verLenExtra;
	} else {
		varEnd = placingZone->nCellsInVerBasic;
		intPointer = placingZone->cells [idxCell].tableInVerBasic;
		backHeight = placingZone->cells [idxCell].verLenBasic;
	}

	// 테이블폼 내 1번째와 마지막 유로폼의 각각의 너비를 가져옴 (길이가 0인 유로폼은 통과) - 세로방향
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

	if (placingZone->cells [idxCell].objType != TABLEFORM)
		return;

	if (placingZone->cells [idxCell].horLen == 0)
		return;

	if (placingZone->bVertical == true) {
		// ================================================== 세로방향
		// 수평 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].horLen - 50;
			sideMargin = 0.025;
		}

		EasyObjectPlacement rectPipe;
		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		moveIn3D ('z', rectPipe.radAng, -0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {									// 중간
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

				elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			}
		}
		moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] / 1000.0 - 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 상부

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

		// 수평 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			moveIn3D ('z', rectPipe.radAng, -0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			for (xx = 0 ; xx < varEnd - 1 ; ++xx) {														// 중간
				if (intPointer [xx] > 0) {
					moveIn3D ('z', rectPipe.radAng, (double)intPointer [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

					elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				}
			}
			moveIn3D ('z', rectPipe.radAng, (double)intPointer [varEnd - 1] / 1000.0 - 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 상부

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		}

		// 수직 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
			sideMargin = 0.025;
		}

		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, (double)firstWidth / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 왼쪽
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', rectPipe.radAng, 0.150 - (double)firstWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 오른쪽
		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

		// 수직 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			if (backHeight % 100 == 0) {
				pipeLength = backHeight - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = backHeight - 50;
				sideMargin = 0.025;
			}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 왼쪽
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			moveIn3D ('x', rectPipe.radAng, 0.150 - (double)lastWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - firstWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 오른쪽
			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		}

		// 핀볼트 세트 - 앞면
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 하부
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {										// 중간
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

				count = realWidthCount;
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
					if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
						if (count > 0) {
							if (count == realWidthCount) {
								// 좌측
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
									moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else if (count == 1) {
								// 우측
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else {
								// 나머지
								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
									moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							}

							--count;
						}
					}
				}
				moveIn3D ('x', pinbolt.radAng, -(double)placingZone->cells [idxCell].horLen / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].verLenBasic / 1000.0 - 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 상부
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		// 핀볼트 세트 - 뒷면
		if (placingZone->bSingleSide == false) {
			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realWidthCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 하부
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			for (xx = 0 ; xx < varEnd - 1 ; ++xx) {																// 중간
				if (intPointer [xx] > 0) {
					moveIn3D ('z', pinbolt.radAng, (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

					count = realWidthCount;
					for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
						if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
							if (count > 0) {
								if (count == realWidthCount) {
									// 좌측
									if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								} else if (count == 1) {
									// 우측
									if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								} else {
									// 나머지
									if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								}

								--count;
							}
						}
					}
					moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), -(double)placingZone->cells [idxCell].horLen / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)backHeight / 1000.0 - 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realWidthCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 상부
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}
		}

		// 결합철물 - 앞면
		EasyObjectPlacement join;
		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('z', join.radAng, 0.450, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('z', join.radAng, (double)placingZone->cells [idxCell].verLenBasic / 1000.0 - 0.450, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		
		// 결합철물 - 뒷면
		if (placingZone->bSingleSide == false) {
			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('z', join.radAng - DegreeToRad (180.0), 0.450, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', join.radAng, -(double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)backHeight / 1000.0 - 0.450, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', join.radAng, -(double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}

		// 헤드피스 - 앞면
		EasyObjectPlacement headpiece;
		headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 47.5) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150) + placingZone->cells [idxCell].horLen + (-lastWidth + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		if (placingZone->cells [idxCell].verLenBasic >= 5300) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 4600) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 3500) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 3000) {
			headpieceUpPosZ = 2200;
		} else if (placingZone->cells [idxCell].verLenBasic >= 2500) {
			headpieceUpPosZ = 1900;
		} else if (placingZone->cells [idxCell].verLenBasic >= 2000) {
			headpieceUpPosZ = 1500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 1500) {
			headpieceUpPosZ = 1100;
		} else if (placingZone->cells [idxCell].verLenBasic >= 1000) {
			headpieceUpPosZ = 800;
		} else {
			headpieceUpPosZ = 150;
		}

		moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 47.5) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150) + placingZone->cells [idxCell].horLen + (-lastWidth + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// 헤드피스 - 뒷면
		if (placingZone->bSingleSide == false) {
			headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(firstWidth - 150 + 47.5) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(-(firstWidth - 150) + placingZone->cells [idxCell].horLen + (-lastWidth + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			if (backHeight >= 5300) {
				headpieceUpPosZ = 2500;
			} else if (backHeight >= 4600) {
				headpieceUpPosZ = 2500;
			} else if (backHeight >= 3500) {
				headpieceUpPosZ = 2500;
			} else if (backHeight >= 3000) {
				headpieceUpPosZ = 2200;
			} else if (backHeight >= 2500) {
				headpieceUpPosZ = 1900;
			} else if (backHeight >= 2000) {
				headpieceUpPosZ = 1500;
			} else if (backHeight >= 1500) {
				headpieceUpPosZ = 1100;
			} else if (backHeight >= 1000) {
				headpieceUpPosZ = 800;
			} else {
				headpieceUpPosZ = 150;
			}

			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(firstWidth - 150 + 47.5) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(-(firstWidth - 150) + placingZone->cells [idxCell].horLen + (-lastWidth + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}
	} else {
		// ================================================== 가로방향 (세로방향이 오른쪽으로 90도 누웠다고 생각하면 됨)
		// 수평 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
			sideMargin = 0.025;
		}

		EasyObjectPlacement rectPipe;
		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', rectPipe.radAng, -0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// 중간
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

				elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			}
		}
		moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 상부

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

		// 수평 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			if (backHeight % 100 == 0) {
				pipeLength = backHeight - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = backHeight - 50;
				sideMargin = 0.025;
			}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// 중간
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

					elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				}
			}
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - 0.450, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 상부

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		}

		// 수직 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].horLen - 50;
			sideMargin = 0.025;
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
				++realHeightCount;
			}
		}

		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', rectPipe.radAng, (double)(placingZone->cells [idxCell].verLenBasic - topHeight) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 왼쪽
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		moveIn3D ('z', rectPipe.radAng, -0.150 + (double)topHeight / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, (double)(-placingZone->cells [idxCell].verLenBasic + bottomHeight) / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 오른쪽
		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 수직 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			if (placingZone->cells [idxCell].horLen % 100 == 0) {
				pipeLength = placingZone->cells [idxCell].horLen - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = placingZone->cells [idxCell].horLen - 50;
				sideMargin = 0.025;
			}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 왼쪽
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			moveIn3D ('z', rectPipe.radAng, 0.150 - (double)bottomHeight / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('z', rectPipe.radAng, (double)(backHeight - topHeight) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 오른쪽
			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

			// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
			realHeightCount = 0;
			bFoundBottomHeight = false;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
				if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
					if (bFoundBottomHeight == false) {
						bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
						bFoundBottomHeight = true;
					}
					topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					++realHeightCount;
				}
			}
		}

		// 핀볼트 세트 - 앞면
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realHeightCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 하부
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {															// 중간
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

				count = realHeightCount;
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++yy) {
					if (placingZone->cells [idxCell].tableInVerBasic [yy] > 0) {
						if (count > 0) {
							if (count == realHeightCount) {
								// 우측
								if (placingZone->cells [idxCell].tableInVerBasic [yy] == 600) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 500) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 450) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 400) {
									moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 300) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 200) {
									moveIn3D ('z', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else if (count == 1) {
								// 좌측
								if (placingZone->cells [idxCell].tableInVerBasic [yy] == 600) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 500) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 450) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 400) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 300) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 200) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									moveIn3D ('z', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							} else {
								// 나머지
								if (placingZone->cells [idxCell].tableInVerBasic [yy] == 600) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 500) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 450) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 400) {
									moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 300) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 200) {
									moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
									moveIn3D ('z', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
								}
							}

							--count;
						}
					}
				}
				moveIn3D ('z', pinbolt.radAng, -(double)placingZone->cells [idxCell].verLenBasic / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realHeightCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 상부
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 핀볼트 세트 - 뒷면
		if (placingZone->bSingleSide == false) {
			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realHeightCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 하부
				if (intPointer [xx] > 0) {
					moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {															// 중간
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', pinbolt.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

					count = realHeightCount;
					for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++yy) {
						if (intPointer [yy] > 0) {
							if (count > 0) {
								if (count == realHeightCount) {
									// 우측
									if (intPointer [yy] == 600) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 500) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 450) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 400) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 300) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 200) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								} else if (count == 1) {
									// 좌측
									if (intPointer [yy] == 600) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 500) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 450) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 400) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 300) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 200) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
											moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								} else {
									// 나머지
									if (intPointer [yy] == 600) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 500) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 450) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 400) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 300) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									} else if (intPointer [yy] == 200) {
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
											elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
										moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
									}
								}

								--count;
							}
						}
					}
					moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), -(double)backHeight / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.450, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realHeightCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 상부
				if (intPointer [xx] > 0) {
					moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
				++realHeightCount;
			}
		}

		// 결합철물 - 앞면
		EasyObjectPlacement join;
		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', join.radAng, (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('x', join.radAng, 0.450, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + placingZone->cells [idxCell].verLenBasic - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', join.radAng, (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('x', join.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.450, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + placingZone->cells [idxCell].verLenBasic - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		
		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 결합철물 - 뒷면
		if (placingZone->bSingleSide == false) {
			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('x', join.radAng - DegreeToRad (180.0), 0.450, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + backHeight - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.450, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + backHeight - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
				++realHeightCount;
			}
		}

		// 헤드피스 - 앞면
		EasyObjectPlacement headpiece;
		headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', headpiece.radAng, (double)(bottomHeight - 150 - 50) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, (double)(-(bottomHeight - 150) + placingZone->cells [idxCell].verLenBasic + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		if (placingZone->cells [idxCell].horLen >= 5300) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].horLen >= 4600) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].horLen >= 3500) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].horLen >= 3000) {
			headpieceUpPosZ = 2200;
		} else if (placingZone->cells [idxCell].horLen >= 2500) {
			headpieceUpPosZ = 1900;
		} else if (placingZone->cells [idxCell].horLen >= 2000) {
			headpieceUpPosZ = 1500;
		} else if (placingZone->cells [idxCell].horLen >= 1500) {
			headpieceUpPosZ = 1100;
		} else if (placingZone->cells [idxCell].horLen >= 1000) {
			headpieceUpPosZ = 800;
		} else {
			headpieceUpPosZ = 150;
		}

		moveIn3D ('z', headpiece.radAng, (double)(bottomHeight - 150 - 50) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, (double)(headpieceUpPosZ) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, (double)(-(bottomHeight - 150) + placingZone->cells [idxCell].verLenBasic + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 헤드피스 - 뒷면
		if (placingZone->bSingleSide == false) {
			headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(bottomHeight - 150 - 50) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(-(bottomHeight - 150) + backHeight + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			if (placingZone->cells [idxCell].horLen >= 5300) {
				headpieceUpPosZ = 2500;
			} else if (placingZone->cells [idxCell].horLen >= 4600) {
				headpieceUpPosZ = 2500;
			} else if (placingZone->cells [idxCell].horLen >= 3500) {
				headpieceUpPosZ = 2500;
			} else if (placingZone->cells [idxCell].horLen >= 3000) {
				headpieceUpPosZ = 2200;
			} else if (placingZone->cells [idxCell].horLen >= 2500) {
				headpieceUpPosZ = 1900;
			} else if (placingZone->cells [idxCell].horLen >= 2000) {
				headpieceUpPosZ = 1500;
			} else if (placingZone->cells [idxCell].horLen >= 1500) {
				headpieceUpPosZ = 1100;
			} else if (placingZone->cells [idxCell].horLen >= 1000) {
				headpieceUpPosZ = 800;
			} else {
				headpieceUpPosZ = 150;
			}

			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(bottomHeight - 150 - 50) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(headpieceUpPosZ) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(-(bottomHeight - 150) + backHeight + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}
	}

	// 결과물 전체 그룹화 (앞면)
	groupElements (elemList_Front);
	elemList_Front.Clear ();

	// 결과물 전체 그룹화 (뒷면)
	groupElements (elemList_Back);
	elemList_Back.Clear ();
}

// 테이블폼 타입C 배치 (유로폼 제외) - 각파이프 1줄, 십자 조인트 바 활용
void	WallTableformPlacingZone::placeTableformC (WallTableformPlacingZone* placingZone, short idxCell)
{
	short	xx, yy, varEnd;
	int		pipeLength;
	int		headpieceUpPosZ;
	double	sideMargin;
	int*	intPointer;
	int		backHeight;
	
	int		firstWidth, lastWidth;
	bool	bFoundFirstWidth;
	short	realWidthCount, count;

	int		topHeight, bottomHeight;
	bool	bFoundBottomHeight;
	short	realHeightCount;

	double	accumLength;
	double	bottomRectPipeOffset, topRectPipeOffset;

	if (placingZone->bExtra == true) {
		varEnd = placingZone->nCellsInVerExtra;
		intPointer = placingZone->cells [idxCell].tableInVerExtra;
		backHeight = placingZone->cells [idxCell].verLenExtra;
	} else {
		varEnd = placingZone->nCellsInVerBasic;
		intPointer = placingZone->cells [idxCell].tableInVerBasic;
		backHeight = placingZone->cells [idxCell].verLenBasic;
	}

	// 테이블폼 내 1번째와 마지막 유로폼의 각각의 너비를 가져옴 (길이가 0인 유로폼은 통과) - 세로방향
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

	if (placingZone->cells [idxCell].objType != TABLEFORM)
		return;

	if (placingZone->cells [idxCell].horLen == 0)
		return;

	if (placingZone->bVertical == true) {
		// ================================================== 세로방향
		// 십자 조인트 바 배치 - 앞면
		EasyObjectPlacement cross;
		cross.init (L("십자 조인트 바 v1.0.gsm"), layerInd_CrossJointBar, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);
		moveIn3D ('y', cross.radAng, -0.0635, &cross.posX, &cross.posY, &cross.posZ);

		for (xx = 0 ; xx < varEnd - 1 ; ++xx) {
			moveIn3D ('z', cross.radAng, (double)intPointer [xx] / 1000.0 - 0.250, &cross.posX, &cross.posY, &cross.posZ);
			count = 0;
			for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++yy) {
				if (placingZone->cells [idxCell].tableInHor [yy] > 0)
					count ++;
			}
			accumLength = 0.0;
			for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++yy) {
				if (placingZone->cells [idxCell].tableInHor [yy] > 0 && count > 1) {
					accumLength += (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0;
					moveIn3D ('x', cross.radAng, (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &cross.posX, &cross.posY, &cross.posZ);
					elemList_Front.Push (cross.placeObject (3, "bRotated", APIParT_Boolean, "1.0", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
					count --;
				}
			}
			moveIn3D ('x', cross.radAng, -accumLength, &cross.posX, &cross.posY, &cross.posZ);
			moveIn3D ('z', cross.radAng, 0.250, &cross.posX, &cross.posY, &cross.posZ);
		}

		// 십자 조인트 바 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			cross.init (L("십자 조인트 바 v1.0.gsm"), layerInd_CrossJointBar, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
			moveIn3D ('y', cross.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.0635, &cross.posX, &cross.posY, &cross.posZ);

			for (xx = 0 ; xx < varEnd - 1 ; ++xx) {
				moveIn3D ('z', cross.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0 - 0.250, &cross.posX, &cross.posY, &cross.posZ);
				count = 0;
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++yy) {
					if (placingZone->cells [idxCell].tableInHor [yy] > 0)
						count ++;
				}
				accumLength = 0.0;
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++yy) {
					if (placingZone->cells [idxCell].tableInHor [yy] > 0 && count > 1) {
						accumLength += (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0;
						moveIn3D ('x', cross.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &cross.posX, &cross.posY, &cross.posZ);
						elemList_Back.Push (cross.placeObject (3, "bRotated", APIParT_Boolean, "1.0", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
						count --;
					}
				}
				moveIn3D ('x', cross.radAng - DegreeToRad (180.0), -accumLength, &cross.posX, &cross.posY, &cross.posZ);
				moveIn3D ('z', cross.radAng - DegreeToRad (180.0), 0.250, &cross.posX, &cross.posY, &cross.posZ);
			}
		}

		// 수평 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].horLen - 50;
			sideMargin = 0.025;
		}

		// 십자 조인트 바에 간섭하지 않도록 위치 조정함
		bottomRectPipeOffset = 0.450;
		topRectPipeOffset = 0.450;

		if (placingZone->bVertical == true) {
			if (placingZone->cells [idxCell].tableInVerBasic [0] == 1200)
				bottomRectPipeOffset = 0.450;
			else if (placingZone->cells [idxCell].tableInVerBasic [0] == 900)
				bottomRectPipeOffset = 0.300;
			else if (placingZone->cells [idxCell].tableInVerBasic [0] == 600)
				bottomRectPipeOffset = 0.150;

			if (placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] == 1200)
				topRectPipeOffset = 0.450;
			else if (placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] == 900)
				topRectPipeOffset = 0.300;
			else if (placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] == 600)
				topRectPipeOffset = 0.150;
		}

		EasyObjectPlacement rectPipe;
		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, bottomRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		moveIn3D ('z', rectPipe.radAng, -bottomRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {									// 중간
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			}
		}

		moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] / 1000.0 - topRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 상부
		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

		// 수평 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), bottomRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			moveIn3D ('z', rectPipe.radAng, -bottomRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			for (xx = 0 ; xx < varEnd - 1 ; ++xx) {														// 중간
				if (intPointer [xx] > 0) {
					moveIn3D ('z', rectPipe.radAng, (double)intPointer [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				}
			}

			moveIn3D ('z', rectPipe.radAng, (double)intPointer [varEnd - 1] / 1000.0 - topRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 상부
			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		}

		// 수직 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
			sideMargin = 0.025;
		}

		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, (double)firstWidth / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 왼쪽
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', rectPipe.radAng, 0.150 - (double)firstWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 오른쪽
		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

		// 수직 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			if (backHeight % 100 == 0) {
				pipeLength = backHeight - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = backHeight - 50;
				sideMargin = 0.025;
			}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 왼쪽
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			moveIn3D ('x', rectPipe.radAng, 0.150 - (double)lastWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - firstWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 오른쪽
			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		}

		// 핀볼트 세트 - 앞면
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, bottomRectPipeOffset, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 하부
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].verLenBasic / 1000.0 - topRectPipeOffset, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 상부
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		// 핀볼트 세트 - 뒷면
		if (placingZone->bSingleSide == false) {
			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), bottomRectPipeOffset, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realWidthCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 하부
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)backHeight / 1000.0 - topRectPipeOffset, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realWidthCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// 상부
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}
		}

		// 결합철물 - 앞면
		EasyObjectPlacement join;
		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('z', join.radAng, bottomRectPipeOffset, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('z', join.radAng, (double)placingZone->cells [idxCell].verLenBasic / 1000.0 - topRectPipeOffset, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		
		// 결합철물 - 뒷면
		if (placingZone->bSingleSide == false) {
			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('z', join.radAng - DegreeToRad (180.0), bottomRectPipeOffset, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', join.radAng, -(double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)backHeight / 1000.0 - topRectPipeOffset, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', join.radAng, -(double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}

		// 헤드피스 - 앞면
		EasyObjectPlacement headpiece;
		headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 47.5) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150) + placingZone->cells [idxCell].horLen + (-lastWidth + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		if (placingZone->cells [idxCell].verLenBasic >= 5300) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 4600) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 3500) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 3000) {
			headpieceUpPosZ = 2200;
		} else if (placingZone->cells [idxCell].verLenBasic >= 2500) {
			headpieceUpPosZ = 1900;
		} else if (placingZone->cells [idxCell].verLenBasic >= 2000) {
			headpieceUpPosZ = 1500;
		} else if (placingZone->cells [idxCell].verLenBasic >= 1500) {
			headpieceUpPosZ = 1100;
		} else if (placingZone->cells [idxCell].verLenBasic >= 1000) {
			headpieceUpPosZ = 800;
		} else {
			headpieceUpPosZ = 150;
		}

		moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 47.5) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150) + placingZone->cells [idxCell].horLen + (-lastWidth + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// 헤드피스 - 뒷면
		if (placingZone->bSingleSide == false) {
			headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(firstWidth - 150 + 47.5) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(-(firstWidth - 150) + placingZone->cells [idxCell].horLen + (-lastWidth + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			if (backHeight >= 5300) {
				headpieceUpPosZ = 2500;
			} else if (backHeight >= 4600) {
				headpieceUpPosZ = 2500;
			} else if (backHeight >= 3500) {
				headpieceUpPosZ = 2500;
			} else if (backHeight >= 3000) {
				headpieceUpPosZ = 2200;
			} else if (backHeight >= 2500) {
				headpieceUpPosZ = 1900;
			} else if (backHeight >= 2000) {
				headpieceUpPosZ = 1500;
			} else if (backHeight >= 1500) {
				headpieceUpPosZ = 1100;
			} else if (backHeight >= 1000) {
				headpieceUpPosZ = 800;
			} else {
				headpieceUpPosZ = 150;
			}

			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(firstWidth - 150 + 47.5) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(-(firstWidth - 150) + placingZone->cells [idxCell].horLen + (-lastWidth + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 A", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}
	} else {
		// ================================================== 가로방향 (세로방향이 오른쪽으로 90도 누웠다고 생각하면 됨)
		// 십자 조인트 바 배치 - 앞면
		EasyObjectPlacement cross;
		cross.init (L("십자 조인트 바 v1.0.gsm"), layerInd_CrossJointBar, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);
		moveIn3D ('y', cross.radAng, -0.0635, &cross.posX, &cross.posY, &cross.posZ);

		for (xx = 0 ; xx < varEnd - 1 ; ++xx) {
			moveIn3D ('z', cross.radAng, (double)intPointer [xx] / 1000.0 - 0.250, &cross.posX, &cross.posY, &cross.posZ);
			count = 0;
			for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++yy) {
				if (placingZone->cells [idxCell].tableInHor [yy] > 0)
					count ++;
			}
			accumLength = 0.0;
			for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++yy) {
				if (placingZone->cells [idxCell].tableInHor [yy] > 0 && count > 1) {
					accumLength += (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0;
					moveIn3D ('x', cross.radAng, (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &cross.posX, &cross.posY, &cross.posZ);
					elemList_Front.Push (cross.placeObject (3, "bRotated", APIParT_Boolean, "1.0", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
					count --;
				}
			}
			moveIn3D ('x', cross.radAng, -accumLength, &cross.posX, &cross.posY, &cross.posZ);
			moveIn3D ('z', cross.radAng, 0.250, &cross.posX, &cross.posY, &cross.posZ);
		}

		// 십자 조인트 바 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			cross.init (L("십자 조인트 바 v1.0.gsm"), layerInd_CrossJointBar, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
			moveIn3D ('y', cross.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.0635, &cross.posX, &cross.posY, &cross.posZ);

			for (xx = 0 ; xx < varEnd - 1 ; ++xx) {
				moveIn3D ('z', cross.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0 - 0.250, &cross.posX, &cross.posY, &cross.posZ);
				count = 0;
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++yy) {
					if (placingZone->cells [idxCell].tableInHor [yy] > 0)
						count ++;
				}
				accumLength = 0.0;
				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++yy) {
					if (placingZone->cells [idxCell].tableInHor [yy] > 0 && count > 1) {
						accumLength += (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0;
						moveIn3D ('x', cross.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [yy] / 1000.0, &cross.posX, &cross.posY, &cross.posZ);
						elemList_Back.Push (cross.placeObject (3, "bRotated", APIParT_Boolean, "1.0", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
						count --;
					}
				}
				moveIn3D ('x', cross.radAng - DegreeToRad (180.0), -accumLength, &cross.posX, &cross.posY, &cross.posZ);
				moveIn3D ('z', cross.radAng - DegreeToRad (180.0), 0.250, &cross.posX, &cross.posY, &cross.posZ);
			}
		}

		// 수평 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
			sideMargin = 0.025;
		}

		// 십자 조인트 바에 간섭하지 않도록 위치 조정함
		bottomRectPipeOffset = 0.450;
		topRectPipeOffset = 0.450;

		if (placingZone->bVertical == true) {
			if (placingZone->cells [idxCell].tableInVerBasic [0] == 1200)
				bottomRectPipeOffset = 0.450;
			else if (placingZone->cells [idxCell].tableInVerBasic [0] == 900)
				bottomRectPipeOffset = 0.300;
			else if (placingZone->cells [idxCell].tableInVerBasic [0] == 600)
				bottomRectPipeOffset = 0.150;

			if (placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] == 1200)
				topRectPipeOffset = 0.450;
			else if (placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] == 900)
				topRectPipeOffset = 0.300;
			else if (placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] == 600)
				topRectPipeOffset = 0.150;
		}

		EasyObjectPlacement rectPipe;
		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, bottomRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', rectPipe.radAng, -bottomRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// 중간
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			}
		}

		moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - topRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 상부
		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

		// 수평 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			if (backHeight % 100 == 0) {
				pipeLength = backHeight - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = backHeight - 50;
				sideMargin = 0.025;
			}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 하부
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), bottomRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -bottomRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// 중간
				if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
					moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				}
			}

			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - topRectPipeOffset, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 상부
			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		}

		// 수직 파이프 배치 - 앞면
		if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
		} else {
			pipeLength = placingZone->cells [idxCell].horLen - 50;
			sideMargin = 0.025;
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
				++realHeightCount;
			}
		}

		rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', rectPipe.radAng, (double)(placingZone->cells [idxCell].verLenBasic - topHeight) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 왼쪽
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
		moveIn3D ('z', rectPipe.radAng, -0.150 + (double)topHeight / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, (double)(-placingZone->cells [idxCell].verLenBasic + bottomHeight) / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 오른쪽
		elemList_Front.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 수직 파이프 배치 - 뒷면
		if (placingZone->bSingleSide == false) {
			if (placingZone->cells [idxCell].horLen % 100 == 0) {
				pipeLength = placingZone->cells [idxCell].horLen - 100;
				sideMargin = 0.050;
			} else {
				pipeLength = placingZone->cells [idxCell].horLen - 50;
				sideMargin = 0.025;
			}

			rectPipe.init (L("비계파이프v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// 왼쪽
			moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
			moveIn3D ('z', rectPipe.radAng, 0.150 - (double)bottomHeight / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('z', rectPipe.radAng, (double)(backHeight - topHeight) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// 오른쪽
			elemList_Back.Push (rectPipe.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));

			// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
			realHeightCount = 0;
			bFoundBottomHeight = false;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
				if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
					if (bFoundBottomHeight == false) {
						bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
						bFoundBottomHeight = true;
					}
					topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					++realHeightCount;
				}
			}
		}

		// 핀볼트 세트 - 앞면
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, bottomRectPipeOffset, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realHeightCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 하부
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0 - topRectPipeOffset, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realHeightCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 상부
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 핀볼트 세트 - 뒷면
		if (placingZone->bSingleSide == false) {
			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), bottomRectPipeOffset, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realHeightCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 하부
				if (intPointer [xx] > 0) {
					moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}

			pinbolt.init (L("핀볼트세트v1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - topRectPipeOffset, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

			count = realHeightCount - 1;
			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// 상부
				if (intPointer [xx] > 0) {
					moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
					if (count > 0) {
						pinbolt.radAng += DegreeToRad (90.0);
						elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
						pinbolt.radAng -= DegreeToRad (90.0);

						--count;
					}
				}
			}
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
				++realHeightCount;
			}
		}

		// 결합철물 - 앞면
		EasyObjectPlacement join;
		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', join.radAng, (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('x', join.radAng, bottomRectPipeOffset, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + placingZone->cells [idxCell].verLenBasic - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', join.radAng, (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('x', join.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0 - topRectPipeOffset, &join.posX, &join.posY, &join.posZ);

		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + placingZone->cells [idxCell].verLenBasic - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		
		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 결합철물 - 뒷면
		if (placingZone->bSingleSide == false) {
			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('x', join.radAng - DegreeToRad (180.0), bottomRectPipeOffset, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + backHeight - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
			moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - topRectPipeOffset, &join.posX, &join.posY, &join.posZ);

			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + backHeight - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
			elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (앞면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
				++realHeightCount;
			}
		}

		// 헤드피스 - 앞면
		EasyObjectPlacement headpiece;
		headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', headpiece.radAng, (double)(bottomHeight - 150 - 50) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, (double)(-(bottomHeight - 150) + placingZone->cells [idxCell].verLenBasic + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		if (placingZone->cells [idxCell].horLen >= 5300) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].horLen >= 4600) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].horLen >= 3500) {
			headpieceUpPosZ = 2500;
		} else if (placingZone->cells [idxCell].horLen >= 3000) {
			headpieceUpPosZ = 2200;
		} else if (placingZone->cells [idxCell].horLen >= 2500) {
			headpieceUpPosZ = 1900;
		} else if (placingZone->cells [idxCell].horLen >= 2000) {
			headpieceUpPosZ = 1500;
		} else if (placingZone->cells [idxCell].horLen >= 1500) {
			headpieceUpPosZ = 1100;
		} else if (placingZone->cells [idxCell].horLen >= 1000) {
			headpieceUpPosZ = 800;
		} else {
			headpieceUpPosZ = 150;
		}

		moveIn3D ('z', headpiece.radAng, (double)(bottomHeight - 150 - 50) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, (double)(headpieceUpPosZ) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, (double)(-(bottomHeight - 150) + placingZone->cells [idxCell].verLenBasic + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList_Front.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// 테이블폼 내 바닥과 꼭대기 유로폼의 각각의 높이를 가져옴 (길이가 0인 유로폼은 통과) - 가로방향 (뒷면)
		realHeightCount = 0;
		bFoundBottomHeight = false;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
			if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
				if (bFoundBottomHeight == false) {
					bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
					bFoundBottomHeight = true;
				}
				topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
				++realHeightCount;
			}
		}

		// 헤드피스 - 뒷면
		if (placingZone->bSingleSide == false) {
			headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(bottomHeight - 150 - 50) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(-(bottomHeight - 150) + backHeight + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (3, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

			headpiece.init (L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

			if (placingZone->cells [idxCell].horLen >= 5300) {
				headpieceUpPosZ = 2500;
			} else if (placingZone->cells [idxCell].horLen >= 4600) {
				headpieceUpPosZ = 2500;
			} else if (placingZone->cells [idxCell].horLen >= 3500) {
				headpieceUpPosZ = 2500;
			} else if (placingZone->cells [idxCell].horLen >= 3000) {
				headpieceUpPosZ = 2200;
			} else if (placingZone->cells [idxCell].horLen >= 2500) {
				headpieceUpPosZ = 1900;
			} else if (placingZone->cells [idxCell].horLen >= 2000) {
				headpieceUpPosZ = 1500;
			} else if (placingZone->cells [idxCell].horLen >= 1500) {
				headpieceUpPosZ = 1100;
			} else if (placingZone->cells [idxCell].horLen >= 1000) {
				headpieceUpPosZ = 800;
			} else {
				headpieceUpPosZ = 150;
			}

			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(bottomHeight - 150 - 50) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.2685, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(headpieceUpPosZ) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(-(bottomHeight - 150) + backHeight + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
			elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "타입 B", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		}
	}

	// 결과물 전체 그룹화 (앞면)
	groupElements (elemList_Front);
	elemList_Front.Clear ();

	// 결과물 전체 그룹화 (뒷면)
	groupElements (elemList_Back);
	elemList_Back.Clear ();
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
			placingZone.BUTTON_ADD_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 10, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_ADD_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_ADD_HOR, "추가");
			DGShowItem (dialogID, placingZone.BUTTON_ADD_HOR);

			// 버튼: 삭제
			placingZone.BUTTON_DEL_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 10, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_DEL_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_DEL_HOR, "삭제");
			DGShowItem (dialogID, placingZone.BUTTON_DEL_HOR);

			// 라벨: 남은 너비
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 290, 17, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "남은 너비");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 남은 너비
			placingZone.EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 375, 10, 80, 25);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH);

			// 라벨: 벽과의 간격
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 50, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "벽과의 간격");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 벽과의 간격
			placingZone.EDITCONTROL_GAP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 105, 45, 70, 23);
			DGShowItem (dialogID, placingZone.EDITCONTROL_GAP);

			// 라벨: 양면/단면
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 220, 50, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "양면/단면");
			DGShowItem (dialogID, itmIdx);
			
			// 체크박스: 단면 여부
			placingZone.CHECKBOX_SINGLESIDE = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 305, 45, 70, 25);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_SINGLESIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_SINGLESIDE, "단면");
			DGShowItem (dialogID, placingZone.CHECKBOX_SINGLESIDE);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_SINGLESIDE, FALSE);
			if (placingZone.bExtra == true)
				DGDisableItem (dialogID, placingZone.CHECKBOX_SINGLESIDE);

			// 라벨: 테이블폼 방향
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 80, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "테이블폼 방향");
			DGShowItem (dialogID, itmIdx);

			// 팝업컨트롤: 테이블폼 방향
			placingZone.POPUP_DIRECTION = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 105, 75, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_DIRECTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM, "세로");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM, "가로");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_DIRECTION);

			// 라벨: 테이블폼 타입
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 220, 80, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "테이블폼 타입");
			DGShowItem (dialogID, itmIdx);

			// 팝업컨트롤: 테이블폼 타입
			placingZone.POPUP_TABLEFORM_TYPE = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 305, 75, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "타입A");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "타입B");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "타입C");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE);

			// 라벨: 푸시풀프롭스 설치 타입
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 390, 80, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "푸시풀프롭스");
			DGShowItem (dialogID, itmIdx);

			// 팝업컨트롤: 푸시풀프롭스 설치 타입
			placingZone.POPUP_PROPS_INSTALL = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 475, 75, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_POPUP_BOTTOM, "안쪽");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_POPUP_BOTTOM, "바깥쪽");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_POPUP_BOTTOM, "양쪽");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_PROPS_INSTALL, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_PROPS_INSTALL);

			//////////////////////////////////////////////////////////// 셀 정보 초기화
			placingZone.initCells (&placingZone, true);

			//////////////////////////////////////////////////////////// 아이템 배치 (정면 관련 버튼)
			// 왼쪽 인코너판넬/아웃코너판넬/아웃코너앵글
			// 버튼
			placingZone.BUTTON_LCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 137, 71, 66);
			DGSetItemFont (dialogID, placingZone.BUTTON_LCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "없음");
			DGShowItem (dialogID, placingZone.BUTTON_LCORNER);
			DGDisableItem (dialogID, placingZone.BUTTON_LCORNER);
			// 객체 타입 (팝업컨트롤)
			placingZone.POPUP_OBJ_TYPE_LCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 20, 137 - 25, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, "인코너판넬");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, "아웃코너판넬");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, "아웃코너앵글");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER);
			// 너비 (Edit컨트롤)
			placingZone.EDITCONTROL_WIDTH_LCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 20, 137 + 68, 70, 23);
			DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);

			// 일반 셀: 기본값은 테이블폼
			itmPosX = 90;
			itmPosY = 137;
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				// 버튼
				placingZone.BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
				DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], "테이블폼");
				DGShowItem (dialogID, placingZone.BUTTON_OBJ [xx]);

				// 객체 타입 (팝업컨트롤)
				placingZone.POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "없음");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "테이블폼");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "유로폼");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "휠러스페이서");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "합판");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "각재");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

				// 너비 (팝업컨트롤)
				placingZone.POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
				}
				DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

				// 너비 (팝업컨트롤) - 처음에는 숨김
				placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
				DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);

				itmPosX += 70;
			}

			// 오른쪽 인코너판넬/아웃코너판넬/아웃코너앵글
			// 버튼
			placingZone.BUTTON_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, 137, 71, 66);
			DGSetItemFont (dialogID, placingZone.BUTTON_RCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "없음");
			DGShowItem (dialogID, placingZone.BUTTON_RCORNER);
			DGDisableItem (dialogID, placingZone.BUTTON_RCORNER);
			// 객체 타입 (팝업컨트롤)
			placingZone.POPUP_OBJ_TYPE_RCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, 137 - 25, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "인코너판넬");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "아웃코너판넬");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "아웃코너앵글");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
			// 너비 (Edit컨트롤)
			placingZone.EDITCONTROL_WIDTH_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 137 + 68, 70, 23);
			DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

			//////////////////////////////////////////////////////////// 아이템 배치 (측면 관련 버튼)
			// 라벨: 측면
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 257, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "측면");
			DGShowItem (dialogID, itmIdx);

			// 버튼: 추가 (낮은쪽)
			placingZone.BUTTON_ADD_VER_BASIC = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 250, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_ADD_VER_BASIC, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_ADD_VER_BASIC, "추가(L)");
			DGShowItem (dialogID, placingZone.BUTTON_ADD_VER_BASIC);

			// 버튼: 삭제 (낮은쪽)
			placingZone.BUTTON_DEL_VER_BASIC = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 250 + 30, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_DEL_VER_BASIC, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_DEL_VER_BASIC, "삭제(L)");
			DGShowItem (dialogID, placingZone.BUTTON_DEL_VER_BASIC);

			// 라벨: 남은 높이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 317, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "남은 높이");
			DGShowItem (dialogID, itmIdx);

			// Edit컨트롤: 남은 높이 (낮은쪽)
			placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 105, 310, 70, 25);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC);
			DGShowItem (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC);

			if (placingZone.bExtra == true) {
				// 버튼: 추가 (높은쪽)
				placingZone.BUTTON_ADD_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250, 70, 25);
				DGSetItemFont (dialogID, placingZone.BUTTON_ADD_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_ADD_VER_EXTRA, "추가(H)");
				DGShowItem (dialogID, placingZone.BUTTON_ADD_VER_EXTRA);

				// 버튼: 삭제 (높은쪽)
				placingZone.BUTTON_DEL_VER_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 185, 250 + 30, 70, 25);
				DGSetItemFont (dialogID, placingZone.BUTTON_DEL_VER_EXTRA, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_DEL_VER_EXTRA, "삭제(H)");
				DGShowItem (dialogID, placingZone.BUTTON_DEL_VER_EXTRA);

				// Edit컨트롤: 남은 높이 (높은쪽)
				placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 185, 310, 70, 25);
				DGDisableItem (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA);
				DGShowItem (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA);
			}

			// 팝업컨트롤: 테이블폼 세로방향 프리셋
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

			// 왼쪽에 낮은쪽
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
				// 오른쪽에 높은쪽
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

			//////////////////////////////////////////////////////////// 다이얼로그 크기 변경, 남은 너비 및 높이 계산
			// 다이얼로그 크기 설정
			dialogSizeX = 550;
			dialogSizeY = 950;
			if (placingZone.nCellsInHor >= 5) {
				DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
			}

			// 남은 너비 계산
			totalWidth = 0.0;
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			// 남은 높이 계산 (낮은쪽)
			totalHeight = 0.0;
			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
				totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx])).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

			if (placingZone.bExtra == true) {
				// 남은 높이 계산 (높은쪽)
				totalHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
					totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx])).ToCStr ().Get ()) / 1000;
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
			}

			// 초기값은 세로방향
			placingZone.bVertical = true;

			break;

		case DG_MSG_CHANGE:
			// 가로/세로 변경할 때
			if (item == placingZone.POPUP_DIRECTION) {
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_DIRECTION, DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION)).ToCStr ().Get ());

				// 가로일 경우
				if (my_strcmp (buffer, "가로") == 0) {
					// 셀 정보 초기화
					placingZone.initCells (&placingZone, false);
					placingZone.bVertical = false;

					// 정면
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

					// 측면
					for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
						// 팝업 전부 비우고
						DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_ALL_ITEMS);

						// 팝업 내용 다시 채우고
						for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
							DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
							_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
						}
						DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);
					}

					if (placingZone.bExtra == true) {
						for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);
						}
					}

					// 프리셋
					DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_ALL_ITEMS);

					for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
						DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
						_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, numbuf);
					}
					DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, "Free");
					DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);

				// 세로일 경우
				} else {
					// 셀 정보 초기화
					placingZone.initCells (&placingZone, true);
					placingZone.bVertical = true;

					// 정면
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

					// 측면
					for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
						// 팝업 전부 비우고
						DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_ALL_ITEMS);

						// 팝업 내용 다시 채우고
						for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
							DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
							_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
						}
						DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);
					}

					if (placingZone.bExtra == true) {
						for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);
						}
					}

					// 프리셋
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

			// 인코너판넬/아웃코너판넬/아웃코너앵글 변경시
			if (item == placingZone.POPUP_OBJ_TYPE_LCORNER) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == NOCORNER) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "없음");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == INCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "인코너판넬");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.500);
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.100);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == OUTCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "아웃코너판넬");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.500);
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.100);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == OUTCORNER_ANGLE) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "아웃코너앵글");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				}
			}

			if (item == placingZone.POPUP_OBJ_TYPE_RCORNER) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == NOCORNER) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "없음");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == INCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "인코너판넬");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.500);
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.100);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == OUTCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "아웃코너판넬");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.500);
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.100);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == OUTCORNER_ANGLE) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "아웃코너앵글");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
				}
			}

			// 객체 타입 변경할 때
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if (item == placingZone.POPUP_OBJ_TYPE [xx]) {
					// 해당 버튼의 이름 변경
					DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], DGPopUpGetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx])));
					
					// 가로/세로 방향 여부에 따라 팝업컨트롤의 내용물이 바뀜
					if (placingZone.bVertical == false) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					} else {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// 팝업 전부 비우고
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// 팝업 내용 다시 채우고
							for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

					// 테이블폼 타입이 아니면 버튼을 잠금
					if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) != TABLEFORM)
						DGDisableItem (dialogID, placingZone.BUTTON_OBJ [xx]);
					else
						DGEnableItem (dialogID, placingZone.BUTTON_OBJ [xx]);

					// 테이블폼/유로폼이면 너비가 팝업컨트롤, 그 외에는 Edit컨트롤, 없으면 모두 숨김
					if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM)) {
						if (!DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE) {
						if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);
					} else {
						if (!DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))	DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);

						// 휠러스페이서, 합판, 각재의 가로 방향 길이의 최소, 최대값
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

			// 프리셋 변경시
			if (item == placingZone.POPUP_HEIGHT_PRESET) {
				// 가로일 때
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

				// 세로일 때
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

			// 테이블폼 너비를 변경할 때
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

			// 남은 너비 계산
			totalWidth = 0.0;
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			// 남은 높이 계산 (낮은쪽)
			totalHeight = 0.0;
			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
				totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx])).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

			if (placingZone.bExtra == true) {
				// 남은 높이 계산 (높은쪽)
				totalHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
					totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx])).ToCStr ().Get ()) / 1000;
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
			}

			break;

		case DG_MSG_CLICK:
			// 확인 버튼
			if (item == DG_OK) {

				// 벽과의 간격
				placingZone.gap = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_GAP);

				// 단면 여부
				placingZone.bSingleSide = (DGGetItemValLong (dialogID, placingZone.CHECKBOX_SINGLESIDE) == TRUE) ? true : false;

				// 테이블폼 방향
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_DIRECTION, DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION)).ToCStr ().Get ());
				if (my_strcmp (buffer, "세로") == 0)
					placingZone.bVertical = true;
				else
					placingZone.bVertical = false;

				// 테이블폼 타입
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DGPopUpGetSelected (dialogID, placingZone.POPUP_TABLEFORM_TYPE)).ToCStr ().Get ());
				if (my_strcmp (buffer, "타입A") == 0)
					placingZone.tableformType = 1;
				else if (my_strcmp (buffer, "타입B") == 0)
					placingZone.tableformType = 2;
				else if (my_strcmp (buffer, "타입C") == 0)
					placingZone.tableformType = 3;

				// 푸시풀프롭스 설치 타입
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_PROPS_INSTALL, DGPopUpGetSelected (dialogID, placingZone.POPUP_PROPS_INSTALL)).ToCStr ().Get ());
				if (my_strcmp (buffer, "없음") == 0)
					placingZone.propsInstallType = 1;
				else if (my_strcmp (buffer, "안쪽") == 0)
					placingZone.propsInstallType = 2;
				else if (my_strcmp (buffer, "바깥쪽") == 0)
					placingZone.propsInstallType = 3;
				else if (my_strcmp (buffer, "양쪽") == 0)
					placingZone.propsInstallType = 4;

				// 인코너판넬/아웃코너판넬/아웃코너앵글 유무 및 길이
				placingZone.typeLcorner = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER);
				placingZone.lenLcorner = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				placingZone.typeRcorner = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
				placingZone.lenRcorner = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

				// 세로 길이, 테이블 내 세로 길이 모두 0으로 초기화
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					placingZone.cells [xx].verLenBasic = 0;
					placingZone.cells [xx].verLenExtra = 0;

					for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInVerBasic) / sizeof (int) ; ++yy)
						placingZone.cells [xx].tableInVerBasic [yy] = 0;
					for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInVerExtra) / sizeof (int) ; ++yy)
						placingZone.cells [xx].tableInVerExtra [yy] = 0;
				}

				// 셀 정보 업데이트
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					// 객체 타입
					placingZone.cells [xx].objType = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

					// 가로 방향 길이 지정
					if (placingZone.cells [xx].objType == NONE) {
						// 가로 길이 0
						placingZone.cells [xx].horLen = 0;

						// 테이블 내 가로 길이 모두 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;

					} else if (placingZone.cells [xx].objType == EUROFORM) {
						// 유로폼 너비
						placingZone.cells [xx].horLen = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ());

						// 테이블 내 가로 길이 모두 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;

					} else if ((placingZone.cells [xx].objType == FILLERSP) || (placingZone.cells [xx].objType == PLYWOOD) || (placingZone.cells [xx].objType == TIMBER)) {
						// 휠러스페이서, 합판, 목재의 너비
						placingZone.cells [xx].horLen = (int)(DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]) * 1000);

						// 테이블 내 가로 길이 모두 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;
					}

					// 세로 방향 길이 설정 (낮은쪽)
					accumLength = 0;
					for (yy = 0 ; yy < placingZone.nCellsInVerBasic ; ++yy) {
						// 테이블 내 세로 길이 설정
						placingZone.cells [xx].tableInVerBasic [yy] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy])).ToCStr ().Get ());
						accumLength += placingZone.cells [xx].tableInVerBasic [yy];
					}
					placingZone.cells [xx].verLenBasic = accumLength;
				
					// 세로 방향 길이 설정 (높은쪽)
					accumLength = 0;
					for (yy = 0 ; yy < placingZone.nCellsInVerExtra ; ++yy) {
						// 테이블 내 세로 길이 설정
						placingZone.cells [xx].tableInVerExtra [yy] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy])).ToCStr ().Get ());
						accumLength += placingZone.cells [xx].tableInVerExtra [yy];
					}
					placingZone.cells [xx].verLenExtra = accumLength;
				}

				// 레이어 설정
				bLayerInd_Euroform = true;		// 유로폼 항상 On
				bLayerInd_RectPipe = false;
				bLayerInd_PinBolt = false;
				bLayerInd_WallTie = false;
				bLayerInd_HeadPiece = false;
				bLayerInd_Props = false;
				bLayerInd_Join = false;

				bLayerInd_SlabTableform = false;
				bLayerInd_Profile = false;

				bLayerInd_Steelform = false;
				bLayerInd_Plywood = true;		// 합판 항상 On
				bLayerInd_Timber = true;		// 각재 항상 On
				bLayerInd_IncornerPanel = false;
				bLayerInd_OutcornerAngle = false;
				bLayerInd_OutcornerPanel = false;
				bLayerInd_RectpipeHanger = false;
				bLayerInd_EuroformHook = false;
				bLayerInd_CrossJointBar = false;
				bLayerInd_Hidden = false;

				bLayerInd_Fillersp = false;
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					if (placingZone.cells [xx].objType == FILLERSP) {
						bLayerInd_Fillersp = true;
						break;
					}
				}

				// 인코너판넬
				if ((placingZone.typeLcorner == INCORNER_PANEL) || (placingZone.typeRcorner == INCORNER_PANEL))
					bLayerInd_IncornerPanel = true;

				// 아웃코너판넬
				if ((placingZone.typeLcorner == OUTCORNER_PANEL) || (placingZone.typeRcorner == OUTCORNER_PANEL))
					bLayerInd_OutcornerPanel = true;

				// 아웃코너앵글
				if ((placingZone.typeLcorner == OUTCORNER_ANGLE) || (placingZone.typeRcorner == OUTCORNER_ANGLE))
					bLayerInd_OutcornerAngle = true;

				bLayerInd_BlueClamp = true;			// 블루클램프 항상 On
				bLayerInd_BlueTimberRail = true;	// 블루목심 항상 On

				if (placingZone.tableformType == 1) {
					bLayerInd_Euroform = true;
					bLayerInd_RectPipe = true;
					bLayerInd_PinBolt = true;
					bLayerInd_HeadPiece = true;
					bLayerInd_Props = true;
					bLayerInd_Join = true;

				} else if (placingZone.tableformType == 2) {
					bLayerInd_Euroform = true;
					bLayerInd_RectPipe = true;
					bLayerInd_PinBolt = true;
					bLayerInd_HeadPiece = true;
					//bLayerInd_Props = true;
					bLayerInd_Join = true;
				
				} else if (placingZone.tableformType == 3) {
					bLayerInd_Euroform = true;
					bLayerInd_RectPipe = true;
					bLayerInd_PinBolt = true;
					bLayerInd_HeadPiece = true;
					//bLayerInd_Props = true;
					bLayerInd_Join = true;
					bLayerInd_CrossJointBar = true;
				}

				placingZone.marginTopBasic = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC);		// 상단 여백 (낮은쪽)
				placingZone.marginTopExtra = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA);		// 상단 여백 (높은쪽)

				placingZone.adjustCellsPosition (&placingZone);		// 셀의 위치 교정

			} else if (item == DG_CANCEL) {
				// 아무 작업도 하지 않음
			} else {
				if ((item == placingZone.BUTTON_ADD_HOR) || (item == placingZone.BUTTON_DEL_HOR)) {
					// 정면 - 추가 버튼 클릭
					if (item == placingZone.BUTTON_ADD_HOR) {
						if (placingZone.nCellsInHor < maxCol) {
							// 우측 인코너 버튼을 지우고
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							// 마지막 셀 버튼 오른쪽에 새로운 셀 버튼을 추가하고
							itmPosX = 90 + (70 * placingZone.nCellsInHor);
							itmPosY = 137;
							// 버튼
							placingZone.BUTTON_OBJ [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], "테이블폼");
							DGShowItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor]);

							// 객체 타입 (팝업컨트롤)
							placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "없음");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "테이블폼");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "유로폼");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "휠러스페이서");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "합판");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "각재");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_TOP+1);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor]);

							// 너비 (팝업컨트롤)
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

							// 너비 (팝업컨트롤) - 처음에는 숨김
							placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);

							itmPosX += 70;

							// 우측 인코너 버튼을 오른쪽 끝에 붙임
							// 버튼
							placingZone.BUTTON_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, 137, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_RCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "없음");
							DGShowItem (dialogID, placingZone.BUTTON_RCORNER);
							DGDisableItem (dialogID, placingZone.BUTTON_RCORNER);
							// 객체 타입 (팝업컨트롤)
							placingZone.POPUP_OBJ_TYPE_RCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, 137 - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "없음");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "인코너판넬");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "아웃코너판넬");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "아웃코너앵글");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							// 너비 (Edit컨트롤)
							placingZone.EDITCONTROL_WIDTH_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 137 + 68, 70, 23);
							DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
							DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							++placingZone.nCellsInHor;
						}
					}

					// 정면 - 삭제 버튼 클릭
					else if (item == placingZone.BUTTON_DEL_HOR) {
						if (placingZone.nCellsInHor > 1) {
							// 우측 인코너 버튼을 지우고
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							// 마지막 셀 버튼을 지우고
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor - 1]);

							// 우측 인코너 버튼을 오른쪽 끝에 붙임
							itmPosX = 90 + (70 * (placingZone.nCellsInHor - 1));
							itmPosY = 137;
							// 버튼
							placingZone.BUTTON_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, 137, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_RCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "없음");
							DGShowItem (dialogID, placingZone.BUTTON_RCORNER);
							DGDisableItem (dialogID, placingZone.BUTTON_RCORNER);
							// 객체 타입 (팝업컨트롤)
							placingZone.POPUP_OBJ_TYPE_RCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, 137 - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "없음");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "인코너판넬");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "아웃코너판넬");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "아웃코너앵글");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							// 너비 (Edit컨트롤)
							placingZone.EDITCONTROL_WIDTH_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 137 + 68, 70, 23);
							DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
							DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							--placingZone.nCellsInHor;
						}
					}

				} else if ((item == placingZone.BUTTON_ADD_VER_BASIC) || (item == placingZone.BUTTON_DEL_VER_BASIC)) {
					// 측면 - 추가(L) 버튼 클릭
					if (item == placingZone.BUTTON_ADD_VER_BASIC) {
						if (placingZone.nCellsInVerBasic < maxRow) {
							// 맨 위에 셀 하나 추가
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

					// 측면 - 삭제(L) 버튼 클릭
					else if (item == placingZone.BUTTON_DEL_VER_BASIC) {
						if (placingZone.nCellsInVerBasic > 1) {
							// 맨 위에 있던 셀 하나 제거
							DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic - 1] - 1);

							--placingZone.nCellsInVerBasic;
						}
					}

				} else if ((item == placingZone.BUTTON_ADD_VER_EXTRA) || (item == placingZone.BUTTON_DEL_VER_EXTRA)) {
					// 측면 - 추가(H) 버튼 클릭
					if (item == placingZone.BUTTON_ADD_VER_EXTRA) {
						if (placingZone.nCellsInVerExtra < maxRow) {
							// 맨 위에 셀 하나 추가
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

					// 측면 - 삭제(H) 버튼 클릭
					else if (item == placingZone.BUTTON_DEL_VER_EXTRA) {
						if (placingZone.nCellsInVerExtra > 1) {
							// 맨 위에 있던 셀 하나 제거
							DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra - 1] - 1);

							--placingZone.nCellsInVerExtra;
						}
					}

				} else {
					// 객체 버튼 클릭 (테이블폼인 경우에만 유효함)
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if ((item == placingZone.BUTTON_OBJ [xx]) && (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM)) {
							clickedIndex = xx;
							if (placingZone.bVertical == true) {
								// 테이블폼 타입 (세로 방향)일 경우, 3번째 다이얼로그(세로방향) 열기
								result = DGBlankModalDialog (770, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3_Vertical, (short) 0);
							} else {
								// 테이블폼 타입 (가로 방향)일 경우, 3번째 다이얼로그(가로방향) 열기
								result = DGBlankModalDialog (770, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3_Horizontal, (short) 0);
							}

							// 콤보박스의 전체 너비 값 변경
							accumLength = 0;

							for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
								accumLength += placingZone.cells [xx].tableInHor [yy];

							bool bFoundWidth = false;

							for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH [xx]) ; ++yy) {
								if (accumLength == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], yy).ToCStr ().Get ())) {
									DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], yy);
									bFoundWidth = true;
									break;
								}
							}

							if (bFoundWidth == false) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (accumLength, numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
								DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
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
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
						totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
					else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
						totalWidth += 0.0;
					else
						totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

				// 남은 높이 계산 (낮은쪽)
				totalHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
					totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx])).ToCStr ().Get ()) / 1000;
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

				if (placingZone.bExtra == true) {
					// 남은 높이 계산 (높은쪽)
					totalHeight = 0.0;
					for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
						totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx])).ToCStr ().Get ()) / 1000;
					}
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
				}

				item = 0;
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

// 객체의 레이어를 선택하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	xx;
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "가설재 레이어 선택하기");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// 레이어 관련 라벨
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "슬래브 테이블폼");
			DGSetItemText (dialogID, LABEL_LAYER_PROFILE, "C형강");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "비계 파이프");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트 세트");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "벽체 타이");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "헤드피스");
			DGSetItemText (dialogID, LABEL_LAYER_PROPS, "푸시풀프롭스");
			DGSetItemText (dialogID, LABEL_LAYER_STEELFORM, "스틸폼");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "각재");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, "휠러스페이서");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "아웃코너앵글");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, "아웃코너판넬");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL, "인코너판넬");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "각파이프행거");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "유로폼 후크");
			DGSetItemText (dialogID, LABEL_LAYER_CROSS_JOINT_BAR, "십자조인트바");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_CLAMP, "블루클램프");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL, "블루목심");
			DGSetItemText (dialogID, LABEL_LAYER_HIDDEN, "숨김");

			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 730, 160, 25);
			DGSetItemFont (dialogID, BUTTON_AUTOSET, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_AUTOSET, "레이어 자동 설정");
			DGShowItem (dialogID, BUTTON_AUTOSET);

			// 유저 컨트롤 초기화
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

			ucb.itemID	 = USERCONTROL_LAYER_PROPS;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROPS, 1);
			if (bLayerInd_Props == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PROPS);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PROPS);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PROPS);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PROPS);
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
			if (bLayerInd_Timber == true) {
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

			ucb.itemID	 = USERCONTROL_LAYER_CROSS_JOINT_BAR;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR, 1);
			if (bLayerInd_CrossJointBar == true) {
				DGEnableItem (dialogID, LABEL_LAYER_CROSS_JOINT_BAR);
				DGEnableItem (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_CROSS_JOINT_BAR);
				DGDisableItem (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
			}

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_CLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, 1);
			if (bLayerInd_BlueClamp == true) {
				DGEnableItem (dialogID, LABEL_LAYER_BLUE_CLAMP);
				DGEnableItem (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_BLUE_CLAMP);
				DGDisableItem (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
			}

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_TIMBER_RAIL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, 1);
			if (bLayerInd_BlueTimberRail == true) {
				DGEnableItem (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
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
			// 레이어 같이 바뀜
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				long selectedLayer;

				selectedLayer = DGGetItemValLong (dialogID, item);

				for (xx = USERCONTROL_LAYER_SLABTABLEFORM ; xx <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL ; ++xx)
					DGSetItemValLong (dialogID, xx, selectedLayer);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 레이어 번호 저장
					if (bLayerInd_SlabTableform == true)	layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					if (bLayerInd_Profile == true)			layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE);
					if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					if (bLayerInd_WallTie == true)			layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					if (bLayerInd_Props == true)			layerInd_Props			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROPS);
					if (bLayerInd_Steelform == true)		layerInd_Steelform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM);
					if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					if (bLayerInd_Timber == true)			layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
					if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
					if (bLayerInd_RectpipeHanger == true)	layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
					if (bLayerInd_EuroformHook == true)		layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
					if (bLayerInd_CrossJointBar == true)	layerInd_CrossJointBar	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
					if (bLayerInd_BlueClamp == true)		layerInd_BlueClamp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
					if (bLayerInd_BlueTimberRail == true)	layerInd_BlueTimberRail	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
					if (bLayerInd_Hidden == true)			layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					if (placingZone.tableformType == 1) {
						layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
						layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
						layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
						layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
						layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
						layerInd_Props			= makeTemporaryLayer (structuralObject_forTableformWall, "PUSH", NULL);
						layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);
						layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformWall, "TIMB", NULL);
						layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformWall, "UFCL", NULL);
						layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformWall, "RAIL", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROPS, layerInd_Props);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

					} else if (placingZone.tableformType == 2) {
						layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
						layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
						layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
						layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
						layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
						layerInd_Props			= makeTemporaryLayer (structuralObject_forTableformWall, "PUSH", NULL);
						layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);
						layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformWall, "TIMB", NULL);
						layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformWall, "UFCL", NULL);
						layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformWall, "RAIL", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROPS, layerInd_Props);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

					} else if (placingZone.tableformType == 3) {
						layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
						layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
						layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
						layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
						layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
						layerInd_Props			= makeTemporaryLayer (structuralObject_forTableformWall, "PUSH", NULL);
						layerInd_CrossJointBar	= makeTemporaryLayer (structuralObject_forTableformWall, "CROS", NULL);
						layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);
						layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformWall, "TIMB", NULL);
						layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformWall, "UFCL", NULL);
						layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformWall, "RAIL", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROPS, layerInd_Props);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR, layerInd_CrossJointBar);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);
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

// 테이블폼 세로방향에 대하여 유로폼의 수평 배열을 변경하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	// clickedIndex: 이전 다이얼로그에서 눌린 버튼의 0-기반 인덱스 번호 (BUTTON_OBJ [xx])

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
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 140, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확 인");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 390, 140, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취 소");
			DGShowItem (dialogID, DG_CANCEL);

			// 기존 너비 (라벨)
			accumLength = 0;
			for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
				accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			sprintf (buffer, "기존 너비: %d", accumLength);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, buffer);
			DGShowItem (dialogID, itmIdx);

			// 변경된 너비 (라벨)
			sprintf (buffer, "변경된 너비: %d", 0);
			placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 20, 100, 23);
			DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			itmPosX = 35;
			itmPosY = 55;

			for (xx = 0 ; xx < 10 ; ++xx) {
				// 구분자
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
				DGShowItem (dialogID, itmIdx);

				// 텍스트(유로폼)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "유로폼");
				DGShowItem (dialogID, itmIdx);

				// 콤보박스
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

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 선택한 콤보박스들의 값을 기반으로 구조체 값을 갱신함
					for (xx = 0 ; xx < 10 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}

					accumLength = 0;
					for (xx = 0 ; xx < 10 ; ++xx) {
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

// 테이블폼 가로방향에 대하여 유로폼의 수평 배열을 변경하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	// clickedIndex: 이전 다이얼로그에서 눌린 버튼의 0-기반 인덱스 번호 (BUTTON_OBJ [xx])

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
			DGSetDialogTitle (dialogID, "테이블폼 (가로방향) 배열 설정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 140, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확 인");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 390, 140, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취 소");
			DGShowItem (dialogID, DG_CANCEL);

			// 기존 너비 (라벨)
			accumLength = 0;
			for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
				accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			sprintf (buffer, "기존 너비: %d", accumLength);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, buffer);
			DGShowItem (dialogID, itmIdx);

			// 변경된 너비 (라벨)
			sprintf (buffer, "변경된 너비: %d", 0);
			placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 20, 100, 23);
			DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			itmPosX = 35;
			itmPosY = 55;

			for (xx = 0 ; xx < 10 ; ++xx) {
				// 구분자
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
				DGShowItem (dialogID, itmIdx);

				// 텍스트(유로폼)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "유로폼");
				DGShowItem (dialogID, itmIdx);

				// 콤보박스
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

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// 변경된 너비 (라벨) 업데이트
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "변경된 너비: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 선택한 콤보박스들의 값을 기반으로 구조체 값을 갱신함
					for (xx = 0 ; xx < 10 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}

					accumLength = 0;
					for (xx = 0 ; xx < 10 ; ++xx) {
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

// 벽 상단의 나머지 영역을 유로폼 또는 합판/각재로 채울지 물어보는 다이얼로그
short DGCALLBACK wallTableformPlacerHandler4 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	short	xx;

	double	initPlywoodHeight = 0.0;
	double	changedPlywoodHeight = 0.0;

	// 다이얼로그에서 선택한 정보를 저장
	bool	bEuroform1, bEuroform2;
	bool	bEuroformStandard1, bEuroformStandard2;
	double	euroformWidth1 = 0.0, euroformWidth2 = 0.0;


	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			if (iMarginSide == 1)
				DGSetDialogTitle (dialogID, "벽 상단 채우기 (낮은쪽)");
			else
				DGSetDialogTitle (dialogID, "벽 상단 채우기 (높은쪽)");

			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 70, 240, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "예");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 160, 240, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "아니오");
			DGShowItem (dialogID, DG_CANCEL);

			// 라벨: 설명
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 10, 260, 23);
			DGSetItemFont (dialogID, LABEL_DESC1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DESC1_TOPREST, "벽 상부에 다음 높이 만큼의 공간이 있습니다.");
			DGShowItem (dialogID, LABEL_DESC1_TOPREST);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 40, 50, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT_TOPREST, "높이");
			DGShowItem (dialogID, LABEL_HEIGHT_TOPREST);

			// Edit 컨트롤: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 140, 40-6, 70, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_HEIGHT_TOPREST);
			if (iMarginSide == 1)
				DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_TOPREST, placingZone.marginTopBasic);
			else
				DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_TOPREST, placingZone.marginTopExtra);
			DGDisableItem (dialogID, EDITCONTROL_HEIGHT_TOPREST);

			// 라벨: 설명
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 50, 80, 200, 23);
			DGSetItemFont (dialogID, LABEL_DESC2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DESC2_TOPREST, "유로폼으로 채우시겠습니까?");
			DGShowItem (dialogID, LABEL_DESC2_TOPREST);

			// 라벨: '위'
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 20, 120, 30, 23);
			DGSetItemFont (dialogID, LABEL_UP_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_UP_TOPREST, "위");
			DGShowItem (dialogID, LABEL_UP_TOPREST);

			// 라벨: '↑'
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 20, 150, 30, 23);
			DGSetItemFont (dialogID, LABEL_ARROWUP_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ARROWUP_TOPREST, "↑");
			DGShowItem (dialogID, LABEL_ARROWUP_TOPREST);

			// 라벨: '아래'
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 20, 180, 30, 23);
			DGSetItemFont (dialogID, LABEL_DOWN_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DOWN_TOPREST, "아래");
			DGShowItem (dialogID, LABEL_DOWN_TOPREST);

			// 체크박스: 폼 On/Off (1단 - 맨 아래)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 70, 180-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST, "유로폼");
			DGShowItem (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST);

			// 체크박스: 폼 On/Off (2단 - 중간)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 70, 150-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST, "유로폼");
			DGShowItem (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST);

			// 라벨: 합판
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 120, 70, 23);
			DGSetItemFont (dialogID, LABEL_PLYWOOD_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
			DGShowItem (dialogID, LABEL_PLYWOOD_TOPREST);

			// 체크박스: 규격폼 (1단 - 맨 아래)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 150, 180-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, "규격폼");
			DGShowItem (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST);
			DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, TRUE);

			// 체크박스: 규격폼 (2단 - 중간)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 150, 150-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, "규격폼");
			DGShowItem (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST);
			DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, TRUE);

			// 팝업 컨트롤: 유로폼 (1단) 너비
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

			// 팝업 컨트롤: 유로폼 (2단) 너비
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

			// Edit 컨트롤: 유로폼 (1단) 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 180-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);

			// Edit 컨트롤: 유로폼 (2단) 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 150-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);

			// Edit 컨트롤: 합판 또는 목재 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 120-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_PLYWOOD_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_PLYWOOD_TOPREST);
			DGDisableItem (dialogID, EDITCONTROL_PLYWOOD_TOPREST);

			if (iMarginSide == 1) {
				if (placingZone.marginTopBasic < EPS) {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "없음");
				} else if (placingZone.marginTopBasic <= 0.100 + EPS) {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "목재");
				} else {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
				}
				DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, placingZone.marginTopBasic);
			} else {
				if (placingZone.marginTopExtra < EPS) {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "없음");
				} else if (placingZone.marginTopExtra <= 0.100 + EPS) {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "목재");
				} else {
					DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
				}
				DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, placingZone.marginTopExtra);
			}

			// 체크박스 값에 따른 항목 활성화/비활성화
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

			// 폼의 너비에 따라 합판/목재 영역의 높이가 달라짐
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
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "없음");
			} else if (changedPlywoodHeight <= 0.100 + EPS) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "목재");
			} else {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, changedPlywoodHeight);

			// 체크박스 값에 따른 항목 활성화/비활성화
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
					// 각 유로폼 및 합판 영역의 높이를 계산함
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
					if (iMarginSide == 1)
						for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx)
							placingZone.marginCellsBasic [xx].bFill = false;
					else
						for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx)
							placingZone.marginCellsExtra [xx].bFill = false;

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