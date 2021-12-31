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
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 보 (1개), 보 측면(전체/일부)을 덮는 모프 (1개)\n옵션 선택 (1): 보 반대쪽 측면을 덮는 모프 (1개)", true);
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
	if (morph1_height > morph2_height) {
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

	// 총 길이에서 유로폼 높이 값으로 나누어서 대략적인 셀 개수(nCells) 초기 지정
	placingZone.nCells = (short)(floor (placingZone.beamLength / 1.200));

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

	// 객체 위치 재조정
	placingZone.alignPlacingZone (&placingZone);

	// 기본 객체 배치하기
	err = placingZone.placeBasicObjects (&placingZone);

	// 부자재 배치하기
	err = placingZone.placeAuxObjects (&placingZone);

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
void	BeamTableformPlacingZone::initCells (BeamTableformPlacingZone* placingZone)
{
	short xx, yy;

	// 영역 정보의 여백 채움 여부 초기화
	placingZone->bFillMarginBegin = false;
	placingZone->bFillMarginEnd = false;

	// 영역 정보의 여백 설정 초기화
	placingZone->marginBegin = 0.0;
	placingZone->marginEnd = 0.0;

	// 보 양끝 셀
	placingZone->beginCellAtLSide.objType = PLYWOOD;
	placingZone->beginCellAtLSide.leftBottomX = 0.0;
	placingZone->beginCellAtLSide.leftBottomY = 0.0;
	placingZone->beginCellAtLSide.leftBottomZ = 0.0;
	placingZone->beginCellAtLSide.ang = 0.0;
	placingZone->beginCellAtLSide.dirLen = 0.0;
	placingZone->beginCellAtLSide.perLen = 0.0;

	placingZone->beginCellAtRSide.objType = PLYWOOD;
	placingZone->beginCellAtRSide.leftBottomX = 0.0;
	placingZone->beginCellAtRSide.leftBottomY = 0.0;
	placingZone->beginCellAtRSide.leftBottomZ = 0.0;
	placingZone->beginCellAtRSide.ang = 0.0;
	placingZone->beginCellAtRSide.dirLen = 0.0;
	placingZone->beginCellAtRSide.perLen = 0.0;

	placingZone->beginCellAtBottom.objType = PLYWOOD;
	placingZone->beginCellAtBottom.leftBottomX = 0.0;
	placingZone->beginCellAtBottom.leftBottomY = 0.0;
	placingZone->beginCellAtBottom.leftBottomZ = 0.0;
	placingZone->beginCellAtBottom.ang = 0.0;
	placingZone->beginCellAtBottom.dirLen = 0.0;
	placingZone->beginCellAtBottom.perLen = 0.0;

	placingZone->endCellAtLSide.objType = PLYWOOD;
	placingZone->endCellAtLSide.leftBottomX = 0.0;
	placingZone->endCellAtLSide.leftBottomY = 0.0;
	placingZone->endCellAtLSide.leftBottomZ = 0.0;
	placingZone->endCellAtLSide.ang = 0.0;
	placingZone->endCellAtLSide.dirLen = 0.0;
	placingZone->endCellAtLSide.perLen = 0.0;

	placingZone->endCellAtRSide.objType = PLYWOOD;
	placingZone->endCellAtRSide.leftBottomX = 0.0;
	placingZone->endCellAtRSide.leftBottomY = 0.0;
	placingZone->endCellAtRSide.leftBottomZ = 0.0;
	placingZone->endCellAtRSide.ang = 0.0;
	placingZone->endCellAtRSide.dirLen = 0.0;
	placingZone->endCellAtRSide.perLen = 0.0;

	placingZone->endCellAtBottom.objType = PLYWOOD;
	placingZone->endCellAtBottom.leftBottomX = 0.0;
	placingZone->endCellAtBottom.leftBottomY = 0.0;
	placingZone->endCellAtBottom.leftBottomZ = 0.0;
	placingZone->endCellAtBottom.ang = 0.0;
	placingZone->endCellAtBottom.dirLen = 0.0;
	placingZone->endCellAtBottom.perLen = 0.0;

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

			placingZone->cellsAtRSide [xx][yy].objType = NONE;
			placingZone->cellsAtRSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtRSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtRSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtRSide [xx][yy].ang = 0.0;
			placingZone->cellsAtRSide [xx][yy].dirLen = 0.0;
			placingZone->cellsAtRSide [xx][yy].perLen = 0.0;
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
		}
	}
}

// 셀(0-기반 인덱스 번호)의 좌하단 점 위치 X 좌표를 구함
double	BeamTableformPlacingZone::getCellPositionLeftBottomX (BeamTableformPlacingZone* placingZone, short idx)
{
	double	distance = (placingZone->bFillMarginBegin == true) ? placingZone->marginBegin : 0;

	for (short xx = 0 ; xx < idx ; ++xx)
		distance += placingZone->cellsAtLSide [0][xx].dirLen;

	return distance;
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	BeamTableformPlacingZone::alignPlacingZone (BeamTableformPlacingZone* placingZone)
{
	short	xx;

	if (placingZone->bFillMarginBegin == true) {
		placingZone->beginCellAtLSide.ang = placingZone->ang;
		placingZone->beginCellAtLSide.leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang));
		placingZone->beginCellAtLSide.leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang));
		placingZone->beginCellAtLSide.leftBottomZ = placingZone->begC.z - placingZone->gapBottom;

		placingZone->beginCellAtRSide.ang = placingZone->ang;
		placingZone->beginCellAtRSide.leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang));
		placingZone->beginCellAtRSide.leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang));
		placingZone->beginCellAtRSide.leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->beginCellAtRSide.ang, placingZone->areaWidth_Bottom + (placingZone->gapSide * 2), &placingZone->beginCellAtRSide.leftBottomX, &placingZone->beginCellAtRSide.leftBottomY, &placingZone->beginCellAtRSide.leftBottomZ);

		placingZone->beginCellAtBottom.ang = placingZone->ang;
		placingZone->beginCellAtBottom.leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang));
		placingZone->beginCellAtBottom.leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang));
		placingZone->beginCellAtBottom.leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
	}

	if (placingZone->bFillMarginEnd == true) {
		placingZone->endCellAtLSide.ang = placingZone->ang;
		placingZone->endCellAtLSide.leftBottomX = placingZone->endC.x + (placingZone->gapSide * sin(placingZone->ang));
		placingZone->endCellAtLSide.leftBottomY = placingZone->endC.y - (placingZone->gapSide * cos(placingZone->ang));
		placingZone->endCellAtLSide.leftBottomZ = placingZone->endC.z - placingZone->gapBottom;

		placingZone->endCellAtRSide.ang = placingZone->ang;
		placingZone->endCellAtRSide.leftBottomX = placingZone->endC.x + (placingZone->gapSide * sin(placingZone->ang));
		placingZone->endCellAtRSide.leftBottomY = placingZone->endC.y - (placingZone->gapSide * cos(placingZone->ang));
		placingZone->endCellAtRSide.leftBottomZ = placingZone->endC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->endCellAtRSide.ang, placingZone->areaWidth_Bottom + (placingZone->gapSide * 2), &placingZone->endCellAtRSide.leftBottomX, &placingZone->endCellAtRSide.leftBottomY, &placingZone->endCellAtRSide.leftBottomZ);

		placingZone->endCellAtBottom.ang = placingZone->ang;
		placingZone->endCellAtBottom.leftBottomX = placingZone->endC.x + (placingZone->gapSide * sin(placingZone->ang));
		placingZone->endCellAtBottom.leftBottomY = placingZone->endC.y - (placingZone->gapSide * cos(placingZone->ang));
		placingZone->endCellAtBottom.leftBottomZ = placingZone->endC.z - placingZone->gapBottom;
	}

	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		// 측면 (아래쪽 유로폼 라인)
		placingZone->cellsAtLSide [0][xx].ang = placingZone->ang;
		placingZone->cellsAtLSide [0][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtLSide [0][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtLSide [0][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;

		placingZone->cellsAtRSide [0][xx].ang = placingZone->ang;
		placingZone->cellsAtRSide [0][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtRSide [0][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtRSide [0][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtRSide [0][xx].ang, placingZone->areaWidth_Bottom + (placingZone->gapSide * 2), &placingZone->cellsAtRSide [0][xx].leftBottomX, &placingZone->cellsAtRSide [0][xx].leftBottomY, &placingZone->cellsAtRSide [0][xx].leftBottomZ);

		// 측면 (휠러스페이서 라인)
		placingZone->cellsAtLSide [1][xx].ang = placingZone->ang;
		placingZone->cellsAtLSide [1][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtLSide [1][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtLSide [1][xx].leftBottomZ = placingZone->begC.z + placingZone->cellsAtLSide [0][xx].perLen - placingZone->gapBottom;

		placingZone->cellsAtRSide [1][xx].ang = placingZone->ang;
		placingZone->cellsAtRSide [1][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtRSide [1][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtRSide [1][xx].leftBottomZ = placingZone->begC.z + placingZone->cellsAtRSide [0][xx].perLen - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtRSide [1][xx].ang, placingZone->areaWidth_Bottom + (placingZone->gapSide * 2), &placingZone->cellsAtRSide [1][xx].leftBottomX, &placingZone->cellsAtRSide [1][xx].leftBottomY, &placingZone->cellsAtRSide [1][xx].leftBottomZ);

		// 측면 (위쪽 유로폼 라인)
		placingZone->cellsAtLSide [2][xx].ang = placingZone->ang;
		placingZone->cellsAtLSide [2][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtLSide [2][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtLSide [2][xx].leftBottomZ = placingZone->begC.z + placingZone->cellsAtLSide [0][xx].perLen + placingZone->cellsAtLSide [1][xx].perLen - placingZone->gapBottom;

		placingZone->cellsAtRSide [2][xx].ang = placingZone->ang;
		placingZone->cellsAtRSide [2][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtRSide [2][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtRSide [2][xx].leftBottomZ = placingZone->begC.z + placingZone->cellsAtRSide [0][xx].perLen + placingZone->cellsAtRSide [1][xx].perLen - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtRSide [2][xx].ang, placingZone->areaWidth_Bottom + (placingZone->gapSide * 2), &placingZone->cellsAtRSide [2][xx].leftBottomX, &placingZone->cellsAtRSide [2][xx].leftBottomY, &placingZone->cellsAtRSide [2][xx].leftBottomZ);

		// 측면 (각재 라인)
		placingZone->cellsAtLSide [3][xx].ang = placingZone->ang;
		placingZone->cellsAtLSide [3][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtLSide [3][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtLSide [3][xx].leftBottomZ = placingZone->begC.z + placingZone->cellsAtLSide [0][xx].perLen + placingZone->cellsAtLSide [1][xx].perLen + placingZone->cellsAtLSide [2][xx].perLen - placingZone->gapBottom;

		placingZone->cellsAtRSide [3][xx].ang = placingZone->ang;
		placingZone->cellsAtRSide [3][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtRSide [3][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtRSide [3][xx].leftBottomZ = placingZone->begC.z + placingZone->cellsAtRSide [0][xx].perLen + placingZone->cellsAtRSide [1][xx].perLen + placingZone->cellsAtRSide [2][xx].perLen - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtRSide [3][xx].ang, placingZone->areaWidth_Bottom + (placingZone->gapSide * 2), &placingZone->cellsAtRSide [3][xx].leftBottomX, &placingZone->cellsAtRSide [3][xx].leftBottomY, &placingZone->cellsAtRSide [3][xx].leftBottomZ);

		// 하부 (유로폼, 휠러, 유로폼)
		placingZone->cellsAtBottom [0][xx].ang = placingZone->ang;
		placingZone->cellsAtBottom [0][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtBottom [0][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtBottom [0][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;

		placingZone->cellsAtBottom [1][xx].ang = placingZone->ang;
		placingZone->cellsAtBottom [1][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtBottom [1][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtBottom [1][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtBottom [1][xx].ang, placingZone->cellsAtBottom [0][xx].perLen, &placingZone->cellsAtBottom [1][xx].leftBottomX, &placingZone->cellsAtBottom [1][xx].leftBottomY, &placingZone->cellsAtBottom [1][xx].leftBottomZ);

		placingZone->cellsAtBottom [2][xx].ang = placingZone->ang;
		placingZone->cellsAtBottom [2][xx].leftBottomX = placingZone->begC.x + (placingZone->gapSide * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cellsAtBottom [2][xx].leftBottomY = placingZone->begC.y - (placingZone->gapSide * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cellsAtBottom [2][xx].leftBottomZ = placingZone->begC.z - placingZone->gapBottom;
		moveIn3D ('y', placingZone->cellsAtBottom [2][xx].ang, placingZone->cellsAtBottom [0][xx].perLen + placingZone->cellsAtBottom [1][xx].perLen, &placingZone->cellsAtBottom [2][xx].leftBottomX, &placingZone->cellsAtBottom [2][xx].leftBottomY, &placingZone->cellsAtBottom [2][xx].leftBottomZ);
	}
}

// 유로폼/휠러/합판/각재를 배치함
GSErrCode	BeamTableformPlacingZone::placeBasicObjects (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy;

	// 원장 사이즈 길이 계산
	bool	bShow;
	bool	bBeginFound;
	short	beginIndex, endIndex;
	double	remainLengthDouble;
	double	lengthDouble;

	// 각재 및 합판 배치에 사용됨
	double	horLen, verLen;
	bool	bTimberMove;
	double	moveZ;
	short	addedPlywood;

	EasyObjectPlacement euroform, fillersp, plywood, timber;
	EasyObjectPlacement plywood1, plywood2, plywood3;

	if (placingZone->bFillMarginBegin == true) {
		// 시작 부분 측면(L) 합판 배치
		if (placingZone->beginCellAtLSide.dirLen > EPS) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->beginCellAtLSide.leftBottomX, placingZone->beginCellAtLSide.leftBottomY, placingZone->beginCellAtLSide.leftBottomZ, placingZone->beginCellAtLSide.ang);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beginCellAtLSide.perLen + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->beginCellAtLSide.dirLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.070),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}

		// 시작 부분 측면(R) 합판 배치
		if (placingZone->beginCellAtRSide.dirLen > EPS) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->beginCellAtRSide.leftBottomX, placingZone->beginCellAtRSide.leftBottomY, placingZone->beginCellAtRSide.leftBottomZ, placingZone->beginCellAtRSide.ang);
			moveIn3D ('x', plywood.radAng, beginCellAtRSide.dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			plywood.radAng += DegreeToRad (180.0);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beginCellAtRSide.perLen + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->beginCellAtRSide.dirLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.070),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}

		// 시작 부분 하부 합판 배치
		if (placingZone->beginCellAtBottom.dirLen > EPS) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->beginCellAtBottom.leftBottomX, placingZone->beginCellAtBottom.leftBottomY, placingZone->beginCellAtBottom.leftBottomZ, placingZone->beginCellAtBottom.ang);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "바닥깔기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beginCellAtBottom.perLen),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->beginCellAtBottom.dirLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.070),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}
	}

	if (placingZone->bFillMarginEnd == true) {
		// 끝 부분 측면(L) 합판 배치
		if (placingZone->endCellAtLSide.dirLen > EPS) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->endCellAtLSide.leftBottomX, placingZone->endCellAtLSide.leftBottomY, placingZone->endCellAtLSide.leftBottomZ, placingZone->endCellAtLSide.ang);
			moveIn3D ('x', plywood.radAng, -placingZone->endCellAtLSide.dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->endCellAtLSide.perLen + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->endCellAtLSide.dirLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.070),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}

		// 끝 부분 측면(R) 합판 배치
		if (placingZone->endCellAtRSide.dirLen > EPS) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->endCellAtRSide.leftBottomX, placingZone->endCellAtRSide.leftBottomY, placingZone->endCellAtRSide.leftBottomZ, placingZone->endCellAtRSide.ang);
			moveIn3D ('z', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			plywood.radAng += DegreeToRad (180.0);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->endCellAtRSide.perLen + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->endCellAtRSide.dirLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.070),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}

		// 끝 부분 하부 합판 배치
		if (placingZone->endCellAtBottom.dirLen > EPS) {
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->endCellAtBottom.leftBottomX, placingZone->endCellAtBottom.leftBottomY, placingZone->endCellAtBottom.leftBottomZ, placingZone->endCellAtBottom.ang);
			moveIn3D ('x', plywood.radAng, -placingZone->endCellAtBottom.dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "바닥깔기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->endCellAtBottom.perLen),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->endCellAtBottom.dirLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.070),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}
	}

	// 측면(L) 유로폼 1단 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);

		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS)) {
			moveIn3D ('x', euroform.radAng, placingZone->cellsAtLSide [0][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			elemList.Push (euroform.placeObject (5,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtLSide [0][xx].perLen * 1000.0),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtLSide [0][xx].dirLen * 1000.0),
				"u_ins", APIParT_CString, "벽눕히기",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
		}
	}

	// 측면(L) 휠러스페이서 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [1][xx].objType == FILLERSP) && (placingZone->cellsAtLSide [1][xx].perLen > 0) && (placingZone->cellsAtLSide [1][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [1][xx].objType != FILLERSP) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [1][yy].dirLen;

			fillersp.init (L("휠러스페이서v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtLSide [1][beginIndex].leftBottomX, placingZone->cellsAtLSide [1][beginIndex].leftBottomY, placingZone->cellsAtLSide [1][beginIndex].leftBottomZ, placingZone->cellsAtLSide [1][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				elemList.Push (fillersp.placeObject (4,
					"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [1][beginIndex].perLen),
					"f_leng", APIParT_Length, format_string ("%f", lengthDouble),
					"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"f_rota", APIParT_Angle, format_string ("%f", 0.0)));
				moveIn3D ('x', fillersp.radAng, lengthDouble, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 측면(L) 유로폼 2단 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtLSide [2][xx].leftBottomX, placingZone->cellsAtLSide [2][xx].leftBottomY, placingZone->cellsAtLSide [2][xx].leftBottomZ, placingZone->cellsAtLSide [2][xx].ang);

		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > EPS) && (placingZone->cellsAtLSide [2][xx].dirLen > EPS)) {
			moveIn3D ('x', euroform.radAng, placingZone->cellsAtLSide [2][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			elemList.Push (euroform.placeObject (5,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtLSide [2][xx].perLen * 1000.0),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtLSide [2][xx].dirLen * 1000.0),
				"u_ins", APIParT_CString, "벽눕히기",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
		}
	}

	// 측면(L) 각재/합판 배치
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [3][xx].perLen > EPS) && (placingZone->cellsAtLSide [3][xx].dirLen > EPS)) {
			if (placingZone->cellsAtLSide [3][xx].objType == TIMBER) {
				bTimberMove = false;
				moveZ = 0.0;
				addedPlywood = 0;
				if (placingZone->cellsAtLSide [3][xx].perLen < 0.040 - EPS) {
					// 40mm 미만이면 앞쪽으로(!) 50*80 각재
					horLen = 0.050;
					verLen = 0.080;
					bTimberMove = true;

					if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.010) < EPS)	addedPlywood = 1;	// 10mm 이면 합판 1장 얹음
					if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.020) < EPS)	addedPlywood = 2;	// 20mm 이면 합판 2장 얹음
					if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.030) < EPS)	addedPlywood = 3;	// 30mm 이면 합판 3장 얹음
				} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.050 - EPS)) {
					// 40mm 이상 50mm 미만이면, 50*40 각재
					horLen = 0.050;
					verLen = 0.040;
				} else if ((placingZone->cellsAtLSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtLSide [3][xx].perLen < 0.080 - EPS)) {
					// 50mm 이상 80mm 미만이면, 80*50 각재
					horLen = 0.080;
					verLen = 0.050;
					moveZ = verLen;

					if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.060) < EPS)	addedPlywood = 1;	// 60mm 이면 합판 1장 얹음
					if (abs (placingZone->cellsAtLSide [3][xx].perLen - 0.070) < EPS)	addedPlywood = 2;	// 70mm 이면 합판 2장 얹음
				} else {
					// 80mm 이상 90mm 미만이면, 80*80 각재
					horLen = 0.080;
					verLen = 0.080;
				}

				if ((placingZone->cellsAtLSide [3][xx].objType == TIMBER) && (placingZone->cellsAtLSide [3][xx].perLen > EPS) && (placingZone->cellsAtLSide [3][xx].dirLen > EPS)) {
					// 연속적인 인덱스 범위 찾기
					if (bBeginFound == false) {
						beginIndex = xx;
						bBeginFound = true;
					}
					endIndex = xx;
				}

				if ((placingZone->cellsAtLSide [3][xx].objType != TIMBER) || (xx == placingZone->nCells-1)) {
					// 원장 사이즈 단위로 끊어서 배치하기 (각재)
					remainLengthDouble = 0.0;
					for (yy = beginIndex ; yy <= endIndex ; ++yy)
						remainLengthDouble += placingZone->cellsAtLSide [3][yy].dirLen;

					timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);

					while (remainLengthDouble > EPS) {
						if (remainLengthDouble > 3.600)
							lengthDouble = 3.600;
						else
							lengthDouble = remainLengthDouble;

						// 각재 설치
						if (bTimberMove == true) {
							moveIn3D ('y', timber.radAng, -0.067, &timber.posX, &timber.posY, &timber.posZ);
							moveIn3D ('z', timber.radAng, -0.080, &timber.posX, &timber.posY, &timber.posZ);
						}
						elemList.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", horLen), "w_h", APIParT_Length, format_string ("%f", verLen), "w_leng", APIParT_Length, format_string ("%f", lengthDouble), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
						if (bTimberMove == true) {
							moveIn3D ('y', timber.radAng, 0.067, &timber.posX, &timber.posY, &timber.posZ);
							moveIn3D ('z', timber.radAng, 0.080, &timber.posX, &timber.posY, &timber.posZ);
						}
						moveIn3D ('x', timber.radAng, lengthDouble, &timber.posX, &timber.posY, &timber.posZ);

						remainLengthDouble -= 3.600;
					}

					// 원장 사이즈 단위로 끊어서 배치하기 (추가 합판)
					remainLengthDouble = 0.0;
					for (yy = beginIndex ; yy <= endIndex ; ++yy)
						remainLengthDouble += placingZone->cellsAtLSide [3][yy].dirLen;

					plywood1.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);
					plywood2.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ + 0.0115, placingZone->cellsAtLSide [3][beginIndex].ang);
					plywood3.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ + 0.0115*2, placingZone->cellsAtLSide [3][beginIndex].ang);

					while (remainLengthDouble > EPS) {
						if (remainLengthDouble > 2.400)
							lengthDouble = 2.400;
						else
							lengthDouble = remainLengthDouble;
						
						if (addedPlywood >= 1) {
							moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							elemList.Push (plywood1.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, -moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('x', plywood1.radAng, lengthDouble, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						}
						if (addedPlywood >= 2) {
							moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							elemList.Push (plywood2.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, -moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('x', plywood2.radAng, lengthDouble, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						}
						if (addedPlywood >= 3) {
							moveIn3D ('y', plywood3.radAng, -0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							moveIn3D ('z', plywood3.radAng, moveZ, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							elemList.Push (plywood3.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							moveIn3D ('y', plywood3.radAng, 0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							moveIn3D ('z', plywood3.radAng, -moveZ, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							moveIn3D ('x', plywood3.radAng, lengthDouble, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						}

						remainLengthDouble -= 2.400;
					}

					bBeginFound = false;
				}
			} else if (placingZone->cellsAtLSide [3][xx].objType == PLYWOOD) {
				// 90mm 이상이면
				if ((placingZone->cellsAtLSide [3][xx].objType == PLYWOOD) && (placingZone->cellsAtLSide [3][xx].perLen > EPS) && (placingZone->cellsAtLSide [3][xx].dirLen > EPS)) {
					// 연속적인 인덱스 범위 찾기
					if (bBeginFound == false) {
						beginIndex = xx;
						bBeginFound = true;
					}
					endIndex = xx;
				}

				if ((placingZone->cellsAtLSide [3][xx].objType != PLYWOOD) || (xx == placingZone->nCells-1)) {
					// 원장 사이즈 단위로 끊어서 배치하기 (합판)
					remainLengthDouble = 0.0;
					for (yy = beginIndex ; yy <= endIndex ; ++yy)
						remainLengthDouble += placingZone->cellsAtLSide [3][yy].dirLen;

					plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [3][beginIndex].leftBottomX, placingZone->cellsAtLSide [3][beginIndex].leftBottomY, placingZone->cellsAtLSide [3][beginIndex].leftBottomZ, placingZone->cellsAtLSide [3][beginIndex].ang);

					while (remainLengthDouble > EPS) {
						if (remainLengthDouble > 2.400)
							lengthDouble = 2.400;
						else
							lengthDouble = remainLengthDouble;

						// 합판 설치
						elemList.Push (plywood.placeObject (13,
							"p_stan", APIParT_CString, "비규격",
							"w_dir", APIParT_CString, "벽눕히기",
							"p_thk", APIParT_CString, "11.5T",
							"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [3][beginIndex].perLen),
							"p_leng", APIParT_Length, format_string ("%f", lengthDouble),
							"p_ang", APIParT_Angle, format_string ("%f", 0.0),
							"sogak", APIParT_Boolean, "1.0",
							"bInverseSogak", APIParT_Boolean, "1.0",
							"prof", APIParT_CString, "소각",
							"gap_a", APIParT_Length, format_string ("%f", 0.0),
							"gap_b", APIParT_Length, format_string ("%f", 0.0),
							"gap_c", APIParT_Length, format_string ("%f", 0.0),
							"gap_d", APIParT_Length, format_string ("%f", 0.0)));
						moveIn3D ('x', plywood.radAng, lengthDouble, &plywood.posX, &plywood.posY, &plywood.posZ);

						remainLengthDouble -= 2.400;
					}
					bBeginFound = false;
				}
			}
		}
	}
	
	// 측면(R) 유로폼 1단 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);

		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > EPS) && (placingZone->cellsAtRSide [0][xx].dirLen > EPS)) {
			euroform.radAng += DegreeToRad (180.0);
			elemList.Push (euroform.placeObject (5,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtRSide [0][xx].perLen * 1000.0),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtRSide [0][xx].dirLen * 1000.0),
				"u_ins", APIParT_CString, "벽눕히기",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
		}
	}

	// 측면(R) 휠러스페이서 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [1][xx].objType == FILLERSP) && (placingZone->cellsAtRSide [1][xx].perLen > 0) && (placingZone->cellsAtRSide [1][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [1][xx].objType != FILLERSP) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [1][yy].dirLen;

			fillersp.init (L("휠러스페이서v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtRSide [1][beginIndex].leftBottomX, placingZone->cellsAtRSide [1][beginIndex].leftBottomY, placingZone->cellsAtRSide [1][beginIndex].leftBottomZ, placingZone->cellsAtRSide [1][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				moveIn3D ('x', fillersp.radAng, lengthDouble, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
				fillersp.radAng += DegreeToRad (180.0);
				elemList.Push (fillersp.placeObject (4,
					"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [1][beginIndex].perLen),
					"f_leng", APIParT_Length, format_string ("%f", lengthDouble),
					"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"f_rota", APIParT_Angle, format_string ("%f", 0.0)));
				fillersp.radAng -= DegreeToRad (180.0);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 측면(R) 유로폼 2단 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtRSide [2][xx].leftBottomX, placingZone->cellsAtRSide [2][xx].leftBottomY, placingZone->cellsAtRSide [2][xx].leftBottomZ, placingZone->cellsAtRSide [2][xx].ang);

		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > EPS) && (placingZone->cellsAtRSide [2][xx].dirLen > EPS)) {
			euroform.radAng += DegreeToRad (180.0);
			elemList.Push (euroform.placeObject (5,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtRSide [2][xx].perLen * 1000.0),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtRSide [2][xx].dirLen * 1000.0),
				"u_ins", APIParT_CString, "벽눕히기",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
		}
	}

	// 측면(R) 각재/합판 배치
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [3][xx].perLen > EPS) && (placingZone->cellsAtRSide [3][xx].dirLen > EPS)) {
			if (placingZone->cellsAtRSide [3][xx].objType == TIMBER) {
				bTimberMove = false;
				moveZ = 0.0;
				addedPlywood = 0;
				if (placingZone->cellsAtRSide [3][xx].perLen < 0.040 - EPS) {
					// 40mm 미만이면 앞쪽으로(!) 50*80 각재
					horLen = 0.050;
					verLen = 0.080;
					bTimberMove = true;

					if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.010) < EPS)	addedPlywood = 1;	// 10mm 이면 합판 1장 얹음
					if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.020) < EPS)	addedPlywood = 2;	// 20mm 이면 합판 2장 얹음
					if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.030) < EPS)	addedPlywood = 3;	// 30mm 이면 합판 3장 얹음
				} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.040 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.050 - EPS)) {
					// 40mm 이상 50mm 미만이면, 50*40 각재
					horLen = 0.050;
					verLen = 0.040;
				} else if ((placingZone->cellsAtRSide [3][xx].perLen >= 0.050 - EPS) && (placingZone->cellsAtRSide [3][xx].perLen < 0.080 - EPS)) {
					// 50mm 이상 80mm 미만이면, 80*50 각재
					horLen = 0.080;
					verLen = 0.050;
					moveZ = verLen;

					if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.060) < EPS)	addedPlywood = 1;	// 60mm 이면 합판 1장 얹음
					if (abs (placingZone->cellsAtRSide [3][xx].perLen - 0.070) < EPS)	addedPlywood = 2;	// 70mm 이면 합판 2장 얹음
				} else {
					// 80mm 이상 90mm 미만이면, 80*80 각재
					horLen = 0.080;
					verLen = 0.080;
				}

				if ((placingZone->cellsAtRSide [3][xx].objType == TIMBER) && (placingZone->cellsAtRSide [3][xx].perLen > EPS) && (placingZone->cellsAtRSide [3][xx].dirLen > EPS)) {
					// 연속적인 인덱스 범위 찾기
					if (bBeginFound == false) {
						beginIndex = xx;
						bBeginFound = true;
					}
					endIndex = xx;
				}

				if ((placingZone->cellsAtRSide [3][xx].objType != TIMBER) || (xx == placingZone->nCells-1)) {
					// 원장 사이즈 단위로 끊어서 배치하기 (각재)
					remainLengthDouble = 0.0;
					for (yy = beginIndex ; yy <= endIndex ; ++yy)
						remainLengthDouble += placingZone->cellsAtRSide [3][yy].dirLen;

					timber.init (L("목재v1.0.gsm"), layerInd_Timber, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);

					while (remainLengthDouble > EPS) {
						if (remainLengthDouble > 3.600)
							lengthDouble = 3.600;
						else
							lengthDouble = remainLengthDouble;

						// 각재 설치
						if (bTimberMove == true) {
							moveIn3D ('y', timber.radAng, 0.067, &timber.posX, &timber.posY, &timber.posZ);
							moveIn3D ('z', timber.radAng, -0.080, &timber.posX, &timber.posY, &timber.posZ);
						}
						moveIn3D ('x', timber.radAng, lengthDouble, &timber.posX, &timber.posY, &timber.posZ);
						timber.radAng += DegreeToRad (180.0);
						elemList.Push (timber.placeObject (6, "w_ins", APIParT_CString, "벽세우기", "w_w", APIParT_Length, format_string ("%f", horLen), "w_h", APIParT_Length, format_string ("%f", verLen), "w_leng", APIParT_Length, format_string ("%f", lengthDouble), "w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
						timber.radAng -= DegreeToRad (180.0);
						if (bTimberMove == true) {
							moveIn3D ('y', timber.radAng, -0.067, &timber.posX, &timber.posY, &timber.posZ);
							moveIn3D ('z', timber.radAng, 0.080, &timber.posX, &timber.posY, &timber.posZ);
						}

						remainLengthDouble -= 3.600;
					}

					// 원장 사이즈 단위로 끊어서 배치하기 (추가 합판)
					remainLengthDouble = 0.0;
					for (yy = beginIndex ; yy <= endIndex ; ++yy)
						remainLengthDouble += placingZone->cellsAtRSide [3][yy].dirLen;

					plywood1.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);
					plywood2.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ + 0.0115, placingZone->cellsAtRSide [3][beginIndex].ang);
					plywood3.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ + 0.0115*2, placingZone->cellsAtRSide [3][beginIndex].ang);

					while (remainLengthDouble > EPS) {
						if (remainLengthDouble > 2.400)
							lengthDouble = 2.400;
						else
							lengthDouble = remainLengthDouble;
						
						if (addedPlywood >= 1) {
							moveIn3D ('x', plywood1.radAng, lengthDouble, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('y', plywood1.radAng, 0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							plywood1.radAng += DegreeToRad (180.0);
							elemList.Push (plywood1.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							plywood1.radAng -= DegreeToRad (180.0);
							moveIn3D ('y', plywood1.radAng, -0.070, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
							moveIn3D ('z', plywood1.radAng, -moveZ, &plywood1.posX, &plywood1.posY, &plywood1.posZ);
						}
						if (addedPlywood >= 2) {
							moveIn3D ('x', plywood2.radAng, lengthDouble, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('y', plywood2.radAng, 0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							plywood2.radAng += DegreeToRad (180.0);
							elemList.Push (plywood2.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							plywood2.radAng -= DegreeToRad (180.0);
							moveIn3D ('y', plywood2.radAng, -0.070, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
							moveIn3D ('z', plywood2.radAng, -moveZ, &plywood2.posX, &plywood2.posY, &plywood2.posZ);
						}
						if (addedPlywood >= 3) {
							moveIn3D ('x', plywood3.radAng, lengthDouble, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							moveIn3D ('y', plywood3.radAng, 0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							moveIn3D ('z', plywood3.radAng, moveZ, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							plywood3.radAng += DegreeToRad (180.0);
							elemList.Push (plywood3.placeObject (7, "p_stan", APIParT_CString, "비규격", "w_dir", APIParT_CString, "바닥덮기", "p_thk", APIParT_CString, "11.5T", "p_wid", APIParT_Length, format_string ("%f", 0.070), "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", 0.0), "sogak", APIParT_Boolean, "0.0"));
							plywood3.radAng -= DegreeToRad (180.0);
							moveIn3D ('y', plywood3.radAng, -0.070, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
							moveIn3D ('z', plywood3.radAng, -moveZ, &plywood3.posX, &plywood3.posY, &plywood3.posZ);
						}

						remainLengthDouble -= 2.400;
					}

					bBeginFound = false;
				}
			} else if (placingZone->cellsAtRSide [3][xx].objType == PLYWOOD) {
				// 90mm 이상이면
				if ((placingZone->cellsAtRSide [3][xx].objType == PLYWOOD) && (placingZone->cellsAtRSide [3][xx].perLen > EPS) && (placingZone->cellsAtRSide [3][xx].dirLen > EPS)) {
					// 연속적인 인덱스 범위 찾기
					if (bBeginFound == false) {
						beginIndex = xx;
						bBeginFound = true;
					}
					endIndex = xx;
				}

				if ((placingZone->cellsAtRSide [3][xx].objType != PLYWOOD) || (xx == placingZone->nCells-1)) {
					// 원장 사이즈 단위로 끊어서 배치하기 (합판)
					remainLengthDouble = 0.0;
					for (yy = beginIndex ; yy <= endIndex ; ++yy)
						remainLengthDouble += placingZone->cellsAtRSide [3][yy].dirLen;

					plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [3][beginIndex].leftBottomX, placingZone->cellsAtRSide [3][beginIndex].leftBottomY, placingZone->cellsAtRSide [3][beginIndex].leftBottomZ, placingZone->cellsAtRSide [3][beginIndex].ang);

					while (remainLengthDouble > EPS) {
						if (remainLengthDouble > 2.400)
							lengthDouble = 2.400;
						else
							lengthDouble = remainLengthDouble;

						// 합판 설치
						moveIn3D ('x', plywood.radAng, lengthDouble, &plywood.posX, &plywood.posY, &plywood.posZ);
						plywood.radAng += DegreeToRad (180.0);
						elemList.Push (plywood.placeObject (13,
							"p_stan", APIParT_CString, "비규격",
							"w_dir", APIParT_CString, "벽눕히기",
							"p_thk", APIParT_CString, "11.5T",
							"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [3][beginIndex].perLen),
							"p_leng", APIParT_Length, format_string ("%f", lengthDouble),
							"p_ang", APIParT_Angle, format_string ("%f", 0.0),
							"sogak", APIParT_Boolean, "1.0",
							"bInverseSogak", APIParT_Boolean, "1.0",
							"prof", APIParT_CString, "소각",
							"gap_a", APIParT_Length, format_string ("%f", 0.0),
							"gap_b", APIParT_Length, format_string ("%f", 0.0),
							"gap_c", APIParT_Length, format_string ("%f", 0.0),
							"gap_d", APIParT_Length, format_string ("%f", 0.0)));
						plywood.radAng -= DegreeToRad (180.0);

						remainLengthDouble -= 2.400;
					}
					bBeginFound = false;
				}
			}
		}
	}

	// 하부 유로폼(L) 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);

		if ((placingZone->cellsAtBottom [0][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [0][xx].perLen > EPS) && (placingZone->cellsAtBottom [0][xx].dirLen > EPS)) {
			moveIn3D ('x', euroform.radAng, placingZone->cellsAtBottom [0][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			moveIn3D ('y', euroform.radAng, placingZone->cellsAtBottom [0][xx].perLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			elemList.Push (euroform.placeObject (5,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtBottom [0][xx].perLen * 1000.0),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtBottom [0][xx].dirLen * 1000.0),
				"u_ins", APIParT_CString, "벽눕히기",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
		}
	}

	// 하부 휠러스페이서 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtBottom [1][xx].objType == FILLERSP) && (placingZone->cellsAtBottom [1][xx].perLen > 0) && (placingZone->cellsAtBottom [1][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtBottom [1][xx].objType != FILLERSP) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtBottom [1][yy].dirLen;

			fillersp.init (L("휠러스페이서v1.0.gsm"), layerInd_Fillerspacer, infoBeam.floorInd, placingZone->cellsAtBottom [1][beginIndex].leftBottomX, placingZone->cellsAtBottom [1][beginIndex].leftBottomY, placingZone->cellsAtBottom [1][beginIndex].leftBottomZ, placingZone->cellsAtBottom [1][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				moveIn3D ('y', fillersp.radAng, placingZone->cellsAtBottom [1][beginIndex].perLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
				elemList.Push (fillersp.placeObject (4,
					"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [1][beginIndex].perLen),
					"f_leng", APIParT_Length, format_string ("%f", lengthDouble),
					"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"f_rota", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
				moveIn3D ('y', fillersp.radAng, -placingZone->cellsAtBottom [1][beginIndex].perLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
				moveIn3D ('x', fillersp.radAng, lengthDouble, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 하부 유로폼(R) 배치
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		euroform.init (L("유로폼v2.0.gsm"), layerInd_Euroform, infoBeam.floorInd, placingZone->cellsAtBottom [2][xx].leftBottomX, placingZone->cellsAtBottom [2][xx].leftBottomY, placingZone->cellsAtBottom [2][xx].leftBottomZ, placingZone->cellsAtBottom [2][xx].ang);

		if ((placingZone->cellsAtBottom [2][xx].objType == EUROFORM) && (placingZone->cellsAtBottom [2][xx].perLen > EPS) && (placingZone->cellsAtBottom [2][xx].dirLen > EPS)) {
			moveIn3D ('x', euroform.radAng, placingZone->cellsAtBottom [2][xx].dirLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			moveIn3D ('y', euroform.radAng, placingZone->cellsAtBottom [2][xx].perLen, &euroform.posX, &euroform.posY, &euroform.posZ);
			elemList.Push (euroform.placeObject (5,
				"eu_stan_onoff", APIParT_Boolean, "1.0",
				"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsAtBottom [2][xx].perLen * 1000.0),
				"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsAtBottom [2][xx].dirLen * 1000.0),
				"u_ins", APIParT_CString, "벽눕히기",
				"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
		}
	}

	// 합판 셀 배치 (측면 뷰에서 합판으로 선택한 경우)
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == PLYWOOD) && (placingZone->cellsAtLSide [0][xx].perLen > EPS) && (placingZone->cellsAtLSide [0][xx].dirLen > EPS)) {
			// 측면(L) 합판 배치
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtLSide [0][xx].leftBottomX, placingZone->cellsAtLSide [0][xx].leftBottomY, placingZone->cellsAtLSide [0][xx].leftBottomZ, placingZone->cellsAtLSide [0][xx].ang);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [0][xx].perLen),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtLSide [0][xx].dirLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// 측면(R) 합판 배치
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtRSide [0][xx].leftBottomX, placingZone->cellsAtRSide [0][xx].leftBottomY, placingZone->cellsAtRSide [0][xx].leftBottomZ, placingZone->cellsAtRSide [0][xx].ang);
			moveIn3D ('x', plywood.radAng, cellsAtRSide [0][xx].dirLen, &plywood.posX, &plywood.posY, &plywood.posZ);
			plywood.radAng += DegreeToRad (180.0);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "벽눕히기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [0][xx].perLen),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtRSide [0][xx].dirLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// 하부 합판 배치
			plywood.init (L("합판v1.0.gsm"), layerInd_Plywood, infoBeam.floorInd, placingZone->cellsAtBottom [0][xx].leftBottomX, placingZone->cellsAtBottom [0][xx].leftBottomY, placingZone->cellsAtBottom [0][xx].leftBottomZ, placingZone->cellsAtBottom [0][xx].ang);
			moveIn3D ('y', plywood.radAng, -0.0615, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "비규격",
				"w_dir", APIParT_CString, "바닥깔기",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].perLen + 0.0615*2),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->cellsAtBottom [0][xx].dirLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "소각",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}
	}

	return	err;
}

// 유로폼/휠러/합판/각재를 채운 후 부자재 설치 (아웃코너앵글, 비계파이프, 핀볼트, 각파이프행거, 블루클램프, 블루목심)
GSErrCode	BeamTableformPlacingZone::placeAuxObjects (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx, yy;

	// 원장 사이즈 길이 계산
	bool	bShow;
	bool	bBeginFound;
	short	beginIndex, endIndex;
	double	remainLengthDouble;
	double	lengthDouble;

	EasyObjectPlacement outangle, hanger, blueClamp, blueTimberRail;
	EasyObjectPlacement pipe1, pipe2;
	EasyObjectPlacement pinbolt1, pinbolt2;

	// 아웃코너앵글 (L) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			outangle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				moveIn3D ('x', outangle.radAng, lengthDouble, &outangle.posX, &outangle.posY, &outangle.posZ);
				outangle.radAng += DegreeToRad (180.0);
				elemList.Push (outangle.placeObject (2, "a_leng", APIParT_Length, format_string ("%f", lengthDouble), "a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
				outangle.radAng -= DegreeToRad (180.0);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 아웃코너앵글 (R) 배치
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.0;
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			outangle.init (L("아웃코너앵글v1.0.gsm"), layerInd_OutcornerAngle, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 2.400)
					lengthDouble = 2.400;
				else
					lengthDouble = remainLengthDouble;

				elemList.Push (outangle.placeObject (2, "a_leng", APIParT_Length, format_string ("%f", lengthDouble), "a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
				moveIn3D ('x', outangle.radAng, lengthDouble, &outangle.posX, &outangle.posY, &outangle.posZ);

				remainLengthDouble -= 2.400;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (L) 배치 - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [0][xx].perLen > 0) && (placingZone->cellsAtLSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [0][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
			moveIn3D ('x', pipe1.radAng, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, -0.0635 - 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.100, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.300) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.100, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.200) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [0][beginIndex].leftBottomX, placingZone->cellsAtLSide [0][beginIndex].leftBottomY, placingZone->cellsAtLSide [0][beginIndex].leftBottomZ, placingZone->cellsAtLSide [0][beginIndex].ang);
			moveIn3D ('x', pipe2.radAng, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, -0.0635 - 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.600 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.500 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.450 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.400 - 0.100, &pipe2.posX, &pipe2.posY, &pipe2.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				elemList.Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3D ('x', pipe1.radAng, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2번 파이프
				if ((abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [0][beginIndex].perLen - 0.400) < EPS)) {
					elemList.Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3D ('x', pipe2.radAng, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (L) 배치 - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtLSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtLSide [2][xx].perLen > 0) && (placingZone->cellsAtLSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtLSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtLSide [2][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
			moveIn3D ('x', pipe1.radAng, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, -0.0635 - 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.100, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.300) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.100, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.200) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtLSide [2][beginIndex].leftBottomX, placingZone->cellsAtLSide [2][beginIndex].leftBottomY, placingZone->cellsAtLSide [2][beginIndex].leftBottomZ, placingZone->cellsAtLSide [2][beginIndex].ang);
			moveIn3D ('x', pipe2.radAng, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, -0.0635 - 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.600 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.500 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.450 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.400 - 0.100, &pipe2.posX, &pipe2.posY, &pipe2.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				elemList.Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				moveIn3D ('x', pipe1.radAng, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				// 2번 파이프
				if ((abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtLSide [2][beginIndex].perLen - 0.400) < EPS)) {
					elemList.Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					moveIn3D ('x', pipe2.radAng, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (R) 배치 - 1단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [0][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [0][xx].perLen > 0) && (placingZone->cellsAtRSide [0][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [0][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [0][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
			moveIn3D ('x', pipe1.radAng, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, 0.0635 + 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.100, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.300) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.100, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.200) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [0][beginIndex].leftBottomX, placingZone->cellsAtRSide [0][beginIndex].leftBottomY, placingZone->cellsAtRSide [0][beginIndex].leftBottomZ, placingZone->cellsAtRSide [0][beginIndex].ang);
			moveIn3D ('x', pipe2.radAng, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, 0.0635 + 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.600 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.500 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.450 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.400 - 0.100, &pipe2.posX, &pipe2.posY, &pipe2.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				moveIn3D ('x', pipe1.radAng, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				pipe1.radAng += DegreeToRad (180.0);
				elemList.Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);
				// 2번 파이프
				if ((abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [0][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3D ('x', pipe2.radAng, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					elemList.Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					pipe2.radAng -= DegreeToRad (180.0);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (R) 배치 - 2단 유로폼
	bShow = false;
	bBeginFound = false;
	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		if ((placingZone->cellsAtRSide [2][xx].objType == EUROFORM) && (placingZone->cellsAtRSide [2][xx].perLen > 0) && (placingZone->cellsAtRSide [2][xx].dirLen > 0)) {
			// 연속적인 인덱스 범위 찾기
			if (bBeginFound == false) {
				beginIndex = xx;
				bBeginFound = true;
				bShow = true;
			}
			endIndex = xx;
		}

		if (((placingZone->cellsAtRSide [2][xx].objType != EUROFORM) || (xx == placingZone->nCells-1)) && (bShow == true)) {
			// 원장 사이즈 단위로 끊어서 배치하기
			remainLengthDouble = 0.100;		// 양쪽으로 50mm 튀어나오므로 길이 미리 추가됨
			for (yy = beginIndex ; yy <= endIndex ; ++yy)
				remainLengthDouble += placingZone->cellsAtRSide [2][yy].dirLen;

			// 1번 파이프
			pipe1.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
			moveIn3D ('x', pipe1.radAng, -0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			moveIn3D ('y', pipe1.radAng, 0.0635 + 0.025, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.150, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.100, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.300) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.100, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
			else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.200) < EPS)
				moveIn3D ('z', pipe1.radAng, 0.050, &pipe1.posX, &pipe1.posY, &pipe1.posZ);

			// 2번 파이프
			pipe2.init (L("비계파이프v1.0.gsm"), layerInd_Rectpipe, infoBeam.floorInd, placingZone->cellsAtRSide [2][beginIndex].leftBottomX, placingZone->cellsAtRSide [2][beginIndex].leftBottomY, placingZone->cellsAtRSide [2][beginIndex].leftBottomZ, placingZone->cellsAtRSide [2][beginIndex].ang);
			moveIn3D ('x', pipe2.radAng, -0.050, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			moveIn3D ('y', pipe2.radAng, 0.0635 + 0.025, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.600 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.500 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.450 - 0.150, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
			else if (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS)
				moveIn3D ('z', pipe2.radAng, 0.400 - 0.100, &pipe2.posX, &pipe2.posY, &pipe2.posZ);

			while (remainLengthDouble > EPS) {
				if (remainLengthDouble > 6.000)
					lengthDouble = 6.000;
				else
					lengthDouble = remainLengthDouble;

				// 1번 파이프
				moveIn3D ('x', pipe1.radAng, lengthDouble, &pipe1.posX, &pipe1.posY, &pipe1.posZ);
				pipe1.radAng += DegreeToRad (180.0);
				elemList.Push (pipe1.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
				pipe1.radAng -= DegreeToRad (180.0);
				// 2번 파이프
				if ((abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.600) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.500) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.450) < EPS) || (abs (placingZone->cellsAtRSide [2][beginIndex].perLen - 0.400) < EPS)) {
					moveIn3D ('x', pipe2.radAng, lengthDouble, &pipe2.posX, &pipe2.posY, &pipe2.posZ);
					pipe2.radAng += DegreeToRad (180.0);
					elemList.Push (pipe2.placeObject (7, "p_comp", APIParT_CString, "사각파이프", "p_leng", APIParT_Length, format_string ("%f", lengthDouble), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "1.0", "holeDir", APIParT_CString, "정면", "holeDia", APIParT_Length, "0.013", "holeDist", APIParT_Length, "0.050"));
					pipe2.radAng -= DegreeToRad (180.0);
				}

				remainLengthDouble -= 6.000;
			}

			bBeginFound = false;
		}
	}

	// 비계파이프 (하부-L) 배치
	// 비계파이프 (하부-R) 배치
	// 비계파이프 (하부-센터) 배치

	// 핀볼트 (L) 배치
	// 핀볼트 (R) 배치
	// 핀볼트 (하부 배치)

	// 각파이프행거 (L) 배치
	// 각파이프행거 (R) 배치





	// 비계파이프
		// 원장 6000mm, 합판 영역으로 50mm 침범
		// 하부
			// 기본적으로 아웃코너앵글 밑에 부착 (타공X)
			// 600 폼은 센터에 파이프 가로지름, 폼 2개 이상인 경우 경계에 파이프 가로지름 (타공O)

	// 핀볼트 (측면에만)
		// 단, 하부의 너비가 600 이상이면 가운데 각파이프에 적용됨

	// 각파이프행거 (하부에만)
		// 아웃코너앵글과 연결되는 각파이프만 사용

	// 블루클램프
		// 각파이프를 피해서 위/아래 2개 부착할 것
		// 객체 속성은 예제 참조

	// 블루목심
		// 1200: (150) 부착 (750) 부착 (300)
		// 900: (450) 부착 (450)
		// 600: (150) 부착 (450)
		// 객체 속성은 예제 참조

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
			DGSetItemText (dialogID, CHECKBOX_TIMBER_LSIDE, "합판/각재");
			DGSetItemText (dialogID, CHECKBOX_T_FORM_LSIDE, "유로폼");
			DGSetItemText (dialogID, CHECKBOX_FILLER_LSIDE, "휠러");
			DGSetItemText (dialogID, CHECKBOX_B_FORM_LSIDE, "유로폼");

			DGSetItemText (dialogID, LABEL_REST_RSIDE, "나머지");
			DGSetItemText (dialogID, CHECKBOX_TIMBER_RSIDE, "합판/각재");
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

			// 유로폼 후크는 사용하지 않음
			DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
			DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);

			break;
		
		case DG_MSG_CHANGE:
			// 왼쪽 간격이 바뀌면 오른쪽 간격도 동일하게 바뀜
			DGSetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2, DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1));

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

					///////////////////////////////////////////////////////////////// 보 양끝
					placingZone.beginCellAtLSide.perLen = placingZone.areaHeight_Left + placingZone.gapBottom;
					placingZone.beginCellAtRSide.perLen = placingZone.areaHeight_Right + placingZone.gapBottom;
					placingZone.beginCellAtBottom.perLen = placingZone.areaWidth_Bottom + (placingZone.gapSide * 2);

					placingZone.endCellAtLSide.perLen = placingZone.areaHeight_Left + placingZone.gapBottom;
					placingZone.endCellAtRSide.perLen = placingZone.areaHeight_Right + placingZone.gapBottom;
					placingZone.endCellAtBottom.perLen = placingZone.areaWidth_Bottom + (placingZone.gapSide * 2);

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
					//layerInd_EuroformHook		= makeTemporaryLayer (structuralObject_forTableformBeam, "HOOK", NULL);
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
					DGSetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE));

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
	short	xx;
	short	itmPosX, itmPosY;
	double	lengthDouble;
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
			DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 0.090);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 2.440);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END, 0.200);

			// 일반 셀: 기본값은 유로폼
			itmPosX = 120+70;
			itmPosY = 72;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				// 버튼
				placingZone.BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
				DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], "유로폼");
				DGShowItem (dialogID, placingZone.BUTTON_OBJ [xx]);
				DGDisableItem (dialogID, placingZone.BUTTON_OBJ [xx]);

				// 객체 타입 (팝업컨트롤)
				placingZone.POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "없음");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "유로폼");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "합판");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

				// 너비 (팝업컨트롤)
				placingZone.POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1200");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "900");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "600");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "0");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

				// 너비 (Edit컨트롤컨트롤) - 처음에는 숨김
				placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
				DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
				DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.090);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 2.440);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.200);

				itmPosX += 70;
			}

			// 오른쪽 끝 여백 채우기 여부 (체크박스)
			placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, "합판\n채우기");
			DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
			// 오른쪽 끝 여백 길이 (Edit컨트롤)
			placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.090);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 2.440);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.200);

			// 총 길이, 남은 길이 표시
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LENGTH, placingZone.beamLength);
			lengthDouble = placingZone.beamLength;
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
					lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1)
					lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_LENGTH);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_LENGTH);

			// 다이얼로그 크기 설정
			dialogSizeX = 500;
			dialogSizeY = 360;
			if (placingZone.nCells >= 4) {
				DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 3), dialogSizeY, DG_TOPLEFT, true);
			}

			break;

		case DG_MSG_CHANGE:

			// 여백 채우기 버튼 체크 유무
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)
				DGEnableItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			else
				DGDisableItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);

			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)
				DGEnableItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			else
				DGDisableItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);

			// 객체 타입 변경시
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (item == placingZone.POPUP_OBJ_TYPE [xx]) {
					// 해당 버튼의 이름 변경
					DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], DGPopUpGetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx])));

					// 없음이면 모두 숨김, 유로폼이면 팝업 표시, 합판이면 Edit컨트롤 표시
					if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE + 1) {
						DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1) {
						DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1) {
						DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					}
				}
			}

			// 남은 길이 표시
			lengthDouble = placingZone.beamLength;
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
			if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
					lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1)
					lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);

			break;

		case DG_MSG_CLICK:

			switch (item) {
				case DG_PREV:
					clickedPrevButton = true;
					break;

				case DG_OK:
					clickedOKButton = true;

					// 합판 채우기 정보 저장
					if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE) {
						placingZone.bFillMarginBegin = true;
						placingZone.marginBegin = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
					} else {
						placingZone.bFillMarginBegin = false;
						placingZone.marginBegin = 0.0;
					}

					if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE) {
						placingZone.bFillMarginEnd = true;
						placingZone.marginEnd = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
					} else {
						placingZone.bFillMarginEnd = false;
						placingZone.marginEnd = 0.0;
					}

					placingZone.beginCellAtLSide.dirLen = placingZone.marginBegin;
					placingZone.beginCellAtRSide.dirLen = placingZone.marginBegin;
					placingZone.beginCellAtBottom.dirLen = placingZone.marginBegin;

					placingZone.endCellAtLSide.dirLen = placingZone.marginEnd;
					placingZone.endCellAtRSide.dirLen = placingZone.marginEnd;
					placingZone.endCellAtBottom.dirLen = placingZone.marginEnd;

					// 셀 정보 저장
					for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE + 1) {
							placingZone.cellsAtLSide [0][xx].objType = NONE;
							placingZone.cellsAtLSide [0][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [0][xx].perLen = 0.0;
							placingZone.cellsAtLSide [1][xx].objType = NONE;
							placingZone.cellsAtLSide [1][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [1][xx].perLen = 0.0;
							placingZone.cellsAtLSide [2][xx].objType = NONE;
							placingZone.cellsAtLSide [2][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [2][xx].perLen = 0.0;
							placingZone.cellsAtLSide [3][xx].objType = NONE;
							placingZone.cellsAtLSide [3][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [3][xx].perLen = 0.0;

							placingZone.cellsAtRSide [0][xx].objType = NONE;
							placingZone.cellsAtRSide [0][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [0][xx].perLen = 0.0;
							placingZone.cellsAtRSide [1][xx].objType = NONE;
							placingZone.cellsAtRSide [1][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [1][xx].perLen = 0.0;
							placingZone.cellsAtRSide [2][xx].objType = NONE;
							placingZone.cellsAtRSide [2][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [2][xx].perLen = 0.0;
							placingZone.cellsAtRSide [3][xx].objType = NONE;
							placingZone.cellsAtRSide [3][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [3][xx].perLen = 0.0;

							placingZone.cellsAtBottom [0][xx].objType = NONE;
							placingZone.cellsAtBottom [0][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [0][xx].perLen = 0.0;
							placingZone.cellsAtBottom [1][xx].objType = NONE;
							placingZone.cellsAtBottom [1][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [1][xx].perLen = 0.0;
							placingZone.cellsAtBottom [2][xx].objType = NONE;
							placingZone.cellsAtBottom [2][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [2][xx].perLen = 0.0;

						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1) {
							lengthDouble = atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;

							placingZone.cellsAtLSide [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtLSide [1][xx].dirLen = lengthDouble;
							placingZone.cellsAtLSide [2][xx].dirLen = lengthDouble;
							placingZone.cellsAtLSide [3][xx].dirLen = lengthDouble;

							placingZone.cellsAtRSide [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtRSide [1][xx].dirLen = lengthDouble;
							placingZone.cellsAtRSide [2][xx].dirLen = lengthDouble;
							placingZone.cellsAtRSide [3][xx].dirLen = lengthDouble;

							placingZone.cellsAtBottom [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtBottom [1][xx].dirLen = lengthDouble;
							placingZone.cellsAtBottom [2][xx].dirLen = lengthDouble;

						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1) {
							lengthDouble = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);

							placingZone.cellsAtLSide [0][xx].objType = PLYWOOD;
							placingZone.cellsAtLSide [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtLSide [0][xx].perLen = placingZone.areaHeight_Left + placingZone.gapBottom;
							placingZone.cellsAtLSide [1][xx].objType = NONE;
							placingZone.cellsAtLSide [1][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [1][xx].perLen = 0.0;
							placingZone.cellsAtLSide [2][xx].objType = NONE;
							placingZone.cellsAtLSide [2][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [2][xx].perLen = 0.0;
							placingZone.cellsAtLSide [3][xx].objType = NONE;
							placingZone.cellsAtLSide [3][xx].dirLen = 0.0;
							placingZone.cellsAtLSide [3][xx].perLen = 0.0;

							placingZone.cellsAtRSide [0][xx].objType = PLYWOOD;
							placingZone.cellsAtRSide [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtRSide [0][xx].perLen = placingZone.areaHeight_Right + placingZone.gapBottom;
							placingZone.cellsAtRSide [1][xx].objType = NONE;
							placingZone.cellsAtRSide [1][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [1][xx].perLen = 0.0;
							placingZone.cellsAtRSide [2][xx].objType = NONE;
							placingZone.cellsAtRSide [2][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [2][xx].perLen = 0.0;
							placingZone.cellsAtRSide [3][xx].objType = NONE;
							placingZone.cellsAtRSide [3][xx].dirLen = 0.0;
							placingZone.cellsAtRSide [3][xx].perLen = 0.0;

							placingZone.cellsAtBottom [0][xx].objType = PLYWOOD;
							placingZone.cellsAtBottom [0][xx].dirLen = lengthDouble;
							placingZone.cellsAtBottom [0][xx].perLen = placingZone.areaWidth_Bottom + (placingZone.gapSide * 2);
							placingZone.cellsAtBottom [1][xx].objType = NONE;
							placingZone.cellsAtBottom [1][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [1][xx].perLen = 0.0;
							placingZone.cellsAtBottom [2][xx].objType = NONE;
							placingZone.cellsAtBottom [2][xx].dirLen = 0.0;
							placingZone.cellsAtBottom [2][xx].perLen = 0.0;
						}
					}

					break;

				case DG_CANCEL:
					break;

				case BUTTON_ADD_COL:
					item = 0;

					if (placingZone.nCells < maxCol) {
						// 오른쪽 끝 여백 채우기 버튼을 지우고
						DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);

						// 마지막 셀 버튼 오른쪽에 새로운 셀 버튼을 추가하고
						itmPosX = 120+70 + (70 * placingZone.nCells);
						itmPosY = 72;
						// 버튼
						placingZone.BUTTON_OBJ [placingZone.nCells] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
						DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells], DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells], "유로폼");
						DGShowItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells]);
						DGDisableItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells]);

						// 객체 타입 (팝업컨트롤)
						placingZone.POPUP_OBJ_TYPE [placingZone.nCells] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
						DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_IS_EXTRASMALL | DG_IS_PLAIN);
						DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM, "없음");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM, "유로폼");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_BOTTOM, "합판");
						DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells], DG_POPUP_TOP+1);
						DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells]);

						// 너비 (팝업컨트롤)
						placingZone.POPUP_WIDTH [placingZone.nCells] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
						DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_IS_LARGE | DG_IS_PLAIN);
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "1200");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "900");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "600");
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_BOTTOM, "0");
						DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], DG_POPUP_TOP);
						DGShowItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells]);
						DGSetItemMinDouble (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], 0.090);
						DGSetItemMaxDouble (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells], 2.440);

						// 너비 (Edit컨트롤) - 처음에는 숨김
						placingZone.EDITCONTROL_WIDTH [placingZone.nCells] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
						DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCells], 0.200);

						itmPosX += 70;

						// 오른쪽 끝 여백 채우기 버튼을 오른쪽 끝에 붙임
						// 오른쪽 끝 여백 채우기 여부 (체크버튼)
						placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
						DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, "합판\n채우기");
						DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
						// 오른쪽 끝 여백 길이 (Edit컨트롤)
						placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
						DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.090);
						DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 2.440);
						DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.200);

						++placingZone.nCells;
						
						// 남은 길이 표시
						lengthDouble = placingZone.beamLength;
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
							if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
								lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
							else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1)
								lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						}
						DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);

						// 다이얼로그 크기 설정
						dialogSizeX = 500;
						dialogSizeY = 360;
						if (placingZone.nCells >= 4) {
							DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 3), dialogSizeY, DG_TOPLEFT, true);
						}
					}

					break;

				case BUTTON_DEL_COL:
					item = 0;

					if (placingZone.nCells > 1) {
						// 오른쪽 끝 여백 채우기 버튼을 지우고
						DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);

						// 마지막 셀 버튼을 지우고
						DGRemoveDialogItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCells - 1]);
						DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCells - 1]);

						// 오른쪽 끝 여백 채우기 버튼을 오른쪽 끝에 붙임
						itmPosX = 120+70 + (70 * (placingZone.nCells - 1));
						itmPosY = 72;
						// 오른쪽 끝 여백 채우기 여부 (체크버튼)
						placingZone.CHECKBOX_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 70, 70, 70);
						DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, DG_IS_LARGE | DG_IS_PLAIN);
						DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, "합판\n채우기");
						DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END);
						DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END, TRUE);
						// 오른쪽 끝 여백 길이 (Edit컨트롤)
						placingZone.EDITCONTROL_MARGIN_RIGHT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 140, 70, 25);
						DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END, 0.200);

						--placingZone.nCells;

						// 남은 길이 표시
						lengthDouble = placingZone.beamLength;
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);
						if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_RIGHT_END) == TRUE)	lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT_END);
						for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
							if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM + 1)
								lengthDouble -= atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000.0;
							else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD + 1)
								lengthDouble -= DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						}
						DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, lengthDouble);

						// 다이얼로그 크기 설정
						dialogSizeX = 500;
						dialogSizeY = 360;
						if (placingZone.nCells >= 4) {
							DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCells - 3), dialogSizeY, DG_TOPLEFT, true);
						}
					}

					break;
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
