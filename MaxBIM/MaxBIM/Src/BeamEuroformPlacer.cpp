#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "BeamEuroformPlacer.hpp"

using namespace beamPlacerDG;

static BeamPlacingZone	placingZone;				// 기본 보 영역 정보
static InfoBeam			infoBeam;					// 보 객체 정보
static short			nInterfereBeams;			// 간섭 보 개수
static InfoBeam			infoOtherBeams [10];		// 간섭 보 정보
static short			layerInd_Euroform;			// 레이어 번호: 유로폼
static short			layerInd_Fillerspacer;		// 레이어 번호: 휠러스페이서
static short			layerInd_Plywood;			// 레이어 번호: 합판
static short			layerInd_Wood;				// 레이어 번호: 목재
static short			layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static short			clickedBtnItemIdx;			// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
static bool				clickedOKButton;			// OK 버튼을 눌렀습니까?
static bool				clickedPrevButton;			// 이전 버튼을 눌렀습니까?
static GS::Array<API_Guid>	elemList;				// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함

// 추가/삭제 버튼 인덱스 저장
static short	ADD_CELLS_FROM_BEGIN_AT_SIDE;
static short	DEL_CELLS_FROM_BEGIN_AT_SIDE;
static short	ADD_CELLS_FROM_END_AT_SIDE;
static short	DEL_CELLS_FROM_END_AT_SIDE;
static short	ADD_CELLS_FROM_BEGIN_AT_BOTTOM;
static short	DEL_CELLS_FROM_BEGIN_AT_BOTTOM;
static short	ADD_CELLS_FROM_END_AT_BOTTOM;
static short	DEL_CELLS_FROM_END_AT_BOTTOM;

// 여백 Edit 컨트롤 인덱스 저장
static short	MARGIN_FROM_BEGIN_AT_SIDE;
static short	MARGIN_FROM_END_AT_SIDE;
static short	MARGIN_FROM_BEGIN_AT_BOTTOM;
static short	MARGIN_FROM_END_AT_BOTTOM;

// 여백 채움/비움 라디오 버튼 인덱스 저장
static short	MARGIN_FILL_FROM_BEGIN_AT_SIDE;
static short	MARGIN_EMPTY_FROM_BEGIN_AT_SIDE;
static short	MARGIN_FILL_FROM_END_AT_SIDE;
static short	MARGIN_EMPTY_FROM_END_AT_SIDE;
static short	MARGIN_FILL_FROM_BEGIN_AT_BOTTOM;
static short	MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM;
static short	MARGIN_FILL_FROM_END_AT_BOTTOM;
static short	MARGIN_EMPTY_FROM_END_AT_BOTTOM;

// 배치 버튼 인덱스 저장
static short	START_INDEX_FROM_BEGIN_AT_SIDE;
static short	START_INDEX_CENTER_AT_SIDE;
static short	END_INDEX_FROM_END_AT_SIDE;
static short	START_INDEX_FROM_BEGIN_AT_BOTTOM;
static short	START_INDEX_CENTER_AT_BOTTOM;
static short	END_INDEX_FROM_END_AT_BOTTOM;

// 간섭보 들어오는 측면 길이를 표현하는 Edit 컨트롤 인덱스
static short	EDITCONTROL_CENTER_LENGTH_SIDE;


// 3번 메뉴: 보에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnBeamEntire (void)
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
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nMorphs = 0;
	long					nBeams = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;
	API_BeamRelation		relData;

	// 모프 3D 구성요소 가져오기
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();

	// 폴리곤 점을 배열로 복사함
	API_Coord3D		nodes_random [20];
	long			nNodes;		// 모프 폴리곤의 정점 좌표 개수
	bool			bIsInPolygon1, bIsInPolygon2;

	// 모프 객체 정보
	InfoMorphForBeam		infoMorph;

	// 점 입력
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	API_Coord3D				other_p1, other_p2;

	// 교차점 찾기
	API_Coord p1, p2, p3, p4, pResult;

	// 작업 층 정보
	API_StoryInfo			storyInfo;
	double					workLevel_beam;


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 보 (1개), 보 측면 전체를 덮는 모프 (1개)", true);
		return err;
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 메인 보 1개 선택해야 함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_MorphID)		// 모프인가?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// 보인가?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nMorphs = morphs.GetSize ();
	nBeams = beams.GetSize ();

	// 보가 1개인가?
	if (nBeams != 1) {
		ACAPI_WriteReport ("보를 1개 선택해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("보 측면 전체를 덮는 모프를 1개 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 보 정보 저장
	infoBeam.guid = beams.Pop ();

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = infoBeam.guid;
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
	
	infoBeam.floorInd	= elem.header.floorInd;
	infoBeam.height		= elem.beam.height;
	infoBeam.width		= elem.beam.width;
	infoBeam.offset		= elem.beam.offset;
	infoBeam.level		= elem.beam.level;
	infoBeam.begC		= elem.beam.begC;
	infoBeam.endC		= elem.beam.endC;

	ACAPI_DisposeElemMemoHdls (&memo);

	// 메인 보와 있을 수 있는 간섭 보들의 정보를 가져옴
	ACAPI_Element_GetRelations (infoBeam.guid, API_BeamID, (void*) &relData);
	nInterfereBeams = 0;

	// 메인 보의 중간을 관통하는 간섭 보
	if (relData.conX != NULL) {
		for (xx = 0 ; xx < relData.nConX ; xx++) {
			BNZeroMemory (&elem, sizeof (API_Element));
			elem.header.guid = (*(relData.conX))[xx].guid;
			ACAPI_Element_Get (&elem);
			
			infoOtherBeams [nInterfereBeams].guid		= (*(relData.conX))[xx].guid;
			infoOtherBeams [nInterfereBeams].floorInd	= elem.header.floorInd;
			infoOtherBeams [nInterfereBeams].height		= elem.beam.height;
			infoOtherBeams [nInterfereBeams].width		= elem.beam.width;
			infoOtherBeams [nInterfereBeams].offset		= elem.beam.offset;
			infoOtherBeams [nInterfereBeams].level		= elem.beam.level;
			infoOtherBeams [nInterfereBeams].begC		= elem.beam.begC;
			infoOtherBeams [nInterfereBeams].endC		= elem.beam.endC;

			++nInterfereBeams;
		}
	}

    ACAPI_DisposeBeamRelationHdls (&relData);

	// 모프 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// 모프의 정보 저장
	infoMorph.guid		= elem.header.guid;
	infoMorph.floorInd	= elem.header.floorInd;
	infoMorph.level		= info3D.bounds.zMin;

	// 모프의 3D 바디를 가져옴
	BNZeroMemory (&component, sizeof (API_Component3D));
	component.header.typeID = API_BodyID;
	component.header.index = info3D.fbody;
	err = ACAPI_3D_GetComponent (&component);

	// 모프의 3D 모델을 가져오지 못하면 종료
	if (err != NoError) {
		ACAPI_WriteReport ("모프의 3D 모델을 가져오지 못했습니다.", true);
		return err;
	}

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;
	
	// 정점 좌표를 임의 순서대로 저장함
	for (xx = 1 ; xx <= nVert ; ++xx) {
		component.header.typeID	= API_VertID;
		component.header.index	= xx;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
			coords.Push (trCoord);
		}
	}
	nNodes = coords.GetSize ();

	// 시작 부분 하단 점 클릭, 끝 부분 상단 점 클릭
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("보의 시작 부분 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("보의 끝 부분 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;

	// 사용자가 클릭한 두 점을 통해 보의 시작점, 끝점을 찾음
	other_p1.x = infoBeam.begC.x;
	other_p1.y = infoBeam.begC.y;
	other_p1.z = info3D.bounds.zMin;

	other_p2.x = infoBeam.endC.x;
	other_p2.y = infoBeam.endC.y;
	other_p2.z = info3D.bounds.zMin;

	// 영역 높이 값을 구함
	placingZone.areaHeight = info3D.bounds.zMax - info3D.bounds.zMin;

	// 클릭한 두 점을 보의 시작점, 끝점과 연결시킴
	if (moreCloserPoint (point1, other_p1, other_p2) == 1) {
		placingZone.begC = other_p1;
		placingZone.endC = other_p2;
	} else {
		placingZone.begC = other_p2;
		placingZone.endC = other_p1;
	}

	// 폴리곤의 점들을 저장함
	for (xx = 0 ; xx < nNodes ; ++xx)
		nodes_random [xx] = coords.Pop ();

	// 만약 선택한 두 점이 폴리곤에 속한 점이 아니면 오류
	bIsInPolygon1 = false;
	bIsInPolygon2 = false;
	for (xx = 0 ; xx < nNodes ; ++xx) {
		if (isSamePoint (point1, nodes_random [xx]))
			bIsInPolygon1 = true;
		if (isSamePoint (point2, nodes_random [xx]))
			bIsInPolygon2 = true;
	}

	if ( !(bIsInPolygon1 && bIsInPolygon2) ) {
		ACAPI_WriteReport ("폴리곤에 속하지 않은 점을 클릭했습니다.", true);
		return err;
	}

	// 영역 모프 제거
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// 두 점 간의 각도를 구함
	dx = placingZone.endC.x - placingZone.begC.x;
	dy = placingZone.endC.y - placingZone.begC.y;
	ang = RadToDegree (atan2 (dy, dx));

	// 나머지 보 영역 정보를 저장함
	placingZone.ang			= DegreeToRad (ang);
	placingZone.level		= infoBeam.level;
	placingZone.beamLength	= GetDistance (infoBeam.begC.x, infoBeam.begC.y, infoBeam.endC.x, infoBeam.endC.y);

	// 간섭 보들의 시작점/끝점 중 메인 보에 가까운 쪽 점을 검색 (간섭 보는 양쪽에 한 쌍만 있다고 가정함)
	p1.x = infoBeam.begC.x;				p1.y = infoBeam.begC.y;
	p2.x = infoBeam.endC.x;				p2.y = infoBeam.endC.y;
	p3.x = infoOtherBeams [0].begC.x;	p3.y = infoOtherBeams [0].begC.y;
	p4.x = infoOtherBeams [0].endC.x;	p4.y = infoOtherBeams [0].endC.y;
	pResult = IntersectionPoint1 (&p1, &p2, &p3, &p4);	// 메인 보와 간섭 보의 교차점 검색

	// 간섭 보 관련 정보 입력
	if (nInterfereBeams > 0) {
		placingZone.bInterfereBeam = true;
		placingZone.posInterfereBeamFromLeft = GetDistance (placingZone.begC.x, placingZone.begC.y, pResult.x, pResult.y) - infoOtherBeams [0].offset;
		placingZone.interfereBeamWidth = infoOtherBeams [0].width;
		placingZone.interfereBeamHeight = infoOtherBeams [0].height;
	} else {
		placingZone.bInterfereBeam = false;
	}

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_beam = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoBeam.floorInd) {
			workLevel_beam = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 영역 정보의 고도 정보 수정 - 불필요

FIRST:

	// placingZone의 Cell 정보 초기화
	initCellsForBeam (&placingZone);

	// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32521, ACAPI_GetOwnResModule (), beamPlacerHandler1, 0);

	if (result == DG_CANCEL)
		return err;

	// 1차 배치 설정
	firstPlacingSettingsForBeam (&placingZone);

	// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정합니다.
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (500, 530, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, beamPlacerHandler2, 0);
	
	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST;

	// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
	if (clickedOKButton != true)
		return err;

	// 1, 2번째 다이얼로그를 통해 입력된 데이터를 기반으로 객체를 배치
	// 측면 객체 배치
	for (xx = 0 ; xx < 4 ; ++xx) {
		for (yy = 0 ; yy < placingZone.nCellsFromBeginAtSide ; ++yy) {
			placingZone.cellsFromBeginAtLSide [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromBeginAtLSide [xx][yy]);
			elemList.Push (placingZone.cellsFromBeginAtLSide [xx][yy].guid);
			placingZone.cellsFromBeginAtRSide [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromBeginAtRSide [xx][yy]);
			elemList.Push (placingZone.cellsFromBeginAtRSide [xx][yy].guid);
		}
	}
	for (xx = 0 ; xx < 4 ; ++xx) {
		placingZone.cellCenterAtLSide [xx].guid = placeLibPartForBeam (placingZone.cellCenterAtLSide [xx]);
		elemList.Push (placingZone.cellCenterAtLSide [xx].guid);
		placingZone.cellCenterAtRSide [xx].guid = placeLibPartForBeam (placingZone.cellCenterAtRSide [xx]);
		elemList.Push (placingZone.cellCenterAtRSide [xx].guid);
	}
	for (xx = 0 ; xx < 4 ; ++xx) {
		for (yy = 0 ; yy < placingZone.nCellsFromEndAtSide ; ++yy) {
			placingZone.cellsFromEndAtLSide [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromEndAtLSide [xx][yy]);
			elemList.Push (placingZone.cellsFromEndAtLSide [xx][yy].guid);
			placingZone.cellsFromEndAtRSide [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromEndAtRSide [xx][yy]);
			elemList.Push (placingZone.cellsFromEndAtRSide [xx][yy].guid);
		}
	}

	// 하부 객체 배치
	for (xx = 0 ; xx < 3 ; ++xx) {
		for (yy = 0 ; yy < placingZone.nCellsFromBeginAtBottom ; ++yy) {
			placingZone.cellsFromBeginAtBottom [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromBeginAtBottom [xx][yy]);
			elemList.Push (placingZone.cellsFromBeginAtBottom [xx][yy].guid);
		}
	}
	for (xx = 0 ; xx < 3 ; ++xx) {
		placingZone.cellCenterAtBottom [xx].guid = placeLibPartForBeam (placingZone.cellCenterAtBottom [xx]);
		elemList.Push (placingZone.cellCenterAtBottom [xx].guid);
	}
	for (xx = 0 ; xx < 3 ; ++xx) {
		for (yy = 0 ; yy < placingZone.nCellsFromEndAtBottom ; ++yy) {
			placingZone.cellsFromEndAtBottom [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromEndAtBottom [xx][yy]);
			elemList.Push (placingZone.cellsFromEndAtBottom [xx][yy].guid);
		}
	}

	// 나머지 영역 채우기 - 합판, 목재
	err = fillRestAreasForBeam (&placingZone);

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

	return	err;
}

// 4번 메뉴: 보에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnBeamPart (void)
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
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nMorphs = 0;
	long					nBeams = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 3D 구성요소 가져오기
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();

	// 폴리곤 점을 배열로 복사하고 순서대로 좌표 값을 얻어냄
	API_Coord3D		nodes_random [20];
	long			nNodes;		// 모프 폴리곤의 정점 좌표 개수
	bool			bIsInPolygon1, bIsInPolygon2;

	// 모프 객체 정보
	InfoMorphForBeam		infoMorph;

	// 점 입력
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	API_Coord3D				other_p1, other_p2;

	// 보와 모프와의 관계를 찾기 위한 변수
	API_Coord				clickedPoint;
	API_Coord				beginPoint;
	API_Coord				endPoint;
	double					distance1, distance2, distance3;

	// 작업 층 정보
	API_StoryInfo			storyInfo;
	double					workLevel_beam;


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 보 (1개), 보 측면 일부를 덮는 모프 (1개)", true);
		return err;
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 메인 보 1개 선택해야 함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_MorphID)		// 모프인가?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// 보인가?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nMorphs = morphs.GetSize ();
	nBeams = beams.GetSize ();

	// 보가 1개인가?
	if (nBeams != 1) {
		ACAPI_WriteReport ("보를 1개 선택해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("보 측면 일부를 덮는 모프를 1개 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 보 정보 저장
	infoBeam.guid = beams.Pop ();

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = infoBeam.guid;
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
	
	infoBeam.floorInd	= elem.header.floorInd;
	infoBeam.height		= elem.beam.height;
	infoBeam.width		= elem.beam.width;
	infoBeam.offset		= elem.beam.offset;
	infoBeam.level		= elem.beam.level;
	infoBeam.begC		= elem.beam.begC;
	infoBeam.endC		= elem.beam.endC;

	ACAPI_DisposeElemMemoHdls (&memo);

	// 모프 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// 모프의 정보 저장
	infoMorph.guid		= elem.header.guid;
	infoMorph.floorInd	= elem.header.floorInd;
	infoMorph.level		= info3D.bounds.zMin;

	// 모프의 3D 바디를 가져옴
	BNZeroMemory (&component, sizeof (API_Component3D));
	component.header.typeID = API_BodyID;
	component.header.index = info3D.fbody;
	err = ACAPI_3D_GetComponent (&component);

	// 모프의 3D 모델을 가져오지 못하면 종료
	if (err != NoError) {
		ACAPI_WriteReport ("모프의 3D 모델을 가져오지 못했습니다.", true);
		return err;
	}

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;
	
	// 정점 좌표를 임의 순서대로 저장함
	for (xx = 1 ; xx <= nVert ; ++xx) {
		component.header.typeID	= API_VertID;
		component.header.index	= xx;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
			coords.Push (trCoord);
		}
	}
	nNodes = coords.GetSize ();

	// 시작 부분 하단 점 클릭, 끝 부분 상단 점 클릭
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("보의 시작 부분 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("보의 끝 부분 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;

	// 사용자가 클릭한 두 점을 통해 보의 시작점, 끝점을 찾음
	other_p1.x = infoBeam.begC.x;
	other_p1.y = infoBeam.begC.y;
	other_p1.z = info3D.bounds.zMin;

	other_p2.x = infoBeam.endC.x;
	other_p2.y = infoBeam.endC.y;
	other_p2.z = info3D.bounds.zMin;

	// 영역 높이 값을 구함
	placingZone.areaHeight = info3D.bounds.zMax - info3D.bounds.zMin;

	// 클릭한 두 점을 보의 시작점, 끝점과 연결시킴
	if (GetDistance (point1, other_p1) < GetDistance (point2, other_p1)) {
		placingZone.begC = other_p1;
		placingZone.endC = other_p2;
	} else {
		placingZone.begC = other_p2;
		placingZone.endC = other_p1;
	}

	// 폴리곤의 점들을 저장함
	for (xx = 0 ; xx < nNodes ; ++xx)
		nodes_random [xx] = coords.Pop ();

	// 만약 선택한 두 점이 폴리곤에 속한 점이 아니면 오류
	bIsInPolygon1 = false;
	bIsInPolygon2 = false;
	for (xx = 0 ; xx < nNodes ; ++xx) {
		if (isSamePoint (point1, nodes_random [xx]))
			bIsInPolygon1 = true;
		if (isSamePoint (point2, nodes_random [xx]))
			bIsInPolygon2 = true;
	}

	if ( !(bIsInPolygon1 && bIsInPolygon2) ) {
		ACAPI_WriteReport ("폴리곤에 속하지 않은 점을 클릭했습니다.", true);
		return err;
	}

	// 영역 모프 제거
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// 두 점 간의 각도를 구함
	dx = placingZone.endC.x - placingZone.begC.x;
	dy = placingZone.endC.y - placingZone.begC.y;
	ang = RadToDegree (atan2 (dy, dx));

	// 나머지 보 영역 정보를 저장함
	placingZone.ang			= DegreeToRad (ang);
	placingZone.level		= infoBeam.level;
	placingZone.beamLength	= GetDistance (point1.x, point1.y, point2.x, point2.y);

	// 보 레퍼런스 라인과 모프 시작점 간의 거리를 측정한다
	if (GetDistance (point1, other_p1) < GetDistance (point2, other_p1)) {
		clickedPoint.x = point1.x;
		clickedPoint.y = point1.y;
		beginPoint.x = placingZone.begC.x;
		beginPoint.y = placingZone.begC.y;
		endPoint.x = placingZone.endC.x;
		endPoint.y = placingZone.endC.y;

		distance1 = distOfPointBetweenLine (clickedPoint, beginPoint, endPoint);
		distance2 = GetDistance (beginPoint.x, beginPoint.y, clickedPoint.x, clickedPoint.y);
		if (abs (distance2 - distance1) > EPS)
			distance3 = sqrt (distance2 * distance2 - distance1 * distance1);	// 보 시작점으로부터 모프 시작점까지의 X 거리
		else
			distance3 = 0.0;

		// 보의 시작/끝점을 모프가 덮은 영역인 일부 구간으로 설정
		placingZone.begC.x = beginPoint.x + (distance3 * cos(placingZone.ang));
		placingZone.begC.y = beginPoint.y + (distance3 * sin(placingZone.ang));
		placingZone.endC.x = beginPoint.x + (GetDistance (point1.x, point1.y, point2.x, point2.y) * cos(placingZone.ang));
		placingZone.endC.y = beginPoint.y + (GetDistance (point1.x, point1.y, point2.x, point2.y) * sin(placingZone.ang));
	} else {
		clickedPoint.x = point1.x;
		clickedPoint.y = point1.y;
		beginPoint.x = placingZone.endC.x;
		beginPoint.y = placingZone.endC.y;
		endPoint.x = placingZone.begC.x;
		endPoint.y = placingZone.begC.y;

		distance1 = distOfPointBetweenLine (clickedPoint, endPoint, beginPoint);
		distance2 = GetDistance (endPoint.x, endPoint.y, clickedPoint.x, clickedPoint.y);
		if (abs (distance2 - distance1) > EPS)
			distance3 = sqrt (distance2 * distance2 - distance1 * distance1);	// 보 시작점으로부터 모프 시작점까지의 X 거리
		else
			distance3 = 0.0;

		// 보의 시작/끝점을 모프가 덮은 영역인 일부 구간으로 설정
		placingZone.begC.x = endPoint.x + (distance3 * cos(placingZone.ang));
		placingZone.begC.y = endPoint.y + (distance3 * sin(placingZone.ang));
		placingZone.endC.x = endPoint.x + (GetDistance (point1.x, point1.y, point2.x, point2.y) * cos(placingZone.ang));
		placingZone.endC.y = endPoint.y + (GetDistance (point1.x, point1.y, point2.x, point2.y) * sin(placingZone.ang));

		infoBeam.offset = -infoBeam.offset;
	}

	// 보 정보 업데이트
	infoBeam.begC.x		= placingZone.begC.x;
	infoBeam.begC.y		= placingZone.begC.y;
	infoBeam.endC.x		= placingZone.endC.x;
	infoBeam.endC.y		= placingZone.endC.y;

	// 간섭 보 관련 정보 입력 - 간섭 보 없음
	placingZone.bInterfereBeam = false;

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_beam = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoBeam.floorInd) {
			workLevel_beam = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 영역 정보의 고도 정보 수정 - 불필요


FIRST:

	// placingZone의 Cell 정보 초기화
	initCellsForBeam (&placingZone);

	// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32521, ACAPI_GetOwnResModule (), beamPlacerHandler1, 0);

	if (result == DG_CANCEL)
		return err;

	// 1차 배치 설정
	firstPlacingSettingsForBeam (&placingZone);

	// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정합니다.
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (500, 530, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, beamPlacerHandler2, 0);
	
	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST;

	// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
	if (clickedOKButton != true)
		return err;

	// 1, 2번째 다이얼로그를 통해 입력된 데이터를 기반으로 객체를 배치
	// 측면 객체 배치
	for (xx = 0 ; xx < 4 ; ++xx) {
		for (yy = 0 ; yy < placingZone.nCellsFromBeginAtSide ; ++yy) {
			placingZone.cellsFromBeginAtLSide [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromBeginAtLSide [xx][yy]);
			elemList.Push (placingZone.cellsFromBeginAtLSide [xx][yy].guid);
			placingZone.cellsFromBeginAtRSide [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromBeginAtRSide [xx][yy]);
			elemList.Push (placingZone.cellsFromBeginAtRSide [xx][yy].guid);
		}
	}
	for (xx = 0 ; xx < 4 ; ++xx) {
		placingZone.cellCenterAtLSide [xx].guid = placeLibPartForBeam (placingZone.cellCenterAtLSide [xx]);
		elemList.Push (placingZone.cellCenterAtLSide [xx].guid);
		placingZone.cellCenterAtRSide [xx].guid = placeLibPartForBeam (placingZone.cellCenterAtRSide [xx]);
		elemList.Push (placingZone.cellCenterAtRSide [xx].guid);
	}
	for (xx = 0 ; xx < 4 ; ++xx) {
		for (yy = 0 ; yy < placingZone.nCellsFromEndAtSide ; ++yy) {
			placingZone.cellsFromEndAtLSide [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromEndAtLSide [xx][yy]);
			elemList.Push (placingZone.cellsFromEndAtLSide [xx][yy].guid);
			placingZone.cellsFromEndAtRSide [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromEndAtRSide [xx][yy]);
			elemList.Push (placingZone.cellsFromEndAtRSide [xx][yy].guid);
		}
	}

	// 하부 객체 배치
	for (xx = 0 ; xx < 3 ; ++xx) {
		for (yy = 0 ; yy < placingZone.nCellsFromBeginAtBottom ; ++yy) {
			placingZone.cellsFromBeginAtBottom [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromBeginAtBottom [xx][yy]);
			elemList.Push (placingZone.cellsFromBeginAtBottom [xx][yy].guid);
		}
	}
	for (xx = 0 ; xx < 3 ; ++xx) {
		placingZone.cellCenterAtBottom [xx].guid = placeLibPartForBeam (placingZone.cellCenterAtBottom [xx]);
		elemList.Push (placingZone.cellCenterAtBottom [xx].guid);
	}
	for (xx = 0 ; xx < 3 ; ++xx) {
		for (yy = 0 ; yy < placingZone.nCellsFromEndAtBottom ; ++yy) {
			placingZone.cellsFromEndAtBottom [xx][yy].guid = placeLibPartForBeam (placingZone.cellsFromEndAtBottom [xx][yy]);
			elemList.Push (placingZone.cellsFromEndAtBottom [xx][yy].guid);
		}
	}

	// 나머지 영역 채우기 - 합판, 목재
	err = fillRestAreasForBeam (&placingZone);

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

	return	err;
}

// Cell 배열을 초기화함
void	initCellsForBeam (BeamPlacingZone* placingZone)
{
	short xx, yy;

	// 영역 정보의 여백 채움 여부 초기화
	placingZone->bFillMarginBeginAtSide = false;
	placingZone->bFillMarginEndAtSide = false;
	placingZone->bFillMarginBeginAtBottom = false;
	placingZone->bFillMarginEndAtBottom = false;

	// 영역 정보의 여백 설정 초기화
	placingZone->marginBeginAtSide = 0.0;
	placingZone->marginEndAtSide = 0.0;
	placingZone->marginBeginAtBottom = 0.0;
	placingZone->marginEndAtBottom = 0.0;

	// 셀 개수 초기화
	placingZone->nCellsFromBeginAtSide = 0;
	placingZone->nCellsFromEndAtSide = 0;
	placingZone->nCellsFromBeginAtBottom = 0;
	placingZone->nCellsFromEndAtBottom = 0;

	// 간섭 보가 들어오는 영역 길이 초기화
	placingZone->centerLengthAtSide = 0.0;

	// 셀 정보 초기화
	for (xx = 0 ; xx < 4 ; ++xx) {
		for (yy = 0 ; yy < 20 ; ++yy) {
			placingZone->cellsFromBeginAtLSide [xx][yy].objType = NONE;
			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsFromBeginAtLSide [xx][yy].ang = 0.0;
			placingZone->cellsFromBeginAtLSide [xx][yy].dirLen = 0.0;
			placingZone->cellsFromBeginAtLSide [xx][yy].perLen = 0.0;
			placingZone->cellsFromBeginAtLSide [xx][yy].attached_side = LEFT_SIDE;

			placingZone->cellsFromBeginAtRSide [xx][yy].objType = NONE;
			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsFromBeginAtRSide [xx][yy].ang = 0.0;
			placingZone->cellsFromBeginAtRSide [xx][yy].dirLen = 0.0;
			placingZone->cellsFromBeginAtRSide [xx][yy].perLen = 0.0;
			placingZone->cellsFromBeginAtRSide [xx][yy].attached_side = RIGHT_SIDE;

			placingZone->cellsFromEndAtLSide [xx][yy].objType = NONE;
			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsFromEndAtLSide [xx][yy].ang = 0.0;
			placingZone->cellsFromEndAtLSide [xx][yy].dirLen = 0.0;
			placingZone->cellsFromEndAtLSide [xx][yy].perLen = 0.0;
			placingZone->cellsFromEndAtLSide [xx][yy].attached_side = LEFT_SIDE;

			placingZone->cellsFromEndAtRSide [xx][yy].objType = NONE;
			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsFromEndAtRSide [xx][yy].ang = 0.0;
			placingZone->cellsFromEndAtRSide [xx][yy].dirLen = 0.0;
			placingZone->cellsFromEndAtRSide [xx][yy].perLen = 0.0;
			placingZone->cellsFromEndAtRSide [xx][yy].attached_side = RIGHT_SIDE;
		}

		placingZone->cellCenterAtLSide [xx].objType = NONE;
		placingZone->cellCenterAtLSide [xx].leftBottomX = 0.0;
		placingZone->cellCenterAtLSide [xx].leftBottomY = 0.0;
		placingZone->cellCenterAtLSide [xx].leftBottomZ = 0.0;
		placingZone->cellCenterAtLSide [xx].ang = 0.0;
		placingZone->cellCenterAtLSide [xx].dirLen = 0.0;
		placingZone->cellCenterAtLSide [xx].perLen = 0.0;
		placingZone->cellCenterAtLSide [xx].attached_side = LEFT_SIDE;

		placingZone->cellCenterAtRSide [xx].objType = NONE;
		placingZone->cellCenterAtRSide [xx].leftBottomX = 0.0;
		placingZone->cellCenterAtRSide [xx].leftBottomY = 0.0;
		placingZone->cellCenterAtRSide [xx].leftBottomZ = 0.0;
		placingZone->cellCenterAtRSide [xx].ang = 0.0;
		placingZone->cellCenterAtRSide [xx].dirLen = 0.0;
		placingZone->cellCenterAtRSide [xx].perLen = 0.0;
		placingZone->cellCenterAtRSide [xx].attached_side = RIGHT_SIDE;
	}

	for (xx = 0 ; xx < 3 ; ++xx) {
		for (yy = 0 ; yy < 20 ; ++yy) {
			placingZone->cellsFromBeginAtBottom [xx][yy].objType = NONE;
			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX = 0.0;
			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY = 0.0;
			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsFromBeginAtBottom [xx][yy].ang = 0.0;
			placingZone->cellsFromBeginAtBottom [xx][yy].dirLen = 0.0;
			placingZone->cellsFromBeginAtBottom [xx][yy].perLen = 0.0;
			placingZone->cellsFromBeginAtBottom [xx][yy].attached_side = BOTTOM_SIDE;

			placingZone->cellsFromEndAtBottom [xx][yy].objType = NONE;
			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX = 0.0;
			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY = 0.0;
			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsFromEndAtBottom [xx][yy].ang = 0.0;
			placingZone->cellsFromEndAtBottom [xx][yy].dirLen = 0.0;
			placingZone->cellsFromEndAtBottom [xx][yy].perLen = 0.0;
			placingZone->cellsFromEndAtBottom [xx][yy].attached_side = BOTTOM_SIDE;
		}

		placingZone->cellCenterAtBottom [xx].objType = NONE;
		placingZone->cellCenterAtBottom [xx].leftBottomX = 0.0;
		placingZone->cellCenterAtBottom [xx].leftBottomY = 0.0;
		placingZone->cellCenterAtBottom [xx].leftBottomZ = 0.0;
		placingZone->cellCenterAtBottom [xx].ang = 0.0;
		placingZone->cellCenterAtBottom [xx].dirLen = 0.0;
		placingZone->cellCenterAtBottom [xx].perLen = 0.0;
		placingZone->cellCenterAtBottom [xx].attached_side = BOTTOM_SIDE;
	}
}

// 1차 배치 설정
void	firstPlacingSettingsForBeam (BeamPlacingZone* placingZone)
{
	short			xx, yy;
	API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	double			centerPos;		// 중심 위치
	double			width;			// 간섭 보가 차지하는 너비
	double			remainLength;	// 남은 길이를 계산하기 위한 임시 변수
	double			xPos;			// 위치 커서
	double			accumDist;		// 이동 거리
	
	const double	formLength1 = 1.200;
	const double	formLength2 = 0.900;
	const double	formLength3 = 0.600;
	double			formLength;

	double			height [4];
	double			left [3];

	
	// 측면에서의 중심 위치 찾기
	if (placingZone->bInterfereBeam == true) {
		centerPos = placingZone->posInterfereBeamFromLeft;	// 간섭 보의 중심 위치
		width = placingZone->interfereBeamWidth;
	} else {
		centerPos = placingZone->beamLength / 2;			// 간섭 보가 없으면 중심을 기준으로 함
		width = 0.0;
	}

	// (1-1) 측면 시작 부분
	remainLength = centerPos - width/2;
	while (remainLength >= 0.600) {
		if (remainLength > formLength1) {
			formLength = formLength1;
			remainLength -= formLength1;
		} else if (remainLength > formLength2) {
			formLength = formLength2;
			remainLength -= formLength2;
		} else {
			formLength = formLength3;
			remainLength -= formLength3;
		}

		// 측면 [0]
		//placingZone->cellsFromBeginAtLSide [0][placingZone->nCellsFromBeginAtSide].objType = EUROFORM;
		placingZone->cellsFromBeginAtLSide [0][placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtLSide [0][placingZone->nCellsFromBeginAtSide].libPart.form.eu_hei = formLength;

		//placingZone->cellsFromBeginAtRSide [0][placingZone->nCellsFromBeginAtSide].objType = EUROFORM;
		placingZone->cellsFromBeginAtRSide [0][placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtRSide [0][placingZone->nCellsFromBeginAtSide].libPart.form.eu_hei = formLength;

		// 측면 [1]
		//placingZone->cellsFromBeginAtLSide [1][placingZone->nCellsFromBeginAtSide].objType = FILLERSPACER;
		placingZone->cellsFromBeginAtLSide [1][placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtLSide [1][placingZone->nCellsFromBeginAtSide].libPart.fillersp.f_leng = formLength;

		//placingZone->cellsFromBeginAtRSide [1][placingZone->nCellsFromBeginAtSide].objType = FILLERSPACER;
		placingZone->cellsFromBeginAtRSide [1][placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtRSide [1][placingZone->nCellsFromBeginAtSide].libPart.fillersp.f_leng = formLength;

		// 측면 [2]
		//placingZone->cellsFromBeginAtLSide [2][placingZone->nCellsFromBeginAtSide].objType = EUROFORM;
		placingZone->cellsFromBeginAtLSide [2][placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtLSide [2][placingZone->nCellsFromBeginAtSide].libPart.form.eu_hei = formLength;

		//placingZone->cellsFromBeginAtRSide [2][placingZone->nCellsFromBeginAtSide].objType = EUROFORM;
		placingZone->cellsFromBeginAtRSide [2][placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtRSide [2][placingZone->nCellsFromBeginAtSide].libPart.form.eu_hei = formLength;

		// 측면 [3]
		//placingZone->cellsFromBeginAtLSide [3][placingZone->nCellsFromBeginAtSide].objType = WOOD;
		placingZone->cellsFromBeginAtLSide [3][placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtLSide [3][placingZone->nCellsFromBeginAtSide].libPart.wood.w_leng = formLength;

		//placingZone->cellsFromBeginAtRSide [3][placingZone->nCellsFromBeginAtSide].objType = WOOD;
		placingZone->cellsFromBeginAtRSide [3][placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtRSide [3][placingZone->nCellsFromBeginAtSide].libPart.wood.w_leng = formLength;

		placingZone->nCellsFromBeginAtSide ++;
	}

	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
		if (placingZone->cellsFromBeginAtLSide [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	// 위치 선정
	height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	height [1] = height [0] + placingZone->cellsFromBeginAtRSide [0][0].perLen;
	height [2] = height [1] + placingZone->cellsFromBeginAtRSide [1][0].perLen;
	height [3] = height [2] + placingZone->cellsFromBeginAtRSide [2][0].perLen;
	for (xx = 0 ; xx < 4 ; ++xx) {
		xPos = centerPos - width/2 - accumDist;
		for (yy = 0 ; yy < placingZone->nCellsFromBeginAtSide ; ++yy) {
			// 좌측
			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ = height [xx];
		
			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX;
			rotatedPoint.y = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY;
			rotatedPoint.z = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX = unrotatedPoint.x;
			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY = unrotatedPoint.y;
			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ = unrotatedPoint.z;

			// 우측
			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY = placingZone->begC.y - infoBeam.width/2 - placingZone->gapSide;
			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ = height [xx];

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX;
			rotatedPoint.y = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY;
			rotatedPoint.z = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX = unrotatedPoint.x;
			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY = unrotatedPoint.y;
			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ = unrotatedPoint.z;

			// 거리 이동
			xPos += placingZone->cellsFromBeginAtRSide [xx][yy].dirLen;
		}
	}

	// (1-2) 측면 끝 부분
	remainLength = placingZone->beamLength - centerPos - width/2;
	while (remainLength >= 0.600) {
		if (remainLength > formLength1) {
			formLength = formLength1;
			remainLength -= formLength1;
		} else if (remainLength > formLength2) {
			formLength = formLength2;
			remainLength -= formLength2;
		} else {
			formLength = formLength3;
			remainLength -= formLength3;
		}

		// 측면 [0]
		//placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide].objType = EUROFORM;
		placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide].dirLen = formLength;
		placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide].libPart.form.eu_hei = formLength;

		//placingZone->cellsFromEndAtRSide [0][placingZone->nCellsFromEndAtSide].objType = EUROFORM;
		placingZone->cellsFromEndAtRSide [0][placingZone->nCellsFromEndAtSide].dirLen = formLength;
		placingZone->cellsFromEndAtRSide [0][placingZone->nCellsFromEndAtSide].libPart.form.eu_hei = formLength;

		// 측면 [1]
		//placingZone->cellsFromEndAtLSide [1][placingZone->nCellsFromEndAtSide].objType = FILLERSPACER;
		placingZone->cellsFromEndAtLSide [1][placingZone->nCellsFromEndAtSide].dirLen = formLength;
		placingZone->cellsFromEndAtLSide [1][placingZone->nCellsFromEndAtSide].libPart.fillersp.f_leng = formLength;

		//placingZone->cellsFromEndAtRSide [1][placingZone->nCellsFromEndAtSide].objType = FILLERSPACER;
		placingZone->cellsFromEndAtRSide [1][placingZone->nCellsFromEndAtSide].dirLen = formLength;
		placingZone->cellsFromEndAtRSide [1][placingZone->nCellsFromEndAtSide].libPart.fillersp.f_leng = formLength;

		// 측면 [2]
		//placingZone->cellsFromEndAtLSide [2][placingZone->nCellsFromEndAtSide].objType = EUROFORM;
		placingZone->cellsFromEndAtLSide [2][placingZone->nCellsFromEndAtSide].dirLen = formLength;
		placingZone->cellsFromEndAtLSide [2][placingZone->nCellsFromEndAtSide].libPart.form.eu_hei = formLength;

		//placingZone->cellsFromEndAtRSide [2][placingZone->nCellsFromEndAtSide].objType = EUROFORM;
		placingZone->cellsFromEndAtRSide [2][placingZone->nCellsFromEndAtSide].dirLen = formLength;
		placingZone->cellsFromEndAtRSide [2][placingZone->nCellsFromEndAtSide].libPart.form.eu_hei = formLength;

		// 측면 [3]
		//placingZone->cellsFromEndAtLSide [3][placingZone->nCellsFromEndAtSide].objType = WOOD;
		placingZone->cellsFromEndAtLSide [3][placingZone->nCellsFromEndAtSide].dirLen = formLength;
		placingZone->cellsFromEndAtLSide [3][placingZone->nCellsFromEndAtSide].libPart.wood.w_leng = formLength;

		//placingZone->cellsFromEndAtRSide [3][placingZone->nCellsFromEndAtSide].objType = WOOD;
		placingZone->cellsFromEndAtRSide [3][placingZone->nCellsFromEndAtSide].dirLen = formLength;
		placingZone->cellsFromEndAtRSide [3][placingZone->nCellsFromEndAtSide].libPart.wood.w_leng = formLength;

		placingZone->nCellsFromEndAtSide ++;
	}

	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
		if (placingZone->cellsFromEndAtLSide [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	// 위치 선정
	height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	height [1] = height [0] + placingZone->cellsFromEndAtRSide [0][0].perLen;
	height [2] = height [1] + placingZone->cellsFromEndAtRSide [1][0].perLen;
	height [3] = height [2] + placingZone->cellsFromEndAtRSide [2][0].perLen;
	for (xx = 0 ; xx < 4 ; ++xx) {
		xPos = centerPos + width/2 + accumDist - placingZone->cellsFromEndAtLSide [0][0].dirLen;
		for (yy = 0 ; yy < placingZone->nCellsFromEndAtSide ; ++yy) {
			// 좌측
			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ = height [xx];
			
			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX;
			rotatedPoint.y = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY;
			rotatedPoint.z = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX = unrotatedPoint.x;
			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY = unrotatedPoint.y;
			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ = unrotatedPoint.z;

			// 우측
			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY = placingZone->begC.y - infoBeam.width/2 - placingZone->gapSide;
			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ = height [xx];

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX;
			rotatedPoint.y = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY;
			rotatedPoint.z = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX = unrotatedPoint.x;
			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY = unrotatedPoint.y;
			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ = unrotatedPoint.z;

			// 거리 이동
			if (yy < placingZone->nCellsFromEndAtSide-1)
				xPos -= placingZone->cellsFromEndAtRSide [xx][yy+1].dirLen;
		}
	}

	// (1-3) 측면 중앙
	if (placingZone->bInterfereBeam == true) {
		//placingZone->cellCenterAtLSide [0].objType = EUROFORM;
		placingZone->cellCenterAtLSide [0].dirLen = width;
		placingZone->cellCenterAtLSide [0].libPart.form.eu_hei2 = width;

		//placingZone->cellCenterAtRSide [0].objType = EUROFORM;
		placingZone->cellCenterAtRSide [0].dirLen = width;
		placingZone->cellCenterAtRSide [0].libPart.form.eu_hei2 = width;

		//placingZone->cellCenterAtLSide [1].objType = FILLERSPACER;
		placingZone->cellCenterAtLSide [1].dirLen = width;
		placingZone->cellCenterAtLSide [1].libPart.fillersp.f_leng = width;

		//placingZone->cellCenterAtRSide [1].objType = FILLERSPACER;
		placingZone->cellCenterAtRSide [1].dirLen = width;
		placingZone->cellCenterAtRSide [1].libPart.fillersp.f_leng = width;

		//placingZone->cellCenterAtLSide [2].objType = EUROFORM;
		placingZone->cellCenterAtLSide [2].dirLen = width;
		placingZone->cellCenterAtLSide [2].libPart.form.eu_hei2 = width;

		//placingZone->cellCenterAtRSide [2].objType = EUROFORM;
		placingZone->cellCenterAtRSide [2].dirLen = width;
		placingZone->cellCenterAtRSide [2].libPart.form.eu_hei2 = width;

		//placingZone->cellCenterAtLSide [3].objType = WOOD;
		placingZone->cellCenterAtLSide [3].dirLen = width;
		placingZone->cellCenterAtLSide [3].libPart.wood.w_leng = width;

		//placingZone->cellCenterAtRSide [3].objType = WOOD;
		placingZone->cellCenterAtRSide [3].dirLen = width;
		placingZone->cellCenterAtRSide [3].libPart.wood.w_leng = width;
	}

	// 위치 선정
	xPos = centerPos - width/2;
	height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	height [1] = height [0] + placingZone->cellCenterAtRSide [0].perLen;
	height [2] = height [1] + placingZone->cellCenterAtRSide [1].perLen;
	height [3] = height [2] + placingZone->cellCenterAtRSide [2].perLen;
	for (xx = 0 ; xx < 4 ; ++xx) {
		// 좌측
		placingZone->cellCenterAtLSide [xx].leftBottomX = placingZone->begC.x + xPos;
		placingZone->cellCenterAtLSide [xx].leftBottomY = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
		placingZone->cellCenterAtLSide [xx].leftBottomZ = height [xx];
		
		axisPoint.x = placingZone->begC.x;
		axisPoint.y = placingZone->begC.y;
		axisPoint.z = placingZone->begC.z;

		rotatedPoint.x = placingZone->cellCenterAtLSide [xx].leftBottomX;
		rotatedPoint.y = placingZone->cellCenterAtLSide [xx].leftBottomY;
		rotatedPoint.z = placingZone->cellCenterAtLSide [xx].leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

		placingZone->cellCenterAtLSide [xx].leftBottomX = unrotatedPoint.x;
		placingZone->cellCenterAtLSide [xx].leftBottomY = unrotatedPoint.y;
		placingZone->cellCenterAtLSide [xx].leftBottomZ = unrotatedPoint.z;

		// 우측
		placingZone->cellCenterAtRSide [xx].leftBottomX = placingZone->begC.x + xPos;
		placingZone->cellCenterAtRSide [xx].leftBottomY = placingZone->begC.y - infoBeam.width/2 - placingZone->gapSide;
		placingZone->cellCenterAtRSide [xx].leftBottomZ = height [xx];

		axisPoint.x = placingZone->begC.x;
		axisPoint.y = placingZone->begC.y;
		axisPoint.z = placingZone->begC.z;

		rotatedPoint.x = placingZone->cellCenterAtRSide [xx].leftBottomX;
		rotatedPoint.y = placingZone->cellCenterAtRSide [xx].leftBottomY;
		rotatedPoint.z = placingZone->cellCenterAtRSide [xx].leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

		placingZone->cellCenterAtRSide [xx].leftBottomX = unrotatedPoint.x;
		placingZone->cellCenterAtRSide [xx].leftBottomY = unrotatedPoint.y;
		placingZone->cellCenterAtRSide [xx].leftBottomZ = unrotatedPoint.z;
	}

	// (2-1) 하부 시작 부분
	remainLength = centerPos - width/2;
	while (remainLength >= 0.600) {
		if (remainLength > formLength1) {
			formLength = formLength1;
			remainLength -= formLength1;
		} else if (remainLength > formLength2) {
			formLength = formLength2;
			remainLength -= formLength2;
		} else {
			formLength = formLength3;
			remainLength -= formLength3;
		}

		// 하부 [0]
		//placingZone->cellsFromBeginAtBottom [0][placingZone->nCellsFromBeginAtBottom].objType = EUROFORM;
		placingZone->cellsFromBeginAtBottom [0][placingZone->nCellsFromBeginAtBottom].dirLen = formLength;
		placingZone->cellsFromBeginAtBottom [0][placingZone->nCellsFromBeginAtBottom].libPart.form.eu_hei = formLength;

		// 하부 [1]
		//placingZone->cellsFromBeginAtBottom [1][placingZone->nCellsFromBeginAtBottom].objType = FILLERSPACER;
		placingZone->cellsFromBeginAtBottom [1][placingZone->nCellsFromBeginAtBottom].dirLen = formLength;
		placingZone->cellsFromBeginAtBottom [1][placingZone->nCellsFromBeginAtBottom].libPart.fillersp.f_leng = formLength;

		// 하부 [2]
		//placingZone->cellsFromBeginAtBottom [2][placingZone->nCellsFromBeginAtBottom].objType = EUROFORM;
		placingZone->cellsFromBeginAtBottom [2][placingZone->nCellsFromBeginAtBottom].dirLen = formLength;
		placingZone->cellsFromBeginAtBottom [2][placingZone->nCellsFromBeginAtBottom].libPart.form.eu_hei = formLength;

		placingZone->nCellsFromBeginAtBottom ++;
	}

	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)
		if (placingZone->cellsFromBeginAtBottom [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;

	// 위치 선정
	left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	left [1] = left [0] - placingZone->cellsFromBeginAtBottom [0][0].perLen;
	left [2] = left [1] - placingZone->cellsFromBeginAtBottom [1][0].perLen;
	for (xx = 0 ; xx < 3 ; ++xx) {
		xPos = centerPos - width/2 - accumDist;
		for (yy = 0 ; yy < placingZone->nCellsFromBeginAtSide ; ++yy) {
			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX = placingZone->begC.x + xPos;
			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY = left [xx];
			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			
			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX;
			rotatedPoint.y = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY;
			rotatedPoint.z = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX = unrotatedPoint.x;
			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY = unrotatedPoint.y;
			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ = unrotatedPoint.z;

			// 거리 이동
			xPos += placingZone->cellsFromBeginAtBottom [xx][yy].dirLen;
		}
	}

	// (2-2) 하부 끝 부분
	remainLength = placingZone->beamLength - centerPos - width/2;
	while (remainLength >= 0.600) {
		if (remainLength > formLength1) {
			formLength = formLength1;
			remainLength -= formLength1;
		} else if (remainLength > formLength2) {
			formLength = formLength2;
			remainLength -= formLength2;
		} else {
			formLength = formLength3;
			remainLength -= formLength3;
		}

		// 하부 [0]
		//placingZone->cellsFromEndAtBottom [0][placingZone->nCellsFromEndAtBottom].objType = EUROFORM;
		placingZone->cellsFromEndAtBottom [0][placingZone->nCellsFromEndAtBottom].dirLen = formLength;
		placingZone->cellsFromEndAtBottom [0][placingZone->nCellsFromEndAtBottom].libPart.form.eu_hei = formLength;

		// 하부 [1]
		//placingZone->cellsFromEndAtBottom [1][placingZone->nCellsFromEndAtBottom].objType = FILLERSPACER;
		placingZone->cellsFromEndAtBottom [1][placingZone->nCellsFromEndAtBottom].dirLen = formLength;
		placingZone->cellsFromEndAtBottom [1][placingZone->nCellsFromEndAtBottom].libPart.fillersp.f_leng = formLength;

		// 하부 [2]
		//placingZone->cellsFromEndAtBottom [2][placingZone->nCellsFromEndAtBottom].objType = EUROFORM;
		placingZone->cellsFromEndAtBottom [2][placingZone->nCellsFromEndAtBottom].dirLen = formLength;
		placingZone->cellsFromEndAtBottom [2][placingZone->nCellsFromEndAtBottom].libPart.form.eu_hei = formLength;

		placingZone->nCellsFromEndAtBottom ++;
	}

	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
		if (placingZone->cellsFromEndAtBottom [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	// 위치 선정
	left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	left [1] = left [0] - placingZone->cellsFromEndAtBottom [0][0].perLen;
	left [2] = left [1] - placingZone->cellsFromEndAtBottom [1][0].perLen;
	for (xx = 0 ; xx < 3 ; ++xx) {
		xPos = centerPos + width/2 + accumDist - placingZone->cellsFromEndAtBottom [0][0].dirLen;
		for (yy = 0 ; yy < placingZone->nCellsFromEndAtBottom ; ++yy) {
			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX = placingZone->begC.x + xPos;
			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY = left [xx];
			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			
			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX;
			rotatedPoint.y = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY;
			rotatedPoint.z = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX = unrotatedPoint.x;
			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY = unrotatedPoint.y;
			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ = unrotatedPoint.z;

			// 거리 이동
			if (yy < placingZone->nCellsFromEndAtBottom-1)
				xPos -= placingZone->cellsFromEndAtBottom [xx][yy+1].dirLen;
		}
	}

	// (2-3) 하부 중앙
	if (placingZone->bInterfereBeam == true) {
		//placingZone->cellCenterAtBottom [0].objType = EUROFORM;
		placingZone->cellCenterAtBottom [0].dirLen = width;
		placingZone->cellCenterAtBottom [0].libPart.form.eu_hei2 = width;

		//placingZone->cellCenterAtBottom [1].objType = FILLERSPACER;
		placingZone->cellCenterAtBottom [1].dirLen = width;
		placingZone->cellCenterAtBottom [1].libPart.fillersp.f_leng = width;

		//placingZone->cellCenterAtBottom [2].objType = EUROFORM;
		placingZone->cellCenterAtBottom [2].dirLen = width;
		placingZone->cellCenterAtBottom [2].libPart.form.eu_hei2 = width;
	}

	// 위치 선정
	xPos = centerPos - width/2;
	left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	left [1] = left [0] - placingZone->cellCenterAtBottom [0].perLen;
	left [2] = left [1] - placingZone->cellCenterAtBottom [1].perLen;
	for (xx = 0 ; xx < 3 ; ++xx) {
		placingZone->cellCenterAtBottom [xx].leftBottomX = placingZone->begC.x + xPos;
		placingZone->cellCenterAtBottom [xx].leftBottomY = left [xx];
		placingZone->cellCenterAtBottom [xx].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
		
		axisPoint.x = placingZone->begC.x;
		axisPoint.y = placingZone->begC.y;
		axisPoint.z = placingZone->begC.z;

		rotatedPoint.x = placingZone->cellCenterAtBottom [xx].leftBottomX;
		rotatedPoint.y = placingZone->cellCenterAtBottom [xx].leftBottomY;
		rotatedPoint.z = placingZone->cellCenterAtBottom [xx].leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

		placingZone->cellCenterAtBottom [xx].leftBottomX = unrotatedPoint.x;
		placingZone->cellCenterAtBottom [xx].leftBottomY = unrotatedPoint.y;
		placingZone->cellCenterAtBottom [xx].leftBottomZ = unrotatedPoint.z;
	}

	// 여백 값 초기화 (측면 시작 부분 여백)
	remainLength = centerPos - width/2;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
		remainLength -= placingZone->cellsFromBeginAtRSide [0][xx].dirLen;
	placingZone->marginBeginAtSide = remainLength;

	// 여백 값 초기화 (측면 끝 부분 여백)
	remainLength = placingZone->beamLength - centerPos - width/2;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
		remainLength -= placingZone->cellsFromEndAtRSide [0][xx].dirLen;
	placingZone->marginEndAtSide = remainLength;

	// 여백 값 초기화 (하부 시작 부분 여백)
	remainLength = centerPos - width/2;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)
		remainLength -= placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	placingZone->marginBeginAtBottom = remainLength;

	// 여백 값 초기화 (하부 끝 부분 여백)
	remainLength = placingZone->beamLength - centerPos - width/2;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
		remainLength -= placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	placingZone->marginEndAtBottom = remainLength;
}

// 측면 시작 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void		addNewColFromBeginAtSide (BeamPlacingZone* target_zone)
{
	short xx;

	for (xx = 0 ; xx < 4 ; ++xx) {
		target_zone->cellsFromBeginAtLSide [xx][target_zone->nCellsFromBeginAtSide] = target_zone->cellsFromBeginAtLSide [xx][target_zone->nCellsFromBeginAtSide - 1];
		target_zone->cellsFromBeginAtRSide [xx][target_zone->nCellsFromBeginAtSide] = target_zone->cellsFromBeginAtRSide [xx][target_zone->nCellsFromBeginAtSide - 1];
	}
	target_zone->nCellsFromBeginAtSide ++;
}

// 측면 시작 부분 - 마지막 열을 삭제함
void		delLastColFromBeginAtSide (BeamPlacingZone* target_zone)
{
	target_zone->nCellsFromBeginAtSide --;
}

// 측면 끝 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void		addNewColFromEndAtSide (BeamPlacingZone* target_zone)
{
	short xx;

	for (xx = 0 ; xx < 4 ; ++xx) {
		target_zone->cellsFromEndAtLSide [xx][target_zone->nCellsFromEndAtSide] = target_zone->cellsFromEndAtLSide [xx][target_zone->nCellsFromEndAtSide - 1];
		target_zone->cellsFromEndAtRSide [xx][target_zone->nCellsFromEndAtSide] = target_zone->cellsFromEndAtRSide [xx][target_zone->nCellsFromEndAtSide - 1];
	}
	target_zone->nCellsFromEndAtSide ++;
}

// 측면 끝 부분 - 마지막 열을 삭제함
void		delLastColFromEndAtSide (BeamPlacingZone* target_zone)
{
	target_zone->nCellsFromEndAtSide --;
}

// 하부 시작 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void		addNewColFromBeginAtBottom (BeamPlacingZone* target_zone)
{
	short xx;

	for (xx = 0 ; xx < 3 ; ++xx) {
		target_zone->cellsFromBeginAtBottom [xx][target_zone->nCellsFromBeginAtBottom] = target_zone->cellsFromBeginAtBottom [xx][target_zone->nCellsFromBeginAtBottom - 1];
	}
	target_zone->nCellsFromBeginAtBottom ++;
}

// 하부 시작 부분 - 마지막 열을 삭제함
void		delLastColFromBeginAtBottom (BeamPlacingZone* target_zone)
{
	target_zone->nCellsFromBeginAtBottom --;
}

// 하부 끝 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void		addNewColFromEndAtBottom (BeamPlacingZone* target_zone)
{
	short xx;

	for (xx = 0 ; xx < 3 ; ++xx) {
		target_zone->cellsFromEndAtBottom [xx][target_zone->nCellsFromEndAtBottom] = target_zone->cellsFromEndAtBottom [xx][target_zone->nCellsFromEndAtBottom - 1];
	}
	target_zone->nCellsFromEndAtBottom ++;
}

// 하부 끝 부분 - 마지막 열을 삭제함
void		delLastColFromEndAtBottom (BeamPlacingZone* target_zone)
{
	target_zone->nCellsFromEndAtBottom --;
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void		alignPlacingZoneForBeam (BeamPlacingZone* placingZone)
{
	short			xx, yy;
	API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	double			centerPos;		// 중심 위치
	double			width_side;		// 측면 중심 유로폼 너비
	double			width_bottom;	// 하부 중심 유로폼 너비
	double			remainLength;	// 남은 길이를 계산하기 위한 임시 변수
	double			xPos;			// 위치 커서
	double			accumDist;		// 이동 거리
	
	double			height [4];
	double			left [3];

	
	// 측면에서의 중심 위치 찾기
	if (placingZone->bInterfereBeam == true)
		centerPos = placingZone->posInterfereBeamFromLeft;	// 간섭 보의 중심 위치
	else
		centerPos = placingZone->beamLength / 2;			// 간섭 보가 없으면 중심을 기준으로 함

	// 중심 유로폼 너비
	if (placingZone->cellCenterAtRSide [0].objType != NONE)
		width_side = placingZone->cellCenterAtRSide [0].dirLen;
	else
		width_side = placingZone->centerLengthAtSide;

	if (placingZone->cellCenterAtBottom [0].objType != NONE)
		width_bottom = placingZone->cellCenterAtBottom [0].dirLen;
	else
		width_bottom = 0.0;


	// (1-1) 측면 시작 부분
	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
		if (placingZone->cellsFromBeginAtLSide [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	// 위치 선정
	height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	height [1] = height [0] + placingZone->cellsFromBeginAtRSide [0][0].perLen;
	height [2] = height [1] + placingZone->cellsFromBeginAtRSide [1][0].perLen;
	height [3] = height [2] + placingZone->cellsFromBeginAtRSide [2][0].perLen;
	for (xx = 0 ; xx < 4 ; ++xx) {
		xPos = centerPos - width_side/2 - accumDist;
		for (yy = 0 ; yy < placingZone->nCellsFromBeginAtSide ; ++yy) {
			if (placingZone->cellsFromBeginAtLSide [xx][yy].objType != NONE) {
				// 좌측
				placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
				placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
				placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ = height [xx];
		
				axisPoint.x = placingZone->begC.x;
				axisPoint.y = placingZone->begC.y;
				axisPoint.z = placingZone->begC.z;

				rotatedPoint.x = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX;
				rotatedPoint.y = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY;
				rotatedPoint.z = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ;
				unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

				placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX = unrotatedPoint.x;
				placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY = unrotatedPoint.y;
				placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ = unrotatedPoint.z;

				// 우측
				placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
				placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
				placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ = height [xx];

				axisPoint.x = placingZone->begC.x;
				axisPoint.y = placingZone->begC.y;
				axisPoint.z = placingZone->begC.z;

				rotatedPoint.x = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX;
				rotatedPoint.y = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY;
				rotatedPoint.z = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ;
				unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

				placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX = unrotatedPoint.x;
				placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY = unrotatedPoint.y;
				placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ = unrotatedPoint.z;

				// 거리 이동
				xPos += placingZone->cellsFromBeginAtRSide [xx][yy].dirLen;
			}
		}
	}

	// (1-2) 측면 끝 부분
	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
		if (placingZone->cellsFromEndAtLSide [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	// 위치 선정
	height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	height [1] = height [0] + placingZone->cellsFromEndAtRSide [0][0].perLen;
	height [2] = height [1] + placingZone->cellsFromEndAtRSide [1][0].perLen;
	height [3] = height [2] + placingZone->cellsFromEndAtRSide [2][0].perLen;
	for (xx = 0 ; xx < 4 ; ++xx) {
		xPos = centerPos + width_side/2 + accumDist - placingZone->cellsFromEndAtLSide [0][0].dirLen;
		for (yy = 0 ; yy < placingZone->nCellsFromEndAtSide ; ++yy) {
			if (placingZone->cellsFromEndAtLSide [xx][yy].objType != NONE) {
				// 좌측
				placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
				placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
				placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ = height [xx];
			
				axisPoint.x = placingZone->begC.x;
				axisPoint.y = placingZone->begC.y;
				axisPoint.z = placingZone->begC.z;

				rotatedPoint.x = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX;
				rotatedPoint.y = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY;
				rotatedPoint.z = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ;
				unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

				placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX = unrotatedPoint.x;
				placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY = unrotatedPoint.y;
				placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ = unrotatedPoint.z;

				// 우측
				placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
				placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
				placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ = height [xx];

				axisPoint.x = placingZone->begC.x;
				axisPoint.y = placingZone->begC.y;
				axisPoint.z = placingZone->begC.z;

				rotatedPoint.x = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX;
				rotatedPoint.y = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY;
				rotatedPoint.z = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ;
				unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

				placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX = unrotatedPoint.x;
				placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY = unrotatedPoint.y;
				placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ = unrotatedPoint.z;

				// 거리 이동
				if (yy < placingZone->nCellsFromEndAtSide-1)
					xPos -= placingZone->cellsFromEndAtRSide [xx][yy+1].dirLen;
			}
		}
	}

	// (1-3) 측면 중앙
	// 위치 선정
	xPos = centerPos - width_side/2;
	height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	height [1] = height [0] + placingZone->cellCenterAtRSide [0].perLen;
	height [2] = height [1] + placingZone->cellCenterAtRSide [1].perLen;
	height [3] = height [2] + placingZone->cellCenterAtRSide [2].perLen;
	for (xx = 0 ; xx < 4 ; ++xx) {
		// 좌측
		placingZone->cellCenterAtLSide [xx].leftBottomX = placingZone->begC.x + xPos;
		placingZone->cellCenterAtLSide [xx].leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
		placingZone->cellCenterAtLSide [xx].leftBottomZ = height [xx];
		
		axisPoint.x = placingZone->begC.x;
		axisPoint.y = placingZone->begC.y;
		axisPoint.z = placingZone->begC.z;

		rotatedPoint.x = placingZone->cellCenterAtLSide [xx].leftBottomX;
		rotatedPoint.y = placingZone->cellCenterAtLSide [xx].leftBottomY;
		rotatedPoint.z = placingZone->cellCenterAtLSide [xx].leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

		placingZone->cellCenterAtLSide [xx].leftBottomX = unrotatedPoint.x;
		placingZone->cellCenterAtLSide [xx].leftBottomY = unrotatedPoint.y;
		placingZone->cellCenterAtLSide [xx].leftBottomZ = unrotatedPoint.z;

		// 우측
		placingZone->cellCenterAtRSide [xx].leftBottomX = placingZone->begC.x + xPos;
		placingZone->cellCenterAtRSide [xx].leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
		placingZone->cellCenterAtRSide [xx].leftBottomZ = height [xx];

		axisPoint.x = placingZone->begC.x;
		axisPoint.y = placingZone->begC.y;
		axisPoint.z = placingZone->begC.z;

		rotatedPoint.x = placingZone->cellCenterAtRSide [xx].leftBottomX;
		rotatedPoint.y = placingZone->cellCenterAtRSide [xx].leftBottomY;
		rotatedPoint.z = placingZone->cellCenterAtRSide [xx].leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

		placingZone->cellCenterAtRSide [xx].leftBottomX = unrotatedPoint.x;
		placingZone->cellCenterAtRSide [xx].leftBottomY = unrotatedPoint.y;
		placingZone->cellCenterAtRSide [xx].leftBottomZ = unrotatedPoint.z;
	}

	// (2-1) 하부 시작 부분
	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)
		if (placingZone->cellsFromBeginAtBottom [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;

	// 위치 선정
	left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	left [1] = left [0] - placingZone->cellsFromBeginAtBottom [0][0].perLen;
	left [2] = left [1] - placingZone->cellsFromBeginAtBottom [1][0].perLen;
	for (xx = 0 ; xx < 3 ; ++xx) {
		xPos = centerPos - width_bottom/2 - accumDist;
		for (yy = 0 ; yy < placingZone->nCellsFromBeginAtBottom ; ++yy) {
			if (placingZone->cellsFromBeginAtBottom [xx][yy].objType != NONE) {
				placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX = placingZone->begC.x + xPos;
				placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY = left [xx] + infoBeam.offset;
				placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			
				axisPoint.x = placingZone->begC.x;
				axisPoint.y = placingZone->begC.y;
				axisPoint.z = placingZone->begC.z;

				rotatedPoint.x = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX;
				rotatedPoint.y = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY;
				rotatedPoint.z = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ;
				unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

				placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX = unrotatedPoint.x;
				placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY = unrotatedPoint.y;
				placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ = unrotatedPoint.z;

				// 거리 이동
				xPos += placingZone->cellsFromBeginAtBottom [xx][yy].dirLen;
			}
		}
	}

	// (2-2) 하부 끝 부분
	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
		if (placingZone->cellsFromEndAtBottom [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	// 위치 선정
	left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	left [1] = left [0] - placingZone->cellsFromEndAtBottom [0][0].perLen;
	left [2] = left [1] - placingZone->cellsFromEndAtBottom [1][0].perLen;
	for (xx = 0 ; xx < 3 ; ++xx) {
		xPos = centerPos + width_bottom/2 + accumDist - placingZone->cellsFromEndAtBottom [0][0].dirLen;
		for (yy = 0 ; yy < placingZone->nCellsFromEndAtBottom ; ++yy) {
			if (placingZone->cellsFromEndAtBottom [xx][yy].objType != NONE) {
				placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX = placingZone->begC.x + xPos;
				placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY = left [xx] + infoBeam.offset;
				placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			
				axisPoint.x = placingZone->begC.x;
				axisPoint.y = placingZone->begC.y;
				axisPoint.z = placingZone->begC.z;

				rotatedPoint.x = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX;
				rotatedPoint.y = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY;
				rotatedPoint.z = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ;
				unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

				placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX = unrotatedPoint.x;
				placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY = unrotatedPoint.y;
				placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ = unrotatedPoint.z;

				// 거리 이동
				if (yy < placingZone->nCellsFromEndAtBottom-1)
					xPos -= placingZone->cellsFromEndAtBottom [xx][yy+1].dirLen;
			}
		}
	}

	// (2-3) 하부 중앙
	// 위치 선정
	xPos = centerPos - width_bottom/2;
	left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	left [1] = left [0] - placingZone->cellCenterAtBottom [0].perLen;
	left [2] = left [1] - placingZone->cellCenterAtBottom [1].perLen;
	for (xx = 0 ; xx < 3 ; ++xx) {
		placingZone->cellCenterAtBottom [xx].leftBottomX = placingZone->begC.x + xPos;
		placingZone->cellCenterAtBottom [xx].leftBottomY = left [xx] + infoBeam.offset;
		placingZone->cellCenterAtBottom [xx].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
		
		axisPoint.x = placingZone->begC.x;
		axisPoint.y = placingZone->begC.y;
		axisPoint.z = placingZone->begC.z;

		rotatedPoint.x = placingZone->cellCenterAtBottom [xx].leftBottomX;
		rotatedPoint.y = placingZone->cellCenterAtBottom [xx].leftBottomY;
		rotatedPoint.z = placingZone->cellCenterAtBottom [xx].leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

		placingZone->cellCenterAtBottom [xx].leftBottomX = unrotatedPoint.x;
		placingZone->cellCenterAtBottom [xx].leftBottomY = unrotatedPoint.y;
		placingZone->cellCenterAtBottom [xx].leftBottomZ = unrotatedPoint.z;
	}

	// 여백 값 초기화 (측면 시작 부분 여백)
	remainLength = centerPos - width_side/2;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx) {
		if (placingZone->cellsFromBeginAtRSide [0][xx].objType != NONE)
			remainLength -= placingZone->cellsFromBeginAtRSide [0][xx].dirLen;
	}
	placingZone->marginBeginAtSide = remainLength;

	// 여백 값 초기화 (측면 끝 부분 여백)
	remainLength = placingZone->beamLength - centerPos - width_side/2;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx) {
		if (placingZone->cellsFromEndAtRSide [0][xx].objType != NONE)
			remainLength -= placingZone->cellsFromEndAtRSide [0][xx].dirLen;
	}
	placingZone->marginEndAtSide = remainLength;

	// 여백 값 초기화 (하부 시작 부분 여백)
	remainLength = centerPos - width_bottom/2;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx) {
		if (placingZone->cellsFromBeginAtBottom [0][xx].objType != NONE)
			remainLength -= placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	}
	placingZone->marginBeginAtBottom = remainLength;

	// 여백 값 초기화 (하부 끝 부분 여백)
	remainLength = placingZone->beamLength - centerPos - width_bottom/2;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx) {
		if (placingZone->cellsFromEndAtBottom [0][xx].objType != NONE)
			remainLength -= placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	}
	placingZone->marginEndAtBottom = remainLength;
}

// 해당 셀 정보를 기반으로 라이브러리 배치
API_Guid	placeLibPartForBeam (CellForBeam objInfo)
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

	std::string		tempString;

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
	if (objInfo.objType == FILLERSPACER)	gsmName = L("휠러스페이서v1.0.gsm");
	if (objInfo.objType == PLYWOOD)			gsmName = L("합판v1.0.gsm");
	if (objInfo.objType == WOOD)			gsmName = L("목재v1.0.gsm");
	if (objInfo.objType == OUTCORNER_ANGLE)	gsmName = L("아웃코너앵글v1.0.gsm");

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
	element.header.floorInd = infoBeam.floorInd;

	if (objInfo.objType == EUROFORM) {
		element.header.layer = layerInd_Euroform;

		// 규격품일 경우,
		if (objInfo.libPart.form.eu_stan_onoff == true) {
			// 규격폼 On/Off
			memo.params [0][27].value.real = TRUE;

			// 너비
			tempString = format_string ("%.0f", objInfo.libPart.form.eu_wid * 1000);
			GS::ucscpy (memo.params [0][28].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

			// 높이
			tempString = format_string ("%.0f", objInfo.libPart.form.eu_hei * 1000);
			GS::ucscpy (memo.params [0][29].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

		// 비규격품일 경우,
		} else {
			// 규격폼 On/Off
			memo.params [0][27].value.real = FALSE;

			// 너비
			memo.params [0][30].value.real = objInfo.libPart.form.eu_wid2;

			// 높이
			memo.params [0][31].value.real = objInfo.libPart.form.eu_hei2;
		}

		// 설치방향
		if (objInfo.libPart.form.u_ins_wall == true) {
			tempString = "벽세우기";
		} else {
			tempString = "벽눕히기";
			if (objInfo.libPart.form.eu_stan_onoff == true) {
				element.object.pos.x += ( objInfo.libPart.form.eu_hei * cos(objInfo.ang) );
				element.object.pos.y += ( objInfo.libPart.form.eu_hei * sin(objInfo.ang) );
				validLength = objInfo.libPart.form.eu_hei;
				validWidth = objInfo.libPart.form.eu_wid;
			} else {
				element.object.pos.x += ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
				element.object.pos.y += ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
				validLength = objInfo.libPart.form.eu_hei2;
				validWidth = objInfo.libPart.form.eu_wid2;
			}
		}
		GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

		// 회전X
		if (objInfo.attached_side == BOTTOM_SIDE) {
			memo.params [0][33].value.real = DegreeToRad (0.0);
		} else if (objInfo.attached_side == LEFT_SIDE) {
			memo.params [0][33].value.real = DegreeToRad (90.0);
			if (objInfo.libPart.form.eu_stan_onoff == true) {
				element.object.pos.x -= ( objInfo.libPart.form.eu_hei * cos(objInfo.ang) );
				element.object.pos.y -= ( objInfo.libPart.form.eu_hei * sin(objInfo.ang) );
			} else {
				element.object.pos.x -= ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
				element.object.pos.y -= ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
			}
			element.object.angle += DegreeToRad (180.0);
		} else {
			memo.params [0][33].value.real = DegreeToRad (90.0);
		}

	} else if (objInfo.objType == FILLERSPACER) {
		element.header.layer = layerInd_Fillerspacer;
		memo.params [0][27].value.real = objInfo.libPart.fillersp.f_thk;	// 두께
		memo.params [0][28].value.real = objInfo.libPart.fillersp.f_leng;	// 길이
		memo.params [0][29].value.real = 0.0;								// 각도

		if (objInfo.attached_side == BOTTOM_SIDE) {
			memo.params [0][30].value.real = DegreeToRad (90.0);	// 회전
		} else if (objInfo.attached_side == LEFT_SIDE) {
			memo.params [0][30].value.real = 0.0;					// 회전
			element.object.pos.x += ( objInfo.libPart.fillersp.f_leng * cos(objInfo.ang) );
			element.object.pos.y += ( objInfo.libPart.fillersp.f_leng * sin(objInfo.ang) );
			element.object.angle += DegreeToRad (180.0);
		} else {
			memo.params [0][30].value.real = 0.0;					// 회전
		}

		validLength = objInfo.libPart.fillersp.f_leng;
		validWidth = objInfo.libPart.fillersp.f_thk;

	} else if (objInfo.objType == PLYWOOD) {
		element.header.layer = layerInd_Plywood;
		GS::ucscpy (memo.params [0][32].value.uStr, L("비규격"));
		GS::ucscpy (memo.params [0][33].value.uStr, L("벽눕히기"));
		GS::ucscpy (memo.params [0][34].value.uStr, L("11.5T"));
		memo.params [0][35].value.real = objInfo.libPart.plywood.p_wid;		// 가로
		memo.params [0][36].value.real = objInfo.libPart.plywood.p_leng;	// 세로
		memo.params [0][38].value.real = TRUE;		// 제작틀 ON
		
		if (objInfo.attached_side == BOTTOM_SIDE) {
			memo.params [0][37].value.real = DegreeToRad (90.0);
		} else if (objInfo.attached_side == LEFT_SIDE) {
			element.object.pos.x += ( objInfo.libPart.plywood.p_leng * cos(objInfo.ang) );
			element.object.pos.y += ( objInfo.libPart.plywood.p_leng * sin(objInfo.ang) );
			element.object.angle += DegreeToRad (180.0);
		}

		validLength = objInfo.libPart.plywood.p_leng;
		validWidth = objInfo.libPart.plywood.p_wid;

	} else if (objInfo.objType == WOOD) {
		element.header.layer = layerInd_Wood;
		memo.params [0][28].value.real = objInfo.libPart.wood.w_w;		// 두께
		memo.params [0][29].value.real = objInfo.libPart.wood.w_h;		// 너비
		memo.params [0][30].value.real = objInfo.libPart.wood.w_leng;	// 길이
		memo.params [0][31].value.real = objInfo.libPart.wood.w_ang;	// 각도
	
		if (objInfo.attached_side == BOTTOM_SIDE) {
			GS::ucscpy (memo.params [0][27].value.uStr, L("바닥눕히기"));	// 설치방향
		} else if (objInfo.attached_side == LEFT_SIDE) {
			GS::ucscpy (memo.params [0][27].value.uStr, L("벽세우기"));		// 설치방향
			element.object.pos.x += ( objInfo.libPart.wood.w_leng * cos(objInfo.ang) );
			element.object.pos.y += ( objInfo.libPart.wood.w_leng * sin(objInfo.ang) );
			element.object.angle += DegreeToRad (180.0);
		} else {
			GS::ucscpy (memo.params [0][27].value.uStr, L("벽세우기"));		// 설치방향
		}

		validLength = objInfo.libPart.wood.w_leng;
		validWidth = objInfo.libPart.wood.w_h;

	} else if (objInfo.objType == OUTCORNER_ANGLE) {
		element.header.layer = layerInd_OutcornerAngle;
		memo.params [0][27].value.real = objInfo.libPart.outangle.a_leng;	// 길이
		memo.params [0][28].value.real = 0.0;								// 각도

		if (objInfo.attached_side == RIGHT_SIDE) {
			element.object.pos.x += ( objInfo.libPart.outangle.a_leng * cos(objInfo.ang) );
			element.object.pos.y += ( objInfo.libPart.outangle.a_leng * sin(objInfo.ang) );
			element.object.angle += DegreeToRad (180.0);
		}

		validLength = objInfo.libPart.outangle.a_leng;
		validWidth = 0.064;
	}

	// 객체 배치
	if ((objInfo.objType != NONE) && (validLength > EPS) && (validWidth > EPS))
		ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// 유로폼/휠러/목재를 채운 후 자투리 공간 채우기 (나머지 합판/목재 및 아웃코너앵글)
GSErrCode	fillRestAreasForBeam (BeamPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx;
	double	centerPos;		// 중심 위치
	double	width_side;		// 측면 중심 유로폼 너비
	double	width_bottom;	// 하부 중심 유로폼 너비
	double	xPos;			// 위치 커서
	double	accumDist;		// 이동 거리

	double	cellWidth_side;
	double	cellHeight_side, cellHeight_bottom;
	CellForBeam		insCell;
	API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;


	// 측면에서의 중심 위치 찾기
	if (placingZone->bInterfereBeam == true)
		centerPos = placingZone->posInterfereBeamFromLeft;	// 간섭 보의 중심 위치
	else
		centerPos = placingZone->beamLength / 2;			// 간섭 보가 없으면 중심을 기준으로 함

	// 중심 유로폼 너비
	if (placingZone->cellCenterAtRSide [0].objType != NONE)
		width_side = placingZone->cellCenterAtRSide [0].dirLen;
	else
		width_side = placingZone->centerLengthAtSide;

	if (placingZone->cellCenterAtBottom [0].objType != NONE)
		width_bottom = placingZone->cellCenterAtBottom [0].dirLen;
	else
		width_bottom = 0.0;

	// 중심 여백 너비 (중심 유로폼이 없을 경우에만 사용함)
	if (placingZone->bInterfereBeam == true)
		cellWidth_side = (placingZone->centerLengthAtSide - placingZone->interfereBeamWidth) / 2;
	else
		cellWidth_side = placingZone->centerLengthAtSide;

	// 측면 합판/목재 높이
	cellHeight_side = placingZone->cellsFromBeginAtRSide [0][0].perLen + placingZone->cellsFromBeginAtRSide [1][0].perLen + placingZone->cellsFromBeginAtRSide [2][0].perLen + placingZone->cellsFromBeginAtRSide [3][0].perLen;
	cellHeight_bottom = placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen;


	// 측면 중앙 셀이 NONE일 경우
	if (placingZone->cellCenterAtRSide [0].objType == NONE) {
		// 너비가 110 미만이면 목재, 110 이상이면 합판
		if (placingZone->bInterfereBeam == true) {
			// 좌측 1/2번째
			insCell.ang = placingZone->ang;
			insCell.attached_side = LEFT_SIDE;
			insCell.dirLen = cellWidth_side;
			insCell.perLen = cellHeight_side;
			insCell.leftBottomX = placingZone->begC.x + centerPos - placingZone->centerLengthAtSide/2;
			insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (cellWidth_side < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = DegreeToRad (90.0);
				insCell.libPart.wood.w_h = cellWidth_side;
				insCell.libPart.wood.w_leng = cellHeight_side;
				insCell.libPart.wood.w_w = 0.040;
				insCell.leftBottomX -= insCell.libPart.wood.w_leng;
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = cellWidth_side;
				insCell.libPart.plywood.p_wid = cellHeight_side;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 좌측 2/2번째
			insCell.ang = placingZone->ang;
			insCell.attached_side = LEFT_SIDE;
			insCell.dirLen = cellWidth_side;
			insCell.perLen = cellHeight_side;
			insCell.leftBottomX = placingZone->begC.x + centerPos + placingZone->centerLengthAtSide/2;
			insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (cellWidth_side < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = DegreeToRad (90.0);
				insCell.libPart.wood.w_h = cellWidth_side;
				insCell.libPart.wood.w_leng = cellHeight_side;
				insCell.libPart.wood.w_w = 0.040;
				insCell.leftBottomX -= (insCell.libPart.wood.w_leng + insCell.libPart.wood.w_h);
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = cellWidth_side;
				insCell.libPart.plywood.p_wid = cellHeight_side;
				insCell.leftBottomX -= insCell.libPart.plywood.p_leng;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 우측 1/2번째
			insCell.ang = placingZone->ang;
			insCell.attached_side = RIGHT_SIDE;
			insCell.dirLen = cellWidth_side;
			insCell.perLen = cellHeight_side;
			insCell.leftBottomX = placingZone->begC.x + centerPos - placingZone->centerLengthAtSide/2;
			insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (cellWidth_side < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = DegreeToRad (90.0);
				insCell.libPart.wood.w_h = cellWidth_side;
				insCell.libPart.wood.w_leng = cellHeight_side;
				insCell.libPart.wood.w_w = 0.040;
				insCell.leftBottomX += insCell.libPart.wood.w_h;
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = cellWidth_side;
				insCell.libPart.plywood.p_wid = cellHeight_side;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 우측 2/2번째
			insCell.ang = placingZone->ang;
			insCell.attached_side = RIGHT_SIDE;
			insCell.dirLen = cellWidth_side;
			insCell.perLen = cellHeight_side;
			insCell.leftBottomX = placingZone->begC.x + centerPos + placingZone->centerLengthAtSide/2;
			insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (cellWidth_side < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = DegreeToRad (90.0);
				insCell.libPart.wood.w_h = cellWidth_side;
				insCell.libPart.wood.w_leng = cellHeight_side;
				insCell.libPart.wood.w_w = 0.040;
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = cellWidth_side;
				insCell.libPart.plywood.p_wid = cellHeight_side;
				insCell.leftBottomX -= insCell.libPart.plywood.p_leng;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));
		}
	}

	// 측면 시작 부분 여백 채움
	if (placingZone->bFillMarginBeginAtSide == true) {
		if (placingZone->marginBeginAtSide > EPS) {
			// 좌측
			insCell.ang = placingZone->ang;
			insCell.attached_side = LEFT_SIDE;
			insCell.dirLen = placingZone->marginBeginAtSide;
			insCell.perLen = cellHeight_side;
			insCell.leftBottomX = placingZone->begC.x;
			insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (placingZone->marginBeginAtSide < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = DegreeToRad (90.0);
				insCell.libPart.wood.w_h = placingZone->marginBeginAtSide;
				insCell.libPart.wood.w_leng = cellHeight_side;
				insCell.libPart.wood.w_w = 0.040;
				insCell.leftBottomX -= cellHeight_side;
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = placingZone->marginBeginAtSide;
				insCell.libPart.plywood.p_wid = cellHeight_side;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 우측
			insCell.ang = placingZone->ang;
			insCell.attached_side = RIGHT_SIDE;
			insCell.dirLen = placingZone->marginBeginAtSide;
			insCell.perLen = cellHeight_side;
			insCell.leftBottomX = placingZone->begC.x;
			insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (placingZone->marginBeginAtSide < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = DegreeToRad (90.0);
				insCell.libPart.wood.w_h = placingZone->marginBeginAtSide;
				insCell.libPart.wood.w_leng = cellHeight_side;
				insCell.libPart.wood.w_w = 0.040;
				insCell.leftBottomX += placingZone->marginBeginAtSide;
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = placingZone->marginBeginAtSide;
				insCell.libPart.plywood.p_wid = cellHeight_side;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));
		}
	}

	// 측면 끝 부분 여백 채움
	if (placingZone->bFillMarginEndAtSide == true) {
		if (placingZone->marginEndAtSide > EPS) {
			// 좌측
			insCell.ang = placingZone->ang;
			insCell.attached_side = LEFT_SIDE;
			insCell.dirLen = placingZone->marginEndAtSide;
			insCell.perLen = cellHeight_side;
			insCell.leftBottomX = placingZone->begC.x + placingZone->beamLength - placingZone->marginEndAtSide;
			insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (placingZone->marginBeginAtSide < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = DegreeToRad (90.0);
				insCell.libPart.wood.w_h = placingZone->marginEndAtSide;
				insCell.libPart.wood.w_leng = cellHeight_side;
				insCell.libPart.wood.w_w = 0.040;
				insCell.leftBottomX -= cellHeight_side;
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = placingZone->marginEndAtSide;
				insCell.libPart.plywood.p_wid = cellHeight_side;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 우측
			insCell.ang = placingZone->ang;
			insCell.attached_side = RIGHT_SIDE;
			insCell.dirLen = placingZone->marginEndAtSide;
			insCell.perLen = cellHeight_side;
			insCell.leftBottomX = placingZone->begC.x + placingZone->beamLength - placingZone->marginEndAtSide;
			insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (placingZone->marginEndAtSide < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = DegreeToRad (90.0);
				insCell.libPart.wood.w_h = placingZone->marginEndAtSide;
				insCell.libPart.wood.w_leng = cellHeight_side;
				insCell.libPart.wood.w_w = 0.040;
				insCell.leftBottomX += placingZone->marginEndAtSide;
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = placingZone->marginEndAtSide;
				insCell.libPart.plywood.p_wid = cellHeight_side;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));
		}
	}

	// 하부 시작 부분 여백 채움
	if (placingZone->bFillMarginBeginAtBottom == true) {
		if (placingZone->marginBeginAtBottom > EPS) {
			insCell.ang = placingZone->ang;
			insCell.attached_side = BOTTOM_SIDE;
			insCell.dirLen = placingZone->marginBeginAtBottom;
			insCell.perLen = cellHeight_bottom;
			insCell.leftBottomX = placingZone->begC.x;
			insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (placingZone->marginBeginAtBottom < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = 0.0;
				insCell.libPart.wood.w_h = placingZone->marginBeginAtBottom;
				insCell.libPart.wood.w_leng = cellHeight_bottom;
				insCell.libPart.wood.w_w = 0.040;
				insCell.ang -= DegreeToRad (90.0);
				insCell.leftBottomX += placingZone->marginBeginAtBottom;
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = placingZone->marginBeginAtBottom;
				insCell.libPart.plywood.p_wid = cellHeight_bottom;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));
		}
	}
	
	// 하부 끝 부분 여백 채움
	if (placingZone->bFillMarginEndAtBottom == true) {
		if (placingZone->marginEndAtBottom > EPS) {
			insCell.ang = placingZone->ang;
			insCell.attached_side = BOTTOM_SIDE;
			insCell.dirLen = placingZone->marginEndAtBottom;
			insCell.perLen = cellHeight_bottom;
			insCell.leftBottomX = placingZone->begC.x + placingZone->beamLength - placingZone->marginEndAtBottom;
			insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

			if (placingZone->marginEndAtBottom < 0.110) {
				insCell.objType = WOOD;
				insCell.libPart.wood.w_ang = 0.0;
				insCell.libPart.wood.w_h = placingZone->marginEndAtBottom;
				insCell.libPart.wood.w_leng = cellHeight_bottom;
				insCell.libPart.wood.w_w = 0.040;
				insCell.ang -= DegreeToRad (90.0);
				insCell.leftBottomX += placingZone->marginEndAtBottom;
			} else {
				insCell.objType = PLYWOOD;
				insCell.libPart.plywood.p_leng = placingZone->marginEndAtBottom;
				insCell.libPart.plywood.p_wid = cellHeight_bottom;
			}

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));
		}
	}

	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
		if (placingZone->cellsFromBeginAtRSide [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromBeginAtRSide [0][xx].dirLen;

	// 아웃코너앵글 설치 (측면 시작 부분)
	xPos = centerPos - width_side/2 - accumDist;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx) {
		if (placingZone->cellsFromBeginAtLSide [0][xx].objType != NONE) {
			// 좌측
			insCell.objType = OUTCORNER_ANGLE;
			insCell.ang = placingZone->ang;
			insCell.attached_side = LEFT_SIDE;
			insCell.leftBottomX = placingZone->begC.x + xPos;
			insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			insCell.libPart.outangle.a_leng = placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 우측
			insCell.objType = OUTCORNER_ANGLE;
			insCell.ang = placingZone->ang;
			insCell.attached_side = RIGHT_SIDE;
			insCell.leftBottomX = placingZone->begC.x + xPos;
			insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			insCell.libPart.outangle.a_leng = placingZone->cellsFromBeginAtRSide [0][xx].dirLen;

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 거리 이동
			xPos += placingZone->cellsFromBeginAtRSide [0][xx].dirLen;
		}
	}

	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
		if (placingZone->cellsFromEndAtRSide [0][xx].objType != NONE)
			accumDist += placingZone->cellsFromEndAtRSide [0][xx].dirLen;

	// 아웃코너앵글 설치 (측면 끝 부분)
	xPos = centerPos + width_side/2 + accumDist - placingZone->cellsFromEndAtLSide [0][0].dirLen;
	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx) {
		if (placingZone->cellsFromEndAtLSide [0][xx].objType != NONE) {
			// 좌측
			insCell.objType = OUTCORNER_ANGLE;
			insCell.ang = placingZone->ang;
			insCell.attached_side = LEFT_SIDE;
			insCell.leftBottomX = placingZone->begC.x + xPos;
			insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			insCell.libPart.outangle.a_leng = placingZone->cellsFromEndAtLSide [0][xx].dirLen;

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 우측
			insCell.objType = OUTCORNER_ANGLE;
			insCell.ang = placingZone->ang;
			insCell.attached_side = RIGHT_SIDE;
			insCell.leftBottomX = placingZone->begC.x + xPos;
			insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			insCell.libPart.outangle.a_leng = placingZone->cellsFromEndAtRSide [0][xx].dirLen;

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 거리 이동
			if (xx < placingZone->nCellsFromEndAtSide-1)
				xPos -= placingZone->cellsFromEndAtRSide [0][xx+1].dirLen;
		}
	}

	// 아웃코너 앵글 설치 (중앙 부분)
	xPos = centerPos - width_side/2;
	if (placingZone->bInterfereBeam == false) {
		if (placingZone->cellCenterAtRSide [0].objType == EUROFORM) {
			// 좌측
			insCell.objType = OUTCORNER_ANGLE;
			insCell.ang = placingZone->ang;
			insCell.attached_side = LEFT_SIDE;
			insCell.leftBottomX = placingZone->begC.x + xPos;
			insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			insCell.libPart.outangle.a_leng = placingZone->cellCenterAtLSide [0].dirLen;

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));

			// 우측
			insCell.objType = OUTCORNER_ANGLE;
			insCell.ang = placingZone->ang;
			insCell.attached_side = RIGHT_SIDE;
			insCell.leftBottomX = placingZone->begC.x + xPos;
			insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
			insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
			insCell.libPart.outangle.a_leng = placingZone->cellCenterAtRSide [0].dirLen;

			axisPoint.x = placingZone->begC.x;
			axisPoint.y = placingZone->begC.y;
			axisPoint.z = placingZone->begC.z;

			rotatedPoint.x = insCell.leftBottomX;
			rotatedPoint.y = insCell.leftBottomY;
			rotatedPoint.z = insCell.leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			insCell.leftBottomX = unrotatedPoint.x;
			insCell.leftBottomY = unrotatedPoint.y;
			insCell.leftBottomZ = unrotatedPoint.z;

			elemList.Push (placeLibPartForBeam (insCell));
		}
	}

	return	err;
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK beamPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	double		h1, h2, h3, h4, hRest;	// 나머지 높이 계산을 위한 변수
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "보에 배치 - 보 단면");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 확인 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 취소 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			//////////////////////////////////////////////////////////// 아이템 배치 (유로폼)
			// 라벨 및 체크박스
			DGSetItemText (dialogID, LABEL_BEAM_SECTION, "보 단면");
			DGSetItemText (dialogID, LABEL_BEAM_HEIGHT, "보 높이");
			DGSetItemText (dialogID, LABEL_BEAM_WIDTH, "보 너비");
			DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT, "총 높이");
			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, "총 너비");

			DGSetItemText (dialogID, LABEL_REST_SIDE, "나머지");
			DGSetItemText (dialogID, CHECKBOX_WOOD_SIDE, "목재");
			DGSetItemText (dialogID, CHECKBOX_T_FORM_SIDE, "유로폼");
			DGSetItemText (dialogID, CHECKBOX_FILLER_SIDE, "휠러");
			DGSetItemText (dialogID, CHECKBOX_B_FORM_SIDE, "유로폼");

			DGSetItemText (dialogID, CHECKBOX_L_FORM_BOTTOM, "유로폼");
			DGSetItemText (dialogID, CHECKBOX_FILLER_BOTTOM, "휠러");
			DGSetItemText (dialogID, CHECKBOX_R_FORM_BOTTOM, "유로폼");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSPACER, "휠러스페이서");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "목재");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "아웃코너앵글");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSPACER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);

			// 보 높이/너비 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_HEIGHT, placingZone.areaHeight);
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, infoBeam.width);

			// 총 높이/너비 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.areaHeight + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, infoBeam.width + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1)*2);

			// 부재별 체크박스-규격 설정
			(DGGetItemValLong (dialogID, CHECKBOX_WOOD_SIDE) == TRUE) ?		DGEnableItem (dialogID, EDITCONTROL_WOOD_SIDE)		:	DGDisableItem (dialogID, EDITCONTROL_WOOD_SIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_SIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_SIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_SIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_SIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_SIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_SIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
			(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

			// 측면 0번, 하부 0번 셀은 무조건 사용해야 함
			DGSetItemValLong (dialogID, CHECKBOX_B_FORM_SIDE, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM, TRUE);
			DGDisableItem (dialogID, CHECKBOX_B_FORM_SIDE);
			DGDisableItem (dialogID, CHECKBOX_L_FORM_BOTTOM);

			// 나머지 값 계산
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_WOOD_SIDE) == TRUE)	h1 = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_SIDE) == TRUE)	h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_SIDE) == TRUE)	h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_SIDE) == TRUE)	h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_SIDE, hRest);

			// 직접 변경해서는 안 되는 항목 잠그기
			DGDisableItem (dialogID, EDITCONTROL_GAP_SIDE2);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_REST_SIDE);

			break;
		
		case DG_MSG_CHANGE:
			// 나머지 값 계산
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_WOOD_SIDE) == TRUE)	h1 = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_SIDE) == TRUE)	h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_SIDE) == TRUE)	h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_SIDE) == TRUE)	h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_SIDE, hRest);

			switch (item) {
				// 측면 간격은 동일함
				// 총 높이/너비 계산
				case EDITCONTROL_GAP_SIDE1:
				case EDITCONTROL_GAP_BOTTOM:
					DGSetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2, DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1));
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.areaHeight + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, infoBeam.width + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1)*2);

					break;

				// 부재별 체크박스-규격 설정
				case CHECKBOX_WOOD_SIDE:
					(DGGetItemValLong (dialogID, CHECKBOX_WOOD_SIDE) == TRUE) ?		DGEnableItem (dialogID, EDITCONTROL_WOOD_SIDE)		:	DGDisableItem (dialogID, EDITCONTROL_WOOD_SIDE);
					break;
				case CHECKBOX_T_FORM_SIDE:
					(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_SIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_SIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_SIDE);
					break;
				case CHECKBOX_FILLER_SIDE:
					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_SIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_SIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_SIDE);
					break;
				case CHECKBOX_B_FORM_SIDE:
					(DGGetItemValLong (dialogID, CHECKBOX_B_FORM_SIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_B_FORM_SIDE)			:	DGDisableItem (dialogID, POPUP_B_FORM_SIDE);
					break;

				case CHECKBOX_L_FORM_BOTTOM:
					(DGGetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_L_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_L_FORM_BOTTOM);
					break;
				case CHECKBOX_FILLER_BOTTOM:
					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
					break;
				case CHECKBOX_R_FORM_BOTTOM:
					(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);
					break;
			}

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 셀 설정 적용
					// 측면 [0]
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_SIDE) == TRUE) {
						for (xx = 0 ; xx < 20 ; ++xx) {
							// 좌측 [0]
							placingZone.cellsFromBeginAtLSide [0][xx].objType = EUROFORM;
							placingZone.cellsFromBeginAtLSide [0][xx].attached_side = LEFT_SIDE;
							placingZone.cellsFromBeginAtLSide [0][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtLSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtLSide [0][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromBeginAtLSide [0][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromBeginAtLSide [0][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtLSide [0][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;

							placingZone.cellsFromEndAtLSide [0][xx].objType = EUROFORM;
							placingZone.cellsFromEndAtLSide [0][xx].attached_side = LEFT_SIDE;
							placingZone.cellsFromEndAtLSide [0][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtLSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtLSide [0][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromEndAtLSide [0][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromEndAtLSide [0][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtLSide [0][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;

							placingZone.cellCenterAtLSide [0].objType = EUROFORM;
							placingZone.cellCenterAtLSide [0].attached_side = LEFT_SIDE;
							placingZone.cellCenterAtLSide [0].ang = placingZone.ang;
							placingZone.cellCenterAtLSide [0].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtLSide [0].libPart.form.eu_stan_onoff = false;
							placingZone.cellCenterAtLSide [0].libPart.form.u_ins_wall = false;
							placingZone.cellCenterAtLSide [0].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtLSide [0].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;

							// 우측 [0]
							placingZone.cellsFromBeginAtRSide [0][xx].objType = EUROFORM;
							placingZone.cellsFromBeginAtRSide [0][xx].attached_side = RIGHT_SIDE;
							placingZone.cellsFromBeginAtRSide [0][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtRSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtRSide [0][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromBeginAtRSide [0][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromBeginAtRSide [0][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtRSide [0][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							
							placingZone.cellsFromEndAtRSide [0][xx].objType = EUROFORM;
							placingZone.cellsFromEndAtRSide [0][xx].attached_side = RIGHT_SIDE;
							placingZone.cellsFromEndAtRSide [0][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtRSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtRSide [0][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromEndAtRSide [0][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromEndAtRSide [0][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtRSide [0][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;

							placingZone.cellCenterAtRSide [0].objType = EUROFORM;
							placingZone.cellCenterAtRSide [0].attached_side = RIGHT_SIDE;
							placingZone.cellCenterAtRSide [0].ang = placingZone.ang;
							placingZone.cellCenterAtRSide [0].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtRSide [0].libPart.form.eu_stan_onoff = false;
							placingZone.cellCenterAtRSide [0].libPart.form.u_ins_wall = false;
							placingZone.cellCenterAtRSide [0].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtRSide [0].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_SIDE)).ToCStr ()) / 1000.0;
						}
					}

					// 측면 [1]
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_SIDE) == TRUE) {
						for (xx = 0 ; xx < 20 ; ++xx) {
							// 좌측 [1]
							placingZone.cellsFromBeginAtLSide [1][xx].objType = FILLERSPACER;
							placingZone.cellsFromBeginAtLSide [1][xx].attached_side = LEFT_SIDE;
							placingZone.cellsFromBeginAtLSide [1][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtLSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);
							placingZone.cellsFromBeginAtLSide [1][xx].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);

							placingZone.cellsFromEndAtLSide [1][xx].objType = FILLERSPACER;
							placingZone.cellsFromEndAtLSide [1][xx].attached_side = LEFT_SIDE;
							placingZone.cellsFromEndAtLSide [1][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtLSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);
							placingZone.cellsFromEndAtLSide [1][xx].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);

							placingZone.cellCenterAtLSide [1].objType = FILLERSPACER;
							placingZone.cellCenterAtLSide [1].attached_side = LEFT_SIDE;
							placingZone.cellCenterAtLSide [1].ang = placingZone.ang;
							placingZone.cellCenterAtLSide [1].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);
							placingZone.cellCenterAtLSide [1].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);

							// 우측 [1]
							placingZone.cellsFromBeginAtRSide [1][xx].objType = FILLERSPACER;
							placingZone.cellsFromBeginAtRSide [1][xx].attached_side = RIGHT_SIDE;
							placingZone.cellsFromBeginAtRSide [1][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtRSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);
							placingZone.cellsFromBeginAtRSide [1][xx].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);

							placingZone.cellsFromEndAtRSide [1][xx].objType = FILLERSPACER;
							placingZone.cellsFromEndAtRSide [1][xx].attached_side = RIGHT_SIDE;
							placingZone.cellsFromEndAtRSide [1][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtRSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);
							placingZone.cellsFromEndAtRSide [1][xx].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);

							placingZone.cellCenterAtRSide [1].objType = FILLERSPACER;
							placingZone.cellCenterAtRSide [1].attached_side = RIGHT_SIDE;
							placingZone.cellCenterAtRSide [1].ang = placingZone.ang;
							placingZone.cellCenterAtRSide [1].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);
							placingZone.cellCenterAtRSide [1].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_SIDE);
						}
					}

					// 측면 [2]
					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_SIDE) == TRUE) {
						for (xx = 0 ; xx < 20 ; ++xx) {
							// 좌측 [2]
							placingZone.cellsFromBeginAtLSide [2][xx].objType = EUROFORM;
							placingZone.cellsFromBeginAtLSide [2][xx].attached_side = LEFT_SIDE;
							placingZone.cellsFromBeginAtLSide [2][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtLSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtLSide [2][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromBeginAtLSide [2][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromBeginAtLSide [2][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtLSide [2][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;

							placingZone.cellsFromEndAtLSide [2][xx].objType = EUROFORM;
							placingZone.cellsFromEndAtLSide [2][xx].attached_side = LEFT_SIDE;
							placingZone.cellsFromEndAtLSide [2][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtLSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtLSide [2][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromEndAtLSide [2][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromEndAtLSide [2][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtLSide [2][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;

							placingZone.cellCenterAtLSide [2].objType = EUROFORM;
							placingZone.cellCenterAtLSide [2].attached_side = LEFT_SIDE;
							placingZone.cellCenterAtLSide [2].ang = placingZone.ang;
							placingZone.cellCenterAtLSide [2].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtLSide [2].libPart.form.eu_stan_onoff = false;
							placingZone.cellCenterAtLSide [2].libPart.form.u_ins_wall = false;
							placingZone.cellCenterAtLSide [2].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtLSide [2].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;

							// 우측 [2]
							placingZone.cellsFromBeginAtRSide [2][xx].objType = EUROFORM;
							placingZone.cellsFromBeginAtRSide [2][xx].attached_side = RIGHT_SIDE;
							placingZone.cellsFromBeginAtRSide [2][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtRSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtRSide [2][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromBeginAtRSide [2][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromBeginAtRSide [2][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtRSide [2][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;

							placingZone.cellsFromEndAtRSide [2][xx].objType = EUROFORM;
							placingZone.cellsFromEndAtRSide [2][xx].attached_side = RIGHT_SIDE;
							placingZone.cellsFromEndAtRSide [2][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtRSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtRSide [2][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromEndAtRSide [2][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromEndAtRSide [2][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtRSide [2][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;

							placingZone.cellCenterAtRSide [2].objType = EUROFORM;
							placingZone.cellCenterAtRSide [2].attached_side = RIGHT_SIDE;
							placingZone.cellCenterAtRSide [2].ang = placingZone.ang;
							placingZone.cellCenterAtRSide [2].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtRSide [2].libPart.form.eu_stan_onoff = false;
							placingZone.cellCenterAtRSide [2].libPart.form.u_ins_wall = false;
							placingZone.cellCenterAtRSide [2].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtRSide [2].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_SIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_SIDE)).ToCStr ()) / 1000.0;
						}
					}

					// 측면 [3]
					if (DGGetItemValLong (dialogID, CHECKBOX_WOOD_SIDE) == TRUE) {
						for (xx = 0 ; xx < 20 ; ++xx) {
							// 좌측 [3]
							placingZone.cellsFromBeginAtLSide [3][xx].objType = WOOD;
							placingZone.cellsFromBeginAtLSide [3][xx].attached_side = LEFT_SIDE;
							placingZone.cellsFromBeginAtLSide [3][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtLSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellsFromBeginAtLSide [3][xx].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellsFromBeginAtLSide [3][xx].libPart.wood.w_w = 0.050;

							placingZone.cellsFromEndAtLSide [3][xx].objType = WOOD;
							placingZone.cellsFromEndAtLSide [3][xx].attached_side = LEFT_SIDE;
							placingZone.cellsFromEndAtLSide [3][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtLSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellsFromEndAtLSide [3][xx].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellsFromEndAtLSide [3][xx].libPart.wood.w_w = 0.050;

							placingZone.cellCenterAtLSide [3].objType = WOOD;
							placingZone.cellCenterAtLSide [3].attached_side = LEFT_SIDE;
							placingZone.cellCenterAtLSide [3].ang = placingZone.ang;
							placingZone.cellCenterAtLSide [3].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellCenterAtLSide [3].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellCenterAtLSide [3].libPart.wood.w_w = 0.050;

							// 우측 [3]
							placingZone.cellsFromBeginAtRSide [3][xx].objType = WOOD;
							placingZone.cellsFromBeginAtRSide [3][xx].attached_side = RIGHT_SIDE;
							placingZone.cellsFromBeginAtRSide [3][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtRSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellsFromBeginAtRSide [3][xx].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellsFromBeginAtRSide [3][xx].libPart.wood.w_w = 0.050;

							placingZone.cellsFromEndAtRSide [3][xx].objType = WOOD;
							placingZone.cellsFromEndAtRSide [3][xx].attached_side = RIGHT_SIDE;
							placingZone.cellsFromEndAtRSide [3][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtRSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellsFromEndAtRSide [3][xx].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellsFromEndAtRSide [3][xx].libPart.wood.w_w = 0.050;

							placingZone.cellCenterAtRSide [3].objType = WOOD;
							placingZone.cellCenterAtRSide [3].attached_side = RIGHT_SIDE;
							placingZone.cellCenterAtRSide [3].ang = placingZone.ang;
							placingZone.cellCenterAtRSide [3].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellCenterAtRSide [3].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_SIDE);
							placingZone.cellCenterAtRSide [3].libPart.wood.w_w = 0.050;
						}
					}

					// 하부 [0]
					if (DGGetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM) == TRUE) {
						for (xx = 0 ; xx < 20 ; ++xx) {
							placingZone.cellsFromBeginAtBottom [0][xx].objType = EUROFORM;
							placingZone.cellsFromBeginAtBottom [0][xx].attached_side = BOTTOM_SIDE;
							placingZone.cellsFromBeginAtBottom [0][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtBottom [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtBottom [0][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromBeginAtBottom [0][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromBeginAtBottom [0][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtBottom [0][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;

							placingZone.cellsFromEndAtBottom [0][xx].objType = EUROFORM;
							placingZone.cellsFromEndAtBottom [0][xx].attached_side = BOTTOM_SIDE;
							placingZone.cellsFromEndAtBottom [0][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtBottom [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtBottom [0][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromEndAtBottom [0][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromEndAtBottom [0][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtBottom [0][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;

							placingZone.cellCenterAtBottom [0].objType = EUROFORM;
							placingZone.cellCenterAtBottom [0].attached_side = BOTTOM_SIDE;
							placingZone.cellCenterAtBottom [0].ang = placingZone.ang;
							placingZone.cellCenterAtBottom [0].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtBottom [0].libPart.form.eu_stan_onoff = false;
							placingZone.cellCenterAtBottom [0].libPart.form.u_ins_wall = false;
							placingZone.cellCenterAtBottom [0].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtBottom [0].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
						}
					}

					// 하부 [1]
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) {
						for (xx = 0 ; xx < 20 ; ++xx) {
							placingZone.cellsFromBeginAtBottom [1][xx].objType = FILLERSPACER;
							placingZone.cellsFromBeginAtBottom [1][xx].attached_side = BOTTOM_SIDE;
							placingZone.cellsFromBeginAtBottom [1][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtBottom [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_BOTTOM);
							placingZone.cellsFromBeginAtBottom [1][xx].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_BOTTOM);

							placingZone.cellsFromEndAtBottom [1][xx].objType = FILLERSPACER;
							placingZone.cellsFromEndAtBottom [1][xx].attached_side = BOTTOM_SIDE;
							placingZone.cellsFromEndAtBottom [1][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtBottom [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_BOTTOM);
							placingZone.cellsFromEndAtBottom [1][xx].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_BOTTOM);

							placingZone.cellCenterAtBottom [1].objType = FILLERSPACER;
							placingZone.cellCenterAtBottom [1].attached_side = BOTTOM_SIDE;
							placingZone.cellCenterAtBottom [1].ang = placingZone.ang;
							placingZone.cellCenterAtBottom [1].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_BOTTOM);
							placingZone.cellCenterAtBottom [1].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_BOTTOM);
						}
					}

					// 하부 [2]
					if (DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) {
						for (xx = 0 ; xx < 20 ; ++xx) {
							placingZone.cellsFromBeginAtBottom [2][xx].objType = EUROFORM;
							placingZone.cellsFromBeginAtBottom [2][xx].attached_side = BOTTOM_SIDE;
							placingZone.cellsFromBeginAtBottom [2][xx].ang = placingZone.ang;
							placingZone.cellsFromBeginAtBottom [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtBottom [2][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromBeginAtBottom [2][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromBeginAtBottom [2][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellsFromBeginAtBottom [2][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;

							placingZone.cellsFromEndAtBottom [2][xx].objType = EUROFORM;
							placingZone.cellsFromEndAtBottom [2][xx].attached_side = BOTTOM_SIDE;
							placingZone.cellsFromEndAtBottom [2][xx].ang = placingZone.ang;
							placingZone.cellsFromEndAtBottom [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtBottom [2][xx].libPart.form.eu_stan_onoff = true;
							placingZone.cellsFromEndAtBottom [2][xx].libPart.form.u_ins_wall = false;
							placingZone.cellsFromEndAtBottom [2][xx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellsFromEndAtBottom [2][xx].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;

							placingZone.cellCenterAtBottom [2].objType = EUROFORM;
							placingZone.cellCenterAtBottom [2].attached_side = BOTTOM_SIDE;
							placingZone.cellCenterAtBottom [2].ang = placingZone.ang;
							placingZone.cellCenterAtBottom [2].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtBottom [2].libPart.form.eu_stan_onoff = false;
							placingZone.cellCenterAtBottom [2].libPart.form.u_ins_wall = false;
							placingZone.cellCenterAtBottom [2].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
							placingZone.cellCenterAtBottom [2].libPart.form.eu_wid2 = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
						}
					}

					// 보와의 간격
					placingZone.gapSide = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1);
					placingZone.gapBottom = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM);

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Fillerspacer	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
					layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);

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
short DGCALLBACK beamPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGSetDialogTitle (dialogID, "보에 배치 - 보 측면");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 400, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 440, 100, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 업데이트 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 360, 100, 25);
			DGSetItemFont (dialogID, DG_UPDATE_BUTTON, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_UPDATE_BUTTON, "업데이트");
			DGShowItem (dialogID, DG_UPDATE_BUTTON);

			// 이전 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 480, 100, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "이전");
			DGShowItem (dialogID, DG_PREV);

			// 라벨: 보 우측면/화부면
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 10, 100, 23);
			DGSetItemFont (dialogID, LABEL_BEAM_SIDE_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_BEAM_SIDE_BOTTOM, "보 우측면/하부면");
			DGShowItem (dialogID, LABEL_BEAM_SIDE_BOTTOM);

			// 라벨: 우측면
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 70, 60, 23);
			DGSetItemFont (dialogID, LABEL_BEAM_SIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_BEAM_SIDE, "우측면");
			DGShowItem (dialogID, LABEL_BEAM_SIDE);

			// 라벨: 하부
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 230, 60, 23);
			DGSetItemFont (dialogID, LABEL_BEAM_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_BEAM_BOTTOM, "하부면");
			DGShowItem (dialogID, LABEL_BEAM_BOTTOM);

			// 측면 시작 부분 여백 채움 여부 - bFillMarginBeginAtSide
			// 라디오 버튼: 여백 (채움)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 채움");
			DGShowItem (dialogID, itmIdx);
			MARGIN_FILL_FROM_BEGIN_AT_SIDE = itmIdx;
			// 라디오 버튼: 여백 (비움)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 135, 70, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 비움");
			DGShowItem (dialogID, itmIdx);
			MARGIN_EMPTY_FROM_BEGIN_AT_SIDE = itmIdx;
			DGSetItemValLong (dialogID, itmIdx, TRUE);

			// 측면 시작 부분 여백
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 50, btnSizeX, btnSizeY);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 55, 45, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 74, 45, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtSide);
			DGShowItem (dialogID, itmIdx);
			DGDisableItem (dialogID, itmIdx);
			MARGIN_FROM_BEGIN_AT_SIDE = itmIdx;
			btnPosX = 150;
			btnPosY = 50;
			// 측면 시작 부분
			for (xx = 0 ; xx < placingZone.nCellsFromBeginAtSide ; ++xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellsFromBeginAtRSide [0][xx].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellsFromBeginAtRSide [0][xx].objType == EUROFORM) {
					txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtRSide [0][xx].dirLen * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				if (xx == 0) START_INDEX_FROM_BEGIN_AT_SIDE = itmIdx;
				btnPosX += 50;
			}
			// 화살표 추가
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "↑");
			DGShowItem (dialogID, itmIdx);
			// 추가/삭제 버튼
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "추가");
			DGShowItem (dialogID, itmIdx);
			ADD_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "삭제");
			DGShowItem (dialogID, itmIdx);
			DEL_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			// 측면 중앙 부분
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			txtButton = "";
			if (placingZone.cellCenterAtRSide [0].objType == NONE) {
				txtButton = "NONE";
			} else if (placingZone.cellCenterAtRSide [0].objType == EUROFORM) {
				txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
			}
			DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			DGShowItem (dialogID, itmIdx);
			START_INDEX_CENTER_AT_SIDE = itmIdx;
			btnPosX += 50;
			// 화살표 추가
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "↑");
			DGShowItem (dialogID, itmIdx);
			// 추가/삭제 버튼
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "추가");
			DGShowItem (dialogID, itmIdx);
			ADD_CELLS_FROM_END_AT_SIDE = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "삭제");
			DGShowItem (dialogID, itmIdx);
			DEL_CELLS_FROM_END_AT_SIDE = itmIdx;
			// 측면 끝 부분
			for (xx = placingZone.nCellsFromEndAtSide-1 ; xx >= 0 ; --xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellsFromEndAtRSide [0][xx].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellsFromEndAtRSide [0][xx].objType == EUROFORM) {
					txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtRSide [0][xx].dirLen * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				if (xx == (placingZone.nCellsFromEndAtSide-1)) END_INDEX_FROM_END_AT_SIDE = itmIdx;
				btnPosX += 50;
			}
			// 측면 끝 부분 여백
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 50, btnSizeX, btnSizeY);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 55, 45, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 74, 45, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtSide);
			DGShowItem (dialogID, itmIdx);
			DGDisableItem (dialogID, itmIdx);
			MARGIN_FROM_END_AT_SIDE = itmIdx;

			// 측면 끝 부분 여백 채움 여부 - bFillMarginEndAtSide
			// 라디오 버튼: 여백 (채움)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 110, 70, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 채움");
			DGShowItem (dialogID, itmIdx);
			MARGIN_FILL_FROM_END_AT_SIDE = itmIdx;
			// 라디오 버튼: 여백 (비움)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 135, 70, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 비움");
			DGShowItem (dialogID, itmIdx);
			MARGIN_EMPTY_FROM_END_AT_SIDE = itmIdx;
			DGSetItemValLong (dialogID, itmIdx, TRUE);

			// 하부 시작 부분 여백 채움 여부 - bFillMarginBeginAtBottom
			// 라디오 버튼: 여백 (채움)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 270, 70, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 채움");
			DGShowItem (dialogID, itmIdx);
			MARGIN_FILL_FROM_BEGIN_AT_BOTTOM = itmIdx;
			// 라디오 버튼: 여백 (비움)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 295, 70, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 비움");
			DGShowItem (dialogID, itmIdx);
			MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM = itmIdx;
			DGSetItemValLong (dialogID, itmIdx, TRUE);

			// 하부 시작 부분 여백
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 210, btnSizeX, btnSizeY);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 215, 45, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 234, 45, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtBottom);
			DGShowItem (dialogID, itmIdx);
			DGDisableItem (dialogID, itmIdx);
			MARGIN_FROM_BEGIN_AT_BOTTOM = itmIdx;
			btnPosX = 150;
			btnPosY = 210;
			// 하부 시작 부분
			for (xx = 0 ; xx < placingZone.nCellsFromBeginAtBottom ; ++xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellsFromBeginAtBottom [0][xx].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellsFromBeginAtBottom [0][xx].objType == EUROFORM) {
					txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtBottom [0][xx].dirLen * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				if (xx == 0) START_INDEX_FROM_BEGIN_AT_BOTTOM = itmIdx;
				btnPosX += 50;
			}
			// 화살표 추가
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "↑");
			DGShowItem (dialogID, itmIdx);
			// 추가/삭제 버튼
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "추가");
			DGShowItem (dialogID, itmIdx);
			ADD_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "삭제");
			DGShowItem (dialogID, itmIdx);
			DEL_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			// 하부 중앙 부분
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			txtButton = "";
			if (placingZone.cellCenterAtBottom [0].objType == NONE) {
				txtButton = "NONE";
			} else if (placingZone.cellCenterAtBottom [0].objType == EUROFORM) {
				txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
			}
			DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			DGShowItem (dialogID, itmIdx);
			START_INDEX_CENTER_AT_BOTTOM = itmIdx;
			btnPosX += 50;
			// 화살표 추가
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "↑");
			DGShowItem (dialogID, itmIdx);
			// 추가/삭제 버튼
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "추가");
			DGShowItem (dialogID, itmIdx);
			ADD_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "삭제");
			DGShowItem (dialogID, itmIdx);
			DEL_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			// 하부 끝 부분
			for (xx = placingZone.nCellsFromEndAtBottom-1 ; xx >= 0 ; --xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellsFromEndAtBottom [0][xx].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellsFromEndAtBottom [0][xx].objType == EUROFORM) {
					txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtBottom [0][xx].dirLen * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				if (xx == (placingZone.nCellsFromEndAtBottom-1)) END_INDEX_FROM_END_AT_BOTTOM = itmIdx;
				btnPosX += 50;
			}
			// 하부 끝 부분 여백
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 210, btnSizeX, btnSizeY);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 215, 45, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 234, 45, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtBottom);
			DGShowItem (dialogID, itmIdx);
			DGDisableItem (dialogID, itmIdx);
			MARGIN_FROM_END_AT_BOTTOM = itmIdx;

			// 하부 끝 부분 여백 채움 여부 - bFillMarginEndAtBottom
			// 라디오 버튼: 여백 (채움)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 270, 70, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 채움");
			DGShowItem (dialogID, itmIdx);
			MARGIN_FILL_FROM_END_AT_BOTTOM = itmIdx;
			// 라디오 버튼: 여백 (비움)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 295, 70, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백 비움");
			DGShowItem (dialogID, itmIdx);
			MARGIN_EMPTY_FROM_END_AT_BOTTOM = itmIdx;
			DGSetItemValLong (dialogID, itmIdx, TRUE);

			// 간섭 보가 붙는 곳 영역 길이 (측면)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 150 + (btnSizeX * placingZone.nCellsFromBeginAtSide), 24, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, itmIdx);
			DGDisableItem (dialogID, itmIdx);
			EDITCONTROL_CENTER_LENGTH_SIDE = itmIdx;

			// 메인 창 크기를 변경
			dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
			dialogSizeY = 490;
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;

		case DG_MSG_CLICK:

			// 업데이트 버튼
			if (item == DG_UPDATE_BUTTON) {
				item = 0;

				// 저장된 측면 시작 여백 여부 저장
				if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
					placingZone.bFillMarginBeginAtSide = true;
				else
					placingZone.bFillMarginBeginAtSide = false;

				// 저장된 측면 끝 여백 여부 저장
				if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
					placingZone.bFillMarginEndAtSide = true;
				else
					placingZone.bFillMarginEndAtSide = false;

				// 저장된 하부 시작 여백 여부 저장
				if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
					placingZone.bFillMarginBeginAtBottom = true;
				else
					placingZone.bFillMarginBeginAtBottom = false;

				// 저장된 하부 끝 여백 여부 저장
				if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
					placingZone.bFillMarginEndAtBottom = true;
				else
					placingZone.bFillMarginEndAtBottom = false;

				// 간섭 보가 붙는 곳 영역 길이 저장
				placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				alignPlacingZoneForBeam (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 측면 시작 부분 여백 채움 여부 - bFillMarginBeginAtSide
				// 라디오 버튼: 여백 (채움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_FILL_FROM_BEGIN_AT_SIDE = itmIdx;
				// 라디오 버튼: 여백 (비움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 135, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 비움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_EMPTY_FROM_BEGIN_AT_SIDE = itmIdx;
				DGSetItemValLong (dialogID, itmIdx, TRUE);

				// 저장된 측면 시작 여백 여부 로드
				if (placingZone.bFillMarginBeginAtSide == true) {
					DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE, TRUE);
					DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_SIDE, FALSE);
				} else {
					DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE, FALSE);
					DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_SIDE, TRUE);
				}

				// 측면 시작 부분 여백
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 50, btnSizeX, btnSizeY);
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 55, 45, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 74, 45, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtSide);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				MARGIN_FROM_BEGIN_AT_SIDE = itmIdx;
				btnPosX = 150;
				btnPosY = 50;
				// 측면 시작 부분
				for (xx = 0 ; xx < placingZone.nCellsFromBeginAtSide ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsFromBeginAtRSide [0][xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsFromBeginAtRSide [0][xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtRSide [0][xx].dirLen * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					if (xx == 0) START_INDEX_FROM_BEGIN_AT_SIDE = itmIdx;
					btnPosX += 50;
				}
				// 화살표 추가
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "↑");
				DGShowItem (dialogID, itmIdx);
				// 추가/삭제 버튼
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "추가");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "삭제");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
				// 측면 중앙 부분
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellCenterAtRSide [0].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellCenterAtRSide [0].objType == EUROFORM) {
					txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				START_INDEX_CENTER_AT_SIDE = itmIdx;
				btnPosX += 50;
				// 화살표 추가
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "↑");
				DGShowItem (dialogID, itmIdx);
				// 추가/삭제 버튼
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "추가");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS_FROM_END_AT_SIDE = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "삭제");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS_FROM_END_AT_SIDE = itmIdx;
				// 측면 끝 부분
				for (xx = placingZone.nCellsFromEndAtSide-1 ; xx >= 0 ; --xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsFromEndAtRSide [0][xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsFromEndAtRSide [0][xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtRSide [0][xx].dirLen * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					if (xx == (placingZone.nCellsFromEndAtSide-1)) END_INDEX_FROM_END_AT_SIDE = itmIdx;
					btnPosX += 50;
				}
				// 측면 끝 부분 여백
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 50, btnSizeX, btnSizeY);
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 55, 45, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 74, 45, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtSide);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				MARGIN_FROM_END_AT_SIDE = itmIdx;

				// 측면 끝 부분 여백 채움 여부 - bFillMarginEndAtSide
				// 라디오 버튼: 여백 (채움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 110, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_FILL_FROM_END_AT_SIDE = itmIdx;
				// 라디오 버튼: 여백 (비움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 135, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 비움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_EMPTY_FROM_END_AT_SIDE = itmIdx;
				DGSetItemValLong (dialogID, itmIdx, TRUE);

				// 저장된 측면 끝 여백 여부 로드
				if (placingZone.bFillMarginEndAtSide == true) {
					DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE, TRUE);
					DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_SIDE, FALSE);
				} else {
					DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE, FALSE);
					DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_SIDE, TRUE);
				}

				// 하부 시작 부분 여백 채움 여부 - bFillMarginBeginAtBottom
				// 라디오 버튼: 여백 (채움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 270, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_FILL_FROM_BEGIN_AT_BOTTOM = itmIdx;
				// 라디오 버튼: 여백 (비움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 295, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 비움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM = itmIdx;
				DGSetItemValLong (dialogID, itmIdx, TRUE);

				// 저장된 하부 시작 여백 여부 로드
				if (placingZone.bFillMarginBeginAtBottom == true) {
					DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM, TRUE);
					DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM, FALSE);
				} else {
					DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM, FALSE);
					DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM, TRUE);
				}

				// 하부 시작 부분 여백
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 210, btnSizeX, btnSizeY);
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 215, 45, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 234, 45, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtBottom);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				MARGIN_FROM_BEGIN_AT_BOTTOM = itmIdx;
				btnPosX = 150;
				btnPosY = 210;
				// 하부 시작 부분
				for (xx = 0 ; xx < placingZone.nCellsFromBeginAtBottom ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsFromBeginAtBottom [0][xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsFromBeginAtBottom [0][xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtBottom [0][xx].dirLen * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					if (xx == 0) START_INDEX_FROM_BEGIN_AT_BOTTOM = itmIdx;
					btnPosX += 50;
				}
				// 화살표 추가
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "↑");
				DGShowItem (dialogID, itmIdx);
				// 추가/삭제 버튼
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "추가");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "삭제");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
				// 하부 중앙 부분
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellCenterAtBottom [0].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellCenterAtBottom [0].objType == EUROFORM) {
					txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				START_INDEX_CENTER_AT_BOTTOM = itmIdx;
				btnPosX += 50;
				// 화살표 추가
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "↑");
				DGShowItem (dialogID, itmIdx);
				// 추가/삭제 버튼
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "추가");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS_FROM_END_AT_BOTTOM = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "삭제");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS_FROM_END_AT_BOTTOM = itmIdx;
				// 하부 끝 부분
				for (xx = placingZone.nCellsFromEndAtBottom-1 ; xx >= 0 ; --xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsFromEndAtBottom [0][xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsFromEndAtBottom [0][xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtBottom [0][xx].dirLen * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					if (xx == (placingZone.nCellsFromEndAtBottom-1)) END_INDEX_FROM_END_AT_BOTTOM = itmIdx;
					btnPosX += 50;
				}
				// 하부 끝 부분 여백
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 210, btnSizeX, btnSizeY);
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 215, 45, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 234, 45, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtBottom);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				MARGIN_FROM_END_AT_BOTTOM = itmIdx;

				// 하부 끝 부분 여백 채움 여부 - bFillMarginEndAtBottom
				// 라디오 버튼: 여백 (채움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 270, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_FILL_FROM_END_AT_BOTTOM = itmIdx;
				// 라디오 버튼: 여백 (비움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 295, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 비움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_EMPTY_FROM_END_AT_BOTTOM = itmIdx;
				DGSetItemValLong (dialogID, itmIdx, TRUE);

				// 저장된 하부 끝 여백 여부 로드
				if (placingZone.bFillMarginEndAtBottom == true) {
					DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM, TRUE);
					DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_BOTTOM, FALSE);
				} else {
					DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM, FALSE);
					DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_BOTTOM, TRUE);
				}

				// 간섭 보가 붙는 곳 영역 길이 (측면)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 150 + (btnSizeX * placingZone.nCellsFromBeginAtSide), 24, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				if (placingZone.cellCenterAtRSide [0].objType == NONE)
					DGEnableItem (dialogID, itmIdx);
				else
					DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_CENTER_LENGTH_SIDE = itmIdx;

				// 간섭 보가 붙는 곳 영역 길이 로드
				DGSetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE, placingZone.centerLengthAtSide);

				// 메인 창 크기를 변경
				dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
				dialogSizeY = 490;
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// 이전 버튼
			if (item == DG_PREV) {
				clickedPrevButton = true;
			}

			// 확인 버튼
			if (item == DG_OK) {
				clickedOKButton = true;

				// 여백 채움/비움 여부 저장
				if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
					placingZone.bFillMarginBeginAtSide = true;
				if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
					placingZone.bFillMarginEndAtSide = true;
				if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
					placingZone.bFillMarginBeginAtBottom = true;
				if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
					placingZone.bFillMarginEndAtBottom = true;

				placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				alignPlacingZoneForBeam (&placingZone);
			}

			// 취소 버튼
			if (item == DG_CANCEL) {
			}

			// 셀 추가/삭제 버튼 8종
			if (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) {
				addNewColFromBeginAtSide (&placingZone);
			}
			if (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) {
				delLastColFromBeginAtSide (&placingZone);
			}
			if (item == ADD_CELLS_FROM_END_AT_SIDE) {
				addNewColFromEndAtSide (&placingZone);
			}
			if (item == DEL_CELLS_FROM_END_AT_SIDE) {
				delLastColFromEndAtSide (&placingZone);
			}
			if (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) {
				addNewColFromBeginAtBottom (&placingZone);
			}
			if (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) {
				delLastColFromBeginAtBottom (&placingZone);
			}
			if (item == ADD_CELLS_FROM_END_AT_BOTTOM) {
				addNewColFromEndAtBottom (&placingZone);
			}
			if (item == DEL_CELLS_FROM_END_AT_BOTTOM) {
				delLastColFromEndAtBottom (&placingZone);
			}

			if ( (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) || (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) || (item == ADD_CELLS_FROM_END_AT_SIDE) || (item == DEL_CELLS_FROM_END_AT_SIDE) ||
				 (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == ADD_CELLS_FROM_END_AT_BOTTOM) || (item == DEL_CELLS_FROM_END_AT_BOTTOM)) {

				item = 0;

				// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
				alignPlacingZoneForBeam (&placingZone);

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 측면 시작 부분 여백 채움 여부 - bFillMarginBeginAtSide
				// 라디오 버튼: 여백 (채움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_FILL_FROM_BEGIN_AT_SIDE = itmIdx;
				// 라디오 버튼: 여백 (비움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 135, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 비움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_EMPTY_FROM_BEGIN_AT_SIDE = itmIdx;
				DGSetItemValLong (dialogID, itmIdx, TRUE);

				// 측면 시작 부분 여백
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 50, btnSizeX, btnSizeY);
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 55, 45, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 74, 45, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtSide);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				MARGIN_FROM_BEGIN_AT_SIDE = itmIdx;
				btnPosX = 150;
				btnPosY = 50;
				// 측면 시작 부분
				for (xx = 0 ; xx < placingZone.nCellsFromBeginAtSide ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsFromBeginAtRSide [0][xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsFromBeginAtRSide [0][xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtRSide [0][xx].dirLen * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					if (xx == 0) START_INDEX_FROM_BEGIN_AT_SIDE = itmIdx;
					btnPosX += 50;
				}
				// 화살표 추가
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "↑");
				DGShowItem (dialogID, itmIdx);
				// 추가/삭제 버튼
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "추가");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "삭제");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
				// 측면 중앙 부분
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellCenterAtRSide [0].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellCenterAtRSide [0].objType == EUROFORM) {
					txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				START_INDEX_CENTER_AT_SIDE = itmIdx;
				btnPosX += 50;
				// 화살표 추가
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "↑");
				DGShowItem (dialogID, itmIdx);
				// 추가/삭제 버튼
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "추가");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS_FROM_END_AT_SIDE = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "삭제");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS_FROM_END_AT_SIDE = itmIdx;
				// 측면 끝 부분
				for (xx = placingZone.nCellsFromEndAtSide-1 ; xx >= 0 ; --xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsFromEndAtRSide [0][xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsFromEndAtRSide [0][xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtRSide [0][xx].dirLen * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					if (xx == (placingZone.nCellsFromEndAtSide-1)) END_INDEX_FROM_END_AT_SIDE = itmIdx;
					btnPosX += 50;
				}
				// 측면 끝 부분 여백
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 50, btnSizeX, btnSizeY);
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 55, 45, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 74, 45, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtSide);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				MARGIN_FROM_END_AT_SIDE = itmIdx;

				// 측면 끝 부분 여백 채움 여부 - bFillMarginEndAtSide
				// 라디오 버튼: 여백 (채움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 110, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_FILL_FROM_END_AT_SIDE = itmIdx;
				// 라디오 버튼: 여백 (비움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 135, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 비움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_EMPTY_FROM_END_AT_SIDE = itmIdx;
				DGSetItemValLong (dialogID, itmIdx, TRUE);

				// 하부 시작 부분 여백 채움 여부 - bFillMarginBeginAtBottom
				// 라디오 버튼: 여백 (채움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 270, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_FILL_FROM_BEGIN_AT_BOTTOM = itmIdx;
				// 라디오 버튼: 여백 (비움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 295, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 비움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM = itmIdx;
				DGSetItemValLong (dialogID, itmIdx, TRUE);

				// 하부 시작 부분 여백
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 210, btnSizeX, btnSizeY);
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 215, 45, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 234, 45, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtBottom);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				MARGIN_FROM_BEGIN_AT_BOTTOM = itmIdx;
				btnPosX = 150;
				btnPosY = 210;
				// 하부 시작 부분
				for (xx = 0 ; xx < placingZone.nCellsFromBeginAtBottom ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsFromBeginAtBottom [0][xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsFromBeginAtBottom [0][xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtBottom [0][xx].dirLen * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					if (xx == 0) START_INDEX_FROM_BEGIN_AT_BOTTOM = itmIdx;
					btnPosX += 50;
				}
				// 화살표 추가
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "↑");
				DGShowItem (dialogID, itmIdx);
				// 추가/삭제 버튼
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "추가");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "삭제");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
				// 하부 중앙 부분
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellCenterAtBottom [0].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellCenterAtBottom [0].objType == EUROFORM) {
					txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
				DGShowItem (dialogID, itmIdx);
				START_INDEX_CENTER_AT_BOTTOM = itmIdx;
				btnPosX += 50;
				// 화살표 추가
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "↑");
				DGShowItem (dialogID, itmIdx);
				// 추가/삭제 버튼
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "추가");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS_FROM_END_AT_BOTTOM = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "삭제");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS_FROM_END_AT_BOTTOM = itmIdx;
				// 하부 끝 부분
				for (xx = placingZone.nCellsFromEndAtBottom-1 ; xx >= 0 ; --xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsFromEndAtBottom [0][xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsFromEndAtBottom [0][xx].objType == EUROFORM) {
						txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtBottom [0][xx].dirLen * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, itmIdx);
					if (xx == (placingZone.nCellsFromEndAtBottom-1)) END_INDEX_FROM_END_AT_BOTTOM = itmIdx;
					btnPosX += 50;
				}
				// 하부 끝 부분 여백
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 210, btnSizeX, btnSizeY);
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 215, 45, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 234, 45, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtBottom);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				MARGIN_FROM_END_AT_BOTTOM = itmIdx;

				// 하부 끝 부분 여백 채움 여부 - bFillMarginEndAtBottom
				// 라디오 버튼: 여백 (채움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 270, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 채움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_FILL_FROM_END_AT_BOTTOM = itmIdx;
				// 라디오 버튼: 여백 (비움)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 295, 70, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백 비움");
				DGShowItem (dialogID, itmIdx);
				MARGIN_EMPTY_FROM_END_AT_BOTTOM = itmIdx;
				DGSetItemValLong (dialogID, itmIdx, TRUE);

				// 간섭 보가 붙는 곳 영역 길이 (측면)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 150 + (btnSizeX * placingZone.nCellsFromBeginAtSide), 24, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				if (placingZone.cellCenterAtRSide [0].objType == NONE)
					DGEnableItem (dialogID, itmIdx);
				else
					DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_CENTER_LENGTH_SIDE = itmIdx;

				// 메인 창 크기를 변경
				dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
				dialogSizeY = 490;
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// 배치 버튼 (측면 시작 부분)
			if ((item >= START_INDEX_FROM_BEGIN_AT_SIDE) && (item < START_INDEX_FROM_BEGIN_AT_SIDE + placingZone.nCellsFromBeginAtSide)) {
				// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
				clickedBtnItemIdx = item;
				result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
				item = 0;
			}
			// 배치 버튼 (측면 중앙 부분)
			if (item == START_INDEX_CENTER_AT_SIDE) {
				// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
				clickedBtnItemIdx = item;
				result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
				item = 0;
			}
			// 배치 버튼 (측면 끝 부분)
			if ((item >= END_INDEX_FROM_END_AT_SIDE) && (item < END_INDEX_FROM_END_AT_SIDE + placingZone.nCellsFromEndAtSide)) {
				// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
				clickedBtnItemIdx = item;
				result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
				item = 0;
			}

			// 배치 버튼 (하부 시작 부분)
			if ((item >= START_INDEX_FROM_BEGIN_AT_BOTTOM) && (item < START_INDEX_FROM_BEGIN_AT_BOTTOM + placingZone.nCellsFromBeginAtBottom)) {
				// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
				clickedBtnItemIdx = item;
				result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
				item = 0;
			}
			// 배치 버튼 (하부 중앙 부분)
			if (item == START_INDEX_CENTER_AT_BOTTOM) {
				// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
				clickedBtnItemIdx = item;
				result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
				item = 0;
			}
			// 배치 버튼 (하부 끝 부분)
			if ((item >= END_INDEX_FROM_END_AT_BOTTOM) && (item < END_INDEX_FROM_END_AT_BOTTOM + placingZone.nCellsFromEndAtBottom)) {
				// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
				clickedBtnItemIdx = item;
				result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
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

// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그
short DGCALLBACK beamPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	xx;
	short	result;
	short	idx;
	short	iCellType;
	short	popupSelectedIdx = 0;

	switch (message) {
		case DG_MSG_INIT:

			iCellType = 0;

			// 배치 버튼 (측면 시작 부분)
			if ((clickedBtnItemIdx >= START_INDEX_FROM_BEGIN_AT_SIDE) && (clickedBtnItemIdx < START_INDEX_FROM_BEGIN_AT_SIDE + placingZone.nCellsFromBeginAtSide)) {
				idx = clickedBtnItemIdx - START_INDEX_FROM_BEGIN_AT_SIDE;
				iCellType = FROM_BEGIN_AT_SIDE;
			}
			// 배치 버튼 (측면 중앙 부분)
			if (clickedBtnItemIdx == START_INDEX_CENTER_AT_SIDE) {
				idx = -1;
				iCellType = CENTER_AT_SIDE;
			}
			// 배치 버튼 (측면 끝 부분)
			if ((clickedBtnItemIdx >= END_INDEX_FROM_END_AT_SIDE) && (clickedBtnItemIdx < END_INDEX_FROM_END_AT_SIDE + placingZone.nCellsFromEndAtSide)) {
				idx = (placingZone.nCellsFromEndAtSide - 1) - (clickedBtnItemIdx - END_INDEX_FROM_END_AT_SIDE);
				iCellType = FROM_END_AT_SIDE;
			}

			// 배치 버튼 (하부 시작 부분)
			if ((clickedBtnItemIdx >= START_INDEX_FROM_BEGIN_AT_BOTTOM) && (clickedBtnItemIdx < START_INDEX_FROM_BEGIN_AT_BOTTOM + placingZone.nCellsFromBeginAtBottom)) {
				idx = clickedBtnItemIdx - START_INDEX_FROM_BEGIN_AT_BOTTOM;
				iCellType = FROM_BEGIN_AT_BOTTOM;
			}
			// 배치 버튼 (하부 중앙 부분)
			if (clickedBtnItemIdx == START_INDEX_CENTER_AT_BOTTOM) {
				idx = -1;
				iCellType = CENTER_AT_BOTTOM;
			}
			// 배치 버튼 (하부 끝 부분)
			if ((clickedBtnItemIdx >= END_INDEX_FROM_END_AT_BOTTOM) && (clickedBtnItemIdx < END_INDEX_FROM_END_AT_BOTTOM + placingZone.nCellsFromEndAtBottom)) {
				idx = (placingZone.nCellsFromEndAtBottom - 1) - (clickedBtnItemIdx - END_INDEX_FROM_END_AT_BOTTOM);
				iCellType = FROM_END_AT_BOTTOM;
			}
			
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
			if (iCellType == FROM_BEGIN_AT_SIDE) {
				if (placingZone.cellsFromBeginAtRSide [0][idx].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

					// 체크박스: 규격폼
					DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_stan_onoff);

					// 유로폼의 높이
					if (placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_stan_onoff == true) {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, POPUP_LENGTH);

						if (abs (placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs (placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs (placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
					} else {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);

						DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_hei2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_LENGTH, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_LENGTH, 1.500);
					}
				}
			}
			if (iCellType == CENTER_AT_SIDE) {
				if (placingZone.cellCenterAtRSide [0].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

					// 체크박스: 규격폼
					DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cellCenterAtRSide [0].libPart.form.eu_stan_onoff);

					// 유로폼의 높이
					if (placingZone.cellCenterAtRSide [0].libPart.form.eu_stan_onoff == true) {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, POPUP_LENGTH);

						if (abs (placingZone.cellCenterAtRSide [0].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs (placingZone.cellCenterAtRSide [0].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs (placingZone.cellCenterAtRSide [0].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
					} else {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);

						DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellCenterAtRSide [0].libPart.form.eu_hei2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_LENGTH, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_LENGTH, 1.500);
					}
				}
			}
			if (iCellType == FROM_END_AT_SIDE) {
				if (placingZone.cellsFromEndAtRSide [0][idx].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

					// 체크박스: 규격폼
					DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_stan_onoff);

					// 유로폼의 높이
					if (placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_stan_onoff == true) {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, POPUP_LENGTH);

						if (abs (placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs (placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs (placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
					} else {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);

						DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_hei2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_LENGTH, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_LENGTH, 1.500);
					}
				}
			}
			if (iCellType == FROM_BEGIN_AT_BOTTOM) {
				if (placingZone.cellsFromBeginAtBottom [0][idx].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

					// 체크박스: 규격폼
					DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_stan_onoff);

					// 유로폼의 높이
					if (placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_stan_onoff == true) {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, POPUP_LENGTH);

						if (abs (placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs (placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs (placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
					} else {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);

						DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_hei2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_LENGTH, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_LENGTH, 1.500);
					}
				}
			}
			if (iCellType == CENTER_AT_BOTTOM) {
				if (placingZone.cellCenterAtBottom [0].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

					// 체크박스: 규격폼
					DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cellCenterAtBottom [0].libPart.form.eu_stan_onoff);

					// 유로폼의 높이
					if (placingZone.cellCenterAtBottom [0].libPart.form.eu_stan_onoff == true) {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, POPUP_LENGTH);

						if (abs (placingZone.cellCenterAtBottom [0].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs (placingZone.cellCenterAtBottom [0].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs (placingZone.cellCenterAtBottom [0].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
					} else {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);

						DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellCenterAtBottom [0].libPart.form.eu_hei2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_LENGTH, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_LENGTH, 1.500);
					}
				}
			}
			if (iCellType == FROM_END_AT_BOTTOM) {
				if (placingZone.cellsFromEndAtBottom [0][idx].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

					// 체크박스: 규격폼
					DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_stan_onoff);

					// 유로폼의 높이
					if (placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_stan_onoff == true) {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, POPUP_LENGTH);

						if (abs (placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs (placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs (placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
					} else {
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);

						DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_hei2);
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
					iCellType = 0;
					idx = -1;

					// 배치 버튼 (측면 시작 부분)
					if ((clickedBtnItemIdx >= START_INDEX_FROM_BEGIN_AT_SIDE) && (clickedBtnItemIdx < START_INDEX_FROM_BEGIN_AT_SIDE + placingZone.nCellsFromBeginAtSide)) {
						idx = clickedBtnItemIdx - START_INDEX_FROM_BEGIN_AT_SIDE;
						iCellType = FROM_BEGIN_AT_SIDE;
					}
					// 배치 버튼 (측면 중앙 부분)
					if (clickedBtnItemIdx == START_INDEX_CENTER_AT_SIDE) {
						idx = -1;
						iCellType = CENTER_AT_SIDE;
					}
					// 배치 버튼 (측면 끝 부분)
					if ((clickedBtnItemIdx >= END_INDEX_FROM_END_AT_SIDE) && (clickedBtnItemIdx < END_INDEX_FROM_END_AT_SIDE + placingZone.nCellsFromEndAtSide)) {
						idx = (placingZone.nCellsFromEndAtSide - 1) - (clickedBtnItemIdx - END_INDEX_FROM_END_AT_SIDE);
						iCellType = FROM_END_AT_SIDE;
					}

					// 배치 버튼 (하부 시작 부분)
					if ((clickedBtnItemIdx >= START_INDEX_FROM_BEGIN_AT_BOTTOM) && (clickedBtnItemIdx < START_INDEX_FROM_BEGIN_AT_BOTTOM + placingZone.nCellsFromBeginAtBottom)) {
						idx = clickedBtnItemIdx - START_INDEX_FROM_BEGIN_AT_BOTTOM;
						iCellType = FROM_BEGIN_AT_BOTTOM;
					}
					// 배치 버튼 (하부 중앙 부분)
					if (clickedBtnItemIdx == START_INDEX_CENTER_AT_BOTTOM) {
						idx = -1;
						iCellType = CENTER_AT_BOTTOM;
					}
					// 배치 버튼 (하부 끝 부분)
					if ((clickedBtnItemIdx >= END_INDEX_FROM_END_AT_BOTTOM) && (clickedBtnItemIdx < END_INDEX_FROM_END_AT_BOTTOM + placingZone.nCellsFromEndAtBottom)) {
						idx = (placingZone.nCellsFromEndAtBottom - 1) - (clickedBtnItemIdx - END_INDEX_FROM_END_AT_BOTTOM);
						iCellType = FROM_END_AT_BOTTOM;
					}

					// 입력한 길이를 해당 셀의 모든 객체들에게 적용함
					if (iCellType == FROM_BEGIN_AT_SIDE) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
							for (xx = 0 ; xx < 4 ; ++xx) {
								placingZone.cellsFromBeginAtLSide [xx][idx].objType = NONE;
								placingZone.cellsFromBeginAtRSide [xx][idx].objType = NONE;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

							// 규격폼으로 저장할 경우
							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
								placingZone.cellsFromBeginAtLSide [0][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtLSide [0][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtLSide [0][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromBeginAtLSide [0][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromBeginAtRSide [0][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtRSide [0][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromBeginAtLSide [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromBeginAtLSide [1][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtLSide [1][idx].libPart.fillersp.f_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromBeginAtRSide [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromBeginAtRSide [1][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtRSide [1][idx].libPart.fillersp.f_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromBeginAtLSide [2][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtLSide [2][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtLSide [2][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromBeginAtLSide [2][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromBeginAtRSide [2][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtRSide [2][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtRSide [2][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromBeginAtRSide [2][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
							
								placingZone.cellsFromBeginAtLSide [3][idx].objType = WOOD;
								placingZone.cellsFromBeginAtLSide [3][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtLSide [3][idx].libPart.wood.w_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromBeginAtRSide [3][idx].objType = WOOD;
								placingZone.cellsFromBeginAtRSide [3][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtRSide [3][idx].libPart.wood.w_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

							// 비규격폼으로 저장할 경우
							} else {
								placingZone.cellsFromBeginAtLSide [0][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtLSide [0][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtLSide [0][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromBeginAtLSide [0][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromBeginAtRSide [0][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtRSide [0][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromBeginAtRSide [0][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromBeginAtLSide [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromBeginAtLSide [1][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtLSide [1][idx].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromBeginAtRSide [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromBeginAtRSide [1][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtRSide [1][idx].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromBeginAtLSide [2][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtLSide [2][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtLSide [2][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromBeginAtLSide [2][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromBeginAtRSide [2][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtRSide [2][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtRSide [2][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromBeginAtRSide [2][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromBeginAtLSide [3][idx].objType = WOOD;
								placingZone.cellsFromBeginAtLSide [3][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtLSide [3][idx].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromBeginAtRSide [3][idx].objType = WOOD;
								placingZone.cellsFromBeginAtRSide [3][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtRSide [3][idx].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
							}
						}
					}
					if (iCellType == CENTER_AT_SIDE) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
							for (xx = 0 ; xx < 4 ; ++xx) {
								placingZone.cellCenterAtLSide [xx].objType = NONE;
								placingZone.cellCenterAtRSide [xx].objType = NONE;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

							// 규격폼으로 저장할 경우
							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
								placingZone.cellCenterAtLSide [0].objType = EUROFORM;
								placingZone.cellCenterAtLSide [0].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtLSide [0].libPart.form.eu_stan_onoff = true;
								placingZone.cellCenterAtLSide [0].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellCenterAtRSide [0].objType = EUROFORM;
								placingZone.cellCenterAtRSide [0].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtRSide [0].libPart.form.eu_stan_onoff = true;
								placingZone.cellCenterAtRSide [0].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellCenterAtLSide [1].objType = FILLERSPACER;
								placingZone.cellCenterAtLSide [1].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtLSide [1].libPart.fillersp.f_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellCenterAtRSide [1].objType = FILLERSPACER;
								placingZone.cellCenterAtRSide [1].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtRSide [1].libPart.fillersp.f_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellCenterAtLSide [2].objType = EUROFORM;
								placingZone.cellCenterAtLSide [2].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtLSide [2].libPart.form.eu_stan_onoff = true;
								placingZone.cellCenterAtLSide [2].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellCenterAtRSide [2].objType = EUROFORM;
								placingZone.cellCenterAtRSide [2].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtRSide [2].libPart.form.eu_stan_onoff = true;
								placingZone.cellCenterAtRSide [2].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellCenterAtLSide [3].objType = WOOD;
								placingZone.cellCenterAtLSide [3].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtLSide [3].libPart.wood.w_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellCenterAtRSide [3].objType = WOOD;
								placingZone.cellCenterAtRSide [3].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtRSide [3].libPart.wood.w_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
							
							// 비규격폼으로 저장할 경우
							} else {
								placingZone.cellCenterAtLSide [0].objType = EUROFORM;
								placingZone.cellCenterAtLSide [0].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtLSide [0].libPart.form.eu_stan_onoff = false;
								placingZone.cellCenterAtLSide [0].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellCenterAtRSide [0].objType = EUROFORM;
								placingZone.cellCenterAtRSide [0].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtRSide [0].libPart.form.eu_stan_onoff = false;
								placingZone.cellCenterAtRSide [0].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellCenterAtLSide [1].objType = FILLERSPACER;
								placingZone.cellCenterAtLSide [1].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtLSide [1].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellCenterAtRSide [1].objType = FILLERSPACER;
								placingZone.cellCenterAtRSide [1].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtRSide [1].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellCenterAtLSide [2].objType = EUROFORM;
								placingZone.cellCenterAtLSide [2].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtLSide [2].libPart.form.eu_stan_onoff = false;
								placingZone.cellCenterAtLSide [2].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellCenterAtRSide [2].objType = EUROFORM;
								placingZone.cellCenterAtRSide [2].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtRSide [2].libPart.form.eu_stan_onoff = false;
								placingZone.cellCenterAtRSide [2].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellCenterAtLSide [3].objType = WOOD;
								placingZone.cellCenterAtLSide [3].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtLSide [3].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellCenterAtRSide [3].objType = WOOD;
								placingZone.cellCenterAtRSide [3].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtRSide [3].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
							}
						}
					}
					if (iCellType == FROM_END_AT_SIDE) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
							for (xx = 0 ; xx < 4 ; ++xx) {
								placingZone.cellsFromEndAtLSide [xx][idx].objType = NONE;
								placingZone.cellsFromEndAtRSide [xx][idx].objType = NONE;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

							// 규격폼으로 저장할 경우
							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
								placingZone.cellsFromEndAtLSide [0][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtLSide [0][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtLSide [0][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromEndAtLSide [0][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromEndAtRSide [0][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtRSide [0][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromEndAtLSide [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromEndAtLSide [1][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtLSide [1][idx].libPart.fillersp.f_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromEndAtRSide [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromEndAtRSide [1][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtRSide [1][idx].libPart.fillersp.f_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromEndAtLSide [2][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtLSide [2][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtLSide [2][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromEndAtLSide [2][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromEndAtRSide [2][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtRSide [2][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtRSide [2][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromEndAtRSide [2][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
							
								placingZone.cellsFromEndAtLSide [3][idx].objType = WOOD;
								placingZone.cellsFromEndAtLSide [3][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtLSide [3][idx].libPart.wood.w_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromEndAtRSide [3][idx].objType = WOOD;
								placingZone.cellsFromEndAtRSide [3][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtRSide [3][idx].libPart.wood.w_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

							// 비규격폼으로 저장할 경우
							} else {
								placingZone.cellsFromEndAtLSide [0][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtLSide [0][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtLSide [0][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromEndAtLSide [0][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromEndAtRSide [0][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtRSide [0][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromEndAtRSide [0][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromEndAtLSide [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromEndAtLSide [1][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtLSide [1][idx].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromEndAtRSide [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromEndAtRSide [1][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtRSide [1][idx].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromEndAtLSide [2][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtLSide [2][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtLSide [2][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromEndAtLSide [2][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromEndAtRSide [2][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtRSide [2][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtRSide [2][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromEndAtRSide [2][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromEndAtLSide [3][idx].objType = WOOD;
								placingZone.cellsFromEndAtLSide [3][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtLSide [3][idx].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromEndAtRSide [3][idx].objType = WOOD;
								placingZone.cellsFromEndAtRSide [3][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtRSide [3][idx].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
							}
						}
					}
					if (iCellType == FROM_BEGIN_AT_BOTTOM) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
							for (xx = 0 ; xx < 3 ; ++xx) {
								placingZone.cellsFromBeginAtBottom [xx][idx].objType = NONE;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

							// 규격폼으로 저장할 경우
							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
								placingZone.cellsFromBeginAtBottom [0][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtBottom [0][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromBeginAtBottom [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromBeginAtBottom [1][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtBottom [1][idx].libPart.fillersp.f_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromBeginAtBottom [2][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtBottom [2][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromBeginAtBottom [2][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromBeginAtBottom [2][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

							// 비규격폼으로 저장할 경우
							} else {
								placingZone.cellsFromBeginAtBottom [0][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtBottom [0][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromBeginAtBottom [0][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromBeginAtBottom [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromBeginAtBottom [1][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtBottom [1][idx].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromBeginAtBottom [2][idx].objType = EUROFORM;
								placingZone.cellsFromBeginAtBottom [2][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromBeginAtBottom [2][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromBeginAtBottom [2][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
							}
						}
					}
					if (iCellType == CENTER_AT_BOTTOM) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
							for (xx = 0 ; xx < 3 ; ++xx) {
								placingZone.cellCenterAtBottom [xx].objType = NONE;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

							// 규격폼으로 저장할 경우
							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
								placingZone.cellCenterAtBottom [0].objType = EUROFORM;
								placingZone.cellCenterAtBottom [0].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtBottom [0].libPart.form.eu_stan_onoff = true;
								placingZone.cellCenterAtBottom [0].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellCenterAtBottom [1].objType = FILLERSPACER;
								placingZone.cellCenterAtBottom [1].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtBottom [1].libPart.fillersp.f_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellCenterAtBottom [2].objType = EUROFORM;
								placingZone.cellCenterAtBottom [2].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellCenterAtBottom [2].libPart.form.eu_stan_onoff = true;
								placingZone.cellCenterAtBottom [2].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

							// 비규격폼으로 저장할 경우
							} else {
								placingZone.cellCenterAtBottom [0].objType = EUROFORM;
								placingZone.cellCenterAtBottom [0].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtBottom [0].libPart.form.eu_stan_onoff = false;
								placingZone.cellCenterAtBottom [0].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellCenterAtBottom [1].objType = FILLERSPACER;
								placingZone.cellCenterAtBottom [1].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtBottom [1].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellCenterAtBottom [2].objType = EUROFORM;
								placingZone.cellCenterAtBottom [2].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellCenterAtBottom [2].libPart.form.eu_stan_onoff = false;
								placingZone.cellCenterAtBottom [2].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
							}
						}
					}
					if (iCellType == FROM_END_AT_BOTTOM) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
							for (xx = 0 ; xx < 3 ; ++xx) {
								placingZone.cellsFromEndAtBottom [xx][idx].objType = NONE;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

							// 규격폼으로 저장할 경우
							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
								placingZone.cellsFromEndAtBottom [0][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtBottom [0][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromEndAtBottom [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromEndAtBottom [1][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtBottom [1][idx].libPart.fillersp.f_leng = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

								placingZone.cellsFromEndAtBottom [2][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtBottom [2][idx].dirLen = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
								placingZone.cellsFromEndAtBottom [2][idx].libPart.form.eu_stan_onoff = true;
								placingZone.cellsFromEndAtBottom [2][idx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;

							// 비규격폼으로 저장할 경우
							} else {
								placingZone.cellsFromEndAtBottom [0][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtBottom [0][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromEndAtBottom [0][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromEndAtBottom [1][idx].objType = FILLERSPACER;
								placingZone.cellsFromEndAtBottom [1][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtBottom [1][idx].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

								placingZone.cellsFromEndAtBottom [2][idx].objType = EUROFORM;
								placingZone.cellsFromEndAtBottom [2][idx].dirLen = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
								placingZone.cellsFromEndAtBottom [2][idx].libPart.form.eu_stan_onoff = false;
								placingZone.cellsFromEndAtBottom [2][idx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);
							}
						}
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
