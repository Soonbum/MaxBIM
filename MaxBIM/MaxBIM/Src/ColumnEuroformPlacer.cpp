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

API_Guid	structuralObject_forEuroformColumn;		// 구조 객체의 GUID

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

// 벽-기둥 DG 항목 인덱스 저장 (길이를 표현하는 Edit컨트롤)
static short	HLEN_DOWN, VLEN_DOWN;				// 가로(벽 아래/왼쪽), 세로(벽 아래/왼쪽)
static short	HLEN_UP, VLEN_UP;					// 가로(벽 위/오른쪽), 세로(벽 위/오른쪽)
static short	HLEN_LT, VLEN_LT;					// 좌상단 가로/세로 길이
static short	HLEN_RT, VLEN_RT;					// 우상단 가로/세로 길이
static short	HLEN_LB, VLEN_LB;					// 좌하단 가로/세로 길이
static short	HLEN_RB, VLEN_RB;					// 우하단 가로/세로 길이
static short	LEN_T1, LEN_T2, LEN_B1, LEN_B2;		// T1, T2, B1, B2
static short	LEN_L1, LEN_L2, LEN_R1, LEN_R2;		// L1, L2, R1, R2
static short	LEN_Lin1_C, LEN_Lin1_W;				// 왼쪽 인코너 셀 1 (위) - 기둥, 벽측 길이
static short	LEN_Lin2_C, LEN_Lin2_W;				// 왼쪽 인코너 셀 2 (아래) - 기둥, 벽측 길이
static short	LEN_Rin1_C, LEN_Rin1_W;				// 오른쪽 인코너 셀 1 (위) - 기둥, 벽측 길이
static short	LEN_Rin2_C, LEN_Rin2_W;				// 오른쪽 인코너 셀 2 (아래) - 기둥, 벽측 길이
static short	LEN_W1, LEN_W2, LEN_W3, LEN_W4;		// 벽 반대쪽 셀 길이

// 추가/삭제 버튼 인덱스 저장
static short	EUROFORM_BUTTON_TOP;
static short	EUROFORM_BUTTON_BOTTOM;
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


// 기둥에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnColumn (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx, yy;
	short			result;
	double			dx, dy, angle;
	double			lowestBeamBottomLevel;
	API_Coord		axisPoint, rotatedPoint, unrotatedPoint;
	API_Coord		unrotatedPoint1, unrotatedPoint2;
	bool			bFoundOneLine;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		columns;
	GS::Array<API_Guid>		walls;
	GS::Array<API_Guid>		beams;
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
	double					workLevel_beam;


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 기둥 (1개), 기둥 측면을 덮는 모프 (1개)\n옵션 선택: 기둥과 맞닿거나 간섭하는 벽(1개), 기둥과 맞닿는 보 (다수)", true);
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
	structuralObject_forEuroformColumn = elem.header.guid;
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

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_column = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (yy = 0 ; yy < (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
		if (storyInfo.data [0][yy].index == infoColumn.floorInd) {
			workLevel_column = storyInfo.data [0][yy].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	infoColumn.bottomOffset += workLevel_column;
	infoColumn.topOffset += workLevel_column;

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
		infoWall.angle			= elem.wall.angle;			// 회전 각도 (단위: Radian)

		infoWall.begC.x			= elem.wall.begC.x;			// 시작점 X
		infoWall.begC.y			= elem.wall.begC.y;			// 시작점 Y
		infoWall.endC.x			= elem.wall.endC.x;			// 끝점 X
		infoWall.endC.y			= elem.wall.endC.y;			// 끝점 Y

		infoWall.nCoords		= elem.wall.poly.nCoords;	// 정점 개수
		
		// 정점 좌표를 저장함
		for (xx = 1 ; xx <= elem.wall.poly.nCoords ; ++xx) {
			infoWall.poly [xx].x = memo.coords [0][xx].x;
			infoWall.poly [xx].y = memo.coords [0][xx].y;
		}

		// 벽면 시작점, 끝점 찾기
		bFoundOneLine = false;
		for (xx = 1 ; xx < infoWall.nCoords ; ++xx) {
			if (GetDistance (infoWall.poly [xx], infoWall.poly [xx+1]) > (infoWall.wallThk + EPS)) {
				if (bFoundOneLine == false) {
					if (GetDistance (infoWall.begC, infoWall.poly [xx]) < GetDistance (infoWall.begC, infoWall.poly [xx+1])) {
						infoWall.begC_1.x = infoWall.poly [xx].x;
						infoWall.begC_1.y = infoWall.poly [xx].y;
						infoWall.endC_1.x = infoWall.poly [xx+1].x;
						infoWall.endC_1.y = infoWall.poly [xx+1].y;
					} else {
						infoWall.begC_1.x = infoWall.poly [xx+1].x;
						infoWall.begC_1.y = infoWall.poly [xx+1].y;
						infoWall.endC_1.x = infoWall.poly [xx].x;
						infoWall.endC_1.y = infoWall.poly [xx].y;
					}
					bFoundOneLine = true;
				} else {
					if (GetDistance (infoWall.begC, infoWall.poly [xx]) < GetDistance (infoWall.begC, infoWall.poly [xx+1])) {
						infoWall.begC_2.x = infoWall.poly [xx].x;
						infoWall.begC_2.y = infoWall.poly [xx].y;
						infoWall.endC_2.x = infoWall.poly [xx+1].x;
						infoWall.endC_2.y = infoWall.poly [xx+1].y;
					} else {
						infoWall.begC_2.x = infoWall.poly [xx+1].x;
						infoWall.begC_2.y = infoWall.poly [xx+1].y;
						infoWall.endC_2.x = infoWall.poly [xx].x;
						infoWall.endC_2.y = infoWall.poly [xx].y;
					}
				}
			}
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 보 정보 저장
	nInterfereBeams = (short)nBeams;

	for (xx = 0 ; xx < nInterfereBeams ; ++xx) {
		infoOtherBeams [xx].guid = beams.Pop ();

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = infoOtherBeams [xx].guid;
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		infoOtherBeams [xx].valid		= true;						// 정보의 유효성
		infoOtherBeams [xx].floorInd	= elem.header.floorInd;		// 층 인덱스
		infoOtherBeams [xx].height		= elem.beam.height;			// 보 높이
		infoOtherBeams [xx].width		= elem.beam.width;			// 보 너비
		infoOtherBeams [xx].offset		= elem.beam.offset;			// 보 중심으로부터 보의 레퍼런스 라인의 오프셋입니다.
		infoOtherBeams [xx].level		= elem.beam.level;			// 바닥 레벨에 대한 보의 위쪽면 높이입니다.
		infoOtherBeams [xx].begC		= elem.beam.begC;			// 보 시작 좌표
		infoOtherBeams [xx].endC		= elem.beam.endC;			// 보 끝 좌표

		ACAPI_DisposeElemMemoHdls (&memo);

		// 작업 층 높이 반영
		BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
		workLevel_beam = 0.0;
		ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
		for (yy = 0 ; yy < (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
			if (storyInfo.data [0][yy].index == infoOtherBeams [xx].floorInd) {
				workLevel_beam = storyInfo.data [0][yy].level;
				break;
			}
		}
		BMKillHandle ((GSHandle *) &storyInfo.data);

		infoOtherBeams [xx].level += workLevel_beam;
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
	placingZone.initCells (&placingZone);

	// 간섭 보 정보 초기화
	placingZone.bInterfereBeam = false;
	placingZone.nInterfereBeams = 0;

	for (xx = 0 ; xx < 4 ; ++xx) {
		placingZone.bottomLevelOfBeams [xx] = 0.0;
		placingZone.bExistBeams [xx] = false;
	}

	// 영역 정보 중 간섭 보 관련 정보 업데이트
	if (nInterfereBeams > 0) {
		placingZone.bInterfereBeam = true;
		placingZone.nInterfereBeams = nInterfereBeams;
			
		for (xx = 0 ; xx < 4 ; ++xx) {
			axisPoint.x = placingZone.origoPos.x;
			axisPoint.y = placingZone.origoPos.y;

			if (infoOtherBeams [xx].valid == false)
				continue;

			rotatedPoint.x = infoOtherBeams [xx].begC.x;
			rotatedPoint.y = infoOtherBeams [xx].begC.y;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, -RadToDegree (placingZone.angle));

			// 기둥의 동/서/남/북 방향에 있는 보의 하단 레벨을 저장함
			if ( (unrotatedPoint.x <= (placingZone.origoPos.x + placingZone.coreWidth/2 + placingZone.venThick)) && (unrotatedPoint.x >= (placingZone.origoPos.x - placingZone.coreWidth/2 - placingZone.venThick)) && (unrotatedPoint.y >= (placingZone.origoPos.y + placingZone.coreDepth/2 + placingZone.venThick)) ) {
				placingZone.bottomLevelOfBeams [NORTH] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
				placingZone.bExistBeams [NORTH] = true;
				placingZone.beams [NORTH] = infoOtherBeams [xx];
			}
			if ( (unrotatedPoint.x <= (placingZone.origoPos.x + placingZone.coreWidth/2 + placingZone.venThick)) && (unrotatedPoint.x >= (placingZone.origoPos.x - placingZone.coreWidth/2 - placingZone.venThick)) && (unrotatedPoint.y <= (placingZone.origoPos.y - placingZone.coreDepth/2 - placingZone.venThick)) ) {
				placingZone.bottomLevelOfBeams [SOUTH] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
				placingZone.bExistBeams [SOUTH] = true;
				placingZone.beams [SOUTH] = infoOtherBeams [xx];
			}
			if ( (unrotatedPoint.y <= (placingZone.origoPos.y + placingZone.coreDepth/2 + placingZone.venThick)) && (unrotatedPoint.y >= (placingZone.origoPos.y - placingZone.coreDepth/2 - placingZone.venThick)) && (unrotatedPoint.x >= (placingZone.origoPos.x + placingZone.coreWidth/2 + placingZone.venThick)) ) {
				placingZone.bottomLevelOfBeams [EAST] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
				placingZone.bExistBeams [EAST] = true;
				placingZone.beams [EAST] = infoOtherBeams [xx];
			}
			if ( (unrotatedPoint.y <= (placingZone.origoPos.y + placingZone.coreDepth/2 + placingZone.venThick)) && (unrotatedPoint.y >= (placingZone.origoPos.y - placingZone.coreDepth/2 - placingZone.venThick)) && (unrotatedPoint.x <= (placingZone.origoPos.x - placingZone.coreWidth/2 - placingZone.venThick)) ) {
				placingZone.bottomLevelOfBeams [WEST] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
				placingZone.bExistBeams [WEST] = true;
				placingZone.beams [WEST] = infoOtherBeams [xx];
			}
		}
	}

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


	// 단독 기둥의 경우
	if (nWalls == 0) {

FIRST_SOLE_COLUMN:
	
		// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32514, ACAPI_GetOwnResModule (), columnPlacerHandler_soleColumn_1, 0);

		if (result == DG_CANCEL)
			return err;

		// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정합니다.
		clickedOKButton = false;
		clickedPrevButton = false;
		result = DGBlankModalDialog (700, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, columnPlacerHandler_soleColumn_2, 0);
	
		// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
		if (clickedPrevButton == true)
			goto FIRST_SOLE_COLUMN;

		// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
		if (clickedOKButton != true)
			return err;

		// 1, 2번째 다이얼로그를 통해 입력된 데이터를 기반으로 객체를 배치
		for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
			placingZone.cellsLT [xx].guid = placingZone.placeLibPart (placingZone.cellsLT [xx]);
			elemList.Push (placingZone.cellsLT [xx].guid);
			placingZone.cellsRT [xx].guid = placingZone.placeLibPart (placingZone.cellsRT [xx]);
			elemList.Push (placingZone.cellsRT [xx].guid);
			placingZone.cellsLB [xx].guid = placingZone.placeLibPart (placingZone.cellsLB [xx]);
			elemList.Push (placingZone.cellsLB [xx].guid);
			placingZone.cellsRB [xx].guid = placingZone.placeLibPart (placingZone.cellsRB [xx]);
			elemList.Push (placingZone.cellsRB [xx].guid);

			placingZone.cellsT1 [xx].guid = placingZone.placeLibPart (placingZone.cellsT1 [xx]);
			elemList.Push (placingZone.cellsT1 [xx].guid);
			placingZone.cellsT2 [xx].guid = placingZone.placeLibPart (placingZone.cellsT2 [xx]);
			elemList.Push (placingZone.cellsT2 [xx].guid);
			placingZone.cellsL1 [xx].guid = placingZone.placeLibPart (placingZone.cellsL1 [xx]);
			elemList.Push (placingZone.cellsL1 [xx].guid);
			placingZone.cellsL2 [xx].guid = placingZone.placeLibPart (placingZone.cellsL2 [xx]);
			elemList.Push (placingZone.cellsL2 [xx].guid);
			placingZone.cellsR1 [xx].guid = placingZone.placeLibPart  (placingZone.cellsR1 [xx]);
			elemList.Push (placingZone.cellsR1 [xx].guid);
			placingZone.cellsR2 [xx].guid = placingZone.placeLibPart (placingZone.cellsR2 [xx]);
			elemList.Push (placingZone.cellsR2 [xx].guid);
			placingZone.cellsB1 [xx].guid = placingZone.placeLibPart (placingZone.cellsB1 [xx]);
			elemList.Push (placingZone.cellsB1 [xx].guid);
			placingZone.cellsB2 [xx].guid = placingZone.placeLibPart (placingZone.cellsB2 [xx]);
			elemList.Push (placingZone.cellsB2 [xx].guid);
		}

		// 나머지 영역 채우기 - 합판, 목재
		err = placingZone.fillRestAreas_soleColumn (&placingZone);

	// 벽체와 맞닿거나 벽체 속 기둥인 경우
	} else {
		// 벽 라인이 회전된 기둥을 기준으로 가로 방향인지, 세로 방향인지 판단함 (벽 각도가 0 혹은 180도이면 가로, 90 혹은 -90도이면 세로)
		axisPoint.x = placingZone.origoPos.x;
		axisPoint.y = placingZone.origoPos.y;

		rotatedPoint.x = infoWall.begC.x;
		rotatedPoint.y = infoWall.begC.y;
		unrotatedPoint1 = getUnrotatedPoint (rotatedPoint, axisPoint, -RadToDegree (placingZone.angle));
		rotatedPoint.x = infoWall.endC.x;
		rotatedPoint.y = infoWall.endC.y;
		unrotatedPoint2 = getUnrotatedPoint (rotatedPoint, axisPoint, -RadToDegree (placingZone.angle));

		dx = unrotatedPoint2.x - unrotatedPoint1.x;
		dy = unrotatedPoint2.y - unrotatedPoint1.y;
		angle = RadToDegree (atan2 (dy, dx));
		if ( abs(abs (angle) - 90.0) < EPS )
			placingZone.bWallHorizontalDirected = false;
		else
			placingZone.bWallHorizontalDirected = true;

		// 벽의 위치 저장
		rotatedPoint.x = infoWall.begC_1.x;
		rotatedPoint.y = infoWall.begC_1.y;
		unrotatedPoint1 = getUnrotatedPoint (rotatedPoint, axisPoint, -RadToDegree (placingZone.angle));

		rotatedPoint.x = infoWall.begC_2.x;
		rotatedPoint.y = infoWall.begC_2.y;
		unrotatedPoint2 = getUnrotatedPoint (rotatedPoint, axisPoint, -RadToDegree (placingZone.angle));

		if (placingZone.bWallHorizontalDirected == true) {
			placingZone.posTopWallLine = max (unrotatedPoint1.y, unrotatedPoint2.y);
			placingZone.posBottomWallLine = min (unrotatedPoint1.y, unrotatedPoint2.y);
			placingZone.posTopColumnLine = placingZone.origoPos.y + placingZone.coreDepth/2 + placingZone.venThick;
			placingZone.posBottomColumnLine = placingZone.origoPos.y - placingZone.coreDepth/2 - placingZone.venThick;
		} else {
			placingZone.posTopWallLine = max (unrotatedPoint1.x, unrotatedPoint2.x);
			placingZone.posBottomWallLine = min (unrotatedPoint1.x, unrotatedPoint2.x);
			placingZone.posTopColumnLine = placingZone.origoPos.x + placingZone.coreWidth/2 + placingZone.venThick;
			placingZone.posBottomColumnLine = placingZone.origoPos.x - placingZone.coreWidth/2 - placingZone.venThick;
		}

		// 벽과 기둥과의 관계 정리
		placingZone.relationCase = 0;
		if ( (placingZone.posTopWallLine - placingZone.posBottomWallLine) > EPS && abs (placingZone.posBottomWallLine - placingZone.posTopColumnLine) < EPS && (placingZone.posTopColumnLine - placingZone.posBottomColumnLine) > EPS )
			placingZone.relationCase = 1;	// CASE 1. 기둥 바로 위에 벽이 붙은 경우
		if ( (placingZone.posTopWallLine - placingZone.posTopColumnLine) > EPS && (placingZone.posTopColumnLine - placingZone.posBottomWallLine) > EPS && (placingZone.posBottomWallLine - placingZone.posBottomColumnLine) > EPS )
			placingZone.relationCase = 2;	// CASE 2. 기둥이 벽 아래에 조금 들어간 경우
		if ( abs (placingZone.posTopWallLine - placingZone.posTopColumnLine) < EPS && (placingZone.posTopColumnLine - placingZone.posBottomWallLine) > EPS && (placingZone.posBottomWallLine - placingZone.posBottomColumnLine) > EPS )
			placingZone.relationCase = 3;	// CASE 3. 기둥 위쪽이 벽 위쪽과 일치한 경우
		if ( (placingZone.posTopColumnLine - placingZone.posTopWallLine) > EPS && (placingZone.posTopWallLine - placingZone.posBottomWallLine) > EPS && (placingZone.posBottomWallLine - placingZone.posBottomColumnLine) > EPS )
			placingZone.relationCase = 4;	// CASE 4. 기둥이 벽을 삼킨 경우
		if ( (placingZone.posTopColumnLine - placingZone.posTopWallLine) > EPS && (placingZone.posTopWallLine - placingZone.posBottomWallLine) > EPS && abs (placingZone.posBottomWallLine - placingZone.posBottomColumnLine) < EPS )
			placingZone.relationCase = 5;	// CASE 5. 기둥 아래쪽이 벽 아래쪽과 일치한 경우
		if ( (placingZone.posTopColumnLine - placingZone.posTopWallLine) > EPS && (placingZone.posTopWallLine - placingZone.posBottomColumnLine) > EPS && (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) > EPS )
			placingZone.relationCase = 6;	// CASE 6. 기둥이 벽 위에 조금 들어간 경우
		if ( (placingZone.posTopColumnLine - placingZone.posBottomColumnLine) > EPS && abs (placingZone.posBottomColumnLine - placingZone.posTopWallLine) < EPS && (placingZone.posTopWallLine - placingZone.posBottomWallLine) > EPS )
			placingZone.relationCase = 7;	// CASE 7. 기둥 바로 아래에 벽이 붙은 경우

		// 단독 기둥의 경우
		if (placingZone.relationCase == 0)
			goto FIRST_SOLE_COLUMN;

FIRST_WALL_COLUMN:
	
		// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32515, ACAPI_GetOwnResModule (), columnPlacerHandler_wallColumn_1, 0);

		if (result == DG_CANCEL)
			return err;

		// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정합니다.
		clickedOKButton = false;
		clickedPrevButton = false;
		result = DGBlankModalDialog (700, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, columnPlacerHandler_wallColumn_2, 0);

		// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
		if (clickedPrevButton == true)
			goto FIRST_WALL_COLUMN;

		// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
		if (clickedOKButton != true)
			return err;

		// 1, 2번째 다이얼로그를 통해 입력된 데이터를 기반으로 객체를 배치
		for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
			placingZone.cellsLT [xx].guid = placingZone.placeLibPart (placingZone.cellsLT [xx]);
			elemList.Push (placingZone.cellsLT [xx].guid);
			placingZone.cellsRT [xx].guid = placingZone.placeLibPart (placingZone.cellsRT [xx]);
			elemList.Push (placingZone.cellsRT [xx].guid);
			placingZone.cellsLB [xx].guid = placingZone.placeLibPart (placingZone.cellsLB [xx]);
			elemList.Push (placingZone.cellsLB [xx].guid);
			placingZone.cellsRB [xx].guid = placingZone.placeLibPart (placingZone.cellsRB [xx]);
			elemList.Push (placingZone.cellsRB [xx].guid);

			placingZone.cellsT1 [xx].guid = placingZone.placeLibPart (placingZone.cellsT1 [xx]);
			elemList.Push (placingZone.cellsT1 [xx].guid);
			placingZone.cellsT2 [xx].guid = placingZone.placeLibPart (placingZone.cellsT2 [xx]);
			elemList.Push (placingZone.cellsT2 [xx].guid);
			placingZone.cellsL1 [xx].guid = placingZone.placeLibPart (placingZone.cellsL1 [xx]);
			elemList.Push (placingZone.cellsL1 [xx].guid);
			placingZone.cellsL2 [xx].guid = placingZone.placeLibPart (placingZone.cellsL2 [xx]);
			elemList.Push (placingZone.cellsL2 [xx].guid);
			placingZone.cellsR1 [xx].guid = placingZone.placeLibPart (placingZone.cellsR1 [xx]);
			elemList.Push (placingZone.cellsR1 [xx].guid);
			placingZone.cellsR2 [xx].guid = placingZone.placeLibPart (placingZone.cellsR2 [xx]);
			elemList.Push (placingZone.cellsR2 [xx].guid);
			placingZone.cellsB1 [xx].guid = placingZone.placeLibPart (placingZone.cellsB1 [xx]);
			elemList.Push (placingZone.cellsB1 [xx].guid);
			placingZone.cellsB2 [xx].guid = placingZone.placeLibPart (placingZone.cellsB2 [xx]);
			elemList.Push (placingZone.cellsB2 [xx].guid);

			placingZone.cellsLin1 [xx].guid = placingZone.placeLibPart (placingZone.cellsLin1 [xx]);
			elemList.Push (placingZone.cellsLin1 [xx].guid);
			placingZone.cellsLin2 [xx].guid = placingZone.placeLibPart (placingZone.cellsLin2 [xx]);
			elemList.Push (placingZone.cellsLin2 [xx].guid);
			placingZone.cellsRin1 [xx].guid = placingZone.placeLibPart (placingZone.cellsRin1 [xx]);
			elemList.Push (placingZone.cellsRin1 [xx].guid);
			placingZone.cellsRin2 [xx].guid = placingZone.placeLibPart (placingZone.cellsRin2 [xx]);
			elemList.Push (placingZone.cellsRin2 [xx].guid);

			placingZone.cellsW1 [xx].guid = placingZone.placeLibPart (placingZone.cellsW1 [xx]);
			elemList.Push (placingZone.cellsW1 [xx].guid);
			placingZone.cellsW2 [xx].guid = placingZone.placeLibPart (placingZone.cellsW2 [xx]);
			elemList.Push (placingZone.cellsW2 [xx].guid);
			placingZone.cellsW3 [xx].guid = placingZone.placeLibPart (placingZone.cellsW3 [xx]);
			elemList.Push (placingZone.cellsW3 [xx].guid);
			placingZone.cellsW4 [xx].guid = placingZone.placeLibPart (placingZone.cellsW4 [xx]);
			elemList.Push (placingZone.cellsW4 [xx].guid);
		}

		// 나머지 영역 채우기 - 합판, 목재
		err = placingZone.fillRestAreas_wallColumn (&placingZone);
	}

	// 결과물 전체 그룹화
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

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// Cell 배열을 초기화함
void	ColumnPlacingZone::initCells (ColumnPlacingZone* placingZone)
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

		placingZone->cellsRT [xx].objType = NONE;
		placingZone->cellsRT [xx].leftBottomX = 0.0;
		placingZone->cellsRT [xx].leftBottomY = 0.0;
		placingZone->cellsRT [xx].leftBottomZ = 0.0;
		placingZone->cellsRT [xx].ang = 0.0;
		placingZone->cellsRT [xx].horLen = 0.0;
		placingZone->cellsRT [xx].verLen = 0.0;
		placingZone->cellsRT [xx].height = 0.0;

		placingZone->cellsLB [xx].objType = NONE;
		placingZone->cellsLB [xx].leftBottomX = 0.0;
		placingZone->cellsLB [xx].leftBottomY = 0.0;
		placingZone->cellsLB [xx].leftBottomZ = 0.0;
		placingZone->cellsLB [xx].ang = 0.0;
		placingZone->cellsLB [xx].horLen = 0.0;
		placingZone->cellsLB [xx].verLen = 0.0;
		placingZone->cellsLB [xx].height = 0.0;

		placingZone->cellsRB [xx].objType = NONE;
		placingZone->cellsRB [xx].leftBottomX = 0.0;
		placingZone->cellsRB [xx].leftBottomY = 0.0;
		placingZone->cellsRB [xx].leftBottomZ = 0.0;
		placingZone->cellsRB [xx].ang = 0.0;
		placingZone->cellsRB [xx].horLen = 0.0;
		placingZone->cellsRB [xx].verLen = 0.0;
		placingZone->cellsRB [xx].height = 0.0;

		placingZone->cellsT1 [xx].objType = NONE;
		placingZone->cellsT1 [xx].leftBottomX = 0.0;
		placingZone->cellsT1 [xx].leftBottomY = 0.0;
		placingZone->cellsT1 [xx].leftBottomZ = 0.0;
		placingZone->cellsT1 [xx].ang = 0.0;
		placingZone->cellsT1 [xx].horLen = 0.0;
		placingZone->cellsT1 [xx].verLen = 0.0;
		placingZone->cellsT1 [xx].height = 0.0;

		placingZone->cellsT2 [xx].objType = NONE;
		placingZone->cellsT2 [xx].leftBottomX = 0.0;
		placingZone->cellsT2 [xx].leftBottomY = 0.0;
		placingZone->cellsT2 [xx].leftBottomZ = 0.0;
		placingZone->cellsT2 [xx].ang = 0.0;
		placingZone->cellsT2 [xx].horLen = 0.0;
		placingZone->cellsT2 [xx].verLen = 0.0;
		placingZone->cellsT2 [xx].height = 0.0;

		placingZone->cellsL1 [xx].objType = NONE;
		placingZone->cellsL1 [xx].leftBottomX = 0.0;
		placingZone->cellsL1 [xx].leftBottomY = 0.0;
		placingZone->cellsL1 [xx].leftBottomZ = 0.0;
		placingZone->cellsL1 [xx].ang = 0.0;
		placingZone->cellsL1 [xx].horLen = 0.0;
		placingZone->cellsL1 [xx].verLen = 0.0;
		placingZone->cellsL1 [xx].height = 0.0;

		placingZone->cellsL2 [xx].objType = NONE;
		placingZone->cellsL2 [xx].leftBottomX = 0.0;
		placingZone->cellsL2 [xx].leftBottomY = 0.0;
		placingZone->cellsL2 [xx].leftBottomZ = 0.0;
		placingZone->cellsL2 [xx].ang = 0.0;
		placingZone->cellsL2 [xx].horLen = 0.0;
		placingZone->cellsL2 [xx].verLen = 0.0;
		placingZone->cellsL2 [xx].height = 0.0;

		placingZone->cellsR1 [xx].objType = NONE;
		placingZone->cellsR1 [xx].leftBottomX = 0.0;
		placingZone->cellsR1 [xx].leftBottomY = 0.0;
		placingZone->cellsR1 [xx].leftBottomZ = 0.0;
		placingZone->cellsR1 [xx].ang = 0.0;
		placingZone->cellsR1 [xx].horLen = 0.0;
		placingZone->cellsR1 [xx].verLen = 0.0;
		placingZone->cellsR1 [xx].height = 0.0;

		placingZone->cellsR2 [xx].objType = NONE;
		placingZone->cellsR2 [xx].leftBottomX = 0.0;
		placingZone->cellsR2 [xx].leftBottomY = 0.0;
		placingZone->cellsR2 [xx].leftBottomZ = 0.0;
		placingZone->cellsR2 [xx].ang = 0.0;
		placingZone->cellsR2 [xx].horLen = 0.0;
		placingZone->cellsR2 [xx].verLen = 0.0;
		placingZone->cellsR2 [xx].height = 0.0;

		placingZone->cellsB1 [xx].objType = NONE;
		placingZone->cellsB1 [xx].leftBottomX = 0.0;
		placingZone->cellsB1 [xx].leftBottomY = 0.0;
		placingZone->cellsB1 [xx].leftBottomZ = 0.0;
		placingZone->cellsB1 [xx].ang = 0.0;
		placingZone->cellsB1 [xx].horLen = 0.0;
		placingZone->cellsB1 [xx].verLen = 0.0;
		placingZone->cellsB1 [xx].height = 0.0;

		placingZone->cellsB2 [xx].objType = NONE;
		placingZone->cellsB2 [xx].leftBottomX = 0.0;
		placingZone->cellsB2 [xx].leftBottomY = 0.0;
		placingZone->cellsB2 [xx].leftBottomZ = 0.0;
		placingZone->cellsB2 [xx].ang = 0.0;
		placingZone->cellsB2 [xx].horLen = 0.0;
		placingZone->cellsB2 [xx].verLen = 0.0;
		placingZone->cellsB2 [xx].height = 0.0;

		placingZone->cellsLin1 [xx].objType = NONE;
		placingZone->cellsLin1 [xx].leftBottomX = 0.0;
		placingZone->cellsLin1 [xx].leftBottomY = 0.0;
		placingZone->cellsLin1 [xx].leftBottomZ = 0.0;
		placingZone->cellsLin1 [xx].ang = 0.0;
		placingZone->cellsLin1 [xx].horLen = 0.0;
		placingZone->cellsLin1 [xx].verLen = 0.0;
		placingZone->cellsLin1 [xx].height = 0.0;

		placingZone->cellsLin2 [xx].objType = NONE;
		placingZone->cellsLin2 [xx].leftBottomX = 0.0;
		placingZone->cellsLin2 [xx].leftBottomY = 0.0;
		placingZone->cellsLin2 [xx].leftBottomZ = 0.0;
		placingZone->cellsLin2 [xx].ang = 0.0;
		placingZone->cellsLin2 [xx].horLen = 0.0;
		placingZone->cellsLin2 [xx].verLen = 0.0;
		placingZone->cellsLin2 [xx].height = 0.0;

		placingZone->cellsRin1 [xx].objType = NONE;
		placingZone->cellsRin1 [xx].leftBottomX = 0.0;
		placingZone->cellsRin1 [xx].leftBottomY = 0.0;
		placingZone->cellsRin1 [xx].leftBottomZ = 0.0;
		placingZone->cellsRin1 [xx].ang = 0.0;
		placingZone->cellsRin1 [xx].horLen = 0.0;
		placingZone->cellsRin1 [xx].verLen = 0.0;
		placingZone->cellsRin1 [xx].height = 0.0;

		placingZone->cellsRin2 [xx].objType = NONE;
		placingZone->cellsRin2 [xx].leftBottomX = 0.0;
		placingZone->cellsRin2 [xx].leftBottomY = 0.0;
		placingZone->cellsRin2 [xx].leftBottomZ = 0.0;
		placingZone->cellsRin2 [xx].ang = 0.0;
		placingZone->cellsRin2 [xx].horLen = 0.0;
		placingZone->cellsRin2 [xx].verLen = 0.0;
		placingZone->cellsRin2 [xx].height = 0.0;

		placingZone->cellsW1 [xx].objType = NONE;
		placingZone->cellsW1 [xx].leftBottomX = 0.0;
		placingZone->cellsW1 [xx].leftBottomY = 0.0;
		placingZone->cellsW1 [xx].leftBottomZ = 0.0;
		placingZone->cellsW1 [xx].ang = 0.0;
		placingZone->cellsW1 [xx].horLen = 0.0;
		placingZone->cellsW1 [xx].verLen = 0.0;
		placingZone->cellsW1 [xx].height = 0.0;

		placingZone->cellsW2 [xx].objType = NONE;
		placingZone->cellsW2 [xx].leftBottomX = 0.0;
		placingZone->cellsW2 [xx].leftBottomY = 0.0;
		placingZone->cellsW2 [xx].leftBottomZ = 0.0;
		placingZone->cellsW2 [xx].ang = 0.0;
		placingZone->cellsW2 [xx].horLen = 0.0;
		placingZone->cellsW2 [xx].verLen = 0.0;
		placingZone->cellsW2 [xx].height = 0.0;

		placingZone->cellsW3 [xx].objType = NONE;
		placingZone->cellsW3 [xx].leftBottomX = 0.0;
		placingZone->cellsW3 [xx].leftBottomY = 0.0;
		placingZone->cellsW3 [xx].leftBottomZ = 0.0;
		placingZone->cellsW3 [xx].ang = 0.0;
		placingZone->cellsW3 [xx].horLen = 0.0;
		placingZone->cellsW3 [xx].verLen = 0.0;
		placingZone->cellsW3 [xx].height = 0.0;

		placingZone->cellsW4 [xx].objType = NONE;
		placingZone->cellsW4 [xx].leftBottomX = 0.0;
		placingZone->cellsW4 [xx].leftBottomY = 0.0;
		placingZone->cellsW4 [xx].leftBottomZ = 0.0;
		placingZone->cellsW4 [xx].ang = 0.0;
		placingZone->cellsW4 [xx].horLen = 0.0;
		placingZone->cellsW4 [xx].verLen = 0.0;
		placingZone->cellsW4 [xx].height = 0.0;
	}

	// 셀 개수 초기화
	placingZone->nCells = 0;
}

// 꼭대기에 셀 추가
void	ColumnPlacingZone::addTopCell (ColumnPlacingZone* target_zone)
{
	if (target_zone->nCells >= 20) return;

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

	target_zone->cellsLin1 [target_zone->nCells] = target_zone->cellsLin1 [target_zone->nCells - 1];
	target_zone->cellsLin2 [target_zone->nCells] = target_zone->cellsLin2 [target_zone->nCells - 1];
	target_zone->cellsRin1 [target_zone->nCells] = target_zone->cellsRin1 [target_zone->nCells - 1];
	target_zone->cellsRin2 [target_zone->nCells] = target_zone->cellsRin2 [target_zone->nCells - 1];

	target_zone->cellsW1 [target_zone->nCells] = target_zone->cellsW1 [target_zone->nCells - 1];
	target_zone->cellsW2 [target_zone->nCells] = target_zone->cellsW2 [target_zone->nCells - 1];
	target_zone->cellsW3 [target_zone->nCells] = target_zone->cellsW3 [target_zone->nCells - 1];
	target_zone->cellsW4 [target_zone->nCells] = target_zone->cellsW4 [target_zone->nCells - 1];

	target_zone->nCells ++;
}

// 꼭대기의 셀 삭제
void	ColumnPlacingZone::delTopCell (ColumnPlacingZone* target_zone)
{
	if (target_zone->nCells <= 1) return;

	target_zone->nCells --;
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	ColumnPlacingZone::alignPlacingZone_soleColumn (ColumnPlacingZone* placingZone)
{
	short	xx;
	double	formWidth, formHeight;
	
	// 높이를 아래부터 시작해서 재조정, 규격폼인지 아닌지 여부를 결정
	placingZone->cellsLT [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsRT [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsLB [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsRB [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsT1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsT2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsL1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsL2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsR1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsR2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsB1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsB2 [0].leftBottomZ = placingZone->bottomOffset;

	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {

		if (xx >= 1) {
			// 좌상단
			placingZone->cellsLT [xx].leftBottomZ = placingZone->cellsLT [xx-1].leftBottomZ + placingZone->cellsLT [xx-1].height;

			// 우상단
			placingZone->cellsRT [xx].leftBottomZ = placingZone->cellsRT [xx-1].leftBottomZ + placingZone->cellsRT [xx-1].height;

			// 좌하단
			placingZone->cellsLB [xx].leftBottomZ = placingZone->cellsLB [xx-1].leftBottomZ + placingZone->cellsLB [xx-1].height;

			// 우하단
			placingZone->cellsRB [xx].leftBottomZ = placingZone->cellsRB [xx-1].leftBottomZ + placingZone->cellsRB [xx-1].height;

			// 위쪽 1
			placingZone->cellsT1 [xx].leftBottomZ = placingZone->cellsT1 [xx-1].leftBottomZ + placingZone->cellsT1 [xx-1].height;

			// 위쪽 2
			placingZone->cellsT2 [xx].leftBottomZ = placingZone->cellsT2 [xx-1].leftBottomZ + placingZone->cellsT2 [xx-1].height;

			// 좌측 1
			placingZone->cellsL1 [xx].leftBottomZ = placingZone->cellsL1 [xx-1].leftBottomZ + placingZone->cellsL1 [xx-1].height;

			// 좌측 2
			placingZone->cellsL2 [xx].leftBottomZ = placingZone->cellsL2 [xx-1].leftBottomZ + placingZone->cellsL2 [xx-1].height;

			// 우측 1
			placingZone->cellsR1 [xx].leftBottomZ = placingZone->cellsR1 [xx-1].leftBottomZ + placingZone->cellsR1 [xx-1].height;

			// 우측 2
			placingZone->cellsR2 [xx].leftBottomZ = placingZone->cellsR2 [xx-1].leftBottomZ + placingZone->cellsR2 [xx-1].height;

			// 아래쪽 1
			placingZone->cellsB1 [xx].leftBottomZ = placingZone->cellsB1 [xx-1].leftBottomZ + placingZone->cellsB1 [xx-1].height;

			// 아래쪽 2
			placingZone->cellsB2 [xx].leftBottomZ = placingZone->cellsB2 [xx-1].leftBottomZ + placingZone->cellsB2 [xx-1].height;
		}

		// 위쪽 1
		formWidth = placingZone->cellsT1 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsT1 [xx].libPart.form.eu_hei;
		placingZone->cellsT1 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsT1 [xx].libPart.form.eu_stan_onoff = true;
		
		// 위쪽 2
		formWidth = placingZone->cellsT2 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsT2 [xx].libPart.form.eu_hei;
		placingZone->cellsT2 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsT2 [xx].libPart.form.eu_stan_onoff = true;

		// 좌측 1
		formWidth = placingZone->cellsL1 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsL1 [xx].libPart.form.eu_hei;
		placingZone->cellsL1 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsL1 [xx].libPart.form.eu_stan_onoff = true;

		// 좌측 2
		formWidth = placingZone->cellsL2 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsL2 [xx].libPart.form.eu_hei;
		placingZone->cellsL2 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsL2 [xx].libPart.form.eu_stan_onoff = true;

		// 우측 1
		formWidth = placingZone->cellsR1 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsR1 [xx].libPart.form.eu_hei;
		placingZone->cellsR1 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsR1 [xx].libPart.form.eu_stan_onoff = true;

		// 우측 2
		formWidth = placingZone->cellsR2 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsR2 [xx].libPart.form.eu_hei;
		placingZone->cellsR2 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsR2 [xx].libPart.form.eu_stan_onoff = true;

		// 아래쪽 1
		formWidth = placingZone->cellsB1 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsB1 [xx].libPart.form.eu_hei;
		placingZone->cellsB1 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsB1 [xx].libPart.form.eu_stan_onoff = true;

		// 아래쪽 2
		formWidth = placingZone->cellsB2 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsB2 [xx].libPart.form.eu_hei;
		placingZone->cellsB2 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsB2 [xx].libPart.form.eu_stan_onoff = true;
	}
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	ColumnPlacingZone::alignPlacingZone_wallColumn (ColumnPlacingZone* placingZone)
{
	short	xx;
	double	formWidth, formHeight;

	// 높이를 아래부터 시작해서 재조정, 규격폼인지 아닌지 여부를 결정
	placingZone->cellsLT [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsRT [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsLB [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsRB [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsT1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsT2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsL1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsL2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsR1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsR2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsB1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsB2 [0].leftBottomZ = placingZone->bottomOffset;

	placingZone->cellsLin1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsLin2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsRin1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsRin2 [0].leftBottomZ = placingZone->bottomOffset;

	placingZone->cellsW1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsW2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsW3 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsW4 [0].leftBottomZ = placingZone->bottomOffset;

	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {

		if (xx >= 1) {
			// 좌상단
			if ((HLEN_LT != 0) && (VLEN_LT != 0))
				placingZone->cellsLT [xx].leftBottomZ = placingZone->cellsLT [xx-1].leftBottomZ + placingZone->cellsLT [xx-1].height;

			// 우상단
			if ((HLEN_RT != 0) && (VLEN_RT != 0))
				placingZone->cellsRT [xx].leftBottomZ = placingZone->cellsRT [xx-1].leftBottomZ + placingZone->cellsRT [xx-1].height;

			// 좌하단
			if ((HLEN_LB != 0) && (VLEN_LB != 0))
				placingZone->cellsLB [xx].leftBottomZ = placingZone->cellsLB [xx-1].leftBottomZ + placingZone->cellsLB [xx-1].height;

			// 우하단
			if ((HLEN_RB != 0) && (VLEN_RB != 0))
				placingZone->cellsRB [xx].leftBottomZ = placingZone->cellsRB [xx-1].leftBottomZ + placingZone->cellsRB [xx-1].height;

			// T1
			if (LEN_T1 != 0)
				placingZone->cellsT1 [xx].leftBottomZ = placingZone->cellsT1 [xx-1].leftBottomZ + placingZone->cellsT1 [xx-1].height;

			// T2
			if (LEN_T2 != 0)
				placingZone->cellsT2 [xx].leftBottomZ = placingZone->cellsT2 [xx-1].leftBottomZ + placingZone->cellsT2 [xx-1].height;

			// B1
			if (LEN_B1 != 0)
				placingZone->cellsB1 [xx].leftBottomZ = placingZone->cellsB1 [xx-1].leftBottomZ + placingZone->cellsB1 [xx-1].height;

			// B2
			if (LEN_B2 != 0)
				placingZone->cellsB2 [xx].leftBottomZ = placingZone->cellsB2 [xx-1].leftBottomZ + placingZone->cellsB2 [xx-1].height;

			// L1
			if (LEN_L1 != 0)
				placingZone->cellsL1 [xx].leftBottomZ = placingZone->cellsL1 [xx-1].leftBottomZ + placingZone->cellsL1 [xx-1].height;

			// L2
			if (LEN_L2 != 0)
				placingZone->cellsL2 [xx].leftBottomZ = placingZone->cellsL2 [xx-1].leftBottomZ + placingZone->cellsL2 [xx-1].height;

			// R1
			if (LEN_R1 != 0)
				placingZone->cellsR1 [xx].leftBottomZ = placingZone->cellsR1 [xx-1].leftBottomZ + placingZone->cellsR1 [xx-1].height;

			// R2
			if (LEN_R2 != 0)
				placingZone->cellsR2 [xx].leftBottomZ = placingZone->cellsR2 [xx-1].leftBottomZ + placingZone->cellsR2 [xx-1].height;

			// 왼쪽 인코너 셀 1 (위)
			if (LEN_Lin1_C != 0)
				placingZone->cellsLin1 [xx].leftBottomZ = placingZone->cellsLin1 [xx-1].leftBottomZ + placingZone->cellsLin1 [xx-1].height;

			// 왼쪽 인코너 셀 2 (아래)
			if (LEN_Lin2_C != 0)
				placingZone->cellsLin2 [xx].leftBottomZ = placingZone->cellsLin2 [xx-1].leftBottomZ + placingZone->cellsLin2 [xx-1].height;

			// 오른쪽 인코너 셀 1 (위)
			if (LEN_Rin1_C != 0)
				placingZone->cellsRin1 [xx].leftBottomZ = placingZone->cellsRin1 [xx-1].leftBottomZ + placingZone->cellsRin1 [xx-1].height;

			// 오른쪽 인코너 셀 2 (아래)
			if (LEN_Rin2_C != 0)
				placingZone->cellsRin2 [xx].leftBottomZ = placingZone->cellsRin2 [xx-1].leftBottomZ + placingZone->cellsRin2 [xx-1].height;

			// W1
			if (LEN_W1 != 0)
				placingZone->cellsW1 [xx].leftBottomZ = placingZone->cellsW1 [xx-1].leftBottomZ + placingZone->cellsW1 [xx-1].height;

			// W2
			if (LEN_W2 != 0)
				placingZone->cellsW2 [xx].leftBottomZ = placingZone->cellsW2 [xx-1].leftBottomZ + placingZone->cellsW2 [xx-1].height;

			// W3
			if (LEN_W3 != 0)
				placingZone->cellsW3 [xx].leftBottomZ = placingZone->cellsW3 [xx-1].leftBottomZ + placingZone->cellsW3 [xx-1].height;

			// W4
			if (LEN_W4 != 0)
				placingZone->cellsW4 [xx].leftBottomZ = placingZone->cellsW4 [xx-1].leftBottomZ + placingZone->cellsW4 [xx-1].height;
		}

		// T1
		if (LEN_T1 != 0) {
			formWidth = placingZone->cellsT1 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsT1 [xx].libPart.form.eu_hei;
			placingZone->cellsT1 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsT1 [xx].libPart.form.eu_stan_onoff = true;
		}

		// T2
		if (LEN_T2 != 0) {
			formWidth = placingZone->cellsT2 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsT2 [xx].libPart.form.eu_hei;
			placingZone->cellsT2 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsT2 [xx].libPart.form.eu_stan_onoff = true;
		}

		// B1
		if (LEN_B1 != 0) {
			formWidth = placingZone->cellsB1 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsB1 [xx].libPart.form.eu_hei;
			placingZone->cellsB1 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsB1 [xx].libPart.form.eu_stan_onoff = true;
		}

		// B2
		if (LEN_B2 != 0) {
			formWidth = placingZone->cellsB2 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsB2 [xx].libPart.form.eu_hei;
			placingZone->cellsB2 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsB2 [xx].libPart.form.eu_stan_onoff = true;
		}

		// L1
		if (LEN_L1 != 0) {
			formWidth = placingZone->cellsL1 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsL1 [xx].libPart.form.eu_hei;
			placingZone->cellsL1 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsL1 [xx].libPart.form.eu_stan_onoff = true;
		}

		// L2
		if (LEN_L2 != 0) {
			formWidth = placingZone->cellsL2 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsL2 [xx].libPart.form.eu_hei;
			placingZone->cellsL2 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsL2 [xx].libPart.form.eu_stan_onoff = true;
		}

		// R1
		if (LEN_R1 != 0) {
			formWidth = placingZone->cellsR1 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsR1 [xx].libPart.form.eu_hei;
			placingZone->cellsR1 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsR1 [xx].libPart.form.eu_stan_onoff = true;
		}

		// R2
		if (LEN_R2 != 0) {
			formWidth = placingZone->cellsR2 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsR2 [xx].libPart.form.eu_hei;
			placingZone->cellsR2 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsR2 [xx].libPart.form.eu_stan_onoff = true;
		}

		// W1
		if (LEN_W1 != 0) {
			formWidth = placingZone->cellsW1 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsW1 [xx].libPart.form.eu_hei;
			placingZone->cellsW1 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsW1 [xx].libPart.form.eu_stan_onoff = true;
		}

		// W2
		if (LEN_W2 != 0) {
			formWidth = placingZone->cellsW2 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsW2 [xx].libPart.form.eu_hei;
			placingZone->cellsW2 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsW2 [xx].libPart.form.eu_stan_onoff = true;
		}

		// W3
		if (LEN_W3 != 0) {
			formWidth = placingZone->cellsW3 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsW3 [xx].libPart.form.eu_hei;
			placingZone->cellsW3 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsW3 [xx].libPart.form.eu_stan_onoff = true;
		}

		// W4
		if (LEN_W4 != 0) {
			formWidth = placingZone->cellsW4 [xx].libPart.form.eu_wid;
			formHeight = placingZone->cellsW4 [xx].libPart.form.eu_hei;
			placingZone->cellsW4 [xx].libPart.form.eu_stan_onoff = false;
			if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
				if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
					placingZone->cellsW4 [xx].libPart.form.eu_stan_onoff = true;
		}
	}
}

// 해당 셀 정보를 기반으로 라이브러리 배치
API_Guid	ColumnPlacingZone::placeLibPart (CellForColumn objInfo)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const	GS::uchar_t* gsmName = NULL;
	double	aParam;
	double	bParam;
	Int32	addParNum;

	double	validLength = 0.0;	// 유효한 길이인가?
	double	validWidth = 0.0;	// 유효한 너비인가?

	char	tempString [20];

	short		xx;
	API_StoryInfo	storyInfo;

	// GUID 변수 초기화
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// 라이브러리 이름 선택
	if (objInfo.objType == NONE)			return element.header.guid;
	if (objInfo.objType == EUROFORM)		gsmName = L("유로폼v2.0.gsm");
	if (objInfo.objType == INCORNER)		gsmName = L("인코너판넬v1.0.gsm");
	if (objInfo.objType == OUTCORNER)		gsmName = L("아웃코너판넬v1.0.gsm");
	if (objInfo.objType == PLYWOOD)			gsmName = L("합판v1.0.gsm");
	if (objInfo.objType == MAGIC_BAR)		gsmName = L("매직바v1.0.gsm");
	if (objInfo.objType == MAGIC_INCORNER)	gsmName = L("매직인코너v1.0.gsm");

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = objInfo.leftBottomX;
	element.object.pos.y = objInfo.leftBottomY;
	element.object.level = objInfo.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = objInfo.ang;
	element.header.floorInd = infoColumn.floorInd;

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoColumn.floorInd) {
			element.object.level -= storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	if (objInfo.objType == EUROFORM) {
		element.header.layer = layerInd_Euroform;

		// 규격품일 경우,
		if (objInfo.libPart.form.eu_stan_onoff == true) {
			// 규격폼 On/Off
			setParameterByName (&memo, "eu_stan_onoff", 1.0);	// 규격폼 On/Off

			// 너비
			sprintf (tempString, "%.0f", objInfo.libPart.form.eu_wid * 1000);
			setParameterByName (&memo, "eu_wid", tempString);

			// 높이
			sprintf (tempString, "%.0f", objInfo.libPart.form.eu_hei * 1000);
			setParameterByName (&memo, "eu_hei", tempString);

		// 비규격품일 경우,
		} else {
			setParameterByName (&memo, "eu_stan_onoff", 0.0);	// 규격폼 On/Off
			setParameterByName (&memo, "eu_wid2", objInfo.libPart.form.eu_wid2);	// 너비
			setParameterByName (&memo, "eu_hei2", objInfo.libPart.form.eu_hei2);	// 높이
		}

		// 설치방향
		if (objInfo.libPart.form.u_ins_wall == true) {
			strcpy (tempString, "벽세우기");
			if (objInfo.libPart.form.eu_stan_onoff == true) {
				validWidth = objInfo.libPart.form.eu_wid;
				validLength = objInfo.libPart.form.eu_hei;
			} else {
				validWidth = objInfo.libPart.form.eu_wid2;
				validLength = objInfo.libPart.form.eu_hei2;
			}
		} else {
			strcpy (tempString, "벽눕히기");
			if (objInfo.libPart.form.eu_stan_onoff == true) {
				element.object.pos.x += ( objInfo.libPart.form.eu_hei * cos(objInfo.ang) );
				element.object.pos.y += ( objInfo.libPart.form.eu_hei * sin(objInfo.ang) );
				validWidth = objInfo.libPart.form.eu_hei;
				validLength = objInfo.libPart.form.eu_wid;
			} else {
				element.object.pos.x += ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
				element.object.pos.y += ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
				validWidth = objInfo.libPart.form.eu_hei2;
				validLength = objInfo.libPart.form.eu_wid2;
			}
		}
		setParameterByName (&memo, "u_ins", tempString);

		// 회전X
		setParameterByName (&memo, "ang_x", DegreeToRad (90.0));	// 회전X

	} else if (objInfo.objType == INCORNER) {
		element.header.layer = layerInd_Incorner;
		setParameterByName (&memo, "wid_s", objInfo.libPart.incorner.wid_s);	// 가로(빨강)
		setParameterByName (&memo, "leng_s", objInfo.libPart.incorner.leng_s);	// 세로(파랑)
		setParameterByName (&memo, "hei_s", objInfo.libPart.incorner.hei_s);	// 높이
		setParameterByName (&memo, "dir_s", "세우기");							// 설치방향

		validWidth = objInfo.libPart.incorner.leng_s + objInfo.libPart.incorner.wid_s;
		validLength = objInfo.libPart.incorner.hei_s;
	} else if (objInfo.objType == OUTCORNER) {
		element.header.layer = layerInd_Outcorner;
		setParameterByName (&memo, "wid_s", objInfo.libPart.outcorner.wid_s);	// 가로(빨강)
		setParameterByName (&memo, "leng_s", objInfo.libPart.outcorner.leng_s);	// 세로(파랑)
		setParameterByName (&memo, "hei_s", objInfo.libPart.outcorner.hei_s);	// 높이
		setParameterByName (&memo, "dir_s", "세우기");							// 설치방향

		validWidth = objInfo.libPart.outcorner.leng_s + objInfo.libPart.outcorner.wid_s;
		validLength = objInfo.libPart.outcorner.hei_s;
	} else if (objInfo.objType == PLYWOOD) {
		element.header.layer = layerInd_Plywood;
		setParameterByName (&memo, "p_stan", "비규격");							// 규격
		setParameterByName (&memo, "w_dir", "벽세우기");						// 설치방향
		setParameterByName (&memo, "p_thk", "11.5T");							// 두께
		setParameterByName (&memo, "p_wid", objInfo.libPart.plywood.p_wid);		// 가로
		setParameterByName (&memo, "p_leng", objInfo.libPart.plywood.p_leng);	// 세로
		setParameterByName (&memo, "sogak", 0.0);								// 제작틀 OFF
		setParameterByName (&memo, "p_ang", DegreeToRad (0.0));					// 각도

		validLength = objInfo.libPart.plywood.p_leng;
		validWidth = objInfo.libPart.plywood.p_wid;
	} else if (objInfo.objType == MAGIC_BAR) {
		element.header.layer = layerInd_MagicBar;
		setParameterByName (&memo, "ZZYZX", objInfo.libPart.mbar.ZZYZX);							// 높이
		setParameterByName (&memo, "angX", objInfo.libPart.mbar.angX);								// 회전X
		setParameterByName (&memo, "angY", objInfo.libPart.mbar.angY);								// 회전Y
		setParameterByName (&memo, "bPlywood", objInfo.libPart.mbar.bPlywood);						// 합판 on/off
		setParameterByName (&memo, "plywoodWidth", objInfo.libPart.mbar.plywoodWidth);				// 합판 너비
		setParameterByName (&memo, "plywoodOverhangH", objInfo.libPart.mbar.plywoodOverhangH);		// 합판 오버행
		setParameterByName (&memo, "plywoodUnderhangH", objInfo.libPart.mbar.plywoodUnderhangH);	// 합판 언더행

		validLength = objInfo.libPart.mbar.ZZYZX;
		validWidth = 0.039;
	} else if (objInfo.objType == MAGIC_INCORNER) {
		element.header.layer = layerInd_MagicIncorner;
		setParameterByName (&memo, "ZZYZX", objInfo.libPart.mincorner.ZZYZX);							// 높이
		setParameterByName (&memo, "type", "100");														// 타입 "100"
		setParameterByName (&memo, "angX", objInfo.libPart.mincorner.angX);								// 회전X
		setParameterByName (&memo, "angY", objInfo.libPart.mincorner.angY);								// 회전Y
		setParameterByName (&memo, "bPlywood", objInfo.libPart.mincorner.bPlywood);						// 합판 on/off
		setParameterByName (&memo, "plywoodWidth", objInfo.libPart.mincorner.plywoodWidth);				// 합판 너비
		setParameterByName (&memo, "plywoodOverhangH", objInfo.libPart.mincorner.plywoodOverhangH);		// 합판 오버행
		setParameterByName (&memo, "plywoodUnderhangH", objInfo.libPart.mincorner.plywoodUnderhangH);	// 합판 언더행

		validLength = objInfo.libPart.mincorner.ZZYZX;
		validWidth = 0.100;
	}

	// 객체 배치
	if ((objInfo.objType != NONE) && (validLength > EPS) && (validWidth > EPS))
		ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// 유로폼/아웃코너판넬을 채운 후 자투리 공간 채우기 (나머지는 합판으로 채움)
GSErrCode	ColumnPlacingZone::fillRestAreas_soleColumn (ColumnPlacingZone* placingZone)
{
	GSErrCode	err = NoError;

	short		xx;
	API_Coord	rotatedPoint;
	double		lineLen;
	double		xLen, yLen;
	double		dx, dy;
	double		leftLenOfBeam, rightLenOfBeam;

	double		heightOfFormArea = 0.0;
	double		columnWidth;
	double		marginHeight;

	CellForColumn	insCell;

	// 셀의 최종 높이를 구함
	for (xx = 0 ; xx < placingZone->nCells ; ++xx)
		heightOfFormArea += placingZone->cellsB1 [xx].height;

	// 북쪽
	if (placingZone->bFillMarginTopAtNorth == true) {
		xLen = (placingZone->coreWidth/2 + placingZone->venThick);
		yLen = (placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

		columnWidth = placingZone->coreWidth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtNorth;

		if (placingZone->bExistBeams [NORTH] == true) {
			dx = placingZone->beams [NORTH].endC.x - placingZone->beams [NORTH].begC.x;
			dy = placingZone->beams [NORTH].endC.y - placingZone->beams [NORTH].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [NORTH].begC, placingZone->beams [NORTH].endC) - placingZone->beams [NORTH].width/2 + placingZone->beams [NORTH].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [NORTH].begC, placingZone->beams [NORTH].endC) - placingZone->beams [NORTH].width/2 - placingZone->beams [NORTH].offset;
			rightLenOfBeam = placingZone->coreWidth + placingZone->venThick*2 - placingZone->beams [NORTH].width - leftLenOfBeam;

			// 합판 (왼쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (아래쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x - leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [NORTH].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (오른쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x - (leftLenOfBeam + placingZone->beams [NORTH].width) * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - (leftLenOfBeam + placingZone->beams [NORTH].width) * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		} else {
			// 합판 (전체)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}
	}

	// 남쪽
	if (placingZone->bFillMarginTopAtSouth == true) {
		xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
		yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

		columnWidth = placingZone->coreWidth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtSouth;

		if (placingZone->bExistBeams [SOUTH] == true) {
			dx = placingZone->beams [SOUTH].endC.x - placingZone->beams [SOUTH].begC.x;
			dy = placingZone->beams [SOUTH].endC.y - placingZone->beams [SOUTH].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [SOUTH].begC, placingZone->beams [SOUTH].endC) - placingZone->beams [SOUTH].width/2 - placingZone->beams [SOUTH].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [SOUTH].begC, placingZone->beams [SOUTH].endC) - placingZone->beams [SOUTH].width/2 + placingZone->beams [SOUTH].offset;
			rightLenOfBeam = placingZone->coreWidth + placingZone->venThick*2 - placingZone->beams [SOUTH].width - leftLenOfBeam;

			// 합판 (왼쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (아래쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x + leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [SOUTH].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (오른쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x + (leftLenOfBeam + placingZone->beams [SOUTH].width) * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + (leftLenOfBeam + placingZone->beams [SOUTH].width) * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		} else {
			// 합판 (전체)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}
	}

	// 서쪽
	if (placingZone->bFillMarginTopAtWest == true) {
		xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
		yLen = (placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

		columnWidth = placingZone->coreDepth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtWest;

		if (placingZone->bExistBeams [WEST] == true) {
			dx = placingZone->beams [WEST].endC.x - placingZone->beams [WEST].begC.x;
			dy = placingZone->beams [WEST].endC.y - placingZone->beams [WEST].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [WEST].begC, placingZone->beams [WEST].endC) - placingZone->beams [WEST].width/2 - placingZone->beams [WEST].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [WEST].begC, placingZone->beams [WEST].endC) - placingZone->beams [WEST].width/2 + placingZone->beams [WEST].offset;
			rightLenOfBeam = placingZone->coreDepth + placingZone->venThick*2 - placingZone->beams [WEST].width - leftLenOfBeam;

			// 합판 (왼쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (아래쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x + leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [WEST].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (오른쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x + (leftLenOfBeam + placingZone->beams [WEST].width) * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - (leftLenOfBeam + placingZone->beams [WEST].width) * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		} else {
			// 합판 (전체)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}
	}

	// 동쪽
	if (placingZone->bFillMarginTopAtEast == true) {
		xLen = (placingZone->coreWidth/2 + placingZone->venThick);
		yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

		columnWidth = placingZone->coreDepth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtEast;

		if (placingZone->bExistBeams [EAST] == true) {
			dx = placingZone->beams [EAST].endC.x - placingZone->beams [EAST].begC.x;
			dy = placingZone->beams [EAST].endC.y - placingZone->beams [EAST].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [EAST].begC, placingZone->beams [EAST].endC) - placingZone->beams [EAST].width/2 + placingZone->beams [EAST].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [EAST].begC, placingZone->beams [EAST].endC) - placingZone->beams [EAST].width/2 - placingZone->beams [EAST].offset;
			rightLenOfBeam = placingZone->coreDepth + placingZone->venThick*2 - placingZone->beams [EAST].width - leftLenOfBeam;

			// 합판 (왼쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (아래쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x - leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [EAST].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (오른쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x - (leftLenOfBeam + placingZone->beams [EAST].width) * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + (leftLenOfBeam + placingZone->beams [EAST].width) * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		} else {
			// 합판 (전체)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}
	}

	return	err;
}

// 유로폼/아웃코너판넬을 채운 후 자투리 공간 채우기 (나머지는 합판으로 채움)
GSErrCode	ColumnPlacingZone::fillRestAreas_wallColumn (ColumnPlacingZone* placingZone)
{
	GSErrCode	err = NoError;

	short		xx;
	API_Coord	rotatedPoint;
	double		lineLen;
	double		xLen, yLen;
	double		dx, dy;
	double		leftLenOfBeam, rightLenOfBeam;

	double		heightOfFormArea = 0.0;
	double		columnWidth;
	double		marginHeight;

	CellForColumn	insCell;

	// 셀의 최종 높이를 구함
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ( (placingZone->relationCase >= 1) && (placingZone->relationCase <= 3) )
			heightOfFormArea += placingZone->cellsB1 [xx].height;
		else
			heightOfFormArea += placingZone->cellsT1 [xx].height;
	}

	// 북쪽
	if (placingZone->bFillMarginTopAtNorth == true) {
		xLen = (placingZone->coreWidth/2 + placingZone->venThick);
		yLen = (placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

		columnWidth = placingZone->coreWidth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtNorth;

		// 보가 있는 경우
		if ((placingZone->bExistBeams [NORTH] == true) && (placingZone->bWallHorizontalDirected == true) && ((placingZone->relationCase == 4) || (placingZone->relationCase == 5) || (placingZone->relationCase == 6) || (placingZone->relationCase == 7))) {
			dx = placingZone->beams [NORTH].endC.x - placingZone->beams [NORTH].begC.x;
			dy = placingZone->beams [NORTH].endC.y - placingZone->beams [NORTH].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [NORTH].begC, placingZone->beams [NORTH].endC) - placingZone->beams [NORTH].width/2 + placingZone->beams [NORTH].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [NORTH].begC, placingZone->beams [NORTH].endC) - placingZone->beams [NORTH].width/2 - placingZone->beams [NORTH].offset;
			rightLenOfBeam = placingZone->coreWidth + placingZone->venThick*2 - placingZone->beams [NORTH].width - leftLenOfBeam;

			// 합판 (왼쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (아래쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x - leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [NORTH].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (오른쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x - (leftLenOfBeam + placingZone->beams [NORTH].width) * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - (leftLenOfBeam + placingZone->beams [NORTH].width) * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		}

		// 보가 없는 경우 1
		if ((placingZone->bExistBeams [NORTH] == false) && (placingZone->bWallHorizontalDirected == true) && ((placingZone->relationCase == 4) || (placingZone->relationCase == 5) || (placingZone->relationCase == 6) || (placingZone->relationCase == 7))) {
			// 합판 (전체)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}

		// 보가 없는 경우 2
		if ((placingZone->bExistBeams [NORTH] == false) && (placingZone->bWallHorizontalDirected == false)) {
			if ((placingZone->relationCase == 1) || (placingZone->relationCase == 2) || (placingZone->relationCase == 3) || (placingZone->relationCase == 4)) {
				// 합판
				xLen = (placingZone->coreWidth/2 + placingZone->venThick);
				yLen = (placingZone->coreDepth/2 + placingZone->venThick);
				lineLen = sqrt (xLen*xLen + yLen*yLen);
				rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle) - (placingZone->posTopColumnLine - placingZone->posBottomWallLine) * cos(placingZone->angle);
				rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle) - (placingZone->posTopColumnLine - placingZone->posBottomWallLine) * sin(placingZone->angle);

				columnWidth = placingZone->posBottomWallLine - placingZone->posBottomColumnLine;
				marginHeight = placingZone->marginTopAtNorth;

				insCell.objType = PLYWOOD;
				insCell.ang = placingZone->angle + DegreeToRad (180);
				insCell.leftBottomX = rotatedPoint.x;
				insCell.leftBottomY = rotatedPoint.y;
				insCell.leftBottomZ = heightOfFormArea;
				insCell.libPart.plywood.p_wid = columnWidth;
				insCell.libPart.plywood.p_leng = marginHeight;

				elemList.Push (placingZone->placeLibPart (insCell));
			}
			if ((placingZone->relationCase == 4) || (placingZone->relationCase == 5) || (placingZone->relationCase == 6) || (placingZone->relationCase == 7)) {
				// 합판
				xLen = (placingZone->coreWidth/2 + placingZone->venThick);
				yLen = (placingZone->coreDepth/2 + placingZone->venThick);
				lineLen = sqrt (xLen*xLen + yLen*yLen);
				rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
				rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

				columnWidth = placingZone->posTopColumnLine - placingZone->posTopWallLine;
				marginHeight = placingZone->marginTopAtNorth;

				insCell.objType = PLYWOOD;
				insCell.ang = placingZone->angle + DegreeToRad (180);
				insCell.leftBottomX = rotatedPoint.x;
				insCell.leftBottomY = rotatedPoint.y;
				insCell.leftBottomZ = heightOfFormArea;
				insCell.libPart.plywood.p_wid = columnWidth;
				insCell.libPart.plywood.p_leng = marginHeight;

				elemList.Push (placingZone->placeLibPart (insCell));
			}
		}
	}

	// 남쪽
	if (placingZone->bFillMarginTopAtSouth == true) {
		xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
		yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

		columnWidth = placingZone->coreWidth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtSouth;

		// 보가 있는 경우
		if ((placingZone->bExistBeams [SOUTH] == true) && (placingZone->bWallHorizontalDirected == true) && ((placingZone->relationCase == 1) || (placingZone->relationCase == 2) || (placingZone->relationCase == 3) || (placingZone->relationCase == 4))) {
			dx = placingZone->beams [SOUTH].endC.x - placingZone->beams [SOUTH].begC.x;
			dy = placingZone->beams [SOUTH].endC.y - placingZone->beams [SOUTH].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [SOUTH].begC, placingZone->beams [SOUTH].endC) - placingZone->beams [SOUTH].width/2 - placingZone->beams [SOUTH].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [SOUTH].begC, placingZone->beams [SOUTH].endC) - placingZone->beams [SOUTH].width/2 + placingZone->beams [SOUTH].offset;
			rightLenOfBeam = placingZone->coreWidth + placingZone->venThick*2 - placingZone->beams [SOUTH].width - leftLenOfBeam;

			// 합판 (왼쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (아래쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x + leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [SOUTH].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (오른쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x + (leftLenOfBeam + placingZone->beams [SOUTH].width) * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + (leftLenOfBeam + placingZone->beams [SOUTH].width) * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		}
		
		// 보가 없는 경우 1
		if ((placingZone->bExistBeams [SOUTH] == false) && (placingZone->bWallHorizontalDirected == true) && ((placingZone->relationCase == 1) || (placingZone->relationCase == 2) || (placingZone->relationCase == 3) || (placingZone->relationCase == 4))) {
			// 합판 (전체)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}

		// 보가 없는 경우 2
		if ((placingZone->bExistBeams [SOUTH] == false) && (placingZone->bWallHorizontalDirected == false)) {
			if ((placingZone->relationCase == 1) || (placingZone->relationCase == 2) || (placingZone->relationCase == 3) || (placingZone->relationCase == 4)) {
				// 합판
				xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
				yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
				lineLen = sqrt (xLen*xLen + yLen*yLen);
				rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
				rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

				columnWidth = placingZone->posBottomWallLine - placingZone->posBottomColumnLine;
				marginHeight = placingZone->marginTopAtSouth;

				insCell.objType = PLYWOOD;
				insCell.ang = placingZone->angle;
				insCell.leftBottomX = rotatedPoint.x;
				insCell.leftBottomY = rotatedPoint.y;
				insCell.leftBottomZ = heightOfFormArea;
				insCell.libPart.plywood.p_wid = columnWidth;
				insCell.libPart.plywood.p_leng = marginHeight;

				elemList.Push (placingZone->placeLibPart (insCell));
			}
			if ((placingZone->relationCase == 4) || (placingZone->relationCase == 5) || (placingZone->relationCase == 6) || (placingZone->relationCase == 7)) {
				// 합판
				xLen = (placingZone->coreWidth/2 + placingZone->venThick);
				yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
				lineLen = sqrt (xLen*xLen + yLen*yLen);
				rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle) - (placingZone->posTopColumnLine - placingZone->posTopWallLine) * cos(placingZone->angle);
				rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle) - (placingZone->posTopColumnLine - placingZone->posTopWallLine) * sin(placingZone->angle);

				columnWidth = placingZone->posTopColumnLine - placingZone->posTopWallLine;
				marginHeight = placingZone->marginTopAtSouth;

				insCell.objType = PLYWOOD;
				insCell.ang = placingZone->angle;
				insCell.leftBottomX = rotatedPoint.x;
				insCell.leftBottomY = rotatedPoint.y;
				insCell.leftBottomZ = heightOfFormArea;
				insCell.libPart.plywood.p_wid = columnWidth;
				insCell.libPart.plywood.p_leng = marginHeight;

				elemList.Push (placingZone->placeLibPart (insCell));
			}
		}
}

	// 서쪽
	if (placingZone->bFillMarginTopAtWest == true) {
		xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
		yLen = (placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

		columnWidth = placingZone->coreDepth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtWest;

		// 보가 있는 경우
		if ((placingZone->bExistBeams [WEST] == true) && (placingZone->bWallHorizontalDirected == false) && ((placingZone->relationCase == 1) || (placingZone->relationCase == 2) || (placingZone->relationCase == 3) || (placingZone->relationCase == 4))) {
			dx = placingZone->beams [WEST].endC.x - placingZone->beams [WEST].begC.x;
			dy = placingZone->beams [WEST].endC.y - placingZone->beams [WEST].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [WEST].begC, placingZone->beams [WEST].endC) - placingZone->beams [WEST].width/2 - placingZone->beams [WEST].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [WEST].begC, placingZone->beams [WEST].endC) - placingZone->beams [WEST].width/2 + placingZone->beams [WEST].offset;
			rightLenOfBeam = placingZone->coreDepth + placingZone->venThick*2 - placingZone->beams [WEST].width - leftLenOfBeam;

			// 합판 (왼쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (아래쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x + leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [WEST].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (오른쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x + (leftLenOfBeam + placingZone->beams [WEST].width) * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - (leftLenOfBeam + placingZone->beams [WEST].width) * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		}

		// 보가 없는 경우 1
		if ((placingZone->bExistBeams [WEST] == false) && (placingZone->bWallHorizontalDirected == false) && ((placingZone->relationCase == 1) || (placingZone->relationCase == 2) || (placingZone->relationCase == 3) || (placingZone->relationCase == 4))) {
			// 합판 (전체)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}

		// 보가 없는 경우 2
		if ((placingZone->bExistBeams [WEST] == false) && (placingZone->bWallHorizontalDirected == true)) {
			if ((placingZone->relationCase == 1) || (placingZone->relationCase == 2) || (placingZone->relationCase == 3) || (placingZone->relationCase == 4)) {
				// 합판
				xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
				yLen = (placingZone->coreDepth/2 + placingZone->venThick);
				lineLen = sqrt (xLen*xLen + yLen*yLen);
				rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle) + (placingZone->posTopColumnLine - placingZone->posBottomWallLine) * sin(placingZone->angle);
				rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle) - (placingZone->posTopColumnLine - placingZone->posBottomWallLine) * cos(placingZone->angle);

				columnWidth = placingZone->posBottomWallLine - placingZone->posBottomColumnLine;
				marginHeight = placingZone->marginTopAtWest;

				insCell.objType = PLYWOOD;
				insCell.ang = placingZone->angle - DegreeToRad (90);
				insCell.leftBottomX = rotatedPoint.x;
				insCell.leftBottomY = rotatedPoint.y;
				insCell.leftBottomZ = heightOfFormArea;
				insCell.libPart.plywood.p_wid = columnWidth;
				insCell.libPart.plywood.p_leng = marginHeight;

				elemList.Push (placingZone->placeLibPart (insCell));
			}
			if ((placingZone->relationCase == 4) || (placingZone->relationCase == 5) || (placingZone->relationCase == 6) || (placingZone->relationCase == 7)) {
				// 합판
				xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
				yLen = (placingZone->coreDepth/2 + placingZone->venThick);
				lineLen = sqrt (xLen*xLen + yLen*yLen);
				rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
				rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

				columnWidth = placingZone->posTopColumnLine - placingZone->posTopWallLine;
				marginHeight = placingZone->marginTopAtWest;

				insCell.objType = PLYWOOD;
				insCell.ang = placingZone->angle - DegreeToRad (90);
				insCell.leftBottomX = rotatedPoint.x;
				insCell.leftBottomY = rotatedPoint.y;
				insCell.leftBottomZ = heightOfFormArea;
				insCell.libPart.plywood.p_wid = columnWidth;
				insCell.libPart.plywood.p_leng = marginHeight;

				elemList.Push (placingZone->placeLibPart (insCell));
			}
		}
	}

	// 동쪽
	if (placingZone->bFillMarginTopAtEast == true) {
		xLen = (placingZone->coreWidth/2 + placingZone->venThick);
		yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

		columnWidth = placingZone->coreDepth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtEast;

		// 보가 있는 경우
		if ((placingZone->bExistBeams [EAST] == true) && (placingZone->bWallHorizontalDirected == false) && ((placingZone->relationCase == 4) || (placingZone->relationCase == 5) || (placingZone->relationCase == 6) || (placingZone->relationCase == 7))) {
			dx = placingZone->beams [EAST].endC.x - placingZone->beams [EAST].begC.x;
			dy = placingZone->beams [EAST].endC.y - placingZone->beams [EAST].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [EAST].begC, placingZone->beams [EAST].endC) - placingZone->beams [EAST].width/2 + placingZone->beams [EAST].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [EAST].begC, placingZone->beams [EAST].endC) - placingZone->beams [EAST].width/2 - placingZone->beams [EAST].offset;
			rightLenOfBeam = placingZone->coreDepth + placingZone->venThick*2 - placingZone->beams [EAST].width - leftLenOfBeam;

			// 합판 (왼쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (아래쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x - leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [EAST].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// 합판 (오른쪽)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x - (leftLenOfBeam + placingZone->beams [EAST].width) * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + (leftLenOfBeam + placingZone->beams [EAST].width) * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		}

		// 보가 없는 경우 1
		if ((placingZone->bExistBeams [EAST] == false) && (placingZone->bWallHorizontalDirected == false) && ((placingZone->relationCase == 4) || (placingZone->relationCase == 5) || (placingZone->relationCase == 6) || (placingZone->relationCase == 7))) {
			// 합판 (전체)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}

		// 보가 없는 경우 2
		if ((placingZone->bExistBeams [EAST] == false) && (placingZone->bWallHorizontalDirected == true)) {
			if ((placingZone->relationCase == 1) || (placingZone->relationCase == 2) || (placingZone->relationCase == 3) || (placingZone->relationCase == 4)) {
				// 합판
				xLen = (placingZone->coreWidth/2 + placingZone->venThick);
				yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
				lineLen = sqrt (xLen*xLen + yLen*yLen);
				rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
				rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

				columnWidth = placingZone->posBottomWallLine - placingZone->posBottomColumnLine;
				marginHeight = placingZone->marginTopAtEast;

				insCell.objType = PLYWOOD;
				insCell.ang = placingZone->angle + DegreeToRad (90);
				insCell.leftBottomX = rotatedPoint.x;
				insCell.leftBottomY = rotatedPoint.y;
				insCell.leftBottomZ = heightOfFormArea;
				insCell.libPart.plywood.p_wid = columnWidth;
				insCell.libPart.plywood.p_leng = marginHeight;

				elemList.Push (placingZone->placeLibPart (insCell));

			}
			if ((placingZone->relationCase == 4) || (placingZone->relationCase == 5) || (placingZone->relationCase == 6) || (placingZone->relationCase == 7)) {
				// 합판
				xLen = (placingZone->coreWidth/2 + placingZone->venThick);
				yLen = (placingZone->coreDepth/2 + placingZone->venThick);
				lineLen = sqrt (xLen*xLen + yLen*yLen);
				rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle) + (placingZone->posTopColumnLine - placingZone->posTopWallLine) * sin(placingZone->angle);
				rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle) - (placingZone->posTopColumnLine - placingZone->posTopWallLine) * cos(placingZone->angle);

				columnWidth = placingZone->posTopColumnLine - placingZone->posTopWallLine;
				marginHeight = placingZone->marginTopAtEast;

				insCell.objType = PLYWOOD;
				insCell.ang = placingZone->angle + DegreeToRad (90);
				insCell.leftBottomX = rotatedPoint.x;
				insCell.leftBottomY = rotatedPoint.y;
				insCell.leftBottomZ = heightOfFormArea;
				insCell.libPart.plywood.p_wid = columnWidth;
				insCell.libPart.plywood.p_leng = marginHeight;

				elemList.Push (placingZone->placeLibPart (insCell));
			}
		}
	}

	return	err;
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK columnPlacerHandler_soleColumn_1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGSetItemText (dialogID, LABEL_COLUMN_DEPTH, "세로");
			DGSetItemText (dialogID, LABEL_COLUMN_WIDTH, "가로");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER, "아웃코너");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, BUTTON_AUTOSET, "레이어 자동 설정");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

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
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_LEFT_3);
				DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_3, 0.0);
				DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_3, 0.0);
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, FALSE);
			}
			if (DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) {
				DGEnableItem (dialogID, EDITCONTROL_BOTTOM_3);
				DGSetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_BOTTOM_3);
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

			// 레이어 같이 바뀜
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_EUROFORM:
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						break;
					case USERCONTROL_LAYER_OUTCORNER:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER));
						break;
					case USERCONTROL_LAYER_PLYWOOD:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						break;
				}
			}

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
						placingZone.cellsL1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) * sin(placingZone.angle);
						placingZone.cellsL1 [xx].leftBottomY = rotatedPoint.y - DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) * cos(placingZone.angle);
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
							placingZone.cellsL2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2)) * sin(placingZone.angle);
							placingZone.cellsL2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2)) * cos(placingZone.angle);
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
						placingZone.cellsR1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2)) * sin(placingZone.angle);
						placingZone.cellsR1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2)) * cos(placingZone.angle);
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
							placingZone.cellsR2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * sin(placingZone.angle);
							placingZone.cellsR2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * cos(placingZone.angle);
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
					layerInd_Outcorner		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					layerInd_Euroform	= makeTemporaryLayer (structuralObject_forEuroformColumn, "UFOM", NULL);
					layerInd_Outcorner	= makeTemporaryLayer (structuralObject_forEuroformColumn, "OUTP", NULL);
					layerInd_Plywood	= makeTemporaryLayer (structuralObject_forEuroformColumn, "PLYW", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER, layerInd_Outcorner);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);

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
short DGCALLBACK columnPlacerHandler_soleColumn_2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGDisableItem (dialogID, DG_UPDATE_BUTTON);

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

			// 보를 의미하는 직사각형
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
			DGShowItem (dialogID, itmIdx);

			// 보 단면
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, itmIdx);
			DGDisableItem (dialogID, itmIdx);
			if (placingZone.bInterfereBeam == true)
				DGSetItemText (dialogID, itmIdx, "보\n있음");
			else
				DGSetItemText (dialogID, itmIdx, "보\n없음");

			// 여백 위치
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백");
			DGShowItem (dialogID, itmIdx);

			// 유로폼 버튼 시작
			btnPosX = 230;
			btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

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

				if (xx == 0)
					EUROFORM_BUTTON_BOTTOM = itmIdx;
				if (xx == placingZone.nCells-1)
					EUROFORM_BUTTON_TOP = itmIdx;
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
			dialogSizeX = 700;
			dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;

		case DG_MSG_CHANGE:

			break;

		case DG_MSG_CLICK:

			// 업데이트 버튼
			if (item == DG_UPDATE_BUTTON) {
				item = 0;

				// 저장된 여백 채움 여부 저장
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				placingZone.alignPlacingZone_soleColumn (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 보를 의미하는 직사각형
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// 보 단면
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, "보\n있음");
				else
					DGSetItemText (dialogID, itmIdx, "보\n없음");

				// 여백 위치
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);

				// 유로폼 버튼 시작
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

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

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
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
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 북
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 남
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 서
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
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
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// 이전 버튼
			if (item == DG_PREV) {
				clickedPrevButton = true;
			}

			// 확인 버튼
			if (item == DG_OK) {
				clickedOKButton = true;

				// 저장된 여백 채움 여부 저장
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				placingZone.alignPlacingZone_soleColumn (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 보를 의미하는 직사각형
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// 보 단면
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, "보\n있음");
				else
					DGSetItemText (dialogID, itmIdx, "보\n없음");

				// 여백 위치
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);

				// 유로폼 버튼 시작
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

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

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
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
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 북
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 남
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 서
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
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
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// 취소 버튼
			if (item == DG_CANCEL) {
			}

			// 셀 추가/삭제 버튼
			if (item == ADD_CELLS) {
				placingZone.addTopCell (&placingZone);
			}
			if (item == DEL_CELLS) {
				placingZone.delTopCell (&placingZone);
			}

			if ((item == ADD_CELLS) || (item == DEL_CELLS)) {
				item = 0;

				// 저장된 여백 채움 여부 저장
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				placingZone.alignPlacingZone_soleColumn (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 보를 의미하는 직사각형
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// 보 단면
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, "보\n있음");
				else
					DGSetItemText (dialogID, itmIdx, "보\n없음");

				// 여백 위치
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);

				// 유로폼 버튼 시작
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

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

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
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
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 북
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 남
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 서
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
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
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// 유로폼 버튼
			if ((item >= EUROFORM_BUTTON_BOTTOM) && (item <= EUROFORM_BUTTON_TOP)) {
				// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
				clickedBtnItemIdx = item;
				result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, columnPlacerHandler_soleColumn_3, 0);
				item = 0;

				// 저장된 여백 채움 여부 저장
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				placingZone.alignPlacingZone_soleColumn (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 보를 의미하는 직사각형
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// 보 단면
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, "보\n있음");
				else
					DGSetItemText (dialogID, itmIdx, "보\n없음");

				// 여백 위치
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);

				// 유로폼 버튼 시작
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

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

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
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
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 북
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 남
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 서
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
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
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
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

// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그
short DGCALLBACK columnPlacerHandler_soleColumn_3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idx = -1;
	short	popupSelectedIdx = -1;
	double	length;

	switch (message) {
		case DG_MSG_INIT:

			// 배치 버튼
			if ((clickedBtnItemIdx >= EUROFORM_BUTTON_BOTTOM) && (clickedBtnItemIdx <= EUROFORM_BUTTON_TOP))
				idx = clickedBtnItemIdx - EUROFORM_BUTTON_BOTTOM;

			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "셀 설정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 저장 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 160, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "저장");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 110, 160, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
			// 라벨: 객체 타입
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 10, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, "객체 타입");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// 팝업컨트롤: 객체 타입을 바꿀 수 있는 콤보박스가 맨 위에 나옴
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 90, 20-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "유로폼");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);

			// 체크박스: 규격폼
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20, 60, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "규격폼");

			// 라벨: 길이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 90, 50, 23);
			DGSetItemFont (dialogID, LABEL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LENGTH, "길이");

			// Edit 컨트롤: 길이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 90-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

			// 팝업 컨트롤: 길이
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 80, 90-6, 80, 25);
			DGSetItemFont (dialogID, POPUP_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM, "600");

			// 초기 입력 필드 표시
			if (placingZone.cellsB1 [idx].objType == NONE) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, NONE + 1);

			} else if (placingZone.cellsB1 [idx].objType == EUROFORM) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

				DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
				DGShowItem (dialogID, LABEL_LENGTH);
				if (placingZone.cellsB1 [idx].libPart.form.eu_stan_onoff == true) {
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
					DGShowItem (dialogID, POPUP_LENGTH);
					if (abs (placingZone.cellsB1 [idx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
					if (abs (placingZone.cellsB1 [idx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
					if (abs (placingZone.cellsB1 [idx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
					DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
				} else {
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, FALSE);
					DGShowItem (dialogID, EDITCONTROL_LENGTH);
					DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellsB1 [idx].libPart.form.eu_hei2);
					DGSetItemMinDouble (dialogID, EDITCONTROL_LENGTH, 0.050);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_LENGTH, 1.500);
				}
			}

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case POPUP_OBJ_TYPE:
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
						DGHideItem (dialogID, CHECKBOX_SET_STANDARD);
						DGHideItem (dialogID, LABEL_LENGTH);
						DGHideItem (dialogID, EDITCONTROL_LENGTH);
						DGHideItem (dialogID, POPUP_LENGTH);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
						DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, POPUP_LENGTH);
						DGHideItem (dialogID, EDITCONTROL_LENGTH);
					}
					break;

				case CHECKBOX_SET_STANDARD:
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
						DGShowItem (dialogID, POPUP_LENGTH);
						DGHideItem (dialogID, EDITCONTROL_LENGTH);
					} else {
						DGHideItem (dialogID, POPUP_LENGTH);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);
					}
					break;
			}
		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					idx = -1;

					// 배치 버튼
					if ((clickedBtnItemIdx >= EUROFORM_BUTTON_BOTTOM) && (clickedBtnItemIdx <= EUROFORM_BUTTON_TOP))
						idx = clickedBtnItemIdx - EUROFORM_BUTTON_BOTTOM;

					// 입력한 길이를 해당 셀의 모든 객체들에게 적용함
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {

						placingZone.cellsLT [idx].objType = NONE;
						placingZone.cellsRT [idx].objType = NONE;
						placingZone.cellsLB [idx].objType = NONE;
						placingZone.cellsRB [idx].objType = NONE;
						placingZone.cellsT1 [idx].objType = NONE;
						placingZone.cellsT2 [idx].objType = NONE;
						placingZone.cellsL1 [idx].objType = NONE;
						placingZone.cellsL2 [idx].objType = NONE;
						placingZone.cellsR1 [idx].objType = NONE;
						placingZone.cellsR2 [idx].objType = NONE;
						placingZone.cellsB1 [idx].objType = NONE;
						placingZone.cellsB2 [idx].objType = NONE;

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

						// 규격폼으로 저장할 경우
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
							length = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
						// 비규격폼으로 저장할 경우
						else
							length = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

						placingZone.cellsLT [idx].objType = OUTCORNER;
						placingZone.cellsLT [idx].height = length;
						placingZone.cellsLT [idx].libPart.outcorner.hei_s = length;

						placingZone.cellsRT [idx].objType = OUTCORNER;
						placingZone.cellsRT [idx].height = length;
						placingZone.cellsRT [idx].libPart.outcorner.hei_s = length;

						placingZone.cellsLB [idx].objType = OUTCORNER;
						placingZone.cellsLB [idx].height = length;
						placingZone.cellsLB [idx].libPart.outcorner.hei_s = length;

						placingZone.cellsRB [idx].objType = OUTCORNER;
						placingZone.cellsRB [idx].height = length;
						placingZone.cellsRB [idx].libPart.outcorner.hei_s = length;
							
						placingZone.cellsT1 [idx].objType = EUROFORM;
						placingZone.cellsT1 [idx].height = length;
						placingZone.cellsT1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsT1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsT2 [idx].objType = EUROFORM;
						placingZone.cellsT2 [idx].height = length;
						placingZone.cellsT2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsT2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsL1 [idx].objType = EUROFORM;
						placingZone.cellsL1 [idx].height = length;
						placingZone.cellsL1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsL1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsL2 [idx].objType = EUROFORM;
						placingZone.cellsL2 [idx].height = length;
						placingZone.cellsL2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsL2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsR1 [idx].objType = EUROFORM;
						placingZone.cellsR1 [idx].height = length;
						placingZone.cellsR1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsR1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsR2 [idx].objType = EUROFORM;
						placingZone.cellsR2 [idx].height = length;
						placingZone.cellsR2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsR2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsB1 [idx].objType = EUROFORM;
						placingZone.cellsB1 [idx].height = length;
						placingZone.cellsB1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsB1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsB2 [idx].objType = EUROFORM;
						placingZone.cellsB2 [idx].height = length;
						placingZone.cellsB2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsB2 [idx].libPart.form.eu_hei2 = length;
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

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK columnPlacerHandler_wallColumn_1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		itmIdx;
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
			DGSetItemText (dialogID, LABEL_COLUMN_SECTION_WC, "기둥 단면");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS_WC, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_WC, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER_WC, "인코너");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_WC, "아웃코너");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD_WC, "합판");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING_WC, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_WC, TRUE);

			DGSetItemText (dialogID, BUTTON_AUTOSET_WC, "레이어 자동 설정");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_WC;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC, 1);

			ucb.itemID	 = USERCONTROL_LAYER_INCORNER_WC;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_WC;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD_WC;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC, 1);

			// 기둥 단면 이미지 모두 숨기기
			DGHideItem (dialogID, ICON_COLUMN_SECTION_01_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_02_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_03_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_04_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_05_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_06_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_07_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_08_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_09_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_10_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_11_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_12_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_13_WC);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_14_WC);

			// 기둥 단면 이미지 표시
			if (placingZone.bWallHorizontalDirected == true) {
				if (placingZone.relationCase == 1)	DGShowItem (dialogID, ICON_COLUMN_SECTION_01_WC);
				if (placingZone.relationCase == 2)	DGShowItem (dialogID, ICON_COLUMN_SECTION_02_WC);
				if (placingZone.relationCase == 3)	DGShowItem (dialogID, ICON_COLUMN_SECTION_03_WC);
				if (placingZone.relationCase == 4)	DGShowItem (dialogID, ICON_COLUMN_SECTION_04_WC);
				if (placingZone.relationCase == 5)	DGShowItem (dialogID, ICON_COLUMN_SECTION_05_WC);
				if (placingZone.relationCase == 6)	DGShowItem (dialogID, ICON_COLUMN_SECTION_06_WC);
				if (placingZone.relationCase == 7)	DGShowItem (dialogID, ICON_COLUMN_SECTION_07_WC);
			} else {
				if (placingZone.relationCase == 1)	DGShowItem (dialogID, ICON_COLUMN_SECTION_08_WC);
				if (placingZone.relationCase == 2)	DGShowItem (dialogID, ICON_COLUMN_SECTION_09_WC);
				if (placingZone.relationCase == 3)	DGShowItem (dialogID, ICON_COLUMN_SECTION_10_WC);
				if (placingZone.relationCase == 4)	DGShowItem (dialogID, ICON_COLUMN_SECTION_11_WC);
				if (placingZone.relationCase == 5)	DGShowItem (dialogID, ICON_COLUMN_SECTION_12_WC);
				if (placingZone.relationCase == 6)	DGShowItem (dialogID, ICON_COLUMN_SECTION_13_WC);
				if (placingZone.relationCase == 7)	DGShowItem (dialogID, ICON_COLUMN_SECTION_14_WC);
			}

			// 보 너비/높이, Edit박스 표시 및 기본값 표시와 항목 잠그기
			if (placingZone.bWallHorizontalDirected == true) {
				if (placingZone.relationCase == 1) {
					// 라벨: 세로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 195, 165, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 240, 220, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 190, 185, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_DOWN = itmIdx;

					// Edit 컨트롤: 가로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 235, 240, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_DOWN = itmIdx;

					// 왼쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 77, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 117, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 152, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 202, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 245, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LB = itmIdx;

					// 오른쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 77, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 117, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 152, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 202, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 245, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RB = itmIdx;

					// 위쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 143, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 207, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 258, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W3 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 322, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W4 = itmIdx;

					// 아래쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 165, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 210, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 260, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 305, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RB = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_DOWN, placingZone.coreDepth + placingZone.venThick * 2);
					DGSetItemValDouble (dialogID, HLEN_DOWN, placingZone.coreWidth + placingZone.venThick * 2);
					DGDisableItem (dialogID, VLEN_DOWN);
					DGDisableItem (dialogID, HLEN_DOWN);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, LEN_Rin2_C);
					DGDisableItem (dialogID, LEN_R1);
					DGDisableItem (dialogID, LEN_R2);
					DGDisableItem (dialogID, VLEN_RB);
					DGDisableItem (dialogID, LEN_W1);
					DGDisableItem (dialogID, LEN_W2);
					DGDisableItem (dialogID, LEN_W3);
					DGDisableItem (dialogID, LEN_W4);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, LEN_Lin2_W, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin2_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_L1, 0.300);
					DGSetItemValDouble (dialogID, LEN_L2, 0.300);
					DGSetItemValDouble (dialogID, VLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin2_W, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_B1, 0.300);
					DGSetItemValDouble (dialogID, LEN_B2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RB, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_R2, DGGetItemValDouble (dialogID, LEN_L2));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin2_W) + DGGetItemValDouble (dialogID, HLEN_RB));
				}
				if ( (placingZone.relationCase == 2) || (placingZone.relationCase == 3) ) {
					// 라벨: 세로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 195, 150, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 240, 195, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 190, 170, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_DOWN = itmIdx;

					// Edit 컨트롤: 가로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 235, 215, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_DOWN = itmIdx;

					// 왼쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 104, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 142, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 180, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 220, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LB = itmIdx;

					// 오른쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 104, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 142, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 180, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 220, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RB = itmIdx;

					// 위쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 143, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 207, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 258, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W3 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 322, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W4 = itmIdx;

					// 아래쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 165, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 210, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 260, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 305, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RB = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_DOWN, placingZone.coreDepth + placingZone.venThick * 2 - (placingZone.posTopColumnLine - placingZone.posBottomWallLine));
					DGSetItemValDouble (dialogID, HLEN_DOWN, placingZone.coreWidth + placingZone.venThick * 2);
					DGDisableItem (dialogID, VLEN_DOWN);
					DGDisableItem (dialogID, HLEN_DOWN);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, LEN_Rin2_C);
					DGDisableItem (dialogID, LEN_R1);
					DGDisableItem (dialogID, VLEN_RB);
					DGDisableItem (dialogID, LEN_W1);
					DGDisableItem (dialogID, LEN_W2);
					DGDisableItem (dialogID, LEN_W3);
					DGDisableItem (dialogID, LEN_W4);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, LEN_Lin2_W, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin2_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_L1, 0.300);
					DGSetItemValDouble (dialogID, VLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin2_W, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_B1, 0.300);
					DGSetItemValDouble (dialogID, LEN_B2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RB, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin2_W) + DGGetItemValDouble (dialogID, HLEN_RB));
				}
				if (placingZone.relationCase == 4) {
					// 라벨: 세로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 195, 210, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 260, 220, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 190, 230, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_DOWN = itmIdx;

					// Edit 컨트롤: 가로 (아래)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 255, 240, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_DOWN = itmIdx;

					// 라벨: 세로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 290, 110, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 240, 100, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 285, 130, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_UP = itmIdx;

					// Edit 컨트롤: 가로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 235, 120, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_UP = itmIdx;

					// 왼쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 94, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 122, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 169, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 215, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 245, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LB = itmIdx;

					// 오른쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 94, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 122, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 169, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 215, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 245, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RB = itmIdx;

					// 위쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 163, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 207, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 258, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 302, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RT = itmIdx;

					// 아래쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 165, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 210, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 260, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 305, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RB = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_DOWN, placingZone.posBottomWallLine - placingZone.posBottomColumnLine);
					DGSetItemValDouble (dialogID, HLEN_DOWN, placingZone.coreWidth + placingZone.venThick * 2);
					DGDisableItem (dialogID, VLEN_DOWN);
					DGDisableItem (dialogID, HLEN_DOWN);

					DGSetItemValDouble (dialogID, VLEN_UP, placingZone.posTopColumnLine - placingZone.posTopWallLine);
					DGSetItemValDouble (dialogID, HLEN_UP, placingZone.coreWidth + placingZone.venThick * 2);
					DGDisableItem (dialogID, VLEN_UP);
					DGDisableItem (dialogID, HLEN_UP);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, HLEN_LT);
					DGDisableItem (dialogID, LEN_T1);
					DGDisableItem (dialogID, LEN_T2);
					DGDisableItem (dialogID, HLEN_RT);
					DGDisableItem (dialogID, VLEN_RT);
					DGDisableItem (dialogID, LEN_Rin1_C);
					DGDisableItem (dialogID, LEN_Rin2_C);
					DGDisableItem (dialogID, VLEN_RB);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, VLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin1_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin2_W, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin2_C, 0.100);
					DGSetItemValDouble (dialogID, VLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin2_W, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_B1, 0.300);
					DGSetItemValDouble (dialogID, LEN_B2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RB, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, HLEN_LT, DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_T1, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_T2, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, HLEN_RT, DGGetItemValDouble (dialogID, HLEN_RB));
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
				}
				if ( (placingZone.relationCase == 5) || (placingZone.relationCase == 6) ) {
					// 라벨: 세로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 195, 145, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 260, 120, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 190, 165, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_UP = itmIdx;

					// Edit 컨트롤: 가로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 255, 140, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_UP = itmIdx;

					// 왼쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 115, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 155, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 192, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 233, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_W = itmIdx;

					// 오른쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 115, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 155, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 192, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 233, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_W = itmIdx;

					// 위쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 165, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 210, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 260, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 305, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RT = itmIdx;

					// 아래쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 143, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 207, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 258, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W3 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 322, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W4 = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_UP, placingZone.posTopColumnLine - placingZone.posTopWallLine);
					DGSetItemValDouble (dialogID, HLEN_UP, placingZone.coreWidth + placingZone.venThick * 2);
					DGDisableItem (dialogID, VLEN_UP);
					DGDisableItem (dialogID, HLEN_UP);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, VLEN_RT);
					DGDisableItem (dialogID, LEN_R1);
					DGDisableItem (dialogID, LEN_Rin1_C);
					DGDisableItem (dialogID, LEN_W1);
					DGDisableItem (dialogID, LEN_W2);
					DGDisableItem (dialogID, LEN_W3);
					DGDisableItem (dialogID, LEN_W4);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, VLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_L1, 0.300);
					DGSetItemValDouble (dialogID, LEN_Lin1_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin1_W, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_T1, 0.300);
					DGSetItemValDouble (dialogID, LEN_T2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RT, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin1_W, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_T1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_T2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin1_W) + DGGetItemValDouble (dialogID, HLEN_RT));
				}
				if (placingZone.relationCase == 7) {
					// 라벨: 세로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 195, 145, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 260, 100, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 190, 165, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_UP = itmIdx;

					// Edit 컨트롤: 가로 (위)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 255, 120, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_UP = itmIdx;

					// 왼쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 95, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 135, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 185, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 223, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 70, 260, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_W = itmIdx;

					// 오른쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 95, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 135, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 185, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 223, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 400, 260, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_W = itmIdx;

					// 위쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 165, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 210, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 260, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 305, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RT = itmIdx;

					// 아래쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 143, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 207, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 258, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W3 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 322, 315, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W4 = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_UP, placingZone.coreDepth + placingZone.venThick * 2);
					DGSetItemValDouble (dialogID, HLEN_UP, placingZone.coreWidth + placingZone.venThick * 2);
					DGDisableItem (dialogID, VLEN_UP);
					DGDisableItem (dialogID, HLEN_UP);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, VLEN_RT);
					DGDisableItem (dialogID, LEN_R1);
					DGDisableItem (dialogID, LEN_R2);
					DGDisableItem (dialogID, LEN_Rin1_C);
					DGDisableItem (dialogID, LEN_W1);
					DGDisableItem (dialogID, LEN_W2);
					DGDisableItem (dialogID, LEN_W3);
					DGDisableItem (dialogID, LEN_W4);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, VLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_L1, 0.300);
					DGSetItemValDouble (dialogID, LEN_L2, 0.300);
					DGSetItemValDouble (dialogID, LEN_Lin1_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin1_W, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_T1, 0.300);
					DGSetItemValDouble (dialogID, LEN_T2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RT, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin1_W, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_R2, DGGetItemValDouble (dialogID, LEN_L2));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_T1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_T2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin1_W) + DGGetItemValDouble (dialogID, HLEN_RT));
				}
			} else {
				if (placingZone.relationCase == 1) {
					// 라벨: 세로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 180, 155, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 230, 210, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 175, 175, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_DOWN = itmIdx;

					// Edit 컨트롤: 가로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 225, 230, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_DOWN = itmIdx;

					// 왼쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 110, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 153, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 203, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 248, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RB = itmIdx;

					// 오른쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 88, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 150, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 200, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W3 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 265, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W4 = itmIdx;

					// 위쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 160, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 203, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 249, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 290, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 331, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_W = itmIdx;

					// 아래쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 160, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 203, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 249, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 290, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 331, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_W = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_DOWN, placingZone.coreDepth + placingZone.venThick * 2);
					DGSetItemValDouble (dialogID, HLEN_DOWN, placingZone.coreWidth + placingZone.venThick * 2);
					DGDisableItem (dialogID, VLEN_DOWN);
					DGDisableItem (dialogID, HLEN_DOWN);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, LEN_Rin2_C);
					DGDisableItem (dialogID, LEN_R1);
					DGDisableItem (dialogID, LEN_R2);
					DGDisableItem (dialogID, VLEN_RB);
					DGDisableItem (dialogID, LEN_W1);
					DGDisableItem (dialogID, LEN_W2);
					DGDisableItem (dialogID, LEN_W3);
					DGDisableItem (dialogID, LEN_W4);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, LEN_Lin2_W, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin2_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_L1, 0.300);
					DGSetItemValDouble (dialogID, LEN_L2, 0.300);
					DGSetItemValDouble (dialogID, VLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_B1, 0.300);
					DGSetItemValDouble (dialogID, LEN_B2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RB, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin2_W, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_R2, DGGetItemValDouble (dialogID, LEN_L2));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin2_W) + DGGetItemValDouble (dialogID, HLEN_RB));
				}
				if ( (placingZone.relationCase == 2) || (placingZone.relationCase == 3) ) {
					// 라벨: 세로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 155, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 240, 210, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 195, 175, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_DOWN = itmIdx;

					// Edit 컨트롤: 가로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 235, 230, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_DOWN = itmIdx;

					// 왼쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 110, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 153, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 203, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 248, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RB = itmIdx;

					// 오른쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 88, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 150, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 200, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W3 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 265, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W4 = itmIdx;

					// 위쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 182, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 223, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 265, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 306, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_W = itmIdx;

					// 아래쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 182, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 223, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 265, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 306, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_W = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_DOWN, placingZone.coreDepth + placingZone.venThick * 2);
					DGSetItemValDouble (dialogID, HLEN_DOWN, placingZone.coreWidth + placingZone.venThick * 2 - (placingZone.posTopColumnLine - placingZone.posBottomWallLine));
					DGDisableItem (dialogID, VLEN_DOWN);
					DGDisableItem (dialogID, HLEN_DOWN);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, LEN_Rin2_C);
					DGDisableItem (dialogID, LEN_R1);
					DGDisableItem (dialogID, VLEN_RB);
					DGDisableItem (dialogID, LEN_W1);
					DGDisableItem (dialogID, LEN_W2);
					DGDisableItem (dialogID, LEN_W3);
					DGDisableItem (dialogID, LEN_W4);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, LEN_Lin2_W, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin2_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_L1, 0.300);
					DGSetItemValDouble (dialogID, VLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_B1, 0.300);
					DGSetItemValDouble (dialogID, LEN_B2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RB, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin2_W, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin2_W) + DGGetItemValDouble (dialogID, HLEN_RB));
				}
				if (placingZone.relationCase == 4) {
					// 라벨: 세로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 180, 150, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 185, 210, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 175, 170, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_DOWN = itmIdx;

					// Edit 컨트롤: 가로 (왼쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 180, 230, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_DOWN = itmIdx;

					// 라벨: 세로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 300, 150, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 290, 210, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 295, 170, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_UP = itmIdx;

					// Edit 컨트롤: 가로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 285, 230, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_UP = itmIdx;

					// 왼쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 110, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 153, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 203, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_B2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 248, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RB = itmIdx;

					// 오른쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 110, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 153, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 203, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 248, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RT = itmIdx;

					// 위쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 152, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 193, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 233, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin2_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 275, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 320, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LT = itmIdx;

					// 아래쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 152, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RB = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 193, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 233, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin2_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 275, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 320, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RT = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_DOWN, placingZone.coreDepth + placingZone.venThick * 2);
					DGSetItemValDouble (dialogID, HLEN_DOWN, placingZone.posBottomWallLine - placingZone.posBottomColumnLine);
					DGDisableItem (dialogID, VLEN_DOWN);
					DGDisableItem (dialogID, HLEN_DOWN);

					DGSetItemValDouble (dialogID, VLEN_UP, placingZone.coreDepth + placingZone.venThick * 2);
					DGSetItemValDouble (dialogID, HLEN_UP, placingZone.posTopColumnLine - placingZone.posTopWallLine);
					DGDisableItem (dialogID, VLEN_UP);
					DGDisableItem (dialogID, HLEN_UP);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, HLEN_LT);
					DGDisableItem (dialogID, LEN_T1);
					DGDisableItem (dialogID, LEN_T2);
					DGDisableItem (dialogID, HLEN_RT);
					DGDisableItem (dialogID, VLEN_RT);
					DGDisableItem (dialogID, LEN_Rin1_C);
					DGDisableItem (dialogID, LEN_Rin2_C);
					DGDisableItem (dialogID, VLEN_RB);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, VLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin1_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin2_W, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin2_C, 0.100);
					DGSetItemValDouble (dialogID, VLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin2_W, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LB, 0.100);
					DGSetItemValDouble (dialogID, LEN_B1, 0.300);
					DGSetItemValDouble (dialogID, LEN_B2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RB, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, HLEN_LT, DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_T1, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_T2, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, HLEN_RT, DGGetItemValDouble (dialogID, HLEN_RB));
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
				}
				if ( (placingZone.relationCase == 5) || (placingZone.relationCase == 6) ) {
					// 라벨: 세로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 280, 155, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 250, 215, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 275, 175, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_UP = itmIdx;

					// Edit 컨트롤: 가로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 245, 235, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_UP = itmIdx;

					// 왼쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 90, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 155, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 205, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W3 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 269, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W4 = itmIdx;

					// 오른쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 110, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 153, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 203, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 248, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RT = itmIdx;

					// 위쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 169, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 210, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 253, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 295, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LT = itmIdx;

					// 아래쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 169, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 210, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 253, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 295, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RT = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_UP, placingZone.coreDepth + placingZone.venThick * 2);
					DGSetItemValDouble (dialogID, HLEN_UP, placingZone.posTopColumnLine - placingZone.posTopWallLine);
					DGDisableItem (dialogID, VLEN_UP);
					DGDisableItem (dialogID, HLEN_UP);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, VLEN_RT);
					DGDisableItem (dialogID, LEN_R1);
					DGDisableItem (dialogID, LEN_Rin1_C);
					DGDisableItem (dialogID, LEN_W1);
					DGDisableItem (dialogID, LEN_W2);
					DGDisableItem (dialogID, LEN_W3);
					DGDisableItem (dialogID, LEN_W4);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, VLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_L1, 0.300);
					DGSetItemValDouble (dialogID, LEN_Lin1_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin1_W, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_T1, 0.300);
					DGSetItemValDouble (dialogID, LEN_T2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RT, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin1_W, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_T1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_T2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin1_W) + DGGetItemValDouble (dialogID, HLEN_RT));
				}
				if (placingZone.relationCase == 7) {
					// 라벨: 세로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 300, 155, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "세로");
					DGShowItem (dialogID, itmIdx);

					// 라벨: 가로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 250, 215, 40, 20);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "가로");
					DGShowItem (dialogID, itmIdx);

					// Edit 컨트롤: 세로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 295, 175, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_UP = itmIdx;

					// Edit 컨트롤: 가로 (오른쪽)
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 245, 235, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_UP = itmIdx;

					// 왼쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 90, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 155, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 205, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W3 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 269, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_W4 = itmIdx;

					// 오른쪽 4개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 110, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_LT = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 153, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 203, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_T2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 390, 248, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					HLEN_RT = itmIdx;

					// 위쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 138, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 179, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Lin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 270, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_L1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 315, 25, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_LT = itmIdx;

					// 아래쪽 5개
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 138, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_W = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 179, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_Rin1_C = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R2 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 270, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					LEN_R1 = itmIdx;
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 315, 335, 40, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGShowItem (dialogID, itmIdx);
					VLEN_RT = itmIdx;

					// 세로/가로 계산
					DGSetItemValDouble (dialogID, VLEN_UP, placingZone.coreWidth + placingZone.venThick * 2);
					DGSetItemValDouble (dialogID, HLEN_UP, placingZone.coreDepth + placingZone.venThick * 2);
					DGDisableItem (dialogID, VLEN_UP);
					DGDisableItem (dialogID, HLEN_UP);

					// 직접 변경해서는 안 되는 항목 잠그기
					DGDisableItem (dialogID, VLEN_RT);
					DGDisableItem (dialogID, LEN_R1);
					DGDisableItem (dialogID, LEN_R2);
					DGDisableItem (dialogID, LEN_Rin1_C);
					DGDisableItem (dialogID, LEN_W1);
					DGDisableItem (dialogID, LEN_W2);
					DGDisableItem (dialogID, LEN_W3);
					DGDisableItem (dialogID, LEN_W4);

					// 기본값 입력해 놓음
					DGSetItemValDouble (dialogID, VLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_L1, 0.300);
					DGSetItemValDouble (dialogID, LEN_L2, 0.300);
					DGSetItemValDouble (dialogID, LEN_Lin1_C, 0.100);
					DGSetItemValDouble (dialogID, LEN_Lin1_W, 0.100);
					DGSetItemValDouble (dialogID, HLEN_LT, 0.100);
					DGSetItemValDouble (dialogID, LEN_T1, 0.300);
					DGSetItemValDouble (dialogID, LEN_T2, 0.300);
					DGSetItemValDouble (dialogID, HLEN_RT, 0.100);
					DGSetItemValDouble (dialogID, LEN_Rin1_W, 0.100);

					// 값 자동 채우기
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_R2, DGGetItemValDouble (dialogID, LEN_L2));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_T1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_T2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin1_W) + DGGetItemValDouble (dialogID, HLEN_RT));
				}
			}

			break;
		
		case DG_MSG_CHANGE:

			// Edit컨트롤이 바뀌는 것을 감지할 것
			if (placingZone.bWallHorizontalDirected == true) {
				if (placingZone.relationCase == 1) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_R2, DGGetItemValDouble (dialogID, LEN_L2));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin2_W) + DGGetItemValDouble (dialogID, HLEN_RB));
				}
				if ( (placingZone.relationCase == 2) || (placingZone.relationCase == 3) ) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin2_W) + DGGetItemValDouble (dialogID, HLEN_RB));
				}
				if (placingZone.relationCase == 4) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, HLEN_LT, DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_T1, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_T2, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, HLEN_RT, DGGetItemValDouble (dialogID, HLEN_RB));
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
				}
				if ( (placingZone.relationCase == 5) || (placingZone.relationCase == 6) ) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_T1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_T2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin1_W) + DGGetItemValDouble (dialogID, HLEN_RT));
				}
				if (placingZone.relationCase == 7) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_R2, DGGetItemValDouble (dialogID, LEN_L2));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_T1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_T2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin1_W) + DGGetItemValDouble (dialogID, HLEN_RT));
				}
			} else {
				if (placingZone.relationCase == 1) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_R2, DGGetItemValDouble (dialogID, LEN_L2));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin2_W) + DGGetItemValDouble (dialogID, HLEN_RB));
				}
				if ( (placingZone.relationCase == 2) || (placingZone.relationCase == 3) ) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin2_W) + DGGetItemValDouble (dialogID, HLEN_RB));
				}
				if (placingZone.relationCase == 4) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, HLEN_LT, DGGetItemValDouble (dialogID, HLEN_LB));
					DGSetItemValDouble (dialogID, LEN_T1, DGGetItemValDouble (dialogID, LEN_B1));
					DGSetItemValDouble (dialogID, LEN_T2, DGGetItemValDouble (dialogID, LEN_B2));
					DGSetItemValDouble (dialogID, HLEN_RT, DGGetItemValDouble (dialogID, HLEN_RB));
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_Rin2_C, DGGetItemValDouble (dialogID, LEN_Lin2_C));
					DGSetItemValDouble (dialogID, VLEN_RB, DGGetItemValDouble (dialogID, VLEN_LB));
				}
				if ( (placingZone.relationCase == 5) || (placingZone.relationCase == 6) ) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_T1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_T2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin1_W) + DGGetItemValDouble (dialogID, HLEN_RT));
				}
				if (placingZone.relationCase == 7) {
					// 값 자동 채우기
					DGSetItemValDouble (dialogID, VLEN_RT, DGGetItemValDouble (dialogID, VLEN_LT));
					DGSetItemValDouble (dialogID, LEN_R1, DGGetItemValDouble (dialogID, LEN_L1));
					DGSetItemValDouble (dialogID, LEN_R2, DGGetItemValDouble (dialogID, LEN_L2));
					DGSetItemValDouble (dialogID, LEN_Rin1_C, DGGetItemValDouble (dialogID, LEN_Lin1_C));
					DGSetItemValDouble (dialogID, LEN_W1, DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT));
					DGSetItemValDouble (dialogID, LEN_W2, DGGetItemValDouble (dialogID, LEN_T1));
					DGSetItemValDouble (dialogID, LEN_W3, DGGetItemValDouble (dialogID, LEN_T2));
					DGSetItemValDouble (dialogID, LEN_W4, DGGetItemValDouble (dialogID, LEN_Rin1_W) + DGGetItemValDouble (dialogID, HLEN_RT));
				}
			}

			// 레이어 같이 바뀜
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING_WC) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_EUROFORM_WC:
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC));
						break;
					case USERCONTROL_LAYER_INCORNER_WC:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC));
						break;
					case USERCONTROL_LAYER_OUTCORNER_WC:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC));
						break;
					case USERCONTROL_LAYER_PLYWOOD_WC:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC));
						break;
				}
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 셀 설정 적용
					if (placingZone.bWallHorizontalDirected == true) {
						for (xx = 0 ; xx < 20 ; ++xx) {
							// 좌상단
							if ((HLEN_LT != 0) && (VLEN_LT != 0)) {
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
								placingZone.cellsLT [xx].horLen = DGGetItemValDouble (dialogID, HLEN_LT);
								placingZone.cellsLT [xx].verLen = DGGetItemValDouble (dialogID, VLEN_LT);
								placingZone.cellsLT [xx].height = 1.200;
								placingZone.cellsLT [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, HLEN_LT);
								placingZone.cellsLT [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, VLEN_LT);
								placingZone.cellsLT [xx].libPart.outcorner.hei_s = 1.200;
							}

							// 우상단
							if ((HLEN_RT != 0) && (VLEN_RT != 0)) {
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
								placingZone.cellsRT [xx].horLen = DGGetItemValDouble (dialogID, HLEN_RT);
								placingZone.cellsRT [xx].verLen = DGGetItemValDouble (dialogID, VLEN_RT);
								placingZone.cellsRT [xx].height = 1.200;
								placingZone.cellsRT [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, VLEN_RT);
								placingZone.cellsRT [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, HLEN_RT);
								placingZone.cellsRT [xx].libPart.outcorner.hei_s = 1.200;
							}

							// 좌하단
							if ((HLEN_LB != 0) && (VLEN_LB != 0)) {
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
								placingZone.cellsLB [xx].horLen = DGGetItemValDouble (dialogID, HLEN_LB);
								placingZone.cellsLB [xx].verLen = DGGetItemValDouble (dialogID, VLEN_LB);
								placingZone.cellsLB [xx].height = 1.200;
								placingZone.cellsLB [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, VLEN_LB);
								placingZone.cellsLB [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, HLEN_LB);
								placingZone.cellsLB [xx].libPart.outcorner.hei_s = 1.200;
							}

							// 우하단
							if ((HLEN_RB != 0) && (VLEN_RB != 0)) {
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
								placingZone.cellsRB [xx].horLen = DGGetItemValDouble (dialogID, HLEN_RB);
								placingZone.cellsRB [xx].verLen = DGGetItemValDouble (dialogID, VLEN_RB);
								placingZone.cellsRB [xx].height = 1.200;
								placingZone.cellsRB [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, HLEN_RB);
								placingZone.cellsRB [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, VLEN_RB);
								placingZone.cellsRB [xx].libPart.outcorner.hei_s = 1.200;
							}

							// T1
							if (LEN_T1 != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsT1 [xx].objType = EUROFORM;
								placingZone.cellsT1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1)) * cos(placingZone.angle);
								placingZone.cellsT1 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1)) * sin(placingZone.angle);
								placingZone.cellsT1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsT1 [xx].ang = placingZone.angle + DegreeToRad (180);
								placingZone.cellsT1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_T1);
								placingZone.cellsT1 [xx].verLen = 0.064;
								placingZone.cellsT1 [xx].height = 1.200;
								placingZone.cellsT1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_T1);
								placingZone.cellsT1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_T1);
								placingZone.cellsT1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsT1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsT1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsT1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsT1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsT1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// T2
							if (LEN_T2 != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsT2 [xx].objType = EUROFORM;
								placingZone.cellsT2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1) + DGGetItemValDouble (dialogID, LEN_T2)) * cos(placingZone.angle);
								placingZone.cellsT2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1) + DGGetItemValDouble (dialogID, LEN_T2)) * sin(placingZone.angle);
								placingZone.cellsT2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsT2 [xx].ang = placingZone.angle + DegreeToRad (180);
								placingZone.cellsT2 [xx].horLen = DGGetItemValDouble (dialogID, LEN_T2);
								placingZone.cellsT2 [xx].verLen = 0.064;
								placingZone.cellsT2 [xx].height = 1.200;
								placingZone.cellsT2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_T2);
								placingZone.cellsT2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_T2);
								placingZone.cellsT2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsT2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsT2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsT2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsT2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsT2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// B1
							if (LEN_B1 != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsB1 [xx].objType = EUROFORM;
								placingZone.cellsB1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, HLEN_LB) * cos(placingZone.angle);
								placingZone.cellsB1 [xx].leftBottomY = rotatedPoint.y + DGGetItemValDouble (dialogID, HLEN_LB) * sin(placingZone.angle);
								placingZone.cellsB1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsB1 [xx].ang = placingZone.angle;
								placingZone.cellsB1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_B1);
								placingZone.cellsB1 [xx].verLen = 0.064;
								placingZone.cellsB1 [xx].height = 1.200;
								placingZone.cellsB1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_B1);
								placingZone.cellsB1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_B1);
								placingZone.cellsB1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsB1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsB1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsB1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsB1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsB1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// B2
							if (LEN_B2 != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsB2 [xx].objType = EUROFORM;
								placingZone.cellsB2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1)) * cos(placingZone.angle);
								placingZone.cellsB2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1)) * sin(placingZone.angle);
								placingZone.cellsB2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsB2 [xx].ang = placingZone.angle;
								placingZone.cellsB2 [xx].horLen = DGGetItemValDouble (dialogID, LEN_B2);
								placingZone.cellsB2 [xx].verLen = 0.064;
								placingZone.cellsB2 [xx].height = 1.200;
								placingZone.cellsB2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_B2);
								placingZone.cellsB2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_B2);
								placingZone.cellsB2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsB2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsB2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsB2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsB2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsB2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// L1
							if (LEN_L1 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsL1 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsL1 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_L1)) * sin(placingZone.angle);
									placingZone.cellsL1 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_L1)) * cos(placingZone.angle);
									placingZone.cellsL1 [xx].ang = placingZone.angle - DegreeToRad (90);
								} else {
									placingZone.cellsL1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, VLEN_LT) * sin(placingZone.angle);
									placingZone.cellsL1 [xx].leftBottomY = rotatedPoint.y - DGGetItemValDouble (dialogID, VLEN_LT) * cos(placingZone.angle);
									placingZone.cellsL1 [xx].ang = placingZone.angle - DegreeToRad (90);
								}
								placingZone.cellsL1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsL1 [xx].horLen = 0.064;
								placingZone.cellsL1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_L1);
								placingZone.cellsL1 [xx].height = 1.200;
								placingZone.cellsL1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_L1);
								placingZone.cellsL1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_L1);
								placingZone.cellsL1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsL1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsL1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsL1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsL1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsL1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// L2
							if (LEN_L2 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsL2 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsL2 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L2)) * sin(placingZone.angle);
									placingZone.cellsL2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L2)) * cos(placingZone.angle);
									placingZone.cellsL2 [xx].ang = placingZone.angle - DegreeToRad (90);
								} else {
									placingZone.cellsL2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1)) * sin(placingZone.angle);
									placingZone.cellsL2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1)) * cos(placingZone.angle);
									placingZone.cellsL2 [xx].ang = placingZone.angle - DegreeToRad (90);
								}
								placingZone.cellsL2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsL2 [xx].horLen = 0.064;
								placingZone.cellsL2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_L2);
								placingZone.cellsL2 [xx].height = 1.200;
								placingZone.cellsL2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_L2);
								placingZone.cellsL2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_L2);
								placingZone.cellsL2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsL2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsL2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsL2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsL2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsL2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// R1
							if (LEN_R1 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsR1 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsR1 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_RB) + DGGetItemValDouble (dialogID, LEN_R2)) * sin(placingZone.angle);
									placingZone.cellsR1 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_RB) + DGGetItemValDouble (dialogID, LEN_R2)) * cos(placingZone.angle);
									placingZone.cellsR1 [xx].ang = placingZone.angle + DegreeToRad (90);
								} else {
									placingZone.cellsR1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1)) * sin(placingZone.angle);
									placingZone.cellsR1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1)) * cos(placingZone.angle);
									placingZone.cellsR1 [xx].ang = placingZone.angle + DegreeToRad (90);
								}
								placingZone.cellsR1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsR1 [xx].horLen = 0.064;
								placingZone.cellsR1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_R1);
								placingZone.cellsR1 [xx].height = 1.200;
								placingZone.cellsR1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_R1);
								placingZone.cellsR1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_R1);
								placingZone.cellsR1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsR1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsR1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsR1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsR1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsR1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// R2
							if (LEN_R2 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsR2 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsR2 [xx].leftBottomX = rotatedPoint.x - DGGetItemValDouble (dialogID, VLEN_RB) * sin(placingZone.angle);
									placingZone.cellsR2 [xx].leftBottomY = rotatedPoint.y + DGGetItemValDouble (dialogID, VLEN_RB) * cos(placingZone.angle);
									placingZone.cellsR2 [xx].ang = placingZone.angle + DegreeToRad (90);
								} else {
									placingZone.cellsR2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2)) * sin(placingZone.angle);
									placingZone.cellsR2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2)) * cos(placingZone.angle);
									placingZone.cellsR2 [xx].ang = placingZone.angle + DegreeToRad (90);
								}
								placingZone.cellsR2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsR2 [xx].horLen = 0.064;
								placingZone.cellsR2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_R2);
								placingZone.cellsR2 [xx].height = 1.200;
								placingZone.cellsR2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_R2);
								placingZone.cellsR2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_R2);
								placingZone.cellsR2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsR2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsR2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsR2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsR2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsR2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// 왼쪽 인코너 셀 1 (위)
							if (LEN_Lin1_C != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsLin1 [xx].objType = INCORNER;
								placingZone.cellsLin1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_Lin1_C)) * sin(placingZone.angle);
								placingZone.cellsLin1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_Lin1_C)) * cos(placingZone.angle);
								placingZone.cellsLin1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsLin1 [xx].ang = placingZone.angle + DegreeToRad (90);
								placingZone.cellsLin1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Lin1_C);
								if (placingZone.relationCase != 4)
									placingZone.cellsLin1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin1_W);
								else
									placingZone.cellsLin1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin2_W);
								placingZone.cellsLin1 [xx].height = 1.200;
								placingZone.cellsLin1 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Lin1_C);
								if (placingZone.relationCase != 4)
									placingZone.cellsLin1 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Lin1_W);
								else
									placingZone.cellsLin1 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Lin2_W);
								placingZone.cellsLin1 [xx].libPart.incorner.hei_s = 1.200;
							}

							// 왼쪽 인코너 셀 2 (아래)
							if (LEN_Lin2_C != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsLin2 [xx].objType = INCORNER;
								placingZone.cellsLin2 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L1) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_Lin2_C)) * sin(placingZone.angle);
								placingZone.cellsLin2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L1) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_Lin2_C)) * cos(placingZone.angle);
								placingZone.cellsLin2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsLin2 [xx].ang = placingZone.angle + DegreeToRad (180);
								placingZone.cellsLin2 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Lin2_W);
								placingZone.cellsLin2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin2_C);
								placingZone.cellsLin2 [xx].height = 1.200;
								placingZone.cellsLin2 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Lin2_W);
								placingZone.cellsLin2 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Lin2_C);
								placingZone.cellsLin2 [xx].libPart.incorner.hei_s = 1.200;
							}

							// 오른쪽 인코너 셀 1 (위)
							if (LEN_Rin1_C != 0) {
								xLen = (placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsRin1 [xx].objType = INCORNER;
								placingZone.cellsRin1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2) + DGGetItemValDouble (dialogID, LEN_Rin1_C)) * sin(placingZone.angle);
								placingZone.cellsRin1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2) + DGGetItemValDouble (dialogID, LEN_Rin1_C)) * cos(placingZone.angle);
								placingZone.cellsRin1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsRin1 [xx].ang = placingZone.angle;
								if (placingZone.relationCase != 4)
									placingZone.cellsRin1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Rin1_W);
								else
									placingZone.cellsRin1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Rin2_W);
								placingZone.cellsRin1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Rin1_C);
								placingZone.cellsRin1 [xx].height = 1.200;
								if (placingZone.relationCase != 4)
									placingZone.cellsRin1 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Rin1_W);
								else
									placingZone.cellsRin1 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Rin2_W);
								placingZone.cellsRin1 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Rin1_C);
								placingZone.cellsRin1 [xx].libPart.incorner.hei_s = 1.200;
							}

							// 오른쪽 인코너 셀 2 (아래)
							if (LEN_Rin2_C != 0) {
								xLen = (placingZone.coreWidth/2 + placingZone.venThick);
								yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsRin2 [xx].objType = INCORNER;
								placingZone.cellsRin2 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_RB) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2) + DGGetItemValDouble (dialogID, LEN_Rin2_C)) * sin(placingZone.angle);
								placingZone.cellsRin2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_RB) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2) + DGGetItemValDouble (dialogID, LEN_Rin2_C)) * cos(placingZone.angle);
								placingZone.cellsRin2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsRin2 [xx].ang = placingZone.angle - DegreeToRad (90);
								placingZone.cellsRin2 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Rin2_C);
								placingZone.cellsRin2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Rin2_W);
								placingZone.cellsRin2 [xx].height = 1.200;
								placingZone.cellsRin2 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Rin2_C);
								placingZone.cellsRin2 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Rin2_W);
								placingZone.cellsRin2 [xx].libPart.incorner.hei_s = 1.200;
							}

							// W1
							if (LEN_W1 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsW1 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsW1 [xx].leftBottomX = rotatedPoint.x - (placingZone.posTopWallLine - placingZone.posTopColumnLine) * sin(placingZone.angle) + DGGetItemValDouble (dialogID, HLEN_LB) * cos(placingZone.angle);
									placingZone.cellsW1 [xx].leftBottomY = rotatedPoint.y + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * cos(placingZone.angle) + DGGetItemValDouble (dialogID, HLEN_LB) * sin(placingZone.angle);
									placingZone.cellsW1 [xx].ang = placingZone.angle + DegreeToRad (180);
									placingZone.cellsW1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB);
									placingZone.cellsW1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB);
									placingZone.cellsW1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB);
								} else {
									placingZone.cellsW1 [xx].leftBottomX = rotatedPoint.x + (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * sin(placingZone.angle) - DGGetItemValDouble (dialogID, LEN_Lin1_W) * cos(placingZone.angle);
									placingZone.cellsW1 [xx].leftBottomY = rotatedPoint.y - (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * cos(placingZone.angle) - DGGetItemValDouble (dialogID, LEN_Lin1_W) * sin(placingZone.angle);
									placingZone.cellsW1 [xx].ang = placingZone.angle;
									placingZone.cellsW1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT);
									placingZone.cellsW1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT);
									placingZone.cellsW1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT);
								}
								placingZone.cellsW1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsW1 [xx].horLen = 0.064;
								placingZone.cellsW1 [xx].height = 1.200;
								placingZone.cellsW1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsW1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsW1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsW1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsW1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsW1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// W2
							if (LEN_W2 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsW2 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsW2 [xx].leftBottomX = rotatedPoint.x - (placingZone.posTopWallLine - placingZone.posTopColumnLine) * sin(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1)) * cos(placingZone.angle);
									placingZone.cellsW2 [xx].leftBottomY = rotatedPoint.y + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1)) * sin(placingZone.angle);
									placingZone.cellsW2 [xx].ang = placingZone.angle + DegreeToRad (180);
									placingZone.cellsW2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_B1);
									placingZone.cellsW2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_B1);
									placingZone.cellsW2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_B1);
								} else {
									placingZone.cellsW2 [xx].leftBottomX = rotatedPoint.x + (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * sin(placingZone.angle) + DGGetItemValDouble (dialogID, HLEN_LT) * cos(placingZone.angle);
									placingZone.cellsW2 [xx].leftBottomY = rotatedPoint.y - (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * cos(placingZone.angle) + DGGetItemValDouble (dialogID, HLEN_LT) * sin(placingZone.angle);
									placingZone.cellsW2 [xx].ang = placingZone.angle;
									placingZone.cellsW2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_T1);
									placingZone.cellsW2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_T1);
									placingZone.cellsW2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_T1);
								}
								placingZone.cellsW2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsW2 [xx].horLen = 0.064;
								placingZone.cellsW2 [xx].height = 1.200;
								placingZone.cellsW2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsW2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsW2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsW2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsW2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsW2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// W3
							if (LEN_W3 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsW3 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsW3 [xx].leftBottomX = rotatedPoint.x - (placingZone.posTopWallLine - placingZone.posTopColumnLine) * sin(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1) + DGGetItemValDouble (dialogID, LEN_B2)) * cos(placingZone.angle);
									placingZone.cellsW3 [xx].leftBottomY = rotatedPoint.y + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1) + DGGetItemValDouble (dialogID, LEN_B2)) * sin(placingZone.angle);
									placingZone.cellsW3 [xx].ang = placingZone.angle + DegreeToRad (180);
									placingZone.cellsW3 [xx].verLen = DGGetItemValDouble (dialogID, LEN_B2);
									placingZone.cellsW3 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_B2);
									placingZone.cellsW3 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_B2);
								} else {
									placingZone.cellsW3 [xx].leftBottomX = rotatedPoint.x + (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * sin(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1)) * cos(placingZone.angle);
									placingZone.cellsW3 [xx].leftBottomY = rotatedPoint.y - (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1)) * sin(placingZone.angle);
									placingZone.cellsW3 [xx].ang = placingZone.angle;
									placingZone.cellsW3 [xx].verLen = DGGetItemValDouble (dialogID, LEN_T2);
									placingZone.cellsW3 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_T2);
									placingZone.cellsW3 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_T2);
								}
								placingZone.cellsW3 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsW3 [xx].horLen = 0.064;
								placingZone.cellsW3 [xx].height = 1.200;
								placingZone.cellsW3 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsW3 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsW3 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsW3 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsW3 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsW3 [xx].libPart.form.eu_stan_onoff = false;
							}

							// W4
							if (LEN_W4 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsW4 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsW4 [xx].leftBottomX = rotatedPoint.x - (placingZone.posTopWallLine - placingZone.posTopColumnLine) * sin(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1) + DGGetItemValDouble (dialogID, LEN_B2) + DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W)) * cos(placingZone.angle);
									placingZone.cellsW4 [xx].leftBottomY = rotatedPoint.y + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1) + DGGetItemValDouble (dialogID, LEN_B2) + DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W)) * sin(placingZone.angle);
									placingZone.cellsW4 [xx].ang = placingZone.angle + DegreeToRad (180);
									placingZone.cellsW4 [xx].verLen = DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W);
									placingZone.cellsW4 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W);
									placingZone.cellsW4 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W);
								} else {
									placingZone.cellsW4 [xx].leftBottomX = rotatedPoint.x + (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * sin(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1) + DGGetItemValDouble (dialogID, LEN_T2)) * cos(placingZone.angle);
									placingZone.cellsW4 [xx].leftBottomY = rotatedPoint.y - (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1) + DGGetItemValDouble (dialogID, LEN_T2)) * sin(placingZone.angle);
									placingZone.cellsW4 [xx].ang = placingZone.angle;
									placingZone.cellsW4 [xx].verLen = DGGetItemValDouble (dialogID, HLEN_RT) + DGGetItemValDouble (dialogID, LEN_Rin1_W);
									placingZone.cellsW4 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, HLEN_RT) + DGGetItemValDouble (dialogID, LEN_Rin1_W);
									placingZone.cellsW4 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, HLEN_RT) + DGGetItemValDouble (dialogID, LEN_Rin1_W);
								}
								placingZone.cellsW4 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsW4 [xx].horLen = 0.064;
								placingZone.cellsW4 [xx].height = 1.200;
								placingZone.cellsW4 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsW4 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsW4 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsW4 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsW4 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsW4 [xx].libPart.form.eu_stan_onoff = false;
							}
						}
					} else {
						for (xx = 0 ; xx < 20 ; ++xx) {
							// 좌상단
							if ((HLEN_LT != 0) && (VLEN_LT != 0)) {
								xLen = (placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsLT [xx].objType = OUTCORNER;
								placingZone.cellsLT [xx].leftBottomX = rotatedPoint.x;
								placingZone.cellsLT [xx].leftBottomY = rotatedPoint.y;
								placingZone.cellsLT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsLT [xx].ang = placingZone.angle - DegreeToRad (180);
								placingZone.cellsLT [xx].horLen = DGGetItemValDouble (dialogID, VLEN_LT);
								placingZone.cellsLT [xx].verLen = DGGetItemValDouble (dialogID, HLEN_LT);
								placingZone.cellsLT [xx].height = 1.200;
								placingZone.cellsLT [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, VLEN_LT);
								placingZone.cellsLT [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, HLEN_LT);
								placingZone.cellsLT [xx].libPart.outcorner.hei_s = 1.200;
							}

							// 우상단
							if ((HLEN_RT != 0) && (VLEN_RT != 0)) {
								xLen = (placingZone.coreWidth/2 + placingZone.venThick);
								yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsRT [xx].objType = OUTCORNER;
								placingZone.cellsRT [xx].leftBottomX = rotatedPoint.x;
								placingZone.cellsRT [xx].leftBottomY = rotatedPoint.y;
								placingZone.cellsRT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsRT [xx].ang = placingZone.angle + DegreeToRad (90);
								placingZone.cellsRT [xx].horLen = DGGetItemValDouble (dialogID, VLEN_RT);
								placingZone.cellsRT [xx].verLen = DGGetItemValDouble (dialogID, HLEN_RT);
								placingZone.cellsRT [xx].height = 1.200;
								placingZone.cellsRT [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, HLEN_RT);
								placingZone.cellsRT [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, VLEN_RT);
								placingZone.cellsRT [xx].libPart.outcorner.hei_s = 1.200;
							}

							// 좌하단
							if ((HLEN_LB != 0) && (VLEN_LB != 0)) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsLB [xx].objType = OUTCORNER;
								placingZone.cellsLB [xx].leftBottomX = rotatedPoint.x;
								placingZone.cellsLB [xx].leftBottomY = rotatedPoint.y;
								placingZone.cellsLB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsLB [xx].ang = placingZone.angle - DegreeToRad (90);
								placingZone.cellsLB [xx].horLen = DGGetItemValDouble (dialogID, VLEN_LB);
								placingZone.cellsLB [xx].verLen = DGGetItemValDouble (dialogID, HLEN_LB);
								placingZone.cellsLB [xx].height = 1.200;
								placingZone.cellsLB [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, HLEN_LB);
								placingZone.cellsLB [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, VLEN_LB);
								placingZone.cellsLB [xx].libPart.outcorner.hei_s = 1.200;
							}

							// 우하단
							if ((HLEN_RB != 0) && (VLEN_RB != 0)) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsRB [xx].objType = OUTCORNER;
								placingZone.cellsRB [xx].leftBottomX = rotatedPoint.x;
								placingZone.cellsRB [xx].leftBottomY = rotatedPoint.y;
								placingZone.cellsRB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsRB [xx].ang = placingZone.angle;
								placingZone.cellsRB [xx].horLen = DGGetItemValDouble (dialogID, VLEN_RB);
								placingZone.cellsRB [xx].verLen = DGGetItemValDouble (dialogID, HLEN_RB);
								placingZone.cellsRB [xx].height = 1.200;
								placingZone.cellsRB [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, VLEN_RB);
								placingZone.cellsRB [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, HLEN_RB);
								placingZone.cellsRB [xx].libPart.outcorner.hei_s = 1.200;
							}

							// T1
							if (LEN_T1 != 0) {
								xLen = (placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsT1 [xx].objType = EUROFORM;
								placingZone.cellsT1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1)) * sin(placingZone.angle);
								placingZone.cellsT1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1)) * cos(placingZone.angle);
								placingZone.cellsT1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsT1 [xx].ang = placingZone.angle + DegreeToRad (90);
								placingZone.cellsT1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_T1);
								placingZone.cellsT1 [xx].verLen = 0.064;
								placingZone.cellsT1 [xx].height = 1.200;
								placingZone.cellsT1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_T1);
								placingZone.cellsT1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_T1);
								placingZone.cellsT1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsT1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsT1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsT1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsT1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsT1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// T2
							if (LEN_T2 != 0) {
								xLen = (placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsT2 [xx].objType = EUROFORM;
								placingZone.cellsT2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1) + DGGetItemValDouble (dialogID, LEN_T2)) * sin(placingZone.angle);
								placingZone.cellsT2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1) + DGGetItemValDouble (dialogID, LEN_T2)) * cos(placingZone.angle);
								placingZone.cellsT2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsT2 [xx].ang = placingZone.angle + DegreeToRad (90);
								placingZone.cellsT2 [xx].horLen = DGGetItemValDouble (dialogID, LEN_T2);
								placingZone.cellsT2 [xx].verLen = 0.064;
								placingZone.cellsT2 [xx].height = 1.200;
								placingZone.cellsT2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_T2);
								placingZone.cellsT2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_T2);
								placingZone.cellsT2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsT2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsT2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsT2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsT2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsT2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// B1
							if (LEN_B1 != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsB1 [xx].objType = EUROFORM;
								placingZone.cellsB1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, HLEN_LB) * sin(placingZone.angle);
								placingZone.cellsB1 [xx].leftBottomY = rotatedPoint.y - DGGetItemValDouble (dialogID, HLEN_LB) * cos(placingZone.angle);
								placingZone.cellsB1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsB1 [xx].ang = placingZone.angle - DegreeToRad (90);
								placingZone.cellsB1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_B1);
								placingZone.cellsB1 [xx].verLen = 0.064;
								placingZone.cellsB1 [xx].height = 1.200;
								placingZone.cellsB1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_B1);
								placingZone.cellsB1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_B1);
								placingZone.cellsB1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsB1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsB1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsB1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsB1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsB1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// B2
							if (LEN_B2 != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsB2 [xx].objType = EUROFORM;
								placingZone.cellsB2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1)) * sin(placingZone.angle);
								placingZone.cellsB2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1)) * cos(placingZone.angle);
								placingZone.cellsB2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsB2 [xx].ang = placingZone.angle - DegreeToRad (90);
								placingZone.cellsB2 [xx].horLen = DGGetItemValDouble (dialogID, LEN_B2);
								placingZone.cellsB2 [xx].verLen = 0.064;
								placingZone.cellsB2 [xx].height = 1.200;
								placingZone.cellsB2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_B2);
								placingZone.cellsB2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_B2);
								placingZone.cellsB2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsB2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsB2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsB2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsB2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsB2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// L1
							if (LEN_L1 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsL1 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsL1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_L1)) * cos(placingZone.angle);
									placingZone.cellsL1 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_L1)) * sin(placingZone.angle);
									placingZone.cellsL1 [xx].ang = placingZone.angle - DegreeToRad (180);
								} else {
									placingZone.cellsL1 [xx].leftBottomX = rotatedPoint.x - DGGetItemValDouble (dialogID, VLEN_LT) * cos(placingZone.angle);
									placingZone.cellsL1 [xx].leftBottomY = rotatedPoint.y - DGGetItemValDouble (dialogID, VLEN_LT) * sin(placingZone.angle);
									placingZone.cellsL1 [xx].ang = placingZone.angle - DegreeToRad (180);
								}
								placingZone.cellsL1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsL1 [xx].horLen = 0.064;
								placingZone.cellsL1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_L1);
								placingZone.cellsL1 [xx].height = 1.200;
								placingZone.cellsL1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_L1);
								placingZone.cellsL1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_L1);
								placingZone.cellsL1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsL1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsL1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsL1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsL1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsL1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// L2
							if (LEN_L2 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsL2 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsL2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L2)) * cos(placingZone.angle);
									placingZone.cellsL2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L2)) * sin(placingZone.angle);
									placingZone.cellsL2 [xx].ang = placingZone.angle - DegreeToRad (180);
								} else {
									placingZone.cellsL2 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1)) * cos(placingZone.angle);
									placingZone.cellsL2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1)) * sin(placingZone.angle);
									placingZone.cellsL2 [xx].ang = placingZone.angle - DegreeToRad (180);
								}
								placingZone.cellsL2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsL2 [xx].horLen = 0.064;
								placingZone.cellsL2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_L2);
								placingZone.cellsL2 [xx].height = 1.200;
								placingZone.cellsL2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_L2);
								placingZone.cellsL2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_L2);
								placingZone.cellsL2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsL2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsL2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsL2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsL2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsL2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// R1
							if (LEN_R1 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsR1 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsR1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_RB) + DGGetItemValDouble (dialogID, LEN_R2)) * cos(placingZone.angle);
									placingZone.cellsR1 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_RB) + DGGetItemValDouble (dialogID, LEN_R2)) * sin(placingZone.angle);
									placingZone.cellsR1 [xx].ang = placingZone.angle;
								} else {
									placingZone.cellsR1 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1)) * cos(placingZone.angle);
									placingZone.cellsR1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1)) * sin(placingZone.angle);
									placingZone.cellsR1 [xx].ang = placingZone.angle;
								}
								placingZone.cellsR1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsR1 [xx].horLen = 0.064;
								placingZone.cellsR1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_R1);
								placingZone.cellsR1 [xx].height = 1.200;
								placingZone.cellsR1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_R1);
								placingZone.cellsR1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_R1);
								placingZone.cellsR1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsR1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsR1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsR1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsR1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsR1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// R2
							if (LEN_R2 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsR2 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsR2 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, VLEN_RB) * cos(placingZone.angle);
									placingZone.cellsR2 [xx].leftBottomY = rotatedPoint.y + DGGetItemValDouble (dialogID, VLEN_RB) * sin(placingZone.angle);
									placingZone.cellsR2 [xx].ang = placingZone.angle;
								} else {
									placingZone.cellsR2 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2)) * cos(placingZone.angle);
									placingZone.cellsR2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2)) * sin(placingZone.angle);
									placingZone.cellsR2 [xx].ang = placingZone.angle;
								}
								placingZone.cellsR2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsR2 [xx].horLen = 0.064;
								placingZone.cellsR2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_R2);
								placingZone.cellsR2 [xx].height = 1.200;
								placingZone.cellsR2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_R2);
								placingZone.cellsR2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_R2);
								placingZone.cellsR2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsR2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsR2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsR2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsR2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsR2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// 왼쪽 인코너 셀 1 (위)
							if (LEN_Lin1_C != 0) {
								xLen = (placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsLin1 [xx].objType = INCORNER;
								placingZone.cellsLin1 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_Lin1_C)) * cos(placingZone.angle);
								placingZone.cellsLin1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_Lin1_C)) * sin(placingZone.angle);
								placingZone.cellsLin1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsLin1 [xx].ang = placingZone.angle;
								placingZone.cellsLin1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Lin1_C);
								if (placingZone.relationCase != 4)
									placingZone.cellsLin1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin1_W);
								else
									placingZone.cellsLin1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin2_W);
								placingZone.cellsLin1 [xx].height = 1.200;
								placingZone.cellsLin1 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Lin1_C);
								if (placingZone.relationCase != 4)
									placingZone.cellsLin1 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Lin1_W);
								else
									placingZone.cellsLin1 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Lin2_W);
								placingZone.cellsLin1 [xx].libPart.incorner.hei_s = 1.200;
							}

							// 왼쪽 인코너 셀 2 (아래)
							if (LEN_Lin2_C != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsLin2 [xx].objType = INCORNER;
								placingZone.cellsLin2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L1) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_Lin2_C)) * cos(placingZone.angle);
								placingZone.cellsLin2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_LB) + DGGetItemValDouble (dialogID, LEN_L1) + DGGetItemValDouble (dialogID, LEN_L2) + DGGetItemValDouble (dialogID, LEN_Lin2_C)) * sin(placingZone.angle);
								placingZone.cellsLin2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsLin2 [xx].ang = placingZone.angle + DegreeToRad (90);
								placingZone.cellsLin2 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Lin2_W);
								placingZone.cellsLin2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin2_C);
								placingZone.cellsLin2 [xx].height = 1.200;
								placingZone.cellsLin2 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Lin2_W);
								placingZone.cellsLin2 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Lin2_C);
								placingZone.cellsLin2 [xx].libPart.incorner.hei_s = 1.200;
							}

							// 오른쪽 인코너 셀 1 (위)
							if (LEN_Rin1_C != 0) {
								xLen = (placingZone.coreWidth/2 + placingZone.venThick);
								yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsRin1 [xx].objType = INCORNER;
								placingZone.cellsRin1 [xx].leftBottomX = rotatedPoint.x - (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2) + DGGetItemValDouble (dialogID, LEN_Rin1_C)) * cos(placingZone.angle);
								placingZone.cellsRin1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, VLEN_RT) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2) + DGGetItemValDouble (dialogID, LEN_Rin1_C)) * sin(placingZone.angle);
								placingZone.cellsRin1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsRin1 [xx].ang = placingZone.angle - DegreeToRad (90);
								if (placingZone.relationCase != 4)
									placingZone.cellsRin1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Rin1_W);
								else
									placingZone.cellsRin1 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Rin2_W);
								placingZone.cellsRin1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Rin1_C);
								placingZone.cellsRin1 [xx].height = 1.200;
								if (placingZone.relationCase != 4)
									placingZone.cellsRin1 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Rin1_W);
								else
									placingZone.cellsRin1 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Rin2_W);
								placingZone.cellsRin1 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Rin1_C);
								placingZone.cellsRin1 [xx].libPart.incorner.hei_s = 1.200;
							}

							// 오른쪽 인코너 셀 2 (아래)
							if (LEN_Rin2_C != 0) {
								xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
								yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsRin2 [xx].objType = INCORNER;
								placingZone.cellsRin2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, VLEN_RB) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2) + DGGetItemValDouble (dialogID, LEN_Rin2_C)) * cos(placingZone.angle);
								placingZone.cellsRin2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, VLEN_RB) + DGGetItemValDouble (dialogID, LEN_R1) + DGGetItemValDouble (dialogID, LEN_R2) + DGGetItemValDouble (dialogID, LEN_Rin2_C)) * sin(placingZone.angle);
								placingZone.cellsRin2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsRin2 [xx].ang = placingZone.angle - DegreeToRad (180);
								placingZone.cellsRin2 [xx].horLen = DGGetItemValDouble (dialogID, LEN_Rin2_C);
								placingZone.cellsRin2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Rin2_W);
								placingZone.cellsRin2 [xx].height = 1.200;
								placingZone.cellsRin2 [xx].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, LEN_Rin2_C);
								placingZone.cellsRin2 [xx].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, LEN_Rin2_W);
								placingZone.cellsRin2 [xx].libPart.incorner.hei_s = 1.200;
							}

							// W1
							if (LEN_W1 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsW1 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsW1 [xx].leftBottomX = rotatedPoint.x + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * cos(placingZone.angle) + DGGetItemValDouble (dialogID, HLEN_LB) * sin(placingZone.angle);
									placingZone.cellsW1 [xx].leftBottomY = rotatedPoint.y + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * sin(placingZone.angle) - DGGetItemValDouble (dialogID, HLEN_LB) * cos(placingZone.angle);
									placingZone.cellsW1 [xx].ang = placingZone.angle + DegreeToRad (90);
									placingZone.cellsW1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB);
									placingZone.cellsW1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB);
									placingZone.cellsW1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_Lin2_W) + DGGetItemValDouble (dialogID, HLEN_LB);
								} else {
									placingZone.cellsW1 [xx].leftBottomX = rotatedPoint.x - (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * cos(placingZone.angle) + DGGetItemValDouble (dialogID, LEN_Lin1_W) * sin(placingZone.angle);
									placingZone.cellsW1 [xx].leftBottomY = rotatedPoint.y + (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * sin(placingZone.angle) + DGGetItemValDouble (dialogID, LEN_Lin1_W) * cos(placingZone.angle);
									placingZone.cellsW1 [xx].ang = placingZone.angle - DegreeToRad (90);
									placingZone.cellsW1 [xx].verLen = DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT);
									placingZone.cellsW1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT);
									placingZone.cellsW1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_Lin1_W) + DGGetItemValDouble (dialogID, HLEN_LT);
								}
								placingZone.cellsW1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsW1 [xx].horLen = 0.064;
								placingZone.cellsW1 [xx].height = 1.200;
								placingZone.cellsW1 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsW1 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsW1 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsW1 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsW1 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsW1 [xx].libPart.form.eu_stan_onoff = false;
							}

							// W2
							if (LEN_W2 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsW2 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsW2 [xx].leftBottomX = rotatedPoint.x + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1)) * sin(placingZone.angle);
									placingZone.cellsW2 [xx].leftBottomY = rotatedPoint.y + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * sin(placingZone.angle) - (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1)) * cos(placingZone.angle);
									placingZone.cellsW2 [xx].ang = placingZone.angle + DegreeToRad (90);
									placingZone.cellsW2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_B1);
									placingZone.cellsW2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_B1);
									placingZone.cellsW2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_B1);
								} else {
									placingZone.cellsW2 [xx].leftBottomX = rotatedPoint.x - (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * cos(placingZone.angle) + DGGetItemValDouble (dialogID, HLEN_LT) * sin(placingZone.angle);
									placingZone.cellsW2 [xx].leftBottomY = rotatedPoint.y + (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * sin(placingZone.angle) - DGGetItemValDouble (dialogID, HLEN_LT) * cos(placingZone.angle);
									placingZone.cellsW2 [xx].ang = placingZone.angle - DegreeToRad (90);
									placingZone.cellsW2 [xx].verLen = DGGetItemValDouble (dialogID, LEN_T1);
									placingZone.cellsW2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_T1);
									placingZone.cellsW2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_T1);
								}
								placingZone.cellsW2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsW2 [xx].horLen = 0.064;
								placingZone.cellsW2 [xx].height = 1.200;
								placingZone.cellsW2 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsW2 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsW2 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsW2 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsW2 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsW2 [xx].libPart.form.eu_stan_onoff = false;
							}

							// W3
							if (LEN_W3 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsW3 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsW3 [xx].leftBottomX = rotatedPoint.x + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1) + DGGetItemValDouble (dialogID, LEN_B2)) * sin(placingZone.angle);
									placingZone.cellsW3 [xx].leftBottomY = rotatedPoint.y + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * sin(placingZone.angle) - (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1) + DGGetItemValDouble (dialogID, LEN_B2)) * cos(placingZone.angle);
									placingZone.cellsW3 [xx].ang = placingZone.angle + DegreeToRad (90);
									placingZone.cellsW3 [xx].verLen = DGGetItemValDouble (dialogID, LEN_B2);
									placingZone.cellsW3 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_B2);
									placingZone.cellsW3 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_B2);
								} else {
									placingZone.cellsW3 [xx].leftBottomX = rotatedPoint.x - (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1)) * sin(placingZone.angle);
									placingZone.cellsW3 [xx].leftBottomY = rotatedPoint.y + (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * sin(placingZone.angle) - (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_L1)) * cos(placingZone.angle);
									placingZone.cellsW3 [xx].ang = placingZone.angle - DegreeToRad (90);
									placingZone.cellsW3 [xx].verLen = DGGetItemValDouble (dialogID, LEN_T2);
									placingZone.cellsW3 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, LEN_T2);
									placingZone.cellsW3 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, LEN_T2);
								}
								placingZone.cellsW3 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsW3 [xx].horLen = 0.064;
								placingZone.cellsW3 [xx].height = 1.200;
								placingZone.cellsW3 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsW3 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsW3 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsW3 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsW3 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsW3 [xx].libPart.form.eu_stan_onoff = false;
							}

							// W4
							if (LEN_W4 != 0) {
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									xLen = (placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								} else {
									xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
									yLen = (placingZone.coreDepth/2 + placingZone.venThick);
								}
								lineLen = sqrt (xLen*xLen + yLen*yLen);
								rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
								rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

								placingZone.cellsW4 [xx].objType = EUROFORM;
								if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
									placingZone.cellsW4 [xx].leftBottomX = rotatedPoint.x + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1) + DGGetItemValDouble (dialogID, LEN_B2) + DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W)) * sin(placingZone.angle);
									placingZone.cellsW4 [xx].leftBottomY = rotatedPoint.y + (placingZone.posTopWallLine - placingZone.posTopColumnLine) * sin(placingZone.angle) - (DGGetItemValDouble (dialogID, HLEN_LB) + DGGetItemValDouble (dialogID, LEN_B1) + DGGetItemValDouble (dialogID, LEN_B2) + DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W)) * cos(placingZone.angle);
									placingZone.cellsW4 [xx].ang = placingZone.angle + DegreeToRad (90);
									placingZone.cellsW4 [xx].verLen = DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W);
									placingZone.cellsW4 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W);
									placingZone.cellsW4 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, HLEN_RB) + DGGetItemValDouble (dialogID, LEN_Rin2_W);
								} else {
									placingZone.cellsW4 [xx].leftBottomX = rotatedPoint.x - (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * cos(placingZone.angle) + (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1) + DGGetItemValDouble (dialogID, LEN_T2)) * sin(placingZone.angle);
									placingZone.cellsW4 [xx].leftBottomY = rotatedPoint.y + (placingZone.posBottomColumnLine - placingZone.posBottomWallLine) * sin(placingZone.angle) - (DGGetItemValDouble (dialogID, HLEN_LT) + DGGetItemValDouble (dialogID, LEN_T1) + DGGetItemValDouble (dialogID, LEN_T2)) * cos(placingZone.angle);
									placingZone.cellsW4 [xx].ang = placingZone.angle - DegreeToRad (90);
									placingZone.cellsW4 [xx].verLen = DGGetItemValDouble (dialogID, HLEN_RT) + DGGetItemValDouble (dialogID, LEN_Rin1_W);
									placingZone.cellsW4 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, HLEN_RT) + DGGetItemValDouble (dialogID, LEN_Rin1_W);
									placingZone.cellsW4 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, HLEN_RT) + DGGetItemValDouble (dialogID, LEN_Rin1_W);
								}
								placingZone.cellsW4 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
								placingZone.cellsW4 [xx].horLen = 0.064;
								placingZone.cellsW4 [xx].height = 1.200;
								placingZone.cellsW4 [xx].libPart.form.eu_hei = 1.200;
								placingZone.cellsW4 [xx].libPart.form.eu_hei2 = 1.200;
								placingZone.cellsW4 [xx].libPart.form.u_ins_wall = true;
								formWidth = placingZone.cellsW4 [xx].libPart.form.eu_wid;
								if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
									placingZone.cellsW4 [xx].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cellsW4 [xx].libPart.form.eu_stan_onoff = false;
							}
						}
					}

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC);
					layerInd_Incorner		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC);
					layerInd_Outcorner		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC);

					break;

				case BUTTON_AUTOSET_WC:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					layerInd_Euroform	= makeTemporaryLayer (structuralObject_forEuroformColumn, "UFOM", NULL);
					layerInd_Incorner	= makeTemporaryLayer (structuralObject_forEuroformColumn, "INCO", NULL);
					layerInd_Outcorner	= makeTemporaryLayer (structuralObject_forEuroformColumn, "OUTP", NULL);
					layerInd_Plywood	= makeTemporaryLayer (structuralObject_forEuroformColumn, "PLYW", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_WC, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_WC, layerInd_Incorner);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_WC, layerInd_Outcorner);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD_WC, layerInd_Plywood);

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
short DGCALLBACK columnPlacerHandler_wallColumn_2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGDisableItem (dialogID, DG_UPDATE_BUTTON);

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

			// 보를 의미하는 직사각형
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
			DGShowItem (dialogID, itmIdx);

			// 보 단면
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, itmIdx);
			DGDisableItem (dialogID, itmIdx);
			if (placingZone.bInterfereBeam == true)
				DGSetItemText (dialogID, itmIdx, "보\n있음");
			else
				DGSetItemText (dialogID, itmIdx, "보\n없음");

			// 여백 위치
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백");
			DGShowItem (dialogID, itmIdx);

			// 유로폼 버튼 시작
			btnPosX = 230;
			btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
					if (placingZone.cellsB1 [xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsB1 [xx].height * 1000);
					}
				} else {
					if (placingZone.cellsT1 [xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsT1 [xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsT1 [xx].height * 1000);
					}
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				btnPosY -= 50;

				if (xx == 0)
					EUROFORM_BUTTON_BOTTOM = itmIdx;
				if (xx == placingZone.nCells-1)
					EUROFORM_BUTTON_TOP = itmIdx;
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
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
					placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;
				else
					placingZone.marginTopAtNorth -= placingZone.cellsT1 [xx].height;
			}

			// 여백 계산 (남)
			if (placingZone.bExistBeams [SOUTH] == true)
				placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
			else
				placingZone.marginTopAtSouth = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
					placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;
				else
					placingZone.marginTopAtSouth -= placingZone.cellsT1 [xx].height;
			}

			// 여백 계산 (서)
			if (placingZone.bExistBeams [WEST] == true)
				placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
			else
				placingZone.marginTopAtWest = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
					placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;
				else
					placingZone.marginTopAtWest -= placingZone.cellsT1 [xx].height;
			}

			// 여백 계산 (동)
			if (placingZone.bExistBeams [EAST] == true)
				placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
			else
				placingZone.marginTopAtEast = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
					placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;
				else
					placingZone.marginTopAtEast -= placingZone.cellsT1 [xx].height;
			}

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
			dialogSizeX = 700;
			dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;

		case DG_MSG_CHANGE:

			break;

		case DG_MSG_CLICK:

			// 업데이트 버튼
			if (item == DG_UPDATE_BUTTON) {
				item = 0;

				// 저장된 여백 채움 여부 저장
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				placingZone.alignPlacingZone_wallColumn (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 보를 의미하는 직사각형
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// 보 단면
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, "보\n있음");
				else
					DGSetItemText (dialogID, itmIdx, "보\n없음");

				// 여백 위치
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);

				// 유로폼 버튼 시작
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
						if (placingZone.cellsB1 [xx].objType == NONE) {
							txtButton = "NONE";
						} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
							txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsB1 [xx].height * 1000);
						}
					} else {
						if (placingZone.cellsT1 [xx].objType == NONE) {
							txtButton = "NONE";
						} else if (placingZone.cellsT1 [xx].objType == EUROFORM) {
							txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsT1 [xx].height * 1000);
						}
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					btnPosY -= 50;

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
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
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 북
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 남
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 서
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 동
				CHECKBOX_EAST_MARGIN = itmIdx;

				// 여백 계산 (북)
				if (placingZone.bExistBeams [NORTH] == true)
					placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
				else
					placingZone.marginTopAtNorth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtNorth -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (남)
				if (placingZone.bExistBeams [SOUTH] == true)
					placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
				else
					placingZone.marginTopAtSouth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtSouth -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (서)
				if (placingZone.bExistBeams [WEST] == true)
					placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
				else
					placingZone.marginTopAtWest = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtWest -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (동)
				if (placingZone.bExistBeams [EAST] == true)
					placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
				else
					placingZone.marginTopAtEast = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtEast -= placingZone.cellsT1 [xx].height;
				}

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
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// 이전 버튼
			if (item == DG_PREV) {
				clickedPrevButton = true;
			}

			// 확인 버튼
			if (item == DG_OK) {
				clickedOKButton = true;

				// 저장된 여백 채움 여부 저장
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				placingZone.alignPlacingZone_wallColumn (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 보를 의미하는 직사각형
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// 보 단면
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, "보\n있음");
				else
					DGSetItemText (dialogID, itmIdx, "보\n없음");

				// 여백 위치
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);

				// 유로폼 버튼 시작
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
						if (placingZone.cellsB1 [xx].objType == NONE) {
							txtButton = "NONE";
						} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
							txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsB1 [xx].height * 1000);
						}
					} else {
						if (placingZone.cellsT1 [xx].objType == NONE) {
							txtButton = "NONE";
						} else if (placingZone.cellsT1 [xx].objType == EUROFORM) {
							txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsT1 [xx].height * 1000);
						}
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					btnPosY -= 50;

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
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
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 북
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 남
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 서
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 동
				CHECKBOX_EAST_MARGIN = itmIdx;

				// 여백 계산 (북)
				if (placingZone.bExistBeams [NORTH] == true)
					placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
				else
					placingZone.marginTopAtNorth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtNorth -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (남)
				if (placingZone.bExistBeams [SOUTH] == true)
					placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
				else
					placingZone.marginTopAtSouth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtSouth -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (서)
				if (placingZone.bExistBeams [WEST] == true)
					placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
				else
					placingZone.marginTopAtWest = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtWest -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (동)
				if (placingZone.bExistBeams [EAST] == true)
					placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
				else
					placingZone.marginTopAtEast = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtEast -= placingZone.cellsT1 [xx].height;
				}

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
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// 취소 버튼
			if (item == DG_CANCEL) {
			}

			// 셀 추가/삭제 버튼
			if (item == ADD_CELLS) {
				placingZone.addTopCell (&placingZone);
			}
			if (item == DEL_CELLS) {
				placingZone.delTopCell (&placingZone);
			}

			if ((item == ADD_CELLS) || (item == DEL_CELLS)) {
				item = 0;

				// 저장된 여백 채움 여부 저장
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				placingZone.alignPlacingZone_wallColumn (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 보를 의미하는 직사각형
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// 보 단면
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, "보\n있음");
				else
					DGSetItemText (dialogID, itmIdx, "보\n없음");

				// 여백 위치
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);

				// 유로폼 버튼 시작
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
						if (placingZone.cellsB1 [xx].objType == NONE) {
							txtButton = "NONE";
						} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
							txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsB1 [xx].height * 1000);
						}
					} else {
						if (placingZone.cellsT1 [xx].objType == NONE) {
							txtButton = "NONE";
						} else if (placingZone.cellsT1 [xx].objType == EUROFORM) {
							txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsT1 [xx].height * 1000);
						}
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					btnPosY -= 50;

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
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
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 북
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 남
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 서
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 동
				CHECKBOX_EAST_MARGIN = itmIdx;

				// 여백 계산 (북)
				if (placingZone.bExistBeams [NORTH] == true)
					placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
				else
					placingZone.marginTopAtNorth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtNorth -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (남)
				if (placingZone.bExistBeams [SOUTH] == true)
					placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
				else
					placingZone.marginTopAtSouth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtSouth -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (서)
				if (placingZone.bExistBeams [WEST] == true)
					placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
				else
					placingZone.marginTopAtWest = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtWest -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (동)
				if (placingZone.bExistBeams [EAST] == true)
					placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
				else
					placingZone.marginTopAtEast = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtEast -= placingZone.cellsT1 [xx].height;
				}

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
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// 유로폼 버튼
			if ((item >= EUROFORM_BUTTON_BOTTOM) && (item <= EUROFORM_BUTTON_TOP)) {
				// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
				clickedBtnItemIdx = item;
				result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, columnPlacerHandler_wallColumn_3, 0);
				item = 0;

				// 저장된 여백 채움 여부 저장
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				placingZone.alignPlacingZone_wallColumn (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 보를 의미하는 직사각형
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// 보 단면
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, "보\n있음");
				else
					DGSetItemText (dialogID, itmIdx, "보\n없음");

				// 여백 위치
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);

				// 유로폼 버튼 시작
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
						if (placingZone.cellsB1 [xx].objType == NONE) {
							txtButton = "NONE";
						} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
							txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsB1 [xx].height * 1000);
						}
					} else {
						if (placingZone.cellsT1 [xx].objType == NONE) {
							txtButton = "NONE";
						} else if (placingZone.cellsT1 [xx].objType == EUROFORM) {
							txtButton = format_string ("유로폼\n↕%.0f", placingZone.cellsT1 [xx].height * 1000);
						}
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					btnPosY -= 50;

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
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
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 북
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 남
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 서
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// 동
				CHECKBOX_EAST_MARGIN = itmIdx;

				// 여백 계산 (북)
				if (placingZone.bExistBeams [NORTH] == true)
					placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
				else
					placingZone.marginTopAtNorth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtNorth -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (남)
				if (placingZone.bExistBeams [SOUTH] == true)
					placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
				else
					placingZone.marginTopAtSouth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtSouth -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (서)
				if (placingZone.bExistBeams [WEST] == true)
					placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
				else
					placingZone.marginTopAtWest = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtWest -= placingZone.cellsT1 [xx].height;
				}

				// 여백 계산 (동)
				if (placingZone.bExistBeams [EAST] == true)
					placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
				else
					placingZone.marginTopAtEast = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
					if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) )
						placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;
					else
						placingZone.marginTopAtEast -= placingZone.cellsT1 [xx].height;
				}

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
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
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

// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그
short DGCALLBACK columnPlacerHandler_wallColumn_3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idx = -1;
	short	popupSelectedIdx = -1;
	double	length;

	switch (message) {
		case DG_MSG_INIT:

			// 배치 버튼
			if ((clickedBtnItemIdx >= EUROFORM_BUTTON_BOTTOM) && (clickedBtnItemIdx <= EUROFORM_BUTTON_TOP))
				idx = clickedBtnItemIdx - EUROFORM_BUTTON_BOTTOM;

			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "셀 설정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 저장 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 160, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "저장");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 110, 160, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
			// 라벨: 객체 타입
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 10, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, "객체 타입");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// 팝업컨트롤: 객체 타입을 바꿀 수 있는 콤보박스가 맨 위에 나옴
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 90, 20-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "유로폼");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);

			// 체크박스: 규격폼
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20, 60, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "규격폼");

			// 라벨: 길이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 90, 50, 23);
			DGSetItemFont (dialogID, LABEL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LENGTH, "길이");

			// Edit 컨트롤: 길이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 90-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

			// 팝업 컨트롤: 길이
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 80, 90-6, 80, 25);
			DGSetItemFont (dialogID, POPUP_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM, "600");

			// 초기 입력 필드 표시
			if ( (placingZone.relationCase >= 1) && (placingZone.relationCase <= 3) ) {
				if (placingZone.cellsB1 [idx].objType == NONE) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, NONE + 1);

				} else if (placingZone.cellsB1 [idx].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

					DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
					DGShowItem (dialogID, LABEL_LENGTH);
					if (placingZone.cellsB1 [idx].libPart.form.eu_stan_onoff == true) {
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
						DGShowItem (dialogID, POPUP_LENGTH);
						if (abs (placingZone.cellsB1 [idx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs (placingZone.cellsB1 [idx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs (placingZone.cellsB1 [idx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
					} else {
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, FALSE);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);
						DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellsB1 [idx].libPart.form.eu_hei2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_LENGTH, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_LENGTH, 1.500);
					}
				}
			} else {
				if (placingZone.cellsT1 [idx].objType == NONE) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, NONE + 1);

				} else if (placingZone.cellsT1 [idx].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

					DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
					DGShowItem (dialogID, LABEL_LENGTH);
					if (placingZone.cellsT1 [idx].libPart.form.eu_stan_onoff == true) {
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
						DGShowItem (dialogID, POPUP_LENGTH);
						if (abs (placingZone.cellsT1 [idx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs (placingZone.cellsT1 [idx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs (placingZone.cellsT1 [idx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
					} else {
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, FALSE);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);
						DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellsT1 [idx].libPart.form.eu_hei2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_LENGTH, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_LENGTH, 1.500);
					}
				}
			}

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case POPUP_OBJ_TYPE:
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
						DGHideItem (dialogID, CHECKBOX_SET_STANDARD);
						DGHideItem (dialogID, LABEL_LENGTH);
						DGHideItem (dialogID, EDITCONTROL_LENGTH);
						DGHideItem (dialogID, POPUP_LENGTH);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
						DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, POPUP_LENGTH);
						DGHideItem (dialogID, EDITCONTROL_LENGTH);
					}
					break;

				case CHECKBOX_SET_STANDARD:
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
						DGShowItem (dialogID, POPUP_LENGTH);
						DGHideItem (dialogID, EDITCONTROL_LENGTH);
					} else {
						DGHideItem (dialogID, POPUP_LENGTH);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);
					}
					break;
			}
		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					idx = -1;

					// 배치 버튼
					if ((clickedBtnItemIdx >= EUROFORM_BUTTON_BOTTOM) && (clickedBtnItemIdx <= EUROFORM_BUTTON_TOP))
						idx = clickedBtnItemIdx - EUROFORM_BUTTON_BOTTOM;

					// 입력한 길이를 해당 셀의 모든 객체들에게 적용함
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {

						placingZone.cellsLT [idx].objType = NONE;
						placingZone.cellsRT [idx].objType = NONE;
						placingZone.cellsLB [idx].objType = NONE;
						placingZone.cellsRB [idx].objType = NONE;
						placingZone.cellsT1 [idx].objType = NONE;
						placingZone.cellsT2 [idx].objType = NONE;
						placingZone.cellsL1 [idx].objType = NONE;
						placingZone.cellsL2 [idx].objType = NONE;
						placingZone.cellsR1 [idx].objType = NONE;
						placingZone.cellsR2 [idx].objType = NONE;
						placingZone.cellsB1 [idx].objType = NONE;
						placingZone.cellsB2 [idx].objType = NONE;

						placingZone.cellsLin1 [idx].objType = NONE;
						placingZone.cellsLin2 [idx].objType = NONE;
						placingZone.cellsRin1 [idx].objType = NONE;
						placingZone.cellsRin2 [idx].objType = NONE;

						placingZone.cellsW1 [idx].objType = NONE;
						placingZone.cellsW2 [idx].objType = NONE;
						placingZone.cellsW3 [idx].objType = NONE;
						placingZone.cellsW4 [idx].objType = NONE;

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

						// 규격폼으로 저장할 경우
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
							length = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
						// 비규격폼으로 저장할 경우
						else
							length = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

						placingZone.cellsLT [idx].objType = OUTCORNER;
						placingZone.cellsLT [idx].height = length;
						placingZone.cellsLT [idx].libPart.outcorner.hei_s = length;

						placingZone.cellsRT [idx].objType = OUTCORNER;
						placingZone.cellsRT [idx].height = length;
						placingZone.cellsRT [idx].libPart.outcorner.hei_s = length;

						placingZone.cellsLB [idx].objType = OUTCORNER;
						placingZone.cellsLB [idx].height = length;
						placingZone.cellsLB [idx].libPart.outcorner.hei_s = length;

						placingZone.cellsRB [idx].objType = OUTCORNER;
						placingZone.cellsRB [idx].height = length;
						placingZone.cellsRB [idx].libPart.outcorner.hei_s = length;
							
						placingZone.cellsT1 [idx].objType = EUROFORM;
						placingZone.cellsT1 [idx].height = length;
						placingZone.cellsT1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsT1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsT2 [idx].objType = EUROFORM;
						placingZone.cellsT2 [idx].height = length;
						placingZone.cellsT2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsT2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsL1 [idx].objType = EUROFORM;
						placingZone.cellsL1 [idx].height = length;
						placingZone.cellsL1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsL1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsL2 [idx].objType = EUROFORM;
						placingZone.cellsL2 [idx].height = length;
						placingZone.cellsL2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsL2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsR1 [idx].objType = EUROFORM;
						placingZone.cellsR1 [idx].height = length;
						placingZone.cellsR1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsR1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsR2 [idx].objType = EUROFORM;
						placingZone.cellsR2 [idx].height = length;
						placingZone.cellsR2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsR2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsB1 [idx].objType = EUROFORM;
						placingZone.cellsB1 [idx].height = length;
						placingZone.cellsB1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsB1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsB2 [idx].objType = EUROFORM;
						placingZone.cellsB2 [idx].height = length;
						placingZone.cellsB2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsB2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsLin1 [idx].objType = INCORNER;
						placingZone.cellsLin1 [idx].height = length;
						placingZone.cellsLin1 [idx].libPart.incorner.hei_s = length;
							
						placingZone.cellsLin2 [idx].objType = INCORNER;
						placingZone.cellsLin2 [idx].height = length;
						placingZone.cellsLin2 [idx].libPart.incorner.hei_s = length;

						placingZone.cellsRin1 [idx].objType = INCORNER;
						placingZone.cellsRin1 [idx].height = length;
						placingZone.cellsRin1 [idx].libPart.incorner.hei_s = length;
							
						placingZone.cellsRin2 [idx].objType = INCORNER;
						placingZone.cellsRin2 [idx].height = length;
						placingZone.cellsRin2 [idx].libPart.incorner.hei_s = length;

						placingZone.cellsW1 [idx].objType = EUROFORM;
						placingZone.cellsW1 [idx].height = length;
						placingZone.cellsW1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsW1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsW2 [idx].objType = EUROFORM;
						placingZone.cellsW2 [idx].height = length;
						placingZone.cellsW2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsW2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsW3 [idx].objType = EUROFORM;
						placingZone.cellsW3 [idx].height = length;
						placingZone.cellsW3 [idx].libPart.form.eu_hei = length;
						placingZone.cellsW3 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsW4 [idx].objType = EUROFORM;
						placingZone.cellsW4 [idx].height = length;
						placingZone.cellsW4 [idx].libPart.form.eu_hei = length;
						placingZone.cellsW4 [idx].libPart.form.eu_hei2 = length;
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
