#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabEuroformPlacer.hpp"


// 2번 메뉴: 슬래브 하부에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy, nn;
	double		dx, dy, ang;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	long					nMorphs = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElemInfo3D			info3D;

	// 모프 3D 구성요소 가져오기
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();

	// 모프 객체 정보
	InfoMorphForSlab		infoMorph;

	// 점 입력
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	API_Coord3D				tempPoint, resultPoint;
	GS::Array<API_Coord3D>&	coordsRotated = GS::Array<API_Coord3D> ();

	// ...
	char  msg[100];

	// 작업 층 정보
	API_StoryInfo			storyInfo;
	double					plusLevel;


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 슬래브 하부를 덮는 모프 (1개)", true);
		return err;
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 모프 1개 선택해야 함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_MorphID)		// 모프인가?
				morphs.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nMorphs = morphs.GetSize ();

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("슬래브 하부를 덮는 모프를 1개 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// 만약 모프가 누워 있어야 함
	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) > EPS) {
		ACAPI_WriteReport ("모프가 누워 있지 않습니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프의 GUID 저장
	infoMorph.guid = elem.header.guid;

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
			if ( (abs (trCoord.z - elem.morph.level) < EPS) && (abs (elem.morph.level - trCoord.z) < EPS) ) {
				coords.Push (trCoord);
			}
		}
	}

	// 하단 점 2개를 클릭
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("좌하단 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("우하단 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;
	
	// 폴리곤 회전각도를 구함
	dx = point2.x - point1.x;
	dy = point2.y - point1.y;
	ang = RadToDegree (atan2 (dy, dx));
	
	// 회전각도 0일 때의 좌표를 계산함
	nn = coords.GetSize ();
	for (xx = 0 ; xx < nn ; ++xx) {
		tempPoint = coords.Pop ();
		resultPoint.x = point1.x + ((tempPoint.x - point1.x)*cos(DegreeToRad (-ang)) - (tempPoint.y - point1.y)*sin(DegreeToRad (-ang)));
		resultPoint.y = point1.y + ((tempPoint.x - point1.x)*sin(DegreeToRad (-ang)) + (tempPoint.y - point1.y)*cos(DegreeToRad (-ang)));
		resultPoint.z = tempPoint.z;

		coordsRotated.Push (resultPoint);
	}
	
	// ...
	// 폴리곤 점을 배열로 복사함
	API_Coord3D		nodes_random [20];
	API_Coord3D		nodes_sequential [20];
	API_Coord3D		tPoint;
	short			nNodes = coordsRotated.GetSize ();
	bool			bFindPoint;
	API_Coord3D		bufPoint;
	double			xOffset1, xOffset2;
	double			yOffset1, yOffset2;

	// 사용자가 클릭한 1, 2번 점을 직접 저장
	nodes_random [0] = point1;
	nodes_random [1] = point2;
	nodes_sequential [0] = point1;
	nodes_sequential [1] = point2;

	// 사용자가 클릭한 1, 2번 점을 제외한 나머지 점들을 저장함
	xx = 0;
	yy = 0;
	while (xx < nNodes) {
		tPoint = coordsRotated.Pop ();
		if ( !( (compareDoubles (tPoint.x, point1.x) == 0) && (compareDoubles (tPoint.y, point1.y) == 0) && (compareDoubles (tPoint.z, point1.z) == 0) ) || !( (compareDoubles (tPoint.x, point2.x) == 0) && (compareDoubles (tPoint.y, point2.y) == 0) && (compareDoubles (tPoint.z, point2.z) == 0) ) ) {
			nodes_random [yy] = tPoint;
			++yy;
		}
		++xx;
	}

	// !!! 중복 점 입력 해결할 것
	// 폴리곤 점 순서대로 저장할 것
	for (xx = 1 ; xx < (nNodes - 1) ; ++xx) {
		
		// nodes_sequential [xx : 1 ~ nNodes-1] 는 입력된 최종 점 (우하단 점부터 시작)
		// nodes_random [yy : 0 ~ nNodes]
			// nodes_random이 nodes_sequential [0 ~ xx-1] 와 동일하면 통과
			// ... 삽입이 된 포인트는 체크하도록 하면 어떨까?
			// nodes_sequential [xx]와 비교 : x축 또는 y축 값이 동일한지 확인한다. - 단, Z값은 무조건 같아야 함
			// 첫 조우	-- 버퍼에 보관
			// 또 조우	-- nodes_sequential [xx]와 버퍼 간의 비교
			//			-- nodes_sequential [xx]와 nodes_random [yy] 간의 비교
			//			-- 둘 중에서 거리가 가까운 쪽을 버퍼로 채택
		


		/*
		bFindPoint = false;

		for (yy = 0 ; yy < nNodes ; ++yy) {

			// 똑같은 좌표의 점은 통과
			if (isAlreadyStored (nodes_random [yy], nodes_sequential, 0, xx) == true)
				continue;

			// 동일 축 상에 있는 가까운 점을 찾을 것
			if ( (compareDoubles (nodes_sequential [xx].x, nodes_random [yy].x) == 0) || (compareDoubles (nodes_sequential [xx].y, nodes_random [yy].y) == 0) ) {

				if (bFindPoint == false) {
					bFindPoint = true;
					bufPoint = nodes_random [yy];
				} else {
					// bufPoint 말고 다른 동일 축 상의 점을 찾았다면,
					xOffset1 = abs (nodes_sequential [xx].x - bufPoint.x);	xOffset2 = abs (nodes_sequential [xx].x - nodes_random [yy].x);
					yOffset1 = abs (nodes_sequential [xx].y - bufPoint.y);	yOffset2 = abs (nodes_sequential [xx].y - nodes_random [yy].y);

					// 최종 점에 더 가까운지 비교할 것
					if ( (abs (xOffset1 - xOffset2) > EPS) || (abs (yOffset1 - yOffset2) > EPS) ) {
						if (xOffset1 > xOffset2)
							bufPoint = nodes_random [yy];

						if (yOffset1 > yOffset2)
							bufPoint = nodes_random [yy];
					}
				}
			}

			// 가까운 점을 nodes_sequential [xx+1]에 저장
			if (bFindPoint == true)
				nodes_sequential [xx+1] = bufPoint;
		}
		*/
	}

	// ... nodes_sequential 를 순서대로 출력
	//err = ACAPI_CallUndoableCommand ("영역 모프 제거", [&] () -> GSErrCode {
	//for (xx = 0 ; xx < nNodes ; ++xx) {
	//	sprintf (msg, "점 %d (%.4f, %.4f, %.4f)", xx, nodes_sequential [xx].x, nodes_sequential [xx].y, nodes_sequential [xx].z);
	//	ACAPI_WriteReport (msg, true);
	//	placeCoordinateLabel (nodes_sequential [xx].x, nodes_sequential [xx].y, nodes_sequential [xx].z, true, format_string ("%d", xx), 1, 0);
	//}
	//return NoError;
	//});

	// 영역의 너비, 높이를 구해야 한다.




	/*
		* 사용 부재(3가지): 유로폼(회전X: 0도, 벽세우기로 고정), 합판(각도: 90도), 목재(설치방향: 바닥눕히기)
		
		3. 작업 층 높이 반영
		4. 모든 영역 정보의 Z 정보를 수정
		5. 사용자 입력 (1차)
			- 기본 배치 폼 선택
				: 규격폼 기준 -- 너비, 높이 (방향은 사용자가 찍은 모프 좌하단 점을 기준으로 함)
				: 설치방향을 어떤 기준으로 정할 것인가? - 모프의 처음 찍은 점(TMX 점 인식)을 좌하단 점이라고 가정함
				: 최하단 점에 기둥이나 보의 간섭이 있으면 어떻게 할 것인가? -> 구간별 길이를 측정해서 처음 긴 선을 아래쪽 라인이라고 하자. -> 아래쪽 긴 라인의 시작/끝 점을 기준으로 각도값 획득
			- 부재별 레이어 설정 (유로폼, 합판, 목재)
			- 유로폼 셀은 전부 붙어 있어도 됨
			- 목재 보강재 위치 셀 버튼도 있어야 함
		6. 사용자 입력 (2차)
			- 타이틀: 가로/세로 채우기
			- 버튼: 1. 남은 길이 확인 // 2. 배  치 // 3. 마무리
			- 배치 버튼을 보여줌 (x번행 y번열 버튼의 폼 크기를 변경하면? - 너비를 바꾸면 y열 전체에 영향을 줌, 높이를 바꾸면 x행 전체에 영향을 줌)
			- 남은 가로/세로 길이: 표시만 함 (양쪽 다 합친 것): 권장 남은 길이 표시 (150~300mm), 권장 길이인지 아닌지 글꼴로 표시
			- 사방 합판 너비를 조정할 수 있어야 함 (위/아래 배분, 왼쪽/오른쪽 배분 - 사용자가 직접.. 그에 따라 유로폼 전체 배열이 이동됨)
			- 배치 버튼 사방에는 버튼 사이마다 보강 목재를 댈지 여부를 선택할 수 있어야 함 (버튼 □■)
		7. 최종 배치
			- (1) 유로폼은 모프 센터를 기준으로 배치할 것
			- (2) 합판(11.5T, 제작틀 Off)을 모프에 맞게 배치하되, 겹치는 부분은 모깎기가 필요함! (난이도 높음)
			- (3) 안쪽에 목재를 댈 것 (Z방향 두께는 50이어야 함, 너비는 80)
			- (4) 목재 보강재도 댈 것 (안쪽 목재와 두께, 너비는 동일 - 길이는 사용자 설정을 따름)
	*/

	return	err;
}

// aPoint가 pointList에 보관이 되었는지 확인함
bool	isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd)
{
	short	xx;

	for (xx = startInd ; xx < endInd ; ++xx) {
		// 모든 좌표 값이 일치할 경우, 이미 포함된 좌표 값이라고 인정함
		if ( (abs (aPoint.x - pointList [xx].x) < EPS) && (abs (aPoint.y - pointList [xx].y) < EPS) && (abs (aPoint.z - pointList [xx].z) < EPS) &&
			(abs (pointList [xx].x - aPoint.x) < EPS) && (abs (pointList [xx].y - aPoint.y) < EPS) && (abs (pointList [xx].z - aPoint.z) < EPS) ) {
			return true;
		}
	}

	return false;
}