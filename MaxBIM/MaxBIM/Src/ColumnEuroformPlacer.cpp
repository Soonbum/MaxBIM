#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "ColumnEuroformPlacer.hpp"

using namespace columnPlacerDG;

static ColumnPlacingZone	placingZone;			// 기본 기둥 영역 정보
static InfoColumn			infoColumn;				// 기둥 객체 정보
static InfoWallForColumn	infoWall;				// 간섭 벽 정보
static short				nInterfereBeams;		// 간섭 보 개수
static InfoBeamForColumn	infoOtherBeams [10];	// 간섭 보 정보
static short			layerInd_Euroform;			// 레이어 번호: 유로폼
static short			layerInd_Incorner;			// 레이어 번호: 인코너판넬
static short			layerInd_Outcorner;			// 레이어 번호: 아웃코너판넬
static short			layerInd_Plywood;			// 레이어 번호: 합판
static short			layerInd_MagicBar;			// 레이어 번호: 매직바
static short			layerInd_MagicIncorner;		// 레이어 번호: 매직인코너
static short			clickedBtnItemIdx;			// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
static bool				clickedOKButton;			// OK 버튼을 눌렀습니까?
static bool				clickedPrevButton;			// 이전 버튼을 눌렀습니까?
static GS::Array<API_Guid>	elemList;				// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함


// 5번 메뉴: 기둥에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnColumn (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx, yy;
	double			dx, dy, ang;
	short			result;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	columns = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	walls = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nMorphs = 0;
	long					nColumns = 0;
	long					nWalls = 0;
	long					nBeams = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 객체 정보
	InfoMorphForColumn		infoMorph;

	// 작업 층 정보
	API_StoryInfo			storyInfo;
	double					workLevel_column;


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 기둥 (1개), 기둥 측면을 덮는 모프 (1개)\n옵션 선택: 기둥과 맞닿거나 간섭하는 벽(1개), 기둥과 맞닿는 보 (다수)", true);
		return err;
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 메인 기둥 1개 선택해야 함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_ColumnID)	// 기둥인가?
				columns.Push (tElem.header.guid);

			if (tElem.header.typeID == API_MorphID)		// 모프인가?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_WallID)		// 벽인가?
				walls.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// 보인가?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nColumns = columns.GetSize ();
	nMorphs = morphs.GetSize ();
	nWalls = walls.GetSize ();
	nBeams = beams.GetSize ();

	// 기둥이 1개인가?
	if (nColumns != 1) {
		ACAPI_WriteReport ("기둥을 1개 선택해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 벽이 1개 이하인가?
	if (nWalls > 1) {
		ACAPI_WriteReport ("기둥에 간섭하는 벽은 1개만 인식 가능합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("기둥 측면을 덮는 모프를 1개 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 기둥 정보 저장
	infoColumn.guid = columns.Pop ();

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = infoColumn.guid;
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
	
	infoColumn.floorInd		= elem.header.floorInd;			// 층 인덱스
	infoColumn.bRectangle	= !elem.column.circleBased;		// 직사각형인가?
	infoColumn.coreAnchor	= elem.column.coreAnchor;		// 코어의 앵커 포인트
	infoColumn.coreWidth	= elem.column.coreWidth;		// 코어의 너비 (X 길이)
	infoColumn.coreDepth	= elem.column.coreDepth;		// 코어의 깊이 (Y 길이)
	infoColumn.venThick		= elem.column.venThick;			// 베니어 두께
	infoColumn.height		= elem.column.height;			// 기둥 높이
	infoColumn.bottomOffset	= elem.column.bottomOffset;		// 바닥 레벨에 대한 기둥 베이스 레벨
	infoColumn.topOffset	= elem.column.topOffset;		// 만약 기둥이 윗층과 연결되어 있는 경우 윗층으로부터의 오프셋
	infoColumn.angle		= elem.column.angle + elem.column.slantDirectionAngle;	// 기둥 축을 중심으로 한 회전 각도 (단위: Radian)
	infoColumn.origoPos		= elem.column.origoPos;			// 기둥 중심 위치

	ACAPI_DisposeElemMemoHdls (&memo);

	// 벽 정보 저장
	if (nWalls != 0) {
		infoWall.guid = walls.Pop ();

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = infoWall.guid;
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		infoWall.wallThk		= elem.wall.thickness;		// 벽 두께
		infoWall.floorInd		= elem.header.floorInd;		// 층 인덱스
		infoWall.bottomOffset	= elem.wall.bottomOffset;	// 벽 하단 오프셋
		infoWall.offset			= elem.wall.offset;			// 시작점에서 레퍼런스 라인으로부터 벽의 기초 라인의 오프셋
		infoWall.angle			= elem.wall.angle;			// 회전 각도 (단위: Radian)

		infoWall.offsetFromOutside		= elem.wall.offsetFromOutside;		// 벽의 레퍼런스 라인과 벽의 바깥쪽 면 간의 거리
		infoWall.referenceLineLocation	= elem.wall.referenceLineLocation;	// 레퍼런스 라인의 위치

		infoWall.begX			= elem.wall.begC.x;			// 시작점 X
		infoWall.begY			= elem.wall.begC.y;			// 시작점 Y
		infoWall.endX			= elem.wall.endC.x;			// 끝점 X
		infoWall.endY			= elem.wall.endC.y;			// 끝점 Y

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 보 정보 저장
	nInterfereBeams = nBeams;

	for (xx = 0 ; xx < nInterfereBeams ; ++xx) {
		infoOtherBeams [xx].guid = beams.Pop ();

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = infoOtherBeams [xx].guid;
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		infoOtherBeams [xx].floorInd	= elem.header.floorInd;		// 층 인덱스
		infoOtherBeams [xx].height		= elem.beam.height;			// 보 높이
		infoOtherBeams [xx].width		= elem.beam.width;			// 보 너비
		infoOtherBeams [xx].offset		= elem.beam.offset;			// 보 중심으로부터 보의 레퍼런스 라인의 오프셋입니다.
		infoOtherBeams [xx].level		= elem.beam.level;			// 바닥 레벨에 대한 보의 위쪽면 높이입니다.
		infoOtherBeams [xx].begC		= elem.beam.begC;			// 보 시작 좌표
		infoOtherBeams [xx].endC		= elem.beam.endC;			// 보 끝 좌표

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 모프 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// 모프의 정보 저장
	infoMorph.guid		= elem.header.guid;
	infoMorph.floorInd	= elem.header.floorInd;
	infoMorph.level		= info3D.bounds.zMin;
	infoMorph.height	= info3D.bounds.zMax - info3D.bounds.zMin;

	// 영역 모프 제거
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_column = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoColumn.floorInd) {
			workLevel_column = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 영역 정보의 고도 정보 수정 - 불필요

	// 영역 정보 저장
	placingZone.bRectangle		= infoColumn.bRectangle;	// 직사각형인가?
	placingZone.coreAnchor		= infoColumn.coreAnchor;	// 코어의 앵커 포인트
	placingZone.coreWidth		= infoColumn.coreWidth;		// 코어의 너비 (X 길이)
	placingZone.coreDepth		= infoColumn.coreDepth;		// 코어의 깊이 (Y 길이)
	placingZone.venThick		= infoColumn.venThick;		// 베니어 두께
	placingZone.height			= infoColumn.height;		// 기둥 높이
	placingZone.bottomOffset	= infoColumn.bottomOffset;	// 바닥 레벨에 대한 기둥 베이스 레벨
	placingZone.topOffset		= infoColumn.topOffset;		// 만약 기둥이 윗층과 연결되어 있는 경우 윗층으로부터의 오프셋
	placingZone.angle			= infoColumn.angle;			// 기둥 축을 중심으로 한 회전 각도 (단위: Radian)
	placingZone.origoPos		= infoColumn.origoPos;		// 기둥 중심 위치

	placingZone.areaHeight		= infoMorph.height;			// 영역 높이

	// placingZone의 Cell 정보 초기화
	initCellsForColumn (&placingZone);

	// 단독 기둥의 경우
	if (nWalls == 0) {

		API_Coord	rotatedPoint;
		double		lineLen;

		lineLen = sqrt (pow (infoColumn.coreWidth/2, 2) + pow (infoColumn.coreDepth/2, 2));
		rotatedPoint.x = infoColumn.origoPos.x + lineLen * cos(atan2 (infoColumn.coreDepth/2, infoColumn.coreWidth/2) + infoColumn.angle);
		rotatedPoint.y = infoColumn.origoPos.y + lineLen * sin(atan2 (infoColumn.coreDepth/2, infoColumn.coreWidth/2) + infoColumn.angle);
		placeCoordinateLabel (rotatedPoint.x, rotatedPoint.y, infoColumn.bottomOffset, true, "RT", 1, infoColumn.floorInd);

		lineLen = sqrt (pow (infoColumn.coreWidth/2, 2) + pow (infoColumn.coreDepth/2, 2));
		rotatedPoint.x = infoColumn.origoPos.x + lineLen * cos(atan2 (infoColumn.coreDepth/2, -infoColumn.coreWidth/2) + infoColumn.angle);
		rotatedPoint.y = infoColumn.origoPos.y + lineLen * sin(atan2 (infoColumn.coreDepth/2, -infoColumn.coreWidth/2) + infoColumn.angle);
		placeCoordinateLabel (rotatedPoint.x, rotatedPoint.y, infoColumn.bottomOffset, true, "LT", 1, infoColumn.floorInd);

		lineLen = sqrt (pow (infoColumn.coreWidth/2, 2) + pow (infoColumn.coreDepth/2, 2));
		rotatedPoint.x = infoColumn.origoPos.x + lineLen * cos(atan2 (-infoColumn.coreDepth/2, -infoColumn.coreWidth/2) + infoColumn.angle);
		rotatedPoint.y = infoColumn.origoPos.y + lineLen * sin(atan2 (-infoColumn.coreDepth/2, -infoColumn.coreWidth/2) + infoColumn.angle);
		placeCoordinateLabel (rotatedPoint.x, rotatedPoint.y, infoColumn.bottomOffset, true, "LB", 1, infoColumn.floorInd);

		lineLen = sqrt (pow (infoColumn.coreWidth/2, 2) + pow (infoColumn.coreDepth/2, 2));
		rotatedPoint.x = infoColumn.origoPos.x + lineLen * cos(atan2 (-infoColumn.coreDepth/2, infoColumn.coreWidth/2) + infoColumn.angle);
		rotatedPoint.y = infoColumn.origoPos.y + lineLen * sin(atan2 (-infoColumn.coreDepth/2, infoColumn.coreWidth/2) + infoColumn.angle);
		placeCoordinateLabel (rotatedPoint.x, rotatedPoint.y, infoColumn.bottomOffset, true, "RB", 1, infoColumn.floorInd);

FIRST_SOLE_COLUMN:

		// 영역 정보 중 간섭 보 관련 정보 업데이트
		// !!!
		//- 메인 기둥과 보들과의 관계를 파악
		//	- 모든 보의 고도를 찾는다.
		//	- 그 중 제일 낮은 보의 고도를 찾아야 함

		// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32531, ACAPI_GetOwnResModule (), columnPlacerHandler1, 0);

	// 벽체와 맞닿거나 벽체 속 기둥인 경우
	} else {

		// ...
		// 벽체 속 기둥의 경우
		// 1. 먼저 기둥의 중심을 찾고, 회전되지 않았을 때의 기둥의 각 꼭지점을 찾는다. (회전각도에 따라 꼭지점이 어디에 있는지 확인해 볼 것)
		// 2. 벽면 안쪽/바깥쪽 라인 2개의 각각의 시작/끝점을 찾고.. 기둥의 중심과 벽면 라인 간의 거리를 측정한다. (거리가 기둥의 너비/2 또는 깊이/2 이하이면 붙거나 간섭)
		// 3. 벽의 위치를 구하는 법
		//		- 기둥 중심을 기준으로 벽의 두 점이 모두 Y값이 크고, X값이 하나는 작고 다른 하나는 큼 -> 벽이 북쪽에 있음
		//		- 기둥 중심을 기준으로 벽의 두 점이 모두 Y값이 작고, X값이 하나는 작고 다른 하나는 큼 -> 벽이 남쪽에 있음
		//		- 기둥 중심을 기준으로 벽의 두 점이 모두 X값이 크고, Y값이 하나는 작고 다른 하나는 큼 -> 벽이 동쪽에 있음
		//		- 기둥 중심을 기준으로 벽의 두 점이 모두 X값이 작고, Y값이 하나는 작고 다른 하나는 큼 -> 벽이 서쪽에 있음
		// 4. 벽의 침범 거리
		//		- ???
	}

	return	err;
}

// Cell 배열을 초기화함
void	initCellsForColumn (ColumnPlacingZone* placingZone)
{
	short	xx;

	// 기둥 위쪽 여백 초기화
	placingZone->marginTopAtNorth = 0.0;
	placingZone->marginTopAtWest = 0.0;
	placingZone->marginTopAtEast = 0.0;
	placingZone->marginTopAtSouth = 0.0;

	// 기둥 위쪽 여백은 기본적으로 채움으로 설정
	placingZone->bFillMarginTopAtNorth = true;
	placingZone->bFillMarginTopAtWest = true;
	placingZone->bFillMarginTopAtEast = true;
	placingZone->bFillMarginTopAtSouth = true;

	// 셀 정보 초기화
	for (xx = 0 ; xx < 20 ; ++xx) {
		placingZone->cellsLT [xx].objType = NONE;
		placingZone->cellsLT [xx].leftBottomX = 0.0;
		placingZone->cellsLT [xx].leftBottomY = 0.0;
		placingZone->cellsLT [xx].leftBottomZ = 0.0;
		placingZone->cellsLT [xx].ang = 0.0;
		placingZone->cellsLT [xx].horLen = 0.0;
		placingZone->cellsLT [xx].verLen = 0.0;
		placingZone->cellsLT [xx].height = 0.0;
		placingZone->cellsLT [xx].anchor = LEFT_TOP;

		placingZone->cellsRT [xx].objType = NONE;
		placingZone->cellsRT [xx].leftBottomX = 0.0;
		placingZone->cellsRT [xx].leftBottomY = 0.0;
		placingZone->cellsRT [xx].leftBottomZ = 0.0;
		placingZone->cellsRT [xx].ang = 0.0;
		placingZone->cellsRT [xx].horLen = 0.0;
		placingZone->cellsRT [xx].verLen = 0.0;
		placingZone->cellsRT [xx].height = 0.0;
		placingZone->cellsRT [xx].anchor = RIGHT_TOP;

		placingZone->cellsLB [xx].objType = NONE;
		placingZone->cellsLB [xx].leftBottomX = 0.0;
		placingZone->cellsLB [xx].leftBottomY = 0.0;
		placingZone->cellsLB [xx].leftBottomZ = 0.0;
		placingZone->cellsLB [xx].ang = 0.0;
		placingZone->cellsLB [xx].horLen = 0.0;
		placingZone->cellsLB [xx].verLen = 0.0;
		placingZone->cellsLB [xx].height = 0.0;
		placingZone->cellsLB [xx].anchor = LEFT_BOTTOM;

		placingZone->cellsRB [xx].objType = NONE;
		placingZone->cellsRB [xx].leftBottomX = 0.0;
		placingZone->cellsRB [xx].leftBottomY = 0.0;
		placingZone->cellsRB [xx].leftBottomZ = 0.0;
		placingZone->cellsRB [xx].ang = 0.0;
		placingZone->cellsRB [xx].horLen = 0.0;
		placingZone->cellsRB [xx].verLen = 0.0;
		placingZone->cellsRB [xx].height = 0.0;
		placingZone->cellsRB [xx].anchor = RIGHT_BOTTOM;

		placingZone->cellsT1 [xx].objType = NONE;
		placingZone->cellsT1 [xx].leftBottomX = 0.0;
		placingZone->cellsT1 [xx].leftBottomY = 0.0;
		placingZone->cellsT1 [xx].leftBottomZ = 0.0;
		placingZone->cellsT1 [xx].ang = 0.0;
		placingZone->cellsT1 [xx].horLen = 0.0;
		placingZone->cellsT1 [xx].verLen = 0.0;
		placingZone->cellsT1 [xx].height = 0.0;
		placingZone->cellsT1 [xx].anchor = LEFT_TOP;

		placingZone->cellsT2 [xx].objType = NONE;
		placingZone->cellsT2 [xx].leftBottomX = 0.0;
		placingZone->cellsT2 [xx].leftBottomY = 0.0;
		placingZone->cellsT2 [xx].leftBottomZ = 0.0;
		placingZone->cellsT2 [xx].ang = 0.0;
		placingZone->cellsT2 [xx].horLen = 0.0;
		placingZone->cellsT2 [xx].verLen = 0.0;
		placingZone->cellsT2 [xx].height = 0.0;
		placingZone->cellsT2 [xx].anchor = RIGHT_TOP;

		placingZone->cellsL1 [xx].objType = NONE;
		placingZone->cellsL1 [xx].leftBottomX = 0.0;
		placingZone->cellsL1 [xx].leftBottomY = 0.0;
		placingZone->cellsL1 [xx].leftBottomZ = 0.0;
		placingZone->cellsL1 [xx].ang = 0.0;
		placingZone->cellsL1 [xx].horLen = 0.0;
		placingZone->cellsL1 [xx].verLen = 0.0;
		placingZone->cellsL1 [xx].height = 0.0;
		placingZone->cellsL1 [xx].anchor = LEFT_TOP;

		placingZone->cellsL2 [xx].objType = NONE;
		placingZone->cellsL2 [xx].leftBottomX = 0.0;
		placingZone->cellsL2 [xx].leftBottomY = 0.0;
		placingZone->cellsL2 [xx].leftBottomZ = 0.0;
		placingZone->cellsL2 [xx].ang = 0.0;
		placingZone->cellsL2 [xx].horLen = 0.0;
		placingZone->cellsL2 [xx].verLen = 0.0;
		placingZone->cellsL2 [xx].height = 0.0;
		placingZone->cellsL2 [xx].anchor = LEFT_BOTTOM;

		placingZone->cellsR1 [xx].objType = NONE;
		placingZone->cellsR1 [xx].leftBottomX = 0.0;
		placingZone->cellsR1 [xx].leftBottomY = 0.0;
		placingZone->cellsR1 [xx].leftBottomZ = 0.0;
		placingZone->cellsR1 [xx].ang = 0.0;
		placingZone->cellsR1 [xx].horLen = 0.0;
		placingZone->cellsR1 [xx].verLen = 0.0;
		placingZone->cellsR1 [xx].height = 0.0;
		placingZone->cellsR1 [xx].anchor = RIGHT_TOP;

		placingZone->cellsR2 [xx].objType = NONE;
		placingZone->cellsR2 [xx].leftBottomX = 0.0;
		placingZone->cellsR2 [xx].leftBottomY = 0.0;
		placingZone->cellsR2 [xx].leftBottomZ = 0.0;
		placingZone->cellsR2 [xx].ang = 0.0;
		placingZone->cellsR2 [xx].horLen = 0.0;
		placingZone->cellsR2 [xx].verLen = 0.0;
		placingZone->cellsR2 [xx].height = 0.0;
		placingZone->cellsR2 [xx].anchor = RIGHT_BOTTOM;

		placingZone->cellsB1 [xx].objType = NONE;
		placingZone->cellsB1 [xx].leftBottomX = 0.0;
		placingZone->cellsB1 [xx].leftBottomY = 0.0;
		placingZone->cellsB1 [xx].leftBottomZ = 0.0;
		placingZone->cellsB1 [xx].ang = 0.0;
		placingZone->cellsB1 [xx].horLen = 0.0;
		placingZone->cellsB1 [xx].verLen = 0.0;
		placingZone->cellsB1 [xx].height = 0.0;
		placingZone->cellsB1 [xx].anchor = LEFT_BOTTOM;

		placingZone->cellsB2 [xx].objType = NONE;
		placingZone->cellsB2 [xx].leftBottomX = 0.0;
		placingZone->cellsB2 [xx].leftBottomY = 0.0;
		placingZone->cellsB2 [xx].leftBottomZ = 0.0;
		placingZone->cellsB2 [xx].ang = 0.0;
		placingZone->cellsB2 [xx].horLen = 0.0;
		placingZone->cellsB2 [xx].verLen = 0.0;
		placingZone->cellsB2 [xx].height = 0.0;
		placingZone->cellsB2 [xx].anchor = RIGHT_BOTTOM;
	}

	// 셀 개수 초기화
	placingZone->nCells = 0;
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK columnPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	//double		h1, h2, h3, h4, hRest;	// 나머지 높이 계산을 위한 변수
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "기둥에 배치 - 기둥 단면");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 확인 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 취소 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			//////////////////////////////////////////////////////////// 아이템 배치 (유로폼)
			// 라벨 및 체크박스
			//DGSetItemText (dialogID, LABEL_BEAM_SECTION, "보 단면");
			//DGSetItemText (dialogID, LABEL_BEAM_HEIGHT, "보 높이");
			//DGSetItemText (dialogID, LABEL_BEAM_WIDTH, "보 너비");
			//DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT, "총 높이");
			//DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, "총 너비");

			//DGSetItemText (dialogID, LABEL_REST_SIDE, "나머지");
			//DGSetItemText (dialogID, CHECKBOX_WOOD_SIDE, "목재");
			//DGSetItemText (dialogID, CHECKBOX_T_FORM_SIDE, "유로폼");
			//DGSetItemText (dialogID, CHECKBOX_FILLER_SIDE, "휠러");
			//DGSetItemText (dialogID, CHECKBOX_B_FORM_SIDE, "유로폼");

			//DGSetItemText (dialogID, CHECKBOX_L_FORM_BOTTOM, "유로폼");
			//DGSetItemText (dialogID, CHECKBOX_FILLER_BOTTOM, "휠러");
			//DGSetItemText (dialogID, CHECKBOX_R_FORM_BOTTOM, "유로폼");

			// 라벨: 레이어 설정
			//DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			//DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			//DGSetItemText (dialogID, LABEL_LAYER_FILLERSPACER, "휠러스페이서");
			//DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			//DGSetItemText (dialogID, LABEL_LAYER_WOOD, "목재");
			//DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "아웃코너앵글");

			// 유저 컨트롤 초기화
			//BNZeroMemory (&ucb, sizeof (ucb));
			//ucb.dialogID = dialogID;
			//ucb.type	 = APIUserControlType_Layer;
			//ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			//ucb.itemID	 = USERCONTROL_LAYER_FILLERSPACER;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, 1);

			//ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			//ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);

			//ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);

			// 보 높이/너비 계산
			//DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_HEIGHT, placingZone.areaHeight);
			//DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, infoBeam.width);

			// 총 높이/너비 계산
			//DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.areaHeight + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));
			//DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, infoBeam.width + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1)*2);

			// 부재별 체크박스-규격 설정
			//(DGGetItemValLong (dialogID, CHECKBOX_WOOD_SIDE) == TRUE) ?		DGEnableItem (dialogID, EDITCONTROL_WOOD_SIDE)		:	DGDisableItem (dialogID, EDITCONTROL_WOOD_SIDE);
			//(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_SIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_SIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_SIDE);
			//(DGGetItemValLong (dialogID, CHECKBOX_FILLER_SIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_SIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_SIDE);
			//(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
			//(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

			// 측면 0번, 하부 0번 셀은 무조건 사용해야 함
			//DGSetItemValLong (dialogID, CHECKBOX_B_FORM_SIDE, TRUE);
			//DGSetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM, TRUE);
			//DGDisableItem (dialogID, CHECKBOX_B_FORM_SIDE);
			//DGDisableItem (dialogID, CHECKBOX_L_FORM_BOTTOM);

			// 나머지 값 계산
			// ...

			// 직접 변경해서는 안 되는 항목 잠그기
			//DGDisableItem (dialogID, EDITCONTROL_GAP_SIDE2);
			//DGDisableItem (dialogID, EDITCONTROL_BEAM_HEIGHT);
			//DGDisableItem (dialogID, EDITCONTROL_BEAM_WIDTH);
			//DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT);
			//DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH);
			//DGDisableItem (dialogID, EDITCONTROL_REST_SIDE);

			break;
		
		case DG_MSG_CHANGE:
			// 나머지 값 계산
			// ...

			switch (item) {
				// 총 높이/너비 계산
				// ...

				// 부재별 체크박스-규격 설정
				//case CHECKBOX_WOOD_SIDE:
				//	(DGGetItemValLong (dialogID, CHECKBOX_WOOD_SIDE) == TRUE) ?		DGEnableItem (dialogID, EDITCONTROL_WOOD_SIDE)		:	DGDisableItem (dialogID, EDITCONTROL_WOOD_SIDE);
				//	break;
				//case CHECKBOX_T_FORM_SIDE:
				//	(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_SIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_SIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_SIDE);
				//	break;
				//case CHECKBOX_FILLER_SIDE:
				//	(DGGetItemValLong (dialogID, CHECKBOX_FILLER_SIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_SIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_SIDE);
				//	break;
				//case CHECKBOX_B_FORM_SIDE:
				//	(DGGetItemValLong (dialogID, CHECKBOX_B_FORM_SIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_B_FORM_SIDE)			:	DGDisableItem (dialogID, POPUP_B_FORM_SIDE);
				//	break;
			}

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 셀 설정 적용
					// ...

					// 레이어 번호 저장
					//layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					//layerInd_Fillerspacer	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER);
					//layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					//layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
					//layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);

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