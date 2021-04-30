#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabTableformPlacer.hpp"

using namespace slabTableformPlacerDG;

static SlabTableformPlacingZone		placingZone;	// 기본 슬래브 하부 영역 정보
static InfoSlab		infoSlab;					// 슬래브 객체 정보
static short		clickedBtnItemIdx;			// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
static bool			clickedOKButton;			// OK 버튼을 눌렀습니까?
static bool			clickedExcludeRestButton;	// 자투리 제외 버튼을 눌렀습니까?
static bool			clickedPrevButton;			// 이전 버튼을 눌렀습니까?
static short		layerInd_SlabTableform;		// 레이어 번호: 슬래브 테이블폼
static short		layerInd_Plywood;			// 레이어 번호: 합판
static short		layerInd_Wood;				// 레이어 번호: 목재
static short		layerInd_Profile;			// 레이어 번호: KS프로파일
static short		layerInd_Fittings;			// 레이어 번호: 결합철물
static short		itemInitIdx = GRIDBUTTON_IDX_START;		// 그리드 버튼 항목 인덱스 시작 번호
static API_Coord3D		firstClickPoint;		// 1번째로 클릭한 점
static short			TButtonStartIdx = 0;	// T 버튼 시작 인덱스
static short			BButtonStartIdx = 0;	// B 버튼 시작 인덱스
static short			LButtonStartIdx = 0;	// L 버튼 시작 인덱스
static short			RButtonStartIdx = 0;	// R 버튼 시작 인덱스
static GS::Array<API_Guid>	elemList;			// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함


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
	InfoMorphForSlabTableform	infoMorph;

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
	
	infoSlab.floorInd		= elem.header.floorInd;
	infoSlab.offsetFromTop	= elem.slab.offsetFromTop;
	infoSlab.thickness		= elem.slab.thickness;
	infoSlab.level			= elem.slab.level;

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
	firstClickPoint = point1;

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
	placingZone.ang = DegreeToRad (ang);
	placingZone.level = nodes_sequential [0].z;

FIRST:

	// [DIALOG] 1번째 다이얼로그에서 테이블폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32518, ACAPI_GetOwnResModule (), slabBottomTableformPlacerHandler1, 0);

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_slab = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoSlab.floorInd) {
			workLevel_slab = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);
	
	// 영역 정보의 고도 정보 수정
	placingZone.level = infoSlab.level + infoSlab.offsetFromTop - infoSlab.thickness - placingZone.gap;

	// 문자열로 된 유로폼의 너비/높이를 실수형으로도 저장
	placingZone.tb_wid_numeric = atof (placingZone.tb_wid.c_str ()) / 1000.0;
	placingZone.tb_hei_numeric = atof (placingZone.tb_hei.c_str ()) / 1000.0;

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
	placingZone.remain_hor = placingZone.outerRight - placingZone.outerLeft;
	placingZone.remain_ver = placingZone.outerTop - placingZone.outerBottom;

	// 테이블폼 가로/세로 방향 개수 세기
	placingZone.tb_count_hor = 0;
	placingZone.tb_count_ver = 0;

	if (placingZone.tb_ori.compare (std::string ("Horizontal")) == 0) {
		placingZone.tb_count_hor = static_cast<short>(placingZone.remain_hor / placingZone.tb_wid_numeric);				// 가로 방향 개수
		placingZone.remain_hor = placingZone.remain_hor - (placingZone.tb_count_hor * placingZone.tb_wid_numeric);		// 가로 방향 나머지
		placingZone.tb_count_ver = static_cast<short>(placingZone.remain_ver / placingZone.tb_hei_numeric);				// 세로 방향 개수
		placingZone.remain_ver = placingZone.remain_ver - (placingZone.tb_count_ver * placingZone.tb_hei_numeric);		// 세로 방향 나머지
	} else {
		placingZone.tb_count_hor = static_cast<short>(placingZone.remain_hor / placingZone.tb_hei_numeric);				// 가로 방향 개수
		placingZone.remain_hor = placingZone.remain_hor - (placingZone.tb_count_hor * placingZone.tb_hei_numeric);		// 가로 방향 나머지
		placingZone.tb_count_ver = static_cast<short>(placingZone.remain_ver / placingZone.tb_wid_numeric);				// 세로 방향 개수
		placingZone.remain_ver = placingZone.remain_ver - (placingZone.tb_count_ver * placingZone.tb_wid_numeric);		// 세로 방향 나머지
	}

	placingZone.remain_hor_updated = placingZone.remain_hor;
	placingZone.remain_ver_updated = placingZone.remain_ver;

	// 유로폼 시작 좌표 설정
	if (placingZone.tb_ori.compare (std::string ("Horizontal")) == 0) {
		placingZone.formArrayWidth = (placingZone.tb_count_hor * placingZone.tb_wid_numeric);
		placingZone.formArrayHeight = (placingZone.tb_count_ver * placingZone.tb_hei_numeric);
	} else {
		placingZone.formArrayWidth = (placingZone.tb_count_hor * placingZone.tb_hei_numeric);
		placingZone.formArrayHeight = (placingZone.tb_count_ver * placingZone.tb_wid_numeric);
	}
	placingZone.leftBottomX = (placingZone.outerLeft + placingZone.outerRight) / 2 - (placingZone.formArrayWidth / 2);
	placingZone.leftBottomY = (placingZone.outerTop + placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2);
	placingZone.leftBottomZ = placingZone.level;
	
	// 위의 점을 unrotated 위치로 업데이트
	rotatedPoint.x = placingZone.leftBottomX;
	rotatedPoint.y = placingZone.leftBottomY;
	rotatedPoint.z = placingZone.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, point1, ang);
	placingZone.leftBottomX = unrotatedPoint.x;
	placingZone.leftBottomY = unrotatedPoint.y;
	placingZone.leftBottomZ = unrotatedPoint.z;
	placingZone.upMove = 0.0;
	placingZone.leftMove = 0.0;

	// placingZone의 Cell 정보 초기화
	placingZone.initCells (&placingZone);

	// 배치를 위한 정보 입력
	placingZone.firstPlacingSettings (&placingZone);

	// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정하거나 보강 목재를 삽입합니다.
	clickedOKButton = false;
	clickedExcludeRestButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (185, 290, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomTableformPlacerHandler2, 0);

	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST;

	// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
	if (clickedOKButton == false)
		return err;

	// 나머지 영역 채우기 - 합판, 목재
	if (clickedExcludeRestButton == false) {
		placingZone.woodWidth = 0.080;	// 목재 기본 너비
		result = DGBlankModalDialog (250, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomTableformPlacerHandler4, 0);	// [DIALOG] 4번째 다이얼로그에서 목재 너비를 변경할지 물어봄
		err = placingZone.fillRestAreas ();
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
void	SlabTableformPlacingZone::initCells (SlabTableformPlacingZone* placingZone)
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
void	SlabTableformPlacingZone::firstPlacingSettings (SlabTableformPlacingZone* placingZone)
{
	short			xx, yy;
	API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	// 유로폼 설정
	for (xx = 0 ; xx < placingZone->tb_count_ver ; ++xx) {
		for (yy = 0 ; yy < placingZone->tb_count_hor ; ++yy) {

			placingZone->cells [xx][yy].objType = SLAB_TABLEFORM;
			placingZone->cells [xx][yy].ang = placingZone->ang;

			if (placingZone->tb_ori.compare (std::string ("Horizontal")) == 0) {
				placingZone->cells [xx][yy].libPart.tableform.direction = true;
				placingZone->cells [xx][yy].horLen = placingZone->tb_wid_numeric;
				placingZone->cells [xx][yy].verLen = placingZone->tb_hei_numeric;
			} else {
				placingZone->cells [xx][yy].libPart.tableform.direction = false;
				placingZone->cells [xx][yy].horLen = placingZone->tb_hei_numeric;
				placingZone->cells [xx][yy].verLen = placingZone->tb_wid_numeric;
			}

			// 여기부터는 셀 위치 지정 루틴
			placingZone->cells [xx][yy].leftBottomX = placingZone->leftBottomX + (placingZone->cells [xx][yy].horLen * yy);
			placingZone->cells [xx][yy].leftBottomY = placingZone->leftBottomY - (placingZone->cells [xx][yy].verLen * xx);
			placingZone->cells [xx][yy].leftBottomZ = placingZone->leftBottomZ;

			axisPoint.x = placingZone->leftBottomX;
			axisPoint.y = placingZone->leftBottomY;
			axisPoint.z = placingZone->leftBottomZ;

			rotatedPoint.x = placingZone->cells [xx][yy].leftBottomX;
			rotatedPoint.y = placingZone->cells [xx][yy].leftBottomY;
			rotatedPoint.z = placingZone->cells [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			placingZone->cells [xx][yy].leftBottomX = unrotatedPoint.x;
			placingZone->cells [xx][yy].leftBottomY = unrotatedPoint.y;
			placingZone->cells [xx][yy].leftBottomZ = unrotatedPoint.z;
		}
	}
}

// 해당 셀과 동일한 행에 있는 다른 셀들의 타입 및 높이를 조정함 (너비는 변경하지 않음)
void	SlabTableformPlacingZone::adjustOtherCellsInSameRow (SlabTableformPlacingZone* target_zone, short row, short col)
{
	short	xx;

	// 모든 열들에 적용
	for (xx = 0 ; xx < target_zone->tb_count_hor ; ++xx) {
		if (xx == col) continue;

		// 해당 셀이 NONE이면, 나머지 셀들의 타입도 NONE
		if (target_zone->cells [row][col].objType == NONE) {

			target_zone->cells [row][xx].objType = NONE;

		// 해당 셀이 SLAB_TABLEFORM이면, 나머지 셀들의 타입도 SLAB_TABLEFORM
		} else if (target_zone->cells [row][col].objType == SLAB_TABLEFORM) {

			target_zone->cells [row][xx].objType = SLAB_TABLEFORM;

			if (target_zone->cells [row][col].libPart.tableform.direction == true) {

					target_zone->cells [row][xx].verLen = target_zone->cells [row][col].verLen;
					target_zone->cells [row][xx].libPart.tableform.verLen = target_zone->cells [row][col].verLen;
			
			} else {

					target_zone->cells [row][xx].verLen = target_zone->cells [row][col].verLen;
					target_zone->cells [row][xx].libPart.tableform.horLen = target_zone->cells [row][col].verLen;
			}

		} else if (target_zone->cells [row][col].objType == PLYWOOD) {

			target_zone->cells [row][xx].objType = PLYWOOD;

			target_zone->cells [row][xx].verLen = target_zone->cells [row][col].verLen;
			target_zone->cells [row][xx].libPart.plywood.p_wid = target_zone->cells [row][xx].horLen;
			target_zone->cells [row][xx].libPart.plywood.p_leng = target_zone->cells [row][xx].verLen;
		}
	}
}

// 해당 셀과 동일한 열에 있는 다른 셀들의 타입 및 너비를 조정함 (높이는 변경하지 않음)
void	SlabTableformPlacingZone::adjustOtherCellsInSameCol (SlabTableformPlacingZone* target_zone, short row, short col)
{
	short	xx;

	// 모든 행들에 적용
	for (xx = 0 ; xx < target_zone->tb_count_ver ; ++xx) {
		if (xx == row) continue;

		// 해당 셀이 NONE이면, 나머지 셀들의 타입도 NONE
		if (target_zone->cells [row][col].objType == NONE) {

			target_zone->cells [xx][col].objType = NONE;

		// 해당 셀이 SLAB_TABLEFORM이면, 나머지 셀들의 타입도 SLAB_TABLEFORM
		} else if (target_zone->cells [row][col].objType == SLAB_TABLEFORM) {

			target_zone->cells [xx][col].objType = SLAB_TABLEFORM;

			if (target_zone->cells [row][col].libPart.tableform.direction == true) {

				target_zone->cells [xx][col].horLen = target_zone->cells [row][col].horLen;
				target_zone->cells [xx][col].libPart.tableform.horLen = target_zone->cells [row][col].horLen;

			} else {

				target_zone->cells [xx][col].horLen = target_zone->cells [row][col].horLen;
				target_zone->cells [xx][col].libPart.tableform.verLen = target_zone->cells [row][col].horLen;
			}

		} else if (target_zone->cells [row][col].objType == PLYWOOD) {

			target_zone->cells [xx][col].objType = PLYWOOD;

			target_zone->cells [xx][col].horLen = target_zone->cells [row][col].horLen;
			target_zone->cells [xx][col].libPart.plywood.p_wid = target_zone->cells [xx][col].horLen;
			target_zone->cells [xx][col].libPart.plywood.p_leng = target_zone->cells [xx][col].verLen;
		}
	}
}

// 새로운 행을 추가함 (행 하나를 늘리고 추가된 행에 마지막 행 정보 복사)
void	SlabTableformPlacingZone::addNewRow (SlabTableformPlacingZone* target_zone)
{
	short	xx;

	// 새로운 행 추가
	target_zone->tb_count_ver ++;

	for (xx = 0 ; xx < target_zone->tb_count_hor ; ++xx)
		target_zone->cells [target_zone->tb_count_ver - 1][xx] = target_zone->cells [target_zone->tb_count_ver - 2][xx];
}

// 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void	SlabTableformPlacingZone::addNewCol (SlabTableformPlacingZone* target_zone)
{
	short	xx;

	// 새로운 열 추가
	target_zone->tb_count_hor ++;

	for (xx = 0 ; xx < target_zone->tb_count_ver ; ++xx)
		target_zone->cells [xx][target_zone->tb_count_hor - 1] = target_zone->cells [xx][target_zone->tb_count_hor - 2];
}

// 마지막 행을 삭제함
void	SlabTableformPlacingZone::delLastRow (SlabTableformPlacingZone* target_zone)
{
	target_zone->tb_count_ver --;
}

// 마지막 열을 삭제함
void	SlabTableformPlacingZone::delLastCol (SlabTableformPlacingZone* target_zone)
{
	target_zone->tb_count_hor --;
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	SlabTableformPlacingZone::alignPlacingZone (SlabTableformPlacingZone* target_zone)
{
	short			xx, yy, zz;
	double			dist_horizontal;
	double			dist_vertical;
	double			total_length;
	API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	// 영역 정보에서 남은 거리 관련 항목들이 업데이트됨
	target_zone->remain_hor_updated = (placingZone.outerRight - placingZone.outerLeft);
	target_zone->remain_ver_updated = (placingZone.outerTop - placingZone.outerBottom);

	// 가로 방향 남은 길이: 각 셀의 너비만큼 차감
	total_length = 0.0;
	for (xx = 0 ; xx < target_zone->tb_count_ver ; ++xx) {
		for (yy = 0 ; yy < target_zone->tb_count_hor ; ++yy)
			if (target_zone->cells [xx][yy].objType != NONE)
				total_length += target_zone->cells [xx][yy].horLen;
		
		// 만약 빈 줄이 아니면 루프 탈출
		if (abs (total_length) > EPS)
			xx = target_zone->tb_count_ver;
	}
	target_zone->formArrayWidth = total_length;
	target_zone->remain_hor_updated -= total_length;

	// 세로 방향 남은 길이: 각 셀의 높이만큼 차감
	total_length = 0.0;
	for (xx = 0 ; xx < target_zone->tb_count_hor ; ++xx) {
		for (yy = 0 ; yy < target_zone->tb_count_ver ; ++yy)
			if (target_zone->cells [yy][xx].objType != NONE)
				total_length += target_zone->cells [yy][xx].verLen;

		// 만약 빈 줄이 아니면 루프 탈출
		if (abs (total_length) > EPS)
			xx = target_zone->tb_count_hor;
	}
	target_zone->formArrayHeight = total_length;
	target_zone->remain_ver_updated -= total_length;

	// 전체 폼 너비/높이를 확인한 뒤 시작 좌표를 옮길 것
	target_zone->leftBottomX = (target_zone->outerLeft + target_zone->outerRight) / 2 - (target_zone->formArrayWidth / 2);
	target_zone->leftBottomY = (target_zone->outerTop + target_zone->outerBottom) / 2 + (target_zone->formArrayHeight / 2);
	target_zone->leftBottomZ = target_zone->level;
	
	// 위의 점을 unrotated 위치로 업데이트
	rotatedPoint.x = target_zone->leftBottomX - target_zone->leftMove;
	rotatedPoint.y = target_zone->leftBottomY - target_zone->upMove;
	rotatedPoint.z = target_zone->leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, firstClickPoint, RadToDegree (target_zone->ang));
	target_zone->leftBottomX = unrotatedPoint.x;
	target_zone->leftBottomY = unrotatedPoint.y;
	target_zone->leftBottomZ = unrotatedPoint.z;

	// 각 Cell마다 위치 및 각도 정보가 업데이트됨
	for (xx = 0 ; xx < target_zone->tb_count_ver ; ++xx) {
		for (yy = 0 ; yy < target_zone->tb_count_hor ; ++yy) {

			dist_horizontal = 0.0;
			dist_vertical = 0.0;

			// 이전 X 방향 누적 거리를 구함
			for (zz = 0 ; zz < yy ; ++zz) {
				if (target_zone->cells [xx][zz].objType != NONE)
					dist_horizontal += target_zone->cells [xx][zz].horLen;
			}

			// 이전 Y 방향 누적 거리를 구함
			for (zz = 0 ; zz < xx ; ++zz) {
				if (target_zone->cells [zz][yy].objType != NONE)
					dist_vertical += target_zone->cells [zz][yy].verLen;
			}

			// 각 셀을 밀착시키기 위해 위치 값을 변경함
			target_zone->cells [xx][yy].leftBottomX = target_zone->leftBottomX + dist_horizontal;
			target_zone->cells [xx][yy].leftBottomY = target_zone->leftBottomY - dist_vertical;
			target_zone->cells [xx][yy].leftBottomZ = target_zone->leftBottomZ;

			axisPoint.x = target_zone->leftBottomX;
			axisPoint.y = target_zone->leftBottomY;
			axisPoint.z = target_zone->leftBottomZ;

			rotatedPoint.x = target_zone->cells [xx][yy].leftBottomX;
			rotatedPoint.y = target_zone->cells [xx][yy].leftBottomY;
			rotatedPoint.z = target_zone->cells [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (target_zone->ang));

			target_zone->cells [xx][yy].leftBottomX = unrotatedPoint.x;
			target_zone->cells [xx][yy].leftBottomY = unrotatedPoint.y;
			target_zone->cells [xx][yy].leftBottomZ = unrotatedPoint.z;
		}
	}
}

// 해당 셀 정보를 기반으로 라이브러리 배치
API_Guid	SlabTableformPlacingZone::placeLibPart (CellForSlabTableform objInfo)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const	GS::uchar_t* gsmName = NULL;
	double	aParam;
	double	bParam;
	Int32	addParNum;

	char	tempString [20];

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
	if (objInfo.objType == SLAB_TABLEFORM)	gsmName = L("슬래브 테이블폼 (콘판넬) v1.0.gsm");
	if (objInfo.objType == PLYWOOD)			gsmName = L("합판v1.0.gsm");
	if (objInfo.objType == WOOD)			gsmName = L("목재v1.0.gsm");

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
	element.header.floorInd = infoSlab.floorInd;

	// 슬래브 테이블폼에 C형강을 부착할 때 계산하는 양끝 여백
	double	marginEnds;

	if (objInfo.objType == SLAB_TABLEFORM) {
		element.header.layer = layerInd_SlabTableform;

		// 가로방향 (Horizontal)
		if (objInfo.libPart.tableform.direction == true) {
			// 타입: 가로 길이 x 세로 길이
			sprintf (tempString, "%.0f x %.0f", round (objInfo.horLen, 3) * 1000, round (objInfo.verLen, 3) * 1000);
			element.object.xRatio = objInfo.horLen;
			element.object.yRatio = objInfo.verLen;

			// 이동하여 위치 바로잡기
			element.object.pos.x += ( objInfo.verLen * sin(objInfo.ang) );
			element.object.pos.y -= ( objInfo.verLen * cos(objInfo.ang) );

			// C형강 설치
			CellForSlabTableform	profile;

			profile.objType = PROFILE;
			profile.leftBottomX = objInfo.leftBottomX;
			profile.leftBottomY = objInfo.leftBottomY;
			profile.leftBottomZ = objInfo.leftBottomZ;
			profile.ang = objInfo.ang - DegreeToRad (90.0);
			profile.libPart.profile.angX = DegreeToRad (270.0);
			profile.libPart.profile.angY = DegreeToRad (0.0);
			profile.libPart.profile.iAnchor = 8;
			profile.libPart.profile.len = floor (objInfo.horLen * 10) / 10;
			marginEnds = objInfo.horLen - profile.libPart.profile.len;

			moveIn3D ('x', profile.ang, 0.300 + 0.006 + 0.020, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('y', profile.ang, marginEnds / 2, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('z', profile.ang, -0.0615, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (profile));
			moveIn3D ('x', profile.ang, -(0.300 + 0.006 + 0.020) + objInfo.verLen - (0.300 - 0.006 - 0.020), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (profile));

			profile.ang = objInfo.ang + DegreeToRad (90.0);
			profile.leftBottomX = objInfo.leftBottomX;
			profile.leftBottomY = objInfo.leftBottomY;
			moveIn3D ('x', profile.ang, -objInfo.verLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('y', profile.ang, -objInfo.horLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('x', profile.ang, 0.300 + 0.006 + 0.020, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('y', profile.ang, marginEnds / 2, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (profile));
			moveIn3D ('x', profile.ang, -(0.300 + 0.006 + 0.020) + objInfo.verLen - (0.300 - 0.006 - 0.020), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (profile));

			// 결합철물 (사각와셔활용) 설치
			CellForSlabTableform	fittings;

			fittings.objType = FITTINGS;
			fittings.leftBottomX = objInfo.leftBottomX;
			fittings.leftBottomY = objInfo.leftBottomY;
			fittings.leftBottomZ = objInfo.leftBottomZ;
			fittings.ang = objInfo.ang;
			fittings.libPart.fittings.angX = DegreeToRad (270.0);
			fittings.libPart.fittings.angY = DegreeToRad (0.0);
			fittings.libPart.fittings.bolt_len = 0.150;
			fittings.libPart.fittings.bolt_dia = 0.012;
			fittings.libPart.fittings.bWasher1 = false;
			fittings.libPart.fittings.washer_pos1 = 0.0;
			fittings.libPart.fittings.bWasher2 = true;
			fittings.libPart.fittings.washer_pos2 = 0.0766;
			fittings.libPart.fittings.washer_size = 0.100;
			strncpy (fittings.libPart.fittings.nutType, "육각너트", strlen ("육각너트"));

			moveIn3D ('x', fittings.ang, 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			moveIn3D ('y', fittings.ang, -0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			moveIn3D ('z', fittings.ang, -0.0499, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (fittings));
			moveIn3D ('y', fittings.ang, 0.300 - objInfo.verLen + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (fittings));
			moveIn3D ('x', fittings.ang, -0.328 + objInfo.horLen - 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (fittings));
			moveIn3D ('y', fittings.ang, -0.300 + objInfo.verLen - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (fittings));
			
		// 세로방향 (Vertical)
		} else {
			// 타입: 세로 길이 x 가로 길이
			sprintf (tempString, "%.0f x %.0f", round (objInfo.verLen, 3) * 1000, round (objInfo.horLen, 3) * 1000);
			element.object.xRatio = objInfo.verLen;
			element.object.yRatio = objInfo.horLen;
			
			// 90도 회전된 상태로 배치
			element.object.angle += DegreeToRad (90.0);

			// 이동하여 위치 바로잡기
			element.object.pos.x += ( objInfo.verLen * sin(objInfo.ang) );
			element.object.pos.y -= ( objInfo.verLen * cos(objInfo.ang) );
			element.object.pos.x += ( objInfo.horLen * cos(objInfo.ang) );
			element.object.pos.y += ( objInfo.horLen * sin(objInfo.ang) );

			// C형강 설치
			CellForSlabTableform	profile;

			profile.objType = PROFILE;
			profile.leftBottomX = objInfo.leftBottomX;
			profile.leftBottomY = objInfo.leftBottomY;
			profile.leftBottomZ = objInfo.leftBottomZ;
			profile.ang = objInfo.ang;
			profile.libPart.profile.angX = DegreeToRad (270.0);
			profile.libPart.profile.angY = DegreeToRad (0.0);
			profile.libPart.profile.iAnchor = 8;
			profile.libPart.profile.len = floor (objInfo.verLen * 10) / 10;
			marginEnds = objInfo.verLen - profile.libPart.profile.len;

			moveIn3D ('x', profile.ang, 0.300 + 0.006 + 0.020, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('y', profile.ang, marginEnds / 2 - objInfo.verLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('z', profile.ang, -0.0615, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (profile));
			moveIn3D ('x', profile.ang, -(0.300 + 0.006 + 0.020) + objInfo.horLen - (0.300 - 0.006 - 0.020), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (profile));

			profile.ang = objInfo.ang + DegreeToRad (180.0);
			profile.leftBottomX = objInfo.leftBottomX;
			profile.leftBottomY = objInfo.leftBottomY;
			moveIn3D ('x', profile.ang, -objInfo.horLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('y', profile.ang, -objInfo.verLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('x', profile.ang, 0.300 + 0.006 + 0.020, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			moveIn3D ('y', profile.ang, marginEnds / 2 + objInfo.verLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (profile));
			moveIn3D ('x', profile.ang, -(0.300 + 0.006 + 0.020) + objInfo.horLen - (0.300 - 0.006 - 0.020), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (profile));

			// 결합철물 (사각와셔활용) 설치
			CellForSlabTableform	fittings;

			fittings.objType = FITTINGS;
			fittings.leftBottomX = objInfo.leftBottomX;
			fittings.leftBottomY = objInfo.leftBottomY;
			fittings.leftBottomZ = objInfo.leftBottomZ;
			fittings.ang = objInfo.ang;
			fittings.libPart.fittings.angX = DegreeToRad (270.0);
			fittings.libPart.fittings.angY = DegreeToRad (0.0);
			fittings.libPart.fittings.bolt_len = 0.150;
			fittings.libPart.fittings.bolt_dia = 0.012;
			fittings.libPart.fittings.bWasher1 = false;
			fittings.libPart.fittings.washer_pos1 = 0.0;
			fittings.libPart.fittings.bWasher2 = true;
			fittings.libPart.fittings.washer_pos2 = 0.0766;
			fittings.libPart.fittings.washer_size = 0.100;
			strncpy (fittings.libPart.fittings.nutType, "육각너트", strlen ("육각너트"));

			moveIn3D ('x', fittings.ang, 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			moveIn3D ('y', fittings.ang, -0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			moveIn3D ('z', fittings.ang, -0.0499, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (fittings));
			moveIn3D ('x', fittings.ang, -0.300 + objInfo.horLen - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (fittings));
			moveIn3D ('y', fittings.ang, 0.328 - objInfo.verLen + 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (fittings));
			moveIn3D ('x', fittings.ang, 0.300 - objInfo.horLen + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeLibPartOnSlabTableform (fittings));
		}
		setParameterByName (&memo, "type", tempString);

	} else if (objInfo.objType == PLYWOOD) {
		element.header.layer = layerInd_Plywood;
		setParameterByName (&memo, "p_stan", "비규격");							// 규격
		setParameterByName (&memo, "w_dir", "바닥깔기");						// 설치방향
		setParameterByName (&memo, "p_thk", "11.5T");							// 두께
		setParameterByName (&memo, "p_wid", objInfo.libPart.plywood.p_wid);		// 가로
		setParameterByName (&memo, "p_leng", objInfo.libPart.plywood.p_leng);	// 세로
		setParameterByName (&memo, "sogak", 0.0);								// 제작틀 OFF
		
	} else if (objInfo.objType == WOOD) {
		element.header.layer = layerInd_Wood;
		setParameterByName (&memo, "w_ins", "바닥눕히기");					// 설치방향
		setParameterByName (&memo, "w_w", objInfo.libPart.wood.w_w);		// 두께
		setParameterByName (&memo, "w_h", objInfo.libPart.wood.w_h);		// 너비
		setParameterByName (&memo, "w_leng", objInfo.libPart.wood.w_leng);	// 길이
		setParameterByName (&memo, "w_ang", objInfo.libPart.wood.w_ang);	// 각도
	}

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// 슬래브 테이블폼의 부속 철물들에 해당하는 라이브러리 배치
API_Guid	SlabTableformPlacingZone::placeLibPartOnSlabTableform (CellForSlabTableform objInfo)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const	GS::uchar_t* gsmName = NULL;
	double	aParam;
	double	bParam;
	Int32	addParNum;

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
	if (objInfo.objType == PROFILE)			gsmName = L("KS프로파일v1.0.gsm");
	if (objInfo.objType == FITTINGS)		gsmName = L("결합철물 (사각와셔활용) v1.0.gsm");

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
	element.header.floorInd = infoSlab.floorInd;

	if (objInfo.objType == PROFILE) {
		element.header.layer = layerInd_Profile;

		setParameterByName (&memo, "angX", objInfo.libPart.profile.angX);	// 회전X
		setParameterByName (&memo, "angY", objInfo.libPart.profile.angY);	// 회전Y

		setParameterByName (&memo, "type", "보");								// 분류 (기둥, 보)
		setParameterByName (&memo, "shape", "C형강");							// 형태 (C형강, H형강...)
		setParameterByName (&memo, "iAnchor", objInfo.libPart.profile.iAnchor);	// 앵커 포인트
		setParameterByName (&memo, "len", objInfo.libPart.profile.len);			// 길이
		setParameterByName (&memo, "ZZYZX", objInfo.libPart.profile.len);		// 길이
		setParameterByName (&memo, "nom", "75 x 40 x 5 x 7");					// 규격
		setParameterByName (&memo, "mat", 19.0);								// 재질

	} else if (objInfo.objType == FITTINGS) {
		element.header.layer = layerInd_Fittings;

		setParameterByName (&memo, "angX", objInfo.libPart.fittings.angX);	// 회전X
		setParameterByName (&memo, "angY", objInfo.libPart.fittings.angY);	// 회전Y

		setParameterByName (&memo, "bolt_len", objInfo.libPart.fittings.bolt_len);			// 볼트 길이
		setParameterByName (&memo, "bolt_dia", objInfo.libPart.fittings.bolt_dia);			// 볼트 직경
		setParameterByName (&memo, "bWasher1", objInfo.libPart.fittings.bWasher1);			// 와셔1 On/Off
		setParameterByName (&memo, "washer_pos1", objInfo.libPart.fittings.washer_pos1);	// 와셔1 위치
		setParameterByName (&memo, "bWasher2", objInfo.libPart.fittings.bWasher2);			// 와셔2 On/Off
		setParameterByName (&memo, "washer_pos2", objInfo.libPart.fittings.washer_pos2);	// 와셔2 위치
		setParameterByName (&memo, "washer_size", objInfo.libPart.fittings.washer_size);	// 와셔 크기
		setParameterByName (&memo, "nutType", objInfo.libPart.fittings.nutType);			// 너트 타입
	}

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// 유로폼을 채운 후 자투리 공간 채우기
GSErrCode	SlabTableformPlacingZone::fillRestAreas (void)
{
	GSErrCode	err = NoError;
	short		xx;
	CellForSlabTableform	insCell;
	double		startXPos, startYPos;
	API_Coord3D	axisPoint, rotatedPoint, unrotatedPoint;

	// 솔리드 연산을 위해 GUID를 저장함
	API_Guid	topAtLeftTop, topAtRightTop;
	API_Guid	bottomAtLeftBottom, bottomAtRightBottom;
	API_Guid	leftAtLeftTop, leftAtLeftBottom;
	API_Guid	rightAtRightTop, rightAtRightBottom;

	// 회전축이 되는 점
	axisPoint.x = firstClickPoint.x;
	axisPoint.y = firstClickPoint.y;
	axisPoint.z = firstClickPoint.y;


	// 합판 설치 (TOP)
	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = axisPoint.x;
	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.upMove;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) + placingZone.upMove;
	insCell.libPart.plywood.p_leng = placingZone.cells [0][0].horLen + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - (placingZone.corner_leftTop.x - placingZone.outerLeft) - placingZone.leftMove;
	insCell.libPart.plywood.w_dir_wall = true;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	topAtLeftTop = placingZone.placeLibPart (insCell);
	elemList.Push (topAtLeftTop);

	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.cells [0][0].horLen - placingZone.leftMove;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.upMove;
	for (xx = 1 ; xx < placingZone.tb_count_hor-1 ; ++xx) {
		insCell.objType = PLYWOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level;
		insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) + placingZone.upMove;
		insCell.libPart.plywood.p_leng = placingZone.cells [0][xx].horLen;
		insCell.libPart.plywood.w_dir_wall = true;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startXPos += placingZone.cells [0][xx].horLen;
	}

	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = startXPos;
	insCell.leftBottomY = startYPos;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) + placingZone.upMove;
	insCell.libPart.plywood.p_leng = placingZone.cells [0][xx].horLen + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - (placingZone.outerRight - placingZone.corner_rightTop.x) + placingZone.leftMove;
	insCell.libPart.plywood.w_dir_wall = true;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	topAtRightTop = placingZone.placeLibPart (insCell);
	elemList.Push (topAtRightTop);


	// 합판 설치 (BOTTOM)
	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = axisPoint.x - (placingZone.corner_leftTop.x - placingZone.outerLeft) + (placingZone.corner_leftBottom.x - placingZone.outerLeft);
	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom);
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.upMove;
	insCell.libPart.plywood.p_leng = placingZone.cells [0][0].horLen + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - (placingZone.corner_leftBottom.x - placingZone.outerLeft) - placingZone.leftMove;
	insCell.libPart.plywood.w_dir_wall = true;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	bottomAtLeftBottom = placingZone.placeLibPart (insCell);
	elemList.Push (bottomAtLeftBottom);

	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.cells [0][0].horLen - placingZone.leftMove;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom);
	for (xx = 1 ; xx < placingZone.tb_count_hor-1 ; ++xx) {
		insCell.objType = PLYWOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level;
		insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.upMove;
		insCell.libPart.plywood.p_leng = placingZone.cells [0][xx].horLen;
		insCell.libPart.plywood.w_dir_wall = true;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		elemList.Push (placingZone.placeLibPart (insCell));
			
		// 다음 셀 배치를 위해 시작 좌표 이동
		startXPos += placingZone.cells [0][xx].horLen;
	}

	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = startXPos;
	insCell.leftBottomY = startYPos;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.upMove;
	insCell.libPart.plywood.p_leng = placingZone.cells [0][xx].horLen + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - (placingZone.outerRight - placingZone.corner_rightBottom.x) + placingZone.leftMove;
	insCell.libPart.plywood.w_dir_wall = true;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	bottomAtRightBottom = placingZone.placeLibPart (insCell);
	elemList.Push (bottomAtRightBottom);


	// 합판 설치 (LEFT)
	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft;
	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.upMove;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = placingZone.cells [0][0].verLen + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - (placingZone.outerTop - placingZone.corner_leftTop.y) + placingZone.upMove;
	insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove;
	insCell.libPart.plywood.w_dir_wall = true;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	leftAtLeftTop = placingZone.placeLibPart (insCell);
	elemList.Push (leftAtLeftTop);

	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.cells [1][0].verLen - placingZone.upMove;
	for (xx = 1 ; xx < placingZone.tb_count_ver-1 ; ++xx) {
		insCell.objType = PLYWOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level;
		insCell.libPart.plywood.p_wid = placingZone.cells [xx][0].verLen;
		insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove;
		insCell.libPart.plywood.w_dir_wall = true;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startYPos -= placingZone.cells [xx+1][0].verLen;
	}

	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = startXPos;
	insCell.leftBottomY = startYPos - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + placingZone.upMove;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = placingZone.cells [xx][0].verLen + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.upMove;
	insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove;
	insCell.libPart.plywood.w_dir_wall = true;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	leftAtLeftBottom = placingZone.placeLibPart (insCell);
	elemList.Push (leftAtLeftBottom);


	// 합판 설치 (RIGHT)
	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) - placingZone.leftMove;
	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.upMove;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = placingZone.cells [0][0].verLen + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - (placingZone.outerTop - placingZone.corner_rightTop.y) + placingZone.upMove;
	insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.leftMove;
	insCell.libPart.plywood.w_dir_wall = true;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	rightAtRightTop = placingZone.placeLibPart (insCell);
	elemList.Push (rightAtRightTop);

	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) - placingZone.leftMove;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.cells [1][0].verLen - placingZone.upMove;
	for (xx = 1 ; xx < placingZone.tb_count_ver-1 ; ++xx) {
		insCell.objType = PLYWOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level;
		insCell.libPart.plywood.p_wid = placingZone.cells [xx][0].verLen;
		insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.leftMove;
		insCell.libPart.plywood.w_dir_wall = true;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startYPos -= placingZone.cells [xx+1][0].verLen;
	}

	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = startXPos;
	insCell.leftBottomY = startYPos - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + (placingZone.corner_rightBottom.y - placingZone.outerBottom) + placingZone.upMove;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = placingZone.cells [xx][0].verLen + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - (placingZone.corner_rightBottom.y - placingZone.outerBottom) - placingZone.upMove;
	insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.leftMove;
	insCell.libPart.plywood.w_dir_wall = true;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	rightAtRightBottom = placingZone.placeLibPart (insCell);
	elemList.Push (rightAtRightBottom);

	// 합판 코너 겹치는 부분은 솔리드 연산으로 빼기
	err = ACAPI_Element_SolidLink_Create (topAtLeftTop,			leftAtLeftTop, APISolid_Substract, APISolidFlag_OperatorAttrib);
	err = ACAPI_Element_SolidLink_Create (topAtRightTop,		rightAtRightTop, APISolid_Substract, APISolidFlag_OperatorAttrib);
	err = ACAPI_Element_SolidLink_Create (bottomAtLeftBottom,	leftAtLeftBottom, APISolid_Substract, APISolidFlag_OperatorAttrib);
	err = ACAPI_Element_SolidLink_Create (bottomAtRightBottom,	rightAtRightBottom, APISolid_Substract, APISolidFlag_OperatorAttrib);


	// 유로폼 둘레 목재 설치 (TOP)
	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + placingZone.woodWidth - placingZone.upMove;
	for (xx = 0 ; xx < placingZone.tb_count_hor ; ++xx) {
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = placingZone.woodWidth;
		insCell.libPart.wood.w_leng = placingZone.cells [0][xx].horLen;
		insCell.libPart.wood.w_w = 0.050;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startXPos += placingZone.cells [0][xx].horLen;
	}


	// 유로폼 둘레 목재 설치 (BOTTOM)
	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.upMove;
	for (xx = 0 ; xx < placingZone.tb_count_hor ; ++xx) {
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = placingZone.woodWidth;
		insCell.libPart.wood.w_leng = placingZone.cells [0][xx].horLen;
		insCell.libPart.wood.w_w = 0.050;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startXPos += placingZone.cells [0][xx].horLen;
	}

	
	// 유로폼 둘레 목재 설치 (LEFT) : 셀 각도가 90 회전하여 시작 좌표 뒷부분의 X,Y축이 바뀌어야 함
	insCell.objType = WOOD;
	insCell.ang = placingZone.ang + DegreeToRad (90.0);
	insCell.leftBottomX = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.upMove;
	insCell.leftBottomY = axisPoint.y - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) + (placingZone.corner_leftTop.x - placingZone.outerLeft) + placingZone.woodWidth + placingZone.leftMove;
	insCell.leftBottomZ = placingZone.level - 0.0115;
	insCell.libPart.wood.w_ang = 0.0;
	insCell.libPart.wood.w_h = placingZone.woodWidth;
	insCell.libPart.wood.w_leng = placingZone.cells [0][0].verLen + placingZone.woodWidth;
	insCell.libPart.wood.w_w = 0.050;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	elemList.Push (placingZone.placeLibPart (insCell));

	startXPos = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.cells [1][0].verLen - placingZone.upMove;
	startYPos = axisPoint.y - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) + (placingZone.corner_leftTop.x - placingZone.outerLeft) + placingZone.woodWidth + placingZone.leftMove;
	for (xx = 1 ; xx < placingZone.tb_count_ver-1 ; ++xx) {
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang + DegreeToRad (90.0);
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = placingZone.woodWidth;
		insCell.libPart.wood.w_leng = placingZone.cells [xx][0].verLen;
		insCell.libPart.wood.w_w = 0.050;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startXPos -= placingZone.cells [xx+1][0].verLen;
	}

	insCell.objType = WOOD;
	insCell.ang = placingZone.ang + DegreeToRad (90.0);
	insCell.leftBottomX = startXPos - placingZone.woodWidth;
	insCell.leftBottomY = startYPos;
	insCell.leftBottomZ = placingZone.level - 0.0115;
	insCell.libPart.wood.w_ang = 0.0;
	insCell.libPart.wood.w_h = placingZone.woodWidth;
	insCell.libPart.wood.w_leng = placingZone.cells [xx][0].verLen + placingZone.woodWidth;
	insCell.libPart.wood.w_w = 0.050;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	elemList.Push (placingZone.placeLibPart (insCell));


	// 유로폼 둘레 목재 설치 (RIGHT) : 셀 각도가 90 회전하여 시작 좌표 뒷부분의 X,Y축이 바뀌어야 함
	insCell.objType = WOOD;
	insCell.ang = placingZone.ang + DegreeToRad (90.0);
	insCell.leftBottomX = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.upMove;
	insCell.leftBottomY = axisPoint.y - (- placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2)) + placingZone.leftMove;
	insCell.leftBottomZ = placingZone.level - 0.0115;
	insCell.libPart.wood.w_ang = 0.0;
	insCell.libPart.wood.w_h = placingZone.woodWidth;
	insCell.libPart.wood.w_leng = placingZone.cells [0][0].verLen + placingZone.woodWidth;
	insCell.libPart.wood.w_w = 0.050;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	elemList.Push (placingZone.placeLibPart (insCell));

	startXPos = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.cells [1][0].verLen - placingZone.upMove;
	startYPos = axisPoint.y - (- placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2)) + placingZone.leftMove;
	for (xx = 1 ; xx < placingZone.tb_count_ver-1 ; ++xx) {
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang + DegreeToRad (90.0);
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = placingZone.woodWidth;
		insCell.libPart.wood.w_leng = placingZone.cells [xx][0].verLen;
		insCell.libPart.wood.w_w = 0.050;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startXPos -= placingZone.cells [xx+1][0].verLen;
	}

	insCell.objType = WOOD;
	insCell.ang = placingZone.ang + DegreeToRad (90.0);
	insCell.leftBottomX = startXPos - placingZone.woodWidth;
	insCell.leftBottomY = startYPos;
	insCell.leftBottomZ = placingZone.level - 0.0115;
	insCell.libPart.wood.w_ang = 0.0;
	insCell.libPart.wood.w_h = placingZone.woodWidth;
	insCell.libPart.wood.w_leng = placingZone.cells [xx][0].verLen + placingZone.woodWidth;
	insCell.libPart.wood.w_w = 0.050;

	// 위치 값을 비회전값으로 변환
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// 셀 배치
	elemList.Push (placingZone.placeLibPart (insCell));


	// 코너쪽 목재 설치 (LEFT-TOP)
	// ...


	// 코너쪽 목재 설치 (RIGHT-TOP)
	// ...


	// 코너쪽 목재 설치 (LEFT-BOTTOM)
	// ...


	// 코너쪽 목재 설치 (RIGHT-BOTTOM)
	// ...


	// 보강 목재 설치 : T 버튼에 해당 (왼쪽부터 시작, 0부터 eu_count_hor-2까지) : LeftBottom에서 RightBottom까지
	startXPos = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) + 0.064;
	startYPos = axisPoint.y + placingZone.corner_leftTop.x - placingZone.outerLeft - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) - placingZone.cells [0][0].horLen + placingZone.woodWidth + placingZone.leftMove;
	for (xx = 0 ; xx < placingZone.tb_count_hor-1 ; ++xx) {
		// 1번
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang + DegreeToRad (90.0);
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = placingZone.woodWidth;
		insCell.libPart.wood.w_leng = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.woodWidth - 0.064 - placingZone.upMove;
		insCell.libPart.wood.w_w = 0.050;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		if (placingZone.topBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 2번
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos - placingZone.woodWidth;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		if (placingZone.topBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startYPos -= placingZone.cells [0][xx+1].horLen;
	}
	
	// 보강 목재 설치 : B 버튼에 해당 (왼쪽부터 시작, 0부터 eu_count_hor-2까지) : LeftTop에서 RightTop까지
	startXPos = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + placingZone.woodWidth - placingZone.upMove;
	startYPos = axisPoint.y + placingZone.corner_leftTop.x - placingZone.outerLeft - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) - placingZone.cells [0][0].horLen + placingZone.woodWidth + placingZone.leftMove;
	for (xx = 0 ; xx < placingZone.tb_count_hor-1 ; ++xx) {
		// 1번
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang + DegreeToRad (90.0);
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = placingZone.woodWidth;
		insCell.libPart.wood.w_leng = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.woodWidth - 0.064 + placingZone.upMove;
		insCell.libPart.wood.w_w = 0.050;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		if (placingZone.bottomBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 2번
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos - placingZone.woodWidth;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		if (placingZone.bottomBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startYPos -= placingZone.cells [0][xx+1].horLen;
	}

	// 보강 목재 설치 : L 버튼에 해당 (위부터 시작, 0부터 eu_count_ver-2까지) : LeftBottom에서 LeftTop까지
	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + 0.064;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.upMove;
	for (xx = placingZone.tb_count_ver-2 ; xx >= 0 ; --xx) {
		// 1번
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = placingZone.woodWidth;
		insCell.libPart.wood.w_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.woodWidth - 0.064 - placingZone.leftMove;
		insCell.libPart.wood.w_w = 0.050;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		if (placingZone.leftBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 2번
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos + placingZone.woodWidth;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		if (placingZone.leftBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startYPos -= placingZone.cells [xx][0].verLen;
	}

	// 보강 목재 설치 : R 버튼에 해당 (위부터 시작, 0부터 eu_count_ver-2까지) : RightBottom에서 RightTop까지
	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.woodWidth) - placingZone.leftMove;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.upMove;
	for (xx = placingZone.tb_count_ver-2 ; xx >= 0 ; --xx) {
		// 1번
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = placingZone.woodWidth;
		insCell.libPart.wood.w_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.woodWidth - 0.064 + placingZone.leftMove;
		insCell.libPart.wood.w_w = 0.050;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		if (placingZone.rightBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 2번
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos + placingZone.woodWidth;

		// 위치 값을 비회전값으로 변환
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// 셀 배치
		if (placingZone.rightBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 다음 셀 배치를 위해 시작 좌표 이동
		startYPos -= placingZone.cells [xx][0].verLen;
	}

	return err;
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK slabBottomTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "슬래브 하부에 배치 (테이블폼)");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			//////////////////////////////////////////////////////////// 아이템 배치 (테이블폼)
			// 라벨: 유로폼 배치 설정
			DGSetItemText (dialogID, LABEL_PLACING_TABLEFORM, "테이블폼 배치 설정");

			// 라벨: 너비
			DGSetItemText (dialogID, LABEL_TABLEFORM_WIDTH, "너비");

			// 라벨: 높이
			DGSetItemText (dialogID, LABEL_TABLEFORM_HEIGHT, "높이");

			// 라벨: 설치방향
			DGSetItemText (dialogID, LABEL_TABLEFORM_ORIENTATION, "설치방향");

			// 라벨: 슬래브와의 간격
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "슬래브와의 간격");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");

			// 라벨: 레이어 - 슬래브 테이블폼
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "테이블폼");

			// 라벨: 레이어 - 합판
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");

			// 라벨: 레이어 - 목재
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "목재");

			// 라벨: 레이어 - 프로파일
			DGSetItemText (dialogID, LABEL_LAYER_PROFILE, "C형강");

			// 라벨: 레이어 - 결합철물
			DGSetItemText (dialogID, LABEL_LAYER_FITTINGS, "결합철물");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_SLABTABLEFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PROFILE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_FITTINGS;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS, 1);
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 유로폼 너비, 높이, 방향
					placingZone.tb_wid = DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_WIDTH))).ToCStr ().Get ();
					placingZone.tb_hei = DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_HEIGHT))).ToCStr ().Get ();
					placingZone.tb_ori = DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_ORIENTATION))).ToCStr ().Get ();

					// 슬래브와의 간격
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// 레이어 번호 저장
					layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
					layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE);
					layerInd_Fittings		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS);

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
short DGCALLBACK slabBottomTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	btnSizeX = 50, btnSizeY = 50;
	short	dialogSizeX, dialogSizeY;
	short	btnInitPosX = 220 + 25;
	short	btnPosX, btnPosY;
	short	xx, yy;
	short	pp, qq;
	short	xx1, yy1;
	short	idxBtn;
	short	lastIdxBtn = 0;
	double	leftMargin, rightMargin, topMargin, bottomMargin;
	std::string		txtButton = "";
	API_Element		elem;
	GSErrCode		err;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "슬래브 하부에 배치 - 테이블폼 배치 수정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 배치 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 210, 130, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "2. 배  치");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼 1
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 250, 130, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "3. 자투리 채우기");
			DGShowItem (dialogID, DG_CANCEL);

			// 종료 버튼 2
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 290, 130, 25);
			DGSetItemFont (dialogID, DG_CANCEL2, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL2, "3. 자투리 제외");
			DGShowItem (dialogID, DG_CANCEL2);

			// 이전 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 330, 130, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "이전");
			DGShowItem (dialogID, DG_PREV);

			// 라벨: 여백(좌)
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 20, 20, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH_LEFT, "좌");
			DGShowItem (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH_LEFT);

			// 라벨: 여백(우)
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 20, 20, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH_RIGHT, "우");
			DGShowItem (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH_RIGHT);

			// 라벨: 여백(상)
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 50, 20, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH_UP, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_VERTICAL_LENGTH_UP, "상");
			DGShowItem (dialogID, LABEL_REMAIN_VERTICAL_LENGTH_UP);

			// 라벨: 여백(상)
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 50, 20, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH_DOWN, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_VERTICAL_LENGTH_DOWN, "하");
			DGShowItem (dialogID, LABEL_REMAIN_VERTICAL_LENGTH_DOWN);

			// Edit 컨트롤: 여백(좌)
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 40, 20-7, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2);

			// Edit 컨트롤: 여백(우)
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 120, 20-7, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2);

			// Edit 컨트롤: 여백(상)
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 40, 50-7, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2);

			// Edit 컨트롤: 여백(하)
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 120, 50-7, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2);

			// 라벨: 테이블폼/휠러스페이서 배치 설정
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 10, 200, 23);
			DGSetItemFont (dialogID, LABEL_GRID_TABLEFORM_WOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_GRID_TABLEFORM_WOOD, "테이블폼/보강 목재 배치 설정");
			DGShowItem (dialogID, LABEL_GRID_TABLEFORM_WOOD);

			// 남은 거리 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 90, 130, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, "1. 남은 길이 확인");
			DGShowItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);
			DGDisableItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);

			// 행 추가 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 130, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_ROW, "행 추가");
			DGShowItem (dialogID, PUSHBUTTON_ADD_ROW);

			// 행 삭제 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 130, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_ROW, "행 삭제");
			DGShowItem (dialogID, PUSHBUTTON_DEL_ROW);

			// 열 추가 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 170, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_COL, "열 추가");
			DGShowItem (dialogID, PUSHBUTTON_ADD_COL);
		
			// 열 삭제 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 170, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_COL, "열 삭제");
			DGShowItem (dialogID, PUSHBUTTON_DEL_COL);

			// 메인 창 크기를 변경
			dialogSizeX = 350 + (btnSizeX * placingZone.tb_count_hor) + 50;
			dialogSizeY = max<short>(450, 150 + (btnSizeY * placingZone.tb_count_ver) + 50);
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			// 그리드 구조체에 따라서 버튼을 동적으로 배치함
			btnPosX = 220 + 25, btnPosY = (btnSizeY * placingZone.tb_count_ver) + 25;
			for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
				for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {
					idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					lastIdxBtn = idxBtn;
					DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

					txtButton = "";
					if (placingZone.cells [xx][yy].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cells [xx][yy].objType == SLAB_TABLEFORM) {
						if (placingZone.cells [xx][yy].libPart.tableform.direction)
							txtButton = format_string ("테이블폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						else
							txtButton = format_string ("테이블폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
						txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					}
					DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, idxBtn);
					btnPosX += btnSizeX;
				}
				btnPosX = btnInitPosX;
				btnPosY -= btnSizeY;
			}

			// 위쪽 보강 목재 체크 박스
			btnPosX = 270 + 12, btnPosY = 48;
			for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
				idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
				lastIdxBtn = idxBtn;
				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

				txtButton = "T";
				DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
				DGShowItem (dialogID, idxBtn);
				btnPosX += 50;
			}

			TButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

			// 아래쪽 보강 목재 체크 박스
			btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.tb_count_ver + 1)) + 27;
			for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
				idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
				lastIdxBtn = idxBtn;
				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

				txtButton = "B";
				DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
				DGShowItem (dialogID, idxBtn);
				btnPosX += 50;
			}

			BButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

			// 왼쪽 보강 목재 체크 박스
			btnPosX = 219, btnPosY = 114;
			for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
				idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
				lastIdxBtn = idxBtn;
				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

				txtButton = "L";
				DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
				DGShowItem (dialogID, idxBtn);
				btnPosY += 50;
			}

			LButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

			// 오른쪽 보강 목재 체크 박스
			btnPosX = 220 + 25 + (btnSizeX * placingZone.tb_count_hor), btnPosY = 114;
			for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
				idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
				lastIdxBtn = idxBtn;
				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL); 

				txtButton = "R";
				DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
				DGShowItem (dialogID, idxBtn);
				btnPosY += 50;
			}

			RButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT:
					// 왼쪽 여백을 변경하면,
					leftMargin = DGGetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT);
					if ((placingZone.remain_hor_updated / 2) > leftMargin) {
						placingZone.leftMove = (placingZone.remain_hor_updated / 2) - leftMargin;
					} else {
						placingZone.leftMove = leftMargin - (placingZone.remain_hor_updated / 2);
					}

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2 - placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2 + placingZone.leftMove);

					break;
				case EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT:
					// 오른쪽 여백을 변경하면,
					rightMargin = DGGetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT);
					if ((placingZone.remain_hor_updated / 2) > rightMargin) {
						placingZone.leftMove = -((placingZone.remain_hor_updated / 2) - rightMargin);
					} else {
						placingZone.leftMove = -(rightMargin - (placingZone.remain_hor_updated / 2));
					}

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2 - placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2 + placingZone.leftMove);

					break;
				case EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP:
					// 위쪽 여백을 변경하면,
					topMargin = DGGetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP);
					if ((placingZone.remain_ver_updated / 2) > topMargin) {
						placingZone.upMove = (placingZone.remain_ver_updated / 2) - topMargin;
					} else {
						placingZone.upMove = topMargin - (placingZone.remain_ver_updated / 2);
					}

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2 - placingZone.upMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2 + placingZone.upMove);

					break;
				case EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN:
					// 아래쪽 여백을 변경하면,
					bottomMargin = DGGetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN);
					if ((placingZone.remain_ver_updated / 2) > bottomMargin) {
						placingZone.upMove = -((placingZone.remain_ver_updated / 2) - bottomMargin);
					} else {
						placingZone.upMove = -(bottomMargin - (placingZone.remain_ver_updated / 2));
					}

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2 - placingZone.upMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2 + placingZone.upMove);

					break;
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case PUSHBUTTON_CONFIRM_REMAIN_LENGTH:
					// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
					item = 0;

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;
					
					// 그리드 버튼 텍스트 업데이트
					for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == SLAB_TABLEFORM) {
								if (placingZone.cells [xx][yy].libPart.tableform.direction)
									txtButton = format_string ("테이블폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("테이블폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
								txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}

							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
							++idxBtn;
						}
					}

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2 - placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2 + placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2 - placingZone.upMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2 + placingZone.upMove);
					DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_BOLD);

					break;

				case DG_OK:
					// 종료하지 않고 배치된 객체를 수정 및 재배치하고 그리드 버튼 속성을 변경함
					item = 0;

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;

					// 그리드 버튼 텍스트 업데이트
					for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == SLAB_TABLEFORM) {
								if (placingZone.cells [xx][yy].libPart.tableform.direction)
									txtButton = format_string ("테이블폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("테이블폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
								txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}

							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
							++idxBtn;
						}
					}

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2 - placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2 + placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2 - placingZone.upMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2 + placingZone.upMove);
					DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_BOLD);

					// 기존 배치된 객체 전부 삭제
					for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {
							elem.header.guid = placingZone.cells [xx][yy].guid;
							if (ACAPI_Element_Get (&elem) != NoError)
								continue;

							API_Elem_Head* headList = new API_Elem_Head [1];
							headList [0] = elem.header;
							err = ACAPI_Element_Delete (&headList, 1);
							delete headList;
						}
					}

					elemList.Clear (true);

					// 업데이트된 셀 정보대로 객체 재배치
					CellForSlabTableform	plywoodCell;	// 합판의 크기가 초과된 경우 사용할 임시 합판용 셀 (분할된 크기 적용)

					for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {
							if (placingZone.cells [xx][yy].objType == PLYWOOD) {
								if ((placingZone.cells [xx][yy].horLen > 1.220) || (placingZone.cells [xx][yy].verLen > 2.440)) {
									// 최대 규격을 초과한 합판의 경우
									pp = 1;
									qq = 1;
									plywoodCell.leftBottomX = placingZone.cells [xx][yy].leftBottomX;
									plywoodCell.leftBottomY = placingZone.cells [xx][yy].leftBottomY;
									plywoodCell.leftBottomZ = placingZone.cells [xx][yy].leftBottomZ;
									plywoodCell.ang = placingZone.cells [xx][yy].ang;
									plywoodCell.objType = placingZone.cells [xx][yy].objType;

									if (placingZone.cells [xx][yy].horLen > 1.220) {
										pp = static_cast<short>(ceil (placingZone.cells [xx][yy].horLen / 1.220));
									}

									if (placingZone.cells [xx][yy].verLen > 2.440) {
										qq = static_cast<short>(ceil (placingZone.cells [xx][yy].verLen / 2.440));
									}

									plywoodCell.horLen = placingZone.cells [xx][yy].horLen / pp;
									plywoodCell.verLen = placingZone.cells [xx][yy].verLen / qq;
									plywoodCell.libPart.plywood.p_wid = placingZone.cells [xx][yy].horLen / pp;
									plywoodCell.libPart.plywood.p_leng = placingZone.cells [xx][yy].verLen / qq;

									for (xx1 = 0 ; xx1 < qq ; ++xx1) {
										for (yy1 = 0 ; yy1 < pp ; ++yy1) {
											plywoodCell.ang -= DegreeToRad (90.0);
											elemList.Push (placingZone.placeLibPart (plywoodCell));
											plywoodCell.ang += DegreeToRad (90.0);
											if ((placingZone.tb_ori.compare (std::string ("Vertical")) == 0) && (placingZone.cells [xx][yy].horLen < placingZone.cells [xx][yy].verLen)) {
												// 이동하지 않음 (합판이 분할되면 문제가 발생할 수 있음)
											} else {
												moveIn3D ('x', plywoodCell.ang, plywoodCell.horLen, &plywoodCell.leftBottomX, &plywoodCell.leftBottomY, &plywoodCell.leftBottomZ);
											}
										}
										moveIn3D ('x', plywoodCell.ang, -plywoodCell.horLen * (pp - 1), &plywoodCell.leftBottomX, &plywoodCell.leftBottomY, &plywoodCell.leftBottomZ);
										moveIn3D ('y', plywoodCell.ang, -plywoodCell.verLen, &plywoodCell.leftBottomX, &plywoodCell.leftBottomY, &plywoodCell.leftBottomZ);
									}
								} else {
									// 일반 규격 내 크기의 합판
									placingZone.cells [xx][yy].ang -= DegreeToRad (90.0);
									placingZone.cells [xx][yy].guid = placingZone.placeLibPart (placingZone.cells [xx][yy]);
									placingZone.cells [xx][yy].ang += DegreeToRad (90.0);
									elemList.Push (placingZone.cells [xx][yy].guid);
								}
							} else {
								// 나머지 객체
								placingZone.cells [xx][yy].guid = placingZone.placeLibPart (placingZone.cells [xx][yy]);
								elemList.Push (placingZone.cells [xx][yy].guid);
							}
						}
					}

					// T, B, L, R 버튼 배열의 푸시 여부를 저장함
					for (xx = 0 ; xx < placingZone.tb_count_hor-1 ; ++xx) {
						if (DGGetItemValLong (dialogID, TButtonStartIdx) == TRUE)
							placingZone.topBoundsCells [xx] = true;
						else
							placingZone.topBoundsCells [xx] = false;
						++TButtonStartIdx;
					}
					for (xx = 0 ; xx < placingZone.tb_count_hor-1 ; ++xx) {
						if (DGGetItemValLong (dialogID, BButtonStartIdx) == TRUE)
							placingZone.bottomBoundsCells [xx] = true;
						else
							placingZone.bottomBoundsCells [xx] = false;
						++BButtonStartIdx;
					}
					for (xx = 0 ; xx < placingZone.tb_count_ver-1 ; ++xx) {
						if (DGGetItemValLong (dialogID, LButtonStartIdx) == TRUE)
							placingZone.leftBoundsCells [xx] = true;
						else
							placingZone.leftBoundsCells [xx] = false;
						++LButtonStartIdx;
					}
					for (xx = 0 ; xx < placingZone.tb_count_ver-1 ; ++xx) {
						if (DGGetItemValLong (dialogID, RButtonStartIdx) == TRUE)
							placingZone.rightBoundsCells [xx] = true;
						else
							placingZone.rightBoundsCells [xx] = false;
						++RButtonStartIdx;
					}

					clickedOKButton = true;

					break;

				case DG_CANCEL:
					break;

				case DG_CANCEL2:
					clickedExcludeRestButton = true;
					break;

				case DG_PREV:
					clickedPrevButton = true;
					break;

				case PUSHBUTTON_ADD_ROW:
					// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
					item = 0;

					// 새로운 행 추가 (행 하나를 늘리고 추가된 행에 마지막 행 정보 복사)
					placingZone.addNewRow (&placingZone);

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;

					// 기존에 배치된 그리드 버튼 모두 삭제 (itemInitIdx부터)
					DGRemoveDialogItems (dialogID, itemInitIdx);
					
					// 메인 창 크기를 변경
					dialogSizeX = 350 + (btnSizeX * placingZone.tb_count_hor) + 50;
					dialogSizeY = max<short>(450, 150 + (btnSizeY * placingZone.tb_count_ver) + 50);
					DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

					// 그리드 구조체에 따라서 버튼을 동적으로 배치함
					btnPosX = 220 + 25, btnPosY = (btnSizeY * placingZone.tb_count_ver) + 25;
					for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {
							idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
							lastIdxBtn = idxBtn;
							DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == SLAB_TABLEFORM) {
								if (placingZone.cells [xx][yy].libPart.tableform.direction)
									txtButton = format_string ("테이블폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("테이블폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
								txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}
							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
							DGShowItem (dialogID, idxBtn);
							btnPosX += btnSizeX;
						}
						btnPosX = btnInitPosX;
						btnPosY -= btnSizeY;
					}

					// 위쪽 보강 목재 체크 박스
					btnPosX = 270 + 12, btnPosY = 48;
					for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "T";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					TButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

					// 아래쪽 보강 목재 체크 박스
					btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.tb_count_ver + 1)) + 27;
					for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "B";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					BButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

					// 왼쪽 보강 목재 체크 박스
					btnPosX = 219, btnPosY = 114;
					for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "L";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					LButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

					// 오른쪽 보강 목재 체크 박스
					btnPosX = 220 + 25 + (btnSizeX * placingZone.tb_count_hor), btnPosY = 114;
					for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL); 

						txtButton = "R";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					RButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2 - placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2 + placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2 - placingZone.upMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2 + placingZone.upMove);

					break;

				case PUSHBUTTON_DEL_ROW:
					// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
					item = 0;

					// 마지막 행 삭제
					placingZone.delLastRow (&placingZone);

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;

					// 기존에 배치된 그리드 버튼 모두 삭제 (itemInitIdx부터)
					DGRemoveDialogItems (dialogID, itemInitIdx);
					
					// 메인 창 크기를 변경
					dialogSizeX = 350 + (btnSizeX * placingZone.tb_count_hor) + 50;
					dialogSizeY = max<short>(450, 150 + (btnSizeY * placingZone.tb_count_ver) + 50);
					DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

					// 그리드 구조체에 따라서 버튼을 동적으로 배치함
					btnPosX = 220 + 25, btnPosY = (btnSizeY * placingZone.tb_count_ver) + 25;
					for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {
							idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
							lastIdxBtn = idxBtn;
							DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == SLAB_TABLEFORM) {
								if (placingZone.cells [xx][yy].libPart.tableform.direction)
									txtButton = format_string ("테이블폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("테이블폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
								txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}
							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
							DGShowItem (dialogID, idxBtn);
							btnPosX += btnSizeX;
						}
						btnPosX = btnInitPosX;
						btnPosY -= btnSizeY;
					}

					// 위쪽 보강 목재 체크 박스
					btnPosX = 270 + 12, btnPosY = 48;
					for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "T";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					TButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

					// 아래쪽 보강 목재 체크 박스
					btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.tb_count_ver + 1)) + 27;
					for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "B";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					BButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

					// 왼쪽 보강 목재 체크 박스
					btnPosX = 219, btnPosY = 114;
					for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "L";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					LButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

					// 오른쪽 보강 목재 체크 박스
					btnPosX = 220 + 25 + (btnSizeX * placingZone.tb_count_hor), btnPosY = 114;
					for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL); 

						txtButton = "R";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					RButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2 - placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2 + placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2 - placingZone.upMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2 + placingZone.upMove);

					break;

				case PUSHBUTTON_ADD_COL:
					// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
					item = 0;

					// 새로운 열 추가 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
					placingZone.addNewCol (&placingZone);

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;

					// 기존에 배치된 그리드 버튼 모두 삭제 (itemInitIdx부터)
					DGRemoveDialogItems (dialogID, itemInitIdx);
					
					// 메인 창 크기를 변경
					dialogSizeX = 350 + (btnSizeX * placingZone.tb_count_hor) + 50;
					dialogSizeY = max<short>(450, 150 + (btnSizeY * placingZone.tb_count_ver) + 50);
					DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

					// 그리드 구조체에 따라서 버튼을 동적으로 배치함
					btnPosX = 220 + 25, btnPosY = (btnSizeY * placingZone.tb_count_ver) + 25;
					for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {
							idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
							lastIdxBtn = idxBtn;
							DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == SLAB_TABLEFORM) {
								if (placingZone.cells [xx][yy].libPart.tableform.direction)
									txtButton = format_string ("테이블폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("테이블폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
								txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}
							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
							DGShowItem (dialogID, idxBtn);
							btnPosX += btnSizeX;
						}
						btnPosX = btnInitPosX;
						btnPosY -= btnSizeY;
					}

					// 위쪽 보강 목재 체크 박스
					btnPosX = 270 + 12, btnPosY = 48;
					for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "T";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					TButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

					// 아래쪽 보강 목재 체크 박스
					btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.tb_count_ver + 1)) + 27;
					for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "B";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					BButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

					// 왼쪽 보강 목재 체크 박스
					btnPosX = 219, btnPosY = 114;
					for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "L";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					LButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

					// 오른쪽 보강 목재 체크 박스
					btnPosX = 220 + 25 + (btnSizeX * placingZone.tb_count_hor), btnPosY = 114;
					for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL); 

						txtButton = "R";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					RButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2 - placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2 + placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2 - placingZone.upMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2 + placingZone.upMove);

					break;

				case PUSHBUTTON_DEL_COL:
					// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
					item = 0;

					// 마지막 열 삭제
					placingZone.delLastCol (&placingZone);

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;

					// 기존에 배치된 그리드 버튼 모두 삭제 (itemInitIdx부터)
					DGRemoveDialogItems (dialogID, itemInitIdx);
					
					// 메인 창 크기를 변경
					dialogSizeX = 350 + (btnSizeX * placingZone.tb_count_hor) + 50;
					dialogSizeY = max<short>(450, 150 + (btnSizeY * placingZone.tb_count_ver) + 50);
					DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

					// 그리드 구조체에 따라서 버튼을 동적으로 배치함
					btnPosX = 220 + 25, btnPosY = (btnSizeY * placingZone.tb_count_ver) + 25;
					for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {
							idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
							lastIdxBtn = idxBtn;
							DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == SLAB_TABLEFORM) {
								if (placingZone.cells [xx][yy].libPart.tableform.direction)
									txtButton = format_string ("테이블폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("테이블폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
								txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}
							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
							DGShowItem (dialogID, idxBtn);
							btnPosX += btnSizeX;
						}
						btnPosX = btnInitPosX;
						btnPosY -= btnSizeY;
					}

					// 위쪽 보강 목재 체크 박스
					btnPosX = 270 + 12, btnPosY = 48;
					for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "T";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					TButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

					// 아래쪽 보강 목재 체크 박스
					btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.tb_count_ver + 1)) + 27;
					for (xx = 0 ; xx < placingZone.tb_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "B";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					BButtonStartIdx = lastIdxBtn - (placingZone.tb_count_hor - 2);

					// 왼쪽 보강 목재 체크 박스
					btnPosX = 219, btnPosY = 114;
					for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "L";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					LButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

					// 오른쪽 보강 목재 체크 박스
					btnPosX = 220 + 25 + (btnSizeX * placingZone.tb_count_hor), btnPosY = 114;
					for (xx = 0 ; xx < placingZone.tb_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL); 

						txtButton = "R";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					RButtonStartIdx = lastIdxBtn - (placingZone.tb_count_ver - 2);

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2 - placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2 + placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2 - placingZone.upMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2 + placingZone.upMove);

					break;

				default:
					// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
					clickedBtnItemIdx = item;
					result = DGBlankModalDialog (240, 260, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, slabBottomTableformPlacerHandler3, 0);

					item = 0;	// 그리드 버튼을 눌렀을 때 창이 닫히지 않게 함

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					placingZone.alignPlacingZone (&placingZone);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;
					
					// 그리드 버튼 텍스트 업데이트
					for (xx = 0 ; xx < placingZone.tb_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.tb_count_hor ; ++yy) {

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == SLAB_TABLEFORM) {
								if (placingZone.cells [xx][yy].libPart.tableform.direction)
									txtButton = format_string ("테이블폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("테이블폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
								txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}

							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
							++idxBtn;
						}
					}

					// 남은 가로/세로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT, placingZone.remain_hor_updated / 2 - placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT, placingZone.remain_hor_updated / 2 + placingZone.leftMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP, placingZone.remain_ver_updated / 2 - placingZone.upMove);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN, placingZone.remain_ver_updated / 2 + placingZone.upMove);
					DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_BOLD);

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
short DGCALLBACK slabBottomTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	short	idxCell;
	short	popupSelectedIdx = 0;
	double	temp;
	short	rIdx, cIdx;		// 행 번호, 열 번호

	switch (message) {
		case DG_MSG_INIT:

			// slabBottomTableformPlacerHandler2 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
			idxCell = (clickedBtnItemIdx - itemInitIdx);
			rIdx = 0;
			while (idxCell >= (placingZone.tb_count_hor)) {
				idxCell -= ((placingZone.tb_count_hor));
				++rIdx;
			}
			cIdx = idxCell;
			
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "Cell 값 설정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 215, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "저장");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 130, 215, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
			// 라벨: 객체 타입
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, "객체 타입");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// 팝업컨트롤: 객체 타입을 바꿀 수 있는 콤보박스가 맨 위에 나옴
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 20-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "테이블폼");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "합판");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, "너비");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, "높이");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION, "설치방향");
				
			// 라디오 버튼: 설치방향 (가로방향)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD, "Horizontal");
			// 라디오 버튼: 설치방향 (세로방향)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_PLYWOOD, "Vertical");

			// 체크박스: 규격폼
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20, 50, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "규격폼");
			DGDisableItem (dialogID, CHECKBOX_SET_STANDARD);
			
			// 테이블폼은 규격폼만 사용할 수 있음
			if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == SLAB_TABLEFORM + 1) {
				DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
			} else {
				DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, FALSE);
			}

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_TABLEFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TABLEFORM_WIDTH_OPTIONS, "너비");

			// 팝업 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 80-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "3032");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "2426");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_TABLEFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TABLEFORM_HEIGHT_OPTIONS, "높이");

			// 팝업 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 110-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "1818");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 110-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_TABLEFORM_ORIENTATION_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TABLEFORM_ORIENTATION_OPTIONS, "설치방향");
			
			// 라디오 버튼: 설치방향 (가로방향)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_TABLEFORM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_TABLEFORM, "Horizontal");
			// 라디오 버튼: 설치방향 (세로방향)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_TABLEFORM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_TABLEFORM, "Vertical");

			// 초기 입력 필드 표시
			if (placingZone.cells [rIdx][cIdx].objType == SLAB_TABLEFORM) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, SLAB_TABLEFORM + 1);

				// 체크박스: 규격폼
				DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
				DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
				DGDisableItem (dialogID, CHECKBOX_SET_STANDARD);

				// 라벨: 너비
				DGShowItem (dialogID, LABEL_TABLEFORM_WIDTH_OPTIONS);

				// 팝업 컨트롤: 너비
				DGShowItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS);
				if (abs(placingZone.cells [rIdx][cIdx].libPart.tableform.horLen - 3.032) < EPS)		popupSelectedIdx = 1;
				if (abs(placingZone.cells [rIdx][cIdx].libPart.tableform.horLen - 2.426) < EPS)		popupSelectedIdx = 2;
				DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS, popupSelectedIdx);

				// 라벨: 높이
				DGShowItem (dialogID, LABEL_TABLEFORM_HEIGHT_OPTIONS);

				// 팝업 컨트롤: 높이
				DGShowItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS);
				if (abs(placingZone.cells [rIdx][cIdx].libPart.tableform.verLen - 1.818) < EPS)		popupSelectedIdx = 1;
				DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS, popupSelectedIdx);

				//	// 라벨: 너비
				//	DGShowItem (dialogID, LABEL_TABLEFORM_WIDTH_OPTIONS);

				//	// Edit 컨트롤: 너비
				//	DGShowItem (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);
				//	DGSetItemValDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2);
				//	DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 0.110);
				//	DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 1.220);

				//	// 라벨: 높이
				//	DGShowItem (dialogID, LABEL_TABLEFORM_HEIGHT_OPTIONS);

				//	// Edit 컨트롤: 높이
				//	DGShowItem (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
				//	DGSetItemValDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, placingZone.cells[rIdx][cIdx].libPart.form.eu_hei2);
				//	DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 0.110);
				//	DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 2.440);
				//}

				// 라벨: 설치방향
				DGShowItem (dialogID, LABEL_TABLEFORM_ORIENTATION_OPTIONS);
				
				// 라디오 버튼: 설치방향 (벽세우기)
				DGShowItem (dialogID, RADIO_ORIENTATION_1_TABLEFORM);
				// 라디오 버튼: 설치방향 (벽눕히기)
				DGShowItem (dialogID, RADIO_ORIENTATION_2_TABLEFORM);

				if (placingZone.cells [rIdx][cIdx].libPart.tableform.direction == true) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_TABLEFORM, true);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_TABLEFORM, false);
				} else if (placingZone.cells [rIdx][cIdx].libPart.tableform.direction == false) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_TABLEFORM, false);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_TABLEFORM, true);
				}

				DGDisableItem (dialogID, RADIO_ORIENTATION_1_TABLEFORM);
				DGDisableItem (dialogID, RADIO_ORIENTATION_2_TABLEFORM);

			} else if (placingZone.cells [rIdx][cIdx].objType == PLYWOOD) {

				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PLYWOOD + 1);

				// 체크박스: 규격폼
				DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
				DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, FALSE);
				DGDisableItem (dialogID, CHECKBOX_SET_STANDARD);

				// 라벨: 너비
				DGShowItem (dialogID, LABEL_TABLEFORM_WIDTH_OPTIONS);

				//// 팝업 컨트롤: 너비
				//DGShowItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS);
				//if (abs(placingZone.cells [rIdx][cIdx].libPart.tableform.horLen - 3.032) < EPS)		popupSelectedIdx = 1;
				//DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS, popupSelectedIdx);

				//// 라벨: 높이
				//DGShowItem (dialogID, LABEL_TABLEFORM_HEIGHT_OPTIONS);

				//// 팝업 컨트롤: 높이
				//DGShowItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS);
				//if (abs(placingZone.cells [rIdx][cIdx].libPart.tableform.verLen - 1.818) < EPS)		popupSelectedIdx = 1;
				//DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS, popupSelectedIdx);

				// 라벨: 너비
				DGShowItem (dialogID, LABEL_TABLEFORM_WIDTH_OPTIONS);

				// Edit 컨트롤: 너비
				DGShowItem (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);
				DGSetItemValDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, placingZone.cells [rIdx][cIdx].horLen);
				DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 0.110);
				//DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 1.220);

				// 라벨: 높이
				DGShowItem (dialogID, LABEL_TABLEFORM_HEIGHT_OPTIONS);

				// Edit 컨트롤: 높이
				DGShowItem (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
				DGSetItemValDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, placingZone.cells[rIdx][cIdx].verLen);
				DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 0.110);
				//DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 2.440);

				// 라벨: 설치방향
				DGShowItem (dialogID, LABEL_TABLEFORM_ORIENTATION_OPTIONS);
				
				// 라디오 버튼: 설치방향 (벽세우기)
				DGShowItem (dialogID, RADIO_ORIENTATION_1_TABLEFORM);
				// 라디오 버튼: 설치방향 (벽눕히기)
				DGShowItem (dialogID, RADIO_ORIENTATION_2_TABLEFORM);

				if (placingZone.cells [rIdx][cIdx].libPart.tableform.direction == true) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_TABLEFORM, true);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_TABLEFORM, false);
				} else if (placingZone.cells [rIdx][cIdx].libPart.tableform.direction == false) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_TABLEFORM, false);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_TABLEFORM, true);
				}

				DGDisableItem (dialogID, RADIO_ORIENTATION_1_TABLEFORM);
				DGDisableItem (dialogID, RADIO_ORIENTATION_2_TABLEFORM);
			}

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case POPUP_OBJ_TYPE:	// 객체 타입 콤보박스 값을 변경할 때마다 입력 필드가 달라짐 (변경해야 하므로 Cell 값을 불러오지 않음)
					//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
					// 일단 항목을 숨기고, 객체 타입 관련 항목만 표시함
					DGHideItem (dialogID, LABEL_WIDTH);
					DGHideItem (dialogID, EDITCONTROL_WIDTH);
					DGHideItem (dialogID, LABEL_HEIGHT);
					DGHideItem (dialogID, EDITCONTROL_HEIGHT);
					DGHideItem (dialogID, LABEL_ORIENTATION);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD);
					DGHideItem (dialogID, CHECKBOX_SET_STANDARD);
					DGHideItem (dialogID, LABEL_TABLEFORM_WIDTH_OPTIONS);
					DGHideItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS);
					DGHideItem (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);
					DGHideItem (dialogID, LABEL_TABLEFORM_HEIGHT_OPTIONS);
					DGHideItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS);
					DGHideItem (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
					DGHideItem (dialogID, LABEL_TABLEFORM_ORIENTATION_OPTIONS);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_TABLEFORM);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_TABLEFORM);

					DGShowItem (dialogID, LABEL_OBJ_TYPE);
					DGShowItem (dialogID, POPUP_OBJ_TYPE);

					// 테이블폼은 규격폼만 사용할 수 있음, 다른 부재는 규격폼 사용 불가
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == SLAB_TABLEFORM + 1) {
						// 체크박스: 규격폼
						DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, true);

						// 라벨: 너비
						DGShowItem (dialogID, LABEL_TABLEFORM_WIDTH_OPTIONS);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_TABLEFORM_HEIGHT_OPTIONS);

						// 규격폼이면,
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
							// 팝업 컨트롤: 너비
							DGShowItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS);
							DGHideItem (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);

							// 팝업 컨트롤: 높이
							DGShowItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS);
							DGHideItem (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
						} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
							// Edit 컨트롤: 너비
							DGHideItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS);
							DGShowItem (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);
							DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 0.110);
							//DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 1.220);

							// Edit 컨트롤: 높이
							DGHideItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS);
							DGShowItem (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
							DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 0.110);
							//DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 2.440);
						}

						// 라벨: 설치방향
						DGShowItem (dialogID, LABEL_TABLEFORM_ORIENTATION_OPTIONS);
				
						// 라디오 버튼: 설치방향 (벽세우기)
						DGShowItem (dialogID, RADIO_ORIENTATION_1_TABLEFORM);
						// 라디오 버튼: 설치방향 (벽눕히기)
						DGShowItem (dialogID, RADIO_ORIENTATION_2_TABLEFORM);

						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_TABLEFORM, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_TABLEFORM, false);

						DGDisableItem (dialogID, RADIO_ORIENTATION_1_TABLEFORM);
						DGDisableItem (dialogID, RADIO_ORIENTATION_2_TABLEFORM);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PLYWOOD + 1) {
						// 체크박스: 규격폼
						DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, false);

						// 라벨: 너비
						DGShowItem (dialogID, LABEL_TABLEFORM_WIDTH_OPTIONS);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_TABLEFORM_HEIGHT_OPTIONS);

						// 규격폼이면,
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
							//// 팝업 컨트롤: 너비
							//DGShowItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS);
							//DGHideItem (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);

							//// 팝업 컨트롤: 높이
							//DGShowItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS);
							//DGHideItem (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
						} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
							// Edit 컨트롤: 너비
							DGHideItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS);
							DGShowItem (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);
							DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 0.110);
							//DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 1.220);

							// Edit 컨트롤: 높이
							DGHideItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS);
							DGShowItem (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
							DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 0.110);
							//DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 2.440);
						}

						// 라벨: 설치방향
						DGShowItem (dialogID, LABEL_TABLEFORM_ORIENTATION_OPTIONS);
				
						// 라디오 버튼: 설치방향 (벽세우기)
						DGShowItem (dialogID, RADIO_ORIENTATION_1_TABLEFORM);
						// 라디오 버튼: 설치방향 (벽눕히기)
						DGShowItem (dialogID, RADIO_ORIENTATION_2_TABLEFORM);

						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_TABLEFORM, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_TABLEFORM, false);

						DGDisableItem (dialogID, RADIO_ORIENTATION_1_TABLEFORM);
						DGDisableItem (dialogID, RADIO_ORIENTATION_2_TABLEFORM);
					}

					break;

				case CHECKBOX_SET_STANDARD:	// 테이블폼의 경우, 규격폼 체크박스 값을 바꿀 때마다 너비, 높이 입력 필드 타입이 바뀜
					//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
						// 팝업 컨트롤: 너비
						DGShowItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS);
						DGHideItem (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);
						// 팝업 컨트롤: 높이
						DGShowItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS);
						DGHideItem (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
					} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
						// Edit 컨트롤: 너비
						DGHideItem (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS);
						DGShowItem (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);
						DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 0.110);
						//DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS, 1.220);
						// Edit 컨트롤: 높이
						DGHideItem (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS);
						DGShowItem (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
						DGSetItemMinDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 0.110);
						//DGSetItemMaxDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS, 2.440);
					}

					break;
			}

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					// slabBottomTableformPlacerHandler2 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
					idxCell = (clickedBtnItemIdx - itemInitIdx);
					rIdx = 0;
					while (idxCell >= (placingZone.tb_count_hor)) {
						idxCell -= ((placingZone.tb_count_hor));
						++rIdx;
					}
					cIdx = idxCell;

					//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
					// 입력한 값을 다시 셀에 저장
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
						placingZone.cells [rIdx][cIdx].objType = NONE;
						placingZone.adjustOtherCellsInSameRow (&placingZone, rIdx, cIdx);
						placingZone.adjustOtherCellsInSameCol (&placingZone, rIdx, cIdx);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == SLAB_TABLEFORM + 1) {
						placingZone.cells [rIdx][cIdx].objType = SLAB_TABLEFORM;

						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
							// 너비
							placingZone.cells [rIdx][cIdx].libPart.tableform.horLen = atof (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_WIDTH_OPTIONS)).ToCStr ()) / 1000.0;
							placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].libPart.tableform.horLen;
							// 높이
							placingZone.cells [rIdx][cIdx].libPart.tableform.verLen = atof (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_HEIGHT_OPTIONS)).ToCStr ()) / 1000.0;
							placingZone.cells [rIdx][cIdx].verLen = placingZone.cells [rIdx][cIdx].libPart.tableform.verLen;
						} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
							// 너비
							placingZone.cells [rIdx][cIdx].libPart.tableform.horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);
							placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].libPart.tableform.horLen;
							// 높이
							placingZone.cells [rIdx][cIdx].libPart.tableform.verLen = DGGetItemValDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
							placingZone.cells [rIdx][cIdx].verLen = placingZone.cells [rIdx][cIdx].libPart.tableform.verLen;
						}

						// 설치방향
						if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_TABLEFORM) == TRUE)
							placingZone.cells [rIdx][cIdx].libPart.tableform.direction = true;
						else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_TABLEFORM) == FALSE) {
							placingZone.cells [rIdx][cIdx].libPart.tableform.direction = false;
							// 가로, 세로 길이 교환
							temp = placingZone.cells [rIdx][cIdx].horLen;
							placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].verLen;
							placingZone.cells [rIdx][cIdx].verLen = temp;
						}

						placingZone.adjustOtherCellsInSameRow (&placingZone, rIdx, cIdx);
						placingZone.adjustOtherCellsInSameCol (&placingZone, rIdx, cIdx);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PLYWOOD + 1) {

						placingZone.cells [rIdx][cIdx].objType = PLYWOOD;

						// 너비
						placingZone.cells [rIdx][cIdx].libPart.plywood.p_wid = DGGetItemValDouble (dialogID, EDITCONTROL_TABLEFORM_WIDTH_OPTIONS);
						placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].libPart.plywood.p_wid;
						// 높이
						placingZone.cells [rIdx][cIdx].libPart.plywood.p_leng = DGGetItemValDouble (dialogID, EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS);
						placingZone.cells [rIdx][cIdx].verLen = placingZone.cells [rIdx][cIdx].libPart.plywood.p_leng;

						if (placingZone.cells [rIdx][cIdx].horLen < placingZone.cells [rIdx][cIdx].verLen) {
							placingZone.adjustOtherCellsInSameCol (&placingZone, rIdx, cIdx);
						} else {
							placingZone.adjustOtherCellsInSameRow (&placingZone, rIdx, cIdx);
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

// 자투리 채우기를 할 때, 목재의 너비를 변경하기 위한 4차 다이얼로그
short DGCALLBACK slabBottomTableformPlacerHandler4 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;

	switch (message) {
		case DG_MSG_INIT:

			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "목재 너비 설정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 50, 110, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 130, 110, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
			// 라벨: 목재 너비
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WOOD_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WOOD_WIDTH, "목재 너비");
			DGShowItem (dialogID, LABEL_WOOD_WIDTH);

			// Edit 컨트롤: 목재 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WOOD_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, EDITCONTROL_WOOD_WIDTH, 0.080);
			DGShowItem (dialogID, EDITCONTROL_WOOD_WIDTH);
			DGSetItemMinDouble (dialogID, EDITCONTROL_WOOD_WIDTH, 0.010);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_WOOD_WIDTH, 0.100);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					placingZone.woodWidth = DGGetItemValDouble (dialogID, EDITCONTROL_WOOD_WIDTH);
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
