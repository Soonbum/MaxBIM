#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "BeamEuroformPlacer.hpp"
#include "SlabEuroformPlacer.hpp"

using namespace beamPlacerDG;

static BeamPlacingZone	placingZone;			// 기본 보 영역 정보
static InfoBeam			infoBeam;				// 보 객체 정보
static short			nInterfereBeams;		// 간섭 보 개수
static InfoBeam			infoOtherBeams [10];	// 간섭 보 정보

// 3번 메뉴: 보에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnBeam (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx;
	double			dx, dy, ang;
	API_Coord3D		rotatedPoint, unrotatedPoint;

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
	placingZone.ang = ang;
	placingZone.level = infoBeam.level;

	// 간섭 보들의 시작점/끝점 중 메인 보에 가까운 쪽 점을 찾는다.
		// 간섭 보는 양쪽에 한쌍씩만 온다고 가정
		// IntersectionPoint1 함수를 이용하여 교차점 찾기
		// 간섭 보 하나를 찾으면 더 이상 찾지 않고 중단
	// 영역 설정
		// 메인 보 전체 길이를 기반으로 다음을 계산할 것
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

	// 작업 층 높이 반영

	// 영역 정보의 고도 정보 수정

	// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음

	// 문자열로 된 유로폼의 너비/높이를 실수형으로도 저장

	// 남은 길이 초기화

	// 유로폼 개수 세기

	// 유로폼 시작 좌표 설정

	// 위의 점을 unrotated 위치로 업데이트

	// placingZone의 Cell 정보 초기화

	// 배치를 위한 정보 입력

	// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정합니다.

	/*
		1. 사용 부재: 유로폼, 합판, 목재, 아웃코너앵글
		2. UI
			(1) 유로폼: 너비, 높이
				영역 모프에 의한 보의 덮는 영역 높이를 보여줌
			(2) 측면 배치, 하부 배치 따로 보여주기
				시작점 여백 보여주기
				끝점 여백 보여주기
				사용 영역/빈공간 영역을 버튼으로 표현할 것
				오프셋 설정 (처음에는 중심.. 시작점 또는 끝점 쪽으로 이동 가능함)
				*남은 길이 계산 버튼
				*열 추가/삭제 버튼
				*배치 버튼 -> 유로폼(규격/비규격) 배치
				*자투리 채우기 버튼 -> 아웃코너앵글, 합판, 목재 배치
	*/

	return	err;
}
