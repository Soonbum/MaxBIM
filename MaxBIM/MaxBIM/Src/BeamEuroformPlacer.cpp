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
static bool				clickedOKButton;			// OK 버튼을 눌렀습니까?
static short			layerInd_Euroform;			// 레이어 번호: 유로폼
static short			layerInd_Plywood;			// 레이어 번호: 합판
static short			layerInd_Wood;				// 레이어 번호: 목재
static short			layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글

// 3번 메뉴: 보에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnBeam (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx;
	double			dx, dy, ang;
	API_Coord3D		rotatedPoint, unrotatedPoint;
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

	// 모프 객체 정보
	InfoMorphForBeam		infoMorph;

	// 점 입력
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	API_Coord3D				tempPoint, resultPoint;
	API_Coord3D				other_p1, other_p2;

	// 회전각도 0일 때의 보의 시작점, 끝점의 좌표를 저장함
	API_Coord3D				nodes [2];

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
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 보 (1개), 보 측면을 덮는 모프 (1개)", true);
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
		ACAPI_WriteReport ("보 측면을 덮는 모프를 1개 선택하셔야 합니다.", true);
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

	// 메인 보의 중간에 붙어 있는 간섭 보
	if (relData.con != NULL) {
		for (xx = 0; xx < relData.nCon; xx++) {
			BNZeroMemory (&elem, sizeof (API_Element));
			elem.header.guid = (*(relData.con))[xx].guid;
			ACAPI_Element_Get (&elem);
			
			infoOtherBeams [nInterfereBeams].guid		= (*(relData.con))[xx].guid;
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

	// 시작 부분 하단 점 클릭, 끝 부분 상단 점 클릭
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("보의 시작 부분 하단 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("보의 끝 부분 상단 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;

	// 영역 모프 제거
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// 사용자가 클릭한 두 점을 통해 보의 시작점, 끝점을 찾음
	other_p1.x = infoBeam.begC.x;
	other_p1.y = infoBeam.begC.y;
	other_p1.z = infoBeam.level - infoBeam.height;

	other_p2.x = infoBeam.endC.x;
	other_p2.y = infoBeam.endC.y;
	other_p2.z = infoBeam.level - infoBeam.height;

	// 영역 높이 값을 구함
	placingZone.areaHeight = other_p2.z - other_p1.z;
	if (moreCloserPoint (point1, other_p1, other_p2) == 1) {
		placingZone.begC = other_p1;
		placingZone.endC = other_p2;
	} else {
		placingZone.begC = other_p2;
		placingZone.endC = other_p1;
	}

	// 두 점 간의 각도를 구함
	dx = placingZone.endC.x - placingZone.begC.x;
	dy = placingZone.endC.y - placingZone.begC.y;
	ang = RadToDegree (atan2 (dy, dx));

	// 시작점이 왼쪽, 끝점이 오른쪽으로 가도록 회전할 것 (회전각도 0일 때의 좌표 계산)
	tempPoint = placingZone.endC;
	resultPoint.x = point1.x + ((tempPoint.x - point1.x)*cos(DegreeToRad (-ang)) - (tempPoint.y - point1.y)*sin(DegreeToRad (-ang)));
	resultPoint.y = point1.y + ((tempPoint.x - point1.x)*sin(DegreeToRad (-ang)) + (tempPoint.y - point1.y)*cos(DegreeToRad (-ang)));
	resultPoint.z = tempPoint.z;

	nodes [0] = placingZone.begC;
	nodes [1] = resultPoint;

	// 나머지 보 영역 정보를 저장함
	placingZone.ang			= DegreeToRad (ang);
	placingZone.level		= infoBeam.level;
	placingZone.beamLength	= GetDistance (placingZone.begC, placingZone.endC);

	// 간섭 보들의 시작점/끝점 중 메인 보에 가까운 쪽 점을 검색 (간섭 보는 양쪽에 한 쌍만 있다고 가정함)
	p1.x = infoBeam.begC.x;				p1.y = infoBeam.begC.y;
	p2.x = infoBeam.endC.x;				p2.y = infoBeam.endC.y;
	p3.x = infoOtherBeams [0].begC.x;	p3.y = infoOtherBeams [0].begC.y;
	p4.x = infoOtherBeams [0].endC.x;	p4.y = infoOtherBeams [0].endC.y;
	pResult = IntersectionPoint1 (&p1, &p2, &p3, &p4);	// 메인 보와 간섭 보의 교차점 검색

	// 간섭 보 관련 정보 입력
	if (nInterfereBeams > 0) {
		placingZone.bInterfereBeam = true;
		placingZone.posInterfereBeamFromLeft = GetDistance (placingZone.begC.x, placingZone.begC.y, pResult.x, pResult.y);
		placingZone.interfereBeamWidth = infoOtherBeams [0].width;
		placingZone.interfereBeamHeight = infoOtherBeams [0].height;
	} else {
		placingZone.bInterfereBeam = false;
	}

	// 영역 정보의 여백 설정 초기화
	placingZone.marginBeginAtSide = 0.0;
	placingZone.marginEndAtSide = 0.0;
	placingZone.marginBeginAtBottom = 0.0;
	placingZone.marginEndAtBottom = 0.0;

	// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32521, ACAPI_GetOwnResModule (), beamPlacerHandler1, 0);

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

	// 영역 정보의 고도 정보 수정
	// ... placingZone.level = ???;

	// 셀 개수 초기화
	placingZone.nCellsFromBeginAtSide = 0;
	placingZone.nCellsFromEndAtSide = 0;
	placingZone.nCellsFromBeginAtBottom = 0;
	placingZone.nCellsFromEndAtBottom = 0;

	// placingZone의 Cell 정보 초기화
	initCellsForBeam (&placingZone);

	// 배치를 위한 정보 입력
	firstPlacingSettingsForBeam (&placingZone);

	// !!! 테스트
	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtSide ; ++xx) {
		placeLibPartForBeam (placingZone.cellsFromBeginAtLSide [xx]);
		placeLibPartForBeam (placingZone.cellsFromBeginAtRSide [xx]);
	}

	// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정합니다.
	clickedOKButton = false;
	//result = DGBlankModalDialog (185, 250, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomPlacerHandler2, 0);

	// 나머지 영역 채우기 - 합판, 목재
	//err = fillRestAreasForSlabBottom ();

	/*
		측면 배치, 하부 배치 따로 보여주기
			시작점 여백 보여주기
			끝점 여백 보여주기
			사용 영역/빈공간 영역을 버튼으로 표현할 것
			*남은 길이 계산 버튼
			*열 추가/삭제 버튼
			*배치 버튼 -> 유로폼(규격/비규격) 배치
			*자투리 채우기 버튼 -> 아웃코너앵글, 합판, 목재 배치
	*/

	return	err;
}

// Cell 배열을 초기화함
void	initCellsForBeam (BeamPlacingZone* placingZone)
{
	short xx;

	for (xx = 0 ; xx < 20 ; ++xx) {
		placingZone->cellsFromBeginAtLSide [xx].objType = NONE;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomX = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomY = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomZ = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].ang = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].dirLen = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].perLen = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].attached_side = LEFT_SIDE;

		placingZone->cellsFromBeginAtRSide [xx].objType = NONE;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomX = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomY = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomZ = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].ang = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].dirLen = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].perLen = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].attached_side = RIGHT_SIDE;

		placingZone->cellsFromEndAtLSide [xx].objType = NONE;
		placingZone->cellsFromEndAtLSide [xx].leftBottomX = 0.0;
		placingZone->cellsFromEndAtLSide [xx].leftBottomY = 0.0;
		placingZone->cellsFromEndAtLSide [xx].leftBottomZ = 0.0;
		placingZone->cellsFromEndAtLSide [xx].ang = 0.0;
		placingZone->cellsFromEndAtLSide [xx].dirLen = 0.0;
		placingZone->cellsFromEndAtLSide [xx].perLen = 0.0;
		placingZone->cellsFromEndAtLSide [xx].attached_side = LEFT_SIDE;

		placingZone->cellsFromEndAtRSide [xx].objType = NONE;
		placingZone->cellsFromEndAtRSide [xx].leftBottomX = 0.0;
		placingZone->cellsFromEndAtRSide [xx].leftBottomY = 0.0;
		placingZone->cellsFromEndAtRSide [xx].leftBottomZ = 0.0;
		placingZone->cellsFromEndAtRSide [xx].ang = 0.0;
		placingZone->cellsFromEndAtRSide [xx].dirLen = 0.0;
		placingZone->cellsFromEndAtRSide [xx].perLen = 0.0;
		placingZone->cellsFromEndAtRSide [xx].attached_side = RIGHT_SIDE;

		placingZone->cellsFromBeginAtBottom [xx].objType = NONE;
		placingZone->cellsFromBeginAtBottom [xx].leftBottomX = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].leftBottomY = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].leftBottomZ = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].ang = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].dirLen = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].perLen = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].attached_side = BOTTOM_SIDE;

		placingZone->cellsFromEndAtBottom [xx].objType = NONE;
		placingZone->cellsFromEndAtBottom [xx].leftBottomX = 0.0;
		placingZone->cellsFromEndAtBottom [xx].leftBottomY = 0.0;
		placingZone->cellsFromEndAtBottom [xx].leftBottomZ = 0.0;
		placingZone->cellsFromEndAtBottom [xx].ang = 0.0;
		placingZone->cellsFromEndAtBottom [xx].dirLen = 0.0;
		placingZone->cellsFromEndAtBottom [xx].perLen = 0.0;
		placingZone->cellsFromEndAtBottom [xx].attached_side = BOTTOM_SIDE;
	}

	placingZone->cellCenterAtLSide.objType = NONE;
	placingZone->cellCenterAtLSide.leftBottomX = 0.0;
	placingZone->cellCenterAtLSide.leftBottomY = 0.0;
	placingZone->cellCenterAtLSide.leftBottomZ = 0.0;
	placingZone->cellCenterAtLSide.ang = 0.0;
	placingZone->cellCenterAtLSide.dirLen = 0.0;
	placingZone->cellCenterAtLSide.perLen = 0.0;
	placingZone->cellCenterAtLSide.attached_side = LEFT_SIDE;

	placingZone->cellCenterAtRSide.objType = NONE;
	placingZone->cellCenterAtRSide.leftBottomX = 0.0;
	placingZone->cellCenterAtRSide.leftBottomY = 0.0;
	placingZone->cellCenterAtRSide.leftBottomZ = 0.0;
	placingZone->cellCenterAtRSide.ang = 0.0;
	placingZone->cellCenterAtRSide.dirLen = 0.0;
	placingZone->cellCenterAtRSide.perLen = 0.0;
	placingZone->cellCenterAtRSide.attached_side = RIGHT_SIDE;

	placingZone->cellCenterAtBottom.objType = NONE;
	placingZone->cellCenterAtBottom.leftBottomX = 0.0;
	placingZone->cellCenterAtBottom.leftBottomY = 0.0;
	placingZone->cellCenterAtBottom.leftBottomZ = 0.0;
	placingZone->cellCenterAtBottom.ang = 0.0;
	placingZone->cellCenterAtBottom.dirLen = 0.0;
	placingZone->cellCenterAtBottom.perLen = 0.0;
	placingZone->cellCenterAtBottom.attached_side = BOTTOM_SIDE;
}

// 1차 배치
void	firstPlacingSettingsForBeam (BeamPlacingZone* placingZone)
{
	short			xx;
	API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	double			centerPos;		// 중심 위치
	double			width;			// 간섭 보가 차지하는 너비
	double			remainLength;	// 남은 길이를 계산하기 위한 임시 변수
	double			xPos;			// 위치 커서
	double			accumDist;		// 이동 거리
	
	double			formLength1 = 1.200;
	double			formLength2 = 0.900;
	double			formLength3 = 0.600;
	double			formLength;
	
	// 간섭 보가 있는 경우
		// 측면의 경우
			// 양 끝의 여백 길이를 입력할 것 - 여백은 목재/합판으로 채움
			// 양쪽 끝에서부터 유로폼 높이 1200/900/600 단위로 자동 배치 진행함 (길이 계산 기준은 간섭 보 영역 바로 옆이지만, 배치는 메인 보 끝에서부터 시작)
			// 간섭 보 옆에는 목재 배치
		// 하부의 경우
			// 양 끝의 여백 길이를 입력할 것 - 여백은 목재/합판으로 채움
			// 양쪽 끝에서부터 유로폼 높이 1200/900/600 단위로 2개씩 자동 배치 진행함
			// 가운데 남는 영역은 비규격폼 배치
	// 간섭 보가 없는 경우
		// 측면의 경우
		// 하부의 경우
			// 양 끝의 여백 길이를 입력할 것 - 여백은 목재/합판으로 채움
			// 양쪽 끝에서부터 유로폼 높이 1200/900/600 단위로 2개씩 자동 배치 진행함
			// 가운데 남는 영역은 비규격폼 배치

	// 측면에서의 중심 위치 찾기
	if (placingZone->bInterfereBeam == true) {
		centerPos = placingZone->posInterfereBeamFromLeft;	// 간섭 보의 중심 위치
		width = placingZone->interfereBeamWidth;
	} else {
		centerPos = placingZone->beamLength / 2;			// 간섭 보가 없으면 중심을 기준으로 함
		width = 0.0;
	}

	// 영역 높이에 따라 유로폼 높이를 자동으로 결정
	if ((placingZone->areaHeight - 0.050) >= 0.600)
		placingZone->eu_wid_numeric = 0.600;
	else if ((placingZone->areaHeight - 0.050) >= 0.500)
		placingZone->eu_wid_numeric = 0.500;
	else if ((placingZone->areaHeight - 0.050) >= 0.450)
		placingZone->eu_wid_numeric = 0.450;
	else if ((placingZone->areaHeight - 0.050) >= 0.400)
		placingZone->eu_wid_numeric = 0.400;
	else if ((placingZone->areaHeight - 0.050) >= 0.300)
		placingZone->eu_wid_numeric = 0.300;
	else
		placingZone->eu_wid_numeric = 0.200;

	// (1-1) 측면 시작 부분
	remainLength = centerPos - width/2;
	while (remainLength < EPS) {
		if (remainLength > formLength1) {
			formLength = formLength1;
			remainLength -= formLength1;
		} else if (remainLength > formLength2) {
			formLength = formLength2;
			remainLength -= formLength2;
		} else if (remainLength > formLength3) {
			formLength = formLength3;
			remainLength -= formLength3;
		}

		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].objType = EUROFORM;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].ang = placingZone->ang;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].perLen = placingZone->eu_wid_numeric;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].attached_side = LEFT_SIDE;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_stan_onoff = true;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].libPart.form.u_ins_wall = false;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_hei = formLength;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_wid = placingZone->eu_wid_numeric;

		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].objType = EUROFORM;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].ang = placingZone->ang;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].perLen = placingZone->eu_wid_numeric;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].attached_side = LEFT_SIDE;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_stan_onoff = true;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].libPart.form.u_ins_wall = false;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_hei = formLength;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_wid = placingZone->eu_wid_numeric;

		placingZone->nCellsFromBeginAtSide ++;
	}

	// 중심부터 끝으로 이동해야 함
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
		accumDist += placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].dirLen;

	// 위치 선정
	xPos = centerPos - width/2 - accumDist;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx) {
		placingZone->cellsFromBeginAtLSide [xx].leftBottomX = xPos;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomY = infoBeam.width/2;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomZ = placingZone->level - infoBeam.height;
		
		placingZone->cellsFromBeginAtRSide [xx].leftBottomX = xPos;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomY = -infoBeam.width/2;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomZ = placingZone->level - infoBeam.height;

		xPos += placingZone->cellsFromBeginAtLSide [xx].dirLen;
	}

	// (1-2) 측면 끝 부분
	//remainLength = placingZone->beamLength - centerPos - width/2;
	// ...

	// (1-3) 측면 가운데 부분
	// ...

	// (2-1) 하부 시작 부분
	// ...

	// (2-2) 하부 끝 부분
	// ...

	// (2-3) 하부 가운데 부분
	// ...


	//		// 여기부터는 셀 위치 지정 루틴
	//		placingZone->cells [xx][yy].leftBottomX = placingZone->leftBottomX + (placingZone->cells [xx][yy].horLen * yy);
	//		placingZone->cells [xx][yy].leftBottomY = placingZone->leftBottomY - (placingZone->cells [xx][yy].verLen * xx);
	//		placingZone->cells [xx][yy].leftBottomZ = placingZone->leftBottomZ;

	//		axisPoint.x = placingZone->leftBottomX;
	//		axisPoint.y = placingZone->leftBottomY;
	//		axisPoint.z = placingZone->leftBottomZ;

	//		rotatedPoint.x = placingZone->cells [xx][yy].leftBottomX;
	//		rotatedPoint.y = placingZone->cells [xx][yy].leftBottomY;
	//		rotatedPoint.z = placingZone->cells [xx][yy].leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		placingZone->cells [xx][yy].leftBottomX = unrotatedPoint.x;
	//		placingZone->cells [xx][yy].leftBottomY = unrotatedPoint.y;
	//		placingZone->cells [xx][yy].leftBottomZ = unrotatedPoint.z;
	//	}
	//}

	// 여백 값 업데이트
	// ...
}

// 해당 셀과 동일한 행에 있는 다른 셀들의 타입 및 높이를 조정함
void		adjustOtherCellsInSameRow (BeamPlacingZone* target_zone, short row, short col)
{
}

// 측면 시작 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void		addNewColFromBeginAtSide (BeamPlacingZone* target_zone)
{
}

// 측면 시작 부분 - 마지막 열을 삭제함
void		delLastColFromBeginAtSide (BeamPlacingZone* target_zone)
{
}

// 측면 끝 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void		addNewColFromEndAtSide (BeamPlacingZone* target_zone)
{
}

// 측면 끝 부분 - 마지막 열을 삭제함
void		delLastColFromEndAtSide (BeamPlacingZone* target_zone)
{
}

// 하부 시작 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void		addNewColFromBeginAtBottom (BeamPlacingZone* target_zone)
{
}

// 하부 시작 부분 - 마지막 열을 삭제함
void		delLastColFromBeginAtBottom (BeamPlacingZone* target_zone)
{
}

// 하부 끝 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void		addNewColFromEndAtBottom (BeamPlacingZone* target_zone)
{
}

// 하부 끝 부분 - 마지막 열을 삭제함
void		delLastColFromEndAtBottom (BeamPlacingZone* target_zone)
{
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void		alignPlacingZoneForBeam (BeamPlacingZone* target_zone)
{
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
			} else {
				element.object.pos.x += ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
				element.object.pos.y += ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
			}
		}
		GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());
		
		// 회전X
		memo.params [0][33].value.real = DegreeToRad (0.0);

	} else if (objInfo.objType == PLYWOOD) {
		element.header.layer = layerInd_Plywood;
		GS::ucscpy (memo.params [0][32].value.uStr, L("비규격"));
		GS::ucscpy (memo.params [0][33].value.uStr, L("바닥깔기"));
		GS::ucscpy (memo.params [0][34].value.uStr, L("11.5T"));
		memo.params [0][35].value.real = objInfo.libPart.plywood.p_wid;		// 가로
		memo.params [0][36].value.real = objInfo.libPart.plywood.p_leng;	// 세로
		memo.params [0][38].value.real = FALSE;		// 제작틀 OFF
		
	} else if (objInfo.objType == WOOD) {
		element.header.layer = layerInd_Wood;
		GS::ucscpy (memo.params [0][27].value.uStr, L("바닥눕히기"));	// 설치방향
		memo.params [0][28].value.real = objInfo.libPart.wood.w_w;		// 두께
		memo.params [0][29].value.real = objInfo.libPart.wood.w_h;		// 너비
		memo.params [0][30].value.real = objInfo.libPart.wood.w_leng;	// 길이
		memo.params [0][31].value.real = objInfo.libPart.wood.w_ang;	// 각도
	
	} else if (objInfo.objType == OUTCORNER_ANGLE) {
		element.header.layer = layerInd_OutcornerAngle;
		// ...
	}

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// 유로폼을 채운 후 자투리 공간 채우기
GSErrCode	fillRestAreasForBeam (void)
{
	// ...

	return	NoError;
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK beamPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "보에 배치");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			//////////////////////////////////////////////////////////// 아이템 배치 (유로폼)
			// 라벨: 유로폼 배치 설정
			DGSetItemText (dialogID, LABEL_PLACING_EUROFORM, "유로폼 배치 설정");

			// 라벨: 너비
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH, "너비");

			// 라벨: 높이
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT, "높이");

			// 라벨: 보와의 간격
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "보와의 간격");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");

			// 라벨: 레이어 - 유로폼
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");

			// 라벨: 레이어 - 목재
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "목재");

			// 라벨: 레이어 - 합판
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "합판");

			// 라벨: 레이어 - 아웃코너앵글
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "아웃코너앵글");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);

			// 유로폼 너비와 높이는 자동으로 계산되므로 입력 받지 않음
			DGDisableItem (dialogID, POPUP_EUROFORM_WIDTH);
			DGDisableItem (dialogID, POPUP_EUROFORM_HEIGHT);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 보와의 간격
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
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
	short	btnSizeX = 50, btnSizeY = 50;
	short	dialogSizeX, dialogSizeY;
	short	btnInitPosX = 220 + 25;
	short	btnPosX, btnPosY;
	short	xx, yy;
	short	idxBtn;
	short	lastIdxBtn = 0;
	std::string		txtButton = "";
	API_Element		elem;
	GSErrCode		err;

	switch (message) {
		case DG_MSG_INIT:
			//// 다이얼로그 타이틀
			//DGSetDialogTitle (dialogID, "슬래브 하부에 배치 - 유로폼 배치 수정");

			////////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			//// 업데이트 버튼
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 210, 130, 25);
			//DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_OK, "2. 배  치");
			//DGShowItem (dialogID, DG_OK);

			//// 종료 버튼
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 250, 130, 25);
			//DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_CANCEL, "3. 자투리 채우기");
			//DGShowItem (dialogID, DG_CANCEL);

			//// 라벨: 남은 가로 길이
			//DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 20, 90, 23);
			//if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
			//	DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			//else
			//	DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, "좌우 여백");
			//DGShowItem (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH);

			//// 라벨: 남은 세로 길이
			//DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 50, 90, 23);
			//if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
			//	DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			//else
			//	DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, "상하 여백");
			//DGShowItem (dialogID, LABEL_REMAIN_VERTICAL_LENGTH);

			//// Edit 컨트롤: 남은 가로 길이
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 130, 20-7, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGShowItem (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH);
			//DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);

			//// Edit 컨트롤: 남은 세로 길이
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 130, 50-7, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGShowItem (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH);
			//DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);

			//// 라벨: 유로폼/휠러스페이서 배치 설정
			//DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 10, 200, 23);
			//DGSetItemFont (dialogID, LABEL_GRID_EUROFORM_WOOD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_GRID_EUROFORM_WOOD, "유로폼/보강 목재 배치 설정");
			//DGShowItem (dialogID, LABEL_GRID_EUROFORM_WOOD);

			//// 남은 거리 확인 버튼
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 90, 130, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, "1. 남은 길이 확인");
			//DGShowItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);

			//// 행 추가 버튼
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 130, 65, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_ADD_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_ADD_ROW, "행 추가");
			//DGShowItem (dialogID, PUSHBUTTON_ADD_ROW);

			//// 행 삭제 버튼
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 130, 65, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_DEL_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_DEL_ROW, "행 삭제");
			//DGShowItem (dialogID, PUSHBUTTON_DEL_ROW);

			//// 열 추가 버튼
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 170, 65, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_ADD_COL, "열 추가");
			//DGShowItem (dialogID, PUSHBUTTON_ADD_COL);
		
			//// 열 삭제 버튼
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 170, 65, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_DEL_COL, "열 삭제");
			//DGShowItem (dialogID, PUSHBUTTON_DEL_COL);

			//// 메인 창 크기를 변경
			//dialogSizeX = 270 + (btnSizeX * placingZone.eu_count_hor) + 50;
			//dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver) + 50);
			//DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			//// 그리드 구조체에 따라서 버튼을 동적으로 배치함
			//btnPosX = 220 + 25, btnPosY = (btnSizeY * placingZone.eu_count_ver) + 25;
			//for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
			//	for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy) {
			//		idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		lastIdxBtn = idxBtn;
			//		DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

			//		txtButton = "";
			//		if (placingZone.cells [xx][yy].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cells [xx][yy].objType == EUROFORM) {
			//			if (placingZone.cells [xx][yy].libPart.form.u_ins_wall)
			//				txtButton = format_string ("유로폼\n(세움)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
			//			else
			//				txtButton = format_string ("유로폼\n(눕힘)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
			//		}
			//		DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
			//		DGShowItem (dialogID, idxBtn);
			//		btnPosX += btnSizeX;
			//	}
			//	btnPosX = btnInitPosX;
			//	btnPosY -= btnSizeY;
			//}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				//case PUSHBUTTON_CONFIRM_REMAIN_LENGTH:
				//	// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
				//	item = 0;

				//	break;

				case DG_OK:
					// 종료하지 않고 배치된 객체를 수정 및 재배치하고 그리드 버튼 속성을 변경함
					item = 0;

					//clickedOKButton = true;

					break;

				case DG_CANCEL:
					break;

				//case PUSHBUTTON_ADD_ROW:
				//	// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
				//	item = 0;

				//	break;

				//case PUSHBUTTON_DEL_ROW:
				//	// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
				//	item = 0;

				//	break;

				//case PUSHBUTTON_ADD_COL:
				//	// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
				//	item = 0;

				//	break;

				//case PUSHBUTTON_DEL_COL:
				//	// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
				//	item = 0;

				//	break;

				default:
					// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
					//clickedBtnItemIdx = item;
					//result = DGBlankModalDialog (240, 260, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);

					item = 0;	// 그리드 버튼을 눌렀을 때 창이 닫히지 않게 함

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

// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그
short DGCALLBACK beamPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	short	idxCell;
	short	popupSelectedIdx = 0;
	double	temp;
	short	rIdx, cIdx;		// 행 번호, 열 번호

	switch (message) {
		case DG_MSG_INIT:

			//// beamPlacerHandler2 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
			//idxCell = (clickedBtnItemIdx - itemInitIdx);
			//rIdx = 0;
			//while (idxCell >= (placingZone.eu_count_hor)) {
			//	idxCell -= ((placingZone.eu_count_hor));
			//	++rIdx;
			//}
			//cIdx = idxCell;
			//
			//// 다이얼로그 타이틀
			//DGSetDialogTitle (dialogID, "Cell 값 설정");

			////////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			//// 적용 버튼
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 215, 70, 25);
			//DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_OK, "저장");
			//DGShowItem (dialogID, DG_OK);

			//// 종료 버튼
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 130, 215, 70, 25);
			//DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_CANCEL, "취소");
			//DGShowItem (dialogID, DG_CANCEL);

			////////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
			//// 라벨: 객체 타입
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 20, 70, 23);
			//DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_OBJ_TYPE, "객체 타입");
			//DGShowItem (dialogID, LABEL_OBJ_TYPE);
			//DGDisableItem (dialogID, LABEL_OBJ_TYPE);

			//// 팝업컨트롤: 객체 타입을 바꿀 수 있는 콤보박스가 맨 위에 나옴
			//DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 20-7, 120, 25);
			//DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "없음");
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "유로폼");
			//DGShowItem (dialogID, POPUP_OBJ_TYPE);
			//DGDisableItem (dialogID, POPUP_OBJ_TYPE);

			//// 라벨: 너비
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 50, 70, 23);
			//DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_WIDTH, "너비");

			//// Edit 컨트롤: 너비
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 50-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			//// 라벨: 높이
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			//DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_HEIGHT, "높이");

			//// Edit 컨트롤: 높이
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			//// 라벨: 설치방향
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			//DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_ORIENTATION, "설치방향");
			//	
			//// 라디오 버튼: 설치방향 (벽세우기)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 140-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD, "벽세우기");
			//// 라디오 버튼: 설치방향 (벽눕히기)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 170-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_2_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_2_PLYWOOD, "벽눕히기");

			//// 체크박스: 규격폼
			//DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20, 50, 70, 25-5);
			//DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "규격폼");

			//// 라벨: 너비
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			//DGSetItemFont (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, "너비");

			//// 팝업 컨트롤: 너비
			//DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 80-7, 100, 25);
			//DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "600");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "500");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "450");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "400");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "300");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "200");

			//// Edit 컨트롤: 너비
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);

			//// 라벨: 높이
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 110, 70, 23);
			//DGSetItemFont (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, "높이");

			//// 팝업 컨트롤: 높이
			//DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 110-7, 100, 25);
			//DGSetItemFont (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "1200");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "900");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "600");

			//// Edit 컨트롤: 높이
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 110-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//
			//// 라벨: 설치방향
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			//DGSetItemFont (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, "설치방향");
			//
			//// 라디오 버튼: 설치방향 (벽세우기)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 140-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_1_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_1_EUROFORM, "벽세우기");
			//// 라디오 버튼: 설치방향 (벽눕히기)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 170-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_2_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_2_EUROFORM, "벽눕히기");

			//// 초기 입력 필드 표시
			//if (placingZone.cells [rIdx][cIdx].objType == EUROFORM) {
			//	DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

			//	// 체크박스: 규격폼
			//	DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
			//	DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff);

			//	if (placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff == true) {
			//		// 라벨: 너비
			//		DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

			//		// 팝업 컨트롤: 너비
			//		DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.600) < EPS)		popupSelectedIdx = 1;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.500) < EPS)		popupSelectedIdx = 2;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.450) < EPS)		popupSelectedIdx = 3;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.400) < EPS)		popupSelectedIdx = 4;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.300) < EPS)		popupSelectedIdx = 5;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.200) < EPS)		popupSelectedIdx = 6;
			//		DGPopUpSelectItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, popupSelectedIdx);

			//		// 라벨: 높이
			//		DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

			//		// 팝업 컨트롤: 높이
			//		DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
			//		DGPopUpSelectItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, popupSelectedIdx);
			//	} else if (placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff == false) {
			//		// 라벨: 너비
			//		DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

			//		// Edit 컨트롤: 너비
			//		DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
			//		DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2);
			//		DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
			//		DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);

			//		// 라벨: 높이
			//		DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

			//		// Edit 컨트롤: 높이
			//		DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
			//		DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, placingZone.cells[rIdx][cIdx].libPart.form.eu_hei2);
			//		DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
			//		DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
			//	}

			//	// 라벨: 설치방향
			//	DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
			//	
			//	// 라디오 버튼: 설치방향 (벽세우기)
			//	DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
			//	// 라디오 버튼: 설치방향 (벽눕히기)
			//	DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

			//	if (placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall == true) {
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, true);
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, false);
			//	} else if (placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall == false) {
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, false);
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, true);
			//	}

			//	DGDisableItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
			//	DGDisableItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);
			//}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					//// beamPlacerHandler2 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
					//idxCell = (clickedBtnItemIdx - itemInitIdx);
					//rIdx = 0;
					//while (idxCell >= (placingZone.eu_count_hor)) {
					//	idxCell -= ((placingZone.eu_count_hor));
					//	++rIdx;
					//}
					//cIdx = idxCell;

					////////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
					//// 입력한 값을 다시 셀에 저장
					//if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
					//	placingZone.cells [rIdx][cIdx].objType = NONE;
					//	adjustOtherCellsInSameRow (&placingZone, rIdx, cIdx);
					//	adjustOtherCellsInSameCol (&placingZone, rIdx, cIdx);

					//} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
					//	placingZone.cells [rIdx][cIdx].objType = EUROFORM;

					//	// 규격폼
					//	if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff = true;
					//	else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE)
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff = false;

					//	if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
					//		// 너비
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS)).ToCStr ()) / 1000.0;
					//		placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_wid;
					//		// 높이
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS)).ToCStr ()) / 1000.0;
					//		placingZone.cells [rIdx][cIdx].verLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_hei;
					//	} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
					//		// 너비
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
					//		placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2;
					//		// 높이
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
					//		placingZone.cells [rIdx][cIdx].verLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_hei2;
					//	}

					//	// 설치방향
					//	if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == TRUE)
					//		placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall = true;
					//	else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == FALSE) {
					//		placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall = false;
					//		// 가로, 세로 길이 교환
					//		temp = placingZone.cells [rIdx][cIdx].horLen;
					//		placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].verLen;
					//		placingZone.cells [rIdx][cIdx].verLen = temp;
					//	}

					//	adjustOtherCellsInSameRow (&placingZone, rIdx, cIdx);
					//	adjustOtherCellsInSameCol (&placingZone, rIdx, cIdx);
					//}

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
