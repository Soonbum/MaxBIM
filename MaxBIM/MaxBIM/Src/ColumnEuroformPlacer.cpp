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
	infoColumn.angle		= elem.column.angle;			// 기둥 축을 중심으로 한 회전 각도 (단위: Radian)
	infoColumn.origoPos		= elem.column.origoPos;			// 기둥 중심 위치

	ACAPI_DisposeElemMemoHdls (&memo);

	// 벽 정보 저장
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

	// !!!
	// 단독 기둥의 경우 -> 이것을 먼저 개발할 것

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

	// 벽도 선택할 것 (벽 1개까지만 인식하면 되나?)
	// 영역 모프 있어야 함
	// 모프 -> zMax - zMin : 영역 높이에 해당함
	// 기둥의 꼭지점 4개를 찾고, 면 4개를 찾는다.



	/*
	- 메인 기둥과 보들과의 관계를 파악
		- 모든 보의 고도를 찾는다.
		- 그 중 제일 낮은 보의 고도를 찾아야 함
	- 1차 DG (보 단면)
		- 기둥 코너: 아웃코너판넬 (기둥 코너가 벽 속에 들어가면 생략)
		- 벽과 맞닿는 부분: 인코너판넬 (벽이 돌출된 경우에만)
		- 유로폼: 기둥 가운데, 그리고 벽면 (기둥 반쪽이 벽속에 매립된 경우)
	- 2차 DG (보 측면)
		- 가장 낮은 보 하단 라인과 유로폼 최상단 라인 간의 여백을 표현해야 함 (80mm 이상이어야 함)
		- UI에서 유효하지 않은 여백이 나오면 텍스트로 경고!
		- 모든 방향은 동일한 높이의 유로폼이 적용됨
	*/

	return	err;
}