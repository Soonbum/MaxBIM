#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabTableformPlacer.hpp"

using namespace slabTableformPlacerDG;

// 슬래브 하부에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy;
	double		dx, dy, ang;
	API_Coord3D	rotatedPoint, unrotatedPoint;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	slabs = GS::Array<API_Guid> ();
	long					nMorphs = 0;
	long					nSlabs = 0;

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

	// 모프 객체 정보
	//InfoMorphForSlab		infoMorph;

	// 점 입력
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	API_Coord3D				tempPoint, resultPoint;
	bool					bIsInPolygon1, bIsInPolygon2;
	
	// 폴리곤 점을 배열로 복사하고 순서대로 좌표 값을 얻어냄
	API_Coord3D		nodes_random [20];
	API_Coord3D		nodes_sequential [20];
	long			nNodes;		// 모프 폴리곤의 정점 좌표 개수
	long			nEntered;
	bool			bFindPoint;
	API_Coord3D		bufPoint;
	short			result;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_slab;

	// 코너 좌표를 구하기 위한 최외곽 좌표 임시 저장
	API_Coord3D		outer_leftTop;
	API_Coord3D		outer_leftBottom;
	API_Coord3D		outer_rightTop;
	API_Coord3D		outer_rightBottom;

	API_Coord3D		outer_leftTopBelow;
	API_Coord3D		outer_leftTopSide;
	API_Coord3D		outer_leftBottomOver;
	API_Coord3D		outer_leftBottomSide;
	API_Coord3D		outer_rightTopBelow;
	API_Coord3D		outer_rightTopSide;
	API_Coord3D		outer_rightBottomOver;
	API_Coord3D		outer_rightBottomSide;


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 슬래브 (1개), 슬래브 하부를 덮는 모프 (1개)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 슬래브 1개, 모프 1개 선택해야 함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_MorphID)		// 모프인가?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_SlabID)		// 슬래브인가?
				slabs.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nMorphs = morphs.GetSize ();
	nSlabs = slabs.GetSize ();

	// 슬래브가 1개인가?
	if (nSlabs != 1) {
		ACAPI_WriteReport ("슬래브를 1개 선택해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("슬래브 하부를 덮는 모프를 1개 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// (1) 슬래브 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = slabs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
	
	//infoSlab.floorInd		= elem.header.floorInd;
	//infoSlab.offsetFromTop	= elem.slab.offsetFromTop;
	//infoSlab.thickness		= elem.slab.thickness;
	//infoSlab.level			= elem.slab.level;

	ACAPI_DisposeElemMemoHdls (&memo);

	// (2) 모프 정보를 가져옴
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

	// 모프의 정보 저장
	//infoMorph.guid		= elem.header.guid;
	//infoMorph.floorInd	= elem.header.floorInd;
	//infoMorph.level		= info3D.bounds.zMin;

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
			//if ( (abs (trCoord.z - elem.morph.level) < EPS) && (abs (elem.morph.level - trCoord.z) < EPS) )
				coords.Push (trCoord);
		}
	}
	nNodes = coords.GetSize ();

	// 하단 점 2개를 클릭
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("모프의 하단 라인의 왼쪽 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;
	//firstClickPoint = point1;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("모프의 하단 라인의 오른쪽 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;

	// 두 점 간의 각도를 구함
	dx = point2.x - point1.x;
	dy = point2.y - point1.y;
	ang = RadToDegree (atan2 (dy, dx));

	// 회전각도 0일 때의 좌표를 계산하고, 폴리곤의 점들을 저장함
	for (xx = 0 ; xx < nNodes ; ++xx) {
		tempPoint = coords.Pop ();
		resultPoint.x = point1.x + ((tempPoint.x - point1.x)*cos(DegreeToRad (-ang)) - (tempPoint.y - point1.y)*sin(DegreeToRad (-ang)));
		resultPoint.y = point1.y + ((tempPoint.x - point1.x)*sin(DegreeToRad (-ang)) + (tempPoint.y - point1.y)*cos(DegreeToRad (-ang)));
		resultPoint.z = tempPoint.z;

		nodes_random [xx] = resultPoint;
	}

	// 사용자가 클릭한 1, 2번 점을 직접 저장
	tempPoint = point2;
	resultPoint.x = point1.x + ((tempPoint.x - point1.x)*cos(DegreeToRad (-ang)) - (tempPoint.y - point1.y)*sin(DegreeToRad (-ang)));
	resultPoint.y = point1.y + ((tempPoint.x - point1.x)*sin(DegreeToRad (-ang)) + (tempPoint.y - point1.y)*cos(DegreeToRad (-ang)));
	resultPoint.z = tempPoint.z;

	nodes_sequential [0] = point1;
	nodes_sequential [1] = resultPoint;

	// 만약 선택한 두 점이 폴리곤에 속한 점이 아니면 오류
	bIsInPolygon1 = false;
	bIsInPolygon2 = false;
	for (xx = 0 ; xx < nNodes ; ++xx) {
		if (isSamePoint (point1, nodes_random [xx]))
			bIsInPolygon1 = true;
		if (isSamePoint (resultPoint, nodes_random [xx]))
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

	bufPoint.x = 0.0;
	bufPoint.y = 0.0;
	bufPoint.z = 0.0;

	// 폴리곤 점 순서대로 저장할 것
	nEntered = 2;	// 이미 2개의 점은 입력되어 있음
	for (xx = 1 ; xx < (nNodes-1) ; ++xx) {
		
		bFindPoint = false;		// 다음 점을 찾았는지 여부 (가까운 점, 먼 점을 구분하기 위함)

		for (yy = 0 ; yy < nNodes ; ++yy) {

			// 이미 저장된 점이 아닌 경우
			if (!isAlreadyStored (nodes_random [yy], nodes_sequential, 0, xx)) {
				// 다음 점이 맞습니까?
				if (isNextPoint (nodes_sequential [xx-1], nodes_sequential [xx], nodes_random [yy])) {

					// 처음 찾은 경우
					if (bFindPoint == false) {
						bFindPoint = true;
						bufPoint = nodes_random [yy];
					}
				
					// 또 찾은 경우
					if (bFindPoint == true) {
						result = moreCloserPoint (nodes_sequential [xx], bufPoint, nodes_random [yy]);	// nodes_sequential [xx]에 가까운 점이 어떤 점입니까?

						if (result == 2)
							bufPoint = nodes_random [yy];
					}
				}
			}
		}

		// 찾은 점을 다음 점으로 추가
		if (bFindPoint) {
			nodes_sequential [xx+1] = bufPoint;
			++nEntered;
		}
	}

	// 그 외 영역 정보를 지정함
	//placingZone.ang = DegreeToRad (ang);
	//placingZone.level = nodes_sequential [0].z;




	//// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
	//result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), slabBottomPlacerHandler1, 0);

	//// 작업 층 높이 반영
	//BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	//workLevel_slab = 0.0;
	//ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	//for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
	//	if (storyInfo.data [0][xx].index == infoSlab.floorInd) {
	//		workLevel_slab = storyInfo.data [0][xx].level;
	//		break;
	//	}
	//}
	//BMKillHandle ((GSHandle *) &storyInfo.data);
	//
	//// 영역 정보의 고도 정보 수정
	//placingZone.level = infoSlab.level + infoSlab.offsetFromTop - infoSlab.thickness - placingZone.gap;

	//// 문자열로 된 유로폼의 너비/높이를 실수형으로도 저장
	//placingZone.eu_wid_numeric = atof (placingZone.eu_wid.c_str ()) / 1000.0;
	//placingZone.eu_hei_numeric = atof (placingZone.eu_hei.c_str ()) / 1000.0;

	//// 최외곽 좌표를 얻음
	//placingZone.outerLeft	= nodes_sequential [0].x;
	//placingZone.outerRight	= nodes_sequential [0].x;
	//placingZone.outerTop	= nodes_sequential [0].y;
	//placingZone.outerBottom	= nodes_sequential [0].y;

	//for (xx = 1 ; xx < nEntered ; ++xx) {
	//	if (nodes_sequential [xx].x < placingZone.outerLeft)
	//		placingZone.outerLeft = nodes_sequential [xx].x;
	//	if (nodes_sequential [xx].x > placingZone.outerRight)
	//		placingZone.outerRight = nodes_sequential [xx].x;
	//	if (nodes_sequential [xx].y > placingZone.outerTop)
	//		placingZone.outerTop = nodes_sequential [xx].y;
	//	if (nodes_sequential [xx].y < placingZone.outerBottom)
	//		placingZone.outerBottom = nodes_sequential [xx].y;
	//}

	//// 가장 꼭지점 좌표를 임시로 생성함
	//outer_leftTop.x		= placingZone.outerLeft;	outer_leftTop.y		= placingZone.outerTop;		outer_leftTop.z		= placingZone.level;
	//outer_leftBottom.x	= placingZone.outerLeft;	outer_leftBottom.y	= placingZone.outerBottom;	outer_leftBottom.z	= placingZone.level;
	//outer_rightTop.x	= placingZone.outerRight;	outer_rightTop.y	= placingZone.outerTop;		outer_rightTop.z	= placingZone.level;
	//outer_rightBottom.x	= placingZone.outerRight;	outer_rightBottom.y	= placingZone.outerBottom;	outer_rightBottom.z	= placingZone.level;

	//API_Coord3D	tPoint;
	//tPoint.x = (placingZone.outerLeft + placingZone.outerRight) / 2;
	//tPoint.y = (placingZone.outerTop + placingZone.outerBottom) / 2;
	//tPoint.z = placingZone.level;

	//outer_leftTopBelow		= tPoint;
	//outer_leftTopSide		= tPoint;
	//outer_leftBottomOver	= tPoint;
	//outer_leftBottomSide	= tPoint;
	//outer_rightTopBelow		= tPoint;
	//outer_rightTopSide		= tPoint;
	//outer_rightBottomOver	= tPoint;
	//outer_rightBottomSide	= tPoint;

	//// 가장 꼭지점 좌표의 바로 안쪽에 있는 점의 좌표를 찾아냄
	//for (xx = 0 ; xx < nEntered ; ++xx) {
	//	if ( (!isSamePoint (outer_leftTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_leftBottom) == 1) && (abs (nodes_sequential [xx].x - outer_leftTop.x) < EPS) )
	//		outer_leftTopBelow = nodes_sequential [xx];

	//	if ( (!isSamePoint (outer_leftTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_rightTop) == 1) && (abs (nodes_sequential [xx].y - outer_leftTop.y) < EPS) )
	//		outer_leftTopSide = nodes_sequential [xx];

	//	if ( (!isSamePoint (outer_leftBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_leftBottom) == 2) && (abs (nodes_sequential [xx].x - outer_leftBottom.x) < EPS) )
	//		outer_leftBottomOver = nodes_sequential [xx];

	//	if ( (!isSamePoint (outer_leftBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftBottom, outer_rightBottom) == 1) && (abs (nodes_sequential [xx].y - outer_leftBottom.y) < EPS) )
	//		outer_leftBottomSide = nodes_sequential [xx];

	//	if ( (!isSamePoint (outer_rightTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_rightTop, outer_rightBottom) == 1) && (abs (nodes_sequential [xx].x - outer_rightTop.x) < EPS) )
	//		outer_rightTopBelow = nodes_sequential [xx];

	//	if ( (!isSamePoint (outer_rightTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_rightTop) == 2) && (abs (nodes_sequential [xx].y - outer_rightTop.y) < EPS) )
	//		outer_rightTopSide = nodes_sequential [xx];

	//	if ( (!isSamePoint (outer_rightBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_rightTop, outer_rightBottom) == 2) && (abs (nodes_sequential [xx].x - outer_rightBottom.x) < EPS) )
	//		outer_rightBottomOver = nodes_sequential [xx];

	//	if ( (!isSamePoint (outer_rightBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftBottom, outer_rightBottom) == 2) && (abs (nodes_sequential [xx].y - outer_rightBottom.y) < EPS) )
	//		outer_rightBottomSide = nodes_sequential [xx];
	//}

	//// 꺾인 코너 혹은 일반 코너의 좌표를 찾아냄
	//for (xx = 0 ; xx < nEntered ; ++xx) {
	//	// 좌상단 코너
	//	if ( (isSamePoint (outer_leftTopBelow, tPoint)) && (isSamePoint (outer_leftTopSide, tPoint)) )
	//		placingZone.corner_leftTop = outer_leftTop;
	//	else {
	//		placingZone.corner_leftTop.x = outer_leftTopSide.x;
	//		placingZone.corner_leftTop.y = outer_leftTopBelow.y;
	//		placingZone.corner_leftTop.z = placingZone.level;
	//	}
	//	
	//	// 좌하단 코너
	//	if ( (isSamePoint (outer_leftBottomOver, tPoint)) && (isSamePoint (outer_leftBottomSide, tPoint)) )
	//		placingZone.corner_leftBottom = outer_leftBottom;
	//	else {
	//		placingZone.corner_leftBottom.x = outer_leftBottomSide.x;
	//		placingZone.corner_leftBottom.y = outer_leftBottomOver.y;
	//		placingZone.corner_leftBottom.z = placingZone.level;
	//	}

	//	// 우상단 코너
	//	if ( (isSamePoint (outer_rightTopBelow, tPoint)) && (isSamePoint (outer_rightTopSide, tPoint)) )
	//		placingZone.corner_rightTop = outer_rightTop;
	//	else {
	//		placingZone.corner_rightTop.x = outer_rightTopSide.x;
	//		placingZone.corner_rightTop.y = outer_rightTopBelow.y;
	//		placingZone.corner_rightTop.z = placingZone.level;
	//	}

	//	// 우하단 코너
	//	if ( (isSamePoint (outer_rightBottomOver, tPoint)) && (isSamePoint (outer_rightBottomSide, tPoint)) )
	//		placingZone.corner_rightBottom = outer_rightBottom;
	//	else {
	//		placingZone.corner_rightBottom.x = outer_rightBottomSide.x;
	//		placingZone.corner_rightBottom.y = outer_rightBottomOver.y;
	//		placingZone.corner_rightBottom.z = placingZone.level;
	//	}
	//}

	//// 코너 안쪽의 좌표를 구함
	//if (placingZone.corner_leftTop.x < placingZone.corner_leftBottom.x)
	//	placingZone.innerLeft = placingZone.corner_leftBottom.x;
	//else
	//	placingZone.innerLeft = placingZone.corner_leftTop.x;

	//if (placingZone.corner_rightTop.x < placingZone.corner_rightBottom.x)
	//	placingZone.innerRight = placingZone.corner_rightTop.x;
	//else
	//	placingZone.innerRight = placingZone.corner_rightBottom.x;

	//if (placingZone.corner_leftTop.y < placingZone.corner_rightTop.y)
	//	placingZone.innerTop = placingZone.corner_leftTop.y;
	//else
	//	placingZone.innerTop = placingZone.corner_rightTop.y;

	//if (placingZone.corner_leftBottom.y < placingZone.corner_rightBottom.y)
	//	placingZone.innerBottom = placingZone.corner_rightBottom.y;
	//else
	//	placingZone.innerBottom = placingZone.corner_leftBottom.y;

	//// 영역 안쪽의 너비와 높이를 구함
	//placingZone.innerWidth = placingZone.innerRight - placingZone.innerLeft;
	//placingZone.innerHeight = placingZone.innerTop - placingZone.innerBottom;

	//// 남은 길이 초기화
	//placingZone.remain_hor = placingZone.outerRight - placingZone.outerLeft;
	//placingZone.remain_ver = placingZone.outerTop - placingZone.outerBottom;

	//// 유로폼 가로/세로 방향 개수 세기
	//placingZone.eu_count_hor = 0;
	//placingZone.eu_count_ver = 0;

	//if (placingZone.eu_ori.compare (std::string ("벽세우기")) == 0) {
	//	placingZone.eu_count_hor = static_cast<short>(placingZone.remain_hor / placingZone.eu_wid_numeric);				// 가로 방향 개수
	//	placingZone.remain_hor = placingZone.remain_hor - (placingZone.eu_count_hor * placingZone.eu_wid_numeric);		// 가로 방향 나머지
	//	placingZone.eu_count_ver = static_cast<short>(placingZone.remain_ver / placingZone.eu_hei_numeric);				// 세로 방향 개수
	//	placingZone.remain_ver = placingZone.remain_ver - (placingZone.eu_count_ver * placingZone.eu_hei_numeric);		// 세로 방향 나머지
	//} else {
	//	placingZone.eu_count_hor = static_cast<short>(placingZone.remain_hor / placingZone.eu_hei_numeric);				// 가로 방향 개수
	//	placingZone.remain_hor = placingZone.remain_hor - (placingZone.eu_count_hor * placingZone.eu_hei_numeric);		// 가로 방향 나머지
	//	placingZone.eu_count_ver = static_cast<short>(placingZone.remain_ver / placingZone.eu_wid_numeric);				// 세로 방향 개수
	//	placingZone.remain_ver = placingZone.remain_ver - (placingZone.eu_count_ver * placingZone.eu_wid_numeric);		// 세로 방향 나머지
	//}

	//placingZone.remain_hor_updated = placingZone.remain_hor;
	//placingZone.remain_ver_updated = placingZone.remain_ver;

	//// 유로폼 시작 좌표 설정
	//if (placingZone.eu_ori.compare (std::string ("벽세우기")) == 0) {
	//	placingZone.formArrayWidth = (placingZone.eu_count_hor * placingZone.eu_wid_numeric);
	//	placingZone.formArrayHeight = (placingZone.eu_count_ver * placingZone.eu_hei_numeric);
	//} else {
	//	placingZone.formArrayWidth = (placingZone.eu_count_hor * placingZone.eu_hei_numeric);
	//	placingZone.formArrayHeight = (placingZone.eu_count_ver * placingZone.eu_wid_numeric);
	//}
	//placingZone.leftBottomX = (placingZone.outerLeft + placingZone.outerRight) / 2 - (placingZone.formArrayWidth / 2);
	//placingZone.leftBottomY = (placingZone.outerTop + placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2);
	//placingZone.leftBottomZ = placingZone.level;
	//
	//// 위의 점을 unrotated 위치로 업데이트
	//rotatedPoint.x = placingZone.leftBottomX;
	//rotatedPoint.y = placingZone.leftBottomY;
	//rotatedPoint.z = placingZone.leftBottomZ;
	//unrotatedPoint = getUnrotatedPoint (rotatedPoint, point1, ang);
	//placingZone.leftBottomX = unrotatedPoint.x;
	//placingZone.leftBottomY = unrotatedPoint.y;
	//placingZone.leftBottomZ = unrotatedPoint.z;

	//// placingZone의 Cell 정보 초기화
	//initCellsForSlabBottom (&placingZone);

	//// 배치를 위한 정보 입력
	//firstPlacingSettingsForSlabBottom (&placingZone);

	//// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정하거나 보강 목재를 삽입합니다.
	//clickedOKButton = false;
	//clickedPrevButton = false;
	//result = DGBlankModalDialog (185, 290, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomPlacerHandler2, 0);

	//// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	//if (clickedPrevButton == true)
	//	goto FIRST;

	//// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
	//if (clickedOKButton == false)
	//	return err;

	//// 나머지 영역 채우기 - 합판, 목재
	//err = fillRestAreasForSlabBottom ();

	//// 결과물 전체 그룹화
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




	return	err;
}