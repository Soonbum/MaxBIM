#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "WallTableformPlacerContinue.hpp"

//namespace wallTableformPlacerContinueDG;

static InfoWall						infoWall;		// 벽 객체 정보

// 벽에 연속으로 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnWallContinually (void)
{
	GSErrCode	err = NoError;
	short		xx, yy;
	double		dx, dy;

	// Selection Manager 관련 변수
	long		nSel;
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	walls = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	polylines = GS::Array<API_Guid> ();
	long					nWalls = 0;
	long					nMorphs = 0;
	long					nPolylines = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 객체 정보
	InfoMorphForWallTableformContinue	infoMorph;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_wall;		// 벽의 작업 층 높이

	// 모프 3D 구성요소 가져오기
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();

	// 모프에서 제외되는 점 4개
	API_Coord3D		excludeP1;
	API_Coord3D		excludeP2;
	API_Coord3D		excludeP3;
	API_Coord3D		excludeP4;

	excludeP1.x = 0;	excludeP1.y = 0;	excludeP1.z = 0;
	excludeP2.x = 1;	excludeP2.y = 0;	excludeP2.z = 0;
	excludeP3.x = 0;	excludeP3.y = 1;	excludeP3.z = 0;
	excludeP4.x = 0;	excludeP4.y = 0;	excludeP4.z = 1;

	// 기타
	char	buffer [256];


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 벽 (1개), 벽 일부를 덮는 모프 (1개), 벽을 감싸는 폴리라인 (여러개)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

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

			if (tElem.header.typeID == API_PolyLineID)	// 폴리라인인가?
				polylines.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nWalls = walls.GetSize ();
	nMorphs = morphs.GetSize ();
	nPolylines = polylines.GetSize ();

	// 벽이 1개인가?
	if (nWalls != 1) {
		ACAPI_WriteReport ("벽을 1개 선택해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("벽의 일부를 덮는 모프를 1개 선택해야 합니다.\n모프는 테이블폼/유로폼을 덮는 높이 영역을 표현해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 폴리라인이 2개 이상인가?
	if ( !(nPolylines >= 2) ) {
		ACAPI_WriteReport ("벽을 감싸는 폴리라인을 최소 2개 이상 선택해야 합니다.", true);
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
	//placingZone.leftBottomX		= infoMorph.leftBottomX;
	//placingZone.leftBottomY		= infoMorph.leftBottomY;
	//placingZone.leftBottomZ		= infoMorph.leftBottomZ;
	//placingZone.horLen			= infoMorph.horLen;
	//placingZone.verLen			= infoMorph.verLen;
	//placingZone.ang				= DegreeToRad (infoMorph.ang);
	
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
	//placingZone.leftBottomZ = infoWall.bottomOffset;

	// 영역 모프 제거
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;


	// [완료] 모프를 이용해 고/저 높이 값을 저장함 -> 구조체 InfoMorphForWallTableform 활용

	// 격자 패턴 점들을 추출
	// 1. 폴리라인을 회전시킴 (X, Y축이 회전되지 않았다고 간주하도록) -> 함수 getUnrotatedPoint 활용
		// 회전축이 되는 좌하단 점은 어떻게 찾는가?
	// 2. 모든 폴리라인을 스캔하면서 x점 vector, y점 vector를 일단 저장하고
	// 3. vector를 정렬한 후, 중복된 점은 모두 소거
	// 4. 정리된 x, y점을 API_Coord로 통합 -> 점 집합

	// 점들 중에서 선 상에 있지 않은 점 소거
	// ... 별도 함수 필요
	// 1. 점 집합 -> 폴리라인들 별로 실행 : 해당 점이 몇 번 폴리라인에 속했는지 찾아서 폴리라인 번호 지정할 것

	// 폴리라인 간의 점 간 거리 측정, 벽 두께와 동일한 쌍을 찾아서 저장하고 x축 위치 오름차순으로 정렬
	// 1. 이중 반복문을 통해 폴리i, 폴리j 간의 점 거리 측정 -> 벽 두께와 동일한 쌍을 찾는다.
	// 2. 각 쌍을 저장 (벽 라인을 따라 붙어 있는 점들)
	// 3. 저장한 쌍을 x축 위치 기준으로 오름차순 정렬, 중복된 쌍은 모두 소거

	// 쌍-폴리라인 순회
	// 쌍의 이전 점과 폴리라인의 이전 점이 같을 때, 쌍의 다음 점과 폴리라인의 다음 점이 같으면 --> 벽 라인 O (이전 점의 flag를 true로 set)
	// 쌍의 이전 점과 폴리라인의 이전 점이 같을 때, 쌍의 다음 점과 폴리라인의 다음 점이 다르면 --> 벽 라인 X (이전 점의 flag를 false로 set)

	// 벽 라인 추출
	// 쌍 순회: 연속적으로 점마다 벽 라인일 경우 전체를 하나의 벽으로 추출

	//if (nMorphs > 0) {
	//	for (xx = 0 ; xx < nMorphs ; ++xx) {
	//		// 모프의 정보를 가져옴
	//		BNZeroMemory (&elem, sizeof (API_Element));
	//		elem.header.guid = morphs.Pop ();
	//		err = ACAPI_Element_Get (&elem);
	//		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	//		// 모프의 점 좌표들을 가져옴
	//		BNZeroMemory (&component, sizeof (API_Component3D));
	//		component.header.typeID = API_BodyID;
	//		component.header.index = info3D.fbody;
	//		err = ACAPI_3D_GetComponent (&component);

	//		nVert = component.body.nVert;
	//		nEdge = component.body.nEdge;
	//		nPgon = component.body.nPgon;
	//		tm = component.body.tranmat;
	//		elemIdx = component.body.head.elemIndex - 1;
	//		bodyIdx = component.body.head.bodyIndex - 1;
	//
	//		// 정점 좌표를 가져옴
	//		for (yy = 1 ; yy <= nVert ; ++yy) {
	//			component.header.typeID	= API_VertID;
	//			component.header.index	= yy;
	//			err = ACAPI_3D_GetComponent (&component);
	//			if (err == NoError) {
	//				trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
	//				trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
	//				trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
	//				coords.Push (trCoord);

	//				sprintf (buffer, "%d ", yy);

	//				if ( !(isSamePoint (excludeP1, trCoord) || isSamePoint (excludeP2, trCoord) || isSamePoint (excludeP3, trCoord) || isSamePoint (excludeP4, trCoord)) ) {
	//					placeCoordinateLabel (trCoord.x, trCoord.y, trCoord.z, true, buffer);
	//				}
	//			}
	//		}
	//	}
	//}

	//if (nPolylines > 0) {
	//	for (xx = 0 ; xx < nPolylines ; ++xx) {
	//		// 폴리라인의 정보를 가져옴
	//		BNZeroMemory (&elem, sizeof (API_Element));
	//		BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//		elem.header.guid = polylines.Pop ();
	//		err = ACAPI_Element_Get (&elem);
	//		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//		// 정점 좌표를 가져옴
	//		for (yy = 1 ; yy <= elem.polyLine.poly.nCoords ; ++yy) {
	//			sprintf (buffer, "%d ", yy);
	//			err = placeCoordinateLabel (memo.coords [0][yy].x, memo.coords [0][yy].y, 0, true, buffer);
	//		}
	//	}
	//}

	return err;
}
