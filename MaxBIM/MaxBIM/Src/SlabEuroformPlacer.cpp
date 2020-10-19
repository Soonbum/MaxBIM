#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabEuroformPlacer.hpp"

using namespace slabBottomPlacerDG;

static SlabPlacingZone	placingZone;			// 기본 슬래브 하부 영역 정보
static InfoSlab			infoSlab;				// 슬래브 객체 정보
static short			layerInd_Euroform;		// 레이어 번호: 유로폼
static short			layerInd_Plywood;		// 레이어 번호: 합판
static short			layerInd_Wood;			// 레이어 번호: 목재


// 2번 메뉴: 슬래브 하부에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy;
	double		dx, dy, ang;

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
	InfoMorphForSlab		infoMorph;

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
	double			workLevel_morph;

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
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 슬래브 (1개), 슬래브 하부를 덮는 모프 (1개)", true);
		return err;
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
	
	infoSlab.floorInd		= elem.header.floorInd;

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
	nNodes = coords.GetSize ();

	// 하단 점 2개를 클릭
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("Left Bottom 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("Right Bottom 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;

	// 두 점 간의 각도를 구함
	dx = point2.x - point1.x;
	dy = point2.y - point1.y;
	ang = RadToDegree (atan2 (dy, dx));

	// 폴리곤의 회전각도를 구함
	if ((ang > 0.0) && (ang < 90.0))	ang = ang;			// 0도 초과 90도 미만이면,
	if ((ang > 90.0) && (ang < 180.0))	ang = ang - 90.0;	// 90도 초과 180도 미만이면,
	if ((ang > 180.0) && (ang < 270.0))	ang = ang - 180.0;	// 180도 초과 270도 미만이면,
	if (ang > 270.0)					ang = ang - 270.0;	// 270도 초과이면,

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

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_morph = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoSlab.floorInd) {
			workLevel_morph = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);
	
	// 영역 정보의 고도 정보 수정
	// ...

	// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32511, ACAPI_GetOwnResModule (), slabBottomPlacerHandler1, 0);

	// 문자열로 된 유로폼의 너비/높이를 실수형으로도 저장
	placingZone.eu_wid_numeric = atof (placingZone.eu_wid.c_str ()) / 1000.0;
	placingZone.eu_hei_numeric = atof (placingZone.eu_hei.c_str ()) / 1000.0;

	// 그 외 영역 정보를 지정함
	placingZone.ang = DegreeToRad (ang);
	placingZone.level = nodes_sequential [0].z;

	// 최외곽 좌표를 얻음
	placingZone.outerLeft	= nodes_sequential [0].x;
	placingZone.outerRight	= nodes_sequential [0].x;
	placingZone.outerTop	= nodes_sequential [0].y;
	placingZone.outerBottom	= nodes_sequential [0].y;

	for (xx = 1 ; xx < nEntered ; ++xx) {
		if (nodes_sequential [xx].x < placingZone.outerLeft)
			placingZone.outerLeft = nodes_sequential [xx].x;
		if (nodes_sequential [xx].x > placingZone.outerRight)
			placingZone.outerRight = nodes_sequential [xx].x;
		if (nodes_sequential [xx].y > placingZone.outerTop)
			placingZone.outerTop = nodes_sequential [xx].y;
		if (nodes_sequential [xx].y < placingZone.outerBottom)
			placingZone.outerBottom = nodes_sequential [xx].y;
	}

	// 가장 꼭지점 좌표를 임시로 생성함
	outer_leftTop.x		= placingZone.outerLeft;	outer_leftTop.y		= placingZone.outerTop;		outer_leftTop.z		= placingZone.level;
	outer_leftBottom.x	= placingZone.outerLeft;	outer_leftBottom.y	= placingZone.outerBottom;	outer_leftBottom.z	= placingZone.level;
	outer_rightTop.x	= placingZone.outerRight;	outer_rightTop.y	= placingZone.outerTop;		outer_rightTop.z	= placingZone.level;
	outer_rightBottom.x	= placingZone.outerRight;	outer_rightBottom.y	= placingZone.outerBottom;	outer_rightBottom.z	= placingZone.level;

	API_Coord3D	tPoint;
	tPoint.x = (placingZone.outerLeft + placingZone.outerRight) / 2;
	tPoint.y = (placingZone.outerTop + placingZone.outerBottom) / 2;
	tPoint.z = placingZone.level;

	outer_leftTopBelow		= tPoint;
	outer_leftTopSide		= tPoint;
	outer_leftBottomOver	= tPoint;
	outer_leftBottomSide	= tPoint;
	outer_rightTopBelow		= tPoint;
	outer_rightTopSide		= tPoint;
	outer_rightBottomOver	= tPoint;
	outer_rightBottomSide	= tPoint;

	// 가장 꼭지점 좌표의 바로 안쪽에 있는 점의 좌표를 찾아냄
	for (xx = 0 ; xx < nEntered ; ++xx) {
		if ( (!isSamePoint (outer_leftTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_leftBottom) == 1) && (abs (nodes_sequential [xx].x - outer_leftTop.x) < EPS) )
			outer_leftTopBelow = nodes_sequential [xx];

		if ( (!isSamePoint (outer_leftTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_rightTop) == 1) && (abs (nodes_sequential [xx].y - outer_leftTop.y) < EPS) )
			outer_leftTopSide = nodes_sequential [xx];

		if ( (!isSamePoint (outer_leftBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_leftBottom) == 2) && (abs (nodes_sequential [xx].x - outer_leftBottom.x) < EPS) )
			outer_leftBottomOver = nodes_sequential [xx];

		if ( (!isSamePoint (outer_leftBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftBottom, outer_rightBottom) == 1) && (abs (nodes_sequential [xx].y - outer_leftBottom.y) < EPS) )
			outer_leftBottomSide = nodes_sequential [xx];

		if ( (!isSamePoint (outer_rightTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_rightTop, outer_rightBottom) == 1) && (abs (nodes_sequential [xx].x - outer_rightTop.x) < EPS) )
			outer_rightTopBelow = nodes_sequential [xx];

		if ( (!isSamePoint (outer_rightTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_rightTop) == 2) && (abs (nodes_sequential [xx].y - outer_rightTop.y) < EPS) )
			outer_rightTopSide = nodes_sequential [xx];

		if ( (!isSamePoint (outer_rightBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_rightTop, outer_rightBottom) == 2) && (abs (nodes_sequential [xx].x - outer_rightBottom.x) < EPS) )
			outer_rightBottomOver = nodes_sequential [xx];

		if ( (!isSamePoint (outer_rightBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftBottom, outer_rightBottom) == 2) && (abs (nodes_sequential [xx].y - outer_rightBottom.y) < EPS) )
			outer_rightBottomSide = nodes_sequential [xx];
	}

	// 꺾인 코너 혹은 일반 코너의 좌표를 찾아냄
	for (xx = 0 ; xx < nEntered ; ++xx) {
		// 좌상단 코너
		if ( (isSamePoint (outer_leftTopBelow, tPoint)) && (isSamePoint (outer_leftTopSide, tPoint)) )
			placingZone.corner_leftTop = outer_leftTop;
		else {
			placingZone.corner_leftTop.x = outer_leftTopSide.x;
			placingZone.corner_leftTop.y = outer_leftTopBelow.y;
			placingZone.corner_leftTop.z = placingZone.level;
		}
		
		// 좌하단 코너
		if ( (isSamePoint (outer_leftBottomOver, tPoint)) && (isSamePoint (outer_leftBottomSide, tPoint)) )
			placingZone.corner_leftBottom = outer_leftBottom;
		else {
			placingZone.corner_leftBottom.x = outer_leftBottomSide.x;
			placingZone.corner_leftBottom.y = outer_leftBottomOver.y;
			placingZone.corner_leftBottom.z = placingZone.level;
		}

		// 우상단 코너
		if ( (isSamePoint (outer_rightTopBelow, tPoint)) && (isSamePoint (outer_rightTopSide, tPoint)) )
			placingZone.corner_rightTop = outer_rightTop;
		else {
			placingZone.corner_rightTop.x = outer_rightTopSide.x;
			placingZone.corner_rightTop.y = outer_rightTopBelow.y;
			placingZone.corner_rightTop.z = placingZone.level;
		}

		// 우하단 코너
		if ( (isSamePoint (outer_rightBottomOver, tPoint)) && (isSamePoint (outer_rightBottomSide, tPoint)) )
			placingZone.corner_rightBottom = outer_rightBottom;
		else {
			placingZone.corner_rightBottom.x = outer_rightBottomSide.x;
			placingZone.corner_rightBottom.y = outer_rightBottomOver.y;
			placingZone.corner_rightBottom.z = placingZone.level;
		}
	}

	// 코너 안쪽의 좌표를 구함
	if (placingZone.corner_leftTop.x < placingZone.corner_leftBottom.x)
		placingZone.innerLeft = placingZone.corner_leftBottom.x;
	else
		placingZone.innerLeft = placingZone.corner_leftTop.x;

	if (placingZone.corner_rightTop.x < placingZone.corner_rightBottom.x)
		placingZone.innerRight = placingZone.corner_rightTop.x;
	else
		placingZone.innerRight = placingZone.corner_rightBottom.x;

	if (placingZone.corner_leftTop.y < placingZone.corner_rightTop.y)
		placingZone.innerTop = placingZone.corner_leftTop.y;
	else
		placingZone.innerTop = placingZone.corner_rightTop.y;

	if (placingZone.corner_leftBottom.y < placingZone.corner_rightBottom.y)
		placingZone.innerBottom = placingZone.corner_rightBottom.y;
	else
		placingZone.innerBottom = placingZone.corner_leftBottom.y;

	// 영역 안쪽의 너비와 높이를 구함
	placingZone.innerWidth = placingZone.innerRight - placingZone.innerLeft;
	placingZone.innerHeight = placingZone.innerTop - placingZone.innerBottom;

	// 남은 길이 초기화
	placingZone.remain_hor = placingZone.innerWidth;
	placingZone.remain_ver = placingZone.innerHeight;

	// 유로폼 가로/세로 방향 개수 세기
	placingZone.eu_count_hor = 0;
	placingZone.eu_count_ver = 0;

	if (placingZone.eu_ori.compare (std::string ("벽세우기")) == 0) {
		placingZone.eu_count_hor = static_cast<short>(placingZone.remain_hor / placingZone.eu_wid_numeric);				// 가로 방향 개수
		placingZone.remain_hor = placingZone.remain_hor - (placingZone.eu_count_hor * placingZone.eu_wid_numeric);		// 가로 방향 나머지
		placingZone.eu_count_ver = static_cast<short>(placingZone.remain_ver / placingZone.eu_hei_numeric);				// 세로 방향 개수
		placingZone.remain_ver = placingZone.remain_ver - (placingZone.eu_count_ver * placingZone.eu_hei_numeric);		// 세로 방향 나머지
	} else {
		placingZone.eu_count_hor = static_cast<short>(placingZone.remain_hor / placingZone.eu_hei_numeric);				// 가로 방향 개수
		placingZone.remain_hor = placingZone.remain_hor - (placingZone.eu_count_hor * placingZone.eu_hei_numeric);		// 가로 방향 나머지
		placingZone.eu_count_ver = static_cast<short>(placingZone.remain_ver / placingZone.eu_wid_numeric);				// 세로 방향 개수
		placingZone.remain_ver = placingZone.remain_ver - (placingZone.eu_count_ver * placingZone.eu_wid_numeric);		// 세로 방향 나머지
	}

	placingZone.remain_hor_updated = placingZone.remain_hor;
	placingZone.remain_ver_updated = placingZone.remain_ver;

	// placingZone의 Cell 정보 초기화
	initCellsForSlabBottom (&placingZone);

	// !!! 체크포인트

	// 배치를 위한 정보 입력
	firstPlacingSettingsForSlabBottom (&placingZone);

	// [DIALOG] 2번째 다이얼로그에서 ... 유로폼/인코너 배치를 수정하거나 휠러스페이서를 삽입합니다.
	//clickedOKButton = false;
	//result = DGBlankModalDialog (185, 250, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomPlacerHandler2, 0);

	//if (clickedOKButton == false)
	//	return err;

	// 1. 유로폼의 가로/세로 수량을 계산한다. (가장자리 너비는 150 이상 300 미만)
	// 2. 합판 설치: 부재당 최대 길이 1800 이하
	// 3. 안쪽 목재 설치 (두께 50, 너비 80): 부재당 최대 길이 1800 이하
	// 4. 바깥쪽 목재 설치 (두께 40, 너비 50): 부재당 최대 길이 1800 이하

	/*
		*** 사용 부재(3가지): 유로폼(회전X: 0도, 벽세우기로 고정), 합판(각도: 90도), 목재(설치방향: 바닥눕히기)
		1. 사용자 입력 (2차)
			- 유로폼 셀은 전부 붙어 있어도 됨
			- 목재 보강재 위치 셀 버튼도 있어야 함
			- 타이틀: 가로/세로 채우기
			- 버튼: 1. 남은 길이 확인 // 2. 배  치 // 3. 마무리
			- 배치 버튼을 보여줌 (x번행 y번열 버튼의 폼 크기를 변경하면? - 너비를 바꾸면 y열 전체에 영향을 줌, 높이를 바꾸면 x행 전체에 영향을 줌)
			- 남은 가로/세로 길이: 표시만 함 (양쪽 다 합친 것): 권장 남은 길이 표시 (150~300mm), 권장 길이인지 아닌지 글꼴로 표시
			- 사방 합판 너비를 조정할 수 있어야 함 (위/아래 배분, 왼쪽/오른쪽 배분 - 사용자가 직접.. 그에 따라 유로폼 전체 배열이 이동됨)
			- 배치 버튼 사방에는 버튼 사이마다 보강 목재를 댈지 여부를 선택할 수 있어야 함 (버튼 □■)
		2. 최종 배치
			- (1) 유로폼은 모프 센터를 기준으로 배치할 것
			- (2) 합판(11.5T, 제작틀 Off)을 모프에 맞게 배치하되, 겹치는 부분은 모깎기가 필요함! (난이도 높음)
			- (3) 안쪽에 목재를 댈 것 (Z방향 두께는 50이어야 함, 너비는 80)
			- (4) 목재 보강재도 댈 것 (안쪽 목재와 두께, 너비는 동일 - 길이는 사용자 설정을 따름)
	*/

	return	err;
}

// aPoint가 bPoint와 같은 점인지 확인함
bool	isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint)
{
	if ( (abs (aPoint.x - bPoint.x) < EPS) && (abs (aPoint.y - bPoint.y) < EPS) && (abs (aPoint.z - bPoint.z) < EPS) &&
		(abs (bPoint.x - aPoint.x) < EPS) && (abs (bPoint.y - aPoint.y) < EPS) && (abs (bPoint.z - aPoint.z) < EPS) ) {
		return true;
	} else
		return false;
}

// aPoint가 pointList에 보관이 되었는지 확인함
bool	isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd)
{
	short	xx;

	for (xx = startInd ; xx <= endInd ; ++xx) {
		// 모든 좌표 값이 일치할 경우, 이미 포함된 좌표 값이라고 인정함
		if ( (abs (aPoint.x - pointList [xx].x) < EPS) && (abs (aPoint.y - pointList [xx].y) < EPS) && (abs (aPoint.z - pointList [xx].z) < EPS) &&
			(abs (pointList [xx].x - aPoint.x) < EPS) && (abs (pointList [xx].y - aPoint.y) < EPS) && (abs (pointList [xx].z - aPoint.z) < EPS) ) {
			return true;
		}
	}

	return false;
}

// nextPoint가 curPoint의 다음 점입니까?
bool	isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint)
{
	bool	cond1 = false;
	bool	cond2_1 = false;
	bool	cond2_2 = false;

	// curPoint와 nextPoint가 같은 Z값을 갖는가?
	if ( (abs (curPoint.z - nextPoint.z) < EPS) && (abs (nextPoint.z - curPoint.z) < EPS) )
		cond1 = true;

	// 예전 점과 현재 점이 Y축 상에 있을 경우, 현재 점과 다음 점은 X축 상에 있어야 하고, 현재 점과 다음 점 간에는 X값의 차이가 있어야 함
	if ((abs (curPoint.x - prevPoint.x) < EPS) && (abs (prevPoint.x - curPoint.x) < EPS) &&
		(abs (curPoint.y - nextPoint.y) < EPS) && (abs (nextPoint.y - curPoint.y) < EPS) &&
		((abs (curPoint.x - nextPoint.x) > EPS) || (abs (nextPoint.x - curPoint.x) > EPS)))
		cond2_1 = true;

	// 예전 점과 현재 점이 X축 상에 있을 경우, 현재 점과 다음 점은 Y축 상에 있어야 하고, 현재 점과 다음 점 간에는 Y값의 차이가 있어야 함
	if ((abs (curPoint.y - prevPoint.y) < EPS) && (abs (prevPoint.y - curPoint.y) < EPS) &&
		(abs (curPoint.x - nextPoint.x) < EPS) && (abs (nextPoint.x - curPoint.x) < EPS) &&
		((abs (curPoint.y - nextPoint.y) > EPS) || (abs (nextPoint.y - curPoint.y) > EPS)))
		cond2_2 = true;

	// 같은 Z값이면서 동일 축 상의 떨어진 거리의 점인 경우
	if (cond1 && (cond2_1 || cond2_2))
		return true;
	else
		return false;
}

// curPoint에 가까운 점이 p1, p2 중 어떤 점입니까?
short	moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2)
{
	double dist1, dist2;

	dist1 = GetDistance (curPoint, p1);
	dist2 = GetDistance (curPoint, p2);

	// curPoint와 p1가 더 가까우면 1 리턴
	if ((dist2 - dist1) > EPS)	return 1;
	
	// curPoint와 p2가 더 가까우면 2 리턴
	if ((dist1 - dist2) > EPS)	return 2;

	// 그 외에는 0 리턴
	return 0;
}

// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴)
API_Coord3D		getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang)
{
	API_Coord3D		unrotatedPoint;

	// 아래에서 unrotatedPoint를 구해야 함

	// 원래 식
	//rotatedPoint.x = axisPoint.x + ((unrotatedPoint.x - axisPoint.x)*cos(DegreeToRad (-ang)) - (unrotatedPoint.y - axisPoint.y)*sin(DegreeToRad (-ang)));
	//rotatedPoint.y = axisPoint.y + ((unrotatedPoint.x - axisPoint.x)*sin(DegreeToRad (-ang)) + (unrotatedPoint.y - axisPoint.y)*cos(DegreeToRad (-ang)));

	// 도출 중...
	//rotatedPoint.x - axisPoint.x = ((unrotatedPoint.x - axisPoint.x)*cos(DegreeToRad (-ang)) - (unrotatedPoint.y - axisPoint.y)*sin(DegreeToRad (-ang)));
	//rotatedPoint.y - axisPoint.y = ((unrotatedPoint.x - axisPoint.x)*sin(DegreeToRad (-ang)) + (unrotatedPoint.y - axisPoint.y)*cos(DegreeToRad (-ang)));

	// ...

	unrotatedPoint.z = rotatedPoint.z;

	return unrotatedPoint;
}

// Cell 배열을 초기화함
void	initCellsForSlabBottom (SlabPlacingZone* placingZone)
{
	short xx, yy;

	for (xx = 0 ; xx < 50 ; ++xx)
		for (yy = 0 ; yy < 50 ; ++yy) {
			placingZone->cells [xx][yy].objType = NONE;
			placingZone->cells [xx][yy].ang = 0.0;
			placingZone->cells [xx][yy].horLen = 0.0;
			placingZone->cells [xx][yy].verLen = 0.0;
			placingZone->cells [xx][yy].leftBottomX = 0.0;
			placingZone->cells [xx][yy].leftBottomY = 0.0;
			placingZone->cells [xx][yy].leftBottomZ = 0.0;
		}
}

// 1차 배치: 유로폼
void	firstPlacingSettingsForSlabBottom (SlabPlacingZone* placingZone)
{
	// ...
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	alignPlacingZoneForSlabBottom (SlabPlacingZone* target_zone)
{
	// ...
}

/*
// 해당 셀 정보를 기반으로 라이브러리 배치
API_Guid	placeLibPartForSlabBottom (CellForSlab objInfo)
{
}

// 유로폼을 채운 후 자투리 공간 채우기
GSErrCode	fillRestAreasForSlabBottom (void)
{
}
*/

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK slabBottomPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "슬래브 하부에 배치");

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

			// 라벨: 설치방향
			DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION, "설치방향");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");

			// 라벨: 레이어 - 유로폼
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");

			// 라벨: 레이어 - 목재
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "목재");

			// 라벨: 레이어 - 합판
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "합판");

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

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 유로폼 너비, 높이, 방향
					placingZone.eu_wid = DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_EUROFORM_WIDTH))).ToCStr ().Get ();
					placingZone.eu_hei = DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_EUROFORM_HEIGHT))).ToCStr ().Get ();
					placingZone.eu_ori = DGPopUpGetItemText (dialogID, POPUP_EUROFORM_ORIENTATION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_EUROFORM_ORIENTATION))).ToCStr ().Get ();

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);

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
