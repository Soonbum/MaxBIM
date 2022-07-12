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
static insulElemForSlabTableform	insulElem;		// 단열재 정보
API_Guid	structuralObject_forTableformSlab;		// 구조 객체의 GUID

static short		clickedBtnItemIdx;			// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
static bool			clickedExcludeRestButton;	// 자투리 제외 버튼을 눌렀습니까?
static bool			clickedPrevButton;			// 이전 버튼을 눌렀습니까?
static short		clickedRow, clickedCol;		// 클릭한 행, 열 인덱스
static short		layerInd_Euroform;			// 레이어 번호: 유로폼
static short		layerInd_ConPanel;			// 레이어 번호: 콘판넬
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
static GS::Array<API_Guid>	elemList_Insulation;	// 그룹화 (단열재)
short	MAX_IND = 50;


// 슬래브 하부에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
	short		xx, yy;
	double		dx, dy, ang;

	// Selection Manager 관련 변수
	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		slabs;
	long					nMorphs = 0;
	long					nSlabs = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 3D 구성요소 가져오기
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
	double			workLevel_slab;


	// 선택한 요소 가져오기 (슬래브 1개, 모프 1개 선택해야 함)
	err = getGuidsOfSelection (&slabs, API_SlabID, &nSlabs);
	err = getGuidsOfSelection (&morphs, API_MorphID, &nMorphs);
	if (err == APIERR_NOPLAN) {
		DGAlert (DG_ERROR, L"오류", L"열린 프로젝트 창이 없습니다.", "", L"확인", "", "");
	}
	if (err == APIERR_NOSEL) {
		DGAlert (DG_ERROR, L"오류", L"아무 것도 선택하지 않았습니다.\n필수 선택: 슬래브 (1개), 슬래브 하부를 덮는 모프 (1개)", "", L"확인", "", "");
	}

	// 슬래브가 1개인가?
	if (nSlabs != 1) {
		DGAlert (DG_ERROR, L"오류", L"슬래브를 1개 선택해야 합니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorphs != 1) {
		DGAlert (DG_ERROR, L"오류", L"슬래브 하부를 덮는 모프를 1개 선택하셔야 합니다.", "", L"확인", "", "");
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
		DGAlert (DG_ERROR, L"오류", L"모프가 누워 있지 않습니다.", "", L"확인", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// 모프의 정보 저장
	infoMorph.guid		= elem.header.guid;
	infoMorph.floorInd	= elem.header.floorInd;
	infoMorph.level		= info3D.bounds.zMin;

	// 모프의 3D 모델을 가져오지 못하면 종료
	if (err != NoError) {
		DGAlert (DG_ERROR, L"오류", L"모프의 3D 모델을 가져오지 못했습니다.", "", L"확인", "", "");
		return err;
	}

	// 모프의 꼭지점들을 수집함
	nNodes = getVerticesOfMorph (infoMorph.guid, &coords);
	
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
		DGAlert (DG_ERROR, L"오류", L"폴리곤에 속하지 않은 점을 클릭했습니다.", "", L"확인", "", "");
		return err;
	}

	// 영역 모프 제거
	GS::Array<API_Element>	elems;
	elems.Push (elem);
	deleteElements (elems);

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

	// 작업 층 높이 반영
	workLevel_slab = getWorkLevel (infoSlab.floorInd);

	// 영역 정보의 고도 정보 수정
	placingZone.level = infoSlab.level + infoSlab.offsetFromTop - infoSlab.thickness - placingZone.gap;
	placingZone.leftBottom.z = placingZone.level;

	if (result != DG_OK)
		return err;

	// [DIALOG] 2번째 다이얼로그에서 테이블폼 배치를 수정하거나 보강 목재를 삽입합니다.
	clickedExcludeRestButton = false;	// 자투리 채우기 제외 여부 클릭 여부
	clickedPrevButton = false;			// 이전 버튼 클릭 여부
	result = DGBlankModalDialog (450, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomTableformPlacerHandler2, 0);

	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST;

	// 셀 배열 공간 채우기
	err = placingZone.fillCellAreas ();

	// 여백 공간 채우기 (합판)
	if (clickedExcludeRestButton == false)
		err = placingZone.fillMarginAreas ();

	// 단열재 배치
	if (placingZone.gap > EPS) {
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), slabBottomTableformPlacerHandler4_Insulation, 0);
		
		if (result == DG_OK)
			err = placingZone.placeInsulations ();
	}
	
	// 장선, 멍에제, 동바리 배치 !!!
	placingZone.place_Joist_Yoke_SupportingPost ();

	// 결과물 전체 그룹화
	groupElements (elemList);
	groupElements (elemList_Insulation);
	// !!! 장선, 멍에제, 동바리 그룹화

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
			placingZone->cells [xx][yy].leftBottomX = 0.0;
			placingZone->cells [xx][yy].leftBottomY = 0.0;
			placingZone->cells [xx][yy].leftBottomZ = placingZone->level;
			placingZone->cells [xx][yy].ang = placingZone->ang;
			// 셀 방향에 따라 달라짐
			if (placingZone->iCellDirection == HORIZONTAL) {
				if (placingZone->iTableformType == CONPANEL) {
					// 콘판넬 크기: 2x6 [606x1820]
					placingZone->cells [xx][yy].horLen = 1.820;
					placingZone->cells [xx][yy].verLen = 0.606;
				} else if (placingZone->iTableformType == PLYWOOD) {
					// 합판 크기: 4x8 [1220x2440]
					placingZone->cells [xx][yy].horLen = 2.440;
					placingZone->cells [xx][yy].verLen = 1.220;
				} else if (placingZone->iTableformType == EUROFORM) {
					// 유로폼 크기: 600x1200
					placingZone->cells [xx][yy].horLen = 1.200;
					placingZone->cells [xx][yy].verLen = 0.600;
				}
			} else {
				if (placingZone->iTableformType == CONPANEL) {
					// 콘판넬 크기: 2x6 [606x1820]
					placingZone->cells [xx][yy].horLen = 0.606;
					placingZone->cells [xx][yy].verLen = 1.820;
				} else if (placingZone->iTableformType == PLYWOOD) {
					// 합판 크기: 4x8 [1220x2440]
					placingZone->cells [xx][yy].horLen = 1.220;
					placingZone->cells [xx][yy].verLen = 2.440;
				} else if (placingZone->iTableformType == EUROFORM) {
					// 유로폼 크기: 600x1200
					placingZone->cells [xx][yy].horLen = 0.600;
					placingZone->cells [xx][yy].verLen = 1.200;
				}
			}
		}
	}

	// 셀 초기 너비, 높이 설정
	if (placingZone->iCellDirection == HORIZONTAL) {
		if (placingZone->iTableformType == CONPANEL) {
			// 콘판넬 크기: 2x6 [606x1820]
			placingZone->initCellHorLen = 1.820;
			placingZone->initCellVerLen = 0.606;
		} else if (placingZone->iTableformType == PLYWOOD) {
			// 합판 크기: 4x8 [1220x2440]
			placingZone->initCellHorLen = 2.440;
			placingZone->initCellVerLen = 1.220;
		} else if (placingZone->iTableformType == EUROFORM) {
			// 유로폼 크기: 600x1200
			placingZone->initCellHorLen = 1.200;
			placingZone->initCellVerLen = 0.600;
		}
	} else {
		if (placingZone->iTableformType == CONPANEL) {
			// 콘판넬 크기: 2x6 [606x1820]
			placingZone->initCellHorLen = 0.606;
			placingZone->initCellVerLen = 1.820;
		} else if (placingZone->iTableformType == PLYWOOD) {
			// 합판 크기: 4x8 [1220x2440]
			placingZone->initCellHorLen = 1.220;
			placingZone->initCellVerLen = 2.440;
		} else if (placingZone->iTableformType == EUROFORM) {
			// 유로폼 크기: 600x1200
			placingZone->initCellHorLen = 0.600;
			placingZone->initCellVerLen = 1.200;
		}
	}

	// 초기 셀 개수 지정
	placingZone->nHorCells = (short)floor (placingZone->borderHorLen / placingZone->initCellHorLen);
	placingZone->nVerCells = (short)floor (placingZone->borderVerLen / placingZone->initCellVerLen);

	// 초기 셀 배열 전체 너비, 높이 지정
	placingZone->cellArrayWidth = 0.0;
	placingZone->cellArrayHeight = 0.0;
	for (xx = 0 ; xx < placingZone->nHorCells ; ++xx)	placingZone->cellArrayWidth += placingZone->cells [0][xx].horLen;
	for (xx = 0 ; xx < placingZone->nVerCells ; ++xx)	placingZone->cellArrayHeight += placingZone->cells [xx][0].verLen;

	// 초기 여백 길이 지정
	placingZone->marginLeft = placingZone->marginRight = (placingZone->borderHorLen - placingZone->cellArrayWidth) / 2;
	placingZone->marginBottom = placingZone->marginTop = (placingZone->borderVerLen - placingZone->cellArrayHeight) / 2;
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
}

// 셀 배열 공간 채우기
GSErrCode	SlabTableformPlacingZone::fillCellAreas (void)
{
	GSErrCode	err = NoError;
	short	xx, yy;
	EasyObjectPlacement	conpanel, plywood, euroform;
	bool	bStandardForm;

	for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
		for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
			if (placingZone.cells [xx][yy].objType == CONPANEL) {
				conpanel.init (L("합판v1.0.gsm"), layerInd_ConPanel, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);

				if (placingZone.iCellDirection == VERTICAL) {
					conpanel.radAng -= DegreeToRad (90.0);
					elemList.Push (conpanel.placeObject (9,
						"g_comp", APIParT_CString, "콘판넬",
						"p_stan", APIParT_CString, "2x6 [606x1820]",
						"w_dir", APIParT_CString, "바닥깔기",
						"p_thk", APIParT_CString, "11.5T",
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"A", APIParT_Length, "0.606",
						"B", APIParT_Length, "1.820"));
				} else {
					moveIn3D ('y', conpanel.radAng, -placingZone.cells [xx][yy].verLen, &conpanel.posX, &conpanel.posY, &conpanel.posZ);
					elemList.Push (conpanel.placeObject (9,
						"g_comp", APIParT_CString, "콘판넬",
						"p_stan", APIParT_CString, "2x6 [606x1820]",
						"w_dir", APIParT_CString, "바닥깔기",
						"p_thk", APIParT_CString, "11.5T",
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"A", APIParT_Length, "0.606",
						"B", APIParT_Length, "1.820"));
				}
			} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
				plywood.init (L("합판v1.0.gsm"), layerInd_ConPanel, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);

				if (placingZone.iCellDirection == VERTICAL) {
					plywood.radAng -= DegreeToRad (90.0);
					elemList.Push (plywood.placeObject (12,
						"g_comp", APIParT_CString, "합판",
						"p_stan", APIParT_CString, "비규격",
						"w_dir", APIParT_CString, "바닥깔기",
						"p_thk", APIParT_CString, "11.5T",
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"sogak", APIParT_Boolean, "0.0",
						"p_wid", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
						"p_leng", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
						"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
						"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen)));
				} else {
					moveIn3D ('y', plywood.radAng, -placingZone.cells [xx][yy].verLen, &plywood.posX, &plywood.posY, &plywood.posZ);
					elemList.Push (plywood.placeObject (12,
						"g_comp", APIParT_CString, "합판",
						"p_stan", APIParT_CString, "비규격",
						"w_dir", APIParT_CString, "바닥깔기",
						"p_thk", APIParT_CString, "11.5T",
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"sogak", APIParT_Boolean, "0.0",
						"p_wid", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
						"p_leng", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
						"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
						"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen)));
				}
			} else if (placingZone.cells [xx][yy].objType == EUROFORM) {
				euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);

				if (placingZone.iCellDirection == VERTICAL) {
					bStandardForm = false;
					if ( ((abs (placingZone.cells [xx][yy].horLen - 0.600) < EPS) || (abs (placingZone.cells [xx][yy].horLen - 0.500) < EPS) ||
						  (abs (placingZone.cells [xx][yy].horLen - 0.450) < EPS) || (abs (placingZone.cells [xx][yy].horLen - 0.400) < EPS) ||
						  (abs (placingZone.cells [xx][yy].horLen - 0.300) < EPS) || (abs (placingZone.cells [xx][yy].horLen - 0.200) < EPS)) &&
						 ((abs (placingZone.cells [xx][yy].verLen - 1.200) < EPS) || (abs (placingZone.cells [xx][yy].verLen - 0.900) < EPS) ||
						  (abs (placingZone.cells [xx][yy].verLen - 0.600) < EPS)) )
						  bStandardForm = true;

					if (bStandardForm == true) {
						elemList.Push (euroform.placeObject (8,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"u_ins", APIParT_CString, "벽세우기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
							"eu_wid", APIParT_CString, format_string ("%.0f", (double)placingZone.cells [xx][yy].horLen * 1000),
							"eu_hei", APIParT_CString, format_string ("%.0f", (double)placingZone.cells [xx][yy].verLen * 1000),
							"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
							"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen)));
					} else {
						elemList.Push (euroform.placeObject (8,
							"eu_stan_onoff", APIParT_Boolean, "0.0",
							"u_ins", APIParT_CString, "벽세우기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
							"eu_wid2", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
							"eu_hei2", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
							"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
							"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen)));
					}
				} else {
					bStandardForm = false;
					if ( ((abs (placingZone.cells [xx][yy].verLen - 0.600) < EPS) || (abs (placingZone.cells [xx][yy].verLen - 0.500) < EPS) ||
						  (abs (placingZone.cells [xx][yy].verLen - 0.450) < EPS) || (abs (placingZone.cells [xx][yy].verLen - 0.400) < EPS) ||
						  (abs (placingZone.cells [xx][yy].verLen - 0.300) < EPS) || (abs (placingZone.cells [xx][yy].verLen - 0.200) < EPS)) &&
						 ((abs (placingZone.cells [xx][yy].horLen - 1.200) < EPS) || (abs (placingZone.cells [xx][yy].horLen - 0.900) < EPS) ||
						  (abs (placingZone.cells [xx][yy].horLen - 0.600) < EPS)) )
						  bStandardForm = true;

					moveIn3D ('y', euroform.radAng, -placingZone.cells [xx][yy].verLen, &euroform.posX, &euroform.posY, &euroform.posZ);
					euroform.radAng += DegreeToRad (90.0);
					if (bStandardForm == true) {
						elemList.Push (euroform.placeObject (8,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"u_ins", APIParT_CString, "벽세우기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
							"eu_wid", APIParT_CString, format_string ("%.0f", (double)placingZone.cells [xx][yy].verLen * 1000),
							"eu_hei", APIParT_CString, format_string ("%.0f", (double)placingZone.cells [xx][yy].horLen * 1000),
							"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
							"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen)));
					} else {
						elemList.Push (euroform.placeObject (8,
							"eu_stan_onoff", APIParT_Boolean, "0.0",
							"u_ins", APIParT_CString, "벽세우기",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
							"eu_wid2", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
							"eu_hei2", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
							"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
							"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen)));
					}
				}
			}
		}
	}

	return err;
}

// 여백 공간 채우기 (합판)
GSErrCode	SlabTableformPlacingZone::fillMarginAreas (void)
{
	GSErrCode	err = NoError;
	double		remainLength;
	double		currentLength;
	EasyObjectPlacement	plywood;

	// 합판 설치 (TOP)
	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
	moveIn3D ('y', placingZone.ang, -placingZone.borderVerLen, &plywood.posX, &plywood.posY, &plywood.posZ);
	remainLength = placingZone.borderHorLen;

	while (remainLength > EPS) {
		if (remainLength > 2.440 + EPS) {
			currentLength = 2.440;
			remainLength -= 2.440;
		} else {
			currentLength = remainLength;
			remainLength = 0;
		}
		
		elemList.Push (plywood.placeObject (12,
			"g_comp", APIParT_CString, "합판",
			"p_stan", APIParT_CString, "비규격",
			"w_dir", APIParT_CString, "바닥깔기",
			"p_thk", APIParT_CString, "11.5T",
			"sogak", APIParT_Boolean, "0.0",
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_wid", APIParT_Length, format_string ("%f", placingZone.marginTop),
			"p_leng", APIParT_Length, format_string ("%f", currentLength),
			"A", APIParT_Length, format_string ("%f", placingZone.marginTop),
			"B", APIParT_Length, format_string ("%f", currentLength)));
		moveIn3D ('x', placingZone.ang, currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
	}

	// 합판 설치 (BOTTOM)
	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
	moveIn3D ('y', placingZone.ang, -placingZone.marginBottom, &plywood.posX, &plywood.posY, &plywood.posZ);
	remainLength = placingZone.borderHorLen;

	while (remainLength > EPS) {
		if (remainLength > 2.440 + EPS) {
			currentLength = 2.440;
			remainLength -= 2.440;
		} else {
			currentLength = remainLength;
			remainLength = 0;
		}

		elemList.Push (plywood.placeObject (12,
			"g_comp", APIParT_CString, "합판",
			"p_stan", APIParT_CString, "비규격",
			"w_dir", APIParT_CString, "바닥깔기",
			"p_thk", APIParT_CString, "11.5T",
			"sogak", APIParT_Boolean, "0.0",
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_wid", APIParT_Length, format_string ("%f", placingZone.marginBottom),
			"p_leng", APIParT_Length, format_string ("%f", currentLength),
			"A", APIParT_Length, format_string ("%f", placingZone.marginBottom),
			"B", APIParT_Length, format_string ("%f", currentLength)));
		moveIn3D ('x', placingZone.ang, currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
	}

	// 합판 설치 (LEFT)
	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
	moveIn3D ('y', placingZone.ang, -placingZone.marginBottom, &plywood.posX, &plywood.posY, &plywood.posZ);
	remainLength = placingZone.borderVerLen - placingZone.marginTop - placingZone.marginBottom;

	while (remainLength > EPS) {
		if (remainLength > 2.440 + EPS) {
			currentLength = 2.440;
			remainLength -= 2.440;
		} else {
			currentLength = remainLength;
			remainLength = 0;
		}

		plywood.radAng += DegreeToRad (-90.0);
		elemList.Push (plywood.placeObject (12,
			"g_comp", APIParT_CString, "합판",
			"p_stan", APIParT_CString, "비규격",
			"w_dir", APIParT_CString, "바닥깔기",
			"p_thk", APIParT_CString, "11.5T",
			"sogak", APIParT_Boolean, "0.0",
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_wid", APIParT_Length, format_string ("%f", placingZone.marginLeft),
			"p_leng", APIParT_Length, format_string ("%f", currentLength),
			"A", APIParT_Length, format_string ("%f", placingZone.marginLeft),
			"B", APIParT_Length, format_string ("%f", currentLength)));
		plywood.radAng -= DegreeToRad (-90.0);
		moveIn3D ('y', placingZone.ang, -currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
	}

	// 합판 설치 (RIGHT)
	plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
	moveIn3D ('y', placingZone.ang, -placingZone.marginBottom, &plywood.posX, &plywood.posY, &plywood.posZ);
	moveIn3D ('x', placingZone.ang, placingZone.borderHorLen - placingZone.marginRight, &plywood.posX, &plywood.posY, &plywood.posZ);
	remainLength = placingZone.borderVerLen - placingZone.marginTop - placingZone.marginBottom;

	while (remainLength > EPS) {
		if (remainLength > 2.440 + EPS) {
			currentLength = 2.440;
			remainLength -= 2.440;
		} else {
			currentLength = remainLength;
			remainLength = 0;
		}

		plywood.radAng += DegreeToRad (-90.0);
		elemList.Push (plywood.placeObject (12,
			"g_comp", APIParT_CString, "합판",
			"p_stan", APIParT_CString, "비규격",
			"w_dir", APIParT_CString, "바닥깔기",
			"p_thk", APIParT_CString, "11.5T",
			"sogak", APIParT_Boolean, "0.0",
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"p_wid", APIParT_Length, format_string ("%f", placingZone.marginRight),
			"p_leng", APIParT_Length, format_string ("%f", currentLength),
			"A", APIParT_Length, format_string ("%f", placingZone.marginRight),
			"B", APIParT_Length, format_string ("%f", currentLength)));
		plywood.radAng -= DegreeToRad (-90.0);
		moveIn3D ('y', placingZone.ang, -currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
	}

	return err;
}

// 단열재 배치
GSErrCode	SlabTableformPlacingZone::placeInsulations (void)
{
	GSErrCode	err = NoError;

	short	xx, yy;
	short	totalXX, totalYY;
	double	horLen, verLen;
	double	remainHorLen, remainVerLen;

	EasyObjectPlacement insul;

	if (insulElem.bLimitSize == true) {
		// 가로/세로 크기 제한이 true일 때
		remainHorLen = placingZone.borderHorLen;
		remainVerLen = placingZone.borderVerLen;
		totalXX = (short)floor (remainHorLen / insulElem.maxHorLen);
		totalYY = (short)floor (remainVerLen / insulElem.maxVerLen);

		insul.init (L("단열재v1.0.gsm"), insulElem.layerInd, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('y', insul.radAng, -placingZone.borderVerLen, &insul.posX, &insul.posY, &insul.posZ);

		for (xx = 0 ; xx < totalXX+1 ; ++xx) {
			for (yy = 0 ; yy < totalYY+1 ; ++yy) {
				(remainHorLen > insulElem.maxHorLen) ? horLen = insulElem.maxHorLen : horLen = remainHorLen;
				(remainVerLen > insulElem.maxVerLen) ? verLen = insulElem.maxVerLen : verLen = remainVerLen;

				elemList_Insulation.Push (insul.placeObject (10,
					"A", APIParT_Length, format_string ("%f", horLen),
					"B", APIParT_Length, format_string ("%f", verLen),
					"ZZYZX", APIParT_Length, format_string ("%f", insulElem.thk),
					"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"bRestrictSize", APIParT_Boolean, (insulElem.bLimitSize ? "1.0" : "0.0"),
					"maxHorLen", APIParT_Length, format_string ("%f", insulElem.maxHorLen),
					"maxVerLen", APIParT_Length, format_string ("%f", insulElem.maxVerLen),
					"bLShape", APIParT_Boolean, "0.0",
					"bVerticalCut", APIParT_Boolean, "0.0"));

				remainVerLen -= insulElem.maxVerLen;
				moveIn3D ('y', insul.radAng, verLen, &insul.posX, &insul.posY, &insul.posZ);
			}
			remainHorLen -= insulElem.maxHorLen;
			remainVerLen = placingZone.borderVerLen;
			moveIn3D ('y', insul.radAng, -placingZone.borderHorLen, &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('x', insul.radAng, horLen, &insul.posX, &insul.posY, &insul.posZ);
		}
	} else {
		// 가로/세로 크기 제한이 false일 때
		horLen = placingZone.borderHorLen;
		verLen = placingZone.borderVerLen;

		insul.init (L("단열재v1.0.gsm"), insulElem.layerInd, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('y', insul.radAng, -placingZone.borderVerLen, &insul.posX, &insul.posY, &insul.posZ);

		elemList_Insulation.Push (insul.placeObject (10,
			"A", APIParT_Length, format_string ("%f", horLen),
			"B", APIParT_Length, format_string ("%f", verLen),
			"ZZYZX", APIParT_Length, format_string ("%f", insulElem.thk),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"bRestrictSize", APIParT_Boolean, (insulElem.bLimitSize ? "1.0" : "0.0"),
			"maxHorLen", APIParT_Length, format_string ("%f", insulElem.maxHorLen),
			"maxVerLen", APIParT_Length, format_string ("%f", insulElem.maxVerLen),
			"bLShape", APIParT_Boolean, "0.0",
			"bVerticalCut", APIParT_Boolean, "0.0"));
	}

	return err;
}

// 장선, 멍에제, 동바리 배치
GSErrCode	SlabTableformPlacingZone::place_Joist_Yoke_SupportingPost (void)
{
	GSErrCode	err = NoError;
	EasyObjectPlacement	timber, yoke, supp;
	double	remainLength, length;
	bool	bShifted;

	// 장선 배치
	if (placingZone.iJoistDirection == HORIZONTAL) {
		// 가로 방향
		timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', timber.radAng, -0.0115, &timber.posX, &timber.posY, &timber.posZ);
		moveIn3D ('x', timber.radAng, placingZone.marginLeft / 2, &timber.posX, &timber.posY, &timber.posZ);
		moveIn3D ('y', timber.radAng, -placingZone.marginBottom - 0.160, &timber.posX, &timber.posY, &timber.posZ);
		
		bShifted = false;
		remainLength = placingZone.borderHorLen - placingZone.marginLeft / 2 - placingZone.marginRight / 2;
		while (remainLength > EPS) {
			if (remainLength > 3.600)
				length = 3.600;
			else
				length = remainLength;

			timber.placeObject (6,
				"w_ins", APIParT_CString, "바닥눕히기",
				"w_w", APIParT_Length, "0.080",
				"w_h", APIParT_Length, "0.080",
				"w_leng", APIParT_Length, format_string ("%f", length),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
			
			if (length > 1.000)
				moveIn3D ('x', timber.radAng, length - 1.000, &timber.posX, &timber.posY, &timber.posZ);
			else
				moveIn3D ('x', timber.radAng, length, &timber.posX, &timber.posY, &timber.posZ);

			if (bShifted == false) {
				moveIn3D ('y', timber.radAng, 0.080, &timber.posX, &timber.posY, &timber.posZ);
				bShifted = true;
			} else {
				moveIn3D ('y', timber.radAng, -0.080, &timber.posX, &timber.posY, &timber.posZ);
				bShifted = false;
			}

			remainLength -= 2.600;
		}
	} else {
		// 세로 방향
		timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', timber.radAng, -0.0115, &timber.posX, &timber.posY, &timber.posZ);
		moveIn3D ('y', timber.radAng, -placingZone.marginBottom / 2, &timber.posX, &timber.posY, &timber.posZ);
		moveIn3D ('x', timber.radAng, placingZone.marginLeft + 0.160, &timber.posX, &timber.posY, &timber.posZ);
		
		bShifted = false;
		remainLength = placingZone.borderVerLen - placingZone.marginBottom / 2 - placingZone.marginTop / 2;
		while (remainLength > EPS) {
			if (remainLength > 3.600)
				length = 3.600;
			else
				length = remainLength;

			timber.radAng -= DegreeToRad (90.0);
			timber.placeObject (6,
				"w_ins", APIParT_CString, "바닥눕히기",
				"w_w", APIParT_Length, "0.080",
				"w_h", APIParT_Length, "0.080",
				"w_leng", APIParT_Length, format_string ("%f", length),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
			timber.radAng += DegreeToRad (90.0);
			
			if (length > 1.000)
				moveIn3D ('y', timber.radAng, -(length - 1.000), &timber.posX, &timber.posY, &timber.posZ);
			else
				moveIn3D ('y', timber.radAng, -length, &timber.posX, &timber.posY, &timber.posZ);

			if (bShifted == false) {
				moveIn3D ('x', timber.radAng, 0.080, &timber.posX, &timber.posY, &timber.posZ);
				bShifted = true;
			} else {
				moveIn3D ('x', timber.radAng, -0.080, &timber.posX, &timber.posY, &timber.posZ);
				bShifted = false;
			}

			remainLength -= 2.600;
		}
	}

	// 멍에제 배치
	if (placingZone.iYokeType == GT24_GIRDER) {
		// GT24 거더
		yoke.init (L("GT24 거더 v1.0.gsm"), layerInd_GT24Girder, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', yoke.radAng, -0.0115 - 0.240 - 0.080, &yoke.posX, &yoke.posY, &yoke.posZ);

		if (placingZone.iJoistDirection == HORIZONTAL) {
			// 장선은 수평 방향, 멍에제는 수직 방향
			moveIn3D ('x', yoke.radAng, placingZone.marginLeft + 0.160, &yoke.posX, &yoke.posY, &yoke.posZ);
			moveIn3D ('y', yoke.radAng, -placingZone.marginBottom, &yoke.posX, &yoke.posY, &yoke.posZ);
			yoke.radAng -= DegreeToRad (90.0);
			yoke.placeObject (6,
				"type", APIParT_CString, "3000",
				"length", APIParT_Length, format_string ("%f", 2.990),
				"change_rot_method", APIParT_Boolean, "0.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"bWood", APIParT_Boolean, "0.0");
			yoke.radAng += DegreeToRad (90.0);
		} else {
			// 장선은 수직 방향, 멍에제는 수평 방향
			moveIn3D ('y', yoke.radAng, -placingZone.marginBottom - 0.160, &yoke.posX, &yoke.posY, &yoke.posZ);
			moveIn3D ('x', yoke.radAng, placingZone.marginLeft, &yoke.posX, &yoke.posY, &yoke.posZ);
			yoke.placeObject (6,
				"type", APIParT_CString, "3000",
				"length", APIParT_Length, format_string ("%f", 2.990),
				"change_rot_method", APIParT_Boolean, "0.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"bWood", APIParT_Boolean, "0.0");
		}
	} else if (placingZone.iYokeType == SANSUNGAK) {
		// 산승각
		timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', timber.radAng, -0.0115 - 0.080, &timber.posX, &timber.posY, &timber.posZ);

		if (placingZone.iJoistDirection == HORIZONTAL) {
			// 장선은 수평 방향, 멍에제는 수직 방향
			moveIn3D ('x', timber.radAng, placingZone.marginLeft + 0.160 + 0.040, &timber.posX, &timber.posY, &timber.posZ);
			moveIn3D ('y', timber.radAng, -placingZone.marginBottom, &timber.posX, &timber.posY, &timber.posZ);
			timber.radAng -= DegreeToRad (90.0);
			timber.placeObject (6,
				"w_ins", APIParT_CString, "바닥눕히기",
				"w_w", APIParT_Length, "0.080",
				"w_h", APIParT_Length, "0.080",
				"w_leng", APIParT_Length, format_string ("%f", 3.000),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
			timber.radAng += DegreeToRad (90.0);
		} else {
			// 장선은 수직 방향, 멍에제는 수평 방향
			moveIn3D ('y', timber.radAng, -placingZone.marginBottom - 0.160 - 0.040, &timber.posX, &timber.posY, &timber.posZ);
			moveIn3D ('x', timber.radAng, placingZone.marginLeft, &timber.posX, &timber.posY, &timber.posZ);
			timber.placeObject (6,
				"w_ins", APIParT_CString, "바닥눕히기",
				"w_w", APIParT_Length, "0.080",
				"w_h", APIParT_Length, "0.080",
				"w_leng", APIParT_Length, format_string ("%f", 3.000),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
		}
	}

	// 동바리 배치
	if (placingZone.iSuppPostType == SUPPORT) {
		// 강관 동바리
		supp.init (L("서포트v1.0.gsm"), layerInd_Steel_Support, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', supp.radAng, -2 - 0.0115 - 0.080, &supp.posX, &supp.posY, &supp.posZ);

		if (placingZone.iYokeType == GT24_GIRDER) {
			moveIn3D ('z', supp.radAng, -0.240 - 0.003, &supp.posX, &supp.posY, &supp.posZ);
		} else if (placingZone.iYokeType == SANSUNGAK) {
			moveIn3D ('z', supp.radAng, -0.080 - 0.003, &supp.posX, &supp.posY, &supp.posZ);
		}

		if (placingZone.iJoistDirection == HORIZONTAL) {
			moveIn3D ('x', supp.radAng, placingZone.marginLeft + 0.160 + 0.040, &supp.posX, &supp.posY, &supp.posZ);
			moveIn3D ('y', supp.radAng, -placingZone.marginBottom - 0.200, &supp.posX, &supp.posY, &supp.posZ);
			supp.placeObject (7,
				"s_bimj", APIParT_Boolean, "0.0",
				"s_stan", APIParT_CString, "V0 (2.0m)",
				"s_leng", APIParT_Length, "2.000",
				"s_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
				"bCrosshead", APIParT_Boolean, "1.0",
				"crossheadType", APIParT_CString, "PERI",
				"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
		} else {
			moveIn3D ('y', supp.radAng, -placingZone.marginBottom - 0.160 - 0.040, &supp.posX, &supp.posY, &supp.posZ);
			moveIn3D ('x', supp.radAng, placingZone.marginLeft + 0.200, &supp.posX, &supp.posY, &supp.posZ);
			supp.placeObject (7,
				"s_bimj", APIParT_Boolean, "0.0",
				"s_stan", APIParT_CString, "V0 (2.0m)",
				"s_leng", APIParT_Length, "2.000",
				"s_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
				"bCrosshead", APIParT_Boolean, "1.0",
				"crossheadType", APIParT_CString, "PERI",
				"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)));
		}
	} else if (placingZone.iSuppPostType == PERI) {
		// PERI 동바리
		supp.init (L("PERI동바리 수직재 v0.1.gsm"), layerInd_PERI_Support, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', supp.radAng, -0.0115 - 0.080, &supp.posX, &supp.posY, &supp.posZ);

		if (placingZone.iYokeType == GT24_GIRDER) {
			moveIn3D ('z', supp.radAng, -0.240 - 0.003, &supp.posX, &supp.posY, &supp.posZ);
		} else if (placingZone.iYokeType == SANSUNGAK) {
			moveIn3D ('z', supp.radAng, -0.080 - 0.003, &supp.posX, &supp.posY, &supp.posZ);
		}

		if (placingZone.iJoistDirection == HORIZONTAL) {
			moveIn3D ('x', supp.radAng, placingZone.marginLeft + 0.160 + 0.040, &supp.posX, &supp.posY, &supp.posZ);
			moveIn3D ('y', supp.radAng, -placingZone.marginBottom - 0.200, &supp.posX, &supp.posY, &supp.posZ);
			supp.placeObject (9,
				"stType", APIParT_CString, "MP 350",
				"bCrosshead", APIParT_Boolean, "1.0",
				"posCrosshead", APIParT_CString, "하단",
				"crossheadType", APIParT_CString, "PERI",
				"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"len_current", APIParT_Length, "2.000",
				"Z", APIParT_Length, "2.000",
				"pos_lever", APIParT_Length, "1.750",
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)));
		} else {
			moveIn3D ('y', supp.radAng, -placingZone.marginBottom - 0.160 - 0.040, &supp.posX, &supp.posY, &supp.posZ);
			moveIn3D ('x', supp.radAng, placingZone.marginLeft + 0.200, &supp.posX, &supp.posY, &supp.posZ);
			supp.placeObject (9,
				"stType", APIParT_CString, "MP 350",
				"bCrosshead", APIParT_Boolean, "1.0",
				"posCrosshead", APIParT_CString, "하단",
				"crossheadType", APIParT_CString, "PERI",
				"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
				"len_current", APIParT_Length, "2.000",
				"Z", APIParT_Length, "2.000",
				"pos_lever", APIParT_Length, "1.750",
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)));
		}
	}

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
			DGSetDialogTitle (dialogID, L"슬래브 하부에 배치");

			//////////////////////////////////////////////////////////// 아이템 배치
			DGSetItemText (dialogID, DG_OK, L"확 인");
			DGSetItemText (dialogID, DG_CANCEL, L"취 소");

			DGSetItemText (dialogID, LABEL_SELECT_TYPE, L"타입 선택");

			DGSetItemText (dialogID, PUSHRADIO_CONPANEL, L"콘판넬");
			DGSetItemText (dialogID, PUSHRADIO_PLYWOOD, L"합판");
			DGSetItemText (dialogID, PUSHRADIO_EUROFORM, L"유로폼");

			DGSetItemText (dialogID, LABEL_OTHER_SETTINGS, L"기타 설정");
			DGSetItemText (dialogID, LABEL_CELL_DIRECTION, L"판넬(폼)방향");
			DGSetItemText (dialogID, LABEL_JOIST_DIRECTION, L"장선방향");
			DGSetItemText (dialogID, LABEL_YOKE_TYPE, L"멍에제 종류");
			DGSetItemText (dialogID, LABEL_SUPPORTING_POST_TYPE, L"동바리 종류");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, L"슬래브와의 간격");

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, L"부재별 레이어 설정");
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, L"레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, L"유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_CONPANEL, L"콘판넬");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, L"합판");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, L"각재");
			DGSetItemText (dialogID, LABEL_LAYER_CPROFILE, L"C형강");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, L"핀볼트");
			DGSetItemText (dialogID, LABEL_LAYER_FITTINGS, L"결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_GT24_GIRDER, L"GT24 거더");
			DGSetItemText (dialogID, LABEL_LAYER_PERI_SUPPORT, L"PERI동바리");
			DGSetItemText (dialogID, LABEL_LAYER_STEEL_SUPPORT, L"강관 동바리");

			DGSetItemText (dialogID, BUTTON_AUTOSET, L"레이어 자동 설정");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_CONPANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CONPANEL, 1);

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

			// 1번째 타입 선택
			DGSetItemValLong (dialogID, PUSHRADIO_CONPANEL, TRUE);

			// 셀방향 추가
			DGPopUpInsertItem (dialogID, POPUP_CELL_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_CELL_DIRECTION, DG_POPUP_BOTTOM, L"가로");
			DGPopUpInsertItem (dialogID, POPUP_CELL_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_CELL_DIRECTION, DG_POPUP_BOTTOM, L"세로");

			// 장선방향 추가
			DGPopUpInsertItem (dialogID, POPUP_JOIST_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_JOIST_DIRECTION, DG_POPUP_BOTTOM, L"가로");
			DGPopUpInsertItem (dialogID, POPUP_JOIST_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_JOIST_DIRECTION, DG_POPUP_BOTTOM, L"세로");

			// 멍에제 종류 추가
			DGPopUpInsertItem (dialogID, POPUP_YOKE_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_YOKE_TYPE, DG_POPUP_BOTTOM, L"GT24 거더");
			DGPopUpInsertItem (dialogID, POPUP_YOKE_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_YOKE_TYPE, DG_POPUP_BOTTOM, L"산승각");

			// 동바리 종류 추가
			DGPopUpInsertItem (dialogID, POPUP_SUPPORTING_POST_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_SUPPORTING_POST_TYPE, DG_POPUP_BOTTOM, L"강관 동바리");
			DGPopUpInsertItem (dialogID, POPUP_SUPPORTING_POST_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_SUPPORTING_POST_TYPE, DG_POPUP_BOTTOM, L"PERI 동바리");

			// 레이어 활성화/비활성화
			DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
			DGDisableItem (dialogID, LABEL_LAYER_CPROFILE);
			DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
			DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);
			DGDisableItem (dialogID, LABEL_LAYER_PERI_SUPPORT);

			DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			DGDisableItem (dialogID, USERCONTROL_LAYER_CPROFILE);
			DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			DGDisableItem (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);

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

			// 레이어 활성화/비활성화
			for (xx = LABEL_LAYER_EUROFORM ; xx <= LABEL_LAYER_STEEL_SUPPORT ; ++xx)
				DGDisableItem (dialogID, xx);
			for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_STEEL_SUPPORT ; ++xx)
				DGDisableItem (dialogID, xx);

			if (DGGetItemValLong (dialogID, PUSHRADIO_CONPANEL) == TRUE) {
				DGEnableItem (dialogID, LABEL_LAYER_CONPANEL);
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);

				DGEnableItem (dialogID, USERCONTROL_LAYER_CONPANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}
			if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE) {
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);

				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}
			if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);

				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}

			if (DGPopUpGetSelected (dialogID, POPUP_YOKE_TYPE) == GT24_GIRDER) {
				DGEnableItem (dialogID, LABEL_LAYER_GT24_GIRDER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
			}

			if (DGPopUpGetSelected (dialogID, POPUP_SUPPORTING_POST_TYPE) == SUPPORT) {
				DGEnableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);
			} else if (DGPopUpGetSelected (dialogID, POPUP_SUPPORTING_POST_TYPE) == PERI) {
				DGEnableItem (dialogID, LABEL_LAYER_PERI_SUPPORT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 타입 선택
					if (DGGetItemValLong (dialogID, PUSHRADIO_CONPANEL) == TRUE)
						placingZone.iTableformType = CONPANEL;
					else if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE)
						placingZone.iTableformType = PLYWOOD;
					else if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE)
						placingZone.iTableformType = EUROFORM;
					else
						placingZone.iTableformType = NONE;

					// 셀 방향
					placingZone.iCellDirection = DGPopUpGetSelected (dialogID, POPUP_CELL_DIRECTION);

					// 장선 방향
					placingZone.iJoistDirection = DGPopUpGetSelected (dialogID, POPUP_JOIST_DIRECTION);

					// 멍에제 종류
					placingZone.iYokeType = DGPopUpGetSelected (dialogID, POPUP_YOKE_TYPE);

					// 동바리 종류
					placingZone.iSuppPostType = DGPopUpGetSelected (dialogID, POPUP_SUPPORTING_POST_TYPE);

					// 슬래브와의 간격값 저장
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_ConPanel		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CONPANEL);
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
					layerInd_ConPanel		= makeTemporaryLayer (structuralObject_forTableformSlab, "CONP", NULL);
					layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformSlab, "PLYW", NULL);
					layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformSlab, "TIMB", NULL);
					layerInd_CProfile		= makeTemporaryLayer (structuralObject_forTableformSlab, "CPRO", NULL);
					layerInd_Pinbolt		= makeTemporaryLayer (structuralObject_forTableformSlab, "PINB", NULL);
					layerInd_Fittings		= makeTemporaryLayer (structuralObject_forTableformSlab, "CLAM", NULL);
					layerInd_GT24Girder		= makeTemporaryLayer (structuralObject_forTableformSlab, "GIDR", NULL);
					layerInd_PERI_Support	= makeTemporaryLayer (structuralObject_forTableformSlab, "MULT", NULL);
					layerInd_Steel_Support	= makeTemporaryLayer (structuralObject_forTableformSlab, "SUPT", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_CONPANEL, layerInd_ConPanel);
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
			// placingZone의 Cell 정보 초기화
			placingZone.initCells (&placingZone);

			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"슬래브 하부에 배치 - 배치 수정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 배치 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 110, 130, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"배치 - 자투리 채우기");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼 1
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 150, 130, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"배치 - 자투리 제외");
			DGShowItem (dialogID, DG_CANCEL);

			// 이전 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 190, 130, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, L"이전");
			DGShowItem (dialogID, DG_PREV);

			// 행 추가 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 30, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_ROW, L"행 추가");
			DGShowItem (dialogID, PUSHBUTTON_ADD_ROW);

			// 행 삭제 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 85, 30, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_ROW, L"행 삭제");
			DGShowItem (dialogID, PUSHBUTTON_DEL_ROW);

			// 열 추가 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 70, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_COL, L"열 추가");
			DGShowItem (dialogID, PUSHBUTTON_ADD_COL);
		
			// 열 삭제 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 85, 70, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_COL, L"열 삭제");
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
					if (placingZone.iTableformType == CONPANEL) {
						txtButton = format_string ("콘판넬\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else if (placingZone.iTableformType == PLYWOOD) {
						txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else if (placingZone.iTableformType == EUROFORM) {
						txtButton = format_string ("유로폼\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else {
						txtButton = format_string ("없음");
					}
					DGSetItemText (dialogID, placingZone.CELL_BUTTON [xx][yy], charToWchar (txtButton.c_str ()));		// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, placingZone.CELL_BUTTON [xx][yy]);
					btnPosX += btnSizeX;
				}
				btnPosX = btnInitPosX;
				btnPosY -= btnSizeY;
			}

			// 라벨: 여백(좌)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"여백(좌)");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 여백(우)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"여백(우)");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 여백(상)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, 30, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"여백(상)");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 여백(하)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY), 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"여백(하)");
			DGShowItem (dialogID, itmIdx);

			// Edit 컨트롤: 여백(좌)
			placingZone.EDITCONTROL_MARGIN_LEFT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.borderHorLen - placingZone.cellArrayWidth - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);

			// Edit 컨트롤: 여백(우)
			placingZone.EDITCONTROL_MARGIN_RIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.borderHorLen - placingZone.cellArrayWidth - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);

			// Edit 컨트롤: 여백(상)
			placingZone.EDITCONTROL_MARGIN_TOP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.borderVerLen - placingZone.cellArrayHeight - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);

			// Edit 컨트롤: 여백(하)
			placingZone.EDITCONTROL_MARGIN_BOTTOM = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY) - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.borderVerLen - placingZone.cellArrayHeight - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);

			break;

		case DG_MSG_CHANGE:
			// 여백(좌) 변경시, 여백(우) 변경됨
			if (item == placingZone.EDITCONTROL_MARGIN_LEFT) {
				placingZone.marginLeft = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				placingZone.marginRight = placingZone.borderHorLen - placingZone.cellArrayWidth - placingZone.marginLeft;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);
			}

			// 여백(우) 변경시, 여백(좌) 변경됨
			if (item == placingZone.EDITCONTROL_MARGIN_RIGHT) {
				placingZone.marginRight = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				placingZone.marginLeft = placingZone.borderHorLen - placingZone.cellArrayWidth - placingZone.marginRight;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);
			}

			// 여백(상) 변경시, 여백(하) 변경됨
			if (item == placingZone.EDITCONTROL_MARGIN_TOP) {
				placingZone.marginTop = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				placingZone.marginBottom = placingZone.borderVerLen - placingZone.cellArrayHeight - placingZone.marginTop;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);
			}

			// 여백(하) 변경시, 여백(상) 변경됨
			if (item == placingZone.EDITCONTROL_MARGIN_BOTTOM) {
				placingZone.marginBottom = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
				placingZone.marginTop = placingZone.borderVerLen - placingZone.cellArrayHeight - placingZone.marginBottom;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);
			}

			break;

		case DG_MSG_CLICK:
			if (item == DG_OK) {
				// 배치 - 자투리 채우기 버튼
				clickedExcludeRestButton = false;

				// 셀 배열 전체 너비, 높이 저장
				placingZone.cellArrayWidth = 0.0;
				placingZone.cellArrayHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.cellArrayWidth += placingZone.cells [0][xx].horLen;
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.cellArrayHeight += placingZone.cells [xx][0].verLen;

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

				// 셀 배열 전체 너비, 높이 저장
				placingZone.cellArrayWidth = 0.0;
				placingZone.cellArrayHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.cellArrayWidth += placingZone.cells [0][xx].horLen;
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.cellArrayHeight += placingZone.cells [xx][0].verLen;

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

				// 셀 배열 전체 너비, 높이 지정
				placingZone.cellArrayWidth = 0.0;
				placingZone.cellArrayHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.cellArrayWidth += placingZone.cells [0][xx].horLen;
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.cellArrayHeight += placingZone.cells [xx][0].verLen;

				// 여백 길이 지정
				placingZone.marginLeft = placingZone.marginRight = (placingZone.borderHorLen - placingZone.cellArrayWidth) / 2;
				placingZone.marginBottom = placingZone.marginTop = (placingZone.borderVerLen - placingZone.cellArrayHeight) / 2;

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
						if (placingZone.iTableformType == CONPANEL) {
							txtButton = format_string ("콘판넬\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else if (placingZone.iTableformType == PLYWOOD) {
							txtButton = format_string ("합판\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else if (placingZone.iTableformType == EUROFORM) {
							txtButton = format_string ("유로폼\n↔%.0f\n↕%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else {
							txtButton = format_string ("없음");
						}
						DGSetItemText (dialogID, placingZone.CELL_BUTTON [xx][yy], charToWchar (txtButton.c_str ()));		// 그리드 버튼 텍스트 지정
						DGShowItem (dialogID, placingZone.CELL_BUTTON [xx][yy]);
						btnPosX += btnSizeX;
					}
					btnPosX = btnInitPosX;
					btnPosY -= btnSizeY;
				}

				// 라벨: 여백(좌)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"여백(좌)");
				DGShowItem (dialogID, itmIdx);

				// 라벨: 여백(우)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"여백(우)");
				DGShowItem (dialogID, itmIdx);

				// 라벨: 여백(상)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, 30, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"여백(상)");
				DGShowItem (dialogID, itmIdx);

				// 라벨: 여백(하)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY), 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"여백(하)");
				DGShowItem (dialogID, itmIdx);

				// Edit 컨트롤: 여백(좌)
				placingZone.EDITCONTROL_MARGIN_LEFT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.borderHorLen - placingZone.cellArrayWidth - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);

				// Edit 컨트롤: 여백(우)
				placingZone.EDITCONTROL_MARGIN_RIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.borderHorLen - placingZone.cellArrayWidth - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);

				// Edit 컨트롤: 여백(상)
				placingZone.EDITCONTROL_MARGIN_TOP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.borderVerLen - placingZone.cellArrayHeight - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);

				// Edit 컨트롤: 여백(하)
				placingZone.EDITCONTROL_MARGIN_BOTTOM = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY) - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.borderVerLen - placingZone.cellArrayHeight - 0.001);
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

	switch (message) {
		case DG_MSG_INIT:
			
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"셀 값 설정");

			////////////////////////////////////////////////////////////  아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 215, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"저장");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 130, 215, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"취소");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
			// 라벨: 객체 타입
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, L"객체 타입");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// 팝업컨트롤: 객체 타입 (콘판넬, 합판, 유로폼)
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 20-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"콘판넬");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"합판");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"유로폼");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);
			if (placingZone.cells [clickedRow][clickedCol].objType == CONPANEL)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, CONPANEL);
			else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PLYWOOD);
			else if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM);
			DGDisableItem (dialogID, POPUP_OBJ_TYPE);

			// 라벨: 너비
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 60, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, L"너비");
			DGShowItem (dialogID, LABEL_WIDTH);

			// Edit컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 60-6, 100, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_WIDTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [clickedRow][clickedCol].horLen);
			if (placingZone.cells [clickedRow][clickedCol].objType == CONPANEL) {
				// 콘판넬 크기: 2x6 [606x1820] - 크기 고정
				DGDisableItem (dialogID, EDITCONTROL_WIDTH);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD) {
				DGEnableItem (dialogID, EDITCONTROL_WIDTH);
				// 합판 크기: 4x8 [1220x2440]
				if (placingZone.iCellDirection == VERTICAL)
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.220);
				else
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 2.440);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM) {
				DGEnableItem (dialogID, EDITCONTROL_WIDTH);
				// 유로폼 크기: 600x1200 (최대 900x1500)
				if (placingZone.iCellDirection == VERTICAL)
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.900);
				else
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.500);
			}

			// 라벨: 높이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 100, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, L"높이");
			DGShowItem (dialogID, LABEL_HEIGHT);

			// Edit컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 100-6, 100, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_HEIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [clickedRow][clickedCol].verLen);
			if (placingZone.cells [clickedRow][clickedCol].objType == CONPANEL) {
				// 콘판넬 크기: 2x6 [606x1820] - 크기 고정
				DGDisableItem (dialogID, EDITCONTROL_HEIGHT);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD) {
				DGEnableItem (dialogID, EDITCONTROL_HEIGHT);
				// 합판 크기: 4x8 [1220x2440]
				if (placingZone.iCellDirection == VERTICAL)
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.440);
				else
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.220);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM) {
				DGEnableItem (dialogID, EDITCONTROL_HEIGHT);
				// 유로폼 크기: 600x1200 (최대 900x1500)
				if (placingZone.iCellDirection == VERTICAL)
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.500);
				else
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 0.900);
			}

			// 라벨: 설치방향
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION, L"설치방향");
			DGShowItem (dialogID, LABEL_ORIENTATION);

			// 팝업컨트롤: 설치방향
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 140-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM, L"가로방향");
			DGPopUpInsertItem (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM, L"세로방향");
			DGShowItem (dialogID, POPUP_ORIENTATION);
			DGDisableItem (dialogID, POPUP_ORIENTATION);

			// 라벨: 경고 문구
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 170, 220, 40);
			DGSetItemFont (dialogID, LABEL_CAUTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_CAUTION, L"비어 있음");

			if (placingZone.iCellDirection == HORIZONTAL)
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, HORIZONTAL);
			else
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, VERTICAL);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					for (xx = 0 ; xx < MAX_IND ; ++xx) {
						placingZone.cells [xx][clickedCol].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
						placingZone.cells [clickedRow][xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
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

// 슬래브 하부의 간격에 단열재를 배치함
short DGCALLBACK slabBottomTableformPlacerHandler4_Insulation (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 타이틀
			DGSetDialogTitle (dialogID, L"단열재 배치");

			// 라벨
			DGSetItemText (dialogID, LABEL_EXPLANATION_INS, L"단열재 규격을 입력하십시오.");
			DGSetItemText (dialogID, LABEL_INSULATION_THK, L"두께");
			DGSetItemText (dialogID, LABEL_INS_HORLEN, L"가로");
			DGSetItemText (dialogID, LABEL_INS_VERLEN, L"세로");

			// 체크박스
			DGSetItemText (dialogID, CHECKBOX_INS_LIMIT_SIZE, L"가로/세로 크기 제한");
			DGSetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE, TRUE);

			// Edit 컨트롤
			DGSetItemValDouble (dialogID, EDITCONTROL_INS_HORLEN, 0.900);
			DGSetItemValDouble (dialogID, EDITCONTROL_INS_VERLEN, 1.800);

			// 레이어
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_INSULATION_LAYER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_INSULATION_LAYER, 1);

			// 버튼
			DGSetItemText (dialogID, DG_OK, L"확인");
			DGSetItemText (dialogID, DG_CANCEL, L"취소");

			// 두께는 자동
			DGSetItemValDouble (dialogID, EDITCONTROL_INSULATION_THK, placingZone.gap);
			DGDisableItem (dialogID, EDITCONTROL_INSULATION_THK);
 
			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case CHECKBOX_INS_LIMIT_SIZE:
					if (DGGetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE) == TRUE) {
						DGEnableItem (dialogID, EDITCONTROL_INS_HORLEN);
						DGEnableItem (dialogID, EDITCONTROL_INS_VERLEN);
					} else {
						DGDisableItem (dialogID, EDITCONTROL_INS_HORLEN);
						DGDisableItem (dialogID, EDITCONTROL_INS_VERLEN);
					}
					break;
			}
 
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 레이어 정보 저장
					insulElem.layerInd = (short)DGGetItemValLong (dialogID, USERCONTROL_INSULATION_LAYER);

					// 두께, 가로, 세로 저장
					insulElem.thk = DGGetItemValDouble (dialogID, EDITCONTROL_INSULATION_THK);
					insulElem.maxHorLen = DGGetItemValDouble (dialogID, EDITCONTROL_INS_HORLEN);
					insulElem.maxVerLen = DGGetItemValDouble (dialogID, EDITCONTROL_INS_VERLEN);
					if (DGGetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE) == TRUE)
						insulElem.bLimitSize = true;
					else
						insulElem.bLimitSize = false;

					break;
				case DG_CANCEL:
					break;
			}
			break;

		case DG_MSG_CLOSE:
			break;
	}

	result = item;

	return	result;
}