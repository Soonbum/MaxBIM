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
static short	layerInd_PinBolt;		// 레이어 번호: 핀볼트 세트 (A타입 전용)
static short	layerInd_WallTie;		// 레이어 번호: 빅체 타이 (A타입 전용, 더 이상 사용하지 않음)
static short	layerInd_Clamp;			// 레이어 번호: 직교 클램프 (더 이상 사용하지 않음)
static short	layerInd_HeadPiece;		// 레이어 번호: 헤드피스 (B타입에서는 빔조인트용 Push-Pull Props 헤드피스)
static short	layerInd_Join;			// 레이어 번호: 결합철물 (B타입에서는 사각파이프 연결철물)
static short	layerInd_Plywood;		// 레이어 번호: 합판
static short	layerInd_Wood;			// 레이어 번호: 목재
static short	layerInd_RectPipeHanger;	// 레이어 번호: 각파이프 행거 (B타입 전용)
static short	layerInd_EuroformHook;		// 레이어 번호: 유로폼 후크 (B타입 전용)
static short	layerInd_Hidden;		// 레이어 번호: 숨김

static GS::Array<API_Guid>	elemList;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함

// 다이얼로그 동적 요소 인덱스 번호 저장
static short	EDITCONTROL_REMAIN_WIDTH;
static short	POPUP_WIDTH [50];
static short	POPUP_PREFER_WIDTH;
static short	EDITCONTROL_RECT_PIPE_WIDTH;
static short	EDITCONTROL_RECT_PIPE_HEIGHT;

static double	preferWidth;
static bool		clickedPrevButton;		// 이전 버튼을 눌렀습니까?


// 벽에 테이블폼을 배치하는 통합 루틴 - 세로 방향
GSErrCode	placeTableformOnWall_Vertical (void)
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

FIRST_VERTICAL:

	clickedPrevButton = false;

	// 테이블폼 개수 초기화
	placingZone.n400w = 0;
	placingZone.n450w = 0;
	placingZone.n500w = 0;
	placingZone.n600w = 0;
	placingZone.n650w = 0;
	placingZone.n700w = 0;
	placingZone.n750w = 0;
	placingZone.n800w = 0;
	placingZone.n850w = 0;
	placingZone.n900w = 0;
	placingZone.n950w = 0;
	placingZone.n1000w = 0;
	placingZone.n1050w = 0;
	placingZone.n1100w = 0;
	placingZone.n1150w = 0;
	placingZone.n1200w = 0;
	placingZone.n1250w = 0;
	placingZone.n1300w = 0;
	placingZone.n1350w = 0;
	placingZone.n1400w = 0;
	placingZone.n1450w = 0;
	placingZone.n1500w = 0;
	placingZone.n1550w = 0;
	placingZone.n1600w = 0;
	placingZone.n1650w = 0;
	placingZone.n1700w = 0;
	placingZone.n1750w = 0;
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
	result = DGBlankModalDialog (500, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler1_Vertical, 0);

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
			} else if (abs (preferWidth - 1.750) < EPS) {
				placingZone.n1750w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.700) < EPS) {
				placingZone.n1700w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.650) < EPS) {
				placingZone.n1650w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.600) < EPS) {
				placingZone.n1600w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.550) < EPS) {
				placingZone.n1550w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.500) < EPS) {
				placingZone.n1500w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.450) < EPS) {
				placingZone.n1450w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.400) < EPS) {
				placingZone.n1400w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.350) < EPS) {
				placingZone.n1350w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.300) < EPS) {
				placingZone.n1300w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.250) < EPS) {
				placingZone.n1250w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.200) < EPS) {
				placingZone.n1200w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.150) < EPS) {
				placingZone.n1150w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.100) < EPS) {
				placingZone.n1100w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.050) < EPS) {
				placingZone.n1050w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.000) < EPS) {
				placingZone.n1000w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.950) < EPS) {
				placingZone.n950w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.900) < EPS) {
				placingZone.n900w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.850) < EPS) {
				placingZone.n850w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.800) < EPS) {
				placingZone.n800w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.750) < EPS) {
				placingZone.n750w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.700) < EPS) {
				placingZone.n700w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.650) < EPS) {
				placingZone.n650w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.600) < EPS) {
				placingZone.n600w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.500) < EPS) {
				placingZone.n500w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.450) < EPS) {
				placingZone.n450w ++;
				tableColumn ++;
			} else if (abs (preferWidth - 0.400) < EPS) {
				placingZone.n400w ++;
				tableColumn ++;
			}
		} else {
			placingZone.n400w ++;
			tableColumn ++;
		}
		width -= preferWidth;
	}

	// 남은 길이 저장
	placingZone.remainWidth = width;

	// 셀 개수 저장
	placingZone.nCells = tableColumn;

	// 상단 여백 높이 설정
	if (placingZone.verLen > 6.000 - EPS) {
		placingZone.marginTop = placingZone.verLen - 6.000;
	} else if (placingZone.verLen > 5.700 - EPS) {
		placingZone.marginTop = placingZone.verLen - 5.700;
	} else if (placingZone.verLen > 5.400 - EPS) {
		placingZone.marginTop = placingZone.verLen - 5.400;
	} else if (placingZone.verLen > 5.100 - EPS) {
		placingZone.marginTop = placingZone.verLen - 5.100;
	} else if (placingZone.verLen > 4.800 - EPS) {
		placingZone.marginTop = placingZone.verLen - 4.800;
	} else if (placingZone.verLen > 4.500 - EPS) {
		placingZone.marginTop = placingZone.verLen - 4.500;
	} else if (placingZone.verLen > 4.200 - EPS) {
		placingZone.marginTop = placingZone.verLen - 4.200;
	} else if (placingZone.verLen > 3.900 - EPS) {
		placingZone.marginTop = placingZone.verLen - 3.900;
	} else if (placingZone.verLen > 3.600 - EPS) {
		placingZone.marginTop = placingZone.verLen - 3.600;
	} else if (placingZone.verLen > 3.300 - EPS) {
		placingZone.marginTop = placingZone.verLen - 3.300;
	} else if (placingZone.verLen > 3.000 - EPS) {
		placingZone.marginTop = placingZone.verLen - 3.000;
	} else if (placingZone.verLen > 2.700 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.700;
	} else if (placingZone.verLen > 2.400 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.400;
	} else if (placingZone.verLen > 2.100 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.100;
	} else if (placingZone.verLen > 1.800 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.800;
	} else if (placingZone.verLen > 1.500 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.500;
	} else {
		placingZone.marginTop = 0;
	}

	// [DIALOG] 2번째 다이얼로그에서 벽 너비 방향의 테이블폼 수량 및 각 셀의 너비/높이를 설정함
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32517, ACAPI_GetOwnResModule (), wallTableformPlacerHandler2_Vertical, 0);

	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST_VERTICAL;

	// 벽과의 간격으로 인해 정보 업데이트
	infoWall.wallThk		+= (placingZone.gap * 2);

	if (result != DG_OK)
		return err;

	// 셀 위치 및 각도 초기화
	placingZone.initCells (&placingZone);

	// 테이블폼 배치하기
	for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
		if (placingZone.type == 1)
			err = placingZone.placeTableformOnWall_Vertical_Type1 (placingZone.cells [xx]);
		else if (placingZone.type == 2)
			err = placingZone.placeTableformOnWall_Vertical_Type2 (placingZone.cells [xx]);
	}

	// [DIALOG] 3번째 다이얼로그에서 벽 상단의 자투리 공간을 다른 규격의 유로폼으로 대체할 것인지 묻습니다.
	result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3_Vertical, 0);

	if (result != DG_OK)
		return err;

	// 벽 상부 남는 영역에 유로폼1단, 유로폼2단, 합판 또는 목재 설치
	for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
		err = placingZone.placeTableformOnWall_Vertical (placingZone.cells [xx], placingZone.upperCells [xx]);
	}

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// 벽에 테이블폼을 배치하는 통합 루틴 - 가로 방향
GSErrCode	placeTableformOnWall_Horizontal (void)
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

FIRST_HORIZONTAL:

	clickedPrevButton = false;

	// 테이블폼 개수 초기화
	placingZone.n1500h = 0;
	placingZone.n1800h = 0;
	placingZone.n2100h = 0;
	placingZone.n2400h = 0;
	placingZone.n2700h = 0;
	placingZone.n3000h = 0;
	placingZone.n3300h = 0;
	placingZone.n3600h = 0;
	placingZone.n3900h = 0;
	placingZone.n4200h = 0;
	placingZone.n4500h = 0;
	placingZone.n4800h = 0;
	placingZone.n5100h = 0;
	placingZone.n5400h = 0;
	placingZone.n5700h = 0;
	placingZone.n6000h = 0;

	// 테이블폼 개수 계산
	tableColumn = 0;
	width = placingZone.horLen;

	// [DIALOG] 1번째 다이얼로그에서 선호하는 테이블폼 너비를 선택함 (선택한 테이블폼에 대하여 수평, 수직 비계파이프의 길이를 미리 보여줌)
	result = DGBlankModalDialog (500, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler1_Horizontal, 0);

	if (result != DG_OK)
		return err;

	// 선호하는 테이블폼 너비에 따라 테이블 개수를 결정함
	while (width > EPS) {
		if (width + EPS >= preferWidth) {
			if (abs (preferWidth - 6.000) < EPS) {
				placingZone.n6000h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 5.700) < EPS) {
				placingZone.n5700h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 5.400) < EPS) {
				placingZone.n5400h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 5.100) < EPS) {
				placingZone.n5100h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 4.800) < EPS) {
				placingZone.n4800h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 4.500) < EPS) {
				placingZone.n4500h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 4.200) < EPS) {
				placingZone.n4200h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 3.900) < EPS) {
				placingZone.n3900h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 3.600) < EPS) {
				placingZone.n3600h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 3.300) < EPS) {
				placingZone.n3300h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 3.000) < EPS) {
				placingZone.n3000h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 2.700) < EPS) {
				placingZone.n2700h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 2.400) < EPS) {
				placingZone.n2400h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 2.100) < EPS) {
				placingZone.n2100h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.800) < EPS) {
				placingZone.n1800h ++;
				tableColumn ++;
			} else if (abs (preferWidth - 1.500) < EPS) {
				placingZone.n1500h ++;
				tableColumn ++;
			}
		} else {
			placingZone.n1500h ++;
			tableColumn ++;
		}
		width -= preferWidth;
	}

	// 남은 길이 저장
	placingZone.remainWidth = width;

	// 셀 개수 저장
	placingZone.nCells = tableColumn;

	// 상단 여백 높이 설정
	if (placingZone.verLen > 2.300 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.300;
	} else if (placingZone.verLen > 2.250 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.250;
	} else if (placingZone.verLen > 2.200 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.200;
	} else if (placingZone.verLen > 2.150 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.150;
	} else if (placingZone.verLen > 2.100 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.100;
	} else if (placingZone.verLen > 2.050 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.050;
	} else if (placingZone.verLen > 2.000 - EPS) {
		placingZone.marginTop = placingZone.verLen - 2.000;
	} else if (placingZone.verLen > 1.950 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.950;
	} else if (placingZone.verLen > 1.900 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.900;
	} else if (placingZone.verLen > 1.850 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.850;
	} else if (placingZone.verLen > 1.800 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.800;
	} else if (placingZone.verLen > 1.750 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.750;
	} else if (placingZone.verLen > 1.700 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.700;
	} else if (placingZone.verLen > 1.650 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.650;
	} else if (placingZone.verLen > 1.600 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.600;
	} else if (placingZone.verLen > 1.550 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.550;
	} else if (placingZone.verLen > 1.500 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.500;
	} else if (placingZone.verLen > 1.450 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.450;
	} else if (placingZone.verLen > 1.400 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.400;
	} else if (placingZone.verLen > 1.350 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.350;
	} else if (placingZone.verLen > 1.300 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.300;
	} else if (placingZone.verLen > 1.250 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.250;
	} else if (placingZone.verLen > 1.200 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.200;
	} else if (placingZone.verLen > 1.150 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.150;
	} else if (placingZone.verLen > 1.100 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.100;
	} else if (placingZone.verLen > 1.050 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.050;
	} else if (placingZone.verLen > 1.000 - EPS) {
		placingZone.marginTop = placingZone.verLen - 1.000;
	} else if (placingZone.verLen > 0.950 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.950;
	} else if (placingZone.verLen > 0.900 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.900;
	} else if (placingZone.verLen > 0.850 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.850;
	} else if (placingZone.verLen > 0.800 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.800;
	} else if (placingZone.verLen > 0.750 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.750;
	} else if (placingZone.verLen > 0.700 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.700;
	} else if (placingZone.verLen > 0.650 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.650;
	} else if (placingZone.verLen > 0.600 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.600;
	} else if (placingZone.verLen > 0.500 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.500;
	} else if (placingZone.verLen > 0.450 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.450;
	} else if (placingZone.verLen > 0.400 - EPS) {
		placingZone.marginTop = placingZone.verLen - 0.400;
	} else {
		placingZone.marginTop = 0;
	}

	// [DIALOG] 2번째 다이얼로그에서 벽 너비 방향의 테이블폼 수량 및 각 셀의 너비/높이를 설정함
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32517, ACAPI_GetOwnResModule (), wallTableformPlacerHandler2_Horizontal, 0);

	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST_HORIZONTAL;

	// 벽과의 간격으로 인해 정보 업데이트
	infoWall.wallThk		+= (placingZone.gap * 2);

	if (result != DG_OK)
		return err;

	// 셀 위치 및 각도 초기화
	placingZone.initCells (&placingZone);

	// 테이블폼 배치하기
	for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
		if (placingZone.type == 1)
			err = placingZone.placeTableformOnWall_Horizontal_Type1 (placingZone.cells [xx]);
		else if (placingZone.type == 2)
			err = placingZone.placeTableformOnWall_Horizontal_Type2 (placingZone.cells [xx]);
	}

	// [DIALOG] 3번째 다이얼로그에서 벽 상단의 자투리 공간을 다른 규격의 유로폼으로 대체할 것인지 묻습니다.
	result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3_Horizontal, 0);

	if (result != DG_OK)
		return err;

	// 벽 상부 남는 영역에 유로폼1단, 유로폼2단, 합판 또는 목재 설치
	for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
		err = placingZone.placeTableformOnWall_Horizontal (placingZone.cells [xx], placingZone.upperCells [xx]);
	}

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// 벽에 테이블폼을 배치하는 통합 루틴 - 커스텀
GSErrCode	placeTableformOnWall_Custom (void)
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

FIRST_CUSTOM:

	clickedPrevButton = false;

	// 테이블폼 개수 초기화
	placingZone.n400w = 0;
	placingZone.n450w = 0;
	placingZone.n500w = 0;
	placingZone.n600w = 0;
	placingZone.n650w = 0;
	placingZone.n700w = 0;
	placingZone.n750w = 0;
	placingZone.n800w = 0;
	placingZone.n850w = 0;
	placingZone.n900w = 0;
	placingZone.n950w = 0;
	placingZone.n1000w = 0;
	placingZone.n1050w = 0;
	placingZone.n1100w = 0;
	placingZone.n1150w = 0;
	placingZone.n1200w = 0;
	placingZone.n1250w = 0;
	placingZone.n1300w = 0;
	placingZone.n1350w = 0;
	placingZone.n1400w = 0;
	placingZone.n1450w = 0;
	placingZone.n1500w = 0;
	placingZone.n1550w = 0;
	placingZone.n1600w = 0;
	placingZone.n1650w = 0;
	placingZone.n1700w = 0;
	placingZone.n1750w = 0;
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

	placingZone.n1500h = 0;
	placingZone.n1800h = 0;
	placingZone.n2100h = 0;
	placingZone.n2400h = 0;
	placingZone.n2700h = 0;
	placingZone.n3000h = 0;
	placingZone.n3300h = 0;
	placingZone.n3600h = 0;
	placingZone.n3900h = 0;
	placingZone.n4200h = 0;
	placingZone.n4500h = 0;
	placingZone.n4800h = 0;
	placingZone.n5100h = 0;
	placingZone.n5400h = 0;
	placingZone.n5700h = 0;
	placingZone.n6000h = 0;

	// 테이블폼 개수 계산
	tableColumn = 0;
	width = placingZone.horLen;

	// [DIALOG] 1번째 다이얼로그에서 테이블폼의 방향과 가로/세로 방향 유로폼의 개수와 각각의 길이를 선택함
	result = DGBlankModalDialog (550, 450, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler1_Custom, 0);

	if (result != DG_OK)
		return err;

	//// 선호하는 테이블폼 너비에 따라 테이블 개수를 결정함
	//while (width > EPS) {
	//	if (width + EPS >= preferWidth) {
	//		if (abs (preferWidth - 2.300) < EPS) {
	//			placingZone.n2300w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 2.250) < EPS) {
	//			placingZone.n2250w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 2.200) < EPS) {
	//			placingZone.n2200w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 2.150) < EPS) {
	//			placingZone.n2150w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 2.100) < EPS) {
	//			placingZone.n2100w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 2.050) < EPS) {
	//			placingZone.n2050w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 2.000) < EPS) {
	//			placingZone.n2000w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.950) < EPS) {
	//			placingZone.n1950w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.900) < EPS) {
	//			placingZone.n1900w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.850) < EPS) {
	//			placingZone.n1850w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.800) < EPS) {
	//			placingZone.n1800w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.750) < EPS) {
	//			placingZone.n1750w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.700) < EPS) {
	//			placingZone.n1700w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.650) < EPS) {
	//			placingZone.n1650w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.600) < EPS) {
	//			placingZone.n1600w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.550) < EPS) {
	//			placingZone.n1550w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.500) < EPS) {
	//			placingZone.n1500w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.450) < EPS) {
	//			placingZone.n1450w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.400) < EPS) {
	//			placingZone.n1400w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.350) < EPS) {
	//			placingZone.n1350w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.300) < EPS) {
	//			placingZone.n1300w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.250) < EPS) {
	//			placingZone.n1250w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.200) < EPS) {
	//			placingZone.n1200w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.150) < EPS) {
	//			placingZone.n1150w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.100) < EPS) {
	//			placingZone.n1100w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.050) < EPS) {
	//			placingZone.n1050w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 1.000) < EPS) {
	//			placingZone.n1000w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.950) < EPS) {
	//			placingZone.n950w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.900) < EPS) {
	//			placingZone.n900w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.850) < EPS) {
	//			placingZone.n850w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.800) < EPS) {
	//			placingZone.n800w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.750) < EPS) {
	//			placingZone.n750w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.700) < EPS) {
	//			placingZone.n700w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.650) < EPS) {
	//			placingZone.n650w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.600) < EPS) {
	//			placingZone.n600w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.500) < EPS) {
	//			placingZone.n500w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.450) < EPS) {
	//			placingZone.n450w ++;
	//			tableColumn ++;
	//		} else if (abs (preferWidth - 0.400) < EPS) {
	//			placingZone.n400w ++;
	//			tableColumn ++;
	//		}
	//	} else {
	//		placingZone.n400w ++;
	//		tableColumn ++;
	//	}
	//	width -= preferWidth;
	//}

	//// 남은 길이 저장
	//placingZone.remainWidth = width;

	//// 셀 개수 저장
	//placingZone.nCells = tableColumn;

	//// 상단 여백 높이 설정
	//if (placingZone.verLen > 6.000 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 6.000;
	//} else if (placingZone.verLen > 5.700 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 5.700;
	//} else if (placingZone.verLen > 5.400 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 5.400;
	//} else if (placingZone.verLen > 5.100 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 5.100;
	//} else if (placingZone.verLen > 4.800 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 4.800;
	//} else if (placingZone.verLen > 4.500 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 4.500;
	//} else if (placingZone.verLen > 4.200 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 4.200;
	//} else if (placingZone.verLen > 3.900 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 3.900;
	//} else if (placingZone.verLen > 3.600 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 3.600;
	//} else if (placingZone.verLen > 3.300 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 3.300;
	//} else if (placingZone.verLen > 3.000 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 3.000;
	//} else if (placingZone.verLen > 2.700 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 2.700;
	//} else if (placingZone.verLen > 2.400 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 2.400;
	//} else if (placingZone.verLen > 2.100 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 2.100;
	//} else if (placingZone.verLen > 1.800 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 1.800;
	//} else if (placingZone.verLen > 1.500 - EPS) {
	//	placingZone.marginTop = placingZone.verLen - 1.500;
	//} else {
	//	placingZone.marginTop = 0;
	//}

	//// [DIALOG] 2번째 다이얼로그에서 벽 너비 방향의 테이블폼 수량 및 각 셀의 너비/높이를 설정함
	//result = DGModalDialog (ACAPI_GetOwnResModule (), 32517, ACAPI_GetOwnResModule (), wallTableformPlacerHandler2_Vertical, 0);

	//// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	//if (clickedPrevButton == true)
	//	goto FIRST_CUSTOM;

	//// 벽과의 간격으로 인해 정보 업데이트
	//infoWall.wallThk		+= (placingZone.gap * 2);

	//if (result != DG_OK)
	//	return err;

	//// 셀 위치 및 각도 초기화
	//placingZone.initCells (&placingZone);

	//// 테이블폼 배치하기
	//for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
	//	err = placingZone.placeTableformOnWall_Vertical (placingZone.cells [xx]);
	//}

	//// [DIALOG] 3번째 다이얼로그에서 벽 상단의 자투리 공간을 다른 규격의 유로폼으로 대체할 것인지 묻습니다.
	//result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3_Vertical, 0);

	//if (result != DG_OK)
	//	return err;

	//// 벽 상부 남는 영역에 유로폼1단, 유로폼2단, 합판 또는 목재 설치
	//for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
	//	err = placingZone.placeTableformOnWall_Vertical (placingZone.cells [xx], placingZone.upperCells [xx]);
	//}

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

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

// 테이블폼 배치하기 - 세로 방향 (타입1)
GSErrCode	WallTableformPlacingZone::placeTableformOnWall_Vertical_Type1 (CellForWallTableform cell)
{
	GSErrCode	err = NoError;
	placementInfoForWallTableform	placementInfo;

	short		xx, yy;
	double		width, height;
	//double		remainder;				// fmod 함수에 쓸 변수
	double		elev_headpiece;
	double		horizontalGap = 0.050;	// 수평재 양쪽 이격거리

	Euroform		params_UFOM;
	SquarePipe		params_SPIP;
	PinBoltSet		params_PINB;
	//WallTie			params_TIE;
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
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.750) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.500;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.700) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.650) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.600) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.550) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.500) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.450) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.500;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.400) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.500;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.350) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.300) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.250) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.200) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.150) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.100) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 1.050) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 1.000) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 0.950) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 0.900) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 0.850) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 0.800) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 0.750) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 0.700) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 0.650) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 0.600) < EPS) {
		placementInfo.nHorEuroform = 1;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 0.500) < EPS) {
		placementInfo.nHorEuroform = 1;
		placementInfo.width [0] = 0.500;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (cell.horLen - 0.450) < EPS) {
		placementInfo.nHorEuroform = 1;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (cell.horLen - 0.400) < EPS) {
		placementInfo.nHorEuroform = 1;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
	} else {
		placementInfo.nHorEuroform = 0;
		placementInfo.width [0] = 0.0;		placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
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
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 4.500) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 0.900;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 4.200) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.900;
		placementInfo.height [3] = 0.900;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 3.900) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.900;
		placementInfo.height [3] = 0.600;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 3.600) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 3.300) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.900;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 3.000) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.600;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 2.700) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 0.900;
		placementInfo.height [2] = 0.600;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 2.400) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 2.100) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 0.900;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 1.800) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.900;
		placementInfo.height [1] = 0.900;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 1.500) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.900;
		placementInfo.height [1] = 0.600;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else {
		placementInfo.nVerEuroform = 0;
		placementInfo.height [0] = 0.0;
		placementInfo.height [1] = 0.0;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
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
	params_UFOM.u_ins_wall = true;

	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		height = 0.0;
		for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
			params_UFOM.width	= placementInfo.width [xx];
			params_UFOM.height	= placementInfo.height [yy];
			height += placementInfo.height [yy];
			elemList.Push (placeUFOM (params_UFOM));
			moveIn3D ('z', params_UFOM.ang, placementInfo.height [yy], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		}
		moveIn3D ('x', params_UFOM.ang, placementInfo.width [xx], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		moveIn3D ('z', params_UFOM.ang, -height, &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
	}

	// 비계 파이프 (수평) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.horLen - (horizontalGap * 2);
	params_SPIP.pipeAng = DegreeToRad (0);

	moveIn3D ('x', params_SPIP.ang, horizontalGap, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.025), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('z', params_SPIP.ang, 0.150 - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

	for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
		if (xx == 0) {
			// 1행
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('z', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('z', params_SPIP.ang, -0.031 - 0.150 + placementInfo.height [xx] - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		} else if (xx == placementInfo.nVerEuroform) {
			// 마지막 행
			moveIn3D ('z', params_SPIP.ang, -0.150, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('z', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
		} else {
			// 나머지 행
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('z', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('z', params_SPIP.ang, -0.031 + placementInfo.height [xx] - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		}
	}

	// 비계 파이프 (수직) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.verLen - 0.100;
	params_SPIP.pipeAng = DegreeToRad (90);

	moveIn3D ('x', params_SPIP.ang, placementInfo.width [0] - 0.150 - 0.035, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.075), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('z', params_SPIP.ang, 0.050, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

	// 1열
	elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('x', params_SPIP.ang, 0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('x', params_SPIP.ang, -0.070 - (placementInfo.width [0] - 0.150) + cell.horLen + (-placementInfo.width [placementInfo.nHorEuroform-1] + 0.150), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	// 2열
	elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('x', params_SPIP.ang, 0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
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

	moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

	// 최하단 행
	moveIn3D ('z', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	width = 0.0;
	for (xx = 0 ; xx < placementInfo.nHorEuroform - 1 ; ++xx) {
		width += placementInfo.width [xx];
		moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		elemList.Push (placePINB (params_PINB));
	}
	// 최상단 행
	moveIn3D ('x', params_PINB.ang, -width, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('z', params_PINB.ang, cell.verLen - 0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	for (xx = 0 ; xx < placementInfo.nHorEuroform - 1 ; ++xx) {
		moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

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

	moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

	// 2 ~ [n-1]행
	if (placementInfo.nHorEuroform >= 3) {
		moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('z', params_PINB.ang, placementInfo.height [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
			width = 0.0;
			for (yy = 0 ; yy < placementInfo.nHorEuroform ; ++yy) {
				// 1열
				if (yy == 0) {
					elemList.Push (placePINB (params_PINB));
					moveIn3D ('x', params_PINB.ang, placementInfo.width [0] - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					width += placementInfo.width [0] - 0.150;
				// 마지막 열
				} else if (yy == placementInfo.nHorEuroform - 1) {
					width += placementInfo.width [placementInfo.nHorEuroform-1] - 0.150;
					moveIn3D ('x', params_PINB.ang, placementInfo.width [placementInfo.nHorEuroform-1] - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					elemList.Push (placePINB (params_PINB));
				// 나머지 열
				} else {
					width += placementInfo.width [yy];
					if (abs (placementInfo.width [yy] - 0.600) < EPS) {
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.500) < EPS) {
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.450) < EPS) {
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.400) < EPS) {
						moveIn3D ('x', params_PINB.ang, 0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.300) < EPS) {
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.200) < EPS) {
						moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, 0.050, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					}
				}
			}
			moveIn3D ('x', params_PINB.ang, -width, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			moveIn3D ('z', params_PINB.ang, placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		}
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

	moveIn3D ('x', params_PINB.ang, placementInfo.width [0] - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('y', params_PINB.ang, -(0.2135), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('z', params_PINB.ang, placementInfo.height [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

	// 1열
	height = 0.0;
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		moveIn3D ('z', params_PINB.ang, placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		height += placementInfo.height [xx];
	}
	// 2열
	moveIn3D ('x', params_PINB.ang, -(placementInfo.width [0] - 0.150) + cell.horLen + (-placementInfo.width [placementInfo.nHorEuroform-1] + 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('z', params_PINB.ang, -height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		moveIn3D ('z', params_PINB.ang, placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		height += placementInfo.height [xx];
	}

	// 벽체 타이
	//if (placingZone.bDoubleSide) {
	//	params_TIE.leftBottomX = cell.leftBottomX;
	//	params_TIE.leftBottomY = cell.leftBottomY;
	//	params_TIE.leftBottomZ = cell.leftBottomZ;
	//	params_TIE.ang = cell.ang;
	//	remainder = fmod ((infoWall.wallThk + 0.327), 0.100);
	//	params_TIE.boltLen = (infoWall.wallThk + 0.327 + (0.100 - remainder));
	//	params_TIE.pipeBeg = 0.0365 + 0.1635;
	//	params_TIE.pipeEnd = 0.0365 + 0.1635 + infoWall.wallThk;
	//	params_TIE.clampBeg = 0.0365;
	//	params_TIE.clampEnd = 0.0365 + infoWall.wallThk + 0.327;

	//	moveIn3D ('x', params_TIE.ang, placementInfo.width [0] - 0.150, &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//	moveIn3D ('y', params_TIE.ang, -(0.1635 + 0.0365), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//	moveIn3D ('z', params_TIE.ang, 0.350, &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);

	//	for (xx = 0 ; xx < 2 ; ++xx) {
	//		for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
	//			// 최하위 행
	//			if (yy == 0) {
	//				elemList.Push (placeTIE (params_TIE));
	//				moveIn3D ('z', params_TIE.ang, placementInfo.height [yy], &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//	
	//			// 최상위 행
	//			} else if (yy == placementInfo.nVerEuroform - 1) {
	//				moveIn3D ('z', params_TIE.ang, placementInfo.height [yy] - 0.350*2, &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//				elemList.Push (placeTIE (params_TIE));
	//				moveIn3D ('x', params_TIE.ang, -(placementInfo.width [0] - 0.150) + cell.horLen + (-placementInfo.width [placementInfo.nHorEuroform-1] + 0.150), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//				moveIn3D ('z', params_TIE.ang, 0.350 - cell.verLen + 0.350, &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//			
	//			// 2 ~ [n-1]행
	//			} else {
	//				elemList.Push (placeTIE (params_TIE));
	//				moveIn3D ('z', params_TIE.ang, placementInfo.height [yy], &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//			}
	//		}
	//	}
	//}

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

	moveIn3D ('x', params_PUSH.ang, placementInfo.width [0] - 0.150 - 0.100, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	moveIn3D ('y', params_PUSH.ang, -0.1725, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	moveIn3D ('z', params_PUSH.ang, 0.231, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);

	// 처음 행
	elemList.Push (placePUSH (params_PUSH));
	moveIn3D ('x', params_PUSH.ang, -(placementInfo.width [0] - 0.150) + cell.horLen + (-placementInfo.width [placementInfo.nHorEuroform-1] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	elemList.Push (placePUSH (params_PUSH));
	moveIn3D ('x', params_PUSH.ang, (placementInfo.width [0] - 0.150) - cell.horLen - (-placementInfo.width [placementInfo.nHorEuroform-1] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	//if (cell.verLen > 4.000) {
	//	elev_headpiece = 4.000 * 0.80;
	//} else {
	//	elev_headpiece = cell.verLen * 0.80;
	//}
	elev_headpiece = 2.100;
	moveIn3D ('z', params_PUSH.ang, -0.231 + elev_headpiece, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	// 마지막 행
	elemList.Push (placePUSH (params_PUSH));
	moveIn3D ('x', params_PUSH.ang, -(placementInfo.width [0] - 0.150) + cell.horLen + (-placementInfo.width [placementInfo.nHorEuroform-1] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	elemList.Push (placePUSH (params_PUSH));

	// 결합철물
	params_JOIN.leftBottomX = cell.leftBottomX;
	params_JOIN.leftBottomY = cell.leftBottomY;
	params_JOIN.leftBottomZ = cell.leftBottomZ;
	params_JOIN.ang = cell.ang;

	moveIn3D ('x', params_JOIN.ang, placementInfo.width [0] - 0.150, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('y', params_JOIN.ang, -0.0455, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('z', params_JOIN.ang, 0.150, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

	// 처음 행
	elemList.Push (placeJOIN (params_JOIN));
	moveIn3D ('x', params_JOIN.ang, -(placementInfo.width [0] - 0.150) + cell.horLen + (-placementInfo.width [placementInfo.nHorEuroform-1] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	elemList.Push (placeJOIN (params_JOIN));
	moveIn3D ('x', params_JOIN.ang, (placementInfo.width [0] - 0.150) - cell.horLen - (-placementInfo.width [placementInfo.nHorEuroform-1] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('z', params_JOIN.ang, cell.verLen - 0.300, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

	// 마지막 행
	elemList.Push (placeJOIN (params_JOIN));
	moveIn3D ('x', params_JOIN.ang, -(placementInfo.width [0] - 0.150) + cell.horLen + (-placementInfo.width [placementInfo.nHorEuroform-1] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
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
		moveIn3D ('x', cell.ang, cell.horLen, &cell.leftBottomX, &cell.leftBottomY, &cell.leftBottomY);
		moveIn3D ('y', cell.ang, infoWall.wallThk, &cell.leftBottomX, &cell.leftBottomY, &cell.leftBottomY);
		cell.ang += DegreeToRad (180.0);

		// 유로폼 설치 (반대편에서 변경됨)
		params_UFOM.leftBottomX = cell.leftBottomX;
		params_UFOM.leftBottomY = cell.leftBottomY;
		params_UFOM.leftBottomZ = cell.leftBottomZ;
		params_UFOM.ang = cell.ang;
		params_UFOM.u_ins_wall = true;

		for (xx = placementInfo.nHorEuroform - 1 ; xx >= 0 ; --xx) {
			height = 0.0;
			for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
				params_UFOM.width	= placementInfo.width [xx];
				params_UFOM.height	= placementInfo.height [yy];
				height += placementInfo.height [yy];
				elemList.Push (placeUFOM (params_UFOM));
				moveIn3D ('z', params_UFOM.ang, placementInfo.height [yy], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
			}
			moveIn3D ('x', params_UFOM.ang, placementInfo.width [xx], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
			moveIn3D ('z', params_UFOM.ang, -height, &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		}

		// 비계 파이프 (수평) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.horLen - (horizontalGap * 2);
		params_SPIP.pipeAng = DegreeToRad (0);

		moveIn3D ('x', params_SPIP.ang, horizontalGap, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.025), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('z', params_SPIP.ang, 0.150 - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

		for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
			if (xx == 0) {
				// 1행
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('z', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('z', params_SPIP.ang, -0.031 - 0.150 + placementInfo.height [xx] - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			} else if (xx == placementInfo.nVerEuroform) {
				// 마지막 행
				moveIn3D ('z', params_SPIP.ang, -0.150, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('z', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
			} else {
				// 나머지 행
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('z', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('z', params_SPIP.ang, -0.031 + placementInfo.height [xx] - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			}
		}

		// 비계 파이프 (수직) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.verLen - 0.100;
		params_SPIP.pipeAng = DegreeToRad (90);

		moveIn3D ('x', params_SPIP.ang, placementInfo.width [placementInfo.nHorEuroform-1] - 0.150 - 0.035, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.075), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('z', params_SPIP.ang, 0.050, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

		// 1열
		elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('x', params_SPIP.ang, 0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('x', params_SPIP.ang, -0.070 - (placementInfo.width [placementInfo.nHorEuroform-1] - 0.150) + cell.horLen + (-placementInfo.width [0] + 0.150), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		// 2열
		elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('x', params_SPIP.ang, 0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
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

		moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		// 최하단 행
		moveIn3D ('z', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		width = 0.0;
		for (xx = placementInfo.nHorEuroform - 1 ; xx > 0 ; --xx) {
			width += placementInfo.width [xx];
			moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

			elemList.Push (placePINB (params_PINB));
		}
		// 최상단 행
		moveIn3D ('x', params_PINB.ang, -width, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('z', params_PINB.ang, cell.verLen - 0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		for (xx = placementInfo.nHorEuroform - 1 ; xx > 0 ; --xx) {
			moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

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

		moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		if (placementInfo.nHorEuroform >= 3) {
			moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			moveIn3D ('z', params_PINB.ang, placementInfo.height [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
				width = 0.0;
				for (yy = placementInfo.nHorEuroform - 1 ; yy >= 0 ; --yy) {
					// 1열
					if (yy == placementInfo.nHorEuroform - 1) {
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('x', params_PINB.ang, placementInfo.width [placementInfo.nHorEuroform-1] - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						width += placementInfo.width [placementInfo.nHorEuroform-1] - 0.150;
					// 마지막 열
					} else if (yy == 0) {
						width += placementInfo.width [0] - 0.150;
						moveIn3D ('x', params_PINB.ang, placementInfo.width [0] - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
					// 나머지 열
					} else {
						width += placementInfo.width [yy];
						if (abs (placementInfo.width [yy] - 0.600) < EPS) {
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.500) < EPS) {
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.450) < EPS) {
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.400) < EPS) {
							moveIn3D ('x', params_PINB.ang, 0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.300) < EPS) {
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.200) < EPS) {
							moveIn3D ('x', params_PINB.ang, 0.050, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						}
					}
				}
				moveIn3D ('x', params_PINB.ang, -width, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
				moveIn3D ('z', params_PINB.ang, placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			}
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

		moveIn3D ('x', params_PINB.ang, placementInfo.width [placementInfo.nHorEuroform-1] - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('y', params_PINB.ang, -(0.2135), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('z', params_PINB.ang, placementInfo.height [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		// 1열
		height = 0.0;
		for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
			elemList.Push (placePINB (params_PINB));
			moveIn3D ('z', params_PINB.ang, placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			height += placementInfo.height [xx];
		}
		// 2열
		moveIn3D ('x', params_PINB.ang, -(placementInfo.width [placementInfo.nHorEuroform-1] - 0.150) + cell.horLen + (-placementInfo.width [0] + 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('z', params_PINB.ang, -height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
			elemList.Push (placePINB (params_PINB));
			moveIn3D ('z', params_PINB.ang, placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
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

		moveIn3D ('x', params_PUSH.ang, placementInfo.width [placementInfo.nHorEuroform-1] - 0.150 - 0.100, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		moveIn3D ('y', params_PUSH.ang, -0.1725, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		moveIn3D ('z', params_PUSH.ang, 0.231, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);

		// 처음 행
		elemList.Push (placePUSH (params_PUSH));
		moveIn3D ('x', params_PUSH.ang, -(placementInfo.width [placementInfo.nHorEuroform-1] - 0.150) + cell.horLen + (-placementInfo.width [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		elemList.Push (placePUSH (params_PUSH));
		moveIn3D ('x', params_PUSH.ang, (placementInfo.width [placementInfo.nHorEuroform-1] - 0.150) - cell.horLen - (-placementInfo.width [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		//if (cell.verLen > 4.000) {
		//	elev_headpiece = 4.000 * 0.80;
		//} else {
		//	elev_headpiece = cell.verLen * 0.80;
		//}
		elev_headpiece = 2.100;
		moveIn3D ('z', params_PUSH.ang, -0.231 + elev_headpiece, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		// 마지막 행
		elemList.Push (placePUSH (params_PUSH));
		moveIn3D ('x', params_PUSH.ang, -(placementInfo.width [placementInfo.nHorEuroform-1] - 0.150) + cell.horLen + (-placementInfo.width [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		elemList.Push (placePUSH (params_PUSH));

		// 결합철물
		params_JOIN.leftBottomX = cell.leftBottomX;
		params_JOIN.leftBottomY = cell.leftBottomY;
		params_JOIN.leftBottomZ = cell.leftBottomZ;
		params_JOIN.ang = cell.ang;

		moveIn3D ('x', params_JOIN.ang, placementInfo.width [placementInfo.nHorEuroform-1] - 0.150, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('y', params_JOIN.ang, -0.0455, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('z', params_JOIN.ang, 0.150, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

		// 처음 행
		elemList.Push (placeJOIN (params_JOIN));
		moveIn3D ('x', params_JOIN.ang, -(placementInfo.width [placementInfo.nHorEuroform-1] - 0.150) + cell.horLen + (-placementInfo.width [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		elemList.Push (placeJOIN (params_JOIN));
		moveIn3D ('x', params_JOIN.ang, (placementInfo.width [placementInfo.nHorEuroform-1] - 0.150) - cell.horLen - (-placementInfo.width [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('z', params_JOIN.ang, cell.verLen - 0.300, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

		// 마지막 행
		elemList.Push (placeJOIN (params_JOIN));
		moveIn3D ('x', params_JOIN.ang, -(placementInfo.width [placementInfo.nHorEuroform-1] - 0.150) + cell.horLen + (-placementInfo.width [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
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

// 테이블폼 배치하기 - 세로 방향 (타입2) ...
GSErrCode	WallTableformPlacingZone::placeTableformOnWall_Vertical_Type2 (CellForWallTableform cell)
{
	GSErrCode	err = NoError;
	placementInfoForWallTableform	placementInfo;

	short		xx, yy;
	double		width, height;
	//double		remainder;				// fmod 함수에 쓸 변수
	double		elev_headpiece;
	double		horizontalGap = 0.050;	// 수평재 양쪽 이격거리
	API_Guid	tempGuid;
	Cylinder	cylinder;

	Euroform		params_UFOM;
	SquarePipe		params_SPIP;
	HeadpieceOfPushPullProps	params_PUSH;
	MetalFittings	params_JOIN;
	EuroformHook	params_HOOK;
	RectPipeHanger	params_HANG;

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
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 2.250) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 2.200) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 2.150) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 2.100) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 2.050) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 2.000) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.950) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.900) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.850) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.800) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.750) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.500;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.700) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.650) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.600) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.550) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.500) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.450) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.500;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.400) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.500;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.350) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.300) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.250) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.200) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.150) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.100) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.050) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 1.000) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 0.950) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 0.900) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 0.850) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 0.800) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.250;
	} else if (abs (cell.horLen - 0.750) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.200;
	} else if (abs (cell.horLen - 0.700) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.250;
		placingZone.verticalBarRightOffset = 0.150;
	} else if (abs (cell.horLen - 0.650) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.200;
		placingZone.verticalBarRightOffset = 0.150;
	} else if (abs (cell.horLen - 0.600) < EPS) {
		placementInfo.nHorEuroform = 1;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 2;
		placingZone.verticalBarLeftOffset = 0.150;
		placingZone.verticalBarRightOffset = 0.150;
	} else if (abs (cell.horLen - 0.500) < EPS) {
		placementInfo.nHorEuroform = 1;
		placementInfo.width [0] = 0.500;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 1;
		placingZone.verticalBarLeftOffset = 0.200;
		placingZone.verticalBarRightOffset = 0.200;
	} else if (abs (cell.horLen - 0.450) < EPS) {
		placementInfo.nHorEuroform = 1;
		placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.025;
		placingZone.nVerticalBar = 1;
		placingZone.verticalBarLeftOffset = 0.200;
		placingZone.verticalBarRightOffset = 0.200;
	} else if (abs (cell.horLen - 0.400) < EPS) {
		placementInfo.nHorEuroform = 1;
		placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		horizontalGap = 0.050;
		placingZone.nVerticalBar = 1;
		placingZone.verticalBarLeftOffset = 0.150;
		placingZone.verticalBarRightOffset = 0.150;
	} else {
		placementInfo.nHorEuroform = 0;
		placementInfo.width [0] = 0.0;		placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		placingZone.nVerticalBar = 0;
		placingZone.verticalBarLeftOffset = 0.0;
		placingZone.verticalBarRightOffset = 0.0;
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
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 4.500) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 0.900;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 4.200) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.900;
		placementInfo.height [3] = 0.900;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 3.900) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.900;
		placementInfo.height [3] = 0.600;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 3.600) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 3.300) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.900;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 3.000) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.600;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 2.700) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 0.900;
		placementInfo.height [2] = 0.600;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 2.400) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 2.100) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 1.200;
		placementInfo.height [1] = 0.900;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 1.800) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.900;
		placementInfo.height [1] = 0.900;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else if (abs (cell.verLen - 1.500) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.900;
		placementInfo.height [1] = 0.600;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
	} else {
		placementInfo.nVerEuroform = 0;
		placementInfo.height [0] = 0.0;
		placementInfo.height [1] = 0.0;
		placementInfo.height [2] = 0.0;
		placementInfo.height [3] = 0.0;
		placementInfo.height [4] = 0.0;
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
	params_UFOM.u_ins_wall = true;

	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		height = 0.0;
		for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
			params_UFOM.width	= placementInfo.width [xx];
			params_UFOM.height	= placementInfo.height [yy];
			height += placementInfo.height [yy];
			elemList.Push (placeUFOM (params_UFOM));
			moveIn3D ('z', params_UFOM.ang, placementInfo.height [yy], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		}
		moveIn3D ('x', params_UFOM.ang, placementInfo.width [xx], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		moveIn3D ('z', params_UFOM.ang, -height, &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
	}

	// 비계 파이프 (수평) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.horLen - (horizontalGap * 2);
	params_SPIP.pipeAng = DegreeToRad (0);

	moveIn3D ('x', params_SPIP.ang, horizontalGap, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.025), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('z', params_SPIP.ang, 0.150 + 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

	cylinder.angleFromPlane = DegreeToRad (90.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = cell.ang;
	cylinder.leftBottomX = params_SPIP.leftBottomX;
	cylinder.leftBottomY = params_SPIP.leftBottomY;
	cylinder.leftBottomZ = params_SPIP.leftBottomZ;

	for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
		if (xx == 0) {
			// 1행
			tempGuid = placeSPIP (params_SPIP);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = params_SPIP.leftBottomX;
			cylinder.leftBottomY = params_SPIP.leftBottomY;
			cylinder.leftBottomZ = params_SPIP.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHOLE (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + cell.horLen - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHOLE (tempGuid, cylinder));
			}
			moveIn3D ('z', params_SPIP.ang, -0.031 - 0.150 + placementInfo.height [xx], &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			moveIn3D ('x', cylinder.ang, 0.300 - cell.horLen + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		} else if (xx == placementInfo.nVerEuroform) {
			// 마지막 행
			moveIn3D ('z', params_SPIP.ang, -0.150 + 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			tempGuid = placeSPIP (params_SPIP);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = params_SPIP.leftBottomX;
			cylinder.leftBottomY = params_SPIP.leftBottomY;
			cylinder.leftBottomZ = params_SPIP.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHOLE (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + cell.horLen - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHOLE (tempGuid, cylinder));
			}
		} else {
			// 나머지 행
			tempGuid = placeSPIP (params_SPIP);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = params_SPIP.leftBottomX;
			cylinder.leftBottomY = params_SPIP.leftBottomY;
			cylinder.leftBottomZ = params_SPIP.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHOLE (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + cell.horLen - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHOLE (tempGuid, cylinder));
			}
			moveIn3D ('z', params_SPIP.ang, placementInfo.height [xx], &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			moveIn3D ('x', cylinder.ang, 0.300 - cell.horLen + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		}
	}

	// 비계 파이프 (수직) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.verLen - 0.100;
	params_SPIP.pipeAng = DegreeToRad (90);

	moveIn3D ('x', params_SPIP.ang, horizontalGap + placingZone.verticalBarLeftOffset, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.075), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('z', params_SPIP.ang, 0.050, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

	// 1열
	cylinder.angleFromPlane = DegreeToRad (0.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = cell.ang;
	cylinder.leftBottomX = params_SPIP.leftBottomX;
	cylinder.leftBottomY = params_SPIP.leftBottomY;
	cylinder.leftBottomZ = params_SPIP.leftBottomZ;

	tempGuid = placeSPIP (params_SPIP);
	elemList.Push (tempGuid);
	moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHOLE (tempGuid, cylinder));
	}
	moveIn3D ('z', cylinder.ang, -0.300 + cell.verLen - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHOLE (tempGuid, cylinder));
	}
	moveIn3D ('x', params_SPIP.ang, -(horizontalGap + placingZone.verticalBarLeftOffset) + cell.horLen - (horizontalGap + placingZone.verticalBarRightOffset), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

	if (placingZone.nVerticalBar > 1) {
		// 2열
		tempGuid = placeSPIP (params_SPIP);
		elemList.Push (tempGuid);
		cylinder.leftBottomX = params_SPIP.leftBottomX;
		cylinder.leftBottomY = params_SPIP.leftBottomY;
		cylinder.leftBottomZ = params_SPIP.leftBottomZ;
		moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHOLE (tempGuid, cylinder));
		}
		moveIn3D ('z', cylinder.ang, -0.300 + cell.verLen - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHOLE (tempGuid, cylinder));
		}
	}

	// 유로폼 후크 배치 (수평 - 최하단, 최상단)
	params_HOOK.leftBottomX = cell.leftBottomX;
	params_HOOK.leftBottomY = cell.leftBottomY;
	params_HOOK.leftBottomZ = cell.leftBottomZ;
	params_HOOK.ang = cell.ang;
	params_HOOK.iiHookType = 2;
	params_HOOK.iHookShape = 2;
	params_HOOK.angX = DegreeToRad (0.0);
	params_HOOK.angY = DegreeToRad (90.0);

	moveIn3D ('y', params_HOOK.ang, -0.0885, &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);

	if (placementInfo.nHorEuroform >= 3) {
		moveIn3D ('x', params_HOOK.ang, placementInfo.width [0], &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
		moveIn3D ('z', params_HOOK.ang, 0.030 + 0.150, &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
		// 1행
		width = 0.0;
		for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
			width += placementInfo.width [xx];
			elemList.Push (placeHOOK (params_HOOK));
			moveIn3D ('x', params_HOOK.ang, placementInfo.width [xx], &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
		}
		moveIn3D ('x', params_HOOK.ang, -width, &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
		moveIn3D ('z', params_HOOK.ang, -0.150 + cell.verLen - 0.150, &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);

		// 마지막 행
		for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
			width += placementInfo.width [xx];
			elemList.Push (placeHOOK (params_HOOK));
			moveIn3D ('x', params_HOOK.ang, placementInfo.width [xx], &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
		}
	}

	// 각파이프행거 배치 (수평 - 최하단, 최상단을 제외한 나머지)
	params_HANG.leftBottomX = cell.leftBottomX;
	params_HANG.leftBottomY = cell.leftBottomY;
	params_HANG.leftBottomZ = cell.leftBottomZ;
	params_HANG.ang = cell.ang;
	params_HANG.angX = DegreeToRad (0.0);
	params_HANG.angY = DegreeToRad (270.0);

	moveIn3D ('y', params_HANG.ang, -0.0635, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);

	// 2 ~ [n-1]행
	if (placementInfo.nHorEuroform >= 3) {
		moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
		moveIn3D ('z', params_HANG.ang, placementInfo.height [0], &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
		for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
			width = 0.0;
			for (yy = 0 ; yy < placementInfo.nHorEuroform ; ++yy) {
				// 1열
				if (yy == 0) {
					elemList.Push (placeHANG (params_HANG));
					moveIn3D ('x', params_HANG.ang, placementInfo.width [0] - 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
					width += placementInfo.width [0] - 0.150;
				// 마지막 열
				} else if (yy == placementInfo.nHorEuroform - 1) {
					width += placementInfo.width [placementInfo.nHorEuroform-1] - 0.150;
					moveIn3D ('x', params_HANG.ang, placementInfo.width [placementInfo.nHorEuroform-1] - 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
					elemList.Push (placeHANG (params_HANG));
				// 나머지 열
				} else {
					width += placementInfo.width [yy];
					if (abs (placementInfo.width [yy] - 0.600) < EPS) {
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.300, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.500) < EPS) {
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.200, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.450) < EPS) {
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.400) < EPS) {
						moveIn3D ('x', params_HANG.ang, 0.100, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.200, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.100, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.300) < EPS) {
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
					} else if (abs (placementInfo.width [yy] - 0.200) < EPS) {
						moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, 0.050, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
					}
				}
			}
			moveIn3D ('x', params_HANG.ang, -width, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
			moveIn3D ('z', params_HANG.ang, placementInfo.height [xx], &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
		}
	}

	// 헤드 피스
	params_PUSH.leftBottomX = cell.leftBottomX;
	params_PUSH.leftBottomY = cell.leftBottomY;
	params_PUSH.leftBottomZ = cell.leftBottomZ;
	params_PUSH.ang = cell.ang;

	moveIn3D ('x', params_PUSH.ang, horizontalGap + placingZone.verticalBarLeftOffset - 0.0375, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	moveIn3D ('y', params_PUSH.ang, -0.2685, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	moveIn3D ('z', params_PUSH.ang, 0.291, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);

	// 처음 행
	elemList.Push (placePUSH2 (params_PUSH));
	moveIn3D ('x', params_PUSH.ang, -(horizontalGap + placingZone.verticalBarLeftOffset - 0.0375) + cell.horLen - (horizontalGap + placingZone.verticalBarRightOffset + 0.0375), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	if (placingZone.nVerticalBar > 1)
		elemList.Push (placePUSH2 (params_PUSH));
	moveIn3D ('x', params_PUSH.ang, (horizontalGap + placingZone.verticalBarLeftOffset - 0.0375) - cell.horLen + (horizontalGap + placingZone.verticalBarRightOffset + 0.0375), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	//if (cell.verLen > 4.000) {
	//	elev_headpiece = 4.000 * 0.80;
	//} else {
	//	elev_headpiece = cell.verLen * 0.80;
	//}
	elev_headpiece = 1.900;
	moveIn3D ('z', params_PUSH.ang, elev_headpiece, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	// 마지막 행
		elemList.Push (placePUSH2 (params_PUSH));
	moveIn3D ('x', params_PUSH.ang, -(horizontalGap + placingZone.verticalBarLeftOffset - 0.0375) + cell.horLen - (horizontalGap + placingZone.verticalBarRightOffset + 0.0375), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	if (placingZone.nVerticalBar > 1)
		elemList.Push (placePUSH2 (params_PUSH));

	// 결합철물
	params_JOIN.leftBottomX = cell.leftBottomX;
	params_JOIN.leftBottomY = cell.leftBottomY;
	params_JOIN.leftBottomZ = cell.leftBottomZ;
	params_JOIN.ang = cell.ang;
	params_JOIN.angX = DegreeToRad (180.0);
	params_JOIN.angY = DegreeToRad (0.0);
	
	moveIn3D ('x', params_JOIN.ang, horizontalGap + placingZone.verticalBarLeftOffset - 0.081, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('y', params_JOIN.ang, -0.1155, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('z', params_JOIN.ang, 0.230, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

	// 처음 열
	for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
		// 1행
		if (xx == 0) {
			elemList.Push (placeJOIN2 (params_JOIN));
			moveIn3D ('z', params_JOIN.ang, placementInfo.height [xx] - 0.180, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		// 마지막 행
		} else if (xx == placementInfo.nVerEuroform) {
			moveIn3D ('z', params_JOIN.ang, -0.120, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
			elemList.Push (placeJOIN2 (params_JOIN));
		// 나머지 행
		} else {
			elemList.Push (placeJOIN2 (params_JOIN));
			moveIn3D ('z', params_JOIN.ang, placementInfo.height [xx], &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		}
	}
	moveIn3D ('x', params_JOIN.ang, -(horizontalGap + placingZone.verticalBarLeftOffset - 0.081) + cell.horLen - (horizontalGap + placingZone.verticalBarRightOffset + 0.081), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('z', params_JOIN.ang, 0.300 - cell.verLen, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

	if (placingZone.nVerticalBar > 1) {
		// 마지막 열
		for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
			// 1행
			if (xx == 0) {
				elemList.Push (placeJOIN2 (params_JOIN));
				moveIn3D ('z', params_JOIN.ang, placementInfo.height [xx] - 0.180, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
			// 마지막 행
			} else if (xx == placementInfo.nVerEuroform) {
				moveIn3D ('z', params_JOIN.ang, -0.120, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
				elemList.Push (placeJOIN2 (params_JOIN));
			// 나머지 행
			} else {
				elemList.Push (placeJOIN2 (params_JOIN));
				moveIn3D ('z', params_JOIN.ang, placementInfo.height [xx], &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
			}
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
		moveIn3D ('x', cell.ang, cell.horLen, &cell.leftBottomX, &cell.leftBottomY, &cell.leftBottomY);
		moveIn3D ('y', cell.ang, infoWall.wallThk, &cell.leftBottomX, &cell.leftBottomY, &cell.leftBottomY);
		cell.ang += DegreeToRad (180.0);

		// 유로폼 설치 (반대편에서 변경됨)
		params_UFOM.leftBottomX = cell.leftBottomX;
		params_UFOM.leftBottomY = cell.leftBottomY;
		params_UFOM.leftBottomZ = cell.leftBottomZ;
		params_UFOM.ang = cell.ang;
		params_UFOM.u_ins_wall = true;

		for (xx = placementInfo.nHorEuroform - 1 ; xx >= 0 ; --xx) {
			height = 0.0;
			for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
				params_UFOM.width	= placementInfo.width [xx];
				params_UFOM.height	= placementInfo.height [yy];
				height += placementInfo.height [yy];
				elemList.Push (placeUFOM (params_UFOM));
				moveIn3D ('z', params_UFOM.ang, placementInfo.height [yy], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
			}
			moveIn3D ('x', params_UFOM.ang, placementInfo.width [xx], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
			moveIn3D ('z', params_UFOM.ang, -height, &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		}

		// 비계 파이프 (수평) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.horLen - (horizontalGap * 2);
		params_SPIP.pipeAng = DegreeToRad (0);

		moveIn3D ('x', params_SPIP.ang, horizontalGap, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.025), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('z', params_SPIP.ang, 0.150 + 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

		cylinder.angleFromPlane = DegreeToRad (90.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = cell.ang;
		cylinder.leftBottomX = params_SPIP.leftBottomX;
		cylinder.leftBottomY = params_SPIP.leftBottomY;
		cylinder.leftBottomZ = params_SPIP.leftBottomZ;
		moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);

		for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
			if (xx == 0) {
				// 1행
				tempGuid = placeSPIP (params_SPIP);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = params_SPIP.leftBottomX;
				cylinder.leftBottomY = params_SPIP.leftBottomY;
				cylinder.leftBottomZ = params_SPIP.leftBottomZ;
				moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHOLE (tempGuid, cylinder));
				}
				moveIn3D ('x', cylinder.ang, -0.300 + cell.horLen - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHOLE (tempGuid, cylinder));
				}
				moveIn3D ('z', params_SPIP.ang, -0.031 - 0.150 + placementInfo.height [xx], &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				moveIn3D ('x', cylinder.ang, 0.300 - cell.horLen + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			} else if (xx == placementInfo.nVerEuroform) {
				// 마지막 행
				moveIn3D ('z', params_SPIP.ang, -0.150 + 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				tempGuid = placeSPIP (params_SPIP);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = params_SPIP.leftBottomX;
				cylinder.leftBottomY = params_SPIP.leftBottomY;
				cylinder.leftBottomZ = params_SPIP.leftBottomZ;
				moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHOLE (tempGuid, cylinder));
				}
				moveIn3D ('x', cylinder.ang, -0.300 + cell.horLen - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHOLE (tempGuid, cylinder));
				}
			} else {
				// 나머지 행
				tempGuid = placeSPIP (params_SPIP);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = params_SPIP.leftBottomX;
				cylinder.leftBottomY = params_SPIP.leftBottomY;
				cylinder.leftBottomZ = params_SPIP.leftBottomZ;
				moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHOLE (tempGuid, cylinder));
				}
				moveIn3D ('x', cylinder.ang, -0.300 + cell.horLen - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHOLE (tempGuid, cylinder));
				}
				moveIn3D ('z', params_SPIP.ang, placementInfo.height [xx], &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				moveIn3D ('x', cylinder.ang, 0.300 - cell.horLen + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			}
		}

		// 비계 파이프 (수직) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.verLen - 0.100;
		params_SPIP.pipeAng = DegreeToRad (90);

		moveIn3D ('x', params_SPIP.ang, horizontalGap + placingZone.verticalBarRightOffset, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.075), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('z', params_SPIP.ang, 0.050, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

		// 1열
		cylinder.angleFromPlane = DegreeToRad (0.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = cell.ang;
		cylinder.leftBottomX = params_SPIP.leftBottomX;
		cylinder.leftBottomY = params_SPIP.leftBottomY;
		cylinder.leftBottomZ = params_SPIP.leftBottomZ;

		tempGuid = placeSPIP (params_SPIP);
		elemList.Push (tempGuid);
		moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHOLE (tempGuid, cylinder));
		}
		moveIn3D ('z', cylinder.ang, -0.300 + cell.verLen - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHOLE (tempGuid, cylinder));
		}
		moveIn3D ('x', params_SPIP.ang, -(horizontalGap + placingZone.verticalBarRightOffset) + cell.horLen - (horizontalGap + placingZone.verticalBarLeftOffset), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

		if (placingZone.nVerticalBar > 1) {
			// 2열
			tempGuid = placeSPIP (params_SPIP);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = params_SPIP.leftBottomX;
			cylinder.leftBottomY = params_SPIP.leftBottomY;
			cylinder.leftBottomZ = params_SPIP.leftBottomZ;
			moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHOLE (tempGuid, cylinder));
			}
			moveIn3D ('z', cylinder.ang, -0.300 + cell.verLen - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHOLE (tempGuid, cylinder));
			}
		}

		// 유로폼 후크 배치 (수평 - 최하단, 최상단)
		params_HOOK.leftBottomX = cell.leftBottomX;
		params_HOOK.leftBottomY = cell.leftBottomY;
		params_HOOK.leftBottomZ = cell.leftBottomZ;
		params_HOOK.ang = cell.ang;
		params_HOOK.iiHookType = 2;
		params_HOOK.iHookShape = 2;
		params_HOOK.angX = DegreeToRad (0.0);
		params_HOOK.angY = DegreeToRad (90.0);

		moveIn3D ('y', params_HOOK.ang, -0.0885, &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);

		if (placementInfo.nHorEuroform >= 3) {
			moveIn3D ('x', params_HOOK.ang, placementInfo.width [placementInfo.nHorEuroform-1], &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
			moveIn3D ('z', params_HOOK.ang, 0.030 + 0.150, &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
			// 1행
			width = 0.0;
			for (xx = placementInfo.nHorEuroform-2 ; xx >= 0 ; --xx) {
				width += placementInfo.width [xx];
				elemList.Push (placeHOOK (params_HOOK));
				moveIn3D ('x', params_HOOK.ang, placementInfo.width [xx], &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
			}
			moveIn3D ('x', params_HOOK.ang, -width, &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
			moveIn3D ('z', params_HOOK.ang, -0.150 + cell.verLen - 0.150, &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);

			// 마지막 행
			for (xx = placementInfo.nHorEuroform-2 ; xx >= 0 ; --xx) {
				width += placementInfo.width [xx];
				elemList.Push (placeHOOK (params_HOOK));
				moveIn3D ('x', params_HOOK.ang, placementInfo.width [xx], &params_HOOK.leftBottomX, &params_HOOK.leftBottomY, &params_HOOK.leftBottomZ);
			}
		}

		// 각파이프행거 배치 (수평 - 최하단, 최상단을 제외한 나머지)
		params_HANG.leftBottomX = cell.leftBottomX;
		params_HANG.leftBottomY = cell.leftBottomY;
		params_HANG.leftBottomZ = cell.leftBottomZ;
		params_HANG.ang = cell.ang;
		params_HANG.angX = DegreeToRad (0.0);
		params_HANG.angY = DegreeToRad (270.0);

		moveIn3D ('y', params_HANG.ang, -0.0635, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);

		// 2 ~ [n-1]행
		if (placementInfo.nHorEuroform >= 3) {
			moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
			moveIn3D ('z', params_HANG.ang, placementInfo.height [0], &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
			for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
				width = 0.0;
				for (yy = placementInfo.nHorEuroform-1 ; yy >= 0 ; --yy) {
					// 1열
					if (yy == placementInfo.nHorEuroform-1) {
						elemList.Push (placeHANG (params_HANG));
						moveIn3D ('x', params_HANG.ang, placementInfo.width [placementInfo.nHorEuroform-1] - 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						width += placementInfo.width [0] - 0.150;
					// 마지막 열
					} else if (yy == 0) {
						width += placementInfo.width [placementInfo.nHorEuroform-1] - 0.150;
						moveIn3D ('x', params_HANG.ang, placementInfo.width [0] - 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						elemList.Push (placeHANG (params_HANG));
					// 나머지 열
					} else {
						width += placementInfo.width [yy];
						if (abs (placementInfo.width [yy] - 0.600) < EPS) {
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.300, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.500) < EPS) {
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.200, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.450) < EPS) {
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.400) < EPS) {
							moveIn3D ('x', params_HANG.ang, 0.100, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.200, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.100, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.300) < EPS) {
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						} else if (abs (placementInfo.width [yy] - 0.200) < EPS) {
							moveIn3D ('x', params_HANG.ang, 0.150, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
							elemList.Push (placeHANG (params_HANG));
							moveIn3D ('x', params_HANG.ang, 0.050, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
						}
					}
				}
				moveIn3D ('x', params_HANG.ang, -width, &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
				moveIn3D ('z', params_HANG.ang, placementInfo.height [xx], &params_HANG.leftBottomX, &params_HANG.leftBottomY, &params_HANG.leftBottomZ);
			}
		}

		// 헤드 피스
		params_PUSH.leftBottomX = cell.leftBottomX;
		params_PUSH.leftBottomY = cell.leftBottomY;
		params_PUSH.leftBottomZ = cell.leftBottomZ;
		params_PUSH.ang = cell.ang;

		moveIn3D ('x', params_PUSH.ang, horizontalGap + placingZone.verticalBarRightOffset - 0.0375, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		moveIn3D ('y', params_PUSH.ang, -0.2685, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		moveIn3D ('z', params_PUSH.ang, 0.291, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);

		// 처음 행
		elemList.Push (placePUSH2 (params_PUSH));
		moveIn3D ('x', params_PUSH.ang, -(horizontalGap + placingZone.verticalBarRightOffset - 0.0375) + cell.horLen - (horizontalGap + placingZone.verticalBarLeftOffset + 0.0375), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		if (placingZone.nVerticalBar > 1)
			elemList.Push (placePUSH2 (params_PUSH));
		moveIn3D ('x', params_PUSH.ang, (horizontalGap + placingZone.verticalBarRightOffset - 0.0375) - cell.horLen + (horizontalGap + placingZone.verticalBarLeftOffset + 0.0375), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		//if (cell.verLen > 4.000) {
		//	elev_headpiece = 4.000 * 0.80;
		//} else {
		//	elev_headpiece = cell.verLen * 0.80;
		//}
		elev_headpiece = 1.900;
		moveIn3D ('z', params_PUSH.ang, elev_headpiece, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		// 마지막 행
		elemList.Push (placePUSH2 (params_PUSH));
		moveIn3D ('x', params_PUSH.ang, -(horizontalGap + placingZone.verticalBarRightOffset - 0.0375) + cell.horLen - (horizontalGap + placingZone.verticalBarLeftOffset + 0.0375), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		if (placingZone.nVerticalBar > 1)
			elemList.Push (placePUSH2 (params_PUSH));

		// 결합철물
		params_JOIN.leftBottomX = cell.leftBottomX;
		params_JOIN.leftBottomY = cell.leftBottomY;
		params_JOIN.leftBottomZ = cell.leftBottomZ;
		params_JOIN.ang = cell.ang;
		params_JOIN.angX = DegreeToRad (180.0);
		params_JOIN.angY = DegreeToRad (0.0);
	
		moveIn3D ('x', params_JOIN.ang, horizontalGap + placingZone.verticalBarRightOffset - 0.081, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('y', params_JOIN.ang, -0.1155, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('z', params_JOIN.ang, 0.230, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

		// 처음 열
		for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
			// 1행
			if (xx == 0) {
				elemList.Push (placeJOIN2 (params_JOIN));
				moveIn3D ('z', params_JOIN.ang, placementInfo.height [xx] - 0.180, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
			// 마지막 행
			} else if (xx == placementInfo.nVerEuroform) {
				moveIn3D ('z', params_JOIN.ang, -0.120, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
				elemList.Push (placeJOIN2 (params_JOIN));
			// 나머지 행
			} else {
				elemList.Push (placeJOIN2 (params_JOIN));
				moveIn3D ('z', params_JOIN.ang, placementInfo.height [xx], &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
			}
		}

		moveIn3D ('x', params_JOIN.ang, -(horizontalGap + placingZone.verticalBarRightOffset - 0.081) + cell.horLen - (horizontalGap + placingZone.verticalBarLeftOffset + 0.081), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('z', params_JOIN.ang, 0.300 - cell.verLen, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

		if (placingZone.nVerticalBar > 1) {
			// 마지막 열
			for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
				// 1행
				if (xx == 0) {
					elemList.Push (placeJOIN2 (params_JOIN));
					moveIn3D ('z', params_JOIN.ang, placementInfo.height [xx] - 0.180, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
				// 마지막 행
				} else if (xx == placementInfo.nVerEuroform) {
					moveIn3D ('z', params_JOIN.ang, -0.120, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
					elemList.Push (placeJOIN2 (params_JOIN));
				// 나머지 행
				} else {
					elemList.Push (placeJOIN2 (params_JOIN));
					moveIn3D ('z', params_JOIN.ang, placementInfo.height [xx], &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
				}
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

	return	err;
}

// 테이블폼 상단 배치하기 - 세로 방향
GSErrCode	WallTableformPlacingZone::placeTableformOnWall_Vertical (CellForWallTableform cell, UpperCellForWallTableform upperCell)
{
	GSErrCode	err = NoError;
	short	xx;
	double	remainWidth = abs (placingZone.marginTop - upperCell.formWidth1 - upperCell.formWidth2);
	double	width;
	placementInfoForWallTableform	placementInfo;

	Euroform	params_UFOM1 [4];
	Euroform	params_UFOM2 [4];
	Plywood		params_PLYW [4];
	Wood		params_TIMB [4];

	placementInfo.nHorEuroform = 0;
	placementInfo.nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		placementInfo.width [xx] = 0.0;
		placementInfo.height [xx] = 0.0;
	}

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
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.750) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.500;
		} else if (abs (cell.horLen - 1.700) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.650) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.600) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.550) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.500) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.600;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.450) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.500;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.400) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.500;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.350) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.300) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.250) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.200) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.150) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.100) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.050) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 1.000) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.950) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.900) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.850) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.800) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.400;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.750) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.700) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.300;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.650) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.200;	placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.600) < EPS) {
			placementInfo.nHorEuroform = 1;
			placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.500) < EPS) {
			placementInfo.nHorEuroform = 1;
			placementInfo.width [0] = 0.500;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.450) < EPS) {
			placementInfo.nHorEuroform = 1;
			placementInfo.width [0] = 0.450;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else if (abs (cell.horLen - 0.400) < EPS) {
			placementInfo.nHorEuroform = 1;
			placementInfo.width [0] = 0.400;	placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		} else {
			placementInfo.nHorEuroform = 0;
			placementInfo.width [0] = 0.0;		placementInfo.width [1] = 0.0;		placementInfo.width [2] = 0.0;		placementInfo.width [3] = 0.0;
		}

		// 합판 또는 목재의 전체 길이
		width = 0.0;
		for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
			width += placementInfo.width [xx];
		}

		//////////////////////////////////////////////////////////////// 현재면
		// 1열
		if (placementInfo.nHorEuroform >= 1) {
			// 1번째 행: 유로폼
			params_UFOM1 [0].leftBottomX = cell.leftBottomX;
			params_UFOM1 [0].leftBottomY = cell.leftBottomY;
			params_UFOM1 [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [0].ang = cell.ang;
			params_UFOM1 [0].u_ins_wall = false;
			params_UFOM1 [0].width = upperCell.formWidth1;
			params_UFOM1 [0].height = placementInfo.width [0];
			moveIn3D ('x', params_UFOM1 [0].ang, placementInfo.width [0], &params_UFOM1 [0].leftBottomX, &params_UFOM1 [0].leftBottomY, &params_UFOM1 [0].leftBottomZ);

			// 2번째 행: 유로폼
			params_UFOM2 [0].leftBottomX = cell.leftBottomX;
			params_UFOM2 [0].leftBottomY = cell.leftBottomY;
			params_UFOM2 [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [0].ang = cell.ang;
			params_UFOM2 [0].u_ins_wall = false;
			params_UFOM2 [0].width = upperCell.formWidth2;
			params_UFOM2 [0].height = placementInfo.width [0];
			moveIn3D ('x', params_UFOM2 [0].ang, placementInfo.width [0], &params_UFOM2 [0].leftBottomX, &params_UFOM2 [0].leftBottomY, &params_UFOM2 [0].leftBottomZ);

			// 3번째 행: 합판 또는 목재
			params_PLYW [0].leftBottomX = cell.leftBottomX;
			params_PLYW [0].leftBottomY = cell.leftBottomY;
			params_PLYW [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [0].ang = cell.ang;
			params_PLYW [0].p_wid = remainWidth;
			params_PLYW [0].p_leng = width;	//placementInfo.width [0];
			params_PLYW [0].w_dir_wall = false;

			params_TIMB [0].leftBottomX = cell.leftBottomX;
			params_TIMB [0].leftBottomY = cell.leftBottomY;
			params_TIMB [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [0].ang = cell.ang;
			params_TIMB [0].w_w = 0.050;
			params_TIMB [0].w_h = remainWidth;
			params_TIMB [0].w_leng = width;	//placementInfo.width [0];
			params_TIMB [0].w_ang = 0.0;
		}

		// 2열
		if (placementInfo.nHorEuroform >= 2) {
			// 1번째 행: 유로폼
			params_UFOM1 [1].leftBottomX = params_UFOM1 [0].leftBottomX;
			params_UFOM1 [1].leftBottomY = params_UFOM1 [0].leftBottomY;
			params_UFOM1 [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [1].ang = cell.ang;
			params_UFOM1 [1].u_ins_wall = false;
			params_UFOM1 [1].width = upperCell.formWidth1;
			params_UFOM1 [1].height = placementInfo.width [1];
			moveIn3D ('x', params_UFOM1 [1].ang, placementInfo.width [1], &params_UFOM1 [1].leftBottomX, &params_UFOM1 [1].leftBottomY, &params_UFOM1 [1].leftBottomZ);

			// 2번째 행: 유로폼
			params_UFOM2 [1].leftBottomX = params_UFOM2 [0].leftBottomX;
			params_UFOM2 [1].leftBottomY = params_UFOM2 [0].leftBottomY;
			params_UFOM2 [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [1].ang = cell.ang;
			params_UFOM2 [1].u_ins_wall = false;
			params_UFOM2 [1].width = upperCell.formWidth2;
			params_UFOM2 [1].height = placementInfo.width [1];
			moveIn3D ('x', params_UFOM2 [1].ang, placementInfo.width [1], &params_UFOM2 [1].leftBottomX, &params_UFOM2 [1].leftBottomY, &params_UFOM2 [1].leftBottomZ);

			// 3번째 행: 합판 또는 목재
			params_PLYW [1].leftBottomX = params_PLYW [0].leftBottomX;
			params_PLYW [1].leftBottomY = params_PLYW [0].leftBottomY;
			params_PLYW [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [1].ang = cell.ang;
			params_PLYW [1].p_wid = remainWidth;
			params_PLYW [1].p_leng = width;	//placementInfo.width [1];
			params_PLYW [1].w_dir_wall = false;
			moveIn3D ('x', params_PLYW [1].ang, placementInfo.width [0], &params_PLYW [1].leftBottomX, &params_PLYW [1].leftBottomY, &params_PLYW [1].leftBottomZ);

			params_TIMB [1].leftBottomX = params_TIMB [0].leftBottomX;
			params_TIMB [1].leftBottomY = params_TIMB [0].leftBottomY;
			params_TIMB [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [1].ang = cell.ang;
			params_TIMB [1].w_w = 0.050;
			params_TIMB [1].w_h = remainWidth;
			params_TIMB [1].w_leng = width;	//placementInfo.width [1];
			params_TIMB [1].w_ang = 0.0;
			moveIn3D ('x', params_TIMB [1].ang, placementInfo.width [0], &params_TIMB [1].leftBottomX, &params_TIMB [1].leftBottomY, &params_TIMB [1].leftBottomZ);
		}

		// 3열
		if (placementInfo.nHorEuroform >= 3) {
			// 1번째 행: 유로폼
			params_UFOM1 [2].leftBottomX = params_UFOM1 [1].leftBottomX;
			params_UFOM1 [2].leftBottomY = params_UFOM1 [1].leftBottomY;
			params_UFOM1 [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [2].ang = cell.ang;
			params_UFOM1 [2].u_ins_wall = false;
			params_UFOM1 [2].width = upperCell.formWidth1;
			params_UFOM1 [2].height = placementInfo.width [2];
			moveIn3D ('x', params_UFOM1 [2].ang, placementInfo.width [2], &params_UFOM1 [2].leftBottomX, &params_UFOM1 [2].leftBottomY, &params_UFOM1 [2].leftBottomZ);

			// 2번째 행: 유로폼
			params_UFOM2 [2].leftBottomX = params_UFOM2 [1].leftBottomX;
			params_UFOM2 [2].leftBottomY = params_UFOM2 [1].leftBottomY;
			params_UFOM2 [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [2].ang = cell.ang;
			params_UFOM2 [2].u_ins_wall = false;
			params_UFOM2 [2].width = upperCell.formWidth2;
			params_UFOM2 [2].height = placementInfo.width [2];
			moveIn3D ('x', params_UFOM2 [2].ang, placementInfo.width [2], &params_UFOM2 [2].leftBottomX, &params_UFOM2 [2].leftBottomY, &params_UFOM2 [2].leftBottomZ);

			// 3번째 행: 합판 또는 목재
			params_PLYW [2].leftBottomX = params_PLYW [1].leftBottomX;
			params_PLYW [2].leftBottomY = params_PLYW [1].leftBottomY;
			params_PLYW [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [2].ang = cell.ang;
			params_PLYW [2].p_wid = remainWidth;
			params_PLYW [2].p_leng = width;	//placementInfo.width [2];
			params_PLYW [2].w_dir_wall = false;
			moveIn3D ('x', params_PLYW [2].ang, placementInfo.width [1], &params_PLYW [2].leftBottomX, &params_PLYW [2].leftBottomY, &params_PLYW [2].leftBottomZ);

			params_TIMB [2].leftBottomX = params_TIMB [1].leftBottomX;
			params_TIMB [2].leftBottomY = params_TIMB [1].leftBottomY;
			params_TIMB [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [2].ang = cell.ang;
			params_TIMB [2].w_w = 0.050;
			params_TIMB [2].w_h = remainWidth;
			params_TIMB [2].w_leng = width;	//placementInfo.width [2];
			params_TIMB [2].w_ang = 0.0;
			moveIn3D ('x', params_TIMB [2].ang, placementInfo.width [1], &params_TIMB [2].leftBottomX, &params_TIMB [2].leftBottomY, &params_TIMB [2].leftBottomZ);
		}

		// 4열
		if (placementInfo.nHorEuroform >= 4) {
			// 1번째 행: 유로폼
			params_UFOM1 [3].leftBottomX = params_UFOM1 [2].leftBottomX;
			params_UFOM1 [3].leftBottomY = params_UFOM1 [2].leftBottomY;
			params_UFOM1 [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [3].ang = cell.ang;
			params_UFOM1 [3].u_ins_wall = false;
			params_UFOM1 [3].width = upperCell.formWidth1;
			params_UFOM1 [3].height = placementInfo.width [3];
			moveIn3D ('x', params_UFOM1 [3].ang, placementInfo.width [3], &params_UFOM1 [3].leftBottomX, &params_UFOM1 [3].leftBottomY, &params_UFOM1 [3].leftBottomZ);

			// 2번째 행: 유로폼
			params_UFOM2 [3].leftBottomX = params_UFOM2 [2].leftBottomX;
			params_UFOM2 [3].leftBottomY = params_UFOM2 [2].leftBottomY;
			params_UFOM2 [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [3].ang = cell.ang;
			params_UFOM2 [3].u_ins_wall = false;
			params_UFOM2 [3].width = upperCell.formWidth2;
			params_UFOM2 [3].height = placementInfo.width [3];
			moveIn3D ('x', params_UFOM2 [3].ang, placementInfo.width [3], &params_UFOM2 [3].leftBottomX, &params_UFOM2 [3].leftBottomY, &params_UFOM2 [3].leftBottomZ);

			// 3번째 행: 합판 또는 목재
			params_PLYW [3].leftBottomX = params_PLYW [2].leftBottomX;
			params_PLYW [3].leftBottomY = params_PLYW [2].leftBottomY;
			params_PLYW [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [3].ang = cell.ang;
			params_PLYW [3].p_wid = remainWidth;
			params_PLYW [3].p_leng = width;	//placementInfo.width [3];
			params_PLYW [3].w_dir_wall = false;
			moveIn3D ('x', params_PLYW [3].ang, placementInfo.width [2], &params_PLYW [3].leftBottomX, &params_PLYW [3].leftBottomY, &params_PLYW [3].leftBottomZ);

			params_TIMB [3].leftBottomX = params_TIMB [2].leftBottomX;
			params_TIMB [3].leftBottomY = params_TIMB [2].leftBottomY;
			params_TIMB [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [3].ang = cell.ang;
			params_TIMB [3].w_w = 0.050;
			params_TIMB [3].w_h = remainWidth;
			params_TIMB [3].w_leng = width;	//placementInfo.width [3];
			params_TIMB [3].w_ang = 0.0;
			moveIn3D ('x', params_TIMB [3].ang, placementInfo.width [2], &params_TIMB [3].leftBottomX, &params_TIMB [3].leftBottomY, &params_TIMB [3].leftBottomZ);
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
				if (xx == 0)
					elemList.Push (placePLYW (params_PLYW [xx]));

			// 목재의 경우
			} else if (remainWidth + EPS > 0) {
				if (xx == 0)
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
				params_UFOM1 [0].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, -placementInfo.width [0], &params_UFOM1 [0].leftBottomX, &params_UFOM1 [0].leftBottomY, &params_UFOM1 [0].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM1 [0].leftBottomX, &params_UFOM1 [0].leftBottomY, &params_UFOM1 [0].leftBottomZ);

				// 2번째 행: 유로폼
				params_UFOM2 [0].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, -placementInfo.width [0], &params_UFOM2 [0].leftBottomX, &params_UFOM2 [0].leftBottomY, &params_UFOM2 [0].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM2 [0].leftBottomX, &params_UFOM2 [0].leftBottomY, &params_UFOM2 [0].leftBottomZ);

				// 3번째 행: 합판 또는 목재
				params_PLYW [0].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [0], &params_PLYW [0].leftBottomX, &params_PLYW [0].leftBottomY, &params_PLYW [0].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_PLYW [0].leftBottomX, &params_PLYW [0].leftBottomY, &params_PLYW [0].leftBottomZ);

				params_TIMB [0].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [0], &params_TIMB [0].leftBottomX, &params_TIMB [0].leftBottomY, &params_TIMB [0].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_TIMB [0].leftBottomX, &params_TIMB [0].leftBottomY, &params_TIMB [0].leftBottomZ);
			}

			// 2열
			if (placementInfo.nHorEuroform >= 2) {
				// 1번째 행: 유로폼
				params_UFOM1 [1].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, -placementInfo.width [1], &params_UFOM1 [1].leftBottomX, &params_UFOM1 [1].leftBottomY, &params_UFOM1 [1].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM1 [1].leftBottomX, &params_UFOM1 [1].leftBottomY, &params_UFOM1 [1].leftBottomZ);

				// 2번째 행: 유로폼
				params_UFOM2 [1].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, -placementInfo.width [1], &params_UFOM2 [1].leftBottomX, &params_UFOM2 [1].leftBottomY, &params_UFOM2 [1].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM2 [1].leftBottomX, &params_UFOM2 [1].leftBottomY, &params_UFOM2 [1].leftBottomZ);

				// 3번째 행: 합판 또는 목재
				params_PLYW [1].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [1], &params_PLYW [1].leftBottomX, &params_PLYW [1].leftBottomY, &params_PLYW [1].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_PLYW [1].leftBottomX, &params_PLYW [1].leftBottomY, &params_PLYW [1].leftBottomZ);

				params_TIMB [1].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [1], &params_TIMB [1].leftBottomX, &params_TIMB [1].leftBottomY, &params_TIMB [1].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_TIMB [1].leftBottomX, &params_TIMB [1].leftBottomY, &params_TIMB [1].leftBottomZ);
			}

			// 3열
			if (placementInfo.nHorEuroform >= 3) {
				// 1번째 행: 유로폼
				params_UFOM1 [2].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, -placementInfo.width [2], &params_UFOM1 [2].leftBottomX, &params_UFOM1 [2].leftBottomY, &params_UFOM1 [2].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM1 [2].leftBottomX, &params_UFOM1 [2].leftBottomY, &params_UFOM1 [2].leftBottomZ);

				// 2번째 행: 유로폼
				params_UFOM2 [2].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, -placementInfo.width [2], &params_UFOM2 [2].leftBottomX, &params_UFOM2 [2].leftBottomY, &params_UFOM2 [2].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM2 [2].leftBottomX, &params_UFOM2 [2].leftBottomY, &params_UFOM2 [2].leftBottomZ);

				// 3번째 행: 합판 또는 목재
				params_PLYW [2].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [2], &params_PLYW [2].leftBottomX, &params_PLYW [2].leftBottomY, &params_PLYW [2].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_PLYW [2].leftBottomX, &params_PLYW [2].leftBottomY, &params_PLYW [2].leftBottomZ);

				params_TIMB [2].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [2], &params_TIMB [2].leftBottomX, &params_TIMB [2].leftBottomY, &params_TIMB [2].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_TIMB [2].leftBottomX, &params_TIMB [2].leftBottomY, &params_TIMB [2].leftBottomZ);
			}

			// 4열
			if (placementInfo.nHorEuroform >= 4) {
				// 1번째 행: 유로폼
				params_UFOM1 [3].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, -placementInfo.width [3], &params_UFOM1 [3].leftBottomX, &params_UFOM1 [3].leftBottomY, &params_UFOM1 [3].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM1 [3].leftBottomX, &params_UFOM1 [3].leftBottomY, &params_UFOM1 [3].leftBottomZ);

				// 2번째 행: 유로폼
				params_UFOM2 [3].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, -placementInfo.width [3], &params_UFOM2 [3].leftBottomX, &params_UFOM2 [3].leftBottomY, &params_UFOM2 [3].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM2 [3].leftBottomX, &params_UFOM2 [3].leftBottomY, &params_UFOM2 [3].leftBottomZ);

				// 3번째 행: 합판 또는 목재
				params_PLYW [3].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [3], &params_PLYW [3].leftBottomX, &params_PLYW [3].leftBottomY, &params_PLYW [3].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_PLYW [3].leftBottomX, &params_PLYW [3].leftBottomY, &params_PLYW [3].leftBottomZ);

				params_TIMB [3].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [3], &params_TIMB [3].leftBottomX, &params_TIMB [3].leftBottomY, &params_TIMB [3].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_TIMB [3].leftBottomX, &params_TIMB [3].leftBottomY, &params_TIMB [3].leftBottomZ);
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
					if (xx == (placementInfo.nHorEuroform - 1))
						elemList.Push (placePLYW (params_PLYW [xx]));

				// 목재의 경우
				} else if (remainWidth + EPS > 0) {
					if (xx == (placementInfo.nHorEuroform - 1))
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

// 테이블폼 배치하기 - 가로 방향 (타입1)
GSErrCode	WallTableformPlacingZone::placeTableformOnWall_Horizontal_Type1 (CellForWallTableform cell)
{
	GSErrCode	err = NoError;
	placementInfoForWallTableform	placementInfo;

	short		xx, yy;
	double		width, height;
	//double		remainder;				// fmod 함수에 쓸 변수
	double		elev_headpiece;
	double		verticalGap = 0.050;	// 수직재 양쪽 이격거리

	Euroform		params_UFOM;
	SquarePipe		params_SPIP;
	PinBoltSet		params_PINB;
	//WallTie			params_TIE;
	//CrossClamp		params_CLAM;
	HeadpieceOfPushPullProps	params_PUSH;
	MetalFittings	params_JOIN;

	placementInfo.nHorEuroform = 0;
	placementInfo.nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		placementInfo.width [xx] = 0.0;
		placementInfo.height [xx] = 0.0;
	}

	if (abs (cell.verLen - 2.300) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 2.250) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 2.200) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.400;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 2.150) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.500;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 2.100) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.300;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 2.050) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.400;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 2.000) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.200;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.950) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.300;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.900) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.500;	placementInfo.height [2] = 0.200;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.850) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.200;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.800) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.750) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.200;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.500;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.700) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.500;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.650) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.600) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.400;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.550) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.500) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.450) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.500;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.400) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.500;	placementInfo.height [1] = 0.400;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.350) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.300) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.200;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.250) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.200;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.200) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.150) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.400;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.100) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.400;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.050) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.300;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.000) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.400;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.950) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.500;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.900) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.850) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.800) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.400;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.750) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.700) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.650) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.200;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.600) < EPS) {
		placementInfo.nVerEuroform = 1;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.500) < EPS) {
		placementInfo.nVerEuroform = 1;
		placementInfo.height [0] = 0.500;	placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.450) < EPS) {
		placementInfo.nVerEuroform = 1;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.400) < EPS) {
		placementInfo.nVerEuroform = 1;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else {
		placementInfo.nVerEuroform = 0;
		placementInfo.height [0] = 0.0;		placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
	}

	if (abs (cell.horLen - 6.000) < EPS) {
		placementInfo.nHorEuroform = 5;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 1.200;
		placementInfo.width [4] = 1.200;
	} else if (abs (cell.horLen - 5.700) < EPS) {
		placementInfo.nHorEuroform = 5;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 1.200;
		placementInfo.width [4] = 0.900;
	} else if (abs (cell.horLen - 5.400) < EPS) {
		placementInfo.nHorEuroform = 5;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 0.900;
		placementInfo.width [4] = 0.900;
	} else if (abs (cell.horLen - 5.100) < EPS) {
		placementInfo.nHorEuroform = 5;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 0.900;
		placementInfo.width [4] = 0.600;
	} else if (abs (cell.horLen - 4.800) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 1.200;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 4.500) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 0.900;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 4.200) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.900;
		placementInfo.width [3] = 0.900;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 3.900) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.900;
		placementInfo.width [3] = 0.600;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 3.600) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 3.300) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.900;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 3.000) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.600;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 2.700) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 0.900;
		placementInfo.width [2] = 0.600;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 2.400) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 2.100) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 0.900;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 1.800) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.900;
		placementInfo.width [1] = 0.900;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 1.500) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.900;
		placementInfo.width [1] = 0.600;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else {
		placementInfo.nHorEuroform = 0;
		placementInfo.width [0] = 0.0;
		placementInfo.width [1] = 0.0;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
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
	params_UFOM.u_ins_wall = false;

	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		height = 0.0;
		for (yy = placementInfo.nVerEuroform-1 ; yy >= 0 ; --yy) {
			params_UFOM.width	= placementInfo.height [yy];
			params_UFOM.height	= placementInfo.width [xx];
			height += placementInfo.height [yy];
			elemList.Push (placeUFOM (params_UFOM));
			moveIn3D ('z', params_UFOM.ang, placementInfo.height [yy], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		}
		moveIn3D ('x', params_UFOM.ang, placementInfo.width [xx], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		moveIn3D ('z', params_UFOM.ang, -height, &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
	}

	// 비계 파이프 (수직) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.verLen - (verticalGap * 2);
	params_SPIP.pipeAng = DegreeToRad (90.0);

	moveIn3D ('z', params_SPIP.ang, verticalGap, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.025), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('x', params_SPIP.ang, 0.150 - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

	for (xx = 0 ; xx <= placementInfo.nHorEuroform ; ++xx) {
		if (xx == 0) {
			// 1행
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, -0.031 - 0.150 + placementInfo.width [xx] - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		} else if (xx == placementInfo.nHorEuroform) {
			// 마지막 행
			moveIn3D ('x', params_SPIP.ang, -0.150, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
		} else {
			// 나머지 행
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, -0.031 + placementInfo.width [xx] - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		}
	}

	// 비계 파이프 (수평) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.horLen - 0.100;
	params_SPIP.pipeAng = DegreeToRad (0);

	height = 0.0;
	for (xx = placementInfo.nVerEuroform-1 ; xx >= 0 ; --xx) {
		height += placementInfo.height [xx];
	}
	moveIn3D ('z', params_SPIP.ang, height - placementInfo.height [0] + 0.150 + 0.035, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.075), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('x', params_SPIP.ang, 0.050, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

	// 1열
	elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, -0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, 0.070 + (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	// 2열
	elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, -0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	elemList.Push (placeSPIP (params_SPIP));

	// 핀볼트 배치 (수직 - 최하단, 최상단)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = FALSE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

	// 최하단 행
	moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
		height += placementInfo.height [xx];
	}
	moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform-1 ; ++xx) {
		height += placementInfo.height [xx];
		moveIn3D ('z', params_PINB.ang, -placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		elemList.Push (placePINB (params_PINB));
	}
	// 최상단 행
	moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('x', params_PINB.ang, cell.horLen - 0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	for (xx = 0 ; xx < placementInfo.nVerEuroform-1 ; ++xx) {
		moveIn3D ('z', params_PINB.ang, -placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		elemList.Push (placePINB (params_PINB));
	}

	// 핀볼트 배치 (수직 - 나머지)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = TRUE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

	// 2 ~ [n-1]행
	if (placementInfo.nVerEuroform >= 3) {
		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_PINB.ang, height - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('x', params_PINB.ang, placementInfo.width [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
			height = 0.0;
			for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
				// 1열
				if (yy == 0) {
					elemList.Push (placePINB (params_PINB));
					moveIn3D ('z', params_PINB.ang, -(placementInfo.height [0] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					height += placementInfo.height [0] - 0.150;
				// 마지막 열
				} else if (yy == placementInfo.nVerEuroform - 1) {
					height += placementInfo.height [placementInfo.nVerEuroform-1] - 0.150;
					moveIn3D ('z', params_PINB.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					elemList.Push (placePINB (params_PINB));
				// 나머지 열
				} else {
					height += placementInfo.height [yy];
					if (abs (placementInfo.height [yy] - 0.600) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.500) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.450) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.400) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.300) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.200) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.050, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					}
				}
			}
			moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		}
	}

	// 핀볼트 배치 (수평)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = TRUE;
	params_PINB.boltLen = 0.150;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
		height += placementInfo.height [xx];
	}
	moveIn3D ('z', params_PINB.ang, height - (placementInfo.height [0] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('y', params_PINB.ang, -(0.2135), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('x', params_PINB.ang, placementInfo.width [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

	// 1열
	width = 0.0;
	for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		width += placementInfo.width [xx];
	}
	// 2열
	moveIn3D ('z', params_PINB.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('x', params_PINB.ang, -width, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		width += placementInfo.width [xx];
	}

	// 벽체 타이
	//if (placingZone.bDoubleSide) {
	//	params_TIE.leftBottomX = cell.leftBottomX;
	//	params_TIE.leftBottomY = cell.leftBottomY;
	//	params_TIE.leftBottomZ = cell.leftBottomZ;
	//	params_TIE.ang = cell.ang;
	//	remainder = fmod ((infoWall.wallThk + 0.327), 0.100);
	//	params_TIE.boltLen = (infoWall.wallThk + 0.327 + (0.100 - remainder));
	//	params_TIE.pipeBeg = 0.0365 + 0.1635;
	//	params_TIE.pipeEnd = 0.0365 + 0.1635 + infoWall.wallThk;
	//	params_TIE.clampBeg = 0.0365;
	//	params_TIE.clampEnd = 0.0365 + infoWall.wallThk + 0.327;

	//	height = 0.0;
	//	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
	//		height += placementInfo.height [xx];
	//	}
	//	moveIn3D ('z', params_TIE.ang, height - (placementInfo.height [0] - 0.150), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//	moveIn3D ('y', params_TIE.ang, -(0.1635 + 0.0365), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//	moveIn3D ('x', params_TIE.ang, 0.350, &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);

	//	for (xx = 0 ; xx < 2 ; ++xx) {
	//		for (yy = 0 ; yy < placementInfo.nHorEuroform ; ++yy) {
	//			// 최하위 행
	//			if (yy == 0) {
	//				elemList.Push (placeTIE (params_TIE));
	//				moveIn3D ('x', params_TIE.ang, placementInfo.width [yy], &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//	
	//			// 최상위 행
	//			} else if (yy == placementInfo.nHorEuroform - 1) {
	//				moveIn3D ('x', params_TIE.ang, placementInfo.width [yy] - 0.350*2, &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//				elemList.Push (placeTIE (params_TIE));
	//				moveIn3D ('z', params_TIE.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//				moveIn3D ('x', params_TIE.ang, (0.350 - cell.horLen + 0.350), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//			
	//			// 2 ~ [n-1]행
	//			} else {
	//				elemList.Push (placeTIE (params_TIE));
	//				moveIn3D ('x', params_TIE.ang, placementInfo.width [yy], &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//			}
	//		}
	//	}
	//}

	// 헤드 피스
	params_PUSH.leftBottomX = cell.leftBottomX;
	params_PUSH.leftBottomY = cell.leftBottomY;
	params_PUSH.leftBottomZ = cell.leftBottomZ;
	params_PUSH.ang = cell.ang;

	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
		height += placementInfo.height [xx];
	}
	width = 0.0;
	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		width += placementInfo.width [xx];
	}
	moveIn3D ('z', params_PUSH.ang, height - (placementInfo.height [0] - 0.150 - 0.100) - 0.200, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	moveIn3D ('y', params_PUSH.ang, -0.1725, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	moveIn3D ('x', params_PUSH.ang, 0.600, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);

	// 처음 행
	elemList.Push (placePUSH_hor (params_PUSH));
	moveIn3D ('z', params_PUSH.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	elemList.Push (placePUSH_hor (params_PUSH));
	moveIn3D ('z', params_PUSH.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) + cell.verLen + (-placementInfo.height [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	elev_headpiece = width - 0.800;
	moveIn3D ('x', params_PUSH.ang, -0.600 + elev_headpiece, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	// 마지막 행
	elemList.Push (placePUSH_hor (params_PUSH));
	moveIn3D ('z', params_PUSH.ang, (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	elemList.Push (placePUSH_hor (params_PUSH));

	// 결합철물
	params_JOIN.leftBottomX = cell.leftBottomX;
	params_JOIN.leftBottomY = cell.leftBottomY;
	params_JOIN.leftBottomZ = cell.leftBottomZ;
	params_JOIN.ang = cell.ang;

	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
		height += placementInfo.height [xx];
	}
	moveIn3D ('z', params_JOIN.ang, height - (placementInfo.height [0] - 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('y', params_JOIN.ang, -0.0455, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('x', params_JOIN.ang, 0.150, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

	// 처음 행
	elemList.Push (placeJOIN (params_JOIN));
	moveIn3D ('z', params_JOIN.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	elemList.Push (placeJOIN (params_JOIN));
	moveIn3D ('z', params_JOIN.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) + cell.verLen + (-placementInfo.height [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('x', params_JOIN.ang, cell.horLen - 0.300, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

	// 마지막 행
	elemList.Push (placeJOIN (params_JOIN));
	moveIn3D ('z', params_JOIN.ang, (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
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
		moveIn3D ('x', cell.ang, cell.horLen, &cell.leftBottomX, &cell.leftBottomY, &cell.leftBottomY);
		moveIn3D ('y', cell.ang, infoWall.wallThk, &cell.leftBottomX, &cell.leftBottomY, &cell.leftBottomY);
		cell.ang += DegreeToRad (180.0);

		// 유로폼 설치 (반대편에서 변경됨)
		params_UFOM.leftBottomX = cell.leftBottomX;
		params_UFOM.leftBottomY = cell.leftBottomY;
		params_UFOM.leftBottomZ = cell.leftBottomZ;
		params_UFOM.ang = cell.ang;
		params_UFOM.u_ins_wall = false;

		for (xx = placementInfo.nHorEuroform-1 ; xx >= 0 ; --xx) {
			height = 0.0;
			for (yy = placementInfo.nVerEuroform-1 ; yy >= 0 ; --yy) {
				params_UFOM.width	= placementInfo.height [yy];
				params_UFOM.height	= placementInfo.width [xx];
				height += placementInfo.height [yy];
				elemList.Push (placeUFOM (params_UFOM));
				moveIn3D ('z', params_UFOM.ang, placementInfo.height [yy], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
			}
			moveIn3D ('x', params_UFOM.ang, placementInfo.width [xx], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
			moveIn3D ('z', params_UFOM.ang, -height, &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		}

		// 비계 파이프 (수직) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.verLen - (verticalGap * 2);
		params_SPIP.pipeAng = DegreeToRad (90.0);

		moveIn3D ('z', params_SPIP.ang, verticalGap, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.025), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('x', params_SPIP.ang, cell.horLen - (0.150 - 0.031), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

		for (xx = 0 ; xx <= placementInfo.nHorEuroform ; ++xx) {
			if (xx == 0) {
				// 1행
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, -0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, 0.031 + 0.150 - placementInfo.width [xx] + 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			} else if (xx == placementInfo.nHorEuroform) {
				// 마지막 행
				moveIn3D ('x', params_SPIP.ang, 0.150, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, -0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
			} else {
				// 나머지 행
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, -0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, 0.031 - placementInfo.width [xx] + 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			}
		}

		// 비계 파이프 (수평) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.horLen - 0.100;
		params_SPIP.pipeAng = DegreeToRad (0);

		height = 0.0;
		for (xx = placementInfo.nVerEuroform-1 ; xx >= 0 ; --xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_SPIP.ang, height - placementInfo.height [0] + 0.150 + 0.035, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.075), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('x', params_SPIP.ang, 0.050, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

		// 1열
		elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, -0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, 0.070 + (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		// 2열
		elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, -0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		elemList.Push (placeSPIP (params_SPIP));

		// 핀볼트 배치 (수직 - 최하단, 최상단)
		params_PINB.leftBottomX = cell.leftBottomX;
		params_PINB.leftBottomY = cell.leftBottomY;
		params_PINB.leftBottomZ = cell.leftBottomZ;
		params_PINB.ang = cell.ang;
		params_PINB.bPinBoltRot90 = FALSE;
		params_PINB.boltLen = 0.100;
		params_PINB.angX = DegreeToRad (270.0);
		params_PINB.angY = DegreeToRad (0.0);

		moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		// 최하단 행
		moveIn3D ('x', params_PINB.ang, cell.horLen - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform-1 ; ++xx) {
			height += placementInfo.height [xx];
			moveIn3D ('z', params_PINB.ang, -placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

			elemList.Push (placePINB (params_PINB));
		}
		// 최상단 행
		moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('x', params_PINB.ang, -cell.horLen + 0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		for (xx = 0 ; xx < placementInfo.nVerEuroform-1 ; ++xx) {
			moveIn3D ('z', params_PINB.ang, -placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

			elemList.Push (placePINB (params_PINB));
		}

		// 핀볼트 배치 (수직 - 나머지)
		params_PINB.leftBottomX = cell.leftBottomX;
		params_PINB.leftBottomY = cell.leftBottomY;
		params_PINB.leftBottomZ = cell.leftBottomZ;
		params_PINB.ang = cell.ang;
		params_PINB.bPinBoltRot90 = TRUE;
		params_PINB.boltLen = 0.100;
		params_PINB.angX = DegreeToRad (270.0);
		params_PINB.angY = DegreeToRad (0.0);

		moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		// 2 ~ [n-1]행
		if (placementInfo.nVerEuroform >= 3) {
			height = 0.0;
			for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
				height += placementInfo.height [xx];
			}
			moveIn3D ('z', params_PINB.ang, height - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			moveIn3D ('x', params_PINB.ang, cell.horLen - placementInfo.width [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
				height = 0.0;
				for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
					// 1열
					if (yy == 0) {
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -(placementInfo.height [0] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						height += placementInfo.height [0] - 0.150;
					// 마지막 열
					} else if (yy == placementInfo.nVerEuroform - 1) {
						height += placementInfo.height [placementInfo.nVerEuroform-1] - 0.150;
						moveIn3D ('z', params_PINB.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
					// 나머지 열
					} else {
						height += placementInfo.height [yy];
						if (abs (placementInfo.height [yy] - 0.600) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.500) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.450) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.400) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.300) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.200) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.050, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						}
					}
				}
				moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
				moveIn3D ('x', params_PINB.ang, -placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			}
		}

		// 핀볼트 배치 (수평)
		params_PINB.leftBottomX = cell.leftBottomX;
		params_PINB.leftBottomY = cell.leftBottomY;
		params_PINB.leftBottomZ = cell.leftBottomZ;
		params_PINB.ang = cell.ang;
		params_PINB.bPinBoltRot90 = TRUE;
		params_PINB.boltLen = 0.150;
		params_PINB.angX = DegreeToRad (270.0);
		params_PINB.angY = DegreeToRad (0.0);

		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_PINB.ang, height - (placementInfo.height [0] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('y', params_PINB.ang, -(0.2135), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('x', params_PINB.ang, cell.horLen - placementInfo.width [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		// 1열
		width = 0.0;
		for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
			elemList.Push (placePINB (params_PINB));
			moveIn3D ('x', params_PINB.ang, -placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			width += placementInfo.width [xx];
		}
		// 2열
		moveIn3D ('z', params_PINB.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('x', params_PINB.ang, width, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
			elemList.Push (placePINB (params_PINB));
			moveIn3D ('x', params_PINB.ang, -placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			width += placementInfo.width [xx];
		}

		// 벽체 타이 (현재면에서 했으므로 생략)

		// 헤드 피스
		params_PUSH.leftBottomX = cell.leftBottomX;
		params_PUSH.leftBottomY = cell.leftBottomY;
		params_PUSH.leftBottomZ = cell.leftBottomZ;
		params_PUSH.ang = cell.ang;

		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		width = 0.0;
		for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
			width += placementInfo.width [xx];
		}
		moveIn3D ('z', params_PUSH.ang, height - (placementInfo.height [0] - 0.150 - 0.100) - 0.200, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		moveIn3D ('y', params_PUSH.ang, -0.1725, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		moveIn3D ('x', params_PUSH.ang, cell.horLen - 0.800, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);

		// 처음 행
		elemList.Push (placePUSH_hor (params_PUSH));
		moveIn3D ('z', params_PUSH.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		elemList.Push (placePUSH_hor (params_PUSH));
		moveIn3D ('z', params_PUSH.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) + cell.verLen + (-placementInfo.height [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		elev_headpiece = width - 0.800;
		moveIn3D ('x', params_PUSH.ang, 0.600 - elev_headpiece, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		// 마지막 행
		elemList.Push (placePUSH_hor (params_PUSH));
		moveIn3D ('z', params_PUSH.ang, (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		elemList.Push (placePUSH_hor (params_PUSH));

		// 결합철물
		params_JOIN.leftBottomX = cell.leftBottomX;
		params_JOIN.leftBottomY = cell.leftBottomY;
		params_JOIN.leftBottomZ = cell.leftBottomZ;
		params_JOIN.ang = cell.ang;

		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_JOIN.ang, height - (placementInfo.height [0] - 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('y', params_JOIN.ang, -0.0455, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('x', params_JOIN.ang, cell.horLen - 0.150, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

		// 처음 행
		elemList.Push (placeJOIN (params_JOIN));
		moveIn3D ('z', params_JOIN.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		elemList.Push (placeJOIN (params_JOIN));
		moveIn3D ('z', params_JOIN.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) + cell.verLen + (-placementInfo.height [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('x', params_JOIN.ang, -cell.horLen + 0.300, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

		// 마지막 행
		elemList.Push (placeJOIN (params_JOIN));
		moveIn3D ('z', params_JOIN.ang, (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
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

// 테이블폼 배치하기 - 가로 방향 (타입2) ...
GSErrCode	WallTableformPlacingZone::placeTableformOnWall_Horizontal_Type2 (CellForWallTableform cell)
{
	GSErrCode	err = NoError;
	placementInfoForWallTableform	placementInfo;

	short		xx, yy;
	double		width, height;
	//double		remainder;				// fmod 함수에 쓸 변수
	double		elev_headpiece;
	double		verticalGap = 0.050;	// 수직재 양쪽 이격거리

	Euroform		params_UFOM;
	SquarePipe		params_SPIP;
	PinBoltSet		params_PINB;
	//WallTie			params_TIE;
	//CrossClamp		params_CLAM;
	HeadpieceOfPushPullProps	params_PUSH;
	MetalFittings	params_JOIN;

	placementInfo.nHorEuroform = 0;
	placementInfo.nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		placementInfo.width [xx] = 0.0;
		placementInfo.height [xx] = 0.0;
	}

	if (abs (cell.verLen - 2.300) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 2.250) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 2.200) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.400;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 2.150) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.500;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 2.100) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.300;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 2.050) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.400;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 2.000) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.200;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.950) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.300;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.900) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.500;	placementInfo.height [2] = 0.200;	placementInfo.height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.850) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.200;	placementInfo.height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.800) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.750) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.200;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.500;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.700) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.500;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.650) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.600) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.400;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.550) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.500) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.600;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.450) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.500;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.400) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.500;	placementInfo.height [1] = 0.400;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.350) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.300) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.200;	placementInfo.height [2] = 0.500;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.250) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.200;	placementInfo.height [2] = 0.450;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.200) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.600;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.150) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.400;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.100) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.400;	placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 1.050) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.300;	placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 1.000) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.400;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.950) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.500;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.900) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.850) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.450;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.800) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.400;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.750) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.700) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.300;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.650) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.200;	placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.600) < EPS) {
		placementInfo.nVerEuroform = 1;
		placementInfo.height [0] = 0.600;	placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.500) < EPS) {
		placementInfo.nVerEuroform = 1;
		placementInfo.height [0] = 0.500;	placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (cell.verLen - 0.450) < EPS) {
		placementInfo.nVerEuroform = 1;
		placementInfo.height [0] = 0.450;	placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (cell.verLen - 0.400) < EPS) {
		placementInfo.nVerEuroform = 1;
		placementInfo.height [0] = 0.400;	placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
		verticalGap = 0.050;
	} else {
		placementInfo.nVerEuroform = 0;
		placementInfo.height [0] = 0.0;		placementInfo.height [1] = 0.0;		placementInfo.height [2] = 0.0;		placementInfo.height [3] = 0.0;
	}

	if (abs (cell.horLen - 6.000) < EPS) {
		placementInfo.nHorEuroform = 5;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 1.200;
		placementInfo.width [4] = 1.200;
	} else if (abs (cell.horLen - 5.700) < EPS) {
		placementInfo.nHorEuroform = 5;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 1.200;
		placementInfo.width [4] = 0.900;
	} else if (abs (cell.horLen - 5.400) < EPS) {
		placementInfo.nHorEuroform = 5;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 0.900;
		placementInfo.width [4] = 0.900;
	} else if (abs (cell.horLen - 5.100) < EPS) {
		placementInfo.nHorEuroform = 5;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 0.900;
		placementInfo.width [4] = 0.600;
	} else if (abs (cell.horLen - 4.800) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 1.200;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 4.500) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 0.900;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 4.200) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.900;
		placementInfo.width [3] = 0.900;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 3.900) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.900;
		placementInfo.width [3] = 0.600;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 3.600) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 1.200;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 3.300) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.900;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 3.000) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.600;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 2.700) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 0.900;
		placementInfo.width [2] = 0.600;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 2.400) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 1.200;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 2.100) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 1.200;
		placementInfo.width [1] = 0.900;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 1.800) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.900;
		placementInfo.width [1] = 0.900;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else if (abs (cell.horLen - 1.500) < EPS) {
		placementInfo.nHorEuroform = 2;
		placementInfo.width [0] = 0.900;
		placementInfo.width [1] = 0.600;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
	} else {
		placementInfo.nHorEuroform = 0;
		placementInfo.width [0] = 0.0;
		placementInfo.width [1] = 0.0;
		placementInfo.width [2] = 0.0;
		placementInfo.width [3] = 0.0;
		placementInfo.width [4] = 0.0;
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
	params_UFOM.u_ins_wall = false;

	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		height = 0.0;
		for (yy = placementInfo.nVerEuroform-1 ; yy >= 0 ; --yy) {
			params_UFOM.width	= placementInfo.height [yy];
			params_UFOM.height	= placementInfo.width [xx];
			height += placementInfo.height [yy];
			elemList.Push (placeUFOM (params_UFOM));
			moveIn3D ('z', params_UFOM.ang, placementInfo.height [yy], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		}
		moveIn3D ('x', params_UFOM.ang, placementInfo.width [xx], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		moveIn3D ('z', params_UFOM.ang, -height, &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
	}

	// 비계 파이프 (수직) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.verLen - (verticalGap * 2);
	params_SPIP.pipeAng = DegreeToRad (90.0);

	moveIn3D ('z', params_SPIP.ang, verticalGap, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.025), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('x', params_SPIP.ang, 0.150 - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

	for (xx = 0 ; xx <= placementInfo.nHorEuroform ; ++xx) {
		if (xx == 0) {
			// 1행
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, -0.031 - 0.150 + placementInfo.width [xx] - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		} else if (xx == placementInfo.nHorEuroform) {
			// 마지막 행
			moveIn3D ('x', params_SPIP.ang, -0.150, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
		} else {
			// 나머지 행
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, 0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			elemList.Push (placeSPIP (params_SPIP));
			moveIn3D ('x', params_SPIP.ang, -0.031 + placementInfo.width [xx] - 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		}
	}

	// 비계 파이프 (수평) 배치
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.horLen - 0.100;
	params_SPIP.pipeAng = DegreeToRad (0);

	height = 0.0;
	for (xx = placementInfo.nVerEuroform-1 ; xx >= 0 ; --xx) {
		height += placementInfo.height [xx];
	}
	moveIn3D ('z', params_SPIP.ang, height - placementInfo.height [0] + 0.150 + 0.035, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.075), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	moveIn3D ('x', params_SPIP.ang, 0.050, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

	// 1열
	elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, -0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, 0.070 + (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	// 2열
	elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, -0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
	elemList.Push (placeSPIP (params_SPIP));

	// 핀볼트 배치 (수직 - 최하단, 최상단)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = FALSE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

	// 최하단 행
	moveIn3D ('x', params_PINB.ang, 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
		height += placementInfo.height [xx];
	}
	moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform-1 ; ++xx) {
		height += placementInfo.height [xx];
		moveIn3D ('z', params_PINB.ang, -placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		elemList.Push (placePINB (params_PINB));
	}
	// 최상단 행
	moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('x', params_PINB.ang, cell.horLen - 0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	for (xx = 0 ; xx < placementInfo.nVerEuroform-1 ; ++xx) {
		moveIn3D ('z', params_PINB.ang, -placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		elemList.Push (placePINB (params_PINB));
	}

	// 핀볼트 배치 (수직 - 나머지)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = TRUE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

	// 2 ~ [n-1]행
	if (placementInfo.nVerEuroform >= 3) {
		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_PINB.ang, height - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('x', params_PINB.ang, placementInfo.width [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
			height = 0.0;
			for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
				// 1열
				if (yy == 0) {
					elemList.Push (placePINB (params_PINB));
					moveIn3D ('z', params_PINB.ang, -(placementInfo.height [0] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					height += placementInfo.height [0] - 0.150;
				// 마지막 열
				} else if (yy == placementInfo.nVerEuroform - 1) {
					height += placementInfo.height [placementInfo.nVerEuroform-1] - 0.150;
					moveIn3D ('z', params_PINB.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					elemList.Push (placePINB (params_PINB));
				// 나머지 열
				} else {
					height += placementInfo.height [yy];
					if (abs (placementInfo.height [yy] - 0.600) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.500) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.450) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.400) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.300) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					} else if (abs (placementInfo.height [yy] - 0.200) < EPS) {
						moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -0.050, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
					}
				}
			}
			moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		}
	}

	// 핀볼트 배치 (수평)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = TRUE;
	params_PINB.boltLen = 0.150;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
		height += placementInfo.height [xx];
	}
	moveIn3D ('z', params_PINB.ang, height - (placementInfo.height [0] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('y', params_PINB.ang, -(0.2135), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('x', params_PINB.ang, placementInfo.width [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

	// 1열
	width = 0.0;
	for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		width += placementInfo.width [xx];
	}
	// 2열
	moveIn3D ('z', params_PINB.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	moveIn3D ('x', params_PINB.ang, -width, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
	for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		moveIn3D ('x', params_PINB.ang, placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		width += placementInfo.width [xx];
	}

	// 벽체 타이
	//if (placingZone.bDoubleSide) {
	//	params_TIE.leftBottomX = cell.leftBottomX;
	//	params_TIE.leftBottomY = cell.leftBottomY;
	//	params_TIE.leftBottomZ = cell.leftBottomZ;
	//	params_TIE.ang = cell.ang;
	//	remainder = fmod ((infoWall.wallThk + 0.327), 0.100);
	//	params_TIE.boltLen = (infoWall.wallThk + 0.327 + (0.100 - remainder));
	//	params_TIE.pipeBeg = 0.0365 + 0.1635;
	//	params_TIE.pipeEnd = 0.0365 + 0.1635 + infoWall.wallThk;
	//	params_TIE.clampBeg = 0.0365;
	//	params_TIE.clampEnd = 0.0365 + infoWall.wallThk + 0.327;

	//	height = 0.0;
	//	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
	//		height += placementInfo.height [xx];
	//	}
	//	moveIn3D ('z', params_TIE.ang, height - (placementInfo.height [0] - 0.150), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//	moveIn3D ('y', params_TIE.ang, -(0.1635 + 0.0365), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//	moveIn3D ('x', params_TIE.ang, 0.350, &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);

	//	for (xx = 0 ; xx < 2 ; ++xx) {
	//		for (yy = 0 ; yy < placementInfo.nHorEuroform ; ++yy) {
	//			// 최하위 행
	//			if (yy == 0) {
	//				elemList.Push (placeTIE (params_TIE));
	//				moveIn3D ('x', params_TIE.ang, placementInfo.width [yy], &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//	
	//			// 최상위 행
	//			} else if (yy == placementInfo.nHorEuroform - 1) {
	//				moveIn3D ('x', params_TIE.ang, placementInfo.width [yy] - 0.350*2, &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//				elemList.Push (placeTIE (params_TIE));
	//				moveIn3D ('z', params_TIE.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//				moveIn3D ('x', params_TIE.ang, (0.350 - cell.horLen + 0.350), &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//			
	//			// 2 ~ [n-1]행
	//			} else {
	//				elemList.Push (placeTIE (params_TIE));
	//				moveIn3D ('x', params_TIE.ang, placementInfo.width [yy], &params_TIE.leftBottomX, &params_TIE.leftBottomY, &params_TIE.leftBottomZ);
	//			}
	//		}
	//	}
	//}

	// 헤드 피스
	params_PUSH.leftBottomX = cell.leftBottomX;
	params_PUSH.leftBottomY = cell.leftBottomY;
	params_PUSH.leftBottomZ = cell.leftBottomZ;
	params_PUSH.ang = cell.ang;

	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
		height += placementInfo.height [xx];
	}
	width = 0.0;
	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		width += placementInfo.width [xx];
	}
	moveIn3D ('z', params_PUSH.ang, height - (placementInfo.height [0] - 0.150 - 0.100) - 0.200, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	moveIn3D ('y', params_PUSH.ang, -0.1725, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	moveIn3D ('x', params_PUSH.ang, 0.600, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);

	// 처음 행
	elemList.Push (placePUSH_hor (params_PUSH));
	moveIn3D ('z', params_PUSH.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	elemList.Push (placePUSH_hor (params_PUSH));
	moveIn3D ('z', params_PUSH.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) + cell.verLen + (-placementInfo.height [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	elev_headpiece = width - 0.800;
	moveIn3D ('x', params_PUSH.ang, -0.600 + elev_headpiece, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	// 마지막 행
	elemList.Push (placePUSH_hor (params_PUSH));
	moveIn3D ('z', params_PUSH.ang, (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
	elemList.Push (placePUSH_hor (params_PUSH));

	// 결합철물
	params_JOIN.leftBottomX = cell.leftBottomX;
	params_JOIN.leftBottomY = cell.leftBottomY;
	params_JOIN.leftBottomZ = cell.leftBottomZ;
	params_JOIN.ang = cell.ang;

	height = 0.0;
	for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
		height += placementInfo.height [xx];
	}
	moveIn3D ('z', params_JOIN.ang, height - (placementInfo.height [0] - 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('y', params_JOIN.ang, -0.0455, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('x', params_JOIN.ang, 0.150, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

	// 처음 행
	elemList.Push (placeJOIN (params_JOIN));
	moveIn3D ('z', params_JOIN.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	elemList.Push (placeJOIN (params_JOIN));
	moveIn3D ('z', params_JOIN.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) + cell.verLen + (-placementInfo.height [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
	moveIn3D ('x', params_JOIN.ang, cell.horLen - 0.300, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

	// 마지막 행
	elemList.Push (placeJOIN (params_JOIN));
	moveIn3D ('z', params_JOIN.ang, (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
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
		moveIn3D ('x', cell.ang, cell.horLen, &cell.leftBottomX, &cell.leftBottomY, &cell.leftBottomY);
		moveIn3D ('y', cell.ang, infoWall.wallThk, &cell.leftBottomX, &cell.leftBottomY, &cell.leftBottomY);
		cell.ang += DegreeToRad (180.0);

		// 유로폼 설치 (반대편에서 변경됨)
		params_UFOM.leftBottomX = cell.leftBottomX;
		params_UFOM.leftBottomY = cell.leftBottomY;
		params_UFOM.leftBottomZ = cell.leftBottomZ;
		params_UFOM.ang = cell.ang;
		params_UFOM.u_ins_wall = false;

		for (xx = placementInfo.nHorEuroform-1 ; xx >= 0 ; --xx) {
			height = 0.0;
			for (yy = placementInfo.nVerEuroform-1 ; yy >= 0 ; --yy) {
				params_UFOM.width	= placementInfo.height [yy];
				params_UFOM.height	= placementInfo.width [xx];
				height += placementInfo.height [yy];
				elemList.Push (placeUFOM (params_UFOM));
				moveIn3D ('z', params_UFOM.ang, placementInfo.height [yy], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
			}
			moveIn3D ('x', params_UFOM.ang, placementInfo.width [xx], &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
			moveIn3D ('z', params_UFOM.ang, -height, &params_UFOM.leftBottomX, &params_UFOM.leftBottomY, &params_UFOM.leftBottomZ);
		}

		// 비계 파이프 (수직) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.verLen - (verticalGap * 2);
		params_SPIP.pipeAng = DegreeToRad (90.0);

		moveIn3D ('z', params_SPIP.ang, verticalGap, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.025), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('x', params_SPIP.ang, cell.horLen - (0.150 - 0.031), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

		for (xx = 0 ; xx <= placementInfo.nHorEuroform ; ++xx) {
			if (xx == 0) {
				// 1행
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, -0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, 0.031 + 0.150 - placementInfo.width [xx] + 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			} else if (xx == placementInfo.nHorEuroform) {
				// 마지막 행
				moveIn3D ('x', params_SPIP.ang, 0.150, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, -0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
			} else {
				// 나머지 행
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, -0.062, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
				elemList.Push (placeSPIP (params_SPIP));
				moveIn3D ('x', params_SPIP.ang, 0.031 - placementInfo.width [xx] + 0.031, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
			}
		}

		// 비계 파이프 (수평) 배치
		params_SPIP.leftBottomX = cell.leftBottomX;
		params_SPIP.leftBottomY = cell.leftBottomY;
		params_SPIP.leftBottomZ = cell.leftBottomZ;
		params_SPIP.ang = cell.ang;
		params_SPIP.length = cell.horLen - 0.100;
		params_SPIP.pipeAng = DegreeToRad (0);

		height = 0.0;
		for (xx = placementInfo.nVerEuroform-1 ; xx >= 0 ; --xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_SPIP.ang, height - placementInfo.height [0] + 0.150 + 0.035, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('y', params_SPIP.ang, -(0.0635 + 0.075), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		moveIn3D ('x', params_SPIP.ang, 0.050, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);

		// 1열
		elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, -0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, 0.070 + (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		// 2열
		elemList.Push (placeSPIP (params_SPIP));	moveIn3D ('z', params_SPIP.ang, -0.070, &params_SPIP.leftBottomX, &params_SPIP.leftBottomY, &params_SPIP.leftBottomZ);
		elemList.Push (placeSPIP (params_SPIP));

		// 핀볼트 배치 (수직 - 최하단, 최상단)
		params_PINB.leftBottomX = cell.leftBottomX;
		params_PINB.leftBottomY = cell.leftBottomY;
		params_PINB.leftBottomZ = cell.leftBottomZ;
		params_PINB.ang = cell.ang;
		params_PINB.bPinBoltRot90 = FALSE;
		params_PINB.boltLen = 0.100;
		params_PINB.angX = DegreeToRad (270.0);
		params_PINB.angY = DegreeToRad (0.0);

		moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		// 최하단 행
		moveIn3D ('x', params_PINB.ang, cell.horLen - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform-1 ; ++xx) {
			height += placementInfo.height [xx];
			moveIn3D ('z', params_PINB.ang, -placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

			elemList.Push (placePINB (params_PINB));
		}
		// 최상단 행
		moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('x', params_PINB.ang, -cell.horLen + 0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		for (xx = 0 ; xx < placementInfo.nVerEuroform-1 ; ++xx) {
			moveIn3D ('z', params_PINB.ang, -placementInfo.height [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

			elemList.Push (placePINB (params_PINB));
		}

		// 핀볼트 배치 (수직 - 나머지)
		params_PINB.leftBottomX = cell.leftBottomX;
		params_PINB.leftBottomY = cell.leftBottomY;
		params_PINB.leftBottomZ = cell.leftBottomZ;
		params_PINB.ang = cell.ang;
		params_PINB.bPinBoltRot90 = TRUE;
		params_PINB.boltLen = 0.100;
		params_PINB.angX = DegreeToRad (270.0);
		params_PINB.angY = DegreeToRad (0.0);

		moveIn3D ('y', params_PINB.ang, -(0.1635), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		// 2 ~ [n-1]행
		if (placementInfo.nVerEuroform >= 3) {
			height = 0.0;
			for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
				height += placementInfo.height [xx];
			}
			moveIn3D ('z', params_PINB.ang, height - 0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			moveIn3D ('x', params_PINB.ang, cell.horLen - placementInfo.width [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
				height = 0.0;
				for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
					// 1열
					if (yy == 0) {
						elemList.Push (placePINB (params_PINB));
						moveIn3D ('z', params_PINB.ang, -(placementInfo.height [0] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						height += placementInfo.height [0] - 0.150;
					// 마지막 열
					} else if (yy == placementInfo.nVerEuroform - 1) {
						height += placementInfo.height [placementInfo.nVerEuroform-1] - 0.150;
						moveIn3D ('z', params_PINB.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						elemList.Push (placePINB (params_PINB));
					// 나머지 열
					} else {
						height += placementInfo.height [yy];
						if (abs (placementInfo.height [yy] - 0.600) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.300, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.500) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.450) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.400) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.200, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.100, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.300) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						} else if (abs (placementInfo.height [yy] - 0.200) < EPS) {
							moveIn3D ('z', params_PINB.ang, -0.150, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
							elemList.Push (placePINB (params_PINB));
							moveIn3D ('z', params_PINB.ang, -0.050, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
						}
					}
				}
				moveIn3D ('z', params_PINB.ang, height, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
				moveIn3D ('x', params_PINB.ang, -placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			}
		}

		// 핀볼트 배치 (수평)
		params_PINB.leftBottomX = cell.leftBottomX;
		params_PINB.leftBottomY = cell.leftBottomY;
		params_PINB.leftBottomZ = cell.leftBottomZ;
		params_PINB.ang = cell.ang;
		params_PINB.bPinBoltRot90 = TRUE;
		params_PINB.boltLen = 0.150;
		params_PINB.angX = DegreeToRad (270.0);
		params_PINB.angY = DegreeToRad (0.0);

		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_PINB.ang, height - (placementInfo.height [0] - 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('y', params_PINB.ang, -(0.2135), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('x', params_PINB.ang, cell.horLen - placementInfo.width [0], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);

		// 1열
		width = 0.0;
		for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
			elemList.Push (placePINB (params_PINB));
			moveIn3D ('x', params_PINB.ang, -placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			width += placementInfo.width [xx];
		}
		// 2열
		moveIn3D ('z', params_PINB.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		moveIn3D ('x', params_PINB.ang, width, &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
		for (xx = 1 ; xx < placementInfo.nHorEuroform ; ++xx) {
			elemList.Push (placePINB (params_PINB));
			moveIn3D ('x', params_PINB.ang, -placementInfo.width [xx], &params_PINB.leftBottomX, &params_PINB.leftBottomY, &params_PINB.leftBottomZ);
			width += placementInfo.width [xx];
		}

		// 벽체 타이 (현재면에서 했으므로 생략)

		// 헤드 피스
		params_PUSH.leftBottomX = cell.leftBottomX;
		params_PUSH.leftBottomY = cell.leftBottomY;
		params_PUSH.leftBottomZ = cell.leftBottomZ;
		params_PUSH.ang = cell.ang;

		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		width = 0.0;
		for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
			width += placementInfo.width [xx];
		}
		moveIn3D ('z', params_PUSH.ang, height - (placementInfo.height [0] - 0.150 - 0.100) - 0.200, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		moveIn3D ('y', params_PUSH.ang, -0.1725, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		moveIn3D ('x', params_PUSH.ang, cell.horLen - 0.800, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);

		// 처음 행
		elemList.Push (placePUSH_hor (params_PUSH));
		moveIn3D ('z', params_PUSH.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		elemList.Push (placePUSH_hor (params_PUSH));
		moveIn3D ('z', params_PUSH.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) + cell.verLen + (-placementInfo.height [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		elev_headpiece = width - 0.800;
		moveIn3D ('x', params_PUSH.ang, 0.600 - elev_headpiece, &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		// 마지막 행
		elemList.Push (placePUSH_hor (params_PUSH));
		moveIn3D ('z', params_PUSH.ang, (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_PUSH.leftBottomX, &params_PUSH.leftBottomY, &params_PUSH.leftBottomZ);
		elemList.Push (placePUSH_hor (params_PUSH));

		// 결합철물
		params_JOIN.leftBottomX = cell.leftBottomX;
		params_JOIN.leftBottomY = cell.leftBottomY;
		params_JOIN.leftBottomZ = cell.leftBottomZ;
		params_JOIN.ang = cell.ang;

		height = 0.0;
		for (xx = 0 ; xx < placementInfo.nVerEuroform ; ++xx) {
			height += placementInfo.height [xx];
		}
		moveIn3D ('z', params_JOIN.ang, height - (placementInfo.height [0] - 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('y', params_JOIN.ang, -0.0455, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('x', params_JOIN.ang, cell.horLen - 0.150, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

		// 처음 행
		elemList.Push (placeJOIN (params_JOIN));
		moveIn3D ('z', params_JOIN.ang, (placementInfo.height [0] - 0.150) - cell.verLen - (-placementInfo.height [placementInfo.nVerEuroform-1] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		elemList.Push (placeJOIN (params_JOIN));
		moveIn3D ('z', params_JOIN.ang, -(placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) + cell.verLen + (-placementInfo.height [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
		moveIn3D ('x', params_JOIN.ang, -cell.horLen + 0.300, &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);

		// 마지막 행
		elemList.Push (placeJOIN (params_JOIN));
		moveIn3D ('z', params_JOIN.ang, (placementInfo.height [placementInfo.nVerEuroform-1] - 0.150) - cell.verLen - (-placementInfo.height [0] + 0.150), &params_JOIN.leftBottomX, &params_JOIN.leftBottomY, &params_JOIN.leftBottomZ);
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

// 테이블폼 상단 배치하기 - 가로 방향
GSErrCode	WallTableformPlacingZone::placeTableformOnWall_Horizontal (CellForWallTableform cell, UpperCellForWallTableform upperCell)
{
	GSErrCode	err = NoError;
	short	xx;
	double	remainWidth = abs (placingZone.marginTop - upperCell.formWidth1 - upperCell.formWidth2);
	double	width;
	double	remainObjLen;
	placementInfoForWallTableform	placementInfo;

	Euroform	params_UFOM1 [5];
	Euroform	params_UFOM2 [5];
	Plywood		params_PLYW [5];
	Wood		params_TIMB [5];

	placementInfo.nHorEuroform = 0;
	placementInfo.nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		placementInfo.width [xx] = 0.0;
		placementInfo.height [xx] = 0.0;
	}

	// 상단 여백을 채우기로 한 경우
	if (upperCell.bFill == true) {
		if (abs (cell.horLen - 6.000) < EPS) {
			placementInfo.nHorEuroform = 5;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 1.200;
			placementInfo.width [3] = 1.200;
			placementInfo.width [4] = 1.200;
		} else if (abs (cell.horLen - 5.700) < EPS) {
			placementInfo.nHorEuroform = 5;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 1.200;
			placementInfo.width [3] = 1.200;
			placementInfo.width [4] = 0.900;
		} else if (abs (cell.horLen - 5.400) < EPS) {
			placementInfo.nHorEuroform = 5;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 1.200;
			placementInfo.width [3] = 0.900;
			placementInfo.width [4] = 0.900;
		} else if (abs (cell.horLen - 5.100) < EPS) {
			placementInfo.nHorEuroform = 5;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 1.200;
			placementInfo.width [3] = 0.900;
			placementInfo.width [4] = 0.600;
		} else if (abs (cell.horLen - 4.800) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 1.200;
			placementInfo.width [3] = 1.200;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 4.500) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 1.200;
			placementInfo.width [3] = 0.900;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 4.200) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 0.900;
			placementInfo.width [3] = 0.900;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 3.900) < EPS) {
			placementInfo.nHorEuroform = 4;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 0.900;
			placementInfo.width [3] = 0.600;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 3.600) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 1.200;
			placementInfo.width [3] = 0.0;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 3.300) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 0.900;
			placementInfo.width [3] = 0.0;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 3.000) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 0.600;
			placementInfo.width [3] = 0.0;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 2.700) < EPS) {
			placementInfo.nHorEuroform = 3;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 0.900;
			placementInfo.width [2] = 0.600;
			placementInfo.width [3] = 0.0;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 2.400) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 1.200;
			placementInfo.width [2] = 0.0;
			placementInfo.width [3] = 0.0;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 2.100) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 1.200;
			placementInfo.width [1] = 0.900;
			placementInfo.width [2] = 0.0;
			placementInfo.width [3] = 0.0;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 1.800) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.900;
			placementInfo.width [1] = 0.900;
			placementInfo.width [2] = 0.0;
			placementInfo.width [3] = 0.0;
			placementInfo.width [4] = 0.0;
		} else if (abs (cell.horLen - 1.500) < EPS) {
			placementInfo.nHorEuroform = 2;
			placementInfo.width [0] = 0.900;
			placementInfo.width [1] = 0.600;
			placementInfo.width [2] = 0.0;
			placementInfo.width [3] = 0.0;
			placementInfo.width [4] = 0.0;
		} else {
			placementInfo.nHorEuroform = 0;
			placementInfo.width [0] = 0.0;
			placementInfo.width [1] = 0.0;
			placementInfo.width [2] = 0.0;
			placementInfo.width [3] = 0.0;
			placementInfo.width [4] = 0.0;
		}

		// 합판 또는 목재의 전체 길이
		width = 0.0;
		for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
			width += placementInfo.width [xx];
		}

		//////////////////////////////////////////////////////////////// 현재면
		// 1열
		if (placementInfo.nHorEuroform >= 1) {
			// 1번째 행: 유로폼
			params_UFOM1 [0].leftBottomX = cell.leftBottomX;
			params_UFOM1 [0].leftBottomY = cell.leftBottomY;
			params_UFOM1 [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [0].ang = cell.ang;
			params_UFOM1 [0].u_ins_wall = false;
			params_UFOM1 [0].width = upperCell.formWidth1;
			params_UFOM1 [0].height = placementInfo.width [0];

			// 2번째 행: 유로폼
			params_UFOM2 [0].leftBottomX = cell.leftBottomX;
			params_UFOM2 [0].leftBottomY = cell.leftBottomY;
			params_UFOM2 [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [0].ang = cell.ang;
			params_UFOM2 [0].u_ins_wall = false;
			params_UFOM2 [0].width = upperCell.formWidth2;
			params_UFOM2 [0].height = placementInfo.width [0];

			// 3번째 행: 합판 또는 목재
			params_PLYW [0].leftBottomX = cell.leftBottomX;
			params_PLYW [0].leftBottomY = cell.leftBottomY;
			params_PLYW [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [0].ang = cell.ang;
			params_PLYW [0].p_wid = remainWidth;
			params_PLYW [0].p_leng = width;	//placementInfo.width [0];
			params_PLYW [0].w_dir_wall = false;

			params_TIMB [0].leftBottomX = cell.leftBottomX;
			params_TIMB [0].leftBottomY = cell.leftBottomY;
			params_TIMB [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [0].ang = cell.ang;
			params_TIMB [0].w_w = 0.050;
			params_TIMB [0].w_h = remainWidth;
			params_TIMB [0].w_leng = width;	//placementInfo.width [0];
			params_TIMB [0].w_ang = 0.0;
		}

		// 2열
		if (placementInfo.nHorEuroform >= 2) {
			// 1번째 행: 유로폼
			params_UFOM1 [1].leftBottomX = params_UFOM1 [0].leftBottomX;
			params_UFOM1 [1].leftBottomY = params_UFOM1 [0].leftBottomY;
			params_UFOM1 [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [1].ang = cell.ang;
			params_UFOM1 [1].u_ins_wall = false;
			params_UFOM1 [1].width = upperCell.formWidth1;
			params_UFOM1 [1].height = placementInfo.width [1];
			moveIn3D ('x', params_UFOM1 [1].ang, placementInfo.width [0], &params_UFOM1 [1].leftBottomX, &params_UFOM1 [1].leftBottomY, &params_UFOM1 [1].leftBottomZ);

			// 2번째 행: 유로폼
			params_UFOM2 [1].leftBottomX = params_UFOM2 [0].leftBottomX;
			params_UFOM2 [1].leftBottomY = params_UFOM2 [0].leftBottomY;
			params_UFOM2 [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [1].ang = cell.ang;
			params_UFOM2 [1].u_ins_wall = false;
			params_UFOM2 [1].width = upperCell.formWidth2;
			params_UFOM2 [1].height = placementInfo.width [1];
			moveIn3D ('x', params_UFOM2 [1].ang, placementInfo.width [0], &params_UFOM2 [1].leftBottomX, &params_UFOM2 [1].leftBottomY, &params_UFOM2 [1].leftBottomZ);

			// 3번째 행: 합판 또는 목재
			params_PLYW [1].leftBottomX = params_PLYW [0].leftBottomX;
			params_PLYW [1].leftBottomY = params_PLYW [0].leftBottomY;
			params_PLYW [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [1].ang = cell.ang;
			params_PLYW [1].p_wid = remainWidth;
			params_PLYW [1].p_leng = width;	//placementInfo.width [1];
			params_PLYW [1].w_dir_wall = false;
			moveIn3D ('x', params_PLYW [1].ang, placementInfo.width [0], &params_PLYW [1].leftBottomX, &params_PLYW [1].leftBottomY, &params_PLYW [1].leftBottomZ);

			params_TIMB [1].leftBottomX = params_TIMB [0].leftBottomX;
			params_TIMB [1].leftBottomY = params_TIMB [0].leftBottomY;
			params_TIMB [1].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [1].ang = cell.ang;
			params_TIMB [1].w_w = 0.050;
			params_TIMB [1].w_h = remainWidth;
			params_TIMB [1].w_leng = width;	//placementInfo.width [1];
			params_TIMB [1].w_ang = 0.0;
			moveIn3D ('x', params_TIMB [1].ang, placementInfo.width [0], &params_TIMB [1].leftBottomX, &params_TIMB [1].leftBottomY, &params_TIMB [1].leftBottomZ);
		}

		// 3열
		if (placementInfo.nHorEuroform >= 3) {
			// 1번째 행: 유로폼
			params_UFOM1 [2].leftBottomX = params_UFOM1 [1].leftBottomX;
			params_UFOM1 [2].leftBottomY = params_UFOM1 [1].leftBottomY;
			params_UFOM1 [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [2].ang = cell.ang;
			params_UFOM1 [2].u_ins_wall = false;
			params_UFOM1 [2].width = upperCell.formWidth1;
			params_UFOM1 [2].height = placementInfo.width [2];
			moveIn3D ('x', params_UFOM1 [2].ang, placementInfo.width [1], &params_UFOM1 [2].leftBottomX, &params_UFOM1 [2].leftBottomY, &params_UFOM1 [2].leftBottomZ);

			// 2번째 행: 유로폼
			params_UFOM2 [2].leftBottomX = params_UFOM2 [1].leftBottomX;
			params_UFOM2 [2].leftBottomY = params_UFOM2 [1].leftBottomY;
			params_UFOM2 [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [2].ang = cell.ang;
			params_UFOM2 [2].u_ins_wall = false;
			params_UFOM2 [2].width = upperCell.formWidth2;
			params_UFOM2 [2].height = placementInfo.width [2];
			moveIn3D ('x', params_UFOM2 [2].ang, placementInfo.width [1], &params_UFOM2 [2].leftBottomX, &params_UFOM2 [2].leftBottomY, &params_UFOM2 [2].leftBottomZ);

			// 3번째 행: 합판 또는 목재
			params_PLYW [2].leftBottomX = params_PLYW [1].leftBottomX;
			params_PLYW [2].leftBottomY = params_PLYW [1].leftBottomY;
			params_PLYW [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [2].ang = cell.ang;
			params_PLYW [2].p_wid = remainWidth;
			params_PLYW [2].p_leng = width;	//placementInfo.width [2];
			params_PLYW [2].w_dir_wall = false;
			moveIn3D ('x', params_PLYW [2].ang, placementInfo.width [1], &params_PLYW [2].leftBottomX, &params_PLYW [2].leftBottomY, &params_PLYW [2].leftBottomZ);

			params_TIMB [2].leftBottomX = params_TIMB [1].leftBottomX;
			params_TIMB [2].leftBottomY = params_TIMB [1].leftBottomY;
			params_TIMB [2].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [2].ang = cell.ang;
			params_TIMB [2].w_w = 0.050;
			params_TIMB [2].w_h = remainWidth;
			params_TIMB [2].w_leng = width;	//placementInfo.width [2];
			params_TIMB [2].w_ang = 0.0;
			moveIn3D ('x', params_TIMB [2].ang, placementInfo.width [1], &params_TIMB [2].leftBottomX, &params_TIMB [2].leftBottomY, &params_TIMB [2].leftBottomZ);
		}

		// 4열
		if (placementInfo.nHorEuroform >= 4) {
			// 1번째 행: 유로폼
			params_UFOM1 [3].leftBottomX = params_UFOM1 [2].leftBottomX;
			params_UFOM1 [3].leftBottomY = params_UFOM1 [2].leftBottomY;
			params_UFOM1 [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [3].ang = cell.ang;
			params_UFOM1 [3].u_ins_wall = false;
			params_UFOM1 [3].width = upperCell.formWidth1;
			params_UFOM1 [3].height = placementInfo.width [3];
			moveIn3D ('x', params_UFOM1 [3].ang, placementInfo.width [2], &params_UFOM1 [3].leftBottomX, &params_UFOM1 [3].leftBottomY, &params_UFOM1 [3].leftBottomZ);

			// 2번째 행: 유로폼
			params_UFOM2 [3].leftBottomX = params_UFOM2 [2].leftBottomX;
			params_UFOM2 [3].leftBottomY = params_UFOM2 [2].leftBottomY;
			params_UFOM2 [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [3].ang = cell.ang;
			params_UFOM2 [3].u_ins_wall = false;
			params_UFOM2 [3].width = upperCell.formWidth2;
			params_UFOM2 [3].height = placementInfo.width [3];
			moveIn3D ('x', params_UFOM2 [3].ang, placementInfo.width [2], &params_UFOM2 [3].leftBottomX, &params_UFOM2 [3].leftBottomY, &params_UFOM2 [3].leftBottomZ);

			// 3번째 행: 합판 또는 목재
			params_PLYW [3].leftBottomX = params_PLYW [2].leftBottomX;
			params_PLYW [3].leftBottomY = params_PLYW [2].leftBottomY;
			params_PLYW [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [3].ang = cell.ang;
			params_PLYW [3].p_wid = remainWidth;
			params_PLYW [3].p_leng = width;	//placementInfo.width [3];
			params_PLYW [3].w_dir_wall = false;
			moveIn3D ('x', params_PLYW [3].ang, placementInfo.width [2], &params_PLYW [3].leftBottomX, &params_PLYW [3].leftBottomY, &params_PLYW [3].leftBottomZ);

			params_TIMB [3].leftBottomX = params_TIMB [2].leftBottomX;
			params_TIMB [3].leftBottomY = params_TIMB [2].leftBottomY;
			params_TIMB [3].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [3].ang = cell.ang;
			params_TIMB [3].w_w = 0.050;
			params_TIMB [3].w_h = remainWidth;
			params_TIMB [3].w_leng = width;	//placementInfo.width [3];
			params_TIMB [3].w_ang = 0.0;
			moveIn3D ('x', params_TIMB [3].ang, placementInfo.width [2], &params_TIMB [3].leftBottomX, &params_TIMB [3].leftBottomY, &params_TIMB [3].leftBottomZ);
		}

		// 5열
		if (placementInfo.nHorEuroform >= 5) {
			// 1번째 행: 유로폼
			params_UFOM1 [4].leftBottomX = params_UFOM1 [3].leftBottomX;
			params_UFOM1 [4].leftBottomY = params_UFOM1 [3].leftBottomY;
			params_UFOM1 [4].leftBottomZ = upperCell.leftBottomZ + cell.verLen;
			params_UFOM1 [4].ang = cell.ang;
			params_UFOM1 [4].u_ins_wall = false;
			params_UFOM1 [4].width = upperCell.formWidth1;
			params_UFOM1 [4].height = placementInfo.width [4];
			moveIn3D ('x', params_UFOM1 [4].ang, placementInfo.width [3], &params_UFOM1 [4].leftBottomX, &params_UFOM1 [4].leftBottomY, &params_UFOM1 [4].leftBottomZ);

			// 2번째 행: 유로폼
			params_UFOM2 [4].leftBottomX = params_UFOM2 [3].leftBottomX;
			params_UFOM2 [4].leftBottomY = params_UFOM2 [3].leftBottomY;
			params_UFOM2 [4].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1;
			params_UFOM2 [4].ang = cell.ang;
			params_UFOM2 [4].u_ins_wall = false;
			params_UFOM2 [4].width = upperCell.formWidth2;
			params_UFOM2 [4].height = placementInfo.width [4];
			moveIn3D ('x', params_UFOM2 [4].ang, placementInfo.width [3], &params_UFOM2 [4].leftBottomX, &params_UFOM2 [4].leftBottomY, &params_UFOM2 [4].leftBottomZ);

			// 3번째 행: 합판 또는 목재
			params_PLYW [4].leftBottomX = params_PLYW [3].leftBottomX;
			params_PLYW [4].leftBottomY = params_PLYW [3].leftBottomY;
			params_PLYW [4].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_PLYW [4].ang = cell.ang;
			params_PLYW [4].p_wid = remainWidth;
			params_PLYW [4].p_leng = width;	//placementInfo.width [4];
			params_PLYW [4].w_dir_wall = false;
			moveIn3D ('x', params_PLYW [4].ang, placementInfo.width [4], &params_PLYW [4].leftBottomX, &params_PLYW [4].leftBottomY, &params_PLYW [4].leftBottomZ);

			params_TIMB [4].leftBottomX = params_TIMB [3].leftBottomX;
			params_TIMB [4].leftBottomY = params_TIMB [3].leftBottomY;
			params_TIMB [4].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
			params_TIMB [4].ang = cell.ang;
			params_TIMB [4].w_w = 0.050;
			params_TIMB [4].w_h = remainWidth;
			params_TIMB [4].w_leng = width;	//placementInfo.width [4];
			params_TIMB [4].w_ang = 0.0;
			moveIn3D ('x', params_TIMB [4].ang, placementInfo.width [4], &params_TIMB [4].leftBottomX, &params_TIMB [4].leftBottomY, &params_TIMB [4].leftBottomZ);
		}

		for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
			if (upperCell.bEuroform1) {
				elemList.Push (placeUFOM_up (params_UFOM1 [xx]));
			}
			if (upperCell.bEuroform2) {
				elemList.Push (placeUFOM_up (params_UFOM2 [xx]));
			}
		}

		// 합판의 경우
		if (remainWidth > 0.110 - EPS) {
			remainObjLen = params_PLYW [0].p_leng;
			while (remainObjLen > 0) {
				if (remainObjLen > 2.440) {
					remainObjLen -= 2.440;
					params_PLYW [0].p_leng = 2.440;
					elemList.Push (placePLYW (params_PLYW [0]));
					moveIn3D ('x', params_PLYW [0].ang, 2.440, &params_PLYW [0].leftBottomX, &params_PLYW [0].leftBottomY, &params_PLYW [0].leftBottomZ);
				} else {
					params_PLYW [0].p_leng = remainObjLen;
					remainObjLen = 0;
					elemList.Push (placePLYW (params_PLYW [0]));
				}
			}

		// 목재의 경우
		} else if (remainWidth > EPS) {
			remainObjLen = params_TIMB [0].w_leng;
			while (remainObjLen > 0) {
				if (remainObjLen > 3.600) {
					remainObjLen -= 3.600;
					params_TIMB [0].w_leng = 3.600;
					elemList.Push (placeTIMB (params_TIMB [0]));
					moveIn3D ('x', params_TIMB [0].ang, 3.600, &params_TIMB [0].leftBottomX, &params_TIMB [0].leftBottomY, &params_TIMB [0].leftBottomZ);
				} else {
					params_TIMB [0].w_leng = remainObjLen;
					remainObjLen = 0;
					elemList.Push (placeTIMB (params_TIMB [0]));
				}
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
				params_UFOM1 [0].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [0], &params_UFOM1 [0].leftBottomX, &params_UFOM1 [0].leftBottomY, &params_UFOM1 [0].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM1 [0].leftBottomX, &params_UFOM1 [0].leftBottomY, &params_UFOM1 [0].leftBottomZ);

				// 2번째 행: 유로폼
				params_UFOM2 [0].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [0], &params_UFOM2 [0].leftBottomX, &params_UFOM2 [0].leftBottomY, &params_UFOM2 [0].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM2 [0].leftBottomX, &params_UFOM2 [0].leftBottomY, &params_UFOM2 [0].leftBottomZ);
			}

			// 2열
			if (placementInfo.nHorEuroform >= 2) {
				// 1번째 행: 유로폼
				params_UFOM1 [1].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [1], &params_UFOM1 [1].leftBottomX, &params_UFOM1 [1].leftBottomY, &params_UFOM1 [1].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM1 [1].leftBottomX, &params_UFOM1 [1].leftBottomY, &params_UFOM1 [1].leftBottomZ);

				// 2번째 행: 유로폼
				params_UFOM2 [1].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [1], &params_UFOM2 [1].leftBottomX, &params_UFOM2 [1].leftBottomY, &params_UFOM2 [1].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM2 [1].leftBottomX, &params_UFOM2 [1].leftBottomY, &params_UFOM2 [1].leftBottomZ);
			}

			// 3열
			if (placementInfo.nHorEuroform >= 3) {
				// 1번째 행: 유로폼
				params_UFOM1 [2].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [2], &params_UFOM1 [2].leftBottomX, &params_UFOM1 [2].leftBottomY, &params_UFOM1 [2].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM1 [2].leftBottomX, &params_UFOM1 [2].leftBottomY, &params_UFOM1 [2].leftBottomZ);

				// 2번째 행: 유로폼
				params_UFOM2 [2].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [2], &params_UFOM2 [2].leftBottomX, &params_UFOM2 [2].leftBottomY, &params_UFOM2 [2].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM2 [2].leftBottomX, &params_UFOM2 [2].leftBottomY, &params_UFOM2 [2].leftBottomZ);
			}

			// 4열
			if (placementInfo.nHorEuroform >= 4) {
				// 1번째 행: 유로폼
				params_UFOM1 [3].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [3], &params_UFOM1 [3].leftBottomX, &params_UFOM1 [3].leftBottomY, &params_UFOM1 [3].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM1 [3].leftBottomX, &params_UFOM1 [3].leftBottomY, &params_UFOM1 [3].leftBottomZ);

				// 2번째 행: 유로폼
				params_UFOM2 [3].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [3], &params_UFOM2 [3].leftBottomX, &params_UFOM2 [3].leftBottomY, &params_UFOM2 [3].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM2 [3].leftBottomX, &params_UFOM2 [3].leftBottomY, &params_UFOM2 [3].leftBottomZ);
			}

			// 5열
			if (placementInfo.nHorEuroform >= 4) {
				// 1번째 행: 유로폼
				params_UFOM1 [4].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [4], &params_UFOM1 [4].leftBottomX, &params_UFOM1 [4].leftBottomY, &params_UFOM1 [4].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM1 [4].leftBottomX, &params_UFOM1 [4].leftBottomY, &params_UFOM1 [4].leftBottomZ);

				// 2번째 행: 유로폼
				params_UFOM2 [4].ang += DegreeToRad (180.0);
				moveIn3D ('x', cell.ang, placementInfo.width [4], &params_UFOM2 [4].leftBottomX, &params_UFOM2 [4].leftBottomY, &params_UFOM2 [4].leftBottomZ);
				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_UFOM2 [4].leftBottomX, &params_UFOM2 [4].leftBottomY, &params_UFOM2 [4].leftBottomZ);
			}

			for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
				if (upperCell.bEuroform1) {
					elemList.Push (placeUFOM_up (params_UFOM1 [xx]));
				}
				if (upperCell.bEuroform2) {
					elemList.Push (placeUFOM_up (params_UFOM2 [xx]));
				}
			}

			// 합판의 경우
			if (remainWidth > 0.110 - EPS) {
				params_PLYW [0].leftBottomX = cell.leftBottomX;
				params_PLYW [0].leftBottomY = cell.leftBottomY;
				params_PLYW [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
				params_PLYW [0].ang = cell.ang;
				params_PLYW [0].p_wid = remainWidth;
				params_PLYW [0].p_leng = width;
				params_PLYW [0].w_dir_wall = false;

				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_PLYW [0].leftBottomX, &params_PLYW [0].leftBottomY, &params_PLYW [0].leftBottomZ);
				params_PLYW [0].ang += DegreeToRad (180.0);

				remainObjLen = params_PLYW [0].p_leng;
				while (remainObjLen > 0) {
					if (remainObjLen > 2.440) {
						remainObjLen -= 2.440;
						params_PLYW [0].p_leng = 2.440;
						moveIn3D ('x', params_PLYW [0].ang, -2.440, &params_PLYW [0].leftBottomX, &params_PLYW [0].leftBottomY, &params_PLYW [0].leftBottomZ);
						elemList.Push (placePLYW (params_PLYW [0]));
					} else {
						params_PLYW [0].p_leng = remainObjLen;
						remainObjLen = 0;
						moveIn3D ('x', params_PLYW [0].ang, -params_PLYW [0].p_leng, &params_PLYW [0].leftBottomX, &params_PLYW [0].leftBottomY, &params_PLYW [0].leftBottomZ);
						elemList.Push (placePLYW (params_PLYW [0]));
					}
				}

			// 목재의 경우
			} else if (remainWidth > EPS) {
				params_TIMB [0].leftBottomX = cell.leftBottomX;
				params_TIMB [0].leftBottomY = cell.leftBottomY;
				params_TIMB [0].leftBottomZ = upperCell.leftBottomZ + cell.verLen + upperCell.formWidth1 + upperCell.formWidth2;
				params_TIMB [0].ang = cell.ang;
				params_TIMB [0].w_w = 0.050;
				params_TIMB [0].w_h = remainWidth;
				params_TIMB [0].w_leng = width;
				params_TIMB [0].w_ang = 0.0;

				moveIn3D ('y', cell.ang, infoWall.wallThk, &params_TIMB [0].leftBottomX, &params_TIMB [0].leftBottomY, &params_TIMB [0].leftBottomZ);
				params_TIMB [0].ang += DegreeToRad (180.0);

				remainObjLen = params_TIMB [0].w_leng;
				while (remainObjLen > 0) {
					if (remainObjLen > 3.600) {
						remainObjLen -= 3.600;
						params_TIMB [0].w_leng = 3.600;
						moveIn3D ('x', params_TIMB [0].ang, -3.600, &params_TIMB [0].leftBottomX, &params_TIMB [0].leftBottomY, &params_TIMB [0].leftBottomZ);
						elemList.Push (placeTIMB (params_TIMB [0]));
					} else {
						params_TIMB [0].w_leng = remainObjLen;
						remainObjLen = 0;
						moveIn3D ('x', params_TIMB [0].ang, -params_TIMB [0].w_leng, &params_TIMB [0].leftBottomX, &params_TIMB [0].leftBottomY, &params_TIMB [0].leftBottomZ);
						elemList.Push (placeTIMB (params_TIMB [0]));
					}
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

// 선호하는 테이블폼 너비를 선택하기 위한 다이얼로그 - 세로 방향
short DGCALLBACK wallTableformPlacerHandler1_Vertical (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1750");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1700");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1650");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1600");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1550");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1450");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1400");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1350");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1300");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1250");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1150");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1100");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1050");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1000");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "950");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "850");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "800");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "700");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "650");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "400");
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
			} else if (abs (width - 1.800) < EPS) {
				pipeWidth = 1.700;
			} else if (abs (width - 1.750) < EPS) {
				pipeWidth = 1.700;
			} else if (abs (width - 1.700) < EPS) {
				pipeWidth = 1.600;
			} else if (abs (width - 1.650) < EPS) {
				pipeWidth = 1.600;
			} else if (abs (width - 1.600) < EPS) {
				pipeWidth = 1.500;
			} else if (abs (width - 1.550) < EPS) {
				pipeWidth = 1.500;
			} else if (abs (width - 1.500) < EPS) {
				pipeWidth = 1.400;
			} else if (abs (width - 1.450) < EPS) {
				pipeWidth = 1.400;
			} else if (abs (width - 1.400) < EPS) {
				pipeWidth = 1.300;
			} else if (abs (width - 1.350) < EPS) {
				pipeWidth = 1.300;
			} else if (abs (width - 1.300) < EPS) {
				pipeWidth = 1.200;
			} else if (abs (width - 1.250) < EPS) {
				pipeWidth = 1.200;
			} else if (abs (width - 1.200) < EPS) {
				pipeWidth = 1.100;
			} else if (abs (width - 1.150) < EPS) {
				pipeWidth = 1.100;
			} else if (abs (width - 1.100) < EPS) {
				pipeWidth = 1.000;
			} else if (abs (width - 1.050) < EPS) {
				pipeWidth = 1.000;
			} else if (abs (width - 1.000) < EPS) {
				pipeWidth = 0.900;
			} else if (abs (width - 0.950) < EPS) {
				pipeWidth = 0.900;
			} else if (abs (width - 0.900) < EPS) {
				pipeWidth = 0.800;
			} else if (abs (width - 0.850) < EPS) {
				pipeWidth = 0.800;
			} else if (abs (width - 0.800) < EPS) {
				pipeWidth = 0.700;
			} else if (abs (width - 0.750) < EPS) {
				pipeWidth = 0.700;
			} else if (abs (width - 0.700) < EPS) {
				pipeWidth = 0.600;
			} else if (abs (width - 0.650) < EPS) {
				pipeWidth = 0.600;
			} else if (abs (width - 0.600) < EPS) {
				pipeWidth = 0.500;
			} else if (abs (width - 0.500) < EPS) {
				pipeWidth = 0.400;
			} else if (abs (width - 0.450) < EPS) {
				pipeWidth = 0.400;
			} else if (abs (width - 0.400) < EPS) {
				pipeWidth = 0.300;
			} else {
				pipeWidth = 0.0;
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

// 테이블폼 배치를 위한 질의를 요청하는 다이얼로그 - 세로 방향
short DGCALLBACK wallTableformPlacerHandler2_Vertical (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGSetDialogTitle (dialogID, "벽에 배치 (테이블폼)");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			// 이전 버튼
			DGSetItemText (dialogID, DG_PREV, "이 전");

			//////////////////////////////////////////////////////////// 아이템 배치 (나머지)
			DGSetItemFont (dialogID, POPUP_TYPE_SELECTOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR, DG_POPUP_BOTTOM, "타입A");
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR, DG_POPUP_BOTTOM, "타입B");

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
			DGDisableItem (dialogID, LABEL_LAYER_WALLTIE);
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "헤드피스");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "목재");
			DGSetItemText (dialogID, LABEL_LAYER_HIDDEN, "숨김");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

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
			DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE);

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

			ucb.itemID	 = USERCONTROL_LAYER_HIDDEN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN, 1);

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
				} else if (placingZone.n1750w > 0) {
					width = 1.750;
					placingZone.n1750w --;
				} else if (placingZone.n1700w > 0) {
					width = 1.700;
					placingZone.n1700w --;
				} else if (placingZone.n1650w > 0) {
					width = 1.650;
					placingZone.n1650w --;
				} else if (placingZone.n1600w > 0) {
					width = 1.600;
					placingZone.n1600w --;
				} else if (placingZone.n1550w > 0) {
					width = 1.550;
					placingZone.n1550w --;
				} else if (placingZone.n1500w > 0) {
					width = 1.500;
					placingZone.n1500w --;
				} else if (placingZone.n1450w > 0) {
					width = 1.450;
					placingZone.n1450w --;
				} else if (placingZone.n1400w > 0) {
					width = 1.400;
					placingZone.n1400w --;
				} else if (placingZone.n1350w > 0) {
					width = 1.350;
					placingZone.n1350w --;
				} else if (placingZone.n1300w > 0) {
					width = 1.300;
					placingZone.n1300w --;
				} else if (placingZone.n1250w > 0) {
					width = 1.250;
					placingZone.n1250w --;
				} else if (placingZone.n1200w > 0) {
					width = 1.200;
					placingZone.n1200w --;
				} else if (placingZone.n1150w > 0) {
					width = 1.150;
					placingZone.n1150w --;
				} else if (placingZone.n1100w > 0) {
					width = 1.100;
					placingZone.n1100w --;
				} else if (placingZone.n1050w > 0) {
					width = 1.050;
					placingZone.n1050w --;
				} else if (placingZone.n1000w > 0) {
					width = 1.000;
					placingZone.n1000w --;
				} else if (placingZone.n950w > 0) {
					width = 0.950;
					placingZone.n950w --;
				} else if (placingZone.n900w > 0) {
					width = 0.900;
					placingZone.n900w --;
				} else if (placingZone.n850w > 0) {
					width = 0.850;
					placingZone.n850w --;
				} else if (placingZone.n800w > 0) {
					width = 0.800;
					placingZone.n800w --;
				} else if (placingZone.n750w > 0) {
					width = 0.750;
					placingZone.n750w --;
				} else if (placingZone.n700w > 0) {
					width = 0.700;
					placingZone.n700w --;
				} else if (placingZone.n650w > 0) {
					width = 0.650;
					placingZone.n650w --;
				} else if (placingZone.n600w > 0) {
					width = 0.600;
					placingZone.n600w --;
				} else if (placingZone.n500w > 0) {
					width = 0.500;
					placingZone.n500w --;
				} else if (placingZone.n450w > 0) {
					width = 0.450;
					placingZone.n450w --;
				} else if (placingZone.n400w > 0) {
					width = 0.400;
					placingZone.n400w --;
				} else
					width = 0.0;

				// 콤보박스의 값 설정
				if (width > EPS) {
					if (width + EPS > 2.300)		DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 1);
					else if (width + EPS > 2.250)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 2);
					else if (width + EPS > 2.200)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 3);
					else if (width + EPS > 2.150)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 4);
					else if (width + EPS > 2.100)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 5);
					else if (width + EPS > 2.050)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 6);
					else if (width + EPS > 2.000)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 7);
					else if (width + EPS > 1.950)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 8);
					else if (width + EPS > 1.900)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 9);
					else if (width + EPS > 1.850)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 10);
					else if (width + EPS > 1.800)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 11);
					else if (width + EPS > 1.750)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 12);
					else if (width + EPS > 1.700)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 13);
					else if (width + EPS > 1.650)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 14);
					else if (width + EPS > 1.600)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 15);
					else if (width + EPS > 1.550)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 16);
					else if (width + EPS > 1.500)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 17);
					else if (width + EPS > 1.450)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 18);
					else if (width + EPS > 1.400)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 19);
					else if (width + EPS > 1.350)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 20);
					else if (width + EPS > 1.300)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 21);
					else if (width + EPS > 1.250)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 22);
					else if (width + EPS > 1.200)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 23);
					else if (width + EPS > 1.150)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 24);
					else if (width + EPS > 1.100)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 25);
					else if (width + EPS > 1.050)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 26);
					else if (width + EPS > 1.000)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 27);
					else if (width + EPS > 0.950)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 28);
					else if (width + EPS > 0.900)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 29);
					else if (width + EPS > 0.850)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 30);
					else if (width + EPS > 0.800)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 31);
					else if (width + EPS > 0.750)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 32);
					else if (width + EPS > 0.700)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 33);
					else if (width + EPS > 0.650)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 34);
					else if (width + EPS > 0.600)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 35);
					else if (width + EPS > 0.500)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 36);
					else if (width + EPS > 0.450)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 37);
					else if (width + EPS > 0.400)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 38);
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

			if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR) == 1) {
				// 벽체타이 레이어 비활성화
				DGDisableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE);

				// 레이어 이름 변경
				DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트 세트");
				DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "벽체 타이");
				DGSetItemText (dialogID, LABEL_LAYER_JOIN, "결합철물");

				// 레이어 같이 바뀜
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					switch (item) {
						case USERCONTROL_LAYER_EUROFORM:
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							break;
						case USERCONTROL_LAYER_RECTPIPE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							break;
						case USERCONTROL_LAYER_PINBOLT:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							break;
						case USERCONTROL_LAYER_WALLTIE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							break;
						case USERCONTROL_LAYER_JOIN:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							break;
						case USERCONTROL_LAYER_HEADPIECE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							break;
						case USERCONTROL_LAYER_PLYWOOD:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							break;
						case USERCONTROL_LAYER_WOOD:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							break;
					}
				}
			} else if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR) == 2) {
				// 벽체타이 레이어 활성화
				DGEnableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_WALLTIE);

				// 레이어 이름 변경
				DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "각파이프 행거");
				DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "유로폼 후크");
				DGSetItemText (dialogID, LABEL_LAYER_JOIN, "연결철물");

				// 레이어 같이 바뀜
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					switch (item) {
						case USERCONTROL_LAYER_EUROFORM:
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							break;
						case USERCONTROL_LAYER_RECTPIPE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							break;
						case USERCONTROL_LAYER_RECTPIPE_HANGER:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							break;
						case USERCONTROL_LAYER_EUROFORM_HOOK:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							break;
						case USERCONTROL_LAYER_JOIN:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							break;
						case USERCONTROL_LAYER_HEADPIECE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							break;
						case USERCONTROL_LAYER_PLYWOOD:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							break;
						case USERCONTROL_LAYER_WOOD:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							break;
					}
				}
			}

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

					if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR) == 1) {
						// 타입 지정
						placingZone.type = 1;

						// 레이어 번호 저장
						layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
						layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
						layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
						layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
						layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
						layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
						layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
						layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
						layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);

					} else if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR) == 2) {
						// 타입 지정
						placingZone.type = 2;

						// 레이어 번호 저장
						layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
						layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
						layerInd_RectPipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
						layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
						layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
						layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
						layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
						layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
						layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);
					}
	
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

// 벽 상단의 합판/목재 영역을 유로폼으로 채울지 물어보는 3차 다이얼로그 - 세로 방향
short DGCALLBACK wallTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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

			if (placingZone.marginTop < EPS) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "없음");
			} else if (placingZone.marginTop < 0.110) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "목재");
			} else {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, placingZone.marginTop);

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

// 선호하는 테이블폼 너비를 선택하기 위한 다이얼로그 - 가로 방향
short DGCALLBACK wallTableformPlacerHandler1_Horizontal (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "6000");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "5700");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "5400");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "5100");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "4800");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "4500");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "4200");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "3900");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "3600");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "3300");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "3000");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2700");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2400");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "2100");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1800");
			DGPopUpInsertItem (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PREFER_WIDTH, DG_POPUP_BOTTOM, "1500");
			DGShowItem (dialogID, POPUP_PREFER_WIDTH);

			// Edit 컨트롤: 세로 파이프 길이
			if ((2.300 - placingZone.verLen) < EPS)
				height = 2.300;
			else if ((2.250 - placingZone.verLen) < EPS)
				height = 2.250;
			else if ((2.200 - placingZone.verLen) < EPS)
				height = 2.200;
			else if ((2.150 - placingZone.verLen) < EPS)
				height = 2.150;
			else if ((2.100 - placingZone.verLen) < EPS)
				height = 2.100;
			else if ((2.050 - placingZone.verLen) < EPS)
				height = 2.050;
			else if ((2.000 - placingZone.verLen) < EPS)
				height = 2.000;
			else if ((1.950 - placingZone.verLen) < EPS)
				height = 1.950;
			else if ((1.900 - placingZone.verLen) < EPS)
				height = 1.900;
			else if ((1.850 - placingZone.verLen) < EPS)
				height = 1.850;
			else if ((1.800 - placingZone.verLen) < EPS)
				height = 1.800;
			else if ((1.750 - placingZone.verLen) < EPS)
				height = 1.750;
			else if ((1.700 - placingZone.verLen) < EPS)
				height = 1.700;
			else if ((1.650 - placingZone.verLen) < EPS)
				height = 1.650;
			else if ((1.600 - placingZone.verLen) < EPS)
				height = 1.600;
			else if ((1.550 - placingZone.verLen) < EPS)
				height = 1.550;
			else if ((1.500 - placingZone.verLen) < EPS)
				height = 1.500;
			else if ((1.450 - placingZone.verLen) < EPS)
				height = 1.450;
			else if ((1.400 - placingZone.verLen) < EPS)
				height = 1.400;
			else if ((1.350 - placingZone.verLen) < EPS)
				height = 1.350;
			else if ((1.300 - placingZone.verLen) < EPS)
				height = 1.300;
			else if ((1.250 - placingZone.verLen) < EPS)
				height = 1.250;
			else if ((1.200 - placingZone.verLen) < EPS)
				height = 1.200;
			else if ((1.150 - placingZone.verLen) < EPS)
				height = 1.150;
			else if ((1.100 - placingZone.verLen) < EPS)
				height = 1.100;
			else if ((1.050 - placingZone.verLen) < EPS)
				height = 1.050;
			else if ((1.000 - placingZone.verLen) < EPS)
				height = 1.000;
			else if ((0.950 - placingZone.verLen) < EPS)
				height = 0.950;
			else if ((0.900 - placingZone.verLen) < EPS)
				height = 0.900;
			else if ((0.850 - placingZone.verLen) < EPS)
				height = 0.850;
			else if ((0.800 - placingZone.verLen) < EPS)
				height = 0.800;
			else if ((0.750 - placingZone.verLen) < EPS)
				height = 0.750;
			else if ((0.700 - placingZone.verLen) < EPS)
				height = 0.700;
			else if ((0.650 - placingZone.verLen) < EPS)
				height = 0.650;
			else if ((0.600 - placingZone.verLen) < EPS)
				height = 0.600;
			else if ((0.500 - placingZone.verLen) < EPS)
				height = 0.500;
			else if ((0.450 - placingZone.verLen) < EPS)
				height = 0.450;
			else if ((0.400 - placingZone.verLen) < EPS)
				height = 0.400;
			else
				height = 0;

			// 비계 파이프의 세로 방향 길이 (바뀌지 않음)
			pipeHeight = height - 0.100;

			// Edit 컨트롤: 가로 파이프 길이
			EDITCONTROL_RECT_PIPE_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 230, 45, 100, 25);
			DGDisableItem (dialogID, EDITCONTROL_RECT_PIPE_WIDTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_RECT_PIPE_WIDTH, 5.900);
			DGShowItem (dialogID, EDITCONTROL_RECT_PIPE_WIDTH);

			// Edit 컨트롤: 세로 파이프 길이
			EDITCONTROL_RECT_PIPE_HEIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 360, 45, 100, 25);
			DGDisableItem (dialogID, EDITCONTROL_RECT_PIPE_HEIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_RECT_PIPE_HEIGHT, pipeHeight);
			DGShowItem (dialogID, EDITCONTROL_RECT_PIPE_HEIGHT);

			break;

		case DG_MSG_CHANGE:

			width = atof (DGPopUpGetItemText (dialogID, POPUP_PREFER_WIDTH, DGPopUpGetSelected (dialogID, POPUP_PREFER_WIDTH)).ToCStr ()) / 1000.0;

			// 비계 파이프의 가로 방향 길이
			if (abs (width - 6.000) < EPS) {
				pipeWidth = 5.900;
			} else if (abs (width - 5.700) < EPS) {
				pipeWidth = 5.600;
			} else if (abs (width - 5.400) < EPS) {
				pipeWidth = 5.300;
			} else if (abs (width - 5.100) < EPS) {
				pipeWidth = 5.000;
			} else if (abs (width - 4.800) < EPS) {
				pipeWidth = 4.700;
			} else if (abs (width - 4.500) < EPS) {
				pipeWidth = 4.400;
			} else if (abs (width - 4.200) < EPS) {
				pipeWidth = 4.100;
			} else if (abs (width - 3.900) < EPS) {
				pipeWidth = 3.800;
			} else if (abs (width - 3.600) < EPS) {
				pipeWidth = 3.500;
			} else if (abs (width - 3.300) < EPS) {
				pipeWidth = 3.200;
			} else if (abs (width - 3.000) < EPS) {
				pipeWidth = 2.900;
			} else if (abs (width - 2.700) < EPS) {
				pipeWidth = 2.600;
			} else if (abs (width - 2.400) < EPS) {
				pipeWidth = 2.300;
			} else if (abs (width - 2.100) < EPS) {
				pipeWidth = 2.000;
			} else if (abs (width - 1.800) < EPS) {
				pipeWidth = 1.700;
			} else if (abs (width - 1.500) < EPS) {
				pipeWidth = 1.400;
			} else {
				pipeWidth = 0.0;
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

// 테이블폼 배치를 위한 질의를 요청하는 다이얼로그 - 가로 방향
short DGCALLBACK wallTableformPlacerHandler2_Horizontal (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGSetDialogTitle (dialogID, "벽에 배치 (테이블폼)");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			// 이전 버튼
			DGSetItemText (dialogID, DG_PREV, "이 전");

			//////////////////////////////////////////////////////////// 아이템 배치 (나머지)
			DGSetItemFont (dialogID, POPUP_TYPE_SELECTOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR, DG_POPUP_BOTTOM, "타입A");
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR, DG_POPUP_BOTTOM, "타입B");

			DGSetItemText (dialogID, LABEL_HEIGHT, "높이");
			DGSetItemText (dialogID, LABEL_WIDTH, "너비");
			DGSetItemText (dialogID, LABEL_ERR_MESSAGE, "높이는 다음 치수만 가능함\n400, 450, 500, 또는 600 ~ 2300 (50 간격)");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "벽과의 간격");

			DGSetItemText (dialogID, LABEL_FILL_SIDE, "양면/단면");
			DGSetItemText (dialogID, RADIOBUTTON_DOUBLE, "양면");
			DGSetItemText (dialogID, RADIOBUTTON_SINGLE, "단면");

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "비계 파이프");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트 세트");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "벽체 타이");
			DGDisableItem (dialogID, LABEL_LAYER_WALLTIE);
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "헤드피스");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "목재");
			DGSetItemText (dialogID, LABEL_LAYER_HIDDEN, "숨김");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

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
			DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE);

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

			ucb.itemID	 = USERCONTROL_LAYER_HIDDEN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN, 1);

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
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "6000");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "5700");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "5400");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "5100");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "4800");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "4500");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "4200");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "3900");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "3600");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "3300");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "3000");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2700");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2400");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2100");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1800");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1500");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "0");
				DGShowItem (dialogID, POPUP_WIDTH [xx]);

				if (placingZone.n6000h > 0) {
					width = 6.000;
					placingZone.n6000h --;
				} else if (placingZone.n5700h > 0) {
					width = 5.700;
					placingZone.n5700h --;
				} else if (placingZone.n5400h > 0) {
					width = 5.400;
					placingZone.n5400h --;
				} else if (placingZone.n5100h > 0) {
					width = 5.100;
					placingZone.n5100h --;
				} else if (placingZone.n4800h > 0) {
					width = 4.800;
					placingZone.n4800h --;
				} else if (placingZone.n4500h > 0) {
					width = 4.500;
					placingZone.n4500h --;
				} else if (placingZone.n4200h > 0) {
					width = 4.200;
					placingZone.n4200h --;
				} else if (placingZone.n3900h > 0) {
					width = 3.900;
					placingZone.n3900h --;
				} else if (placingZone.n3600h > 0) {
					width = 3.600;
					placingZone.n3600h --;
				} else if (placingZone.n3300h > 0) {
					width = 3.300;
					placingZone.n3300h --;
				} else if (placingZone.n3000h > 0) {
					width = 3.000;
					placingZone.n3000h --;
				} else if (placingZone.n2700h > 0) {
					width = 2.700;
					placingZone.n2700h --;
				} else if (placingZone.n2400h > 0) {
					width = 2.400;
					placingZone.n2400h --;
				} else if (placingZone.n2100h > 0) {
					width = 2.100;
					placingZone.n2100h --;
				} else if (placingZone.n1800h > 0) {
					width = 1.800;
					placingZone.n1800h --;
				} else if (placingZone.n1500h > 0) {
					width = 1.500;
					placingZone.n1500h --;
				} else
					width = 0.0;

				// 콤보박스의 값 설정
				if (width > EPS) {
					if (width + EPS > 6.000)		DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 1);
					else if (width + EPS > 5.700)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 2);
					else if (width + EPS > 5.400)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 3);
					else if (width + EPS > 5.100)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 4);
					else if (width + EPS > 4.800)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 5);
					else if (width + EPS > 4.500)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 6);
					else if (width + EPS > 4.200)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 7);
					else if (width + EPS > 3.900)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 8);
					else if (width + EPS > 3.600)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 9);
					else if (width + EPS > 3.300)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 10);
					else if (width + EPS > 3.000)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 11);
					else if (width + EPS > 2.700)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 12);
					else if (width + EPS > 2.400)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 13);
					else if (width + EPS > 2.100)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 14);
					else if (width + EPS > 1.800)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 15);
					else if (width + EPS > 1.500)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 16);
				}
				buttonPosX += 100;
			}

			// 높이 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.verLen);
			DGDisableItem (dialogID, EDITCONTROL_HEIGHT);

			// 높이 값이 받아들일 수 있는 값인가?
			if ( (abs (placingZone.verLen - 0.400) < EPS) || (abs (placingZone.verLen - 0.450) < EPS) || (abs (placingZone.verLen - 0.500) < EPS) || (abs (placingZone.verLen - 0.600) < EPS) ||
				(abs (placingZone.verLen - 0.650) < EPS) || (abs (placingZone.verLen - 0.700) < EPS) || (abs (placingZone.verLen - 0.750) < EPS) || (abs (placingZone.verLen - 0.800) < EPS) ||
				(abs (placingZone.verLen - 0.850) < EPS) || (abs (placingZone.verLen - 0.900) < EPS) || (abs (placingZone.verLen - 0.950) < EPS) || (abs (placingZone.verLen - 1.000) < EPS) ||
				(abs (placingZone.verLen - 1.050) < EPS) || (abs (placingZone.verLen - 1.100) < EPS) || (abs (placingZone.verLen - 1.150) < EPS) || (abs (placingZone.verLen - 1.200) < EPS) ||
				(abs (placingZone.verLen - 1.250) < EPS) || (abs (placingZone.verLen - 1.300) < EPS) || (abs (placingZone.verLen - 1.350) < EPS) || (abs (placingZone.verLen - 1.400) < EPS) ||
				(abs (placingZone.verLen - 1.450) < EPS) || (abs (placingZone.verLen - 1.500) < EPS) || (abs (placingZone.verLen - 1.550) < EPS) || (abs (placingZone.verLen - 1.600) < EPS) ||
				(abs (placingZone.verLen - 1.650) < EPS) || (abs (placingZone.verLen - 1.700) < EPS) || (abs (placingZone.verLen - 1.750) < EPS) || (abs (placingZone.verLen - 1.800) < EPS) ||
				(abs (placingZone.verLen - 1.850) < EPS) || (abs (placingZone.verLen - 1.900) < EPS) || (abs (placingZone.verLen - 1.950) < EPS) || (abs (placingZone.verLen - 2.000) < EPS) ||
				(abs (placingZone.verLen - 2.050) < EPS) || (abs (placingZone.verLen - 2.100) < EPS) || (abs (placingZone.verLen - 2.150) < EPS) || (abs (placingZone.verLen - 2.200) < EPS) ||
				(abs (placingZone.verLen - 2.250) < EPS) || (abs (placingZone.verLen - 2.300) < EPS) ) {

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

			if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR) == 1) {
				// 벽체타이 레이어 비활성화
				DGDisableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE);

				// 레이어 이름 변경
				DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트 세트");
				DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "벽체 타이");
				DGSetItemText (dialogID, LABEL_LAYER_JOIN, "결합철물");

				// 레이어 같이 바뀜
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					switch (item) {
						case USERCONTROL_LAYER_EUROFORM:
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							break;
						case USERCONTROL_LAYER_RECTPIPE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							break;
						case USERCONTROL_LAYER_PINBOLT:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
							break;
						case USERCONTROL_LAYER_WALLTIE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
							break;
						case USERCONTROL_LAYER_JOIN:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							break;
						case USERCONTROL_LAYER_HEADPIECE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							break;
						case USERCONTROL_LAYER_PLYWOOD:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							break;
						case USERCONTROL_LAYER_WOOD:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							break;
					}
				}
			} else if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR) == 2) {
				// 벽체타이 레이어 활성화
				DGEnableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_WALLTIE);

				// 레이어 이름 변경
				DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "각파이프 행거");
				DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "유로폼 후크");
				DGSetItemText (dialogID, LABEL_LAYER_JOIN, "연결철물");

				// 레이어 같이 바뀜
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					switch (item) {
						case USERCONTROL_LAYER_EUROFORM:
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
							break;
						case USERCONTROL_LAYER_RECTPIPE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
							break;
						case USERCONTROL_LAYER_RECTPIPE_HANGER:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
							break;
						case USERCONTROL_LAYER_EUROFORM_HOOK:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
							break;
						case USERCONTROL_LAYER_JOIN:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
							break;
						case USERCONTROL_LAYER_HEADPIECE:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
							break;
						case USERCONTROL_LAYER_PLYWOOD:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
							break;
						case USERCONTROL_LAYER_WOOD:
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD));
							break;
					}
				}
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 셀의 너비/높이 저장
					for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
						placingZone.cells [xx].horLen = atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, POPUP_WIDTH [xx])).ToCStr ()) / 1000.0;
						
						if ((2.300 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.300;
						else if ((2.250 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.250;
						else if ((2.200 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.200;
						else if ((2.150 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.150;
						else if ((2.100 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.100;
						else if ((2.050 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.050;
						else if ((2.000 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.000;
						else if ((1.950 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.950;
						else if ((1.900 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.900;
						else if ((1.850 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.850;
						else if ((1.800 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.800;
						else if ((1.750 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.750;
						else if ((1.700 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.700;
						else if ((1.650 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.650;
						else if ((1.600 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.600;
						else if ((1.550 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.550;
						else if ((1.500 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.500;
						else if ((1.450 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.450;
						else if ((1.400 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.400;
						else if ((1.350 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.350;
						else if ((1.300 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.300;
						else if ((1.250 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.250;
						else if ((1.200 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.200;
						else if ((1.150 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.150;
						else if ((1.100 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.100;
						else if ((1.050 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.050;
						else if ((1.000 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.000;
						else if ((0.950 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.950;
						else if ((0.900 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.900;
						else if ((0.850 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.850;
						else if ((0.800 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.800;
						else if ((0.750 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.750;
						else if ((0.700 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.700;
						else if ((0.650 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.650;
						else if ((0.600 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.600;
						else if ((0.500 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.500;
						else if ((0.450 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.450;
						else if ((0.400 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 0.400;
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

					if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR) == 1) {
						// 타입 지정
						placingZone.type = 1;

						// 레이어 번호 저장
						layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
						layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
						layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
						layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
						layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
						layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
						layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
						layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
						layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);

					} else if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR) == 2) {
						// 타입 지정
						placingZone.type = 2;

						// 레이어 번호 저장
						layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
						layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
						layerInd_RectPipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
						layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
						layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
						layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
						layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
						layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
						layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);
					}
	
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

// 벽 상단의 합판/목재 영역을 유로폼으로 채울지 물어보는 3차 다이얼로그 - 가로 방향
short DGCALLBACK wallTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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

			if (placingZone.marginTop < EPS) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "없음");
			} else if (placingZone.marginTop < 0.110) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "목재");
			} else {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, placingZone.marginTop);

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

// 테이블폼 맞춤 제작을 위한 다이얼로그 (테이블폼 방향, 유로폼 가로/세로 개수 및 길이)
short DGCALLBACK wallTableformPlacerHandler1_Custom (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	char	buffer [256];
	short	itmPosX, itmPosY;
	short	boxSizeX = 120, boxSizeY = 120;
	double	totalWidth, totalHeight;
	short	xx, yy, zz;
	static short	customTableRow = 1;		// 행 현재 개수
	static short	customTableCol = 1;		// 열 현재 개수
	const short		maxRow = 5;			// 행 최대 개수
	const short		maxCol = 5;			// 열 최대 개수
	static short	dialogSizeX;		// 현재 다이얼로그 크기 X
	static short	dialogSizeY;		// 현재 다이얼로그 크기 Y
	bool	bChanged;			// 화면이 변경되는가?
	static short	itmIdx_width [10][10];
	static short	itmIdx_height [10][10];

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "테이블폼 맞춤 제작하기");

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
			// 팝업컨트롤: 타입
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 1, 95, 10, 70, 25);
			DGSetItemFont (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM, "타입A");
			DGPopUpInsertItem (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE_SELECTOR_CUSTOM, DG_POPUP_BOTTOM, "타입B");
			DGShowItem (dialogID, POPUP_TYPE_SELECTOR_CUSTOM);

			// 라벨: 테이블폼 방향
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 20, 80, 23);
			DGSetItemFont (dialogID, LABEL_TABLEFORM_ORIENTATION_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TABLEFORM_ORIENTATION_CUSTOM, "테이블폼 방향");
			DGShowItem (dialogID, LABEL_TABLEFORM_ORIENTATION_CUSTOM);

			// 팝업컨트롤: 테이블폼 방향
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 10, 300, 15, 100, 23);
			DGSetItemFont (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_BOTTOM, "세로방향");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_BOTTOM, "가로방향");
			DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM, DG_POPUP_TOP);
			DGShowItem (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM);

			// 라벨: 총 너비 ("min ~ max 가능")
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 120, 60, 150, 25);
			if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION)
				strcpy (buffer, "총 너비\n(400 ~ 2300 가능)");
			else
				strcpy (buffer, "총 너비\n(1500 ~ 6000 가능)");
			DGSetItemFont (dialogID, LABEL_TOTAL_WIDTH_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH_CUSTOM, buffer);
			DGShowItem (dialogID, LABEL_TOTAL_WIDTH_CUSTOM);
			
			// Edit컨트롤: 총 너비
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 160, 90, 70, 25);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH_CUSTOM);
			DGShowItem (dialogID, EDITCONTROL_TOTAL_WIDTH_CUSTOM);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH_CUSTOM, placingZone.horLen);

			// 라벨: 남은 너비
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 250, 60, 100, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_WIDTH_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_WIDTH_CUSTOM, "남은 너비");
			DGShowItem (dialogID, LABEL_REMAIN_WIDTH_CUSTOM);
			
			// Edit컨트롤: 남은 너비
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 265, 90, 70, 25);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM);

			// 라벨: 총 높이 ("min ~ max 가능")
			// 가로방향, 세로방향에 따라 뒤의 텍스트가 달라짐
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 10, 140, 150, 25);
			if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION)
				strcpy (buffer, "총 높이\n(1500 ~ 6000 가능)");
			else
				strcpy (buffer, "총 높이\n(400 ~ 2300 가능)");
			DGSetItemFont (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM, buffer);
			DGShowItem (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM);

			// Edit컨트롤: 총 높이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 50, 170, 70, 25);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT_CUSTOM);
			DGShowItem (dialogID, EDITCONTROL_TOTAL_HEIGHT_CUSTOM);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT_CUSTOM, placingZone.verLen);

			// 라벨: 남은 높이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 35, 220, 100, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_HEIGHT_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_HEIGHT_CUSTOM, "남은 높이");
			DGShowItem (dialogID, LABEL_REMAIN_HEIGHT_CUSTOM);

			// Edit컨트롤: 남은 높이
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 50, 240, 70, 25);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM);

			// 버튼: 열 추가
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 435, 90, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_COL_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_COL_CUSTOM, "열 추가");
			DGShowItem (dialogID, BUTTON_ADD_COL_CUSTOM);

			// 버튼: 열 삭제
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 360, 90, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_COL_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_COL_CUSTOM, "열 삭제");
			DGShowItem (dialogID, BUTTON_DEL_COL_CUSTOM);

			// 버튼: 행 추가
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 50, 330, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_ROW_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_ROW_CUSTOM, "행 추가");
			DGShowItem (dialogID, BUTTON_ADD_ROW_CUSTOM);

			// 버튼: 행 삭제
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 50, 300, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_ROW_CUSTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_ROW_CUSTOM, "행 삭제");
			DGShowItem (dialogID, BUTTON_DEL_ROW_CUSTOM);

			// 다이얼로그 크기 초기화
			dialogSizeX = 550;
			dialogSizeY = 450;

			// 처음에는 행 개수 2개, 열 개수를 2개로 시작함
			customTableRow = 2;
			customTableCol = 2;

			// 여기부터는 itmIdx의 개수가 동적: REST_ITEM_START_CUSTOM부터
			itmPosX = 170;
			itmPosY = 150 + (customTableRow-1) * boxSizeY;

			for (xx = 0 ; xx < customTableRow ; ++xx) {
				for (yy = 0 ; yy < customTableCol ; ++yy) {
					// 구분자
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, boxSizeX, boxSizeY);
					DGShowItem (dialogID, itmIdx);

					// 팝업컨트롤: 너비
					itmIdx_width [xx][yy] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, itmPosX + boxSizeX/2 - 30, itmPosY + boxSizeY - 30, 60, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGPopUpDisableDraw (dialogID, itmIdx);
					if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
					} else {
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
					}
					DGPopUpEnableDraw (dialogID, itmIdx);
					DGShowItem (dialogID, itmIdx);
			
					// 팝업컨트롤: 높이
					itmIdx_height [xx][yy] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, itmPosX + 5, itmPosY + boxSizeY/2 - 15, 60, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGPopUpDisableDraw (dialogID, itmIdx);
					if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
					} else {
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
						DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
					}
					DGPopUpEnableDraw (dialogID, itmIdx);
					DGShowItem (dialogID, itmIdx);

					itmPosX += boxSizeX;
				}
				itmPosX = 170;
				itmPosY -= boxSizeY;
			}

			// 남은 너비 표시
			totalWidth = 0.0;
			for (xx = 0 ; xx < customTableCol ; ++xx) {
				totalWidth += atof (DGPopUpGetItemText (dialogID, itmIdx_width [0][xx], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_width [0][xx]))).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM, placingZone.horLen - totalWidth);

			// 남은 높이 표시
			totalHeight = 0.0;
			for (xx = 0 ; xx < customTableRow ; ++xx) {
				totalHeight += atof (DGPopUpGetItemText (dialogID, itmIdx_height [xx][0], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_height [xx][0]))).ToCStr ().Get ()) / 1000;
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM, placingZone.verLen - totalHeight);
			
			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case POPUP_TABLEFORM_ORIENTATION_CUSTOM:
					// 테이블폼 방향이 바뀌면?
					if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
						DGSetItemText (dialogID, LABEL_TOTAL_WIDTH_CUSTOM, "총 너비\n(400 ~ 2300 가능)");
						DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM, "총 높이\n(1500 ~ 6000 가능)");

						// 테이블폼 너비/높이 팝업컨트롤의 값이 바뀌어야 함
						for (xx = 0 ; xx <= (customTableCol * customTableRow) ; ++xx) {
							// 팝업컨트롤: 너비
							itmIdx = REST_ITEM_START_CUSTOM + xx*3 + 1;
							while (DGPopUpGetItemCount (dialogID, itmIdx) > 0)
							DGPopUpDeleteItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
			
							// 팝업컨트롤: 높이
							itmIdx = REST_ITEM_START_CUSTOM + xx*3 + 2;
							while (DGPopUpGetItemCount (dialogID, itmIdx) > 0)
								DGPopUpDeleteItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
						}
					} else {
						DGSetItemText (dialogID, LABEL_TOTAL_WIDTH_CUSTOM, "총 너비\n(1500 ~ 6000 가능)");
						DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT_CUSTOM, "총 높이\n(400 ~ 2300 가능)");

						// 테이블폼 너비/높이 팝업컨트롤의 값이 바뀌어야 함
						for (xx = 0 ; xx <= (customTableCol * customTableRow) ; ++xx) {
							// 팝업컨트롤: 너비
							itmIdx = REST_ITEM_START_CUSTOM + xx*3 + 1;
							while (DGPopUpGetItemCount (dialogID, itmIdx) > 0)
								DGPopUpDeleteItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
			
							// 팝업컨트롤: 높이
							itmIdx = REST_ITEM_START_CUSTOM + xx*3 + 2;
							while (DGPopUpGetItemCount (dialogID, itmIdx) > 0)
								DGPopUpDeleteItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
						}
					}

					// 남은 너비 표시
					totalWidth = 0.0;
					for (xx = 0 ; xx < customTableCol ; ++xx) {
						totalWidth += atof (DGPopUpGetItemText (dialogID, itmIdx_width [0][xx], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_width [0][xx]))).ToCStr ().Get ()) / 1000;
					}
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM, placingZone.horLen - totalWidth);

					// 남은 높이 표시
					totalHeight = 0.0;
					for (xx = 0 ; xx < customTableRow ; ++xx) {
						totalHeight += atof (DGPopUpGetItemText (dialogID, itmIdx_height [xx][0], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_height [xx][0]))).ToCStr ().Get ()) / 1000;
					}
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM, placingZone.verLen - totalHeight);

					break;

				default:
					for (xx = 0 ; xx < customTableRow ; ++xx) {
						for (yy = 0 ; yy < customTableCol ; ++yy) {
							if (item == itmIdx_width [xx][yy]) {
								// 어떤 유로폼의 너비를 변경하면, 같은 열의 모든 행 유로폼 너비도 같이 변경됨
								for (zz = 0 ; zz < customTableRow ; ++zz) {
									if (xx != zz) {
										DGPopUpSelectItem (dialogID, itmIdx_width [zz][yy], DGPopUpGetSelected (dialogID, itmIdx_width [xx][yy]));
									}
								}
							}

							if (item == itmIdx_height [xx][yy]) {
								// 어떤 유로폼의 높이를 변경하면, 같은 행의 모든 열 유로폼 너비도 같이 변경됨
								for (zz = 0 ; zz < customTableCol ; ++zz) {
									if (yy != zz) {
										DGPopUpSelectItem (dialogID, itmIdx_height [xx][zz], DGPopUpGetSelected (dialogID, itmIdx_height [xx][yy]));
									}
								}
							}
						}
					}

					// 남은 너비 표시
					totalWidth = 0.0;
					for (xx = 0 ; xx < customTableCol ; ++xx) {
						totalWidth += atof (DGPopUpGetItemText (dialogID, itmIdx_width [0][xx], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_width [0][xx]))).ToCStr ().Get ()) / 1000;
					}
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM, placingZone.horLen - totalWidth);

					// 남은 높이 표시
					totalHeight = 0.0;
					for (xx = 0 ; xx < customTableRow ; ++xx) {
						totalHeight += atof (DGPopUpGetItemText (dialogID, itmIdx_height [xx][0], static_cast<short>(DGGetItemValLong (dialogID, itmIdx_height [xx][0]))).ToCStr ().Get ()) / 1000;
					}
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM, placingZone.verLen - totalHeight);

					break;
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR_CUSTOM) == 1) {
						// 타입 지정
						placingZone.type = 1;

					} else if (DGPopUpGetSelected (dialogID, POPUP_TYPE_SELECTOR_CUSTOM) == 2) {
						// 타입 지정
						placingZone.type = 2;

					}

					// ... 세로방향이면?
					// ... 가로방향이면?

					break;

				case DG_CANCEL:
					break;

				case BUTTON_ADD_COL_CUSTOM:
				case BUTTON_DEL_COL_CUSTOM:
				case BUTTON_ADD_ROW_CUSTOM:
				case BUTTON_DEL_ROW_CUSTOM:
					bChanged = false;

					if (item == BUTTON_ADD_COL_CUSTOM) {
						if (customTableCol < maxCol) {
							customTableCol ++;
							bChanged = true;
						}
					}
					if (item == BUTTON_DEL_COL_CUSTOM) {
						if (customTableCol >= 2) {
							customTableCol --;
							bChanged = true;
						}
					}
					if (item == BUTTON_ADD_ROW_CUSTOM) {
						if (customTableRow < maxRow) {
							customTableRow ++;
							bChanged = true;
						}
					}
					if (item == BUTTON_DEL_ROW_CUSTOM) {
						if (customTableRow >= 2) {
							customTableRow --;
							bChanged = true;
						}
					}

					item = 0;

					if (bChanged == true) {
						DGRemoveDialogItems (dialogID, REST_ITEM_START_CUSTOM);

						// 테이블폼 관련 항목 재배치
						itmPosX = 170;
						itmPosY = 150 + (customTableRow-1) * boxSizeY;

						for (xx = 0 ; xx < customTableRow ; ++xx) {
							for (yy = 0 ; yy < customTableCol ; ++yy) {
								// 구분자
								itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, boxSizeX, boxSizeY);
								DGShowItem (dialogID, itmIdx);

								// 팝업컨트롤: 너비
								itmIdx_width [xx][yy] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, itmPosX + boxSizeX/2 - 30, itmPosY + boxSizeY - 30, 60, 23);
								DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
								//DGPopUpDisableDraw (dialogID, itmIdx);
								if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
								} else {
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
								}
								//DGPopUpEnableDraw (dialogID, itmIdx);
								DGShowItem (dialogID, itmIdx);
			
								// 팝업컨트롤: 높이
								itmIdx_height [xx][yy] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, itmPosX + 5, itmPosY + boxSizeY/2 - 15, 60, 23);
								DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
								//DGPopUpDisableDraw (dialogID, itmIdx);
								if (DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_ORIENTATION_CUSTOM) == VERTICAL_DIRECTION) {
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
								} else {
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "600");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "500");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "450");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "400");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "300");
									DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
									DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "200");
								}
								//DGPopUpEnableDraw (dialogID, itmIdx);
								DGShowItem (dialogID, itmIdx);

								itmPosX += boxSizeX;
							}
							itmPosX = 170;
							itmPosY -= boxSizeY;
						}

						// 남은 너비 표시
						totalWidth = 0.0;
						for (xx = 0 ; xx < customTableCol ; ++xx) {
							totalWidth += atof (DGPopUpGetItemText (dialogID, REST_ITEM_START_CUSTOM + xx*3 + 1, static_cast<short>(DGGetItemValLong (dialogID, REST_ITEM_START_CUSTOM + xx*3 + 1))).ToCStr ().Get ()) / 1000;
						}
						DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH_CUSTOM, placingZone.horLen - totalWidth);

						// 남은 높이 표시
						totalHeight = 0.0;
						for (xx = 0 ; xx < customTableRow ; ++xx) {
							totalHeight += atof (DGPopUpGetItemText (dialogID, REST_ITEM_START_CUSTOM + customTableCol*3*xx + 2, static_cast<short>(DGGetItemValLong (dialogID, REST_ITEM_START_CUSTOM + customTableCol*3*xx + 2))).ToCStr ().Get ()) / 1000;
						}
						DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT_CUSTOM, placingZone.verLen - totalHeight);

						// 다이얼로그 크기 변경
						dialogSizeX = 550;
						if (customTableCol > 2)	dialogSizeX += boxSizeX * (customTableCol-2);
						dialogSizeY = 450;
						if (customTableRow > 2)	dialogSizeY += boxSizeY * (customTableRow-2);
						DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
					}

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

// 배치: 유로폼
API_Guid	WallTableformPlacingZone::placeUFOM (Euroform params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("유로폼v2.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				tempStr [20];
	
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

	setParameterByName (&memo, "eu_stan_onoff", 1.0);			// 규격품

	// 너비
	sprintf (tempStr, "%.0f", params.width * 1000);
	setParameterByName (&memo, "eu_wid", tempStr);

	// 높이
	sprintf (tempStr, "%.0f", params.height * 1000);
	setParameterByName (&memo, "eu_hei", tempStr);

	// 설치방향
	if (params.u_ins_wall == true)
		setParameterByName (&memo, "u_ins", "벽세우기");
	else {
		setParameterByName (&memo, "u_ins", "벽눕히기");
		moveIn2D ('x', params.ang, params.height, &elem.object.pos.x, &elem.object.pos.y);
	}
	setParameterByName (&memo, "ang_x", DegreeToRad (90.0));	// 회전X

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

	const GS::uchar_t*	gsmName = L("유로폼v2.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				tempStr [20];
	
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
		setParameterByName (&memo, "eu_stan_onoff", 1.0);		// 규격품

		// 너비
		sprintf (tempStr, "%.0f", params.width * 1000);
		setParameterByName (&memo, "eu_wid", tempStr);

		// 높이
		sprintf (tempStr, "%.0f", params.height * 1000);
		setParameterByName (&memo, "eu_hei", tempStr);
	} else {
		setParameterByName (&memo, "eu_stan_onoff", 0.0);		// 비규격품
		setParameterByName (&memo, "eu_wid2", params.width);	// 너비
		setParameterByName (&memo, "eu_hei2", params.height);	// 높이
	}

	// 설치방향
	if (params.u_ins_wall == true)
		setParameterByName (&memo, "u_ins", "벽세우기");
	else {
		setParameterByName (&memo, "u_ins", "벽눕히기");
		moveIn2D ('x', params.ang, params.height, &elem.object.pos.x, &elem.object.pos.y);
	}
	setParameterByName (&memo, "ang_x", DegreeToRad (90.0));	// 회전X

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

	const GS::uchar_t*	gsmName = L("비계파이프v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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

	setParameterByName (&memo, "p_comp", "사각파이프");		// 사각파이프
	setParameterByName (&memo, "p_leng", params.length);	// 길이
	setParameterByName (&memo, "p_ang", params.pipeAng);	// 각도

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

	const GS::uchar_t*	gsmName = L("핀볼트세트v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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
		setParameterByName (&memo, "bRotated", 1.0);
	else
		setParameterByName (&memo, "bRotated", 0.0);

	setParameterByName (&memo, "bolt_len", params.boltLen);		// 볼트 길이
	setParameterByName (&memo, "bolt_dia", 0.010);				// 볼트 직경
	setParameterByName (&memo, "washer_pos", 0.050);			// 와셔 위치
	setParameterByName (&memo, "washer_size", 0.100);			// 와셔 크기
	setParameterByName (&memo, "angX", params.angX);			// X축 회전
	setParameterByName (&memo, "angY", params.angY);			// Y축 회전

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

	const GS::uchar_t*	gsmName = L("벽체 타이 v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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

	setParameterByName (&memo, "boltLen", params.boltLen);		// 볼트 길이 (벽 두께 + 327mm 초과이며 100 단위로 나눠지는 가장 작은 수)
	setParameterByName (&memo, "boltDia", 0.012);				// 볼트 직경
	setParameterByName (&memo, "bSqrWasher", 1.0);				// 사각와샤
	setParameterByName (&memo, "washer_size", 0.100);			// 사각와샤 크기
	setParameterByName (&memo, "nutType", "타입 1");			// 너트 타입
	setParameterByName (&memo, "bEmbedPipe", 1.0);				// 벽체 내장 파이프
	setParameterByName (&memo, "pipeInnerDia", 0.012);			// 파이프 내경
	setParameterByName (&memo, "pipeThk", 0.002);				// 파이프 두께
	
	// 파이프 시작점, 끝점 (벽 두께만큼 차이)
	setParameterByName (&memo, "pipeBeginPos", params.pipeBeg);
	setParameterByName (&memo, "pipeEndPos", params.pipeEnd);
	
	// 좌,우측 조임쇠 위치 (벽 두께 + 327mm 만큼 차이)
	setParameterByName (&memo, "posLClamp", params.clampBeg);
	setParameterByName (&memo, "posRClamp", params.clampEnd);
	
	setParameterByName (&memo, "angY", DegreeToRad (0.0));		// 회전 Y

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

	const GS::uchar_t*	gsmName = L("직교클램프v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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

	setParameterByName (&memo, "angX", params.angX);		// 본체 회전 (X)
	setParameterByName (&memo, "body_ang", params.angY);	// 본체 회전 (Y)
	setParameterByName (&memo, "anchor_bolt_ang", 0.018);	// 고정볼트 조이기

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 헤드피스 (세로 방향: 타입 A)
API_Guid	WallTableformPlacingZone::placePUSH (HeadpieceOfPushPullProps params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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

	setParameterByName (&memo, "type", "타입 A");			// 타입
	setParameterByName (&memo, "plateThk", 0.009);			// 철판 두께
	setParameterByName (&memo, "angX", DegreeToRad (0.0));	// 회전X
	setParameterByName (&memo, "angY", DegreeToRad (0.0));	// 회전Y

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 헤드피스 (가로 방향: 타입 B)
API_Guid	WallTableformPlacingZone::placePUSH_hor (HeadpieceOfPushPullProps params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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

	setParameterByName (&memo, "type", "타입 B");			// 타입
	setParameterByName (&memo, "plateThk", 0.009);			// 철판 두께
	setParameterByName (&memo, "angX", DegreeToRad (0.0));	// 회전X
	setParameterByName (&memo, "angY", DegreeToRad (90.0));	// 회전Y
	elem.object.level += 0.200;

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

	const GS::uchar_t*	gsmName = L("결합철물 (사각와셔활용) v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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

	setParameterByName (&memo, "washer_pos2", 0.108);		// 와셔2 위치
	setParameterByName (&memo, "angX", DegreeToRad (0.0));	// 회전X
	setParameterByName (&memo, "angY", DegreeToRad (0.0));	// 회전Y

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

	const GS::uchar_t*	gsmName = L("합판v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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

	setParameterByName (&memo, "p_stan", "비규격");			// 규격
	setParameterByName (&memo, "p_wid", params.p_wid);		// 가로
	setParameterByName (&memo, "p_leng", params.p_leng);	// 세로
		
	// 설치방향
	if (params.w_dir_wall == true)
		setParameterByName (&memo, "w_dir", "벽세우기");
	else
		setParameterByName (&memo, "w_dir", "벽눕히기");

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

	const GS::uchar_t*	gsmName = L("목재v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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

	setParameterByName (&memo, "w_ins", "벽세우기");		// 설치방향
	setParameterByName (&memo, "w_w", params.w_w);			// 두께
	setParameterByName (&memo, "w_h", params.w_h);			// 너비
	setParameterByName (&memo, "w_leng", params.w_leng);	// 길이
	setParameterByName (&memo, "w_ang", params.w_ang);		// 각도

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

// 배치: 사각파이프 연결철물
API_Guid	WallTableformPlacingZone::placeJOIN2 (MetalFittings params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("사각파이프 연결철물 v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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
	elem.header.layer = layerInd_Join;

	setParameterByName (&memo, "angX", params.angX);	// 회전X
	setParameterByName (&memo, "angY", params.angY);	// 회전Y

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 빔조인트용 Push-Pull Props
API_Guid	WallTableformPlacingZone::placePUSH2 (HeadpieceOfPushPullProps params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 유로폼 후크
API_Guid	WallTableformPlacingZone::placeHOOK (EuroformHook params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("유로폼 후크.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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
	elem.object.angle = params.ang + DegreeToRad (180.0);
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_EuroformHook;

	setParameterByName (&memo, "rotationX", params.angX);			// X축 회전
	setParameterByName (&memo, "rotationY", params.angY);			// Y축 회전
	setParameterByName (&memo, "iHookType", params.iiHookType);		// (1)수직-대, (2)수평-소
	setParameterByName (&memo, "iHookShape", params.iHookShape);	// (1)원형, (2)사각

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 배치: 각파이프 행거
API_Guid	WallTableformPlacingZone::placeHANG (RectPipeHanger params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("각파이프행거.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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
	elem.object.angle = params.ang - DegreeToRad (90);
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_RectPipeHanger;

	setParameterByName (&memo, "m_type", "각파이프행거");	// 품명
	setParameterByName (&memo, "angX", params.angX);		// 회전X
	setParameterByName (&memo, "angY", params.angY);		// 회전Y

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 타공을 위한 기둥 객체를 배치하고 숨김, "원통 19" 객체를 이용함
API_Guid	WallTableformPlacingZone::placeHOLE (API_Guid guid_Target, Cylinder operator_Object)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("원통 19.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

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
	elem.object.pos.x = operator_Object.leftBottomX;
	elem.object.pos.y = operator_Object.leftBottomY;
	elem.object.level = operator_Object.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = operator_Object.ang;
	elem.header.floorInd = infoWall.floorInd;

	// 레이어
	elem.header.layer = layerInd_Hidden;

	// 편집 모드는 "각도-길이" 고정
	setParameterByName (&memo, "edit_mode", "각도-길이");
	setParameterByName (&memo, "end_mode", "직각");
	setParameterByName (&memo, "gamma", operator_Object.angleFromPlane);
	setParameterByName (&memo, "length", operator_Object.length);
	setParameterByName (&memo, "radius_1", operator_Object.radius);

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// guid_Target 타공하기
	ACAPI_Element_SolidLink_Create (guid_Target, elem.header.guid, APISolid_Substract, 0);

	return	elem.header.guid;
}