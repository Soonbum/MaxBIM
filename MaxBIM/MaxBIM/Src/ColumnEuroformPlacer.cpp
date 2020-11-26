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

// 추가/삭제 버튼 인덱스 저장
static short	ADD_CELLS;
static short	DEL_CELLS;
static short	CHECKBOX_NORTH_MARGIN;
static short	CHECKBOX_SOUTH_MARGIN;
static short	CHECKBOX_WEST_MARGIN;
static short	CHECKBOX_EAST_MARGIN;
static short	EDITCONTROL_NORTH_MARGIN;
static short	EDITCONTROL_SOUTH_MARGIN;
static short	EDITCONTROL_WEST_MARGIN;
static short	EDITCONTROL_EAST_MARGIN;


// 5번 메뉴: 기둥에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnColumn (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx, yy;
	double			dx, dy, ang;
	short			result;
	double			lowestBeamBottomLevel;

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
	//API_Elem_Head* headList = new API_Elem_Head [1];
	//headList [0] = elem.header;
	//err = ACAPI_Element_Delete (&headList, 1);
	//delete headList;

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

FIRST_SOLE_COLUMN:
	
		// 영역 정보 중 간섭 보 관련 정보 업데이트
		if (nInterfereBeams > 0) {
			placingZone.bInterfereBeam = true;
			placingZone.nInterfereBeams = nInterfereBeams;
			
			for (xx = 0 ; xx < 4 ; ++xx) {
				placingZone.bottomLevelOfBeams [xx] = 0.0;
				placingZone.bExistBeams [xx] = false;
			}

			for (xx = 0 ; xx < nInterfereBeams ; ++xx) {

				// !!!
				//char msg [200];
				//sprintf (msg, "보 높이 [%d]: %.4f", xx+1, infoOtherBeams [xx].level - infoOtherBeams [xx].height);
				//ACAPI_WriteReport (msg, true);

				// 기둥의 동/서/남/북 방향에 있는 보의 하단 레벨을 저장함
				if ( (infoOtherBeams [xx].begC.x <= (placingZone.origoPos.x + placingZone.coreWidth/2)) && (infoOtherBeams [xx].begC.x >= (placingZone.origoPos.x - placingZone.coreWidth/2)) && (infoOtherBeams [xx].begC.y > (placingZone.origoPos.y + placingZone.coreDepth/2)) ) {
					placingZone.bottomLevelOfBeams [NORTH] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
					placingZone.bExistBeams [NORTH] = true;
				}
				if ( (infoOtherBeams [xx].begC.x <= (placingZone.origoPos.x + placingZone.coreWidth/2)) && (infoOtherBeams [xx].begC.x >= (placingZone.origoPos.x - placingZone.coreWidth/2)) && (infoOtherBeams [xx].begC.y < (placingZone.origoPos.y - placingZone.coreDepth/2)) ) {
					placingZone.bottomLevelOfBeams [SOUTH] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
					placingZone.bExistBeams [SOUTH] = true;
				}
				if ( (infoOtherBeams [xx].begC.y <= (placingZone.origoPos.y + placingZone.coreDepth/2)) && (infoOtherBeams [xx].begC.y >= (placingZone.origoPos.y - placingZone.coreDepth/2)) && (infoOtherBeams [xx].begC.x > (placingZone.origoPos.x + placingZone.coreWidth/2)) ) {
					placingZone.bottomLevelOfBeams [EAST] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
					placingZone.bExistBeams [EAST] = true;
				}
				if ( (infoOtherBeams [xx].begC.y <= (placingZone.origoPos.y + placingZone.coreDepth/2)) && (infoOtherBeams [xx].begC.y >= (placingZone.origoPos.y - placingZone.coreDepth/2)) && (infoOtherBeams [xx].begC.x < (placingZone.origoPos.x - placingZone.coreWidth/2)) ) {
					placingZone.bottomLevelOfBeams [WEST] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
					placingZone.bExistBeams [WEST] = true;
				}
			}
		} else {
			placingZone.bInterfereBeam = false;
			placingZone.nInterfereBeams = 0;

			for (xx = 0 ; xx < 4 ; ++xx) {
				placingZone.bottomLevelOfBeams [xx] = 0.0;
				placingZone.bExistBeams [xx] = false;
			}
		}

		// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32531, ACAPI_GetOwnResModule (), columnPlacerHandler1, 0);

		if (result == DG_CANCEL)
			return err;

		// 영역 높이와 보 하단 레벨을 고려하여 셀의 개수를 연산
		lowestBeamBottomLevel = placingZone.bottomOffset + placingZone.areaHeight;	// 기본값: 영역 높이
		if (nInterfereBeams > 0) {
			// 가장 낮은 보 하단 레벨을 기준으로 셀 개수 연산
			for (xx = 0 ; xx < 4 ; ++xx) {
				if (placingZone.bExistBeams [xx] == true)
					if (lowestBeamBottomLevel > placingZone.bottomLevelOfBeams [xx])
						lowestBeamBottomLevel = placingZone.bottomLevelOfBeams [xx];
			}
		}

		placingZone.nCells = static_cast<short>((lowestBeamBottomLevel + EPS) / 1.200);		// 높이 1200 폼 기준으로 들어갈 수 있는 최대 셀 개수 연산

		// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정합니다.
		clickedOKButton = false;
		clickedPrevButton = false;
		result = DGBlankModalDialog (700, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, columnPlacerHandler2, 0);
	
		// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
		if (clickedPrevButton == true)
			goto FIRST_SOLE_COLUMN;

		// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
		if (clickedOKButton != true)
			return err;

		// 1, 2번째 다이얼로그를 통해 입력된 데이터를 기반으로 객체를 배치
		// ...

		// 나머지 영역 채우기 - 합판, 목재
		//err = fillRestAreasForColumn (&placingZone);

		// 결과물 전체 그룹화
		//if (!elemList.IsEmpty ()) {
		//	GSSize nElems = elemList.GetSize ();
		//	API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		//	if (elemHead != NULL) {
		//		for (GSIndex i = 0; i < nElems; i++)
		//			(*elemHead)[i].guid = elemList[i];

		//		ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

		//		BMKillHandle ((GSHandle *) &elemHead);
		//	}
		//}

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
		placingZone->cellsT1 [xx].anchor = TOP;

		placingZone->cellsT2 [xx].objType = NONE;
		placingZone->cellsT2 [xx].leftBottomX = 0.0;
		placingZone->cellsT2 [xx].leftBottomY = 0.0;
		placingZone->cellsT2 [xx].leftBottomZ = 0.0;
		placingZone->cellsT2 [xx].ang = 0.0;
		placingZone->cellsT2 [xx].horLen = 0.0;
		placingZone->cellsT2 [xx].verLen = 0.0;
		placingZone->cellsT2 [xx].height = 0.0;
		placingZone->cellsT2 [xx].anchor = TOP;

		placingZone->cellsL1 [xx].objType = NONE;
		placingZone->cellsL1 [xx].leftBottomX = 0.0;
		placingZone->cellsL1 [xx].leftBottomY = 0.0;
		placingZone->cellsL1 [xx].leftBottomZ = 0.0;
		placingZone->cellsL1 [xx].ang = 0.0;
		placingZone->cellsL1 [xx].horLen = 0.0;
		placingZone->cellsL1 [xx].verLen = 0.0;
		placingZone->cellsL1 [xx].height = 0.0;
		placingZone->cellsL1 [xx].anchor = LEFT;

		placingZone->cellsL2 [xx].objType = NONE;
		placingZone->cellsL2 [xx].leftBottomX = 0.0;
		placingZone->cellsL2 [xx].leftBottomY = 0.0;
		placingZone->cellsL2 [xx].leftBottomZ = 0.0;
		placingZone->cellsL2 [xx].ang = 0.0;
		placingZone->cellsL2 [xx].horLen = 0.0;
		placingZone->cellsL2 [xx].verLen = 0.0;
		placingZone->cellsL2 [xx].height = 0.0;
		placingZone->cellsL2 [xx].anchor = LEFT;

		placingZone->cellsR1 [xx].objType = NONE;
		placingZone->cellsR1 [xx].leftBottomX = 0.0;
		placingZone->cellsR1 [xx].leftBottomY = 0.0;
		placingZone->cellsR1 [xx].leftBottomZ = 0.0;
		placingZone->cellsR1 [xx].ang = 0.0;
		placingZone->cellsR1 [xx].horLen = 0.0;
		placingZone->cellsR1 [xx].verLen = 0.0;
		placingZone->cellsR1 [xx].height = 0.0;
		placingZone->cellsR1 [xx].anchor = RIGHT;

		placingZone->cellsR2 [xx].objType = NONE;
		placingZone->cellsR2 [xx].leftBottomX = 0.0;
		placingZone->cellsR2 [xx].leftBottomY = 0.0;
		placingZone->cellsR2 [xx].leftBottomZ = 0.0;
		placingZone->cellsR2 [xx].ang = 0.0;
		placingZone->cellsR2 [xx].horLen = 0.0;
		placingZone->cellsR2 [xx].verLen = 0.0;
		placingZone->cellsR2 [xx].height = 0.0;
		placingZone->cellsR2 [xx].anchor = RIGHT;

		placingZone->cellsB1 [xx].objType = NONE;
		placingZone->cellsB1 [xx].leftBottomX = 0.0;
		placingZone->cellsB1 [xx].leftBottomY = 0.0;
		placingZone->cellsB1 [xx].leftBottomZ = 0.0;
		placingZone->cellsB1 [xx].ang = 0.0;
		placingZone->cellsB1 [xx].horLen = 0.0;
		placingZone->cellsB1 [xx].verLen = 0.0;
		placingZone->cellsB1 [xx].height = 0.0;
		placingZone->cellsB1 [xx].anchor = BOTTOM;

		placingZone->cellsB2 [xx].objType = NONE;
		placingZone->cellsB2 [xx].leftBottomX = 0.0;
		placingZone->cellsB2 [xx].leftBottomY = 0.0;
		placingZone->cellsB2 [xx].leftBottomZ = 0.0;
		placingZone->cellsB2 [xx].ang = 0.0;
		placingZone->cellsB2 [xx].horLen = 0.0;
		placingZone->cellsB2 [xx].verLen = 0.0;
		placingZone->cellsB2 [xx].height = 0.0;
		placingZone->cellsB2 [xx].anchor = BOTTOM;
	}

	// 셀 개수 초기화
	placingZone->nCells = 0;
}

// 꼭대기에 셀 추가
void	addTopCell (ColumnPlacingZone* target_zone)
{
	target_zone->cellsLT [target_zone->nCells] = target_zone->cellsLT [target_zone->nCells - 1];
	target_zone->cellsRT [target_zone->nCells] = target_zone->cellsRT [target_zone->nCells - 1];
	target_zone->cellsLB [target_zone->nCells] = target_zone->cellsLB [target_zone->nCells - 1];
	target_zone->cellsRB [target_zone->nCells] = target_zone->cellsRB [target_zone->nCells - 1];
	target_zone->cellsT1 [target_zone->nCells] = target_zone->cellsT1 [target_zone->nCells - 1];
	target_zone->cellsT2 [target_zone->nCells] = target_zone->cellsT2 [target_zone->nCells - 1];
	target_zone->cellsL1 [target_zone->nCells] = target_zone->cellsL1 [target_zone->nCells - 1];
	target_zone->cellsL2 [target_zone->nCells] = target_zone->cellsL2 [target_zone->nCells - 1];
	target_zone->cellsR1 [target_zone->nCells] = target_zone->cellsR1 [target_zone->nCells - 1];
	target_zone->cellsR2 [target_zone->nCells] = target_zone->cellsR2 [target_zone->nCells - 1];
	target_zone->cellsB1 [target_zone->nCells] = target_zone->cellsB1 [target_zone->nCells - 1];
	target_zone->cellsB2 [target_zone->nCells] = target_zone->cellsB2 [target_zone->nCells - 1];

	target_zone->nCells ++;
}

// 꼭대기의 셀 삭제
void	delTopCell (ColumnPlacingZone* target_zone)
{
	target_zone->nCells --;
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK columnPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	API_UCCallbackType	ucb;

	API_Coord	rotatedPoint;
	double		lineLen;
	double		xLen, yLen;
	double		formWidth;

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
			DGSetItemText (dialogID, LABEL_COLUMN_SECTION, "기둥 단면");
			DGSetItemText (dialogID, LABEL_COLUMN_WIDTH, "가로");
			DGSetItemText (dialogID, LABEL_COLUMN_DEPTH, "세로");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER, "인코너");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER, "아웃코너");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_MAGIC_BAR, "매직 바");
			DGSetItemText (dialogID, LABEL_LAYER_MAGIC_INCORNER, "매직 인코너");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_INCORNER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_MAGIC_BAR;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_MAGIC_BAR, 1);

			ucb.itemID	 = USERCONTROL_LAYER_MAGIC_INCORNER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_MAGIC_INCORNER, 1);

			// 기둥 가로/세로 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_COLUMN_WIDTH, placingZone.coreWidth + placingZone.venThick * 2);
			DGSetItemValDouble (dialogID, EDITCONTROL_COLUMN_DEPTH, placingZone.coreDepth + placingZone.venThick * 2);
			DGDisableItem (dialogID, EDITCONTROL_COLUMN_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_COLUMN_DEPTH);

			// 부재별 체크박스-규격 설정
			(DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) ?		DGEnableItem (dialogID, EDITCONTROL_LEFT_3)		: 	DGDisableItem (dialogID, EDITCONTROL_LEFT_3);
			(DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_BOTTOM_3)	: 	DGDisableItem (dialogID, EDITCONTROL_BOTTOM_3);

			// 직접 변경해서는 안 되는 항목 잠그기
			DGDisableItem (dialogID, EDITCONTROL_TOP_1);
			DGDisableItem (dialogID, EDITCONTROL_TOP_2);
			DGDisableItem (dialogID, EDITCONTROL_TOP_3);
			DGDisableItem (dialogID, EDITCONTROL_TOP_4);
			DGDisableItem (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_1);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_2);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_3);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_4);
			DGDisableItem (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM);

			// 기본값 입력해 놓음
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_4, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_4, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_4, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4, 0.100);

			break;
		
		case DG_MSG_CHANGE:

			// 부재별 체크박스-규격 설정
			if (DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) {
				DGEnableItem (dialogID, EDITCONTROL_LEFT_3);
				DGEnableItem (dialogID, EDITCONTROL_RIGHT_3);
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_LEFT_3);
				DGDisableItem (dialogID, EDITCONTROL_RIGHT_3);
				DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_3, 0.0);
				DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_3, 0.0);
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, FALSE);
			}
			if (DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) {
				DGEnableItem (dialogID, EDITCONTROL_BOTTOM_3);
				DGEnableItem (dialogID, EDITCONTROL_TOP_3);
				DGSetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_BOTTOM_3);
				DGDisableItem (dialogID, EDITCONTROL_TOP_3);
				DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3, 0.0);
				DGSetItemValDouble (dialogID, EDITCONTROL_TOP_3, 0.0);
				DGSetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM, FALSE);
			}

			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_1, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_2, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_3, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_4, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4));

			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_1, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_2, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_3, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_4, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4));

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 셀 설정 적용
					for (xx = 0 ; xx < 20 ; ++xx) {
						// 좌상단
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsLT [xx].objType = OUTCORNER;
						placingZone.cellsLT [xx].leftBottomX = rotatedPoint.x;
						placingZone.cellsLT [xx].leftBottomY = rotatedPoint.y;
						placingZone.cellsLT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsLT [xx].ang = placingZone.angle - DegreeToRad (90);
						placingZone.cellsLT [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1);
						placingZone.cellsLT [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1);
						placingZone.cellsLT [xx].height = 1.200;
						placingZone.cellsLT [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1);
						placingZone.cellsLT [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1);
						placingZone.cellsLT [xx].libPart.outcorner.hei_s = 1.200;

						// 우상단
						xLen = (placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsRT [xx].objType = OUTCORNER;
						placingZone.cellsRT [xx].leftBottomX = rotatedPoint.x;
						placingZone.cellsRT [xx].leftBottomY = rotatedPoint.y;
						placingZone.cellsRT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsRT [xx].ang = placingZone.angle + DegreeToRad (180);
						placingZone.cellsRT [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4);
						placingZone.cellsRT [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1);
						placingZone.cellsRT [xx].height = 1.200;
						placingZone.cellsRT [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1);
						placingZone.cellsRT [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4);
						placingZone.cellsRT [xx].libPart.outcorner.hei_s = 1.200;

						// 좌하단
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsLB [xx].objType = OUTCORNER;
						placingZone.cellsLB [xx].leftBottomX = rotatedPoint.x;
						placingZone.cellsLB [xx].leftBottomY = rotatedPoint.y;
						placingZone.cellsLB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsLB [xx].ang = placingZone.angle;
						placingZone.cellsLB [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1);
						placingZone.cellsLB [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4);
						placingZone.cellsLB [xx].height = 1.200;
						placingZone.cellsLB [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4);
						placingZone.cellsLB [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1);
						placingZone.cellsLB [xx].libPart.outcorner.hei_s = 1.200;

						// 우하단
						xLen = (placingZone.coreWidth/2 + placingZone.venThick);
						yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsRB [xx].objType = OUTCORNER;
						placingZone.cellsRB [xx].leftBottomX = rotatedPoint.x;
						placingZone.cellsRB [xx].leftBottomY = rotatedPoint.y;
						placingZone.cellsRB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsRB [xx].ang = placingZone.angle + DegreeToRad (90);
						placingZone.cellsRB [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4);
						placingZone.cellsRB [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4);
						placingZone.cellsRB [xx].height = 1.200;
						placingZone.cellsRB [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4);
						placingZone.cellsRB [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4);
						placingZone.cellsRB [xx].libPart.outcorner.hei_s = 1.200;

						// 위쪽 1
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsT1 [xx].objType = EUROFORM;
						placingZone.cellsT1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2)) * cos(placingZone.angle);
						placingZone.cellsT1 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2)) * sin(placingZone.angle);
						placingZone.cellsT1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsT1 [xx].ang = placingZone.angle + DegreeToRad (180);
						placingZone.cellsT1 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2);
						placingZone.cellsT1 [xx].verLen = 0.064;
						placingZone.cellsT1 [xx].height = 1.200;
						placingZone.cellsT1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2);
						placingZone.cellsT1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2);
						placingZone.cellsT1 [xx].libPart.form.eu_hei = 1.200;
						placingZone.cellsT1 [xx].libPart.form.eu_hei2 = 1.200;
						placingZone.cellsT1 [xx].libPart.form.u_ins_wall = true;
						formWidth = placingZone.cellsT1 [xx].libPart.form.eu_wid;
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsT1 [xx].libPart.form.eu_stan_onoff = true;
						else
							placingZone.cellsT1 [xx].libPart.form.eu_stan_onoff = false;

						// 위쪽 2
						if (DGGetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM) == TRUE) {
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsT2 [xx].objType = EUROFORM;
							placingZone.cellsT2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3)) * cos(placingZone.angle);
							placingZone.cellsT2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3)) * sin(placingZone.angle);
							placingZone.cellsT2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsT2 [xx].ang = placingZone.angle + DegreeToRad (180);
							placingZone.cellsT2 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3);
							placingZone.cellsT2 [xx].verLen = 0.064;
							placingZone.cellsT2 [xx].height = 1.200;
							placingZone.cellsT2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3);
							placingZone.cellsT2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3);
							placingZone.cellsT2 [xx].libPart.form.eu_hei = 1.200;
							placingZone.cellsT2 [xx].libPart.form.eu_hei2 = 1.200;
							placingZone.cellsT2 [xx].libPart.form.u_ins_wall = true;
							formWidth = placingZone.cellsT2 [xx].libPart.form.eu_wid;
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsT2 [xx].libPart.form.eu_stan_onoff = true;
							else
								placingZone.cellsT2 [xx].libPart.form.eu_stan_onoff = false;
						}

						// 좌측 1
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsL1 [xx].objType = EUROFORM;
						placingZone.cellsL1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) * cos(placingZone.angle);
						placingZone.cellsL1 [xx].leftBottomY = rotatedPoint.y - DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) * sin(placingZone.angle);
						placingZone.cellsL1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsL1 [xx].ang = placingZone.angle - DegreeToRad (90);
						placingZone.cellsL1 [xx].horLen = 0.064;
						placingZone.cellsL1 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2);
						placingZone.cellsL1 [xx].height = 1.200;
						placingZone.cellsL1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2);
						placingZone.cellsL1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2);
						placingZone.cellsL1 [xx].libPart.form.eu_hei = 1.200;
						placingZone.cellsL1 [xx].libPart.form.eu_hei2 = 1.200;
						placingZone.cellsL1 [xx].libPart.form.u_ins_wall = true;
						formWidth = placingZone.cellsL1 [xx].libPart.form.eu_wid;
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsL1 [xx].libPart.form.eu_stan_onoff = true;
						else
							placingZone.cellsL1 [xx].libPart.form.eu_stan_onoff = false;

						// 좌측 2
						if (DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) {
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsL2 [xx].objType = EUROFORM;
							placingZone.cellsL2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2)) * cos(placingZone.angle);
							placingZone.cellsL2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2)) * sin(placingZone.angle);
							placingZone.cellsL2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsL2 [xx].ang = placingZone.angle - DegreeToRad (90);
							placingZone.cellsL2 [xx].horLen = 0.064;
							placingZone.cellsL2 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3);
							placingZone.cellsL2 [xx].height = 1.200;
							placingZone.cellsL2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3);
							placingZone.cellsL2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3);
							placingZone.cellsL2 [xx].libPart.form.eu_hei = 1.200;
							placingZone.cellsL2 [xx].libPart.form.eu_hei2 = 1.200;
							placingZone.cellsL2 [xx].libPart.form.u_ins_wall = true;
							formWidth = placingZone.cellsL2 [xx].libPart.form.eu_wid;
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsL2 [xx].libPart.form.eu_stan_onoff = true;
							else
								placingZone.cellsL2 [xx].libPart.form.eu_stan_onoff = false;
						}

						// 우측 1
						xLen = (placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsR1 [xx].objType = EUROFORM;
						placingZone.cellsR1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2)) * cos(placingZone.angle);
						placingZone.cellsR1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2)) * sin(placingZone.angle);
						placingZone.cellsR1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsR1 [xx].ang = placingZone.angle + DegreeToRad (90);
						placingZone.cellsR1 [xx].horLen = 0.064;
						placingZone.cellsR1 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2);
						placingZone.cellsR1 [xx].height = 1.200;
						placingZone.cellsR1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2);
						placingZone.cellsR1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2);
						placingZone.cellsR1 [xx].libPart.form.eu_hei = 1.200;
						placingZone.cellsR1 [xx].libPart.form.eu_hei2 = 1.200;
						placingZone.cellsR1 [xx].libPart.form.u_ins_wall = true;
						formWidth = placingZone.cellsR1 [xx].libPart.form.eu_wid;
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsR1 [xx].libPart.form.eu_stan_onoff = true;
						else
							placingZone.cellsR1 [xx].libPart.form.eu_stan_onoff = false;

						// 우측 2
						if (DGGetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM) == TRUE) { 
							xLen = (placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsR2 [xx].objType = EUROFORM;
							placingZone.cellsR2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * cos(placingZone.angle);
							placingZone.cellsR2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * sin(placingZone.angle);
							placingZone.cellsR2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsR2 [xx].ang = placingZone.angle + DegreeToRad (90);
							placingZone.cellsR2 [xx].horLen = 0.064;
							placingZone.cellsR2 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3);
							placingZone.cellsR2 [xx].height = 1.200;
							placingZone.cellsR2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3);
							placingZone.cellsR2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3);
							placingZone.cellsR2 [xx].libPart.form.eu_hei = 1.200;
							placingZone.cellsR2 [xx].libPart.form.eu_hei2 = 1.200;
							placingZone.cellsR2 [xx].libPart.form.u_ins_wall = true;
							formWidth = placingZone.cellsR2 [xx].libPart.form.eu_wid;
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsR2 [xx].libPart.form.eu_stan_onoff = true;
							else
								placingZone.cellsR2 [xx].libPart.form.eu_stan_onoff = false;
						}

						// 아래쪽 1
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsB1 [xx].objType = EUROFORM;
						placingZone.cellsB1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) * cos(placingZone.angle);
						placingZone.cellsB1 [xx].leftBottomY = rotatedPoint.y + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) * sin(placingZone.angle);
						placingZone.cellsB1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsB1 [xx].ang = placingZone.angle;
						placingZone.cellsB1 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2);
						placingZone.cellsB1 [xx].verLen = 0.064;
						placingZone.cellsB1 [xx].height = 1.200;
						placingZone.cellsB1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2);
						placingZone.cellsB1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2);
						placingZone.cellsB1 [xx].libPart.form.eu_hei = 1.200;
						placingZone.cellsB1 [xx].libPart.form.eu_hei2 = 1.200;
						placingZone.cellsB1 [xx].libPart.form.u_ins_wall = true;
						formWidth = placingZone.cellsB1 [xx].libPart.form.eu_wid;
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsB1 [xx].libPart.form.eu_stan_onoff = true;
						else
							placingZone.cellsB1 [xx].libPart.form.eu_stan_onoff = false;

						// 아래쪽 2
						if (DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) {
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsB2 [xx].objType = EUROFORM;
							placingZone.cellsB2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2)) * cos(placingZone.angle);
							placingZone.cellsB2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2)) * sin(placingZone.angle);
							placingZone.cellsB2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsB2 [xx].ang = placingZone.angle;
							placingZone.cellsB2 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3);
							placingZone.cellsB2 [xx].verLen = 0.064;
							placingZone.cellsB2 [xx].height = 1.200;
							placingZone.cellsB2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3);
							placingZone.cellsB2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3);
							placingZone.cellsB2 [xx].libPart.form.eu_hei = 1.200;
							placingZone.cellsB2 [xx].libPart.form.eu_hei2 = 1.200;
							placingZone.cellsB2 [xx].libPart.form.u_ins_wall = true;
							formWidth = placingZone.cellsB2 [xx].libPart.form.eu_wid;
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsB2 [xx].libPart.form.eu_stan_onoff = true;
							else
								placingZone.cellsB2 [xx].libPart.form.eu_stan_onoff = false;
						}
					}

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Incorner		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER);
					layerInd_Outcorner		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_MagicBar		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_MAGIC_BAR);
					layerInd_MagicIncorner	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_MAGIC_INCORNER);

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

// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK columnPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	btnSizeX = 50, btnSizeY = 50;
	short	dialogSizeX, dialogSizeY;
	short	btnPosX, btnPosY;
	short	xx;
	std::string		txtButton = "";

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "기둥에 배치 - 기둥 측면");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 100, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 140, 100, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 업데이트 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 60, 100, 25);
			DGSetItemFont (dialogID, DG_UPDATE_BUTTON, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_UPDATE_BUTTON, "업데이트");
			DGShowItem (dialogID, DG_UPDATE_BUTTON);

			// 이전 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 180, 100, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "이전");
			DGShowItem (dialogID, DG_PREV);

			// 라벨: 기둥 측면
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 10, 100, 23);
			DGSetItemFont (dialogID, LABEL_COLUMN_SIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_COLUMN_SIDE, "기둥 측면");
			DGShowItem (dialogID, LABEL_COLUMN_SIDE);

			// 보 단면
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
			DGSetItemFont (dialogID, BUTTON_BEAM_SECTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, BUTTON_BEAM_SECTION);
			if (placingZone.bInterfereBeam == true) {
				DGEnableItem (dialogID, BUTTON_BEAM_SECTION);
				DGSetItemText (dialogID, BUTTON_BEAM_SECTION, "보\n있음");
			} else {
				DGDisableItem (dialogID, BUTTON_BEAM_SECTION);
				DGSetItemText (dialogID, BUTTON_BEAM_SECTION, "보\n없음");
			}

			// 여백 위치
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
			DGSetItemFont (dialogID, LABEL_MARGIN_PLACE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_MARGIN_PLACE, "여백");
			DGShowItem (dialogID, LABEL_MARGIN_PLACE);

			// 보를 의미하는 직사각형
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
			DGShowItem (dialogID, itmIdx);

			// 버튼 시작
			btnPosX = 230;
			btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
			for (xx = placingZone.nCells-1 ; xx >= 0 ; --xx) {

				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellsB1 [xx].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
					txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsB1 [xx].height * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				btnPosY -= 50;
			}

			// 추가/삭제 버튼
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 113, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "추가");
			DGShowItem (dialogID, itmIdx);
			ADD_CELLS = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "삭제");
			DGShowItem (dialogID, itmIdx);
			DEL_CELLS = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "←");
			DGShowItem (dialogID, itmIdx);

			// 보 단면을 의미하는 직사각형
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
			DGShowItem (dialogID, itmIdx);

			// 라벨: 동서남북
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "북");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "남");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "서");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "동");
			DGShowItem (dialogID, itmIdx);

			// 체크박스: 동서남북 여백 채움 여부
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 채움");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// 북
			CHECKBOX_NORTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 채움");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// 남
			CHECKBOX_SOUTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 채움");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// 서
			CHECKBOX_WEST_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 채움");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// 동
			CHECKBOX_EAST_MARGIN = itmIdx;

			// 여백 계산 (북)
			if (placingZone.bExistBeams [NORTH] == true)
				placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
			else
				placingZone.marginTopAtNorth = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;

			// 여백 계산 (남)
			if (placingZone.bExistBeams [SOUTH] == true)
				placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
			else
				placingZone.marginTopAtSouth = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;

			// 여백 계산 (서)
			if (placingZone.bExistBeams [WEST] == true)
				placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
			else
				placingZone.marginTopAtWest = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;

			// 여백 계산 (동)
			if (placingZone.bExistBeams [EAST] == true)
				placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
			else
				placingZone.marginTopAtEast = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;

			// Edit 컨트롤: 동서남북 여백 치수 표시
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 75, 40, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtNorth);
			DGShowItem (dialogID, itmIdx);	// 북
			DGDisableItem (dialogID, itmIdx);
			EDITCONTROL_NORTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 182, 40, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtSouth);
			DGShowItem (dialogID, itmIdx);	// 남
			DGDisableItem (dialogID, itmIdx);
			EDITCONTROL_SOUTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 440, 142, 40, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtWest);
			DGShowItem (dialogID, itmIdx);	// 서
			DGDisableItem (dialogID, itmIdx);
			EDITCONTROL_WEST_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 600, 142, 40, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtEast);
			DGShowItem (dialogID, itmIdx);	// 동
			DGDisableItem (dialogID, itmIdx);
			EDITCONTROL_EAST_MARGIN = itmIdx;

			// 메인 창 크기를 변경
			//dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
			//dialogSizeY = 490;
			//DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;

		case DG_MSG_CHANGE:

			break;

		case DG_MSG_CLICK:

			// 업데이트 버튼
			if (item == DG_UPDATE_BUTTON) {
				item = 0;

				//// 저장된 측면 시작 여백 여부 저장
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
				//	placingZone.bFillMarginBeginAtSide = true;
				//else
				//	placingZone.bFillMarginBeginAtSide = false;

				//// 저장된 측면 끝 여백 여부 저장
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
				//	placingZone.bFillMarginEndAtSide = true;
				//else
				//	placingZone.bFillMarginEndAtSide = false;

				//// 저장된 하부 시작 여백 여부 저장
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
				//	placingZone.bFillMarginBeginAtBottom = true;
				//else
				//	placingZone.bFillMarginBeginAtBottom = false;

				//// 저장된 하부 끝 여백 여부 저장
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
				//	placingZone.bFillMarginEndAtBottom = true;
				//else
				//	placingZone.bFillMarginEndAtBottom = false;

				//// 간섭 보가 붙는 곳 영역 길이 저장
				//placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

				//// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				//alignPlacingZoneForBeam (&placingZone);

				//// 변경 가능성이 있는 DG 항목 모두 제거
				//DGRemoveDialogItems (dialogID, AFTER_ALL);

				//// 측면 시작 부분 여백 채움 여부 - bFillMarginBeginAtSide
				//// 라디오 버튼: 여백 (채움)
				//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
				//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				//DGSetItemText (dialogID, itmIdx, "여백 채움");
				//DGShowItem (dialogID, itmIdx);
				//MARGIN_FILL_FROM_BEGIN_AT_SIDE = itmIdx;
				// ...

				//// 메인 창 크기를 변경
				//dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
				//dialogSizeY = 490;
				//DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// 이전 버튼
			if (item == DG_PREV) {
				clickedPrevButton = true;
			}

			// 확인 버튼
			if (item == DG_OK) {
				clickedOKButton = true;

				//// 여백 채움/비움 여부 저장
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
				//	placingZone.bFillMarginBeginAtSide = true;
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
				//	placingZone.bFillMarginEndAtSide = true;
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
				//	placingZone.bFillMarginBeginAtBottom = true;
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
				//	placingZone.bFillMarginEndAtBottom = true;

				//placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

				//// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				//alignPlacingZoneForBeam (&placingZone);
			}

			// 취소 버튼
			if (item == DG_CANCEL) {
			}

			//// 셀 추가/삭제 버튼 8종
			//if (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) {
			//	addNewColFromBeginAtSide (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) {
			//	delLastColFromBeginAtSide (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_END_AT_SIDE) {
			//	addNewColFromEndAtSide (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_END_AT_SIDE) {
			//	delLastColFromEndAtSide (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) {
			//	addNewColFromBeginAtBottom (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) {
			//	delLastColFromBeginAtBottom (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_END_AT_BOTTOM) {
			//	addNewColFromEndAtBottom (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_END_AT_BOTTOM) {
			//	delLastColFromEndAtBottom (&placingZone);
			//}

			//if ( (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) || (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) || (item == ADD_CELLS_FROM_END_AT_SIDE) || (item == DEL_CELLS_FROM_END_AT_SIDE) ||
			//	 (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == ADD_CELLS_FROM_END_AT_BOTTOM) || (item == DEL_CELLS_FROM_END_AT_BOTTOM)) {

			//	item = 0;

			//	// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
			//	alignPlacingZoneForBeam (&placingZone);

			//	// 변경 가능성이 있는 DG 항목 모두 제거
			//	DGRemoveDialogItems (dialogID, AFTER_ALL);

			//	// 측면 시작 부분 여백 채움 여부 - bFillMarginBeginAtSide
			//	// 라디오 버튼: 여백 (채움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
			// ...

			//	// 메인 창 크기를 변경
			//	dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
			//	dialogSizeY = 490;
			//	DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			//}

			//// 배치 버튼 (측면 시작 부분)
			//if ((item >= START_INDEX_FROM_BEGIN_AT_SIDE) && (item < START_INDEX_FROM_BEGIN_AT_SIDE + placingZone.nCellsFromBeginAtSide)) {
			//	// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
			//// 배치 버튼 (측면 중앙 부분)
			//if (item == START_INDEX_CENTER_AT_SIDE) {
			//	// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
			//// 배치 버튼 (측면 끝 부분)
			//if ((item >= END_INDEX_FROM_END_AT_SIDE) && (item < END_INDEX_FROM_END_AT_SIDE + placingZone.nCellsFromEndAtSide)) {
			//	// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}

			//// 배치 버튼 (하부 시작 부분)
			//if ((item >= START_INDEX_FROM_BEGIN_AT_BOTTOM) && (item < START_INDEX_FROM_BEGIN_AT_BOTTOM + placingZone.nCellsFromBeginAtBottom)) {
			//	// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
			//// 배치 버튼 (하부 중앙 부분)
			//if (item == START_INDEX_CENTER_AT_BOTTOM) {
			//	// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
			//// 배치 버튼 (하부 끝 부분)
			//if ((item >= END_INDEX_FROM_END_AT_BOTTOM) && (item < END_INDEX_FROM_END_AT_BOTTOM + placingZone.nCellsFromEndAtBottom)) {
			//	// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}
