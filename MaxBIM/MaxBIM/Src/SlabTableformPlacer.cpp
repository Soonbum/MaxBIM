#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabTableformPlacer.hpp"

using namespace slabTableformPlacerDG;

static SlabTableformPlacingZone		placingZone;	// 기본 슬래브 하부 영역 정보
static InfoSlab		infoSlab;						// 슬래브 객체 정보

API_Guid	structuralObject_forTableformSlab;		// 구조 객체의 GUID

static short		clickedBtnItemIdx;			// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
static bool			clickedExcludeRestButton;	// 자투리 제외 버튼을 눌렀습니까?
static bool			clickedPrevButton;			// 이전 버튼을 눌렀습니까?
static short		clickedRow, clickedCol;		// 클릭한 행, 열 인덱스
static short		layerInd_Euroform;			// 레이어 번호: 유로폼
static short		layerInd_SlabTableform;		// 레이어 번호: 슬래브 테이블폼
static short		layerInd_Plywood;			// 레이어 번호: 합판
static short		layerInd_Timber;			// 레이어 번호: 각재
static short		layerInd_CProfile;			// 레이어 번호: KS프로파일 - C형강
static short		layerInd_Pinbolt;			// 레이어 번호: 핀볼트
static short		layerInd_Fittings;			// 레이어 번호: 결합철물
static short		layerInd_GT24Girder;		// 레이어 번호: GT24 거더
static short		layerInd_PERI_Support;		// 레이어 번호: PERI동바리 수직재
static short		layerInd_Steel_Support;		// 레이어 번호: 강관 동바리
static API_Coord3D		firstClickPoint;		// 1번째로 클릭한 점
static GS::Array<API_Guid>	elemList;			// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함
short	MAX_IND = 50;


// 슬래브 하부에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy;
	double		dx, dy, ang;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		slabs;
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
	GS::Array<API_Coord3D>	coords;

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


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		WriteReport_Alert ("열린 프로젝트 창이 없습니다.");
	}
	if (err == APIERR_NOSEL) {
		WriteReport_Alert ("아무 것도 선택하지 않았습니다.\n필수 선택: 슬래브 (1개), 슬래브 하부를 덮는 모프 (1개)");
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
		WriteReport_Alert ("슬래브를 1개 선택해야 합니다.");
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorphs != 1) {
		WriteReport_Alert ("슬래브 하부를 덮는 모프를 1개 선택하셔야 합니다.");
		err = APIERR_GENERAL;
		return err;
	}

	// (1) 슬래브 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = slabs.Pop ();
	structuralObject_forTableformSlab = elem.header.guid;
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
		WriteReport_Alert ("모프가 누워 있지 않습니다.");
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
		WriteReport_Alert ("모프의 3D 모델을 가져오지 못했습니다.");
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
			if ( (abs (trCoord.x - 1.0) < EPS) || (abs (trCoord.y - 0.0) < EPS) || (abs (trCoord.z - 0.0) < EPS) )
				;
			else if ( (abs (trCoord.x - 0.0) < EPS) || (abs (trCoord.y - 1.0) < EPS) || (abs (trCoord.z - 0.0) < EPS) )
				;
			else if ( (abs (trCoord.x - 0.0) < EPS) || (abs (trCoord.y - 0.0) < EPS) || (abs (trCoord.z - 1.0) < EPS) )
				;
			else if ( (abs (trCoord.x - 0.0) < EPS) || (abs (trCoord.y - 0.0) < EPS) || (abs (trCoord.z - 0.0) < EPS) )
				;
			else
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
		WriteReport_Alert ("폴리곤에 속하지 않은 점을 클릭했습니다.");
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

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_slab = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoSlab.floorInd) {
			workLevel_slab = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 최외곽 가로, 세로 길이 계산
	double	xMin, xMax;
	double	yMin, yMax;

	xMin = xMax = nodes_sequential [0].x;
	yMin = yMax = nodes_sequential [0].y;

	for (xx = 1 ; xx < nEntered ; ++xx) {
		if (xMin > nodes_sequential [xx].x)		xMin = nodes_sequential [xx].x;
		if (xMax < nodes_sequential [xx].x)		xMax = nodes_sequential [xx].x;
		
		if (yMin > nodes_sequential [xx].y)		yMin = nodes_sequential [xx].y;
		if (yMax < nodes_sequential [xx].y)		yMax = nodes_sequential [xx].y;
	}

	placingZone.borderHorLen = xMax - xMin;
	placingZone.borderVerLen = yMax - yMin;

	// 최외곽 좌하단 점, 우상단 점 찾기
	if (GetDistance (nodes_sequential [0], nodes_sequential [nEntered - 2]) < (placingZone.borderHorLen/2)) {
		// 코너가 꺾인 모프일 경우
		placingZone.bRectangleArea = false;

		// 좌하단 점
		placingZone.leftBottom = getUnrotatedPoint (nodes_sequential [nEntered - 2], nodes_sequential [0], RadToDegree (placingZone.ang));
		moveIn2D ('y', placingZone.ang, GetDistance (nodes_sequential [0], nodes_sequential [nEntered - 1]), &placingZone.leftBottom.x, &placingZone.leftBottom.y);
		// 우상단 점
		placingZone.rightTop = placingZone.leftBottom;
		moveIn2D ('x', placingZone.ang, placingZone.borderHorLen, &placingZone.rightTop.x, &placingZone.rightTop.y);
		moveIn2D ('y', placingZone.ang, -placingZone.borderVerLen, &placingZone.rightTop.x, &placingZone.rightTop.y);
	} else {
		// 코너가 꺾인 모프가 아닐 경우
		placingZone.bRectangleArea = true;

		// 좌하단 점
		placingZone.leftBottom = nodes_sequential [0];
		// 우상단 점
		placingZone.rightTop = placingZone.leftBottom;
		moveIn2D ('x', placingZone.ang, placingZone.borderHorLen, &placingZone.rightTop.x, &placingZone.rightTop.y);
		moveIn2D ('y', placingZone.ang, -placingZone.borderVerLen, &placingZone.rightTop.x, &placingZone.rightTop.y);
	}	

	// 최외곽 중심 점 찾기
	placingZone.center = placingZone.leftBottom;
	moveIn2D ('x', placingZone.ang, placingZone.borderHorLen/2, &placingZone.center.x, &placingZone.center.y);
	moveIn2D ('y', placingZone.ang, -placingZone.borderVerLen/2, &placingZone.center.x, &placingZone.center.y);

FIRST:

	// [DIALOG] 1번째 다이얼로그에서 테이블폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32502, ACAPI_GetOwnResModule (), slabBottomTableformPlacerHandler1, 0);

	// 영역 정보의 고도 정보 수정
	placingZone.level = infoSlab.level + workLevel_slab + infoSlab.offsetFromTop - infoSlab.thickness - placingZone.gap;

	if (result != DG_OK)
		return err;

	// placingZone의 Cell 정보 초기화
	placingZone.initCells (&placingZone);

	// [DIALOG] 2번째 다이얼로그에서 테이블폼 배치를 수정하거나 보강 목재를 삽입합니다.
	clickedExcludeRestButton = false;	// 자투리 채우기 제외 여부 클릭 여부
	clickedPrevButton = false;			// 이전 버튼 클릭 여부
	result = DGBlankModalDialog (450, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomTableformPlacerHandler2, 0);

	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST;

	// 테이블폼 영역 채우기
	err = placingZone.fillTableformAreas ();

	// 나머지 영역 채우기 - 합판, 목재
	if (clickedExcludeRestButton == false)
		err = placingZone.fillRestAreas ();

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

	// 초기 셀 설정
	for (xx = 0 ; xx < MAX_IND ; ++xx) {
		for (yy = 0 ; yy < MAX_IND ; ++yy) {
			placingZone->cells [xx][yy].objType = placingZone->iTableformType;
			placingZone->cells [xx][yy].ang = placingZone->ang;
			placingZone->cells [xx][yy].horLen = placingZone->initHorLen;
			placingZone->cells [xx][yy].verLen = placingZone->initVerLen;
			placingZone->cells [xx][yy].leftBottomX = 0.0;
			placingZone->cells [xx][yy].leftBottomY = 0.0;
			placingZone->cells [xx][yy].leftBottomZ = placingZone->level;
		}
	}

	// 초기 셀 개수 지정
	if (placingZone->bVertical == true) {
		placingZone->nHorCells = (short)floor (placingZone->borderHorLen / placingZone->initHorLen);
		placingZone->nVerCells = (short)floor (placingZone->borderVerLen / placingZone->initVerLen);
	} else {
		placingZone->nHorCells = (short)floor (placingZone->borderHorLen / placingZone->initVerLen);
		placingZone->nVerCells = (short)floor (placingZone->borderVerLen / placingZone->initHorLen);
	}

	// 초기 테이블폼 배열 전체 너비, 높이 지정
	placingZone->formArrayWidth = 0.0;
	placingZone->formArrayHeight = 0.0;
	if (placingZone->bVertical == true) {
		for (xx = 0 ; xx < placingZone->nHorCells ; ++xx)	placingZone->formArrayWidth += placingZone->cells [0][xx].horLen;
		for (xx = 0 ; xx < placingZone->nVerCells ; ++xx)	placingZone->formArrayHeight += placingZone->cells [xx][0].verLen;
	} else {
		for (xx = 0 ; xx < placingZone->nHorCells ; ++xx)	placingZone->formArrayWidth += placingZone->cells [0][xx].verLen;
		for (xx = 0 ; xx < placingZone->nVerCells ; ++xx)	placingZone->formArrayHeight += placingZone->cells [xx][0].horLen;
	}

	// 초기 여백 길이 지정
	placingZone->marginLeft = placingZone->marginRight = (placingZone->borderHorLen - placingZone->formArrayWidth) / 2;
	placingZone->marginBottom = placingZone->marginTop = (placingZone->borderVerLen - placingZone->formArrayHeight) / 2;
}

// Cell 배열의 위치를 조정함
void	SlabTableformPlacingZone::alignPlacingZone (SlabTableformPlacingZone* placingZone)
{
	short	xx, yy;
	double	accumLength;

	API_Coord3D	point = placingZone->leftBottom;

	moveIn2D ('x', placingZone->ang, placingZone->marginLeft, &point.x, &point.y);
	moveIn2D ('y', placingZone->ang, -placingZone->marginBottom, &point.x, &point.y);

	// 테이블폼 시작 좌표 지정
	placingZone->leftBottomX = point.x;
	placingZone->leftBottomY = point.y;
	placingZone->leftBottomZ = placingZone->level;

	if (placingZone->bVertical) {
		for (xx = 0 ; xx < placingZone->nVerCells ; ++xx) {
			accumLength = 0.0;
			for (yy = 0 ; yy < placingZone->nHorCells ; ++yy) {
				placingZone->cells [xx][yy].ang = placingZone->ang;
				placingZone->cells [xx][yy].leftBottomX = point.x;
				placingZone->cells [xx][yy].leftBottomY = point.y;
				placingZone->cells [xx][yy].leftBottomZ = placingZone->level;

				moveIn2D ('x', placingZone->ang, placingZone->cells [xx][yy].horLen, &point.x, &point.y);
				accumLength += placingZone->cells [xx][yy].horLen;
			}
			moveIn2D ('x', placingZone->ang, -accumLength, &point.x, &point.y);
			moveIn2D ('y', placingZone->ang, -placingZone->cells [xx][0].verLen, &point.x, &point.y);
		}
	} else {
		for (xx = 0 ; xx < placingZone->nVerCells ; ++xx) {
			accumLength = 0.0;
			for (yy = 0 ; yy < placingZone->nHorCells ; ++yy) {
				placingZone->cells [xx][yy].ang = placingZone->ang;
				placingZone->cells [xx][yy].leftBottomX = point.x;
				placingZone->cells [xx][yy].leftBottomY = point.y;
				placingZone->cells [xx][yy].leftBottomZ = placingZone->level;

				moveIn2D ('x', placingZone->ang, placingZone->cells [xx][yy].verLen, &point.x, &point.y);
				accumLength += placingZone->cells [xx][yy].verLen;
			}
			moveIn2D ('x', placingZone->ang, -accumLength, &point.x, &point.y);
			moveIn2D ('y', placingZone->ang, -placingZone->cells [xx][0].horLen, &point.x, &point.y);
		}
	}
}

//// 해당 셀 정보를 기반으로 라이브러리 배치
//API_Guid	SlabTableformPlacingZone::placeLibPart (CellForSlabTableform objInfo)
//{
//	GSErrCode	err = NoError;
//
//	API_Element			element;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const	GS::uchar_t* gsmName = NULL;
//	double	aParam;
//	double	bParam;
//	Int32	addParNum;
//
//	char	tempString [20];
//
//	// GUID 변수 초기화
//	element.header.guid.clock_seq_hi_and_reserved = 0;
//	element.header.guid.clock_seq_low = 0;
//	element.header.guid.node[0] = 0;
//	element.header.guid.node[1] = 0;
//	element.header.guid.node[2] = 0;
//	element.header.guid.node[3] = 0;
//	element.header.guid.node[4] = 0;
//	element.header.guid.node[5] = 0;
//	element.header.guid.time_hi_and_version = 0;
//	element.header.guid.time_low = 0;
//	element.header.guid.time_mid = 0;
//
//	// 라이브러리 이름 선택
//	if (objInfo.objType == NONE)			return element.header.guid;
//	if (objInfo.objType == SLAB_TABLEFORM)	gsmName = L("슬래브 테이블폼 (콘판넬) v1.0.gsm");
//	if (objInfo.objType == PLYWOOD)			gsmName = L("합판v1.0.gsm");
//	if (objInfo.objType == WOOD)			gsmName = L("목재v1.0.gsm");
//
//	// 객체 로드
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return element.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	BNZeroMemory (&element, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//
//	element.header.typeID = API_ObjectID;
//	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&element, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// 라이브러리의 파라미터 값 입력
//	element.object.libInd = libPart.index;
//	element.object.reflected = false;
//	element.object.pos.x = objInfo.leftBottomX;
//	element.object.pos.y = objInfo.leftBottomY;
//	element.object.level = objInfo.leftBottomZ;
//	element.object.xRatio = aParam;
//	element.object.yRatio = bParam;
//	element.object.angle = objInfo.ang;
//	element.header.floorInd = infoSlab.floorInd;
//
//	// 슬래브 테이블폼에 C형강을 부착할 때 계산하는 양끝 여백
//	double	marginEnds;
//
//	if (objInfo.objType == SLAB_TABLEFORM) {
//		element.header.layer = layerInd_SlabTableform;
//
//		// 가로방향 (Horizontal)
//		if (objInfo.libPart.tableform.direction == true) {
//			// 타입: 가로 길이 x 세로 길이
//			sprintf (tempString, "%.0f x %.0f", round (objInfo.horLen, 3) * 1000, round (objInfo.verLen, 3) * 1000);
//			element.object.xRatio = objInfo.horLen;
//			element.object.yRatio = objInfo.verLen;
//
//			// 이동하여 위치 바로잡기
//			element.object.pos.x += ( objInfo.verLen * sin(objInfo.ang) );
//			element.object.pos.y -= ( objInfo.verLen * cos(objInfo.ang) );
//
//			setParameterByName (&memo, "bLprofileOnNorth", objInfo.libPart.tableform.bLprofileOnNorth);
//			setParameterByName (&memo, "bLprofileOnSouth", objInfo.libPart.tableform.bLprofileOnSouth);
//			setParameterByName (&memo, "bLprofileOnWest", objInfo.libPart.tableform.bLprofileOnWest);
//			setParameterByName (&memo, "bLprofileOnEast", objInfo.libPart.tableform.bLprofileOnEast);
//
//			// C형강 설치
//			CellForSlabTableform	cprofile;
//
//			cprofile.objType = CPROFILE;
//			cprofile.leftBottomX = objInfo.leftBottomX;
//			cprofile.leftBottomY = objInfo.leftBottomY;
//			cprofile.leftBottomZ = objInfo.leftBottomZ;
//			cprofile.ang = objInfo.ang - DegreeToRad (90.0);
//			cprofile.libPart.cprofile.angX = DegreeToRad (270.0);
//			cprofile.libPart.cprofile.angY = DegreeToRad (0.0);
//			cprofile.libPart.cprofile.iAnchor = 8;
//			cprofile.libPart.cprofile.len = floor (objInfo.horLen * 10) / 10;
//			marginEnds = objInfo.horLen - cprofile.libPart.cprofile.len;
//
//			moveIn3D ('x', cprofile.ang, 0.300 + 0.006 + 0.020, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, marginEnds / 2, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('z', cprofile.ang, -0.0615, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//			moveIn3D ('x', cprofile.ang, -(0.300 + 0.006 + 0.020) + objInfo.verLen - (0.300 - 0.006 - 0.020), &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//
//			cprofile.ang = objInfo.ang + DegreeToRad (90.0);
//			cprofile.leftBottomX = objInfo.leftBottomX;
//			cprofile.leftBottomY = objInfo.leftBottomY;
//			moveIn3D ('x', cprofile.ang, -objInfo.verLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, -objInfo.horLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('x', cprofile.ang, 0.300 + 0.006 + 0.020, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, marginEnds / 2, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//			moveIn3D ('x', cprofile.ang, -(0.300 + 0.006 + 0.020) + objInfo.verLen - (0.300 - 0.006 - 0.020), &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//
//			// 결합철물 (사각와셔활용) 설치
//			CellForSlabTableform	fittings;
//
//			fittings.objType = FITTINGS;
//			fittings.leftBottomX = objInfo.leftBottomX;
//			fittings.leftBottomY = objInfo.leftBottomY;
//			fittings.leftBottomZ = objInfo.leftBottomZ;
//			fittings.ang = objInfo.ang;
//			fittings.libPart.fittings.angX = DegreeToRad (270.0);
//			fittings.libPart.fittings.angY = DegreeToRad (0.0);
//			fittings.libPart.fittings.bolt_len = 0.150;
//			fittings.libPart.fittings.bolt_dia = 0.012;
//			fittings.libPart.fittings.bWasher1 = false;
//			fittings.libPart.fittings.washer_pos1 = 0.0;
//			fittings.libPart.fittings.bWasher2 = true;
//			fittings.libPart.fittings.washer_pos2 = 0.0766;
//			fittings.libPart.fittings.washer_size = 0.100;
//			strncpy (fittings.libPart.fittings.nutType, "육각너트", strlen ("육각너트"));
//
//			moveIn3D ('x', fittings.ang, 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			moveIn3D ('y', fittings.ang, -0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			moveIn3D ('z', fittings.ang, -0.0499, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('y', fittings.ang, 0.300 - objInfo.verLen + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('x', fittings.ang, -0.328 + objInfo.horLen - 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('y', fittings.ang, -0.300 + objInfo.verLen - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			
//		// 세로방향 (Vertical)
//		} else {
//			// 타입: 세로 길이 x 가로 길이
//			sprintf (tempString, "%.0f x %.0f", round (objInfo.verLen, 3) * 1000, round (objInfo.horLen, 3) * 1000);
//			element.object.xRatio = objInfo.verLen;
//			element.object.yRatio = objInfo.horLen;
//			
//			// 90도 회전된 상태로 배치
//			element.object.angle += DegreeToRad (90.0);
//
//			// 이동하여 위치 바로잡기
//			element.object.pos.x += ( objInfo.verLen * sin(objInfo.ang) );
//			element.object.pos.y -= ( objInfo.verLen * cos(objInfo.ang) );
//			element.object.pos.x += ( objInfo.horLen * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.horLen * sin(objInfo.ang) );
//
//			setParameterByName (&memo, "bLprofileOnNorth", objInfo.libPart.tableform.bLprofileOnNorth);
//			setParameterByName (&memo, "bLprofileOnSouth", objInfo.libPart.tableform.bLprofileOnSouth);
//			setParameterByName (&memo, "bLprofileOnWest", objInfo.libPart.tableform.bLprofileOnWest);
//			setParameterByName (&memo, "bLprofileOnEast", objInfo.libPart.tableform.bLprofileOnEast);
//
//			// C형강 설치
//			CellForSlabTableform	cprofile;
//
//			cprofile.objType = CPROFILE;
//			cprofile.leftBottomX = objInfo.leftBottomX;
//			cprofile.leftBottomY = objInfo.leftBottomY;
//			cprofile.leftBottomZ = objInfo.leftBottomZ;
//			cprofile.ang = objInfo.ang;
//			cprofile.libPart.cprofile.angX = DegreeToRad (270.0);
//			cprofile.libPart.cprofile.angY = DegreeToRad (0.0);
//			cprofile.libPart.cprofile.iAnchor = 8;
//			cprofile.libPart.cprofile.len = floor (objInfo.verLen * 10) / 10;
//			marginEnds = objInfo.verLen - cprofile.libPart.cprofile.len;
//
//			moveIn3D ('x', cprofile.ang, 0.300 + 0.006 + 0.020, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, marginEnds / 2 - objInfo.verLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('z', cprofile.ang, -0.0615, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//			moveIn3D ('x', cprofile.ang, -(0.300 + 0.006 + 0.020) + objInfo.horLen - (0.300 - 0.006 - 0.020), &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//
//			cprofile.ang = objInfo.ang + DegreeToRad (180.0);
//			cprofile.leftBottomX = objInfo.leftBottomX;
//			cprofile.leftBottomY = objInfo.leftBottomY;
//			moveIn3D ('x', cprofile.ang, -objInfo.horLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, -objInfo.verLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('x', cprofile.ang, 0.300 + 0.006 + 0.020, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, marginEnds / 2 + objInfo.verLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//			moveIn3D ('x', cprofile.ang, -(0.300 + 0.006 + 0.020) + objInfo.horLen - (0.300 - 0.006 - 0.020), &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//
//			// 결합철물 (사각와셔활용) 설치
//			CellForSlabTableform	fittings;
//
//			fittings.objType = FITTINGS;
//			fittings.leftBottomX = objInfo.leftBottomX;
//			fittings.leftBottomY = objInfo.leftBottomY;
//			fittings.leftBottomZ = objInfo.leftBottomZ;
//			fittings.ang = objInfo.ang;
//			fittings.libPart.fittings.angX = DegreeToRad (270.0);
//			fittings.libPart.fittings.angY = DegreeToRad (0.0);
//			fittings.libPart.fittings.bolt_len = 0.150;
//			fittings.libPart.fittings.bolt_dia = 0.012;
//			fittings.libPart.fittings.bWasher1 = false;
//			fittings.libPart.fittings.washer_pos1 = 0.0;
//			fittings.libPart.fittings.bWasher2 = true;
//			fittings.libPart.fittings.washer_pos2 = 0.0766;
//			fittings.libPart.fittings.washer_size = 0.100;
//			strncpy (fittings.libPart.fittings.nutType, "육각너트", strlen ("육각너트"));
//
//			moveIn3D ('x', fittings.ang, 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			moveIn3D ('y', fittings.ang, -0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			moveIn3D ('z', fittings.ang, -0.0499, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('x', fittings.ang, -0.300 + objInfo.horLen - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('y', fittings.ang, 0.328 - objInfo.verLen + 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('x', fittings.ang, 0.300 - objInfo.horLen + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//		}
//		setParameterByName (&memo, "type", tempString);
//
//	} else if (objInfo.objType == PLYWOOD) {
//		element.header.layer = layerInd_Plywood;
//		setParameterByName (&memo, "p_stan", "비규격");							// 규격
//		setParameterByName (&memo, "w_dir", "바닥깔기");						// 설치방향
//		setParameterByName (&memo, "p_thk", "11.5T");							// 두께
//		setParameterByName (&memo, "p_wid", objInfo.libPart.plywood.p_wid);		// 가로
//		setParameterByName (&memo, "p_leng", objInfo.libPart.plywood.p_leng);	// 세로
//		setParameterByName (&memo, "sogak", 0.0);								// 제작틀 OFF
//		
//	} else if (objInfo.objType == WOOD) {
//		element.header.layer = layerInd_Wood;
//		setParameterByName (&memo, "w_ins", "바닥눕히기");					// 설치방향
//		setParameterByName (&memo, "w_w", objInfo.libPart.wood.w_w);		// 두께
//		setParameterByName (&memo, "w_h", objInfo.libPart.wood.w_h);		// 너비
//		setParameterByName (&memo, "w_leng", objInfo.libPart.wood.w_leng);	// 길이
//		setParameterByName (&memo, "w_ang", objInfo.libPart.wood.w_ang);	// 각도
//	}
//
//	// 객체 배치
//	ACAPI_Element_Create (&element, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return element.header.guid;
//}
//
//// 슬래브 테이블폼의 부속 철물들에 해당하는 라이브러리 배치
//API_Guid	SlabTableformPlacingZone::placeLibPartOnSlabTableform (CellForSlabTableform objInfo)
//{
//	GSErrCode	err = NoError;
//
//	API_Element			element;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const	GS::uchar_t* gsmName = NULL;
//	double	aParam;
//	double	bParam;
//	Int32	addParNum;
//
//	// GUID 변수 초기화
//	element.header.guid.clock_seq_hi_and_reserved = 0;
//	element.header.guid.clock_seq_low = 0;
//	element.header.guid.node[0] = 0;
//	element.header.guid.node[1] = 0;
//	element.header.guid.node[2] = 0;
//	element.header.guid.node[3] = 0;
//	element.header.guid.node[4] = 0;
//	element.header.guid.node[5] = 0;
//	element.header.guid.time_hi_and_version = 0;
//	element.header.guid.time_low = 0;
//	element.header.guid.time_mid = 0;
//
//	// 라이브러리 이름 선택
//	if (objInfo.objType == NONE)			return element.header.guid;
//	if (objInfo.objType == CPROFILE)		gsmName = L("KS프로파일v1.0.gsm");
//	if (objInfo.objType == FITTINGS)		gsmName = L("결합철물 (사각와셔활용) v1.0.gsm");
//
//	// 객체 로드
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return element.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	BNZeroMemory (&element, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//
//	element.header.typeID = API_ObjectID;
//	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&element, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// 라이브러리의 파라미터 값 입력
//	element.object.libInd = libPart.index;
//	element.object.reflected = false;
//	element.object.pos.x = objInfo.leftBottomX;
//	element.object.pos.y = objInfo.leftBottomY;
//	element.object.level = objInfo.leftBottomZ;
//	element.object.xRatio = aParam;
//	element.object.yRatio = bParam;
//	element.object.angle = objInfo.ang;
//	element.header.floorInd = infoSlab.floorInd;
//
//	if (objInfo.objType == CPROFILE) {
//		element.header.layer = layerInd_CProfile;
//
//		setParameterByName (&memo, "angX", objInfo.libPart.cprofile.angX);	// 회전X
//		setParameterByName (&memo, "angY", objInfo.libPart.cprofile.angY);	// 회전Y
//
//		setParameterByName (&memo, "type", "보");									// 분류 (기둥, 보)
//		setParameterByName (&memo, "shape", "C형강");								// 형태 (C형강, H형강...)
//		setParameterByName (&memo, "iAnchor", objInfo.libPart.cprofile.iAnchor);	// 앵커 포인트
//		setParameterByName (&memo, "len", objInfo.libPart.cprofile.len);			// 길이
//		setParameterByName (&memo, "ZZYZX", objInfo.libPart.cprofile.len);			// 길이
//		setParameterByName (&memo, "nom", "75 x 40 x 5 x 7");						// 규격
//		setParameterByName (&memo, "HH", 0.075);
//		setParameterByName (&memo, "BB", 0.040);
//		setParameterByName (&memo, "t1", 0.005);
//		setParameterByName (&memo, "t2", 0.007);
//		setParameterByName (&memo, "mat", 19.0);									// 재질
//
//	} else if (objInfo.objType == FITTINGS) {
//		element.header.layer = layerInd_Fittings;
//
//		setParameterByName (&memo, "angX", objInfo.libPart.fittings.angX);	// 회전X
//		setParameterByName (&memo, "angY", objInfo.libPart.fittings.angY);	// 회전Y
//
//		setParameterByName (&memo, "bolt_len", objInfo.libPart.fittings.bolt_len);			// 볼트 길이
//		setParameterByName (&memo, "bolt_dia", objInfo.libPart.fittings.bolt_dia);			// 볼트 직경
//		setParameterByName (&memo, "bWasher1", objInfo.libPart.fittings.bWasher1);			// 와셔1 On/Off
//		setParameterByName (&memo, "washer_pos1", objInfo.libPart.fittings.washer_pos1);	// 와셔1 위치
//		setParameterByName (&memo, "bWasher2", objInfo.libPart.fittings.bWasher2);			// 와셔2 On/Off
//		setParameterByName (&memo, "washer_pos2", objInfo.libPart.fittings.washer_pos2);	// 와셔2 위치
//		setParameterByName (&memo, "washer_size", objInfo.libPart.fittings.washer_size);	// 와셔 크기
//		setParameterByName (&memo, "nutType", objInfo.libPart.fittings.nutType);			// 너트 타입
//	
//	}
//
//	// 객체 배치
//	ACAPI_Element_Create (&element, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return element.header.guid;
//}

// 테이블폼 공간 채우기
GSErrCode	SlabTableformPlacingZone::fillTableformAreas (void)
{
	GSErrCode	err = NoError;
	short	xx, yy;

	// 타입, 설치방향에 따라 다르게 배치함
	if (placingZone.iTableformType == EUROFORM) {
		// 유로폼 배치
		EasyObjectPlacement euroform;

		for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
			for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
				euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);
				if (placingZone.bVertical == false) {
					moveIn3D ('y', euroform.radAng, -placingZone.cells [xx][yy].horLen, &euroform.posX, &euroform.posY, &euroform.posZ);
					euroform.radAng += DegreeToRad (90.0);
				}
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone.cells [xx][yy].horLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone.cells [xx][yy].verLen * 1000),
					"u_ins", APIParT_CString, "벽세우기",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
			}
		}

		// C형강 배치
		// ...

		// 핀볼트 배치
		// ...
	} else if (placingZone.iTableformType == TABLEFORM) {
		// 콘판넬 배치
		EasyObjectPlacement tableform;

		char verLenIntStr [12];
		char horLenIntStr [12];
		char typeStr [24];
		char verLenDoubleStr [12];
		char horLenDoubleStr [12];

		for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
			for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
				tableform.init (L("슬래브 테이블폼 (콘판넬) v1.0.gsm"), layerInd_SlabTableform, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);
				if (placingZone.bVertical == true) {
					tableform.radAng -= DegreeToRad (90.0);
				} else {
					moveIn3D ('y', tableform.radAng, -placingZone.cells [xx][yy].horLen, &tableform.posX, &tableform.posY, &tableform.posZ);
				}
				sprintf (verLenIntStr, "%.0f", placingZone.cells [xx][yy].verLen * 1000);
				sprintf (horLenIntStr, "%.0f", placingZone.cells [xx][yy].horLen * 1000);
				sprintf (typeStr, "%s x %s", verLenIntStr, horLenIntStr);
				sprintf (verLenDoubleStr, "%f", placingZone.cells [xx][yy].verLen);
				sprintf (horLenDoubleStr, "%f", placingZone.cells [xx][yy].horLen);
				elemList.Push (tableform.placeObject (8,
					"type", APIParT_CString, typeStr,
					"A", APIParT_Length, verLenDoubleStr,
					"B", APIParT_Length, horLenDoubleStr,
					"ZZYZX", APIParT_Length, "0.0615",
					"bLprofileOnNorth", APIParT_Boolean, "0.0",
					"bLprofileOnSouth", APIParT_Boolean, "0.0",
					"bLprofileOnWest", APIParT_Boolean, "0.0",
					"bLprofileOnEast", APIParT_Boolean, "0.0"));
			}
		}

		// C형강 배치
		// ...

		// 결합철물 배치
		// ...

		// PERI동바리 배치
		// ...
	} else if (placingZone.iTableformType == PLYWOOD) {
		// 합판 배치
		EasyObjectPlacement plywood;

		for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
			for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
				plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);
				if (placingZone.bVertical == true) {
					plywood.radAng -= DegreeToRad (90.0);
				} else {
					moveIn3D ('y', plywood.radAng, -placingZone.cells [xx][yy].horLen, &plywood.posX, &plywood.posY, &plywood.posZ);
				}
				elemList.Push (plywood.placeObject (7,
					"p_stan", APIParT_CString, "비규격",
					"w_dir", APIParT_CString, "바닥깔기",
					"p_thk", APIParT_CString, "11.5T",
					"p_wid", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
					"p_leng", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
					"p_ang", APIParT_Angle, format_string ("%f", 0.0),
					"sogak", APIParT_Boolean, "0.0"));
			}
		}

		// GT24 거더 배치
		// ...

		// PERI동바리 배치
		// ...

		// 강관동바리 배치
		// ...
	}

	return err;
}

// 자투리 공간 채우기 (합판, 각재)
GSErrCode	SlabTableformPlacingZone::fillRestAreas (void)
{
	GSErrCode	err = NoError;
	short		xx;
//	EasyObjectPlacement	plywood, timber;
//	double		startXPos, startYPos;
//	API_Coord3D	axisPoint, rotatedPoint, unrotatedPoint;
//
//	// 회전축이 되는 점
//	axisPoint.x = firstClickPoint.x;
//	axisPoint.y = firstClickPoint.y;
//	axisPoint.z = firstClickPoint.y;
//
//	// 합판 설치 (TOP)
//	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, axisPoint.x, axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.upMove, placingZone.level, placingZone.ang);
//
//	// 위치 값을 비회전값으로 변환
//	rotatedPoint.x = plywood.posX;
//	rotatedPoint.y = plywood.posY;
//	rotatedPoint.z = plywood.posZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (plywood.radAng));
//	plywood.posX = unrotatedPoint.x;
//	plywood.posY = unrotatedPoint.y;
//	plywood.posZ = unrotatedPoint.z;
//
//	remainLength = placingZone.outerRight - placingZone.outerLeft;
//	while (remainLength > EPS) {
//		if (remainLength > (2.440 + EPS)) {
//			currentLength = 2.440;
//			remainLength -= 2.440;
//		} else {
//			currentLength = remainLength;
//			remainLength = 0;
//		}
//
//		elemList.Push (plywood.placeObject (7,
//			"p_stan", APIParT_CString, "비규격",
//			"w_dir", APIParT_CString, "바닥깔기",
//			"p_thk", APIParT_CString, "11.5T",
//			"p_wid", APIParT_Length, format_string ("%f", (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) + placingZone.upMove),
//			"p_leng", APIParT_Length, format_string ("%f", currentLength),
//			"p_ang", APIParT_Angle, format_string ("%f", 0.0),
//			"sogak", APIParT_Boolean, "0.0"));
//		moveIn3D ('x', plywood.radAng, currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
//	}
//
//	// 합판 설치 (BOTTOM)
//	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, axisPoint.x, axisPoint.y - (placingZone.outerTop - placingZone.outerBottom), placingZone.level, placingZone.ang);
//
//	// 위치 값을 비회전값으로 변환
//	rotatedPoint.x = plywood.posX;
//	rotatedPoint.y = plywood.posY;
//	rotatedPoint.z = plywood.posZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (plywood.radAng));
//	plywood.posX = unrotatedPoint.x;
//	plywood.posY = unrotatedPoint.y;
//	plywood.posZ = unrotatedPoint.z;
//
//	remainLength = placingZone.outerRight - placingZone.outerLeft;
//	while (remainLength > EPS) {
//		if (remainLength > (2.440 + EPS)) {
//			currentLength = 2.440;
//			remainLength -= 2.440;
//		} else {
//			currentLength = remainLength;
//			remainLength = 0;
//		}
//
//		elemList.Push (plywood.placeObject (7,
//			"p_stan", APIParT_CString, "비규격",
//			"w_dir", APIParT_CString, "바닥깔기",
//			"p_thk", APIParT_CString, "11.5T",
//			"p_wid", APIParT_Length, format_string ("%f", (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.upMove),
//			"p_leng", APIParT_Length, format_string ("%f", currentLength),
//			"p_ang", APIParT_Angle, format_string ("%f", 0.0),
//			"sogak", APIParT_Boolean, "0.0"));
//		moveIn3D ('x', plywood.radAng, currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
//	}
//
//	// 합판 설치 (LEFT)
//	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, axisPoint.x + ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove), axisPoint.y - ((placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) + placingZone.upMove), placingZone.level, placingZone.ang);
//
//	// 위치 값을 비회전값으로 변환
//	rotatedPoint.x = plywood.posX;
//	rotatedPoint.y = plywood.posY;
//	rotatedPoint.z = plywood.posZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (plywood.radAng));
//	plywood.posX = unrotatedPoint.x;
//	plywood.posY = unrotatedPoint.y;
//	plywood.posZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayHeight;
//	while (remainLength > EPS) {
//		if (remainLength > (2.440 + EPS)) {
//			currentLength = 2.440;
//			remainLength -= 2.440;
//		} else {
//			currentLength = remainLength;
//			remainLength = 0;
//		}
//
//		moveIn3D ('y', plywood.radAng, -currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
//		plywood.radAng += DegreeToRad (90.0);
//		elemList.Push (plywood.placeObject (7,
//			"p_stan", APIParT_CString, "비규격",
//			"w_dir", APIParT_CString, "바닥깔기",
//			"p_thk", APIParT_CString, "11.5T",
//			"p_wid", APIParT_Length, format_string ("%f", (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove),
//			"p_leng", APIParT_Length, format_string ("%f", currentLength),
//			"p_ang", APIParT_Angle, format_string ("%f", 0.0),
//			"sogak", APIParT_Boolean, "0.0"));
//		plywood.radAng -= DegreeToRad (90.0);
//	}
//
//	// 합판 설치 (RIGHT)
//	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, axisPoint.x + (placingZone.outerRight - placingZone.outerLeft), axisPoint.y - ((placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) + placingZone.upMove), placingZone.level, placingZone.ang);
//
//	// 위치 값을 비회전값으로 변환
//	rotatedPoint.x = plywood.posX;
//	rotatedPoint.y = plywood.posY;
//	rotatedPoint.z = plywood.posZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (plywood.radAng));
//	plywood.posX = unrotatedPoint.x;
//	plywood.posY = unrotatedPoint.y;
//	plywood.posZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayHeight;
//	while (remainLength > EPS) {
//		if (remainLength > (2.440 + EPS)) {
//			currentLength = 2.440;
//			remainLength -= 2.440;
//		} else {
//			currentLength = remainLength;
//			remainLength = 0;
//		}
//
//		moveIn3D ('y', plywood.radAng, -currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
//		plywood.radAng += DegreeToRad (90.0);
//		elemList.Push (plywood.placeObject (7,
//			"p_stan", APIParT_CString, "비규격",
//			"w_dir", APIParT_CString, "바닥깔기",
//			"p_thk", APIParT_CString, "11.5T",
//			"p_wid", APIParT_Length, format_string ("%f", (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.leftMove),
//			"p_leng", APIParT_Length, format_string ("%f", currentLength),
//			"p_ang", APIParT_Angle, format_string ("%f", 0.0),
//			"sogak", APIParT_Boolean, "0.0"));
//		plywood.radAng-= DegreeToRad (90.0);
//	}
//
//	// 테이블폼 둘레 목재 설치 (TOP)
//	insCell.objType = WOOD;
//	insCell.ang = placingZone.ang;
//	insCell.leftBottomX = axisPoint.x + ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove) - 0.0545;
//	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + 0.0545 - placingZone.upMove;
//	insCell.leftBottomZ = placingZone.level - 0.0115;	// 합판 두께만큼 내려옴
//	insCell.libPart.wood.w_ang = 0.0;
//	insCell.libPart.wood.w_h = 0.050;	// 너비
//	//insCell.libPart.wood.w_leng;
//	insCell.libPart.wood.w_w = 0.040;	// 두께
//	
//	// 위치 값을 비회전값으로 변환
//	rotatedPoint.x = insCell.leftBottomX;
//	rotatedPoint.y = insCell.leftBottomY;
//	rotatedPoint.z = insCell.leftBottomZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
//	insCell.leftBottomX = unrotatedPoint.x;
//	insCell.leftBottomY = unrotatedPoint.y;
//	insCell.leftBottomZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayWidth + 0.0545 * 2;
//	while (remainLength > EPS) {
//		if (remainLength > (3.600 + EPS)) {
//			insCell.libPart.wood.w_leng = 3.600;
//			remainLength -= 3.600;
//		} else {
//			insCell.libPart.wood.w_leng = remainLength;
//			remainLength = 0;
//		}
//
//		elemList.Push (placingZone.placeLibPart (insCell));
//		moveIn3D ('x', insCell.ang, insCell.libPart.wood.w_leng, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
//	}
//
//	// 테이블폼 둘레 목재 설치 (BOTTOM)
//	insCell.objType = WOOD;
//	insCell.ang = placingZone.ang;
//	insCell.leftBottomX = axisPoint.x + ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove) - 0.0545;
//	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) + ((placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.upMove) - 0.005;
//	insCell.leftBottomZ = placingZone.level - 0.0115;	// 합판 두께만큼 내려옴
//	insCell.libPart.wood.w_ang = 0.0;
//	insCell.libPart.wood.w_h = 0.050;	// 너비
//	//insCell.libPart.wood.w_leng;
//	insCell.libPart.wood.w_w = 0.040;	// 두께
//	
//	// 위치 값을 비회전값으로 변환
//	rotatedPoint.x = insCell.leftBottomX;
//	rotatedPoint.y = insCell.leftBottomY;
//	rotatedPoint.z = insCell.leftBottomZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
//	insCell.leftBottomX = unrotatedPoint.x;
//	insCell.leftBottomY = unrotatedPoint.y;
//	insCell.leftBottomZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayWidth + 0.0545 * 2;
//	while (remainLength > EPS) {
//		if (remainLength > (3.600 + EPS)) {
//			insCell.libPart.wood.w_leng = 3.600;
//			remainLength -= 3.600;
//		} else {
//			insCell.libPart.wood.w_leng = remainLength;
//			remainLength = 0;
//		}
//
//		elemList.Push (placingZone.placeLibPart (insCell));
//		moveIn3D ('x', insCell.ang, insCell.libPart.wood.w_leng, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
//	}
//
//	// 테이블폼 둘레 목재 설치 (LEFT)
//	insCell.objType = WOOD;
//	insCell.ang = placingZone.ang;
//	insCell.leftBottomX = axisPoint.x + ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove) - 0.0545;
//	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.upMove + 0.005;
//	insCell.leftBottomZ = placingZone.level - 0.0115;	// 합판 두께만큼 내려옴
//	insCell.libPart.wood.w_ang = 0.0;
//	insCell.libPart.wood.w_h = 0.050;	// 너비
//	//insCell.libPart.wood.w_leng;
//	insCell.libPart.wood.w_w = 0.040;	// 두께
//	
//	// 위치 값을 비회전값으로 변환
//	rotatedPoint.x = insCell.leftBottomX;
//	rotatedPoint.y = insCell.leftBottomY;
//	rotatedPoint.z = insCell.leftBottomZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
//	insCell.leftBottomX = unrotatedPoint.x;
//	insCell.leftBottomY = unrotatedPoint.y;
//	insCell.leftBottomZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayHeight + 0.005 * 2;
//	while (remainLength > EPS) {
//		if (remainLength > (3.600 + EPS)) {
//			insCell.libPart.wood.w_leng = 3.600;
//			remainLength -= 3.600;
//		} else {
//			insCell.libPart.wood.w_leng = remainLength;
//			remainLength = 0;
//		}
//
//		moveIn3D ('y', insCell.ang, -insCell.libPart.wood.w_leng, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
//		insCell.ang += DegreeToRad (90.0);
//		elemList.Push (placingZone.placeLibPart (insCell));
//		insCell.ang -= DegreeToRad (90.0);
//	}
//
//	// 테이블폼 둘레 목재 설치 (RIGHT)
//	insCell.objType = WOOD;
//	insCell.ang = placingZone.ang;
//	insCell.leftBottomX = axisPoint.x + (placingZone.outerRight - placingZone.outerLeft) - ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.leftMove) + 0.005;
//	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.upMove + 0.005;
//	insCell.leftBottomZ = placingZone.level - 0.0115;	// 합판 두께만큼 내려옴
//	insCell.libPart.wood.w_ang = 0.0;
//	insCell.libPart.wood.w_h = 0.050;	// 너비
//	//insCell.libPart.wood.w_leng;
//	insCell.libPart.wood.w_w = 0.040;	// 두께
//	
//	// 위치 값을 비회전값으로 변환
//	rotatedPoint.x = insCell.leftBottomX;
//	rotatedPoint.y = insCell.leftBottomY;
//	rotatedPoint.z = insCell.leftBottomZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
//	insCell.leftBottomX = unrotatedPoint.x;
//	insCell.leftBottomY = unrotatedPoint.y;
//	insCell.leftBottomZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayHeight + 0.005 * 2;
//	while (remainLength > EPS) {
//		if (remainLength > (3.600 + EPS)) {
//			insCell.libPart.wood.w_leng = 3.600;
//			remainLength -= 3.600;
//		} else {
//			insCell.libPart.wood.w_leng = remainLength;
//			remainLength = 0;
//		}
//
//		moveIn3D ('y', insCell.ang, -insCell.libPart.wood.w_leng, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
//		insCell.ang += DegreeToRad (90.0);
//		elemList.Push (placingZone.placeLibPart (insCell));
//		insCell.ang -= DegreeToRad (90.0);
//	}

	return err;
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK slabBottomTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		xx;
	short		result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "슬래브 하부에 배치");

			//////////////////////////////////////////////////////////// 아이템 배치
			DGSetItemText (dialogID, DG_OK, "확 인");
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			DGSetItemText (dialogID, LABEL_SELECT_TYPE, "타입 선택");

			DGSetItemText (dialogID, PUSHRADIO_EUROFORM, "유로폼");
			DGSetItemText (dialogID, PUSHRADIO_TABLEFORM, "콘판넬");
			DGSetItemText (dialogID, PUSHRADIO_PLYWOOD, "합판");

			DGSetItemText (dialogID, LABEL_CELL_SETTINGS, "셀 설정");
			DGSetItemText (dialogID, LABEL_TABLEFORM_WIDTH, "너비");
			DGSetItemText (dialogID, LABEL_TABLEFORM_HEIGHT, "높이");
			DGSetItemText (dialogID, LABEL_TABLEFORM_ORIENTATION, "설치방향");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "슬래브와의 간격");

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "콘판넬");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "각재");
			DGSetItemText (dialogID, LABEL_LAYER_CPROFILE, "C형강");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트");
			DGSetItemText (dialogID, LABEL_LAYER_FITTINGS, "결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_GT24_GIRDER, "GT24 거더");
			DGSetItemText (dialogID, LABEL_LAYER_PERI_SUPPORT, "PERI동바리");
			DGSetItemText (dialogID, LABEL_LAYER_STEEL_SUPPORT, "강관 동바리");

			DGSetItemText (dialogID, BUTTON_AUTOSET, "레이어 자동 설정");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_SLABTABLEFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_CPROFILE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CPROFILE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_FITTINGS;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS, 1);

			ucb.itemID	 = USERCONTROL_LAYER_GT24_GIRDER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_GT24_GIRDER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PERI_SUPPORT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PERI_SUPPORT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_STEEL_SUPPORT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT, 1);

			// 방향 추가
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION, DG_POPUP_BOTTOM, "세로방향");	// 초기값
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION, DG_POPUP_BOTTOM, "가로방향");

			// 1번째 타입 선택
			DGSetItemValLong (dialogID, PUSHRADIO_EUROFORM, TRUE);		// 초기값: 유로폼

			// 너비 추가
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "200");

			// 높이 추가
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "600");

			// 레이어 활성화/비활성화
			DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
			DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);
			DGDisableItem (dialogID, LABEL_LAYER_GT24_GIRDER);
			DGDisableItem (dialogID, LABEL_LAYER_PERI_SUPPORT);
			DGDisableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);

			DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			DGDisableItem (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
			DGDisableItem (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
			DGDisableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);

			break;

		case DG_MSG_CHANGE:
			// 레이어 같이 바뀜
			if ((item >= USERCONTROL_LAYER_EUROFORM) && (item <= USERCONTROL_LAYER_STEEL_SUPPORT)) {
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					long selectedLayer;

					selectedLayer = DGGetItemValLong (dialogID, item);

					for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_STEEL_SUPPORT ; ++xx)
						DGSetItemValLong (dialogID, xx, selectedLayer);
				}
			}

			// 너비, 높이 팝업컨트롤 항목
			if ((item == PUSHRADIO_EUROFORM) || (item == PUSHRADIO_TABLEFORM) || (item == PUSHRADIO_PLYWOOD)) {
				// 팝업 내용을 모두 지우고,
				while (DGPopUpGetItemCount (dialogID, POPUP_TABLEFORM_WIDTH) > 0)
					DGPopUpDeleteItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
				while (DGPopUpGetItemCount (dialogID, POPUP_TABLEFORM_HEIGHT) > 0)
					DGPopUpDeleteItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);

				// 유로폼(1) 선택시
				if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE) {
					DGSetItemText (dialogID, LABEL_CELL_SETTINGS, "셀 설정");

					// 너비 추가
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "600");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "500");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "450");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "400");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "300");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "200");

					// 높이 추가
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "1200");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "900");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "600");
				}
				// 콘판넬(2) 선택시
				if (DGGetItemValLong (dialogID, PUSHRADIO_TABLEFORM) == TRUE) {
					DGSetItemText (dialogID, LABEL_CELL_SETTINGS, "셀 설정 (1818 x 3032, 1818 x 2426,\n1212 x 1820 만 유효함)");

					// 너비 추가
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "1818");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "1212");

					// 높이 추가
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "3032");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "2426");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "1820");
				}
				// 합판(3) 선택시
				if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE) {
					DGSetItemText (dialogID, LABEL_CELL_SETTINGS, "셀 설정");

					// 너비 추가
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "1220");

					// 높이 추가
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "2440");
				}
			}

			// 레이어 활성화/비활성화
			for (xx = LABEL_LAYER_EUROFORM ; xx <= LABEL_LAYER_STEEL_SUPPORT ; ++xx)
				DGEnableItem (dialogID, xx);
			for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_STEEL_SUPPORT ; ++xx)
				DGEnableItem (dialogID, xx);

			if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE) {
				DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);
				DGDisableItem (dialogID, LABEL_LAYER_GT24_GIRDER);
				DGDisableItem (dialogID, LABEL_LAYER_PERI_SUPPORT);
				DGDisableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);

				DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
				DGDisableItem (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);
			}
			if (DGGetItemValLong (dialogID, PUSHRADIO_TABLEFORM) == TRUE) {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, LABEL_LAYER_GT24_GIRDER);
				DGDisableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);

				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);
			}
			// 합판(3) 선택시
			if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE) {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, LABEL_LAYER_CPROFILE);
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);

				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_CPROFILE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			}

			// 콘판넬일 때 유효한 너비, 높이가 아니면 다음 단계로 진행할 수 없음
			if (DGGetItemValLong (dialogID, PUSHRADIO_TABLEFORM) == TRUE) {
				// 유효한 너비, 높이 조합은 다음과 같음: 1818 x 3032, 1818 x 2426, 1212 x 1820
				int width = atoi (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_WIDTH))).ToCStr ().Get ());
				int height = atoi (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_HEIGHT))).ToCStr ().Get ());

				if ( ((width == 1818) && (height == 3032)) || ((width == 1818) && (height == 2426)) || ((width == 1212) && (height == 1820)) )
					DGEnableItem (dialogID, DG_OK);
				else
					DGDisableItem (dialogID, DG_OK);
			} else {
				DGEnableItem (dialogID, DG_OK);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 셀 너비, 높이, 방향값 저장
					placingZone.initHorLen = atof (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_WIDTH))).ToCStr ().Get ()) / 1000.0;
					placingZone.initVerLen = atof (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_HEIGHT))).ToCStr ().Get ()) / 1000.0;
					if (my_strcmp (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_ORIENTATION))).ToCStr ().Get (), "세로방향") == 0)
						placingZone.bVertical = true;
					else
						placingZone.bVertical = false;

					// 타입 선택
					if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE)
						placingZone.iTableformType = EUROFORM;
					else if (DGGetItemValLong (dialogID, PUSHRADIO_TABLEFORM) == TRUE)
						placingZone.iTableformType = TABLEFORM;
					else if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE)
						placingZone.iTableformType = PLYWOOD;
					else
						placingZone.iTableformType = NONE;

					// 슬래브와의 간격값 저장
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					layerInd_CProfile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CPROFILE);
					layerInd_Pinbolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_Fittings		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS);
					layerInd_GT24Girder		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
					layerInd_PERI_Support	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
					layerInd_Steel_Support	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformSlab, "UFOM", NULL);
					layerInd_SlabTableform	= makeTemporaryLayer (structuralObject_forTableformSlab, "CONP", NULL);
					layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformSlab, "PLYW", NULL);
					layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformSlab, "TIMB", NULL);
					layerInd_CProfile		= makeTemporaryLayer (structuralObject_forTableformSlab, "CPRO", NULL);
					layerInd_Pinbolt		= makeTemporaryLayer (structuralObject_forTableformSlab, "PINB", NULL);
					layerInd_Fittings		= makeTemporaryLayer (structuralObject_forTableformSlab, "CLAM", NULL);
					layerInd_GT24Girder		= makeTemporaryLayer (structuralObject_forTableformSlab, "GIDR", NULL);
					layerInd_PERI_Support	= makeTemporaryLayer (structuralObject_forTableformSlab, "MULT", NULL);
					layerInd_Steel_Support	= makeTemporaryLayer (structuralObject_forTableformSlab, "SUPT", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, layerInd_SlabTableform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_CPROFILE, layerInd_CProfile);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_Pinbolt);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS, layerInd_Fittings);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_GT24_GIRDER, layerInd_GT24Girder);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PERI_SUPPORT, layerInd_PERI_Support);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT, layerInd_Steel_Support);

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
	short	xx, yy;
	short	btnSizeX = 50, btnSizeY = 50;
	short	dialogSizeX, dialogSizeY;
	short	btnInitPosX;
	short	btnPosX, btnPosY;
	short	itmIdx;
	std::string		txtButton = "";

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "슬래브 하부에 배치 - 배치 수정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 배치 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 110, 130, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "배치 - 자투리 채우기");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼 1
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 150, 130, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "배치 - 자투리 제외");
			DGShowItem (dialogID, DG_CANCEL);

			// 이전 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 190, 130, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "이전");
			DGShowItem (dialogID, DG_PREV);

			// 행 추가 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 30, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_ROW, "행 추가");
			DGShowItem (dialogID, PUSHBUTTON_ADD_ROW);

			// 행 삭제 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 85, 30, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_ROW, "행 삭제");
			DGShowItem (dialogID, PUSHBUTTON_DEL_ROW);

			// 열 추가 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 70, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_COL, "열 추가");
			DGShowItem (dialogID, PUSHBUTTON_ADD_COL);
		
			// 열 삭제 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 85, 70, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_COL, "열 삭제");
			DGShowItem (dialogID, PUSHBUTTON_DEL_COL);

			// 메인 창 크기를 변경
			dialogSizeX = 450 + (btnSizeX * (placingZone.nHorCells-1));
			dialogSizeY = 300 + (btnSizeY * (placingZone.nVerCells-1));
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			// 그리드 구조체에 따라서 버튼을 동적으로 배치함
			btnInitPosX = btnPosX = 220 + 25;
			btnPosY = (btnSizeY * placingZone.nVerCells) + 25;
			for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
				for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
					placingZone.CELL_BUTTON [xx][yy] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, placingZone.CELL_BUTTON [xx][yy], DG_IS_SMALL);

					// 타입에 따라 셀 종류가 달라짐
					if (placingZone.iTableformType == EUROFORM) {
						if (placingZone.bVertical == true) {
							txtButton = format_string ("유로폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else {
							txtButton = format_string ("유로폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
						}
					} else if (placingZone.iTableformType == TABLEFORM) {
						if (placingZone.bVertical == true) {
							txtButton = format_string ("콘판넬\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else {
							txtButton = format_string ("콘판넬\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
						}
					} else if (placingZone.iTableformType == PLYWOOD) {
						if (placingZone.bVertical == true) {
							txtButton = format_string ("합판\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else {
							txtButton = format_string ("합판\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
						}
					} else {
						txtButton = format_string ("없음");
					}
					DGSetItemText (dialogID, placingZone.CELL_BUTTON [xx][yy], txtButton.c_str ());		// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, placingZone.CELL_BUTTON [xx][yy]);
					btnPosX += btnSizeX;
				}
				btnPosX = btnInitPosX;
				btnPosY -= btnSizeY;
			}

			// 라벨: 여백(좌)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백(좌)");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 여백(우)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백(우)");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 여백(상)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, 30, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백(상)");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 여백(하)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY), 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "여백(하)");
			DGShowItem (dialogID, itmIdx);

			// Edit 컨트롤: 여백(좌)
			placingZone.EDITCONTROL_MARGIN_LEFT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.borderHorLen - placingZone.formArrayWidth - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);

			// Edit 컨트롤: 여백(우)
			placingZone.EDITCONTROL_MARGIN_RIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.borderHorLen - placingZone.formArrayWidth - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);

			// Edit 컨트롤: 여백(상)
			placingZone.EDITCONTROL_MARGIN_TOP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.borderVerLen - placingZone.formArrayHeight - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);

			// Edit 컨트롤: 여백(하)
			placingZone.EDITCONTROL_MARGIN_BOTTOM = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY) - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.borderVerLen - placingZone.formArrayHeight - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);

			break;

		case DG_MSG_CHANGE:
			// 여백(좌) 변경시, 여백(우) 변경됨
			if (item == placingZone.EDITCONTROL_MARGIN_LEFT) {
				placingZone.marginLeft = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				placingZone.marginRight = placingZone.borderHorLen - placingZone.formArrayWidth - placingZone.marginLeft;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);
			}

			// 여백(우) 변경시, 여백(좌) 변경됨
			if (item == placingZone.EDITCONTROL_MARGIN_RIGHT) {
				placingZone.marginRight = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				placingZone.marginLeft = placingZone.borderHorLen - placingZone.formArrayWidth - placingZone.marginRight;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);
			}

			// 여백(상) 변경시, 여백(하) 변경됨
			if (item == placingZone.EDITCONTROL_MARGIN_TOP) {
				placingZone.marginTop = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				placingZone.marginBottom = placingZone.borderVerLen - placingZone.formArrayHeight - placingZone.marginTop;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);
			}

			// 여백(하) 변경시, 여백(상) 변경됨
			if (item == placingZone.EDITCONTROL_MARGIN_BOTTOM) {
				placingZone.marginBottom = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
				placingZone.marginTop = placingZone.borderVerLen - placingZone.formArrayHeight - placingZone.marginBottom;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);
			}

			break;

		case DG_MSG_CLICK:
			if (item == DG_OK) {
				// 배치 - 자투리 채우기 버튼
				clickedExcludeRestButton = false;

				// 테이블폼 배열 전체 너비, 높이 저장
				placingZone.formArrayWidth = 0.0;
				placingZone.formArrayHeight = 0.0;
				if (placingZone.bVertical == true) {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].horLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].verLen;
				} else {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].verLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].horLen;
				}

				// 여백 길이 저장
				placingZone.marginLeft = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				placingZone.marginRight = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				placingZone.marginTop = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				placingZone.marginBottom = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);

				// 셀 위치 재조정
				placingZone.alignPlacingZone (&placingZone);

			} else if (item == DG_CANCEL) {
				// 배치 - 자투리 제외 버튼
				clickedExcludeRestButton = true;

				// 테이블폼 배열 전체 너비, 높이 저장
				placingZone.formArrayWidth = 0.0;
				placingZone.formArrayHeight = 0.0;
				if (placingZone.bVertical == true) {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].horLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].verLen;
				} else {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].verLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].horLen;
				}

				// 여백 길이 저장
				placingZone.marginLeft = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				placingZone.marginRight = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				placingZone.marginTop = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				placingZone.marginBottom = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);

				// 셀 위치 재조정
				placingZone.alignPlacingZone (&placingZone);

			} else if (item == DG_PREV) {
				// 이전 버튼
				clickedPrevButton = true;

			} else {
				if (item == PUSHBUTTON_ADD_ROW)
					++ placingZone.nVerCells;	// 행 추가 버튼

				if (item == PUSHBUTTON_DEL_ROW)
					-- placingZone.nVerCells;	// 행 삭제 버튼

				if (item == PUSHBUTTON_ADD_COL)
					++ placingZone.nHorCells;	// 열 추가 버튼

				if (item == PUSHBUTTON_DEL_COL)
					-- placingZone.nHorCells;	// 열 삭제 버튼

				// 그리드 버튼 클릭시
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
					for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
						if (item == placingZone.CELL_BUTTON [xx][yy]) {
							// 클릭한 인덱스 값을 저장함
							clickedRow = xx;
							clickedCol = yy;

							// 3번째 다이얼로그 열기 (셀 정보 변경)
							result = DGBlankModalDialog (250, 270, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomTableformPlacerHandler3, 0);
						}
					}
				}

				// 변경 가능성이 있는 DG 항목 모두 제거
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// 테이블폼 배열 전체 너비, 높이 지정
				placingZone.formArrayWidth = 0.0;
				placingZone.formArrayHeight = 0.0;
				if (placingZone.bVertical == true) {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].horLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].verLen;
				} else {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].verLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].horLen;
				}

				// 여백 길이 지정
				placingZone.marginLeft = placingZone.marginRight = (placingZone.borderHorLen - placingZone.formArrayWidth) / 2;
				placingZone.marginBottom = placingZone.marginTop = (placingZone.borderVerLen - placingZone.formArrayHeight) / 2;

				// 메인 창 크기를 변경
				dialogSizeX = 450 + (btnSizeX * (placingZone.nHorCells-1));
				dialogSizeY = 300 + (btnSizeY * (placingZone.nVerCells-1));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

				// 그리드 구조체에 따라서 버튼을 동적으로 배치함
				btnInitPosX = btnPosX = 220 + 25;
				btnPosY = (btnSizeY * placingZone.nVerCells) + 25;
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
					for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
						placingZone.CELL_BUTTON [xx][yy] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
						DGSetItemFont (dialogID, placingZone.CELL_BUTTON [xx][yy], DG_IS_SMALL);

						// 타입에 따라 셀 종류가 달라짐
						if (placingZone.iTableformType == EUROFORM) {
							if (placingZone.bVertical == true) {
								txtButton = format_string ("유로폼\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else {
								txtButton = format_string ("유로폼\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
							}
						} else if (placingZone.iTableformType == TABLEFORM) {
							if (placingZone.bVertical == true) {
								txtButton = format_string ("콘판넬\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else {
								txtButton = format_string ("콘판넬\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
							}
						} else if (placingZone.iTableformType == PLYWOOD) {
							if (placingZone.bVertical == true) {
								txtButton = format_string ("합판\n(세로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else {
								txtButton = format_string ("합판\n(가로)\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
							}
						} else {
							txtButton = format_string ("없음");
						}
						DGSetItemText (dialogID, placingZone.CELL_BUTTON [xx][yy], txtButton.c_str ());		// 그리드 버튼 텍스트 지정
						DGShowItem (dialogID, placingZone.CELL_BUTTON [xx][yy]);
						btnPosX += btnSizeX;
					}
					btnPosX = btnInitPosX;
					btnPosY -= btnSizeY;
				}

				// 라벨: 여백(좌)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백(좌)");
				DGShowItem (dialogID, itmIdx);

				// 라벨: 여백(우)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백(우)");
				DGShowItem (dialogID, itmIdx);

				// 라벨: 여백(상)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, 30, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백(상)");
				DGShowItem (dialogID, itmIdx);

				// 라벨: 여백(하)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY), 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "여백(하)");
				DGShowItem (dialogID, itmIdx);

				// Edit 컨트롤: 여백(좌)
				placingZone.EDITCONTROL_MARGIN_LEFT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.borderHorLen - placingZone.formArrayWidth - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);

				// Edit 컨트롤: 여백(우)
				placingZone.EDITCONTROL_MARGIN_RIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.borderHorLen - placingZone.formArrayWidth - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);

				// Edit 컨트롤: 여백(상)
				placingZone.EDITCONTROL_MARGIN_TOP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.borderVerLen - placingZone.formArrayHeight - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);

				// Edit 컨트롤: 여백(하)
				placingZone.EDITCONTROL_MARGIN_BOTTOM = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY) - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.borderVerLen - placingZone.formArrayHeight - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);

				item = 0;
			}

			break;

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
	short	xx;
	short	result;
	short	itmIdx;
	int		widthInt, heightInt;
	double	widthDouble, heightDouble;

	switch (message) {
		case DG_MSG_INIT:
			
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "Cell 값 설정");

			////////////////////////////////////////////////////////////  아이템 배치 (기본 버튼)
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
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, "객체 타입");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// 팝업컨트롤: 객체 타입 (유로폼, 슬래브 테이블폼 (콘판넬), 합판)
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 20-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "유로폼");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "콘판넬");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "합판");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);
			if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM);
			else if (placingZone.cells [clickedRow][clickedCol].objType == TABLEFORM)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, TABLEFORM);
			else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PLYWOOD);
			DGDisableItem (dialogID, POPUP_OBJ_TYPE);

			// 라벨: 너비
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 60, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, "너비");
			DGShowItem (dialogID, LABEL_WIDTH);

			// 팝업컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 60-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, POPUP_WIDTH);

			// 라벨: 높이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 100, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, "높이");
			DGShowItem (dialogID, LABEL_HEIGHT);

			// 팝업컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 100-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, POPUP_HEIGHT);

			// 라벨: 설치방향
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION, "설치방향");
			DGShowItem (dialogID, LABEL_ORIENTATION);

			// 팝업컨트롤: 설치방향
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 140-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM, "세로방향");
			DGPopUpInsertItem (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM, "가로방향");
			DGShowItem (dialogID, POPUP_ORIENTATION);

			// 팝업컨트롤(너비, 높이)의 내용은 객체 타입에 따라 달라짐
			if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM) {
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "600");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "500");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "450");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "400");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "300");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "200");

				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "1200");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "900");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "600");
			} else if (placingZone.cells [clickedRow][clickedCol].objType == TABLEFORM) {
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "1818");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "1212");

				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "3032");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "2426");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "1820");
			} else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD) {
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "1220");

				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "2440");
			}

			// 라벨: 경고 문구
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 170, 220, 40);
			DGSetItemFont (dialogID, LABEL_CAUTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_CAUTION, "1818 x 3032, 1818 x 2426,\n1212 x 1820 만 유효함");

			// 초기 입력 필드 표시
			if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM) {
				if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.600) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.500) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.450) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 3);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.400) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 4);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.300) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 5);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.200) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 6);

				if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 1.200) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
				else if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 0.900) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);
				else if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 0.600) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 3);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == TABLEFORM) {
				if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 1.818) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 1.212) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);

				if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 3.032) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
				else if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 2.426) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);
				else if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 1.820) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 3);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD) {
				if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 1.220) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);

				if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 2.440) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
			}

			if (placingZone.bVertical == true)
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, 1);
			else
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, 2);

			break;

		case DG_MSG_CHANGE:
			// 콘판넬일 때 유효한 너비, 높이가 아니면 다음 단계로 진행할 수 없음
			if (placingZone.cells [clickedRow][clickedCol].objType == TABLEFORM) {
				// 유효한 너비, 높이 조합은 다음과 같음: 1818 x 3032, 1818 x 2426, 1212 x 1820
				widthInt = atoi (DGPopUpGetItemText (dialogID, POPUP_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_WIDTH))).ToCStr ().Get ());
				heightInt = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT))).ToCStr ().Get ());

				if ( ((widthInt == 1818) && (heightInt == 3032)) || ((widthInt == 1818) && (heightInt == 2426)) || ((widthInt == 1212) && (heightInt == 1820)) ) {
					DGEnableItem (dialogID, DG_OK);
					DGHideItem (dialogID, LABEL_CAUTION);
				} else {
					DGDisableItem (dialogID, DG_OK);
					DGShowItem (dialogID, LABEL_CAUTION);
				}
			} else {
				DGEnableItem (dialogID, DG_OK);
				DGHideItem (dialogID, LABEL_CAUTION);
			}

			 break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 방향 저장
					if (DGPopUpGetSelected (dialogID, POPUP_ORIENTATION) == 1)
						placingZone.bVertical = true;
					else
						placingZone.bVertical = false;

					widthDouble = atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_WIDTH))).ToCStr ().Get ()) / 1000.0;
					heightDouble = atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT))).ToCStr ().Get ()) / 1000.0;

					if (placingZone.bVertical == true) {
						for (xx = 0 ; xx < MAX_IND ; ++xx) {
							placingZone.cells [xx][clickedCol].horLen = widthDouble;	// 너비 저장 - 동일한 열의 모든 너비도 변경해야 함
							placingZone.cells [clickedRow][xx].verLen = heightDouble;	// 높이 저장 - 동일한 행의 모든 높이도 변경해야 함
						}
					} else {
						for (xx = 0 ; xx < MAX_IND ; ++xx) {
							placingZone.cells [clickedRow][xx].horLen = widthDouble;	// 너비 저장 - 동일한 행의 모든 너비도 변경해야 함
							placingZone.cells [xx][clickedCol].verLen = heightDouble;	// 높이 저장 - 동일한 열의 모든 높이도 변경해야 함
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
			break;
	}

	result = item;

	return	result;
}
