#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "WallTableformPlacer.hpp"

using namespace wallTableformPlacerDG;

static WallTableformPlacingZone		placingZone;	// 기본 벽면 영역 정보
static InfoWall						infoWall;		// 벽 객체 정보

static short	layerInd_Euroform;		// 레이어 번호: 유로폼
static short	layerInd_RectPipe;		// 레이어 번호: 비계 파이프
static short	layerInd_PinBolt;		// 레이어 번호: 핀볼트 세트
static short	layerInd_WallTie;		// 레이어 번호: 빅체 타이
static short	layerInd_Clamp;			// 레이어 번호: 직교 클램프
static short	layerInd_HeadPiece;		// 레이어 번호: 헤드피스
static short	layerInd_Join;			// 레이어 번호: 결합철물
static short	layerInd_Plywood;		// 레이어 번호: 합판
static short	layerInd_Wood;			// 레이어 번호: 목재

const GS::uchar_t*	gsmUFOM = L("유로폼v2.0.gsm");
const GS::uchar_t*	gsmSPIP = L("비계파이프v1.0.gsm");
const GS::uchar_t*	gsmPINB = L("핀볼트세트v1.0.gsm");
const GS::uchar_t*	gsmTIE = L("벽체 타이 v1.0.gsm");
const GS::uchar_t*	gsmCLAM = L("직교클램프v1.0.gsm");
const GS::uchar_t*	gsmPUSH = L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm");
const GS::uchar_t*	gsmJOIN = L("결합철물 (사각와셔활용) v1.0.gsm");
const GS::uchar_t*	gsmPLYW = L("합판v1.0.gsm");
const GS::uchar_t*	gsmTIMB = L("목재v1.0.gsm");

static GS::Array<API_Guid>	elemList;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함

// 다이얼로그 동적 요소 인덱스 번호 저장
static short	EDITCONTROL_REMAIN_WIDTH;
static short	POPUP_WIDTH [50];
static short	POPUP_PREFER_WIDTH;
static short	EDITCONTROL_RECT_PIPE_WIDTH;
static short	EDITCONTROL_RECT_PIPE_HEIGHT;

static double	preferWidth;
static bool		clickedPrevButton;		// 이전 버튼을 눌렀습니까?


// 벽에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx;
	double		dx, dy;
	double		width;
	short		tableColumn;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	walls = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	long					nWalls = 0;
	long					nMorphs = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 객체 정보
	InfoMorphForWallTableform	infoMorph;

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
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 벽 (1개), 벽을 덮는 모프 (1개)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 벽 1개, 모프 1개 선택해야 함
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

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("벽을 덮는 모프를 1개 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// (1) 벽 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = walls.Pop ();
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
	infoMorph.guid = elem.header.guid;

	// 모프의 좌하단, 우상단 점 지정
	if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
		// 좌하단 좌표 결정
		infoMorph.leftBottomX = elem.morph.tranmat.tmx [3];
		infoMorph.leftBottomY = elem.morph.tranmat.tmx [7];
		infoMorph.leftBottomZ = elem.morph.tranmat.tmx [11];

		// 우상단 좌표는?
		if (abs (infoMorph.leftBottomX - info3D.bounds.xMin) < EPS)
			infoMorph.rightTopX = info3D.bounds.xMax;
		else
			infoMorph.rightTopX = info3D.bounds.xMin;
		if (abs (infoMorph.leftBottomY - info3D.bounds.yMin) < EPS)
			infoMorph.rightTopY = info3D.bounds.yMax;
		else
			infoMorph.rightTopY = info3D.bounds.yMin;
		if (abs (infoMorph.leftBottomZ - info3D.bounds.zMin) < EPS)
			infoMorph.rightTopZ = info3D.bounds.zMax;
		else
			infoMorph.rightTopZ = info3D.bounds.zMin;
	} else {
		// 우상단 좌표 결정
		infoMorph.rightTopX = elem.morph.tranmat.tmx [3];
		infoMorph.rightTopY = elem.morph.tranmat.tmx [7];
		infoMorph.rightTopZ = elem.morph.tranmat.tmx [11];

		// 좌하단 좌표는?
		if (abs (infoMorph.rightTopX - info3D.bounds.xMin) < EPS)
			infoMorph.leftBottomX = info3D.bounds.xMax;
		else
			infoMorph.leftBottomX = info3D.bounds.xMin;
		if (abs (infoMorph.rightTopY - info3D.bounds.yMin) < EPS)
			infoMorph.leftBottomY = info3D.bounds.yMax;
		else
			infoMorph.leftBottomY = info3D.bounds.yMin;
		if (abs (infoMorph.rightTopZ - info3D.bounds.zMin) < EPS)
			infoMorph.leftBottomZ = info3D.bounds.zMax;
		else
			infoMorph.leftBottomZ = info3D.bounds.zMin;
	}

	// 모프의 Z축 회전 각도 (벽의 설치 각도)
	dx = infoMorph.rightTopX - infoMorph.leftBottomX;
	dy = infoMorph.rightTopY - infoMorph.leftBottomY;
	infoMorph.ang = RadToDegree (atan2 (dy, dx));

	// 모프의 가로 길이
	infoMorph.horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);

	// 모프의 세로 길이
	infoMorph.verLen = abs (info3D.bounds.zMax - info3D.bounds.zMin);

	// 벽면 모프를 통해 영역 정보 업데이트
	placingZone.leftBottomX		= infoMorph.leftBottomX;
	placingZone.leftBottomY		= infoMorph.leftBottomY;
	placingZone.leftBottomZ		= infoMorph.leftBottomZ;
	placingZone.horLen			= infoMorph.horLen;
	placingZone.verLen			= infoMorph.verLen;
	placingZone.ang				= DegreeToRad (infoMorph.ang);
	
	// 작업 층 높이 반영 -- 모프
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_wall = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoWall.floorInd) {
			workLevel_wall = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 영역 정보의 고도 정보를 수정
	placingZone.leftBottomZ = infoWall.bottomOffset;

	// 영역 모프 제거
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

FIRST:

	clickedPrevButton = false;

	// 테이블폼 개수 초기화
	placingZone.n1800w = 0;
	placingZone.n1850w = 0;
	placingZone.n1900w = 0;
	placingZone.n1950w = 0;
	placingZone.n2000w = 0;
	placingZone.n2050w = 0;
	placingZone.n2100w = 0;
	placingZone.n2150w = 0;
	placingZone.n2200w = 0;
	placingZone.n2250w = 0;
	placingZone.n2300w = 0;

	// 테이블폼 개수 계산
	tableColumn = 0;
	width = placingZone.horLen;

	// [DIALOG] 1번째 다이얼로그에서 선호하는 테이블폼 너비를 선택함 (선택한 테이블폼에 대하여 수평, 수직 비계파이프의 길이를 미리 보여줌)
	result = DGBlankModalDialog (500, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler1, 0);

	if (result != DG_OK)
		return err;

	// 선호하는 테이블폼 너비에 따라 테이블 개수를 결정함
	while (width > EPS) {
		if (width + EPS >= preferWidth) {
			if (abs (preferWidth - 2.300) < EPS) {
				placingZone.n2300w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 2.250) < EPS) {
				placingZone.n2250w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 2.200) < EPS) {
				placingZone.n2200w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 2.150) < EPS) {
				placingZone.n2150w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 2.100) < EPS) {
				placingZone.n2100w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 2.050) < EPS) {
				placingZone.n2050w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 2.000) < EPS) {
				placingZone.n2000w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.950) < EPS) {
				placingZone.n1950w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.900) < EPS) {
				placingZone.n1900w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.850) < EPS) {
				placingZone.n1850w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.800) < EPS) {
				placingZone.n1800w ++;
				tableColumn ++;
			}
		} else {
			placingZone.n1800w ++;
			tableColumn ++;
		}
		width -= preferWidth;
	}

	// 남은 길이 저장
	placingZone.remainWidth = width;

	// 셀 개수 저장
	placingZone.nCells = tableColumn;

	// 상단 여백 높이 설정
	if (placingZone.verLen > 6.000 + EPS) {
		placingZone.marginTop = placingZone.verLen - 6.000;
	} else if (placingZone.verLen > 5.700 + EPS) {
		placingZone.marginTop = placingZone.verLen - 5.700;
	} else if (placingZone.verLen > 5.400 + EPS) {
		placingZone.marginTop = placingZone.verLen - 5.400;
	} else if (placingZone.verLen > 5.100 + EPS) {
		placingZone.marginTop = placingZone.verLen - 5.100;
	} else if (placingZone.verLen > 4.800 + EPS) {
		placingZone.marginTop = placingZone.verLen - 4.800;
	} else if (placingZone.verLen > 4.500 + EPS) {
		placingZone.marginTop = placingZone.verLen - 4.500;
	} else if (placingZone.verLen > 4.200 + EPS) {
		placingZone.marginTop = placingZone.verLen - 4.200;
	} else if (placingZone.verLen > 3.900 + EPS) {
		placingZone.marginTop = placingZone.verLen - 3.900;
	} else if (placingZone.verLen > 3.600 + EPS) {
		placingZone.marginTop = placingZone.verLen - 3.600;
	} else if (placingZone.verLen > 3.300 + EPS) {
		placingZone.marginTop = placingZone.verLen - 3.300;
	} else if (placingZone.verLen > 3.000 + EPS) {
		placingZone.marginTop = placingZone.verLen - 3.000;
	} else if (placingZone.verLen > 2.700 + EPS) {
		placingZone.marginTop = placingZone.verLen - 2.700;
	} else if (placingZone.verLen > 2.400 + EPS) {
		placingZone.marginTop = placingZone.verLen - 2.400;
	} else if (placingZone.verLen > 2.100 + EPS) {
		placingZone.marginTop = placingZone.verLen - 2.100;
	} else if (placingZone.verLen > 1.800 + EPS) {
		placingZone.marginTop = placingZone.verLen - 1.800;
	} else if (placingZone.verLen > 1.500 + EPS) {
		placingZone.marginTop = placingZone.verLen - 1.500;
	} else {
		placingZone.marginTop = 0;
	}

	// [DIALOG] 2번째 다이얼로그에서 벽 너비 방향의 테이블폼 수량 및 각 셀의 너비/높이를 설정함
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32517, ACAPI_GetOwnResModule (), wallTableformPlacerHandler2, 0);

	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST;

	// 벽과의 간격으로 인해 정보 업데이트
	infoWall.wallThk		+= (placingZone.gap * 2);

	if (result != DG_OK)
		return err;

	// 셀 위치 및 각도 초기화
	placingZone.initCells (&placingZone);

	// 테이블폼 배치하기
	for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
		err = placingZone.placeTableformOnWall (placingZone.cells [xx]);
	}

	// [DIALOG] 3번째 다이얼로그에서 벽 상단의 자투리 공간을 다른 규격의 유로폼으로 대체할 것인지 묻습니다.
	result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3, 0);

	if (result != DG_OK)
		return err;

	// 벽 상부 남는 영역에 유로폼1단, 유로폼2단, 합판 또는 목재 설치
	for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
		err = placingZone.placeTableformOnWall (placingZone.cells [xx], placingZone.upperCells [xx]);
	}

	return	err;
}

// Cell 배열을 초기화함
void	WallTableformPlacingZone::initCells (WallTableformPlacingZone* placingZone)
{
	short	xx;

	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		placingZone->cells [xx].ang = placingZone->ang;
		placingZone->cells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cells [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cells [xx].leftBottomZ = placingZone->leftBottomZ;

		placingZone->upperCells [xx].ang = placingZone->ang;
		placingZone->upperCells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->upperCells [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->upperCells [xx].leftBottomZ = placingZone->leftBottomZ;
		
		placingZone->upperCells [xx].bFill = false;
		placingZone->upperCells [xx].bEuroform1 = false;
		placingZone->upperCells [xx].bEuroformStandard1 = false;
		placingZone->upperCells [xx].formWidth1 = 0.0;
		placingZone->upperCells [xx].bEuroform2 = false;
		placingZone->upperCells [xx].bEuroformStandard2 = false;
		placingZone->upperCells [xx].formWidth2 = 0.0;
	}
}

// 테이블폼 배치하기
GSErrCode	WallTableformPlacingZone::placeTableformOnWall (CellForWallTableform cell)
{
	GSErrCode	err = NoError;
	placementInfoForWallTableform	placementInfo;

	short		xx, yy;
	double		width, height;
	double		remainder;				// fmod 함수에 쓸 변수
	double		elev_headpiece;
	double		horizontalGap = 0.050;	// 수평재 양쪽 이격거리

	Euroform		params_UFOM;
	SquarePipe		params_SPIP;
	PinBoltSet		params_PINB;
	WallTie			params_TIE;
	//CrossClamp		params_CLAM;
	HeadpieceOfPushPullProps	params_PUSH;
	MetalFittings	params_JOIN;

	placementInfo.nHorEuroform = 0;
	placementInfo.nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		placementInfo.width [xx] = 0.0;
		placementInfo.height [xx] = 0.0;
	}

	if (abs (cell.horLen - 2.300) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 2.250) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 2.200) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 2.150) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 2.100) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 2.050) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 2.000) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.950) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.900) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.850) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.800) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.600;
		horizontalGap = 0.050;
	} else {
		placementInfo.nHorEuroform = 0;
	}

	if (abs (cell.verLen - 6.000) < EPS) {
		placementInfo.nVerEuroform = 5;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 1.200;
		placementInfo.height [4] = 1.200;
	} else if (abs (cell.verLen - 5.700) < EPS) {
		placementInfo.nVerEuroform = 5;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 1.200;
		placementInfo.height [4] = 0.900;
	} else if (abs (cell.verLen - 5.400) < EPS) {
		placementInfo.nVerEuroform = 5;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 0.900;
		placementInfo.height [4] = 0.900;
	} else if (abs (cell.verLen - 5.100) < EPS) {
		placementInfo.nVerEuroform = 5;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 0.900;
		placementInfo.height [4] = 0.600;
	} else if (abs (cell.verLen - 4.800) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 1.200;
	} else if (abs (cell.verLen - 4.500) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 0.900;
	} else if (abs (cell.verLen - 4.200) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.900;
		placementInfo.height [3] = 0.900;
	} else if (abs (cell.verLen - 3.900) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.900;
		placementInfo.height [3] = 0.600;
	} else if (abs (cell.verLen - 3.600) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
	} else if (abs (cell.verLen - 3.300) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.900;
	} else if (abs (cell.verLen - 3.000) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.600;
	} else if (abs (cell.verLen - 2.700) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 0.900;
		placementInfo.height [2] = 0.600;
	} else if (abs (cell.verLen - 2.400) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
	} else if (abs (cell.verLen - 2.100) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 0.900;
	} else if (abs (cell.verLen - 1.800) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.900;
		placementInfo.height [1] = 0.900;
	} else if (abs (cell.verLen - 1.500) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.900;
		placementInfo.height [1] = 0.600;
	} else {
		placementInfo.nVerEuroform = 0;
	}

	// 너비나 높이가 0이면 아무것도 배치하지 않음
	if ((placementInfo.nHorEuroform == 0) || (placementInfo.nVerEuroform == 0))
		return	NoError;

	//////////////////////////////////////////////////////////////// 현재면
	// 유로폼 설치
	params_UFOM.leftBottomX = cell.leftBottomX;
	params_UFOM.leftBottomY = cell.leftBottomY;
	params_UFOM.leftBottomZ = cell.leftBottomZ;
	params_UFOM.ang = cell.ang;

	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		height = 0.0;
		for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
			params_UFOM.width	= placementInfo.width [xx];
			params_UFOM.height	= placementInfo.height [yy];
			height += placementInfo.height [yy];
			elemList.Push (placeUFOM (params_UFOM));	params_UFOM.leftBottomZ = moveZ (params_UFOM.leftBottomZ, placementInfo.height [yy]);
		}
		params_UFOM.leftBottomX = moveXinParallel (params_UFOM.leftBottomX, params_UFOM.ang, placementInfo.width [xx]);
		params_UFOM.leftBottomY = moveYinParallel (params_UFOM.leftBottomY, params_UFOM.ang, placementInfo.width [xx]);
		params_UFOM.leftBottomZ = moveZ (params_UFOM.leftBottomZ, -height);
	}

	// 비계 파이프 (수평) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.horLen - (horizontalGap * 2);
	params_SPIP.pipeAng = DegreeToRad (0);

	params_SPIP.leftBottomX = moveXinPerpend (params_SPIP.leftBottomX, params_SPIP.ang, -(0.0635 + 0.025));
	params_SPIP.leftBottomY = moveYinPerpend (params_SPIP.leftBottomY, params_SPIP.ang, -(0.0635 + 0.025));
	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, horizontalGap);
	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, horizontalGap);
	params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.150 - 0.031);

	for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
		if (xx == 0) {
			// 1행
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.031 - 0.150 + placementInfo.height [xx] - 0.031);
		} else if (xx == placementInfo.nVerEuroform) {
			// 마지막 행
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.150);
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
			elemList.Push (placeSPIP (params_SPIP));
		} else {
			// 나머지 행
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.031 + placementInfo.height [xx] - 0.031);
		}
	}

	// 비계 파이프 (수직) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.verLen - 0.100;
	params_SPIP.pipeAng = DegreeToRad (90);

	params_SPIP.leftBottomX = moveXinPerpend (params_SPIP.leftBottomX, params_SPIP.ang, -(0.0635 + 0.075));
	params_SPIP.leftBottomY = moveYinPerpend (params_SPIP.leftBottomY, params_SPIP.ang, -(0.0635 + 0.075));
	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.450 - 0.035);
	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.450 - 0.035);
	params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.050);

	// 1열
	elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.070);						params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.070);
	elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, cell.horLen - 0.900 - 0.070);	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, cell.horLen - 0.900 - 0.070);
	// 2열
	elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.070);						params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.070);
	elemList.Push (placeSPIP (params_SPIP));

	// 핀볼트 배치 (수평 - 최하단, 최상단)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = TRUE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.1635));
	params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.1635));

	// 최하단 행
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, 0.150);
	width = 0.0;
	for (xx = 0 ; xx < placementInfo.nHorEuroform - 1 ; ++xx) {
		width += placementInfo.width [xx];
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, placementInfo.width [xx]);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, placementInfo.width [xx]);

		elemList.Push (placePINB (params_PINB));
	}
	// 최상단 행
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, -width);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, -width);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, cell.verLen - 0.300);
	for (xx = 0 ; xx < placementInfo.nHorEuroform - 1 ; ++xx) {
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, placementInfo.width [xx]);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, placementInfo.width [xx]);

		elemList.Push (placePINB (params_PINB));
	}

	// 핀볼트 배치 (수평 - 나머지)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = FALSE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.1635));
	params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.1635));

	// 2 ~ [n-1]행
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [0]);
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		width = 0.0;
		for (yy = 0 ; yy < placementInfo.nHorEuroform ; ++yy) {
			// 1열
			if (yy == 0) {
				elemList.Push (placePINB (params_PINB));
				params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
				params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
				width += 0.450;
			// 마지막 열
			} else if (yy == placementInfo.nHorEuroform - 1) {
				width += 0.450;
				params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
				params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
				elemList.Push (placePINB (params_PINB));
			// 나머지 열
			} else {
				width += placementInfo.width [yy];
				if (abs (placementInfo.width [yy] - 0.600) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.300);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.300);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.500) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.200);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.200);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.450) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.400) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.100);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.100);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.200);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.200);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.100);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.100);
				} else if (abs (placementInfo.width [yy] - 0.300) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.200) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.050);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.050);
				}
			}
		}
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, -width);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, -width);
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
	}

	// 핀볼트 배치 (수직)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = FALSE;
	params_PINB.boltLen = 0.150;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.2135));
	params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.2135));
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [0]);

	// 1열
	height = 0.0;
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
		height += placementInfo.height [xx];
	}
	// 2열
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, cell.horLen - 0.900);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, cell.horLen - 0.900);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, -height);
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
		height += placementInfo.height [xx];
	}

	// 벽체 타이
	if (placingZone.bDoubleSide) {
		params_TIE.leftBottomX = cell.leftBottomX;
		params_TIE.leftBottomY = cell.leftBottomY;
		params_TIE.leftBottomZ = cell.leftBottomZ;
		params_TIE.ang = cell.ang;
		remainder = fmod ((infoWall.wallThk + 0.327), 0.100);
		params_TIE.boltLen = (infoWall.wallThk + 0.327 + (0.100 - remainder));
		params_TIE.pipeBeg = 0.0365 + 0.1635;
		params_TIE.pipeEnd = 0.0365 + 0.1635 + infoWall.wallThk;
		params_TIE.clampBeg = 0.0365;
		params_TIE.clampEnd = 0.0365 + infoWall.wallThk + 0.327;

		params_TIE.leftBottomX = moveXinPerpend (params_TIE.leftBottomX, params_TIE.ang, -(0.1635 + 0.0365));
		params_TIE.leftBottomY = moveYinPerpend (params_TIE.leftBottomY, params_TIE.ang, -(0.1635 + 0.0365));
		params_TIE.leftBottomX = moveXinParallel (params_TIE.leftBottomX, params_TIE.ang, 0.450);
		params_TIE.leftBottomY = moveYinParallel (params_TIE.leftBottomY, params_TIE.ang, 0.450);
		params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, 0.270);

		for (xx = 0 ; xx < 2 ; ++xx) {
			for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
				// 최하위 행
				if (yy == 0) {
					elemList.Push (placeTIE (params_TIE));
					params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, -0.270 + placementInfo.height [yy] + 0.150);
		
				// 최상위 행
				} else if (yy == placementInfo.nVerEuroform - 1) {
					params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, placementInfo.height [yy] - 0.150 - 0.230 - 0.050);
					elemList.Push (placeTIE (params_TIE));
					params_TIE.leftBottomX = moveXinParallel (params_TIE.leftBottomX, params_TIE.ang, cell.horLen - 0.900);
					params_TIE.leftBottomY = moveYinParallel (params_TIE.leftBottomY, params_TIE.ang, cell.horLen - 0.900);
					params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, 0.280 - cell.verLen + 0.270);
		
				// 2 ~ [n-1]행
				} else {
					//elemList.Push (placeTIE (params_TIE));
					params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, placementInfo.height [yy]);
				}
			}
		}
	}

	// 직교 클램프
	//params_CLAM.leftBottomX = cell.leftBottomX;
	//params_CLAM.leftBottomY = cell.leftBottomY;
	//params_CLAM.leftBottomZ = cell.leftBottomZ;
	//params_CLAM.ang = cell.ang;
	//params_CLAM.angX = DegreeToRad (0.0);
	//params_CLAM.angY = DegreeToRad (0.0);

	//params_CLAM.leftBottomX = moveXinPerpend (params_CLAM.leftBottomX, params_CLAM.ang, -0.1835);
	//params_CLAM.leftBottomY = moveYinPerpend (params_CLAM.leftBottomY, params_CLAM.ang, -0.1835);
	//params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.450 - 0.035);
	//params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.450 - 0.035);
	//params_CLAM.leftBottomZ = moveZ (params_CLAM.leftBottomZ, 0.099);

	//for (xx = 0 ; xx < 2 ; ++xx) {
	//	elemList.Push (placeCLAM (params_CLAM));
	//	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.070);
	//	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.070);
	//	elemList.Push (placeCLAM (params_CLAM));
	//	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, -0.035 - 0.450 + cell.horLen - 0.450 - 0.035);
	//	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, -0.035 - 0.450 + cell.horLen - 0.450 - 0.035);
	//	elemList.Push (placeCLAM (params_CLAM));
	//	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.070);
	//	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.070);
	//	elemList.Push (placeCLAM (params_CLAM));
	//	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, -0.035 + 0.450 - cell.horLen + 0.450 - 0.035);
	//	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, -0.035 + 0.450 - cell.horLen + 0.450 - 0.035);
	//	params_CLAM.leftBottomZ = moveZ (params_CLAM.leftBottomZ, -0.099 + cell.verLen - 0.099);	params_CLAM.angY = DegreeToRad (180.0);
	//}

	// 헤드 피스
	params_PUSH.leftBottomX = cell.leftBottomX;
	params_PUSH.leftBottomY = cell.leftBottomY;
	params_PUSH.leftBottomZ = cell.leftBottomZ;
	params_PUSH.ang = cell.ang;

	params_PUSH.leftBottomX = moveXinPerpend (params_PUSH.leftBottomX, params_PUSH.ang, -0.1725);
	params_PUSH.leftBottomY = moveYinPerpend (params_PUSH.leftBottomY, params_PUSH.ang, -0.1725);
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, 0.450 - 0.100);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, 0.450 - 0.100);
	params_PUSH.leftBottomZ = moveZ (params_PUSH.leftBottomZ, 0.350);

	// 처음 행
	elemList.Push (placePUSH (params_PUSH));
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, cell.horLen - 0.900);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, cell.horLen - 0.900);
	elemList.Push (placePUSH (params_PUSH));
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, 0.900 - cell.horLen);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, 0.900 - cell.horLen);
	if (cell.verLen > 4.000) {
		elev_headpiece = 4.000 * 0.80;
	} else {
		elev_headpiece = cell.verLen * 0.80;
	}
	params_PUSH.leftBottomZ = moveZ (params_PUSH.leftBottomZ, -0.350 + elev_headpiece);
	// 마지막 행
	elemList.Push (placePUSH (params_PUSH));
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, cell.horLen - 0.900);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, cell.horLen - 0.900);
	elemList.Push (placePUSH (params_PUSH));

	// 결합철물
	params_JOIN.leftBottomX = cell.leftBottomX;
	params_JOIN.leftBottomY = cell.leftBottomY;
	params_JOIN.leftBottomZ = cell.leftBottomZ;
	params_JOIN.ang = cell.ang;

	params_JOIN.leftBottomX = moveXinPerpend (params_JOIN.leftBottomX, params_JOIN.ang, -0.0455);
	params_JOIN.leftBottomY = moveYinPerpend (params_JOIN.leftBottomY, params_JOIN.ang, -0.0455);
	params_JOIN.leftBottomX = moveXinParallel (params_JOIN.leftBottomX, params_JOIN.ang, 0.450);
	params_JOIN.leftBottomY = moveYinParallel (params_JOIN.leftBottomY, params_JOIN.ang, 0.450);
	params_JOIN.leftBottomZ = moveZ (params_JOIN.leftBottomZ, 0.150);

	// 처음 행
	elemList.Push (placeJOIN (params_JOIN));
	params_JOIN.leftBottomX = moveXinParallel (params_JOIN.leftBottomX, params_JOIN.ang, cell.horLen - 0.900);
	params_JOIN.leftBottomY = moveYinParallel (params_JOIN.leftBottomY, params_JOIN.ang, cell.horLen - 0.900);
	elemList.Push (placeJOIN (params_JOIN));
	params_JOIN.leftBottomX = moveXinParallel (params_JOIN.leftBottomX, params_JOIN.ang, 0.900 - cell.horLen);
	params_JOIN.leftBottomY = moveYinParallel (params_JOIN.leftBottomY, params_JOIN.ang, 0.900 - cell.horLen);
	params_JOIN.leftBottomZ = moveZ (params_JOIN.leftBottomZ, cell.verLen - 0.300);

	// 마지막 행
	elemList.Push (placeJOIN (params_JOIN));
	params_JOIN.leftBottomX = moveXinParallel (params_JOIN.leftBottomX, params_JOIN.ang, cell.horLen - 0.900);
	params_JOIN.leftBottomY = moveYinParallel (params_JOIN.leftBottomY, params_JOIN.ang, cell.horLen - 0.900);
	elemList.Push (placeJOIN (params_JOIN));

	// 그룹화하기
	if (!elemList.IsEmpty ()) {
		GSSize nElems = elemList.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList[i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
	}
	elemList.Clear (false);

	//////////////////////////////////////////////////////////////// 반대면
	if (placingZone.bDoubleSide) {
		cell.leftBottomX = moveXinParallel (cell.leftBottomX, cell.ang, cell.horLen);
		cell.leftBottomY = moveYinParallel (cell.leftBottomY, cell.ang, cell.horLen);
		cell.leftBottomX = moveXinPerpend (cell.leftBottomX, cell.ang, infoWall.wallThk);
		cell.leftBottomY = moveYinPerpend (cell.leftBottomY, cell.ang, infoWall.wallThk);
		cell.ang += DegreeToRad (180.0);

		// 유로폼 설치 (반대편에서 변경됨)
		params_UFOM.leftBottomX = cell.leftBottomX;
		params_UFOM.leftBottomY = cell.leftBottomY;
		params_UFOM.leftBottomZ = cell.leftBottomZ;
		params_UFOM.ang = cell.ang;

		for (xx = placementInfo.nHorEuroform - 1 ; xx >= 0 ; --xx) {
			height = 0.0;
			for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
				params_UFOM.width	= placementInfo.width [xx];
				params_UFOM.height	= placementInfo.height [yy];
				height += placementInfo.height [yy];
				elemList.Push (placeUFOM (params_UFOM));	params_UFOM.leftBottomZ = moveZ (params_UFOM.leftBottomZ, placementInfo.height [yy]);
			}
			params_UFOM.leftBottomX = moveXinParallel (params_UFOM.leftBottomX, params_UFOM.ang, placementInfo.width [xx]);
			params_UFOM.leftBottomY = moveYinParallel (params_UFOM.leftBottomY, params_UFOM.ang, placementInfo.width [xx]);
			params_UFOM.leftBottomZ = moveZ (params_UFOM.leftBottomZ, -height);
		}

		// 비계 파이프 (수평) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.horLen - (horizontalGap * 2);
		params_SPIP.pipeAng = DegreeToRad (0);

		params_SPIP.leftBottomX = moveXinPerpend (params_SPIP.leftBottomX, params_SPIP.ang, -(0.0635 + 0.025));
		params_SPIP.leftBottomY = moveYinPerpend (params_SPIP.leftBottomY, params_SPIP.ang, -(0.0635 + 0.025));
		params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, horizontalGap);
		params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, horizontalGap);
		params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.150 - 0.031);

		for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
			if (xx == 0) {
				// 1행
				elemList.Push (placeSPIP (params_SPIP));
				params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
				elemList.Push (placeSPIP (params_SPIP));
				params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.031 - 0.150 + placementInfo.height [xx] - 0.031);
			} else if (xx == placementInfo.nVerEuroform) {
				// 마지막 행
				params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.150);
				elemList.Push (placeSPIP (params_SPIP));
				params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
				elemList.Push (placeSPIP (params_SPIP));
			} else {
				// 나머지 행
				elemList.Push (placeSPIP (params_SPIP));
				params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
				elemList.Push (placeSPIP (params_SPIP));
				params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.031 + placementInfo.height [xx] - 0.031);
			}
		}

		// 비계 파이프 (수직) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.verLen - 0.100;
		params_SPIP.pipeAng = DegreeToRad (90);

		params_SPIP.leftBottomX = moveXinPerpend (params_SPIP.leftBottomX, params_SPIP.ang, -(0.0635 + 0.075));
		params_SPIP.leftBottomY = moveYinPerpend (params_SPIP.leftBottomY, params_SPIP.ang, -(0.0635 + 0.075));
		params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.450 - 0.035);
		params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.450 - 0.035);
		params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.050);

		// 1열
		elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.070);						params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.070);
		elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, cell.horLen - 0.900 - 0.070);	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, cell.horLen - 0.900 - 0.070);
		// 2열
		elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.070);						params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.070);
		elemList.Push (placeSPIP (params_SPIP));

		// 핀볼트 배치 (수평 - 최하단, 최상단) (반대편에서 변경됨)
		params_PINB.leftBottomX = cell.leftBottomX;
		params_PINB.leftBottomY = cell.leftBottomY;
		params_PINB.leftBottomZ = cell.leftBottomZ;
		params_PINB.ang = cell.ang;
		params_PINB.bPinBoltRot90 = TRUE;
		params_PINB.boltLen = 0.100;
		params_PINB.angX = DegreeToRad (270.0);
		params_PINB.angY = DegreeToRad (0.0);

		params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.1635));
		params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.1635));

		// 최하단 행
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, 0.150);
		width = 0.0;
		for (xx = placementInfo.nHorEuroform - 1 ; xx > 0 ; --xx) {
			width += placementInfo.width [xx];
			params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, placementInfo.width [xx]);
			params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, placementInfo.width [xx]);

			elemList.Push (placePINB (params_PINB));
		}
		// 최상단 행
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, -width);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, -width);
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, cell.verLen - 0.300);
		for (xx = placementInfo.nHorEuroform - 1 ; xx > 0 ; --xx) {
			params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, placementInfo.width [xx]);
			params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, placementInfo.width [xx]);

			elemList.Push (placePINB (params_PINB));
		}

		// 핀볼트 배치 (수평 - 나머지) (반대편에서 변경됨)
		params_PINB.leftBottomX = cell.leftBottomX;
		params_PINB.leftBottomY = cell.leftBottomY;
		params_PINB.leftBottomZ = cell.leftBottomZ;
		params_PINB.ang = cell.ang;
		params_PINB.bPinBoltRot90 = FALSE;
		params_PINB.boltLen = 0.100;
		params_PINB.angX = DegreeToRad (270.0);
		params_PINB.angY = DegreeToRad (0.0);

		params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.1635));
		params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.1635));

		// 2 ~ [n-1]행
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [0]);
		for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
			width = 0.0;
			for (yy = placementInfo.nHorEuroform - 1 ; yy >= 0 ; --yy) {
				// 1열
				if (yy == placementInfo.nHorEuroform - 1) {
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
					width += 0.450;
				// 마지막 열
				} else if (yy == 0) {
					width += 0.450;
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
					elemList.Push (placePINB (params_PINB));
				// 나머지 열
				} else {
					width += placementInfo.width [yy];
					if (abs (placementInfo.width [yy] - 0.600) < EPS) {
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.300);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.300);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					} else if (abs (placementInfo.width [yy] - 0.500) < EPS) {
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.200);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.200);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					} else if (abs (placementInfo.width [yy] - 0.450) < EPS) {
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					} else if (abs (placementInfo.width [yy] - 0.400) < EPS) {
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.100);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.100);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.200);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.200);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.100);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.100);
					} else if (abs (placementInfo.width [yy] - 0.300) < EPS) {
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					} else if (abs (placementInfo.width [yy] - 0.200) < EPS) {
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.050);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.050);
						elemList.Push (placePINB (params_PINB));
						params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
						params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					}
				}
			}
			params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, -width);
			params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, -width);
			params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
		}

		// 핀볼트 배치 (수직)
		params_PINB.leftBottomX = cell.leftBottomX;
		params_PINB.leftBottomY = cell.leftBottomY;
		params_PINB.leftBottomZ = cell.leftBottomZ;
		params_PINB.ang = cell.ang;
		params_PINB.bPinBoltRot90 = FALSE;
		params_PINB.boltLen = 0.150;
		params_PINB.angX = DegreeToRad (270.0);
		params_PINB.angY = DegreeToRad (0.0);

		params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.2135));
		params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.2135));
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [0]);

		// 1열
		height = 0.0;
		for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
			elemList.Push (placePINB (params_PINB));
			params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
			height += placementInfo.height [xx];
		}
		// 2열
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, cell.horLen - 0.900);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, cell.horLen - 0.900);
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, -height);
		for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
			elemList.Push (placePINB (params_PINB));
			params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
			height += placementInfo.height [xx];
		}

		// 벽체 타이 (현재면에서 했으므로 생략)

		// 직교 클램프
		//params_CLAM.leftBottomX = cell.leftBottomX;
		//params_CLAM.leftBottomY = cell.leftBottomY;
		//params_CLAM.leftBottomZ = cell.leftBottomZ;
		//params_CLAM.ang = cell.ang;
		//params_CLAM.angX = DegreeToRad (0.0);
		//params_CLAM.angY = DegreeToRad (0.0);

		//params_CLAM.leftBottomX = moveXinPerpend (params_CLAM.leftBottomX, params_CLAM.ang, -0.1835);
		//params_CLAM.leftBottomY = moveYinPerpend (params_CLAM.leftBottomY, params_CLAM.ang, -0.1835);
		//params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.450 - 0.035);
		//params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.450 - 0.035);
		//params_CLAM.leftBottomZ = moveZ (params_CLAM.leftBottomZ, 0.099);

		//for (xx = 0 ; xx < 2 ; ++xx) {
		//	elemList.Push (placeCLAM (params_CLAM));
		//	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.070);
		//	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.070);
		//	elemList.Push (placeCLAM (params_CLAM));
		//	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, -0.035 - 0.450 + cell.horLen - 0.450 - 0.035);
		//	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, -0.035 - 0.450 + cell.horLen - 0.450 - 0.035);
		//	elemList.Push (placeCLAM (params_CLAM));
		//	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.070);
		//	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.070);
		//	elemList.Push (placeCLAM (params_CLAM));
		//	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, -0.035 + 0.450 - cell.horLen + 0.450 - 0.035);
		//	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, -0.035 + 0.450 - cell.horLen + 0.450 - 0.035);
		//	params_CLAM.leftBottomZ = moveZ (params_CLAM.leftBottomZ, -0.099 + cell.verLen - 0.099);	params_CLAM.angY = DegreeToRad (180.0);
		//}

		// 헤드 피스
		params_PUSH.leftBottomX = cell.leftBottomX;
		params_PUSH.leftBottomY = cell.leftBottomY;
		params_PUSH.leftBottomZ = cell.leftBottomZ;
		params_PUSH.ang = cell.ang;

		params_PUSH.leftBottomX = moveXinPerpend (params_PUSH.leftBottomX, params_PUSH.ang, -0.1725);
		params_PUSH.leftBottomY = moveYinPerpend (params_PUSH.leftBottomY, params_PUSH.ang, -0.1725);
		params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, 0.450 - 0.100);
		params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, 0.450 - 0.100);
		params_PUSH.leftBottomZ = moveZ (params_PUSH.leftBottomZ, 0.350);

		// 처음 행
		elemList.Push (placePUSH (params_PUSH));
		params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, cell.horLen - 0.900);
		params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, cell.horLen - 0.900);
		elemList.Push (placePUSH (params_PUSH));
		params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, 0.900 - cell.horLen);
		params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, 0.900 - cell.horLen);
		if (cell.verLen > 4.000) {
			elev_headpiece = 4.000 * 0.80;
		} else {
			elev_headpiece = cell.verLen * 0.80;
		}
		params_PUSH.leftBottomZ = moveZ (params_PUSH.leftBottomZ, -0.350 + elev_headpiece);
		// 마지막 행
		elemList.Push (placePUSH (params_PUSH));
		params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, cell.horLen - 0.900);
		params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, cell.horLen - 0.900);
		elemList.Push (placePUSH (params_PUSH));

		// 결합철물
		params_JOIN.leftBottomX = cell.leftBottomX;
		params_JOIN.leftBottomY = cell.leftBottomY;
		params_JOIN.leftBottomZ = cell.leftBottomZ;
		params_JOIN.ang = cell.ang;

		params_JOIN.leftBottomX = moveXinPerpend (params_JOIN.leftBottomX, params_JOIN.ang, -0.0455);
		params_JOIN.leftBottomY = moveYinPerpend (params_JOIN.leftBottomY, params_JOIN.ang, -0.0455);
		params_JOIN.leftBottomX = moveXinParallel (params_JOIN.leftBottomX, params_JOIN.ang, 0.450);
		params_JOIN.leftBottomY = moveYinParallel (params_JOIN.leftBottomY, params_JOIN.ang, 0.450);
		params_JOIN.leftBottomZ = moveZ (params_JOIN.leftBottomZ, 0.150);

		// 처음 행
		elemList.Push (placeJOIN (params_JOIN));
		params_JOIN.leftBottomX = moveXinParallel (params_JOIN.leftBottomX, params_JOIN.ang, cell.horLen - 0.900);
		params_JOIN.leftBottomY = moveYinParallel (params_JOIN.leftBottomY, params_JOIN.ang, cell.horLen - 0.900);
		elemList.Push (placeJOIN (params_JOIN));
		params_JOIN.leftBottomX = moveXinParallel (params_JOIN.leftBottomX, params_JOIN.ang, 0.900 - cell.horLen);
		params_JOIN.leftBottomY = moveYinParallel (params_JOIN.leftBottomY, params_JOIN.ang, 0.900 - cell.horLen);
		params_JOIN.leftBottomZ = moveZ (params_JOIN.leftBottomZ, cell.verLen - 0.300);

		// 마지막 행
		elemList.Push (placeJOIN (params_JOIN));
		params_JOIN.leftBottomX = moveXinParallel (params_JOIN.leftBottomX, params_JOIN.ang, cell.horLen - 0.900);
		params_JOIN.leftBottomY = moveYinParallel (params_JOIN.leftBottomY, params_JOIN.ang, cell.horLen - 0.900);
		elemList.Push (placeJOIN (params_JOIN));

		// 그룹화하기
		if (!elemList.IsEmpty ()) {
			GSSize nElems = elemList.GetSize ();
			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
			if (elemHead != NULL) {
				for (GSIndex i = 0; i < nElems; i++)
					(*elemHead)[i].guid = elemList[i];

				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

				BMKillHandle ((GSHandle *) &elemHead);
			}
		}
		elemList.Clear (false);
	}

	return	err;
}

// 테이블폼 상단 배치하기
GSErrCode	WallTableformPlacingZone::placeTableformOnWall (CellForWallTableform cell, UpperCellForWallTableform upperCell)
{
	GSErrCode	err = NoError;
	short	xx;
	double	remainWidth = abs (placingZone.marginTop - upperCell.formWidth1 - upperCell.formWidth2);
	placementInfoForWallTableform	placementInfo;

	Euroform	params_UFOM1 [4];
	Euroform	params_UFOM2 [4];
	Plywood		params_PLYW [4];
	Wood		params_TIMB [4];


	// 상단 여백을 채우기로 한 경우
	if (upperCell.bFill == true) {
		if (abs (cell.horLen - 2.300) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 2.250) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 2.200) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 2.150) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 2.100) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 2.050) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 2.000) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 1.950) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 1.900) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 1.850) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
		} else if (abs (cell.horLen - 1.800) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.600;
		} else {
			placementInfo.nHorEuroform = 0;
		}

		//////////////////////////////////////////////////////////////// 현재면
		// 1열
		if (placementInfo.nHorEuroform >= 1) {
			// 1번째 행: 유로폼
			params_UFOM1 [0].leftBottomX = cell.leftBottomX;
			params_UFOM1 [0].leftBottomY = cell.leftBottomY;
			params_UFOM1 [0].leftBottomX = moveXinParallel (params_UFOM1 [0].leftBottomX, params_UFOM1 [0].ang, placementInfo.width [0]);
			params_UFOM1 [0].leftBottomY = moveYinParallel (params_UFOM1 [0].leftBottomY, params_UFOM1 [0].ang, placementInfo.width [0]);
			params_UFOM1 [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [0].ang = cell.ang;
			params_UFOM1 [0].width = upperCell.formWidth1;
			params_UFOM1 [0].height = placementInfo.width [0];

			// 2번째 행: 유로폼
			params_UFOM2 [0].leftBottomX = cell.leftBottomX;
			params_UFOM2 [0].leftBottomY = cell.leftBottomY;
			params_UFOM2 [0].leftBottomX = moveXinParallel (params_UFOM2 [0].leftBottomX, params_UFOM2 [0].ang, placementInfo.width [0]);
			params_UFOM2 [0].leftBottomY = moveYinParallel (params_UFOM2 [0].leftBottomY, params_UFOM2 [0].ang, placementInfo.width [0]);
			params_UFOM2 [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [0].ang = cell.ang;
			params_UFOM2 [0].width = upperCell.formWidth2;
			params_UFOM2 [0].height = placementInfo.width [0];

			// 3번째 행: 합판 또는 목재
			params_PLYW [0].leftBottomX = cell.leftBottomX;
			params_PLYW [0].leftBottomY = cell.leftBottomY;
			params_PLYW [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [0].ang = cell.ang;
			params_PLYW [0].p_wid = remainWidth;
			params_PLYW [0].p_leng = placementInfo.width [0];
			params_PLYW [0].w_dir_wall = false;

			params_TIMB [0].leftBottomX = cell.leftBottomX;
			params_TIMB [0].leftBottomY = cell.leftBottomY;
			params_TIMB [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [0].ang = cell.ang;
			params_TIMB [0].w_w = 0.040;
			params_TIMB [0].w_h = remainWidth;
			params_TIMB [0].w_leng = placementInfo.width [0];
			params_TIMB [0].w_ang = 0.0;
		}

		// 2열
		if (placementInfo.nHorEuroform >= 2) {
			// 1번째 행: 유로폼
			params_UFOM1 [1].leftBottomX = moveXinParallel (params_UFOM1 [0].leftBottomX, params_UFOM1 [0].ang, placementInfo.width [1]);
			params_UFOM1 [1].leftBottomY = moveYinParallel (params_UFOM1 [0].leftBottomY, params_UFOM1 [0].ang, placementInfo.width [1]);
			params_UFOM1 [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [1].ang = cell.ang;
			params_UFOM1 [1].width = upperCell.formWidth1;
			params_UFOM1 [1].height = placementInfo.width [1];

			// 2번째 행: 유로폼
			params_UFOM2 [1].leftBottomX = moveXinParallel (params_UFOM2 [0].leftBottomX, params_UFOM2 [0].ang, placementInfo.width [1]);
			params_UFOM2 [1].leftBottomY = moveYinParallel (params_UFOM2 [0].leftBottomY, params_UFOM2 [0].ang, placementInfo.width [1]);
			params_UFOM2 [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [1].ang = cell.ang;
			params_UFOM2 [1].width = upperCell.formWidth2;
			params_UFOM2 [1].height = placementInfo.width [1];

			// 3번째 행: 합판 또는 목재
			params_PLYW [1].leftBottomX = moveXinParallel (params_PLYW [0].leftBottomX, params_PLYW [0].ang, placementInfo.width [0]);
			params_PLYW [1].leftBottomY = moveYinParallel (params_PLYW [0].leftBottomY, params_PLYW [0].ang, placementInfo.width [0]);
			params_PLYW [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [1].ang = cell.ang;
			params_PLYW [1].p_wid = remainWidth;
			params_PLYW [1].p_leng = placementInfo.width [1];
			params_PLYW [1].w_dir_wall = false;

			params_TIMB [1].leftBottomX = moveXinParallel (params_TIMB [0].leftBottomX, params_TIMB [0].ang, placementInfo.width [0]);
			params_TIMB [1].leftBottomY = moveYinParallel (params_TIMB [0].leftBottomY, params_TIMB [0].ang, placementInfo.width [0]);
			params_TIMB [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [1].ang = cell.ang;
			params_TIMB [1].w_w = 0.040;
			params_TIMB [1].w_h = remainWidth;
			params_TIMB [1].w_leng = placementInfo.width [1];
			params_TIMB [1].w_ang = 0.0;
		}

		// 3열
		if (placementInfo.nHorEuroform >= 3) {
			// 1번째 행: 유로폼
			params_UFOM1 [2].leftBottomX = moveXinParallel (params_UFOM1 [1].leftBottomX, params_UFOM1 [1].ang, placementInfo.width [2]);
			params_UFOM1 [2].leftBottomY = moveYinParallel (params_UFOM1 [1].leftBottomY, params_UFOM1 [1].ang, placementInfo.width [2]);
			params_UFOM1 [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [2].ang = cell.ang;
			params_UFOM1 [2].width = upperCell.formWidth1;
			params_UFOM1 [2].height = placementInfo.width [2];

			// 2번째 행: 유로폼
			params_UFOM2 [2].leftBottomX = moveXinParallel (params_UFOM2 [1].leftBottomX, params_UFOM2 [1].ang, placementInfo.width [2]);
			params_UFOM2 [2].leftBottomY = moveYinParallel (params_UFOM2 [1].leftBottomY, params_UFOM2 [1].ang, placementInfo.width [2]);
			params_UFOM2 [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [2].ang = cell.ang;
			params_UFOM2 [2].width = upperCell.formWidth2;
			params_UFOM2 [2].height = placementInfo.width [2];

			// 3번째 행: 합판 또는 목재
			params_PLYW [2].leftBottomX = moveXinParallel (params_PLYW [1].leftBottomX, params_PLYW [1].ang, placementInfo.width [1]);
			params_PLYW [2].leftBottomY = moveYinParallel (params_PLYW [1].leftBottomY, params_PLYW [1].ang, placementInfo.width [1]);
			params_PLYW [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [2].ang = cell.ang;
			params_PLYW [2].p_wid = remainWidth;
			params_PLYW [2].p_leng = placementInfo.width [2];
			params_PLYW [2].w_dir_wall = false;

			params_TIMB [2].leftBottomX = moveXinParallel (params_TIMB [1].leftBottomX, params_TIMB [1].ang, placementInfo.width [1]);
			params_TIMB [2].leftBottomY = moveYinParallel (params_TIMB [1].leftBottomY, params_TIMB [1].ang, placementInfo.width [1]);
			params_TIMB [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [2].ang = cell.ang;
			params_TIMB [2].w_w = 0.040;
			params_TIMB [2].w_h = remainWidth;
			params_TIMB [2].w_leng = placementInfo.width [2];
			params_TIMB [2].w_ang = 0.0;
		}

		// 4열
		if (placementInfo.nHorEuroform >= 4) {
			// 1번째 행: 유로폼
			params_UFOM1 [3].leftBottomX = moveXinParallel (params_UFOM1 [2].leftBottomX, params_UFOM1 [2].ang, placementInfo.width [3]);
			params_UFOM1 [3].leftBottomY = moveYinParallel (params_UFOM1 [2].leftBottomY, params_UFOM1 [2].ang, placementInfo.width [3]);
			params_UFOM1 [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [3].ang = cell.ang;
			params_UFOM1 [3].width = upperCell.formWidth1;
			params_UFOM1 [3].height = placementInfo.width [3];

			// 2번째 행: 유로폼
			params_UFOM2 [3].leftBottomX = moveXinParallel (params_UFOM2 [2].leftBottomX, params_UFOM2 [2].ang, placementInfo.width [3]);
			params_UFOM2 [3].leftBottomY = moveYinParallel (params_UFOM2 [2].leftBottomY, params_UFOM2 [2].ang, placementInfo.width [3]);
			params_UFOM2 [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [3].ang = cell.ang;
			params_UFOM2 [3].width = upperCell.formWidth2;
			params_UFOM2 [3].height = placementInfo.width [3];

			// 3번째 행: 합판 또는 목재
			params_PLYW [3].leftBottomX = moveXinParallel (params_PLYW [2].leftBottomX, params_PLYW [2].ang, placementInfo.width [2]);
			params_PLYW [3].leftBottomY = moveYinParallel (params_PLYW [2].leftBottomY, params_PLYW [2].ang, placementInfo.width [2]);
			params_PLYW [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [3].ang = cell.ang;
			params_PLYW [3].p_wid = remainWidth;
			params_PLYW [3].p_leng = placementInfo.width [3];
			params_PLYW [3].w_dir_wall = false;

			params_TIMB [3].leftBottomX = moveXinParallel (params_TIMB [2].leftBottomX, params_TIMB [2].ang, placementInfo.width [2]);
			params_TIMB [3].leftBottomY = moveYinParallel (params_TIMB [2].leftBottomY, params_TIMB [2].ang, placementInfo.width [2]);
			params_TIMB [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [3].ang = cell.ang;
			params_TIMB [3].w_w = 0.040;
			params_TIMB [3].w_h = remainWidth;
			params_TIMB [3].w_leng = placementInfo.width [3];
			params_TIMB [3].w_ang = 0.0;
		}

		for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
			if (upperCell.bEuroform1) {
				elemList.Push (placeUFOM_up (params_UFOM1 [xx]));
			}
			if (upperCell.bEuroform2) {
				elemList.Push (placeUFOM_up (params_UFOM2 [xx]));
			}

			// 합판의 경우
			if (remainWidth > 0.110 - EPS) {
				elemList.Push (placePLYW (params_PLYW [xx]));

			// 목재의 경우
			} else if (remainWidth + EPS > 0) {

				elemList.Push (placeTIMB (params_TIMB [xx]));
			}
		}

		// 그룹화하기
		if (!elemList.IsEmpty ()) {
			GSSize nElems = elemList.GetSize ();
			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
			if (elemHead != NULL) {
				for (GSIndex i = 0; i < nElems; i++)
					(*elemHead)[i].guid = elemList[i];

				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

				BMKillHandle ((GSHandle *) &elemHead);
			}
		}
		elemList.Clear (false);

		//////////////////////////////////////////////////////////////// 반대면
		if (placingZone.bDoubleSide) {
			// 1열
			if (placementInfo.nHorEuroform >= 1) {
				// 1번째 행: 유로폼
				params_UFOM1 [0].leftBottomX = moveXinParallel (params_UFOM1 [0].leftBottomX, cell.ang, -placementInfo.width [0]);
				params_UFOM1 [0].leftBottomY = moveYinParallel (params_UFOM1 [0].leftBottomY, cell.ang, -placementInfo.width [0]);
				params_UFOM1 [0].leftBottomX = moveXinPerpend (params_UFOM1 [0].leftBottomX, cell.ang, infoWall.wallThk);
				params_UFOM1 [0].leftBottomY = moveYinPerpend (params_UFOM1 [0].leftBottomY, cell.ang, infoWall.wallThk);
				params_UFOM1 [0].ang += DegreeToRad (180.0);

				// 2번째 행: 유로폼
				params_UFOM2 [0].leftBottomX = moveXinParallel (params_UFOM2 [0].leftBottomX, cell.ang, -placementInfo.width [0]);
				params_UFOM2 [0].leftBottomY = moveYinParallel (params_UFOM2 [0].leftBottomY, cell.ang, -placementInfo.width [0]);
				params_UFOM2 [0].leftBottomX = moveXinPerpend (params_UFOM2 [0].leftBottomX, cell.ang, infoWall.wallThk);
				params_UFOM2 [0].leftBottomY = moveYinPerpend (params_UFOM2 [0].leftBottomY, cell.ang, infoWall.wallThk);
				params_UFOM2 [0].ang += DegreeToRad (180.0);

				// 3번째 행: 합판 또는 목재
				params_PLYW [0].leftBottomX = moveXinParallel (params_PLYW [0].leftBottomX, cell.ang, placementInfo.width [0]);
				params_PLYW [0].leftBottomY = moveYinParallel (params_PLYW [0].leftBottomY, cell.ang, placementInfo.width [0]);
				params_PLYW [0].leftBottomX = moveXinPerpend (params_PLYW [0].leftBottomX, cell.ang, infoWall.wallThk);
				params_PLYW [0].leftBottomY = moveYinPerpend (params_PLYW [0].leftBottomY, cell.ang, infoWall.wallThk);
				params_PLYW [0].ang += DegreeToRad (180.0);

				params_TIMB [0].leftBottomX = moveXinParallel (params_TIMB [0].leftBottomX, cell.ang, placementInfo.width [0]);
				params_TIMB [0].leftBottomY = moveYinParallel (params_TIMB [0].leftBottomY, cell.ang, placementInfo.width [0]);
				params_TIMB [0].leftBottomX = moveXinPerpend (params_TIMB [0].leftBottomX, cell.ang, infoWall.wallThk);
				params_TIMB [0].leftBottomY = moveYinPerpend (params_TIMB [0].leftBottomY, cell.ang, infoWall.wallThk);
				params_TIMB [0].ang += DegreeToRad (180.0);
			}

			// 2열
			if (placementInfo.nHorEuroform >= 2) {
				// 1번째 행: 유로폼
				params_UFOM1 [1].leftBottomX = moveXinParallel (params_UFOM1 [1].leftBottomX, cell.ang, -placementInfo.width [1]);
				params_UFOM1 [1].leftBottomY = moveYinParallel (params_UFOM1 [1].leftBottomY, cell.ang, -placementInfo.width [1]);
				params_UFOM1 [1].leftBottomX = moveXinPerpend (params_UFOM1 [1].leftBottomX, cell.ang, infoWall.wallThk);
				params_UFOM1 [1].leftBottomY = moveYinPerpend (params_UFOM1 [1].leftBottomY, cell.ang, infoWall.wallThk);
				params_UFOM1 [1].ang += DegreeToRad (180.0);

				// 2번째 행: 유로폼
				params_UFOM2 [1].leftBottomX = moveXinParallel (params_UFOM2 [1].leftBottomX, cell.ang, -placementInfo.width [1]);
				params_UFOM2 [1].leftBottomY = moveYinParallel (params_UFOM2 [1].leftBottomY, cell.ang, -placementInfo.width [1]);
				params_UFOM2 [1].leftBottomX = moveXinPerpend (params_UFOM2 [1].leftBottomX, cell.ang, infoWall.wallThk);
				params_UFOM2 [1].leftBottomY = moveYinPerpend (params_UFOM2 [1].leftBottomY, cell.ang, infoWall.wallThk);
				params_UFOM2 [1].ang += DegreeToRad (180.0);

				// 3번째 행: 합판 또는 목재
				params_PLYW [1].leftBottomX = moveXinParallel (params_PLYW [1].leftBottomX, cell.ang, placementInfo.width [1]);
				params_PLYW [1].leftBottomY = moveYinParallel (params_PLYW [1].leftBottomY, cell.ang, placementInfo.width [1]);
				params_PLYW [1].leftBottomX = moveXinPerpend (params_PLYW [1].leftBottomX, cell.ang, infoWall.wallThk);
				params_PLYW [1].leftBottomY = moveYinPerpend (params_PLYW [1].leftBottomY, cell.ang, infoWall.wallThk);
				params_PLYW [1].ang += DegreeToRad (180.0);

				params_TIMB [1].leftBottomX = moveXinParallel (params_TIMB [1].leftBottomX, cell.ang, placementInfo.width [1]);
				params_TIMB [1].leftBottomY = moveYinParallel (params_TIMB [1].leftBottomY, cell.ang, placementInfo.width [1]);
				params_TIMB [1].leftBottomX = moveXinPerpend (params_TIMB [1].leftBottomX, cell.ang, infoWall.wallThk);
				params_TIMB [1].leftBottomY = moveYinPerpend (params_TIMB [1].leftBottomY, cell.ang, infoWall.wallThk);
				params_TIMB [1].ang += DegreeToRad (180.0);
			}

			// 3열
			if (placementInfo.nHorEuroform >= 3) {
				// 1번째 행: 유로폼
				params_UFOM1 [2].leftBottomX = moveXinParallel (params_UFOM1 [2].leftBottomX, cell.ang, -placementInfo.width [2]);
				params_UFOM1 [2].leftBottomY = moveYinParallel (params_UFOM1 [2].leftBottomY, cell.ang, -placementInfo.width [2]);
				params_UFOM1 [2].leftBottomX = moveXinPerpend (params_UFOM1 [2].leftBottomX, cell.ang, infoWall.wallThk);
				params_UFOM1 [2].leftBottomY = moveYinPerpend (params_UFOM1 [2].leftBottomY, cell.ang, infoWall.wallThk);
				params_UFOM1 [2].ang += DegreeToRad (180.0);

				// 2번째 행: 유로폼
				params_UFOM2 [2].leftBottomX = moveXinParallel (params_UFOM2 [2].leftBottomX, cell.ang, -placementInfo.width [2]);
				params_UFOM2 [2].leftBottomY = moveYinParallel (params_UFOM2 [2].leftBottomY, cell.ang, -placementInfo.width [2]);
				params_UFOM2 [2].leftBottomX = moveXinPerpend (params_UFOM2 [2].leftBottomX, cell.ang, infoWall.wallThk);
				params_UFOM2 [2].leftBottomY = moveYinPerpend (params_UFOM2 [2].leftBottomY, cell.ang, infoWall.wallThk);
				params_UFOM2 [2].ang += DegreeToRad (180.0);

				// 3번째 행: 합판 또는 목재
				params_PLYW [2].leftBottomX = moveXinParallel (params_PLYW [2].leftBottomX, cell.ang, placementInfo.width [2]);
				params_PLYW [2].leftBottomY = moveYinParallel (params_PLYW [2].leftBottomY, cell.ang, placementInfo.width [2]);
				params_PLYW [2].leftBottomX = moveXinPerpend (params_PLYW [2].leftBottomX, cell.ang, infoWall.wallThk);
				params_PLYW [2].leftBottomY = moveYinPerpend (params_PLYW [2].leftBottomY, cell.ang, infoWall.wallThk);
				params_PLYW [2].ang += DegreeToRad (180.0);

				params_TIMB [2].leftBottomX = moveXinParallel (params_TIMB [2].leftBottomX, cell.ang, placementInfo.width [2]);
				params_TIMB [2].leftBottomY = moveYinParallel (params_TIMB [2].leftBottomY, cell.ang, placementInfo.width [2]);
				params_TIMB [2].leftBottomX = moveXinPerpend (params_TIMB [2].leftBottomX, cell.ang, infoWall.wallThk);
				params_TIMB [2].leftBottomY = moveYinPerpend (params_TIMB [2].leftBottomY, cell.ang, infoWall.wallThk);
				params_TIMB [2].ang += DegreeToRad (180.0);
			}

			// 4열
			if (placementInfo.nHorEuroform >= 4) {
				// 1번째 행: 유로폼
				params_UFOM1 [3].leftBottomX = moveXinParallel (params_UFOM1 [3].leftBottomX, cell.ang, -placementInfo.width [3]);
				params_UFOM1 [3].leftBottomY = moveYinParallel (params_UFOM1 [3].leftBottomY, cell.ang, -placementInfo.width [3]);
				params_UFOM1 [3].leftBottomX = moveXinPerpend (params_UFOM1 [3].leftBottomX, cell.ang, infoWall.wallThk);
				params_UFOM1 [3].leftBottomY = moveYinPerpend (params_UFOM1 [3].leftBottomY, cell.ang, infoWall.wallThk);
				params_UFOM1 [3].ang += DegreeToRad (180.0);

				// 2번째 행: 유로폼
				params_UFOM2 [3].leftBottomX = moveXinParallel (params_UFOM2 [3].leftBottomX, cell.ang, -placementInfo.width [3]);
				params_UFOM2 [3].leftBottomY = moveYinParallel (params_UFOM2 [3].leftBottomY, cell.ang, -placementInfo.width [3]);
				params_UFOM2 [3].leftBottomX = moveXinPerpend (params_UFOM2 [3].leftBottomX, cell.ang, infoWall.wallThk);
				params_UFOM2 [3].leftBottomY = moveYinPerpend (params_UFOM2 [3].leftBottomY, cell.ang, infoWall.wallThk);
				params_UFOM2 [3].ang += DegreeToRad (180.0);

				// 3번째 행: 합판 또는 목재
				params_PLYW [3].leftBottomX = moveXinParallel (params_PLYW [3].leftBottomX, cell.ang, placementInfo.width [3]);
				params_PLYW [3].leftBottomY = moveYinParallel (params_PLYW [3].leftBottomY, cell.ang, placementInfo.width [3]);
				params_PLYW [3].leftBottomX = moveXinPerpend (params_PLYW [3].leftBottomX, cell.ang, infoWall.wallThk);
				params_PLYW [3].leftBottomY = moveYinPerpend (params_PLYW [3].leftBottomY, cell.ang, infoWall.wallThk);
				params_PLYW [3].ang += DegreeToRad (180.0);

				params_TIMB [3].leftBottomX = moveXinParallel (params_TIMB [3].leftBottomX, cell.ang, placementInfo.width [3]);
				params_TIMB [3].leftBottomY = moveYinParallel (params_TIMB [3].leftBottomY, cell.ang, placementInfo.width [3]);
				params_TIMB [3].leftBottomX = moveXinPerpend (params_TIMB [3].leftBottomX, cell.ang, infoWall.wallThk);
				params_TIMB [3].leftBottomY = moveYinPerpend (params_TIMB [3].leftBottomY, cell.ang, infoWall.wallThk);
				params_TIMB [3].ang += DegreeToRad (180.0);
			}

			for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
				if (upperCell.bEuroform1) {
					elemList.Push (placeUFOM_up (params_UFOM1 [xx]));
				}
				if (upperCell.bEuroform2) {
					elemList.Push (placeUFOM_up (params_UFOM2 [xx]));
				}

				// 합판의 경우
				if (remainWidth > 0.110 - EPS) {
					elemList.Push (placePLYW (params_PLYW [xx]));

				// 목재의 경우
				} else if (remainWidth + EPS > 0) {

					elemList.Push (placeTIMB (params_TIMB [xx]));
				}
			}

			// 그룹화하기
			if (!elemList.IsEmpty ()) {
				GSSize nElems = elemList.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList[i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
			}
			elemList.Clear (false);
		}
	}

	return	err;
}

// 해당 셀의 좌하단 좌표X 위치를 리턴
double	WallTableformPlacingZone::getCellPositionLeftBottomX (WallTableformPlacingZone *placingZone, short idx)
{
	double		distance = 0.0;
	short		xx;

	for (xx = 0 ; xx < idx ; ++xx) {
		distance += placingZone->cells [xx].horLen;
	}

	return distance;
}

// 선호하는 테이블폼 너비를 선택하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	double	width, height;
	double	pipeWidth, pipeHeight;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "선호하는 테이블폼 너비 선택하기");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 10, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확 인");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 50, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취 소");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// 아이템 배치 (나머지)
			// 라벨: 메인 테이블폼 너비
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 120, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "메인 테이블폼 너비");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 가로 파이프 길이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 230, 20, 120, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "가로 파이프 길이");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 세로 파이프 길이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 360, 20, 120, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "세로 파이프 길이");
			DGShowItem (dialogID, itmIdx);

			// 팝업 컨트롤: 선호하는 테이블폼 너비
			POPUP_PREFER_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 45, 120, 25);
			DGSetItemFont (dialogID, POPUP_PREFER_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2200");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2150");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2100");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2050");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2000");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1950");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1900");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1850");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1800");
			DGShowItem (dialogID, POPUP_PREFER_WIDTH);

			// Edit 컨트롤: 가로 파이프 길이
			EDITCONTROL_RECT_PIPE_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 230, 45, 100, 25);
			DGDisableItem (dialogID, EDITCONTROL_RECT_PIPE_WIDTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_RECT_PIPE_WIDTH, 2.200);
			DGShowItem (dialogID, EDITCONTROL_RECT_PIPE_WIDTH);

			// Edit 컨트롤: 세로 파이프 길이
			if ((6.000 - placingZone.verLen) < EPS)
				height = 6.000;
			else if ((5.700 - placingZone.verLen) < EPS)
				height = 5.700;
			else if ((5.400 - placingZone.verLen) < EPS)
				height = 5.400;
			else if ((5.100 - placingZone.verLen) < EPS)
				height = 5.100;
			else if ((4.800 - placingZone.verLen) < EPS)
				height = 4.800;
			else if ((4.500 - placingZone.verLen) < EPS)
				height = 4.500;
			else if ((4.200 - placingZone.verLen) < EPS)
				height = 4.200;
			else if ((3.900 - placingZone.verLen) < EPS)
				height = 3.900;
			else if ((3.600 - placingZone.verLen) < EPS)
				height = 3.600;
			else if ((3.300 - placingZone.verLen) < EPS)
				height = 3.300;
			else if ((3.000 - placingZone.verLen) < EPS)
				height = 3.000;
			else if ((2.700 - placingZone.verLen) < EPS)
				height = 2.700;
			else if ((2.400 - placingZone.verLen) < EPS)
				height = 2.400;
			else if ((2.100 - placingZone.verLen) < EPS)
				height = 2.100;
			else if ((1.800 - placingZone.verLen) < EPS)
				height = 1.800;
			else if ((1.500 - placingZone.verLen) < EPS)
				height = 1.500;
			else
				height = 0;

			// 비계 파이프의 세로 방향 길이 (바뀌지 않음)
			pipeHeight = height - 0.100;

			EDITCONTROL_RECT_PIPE_HEIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 360, 45, 100, 25);
			DGDisableItem (dialogID, EDITCONTROL_RECT_PIPE_HEIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_RECT_PIPE_HEIGHT, pipeHeight);
			DGShowItem (dialogID, EDITCONTROL_RECT_PIPE_HEIGHT);

			break;

		case DG_MSG_CHANGE:

			width = atof (DGPopUpGetItemText (dialogID, POPUP_PREFER_WIDTH, DGPopUpGetSelected (dialogID, POPUP_PREFER_WIDTH)).ToCStr ()) / 1000.0;

			// 비계 파이프의 가로 방향 길이
			if (abs (width - 2.300) < EPS) {
				pipeWidth = 2.200;
			} else if (abs (width - 2.250) < EPS) {
				pipeWidth = 2.200;
			} else if (abs (width - 2.200) < EPS) {
				pipeWidth = 2.100;
			} else if (abs (width - 2.150) < EPS) {
				pipeWidth = 2.100;
			} else if (abs (width - 2.100) < EPS) {
				pipeWidth = 2.000;
			} else if (abs (width - 2.050) < EPS) {
				pipeWidth = 2.000;
			} else if (abs (width - 2.000) < EPS) {
				pipeWidth = 1.900;
			} else if (abs (width - 1.950) < EPS) {
				pipeWidth = 1.900;
			} else if (abs (width - 1.900) < EPS) {
				pipeWidth = 1.800;
			} else if (abs (width - 1.850) < EPS) {
				pipeWidth = 1.800;
			} else {
				pipeWidth = 1.700;
			}

			DGSetItemValDouble (dialogID, EDITCONTROL_RECT_PIPE_WIDTH, pipeWidth);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					// 선호하는 테이블폼 너비
					preferWidth = atof (DGPopUpGetItemText (dialogID, POPUP_PREFER_WIDTH, DGPopUpGetSelected (dialogID, POPUP_PREFER_WIDTH)).ToCStr ()) / 1000.0;
					

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

// 테이블폼 배치를 위한 질의를 요청하는 다이얼로그
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	short	xx;
	short	buttonPosX;		// 동적인 버튼 위치
	short	itmIdx;
	double	width;

	char	labelStr [32];

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "테이블폼 벽에 배치");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			// 이전 버튼
			DGSetItemText (dialogID, DG_PREV, "이 전");

			//////////////////////////////////////////////////////////// 아이템 배치 (나머지)
			DGSetItemText (dialogID, LABEL_HEIGHT, "높이");
			DGSetItemText (dialogID, LABEL_WIDTH, "너비");
			DGSetItemText (dialogID, LABEL_ERR_MESSAGE, "높이는 다음 치수만 가능함\n1500, 1800, 2100, 2400, 2700, 3000, 3300, 3600, 3900\n4200, 4500, 4800, 5100, 5400, 5700, 6000");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "벽과의 간격");

			DGSetItemText (dialogID, LABEL_FILL_SIDE, "양면/단면");
			DGSetItemText (dialogID, RADIOBUTTON_DOUBLE, "양면");
			DGSetItemText (dialogID, RADIOBUTTON_SINGLE, "단면");

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "비계 파이프");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트 세트");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "벽체 타이");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "헤드피스");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "목재");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WALLTIE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_JOIN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);

			// 기본값: 양면 채우기
			DGSetItemValLong (dialogID, RADIOBUTTON_DOUBLE, TRUE);

			// 남은 너비
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 280, 20, 70, 23);
			DGSetItemText (dialogID, itmIdx, "남은 너비");
			DGShowItem (dialogID, itmIdx);

			EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 350, 13, 50, 25);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, placingZone.remainWidth);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_WIDTH);

			// 다이얼로그 너비 변경
			DGSetDialogSize (dialogID, DG_CLIENT, 350 + (placingZone.nCells * 100), 500, DG_TOPLEFT, true);
			
			// 구분자
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 170, 45, placingZone.nCells * 100 + 50, 110);
			DGShowItem (dialogID, itmIdx);

			buttonPosX = 195;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, buttonPosX, 50, 99, 100);
				DGShowItem (dialogID, itmIdx);

				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, buttonPosX + 20, 60, 60, 23);
				sprintf (labelStr, "너비 (%d)", xx + 1);
				DGSetItemText (dialogID, itmIdx, labelStr);
				DGShowItem (dialogID, itmIdx);

				// 너비에 해당하는 콤보박스
				POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, buttonPosX + 20, 100, 60, 25);
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
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "0");
				DGShowItem (dialogID, POPUP_WIDTH [xx]);

				if (placingZone.n2300w > 0) {
					width = 2.300;
					placingZone.n2300w --;
				} else if (placingZone.n2250w > 0) {
					width = 2.250;
					placingZone.n2250w --;
				} else if (placingZone.n2200w > 0) {
					width = 2.200;
					placingZone.n2200w --;
				} else if (placingZone.n2150w > 0) {
					width = 2.150;
					placingZone.n2150w --;
				} else if (placingZone.n2100w > 0) {
					width = 2.100;
					placingZone.n2100w --;
				} else if (placingZone.n2050w > 0) {
					width = 2.050;
					placingZone.n2050w --;
				} else if (placingZone.n2000w > 0) {
					width = 2.000;
					placingZone.n2000w --;
				} else if (placingZone.n1950w > 0) {
					width = 1.950;
					placingZone.n1950w --;
				} else if (placingZone.n1900w > 0) {
					width = 1.900;
					placingZone.n1900w --;
				} else if (placingZone.n1850w > 0) {
					width = 1.850;
					placingZone.n1850w --;
				} else if (placingZone.n1800w > 0) {
					width = 1.800;
					placingZone.n1800w --;
				} else
					width = 0.0;

				// 콤보박스의 값 설정
				if (width > EPS) {
					if (width + EPS > 2.300)		DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 1);
					else if (width  + EPS> 2.250)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 2);
					else if (width + EPS > 2.200)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 3);
					else if (width + EPS > 2.150)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 4);
					else if (width + EPS > 2.100)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 5);
					else if (width + EPS > 2.050)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 6);
					else if (width + EPS > 2.000)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 7);
					else if (width + EPS > 1.950)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 8);
					else if (width + EPS > 1.900)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 9);
					else if (width + EPS > 1.850)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 10);
					else if (width + EPS > 1.800)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 11);
				}
				buttonPosX += 100;
			}

			// 높이 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.verLen);
			DGDisableItem (dialogID, EDITCONTROL_HEIGHT);

			// 높이 값이 받아들일 수 있는 값인가?
			if ( (abs (placingZone.verLen - 1.500) < EPS) || (abs (placingZone.verLen - 1.800) < EPS) || (abs (placingZone.verLen - 2.100) < EPS) || (abs (placingZone.verLen - 2.400) < EPS) ||
				(abs (placingZone.verLen - 2.700) < EPS) || (abs (placingZone.verLen - 3.000) < EPS) || (abs (placingZone.verLen - 3.300) < EPS) || (abs (placingZone.verLen - 3.600) < EPS) ||
				(abs (placingZone.verLen - 3.900) < EPS) || (abs (placingZone.verLen - 4.200) < EPS) || (abs (placingZone.verLen - 4.500) < EPS) || (abs (placingZone.verLen - 4.800) < EPS) ||
				(abs (placingZone.verLen - 5.100) < EPS) || (abs (placingZone.verLen - 5.400) < EPS) || (abs (placingZone.verLen - 5.700) < EPS) || (abs (placingZone.verLen - 6.000) < EPS) ) {

				DGHideItem (dialogID, LABEL_ERR_MESSAGE);
			} else {
				DGShowItem (dialogID, LABEL_ERR_MESSAGE);
			}

			// 너비 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.horLen);
			DGDisableItem (dialogID, EDITCONTROL_WIDTH);

			break;

		case DG_MSG_CHANGE:

			width = 0;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				width += atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, POPUP_WIDTH [xx])).ToCStr ()) / 1000.0;
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - width);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 셀의 너비/높이 저장
					for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
						placingZone.cells [xx].horLen = atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, POPUP_WIDTH [xx])).ToCStr ()) / 1000.0;
						
						if ((6.000 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 6.000;
						else if ((5.700 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 5.700;
						else if ((5.400 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 5.400;
						else if ((5.100 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 5.100;
						else if ((4.800 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 4.800;
						else if ((4.500 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 4.500;
						else if ((4.200 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 4.200;
						else if ((3.900 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 3.900;
						else if ((3.600 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 3.600;
						else if ((3.300 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 3.300;
						else if ((3.000 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 3.000;
						else if ((2.700 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.700;
						else if ((2.400 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.400;
						else if ((2.100 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.100;
						else if ((1.800 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.800;
						else if ((1.500 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.500;
						else
							placingZone.cells [xx].verLen = 0;
					}

					// 벽와의 간격
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// 양면/단면
					if (DGGetItemValLong (dialogID, RADIOBUTTON_DOUBLE) == TRUE)
						placingZone.bDoubleSide = true;
					else
						placingZone.bDoubleSide = false;

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
	
					break;
				case DG_CANCEL:
					break;

				case DG_PREV:
					clickedPrevButton = true;
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

// 벽 상단의 합판/목재 영역을 유로폼으로 채울지 물어보는 3차 다이얼로그
short DGCALLBACK wallTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGSetDialogTitle (dialogID, "벽 상단 채우기");

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
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 140, 40-6, 70, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_HEIGHT_TOPREST);
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_TOPREST, placingZone.marginTop);
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
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 70, 180-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST, "유로폼");
			DGShowItem (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST);

			// 체크박스: 폼 On/Off (2단 - 중간)
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 70, 150-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST, "유로폼");
			DGShowItem (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST);

			// 라벨: 합판
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 120, 70, 23);
			DGSetItemFont (dialogID, LABEL_PLYWOOD_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
			DGShowItem (dialogID, LABEL_PLYWOOD_TOPREST);

			// 체크박스: 규격폼 (1단 - 맨 아래)
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 150, 180-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, "규격폼");
			DGShowItem (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST);
			DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, TRUE);

			// 체크박스: 규격폼 (2단 - 중간)
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 150, 150-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, "규격폼");
			DGShowItem (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST);
			DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, TRUE);

			// 팝업 컨트롤: 유로폼 (1단) 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 220, 180-6, 70, 25);
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
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 220, 150-6, 70, 25);
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
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 180-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);

			// Edit 컨트롤: 유로폼 (2단) 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 150-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);

			// Edit 컨트롤: 합판 또는 목재 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 120-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_PLYWOOD_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_PLYWOOD_TOPREST);
			DGDisableItem (dialogID, EDITCONTROL_PLYWOOD_TOPREST);

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

			initPlywoodHeight = placingZone.marginTop;

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
			} else if (changedPlywoodHeight < 0.110) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "목재");
			} else {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, changedPlywoodHeight);

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

					initPlywoodHeight = placingZone.marginTop;
			
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

					for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
						placingZone.upperCells [xx].bFill = true;

						placingZone.upperCells [xx].bEuroform1 = bEuroform1;
						placingZone.upperCells [xx].bEuroformStandard1 = bEuroformStandard1;
						placingZone.upperCells [xx].formWidth1 = euroformWidth1;

						placingZone.upperCells [xx].bEuroform2 = bEuroform2;
						placingZone.upperCells [xx].bEuroformStandard2 = bEuroformStandard2;
						placingZone.upperCells [xx].formWidth2 = euroformWidth2;
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

// 이동 후의 X 좌표를 알려줌 (Z 회전각도 고려) - 벽과 평행한 방향으로 이동
double		WallTableformPlacingZone::moveXinParallel (double prevPosX, double ang, double offset)
{
	return	prevPosX + (offset * cos(ang));
}

// 이동 후의 Y 좌표를 알려줌 (Z 회전각도 고려) - 벽과 평행한 방향으로 이동
double		WallTableformPlacingZone::moveYinParallel (double prevPosY, double ang, double offset)
{
	return	prevPosY + (offset * sin(ang));
}

// 이동 후의 X 좌표를 알려줌 (Z 회전각도 고려) - 벽과 수직한 방향으로 이동
double		WallTableformPlacingZone::moveXinPerpend (double prevPosX, double ang, double offset)
{
	return	prevPosX - (offset * sin(ang));
}

// 이동 후의 Y 좌표를 알려줌 (Z 회전각도 고려) - 벽과 수직한 방향으로 이동
double		WallTableformPlacingZone::moveYinPerpend (double prevPosY, double ang, double offset)
{
	return	prevPosY + (offset * cos(ang));
}

// 이동 후의 Z 좌표를 알려줌 (Z 회전각도 고려)
double		WallTableformPlacingZone::moveZ (double prevPosZ, double offset)
{
	return	prevPosZ + offset;
}

// 배치: 유로폼
API_Guid	WallTableformPlacingZone::placeUFOM (Euroform params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmUFOM;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;
	
	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_Euroform;

	memo.params [0][27].value.real = TRUE;	// 규격폼

	// 너비
	tempStr = format_string ("%.0f", params.width * 1000);
	GS::ucscpy (memo.params [0][28].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// 높이
	tempStr = format_string ("%.0f", params.height * 1000);
	GS::ucscpy (memo.params [0][29].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// 설치방향
	tempStr = "벽세우기";
	GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());
		
	memo.params [0][33].value.real = DegreeToRad (90.0);	// 회전X

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 유로폼 (상부)
API_Guid	WallTableformPlacingZone::placeUFOM_up (Euroform params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmUFOM;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;
	
	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_Euroform;

	// 규격폼 크기인지 확인할 것
	bool bStandardWidth, bStandardHeight;

	if ((abs (params.width - 0.600) < EPS) || (abs (params.width - 0.500) < EPS) || (abs (params.width - 0.450) < EPS) || (abs (params.width - 0.400) < EPS) || (abs (params.width - 0.300) < EPS) || (abs (params.width - 0.200) < EPS))
		bStandardWidth = true;
	else
		bStandardWidth = false;

	if ((abs (params.height - 1.200) < EPS) || (abs (params.height - 0.900) < EPS) || (abs (params.height - 0.600) < EPS))
		bStandardHeight = true;
	else
		bStandardHeight = false;

	if (bStandardWidth && bStandardHeight) {
		memo.params [0][27].value.real = TRUE;	// 규격폼

		// 너비
		tempStr = format_string ("%.0f", params.width * 1000);
		GS::ucscpy (memo.params [0][28].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// 높이
		tempStr = format_string ("%.0f", params.height * 1000);
		GS::ucscpy (memo.params [0][29].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());
	} else {
		memo.params [0][27].value.real = FALSE;	// 비규격폼

		memo.params [0][30].value.real = params.width;	// 너비
		memo.params [0][31].value.real = params.height;	// 높이
	}

	// 설치방향
	tempStr = "벽눕히기";
	GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());
		
	memo.params [0][33].value.real = DegreeToRad (90.0);	// 회전X

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 비계 파이프
API_Guid	WallTableformPlacingZone::placeSPIP (SquarePipe params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmSPIP;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_RectPipe;

	// 사각파이프
	tempStr = "사각파이프";
	GS::ucscpy (memo.params [0][24].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	memo.params [0][27].value.real = params.length;		// 길이
	memo.params [0][28].value.real = params.pipeAng;	// 각도

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 핀볼트 세트
API_Guid	WallTableformPlacingZone::placePINB (PinBoltSet params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmPINB;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_PinBolt;

	// 핀볼트 90도 회전
	if (params.bPinBoltRot90)
		memo.params [0][9].value.real = TRUE;
	else
		memo.params [0][9].value.real = FALSE;

	memo.params [0][10].value.real = params.boltLen;	// 볼트 길이
	memo.params [0][11].value.real = 0.010;				// 볼트 직경
	memo.params [0][12].value.real = 0.050;				// 와셔 위치
	memo.params [0][13].value.real = 0.100;				// 와셔 크기
	memo.params [0][17].value.real = params.angX;		// X축 회전
	memo.params [0][18].value.real = params.angY;		// Y축 회전

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 벽체 타이
API_Guid	WallTableformPlacingZone::placeTIE (WallTie params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmTIE;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang + DegreeToRad (90.0);
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_WallTie;

	memo.params [0][9].value.real = params.boltLen;		// 볼트 길이 (벽 두께 + 327mm 초과이며 100 단위로 나눠지는 가장 작은 수)
	memo.params [0][10].value.real = 0.012;		// 볼트 직경
	memo.params [0][11].value.real = TRUE;		// 사각와샤
	memo.params [0][12].value.real = 0.100;		// 사각와샤 크기
	
	// 너트 타입
	tempStr = "타입 1";
	GS::ucscpy (memo.params [0][13].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());
	
	memo.params [0][14].value.real = TRUE;		// 벽체 내장 파이프
	memo.params [0][16].value.real = 0.012;		// 파이프 내경
	memo.params [0][17].value.real = 0.002;		// 파이프 두께
	
	// 파이프 시작점, 끝점 (벽 두께만큼 차이)
	memo.params [0][18].value.real = params.pipeBeg;
	memo.params [0][19].value.real = params.pipeEnd;
	
	// 좌,우측 조임쇠 위치 (벽 두께 + 327mm 만큼 차이)
	memo.params [0][20].value.real = params.clampBeg;
	memo.params [0][21].value.real = params.clampEnd;
	
	memo.params [0][22].value.real = DegreeToRad (0.0);		// 회전 Y

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 직교 클램프
API_Guid	WallTableformPlacingZone::placeCLAM (CrossClamp params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmCLAM;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_Clamp;

	memo.params [0][9].value.real = params.angX;	// 본체 회전 (X)
	memo.params [0][10].value.real = params.angY;	// 본체 회전 (Y)
	memo.params [0][11].value.real = 0.018;			// 고정볼트 조이기

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 헤드피스
API_Guid	WallTableformPlacingZone::placePUSH (HeadpieceOfPushPullProps params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmPUSH;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_HeadPiece;

	tempStr = "타입 A";
	GS::ucscpy (memo.params [0][9].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());
	memo.params [0][10].value.real = 0.009;					// 철판 두께
	memo.params [0][11].value.real = DegreeToRad (0.0);		// 회전X
	memo.params [0][12].value.real = DegreeToRad (0.0);		// 회전Y

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 결합철물
API_Guid	WallTableformPlacingZone::placeJOIN (MetalFittings params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmJOIN;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang + DegreeToRad (180);
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_Join;

	memo.params [0][13].value.real = 0.108;					// 와셔2 위치
	memo.params [0][19].value.real = DegreeToRad (0.0);		// 회전X
	memo.params [0][20].value.real = DegreeToRad (0.0);		// 회전Y

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 합판
API_Guid	WallTableformPlacingZone::placePLYW (Plywood params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmPLYW;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_Plywood;

	GS::ucscpy (memo.params [0][32].value.uStr, L("비규격"));
	memo.params [0][35].value.real = params.p_wid;		// 가로
	memo.params [0][36].value.real = params.p_leng;		// 세로
		
	// 설치방향
	if (params.w_dir_wall == true)
		tempStr = "벽세우기";
	else
		tempStr = "벽눕히기";
	GS::ucscpy (memo.params [0][33].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 목재
API_Guid	WallTableformPlacingZone::placeTIMB (Wood params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmTIMB;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_Wood;

	GS::ucscpy (memo.params [0][27].value.uStr, L("벽세우기"));		// 설치방향
	memo.params [0][28].value.real = params.w_w;		// 두께
	memo.params [0][29].value.real = params.w_h;		// 너비
	memo.params [0][30].value.real = params.w_leng;		// 길이
	memo.params [0][31].value.real = params.w_ang;		// 각도

	// 목재가 세로로 길게 배치될 경우
	if ( abs (RadToDegree (params.w_ang) - 90.0) < EPS ) {
		elem.object.pos.x += ( params.w_h * cos(params.ang) );
		elem.object.pos.y += ( params.w_h * sin(params.ang) );
	}

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}