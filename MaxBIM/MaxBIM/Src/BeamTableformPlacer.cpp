#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "BeamTableformPlacer.hpp"

using namespace beamTableformPlacerDG;

static BeamTableformPlacingZone		placingZone;			// 기본 보 영역 정보
static InfoBeam			infoBeam;							// 보 객체 정보
API_Guid				structuralObject_forTableformBeam;	// 구조 객체의 GUID

static short	layerInd_Euroform;			// 레이어 번호: 유로폼
static short	layerInd_Plywood;			// 레이어 번호: 합판
static short	layerInd_Timber;			// 레이어 번호: 각재
static short	layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static short	layerInd_Fillerspacer;		// 레이어 번호: 휠러스페이서
static short	layerInd_Rectpipe;			// 레이어 번호: 비계파이프
static short	layerInd_RectpipeHanger;	// 레이어 번호: 각파이프행거
static short	layerInd_Pinbolt;			// 레이어 번호: 핀볼트
static short	layerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
static short	layerInd_BlueClamp;			// 레이어 번호: 블루클램프
static short	layerInd_BlueTimberRail;	// 레이어 번호: 블루목심
static short	clickedBtnItemIdx;			// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
static bool		clickedOKButton;			// OK 버튼을 눌렀습니까?
static bool		clickedPrevButton;			// 이전 버튼을 눌렀습니까?
static GS::Array<API_Guid>	elemList;		// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함


// 보에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnBeam (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx, yy;
	double			dx, dy;
	short			result;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		beams;
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
	GS::Array<API_Coord3D>	coords;
	long					nNodes;

	// 모프 객체 정보
	InfoMorphForBeamTableform	infoMorph [2];
	API_Coord3D					morph1_point [2];
	API_Coord3D					morph2_point [2];
	double						morph1_height = 0.0;
	double						morph2_height = 0.0;

	// 작업 층 정보
	API_StoryInfo			storyInfo;
	double					workLevel_beam;


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 보 (1개), 보 측면(전체/일부)을 덮는 모프 (1개)\n옵션 선택 (1): 보 반대쪽 측면을 덮는 모프 (1개)\n옵션 선택 (2): 보 아래쪽 면을 덮는 모프 (1개)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 보 1개, 모프 1~2개 선택해야 함
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

	// 모프가 1~2개인가?
	if ( !((nMorphs >= 1) && (nMorphs <= 2)) ) {
		ACAPI_WriteReport ("보 측면(전체/일부)을 덮는 모프를 1개 선택하셔야 합니다.\n덮는 높이가 비대칭이면 보 반대쪽 측면을 덮는 모프도 있어야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 보 정보 저장
	infoBeam.guid = beams.Pop ();

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = infoBeam.guid;
	structuralObject_forTableformBeam = elem.header.guid;
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

	for (xx = 0 ; xx < nMorphs ; ++xx) {
		// 모프 정보를 가져옴
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = morphs.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

		// 모프의 정보 저장
		infoMorph [xx].guid		= elem.header.guid;
		infoMorph [xx].floorInd	= elem.header.floorInd;
		infoMorph [xx].level	= info3D.bounds.zMin;

		// 모프의 3D 바디를 가져옴
		BNZeroMemory (&component, sizeof (API_Component3D));
		component.header.typeID = API_BodyID;
		component.header.index = info3D.fbody;
		err = ACAPI_3D_GetComponent (&component);

		nVert = component.body.nVert;
		nEdge = component.body.nEdge;
		nPgon = component.body.nPgon;
		tm = component.body.tranmat;
		elemIdx = component.body.head.elemIndex - 1;
		bodyIdx = component.body.head.bodyIndex - 1;
		
		// 정점 좌표를 임의 순서대로 저장함
		for (yy = 1 ; yy <= nVert ; ++yy) {
			component.header.typeID	= API_VertID;
			component.header.index	= yy;
			err = ACAPI_3D_GetComponent (&component);
			if (err == NoError) {
				trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
				trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
				trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
				coords.Push (trCoord);
			}
		}
		nNodes = coords.GetSize ();

		// 1번째 모프의 양 끝점 추출
		if (xx == 0) {
			morph1_point [0] = coords [0];	// 모프의 1번째 점 저장
			for (yy = 1 ; yy < nNodes ; ++yy) {
				// x, y 좌표 값이 같으면 통과하고, 다르면 2번째 점으로 저장
				if ( (abs (morph1_point [0].x - coords [yy].x) < EPS) && (abs (morph1_point [0].y - coords [yy].y) < EPS) ) {
					continue;
				} else {
					morph1_point [1] = coords [yy];
					break;
				}
			}

			morph1_height = info3D.bounds.zMax - info3D.bounds.zMin;
		}

		// 2번째 모프의 양 끝점 추출
		if (xx == 1) {
			morph2_point [0] = coords [0];	// 모프의 1번째 점 저장
			for (yy = 1 ; yy < nNodes ; ++yy) {
				// x, y 좌표 값이 같으면 통과하고, 다르면 2번째 점으로 저장
				if ( (abs (morph2_point [0].x - coords [yy].x) < EPS) && (abs (morph2_point [0].y - coords [yy].y) < EPS) ) {
					continue;
				} else {
					morph2_point [1] = coords [yy];
					break;
				}
			}

			morph2_height = info3D.bounds.zMax - info3D.bounds.zMin;
		}

		// 1번째 모프를 기반으로 좌하단, 우상단 점 가져오기
		if (xx == 0) {
			// 모프의 좌하단, 우상단 점 지정
			if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
				// 좌하단 좌표 결정
				infoMorph [xx].leftBottomX = elem.morph.tranmat.tmx [3];
				infoMorph [xx].leftBottomY = elem.morph.tranmat.tmx [7];
				infoMorph [xx].leftBottomZ = elem.morph.tranmat.tmx [11];

				// 우상단 좌표는?
				if (abs (infoMorph [xx].leftBottomX - info3D.bounds.xMin) < EPS)
					infoMorph [xx].rightTopX = info3D.bounds.xMax;
				else
					infoMorph [xx].rightTopX = info3D.bounds.xMin;
				if (abs (infoMorph [xx].leftBottomY - info3D.bounds.yMin) < EPS)
					infoMorph [xx].rightTopY = info3D.bounds.yMax;
				else
					infoMorph [xx].rightTopY = info3D.bounds.yMin;
				if (abs (infoMorph [xx].leftBottomZ - info3D.bounds.zMin) < EPS)
					infoMorph [xx].rightTopZ = info3D.bounds.zMax;
				else
					infoMorph [xx].rightTopZ = info3D.bounds.zMin;
			} else {
				// 우상단 좌표 결정
				infoMorph [xx].rightTopX = elem.morph.tranmat.tmx [3];
				infoMorph [xx].rightTopY = elem.morph.tranmat.tmx [7];
				infoMorph [xx].rightTopZ = elem.morph.tranmat.tmx [11];

				// 좌하단 좌표는?
				if (abs (infoMorph [xx].rightTopX - info3D.bounds.xMin) < EPS)
					infoMorph [xx].leftBottomX = info3D.bounds.xMax;
				else
					infoMorph [xx].leftBottomX = info3D.bounds.xMin;
				if (abs (infoMorph [xx].rightTopY - info3D.bounds.yMin) < EPS)
					infoMorph [xx].leftBottomY = info3D.bounds.yMax;
				else
					infoMorph [xx].leftBottomY = info3D.bounds.yMin;
				if (abs (infoMorph [xx].rightTopZ - info3D.bounds.zMin) < EPS)
					infoMorph [xx].leftBottomZ = info3D.bounds.zMax;
				else
					infoMorph [xx].leftBottomZ = info3D.bounds.zMin;
			}

			// 모프의 Z축 회전 각도
			dx = infoMorph [xx].rightTopX - infoMorph [xx].leftBottomX;
			dy = infoMorph [xx].rightTopY - infoMorph [xx].leftBottomY;
			infoMorph [xx].ang = atan2 (dy, dx);
		}

		// 저장된 좌표값 버리기
		coords.Clear ();

		// 영역 모프 제거
		API_Elem_Head* headList = new API_Elem_Head [1];
		headList [0] = elem.header;
		err = ACAPI_Element_Delete (&headList, 1);
		delete headList;
	}

	// 영역 높이 저장
	if (nMorphs == 2) {
		if (morph1_height > morph2_height) {
			placingZone.areaHeight_Left = morph1_height;
			placingZone.areaHeight_Right = morph2_height;
		} else {
			placingZone.areaHeight_Left = morph2_height;
			placingZone.areaHeight_Right = morph1_height;
		}
	} else {
		placingZone.areaHeight_Left = morph1_height;
		placingZone.areaHeight_Right = morph1_height;
	}

	// 보 길이
	placingZone.beamLength = GetDistance (morph1_point [0], morph1_point [1]);

	// 보 너비
	placingZone.areaWidth_Bottom = infoBeam.width;

	// 보 오프셋
	placingZone.offset = infoBeam.offset;

	// 보 윗면 고도
	placingZone.level = infoBeam.level;

	// 배치 기준 시작점, 끝점 (영역 모프의 높이가 높은쪽이 기준이 됨)
	if ((infoMorph [0].rightTopZ - infoMorph [0].leftBottomZ) - (infoMorph [1].rightTopZ - infoMorph [1].leftBottomZ) > EPS) {
		placingZone.begC.x = infoMorph [0].leftBottomX;
		placingZone.begC.y = infoMorph [0].leftBottomY;
		placingZone.begC.z = infoMorph [0].leftBottomZ;

		placingZone.endC.x = infoMorph [0].rightTopX;
		placingZone.endC.y = infoMorph [0].rightTopY;
		placingZone.endC.z = infoMorph [0].leftBottomZ;

		placingZone.ang = infoMorph [0].ang;
	} else {
		placingZone.begC.x = infoMorph [1].leftBottomX;
		placingZone.begC.y = infoMorph [1].leftBottomY;
		placingZone.begC.z = infoMorph [1].leftBottomZ;

		placingZone.endC.x = infoMorph [1].rightTopX;
		placingZone.endC.y = infoMorph [1].rightTopY;
		placingZone.endC.z = infoMorph [1].leftBottomZ;

		placingZone.ang = infoMorph [1].ang;
	}

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_beam = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoBeam.floorInd) {
			workLevel_beam = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

FIRST:

	// placingZone의 Cell 정보 초기화
	placingZone.initCells (&placingZone);

	// [DIALOG] 1번째 다이얼로그에서 유로폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32522, ACAPI_GetOwnResModule (), beamTableformPlacerHandler1, 0);

	if (result == DG_CANCEL)
		return err;

	// !!!
	// 총 길이에서 유로폼 높이 값으로 나누어서 대략적인 셀 개수(nCells) 초기 지정

	// [DIALOG] 2번째 다이얼로그에서 유로폼 배치를 수정합니다.
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (500, 360, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, beamTableformPlacerHandler2, 0);
	
	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST;

	// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
	if (clickedOKButton != true)
		return err;

	// ... 배치하기

	// ... 나머지 영역 채우기 - 합판, 각재
	err = placingZone.fillRestAreas (&placingZone);

	// 결과물 전체 그룹화
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

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// Cell 배열을 초기화함
void	BeamTableformPlacingZone::initCells (BeamTableformPlacingZone* placingZone)
{
	short xx, yy;

	// 영역 정보의 여백 채움 여부 초기화
	placingZone->bFillMarginBegin = false;
	placingZone->bFillMarginEnd = false;

	// 영역 정보의 여백 설정 초기화
	placingZone->marginBegin = 0.0;
	placingZone->marginEnd = 0.0;

	// 셀 개수 초기화
	placingZone->nCells = 0;

	// 셀 정보 초기화
	for (xx = 0 ; xx < 4 ; ++xx) {
		for (yy = 0 ; yy < 50 ; ++yy) {
			placingZone->cellsAtLSide [xx][yy].objType = NONE;
			placingZone->cellsAtLSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtLSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtLSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtLSide [xx][yy].ang = 0.0;
			placingZone->cellsAtLSide [xx][yy].dirLen = 0.0;
			placingZone->cellsAtLSide [xx][yy].perLen = 0.0;
			placingZone->cellsAtLSide [xx][yy].attached_side = LEFT_SIDE;

			placingZone->cellsAtRSide [xx][yy].objType = NONE;
			placingZone->cellsAtRSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtRSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtRSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtRSide [xx][yy].ang = 0.0;
			placingZone->cellsAtRSide [xx][yy].dirLen = 0.0;
			placingZone->cellsAtRSide [xx][yy].perLen = 0.0;
			placingZone->cellsAtRSide [xx][yy].attached_side = RIGHT_SIDE;

		}
	}

	for (xx = 0 ; xx < 3 ; ++xx) {
		for (yy = 0 ; yy < 50 ; ++yy) {
			placingZone->cellsAtBottom [xx][yy].objType = NONE;
			placingZone->cellsAtBottom [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtBottom [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtBottom [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtBottom [xx][yy].ang = 0.0;
			placingZone->cellsAtBottom [xx][yy].dirLen = 0.0;
			placingZone->cellsAtBottom [xx][yy].perLen = 0.0;
			placingZone->cellsAtBottom [xx][yy].attached_side = BOTTOM_SIDE;
		}
	}
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	BeamTableformPlacingZone::alignPlacingZone (BeamTableformPlacingZone* placingZone)
{
	//short			xx, yy;
	//API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	//double			centerPos;		// 중심 위치
	//double			width_side;		// 측면 중심 유로폼 너비
	//double			width_bottom;	// 하부 중심 유로폼 너비
	//double			remainLength;	// 남은 길이를 계산하기 위한 임시 변수
	//double			xPos;			// 위치 커서
	//double			accumDist;		// 이동 거리
	//
	//double			height [4];
	//double			left [3];

	//
	//// 측면에서의 중심 위치 찾기
	//if (placingZone->bInterfereBeam == true)
	//	centerPos = placingZone->posInterfereBeamFromLeft;	// 간섭 보의 중심 위치
	//else
	//	centerPos = placingZone->beamLength / 2;			// 간섭 보가 없으면 중심을 기준으로 함

	//// 중심 유로폼 너비
	//if (placingZone->cellCenterAtRSide [0].objType != NONE)
	//	width_side = placingZone->cellCenterAtRSide [0].dirLen;
	//else
	//	width_side = placingZone->centerLengthAtSide;

	//if (placingZone->cellCenterAtBottom [0].objType != NONE)
	//	width_bottom = placingZone->cellCenterAtBottom [0].dirLen;
	//else
	//	width_bottom = 0.0;


	//// (1-1) 측면 시작 부분
	//// 중심부터 끝으로 이동해야 함
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//	if (placingZone->cellsFromBeginAtLSide [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//// 위치 선정
	//height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//height [1] = height [0] + placingZone->cellsFromBeginAtRSide [0][0].perLen;
	//height [2] = height [1] + placingZone->cellsFromBeginAtRSide [1][0].perLen;
	//height [3] = height [2] + placingZone->cellsFromBeginAtRSide [2][0].perLen;
	//for (xx = 0 ; xx < 4 ; ++xx) {
	//	xPos = centerPos - width_side/2 - accumDist;
	//	for (yy = 0 ; yy < placingZone->nCellsFromBeginAtSide ; ++yy) {
	//		if (placingZone->cellsFromBeginAtLSide [xx][yy].objType != NONE) {
	//			// 좌측
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ = height [xx];
	//	
	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// 우측
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ = height [xx];

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// 거리 이동
	//			xPos += placingZone->cellsFromBeginAtRSide [xx][yy].dirLen;
	//		}
	//	}
	//}

	//// (1-2) 측면 끝 부분
	//// 중심부터 끝으로 이동해야 함
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//	if (placingZone->cellsFromEndAtLSide [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//// 위치 선정
	//height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//height [1] = height [0] + placingZone->cellsFromEndAtRSide [0][0].perLen;
	//height [2] = height [1] + placingZone->cellsFromEndAtRSide [1][0].perLen;
	//height [3] = height [2] + placingZone->cellsFromEndAtRSide [2][0].perLen;
	//for (xx = 0 ; xx < 4 ; ++xx) {
	//	xPos = centerPos + width_side/2 + accumDist - placingZone->cellsFromEndAtLSide [0][0].dirLen;
	//	for (yy = 0 ; yy < placingZone->nCellsFromEndAtSide ; ++yy) {
	//		if (placingZone->cellsFromEndAtLSide [xx][yy].objType != NONE) {
	//			// 좌측
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ = height [xx];
	//		
	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// 우측
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ = height [xx];

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// 거리 이동
	//			if (yy < placingZone->nCellsFromEndAtSide-1)
	//				xPos -= placingZone->cellsFromEndAtRSide [xx][yy+1].dirLen;
	//		}
	//	}
	//}

	//// (1-3) 측면 중앙
	//// 위치 선정
	//xPos = centerPos - width_side/2;
	//height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//height [1] = height [0] + placingZone->cellCenterAtRSide [0].perLen;
	//height [2] = height [1] + placingZone->cellCenterAtRSide [1].perLen;
	//height [3] = height [2] + placingZone->cellCenterAtRSide [2].perLen;
	//for (xx = 0 ; xx < 4 ; ++xx) {
	//	// 좌측
	//	placingZone->cellCenterAtLSide [xx].leftBottomX = placingZone->begC.x + xPos;
	//	placingZone->cellCenterAtLSide [xx].leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	placingZone->cellCenterAtLSide [xx].leftBottomZ = height [xx];
	//	
	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = placingZone->cellCenterAtLSide [xx].leftBottomX;
	//	rotatedPoint.y = placingZone->cellCenterAtLSide [xx].leftBottomY;
	//	rotatedPoint.z = placingZone->cellCenterAtLSide [xx].leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	placingZone->cellCenterAtLSide [xx].leftBottomX = unrotatedPoint.x;
	//	placingZone->cellCenterAtLSide [xx].leftBottomY = unrotatedPoint.y;
	//	placingZone->cellCenterAtLSide [xx].leftBottomZ = unrotatedPoint.z;

	//	// 우측
	//	placingZone->cellCenterAtRSide [xx].leftBottomX = placingZone->begC.x + xPos;
	//	placingZone->cellCenterAtRSide [xx].leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	placingZone->cellCenterAtRSide [xx].leftBottomZ = height [xx];

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = placingZone->cellCenterAtRSide [xx].leftBottomX;
	//	rotatedPoint.y = placingZone->cellCenterAtRSide [xx].leftBottomY;
	//	rotatedPoint.z = placingZone->cellCenterAtRSide [xx].leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	placingZone->cellCenterAtRSide [xx].leftBottomX = unrotatedPoint.x;
	//	placingZone->cellCenterAtRSide [xx].leftBottomY = unrotatedPoint.y;
	//	placingZone->cellCenterAtRSide [xx].leftBottomZ = unrotatedPoint.z;
	//}

	//// (2-1) 하부 시작 부분
	//// 중심부터 끝으로 이동해야 함
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)
	//	if (placingZone->cellsFromBeginAtBottom [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;

	//// 위치 선정
	//left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	//left [1] = left [0] - placingZone->cellsFromBeginAtBottom [0][0].perLen;
	//left [2] = left [1] - placingZone->cellsFromBeginAtBottom [1][0].perLen;
	//for (xx = 0 ; xx < 3 ; ++xx) {
	//	xPos = centerPos - width_bottom/2 - accumDist;
	//	for (yy = 0 ; yy < placingZone->nCellsFromBeginAtBottom ; ++yy) {
	//		if (placingZone->cellsFromBeginAtBottom [xx][yy].objType != NONE) {
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY = left [xx] + infoBeam.offset;
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		
	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// 거리 이동
	//			xPos += placingZone->cellsFromBeginAtBottom [xx][yy].dirLen;
	//		}
	//	}
	//}

	//// (2-2) 하부 끝 부분
	//// 중심부터 끝으로 이동해야 함
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//	if (placingZone->cellsFromEndAtBottom [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//// 위치 선정
	//left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	//left [1] = left [0] - placingZone->cellsFromEndAtBottom [0][0].perLen;
	//left [2] = left [1] - placingZone->cellsFromEndAtBottom [1][0].perLen;
	//for (xx = 0 ; xx < 3 ; ++xx) {
	//	xPos = centerPos + width_bottom/2 + accumDist - placingZone->cellsFromEndAtBottom [0][0].dirLen;
	//	for (yy = 0 ; yy < placingZone->nCellsFromEndAtBottom ; ++yy) {
	//		if (placingZone->cellsFromEndAtBottom [xx][yy].objType != NONE) {
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY = left [xx] + infoBeam.offset;
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		
	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// 거리 이동
	//			if (yy < placingZone->nCellsFromEndAtBottom-1)
	//				xPos -= placingZone->cellsFromEndAtBottom [xx][yy+1].dirLen;
	//		}
	//	}
	//}

	//// (2-3) 하부 중앙
	//// 위치 선정
	//xPos = centerPos - width_bottom/2;
	//left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	//left [1] = left [0] - placingZone->cellCenterAtBottom [0].perLen;
	//left [2] = left [1] - placingZone->cellCenterAtBottom [1].perLen;
	//for (xx = 0 ; xx < 3 ; ++xx) {
	//	placingZone->cellCenterAtBottom [xx].leftBottomX = placingZone->begC.x + xPos;
	//	placingZone->cellCenterAtBottom [xx].leftBottomY = left [xx] + infoBeam.offset;
	//	placingZone->cellCenterAtBottom [xx].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	
	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = placingZone->cellCenterAtBottom [xx].leftBottomX;
	//	rotatedPoint.y = placingZone->cellCenterAtBottom [xx].leftBottomY;
	//	rotatedPoint.z = placingZone->cellCenterAtBottom [xx].leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	placingZone->cellCenterAtBottom [xx].leftBottomX = unrotatedPoint.x;
	//	placingZone->cellCenterAtBottom [xx].leftBottomY = unrotatedPoint.y;
	//	placingZone->cellCenterAtBottom [xx].leftBottomZ = unrotatedPoint.z;
	//}

	//// 여백 값 초기화 (측면 시작 부분 여백)
	//remainLength = centerPos - width_side/2;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx) {
	//	if (placingZone->cellsFromBeginAtRSide [0][xx].objType != NONE)
	//		remainLength -= placingZone->cellsFromBeginAtRSide [0][xx].dirLen;
	//}
	//placingZone->marginBeginAtSide = remainLength;

	//// 여백 값 초기화 (측면 끝 부분 여백)
	//remainLength = placingZone->beamLength - centerPos - width_side/2;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx) {
	//	if (placingZone->cellsFromEndAtRSide [0][xx].objType != NONE)
	//		remainLength -= placingZone->cellsFromEndAtRSide [0][xx].dirLen;
	//}
	//placingZone->marginEndAtSide = remainLength;

	//// 여백 값 초기화 (하부 시작 부분 여백)
	//remainLength = centerPos - width_bottom/2;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx) {
	//	if (placingZone->cellsFromBeginAtBottom [0][xx].objType != NONE)
	//		remainLength -= placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//}
	//placingZone->marginBeginAtBottom = remainLength;

	//// 여백 값 초기화 (하부 끝 부분 여백)
	//remainLength = placingZone->beamLength - centerPos - width_bottom/2;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx) {
	//	if (placingZone->cellsFromEndAtBottom [0][xx].objType != NONE)
	//		remainLength -= placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//}
	//placingZone->marginEndAtBottom = remainLength;
}

// 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
void	BeamTableformPlacingZone::addNewCol (BeamTableformPlacingZone* placingZone)
{
	//
}

// 마지막 열을 삭제함
void	BeamTableformPlacingZone::delLastCol (BeamTableformPlacingZone* placingZone)
{
	//
}

//// 해당 셀 정보를 기반으로 라이브러리 배치
//API_Guid	BeamTableformPlacingZone::placeLibPart (CellForBeamTableform objInfo)
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
//	double	validLength = 0.0;	// 유효한 길이인가?
//	double	validWidth = 0.0;	// 유효한 너비인가?
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
//	if (objInfo.objType == EUROFORM)		gsmName = L("유로폼v2.0.gsm");
//	if (objInfo.objType == PLYWOOD)			gsmName = L("합판v1.0.gsm");
//	if (objInfo.objType == WOOD)			gsmName = L("목재v1.0.gsm");
//	if (objInfo.objType == OUTCORNER_ANGLE)	gsmName = L("아웃코너앵글v1.0.gsm");
//	if (objInfo.objType == FILLERSPACER)	gsmName = L("휠러스페이서v1.0.gsm");
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
//	element.header.floorInd = infoBeam.floorInd;
//
//	if (objInfo.objType == EUROFORM) {
//		element.header.layer = layerInd_Euroform;
//
//		// 규격품일 경우,
//		if (objInfo.libPart.form.eu_stan_onoff == true) {
//			setParameterByName (&memo, "eu_stan_onoff", 1.0);	// 규격폼 On/Off
//
//			// 너비
//			sprintf (tempString, "%.0f", objInfo.libPart.form.eu_wid * 1000);
//			setParameterByName (&memo, "eu_wid", tempString);
//
//			// 높이
//			sprintf (tempString, "%.0f", objInfo.libPart.form.eu_hei * 1000);
//			setParameterByName (&memo, "eu_hei", tempString);
//
//		// 비규격품일 경우,
//		} else {
//			setParameterByName (&memo, "eu_stan_onoff", 0.0);	// 규격폼 On/Off
//			setParameterByName (&memo, "eu_wid2", objInfo.libPart.form.eu_wid2);	// 너비
//			setParameterByName (&memo, "eu_hei2", objInfo.libPart.form.eu_hei2);	// 높이
//		}
//
//		// 설치방향
//		if (objInfo.libPart.form.u_ins_wall == true) {
//			strcpy (tempString, "벽세우기");
//		} else {
//			strcpy (tempString, "벽눕히기");
//			if (objInfo.libPart.form.eu_stan_onoff == true) {
//				element.object.pos.x += ( objInfo.libPart.form.eu_hei * cos(objInfo.ang) );
//				element.object.pos.y += ( objInfo.libPart.form.eu_hei * sin(objInfo.ang) );
//				validLength = objInfo.libPart.form.eu_hei;
//				validWidth = objInfo.libPart.form.eu_wid;
//			} else {
//				element.object.pos.x += ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
//				element.object.pos.y += ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
//				validLength = objInfo.libPart.form.eu_hei2;
//				validWidth = objInfo.libPart.form.eu_wid2;
//			}
//		}
//		setParameterByName (&memo, "u_ins", tempString);
//
//		// 회전X
//		if (objInfo.attached_side == BOTTOM_SIDE) {
//			setParameterByName (&memo, "ang_x", DegreeToRad (0.0));
//		} else if (objInfo.attached_side == LEFT_SIDE) {
//			setParameterByName (&memo, "ang_x", DegreeToRad (90.0));
//			if (objInfo.libPart.form.eu_stan_onoff == true) {
//				element.object.pos.x -= ( objInfo.libPart.form.eu_hei * cos(objInfo.ang) );
//				element.object.pos.y -= ( objInfo.libPart.form.eu_hei * sin(objInfo.ang) );
//			} else {
//				element.object.pos.x -= ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
//				element.object.pos.y -= ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
//			}
//			element.object.angle += DegreeToRad (180.0);
//		} else {
//			setParameterByName (&memo, "ang_x", DegreeToRad (90.0));
//		}
//
//	} else if (objInfo.objType == FILLERSPACER) {
//		element.header.layer = layerInd_Fillerspacer;
//		setParameterByName (&memo, "f_thk", objInfo.libPart.fillersp.f_thk);	// 두께
//		setParameterByName (&memo, "f_leng", objInfo.libPart.fillersp.f_leng);	// 길이
//		setParameterByName (&memo, "f_ang", 0.0);								// 각도
//
//		// 회전
//		if (objInfo.attached_side == BOTTOM_SIDE) {
//			setParameterByName (&memo, "f_rota", DegreeToRad (90.0));
//		} else if (objInfo.attached_side == LEFT_SIDE) {
//			setParameterByName (&memo, "f_rota", DegreeToRad (0.0));
//			element.object.pos.x += ( objInfo.libPart.fillersp.f_leng * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.libPart.fillersp.f_leng * sin(objInfo.ang) );
//			element.object.angle += DegreeToRad (180.0);
//		} else {
//			setParameterByName (&memo, "f_rota", DegreeToRad (0.0));
//		}
//
//		validLength = objInfo.libPart.fillersp.f_leng;
//		validWidth = objInfo.libPart.fillersp.f_thk;
//
//	} else if (objInfo.objType == PLYWOOD) {
//		element.header.layer = layerInd_Plywood;
//		setParameterByName (&memo, "p_stan", "비규격");							// 규격
//		setParameterByName (&memo, "w_dir", "벽눕히기");						// 설치방향
//		setParameterByName (&memo, "p_thk", "11.5T");							// 두께
//		setParameterByName (&memo, "p_wid", objInfo.libPart.plywood.p_wid);		// 가로
//		setParameterByName (&memo, "p_leng", objInfo.libPart.plywood.p_leng);	// 세로
//		setParameterByName (&memo, "sogak", 1.0);								// 제작틀 ON
//		setParameterByName (&memo, "bInverseSogak", 1.0);						// 목재 조립방향 반전
//		setParameterByName (&memo, "gap_a", 0.0);
//		setParameterByName (&memo, "gap_b", 0.0);
//		setParameterByName (&memo, "gap_c", 0.0);
//		setParameterByName (&memo, "gap_d", 0.0);
//		
//		// 각도
//		if (objInfo.attached_side == BOTTOM_SIDE) {
//			setParameterByName (&memo, "p_ang", DegreeToRad (90.0));
//		} else if (objInfo.attached_side == LEFT_SIDE) {
//			setParameterByName (&memo, "p_ang", DegreeToRad (0.0));
//			element.object.pos.x += ( objInfo.libPart.plywood.p_leng * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.libPart.plywood.p_leng * sin(objInfo.ang) );
//			element.object.angle += DegreeToRad (180.0);
//		}
//
//		validLength = objInfo.libPart.plywood.p_leng;
//		validWidth = objInfo.libPart.plywood.p_wid;
//
//	} else if (objInfo.objType == WOOD) {
//		element.header.layer = layerInd_Timber;
//
//		setParameterByName (&memo, "w_w", objInfo.libPart.wood.w_w);		// 두께
//		setParameterByName (&memo, "w_h", objInfo.libPart.wood.w_h);		// 너비
//		setParameterByName (&memo, "w_leng", objInfo.libPart.wood.w_leng);	// 길이
//		setParameterByName (&memo, "w_ang", objInfo.libPart.wood.w_ang);	// 각도
//	
//		if (objInfo.attached_side == BOTTOM_SIDE) {
//			setParameterByName (&memo, "w_ins", "바닥눕히기");				// 설치방향
//		} else if (objInfo.attached_side == LEFT_SIDE) {
//			setParameterByName (&memo, "w_ins", "벽세우기");				// 설치방향
//			element.object.pos.x += ( objInfo.libPart.wood.w_leng * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.libPart.wood.w_leng * sin(objInfo.ang) );
//			element.object.angle += DegreeToRad (180.0);
//		} else {
//			setParameterByName (&memo, "w_ins", "벽세우기");				// 설치방향
//		}
//
//		validLength = objInfo.libPart.wood.w_leng;
//		validWidth = objInfo.libPart.wood.w_h;
//
//	} else if (objInfo.objType == OUTCORNER_ANGLE) {
//		element.header.layer = layerInd_OutcornerAngle;
//		setParameterByName (&memo, "a_leng", objInfo.libPart.outangle.a_leng);	// 길이
//		setParameterByName (&memo, "a_ang", 0.0);								// 각도
//
//		if (objInfo.attached_side == RIGHT_SIDE) {
//			element.object.pos.x += ( objInfo.libPart.outangle.a_leng * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.libPart.outangle.a_leng * sin(objInfo.ang) );
//			element.object.angle += DegreeToRad (180.0);
//		}
//
//		validLength = objInfo.libPart.outangle.a_leng;
//		validWidth = 0.064;
//	}
//
//	// 객체 배치
//	if ((objInfo.objType != NONE) && (validLength > EPS) && (validWidth > EPS))
//		ACAPI_Element_Create (&element, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return element.header.guid;
//}
//
//// 라이브러리 배치: 유로폼 후크
//API_Guid	BeamTableformPlacingZone::placeLibPart (EuroformHook params)
//{
//	GSErrCode	err = NoError;
//	API_Element			elem;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const GS::uchar_t*	gsmName = L("유로폼 후크.gsm");
//	double				aParam;
//	double				bParam;
//	Int32				addParNum;
//
//	// 객체 로드
//	BNZeroMemory (&elem, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return elem.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	elem.header.typeID = API_ObjectID;
//	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&elem, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// 라이브러리의 파라미터 값 입력
//	elem.object.libInd = libPart.index;
//	elem.object.reflected = false;
//	elem.object.pos.x = params.leftBottomX;
//	elem.object.pos.y = params.leftBottomY;
//	elem.object.level = params.leftBottomZ;
//	elem.object.xRatio = aParam;
//	elem.object.yRatio = bParam;
//	elem.object.angle = params.ang + DegreeToRad (180.0);
//	elem.header.floorInd = infoBeam.floorInd;
//
//	// 레이어
//	elem.header.layer = layerInd_EuroformHook;
//
//	setParameterByName (&memo, "rotationX", params.angX);			// X축 회전
//	setParameterByName (&memo, "rotationY", params.angY);			// Y축 회전
//	setParameterByName (&memo, "iHookType", params.iHookType);		// (1)수직-대, (2)수평-소
//	setParameterByName (&memo, "iHookShape", params.iHookShape);	// (1)원형, (2)사각
//
//	// 객체 배치
//	ACAPI_Element_Create (&elem, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return	elem.header.guid;
//}
//
//// 라이브러리 배치: 각파이프행거
//API_Guid	BeamTableformPlacingZone::placeLibPart (RectPipeHanger params)
//{
//	GSErrCode	err = NoError;
//	API_Element			elem;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const GS::uchar_t*	gsmName = L("각파이프행거.gsm");
//	double				aParam;
//	double				bParam;
//	Int32				addParNum;
//
//	// 객체 로드
//	BNZeroMemory (&elem, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return elem.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	elem.header.typeID = API_ObjectID;
//	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&elem, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// 라이브러리의 파라미터 값 입력
//	elem.object.libInd = libPart.index;
//	elem.object.reflected = false;
//	elem.object.pos.x = params.leftBottomX;
//	elem.object.pos.y = params.leftBottomY;
//	elem.object.level = params.leftBottomZ;
//	elem.object.xRatio = aParam;
//	elem.object.yRatio = bParam;
//	elem.object.angle = params.ang - DegreeToRad (90);
//	elem.header.floorInd = infoBeam.floorInd;
//
//	// 레이어
//	elem.header.layer = layerInd_RectpipeHanger;
//
//	setParameterByName (&memo, "m_type", "각파이프행거");	// 품명
//	setParameterByName (&memo, "angX", params.angX);		// 회전X
//	setParameterByName (&memo, "angY", params.angY);		// 회전Y
//
//	// 객체 배치
//	ACAPI_Element_Create (&elem, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return	elem.header.guid;
//}
//
//// 라이브러리 배치: 비계파이프
//API_Guid	BeamTableformPlacingZone::placeLibPart (SquarePipe params)
//{
//	GSErrCode	err = NoError;
//	API_Element			elem;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const GS::uchar_t*	gsmName = L("비계파이프v1.0.gsm");
//	double				aParam;
//	double				bParam;
//	Int32				addParNum;
//
//	// 객체 로드
//	BNZeroMemory (&elem, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return elem.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	elem.header.typeID = API_ObjectID;
//	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&elem, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// 라이브러리의 파라미터 값 입력
//	elem.object.libInd = libPart.index;
//	elem.object.reflected = false;
//	elem.object.pos.x = params.leftBottomX;
//	elem.object.pos.y = params.leftBottomY;
//	elem.object.level = params.leftBottomZ;
//	elem.object.xRatio = aParam;
//	elem.object.yRatio = bParam;
//	elem.object.angle = params.ang;
//	elem.header.floorInd = infoBeam.floorInd;
//
//	// 레이어
//	elem.header.layer = layerInd_Rectpipe;
//
//	setParameterByName (&memo, "p_comp", "사각파이프");		// 사각파이프
//	setParameterByName (&memo, "p_leng", params.length);	// 길이
//	setParameterByName (&memo, "p_ang", params.pipeAng);	// 각도
//
//	// 객체 배치
//	ACAPI_Element_Create (&elem, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return	elem.header.guid;
//}
//
//// 라이브러리 배치: 블루목심
//API_Guid	BeamTableformPlacingZone::placeLibPart (BlueTimberRail params)
//{
//	GSErrCode	err = NoError;
//	API_Element			elem;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const GS::uchar_t*	gsmName = L("블루목심v1.0.gsm");
//	double				aParam;
//	double				bParam;
//	Int32				addParNum;
//
//	// 객체 로드
//	BNZeroMemory (&elem, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return elem.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	elem.header.typeID = API_ObjectID;
//	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&elem, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// 라이브러리의 파라미터 값 입력
//	elem.object.libInd = libPart.index;
//	elem.object.reflected = false;
//	elem.object.pos.x = params.leftBottomX;
//	elem.object.pos.y = params.leftBottomY;
//	elem.object.level = params.leftBottomZ;
//	elem.object.xRatio = aParam;
//	elem.object.yRatio = bParam;
//	elem.object.angle = params.ang;
//	elem.header.floorInd = infoBeam.floorInd;
//
//	// 레이어
//	elem.header.layer = layerInd_TimberRail;
//
//	setParameterByName (&memo, "railType", params.railType);	// 규격
//	setParameterByName (&memo, "angX", params.angX);			// 회전X
//	setParameterByName (&memo, "angY", params.angY);			// 회전Y
//
//	// 객체 배치
//	ACAPI_Element_Create (&elem, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return	elem.header.guid;
//}

// 유로폼/휠러/각재를 채운 후 자투리 공간 채우기 (나머지 합판/각재 및 아웃코너앵글 + 비계파이프, 각파이프행거, 핀볼트, 유로폼 후크, 블루클램프, 블루목심)
GSErrCode	BeamTableformPlacingZone::fillRestAreas (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	//short	xx;
	//double	centerPos;		// 중심 위치
	//double	width_side;		// 측면 중심 유로폼 너비
	//double	width_bottom;	// 하부 중심 유로폼 너비
	//double	xPos;			// 위치 커서
	//double	accumDist;		// 이동 거리
	//double	length_outa;	// 아웃코너앵글 길이 계산
	//double	length_pipe;	// 비계파이프 길이 계산

	//double	cellWidth_side;
	//double	cellHeight_side, cellHeight_bottom;
	//CellForBeamTableform	insCell;
	//API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	//SquarePipe		squarePipe;		// 비계파이프
	//EuroformHook	hook;			// 유로폼 후크
	//RectPipeHanger	hanger;			// 각파이프 행거
	//BlueTimberRail	timberRail;		// 블루목심


	//// 측면에서의 중심 위치 찾기
	//if (placingZone->bInterfereBeam == true)
	//	centerPos = placingZone->posInterfereBeamFromLeft;	// 간섭 보의 중심 위치
	//else
	//	centerPos = placingZone->beamLength / 2;			// 간섭 보가 없으면 중심을 기준으로 함

	//// 중심 유로폼 너비
	//if (placingZone->cellCenterAtRSide [0].objType != NONE)
	//	width_side = placingZone->cellCenterAtRSide [0].dirLen;
	//else
	//	width_side = placingZone->centerLengthAtSide;

	//if (placingZone->cellCenterAtBottom [0].objType != NONE)
	//	width_bottom = placingZone->cellCenterAtBottom [0].dirLen;
	//else
	//	width_bottom = 0.0;

	//// 중심 여백 너비 (중심 유로폼이 없을 경우에만 사용함)
	//if (placingZone->bInterfereBeam == true)
	//	cellWidth_side = (placingZone->centerLengthAtSide - placingZone->interfereBeamWidth) / 2;
	//else
	//	cellWidth_side = placingZone->centerLengthAtSide;

	//// 측면 합판/목재 높이
	//cellHeight_side = placingZone->cellsFromBeginAtRSide [0][0].perLen + placingZone->cellsFromBeginAtRSide [1][0].perLen + placingZone->cellsFromBeginAtRSide [2][0].perLen + placingZone->cellsFromBeginAtRSide [3][0].perLen;
	//cellHeight_bottom = placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen;


	//// 측면 중앙 셀이 NONE일 경우
	//if (placingZone->cellCenterAtRSide [0].objType == NONE) {
	//	// 너비가 110 미만이면 목재, 110 이상이면 합판
	//	if (placingZone->bInterfereBeam == true) {
	//		// 좌측 1/2번째
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.dirLen = cellWidth_side;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + centerPos - placingZone->centerLengthAtSide/2;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (cellWidth_side < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = cellWidth_side;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX -= insCell.libPart.wood.w_leng;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = cellWidth_side;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// 좌측 2/2번째
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.dirLen = cellWidth_side;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + centerPos + placingZone->centerLengthAtSide/2;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (cellWidth_side < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = cellWidth_side;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX -= (insCell.libPart.wood.w_leng + insCell.libPart.wood.w_h);
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = cellWidth_side;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//			insCell.leftBottomX -= insCell.libPart.plywood.p_leng;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// 우측 1/2번째
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.dirLen = cellWidth_side;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + centerPos - placingZone->centerLengthAtSide/2;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (cellWidth_side < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = cellWidth_side;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX += insCell.libPart.wood.w_h;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = cellWidth_side;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// 우측 2/2번째
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.dirLen = cellWidth_side;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + centerPos + placingZone->centerLengthAtSide/2;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (cellWidth_side < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = cellWidth_side;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = cellWidth_side;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//			insCell.leftBottomX -= insCell.libPart.plywood.p_leng;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// 측면 시작 부분 여백 채움
	//if (placingZone->bFillMarginBeginAtSide == true) {
	//	if (placingZone->marginBeginAtSide > EPS) {
	//		// 좌측
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.dirLen = placingZone->marginBeginAtSide;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginBeginAtSide < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = placingZone->marginBeginAtSide;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX -= cellHeight_side;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginBeginAtSide;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// 우측
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.dirLen = placingZone->marginBeginAtSide;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginBeginAtSide < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = placingZone->marginBeginAtSide;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX += placingZone->marginBeginAtSide;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginBeginAtSide;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// 측면 끝 부분 여백 채움
	//if (placingZone->bFillMarginEndAtSide == true) {
	//	if (placingZone->marginEndAtSide > EPS) {
	//		// 좌측
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.dirLen = placingZone->marginEndAtSide;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + placingZone->beamLength - placingZone->marginEndAtSide;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginBeginAtSide < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = placingZone->marginEndAtSide;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX -= cellHeight_side;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginEndAtSide;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// 우측
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.dirLen = placingZone->marginEndAtSide;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + placingZone->beamLength - placingZone->marginEndAtSide;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginEndAtSide < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = placingZone->marginEndAtSide;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX += placingZone->marginEndAtSide;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginEndAtSide;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// 하부 시작 부분 여백 채움
	//if (placingZone->bFillMarginBeginAtBottom == true) {
	//	if (placingZone->marginBeginAtBottom > EPS) {
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = BOTTOM_SIDE;
	//		insCell.dirLen = placingZone->marginBeginAtBottom;
	//		insCell.perLen = cellHeight_bottom;
	//		insCell.leftBottomX = placingZone->begC.x;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginBeginAtBottom < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = 0.0;
	//			insCell.libPart.wood.w_h = placingZone->marginBeginAtBottom;
	//			insCell.libPart.wood.w_leng = cellHeight_bottom;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.ang -= DegreeToRad (90.0);
	//			insCell.leftBottomX += placingZone->marginBeginAtBottom;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginBeginAtBottom;
	//			insCell.libPart.plywood.p_wid = cellHeight_bottom;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}
	//
	//// 하부 끝 부분 여백 채움
	//if (placingZone->bFillMarginEndAtBottom == true) {
	//	if (placingZone->marginEndAtBottom > EPS) {
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = BOTTOM_SIDE;
	//		insCell.dirLen = placingZone->marginEndAtBottom;
	//		insCell.perLen = cellHeight_bottom;
	//		insCell.leftBottomX = placingZone->begC.x + placingZone->beamLength - placingZone->marginEndAtBottom;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginEndAtBottom < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = 0.0;
	//			insCell.libPart.wood.w_h = placingZone->marginEndAtBottom;
	//			insCell.libPart.wood.w_leng = cellHeight_bottom;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.ang -= DegreeToRad (90.0);
	//			insCell.leftBottomX += placingZone->marginEndAtBottom;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginEndAtBottom;
	//			insCell.libPart.plywood.p_wid = cellHeight_bottom;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// 중심부터 끝으로 이동해야 함
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//	if (placingZone->cellsFromBeginAtRSide [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromBeginAtRSide [0][xx].dirLen;

	//// 아웃코너앵글 설치 (측면 시작 부분)
	//length_outa = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx) {
	//	if (placingZone->cellsFromBeginAtLSide [0][xx].objType != NONE) {
	//		length_outa += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//}

	//xPos = centerPos - width_side/2 - accumDist;
	//while (length_outa > EPS) {
	//	// 좌측
	//	insCell.objType = OUTCORNER_ANGLE;
	//	insCell.ang = placingZone->ang;
	//	insCell.attached_side = LEFT_SIDE;
	//	insCell.leftBottomX = placingZone->begC.x + xPos;
	//	insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (length_outa > 2.400)
	//		insCell.libPart.outangle.a_leng = 2.400;
	//	else
	//		insCell.libPart.outangle.a_leng = length_outa;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = insCell.leftBottomX;
	//	rotatedPoint.y = insCell.leftBottomY;
	//	rotatedPoint.z = insCell.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	insCell.leftBottomX = unrotatedPoint.x;
	//	insCell.leftBottomY = unrotatedPoint.y;
	//	insCell.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placingZone->placeLibPart (insCell));

	//	// 우측
	//	insCell.objType = OUTCORNER_ANGLE;
	//	insCell.ang = placingZone->ang;
	//	insCell.attached_side = RIGHT_SIDE;
	//	insCell.leftBottomX = placingZone->begC.x + xPos;
	//	insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (length_outa > 2.400)
	//		insCell.libPart.outangle.a_leng = 2.400;
	//	else
	//		insCell.libPart.outangle.a_leng = length_outa;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = insCell.leftBottomX;
	//	rotatedPoint.y = insCell.leftBottomY;
	//	rotatedPoint.z = insCell.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	insCell.leftBottomX = unrotatedPoint.x;
	//	insCell.leftBottomY = unrotatedPoint.y;
	//	insCell.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placingZone->placeLibPart (insCell));

	//	// 거리 이동
	//	xPos += insCell.libPart.outangle.a_leng;

	//	// 남은 거리 감소
	//	if (length_outa > 2.400)
	//		length_outa -= 2.400;
	//	else
	//		length_outa = 0.0;
	//}

	//// 중심부터 끝으로 이동해야 함
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//	if (placingZone->cellsFromEndAtRSide [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromEndAtRSide [0][xx].dirLen;

	//// 아웃코너앵글 설치 (측면 끝 부분)
	//length_outa = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx) {
	//	if (placingZone->cellsFromEndAtLSide [0][xx].objType != NONE) {
	//		length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	}
	//}

	//xPos = centerPos + width_side/2;
	//while (length_outa > EPS) {
	//	// 좌측
	//	insCell.objType = OUTCORNER_ANGLE;
	//	insCell.ang = placingZone->ang;
	//	insCell.attached_side = LEFT_SIDE;
	//	insCell.leftBottomX = placingZone->begC.x + xPos;
	//	insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (length_outa > 2.400)
	//		insCell.libPart.outangle.a_leng = 2.400;
	//	else
	//		insCell.libPart.outangle.a_leng = length_outa;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = insCell.leftBottomX;
	//	rotatedPoint.y = insCell.leftBottomY;
	//	rotatedPoint.z = insCell.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	insCell.leftBottomX = unrotatedPoint.x;
	//	insCell.leftBottomY = unrotatedPoint.y;
	//	insCell.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placingZone->placeLibPart (insCell));

	//	// 우측
	//	insCell.objType = OUTCORNER_ANGLE;
	//	insCell.ang = placingZone->ang;
	//	insCell.attached_side = RIGHT_SIDE;
	//	insCell.leftBottomX = placingZone->begC.x + xPos;
	//	insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (length_outa > 2.400)
	//		insCell.libPart.outangle.a_leng = 2.400;
	//	else
	//		insCell.libPart.outangle.a_leng = length_outa;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = insCell.leftBottomX;
	//	rotatedPoint.y = insCell.leftBottomY;
	//	rotatedPoint.z = insCell.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	insCell.leftBottomX = unrotatedPoint.x;
	//	insCell.leftBottomY = unrotatedPoint.y;
	//	insCell.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placingZone->placeLibPart (insCell));

	//	// 거리 이동
	//	xPos += insCell.libPart.outangle.a_leng;

	//	// 남은 거리 감소
	//	if (length_outa > 2.400)
	//		length_outa -= 2.400;
	//	else
	//		length_outa = 0.0;
	//}

	//// 아웃코너 앵글 설치 (중앙 부분)
	//xPos = centerPos - width_side/2;
	//if (placingZone->bInterfereBeam == false) {
	//	if (placingZone->cellCenterAtRSide [0].objType == EUROFORM) {
	//		// 좌측
	//		insCell.objType = OUTCORNER_ANGLE;
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.leftBottomX = placingZone->begC.x + xPos;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		insCell.libPart.outangle.a_leng = placingZone->cellCenterAtLSide [0].dirLen;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// 우측
	//		insCell.objType = OUTCORNER_ANGLE;
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.leftBottomX = placingZone->begC.x + xPos;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		insCell.libPart.outangle.a_leng = placingZone->cellCenterAtRSide [0].dirLen;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// 비계파이프 1단 (측면 시작 부분 - 왼쪽), 만약 센터 여백이 없으면 (측면 끝 부분 - 왼쪽)까지
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1행

	//		if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//			if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2행
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 2단 (측면 시작 부분 - 왼쪽), 만약 센터 여백이 없으면 (측면 끝 부분 - 왼쪽)까지
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1행

	//		if (placingZone->cellsFromBeginAtLSide [2][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//			if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2행
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 1단 (측면 시작 부분 - 오른쪽), 만약 센터 여백이 없으면 (측면 끝 부분 - 오른쪽)까지
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1행

	//		if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//			if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2행
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 2단 (측면 시작 부분 - 오른쪽), 만약 센터 여백이 없으면 (측면 끝 부분 - 오른쪽)까지
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1행

	//		if (placingZone->cellsFromBeginAtLSide [2][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//			if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2행
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 1단 (측면 끝 부분 - 왼쪽), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromEndAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//if ((placingZone->cellsFromEndAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1행

	//		if (placingZone->cellsFromEndAtLSide [0][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//			if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2행
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 2단 (측면 끝 부분 - 왼쪽), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromEndAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//if ((placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1행

	//		if (placingZone->cellsFromEndAtLSide [2][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//			if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2행
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 1단 (측면 끝 부분 - 오른쪽), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromEndAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//if ((placingZone->cellsFromEndAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1행

	//		if (placingZone->cellsFromEndAtLSide [0][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//			squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//			if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2행
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 2단 (측면 끝 부분 - 오른쪽), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromEndAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//if ((placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1행

	//		if (placingZone->cellsFromEndAtLSide [2][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//			squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//			if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2행
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 좌측 (하부 시작 부분), 만약 센터 여백이 없으면 (좌측 - 하부 끝 부분)까지
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtBottom [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtBottom [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 우측 (하부 시작 부분), 만약 센터 여백이 없으면 (우측 - 하부 끝 부분)까지
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtBottom [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtBottom [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 중앙 (하부 시작 부분), 만약 센터 여백이 없으면 (중앙 - 하부 끝 부분)까지
	//if (placingZone->cellsFromBeginAtBottom [2][0].perLen > EPS) {
	//	xPos = 0.0;
	//	length_pipe = 0.100;
	//	for (xx = 0 ; xx < nCellsFromBeginAtBottom ; ++xx)
	//		length_pipe += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//	if (abs (placingZone->cellCenterAtBottom [0].dirLen) < EPS) {
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//			length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//	}

	//	if ((placingZone->cellsFromBeginAtBottom [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0)) {
	//		if (round (placingZone->cellsFromBeginAtBottom [0][0].perLen, 3) > round (placingZone->cellsFromBeginAtBottom [2][0].perLen, 3)) {
	//			if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			accumDist = -0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		accumDist = -0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		accumDist = -0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		accumDist = -0.250 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		accumDist = -0.150 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		accumDist = -0.150 + 0.030;
	//		} else {
	//			if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			accumDist = 0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		accumDist = 0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		accumDist = 0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		accumDist = 0.250 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		accumDist = 0.150 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		accumDist = 0.150 + 0.030;
	//			
	//			accumDist -= (placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen);
	//		}

	//		while (length_pipe > 0.0) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + accumDist;
	//			squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//			squarePipe.ang = placingZone->ang;
	//			if (length_pipe > 6.000)
	//				squarePipe.length = 6.000;
	//			else
	//				squarePipe.length = length_pipe;
	//			squarePipe.pipeAng = DegreeToRad (0.0);

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));

	//			xPos += squarePipe.length;

	//			if (length_pipe > 6.000)
	//				length_pipe -= 6.000;
	//			else
	//				length_pipe = 0.0;
	//		}
	//	}
	//}

	//// 비계파이프 좌측 (하부 끝 부분), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//if ((placingZone->cellsFromBeginAtBottom [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0) && (abs (placingZone->cellCenterAtBottom [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtBottom [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 우측 (하부 끝 부분), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//if ((placingZone->cellsFromBeginAtBottom [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0) && (abs (placingZone->cellCenterAtBottom [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtBottom [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 비계파이프 중앙 (하부 끝 부분), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//if ((placingZone->cellsFromBeginAtBottom [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0) && (abs (placingZone->cellCenterAtBottom [0].dirLen) > EPS)) {
	//	if (round (placingZone->cellsFromBeginAtBottom [0][0].perLen, 3) > round (placingZone->cellsFromBeginAtBottom [2][0].perLen, 3)) {
	//		if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			accumDist = -0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		accumDist = -0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		accumDist = -0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		accumDist = -0.250 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		accumDist = -0.150 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		accumDist = -0.150 + 0.030;
	//	} else {
	//		if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			accumDist = 0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		accumDist = 0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		accumDist = 0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		accumDist = 0.250 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		accumDist = 0.150 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		accumDist = 0.150 + 0.030;
	//			
	//		accumDist -= (placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen);
	//	}

	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtBottom [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + accumDist;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// 유로폼 후크 - 비계파이프 1단 (측면 시작 부분 - 왼쪽), 만약 센터 여백이 없으면 (측면 끝 부분 - 왼쪽)까지
	//hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//hook.ang = placingZone->ang;
	//hook.angX = DegreeToRad (180.0);
	//hook.angY = DegreeToRad (270.0);
	//hook.iHookType = 2;
	//hook.iHookShape = 2;

	//axisPoint.x = placingZone->begC.x;
	//axisPoint.y = placingZone->begC.y;
	//axisPoint.z = placingZone->begC.z;

	//rotatedPoint.x = hook.leftBottomX;
	//rotatedPoint.y = hook.leftBottomY;
	//rotatedPoint.z = hook.leftBottomZ;
	//unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//hook.leftBottomX = unrotatedPoint.x;
	//hook.leftBottomY = unrotatedPoint.y;
	//hook.leftBottomZ = unrotatedPoint.z;

	//elemList.Push (placeLibPart (hook));

	//accumDist = 0.0;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//} else {
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//}
	//moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//elemList.Push (placeLibPart (hook));

	//if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		hook.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));
	//}

	//// 유로폼 후크 - 비계파이프 2단 (측면 시작 부분 - 왼쪽), 만약 센터 여백이 없으면 (측면 끝 부분 - 왼쪽)까지
	//if ((placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen;
	//	if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));

	//	if (placingZone->cellsFromBeginAtLSide [2][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//		hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (hook));

	//		accumDist = 0.0;
	//		if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//			for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//			for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//		} else {
	//			for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		}
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		elemList.Push (placeLibPart (hook));
	//	}
	//}

	//// 유로폼 후크 - 비계파이프 1단 (측면 시작 부분 - 오른쪽), 만약 센터 여백이 없으면 (측면 끝 부분 - 오른쪽)까지
	//hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//hook.angX = DegreeToRad (180.0);
	//hook.angY = DegreeToRad (270.0);
	//hook.iHookType = 2;
	//hook.iHookShape = 2;

	//axisPoint.x = placingZone->begC.x;
	//axisPoint.y = placingZone->begC.y;
	//axisPoint.z = placingZone->begC.z;

	//rotatedPoint.x = hook.leftBottomX;
	//rotatedPoint.y = hook.leftBottomY;
	//rotatedPoint.z = hook.leftBottomZ;
	//unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//hook.leftBottomX = unrotatedPoint.x;
	//hook.leftBottomY = unrotatedPoint.y;
	//hook.leftBottomZ = unrotatedPoint.z;

	//hook.ang = placingZone->ang + DegreeToRad (180.0);
	//elemList.Push (placeLibPart (hook));
	//hook.ang = placingZone->ang;

	//accumDist = 0.0;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//} else {
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//}
	//moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//hook.ang = placingZone->ang + DegreeToRad (180.0);
	//elemList.Push (placeLibPart (hook));
	//hook.ang = placingZone->ang;

	//if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		hook.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;
	//}

	//// 유로폼 후크 - 비계파이프 2단 (측면 시작 부분 - 오른쪽), 만약 센터 여백이 없으면 (측면 끝 부분 - 오른쪽)까지
	//if ((placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen;
	//	if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	if (placingZone->cellsFromBeginAtLSide [2][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//		hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;

	//		accumDist = 0.0;
	//		if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//			for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//			for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//		} else {
	//			for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		}
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;
	//	}
	//}

	//// 유로폼 후크 - 비계파이프 1단 (측면 끝 부분 - 왼쪽), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//if ((placingZone->cellsFromEndAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));

	//	if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//		hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (hook));

	//		accumDist = 0.0;
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		elemList.Push (placeLibPart (hook));
	//	}
	//}

	//// 유로폼 후크 - 비계파이프 2단 (측면 끝 부분 - 왼쪽), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//if ((placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen;
	//	if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));

	//	if (placingZone->cellsFromEndAtLSide [2][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//		hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (hook));

	//		accumDist = 0.0;
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		elemList.Push (placeLibPart (hook));
	//	}
	//}

	//// 유로폼 후크 - 비계파이프 1단 (측면 끝 부분 - 오른쪽), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//if ((placingZone->cellsFromEndAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//		hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;

	//		accumDist = 0.0;
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;
	//	}
	//}

	//// 유로폼 후크 - 비계파이프 2단 (측면 끝 부분 - 오른쪽), 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//if ((placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen;
	//	if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	if (placingZone->cellsFromEndAtLSide [2][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//		hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;

	//		accumDist = 0.0;
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;
	//	}
	//}

	//// 유로폼 후크 - 비계파이프 중앙 (하부 시작 부분), 만약 센터 여백이 없으면 (중앙 - 하부 끝 부분)까지
	//if (round (placingZone->cellsFromBeginAtBottom [0][0].perLen, 3) > round (placingZone->cellsFromBeginAtBottom [2][0].perLen, 3)) {
	//	if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		accumDist = -0.250 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		accumDist = -0.150 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		accumDist = -0.150 + 0.030;
	//} else {
	//	if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		accumDist = 0.250 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		accumDist = 0.150 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		accumDist = 0.150 + 0.030;
	//			
	//	accumDist -= (placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen);
	//}

	//if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen) > EPS) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + placingZone->cellsFromBeginAtBottom [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + accumDist;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (270.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtBottom [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)		accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)		accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtBottom-1 ; ++xx)	accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));
	//}

	//// 유로폼 후크 - 비계파이프 중앙 (하부 끝 부분), 만약 센터 여백이 없으면 제외
	//if (round (placingZone->cellsFromBeginAtBottom [0][0].perLen, 3) > round (placingZone->cellsFromBeginAtBottom [2][0].perLen, 3)) {
	//	if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		accumDist = -0.250 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		accumDist = -0.150 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		accumDist = -0.150 + 0.030;
	//} else {
	//	if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		accumDist = 0.250 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		accumDist = 0.150 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		accumDist = 0.150 + 0.030;
	//			
	//	accumDist -= (placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen);
	//}

	//xPos = centerPos;
	//if ((abs (placingZone->cellsFromBeginAtBottom [2][0].perLen) > EPS) && (abs (placingZone->cellCenterAtBottom [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtBottom [0].dirLen / 2) + placingZone->cellsFromEndAtBottom [0][placingZone->nCellsFromEndAtBottom-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + accumDist;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (270.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtBottom-1 ; ++xx)	accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));
	//}

	//// 각파이프 행거 - 비계파이프 좌측 시작 부분, 만약 센터 여백이 없으면 (좌측 - 끝 부분)까지
	//if (placingZone->nCellsFromBeginAtSide > 0) {
	//	hanger.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.150;
	//	hanger.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	hanger.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0635;
	//	hanger.ang = placingZone->ang;
	//	hanger.angX = DegreeToRad (0.0);
	//	hanger.angY = DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hanger.leftBottomX;
	//	rotatedPoint.y = hanger.leftBottomY;
	//	rotatedPoint.z = hanger.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hanger.leftBottomX = unrotatedPoint.x;
	//	hanger.leftBottomY = unrotatedPoint.y;
	//	hanger.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hanger));

	//	length_outa = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		length_outa += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	if (placingZone->cellCenterAtLSide [0].dirLen < EPS) {
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	}

	//	moveIn3D ('x', hanger.ang, length_outa - 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	elemList.Push (placeLibPart (hanger));

	//	accumDist = 0.150 - (round (length_outa / 0.300, 3) * 0.150);
	//	moveIn3D ('x', hanger.ang, accumDist, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	elemList.Push (placeLibPart (hanger));
	//}

	//// 각파이프 행거 - 비계파이프 우측 시작 부분, 만약 센터 여백이 없으면 (우측 - 끝 부분)까지
	//if (placingZone->nCellsFromBeginAtSide > 0) {
	//	hanger.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.150;
	//	hanger.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	hanger.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0635;
	//	hanger.ang = placingZone->ang;
	//	hanger.angX = DegreeToRad (0.0);
	//	hanger.angY = DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hanger.leftBottomX;
	//	rotatedPoint.y = hanger.leftBottomY;
	//	rotatedPoint.z = hanger.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hanger.leftBottomX = unrotatedPoint.x;
	//	hanger.leftBottomY = unrotatedPoint.y;
	//	hanger.leftBottomZ = unrotatedPoint.z;

	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;

	//	length_outa = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		length_outa += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	if (placingZone->cellCenterAtLSide [0].dirLen < EPS) {
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	}

	//	moveIn3D ('x', hanger.ang, length_outa - 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;

	//	accumDist = 0.150 - (round (length_outa / 0.300, 3) * 0.150);
	//	moveIn3D ('x', hanger.ang, accumDist, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;
	//}

	//// 각파이프 행거 - 비계파이프 좌측 끝 부분, 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//if ((placingZone->nCellsFromEndAtSide > 0) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	hanger.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + 0.150;
	//	hanger.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	hanger.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0635;
	//	hanger.ang = placingZone->ang;
	//	hanger.angX = DegreeToRad (0.0);
	//	hanger.angY = DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hanger.leftBottomX;
	//	rotatedPoint.y = hanger.leftBottomY;
	//	rotatedPoint.z = hanger.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hanger.leftBottomX = unrotatedPoint.x;
	//	hanger.leftBottomY = unrotatedPoint.y;
	//	hanger.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hanger));

	//	length_outa = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//	moveIn3D ('x', hanger.ang, length_outa - 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	elemList.Push (placeLibPart (hanger));

	//	accumDist = 0.150 - (round (length_outa / 0.300, 3) * 0.150);
	//	moveIn3D ('x', hanger.ang, accumDist, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	elemList.Push (placeLibPart (hanger));
	//}

	//// 각파이프 행거 - 비계파이프 우측 끝 부분, 만약 센터 여백이 없으면 제외
	//xPos = centerPos;
	//if ((placingZone->nCellsFromEndAtSide > 0) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	hanger.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + 0.150;
	//	hanger.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	hanger.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0635;
	//	hanger.ang = placingZone->ang;
	//	hanger.angX = DegreeToRad (0.0);
	//	hanger.angY = DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hanger.leftBottomX;
	//	rotatedPoint.y = hanger.leftBottomY;
	//	rotatedPoint.z = hanger.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hanger.leftBottomX = unrotatedPoint.x;
	//	hanger.leftBottomY = unrotatedPoint.y;
	//	hanger.leftBottomZ = unrotatedPoint.z;

	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;

	//	length_outa = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//	moveIn3D ('x', hanger.ang, length_outa - 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;

	//	accumDist = 0.150 - (round (length_outa / 0.300, 3) * 0.150);
	//	moveIn3D ('x', hanger.ang, accumDist, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;
	//}

	//strcpy (timberRail.railType, "블루목심 2");

	//// 블루목심 - 유로폼 1단 (측면 시작 부분 - 왼쪽)
	//if (placingZone->bFillMarginBeginAtSide == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (90.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + accumDist - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023 - 0.194;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (270.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// 블루목심 - 유로폼 2단 (측면 시작 부분 - 왼쪽)
	//if (placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginBeginAtSide == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen + 0.023;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (90.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//			accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + accumDist - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen + 0.023 - 0.194;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (270.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// 블루목심 - 유로폼 1단 (측면 시작 부분 - 오른쪽)
	//if (placingZone->bFillMarginBeginAtSide == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023 - 0.194;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (270.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + accumDist - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (90.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// 블루목심 - 유로폼 2단 (측면 시작 부분 - 오른쪽)
	//if (placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginBeginAtSide == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen + 0.023 - 0.194;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (270.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//			accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + accumDist - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen + 0.023;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (90.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// 블루목심 - 유로폼 1단 (측면 끝 부분 - 왼쪽)
	//if (placingZone->bFillMarginEndAtSide == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023 - 0.194;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (270.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - accumDist + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (90.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// 블루목심 - 유로폼 2단 (측면 끝 부분 - 왼쪽)
	//if (placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginEndAtSide == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen + 0.023 - 0.194;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (270.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - accumDist + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen + 0.023;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (90.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// 블루목심 - 유로폼 1단 (측면 끝 부분 - 오른쪽)
	//if (placingZone->bFillMarginEndAtSide == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (90.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - accumDist + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023 - 0.194;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (270.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// 블루목심 - 유로폼 2단 (측면 끝 부분 - 오른쪽)
	//if (placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginEndAtSide == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen + 0.023;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (90.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - accumDist + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen + 0.023 - 0.194;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (270.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// 블루목심 - 유로폼 1단 (하부 시작 부분)
	//if (placingZone->bFillMarginBeginAtBottom == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - 0.023 + 0.194;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//	if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//	timberRail.angX = DegreeToRad (90.0);
	//	timberRail.angY = DegreeToRad (0.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtBottom [0].objType == PLYWOOD) && (placingZone->cellCenterAtBottom [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)
	//		accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + accumDist - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - 0.023;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//	if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//	timberRail.angX = DegreeToRad (90.0);
	//	timberRail.angY = DegreeToRad (180.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// 블루목심 - 유로폼 2단 (하부 시작 부분)
	//if (placingZone->cellsFromBeginAtBottom [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginBeginAtBottom == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - placingZone->cellsFromBeginAtBottom [0][0].perLen - placingZone->cellsFromBeginAtBottom [1][0].perLen - 0.023 + 0.194;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//		if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//		timberRail.angX = DegreeToRad (90.0);
	//		timberRail.angY = DegreeToRad (0.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtBottom [0].objType == PLYWOOD) && (placingZone->cellCenterAtBottom [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)
	//			accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + accumDist - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - placingZone->cellsFromBeginAtBottom [0][0].perLen - placingZone->cellsFromBeginAtBottom [1][0].perLen - 0.023;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//		if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//		timberRail.angX = DegreeToRad (90.0);
	//		timberRail.angY = DegreeToRad (180.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// 블루목심 - 유로폼 1단 (하부 끝 부분)
	//if (placingZone->bFillMarginEndAtBottom == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtBottom * marginEndAtBottom) - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - 0.023;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//	if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//	timberRail.angX = DegreeToRad (90.0);
	//	timberRail.angY = DegreeToRad (180.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtBottom [0].objType == PLYWOOD) && (placingZone->cellCenterAtBottom [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//		accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtBottom * marginEndAtBottom) - accumDist + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - 0.023 + 0.194;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//	if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//	timberRail.angX = DegreeToRad (90.0);
	//	timberRail.angY = DegreeToRad (00.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// 블루목심 - 유로폼 2단 (하부 끝 부분)
	//if (placingZone->cellsFromEndAtBottom [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginEndAtBottom == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtBottom * marginEndAtBottom) - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - placingZone->cellsFromEndAtBottom [0][0].perLen - placingZone->cellsFromEndAtBottom [1][0].perLen - 0.023;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//		if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//		timberRail.angX = DegreeToRad (90.0);
	//		timberRail.angY = DegreeToRad (180.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtBottom [0].objType == PLYWOOD) && (placingZone->cellCenterAtBottom [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//			accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtBottom * marginEndAtBottom) - accumDist + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - placingZone->cellsFromEndAtBottom [0][0].perLen - placingZone->cellsFromEndAtBottom [1][0].perLen - 0.023 + 0.194;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//		if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//		timberRail.angX = DegreeToRad (90.0);
	//		timberRail.angY = DegreeToRad (00.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	return	err;
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGSetItemText (dialogID, LABEL_BEAM_LEFT_HEIGHT, "보 높이 (L)");
			DGSetItemText (dialogID, LABEL_BEAM_RIGHT_HEIGHT, "보 높이 (R)");
			DGSetItemText (dialogID, LABEL_BEAM_WIDTH, "보 너비");
			DGSetItemText (dialogID, LABEL_TOTAL_LEFT_HEIGHT, "총 높이 (L)");
			DGSetItemText (dialogID, LABEL_TOTAL_RIGHT_HEIGHT, "총 높이 (R)");
			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, "총 너비");

			DGSetItemText (dialogID, LABEL_REST_LSIDE, "나머지");
			DGSetItemText (dialogID, CHECKBOX_TIMBER_LSIDE, "각재");
			DGSetItemText (dialogID, CHECKBOX_T_FORM_LSIDE, "유로폼");
			DGSetItemText (dialogID, CHECKBOX_FILLER_LSIDE, "휠러");
			DGSetItemText (dialogID, CHECKBOX_B_FORM_LSIDE, "유로폼");

			DGSetItemText (dialogID, LABEL_REST_RSIDE, "나머지");
			DGSetItemText (dialogID, CHECKBOX_TIMBER_RSIDE, "각재");
			DGSetItemText (dialogID, CHECKBOX_T_FORM_RSIDE, "유로폼");
			DGSetItemText (dialogID, CHECKBOX_FILLER_RSIDE, "휠러");
			DGSetItemText (dialogID, CHECKBOX_B_FORM_RSIDE, "유로폼");

			DGSetItemText (dialogID, CHECKBOX_L_FORM_BOTTOM, "유로폼");
			DGSetItemText (dialogID, CHECKBOX_FILLER_BOTTOM, "휠러");
			DGSetItemText (dialogID, CHECKBOX_R_FORM_BOTTOM, "유로폼");

			DGSetItemText (dialogID, BUTTON_COPY_TO_RIGHT, "복사\n→");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "각재");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "아웃코너앵글");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSPACER, "휠러스페이서");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "비계파이프");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "각파이프행거");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트세트");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "유로폼 후크");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_CLAMP, "블루클램프");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL, "블루목심");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, BUTTON_AUTOSET, "레이어 자동 설정");

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

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSPACER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_CLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_TIMBER_RAIL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, 1);

			// 보 높이/너비 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT, placingZone.areaHeight_Left);		// 보 높이 (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT, placingZone.areaHeight_Right);		// 보 높이 (R)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, placingZone.areaWidth_Bottom);			// 보 너비

			// 총 높이/너비 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT, placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));		// 총 높이 (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.areaWidth_Bottom + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1) + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2));		// 총 너비
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT, placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));	// 총 높이 (R)

			// 부재별 체크박스-규격 설정
			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_LSIDE);

			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
			(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_RSIDE);

			// 측면 0번, 하부 0번 셀은 무조건 사용해야 함
			DGSetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM, TRUE);
			DGDisableItem (dialogID, CHECKBOX_B_FORM_LSIDE);
			DGDisableItem (dialogID, CHECKBOX_B_FORM_RSIDE);
			DGDisableItem (dialogID, CHECKBOX_L_FORM_BOTTOM);

			// 나머지 값 계산
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest);

			// 직접 변경해서는 안 되는 항목 잠그기
			DGDisableItem (dialogID, EDITCONTROL_GAP_SIDE2);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_REST_LSIDE);
			DGDisableItem (dialogID, EDITCONTROL_REST_RSIDE);

			// 레이어 옵션 활성화/비활성화
			if ((DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}

			break;
		
		case DG_MSG_CHANGE:
			// 보 높이/너비 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT, placingZone.areaHeight_Left);		// 보 높이 (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT, placingZone.areaHeight_Right);		// 보 높이 (R)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, placingZone.areaWidth_Bottom);			// 보 너비

			// 총 높이/너비 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT, placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));		// 총 높이 (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.areaWidth_Bottom + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1) + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2));		// 총 너비
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT, placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));	// 총 높이 (R)

			// 부재별 체크박스-규격 설정
			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_LSIDE);

			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
			(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_RSIDE);

			// 나머지 값 계산
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest);

			// 왼쪽 간격이 바뀌면 오른쪽 간격도 동일하게 바뀜
			DGSetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2, DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1));

			// 레이어 같이 바뀜
			if ((item >= USERCONTROL_LAYER_EUROFORM) && (item <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL)) {
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					long selectedLayer;

					selectedLayer = DGGetItemValLong (dialogID, item);

					for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL ; ++xx)
						DGSetItemValLong (dialogID, xx, selectedLayer);
				}
			}

			// 레이어 옵션 활성화/비활성화
			if ((DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					///////////////////////////////////////////////////////////////// 왼쪽 측면 (높은쪽)
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [0][xx].objType = EUROFORM;
							placingZone.cellsAtLSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [1][xx].objType = FILLERSP;
							placingZone.cellsAtLSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [2][xx].objType = EUROFORM;
							placingZone.cellsAtLSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							if (DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE) >= 0.090 - EPS)
								placingZone.cellsAtLSide [3][xx].objType = PLYWOOD;
							else
								placingZone.cellsAtLSide [3][xx].objType = TIMBER;
							placingZone.cellsAtLSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
						}
					}

					///////////////////////////////////////////////////////////////// 오른쪽 측면 (낮은쪽)
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [0][xx].objType = EUROFORM;
							placingZone.cellsAtRSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [1][xx].objType = FILLERSP;
							placingZone.cellsAtRSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [2][xx].objType = EUROFORM;
							placingZone.cellsAtRSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							if (DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE) >= 0.090 - EPS)
								placingZone.cellsAtRSide [3][xx].objType = PLYWOOD;
							else
								placingZone.cellsAtRSide [3][xx].objType = TIMBER;
							placingZone.cellsAtRSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
						}
					}

					///////////////////////////////////////////////////////////////// 하부
					if (DGGetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtBottom [0][xx].objType = EUROFORM;
							placingZone.cellsAtBottom [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtBottom [1][xx].objType = FILLERSP;
							placingZone.cellsAtBottom [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_BOTTOM);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtBottom [2][xx].objType = EUROFORM;
							placingZone.cellsAtBottom [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
						}
					}

					// 보와의 간격
					placingZone.gapSide = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1);
					placingZone.gapBottom = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM);

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					layerInd_Fillerspacer	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER);
					layerInd_Rectpipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
					layerInd_Pinbolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
					layerInd_BlueClamp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
					layerInd_BlueTimberRail	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					layerInd_Euroform			= makeTemporaryLayer (structuralObject_forTableformBeam, "UFOM", NULL);
					layerInd_Plywood			= makeTemporaryLayer (structuralObject_forTableformBeam, "PLYW", NULL);
					layerInd_Timber				= makeTemporaryLayer (structuralObject_forTableformBeam, "TIMB", NULL);
					layerInd_OutcornerAngle		= makeTemporaryLayer (structuralObject_forTableformBeam, "OUTA", NULL);
					layerInd_Fillerspacer		= makeTemporaryLayer (structuralObject_forTableformBeam, "FISP", NULL);
					layerInd_Rectpipe			= makeTemporaryLayer (structuralObject_forTableformBeam, "SPIP", NULL);
					layerInd_RectpipeHanger		= makeTemporaryLayer (structuralObject_forTableformBeam, "JOIB", NULL);
					layerInd_Pinbolt			= makeTemporaryLayer (structuralObject_forTableformBeam, "PINB", NULL);
					layerInd_EuroformHook		= makeTemporaryLayer (structuralObject_forTableformBeam, "HOOK", NULL);
					layerInd_BlueClamp			= makeTemporaryLayer (structuralObject_forTableformBeam, "UFCL", NULL);
					layerInd_BlueTimberRail		= makeTemporaryLayer (structuralObject_forTableformBeam, "RAIL", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, layerInd_OutcornerAngle);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, layerInd_Fillerspacer);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_Rectpipe);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, layerInd_RectpipeHanger);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_Pinbolt);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, layerInd_EuroformHook);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

					break;

				case DG_CANCEL:
					break;

				case BUTTON_COPY_TO_RIGHT:
					item = 0;

					// 체크박스 상태 복사
					DGSetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE));
					DGSetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE));
					DGSetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE));
					DGSetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE, DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE));

					// 팝업 컨트롤 및 Edit컨트롤 값 복사
					DGPopUpSelectItem (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE));
					DGSetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE, DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE));
					DGPopUpSelectItem (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE));
					DGSetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE, DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE));

					// 보 높이/너비 계산
					DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT, placingZone.areaHeight_Left);		// 보 높이 (L)
					DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT, placingZone.areaHeight_Right);		// 보 높이 (R)
					DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, placingZone.areaWidth_Bottom);			// 보 너비

					// 총 높이/너비 계산
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT, placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));		// 총 높이 (L)
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.areaWidth_Bottom + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1) + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2));		// 총 너비
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT, placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));	// 총 높이 (R)

					// 부재별 체크박스-규격 설정
					(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_LSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_LSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_LSIDE);

					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
					(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

					(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_RSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_RSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_RSIDE);

					// 나머지 값 계산
					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
					hRest = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest);

					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
					hRest = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest);

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
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	btnSizeX = 50, btnSizeY = 50;
	short	btnPosX, btnPosY;
	short	xx;
	short	itmPosX, itmPosY;
	std::string		txtButton = "";
	const short		maxCol = 50;		// 열 최대 개수
	static short	dialogSizeX = 500;	// 현재 다이얼로그 크기 X
	static short	dialogSizeY = 360;	// 현재 다이얼로그 크기 Y

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "보에 배치 - 보 측면");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 230, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 270, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 이전 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 310, 70, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "이전");
			DGShowItem (dialogID, DG_PREV);

			// 라벨: 보 측면
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 10, 100, 23);
			DGSetItemFont (dialogID, LABEL_BEAM_SIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_BEAM_SIDE, "보 측면");
			DGShowItem (dialogID, LABEL_BEAM_SIDE);

			// 라벨: 총 길이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 12, 60, 23);
			DGSetItemFont (dialogID, LABEL_TOTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TOTAL_LENGTH, "총 길이");
			DGShowItem (dialogID, LABEL_TOTAL_LENGTH);

			// Edit컨트롤: 총 길이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 165, 5, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_TOTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_TOTAL_LENGTH);

			// 라벨: 남은 길이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 230, 12, 60, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_LENGTH, "남은 길이");
			DGShowItem (dialogID, LABEL_REMAIN_LENGTH);

			// Edit컨트롤: 남은 길이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 300, 5, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_LENGTH);

			// 버튼: 추가
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 70, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_COL, "추가");
			DGShowItem (dialogID, BUTTON_ADD_COL);

			// 버튼: 삭제
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 105, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_COL, "삭제");
			DGShowItem (dialogID, BUTTON_DEL_COL);

			// 왼쪽 끝 여백 채우기 여부 (체크박스)
			placingZone.CHECKBOX_MARGIN_LEFT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 120, 70, 70, 70);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, "합판\n채우기");
			DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, TRUE);
			// 왼쪽 끝 여백 길이 (Edit컨트롤)
			placingZone.EDITCONTROL_MARGIN_LEFT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 120, 140, 70, 25);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);

			// 일반 셀: 기본값은 유로폼 !!!
			itmPosX = 120+70;
			itmPosY = 70;
			//for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
			//	// 버튼
			//	placingZone.BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
			//	DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], "테이블폼");
			//	DGShowItem (dialogID, placingZone.BUTTON_OBJ [xx]);

			//	// 객체 타입 (팝업컨트롤)
			//	placingZone.POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
			//	DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "없음");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "테이블폼");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "유로폼");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "휠러스페이서");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "합판");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "각재");
			//	DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
			//	DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

			//	// 너비 (팝업컨트롤)
			//	placingZone.POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
			//	DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
			//	for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
			//		DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
			//		_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
			//		DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
			//	}
			//	DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
			//	DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

			//	// 너비 (팝업컨트롤) - 처음에는 숨김
			//	placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
			//	DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);

			//	itmPosX += 70;
			//}

			// 오른쪽 끝 여백 채우기 여부 (체크박스)
			//placingZone.CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
			//DGSetItemFont (dialogID, placingZone.CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, placingZone.CHECKBOX_RINCORNER, "인코너");
			//DGShowItem (dialogID, placingZone.CHECKBOX_RINCORNER);
			//DGSetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER, TRUE);
			// 오른쪽 끝 여백 길이 (Edit컨트롤)
			//placingZone.EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
			//DGShowItem (dialogID, placingZone.EDITCONTROL_RINCORNER);
			//DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER, 0.100);

			// 다이얼로그 크기 설정
			dialogSizeX = 500;
			dialogSizeY = 360;
			//if (placingZone.nCellsInHor >= 5) {
			//	DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
			//}

			break;

		case DG_MSG_CLICK:

			switch (item) {
				case DG_PREV:
					clickedPrevButton = true;
					break;

				case DG_OK:
					break;

				case DG_CANCEL:
					break;
			}

			//// 확인 버튼
			//if (item == DG_OK) {
			//	clickedOKButton = true;

			//	// 여백 채움/비움 여부 저장
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
			//		placingZone.bFillMarginBeginAtSide = true;
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
			//		placingZone.bFillMarginEndAtSide = true;
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
			//		placingZone.bFillMarginBeginAtBottom = true;
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
			//		placingZone.bFillMarginEndAtBottom = true;

			//	placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

			//	// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
			//	placingZone.alignPlacingZone (&placingZone);
			//}

			//// 취소 버튼
			//if (item == DG_CANCEL) {
			//}

			//// 셀 추가/삭제 버튼 8종
			//if (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) {
			//	placingZone.addNewColFromBeginAtSide (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) {
			//	placingZone.delLastColFromBeginAtSide (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_END_AT_SIDE) {
			//	placingZone.addNewColFromEndAtSide (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_END_AT_SIDE) {
			//	placingZone.delLastColFromEndAtSide (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) {
			//	placingZone.addNewColFromBeginAtBottom (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) {
			//	placingZone.delLastColFromBeginAtBottom (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_END_AT_BOTTOM) {
			//	placingZone.addNewColFromEndAtBottom (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_END_AT_BOTTOM) {
			//	placingZone.delLastColFromEndAtBottom (&placingZone);
			//}

			//if ( (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) || (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) || (item == ADD_CELLS_FROM_END_AT_SIDE) || (item == DEL_CELLS_FROM_END_AT_SIDE) ||
			//	 (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == ADD_CELLS_FROM_END_AT_BOTTOM) || (item == DEL_CELLS_FROM_END_AT_BOTTOM)) {

			//	item = 0;

			//	// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
			//	placingZone.alignPlacingZone (&placingZone);

			//	// 변경 가능성이 있는 DG 항목 모두 제거
			//	DGRemoveDialogItems (dialogID, AFTER_ALL);

			//	// 측면 시작 부분 여백 채움 여부 - bFillMarginBeginAtSide
			//	// 라디오 버튼: 여백 (채움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 채움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_BEGIN_AT_SIDE = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// 라디오 버튼: 여백 (비움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 135, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 비움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_BEGIN_AT_SIDE = itmIdx;

			//	// 측면 시작 부분 여백
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 50, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 55, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 74, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtSide);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_BEGIN_AT_SIDE = itmIdx;
			//	btnPosX = 150;
			//	btnPosY = 50;
			//	// 측면 시작 부분
			//	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtSide ; ++xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromBeginAtRSide [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromBeginAtRSide [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtRSide [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == 0) START_INDEX_FROM_BEGIN_AT_SIDE = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// 화살표 추가
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "↑");
			//	DGShowItem (dialogID, itmIdx);
			//	// 추가/삭제 버튼
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "추가");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "삭제");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			//	// 측면 중앙 부분
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	txtButton = "";
			//	if (placingZone.cellCenterAtRSide [0].objType == NONE) {
			//		txtButton = "NONE";
			//	} else if (placingZone.cellCenterAtRSide [0].objType == EUROFORM) {
			//		txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
			//	} else if (placingZone.cellCenterAtRSide [0].objType == PLYWOOD) {
			//		txtButton = format_string ("합판\n↔%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
			//	}
			//	DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//	DGShowItem (dialogID, itmIdx);
			//	START_INDEX_CENTER_AT_SIDE = itmIdx;
			//	btnPosX += 50;
			//	// 화살표 추가
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "↑");
			//	DGShowItem (dialogID, itmIdx);
			//	// 추가/삭제 버튼
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "추가");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_END_AT_SIDE = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "삭제");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_END_AT_SIDE = itmIdx;
			//	// 측면 끝 부분
			//	for (xx = placingZone.nCellsFromEndAtSide-1 ; xx >= 0 ; --xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromEndAtRSide [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromEndAtRSide [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtRSide [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == (placingZone.nCellsFromEndAtSide-1)) END_INDEX_FROM_END_AT_SIDE = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// 측면 끝 부분 여백
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 50, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 55, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 74, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtSide);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_END_AT_SIDE = itmIdx;

			//	// 측면 끝 부분 여백 채움 여부 - bFillMarginEndAtSide
			//	// 라디오 버튼: 여백 (채움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 110, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 채움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_END_AT_SIDE = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// 라디오 버튼: 여백 (비움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 135, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 비움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_END_AT_SIDE = itmIdx;

			//	// 하부 시작 부분 여백 채움 여부 - bFillMarginBeginAtBottom
			//	// 라디오 버튼: 여백 (채움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 270, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 채움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// 라디오 버튼: 여백 (비움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 295, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 비움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM = itmIdx;

			//	// 하부 시작 부분 여백
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 210, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 215, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 234, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtBottom);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	btnPosX = 150;
			//	btnPosY = 210;
			//	// 하부 시작 부분
			//	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtBottom ; ++xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromBeginAtBottom [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromBeginAtBottom [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtBottom [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == 0) START_INDEX_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// 화살표 추가
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "↑");
			//	DGShowItem (dialogID, itmIdx);
			//	// 추가/삭제 버튼
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "추가");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "삭제");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	// 하부 중앙 부분
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	txtButton = "";
			//	if (placingZone.cellCenterAtBottom [0].objType == NONE) {
			//		txtButton = "NONE";
			//	} else if (placingZone.cellCenterAtBottom [0].objType == EUROFORM) {
			//		txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
			//	} else if (placingZone.cellCenterAtBottom [0].objType == PLYWOOD) {
			//		txtButton = format_string ("합판\n↔%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
			//	}
			//	DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//	DGShowItem (dialogID, itmIdx);
			//	START_INDEX_CENTER_AT_BOTTOM = itmIdx;
			//	btnPosX += 50;
			//	// 화살표 추가
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "↑");
			//	DGShowItem (dialogID, itmIdx);
			//	// 추가/삭제 버튼
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "추가");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "삭제");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			//	// 하부 끝 부분
			//	for (xx = placingZone.nCellsFromEndAtBottom-1 ; xx >= 0 ; --xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromEndAtBottom [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromEndAtBottom [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtBottom [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == (placingZone.nCellsFromEndAtBottom-1)) END_INDEX_FROM_END_AT_BOTTOM = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// 하부 끝 부분 여백
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 210, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 215, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 234, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtBottom);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_END_AT_BOTTOM = itmIdx;

			//	// 하부 끝 부분 여백 채움 여부 - bFillMarginEndAtBottom
			//	// 라디오 버튼: 여백 (채움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 270, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 채움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_END_AT_BOTTOM = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// 라디오 버튼: 여백 (비움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 295, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 비움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_END_AT_BOTTOM = itmIdx;

			//	// 간섭 보가 붙는 곳 영역 길이 (측면)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 150 + (btnSizeX * placingZone.nCellsFromBeginAtSide), 24, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGShowItem (dialogID, itmIdx);
			//	if (placingZone.cellCenterAtRSide [0].objType == NONE) {
			//		DGEnableItem (dialogID, itmIdx);
			//		DGEnableItem (dialogID, DG_UPDATE_BUTTON);
			//	} else {
			//		DGDisableItem (dialogID, itmIdx);
			//		DGDisableItem (dialogID, DG_UPDATE_BUTTON);
			//	}
			//	EDITCONTROL_CENTER_LENGTH_SIDE = itmIdx;

			//	// 메인 창 크기를 변경
			//	dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
			//	dialogSizeY = 490;
			//	DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			//}

			//// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
			//if ( ((item >= START_INDEX_FROM_BEGIN_AT_SIDE) && (item < START_INDEX_FROM_BEGIN_AT_SIDE + placingZone.nCellsFromBeginAtSide))			// 배치 버튼 (측면 시작 부분)
			//	|| (item == START_INDEX_CENTER_AT_SIDE)																								// 배치 버튼 (측면 중앙 부분)
			//	|| ((item >= END_INDEX_FROM_END_AT_SIDE) && (item < END_INDEX_FROM_END_AT_SIDE + placingZone.nCellsFromEndAtSide))					// 배치 버튼 (측면 끝 부분)
			//	|| ((item >= START_INDEX_FROM_BEGIN_AT_BOTTOM) && (item < START_INDEX_FROM_BEGIN_AT_BOTTOM + placingZone.nCellsFromBeginAtBottom))	// 배치 버튼 (하부 시작 부분)
			//	|| (item == START_INDEX_CENTER_AT_BOTTOM)																							// 배치 버튼 (하부 중앙 부분)
			//	|| ((item >= END_INDEX_FROM_END_AT_BOTTOM) && (item < END_INDEX_FROM_END_AT_BOTTOM + placingZone.nCellsFromEndAtBottom))			// 배치 버튼 (하부 끝 부분)
			//	) {
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamTableformPlacerHandler3, 0);
			//	item = 0;

			//	// 저장된 측면 시작 여백 여부 저장
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
			//		placingZone.bFillMarginBeginAtSide = true;
			//	else
			//		placingZone.bFillMarginBeginAtSide = false;

			//	// 저장된 측면 끝 여백 여부 저장
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
			//		placingZone.bFillMarginEndAtSide = true;
			//	else
			//		placingZone.bFillMarginEndAtSide = false;

			//	// 저장된 하부 시작 여백 여부 저장
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
			//		placingZone.bFillMarginBeginAtBottom = true;
			//	else
			//		placingZone.bFillMarginBeginAtBottom = false;

			//	// 저장된 하부 끝 여백 여부 저장
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
			//		placingZone.bFillMarginEndAtBottom = true;
			//	else
			//		placingZone.bFillMarginEndAtBottom = false;

			//	// 간섭 보가 붙는 곳 영역 길이 저장
			//	placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

			//	// 셀 정보 변경 발생, 모든 셀의 위치 값을 업데이트
			//	placingZone.alignPlacingZone (&placingZone);

			//	// 변경 가능성이 있는 DG 항목 모두 제거
			//	DGRemoveDialogItems (dialogID, AFTER_ALL);

			//	// 측면 시작 부분 여백 채움 여부 - bFillMarginBeginAtSide
			//	// 라디오 버튼: 여백 (채움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 채움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_BEGIN_AT_SIDE = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// 라디오 버튼: 여백 (비움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 135, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 비움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_BEGIN_AT_SIDE = itmIdx;

			//	// 저장된 측면 시작 여백 여부 로드
			//	if (placingZone.bFillMarginBeginAtSide == true) {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE, TRUE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_SIDE, FALSE);
			//	} else {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE, FALSE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_SIDE, TRUE);
			//	}

			//	// 측면 시작 부분 여백
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 50, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 55, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 74, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtSide);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_BEGIN_AT_SIDE = itmIdx;
			//	btnPosX = 150;
			//	btnPosY = 50;
			//	// 측면 시작 부분
			//	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtSide ; ++xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromBeginAtRSide [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromBeginAtRSide [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtRSide [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == 0) START_INDEX_FROM_BEGIN_AT_SIDE = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// 화살표 추가
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "↑");
			//	DGShowItem (dialogID, itmIdx);
			//	// 추가/삭제 버튼
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "추가");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "삭제");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			//	// 측면 중앙 부분
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	txtButton = "";
			//	if (placingZone.cellCenterAtRSide [0].objType == NONE) {
			//		txtButton = "NONE";
			//	} else if (placingZone.cellCenterAtRSide [0].objType == EUROFORM) {
			//		txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
			//	} else if (placingZone.cellCenterAtRSide [0].objType == PLYWOOD) {
			//		txtButton = format_string ("합판\n↔%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
			//	}
			//	DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//	DGShowItem (dialogID, itmIdx);
			//	START_INDEX_CENTER_AT_SIDE = itmIdx;
			//	btnPosX += 50;
			//	// 화살표 추가
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "↑");
			//	DGShowItem (dialogID, itmIdx);
			//	// 추가/삭제 버튼
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "추가");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_END_AT_SIDE = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "삭제");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_END_AT_SIDE = itmIdx;
			//	// 측면 끝 부분
			//	for (xx = placingZone.nCellsFromEndAtSide-1 ; xx >= 0 ; --xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromEndAtRSide [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromEndAtRSide [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtRSide [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == (placingZone.nCellsFromEndAtSide-1)) END_INDEX_FROM_END_AT_SIDE = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// 측면 끝 부분 여백
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 50, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 55, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 74, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtSide);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_END_AT_SIDE = itmIdx;

			//	// 측면 끝 부분 여백 채움 여부 - bFillMarginEndAtSide
			//	// 라디오 버튼: 여백 (채움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 110, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 채움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_END_AT_SIDE = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// 라디오 버튼: 여백 (비움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 135, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 비움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_END_AT_SIDE = itmIdx;

			//	// 저장된 측면 끝 여백 여부 로드
			//	if (placingZone.bFillMarginEndAtSide == true) {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE, TRUE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_SIDE, FALSE);
			//	} else {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE, FALSE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_SIDE, TRUE);
			//	}

			//	// 하부 시작 부분 여백 채움 여부 - bFillMarginBeginAtBottom
			//	// 라디오 버튼: 여백 (채움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 270, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 채움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// 라디오 버튼: 여백 (비움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 295, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 비움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM = itmIdx;

			//	// 저장된 하부 시작 여백 여부 로드
			//	if (placingZone.bFillMarginBeginAtBottom == true) {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM, TRUE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM, FALSE);
			//	} else {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM, FALSE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM, TRUE);
			//	}

			//	// 하부 시작 부분 여백
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 210, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 215, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 234, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtBottom);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	btnPosX = 150;
			//	btnPosY = 210;
			//	// 하부 시작 부분
			//	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtBottom ; ++xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromBeginAtBottom [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromBeginAtBottom [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromBeginAtBottom [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == 0) START_INDEX_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// 화살표 추가
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "↑");
			//	DGShowItem (dialogID, itmIdx);
			//	// 추가/삭제 버튼
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "추가");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "삭제");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	// 하부 중앙 부분
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	txtButton = "";
			//	if (placingZone.cellCenterAtBottom [0].objType == NONE) {
			//		txtButton = "NONE";
			//	} else if (placingZone.cellCenterAtBottom [0].objType == EUROFORM) {
			//		txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
			//	} else if (placingZone.cellCenterAtBottom [0].objType == PLYWOOD) {
			//		txtButton = format_string ("합판\n↔%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
			//	}
			//	DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//	DGShowItem (dialogID, itmIdx);
			//	START_INDEX_CENTER_AT_BOTTOM = itmIdx;
			//	btnPosX += 50;
			//	// 화살표 추가
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "↑");
			//	DGShowItem (dialogID, itmIdx);
			//	// 추가/삭제 버튼
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "추가");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "삭제");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			//	// 하부 끝 부분
			//	for (xx = placingZone.nCellsFromEndAtBottom-1 ; xx >= 0 ; --xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromEndAtBottom [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromEndAtBottom [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("유로폼\n↔%.0f", placingZone.cellsFromEndAtBottom [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// 그리드 버튼 텍스트 지정
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == (placingZone.nCellsFromEndAtBottom-1)) END_INDEX_FROM_END_AT_BOTTOM = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// 하부 끝 부분 여백
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 210, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 215, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 234, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtBottom);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_END_AT_BOTTOM = itmIdx;

			//	// 하부 끝 부분 여백 채움 여부 - bFillMarginEndAtBottom
			//	// 라디오 버튼: 여백 (채움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 270, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 채움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_END_AT_BOTTOM = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// 라디오 버튼: 여백 (비움)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 295, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "여백 비움");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_END_AT_BOTTOM = itmIdx;

			//	// 저장된 하부 끝 여백 여부 로드
			//	if (placingZone.bFillMarginEndAtBottom == true) {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM, TRUE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_BOTTOM, FALSE);
			//	} else {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM, FALSE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_BOTTOM, TRUE);
			//	}

			//	// 간섭 보가 붙는 곳 영역 길이 (측면)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 150 + (btnSizeX * placingZone.nCellsFromBeginAtSide), 24, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGShowItem (dialogID, itmIdx);
			//	if (placingZone.cellCenterAtRSide [0].objType == NONE) {
			//		DGEnableItem (dialogID, itmIdx);
			//		DGEnableItem (dialogID, DG_UPDATE_BUTTON);
			//	} else {
			//		DGDisableItem (dialogID, itmIdx);
			//		DGDisableItem (dialogID, DG_UPDATE_BUTTON);
			//	}
			//	EDITCONTROL_CENTER_LENGTH_SIDE = itmIdx;

			//	// 간섭 보가 붙는 곳 영역 길이 로드
			//	DGSetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE, placingZone.centerLengthAtSide);

			//	// 메인 창 크기를 변경
			//	dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
			//	dialogSizeY = 490;
			//	DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			//}

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
