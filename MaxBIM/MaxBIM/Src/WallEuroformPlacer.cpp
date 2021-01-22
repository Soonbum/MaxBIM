#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "WallEuroformPlacer.hpp"

using namespace wallPlacerDG;

static WallPlacingZone	placingZone;			// 기본 벽면 영역 정보
static WallPlacingZone	placingZoneBackside;	// 반대쪽 벽면에도 벽면 영역 정보 부여, 벽 기준으로 대칭됨 (placingZone과 달리 오른쪽부터 객체를 설치함)
static InfoWall			infoWall;				// 벽 객체 정보
static short			clickedBtnItemIdx;		// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
static bool				clickedOKButton;		// OK 버튼을 눌렀습니까?
static bool				clickedPrevButton;		// 이전 버튼을 눌렀습니까?
static short			layerInd_Incorner;		// 레이어 번호: 인코너
static short			layerInd_Euroform;		// 레이어 번호: 유로폼
static short			layerInd_Fillerspacer;	// 레이어 번호: 휠러스페이서
static short			layerInd_Plywood;		// 레이어 번호: 합판
static short			layerInd_Wood;			// 레이어 번호: 목재
static short			itemInitIdx = GRIDBUTTON_IDX_START;		// 그리드 버튼 항목 인덱스 시작번호
static short			numberOfinterfereBeam;	// 몇 번째 간섭 보인가?
static GS::Array<API_Guid>	elemList;			// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함


// 1번 메뉴: 벽에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx, yy;
	double		dx, dy, ang1, ang2;
	double		xPosLB, yPosLB, zPosLB;
	double		xPosRT, yPosRT, zPosRT;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	walls = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nWalls = 0;
	long					nMorphs = 0;
	long					nBeams = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 객체 정보
	InfoMorphForWall		infoMorph;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_wall;		// 벽의 작업 층 높이
	double			workLevel_beam;		// 보의 작업 층 높이


	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);	// 선택한 요소 가져오기
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 벽 (1개), 벽을 덮는 모프 (1개)\n옵션 선택: 모프 위쪽과 맞닿는 보 (다수)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 벽 1개, 모프 1개, 보 0개 이상 선택해야 함
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

			if (tElem.header.typeID == API_BeamID)		// 보인가?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nWalls = walls.GetSize ();
	nMorphs = morphs.GetSize ();
	nBeams = beams.GetSize ();

	// 벽이 1개인가?
	if (nWalls != 1) {
		ACAPI_WriteReport ("벽을 1개 선택해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("벽을 덮는 모프를 1개 선택하셔야 합니다.", true);
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
	placingZone.leftBottomX		= infoMorph.leftBottomX;
	placingZone.leftBottomY		= infoMorph.leftBottomY;
	placingZone.leftBottomZ		= infoMorph.leftBottomZ;
	placingZone.horLen			= infoMorph.horLen;
	placingZone.verLen			= infoMorph.verLen;
	placingZone.ang				= DegreeToRad (infoMorph.ang);
	placingZone.nInterfereBeams	= (short)nBeams;
	
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
	placingZone.leftBottomZ = infoWall.bottomOffset;

	// 영역 모프 제거
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// (3) 선택한 보가 있다면,
	for (xx = 0 ; xx < nBeams ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = beams.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		// 벽 면에 가장 가까운 쪽에 있는 점을 찾는다
		dx = elem.beam.begC.x - infoMorph.leftBottomX;
		dy = elem.beam.begC.y - infoMorph.leftBottomY;
		ang1 = RadToDegree (atan2 (dy, dx));	// 보 시작점과 벽 좌하단점 간의 각도
		dx = elem.beam.endC.x - infoMorph.leftBottomX;
		dy = elem.beam.endC.y - infoMorph.leftBottomY;
		ang2 = RadToDegree (atan2 (dy, dx));	// 보 끝점과 벽 좌하단점 간의 각도

		if (abs (infoMorph.ang - ang1) < abs (infoMorph.ang - ang2)) {
			// 보의 LeftBottom 좌표
			xPosLB = elem.beam.begC.x - elem.beam.width/2 * cos(DegreeToRad (infoMorph.ang));
			yPosLB = elem.beam.begC.y - elem.beam.width/2 * sin(DegreeToRad (infoMorph.ang));
			zPosLB = elem.beam.level - elem.beam.height;

			// 보의 RightTop 좌표
			xPosRT = elem.beam.begC.x + elem.beam.width/2 * cos(DegreeToRad (infoMorph.ang));
			yPosRT = elem.beam.begC.y + elem.beam.width/2 * sin(DegreeToRad (infoMorph.ang));
			zPosRT = elem.beam.level;
		} else {
			// 보의 LeftBottom 좌표
			xPosLB = elem.beam.endC.x - elem.beam.width/2 * cos(DegreeToRad (infoMorph.ang));
			yPosLB = elem.beam.endC.y - elem.beam.width/2 * sin(DegreeToRad (infoMorph.ang));
			zPosLB = elem.beam.level - elem.beam.height;

			// 보의 RightTop 좌표
			xPosRT = elem.beam.endC.x + elem.beam.width/2 * cos(DegreeToRad (infoMorph.ang));
			yPosRT = elem.beam.endC.y + elem.beam.width/2 * sin(DegreeToRad (infoMorph.ang));
			zPosRT = elem.beam.level;
		}

		placingZone.beams [xx].floorInd		= elem.header.floorInd;
		placingZone.beams [xx].bottomOffset	= elem.beam.level;
		placingZone.beams [xx].leftBottomX	= xPosLB;
		placingZone.beams [xx].leftBottomY	= yPosLB;
		placingZone.beams [xx].leftBottomZ	= zPosLB;
		placingZone.beams [xx].horLen		= elem.beam.width;
		placingZone.beams [xx].verLen		= elem.beam.height;

		// 작업 층 높이 반영 -- 보
		BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
		workLevel_beam = 0.0;
		ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
		for (yy = 0 ; yy < (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
			if (storyInfo.data [0][yy].index == elem.header.floorInd) {
				workLevel_beam = storyInfo.data [0][yy].level;
				break;
			}
		}
		BMKillHandle ((GSHandle *) &storyInfo.data);

		// 보의 고도 정보를 수정
		placingZone.beams [xx].leftBottomZ = placingZone.leftBottomZ + placingZone.verLen - elem.beam.height;

		ACAPI_DisposeElemMemoHdls (&memo);
	}

FIRST:

	// [DIALOG] 1번째 다이얼로그에서 인코너, 유로폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32511, ACAPI_GetOwnResModule (), wallPlacerHandler1, 0);

	// 벽과의 간격으로 인해 정보 업데이트
	infoWall.wallThk		+= (placingZone.gap * 2);
	for (xx = 0 ; xx < placingZone.nInterfereBeams ; ++xx) {
		placingZone.beams [xx].leftBottomX += (placingZone.gap * sin(placingZone.ang));
		placingZone.beams [xx].leftBottomY -= (placingZone.gap * cos(placingZone.ang));
	}

	// 문자열로 된 유로폼의 너비/높이를 실수형으로도 저장
	placingZone.eu_wid_numeric = atof (placingZone.eu_wid.c_str ()) / 1000.0;
	placingZone.eu_hei_numeric = atof (placingZone.eu_hei.c_str ()) / 1000.0;

	// 남은 길이 초기화
	placingZone.remain_hor = placingZone.horLen;
	placingZone.remain_ver = placingZone.verLen;

	if (result != DG_OK)
		return err;

	// 인코너 양쪽 길이만큼 차감
	if (placingZone.bLIncorner == true)		placingZone.remain_hor = placingZone.remain_hor - placingZone.lenLIncorner;
	if (placingZone.bRIncorner == true)		placingZone.remain_hor = placingZone.remain_hor - placingZone.lenRIncorner;

	// 간섭보의 영향을 받지 않는 세로 길이 업데이트
	placingZone.remain_ver_wo_beams = placingZone.verLen;
	if (placingZone.nInterfereBeams > 0) {
		for (xx = 0 ; xx < placingZone.nInterfereBeams ; ++xx) {
			if (placingZone.remain_ver_wo_beams > placingZone.beams [xx].leftBottomZ)
				placingZone.remain_ver_wo_beams = placingZone.beams [xx].leftBottomZ;
		}
	}
	placingZone.remain_ver_wo_beams = placingZone.remain_ver_wo_beams;
	placingZone.remain_ver = placingZone.remain_ver_wo_beams;

	// 유로폼 가로/세로 방향 개수 세기
	placingZone.eu_count_hor = 0;
	placingZone.eu_count_ver = 0;

	if (placingZone.eu_ori.compare (std::string ("벽세우기")) == 0) {
		placingZone.eu_count_hor = static_cast<short>((placingZone.remain_hor + EPS) / placingZone.eu_wid_numeric);		// 가로 방향 개수
		placingZone.remain_hor = placingZone.remain_hor - (placingZone.eu_count_hor * placingZone.eu_wid_numeric);		// 가로 방향 나머지
		placingZone.eu_count_ver = static_cast<short>((placingZone.remain_ver + EPS) / placingZone.eu_hei_numeric);		// 세로 방향 개수
		placingZone.remain_ver = placingZone.remain_ver - (placingZone.eu_count_ver * placingZone.eu_hei_numeric);		// 세로 방향 나머지
	} else {
		placingZone.eu_count_hor = static_cast<short>((placingZone.remain_hor + EPS) / placingZone.eu_hei_numeric);		// 가로 방향 개수
		placingZone.remain_hor = placingZone.remain_hor - (placingZone.eu_count_hor * placingZone.eu_hei_numeric);		// 가로 방향 나머지
		placingZone.eu_count_ver = static_cast<short>((placingZone.remain_ver + EPS) / placingZone.eu_wid_numeric);		// 세로 방향 개수
		placingZone.remain_ver = placingZone.remain_ver - (placingZone.eu_count_ver * placingZone.eu_wid_numeric);		// 세로 방향 나머지
	}

	// 가로 나머지 길이 값 분리 (고정값, 변동값)
	placingZone.remain_hor_updated = placingZone.remain_hor;

	// placingZone의 Cell 정보 초기화
	placingZone.nCells = (placingZone.eu_count_hor * 2) + 3;
	initCellsForWall (&placingZone);
	
	// 반대쪽 벽에 대한 벽면 영역 정보 초기화
	initCellsForWall (&placingZoneBackside);
	placingZoneBackside.nCells = placingZone.nCells;

	// 배치를 위한 정보 입력
	firstPlacingSettingsForWall (&placingZone);
	copyPlacingZoneSymmetricForWall (&placingZone, &placingZoneBackside, &infoWall);

	// [DIALOG] 2번째 다이얼로그에서 유로폼/인코너 배치를 수정하거나 휠러스페이서 등을 삽입합니다.
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (185, 250, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallPlacerHandler2, 0);

	// 이전 버튼을 누르면 1번째 다이얼로그 다시 실행
	if (clickedPrevButton == true)
		goto FIRST;

	// 2번째 다이얼로그에서 OK 버튼을 눌러야만 다음 단계로 넘어감
	if (clickedOKButton == false)
		return err;

	// 자투리 공간 채우기
	err = fillRestAreasForWall ();

	// [DIALOG] 4번째 다이얼로그에서 채워진 자투리 공간 중에서 합판 영역을 다른 규격의 유로폼으로 대체할 것인지 묻습니다.
	for (xx = 0 ; xx < placingZone.nInterfereBeams ; ++xx) {
		numberOfinterfereBeam = xx;
		result = DGBlankModalDialog (300, 320, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallPlacerHandler4, 0);
	}
	
	// [DIALOG] 5번째 다이얼로그에서 벽 상단의 자투리 공간을 다른 규격의 유로폼으로 대체할 것인지 묻습니다.
	result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallPlacerHandler5, 0);

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

	return	err;
}

// 가로 채우기까지 완료된 후 자투리 공간 채우기
GSErrCode	fillRestAreasForWall (void)
{
	GSErrCode	err = NoError;

	short	xx, yy, zz;
	short	indInterfereBeam;		// 중첩되는 보의 인덱스 (-1은 중첩 없음)

	double	cellLeftX = 0.0, cellRightX = 0.0;	// 셀의 L/R측 X 좌표
	double	beamLeftX = 0.0, beamRightX = 0.0;	// 보의 L/R측 X 좌표
	double	dist;

	CellForWall		insCell, insCellB;		// 삽입할 임시 셀
	double	insertedHeight;			// 회전시킨 유로폼(눕힘) 삽입으로 인해 추가된 높이
	double	insertedLeft;			// 회전시킨 유로폼(세움) 삽입으로 인해 추가된 거리
	double	insertedRight;			// 회전시킨 유로폼(세움) 삽입으로 인해 추가된 거리

	for (xx = placingZone.eu_count_ver ; xx < placingZone.eu_count_ver + 2 ; ++xx) {
		for (yy = 0 ; yy < placingZone.nCells ; ++yy) {

			insertedHeight = 0.0;
			insertedLeft = 0.0;
			insertedRight = 0.0;

			// 세로 공간으로 남는 길이가 아예 없으면 루프 종료
			if ( abs (placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen)) < EPS)
				break;

			// 보의 중첩 관계 확인 - 중첩되는 보의 인덱스를 먼저 추출
			indInterfereBeam = -1;
			for (zz = 0 ; zz < placingZone.nInterfereBeams ; ++zz) {

				// 간섭하는 보의 인덱스 번호를 구함
				if (indInterfereBeam == -1) {
					if (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen * 2 >= placingZone.beams [zz].leftBottomZ) {
						dist = GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.cells [xx-1][yy].leftBottomX, placingZone.cells [xx-1][yy].leftBottomY);

						cellLeftX	= dist;
						cellRightX	= dist + placingZone.cells [xx-1][yy].horLen;
						dist		= GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.beams [zz].leftBottomX, placingZone.beams [zz].leftBottomY);
						beamLeftX	= dist;
						beamRightX	= dist + placingZone.beams [zz].horLen;

						// 보가 셀의 오른쪽으로 침범한 경우
						if ( (cellLeftX < beamLeftX) && (beamLeftX < cellRightX) && (cellRightX <= beamRightX) )
							indInterfereBeam = zz;

						// 보가 셀의 왼쪽으로 침범한 경우
						if ( (cellLeftX < beamRightX) && (beamRightX < cellRightX) && (beamLeftX <= cellLeftX) )
							indInterfereBeam = zz;

						// 보가 셀 안에 들어오는 경우
						if ( (cellLeftX < beamLeftX) && (beamRightX < cellRightX) )
							indInterfereBeam = zz;

						// 보가 셀 영역을 다 침범한 경우
						if ( (beamLeftX <= cellLeftX) && (cellRightX <= beamRightX) )
							indInterfereBeam = zz;
					}
				}
			}

			// 간섭이 없으면
			if (indInterfereBeam == -1) {

				// 현재 높이에서 아래 객체들을 올려도 되는 높이인가?
				if ( (placingZone.leftBottomZ + placingZone.verLen >= (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen * 2)) || (abs (placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen * 2)) < EPS) ) {
					placingZone.cells [xx][yy] = placingZone.cells [xx-1][yy];
					placingZone.cells [xx][yy].leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen;
					placingZone.cells [xx][yy].guid = placeLibPartForWall (placingZone.cells [xx][yy]);
					elemList.Push (placingZone.cells [xx][yy].guid);

					placingZoneBackside.cells [xx][yy] = placingZoneBackside.cells [xx-1][yy];
					placingZoneBackside.cells [xx][yy].leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen;
					placingZoneBackside.cells [xx][yy].guid = placeLibPartForWall (placingZoneBackside.cells [xx][yy]);
					elemList.Push (placingZoneBackside.cells [xx][yy].guid);
					
				// 높이가 부족하면
				} else if ( (placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen)) >= 0.010) {

					placingZone.cells [xx][yy] = placingZone.cells [xx-1][yy];
					placingZone.cells [xx][yy].leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen;

					placingZoneBackside.cells [xx][yy] = placingZoneBackside.cells [xx-1][yy];
					placingZoneBackside.cells [xx][yy].leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen;

					// 인코너, 인코너에 인접한 휠러스페이서는 끝까지 올림
					if ( ((placingZone.cells [xx][yy].objType == INCORNER) && ( (yy == 0) ) || (yy == (placingZone.nCells - 1 ))) || ((placingZone.cells [xx][yy].objType == FILLERSPACER) && ( (yy == 1) || (yy == (placingZone.nCells - 2)) )) ) {
						
						if (placingZone.cells [xx][yy].objType == INCORNER) {
							placingZone.cells [xx][yy].libPart.incorner.hei_s = placingZone.leftBottomZ + placingZone.verLen - placingZone.cells [xx][yy].leftBottomZ;
							placingZoneBackside.cells [xx][yy].libPart.incorner.hei_s = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - placingZoneBackside.cells [xx][yy].leftBottomZ;
						}
						if (placingZone.cells [xx][yy].objType == FILLERSPACER) {
							placingZone.cells [xx][yy].libPart.fillersp.f_leng = placingZone.leftBottomZ + placingZone.verLen - placingZone.cells [xx][yy].leftBottomZ;
							placingZoneBackside.cells [xx][yy].libPart.fillersp.f_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - placingZoneBackside.cells [xx][yy].leftBottomZ;
						}

					// 그 외의 객체이면, 목재/합판으로 채우기
					} else {
						if ( ((placingZone.leftBottomZ + placingZone.verLen - placingZone.cells [xx][yy].leftBottomZ) < 0.110) || (placingZone.cells [xx][yy].horLen < 0.110) ) {

							placingZone.cells [xx][yy].objType = WOOD;
							placingZone.cells [xx][yy].libPart.wood.w_w = 0.080;				// 두께: 80mm
							placingZone.cells [xx][yy].libPart.wood.w_leng = placingZone.cells [xx][yy].horLen;
							placingZone.cells [xx][yy].libPart.wood.w_h = placingZone.leftBottomZ + placingZone.verLen - placingZone.cells [xx][yy].leftBottomZ;
							placingZone.cells [xx][yy].libPart.wood.w_ang = 0;

							placingZoneBackside.cells [xx][yy].objType = WOOD;
							placingZoneBackside.cells [xx][yy].libPart.wood.w_w = 0.080;		// 두께: 80mm
							placingZoneBackside.cells [xx][yy].libPart.wood.w_leng = placingZoneBackside.cells [xx][yy].horLen;
							placingZoneBackside.cells [xx][yy].libPart.wood.w_h = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - placingZoneBackside.cells [xx][yy].leftBottomZ;
							placingZoneBackside.cells [xx][yy].libPart.wood.w_ang = 0;

						} else {

							placingZone.cells [xx][yy].objType = PLYWOOD;
							placingZone.cells [xx][yy].libPart.plywood.p_wid = placingZone.cells [xx][yy].horLen;
							placingZone.cells [xx][yy].libPart.plywood.p_leng = placingZone.leftBottomZ + placingZone.verLen - placingZone.cells [xx][yy].leftBottomZ;
							placingZone.cells [xx][yy].libPart.plywood.w_dir_wall = true;

							placingZoneBackside.cells [xx][yy].objType = PLYWOOD;
							placingZoneBackside.cells [xx][yy].libPart.plywood.p_wid = placingZoneBackside.cells [xx][yy].horLen;
							placingZoneBackside.cells [xx][yy].libPart.plywood.p_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - placingZoneBackside.cells [xx][yy].leftBottomZ;
							placingZoneBackside.cells [xx][yy].libPart.plywood.w_dir_wall = true;
						}
					}
						
					placingZone.cells [xx][yy].horLen = placingZone.cells [xx-1][yy].horLen;
					placingZone.cells [xx][yy].verLen = placingZone.leftBottomZ + placingZone.verLen - placingZone.cells [xx][yy].leftBottomZ;
					placingZone.cells [xx][yy].guid = placeLibPartForWall (placingZone.cells [xx][yy]);
					elemList.Push (placingZone.cells [xx][yy].guid);

					placingZoneBackside.cells [xx][yy].horLen = placingZoneBackside.cells [xx-1][yy].horLen;
					placingZoneBackside.cells [xx][yy].verLen = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - placingZoneBackside.cells [xx][yy].leftBottomZ;
					placingZoneBackside.cells [xx][yy].guid = placeLibPartForWall (placingZoneBackside.cells [xx][yy]);
					elemList.Push (placingZoneBackside.cells [xx][yy].guid);

					// 상단 자투리 공간 셀 - 정보 저장
					placingZone.topRestCells [yy] = placingZone.cells [xx][yy];
					placingZoneBackside.topRestCells [yy] = placingZoneBackside.cells [xx][yy];
				}

			// 간섭이 있으면, 보 주변에 합판이나 목재로 채움
			} else {
				if ((placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen) <= placingZone.beams [indInterfereBeam].leftBottomZ) {
					// 보가 셀의 오른쪽으로 침범한 경우
					if ( (cellLeftX < beamLeftX) && (beamLeftX < cellRightX) && (cellRightX <= beamRightX) ) {
						// 현재 셀의 타입이 유로폼(벽세우기)인 경우에 한해 회전시켜 배치 시도
						if ( (placingZone.cells [xx-1][yy].objType == EUROFORM) && (placingZone.cells [xx-1][yy].libPart.form.u_ins_wall == true) ) {
							// 유로폼을 눕혔을 때 여유가 되면,
							if ( (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen) <= placingZone.beams [indInterfereBeam].leftBottomZ) {
								// 유로폼 눕혀서 배치
								insCell = placingZone.cells [xx-1][yy];
								insCell.objType = EUROFORM;
								insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX + (placingZone.cells [xx-1][yy].horLen * cos(placingZone.cells [xx-1][yy].ang));
								insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY + (placingZone.cells [xx-1][yy].horLen * sin(placingZone.cells [xx-1][yy].ang));
								insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen;
								insCell.ang = placingZone.cells [xx-1][yy].ang;
								insCell.libPart.form.u_ins_wall = false;

								insCellB = placingZoneBackside.cells [xx-1][yy];
								insCellB.objType = EUROFORM;
								insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX;
								insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY;
								insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen;
								insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;
								insCellB.libPart.form.u_ins_wall = false;

								elemList.Push (placeLibPartForWall (insCell));
								elemList.Push (placeLibPartForWall (insCellB));

								if (insCell.libPart.form.eu_stan_onoff == true)
									insertedHeight += insCell.libPart.form.eu_wid;
								else
									insertedHeight += insCell.libPart.form.eu_wid2;
							}
						}
							
						// 현재 셀의 타입이 유로폼(벽눕히기)인 경우에 한해 회전시켜 배치 시도
						if ( (placingZone.cells [xx-1][yy].objType == EUROFORM) && (placingZone.cells [xx-1][yy].libPart.form.u_ins_wall == false) ) {
							// 유로폼을 세웠을 때 여유가 되면,
							if ( (placingZone.leftBottomZ + placingZone.verLen >= (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen)) ) {
								// 보 좌측면 너비가 유로폼 짧은쪽 길이 이상이면,
								if ( (beamRightX - cellLeftX) >= placingZone.cells [xx-1][yy].verLen ) {
									// 유로폼 세워서 배치
									insCell = placingZone.cells [xx-1][yy];
									insCell.objType = EUROFORM;
									insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX;
									insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY;
									insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen;
									insCell.ang = placingZone.cells [xx-1][yy].ang;
									insCell.libPart.form.u_ins_wall = true;

									insCellB = placingZoneBackside.cells [xx-1][yy];
									insCellB.objType = EUROFORM;
									insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX + (placingZoneBackside.cells [xx-1][yy].verLen * cos(placingZoneBackside.cells [xx-1][yy].ang));
									insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY + (placingZoneBackside.cells [xx-1][yy].verLen * sin(placingZoneBackside.cells [xx-1][yy].ang));
									insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen;
									insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;
									insCellB.libPart.form.u_ins_wall = true;

									elemList.Push (placeLibPartForWall (insCell));
									elemList.Push (placeLibPartForWall (insCellB));

									// 위에 목재/합판 추가
									insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX;
									insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY;
									insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen;
									insCell.ang = placingZone.cells [xx-1][yy].ang;

									insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX + (placingZoneBackside.cells [xx-1][yy].verLen * cos(placingZoneBackside.cells [xx-1][yy].ang));
									insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY + (placingZoneBackside.cells [xx-1][yy].verLen * sin(placingZoneBackside.cells [xx-1][yy].ang));
									insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + placingZoneBackside.cells [xx-1][yy].horLen;
									insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;

									if ((placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen)) < 0.110) {
										// 폭이 110mm 미만이면 목재
										insCell.objType = WOOD;
										insCell.libPart.wood.w_w = 0.080;		// 두께: 80mm
										insCell.libPart.wood.w_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen);
										insCell.libPart.wood.w_h = placingZone.cells [xx-1][yy].verLen;
										insCell.libPart.wood.w_ang = DegreeToRad (90.0);

										insCellB.objType = WOOD;
										insCellB.libPart.wood.w_w = 0.080;		// 두께: 80mm
										insCellB.libPart.wood.w_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen);
										insCellB.libPart.wood.w_h = placingZone.cells [xx-1][yy].verLen;
										insCellB.libPart.wood.w_ang = DegreeToRad (90.0);
									} else {
										// 폭이 110mm 이상이면 합판
										insCell.objType = PLYWOOD;
										insCell.libPart.plywood.p_wid = placingZone.cells [xx-1][yy].verLen;
										insCell.libPart.plywood.p_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen);
										insCell.libPart.plywood.w_dir_wall = true;

										insCellB.objType = PLYWOOD;
										insCellB.libPart.plywood.p_wid = placingZone.cells [xx-1][yy].verLen;
										insCellB.libPart.plywood.p_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen);
										insCellB.libPart.plywood.w_dir_wall = true;
									}
									elemList.Push (placeLibPartForWall (insCell));
									elemList.Push (placeLibPartForWall (insCellB));

									// 좌표값 수정
									insertedLeft = placingZone.cells [xx-1][yy].verLen;
								}
							}
						}

						// 보 좌측면
						insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX + insertedLeft * cos(placingZone.cells [xx-1][yy].ang);
						insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY + insertedLeft * sin(placingZone.cells [xx-1][yy].ang);
						insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + insertedHeight;
						insCell.ang = placingZone.cells [xx-1][yy].ang;

						insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX + ((placingZoneBackside.cells [xx-1][yy].horLen - (beamLeftX - cellLeftX)) * cos(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY + ((placingZoneBackside.cells [xx-1][yy].horLen - (beamLeftX - cellLeftX)) * sin(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight;
						insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;

						if ((beamLeftX - cellLeftX) < 0.110) {
							// 폭이 110mm 미만이면 목재
							insCell.objType = WOOD;
							insCell.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCell.libPart.wood.w_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + insertedHeight);
							insCell.libPart.wood.w_h = beamLeftX - cellLeftX - insertedLeft;
							insCell.libPart.wood.w_ang = DegreeToRad (90.0);
							insCell.horLen = insCell.libPart.wood.w_h;
							insCell.verLen = insCell.libPart.wood.w_leng;

							insCellB.objType = WOOD;
							insCellB.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCellB.libPart.wood.w_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight);
							insCellB.libPart.wood.w_h = beamLeftX - cellLeftX - insertedLeft;
							insCellB.libPart.wood.w_ang = DegreeToRad (90.0);
							insCellB.horLen = insCellB.libPart.wood.w_h;
							insCellB.verLen = insCellB.libPart.wood.w_leng;
						} else {
							// 폭이 110mm 이상이면 합판
							insCell.objType = PLYWOOD;
							insCell.libPart.plywood.p_wid = beamLeftX - cellLeftX - insertedLeft;
							insCell.libPart.plywood.p_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + insertedHeight);
							insCell.libPart.plywood.w_dir_wall = true;
							insCell.horLen = insCell.libPart.plywood.p_wid;
							insCell.verLen = insCell.libPart.plywood.p_leng;

							insCellB.objType = PLYWOOD;
							insCellB.libPart.plywood.p_wid = beamLeftX - cellLeftX - insertedLeft;
							insCellB.libPart.plywood.p_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight);
							insCellB.libPart.plywood.w_dir_wall = true;
							insCellB.horLen = insCellB.libPart.plywood.p_wid;
							insCellB.verLen = insCellB.libPart.plywood.p_leng;
						}
						insCell.guid = placeLibPartForWall (insCell);
						insCellB.guid = placeLibPartForWall (insCellB);
						elemList.Push (insCell.guid);
						elemList.Push (insCellB.guid);
						placingZone.woods [indInterfereBeam][0] = insCell;
						placingZoneBackside.woods [indInterfereBeam][0] = insCellB;

						// 보 아래면
						insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX - ((beamLeftX - cellLeftX) * cos(placingZoneBackside.cells [xx-1][yy].ang));
						insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY - ((beamLeftX - cellLeftX) * sin(placingZoneBackside.cells [xx-1][yy].ang));
						insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + insertedHeight;
						insCell.ang = placingZone.cells [xx-1][yy].ang;

						insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX + ((cellRightX - beamRightX) * cos(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY + ((cellRightX - beamRightX) * sin(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight;
						insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;

						if ((placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen)) < 0.110) {
							// 폭이 110mm 미만이면 목재
							insCell.objType = WOOD;
							insCell.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCell.libPart.wood.w_leng = placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight);
							insCell.libPart.wood.w_h = beamRightX - beamLeftX;
							insCell.libPart.wood.w_ang = DegreeToRad (90.0);
							insCell.horLen = insCell.libPart.wood.w_h;
							insCell.verLen = insCell.libPart.wood.w_leng;

							insCellB.objType = WOOD;
							insCellB.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCellB.libPart.wood.w_leng = placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight);
							insCellB.libPart.wood.w_h = beamRightX - beamLeftX;
							insCellB.libPart.wood.w_ang = DegreeToRad (90.0);
							insCellB.horLen = insCellB.libPart.wood.w_h;
							insCellB.verLen = insCellB.libPart.wood.w_leng;
						} else {
							// 폭이 110mm 이상이면 합판
							insCell.objType = PLYWOOD;
							insCell.libPart.plywood.p_wid = beamRightX - beamLeftX;
							insCell.libPart.plywood.p_leng = placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight);
							insCell.libPart.plywood.w_dir_wall = true;
							insCell.horLen = insCell.libPart.plywood.p_wid;
							insCell.verLen = insCell.libPart.plywood.p_leng;

							insCellB.objType = PLYWOOD;
							insCellB.libPart.plywood.p_wid = beamRightX - beamLeftX;
							insCellB.libPart.plywood.p_leng = placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight);
							insCellB.libPart.plywood.w_dir_wall = true;
							insCellB.horLen = insCellB.libPart.plywood.p_wid;
							insCellB.verLen = insCellB.libPart.plywood.p_leng;
						}
						insCell.guid = placeLibPartForWall (insCell);
						insCellB.guid = placeLibPartForWall (insCellB);
						elemList.Push (insCell.guid);
						elemList.Push (insCellB.guid);
						placingZone.woods [indInterfereBeam][1] = insCell;
						placingZoneBackside.woods [indInterfereBeam][1] = insCellB;
					}

					// 보가 셀의 왼쪽으로 침범한 경우
					if ( (cellLeftX < beamRightX) && (beamRightX < cellRightX) && (beamLeftX <= cellLeftX) ) {
						// 현재 셀의 타입이 유로폼(벽세우기)인 경우
						if ( (placingZone.cells [xx-1][yy].objType == EUROFORM) && (placingZone.cells [xx-1][yy].libPart.form.u_ins_wall == true) ) {
							// 유로폼을 눕혔을 때 여유가 되면,
							if ( (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen) <= placingZone.beams [indInterfereBeam].leftBottomZ) {
								// 유로폼 눕혀서 배치되는 것을 대비하여 빈 공간 확보
								insertedHeight += placingZone.cells [xx-1][yy].horLen;
							}
						}

						// 현재 셀의 타입이 유로폼(벽눕히기)인 경우에 한해 회전시켜 배치 시도
						if ( (placingZone.cells [xx-1][yy].objType == EUROFORM) && (placingZone.cells [xx-1][yy].libPart.form.u_ins_wall == false) ) {
							// 유로폼을 세웠을 때 여유가 되면,
							if ( (placingZone.leftBottomZ + placingZone.verLen >= (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen)) ) {
								// 보 우측면 너비가 유로폼 짧은쪽 길이 이상이면,
								if ( (cellRightX - beamRightX) >= placingZone.cells [xx-1][yy].verLen ) {
									// 유로폼을 세워서 배치
									insCell = placingZone.cells [xx-1][yy];
									insCell.objType = EUROFORM;
									insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX + (placingZone.cells [xx-1][yy].verLen * cos(placingZone.cells [xx-1][yy].ang));
									insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY + (placingZone.cells [xx-1][yy].verLen * sin(placingZone.cells [xx-1][yy].ang));
									insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen;
									insCell.ang = placingZone.cells [xx-1][yy].ang;
									insCell.libPart.form.u_ins_wall = true;

									insCellB = placingZoneBackside.cells [xx-1][yy];
									insCellB.objType = EUROFORM;
									insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX;
									insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY;
									insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen;
									insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;
									insCellB.libPart.form.u_ins_wall = true;

									elemList.Push (placeLibPartForWall (insCell));
									elemList.Push (placeLibPartForWall (insCellB));

									// 위에 목재/합판 추가
									insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX + (placingZone.cells [xx-1][yy].verLen * cos(placingZone.cells [xx-1][yy].ang));
									insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY + (placingZone.cells [xx-1][yy].verLen * sin(placingZone.cells [xx-1][yy].ang));
									insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen;
									insCell.ang = placingZone.cells [xx-1][yy].ang;

									insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX;
									insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY;
									insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + placingZoneBackside.cells [xx-1][yy].horLen;
									insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;

									if ( (placingZone.cells [xx-1][yy].verLen < 0.110) || ((placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen)) < 0.110) ) {
										// 폭이 110mm 미만이면 목재
										insCell.objType = WOOD;
										insCell.libPart.wood.w_w = 0.080;		// 두께: 80mm
										insCell.libPart.wood.w_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen);
										insCell.libPart.wood.w_h = placingZone.cells [xx-1][yy].verLen;
										insCell.libPart.wood.w_ang = DegreeToRad (90.0);

										insCellB.objType = WOOD;
										insCellB.libPart.wood.w_w = 0.080;		// 두께: 80mm
										insCellB.libPart.wood.w_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen);
										insCellB.libPart.wood.w_h = placingZone.cells [xx-1][yy].verLen;
										insCellB.libPart.wood.w_ang = DegreeToRad (90.0);
									} else {
										// 폭이 110mm 이상이면 합판
										insCell.objType = PLYWOOD;
										insCell.libPart.plywood.p_wid = placingZone.cells [xx-1][yy].verLen;
										insCell.libPart.plywood.p_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen);
										insCell.libPart.plywood.w_dir_wall = true;

										insCellB.objType = PLYWOOD;
										insCellB.libPart.plywood.p_wid = placingZone.cells [xx-1][yy].verLen;
										insCellB.libPart.plywood.p_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + placingZone.cells [xx-1][yy].horLen);
										insCellB.libPart.plywood.w_dir_wall = true;
									}
									elemList.Push (placeLibPartForWall (insCell));
									elemList.Push (placeLibPartForWall (insCellB));

									// 좌표값 수정
									insertedRight = placingZone.cells [xx-1][yy].verLen;
								}
							}
						}

						// 보 우측면
						insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX - ((cellLeftX - beamRightX) * cos(placingZone.cells [xx-1][yy].ang));
						insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY - ((cellLeftX - beamRightX) * sin(placingZone.cells [xx-1][yy].ang));
						insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + insertedHeight;
						insCell.ang = placingZone.cells [xx-1][yy].ang;

						insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX + (insertedRight * cos(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY + (insertedRight * sin(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight;
						insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;

						if ((cellRightX - beamRightX) < 0.110) {
							// 폭이 110mm 미만이면 목재
							insCell.objType = WOOD;
							insCell.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCell.libPart.wood.w_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + insertedHeight);
							insCell.libPart.wood.w_h = cellRightX - beamRightX - insertedRight;
							insCell.libPart.wood.w_ang = DegreeToRad (90.0);
							insCell.horLen = insCell.libPart.wood.w_h;
							insCell.verLen = insCell.libPart.wood.w_leng;

							insCellB.objType = WOOD;
							insCellB.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCellB.libPart.wood.w_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight);
							insCellB.libPart.wood.w_h = cellRightX - beamRightX - insertedRight;
							insCellB.libPart.wood.w_ang = DegreeToRad (90.0);
							insCellB.horLen = insCellB.libPart.wood.w_h;
							insCellB.verLen = insCellB.libPart.wood.w_leng;
						} else {
							// 폭이 110mm 이상이면 합판
							insCell.objType = PLYWOOD;
							insCell.libPart.plywood.p_wid = cellRightX - beamRightX - insertedRight;
							insCell.libPart.plywood.p_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen + insertedHeight);
							insCell.libPart.plywood.w_dir_wall = true;
							insCell.horLen = insCell.libPart.plywood.p_wid;
							insCell.verLen = insCell.libPart.plywood.p_leng;

							insCellB.objType = PLYWOOD;
							insCellB.libPart.plywood.p_wid = cellRightX - beamRightX - insertedRight;
							insCellB.libPart.plywood.p_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen + insertedHeight);
							insCellB.libPart.plywood.w_dir_wall = true;
							insCellB.horLen = insCellB.libPart.plywood.p_wid;
							insCellB.verLen = insCellB.libPart.plywood.p_leng;
						}
						insCell.guid = placeLibPartForWall (insCell);
						insCellB.guid = placeLibPartForWall (insCellB);
						elemList.Push (insCell.guid);
						elemList.Push (insCellB.guid);
						placingZone.woods [indInterfereBeam][2] = insCell;
						placingZoneBackside.woods [indInterfereBeam][2] = insCellB;
					}

					// 보가 셀 안에 들어오는 경우
					if ( (cellLeftX < beamLeftX) && (beamRightX < cellRightX) ) {

						// 보 좌측면
						insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX;
						insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY;
						insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen;
						insCell.ang = placingZone.cells [xx-1][yy].ang;

						insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX + ((placingZoneBackside.cells [xx-1][yy].horLen - (beamLeftX - cellLeftX)) * cos(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY + ((placingZoneBackside.cells [xx-1][yy].horLen - (beamLeftX - cellLeftX)) * sin(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen;
						insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;

						if ( ((beamLeftX - cellLeftX) < 0.110) || (placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen) < 0.110) ) {
							// 폭이 110mm 미만이면 목재
							insCell.objType = WOOD;
							insCell.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCell.libPart.wood.w_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen);
							insCell.libPart.wood.w_h = beamLeftX - cellLeftX;
							insCell.libPart.wood.w_ang = DegreeToRad (90.0);
							insCell.horLen = insCell.libPart.wood.w_h;
							insCell.verLen = insCell.libPart.wood.w_leng;

							insCellB.objType = WOOD;
							insCellB.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCellB.libPart.wood.w_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen);
							insCellB.libPart.wood.w_h = beamLeftX - cellLeftX;
							insCellB.libPart.wood.w_ang = DegreeToRad (90.0);
							insCellB.horLen = insCellB.libPart.wood.w_h;
							insCellB.verLen = insCellB.libPart.wood.w_leng;
						} else {
							// 폭이 110mm 이상이면 합판
							insCell.objType = PLYWOOD;
							insCell.libPart.plywood.p_wid = beamLeftX - cellLeftX;
							insCell.libPart.plywood.p_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen);
							insCell.libPart.plywood.w_dir_wall = true;
							insCell.horLen = insCell.libPart.plywood.p_wid;
							insCell.verLen = insCell.libPart.plywood.p_leng;

							insCellB.objType = PLYWOOD;
							insCellB.libPart.plywood.p_wid = beamLeftX - cellLeftX;
							insCellB.libPart.plywood.p_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen);
							insCellB.libPart.plywood.w_dir_wall = true;
							insCellB.horLen = insCellB.libPart.plywood.p_wid;
							insCellB.verLen = insCellB.libPart.plywood.p_leng;
						}
						insCell.guid = placeLibPartForWall (insCell);
						insCellB.guid = placeLibPartForWall (insCellB);
						elemList.Push (insCell.guid);
						elemList.Push (insCellB.guid);
						placingZone.woods [indInterfereBeam][0] = insCell;
						placingZoneBackside.woods [indInterfereBeam][0] = insCellB;

						// 보 아래면
						insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX - ((beamLeftX - cellLeftX) * cos(placingZoneBackside.cells [xx-1][yy].ang));
						insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY - ((beamLeftX - cellLeftX) * sin(placingZoneBackside.cells [xx-1][yy].ang));
						insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen;
						insCell.ang = placingZone.cells [xx-1][yy].ang;

						insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX + ((cellRightX - beamRightX) * cos(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY + ((cellRightX - beamRightX) * sin(placingZoneBackside.cells [xx-1][yy].ang));
						insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen;
						insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;

						if ( ((beamRightX - beamLeftX) < 0.110) || ((placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen)) < 0.110) ) {
							// 폭이 110mm 미만이면 목재
							insCell.objType = WOOD;
							insCell.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCell.libPart.wood.w_leng = placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen);
							insCell.libPart.wood.w_h = beamRightX - beamLeftX;
							insCell.libPart.wood.w_ang = DegreeToRad (90.0);
							insCell.horLen = insCell.libPart.wood.w_h;
							insCell.verLen = insCell.libPart.wood.w_leng;

							insCellB.objType = WOOD;
							insCellB.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCellB.libPart.wood.w_leng = placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen);
							insCellB.libPart.wood.w_h = beamRightX - beamLeftX;
							insCellB.libPart.wood.w_ang = DegreeToRad (90.0);
							insCellB.horLen = insCellB.libPart.wood.w_h;
							insCellB.verLen = insCellB.libPart.wood.w_leng;
						} else {
							// 폭이 110mm 이상이면 합판
							insCell.objType = PLYWOOD;
							insCell.libPart.plywood.p_wid = beamRightX - beamLeftX;
							insCell.libPart.plywood.p_leng = placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen);
							insCell.libPart.plywood.w_dir_wall = true;
							insCell.horLen = insCell.libPart.plywood.p_wid;
							insCell.verLen = insCell.libPart.plywood.p_leng;

							insCellB.objType = PLYWOOD;
							insCellB.libPart.plywood.p_wid = beamRightX - beamLeftX;
							insCellB.libPart.plywood.p_leng = placingZone.beams [indInterfereBeam].leftBottomZ - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen);
							insCellB.libPart.plywood.w_dir_wall = true;
							insCellB.horLen = insCellB.libPart.plywood.p_wid;
							insCellB.verLen = insCellB.libPart.plywood.p_leng;
						}
						insCell.guid = placeLibPartForWall (insCell);
						insCellB.guid = placeLibPartForWall (insCellB);
						elemList.Push (insCell.guid);
						elemList.Push (insCellB.guid);
						placingZone.woods [indInterfereBeam][1] = insCell;
						placingZoneBackside.woods [indInterfereBeam][1] = insCellB;

						// 보 우측면
						insCell.leftBottomX = placingZone.cells [xx-1][yy].leftBottomX - ((beamRightX - cellLeftX) * cos(placingZoneBackside.cells [xx-1][yy].ang));
						insCell.leftBottomY = placingZone.cells [xx-1][yy].leftBottomY - ((beamRightX - cellLeftX) * sin(placingZoneBackside.cells [xx-1][yy].ang));
						insCell.leftBottomZ = placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen;
						insCell.ang = placingZone.cells [xx-1][yy].ang;

						insCellB.leftBottomX = placingZoneBackside.cells [xx-1][yy].leftBottomX;
						insCellB.leftBottomY = placingZoneBackside.cells [xx-1][yy].leftBottomY;
						insCellB.leftBottomZ = placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen;
						insCellB.ang = placingZoneBackside.cells [xx-1][yy].ang;

						if ( ((cellRightX - beamRightX) < 0.110) || ((placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen)) < 0.110) )  {
							// 폭이 110mm 미만이면 목재
							insCell.objType = WOOD;
							insCell.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCell.libPart.wood.w_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen);
							insCell.libPart.wood.w_h = cellRightX - beamRightX;
							insCell.libPart.wood.w_ang = DegreeToRad (90.0);
							insCell.horLen = insCell.libPart.wood.w_h;
							insCell.verLen = insCell.libPart.wood.w_leng;

							insCellB.objType = WOOD;
							insCellB.libPart.wood.w_w = 0.080;		// 두께: 80mm
							insCellB.libPart.wood.w_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen);
							insCellB.libPart.wood.w_h = cellRightX - beamRightX;
							insCellB.libPart.wood.w_ang = DegreeToRad (90.0);
							insCellB.horLen = insCellB.libPart.wood.w_h;
							insCellB.verLen = insCellB.libPart.wood.w_leng;
						} else {
							// 폭이 110mm 이상이면 합판
							insCell.objType = PLYWOOD;
							insCell.libPart.plywood.p_wid = cellRightX - beamRightX;
							insCell.libPart.plywood.p_leng = placingZone.leftBottomZ + placingZone.verLen - (placingZone.cells [xx-1][yy].leftBottomZ + placingZone.cells [xx-1][yy].verLen);
							insCell.libPart.plywood.w_dir_wall = true;
							insCell.horLen = insCell.libPart.plywood.p_wid;
							insCell.verLen = insCell.libPart.plywood.p_leng;

							insCellB.objType = PLYWOOD;
							insCellB.libPart.plywood.p_wid = cellRightX - beamRightX;
							insCellB.libPart.plywood.p_leng = placingZoneBackside.leftBottomZ + placingZoneBackside.verLen - (placingZoneBackside.cells [xx-1][yy].leftBottomZ + placingZoneBackside.cells [xx-1][yy].verLen);
							insCellB.libPart.plywood.w_dir_wall = true;
							insCellB.horLen = insCellB.libPart.plywood.p_wid;
							insCellB.verLen = insCellB.libPart.plywood.p_leng;
						}
						insCell.guid = placeLibPartForWall (insCell);
						insCellB.guid = placeLibPartForWall (insCellB);
						elemList.Push (insCell.guid);
						elemList.Push (insCellB.guid);
						placingZone.woods [indInterfereBeam][2] = insCell;
						placingZoneBackside.woods [indInterfereBeam][2] = insCellB;
					}

					// 보가 셀 영역을 다 침범한 경우
					if ( (beamLeftX <= cellLeftX) && (cellRightX <= beamRightX) ) {
						// 대응하지 않음
					}
				}
			}
		}
	}

	return err;
}

// 해당 셀 정보를 기반으로 라이브러리 배치
API_Guid	placeLibPartForWall (CellForWall objInfo)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const	GS::uchar_t* gsmName = NULL;
	double	aParam;
	double	bParam;
	Int32	addParNum;

	std::string		tempString;

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
	if (objInfo.objType == INCORNER)		gsmName = L("인코너판넬v1.0.gsm");
	if (objInfo.objType == EUROFORM)		gsmName = L("유로폼v2.0.gsm");
	if (objInfo.objType == FILLERSPACER)	gsmName = L("휠러스페이서v1.0.gsm");
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
	element.header.floorInd = infoWall.floorInd;

	if (objInfo.objType == INCORNER) {
		element.header.layer = layerInd_Incorner;
		memo.params [0][27].value.real = objInfo.libPart.incorner.wid_s;	// 가로(빨강)
		memo.params [0][28].value.real = objInfo.libPart.incorner.leng_s;	// 세로(파랑)
		memo.params [0][29].value.real = objInfo.libPart.incorner.hei_s;	// 높이
		GS::ucscpy (memo.params [0][30].value.uStr, L("세우기"));			// 설치방향

	} else if (objInfo.objType == EUROFORM) {
		element.header.layer = layerInd_Euroform;

		// 규격품일 경우,
		if (objInfo.libPart.form.eu_stan_onoff == true) {
			// 규격폼 On/Off
			memo.params [0][27].value.real = TRUE;

			// 너비
			tempString = format_string ("%.0f", objInfo.libPart.form.eu_wid * 1000);
			GS::ucscpy (memo.params [0][28].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

			// 높이
			tempString = format_string ("%.0f", objInfo.libPart.form.eu_hei * 1000);
			GS::ucscpy (memo.params [0][29].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

		// 비규격품일 경우,
		} else {
			// 규격폼 On/Off
			memo.params [0][27].value.real = FALSE;

			// 너비
			memo.params [0][30].value.real = objInfo.libPart.form.eu_wid2;

			// 높이
			memo.params [0][31].value.real = objInfo.libPart.form.eu_hei2;
		}

		// 설치방향
		if (objInfo.libPart.form.u_ins_wall == true) {
			tempString = "벽세우기";
		} else {
			tempString = "벽눕히기";
			element.object.pos.x += ( objInfo.horLen * cos(objInfo.ang) );
			element.object.pos.y += ( objInfo.horLen * sin(objInfo.ang) );
		}
		GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());
		
		// 회전X
		memo.params [0][33].value.real = DegreeToRad (90.0);

	} else if (objInfo.objType == FILLERSPACER) {
		element.header.layer = layerInd_Fillerspacer;
		memo.params [0][27].value.real = objInfo.libPart.fillersp.f_thk;	// 두께
		memo.params [0][28].value.real = objInfo.libPart.fillersp.f_leng;	// 길이
		element.object.pos.x += ( objInfo.libPart.fillersp.f_thk * cos(objInfo.ang) );
		element.object.pos.y += ( objInfo.libPart.fillersp.f_thk * sin(objInfo.ang) );

	} else if (objInfo.objType == PLYWOOD) {
		element.header.layer = layerInd_Plywood;
		GS::ucscpy (memo.params [0][32].value.uStr, L("비규격"));
		memo.params [0][35].value.real = objInfo.libPart.plywood.p_wid;		// 가로
		memo.params [0][36].value.real = objInfo.libPart.plywood.p_leng;	// 세로
		
		// 설치방향
		if (objInfo.libPart.plywood.w_dir_wall == true)
			tempString = "벽세우기";
		else
			tempString = "벽눕히기";
		GS::ucscpy (memo.params [0][33].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());
	
	} else if (objInfo.objType == WOOD) {
		element.header.layer = layerInd_Wood;
		GS::ucscpy (memo.params [0][27].value.uStr, L("벽세우기"));		// 설치방향
		memo.params [0][28].value.real = objInfo.libPart.wood.w_w;		// 두께
		memo.params [0][29].value.real = objInfo.libPart.wood.w_h;		// 너비
		memo.params [0][30].value.real = objInfo.libPart.wood.w_leng;	// 길이
		memo.params [0][31].value.real = objInfo.libPart.wood.w_ang;	// 각도

		// 목재가 세로로 길게 배치될 경우
		if ( abs (RadToDegree (objInfo.libPart.wood.w_ang) - 90.0) < EPS ) {
			element.object.pos.x += ( objInfo.libPart.wood.w_h * cos(objInfo.ang) );
			element.object.pos.y += ( objInfo.libPart.wood.w_h * sin(objInfo.ang) );
		}
	}

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// [arr1]행 - 해당 셀의 좌하단 좌표X 위치를 리턴
double	getCellPositionLeftBottomXForWall (WallPlacingZone *src_zone, short arr1, short idx)
{
	double		distance = 0.0;
	short		xx;

	for (xx = 0 ; xx < idx ; ++xx) {
		if (src_zone->cells [arr1][xx].objType != NONE)
			distance += src_zone->cells [arr1][xx].horLen;
	}

	return distance;
}

// [arr1]행 - 전체 셀의 최하단 좌표Z 위치를 설정
void	setCellPositionLeftBottomZForWall (WallPlacingZone *src_zone, short arr1, double new_hei)
{
	short		xx;

	for (xx = 0 ; xx < src_zone->nCells ; ++xx)
		src_zone->cells [arr1][xx].leftBottomZ = new_hei;
}

// Cell 배열을 초기화함
void	initCellsForWall (WallPlacingZone* placingZone)
{
	short xx, yy;

	for (xx = 0 ; xx < 50 ; ++xx)
		for (yy = 0 ; yy < 100 ; ++yy) {
			placingZone->cells [xx][yy].objType = NONE;
			placingZone->cells [xx][yy].ang = 0.0;
			placingZone->cells [xx][yy].horLen = 0.0;
			placingZone->cells [xx][yy].verLen = 0.0;
			placingZone->cells [xx][yy].leftBottomX = 0.0;
			placingZone->cells [xx][yy].leftBottomY = 0.0;
			placingZone->cells [xx][yy].leftBottomZ = 0.0;
		}
}

// 1차 배치: 인코너, 유로폼
void	firstPlacingSettingsForWall (WallPlacingZone* placingZone)
{
	short			xx, yy, zz;
	std::string		tempString;

	// 왼쪽 인코너 설정
	if (placingZone->bLIncorner) {
		for (xx = 0 ; xx < placingZone->eu_count_ver ; ++xx) {
			placingZone->cells [xx][0].objType = INCORNER;
			placingZone->cells [xx][0].horLen = placingZone->lenLIncorner;
			if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0)
				placingZone->cells [xx][0].verLen = placingZone->eu_hei_numeric;
			else
				placingZone->cells [xx][0].verLen = placingZone->eu_wid_numeric;
			placingZone->cells [xx][0].ang = placingZone->ang + DegreeToRad (-90);
			placingZone->cells [xx][0].leftBottomX = placingZone->leftBottomX;
			placingZone->cells [xx][0].leftBottomY = placingZone->leftBottomY;
			placingZone->cells [xx][0].leftBottomZ = placingZone->leftBottomZ + (xx * placingZone->cells [0][0].verLen);
			placingZone->cells [xx][0].libPart.incorner.wid_s = 0.100;									// 인코너패널 - 가로(빨강)
			placingZone->cells [xx][0].libPart.incorner.leng_s = placingZone->lenLIncorner;				// 인코너패널 - 세로(파랑)
			if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0)
				placingZone->cells [xx][0].libPart.incorner.hei_s = placingZone->eu_hei_numeric;		// 인코너패널 - 높이
			else
				placingZone->cells [xx][0].libPart.incorner.hei_s = placingZone->eu_wid_numeric;		// 인코너패널 - 높이
		}
	}

	// 유로폼 설정
	for (xx = 0 ; xx < placingZone->eu_count_ver ; ++xx) {
		for (yy = 1 ; yy <= placingZone->eu_count_hor ; ++yy) {
			zz = yy * 2;	// Cell의 객체 정보를 채움 (인덱스: 2, 4, 6 식으로)

			placingZone->cells [xx][zz].objType = EUROFORM;
			placingZone->cells [xx][zz].ang = placingZone->ang;

			if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0) {
				placingZone->cells [xx][zz].libPart.form.u_ins_wall = true;
				placingZone->cells [xx][zz].horLen = placingZone->eu_wid_numeric;
				placingZone->cells [xx][zz].verLen = placingZone->eu_hei_numeric;
			} else {
				placingZone->cells [xx][zz].libPart.form.u_ins_wall = false;
				placingZone->cells [xx][zz].horLen = placingZone->eu_hei_numeric;
				placingZone->cells [xx][zz].verLen = placingZone->eu_wid_numeric;
			}

			placingZone->cells [xx][zz].leftBottomX = placingZone->leftBottomX + (getCellPositionLeftBottomXForWall (placingZone, xx, zz) * cos(placingZone->ang));
			placingZone->cells [xx][zz].leftBottomY = placingZone->leftBottomY + (getCellPositionLeftBottomXForWall (placingZone, xx, zz) * sin(placingZone->ang));
			placingZone->cells [xx][zz].leftBottomZ = placingZone->leftBottomZ + (xx * placingZone->cells [0][zz].verLen);
			placingZone->cells [xx][zz].libPart.form.eu_stan_onoff = true;
			placingZone->cells [xx][zz].libPart.form.eu_wid = placingZone->eu_wid_numeric;
			placingZone->cells [xx][zz].libPart.form.eu_hei = placingZone->eu_hei_numeric;
		}
	}

	// 오른쪽 인코너 설정
	if (placingZone->bRIncorner) {
		for (xx = 0 ; xx < placingZone->eu_count_ver ; ++xx) {
			placingZone->cells [xx][placingZone->nCells - 1].objType = INCORNER;
			placingZone->cells [xx][placingZone->nCells - 1].horLen = placingZone->lenRIncorner;
			if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0)
				placingZone->cells [xx][placingZone->nCells - 1].verLen = placingZone->eu_hei_numeric;
			else
				placingZone->cells [xx][placingZone->nCells - 1].verLen = placingZone->eu_wid_numeric;
			placingZone->cells [xx][placingZone->nCells - 1].ang = placingZone->ang + DegreeToRad (-180);
			placingZone->cells [xx][placingZone->nCells - 1].leftBottomX = placingZone->leftBottomX + ((getCellPositionLeftBottomXForWall (placingZone, xx, placingZone->nCells - 1) + placingZone->lenRIncorner) * cos(placingZone->ang));
			placingZone->cells [xx][placingZone->nCells - 1].leftBottomY = placingZone->leftBottomY + ((getCellPositionLeftBottomXForWall (placingZone, xx, placingZone->nCells - 1) + placingZone->lenRIncorner) * sin(placingZone->ang));
			placingZone->cells [xx][placingZone->nCells - 1].leftBottomZ = placingZone->leftBottomZ + (xx * placingZone->cells [0][placingZone->nCells - 1].verLen);
			placingZone->cells [xx][placingZone->nCells - 1].libPart.incorner.wid_s = placingZone->lenRIncorner;			// 인코너패널 - 가로(빨강)
			placingZone->cells [xx][placingZone->nCells - 1].libPart.incorner.leng_s = 0.100;								// 인코너패널 - 세로(파랑)
			if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0)
				placingZone->cells [xx][placingZone->nCells - 1].libPart.incorner.hei_s = placingZone->eu_hei_numeric;		// 인코너패널 - 높이
			else
				placingZone->cells [xx][placingZone->nCells - 1].libPart.incorner.hei_s = placingZone->eu_wid_numeric;		// 인코너패널 - 높이
		}
	}
}

// 원본 벽면 영역 정보를 대칭하는 반대쪽에도 복사함
void	copyPlacingZoneSymmetricForWall (WallPlacingZone* src_zone, WallPlacingZone* dst_zone, InfoWall* infoWall)
{
	short	xx, yy;

	// 벽면 영역 정보 초기화
	dst_zone->leftBottomX			= src_zone->leftBottomX - (infoWall->wallThk * sin(src_zone->ang));
	dst_zone->leftBottomY			= src_zone->leftBottomY + (infoWall->wallThk * cos(src_zone->ang));
	dst_zone->leftBottomZ			= src_zone->leftBottomZ;
	dst_zone->horLen				= src_zone->horLen;
	dst_zone->verLen				= src_zone->verLen;
	dst_zone->ang					= src_zone->ang;

	dst_zone->nInterfereBeams		= src_zone->nInterfereBeams;
	for (xx = 0 ; xx < 30 ; ++xx)
		dst_zone->beams [xx]			= src_zone->beams [xx];

	dst_zone->remain_hor			= src_zone->remain_hor;
	dst_zone->remain_hor_updated	= src_zone->remain_hor_updated;
	dst_zone->remain_ver			= src_zone->remain_ver;
	dst_zone->remain_ver_wo_beams	= src_zone->remain_ver_wo_beams;

	dst_zone->bLIncorner			= src_zone->bLIncorner;
	dst_zone->bRIncorner			= src_zone->bRIncorner;
	dst_zone->lenLIncorner			= src_zone->lenLIncorner;
	dst_zone->lenRIncorner			= src_zone->lenRIncorner;

	dst_zone->eu_wid				= src_zone->eu_wid;
	dst_zone->eu_wid_numeric		= src_zone->eu_wid_numeric;
	dst_zone->eu_hei				= src_zone->eu_hei;
	dst_zone->eu_hei_numeric		= src_zone->eu_hei_numeric;
	dst_zone->eu_ori				= src_zone->eu_ori;
	dst_zone->eu_count_hor			= src_zone->eu_count_hor;
	dst_zone->eu_count_ver			= src_zone->eu_count_ver;

	dst_zone->nCells				= src_zone->nCells;


	// Cell 변수를 대칭적으로 복사
	for (xx = 0 ; xx < dst_zone->eu_count_ver ; ++xx) {
		for (yy = 0 ; yy < dst_zone->nCells ; ++yy) {

			// 아무것도 없으면,
			if (src_zone->cells [xx][yy].objType == NONE) {

				dst_zone->cells [xx][yy].objType			= NONE;
				dst_zone->cells [xx][yy].horLen				= 0;
				dst_zone->cells [xx][yy].verLen				= 0;
				dst_zone->cells [xx][yy].ang				= dst_zone->ang;
				dst_zone->cells [xx][yy].leftBottomX		= dst_zone->leftBottomX + (getCellPositionLeftBottomXForWall (src_zone, xx, yy) * cos(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomY		= dst_zone->leftBottomY + (getCellPositionLeftBottomXForWall (src_zone, xx, yy) * sin(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomZ		= src_zone->leftBottomZ;

			// 인코너인 경우,
			} else if (src_zone->cells [xx][yy].objType == INCORNER) {
				// 왼쪽 인코너
				if ((placingZoneBackside.bLIncorner) && (yy == 0)) {
					dst_zone->cells [xx][yy].objType						= INCORNER;
					dst_zone->cells [xx][yy].horLen							= src_zone->cells [xx][yy].horLen;
					dst_zone->cells [xx][yy].verLen							= src_zone->cells [xx][yy].verLen;
					dst_zone->cells [xx][yy].ang							= dst_zone->ang;
					dst_zone->cells [xx][yy].leftBottomX					= dst_zone->leftBottomX;
					dst_zone->cells [xx][yy].leftBottomY					= dst_zone->leftBottomY;
					dst_zone->cells [xx][yy].leftBottomZ					= src_zone->cells [xx][yy].leftBottomZ;
					dst_zone->cells [xx][yy].libPart.incorner.wid_s			= src_zone->cells [xx][yy].libPart.incorner.leng_s;		// 인코너패널 - 가로(빨강)
					dst_zone->cells [xx][yy].libPart.incorner.leng_s		= src_zone->cells [xx][yy].libPart.incorner.wid_s;		// 인코너패널 - 세로(파랑)
					dst_zone->cells [xx][yy].libPart.incorner.hei_s			= src_zone->cells [xx][yy].libPart.incorner.hei_s;		// 인코너패널 - 높이

				// 오른쪽 인코너
				} else if ((placingZoneBackside.bRIncorner) && (yy == src_zone->nCells - 1)) {
					dst_zone->cells [xx][yy].objType						= INCORNER;
					dst_zone->cells [xx][yy].horLen							= src_zone->cells [xx][yy].horLen;
					dst_zone->cells [xx][yy].verLen							= src_zone->cells [xx][yy].verLen;
					dst_zone->cells [xx][yy].ang							= dst_zone->ang + DegreeToRad (90);
					dst_zone->cells [xx][yy].leftBottomX					= dst_zone->leftBottomX + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->lenRIncorner) * cos(src_zone->ang));
					dst_zone->cells [xx][yy].leftBottomY					= dst_zone->leftBottomY + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->lenRIncorner) * sin(src_zone->ang));
					dst_zone->cells [xx][yy].leftBottomZ					= src_zone->cells [xx][yy].leftBottomZ;
					dst_zone->cells [xx][yy].libPart.incorner.wid_s			= src_zone->cells [xx][yy].libPart.incorner.leng_s;		// 인코너패널 - 가로(빨강)
					dst_zone->cells [xx][yy].libPart.incorner.leng_s		= src_zone->cells [xx][yy].libPart.incorner.wid_s;		// 인코너패널 - 세로(파랑)
					dst_zone->cells [xx][yy].libPart.incorner.hei_s			= src_zone->cells [xx][yy].libPart.incorner.hei_s;		// 인코너패널 - 높이

				// 기타 위치
				} else {
					dst_zone->cells [xx][yy].objType						= INCORNER;
					dst_zone->cells [xx][yy].horLen							= src_zone->cells [xx][yy].horLen;
					dst_zone->cells [xx][yy].verLen							= src_zone->cells [xx][yy].verLen;
					dst_zone->cells [xx][yy].ang							= dst_zone->ang + DegreeToRad (90);
					dst_zone->cells [xx][yy].leftBottomX					= dst_zone->leftBottomX + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->lenRIncorner) * cos(src_zone->ang));
					dst_zone->cells [xx][yy].leftBottomY					= dst_zone->leftBottomY + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->lenRIncorner) * sin(src_zone->ang));
					dst_zone->cells [xx][yy].leftBottomZ					= src_zone->cells [xx][yy].leftBottomZ;
					dst_zone->cells [xx][yy].libPart.incorner.wid_s			= src_zone->cells [xx][yy].libPart.incorner.leng_s;		// 인코너패널 - 가로(빨강)
					dst_zone->cells [xx][yy].libPart.incorner.leng_s		= src_zone->cells [xx][yy].libPart.incorner.wid_s;		// 인코너패널 - 세로(파랑)
					dst_zone->cells [xx][yy].libPart.incorner.hei_s			= src_zone->cells [xx][yy].libPart.incorner.hei_s;		// 인코너패널 - 높이
				}
		
			// 유로폼인 경우,
			} else if (src_zone->cells [xx][yy].objType == EUROFORM) {
		
				dst_zone->cells [xx][yy].objType					= EUROFORM;
				dst_zone->cells [xx][yy].horLen						= src_zone->cells [xx][yy].horLen;
				dst_zone->cells [xx][yy].verLen						= src_zone->cells [xx][yy].verLen;
				dst_zone->cells [xx][yy].ang						= dst_zone->ang + DegreeToRad (180);

				dst_zone->cells [xx][yy].leftBottomX = dst_zone->leftBottomX + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->cells [xx][yy].horLen) * cos(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomY = dst_zone->leftBottomY + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->cells [xx][yy].horLen) * sin(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomZ					= src_zone->cells [xx][yy].leftBottomZ;
				dst_zone->cells [xx][yy].libPart.form.eu_stan_onoff		= src_zone->cells [xx][yy].libPart.form.eu_stan_onoff;
				dst_zone->cells [xx][yy].libPart.form.eu_wid			= src_zone->cells [xx][yy].libPart.form.eu_wid;
				dst_zone->cells [xx][yy].libPart.form.eu_hei			= src_zone->cells [xx][yy].libPart.form.eu_hei;
				dst_zone->cells [xx][yy].libPart.form.eu_wid2			= src_zone->cells [xx][yy].libPart.form.eu_wid2;
				dst_zone->cells [xx][yy].libPart.form.eu_hei2			= src_zone->cells [xx][yy].libPart.form.eu_hei2;
				dst_zone->cells [xx][yy].libPart.form.u_ins_wall		= src_zone->cells [xx][yy].libPart.form.u_ins_wall;

			// 휠러스페이서인 경우,
			} else if (src_zone->cells [xx][yy].objType == FILLERSPACER) {
		
				dst_zone->cells [xx][yy].objType			= FILLERSPACER;
				dst_zone->cells [xx][yy].horLen				= src_zone->cells [xx][yy].horLen;
				dst_zone->cells [xx][yy].verLen				= src_zone->cells [xx][yy].verLen;
				dst_zone->cells [xx][yy].ang				= dst_zone->ang + DegreeToRad (180);
				dst_zone->cells [xx][yy].leftBottomX		= dst_zone->leftBottomX + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->cells [xx][yy].horLen) * cos(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomY		= dst_zone->leftBottomY + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->cells [xx][yy].horLen) * sin(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomZ		= src_zone->cells [xx][yy].leftBottomZ;
				dst_zone->cells [xx][yy].libPart.fillersp.f_leng	= src_zone->cells [xx][yy].libPart.fillersp.f_leng;
				dst_zone->cells [xx][yy].libPart.fillersp.f_thk		= src_zone->cells [xx][yy].libPart.fillersp.f_thk;

			// 합판인 경우,
			} else if (src_zone->cells [xx][yy].objType == PLYWOOD) {

				dst_zone->cells [xx][yy].objType			= PLYWOOD;
				dst_zone->cells [xx][yy].horLen				= src_zone->cells [xx][yy].horLen;
				dst_zone->cells [xx][yy].verLen				= src_zone->cells [xx][yy].verLen;
				dst_zone->cells [xx][yy].ang				= dst_zone->ang + DegreeToRad (180);
				dst_zone->cells [xx][yy].leftBottomX		= dst_zone->leftBottomX + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->cells [xx][yy].horLen) * cos(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomY		= dst_zone->leftBottomY + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->cells [xx][yy].horLen) * sin(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomZ		= src_zone->cells [xx][yy].leftBottomZ;
				dst_zone->cells [xx][yy].libPart.plywood.p_leng		= src_zone->cells [xx][yy].libPart.plywood.p_leng;
				dst_zone->cells [xx][yy].libPart.plywood.p_wid		= src_zone->cells [xx][yy].libPart.plywood.p_wid;
				dst_zone->cells [xx][yy].libPart.plywood.w_dir_wall	= src_zone->cells [xx][yy].libPart.plywood.w_dir_wall;
		
			// 목재인 경우,
			} else if (src_zone->cells [xx][yy].objType == WOOD) {

				dst_zone->cells [xx][yy].objType			= WOOD;
				dst_zone->cells [xx][yy].horLen				= src_zone->cells [xx][yy].horLen;
				dst_zone->cells [xx][yy].verLen				= src_zone->cells [xx][yy].verLen;
				dst_zone->cells [xx][yy].ang				= dst_zone->ang + DegreeToRad (180);
				dst_zone->cells [xx][yy].leftBottomX		= dst_zone->leftBottomX + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->cells [xx][yy].horLen) * cos(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomY		= dst_zone->leftBottomY + ((getCellPositionLeftBottomXForWall (src_zone, xx, yy) + src_zone->cells [xx][yy].horLen) * sin(src_zone->ang));
				dst_zone->cells [xx][yy].leftBottomZ		= src_zone->cells [xx][yy].leftBottomZ;
				dst_zone->cells [xx][yy].libPart.wood.w_h		= src_zone->cells [xx][yy].libPart.wood.w_h;
				dst_zone->cells [xx][yy].libPart.wood.w_leng	= src_zone->cells [xx][yy].libPart.wood.w_leng;
				dst_zone->cells [xx][yy].libPart.wood.w_w		= src_zone->cells [xx][yy].libPart.wood.w_w;
				dst_zone->cells [xx][yy].libPart.wood.w_ang		= src_zone->cells [xx][yy].libPart.wood.w_ang;
			}
		}
	}
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	alignPlacingZoneForWall (WallPlacingZone* target_zone)
{
	short			xx, yy;

	// 각 Cell마다 위치 및 각도 정보가 업데이트됨
	for (xx = 0 ; xx < target_zone->eu_count_ver ; ++xx) {
		for (yy = 0 ; yy < target_zone->nCells ; ++yy) {
			// 인코너인 경우,
			if (target_zone->cells [xx][yy].objType == INCORNER) {

				if (yy == 0) {
					target_zone->cells [xx][yy].ang			= target_zone->ang + DegreeToRad (-90);
					target_zone->cells [xx][yy].leftBottomX	= target_zone->leftBottomX;
					target_zone->cells [xx][yy].leftBottomY	= target_zone->leftBottomY;
				} else {
					target_zone->cells [xx][yy].ang			= target_zone->ang + DegreeToRad (-180);
					target_zone->cells [xx][yy].leftBottomX	= target_zone->leftBottomX + ((getCellPositionLeftBottomXForWall (target_zone, xx, yy) + target_zone->lenRIncorner) * cos(target_zone->ang));
					target_zone->cells [xx][yy].leftBottomY	= target_zone->leftBottomY + ((getCellPositionLeftBottomXForWall (target_zone, xx, yy) + target_zone->lenRIncorner) * sin(target_zone->ang));
				}
				target_zone->cells [xx][yy].leftBottomZ = target_zone->leftBottomZ + (target_zone->cells [0][yy].verLen * xx);

			// 나머지: NONE, 유로폼, 휠러스페이서, 합판, 목재인 경우
			} else {

				target_zone->cells [xx][yy].ang			= target_zone->ang;
				target_zone->cells [xx][yy].leftBottomX	= target_zone->leftBottomX + (getCellPositionLeftBottomXForWall (target_zone, xx, yy) * cos(target_zone->ang));;
				target_zone->cells [xx][yy].leftBottomY	= target_zone->leftBottomY + (getCellPositionLeftBottomXForWall (target_zone, xx, yy) * sin(target_zone->ang));;
				target_zone->cells [xx][yy].leftBottomZ = target_zone->leftBottomZ + (target_zone->cells [0][yy].verLen * xx);
			}
		}
	}

	// 영역 정보에서 남은 거리 관련 항목들이 업데이트됨
	target_zone->remain_hor_updated = target_zone->remain_hor = target_zone->horLen;

	// 가로 방향 남은 길이: 각 셀의 너비만큼 차감
	for (yy = 0 ; yy < target_zone->nCells ; ++yy) {
		if (target_zone->cells [0][yy].objType != NONE) {
			target_zone->remain_hor_updated -= target_zone->cells [0][yy].horLen;
		}
	}
}

// src행의 Cell 전체 라인을 dst행으로 복사
void	copyCellsToAnotherLineForWall (WallPlacingZone* target_zone, short src_row, short dst_row)
{
	short xx;

	for (xx = 0 ; xx <= target_zone->nCells ; ++xx)
		target_zone->cells [dst_row][xx] = target_zone->cells [src_row][xx];
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK wallPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "유로폼 벽에 배치");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			//////////////////////////////////////////////////////////// 아이템 배치 (인코너 관련)
			// 라벨: 인코너 배치 설정
			DGSetItemText (dialogID, LABEL_INCORNER, "인코너 배치 설정");

			// 체크박스: 왼쪽 인코너
			DGSetItemText (dialogID, CHECKBOX_SET_LEFT_INCORNER, "왼쪽");
			DGSetItemValLong (dialogID, CHECKBOX_SET_LEFT_INCORNER, true);

			// Edit 컨트롤: 왼쪽 인코너
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_INCORNER, 0.100);

			// 체크박스: 오른쪽 인코너
			DGSetItemText (dialogID, CHECKBOX_SET_RIGHT_INCORNER, "오른쪽");
			DGSetItemValLong (dialogID, CHECKBOX_SET_RIGHT_INCORNER, true);

			// Edit 컨트롤: 오른쪽 인코너
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_INCORNER, 0.100);

			//////////////////////////////////////////////////////////// 아이템 배치 (유로폼)
			// 라벨: 유로폼 배치 설정
			DGSetItemText (dialogID, LABEL_PLACING_EUROFORM, "유로폼 배치 설정");

			// 라벨: 너비
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH, "너비");

			// 라벨: 높이
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT, "높이");

			// 라벨: 설치방향
			DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION, "설치방향");

			// 라벨: 슬래브와의 간격
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "벽과의 간격");

			// 라벨: 레이어 설정
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");

			// 라벨: 레이어 - 인코너
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER, "인코너");

			// 라벨: 레이어 - 유로폼
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");

			// 라벨: 레이어 - 휠러스페이서
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSPACER, "휠러스페이서");

			// 라벨: 레이어 - 목재
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");

			// 라벨: 레이어 - 합판
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "목재");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_INCORNER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSPACER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);

			break;

		case DG_MSG_CHANGE:
			// 체크박스 제어 (인코너 관련)
			if (DGGetItemValLong (dialogID, CHECKBOX_SET_LEFT_INCORNER) == 1)
				DGShowItem (dialogID, EDITCONTROL_LEFT_INCORNER);
			else
				DGHideItem (dialogID, EDITCONTROL_LEFT_INCORNER);

			if (DGGetItemValLong (dialogID, CHECKBOX_SET_RIGHT_INCORNER) == 1)
				DGShowItem (dialogID, EDITCONTROL_RIGHT_INCORNER);
			else
				DGHideItem (dialogID, EDITCONTROL_RIGHT_INCORNER);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// 유로폼 너비, 높이, 방향
					placingZone.eu_wid = DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_EUROFORM_WIDTH))).ToCStr ().Get ();
					placingZone.eu_hei = DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_EUROFORM_HEIGHT))).ToCStr ().Get ();
					placingZone.eu_ori = DGPopUpGetItemText (dialogID, POPUP_EUROFORM_ORIENTATION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_EUROFORM_ORIENTATION))).ToCStr ().Get ();

					// 좌우 인코너 여백
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_LEFT_INCORNER) == TRUE)
						placingZone.bLIncorner = true;
					else
						placingZone.bLIncorner = false;
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_RIGHT_INCORNER) == TRUE)
						placingZone.bRIncorner = true;
					else
						placingZone.bRIncorner = false;
					placingZone.lenLIncorner = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_INCORNER);
					placingZone.lenRIncorner = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_INCORNER);

					// 벽와의 간격
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// 레이어 번호 저장
					layerInd_Incorner		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER);
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Fillerspacer	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER);
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

// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK wallPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	btnSizeX = 50, btnSizeY = 50;
	short	dialogSizeX, dialogSizeY;
	short	groupboxSizeX, groupboxSizeY;
	short	btnInitPosX = 220;
	short	btnPosX = 220, btnPosY = (btnSizeY * placingZone.eu_count_ver);
	short	xx, yy;
	short	idxBtn;
	short	lastIdxBtn;
	short	idxCell;
	short	idxCell_prev = -1, idxCell_next = -1;
	std::string		txtButton = "";
	API_Element		elem;
	GSErrCode		err;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "유로폼 벽에 배치 - 가로 채우기");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 배치 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 100, 130, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "배  치");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 140, 130, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "자투리 채우기");
			DGShowItem (dialogID, DG_CANCEL);

			// 이전 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 180, 130, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "이전");
			DGShowItem (dialogID, DG_PREV);

			//////////////////////////////////////////////////////////// 아이템 배치 (인코너 관련)
			// 라벨: 남은 가로 길이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 20, 90, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, "남은 가로 길이");
			DGShowItem (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH);

			// Edit 컨트롤: 남은 가로 길이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 120, 20-7, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated);

			// 그룹박스: 유로폼/휠러스페이서 배치 설정
			groupboxSizeX = 40 + (btnSizeX * (placingZone.eu_count_hor + 3));
			groupboxSizeY = 70 + (btnSizeY * placingZone.eu_count_ver);
			DGAppendDialogItem (dialogID, DG_ITM_GROUPBOX, DG_GT_PRIMARY, 0, 200, 10, groupboxSizeX, groupboxSizeY);
			DGSetItemFont (dialogID, GROUPBOX_GRID_EUROFORM_FILLERSPACER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, GROUPBOX_GRID_EUROFORM_FILLERSPACER, "유로폼/휠러스페이서 배치 설정");
			DGShowItem (dialogID, GROUPBOX_GRID_EUROFORM_FILLERSPACER);

			// 남은 거리 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 60, 130, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, "남은 길이 확인");
			DGShowItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);
			DGDisableItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);

			// 메인 창 크기를 변경
			dialogSizeX = 270 + (btnSizeX * (placingZone.eu_count_hor + 3));
			dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver));
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_CENTER, true);

			// 그리드 구조체에 따라서 버튼을 동적으로 배치함
			for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
				for (yy = 0 ; yy < placingZone.nCells ; yy += 2) {
					idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					lastIdxBtn = idxBtn;
					DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

					// 버튼 인덱스로 셀 인덱스를 구함
					idxCell = ((idxBtn - itemInitIdx) * 2) - (xx * (placingZone.eu_count_hor + 2)) * 2;

					txtButton = "";
					if (placingZone.cells [0][idxCell].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cells [0][idxCell].objType == INCORNER) {
						txtButton = format_string ("인코너\n↔%.0f\n↕%.0f", placingZone.cells [0][idxCell].horLen * 1000, placingZone.cells [0][idxCell].verLen * 1000);
					} else if (placingZone.cells [0][idxCell].objType == EUROFORM) {
						if (placingZone.cells [0][idxCell].libPart.form.u_ins_wall)
							txtButton = format_string ("유로폼\n(세움)\n↔%.0f\n↕%.0f", placingZone.cells [0][idxCell].horLen * 1000, placingZone.cells [0][idxCell].verLen * 1000);
						else
							txtButton = format_string ("유로폼\n(눕힘)\n↔%.0f\n↕%.0f", placingZone.cells [0][idxCell].horLen * 1000, placingZone.cells [0][idxCell].verLen * 1000);
					} else if (placingZone.cells [0][idxCell].objType == FILLERSPACER) {
						txtButton = format_string ("휠러\n↔%.0f\n↕%.0f", placingZone.cells [0][idxCell].horLen * 1000, placingZone.cells [0][idxCell].verLen * 1000);
					} else if (placingZone.cells [0][idxCell].objType == PLYWOOD) {
						if (placingZone.cells [0][idxCell].libPart.plywood.w_dir_wall)
							txtButton = format_string ("합판\n(세움)\n↔%.0f\n↕%.0f", placingZone.cells [0][idxCell].horLen * 1000, placingZone.cells [0][idxCell].verLen * 1000);
						else
							txtButton = format_string ("합판\n(눕힘)\n↔%.0f\n↕%.0f", placingZone.cells [0][idxCell].horLen * 1000, placingZone.cells [0][idxCell].verLen * 1000);
					} else if (placingZone.cells [0][idxCell].objType == WOOD) {
						txtButton = format_string ("목재\n↔%.0f\n↕%.0f", placingZone.cells [0][idxCell].horLen * 1000, placingZone.cells [0][idxCell].verLen * 1000);
					}
					DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
					DGShowItem (dialogID, idxBtn);
					btnPosX += btnSizeX;
				}
				btnPosX = btnInitPosX;
				btnPosY -= btnSizeY;
			}

			break;

		case DG_MSG_CHANGE:

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case PUSHBUTTON_CONFIRM_REMAIN_LENGTH:
					// 종료하지 않고 남은 가로 거리와 그리드 버튼 속성을 변경함
					item = 0;

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					alignPlacingZoneForWall (&placingZone);
					copyPlacingZoneSymmetricForWall (&placingZone, &placingZoneBackside, &infoWall);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;
					
					// 그리드 버튼 텍스트 업데이트
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.nCells ; yy += 2) {

							// 버튼 인덱스로 셀 인덱스를 구함
							idxCell = ((idxBtn - itemInitIdx) * 2) - (xx * (placingZone.eu_count_hor + 2)) * 2;

							txtButton = "";
							if (placingZone.cells [0][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [0][yy].objType == INCORNER) {
								txtButton = format_string ("인코너\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == EUROFORM) {
								if (placingZone.cells [0][yy].libPart.form.u_ins_wall)
									txtButton = format_string ("유로폼\n(세움)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
								else
									txtButton = format_string ("유로폼\n(눕힘)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == FILLERSPACER) {
								txtButton = format_string ("휠러\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == PLYWOOD) {
								if (placingZone.cells [0][yy].libPart.plywood.w_dir_wall)
									txtButton = format_string ("합판\n(세움)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
								else
									txtButton = format_string ("합판\n(눕힘)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == WOOD) {
								txtButton = format_string ("목재\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							}
							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
							
							// 만약 버튼 인접 셀이 '없음'이 아니라면 해당 셀의 글꼴을 변경함
							if ( (idxCell > 0) && (idxCell < (placingZone.nCells - 1)) ) {
								idxCell_prev = idxCell - 1;
								idxCell_next = idxCell + 1;
							} else if (idxCell == 0) {
								idxCell_prev = -1;
								idxCell_next = idxCell + 1;
							} else if (idxCell == (placingZone.nCells - 1)) {
								idxCell_prev = idxCell - 1;
								idxCell_next = -1;
							}

							// 인접 셀의 객체 종류가 NONE이 아니면 버튼 글꼴 변경
							DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_PLAIN);
							if (yy == 0) {
								if (placingZone.cells [0][yy+1].objType != NONE)
									DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
							} else if ( (yy > 0) && (yy < (placingZone.nCells - 2)) ) {
								if ( (placingZone.cells [0][yy-1].objType != NONE) || (placingZone.cells [0][yy+1].objType != NONE) )
									DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
							} else if ( yy == (placingZone.nCells - 1) ) {
								if (placingZone.cells [0][yy-1].objType != NONE)
									DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
							}

							++idxBtn;
						}
					}

					// 남은 가로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated);
					DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_BOLD);

					break;

				case DG_OK:
					// 종료하지 않고 배치된 객체를 수정 및 재배치하고 그리드 버튼 속성을 변경함
					item = 0;

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					alignPlacingZoneForWall (&placingZone);
					copyPlacingZoneSymmetricForWall (&placingZone, &placingZoneBackside, &infoWall);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;

					// 그리드 버튼 텍스트 업데이트
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.nCells ; yy += 2) {

							// 셀 인덱스로 버튼 인덱스를 구함
							idxCell = ((idxBtn - itemInitIdx) * 2) - (xx * (placingZone.eu_count_hor + 2)) * 2;		// 버튼 인덱스로 셀 인덱스를 구함

							txtButton = "";
							if (placingZone.cells [0][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [0][yy].objType == INCORNER) {
								txtButton = format_string ("인코너\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == EUROFORM) {
								if (placingZone.cells [0][yy].libPart.form.u_ins_wall)
									txtButton = format_string ("유로폼\n(세움)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
								else
									txtButton = format_string ("유로폼\n(눕힘)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == FILLERSPACER) {
								txtButton = format_string ("휠러\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == PLYWOOD) {
								if (placingZone.cells [0][yy].libPart.plywood.w_dir_wall)
									txtButton = format_string ("합판\n(세움)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
								else
									txtButton = format_string ("합판\n(눕힘)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == WOOD) {
								txtButton = format_string ("목재\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							}
							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정

							// 만약 버튼 인접 셀이 '없음'이 아니라면 해당 셀의 글꼴을 변경함
							if ( (idxCell > 0) && (idxCell < (placingZone.nCells - 1)) ) {
								idxCell_prev = idxCell - 1;
								idxCell_next = idxCell + 1;
							} else if (idxCell == 0) {
								idxCell_prev = -1;
								idxCell_next = idxCell + 1;
							} else if (idxCell == (placingZone.nCells - 1)) {
								idxCell_prev = idxCell - 1;
								idxCell_next = -1;
							}

							// 인접 셀의 객체 종류가 NONE이 아니면 버튼 글꼴 변경
							DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_PLAIN);
							if (yy == 0) {
								if (placingZone.cells [0][yy+1].objType != NONE)
									DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
							} else if ( (yy > 0) && (yy < (placingZone.nCells - 2)) ) {
								if ( (placingZone.cells [0][yy-1].objType != NONE) || (placingZone.cells [0][yy+1].objType != NONE) )
									DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
							} else if ( yy == (placingZone.nCells - 1) ) {
								if (placingZone.cells [0][yy-1].objType != NONE)
									DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
							}

							++idxBtn;
						}
					}

					// 남은 가로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated);
					DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_BOLD);

					// 기존 배치된 객체 전부 삭제
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.nCells ; ++yy) {
							elem.header.guid = placingZone.cells [xx][yy].guid;
							if (ACAPI_Element_Get (&elem) != NoError)
								continue;

							API_Elem_Head* headList = new API_Elem_Head [1];
							headList [0] = elem.header;
							err = ACAPI_Element_Delete (&headList, 1);
							delete headList;
						}
					}

					for (xx = 0 ; xx < placingZoneBackside.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZoneBackside.nCells ; ++yy) {
							elem.header.guid = placingZoneBackside.cells [xx][yy].guid;
							if (ACAPI_Element_Get (&elem) != NoError)
								continue;

							API_Elem_Head* headList = new API_Elem_Head [1];
							headList [0] = elem.header;
							err = ACAPI_Element_Delete (&headList, 1);
							delete headList;
						}
					}

					// 업데이트된 셀 정보대로 객체 재배치
					//////////////////////////////////////////////////////////// 벽 앞쪽
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx)
						for (yy = 0 ; yy < placingZone.nCells ; ++yy)
							placingZone.cells [xx][yy].guid = placeLibPartForWall (placingZone.cells [xx][yy]);

					//////////////////////////////////////////////////////////// 벽 뒤쪽
					for (xx = 0 ; xx < placingZoneBackside.eu_count_ver ; ++xx)
						for (yy = 0 ; yy < placingZoneBackside.nCells ; ++yy)
							placingZoneBackside.cells [xx][yy].guid = placeLibPartForWall (placingZoneBackside.cells [xx][yy]);

					clickedOKButton = true;

					break;
				
				case DG_CANCEL:
					// 자투리 채우기로 넘어갈 때 배치된 유로폼들의 모든 GUID를 저장함
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx)
						for (yy = 0 ; yy < placingZone.nCells ; ++yy)
							elemList.Push (placingZone.cells [xx][yy].guid);

					for (xx = 0 ; xx < placingZoneBackside.eu_count_ver ; ++xx)
						for (yy = 0 ; yy < placingZoneBackside.nCells ; ++yy)
							elemList.Push (placingZoneBackside.cells [xx][yy].guid);

					break;

				case DG_PREV:
					clickedPrevButton = true;
					break;

				default:
					// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창(3번째 다이얼로그)이 나옴
					clickedBtnItemIdx = item;
					result = DGBlankModalDialog (240*3, 260, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, wallPlacerHandler3, 0);

					item = 0;	// 그리드 버튼을 눌렀을 때 창이 닫히지 않게 함

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					alignPlacingZoneForWall (&placingZone);
					copyPlacingZoneSymmetricForWall (&placingZone, &placingZoneBackside, &infoWall);

					// 버튼 인덱스 iteration 준비
					idxBtn = itemInitIdx;
					
					// 그리드 버튼 텍스트 업데이트
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.nCells ; yy += 2) {

							// 버튼 인덱스로 셀 인덱스를 구함
							idxCell = ((idxBtn - itemInitIdx) * 2) - (xx * (placingZone.eu_count_hor + 2)) * 2;

							txtButton = "";
							if (placingZone.cells [0][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [0][yy].objType == INCORNER) {
								txtButton = format_string ("인코너\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == EUROFORM) {
								if (placingZone.cells [0][yy].libPart.form.u_ins_wall)
									txtButton = format_string ("유로폼\n(세움)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
								else
									txtButton = format_string ("유로폼\n(눕힘)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == FILLERSPACER) {
								txtButton = format_string ("휠러\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == PLYWOOD) {
								if (placingZone.cells [0][yy].libPart.plywood.w_dir_wall)
									txtButton = format_string ("합판\n(세움)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
								else
									txtButton = format_string ("합판\n(눕힘)\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							} else if (placingZone.cells [0][yy].objType == WOOD) {
								txtButton = format_string ("목재\n↔%.0f\n↕%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
							}
							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// 그리드 버튼 텍스트 지정
							
							// 만약 버튼 인접 셀이 '없음'이 아니라면 해당 셀의 글꼴을 변경함
							if ( (idxCell > 0) && (idxCell < (placingZone.nCells - 1)) ) {
								idxCell_prev = idxCell - 1;
								idxCell_next = idxCell + 1;
							} else if (idxCell == 0) {
								idxCell_prev = -1;
								idxCell_next = idxCell + 1;
							} else if (idxCell == (placingZone.nCells - 1)) {
								idxCell_prev = idxCell - 1;
								idxCell_next = -1;
							}

							// 인접 셀의 객체 종류가 NONE이 아니면 버튼 글꼴 변경
							DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_PLAIN);
							if (yy == 0) {
								if (placingZone.cells [0][yy+1].objType != NONE)
									DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
							} else if ( (yy > 0) && (yy < (placingZone.nCells - 2)) ) {
								if ( (placingZone.cells [0][yy-1].objType != NONE) || (placingZone.cells [0][yy+1].objType != NONE) )
									DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
							} else if ( yy == (placingZone.nCells - 1) ) {
								if (placingZone.cells [0][yy-1].objType != NONE)
									DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
							}

							++idxBtn;
						}
					}

					// 남은 가로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated);
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
short DGCALLBACK wallPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	short	idxCell;
	short	idxCell_prev = -1, idxCell_next = -1;
	short	popupSelectedIdx = 0;
	short	xx;
	double	temp;

	switch (message) {
		case DG_MSG_INIT:

			// wallPlacerHandler2 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
			idxCell = (clickedBtnItemIdx - itemInitIdx) * 2;
			while (idxCell >= ((placingZone.eu_count_hor + 2) * 2))
				idxCell -= ((placingZone.eu_count_hor + 2) * 2);

			// 현재 셀이 중간 셀이면,
			if ( (idxCell > 0) && (idxCell < (placingZone.nCells - 1)) ) {
				idxCell_prev = idxCell - 1;
				idxCell_next = idxCell + 1;
			// 현재 셀이 맨 처음 셀이면,
			} else if (idxCell == 0) {
				idxCell_prev = -1;
				idxCell_next = idxCell + 1;
			// 현재 셀이 맨 끝 셀이면,
			} else if (idxCell == (placingZone.nCells - 1)) {
				idxCell_prev = idxCell - 1;
				idxCell_next = -1;
			}

			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "Cell 값 설정");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40+240, 215, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "저장");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 130+240, 215, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
			// 라벨: 객체 타입
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 15, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, "객체 타입\n[클릭한 셀]");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// 팝업컨트롤: 객체 타입을 바꿀 수 있는 콤보박스가 맨 위에 나옴
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+240, 20-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "인코너판넬");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "유로폼");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "휠러스페이서");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "합판");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "목재");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, "너비");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, "높이");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 두께
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_THK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_THK, "두께");

			// Edit 컨트롤: 두께
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 110-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_THK, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION, "설치방향");
				
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100+240, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100+240, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_PLYWOOD, "벽눕히기");

			// 체크박스: 규격폼
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20+240, 50, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "규격폼");

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, "너비");

			// 팝업 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+240, 80-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "200");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, "높이");

			// 팝업 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+240, 110-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "600");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 110-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, "설치방향");
			
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100+240, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_EUROFORM, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100+240, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_EUROFORM, "벽눕히기");

			// 초기 입력 필드 표시
			if (placingZone.cells [0][idxCell].objType == INCORNER) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, INCORNER + 1);

				// 라벨: 너비
				DGShowItem (dialogID, LABEL_WIDTH);

				// Edit 컨트롤: 너비
				DGShowItem (dialogID, EDITCONTROL_WIDTH);
				if (idxCell == 0)
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.incorner.leng_s);
				else if (idxCell > 0)
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.incorner.wid_s);
				DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.080);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.500);

				// 라벨 높이
				DGShowItem (dialogID, LABEL_HEIGHT);

				// Edit 컨트롤: 높이
				DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [0][idxCell].libPart.incorner.hei_s);
				DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.050);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.500);

			} else if (placingZone.cells [0][idxCell].objType == EUROFORM) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

				// 체크박스: 규격폼
				DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
				DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff);

				if (placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff == true) {
					// 라벨: 너비
					DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

					// 팝업 컨트롤: 너비
					DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
					if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.600) < EPS)		popupSelectedIdx = 1;
					if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.500) < EPS)		popupSelectedIdx = 2;
					if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.450) < EPS)		popupSelectedIdx = 3;
					if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.400) < EPS)		popupSelectedIdx = 4;
					if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.300) < EPS)		popupSelectedIdx = 5;
					if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.200) < EPS)		popupSelectedIdx = 6;
					DGPopUpSelectItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, popupSelectedIdx);

					// 라벨: 높이
					DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

					// 팝업 컨트롤: 높이
					DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
					if (abs(placingZone.cells [0][idxCell].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
					if (abs(placingZone.cells [0][idxCell].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
					if (abs(placingZone.cells [0][idxCell].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
					DGPopUpSelectItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, popupSelectedIdx);
				} else if (placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff == false) {
					// 라벨: 너비
					DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

					// Edit 컨트롤: 너비
					DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
					DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, placingZone.cells [0][idxCell].libPart.form.eu_wid2);
					DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);

					// 라벨: 높이
					DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

					// Edit 컨트롤: 높이
					DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
					DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, placingZone.cells [0][idxCell].libPart.form.eu_hei2);
					DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
				}

				// 라벨: 설치방향
				DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
				
				// 라디오 버튼: 설치방향 (벽세우기)
				DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
				// 라디오 버튼: 설치방향 (벽눕히기)
				DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

				if (placingZone.cells [0][idxCell].libPart.form.u_ins_wall == true) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, true);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, false);
				} else if (placingZone.cells [0][idxCell].libPart.form.u_ins_wall == false) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, false);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, true);
				}
			} else if (placingZone.cells [0][idxCell].objType == FILLERSPACER) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, FILLERSPACER + 1);

				// 라벨: 너비
				DGShowItem (dialogID, LABEL_WIDTH);

				// Edit 컨트롤: 너비
				DGShowItem (dialogID, EDITCONTROL_WIDTH);
				DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.fillersp.f_thk);
				DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.010);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.050);

				// 라벨 높이
				DGShowItem (dialogID, LABEL_HEIGHT);

				// Edit 컨트롤: 높이
				DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [0][idxCell].libPart.fillersp.f_leng);
				DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.150);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.400);

			} else if (placingZone.cells [0][idxCell].objType == PLYWOOD) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PLYWOOD + 1);

				// 라벨: 너비
				DGShowItem (dialogID, LABEL_WIDTH);

				// Edit 컨트롤: 너비
				DGShowItem (dialogID, EDITCONTROL_WIDTH);
				DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.plywood.p_wid);
				DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.110);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.220);

				// 라벨: 높이
				DGShowItem (dialogID, LABEL_HEIGHT);

				// Edit 컨트롤: 높이
				DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [0][idxCell].libPart.plywood.p_leng);
				DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.110);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.440);

				// 라벨: 설치방향
				DGShowItem (dialogID, LABEL_ORIENTATION);
				
				// 라디오 버튼: 설치방향 (벽세우기)
				DGShowItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD);
				// 라디오 버튼: 설치방향 (벽눕히기)
				DGShowItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD);

				if (placingZone.cells [0][idxCell].libPart.plywood.w_dir_wall == true) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD, true);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD, false);
				} else if (placingZone.cells [0][idxCell].libPart.plywood.w_dir_wall == false) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD, false);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD, true);
				}
			} else if (placingZone.cells [0][idxCell].objType == WOOD) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, WOOD + 1);

				// 라벨: 너비
				DGShowItem (dialogID, LABEL_WIDTH);

				// Edit 컨트롤: 너비
				DGShowItem (dialogID, EDITCONTROL_WIDTH);
				DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.wood.w_h);
				DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.005);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.000);

				// 라벨: 높이
				DGShowItem (dialogID, LABEL_HEIGHT);

				// Edit 컨트롤: 높이
				DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [0][idxCell].libPart.wood.w_leng);
				DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.010);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 3.600);

				// 라벨: 두께
				DGShowItem (dialogID, LABEL_THK);

				// Edit 컨트롤: 두께
				DGShowItem (dialogID, EDITCONTROL_THK);
				DGSetItemValDouble (dialogID, EDITCONTROL_THK, placingZone.cells [0][idxCell].libPart.wood.w_w);
				DGSetItemMinDouble (dialogID, EDITCONTROL_THK, 0.005);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_THK, 1.000);
			}

			//////////////////////////////////////////////////////////// 필드 생성 (예전 셀)
			// 라벨: 객체 타입
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 15, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE_PREV, "객체 타입\n[예전 셀]");
			if (idxCell_prev != -1)	DGShowItem (dialogID, LABEL_OBJ_TYPE_PREV);

			// 팝업컨트롤: 객체 타입을 바꿀 수 있는 콤보박스가 맨 위에 나옴
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+0, 20-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM, "인코너판넬");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM, "유로폼");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM, "휠러스페이서");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM, "합판");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_PREV, DG_POPUP_BOTTOM, "목재");
			if (idxCell_prev != -1)	DGShowItem (dialogID, POPUP_OBJ_TYPE_PREV);

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH_PREV, "너비");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+0, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH_PREV, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT_PREV, "높이");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+0, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT_PREV, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 두께
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_THK_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_THK_PREV, "두께");

			// Edit 컨트롤: 두께
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+0, 110-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_THK_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION_PREV, "설치방향");
				
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 779, 100+0, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 779, 100+0, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_PLYWOOD_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_PLYWOOD_PREV, "벽눕히기");

			// 체크박스: 규격폼
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20+0, 50, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_PREV, "규격폼");

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_PREV, "너비");

			// 팝업 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+0, 80-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DG_POPUP_BOTTOM, "200");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+0, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_PREV, "높이");

			// 팝업 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+0, 110-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, DG_POPUP_BOTTOM, "600");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+0, 110-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_PREV, "설치방향");
			
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 780, 100+0, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 780, 100+0, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_EUROFORM_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_EUROFORM_PREV, "벽눕히기");

			// 초기 입력 필드 표시
			if (idxCell_prev != -1) {
				if (placingZone.cells [0][idxCell_prev].objType == INCORNER) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_PREV, INCORNER + 1);

					// 라벨: 너비
					DGShowItem (dialogID, LABEL_WIDTH_PREV);

					// Edit 컨트롤: 너비
					DGShowItem (dialogID, EDITCONTROL_WIDTH_PREV);
					if (idxCell_prev == 0)
						DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV, placingZone.cells [0][idxCell_prev].libPart.incorner.leng_s);
					else if (idxCell_prev > 0)
						DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV, placingZone.cells [0][idxCell_prev].libPart.incorner.wid_s);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.080);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.500);

					// 라벨 높이
					DGShowItem (dialogID, LABEL_HEIGHT_PREV);

					// Edit 컨트롤: 높이
					DGShowItem (dialogID, EDITCONTROL_HEIGHT_PREV);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV, placingZone.cells [0][idxCell_prev].libPart.incorner.hei_s);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 0.050);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 1.500);

				} else if (placingZone.cells [0][idxCell_prev].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_PREV, EUROFORM + 1);

					// 체크박스: 규격폼
					DGShowItem (dialogID, CHECKBOX_SET_STANDARD_PREV);
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV, placingZone.cells [0][idxCell_prev].libPart.form.eu_stan_onoff);

					if (placingZone.cells [0][idxCell_prev].libPart.form.eu_stan_onoff == true) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_PREV);

						// 팝업 컨트롤: 너비
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV);
						if (abs(placingZone.cells [0][idxCell_prev].libPart.form.eu_wid - 0.600) < EPS)		popupSelectedIdx = 1;
						if (abs(placingZone.cells [0][idxCell_prev].libPart.form.eu_wid - 0.500) < EPS)		popupSelectedIdx = 2;
						if (abs(placingZone.cells [0][idxCell_prev].libPart.form.eu_wid - 0.450) < EPS)		popupSelectedIdx = 3;
						if (abs(placingZone.cells [0][idxCell_prev].libPart.form.eu_wid - 0.400) < EPS)		popupSelectedIdx = 4;
						if (abs(placingZone.cells [0][idxCell_prev].libPart.form.eu_wid - 0.300) < EPS)		popupSelectedIdx = 5;
						if (abs(placingZone.cells [0][idxCell_prev].libPart.form.eu_wid - 0.200) < EPS)		popupSelectedIdx = 6;
						DGPopUpSelectItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, popupSelectedIdx);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_PREV);

						// 팝업 컨트롤: 높이
						DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV);
						if (abs(placingZone.cells [0][idxCell_prev].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs(placingZone.cells [0][idxCell_prev].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs(placingZone.cells [0][idxCell_prev].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, popupSelectedIdx);
					} else if (placingZone.cells [0][idxCell_prev].libPart.form.eu_stan_onoff == false) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_PREV);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV);
						DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV, placingZone.cells [0][idxCell_prev].libPart.form.eu_wid2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV, 0.900);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_PREV);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV);
						DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV, placingZone.cells [0][idxCell_prev].libPart.form.eu_hei2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV, 1.500);
					}

					// 라벨: 설치방향
					DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_PREV);
				
					// 라디오 버튼: 설치방향 (벽세우기)
					DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV);
					// 라디오 버튼: 설치방향 (벽눕히기)
					DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM_PREV);

					if (placingZone.cells [0][idxCell_prev].libPart.form.u_ins_wall == true) {
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM_PREV, false);
					} else if (placingZone.cells [0][idxCell_prev].libPart.form.u_ins_wall == false) {
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV, false);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM_PREV, true);
					}
				} else if (placingZone.cells [0][idxCell_prev].objType == FILLERSPACER) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_PREV, FILLERSPACER + 1);

					// 라벨: 너비
					DGShowItem (dialogID, LABEL_WIDTH_PREV);

					// Edit 컨트롤: 너비
					DGShowItem (dialogID, EDITCONTROL_WIDTH_PREV);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV, placingZone.cells [0][idxCell_prev].libPart.fillersp.f_thk);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.010);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.050);

					// 라벨 높이
					DGShowItem (dialogID, LABEL_HEIGHT_PREV);

					// Edit 컨트롤: 높이
					DGShowItem (dialogID, EDITCONTROL_HEIGHT_PREV);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV, placingZone.cells [0][idxCell_prev].libPart.fillersp.f_leng);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 0.150);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 2.400);

				} else if (placingZone.cells [0][idxCell_prev].objType == PLYWOOD) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_PREV, PLYWOOD + 1);

					// 라벨: 너비
					DGShowItem (dialogID, LABEL_WIDTH_PREV);

					// Edit 컨트롤: 너비
					DGShowItem (dialogID, EDITCONTROL_WIDTH_PREV);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV, placingZone.cells [0][idxCell_prev].libPart.plywood.p_wid);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.110);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_PREV, 1.220);

					// 라벨: 높이
					DGShowItem (dialogID, LABEL_HEIGHT_PREV);

					// Edit 컨트롤: 높이
					DGShowItem (dialogID, EDITCONTROL_HEIGHT_PREV);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV, placingZone.cells [0][idxCell_prev].libPart.plywood.p_leng);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 0.110);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 2.440);

					// 라벨: 설치방향
					DGShowItem (dialogID, LABEL_ORIENTATION_PREV);
				
					// 라디오 버튼: 설치방향 (벽세우기)
					DGShowItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV);
					// 라디오 버튼: 설치방향 (벽눕히기)
					DGShowItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD_PREV);

					if (placingZone.cells [0][idxCell_prev].libPart.plywood.w_dir_wall == true) {
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD_PREV, false);
					} else if (placingZone.cells [0][idxCell_prev].libPart.plywood.w_dir_wall == false) {
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV, false);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD_PREV, true);
					}
				} else if (placingZone.cells [0][idxCell_prev].objType == WOOD) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_PREV, WOOD + 1);

					// 라벨: 너비
					DGShowItem (dialogID, LABEL_WIDTH_PREV);

					// Edit 컨트롤: 너비
					DGShowItem (dialogID, EDITCONTROL_WIDTH_PREV);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV, placingZone.cells [0][idxCell_prev].libPart.wood.w_h);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.005);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_PREV, 1.000);

					// 라벨: 높이
					DGShowItem (dialogID, LABEL_HEIGHT_PREV);

					// Edit 컨트롤: 높이
					DGShowItem (dialogID, EDITCONTROL_HEIGHT_PREV);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV, placingZone.cells [0][idxCell_prev].libPart.wood.w_leng);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 0.010);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 3.600);

					// 라벨: 두께
					DGShowItem (dialogID, LABEL_THK_PREV);

					// Edit 컨트롤: 두께
					DGShowItem (dialogID, EDITCONTROL_THK_PREV);
					DGSetItemValDouble (dialogID, EDITCONTROL_THK_PREV, placingZone.cells [0][idxCell_prev].libPart.wood.w_w);
					DGSetItemMinDouble (dialogID, EDITCONTROL_THK_PREV, 0.005);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_THK_PREV, 1.000);
				}
			}

			//////////////////////////////////////////////////////////// 필드 생성 (다음 셀)
			// 라벨: 객체 타입
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 15, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE_NEXT, "객체 타입\n[다음 셀]");
			if (idxCell_next != -1)	DGShowItem (dialogID, LABEL_OBJ_TYPE_NEXT);

			// 팝업컨트롤: 객체 타입을 바꿀 수 있는 콤보박스가 맨 위에 나옴
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+480, 20-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM, "인코너판넬");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM, "유로폼");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM, "휠러스페이서");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM, "합판");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE_NEXT, DG_POPUP_BOTTOM, "목재");
			if (idxCell_next != -1)	DGShowItem (dialogID, POPUP_OBJ_TYPE_NEXT);

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH_NEXT, "너비");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+480, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH_NEXT, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT_NEXT, "높이");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+480, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 두께
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_THK_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_THK_NEXT, "두께");

			// Edit 컨트롤: 두께
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+480, 110-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_THK_NEXT, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION_NEXT, "설치방향");
				
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 781, 100+480, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 781, 100+480, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_PLYWOOD_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_PLYWOOD_NEXT, "벽눕히기");

			// 체크박스: 규격폼
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20+480, 50, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_NEXT, "규격폼");

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_NEXT, "너비");

			// 팝업 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+480, 80-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DG_POPUP_BOTTOM, "200");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+480, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_NEXT, "높이");

			// 팝업 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+480, 110-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, DG_POPUP_BOTTOM, "600");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+480, 110-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_NEXT, "설치방향");
			
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 782, 100+480, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 782, 100+480, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_EUROFORM_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_EUROFORM_NEXT, "벽눕히기");

			// 초기 입력 필드 표시
			if (idxCell_next != -1) {
				if (placingZone.cells [0][idxCell_next].objType == INCORNER) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_NEXT, INCORNER + 1);

					// 라벨: 너비
					DGShowItem (dialogID, LABEL_WIDTH_NEXT);

					// Edit 컨트롤: 너비
					DGShowItem (dialogID, EDITCONTROL_WIDTH_NEXT);
					if (idxCell_next == 0)
						DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT, placingZone.cells [0][idxCell_next].libPart.incorner.leng_s);
					else if (idxCell_next > 0)
						DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT, placingZone.cells [0][idxCell_next].libPart.incorner.wid_s);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.080);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.500);

					// 라벨 높이
					DGShowItem (dialogID, LABEL_HEIGHT_NEXT);

					// Edit 컨트롤: 높이
					DGShowItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, placingZone.cells [0][idxCell_next].libPart.incorner.hei_s);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 0.050);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 1.500);

				} else if (placingZone.cells [0][idxCell_next].objType == EUROFORM) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_NEXT, EUROFORM + 1);

					// 체크박스: 규격폼
					DGShowItem (dialogID, CHECKBOX_SET_STANDARD_NEXT);
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT, placingZone.cells [0][idxCell_next].libPart.form.eu_stan_onoff);

					if (placingZone.cells [0][idxCell_next].libPart.form.eu_stan_onoff == true) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_NEXT);

						// 팝업 컨트롤: 너비
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT);
						if (abs(placingZone.cells [0][idxCell_next].libPart.form.eu_wid - 0.600) < EPS)		popupSelectedIdx = 1;
						if (abs(placingZone.cells [0][idxCell_next].libPart.form.eu_wid - 0.500) < EPS)		popupSelectedIdx = 2;
						if (abs(placingZone.cells [0][idxCell_next].libPart.form.eu_wid - 0.450) < EPS)		popupSelectedIdx = 3;
						if (abs(placingZone.cells [0][idxCell_next].libPart.form.eu_wid - 0.400) < EPS)		popupSelectedIdx = 4;
						if (abs(placingZone.cells [0][idxCell_next].libPart.form.eu_wid - 0.300) < EPS)		popupSelectedIdx = 5;
						if (abs(placingZone.cells [0][idxCell_next].libPart.form.eu_wid - 0.200) < EPS)		popupSelectedIdx = 6;
						DGPopUpSelectItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, popupSelectedIdx);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_NEXT);

						// 팝업 컨트롤: 높이
						DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT);
						if (abs(placingZone.cells [0][idxCell_next].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
						if (abs(placingZone.cells [0][idxCell_next].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
						if (abs(placingZone.cells [0][idxCell_next].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
						DGPopUpSelectItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, popupSelectedIdx);
					} else if (placingZone.cells [0][idxCell_next].libPart.form.eu_stan_onoff == false) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_NEXT);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT);
						DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT, placingZone.cells [0][idxCell_next].libPart.form.eu_wid2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT, 0.900);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_NEXT);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT);
						DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT, placingZone.cells [0][idxCell_next].libPart.form.eu_hei2);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT, 1.500);
					}

					// 라벨: 설치방향
					DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_NEXT);
				
					// 라디오 버튼: 설치방향 (벽세우기)
					DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT);
					// 라디오 버튼: 설치방향 (벽눕히기)
					DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM_NEXT);

					if (placingZone.cells [0][idxCell_next].libPart.form.u_ins_wall == true) {
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM_NEXT, false);
					} else if (placingZone.cells [0][idxCell_next].libPart.form.u_ins_wall == false) {
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT, false);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM_NEXT, true);
					}
				} else if (placingZone.cells [0][idxCell_next].objType == FILLERSPACER) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_NEXT, FILLERSPACER + 1);

					// 라벨: 너비
					DGShowItem (dialogID, LABEL_WIDTH_NEXT);

					// Edit 컨트롤: 너비
					DGShowItem (dialogID, EDITCONTROL_WIDTH_NEXT);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT, placingZone.cells [0][idxCell_next].libPart.fillersp.f_thk);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.010);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.050);

					// 라벨 높이
					DGShowItem (dialogID, LABEL_HEIGHT_NEXT);

					// Edit 컨트롤: 높이
					DGShowItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, placingZone.cells [0][idxCell_next].libPart.fillersp.f_leng);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 0.150);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 2.400);

				} else if (placingZone.cells [0][idxCell_next].objType == PLYWOOD) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_NEXT, PLYWOOD + 1);

					// 라벨: 너비
					DGShowItem (dialogID, LABEL_WIDTH_NEXT);

					// Edit 컨트롤: 너비
					DGShowItem (dialogID, EDITCONTROL_WIDTH_NEXT);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT, placingZone.cells [0][idxCell_next].libPart.plywood.p_wid);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.110);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 1.220);

					// 라벨: 높이
					DGShowItem (dialogID, LABEL_HEIGHT_NEXT);

					// Edit 컨트롤: 높이
					DGShowItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, placingZone.cells [0][idxCell_next].libPart.plywood.p_leng);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 0.110);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 2.440);

					// 라벨: 설치방향
					DGShowItem (dialogID, LABEL_ORIENTATION_NEXT);
				
					// 라디오 버튼: 설치방향 (벽세우기)
					DGShowItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT);
					// 라디오 버튼: 설치방향 (벽눕히기)
					DGShowItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD_NEXT);

					if (placingZone.cells [0][idxCell_next].libPart.plywood.w_dir_wall == true) {
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD_NEXT, false);
					} else if (placingZone.cells [0][idxCell_next].libPart.plywood.w_dir_wall == false) {
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT, false);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD_NEXT, true);
					}
				} else if (placingZone.cells [0][idxCell_next].objType == WOOD) {
					DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE_NEXT, WOOD + 1);

					// 라벨: 너비
					DGShowItem (dialogID, LABEL_WIDTH_NEXT);

					// Edit 컨트롤: 너비
					DGShowItem (dialogID, EDITCONTROL_WIDTH_NEXT);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT, placingZone.cells [0][idxCell_next].libPart.wood.w_h);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.005);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 1.000);

					// 라벨: 높이
					DGShowItem (dialogID, LABEL_HEIGHT_NEXT);

					// Edit 컨트롤: 높이
					DGShowItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, placingZone.cells [0][idxCell_next].libPart.wood.w_leng);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 0.010);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 3.600);

					// 라벨: 두께
					DGShowItem (dialogID, LABEL_HEIGHT_NEXT);

					// Edit 컨트롤: 두께
					DGShowItem (dialogID, EDITCONTROL_THK_NEXT);
					DGSetItemValDouble (dialogID, EDITCONTROL_THK_NEXT, placingZone.cells [0][idxCell_next].libPart.wood.w_w);
					DGSetItemMinDouble (dialogID, EDITCONTROL_THK_NEXT, 0.005);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_THK_NEXT, 1.000);
				}
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
					DGHideItem (dialogID, LABEL_THK);
					DGHideItem (dialogID, EDITCONTROL_THK);
					DGHideItem (dialogID, LABEL_ORIENTATION);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD);
					DGHideItem (dialogID, CHECKBOX_SET_STANDARD);
					DGHideItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);
					DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
					DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
					DGHideItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);
					DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
					DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
					DGHideItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

					DGShowItem (dialogID, LABEL_OBJ_TYPE);
					DGShowItem (dialogID, POPUP_OBJ_TYPE);

					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == INCORNER + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.080);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.500);

						// 라벨 높이
						DGShowItem (dialogID, LABEL_HEIGHT);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.500);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
						// 체크박스: 규격폼
						DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, true);

						// 라벨: 너비
						DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

						// 규격폼이면,
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
							// 팝업 컨트롤: 너비
							DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);

							// 팝업 컨트롤: 높이
							DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
						} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
							// Edit 컨트롤: 너비
							DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
							DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
							DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);

							// Edit 컨트롤: 높이
							DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
							DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
							DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
						}

						// 라벨: 설치방향
						DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
				
						// 라디오 버튼: 설치방향 (벽세우기)
						DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
						// 라디오 버튼: 설치방향 (벽눕히기)
						DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, false);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == FILLERSPACER + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.010);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.050);

						// 라벨 높이
						DGShowItem (dialogID, LABEL_HEIGHT);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.150);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.400);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PLYWOOD + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.110);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.220);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_HEIGHT);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.110);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.440);

						// 라벨: 설치방향
						DGShowItem (dialogID, LABEL_ORIENTATION);
				
						// 라디오 버튼: 설치방향 (벽세우기)
						DGShowItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD);
						// 라디오 버튼: 설치방향 (벽눕히기)
						DGShowItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD);

						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD, false);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == WOOD + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.005);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.000);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_HEIGHT);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.010);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 3.600);

						// 라벨: 두께
						DGShowItem (dialogID, LABEL_THK);

						// Edit 컨트롤: 두께
						DGShowItem (dialogID, EDITCONTROL_THK);
						DGSetItemMinDouble (dialogID, EDITCONTROL_THK, 0.005);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_THK, 1.000);
					}

					break;

				case POPUP_OBJ_TYPE_PREV:	// 객체 타입 콤보박스 값을 변경할 때마다 입력 필드가 달라짐 (변경해야 하므로 Cell 값을 불러오지 않음)
					//////////////////////////////////////////////////////////// 필드 생성 (예전 셀)
					// 일단 항목을 숨기고, 객체 타입 관련 항목만 표시함
					DGHideItem (dialogID, LABEL_WIDTH_PREV);
					DGHideItem (dialogID, EDITCONTROL_WIDTH_PREV);
					DGHideItem (dialogID, LABEL_HEIGHT_PREV);
					DGHideItem (dialogID, EDITCONTROL_HEIGHT_PREV);
					DGHideItem (dialogID, LABEL_THK_PREV);
					DGHideItem (dialogID, EDITCONTROL_THK_PREV);
					DGHideItem (dialogID, LABEL_ORIENTATION_PREV);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD_PREV);
					DGHideItem (dialogID, CHECKBOX_SET_STANDARD_PREV);
					DGHideItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_PREV);
					DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV);
					DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV);
					DGHideItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_PREV);
					DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV);
					DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV);
					DGHideItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_PREV);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_EUROFORM_PREV);

					DGShowItem (dialogID, LABEL_OBJ_TYPE_PREV);
					DGShowItem (dialogID, POPUP_OBJ_TYPE_PREV);

					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == INCORNER + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH_PREV);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.080);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.500);

						// 라벨 높이
						DGShowItem (dialogID, LABEL_HEIGHT_PREV);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 1.500);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == EUROFORM + 1) {
						// 체크박스: 규격폼
						DGShowItem (dialogID, CHECKBOX_SET_STANDARD_PREV);
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV, true);

						// 라벨: 너비
						DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_PREV);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_PREV);

						// 규격폼이면,
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == TRUE) {
							// 팝업 컨트롤: 너비
							DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV);

							// 팝업 컨트롤: 높이
							DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV);
						} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == FALSE) {
							// Edit 컨트롤: 너비
							DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV);
							DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV);
							DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV, 0.050);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV, 0.900);

							// Edit 컨트롤: 높이
							DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV);
							DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV);
							DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV, 0.050);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV, 1.500);
						}

						// 라벨: 설치방향
						DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_PREV);
				
						// 라디오 버튼: 설치방향 (벽세우기)
						DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV);
						// 라디오 버튼: 설치방향 (벽눕히기)
						DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM_PREV);

						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM_PREV, false);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == FILLERSPACER + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH_PREV);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.010);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.050);

						// 라벨 높이
						DGShowItem (dialogID, LABEL_HEIGHT_PREV);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 0.150);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 2.400);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == PLYWOOD + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH_PREV);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.110);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_PREV, 1.220);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_HEIGHT_PREV);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 0.110);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 2.440);

						// 라벨: 설치방향
						DGShowItem (dialogID, LABEL_ORIENTATION_PREV);
				
						// 라디오 버튼: 설치방향 (벽세우기)
						DGShowItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV);
						// 라디오 버튼: 설치방향 (벽눕히기)
						DGShowItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD_PREV);

						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD_PREV, false);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == WOOD + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH_PREV);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_PREV, 0.005);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_PREV, 1.000);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_HEIGHT_PREV);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 0.010);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_PREV, 3.600);

						// 라벨: 두께
						DGShowItem (dialogID, LABEL_THK_PREV);

						// Edit 컨트롤: 두께
						DGShowItem (dialogID, EDITCONTROL_THK_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_THK_PREV, 0.005);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_THK_PREV, 1.000);
					}

					break;

				case POPUP_OBJ_TYPE_NEXT:	// 객체 타입 콤보박스 값을 변경할 때마다 입력 필드가 달라짐 (변경해야 하므로 Cell 값을 불러오지 않음)
					//////////////////////////////////////////////////////////// 필드 생성 (다음 셀)
					// 일단 항목을 숨기고, 객체 타입 관련 항목만 표시함
					DGHideItem (dialogID, LABEL_WIDTH_NEXT);
					DGHideItem (dialogID, EDITCONTROL_WIDTH_NEXT);
					DGHideItem (dialogID, LABEL_HEIGHT_NEXT);
					DGHideItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
					DGHideItem (dialogID, LABEL_THK_NEXT);
					DGHideItem (dialogID, EDITCONTROL_THK_NEXT);
					DGHideItem (dialogID, LABEL_ORIENTATION_NEXT);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD_NEXT);
					DGHideItem (dialogID, CHECKBOX_SET_STANDARD_NEXT);
					DGHideItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_NEXT);
					DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT);
					DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT);
					DGHideItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_NEXT);
					DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT);
					DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT);
					DGHideItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_NEXT);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_EUROFORM_NEXT);

					DGShowItem (dialogID, LABEL_OBJ_TYPE_NEXT);
					DGShowItem (dialogID, POPUP_OBJ_TYPE_NEXT);

					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == INCORNER + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH_NEXT);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.080);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.500);

						// 라벨 높이
						DGShowItem (dialogID, LABEL_HEIGHT_NEXT);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 1.500);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == EUROFORM + 1) {
						// 체크박스: 규격폼
						DGShowItem (dialogID, CHECKBOX_SET_STANDARD_NEXT);
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT, true);

						// 라벨: 너비
						DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_NEXT);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_NEXT);

						// 규격폼이면,
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT) == TRUE) {
							// 팝업 컨트롤: 너비
							DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT);

							// 팝업 컨트롤: 높이
							DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT);
						} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT) == FALSE) {
							// Edit 컨트롤: 너비
							DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT);
							DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT);
							DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT, 0.050);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT, 0.900);

							// Edit 컨트롤: 높이
							DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT);
							DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT);
							DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT, 0.050);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT, 1.500);
						}

						// 라벨: 설치방향
						DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_NEXT);
				
						// 라디오 버튼: 설치방향 (벽세우기)
						DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT);
						// 라디오 버튼: 설치방향 (벽눕히기)
						DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM_NEXT);

						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM_NEXT, false);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == FILLERSPACER + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH_NEXT);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.010);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.050);

						// 라벨 높이
						DGShowItem (dialogID, LABEL_HEIGHT_NEXT);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 0.150);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 2.400);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == PLYWOOD + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH_NEXT);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.110);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 1.220);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_HEIGHT_NEXT);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 0.110);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 2.440);

						// 라벨: 설치방향
						DGShowItem (dialogID, LABEL_ORIENTATION_NEXT);
				
						// 라디오 버튼: 설치방향 (벽세우기)
						DGShowItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT);
						// 라디오 버튼: 설치방향 (벽눕히기)
						DGShowItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD_NEXT);

						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD_NEXT, false);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == WOOD + 1) {
						// 라벨: 너비
						DGShowItem (dialogID, LABEL_WIDTH_NEXT);

						// Edit 컨트롤: 너비
						DGShowItem (dialogID, EDITCONTROL_WIDTH_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 0.005);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH_NEXT, 1.000);

						// 라벨: 높이
						DGShowItem (dialogID, LABEL_HEIGHT_NEXT);

						// Edit 컨트롤: 높이
						DGShowItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 0.010);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT_NEXT, 3.600);

						// 라벨: 두께
						DGShowItem (dialogID, LABEL_THK_NEXT);

						// Edit 컨트롤: 두께
						DGShowItem (dialogID, EDITCONTROL_THK_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_THK_NEXT, 0.005);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_THK_NEXT, 1.000);
					}

					break;

				case CHECKBOX_SET_STANDARD:	// 유로폼의 경우, 규격폼 체크박스 값을 바꿀 때마다 너비, 높이 입력 필드 타입이 바뀜
					//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
						// 팝업 컨트롤: 너비
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
						// 팝업 컨트롤: 높이
						DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
					} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
						// Edit 컨트롤: 너비
						DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);
						// Edit 컨트롤: 높이
						DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
					}

					break;

				case CHECKBOX_SET_STANDARD_PREV:	// 유로폼의 경우, 규격폼 체크박스 값을 바꿀 때마다 너비, 높이 입력 필드 타입이 바뀜
					//////////////////////////////////////////////////////////// 필드 생성 (예전 셀)
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == TRUE) {
						// 팝업 컨트롤: 너비
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV);
						// 팝업 컨트롤: 높이
						DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV);
					} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == FALSE) {
						// Edit 컨트롤: 너비
						DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV, 0.900);
						// Edit 컨트롤: 높이
						DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV, 1.500);
					}

					break;

				case CHECKBOX_SET_STANDARD_NEXT:	// 유로폼의 경우, 규격폼 체크박스 값을 바꿀 때마다 너비, 높이 입력 필드 타입이 바뀜
					//////////////////////////////////////////////////////////// 필드 생성 (다음 셀)
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT) == TRUE) {
						// 팝업 컨트롤: 너비
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT);
						// 팝업 컨트롤: 높이
						DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT);
					} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT) == FALSE) {
						// Edit 컨트롤: 너비
						DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT, 0.900);
						// Edit 컨트롤: 높이
						DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT, 1.500);
					}

					break;
			}

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					// wallPlacerHandler2 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
					idxCell = (clickedBtnItemIdx - itemInitIdx) * 2;
					while (idxCell >= ((placingZone.eu_count_hor + 2) * 2))
						idxCell -= ((placingZone.eu_count_hor + 2) * 2);

					// 현재 셀이 중간 셀이면,
					if ( (idxCell > 0) && (idxCell < (placingZone.nCells - 1)) ) {
						idxCell_prev = idxCell - 1;
						idxCell_next = idxCell + 1;
					// 현재 셀이 맨 처음 셀이면,
					} else if (idxCell == 0) {
						idxCell_prev = -1;
						idxCell_next = idxCell + 1;
					// 현재 셀이 맨 끝 셀이면,
					} else if (idxCell == (placingZone.nCells - 1)) {
						idxCell_prev = idxCell - 1;
						idxCell_next = -1;
					}

					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {

						//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
						// 입력한 값을 다시 셀에 저장
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
							placingZone.cells [xx][idxCell].objType = NONE;

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == INCORNER + 1) {
							placingZone.cells [xx][idxCell].objType = INCORNER;

							// 너비
							if (idxCell == 0) {
								placingZone.cells [xx][idxCell].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
								placingZone.cells [xx][idxCell].libPart.incorner.wid_s = 0.100;
							} else if (idxCell > 0) {
								placingZone.cells [xx][idxCell].libPart.incorner.leng_s = 0.100;
								placingZone.cells [xx][idxCell].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
							}
							placingZone.cells [xx][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

							// 높이
							placingZone.cells [xx][idxCell].libPart.incorner.hei_s = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
							placingZone.cells [xx][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
							placingZone.cells [xx][idxCell].objType = EUROFORM;

							// 규격폼
							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
								placingZone.cells [xx][idxCell].libPart.form.eu_stan_onoff = true;
							else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE)
								placingZone.cells [xx][idxCell].libPart.form.eu_stan_onoff = false;

							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
								// 너비
								placingZone.cells [xx][idxCell].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS)).ToCStr ()) / 1000.0;
								placingZone.cells [xx][idxCell].horLen = placingZone.cells [xx][idxCell].libPart.form.eu_wid;
								// 높이
								placingZone.cells [xx][idxCell].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS)).ToCStr ()) / 1000.0;
								placingZone.cells [xx][idxCell].verLen = placingZone.cells [xx][idxCell].libPart.form.eu_hei;
							} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
								// 너비
								placingZone.cells [xx][idxCell].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
								placingZone.cells [xx][idxCell].horLen = placingZone.cells [xx][idxCell].libPart.form.eu_wid2;
								// 높이
								placingZone.cells [xx][idxCell].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
								placingZone.cells [xx][idxCell].verLen = placingZone.cells [xx][idxCell].libPart.form.eu_hei2;
							}

							// 설치방향
							if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == TRUE)
								placingZone.cells [xx][idxCell].libPart.form.u_ins_wall = true;
							else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == FALSE) {
								placingZone.cells [xx][idxCell].libPart.form.u_ins_wall = false;
								// 가로, 세로 길이 교환
								temp = placingZone.cells [xx][idxCell].horLen;
								placingZone.cells [xx][idxCell].horLen = placingZone.cells [xx][idxCell].verLen;
								placingZone.cells [xx][idxCell].verLen = temp;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == FILLERSPACER + 1) {
							placingZone.cells [xx][idxCell].objType = FILLERSPACER;

							// 너비
							placingZone.cells [xx][idxCell].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
							placingZone.cells [xx][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

							// 높이
							placingZone.cells [xx][idxCell].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
							placingZone.cells [xx][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PLYWOOD + 1) {
							placingZone.cells [xx][idxCell].objType = PLYWOOD;

							// 너비
							placingZone.cells [xx][idxCell].libPart.plywood.p_wid = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
							placingZone.cells [xx][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

							// 높이
							placingZone.cells [xx][idxCell].libPart.plywood.p_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
							placingZone.cells [xx][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);

							// 설치방향
							if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD) == TRUE)
								placingZone.cells [xx][idxCell].libPart.plywood.w_dir_wall = true;
							else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD) == FALSE) {
								placingZone.cells [xx][idxCell].libPart.plywood.w_dir_wall = false;
								// 가로, 세로 길이 교환
								temp = placingZone.cells [xx][idxCell].horLen;
								placingZone.cells [xx][idxCell].horLen = placingZone.cells [xx][idxCell].verLen;
								placingZone.cells [xx][idxCell].verLen = temp;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == WOOD + 1) {
							placingZone.cells [xx][idxCell].objType = WOOD;

							// 너비
							placingZone.cells [xx][idxCell].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
							placingZone.cells [xx][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

							// 높이
							placingZone.cells [xx][idxCell].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
							placingZone.cells [xx][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);

							// 두께
							placingZone.cells [xx][idxCell].libPart.wood.w_w = DGGetItemValDouble (dialogID, EDITCONTROL_THK);

							// 각도: 90도
							placingZone.cells [xx][idxCell].libPart.wood.w_ang = DegreeToRad (90.0);
						}

						//////////////////////////////////////////////////////////// 필드 생성 (예전 셀)
						if (idxCell_prev != -1) {
							// 입력한 값을 다시 셀에 저장
							if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == NONE + 1) {
								placingZone.cells [xx][idxCell_prev].objType = NONE;

							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == INCORNER + 1) {
								placingZone.cells [xx][idxCell_prev].objType = INCORNER;

								// 너비
								if (idxCell_prev == 0) {
									placingZone.cells [xx][idxCell_prev].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);
									placingZone.cells [xx][idxCell_prev].libPart.incorner.wid_s = 0.100;
								} else if (idxCell_prev > 0) {
									placingZone.cells [xx][idxCell_prev].libPart.incorner.leng_s = 0.100;
									placingZone.cells [xx][idxCell_prev].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);
								}
								placingZone.cells [xx][idxCell_prev].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);

								// 높이
								placingZone.cells [xx][idxCell_prev].libPart.incorner.hei_s = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
								placingZone.cells [xx][idxCell_prev].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == EUROFORM + 1) {
								placingZone.cells [xx][idxCell_prev].objType = EUROFORM;

								// 규격폼
								if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == TRUE)
									placingZone.cells [xx][idxCell_prev].libPart.form.eu_stan_onoff = true;
								else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == FALSE)
									placingZone.cells [xx][idxCell_prev].libPart.form.eu_stan_onoff = false;

								if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == TRUE) {
									// 너비
									placingZone.cells [xx][idxCell_prev].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV)).ToCStr ()) / 1000.0;
									placingZone.cells [xx][idxCell_prev].horLen = placingZone.cells [xx][idxCell_prev].libPart.form.eu_wid;
									// 높이
									placingZone.cells [xx][idxCell_prev].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV)).ToCStr ()) / 1000.0;
									placingZone.cells [xx][idxCell_prev].verLen = placingZone.cells [xx][idxCell_prev].libPart.form.eu_hei;
								} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == FALSE) {
									// 너비
									placingZone.cells [xx][idxCell_prev].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV);
									placingZone.cells [xx][idxCell_prev].horLen = placingZone.cells [xx][idxCell_prev].libPart.form.eu_wid2;
									// 높이
									placingZone.cells [xx][idxCell_prev].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV);
									placingZone.cells [xx][idxCell_prev].verLen = placingZone.cells [xx][idxCell_prev].libPart.form.eu_hei2;
								}

								// 설치방향
								if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV) == TRUE)
									placingZone.cells [xx][idxCell_prev].libPart.form.u_ins_wall = true;
								else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV) == FALSE) {
									placingZone.cells [xx][idxCell_prev].libPart.form.u_ins_wall = false;
									// 가로, 세로 길이 교환
									temp = placingZone.cells [xx][idxCell_prev].horLen;
									placingZone.cells [xx][idxCell_prev].horLen = placingZone.cells [xx][idxCell_prev].verLen;
									placingZone.cells [xx][idxCell_prev].verLen = temp;
								}
							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == FILLERSPACER + 1) {
								placingZone.cells [xx][idxCell_prev].objType = FILLERSPACER;

								// 너비
								placingZone.cells [xx][idxCell_prev].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);
								placingZone.cells [xx][idxCell_prev].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);

								// 높이
								placingZone.cells [xx][idxCell_prev].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
								placingZone.cells [xx][idxCell_prev].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == PLYWOOD + 1) {
								placingZone.cells [xx][idxCell_prev].objType = PLYWOOD;

								// 너비
								placingZone.cells [xx][idxCell_prev].libPart.plywood.p_wid = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);
								placingZone.cells [xx][idxCell_prev].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);

								// 높이
								placingZone.cells [xx][idxCell_prev].libPart.plywood.p_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
								placingZone.cells [xx][idxCell_prev].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);

								// 설치방향
								if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV) == TRUE)
									placingZone.cells [xx][idxCell_prev].libPart.plywood.w_dir_wall = true;
								else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV) == FALSE) {
									placingZone.cells [xx][idxCell_prev].libPart.plywood.w_dir_wall = false;
									// 가로, 세로 길이 교환
									temp = placingZone.cells [xx][idxCell_prev].horLen;
									placingZone.cells [xx][idxCell_prev].horLen = placingZone.cells [xx][idxCell_prev].verLen;
									placingZone.cells [xx][idxCell_prev].verLen = temp;
								}
							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == WOOD + 1) {
								placingZone.cells [xx][idxCell_prev].objType = WOOD;

								// 너비
								placingZone.cells [xx][idxCell_prev].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);
								placingZone.cells [xx][idxCell_prev].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);

								// 높이
								placingZone.cells [xx][idxCell_prev].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
								placingZone.cells [xx][idxCell_prev].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);

								// 두께
								placingZone.cells [xx][idxCell_prev].libPart.wood.w_w = DGGetItemValDouble (dialogID, EDITCONTROL_THK_PREV);
							
								// 각도: 90도
								placingZone.cells [xx][idxCell_prev].libPart.wood.w_ang = DegreeToRad (90.0);
							}
						}

						//////////////////////////////////////////////////////////// 필드 생성 (다음 셀)
						if (idxCell_next != -1) {
							// 입력한 값을 다시 셀에 저장
							if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == NONE + 1) {
								placingZone.cells [xx][idxCell_next].objType = NONE;

							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == INCORNER + 1) {
								placingZone.cells [xx][idxCell_next].objType = INCORNER;

								// 너비
								if (idxCell_next == 0) {
									placingZone.cells [xx][idxCell_next].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);
									placingZone.cells [xx][idxCell_next].libPart.incorner.wid_s = 0.100;
								} else if (idxCell_next > 0) {
									placingZone.cells [xx][idxCell_next].libPart.incorner.leng_s = 0.100;
									placingZone.cells [xx][idxCell_next].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);
								}
								placingZone.cells [xx][idxCell_next].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);

								// 높이
								placingZone.cells [xx][idxCell_next].libPart.incorner.hei_s = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
								placingZone.cells [xx][idxCell_next].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == EUROFORM + 1) {
								placingZone.cells [xx][idxCell_next].objType = EUROFORM;

								// 규격폼
								if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT) == TRUE)
									placingZone.cells [xx][idxCell_next].libPart.form.eu_stan_onoff = true;
								else
									placingZone.cells [xx][idxCell_next].libPart.form.eu_stan_onoff = false;

								if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT) == TRUE) {
									// 너비
									placingZone.cells [xx][idxCell_next].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT)).ToCStr ()) / 1000.0;
									placingZone.cells [xx][idxCell_next].horLen = placingZone.cells [xx][idxCell_next].libPart.form.eu_wid;
									// 높이
									placingZone.cells [xx][idxCell_next].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT)).ToCStr ()) / 1000.0;
									placingZone.cells [xx][idxCell_next].verLen = placingZone.cells [xx][idxCell_next].libPart.form.eu_hei;
								} else {
									// 너비
									placingZone.cells [xx][idxCell_next].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT);
									placingZone.cells [xx][idxCell_next].horLen = placingZone.cells [xx][idxCell_next].libPart.form.eu_wid2;
									// 높이
									placingZone.cells [xx][idxCell_next].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT);
									placingZone.cells [xx][idxCell_next].verLen = placingZone.cells [xx][idxCell_next].libPart.form.eu_hei2;
								}

								// 설치방향
								if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT) == TRUE)
									placingZone.cells [xx][idxCell_next].libPart.form.u_ins_wall = true;
								else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT) == FALSE) {
									placingZone.cells [xx][idxCell_next].libPart.form.u_ins_wall = false;
									// 가로, 세로 길이 교환
									temp = placingZone.cells [xx][idxCell_next].horLen;
									placingZone.cells [xx][idxCell_next].horLen = placingZone.cells [xx][idxCell_next].verLen;
									placingZone.cells [xx][idxCell_next].verLen = temp;
								}
							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == FILLERSPACER + 1) {
								placingZone.cells [xx][idxCell_next].objType = FILLERSPACER;

								// 너비
								placingZone.cells [xx][idxCell_next].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);
								placingZone.cells [xx][idxCell_next].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);

								// 높이
								placingZone.cells [xx][idxCell_next].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
								placingZone.cells [xx][idxCell_next].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == PLYWOOD + 1) {
								placingZone.cells [xx][idxCell_next].objType = PLYWOOD;

								// 너비
								placingZone.cells [xx][idxCell_next].libPart.plywood.p_wid = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);
								placingZone.cells [xx][idxCell_next].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);

								// 높이
								placingZone.cells [xx][idxCell_next].libPart.plywood.p_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
								placingZone.cells [xx][idxCell_next].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);

								// 설치방향
								if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT) == TRUE)
									placingZone.cells [xx][idxCell_next].libPart.plywood.w_dir_wall = true;
								else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT) == FALSE){
									placingZone.cells [xx][idxCell_next].libPart.plywood.w_dir_wall = false;
									// 가로, 세로 길이 교환
									temp = placingZone.cells [xx][idxCell_next].horLen;
									placingZone.cells [xx][idxCell_next].horLen = placingZone.cells [xx][idxCell_next].verLen;
									placingZone.cells [xx][idxCell_next].verLen = temp;
								}
							} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == WOOD + 1) {
								placingZone.cells [xx][idxCell_next].objType = WOOD;

								// 너비
								placingZone.cells [xx][idxCell_next].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);
								placingZone.cells [xx][idxCell_next].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);

								// 높이
								placingZone.cells [xx][idxCell_next].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
								placingZone.cells [xx][idxCell_next].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);

								// 두께
								placingZone.cells [xx][idxCell_next].libPart.wood.w_w = DGGetItemValDouble (dialogID, EDITCONTROL_THK_NEXT);

								// 각도: 90도
								placingZone.cells [xx][idxCell_next].libPart.wood.w_ang = DegreeToRad (90.0);
							}
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

// 보 하부의 합판/목재 영역을 유로폼으로 채울지 물어보는 4차 다이얼로그
short DGCALLBACK wallPlacerHandler4 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	GSErrCode	err = NoError;
	API_Element	elem;
	short	result;
	short	idxItem;
	CellForWall		insCell, insCellB;	// 삽입할 임시 셀
	double	spWidth = 0.0;		// 보 하부 합판/목재가 차지한 총 너비
	double	spHeight = 0.0;		// 보 하부 합판/목재가 차지한 총 높이
	double	temp;
	double	remainHeight;
	short	xx;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "보 하부 채우기");

			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 70, 280, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "예");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 160, 280, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "아니오");
			DGShowItem (dialogID, DG_CANCEL);

			// 라벨: 설명
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 50, 10, 230, 23);
			DGSetItemFont (dialogID, LABEL_DESC1_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DESC1_BEAMAROUND, "보 하부에 다음 크기의 공간이 있습니다.");
			DGShowItem (dialogID, LABEL_DESC1_BEAMAROUND);

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 40, 50, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH_BEAMAROUND, "너비");
			DGShowItem (dialogID, LABEL_WIDTH_BEAMAROUND);

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 140, 40-6, 70, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_WIDTH_BEAMAROUND);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 65, 50, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT_BEAMAROUND, "높이");
			DGShowItem (dialogID, LABEL_HEIGHT_BEAMAROUND);

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 140, 65-6, 70, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_HEIGHT_BEAMAROUND);

			// 라벨: 설명
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 50, 100, 200, 23);
			DGSetItemFont (dialogID, LABEL_DESC2_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DESC2_BEAMAROUND, "유로폼으로 채우시겠습니까?");
			DGShowItem (dialogID, LABEL_DESC2_BEAMAROUND);

			// 체크박스: 규격폼
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 50, 120, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_BEAMAROUND, "규격폼");
			DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_BEAMAROUND, TRUE);
			DGShowItem (dialogID, CHECKBOX_SET_STANDARD_BEAMAROUND);

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 155, 50, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, "너비");
			DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND);

			// 팝업 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 140, 155-6, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM, "200");
			DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND);

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 140, 155-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 180, 50, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, "높이");
			DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND);

			// 팝업 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 140, 180-6, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DG_POPUP_BOTTOM, "600");
			DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND);

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 140, 180-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 210, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_BEAMAROUND, "설치방향");
			DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS_BEAMAROUND);
			
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 780, 140, 205, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_EUROFORM_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_EUROFORM_BEAMAROUND, "벽세우기");
			DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM_BEAMAROUND);
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 780, 140, 230, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_EUROFORM_BEAMAROUND, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_EUROFORM_BEAMAROUND, "벽눕히기");
			DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM_BEAMAROUND);
			DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM_BEAMAROUND, TRUE);

			// 간섭 보(numberOfinterfereBeam) 하부의 합판/목재가 차지하는 공간
			spWidth = placingZone.woods [numberOfinterfereBeam][0].horLen + placingZone.woods [numberOfinterfereBeam][1].horLen + placingZone.woods [numberOfinterfereBeam][2].horLen;
			spHeight = placingZone.woods [numberOfinterfereBeam][1].verLen;
			DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH_BEAMAROUND, spWidth);
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_BEAMAROUND, spHeight);
			DGDisableItem (dialogID, EDITCONTROL_WIDTH_BEAMAROUND);
			DGDisableItem (dialogID, EDITCONTROL_HEIGHT_BEAMAROUND);

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case CHECKBOX_SET_STANDARD_BEAMAROUND:
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_BEAMAROUND) == TRUE) {
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND);
						DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND);
					} else {
						DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND);
						DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND);
					}
					break;
			}
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 지정한 규격, 크기의 유로폼 정보 입력
					spWidth = placingZone.woods [numberOfinterfereBeam][0].horLen + placingZone.woods [numberOfinterfereBeam][1].horLen + placingZone.woods [numberOfinterfereBeam][2].horLen;
					spHeight = placingZone.woods [numberOfinterfereBeam][1].verLen;

					insCell.objType = EUROFORM;
					insCell.leftBottomX = placingZone.woods [numberOfinterfereBeam][0].leftBottomX;
					insCell.leftBottomY = placingZone.woods [numberOfinterfereBeam][0].leftBottomY;
					insCell.leftBottomZ = placingZone.woods [numberOfinterfereBeam][0].leftBottomZ;
					insCell.ang = placingZone.woods [numberOfinterfereBeam][0].ang;
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_BEAMAROUND) == TRUE)
						insCell.libPart.form.eu_stan_onoff = true;
					else
						insCell.libPart.form.eu_stan_onoff = false;
					if (insCell.libPart.form.eu_stan_onoff == true) {
						insCell.libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND)).ToCStr ()) / 1000.0;
						insCell.libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND)).ToCStr ()) / 1000.0;
						insCell.horLen = insCell.libPart.form.eu_wid;
						insCell.verLen = insCell.libPart.form.eu_hei;
					} else {
						insCell.libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND);
						insCell.libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND);
						insCell.horLen = insCell.libPart.form.eu_wid2;
						insCell.verLen = insCell.libPart.form.eu_hei2;
					}
					if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_BEAMAROUND) == TRUE) {
						insCell.libPart.form.u_ins_wall = true;
					} else {
						insCell.libPart.form.u_ins_wall = false;
						temp = insCell.horLen;
						insCell.horLen = insCell.verLen;
						insCell.verLen = temp;
					}

					insCellB.objType = EUROFORM;
					insCellB.leftBottomX = placingZoneBackside.woods [numberOfinterfereBeam][0].leftBottomX + (placingZoneBackside.woods [numberOfinterfereBeam][0].horLen - spWidth) * cos(placingZoneBackside.woods [numberOfinterfereBeam][0].ang);
					insCellB.leftBottomY = placingZoneBackside.woods [numberOfinterfereBeam][0].leftBottomY + (placingZoneBackside.woods [numberOfinterfereBeam][0].horLen - spWidth) * sin(placingZoneBackside.woods [numberOfinterfereBeam][0].ang);
					insCellB.leftBottomZ = placingZoneBackside.woods [numberOfinterfereBeam][0].leftBottomZ;
					insCellB.ang = placingZoneBackside.woods [numberOfinterfereBeam][0].ang;
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_BEAMAROUND) == TRUE)
						insCellB.libPart.form.eu_stan_onoff = true;
					else
						insCellB.libPart.form.eu_stan_onoff = false;
					if (insCellB.libPart.form.eu_stan_onoff == true) {
						insCellB.libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND)).ToCStr ()) / 1000.0;
						insCellB.libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND)).ToCStr ()) / 1000.0;
						insCellB.horLen = insCellB.libPart.form.eu_wid;
						insCellB.verLen = insCellB.libPart.form.eu_hei;
					} else {
						insCellB.libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND);
						insCellB.libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND);
						insCellB.horLen = insCellB.libPart.form.eu_wid2;
						insCellB.verLen = insCellB.libPart.form.eu_hei2;
					}
					if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_BEAMAROUND) == TRUE) {
						insCellB.libPart.form.u_ins_wall = true;
					} else {
						insCellB.libPart.form.u_ins_wall = false;
						temp = insCellB.horLen;
						insCellB.horLen = insCellB.verLen;
						insCellB.verLen = temp;
					}

					// 유로폼 배치 후 남는 보 하부의 여유공간 높이
					remainHeight = placingZone.beams [numberOfinterfereBeam].leftBottomZ - placingZone.woods [numberOfinterfereBeam][0].leftBottomZ - insCell.verLen;

					// 기존 보 주변 합판/목재 제거
					for (xx = 0 ; xx < 3 ; ++xx) {
						elem.header.guid = placingZone.woods [numberOfinterfereBeam][xx].guid;
						if (ACAPI_Element_Get (&elem) != NoError)
							continue;

						API_Elem_Head* headList = new API_Elem_Head [1];
						headList [0] = elem.header;
						err = ACAPI_Element_Delete (&headList, 1);
						delete headList;
					}

					for (xx = 0 ; xx < 3 ; ++xx) {
						elem.header.guid = placingZoneBackside.woods [numberOfinterfereBeam][xx].guid;
						if (ACAPI_Element_Get (&elem) != NoError)
							continue;

						API_Elem_Head* headList = new API_Elem_Head [1];
						headList [0] = elem.header;
						err = ACAPI_Element_Delete (&headList, 1);
						delete headList;
					}
						
					// 사용자가 입력한 추가 유로폼 삽입
					elemList.Push (placeLibPartForWall (insCell));
					elemList.Push (placeLibPartForWall (insCellB));

					// 유로폼 크기에 의해 기존 합판/목재의 크기를 변경하고 위로 올림
					// 보 좌측면
					if (placingZone.woods [numberOfinterfereBeam][0].objType == WOOD) {
						placingZone.woods [numberOfinterfereBeam][0].libPart.wood.w_leng -= insCell.verLen;
						placingZoneBackside.woods [numberOfinterfereBeam][0].libPart.wood.w_leng -= insCellB.verLen;
					} else {
						placingZone.woods [numberOfinterfereBeam][0].libPart.plywood.p_leng -= insCell.verLen;
						placingZoneBackside.woods [numberOfinterfereBeam][0].libPart.plywood.p_leng -= insCellB.verLen;
					}
					placingZone.woods [numberOfinterfereBeam][0].leftBottomZ += insCell.verLen;
					placingZoneBackside.woods [numberOfinterfereBeam][0].leftBottomZ += insCellB.verLen;
					elemList.Push (placeLibPartForWall (placingZone.woods [numberOfinterfereBeam][0]));
					elemList.Push (placeLibPartForWall (placingZoneBackside.woods [numberOfinterfereBeam][0]));

					// 보 아래면
					if (remainHeight > EPS) {
						// 높이가 110mm 미만이면 목재
						if (remainHeight < 0.110) {
							placingZone.woods [numberOfinterfereBeam][1].objType = WOOD;
							placingZone.woods [numberOfinterfereBeam][1].libPart.wood.w_ang = 0;
							placingZone.woods [numberOfinterfereBeam][1].libPart.wood.w_h = remainHeight;
							placingZone.woods [numberOfinterfereBeam][1].libPart.wood.w_leng = placingZone.beams [numberOfinterfereBeam].horLen;
							placingZone.woods [numberOfinterfereBeam][1].libPart.wood.w_w = 0.080;		// 두께: 80mm
							
							placingZoneBackside.woods [numberOfinterfereBeam][1].objType = WOOD;
							placingZoneBackside.woods [numberOfinterfereBeam][1].libPart.wood.w_ang = 0;
							placingZoneBackside.woods [numberOfinterfereBeam][1].libPart.wood.w_h = remainHeight;
							placingZoneBackside.woods [numberOfinterfereBeam][1].libPart.wood.w_leng = placingZone.beams [numberOfinterfereBeam].horLen;
							placingZoneBackside.woods [numberOfinterfereBeam][1].libPart.wood.w_w = 0.080;		// 두께: 80mm
						// 높이가 110mm 이상이면 합판
						} else {
							placingZone.woods [numberOfinterfereBeam][1].objType = PLYWOOD;
							placingZone.woods [numberOfinterfereBeam][1].libPart.plywood.p_leng = remainHeight;
							placingZone.woods [numberOfinterfereBeam][1].libPart.plywood.p_wid = placingZone.beams [numberOfinterfereBeam].horLen;
							placingZone.woods [numberOfinterfereBeam][1].libPart.plywood.w_dir_wall = true;

							placingZoneBackside.woods [numberOfinterfereBeam][1].objType = PLYWOOD;
							placingZoneBackside.woods [numberOfinterfereBeam][1].libPart.plywood.p_leng = remainHeight;
							placingZoneBackside.woods [numberOfinterfereBeam][1].libPart.plywood.p_wid = placingZone.beams [numberOfinterfereBeam].horLen;
							placingZoneBackside.woods [numberOfinterfereBeam][1].libPart.plywood.w_dir_wall = true;
						}
						placingZone.woods [numberOfinterfereBeam][1].leftBottomZ += insCell.verLen;
						placingZoneBackside.woods [numberOfinterfereBeam][1].leftBottomZ += insCell.verLen;

						elemList.Push (placeLibPartForWall (placingZone.woods [numberOfinterfereBeam][1]));
						elemList.Push (placeLibPartForWall (placingZoneBackside.woods [numberOfinterfereBeam][1]));
					}

					// 보 우측면
					if (placingZone.woods [numberOfinterfereBeam][2].objType == WOOD) {
						placingZone.woods [numberOfinterfereBeam][2].libPart.wood.w_leng -= insCell.verLen;
						placingZoneBackside.woods [numberOfinterfereBeam][2].libPart.wood.w_leng -= insCellB.verLen;
					} else {
						placingZone.woods [numberOfinterfereBeam][2].libPart.plywood.p_leng -= insCell.verLen;
						placingZoneBackside.woods [numberOfinterfereBeam][2].libPart.plywood.p_leng -= insCellB.verLen;
					}
					placingZone.woods [numberOfinterfereBeam][2].leftBottomZ += insCell.verLen;
					placingZoneBackside.woods [numberOfinterfereBeam][2].leftBottomZ += insCellB.verLen;
					elemList.Push (placeLibPartForWall (placingZone.woods [numberOfinterfereBeam][2]));
					elemList.Push (placeLibPartForWall (placingZoneBackside.woods [numberOfinterfereBeam][2]));

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

// 벽 상단의 합판/목재 영역을 유로폼으로 채울지 물어보는 5차 다이얼로그
short DGCALLBACK wallPlacerHandler5 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	API_Element		elem [6];
	API_Elem_Head*	headList;

	short	result;
	short	idxItem;
	short	xx, tt;
	short	processedIndex = 0;

	double	initPlywoodHeight = 0.0;
	double	changedPlywoodHeight = 0.0;

	// 다이얼로그에서 선택한 정보를 저장
	bool	bEuroform1, bEuroform2;
	bool	bEufoformStandard1, bEufoformStandard2;
	double	euroformWidth1 = 0.0, euroformWidth2 = 0.0;

	// 연속된 셀 크기가 유로폼 높이와 일치하는지 확인함
	short	ind1, ind2, ind3;
	bool	bValid3Window, bValid2Window, bValid1Window;
	double	width3Window, width2Window, width1Window;
	bool	bFindWidth;
	double	totalWidth;
	CellForWall		insCell, insCellB;
	double	backsideDistance = 0.0;


	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "벽 상단 채우기");

			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 70, 240, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "예");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 160, 240, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "아니오");
			DGShowItem (dialogID, DG_CANCEL);

			// 라벨: 설명
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 10, 260, 23);
			DGSetItemFont (dialogID, LABEL_DESC1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DESC1_TOPREST, "벽 상부에 다음 높이 만큼의 공간이 있습니다.");
			DGShowItem (dialogID, LABEL_DESC1_TOPREST);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 40, 50, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT_TOPREST, "높이");
			DGShowItem (dialogID, LABEL_HEIGHT_TOPREST);

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 140, 40-6, 70, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_HEIGHT_TOPREST);
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (placingZone.topRestCells [xx].objType == PLYWOOD) {
					initPlywoodHeight = placingZone.topRestCells [xx].verLen;
					break;
				}
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT_TOPREST, initPlywoodHeight);
			DGDisableItem (dialogID, EDITCONTROL_HEIGHT_TOPREST);

			// 라벨: 설명
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 50, 80, 200, 23);
			DGSetItemFont (dialogID, LABEL_DESC2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DESC2_TOPREST, "유로폼으로 채우시겠습니까?");
			DGShowItem (dialogID, LABEL_DESC2_TOPREST);

			// 라벨: '위'
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 20, 120, 30, 23);
			DGSetItemFont (dialogID, LABEL_UP_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_UP_TOPREST, "위");
			DGShowItem (dialogID, LABEL_UP_TOPREST);

			// 라벨: '↑'
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 20, 150, 30, 23);
			DGSetItemFont (dialogID, LABEL_ARROWUP_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ARROWUP_TOPREST, "↑");
			DGShowItem (dialogID, LABEL_ARROWUP_TOPREST);

			// 라벨: '아래'
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 20, 180, 30, 23);
			DGSetItemFont (dialogID, LABEL_DOWN_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DOWN_TOPREST, "아래");
			DGShowItem (dialogID, LABEL_DOWN_TOPREST);

			// 체크박스: 폼 On/Off (1단 - 맨 아래)
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 70, 180-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST, "유로폼");
			DGShowItem (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST);

			// 체크박스: 폼 On/Off (2단 - 중간)
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 70, 150-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST, "유로폼");
			DGShowItem (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST);

			// 라벨: 합판
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 70, 120, 70, 23);
			DGSetItemFont (dialogID, LABEL_PLYWOOD_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
			DGShowItem (dialogID, LABEL_PLYWOOD_TOPREST);

			// 체크박스: 규격폼 (1단 - 맨 아래)
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 150, 180-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, "규격폼");
			DGShowItem (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST);
			DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST, TRUE);

			// 체크박스: 규격폼 (2단 - 중간)
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 150, 150-6, 70, 25);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, "규격폼");
			DGShowItem (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST);
			DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST, TRUE);

			// 팝업 컨트롤: 유로폼 (1단) 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 220, 180-6, 70, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_POPUP_BOTTOM, "200");
			DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);

			// 팝업 컨트롤: 유로폼 (2단) 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 220, 150-6, 70, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_POPUP_BOTTOM, "200");
			DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);

			// Edit 컨트롤: 유로폼 (1단) 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 180-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);

			// Edit 컨트롤: 유로폼 (2단) 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 150-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);

			// Edit 컨트롤: 합판 또는 목재 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 220, 120-6, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_PLYWOOD_TOPREST, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_PLYWOOD_TOPREST);
			DGDisableItem (dialogID, EDITCONTROL_PLYWOOD_TOPREST);

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case CHECKBOX_SET_STANDARD_1_TOPREST:
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST) == TRUE) {
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
					} else {
						DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
					}
					break;
				
				case CHECKBOX_SET_STANDARD_2_TOPREST:
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST) == TRUE) {
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
					} else {
						DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
					}
					break;
			}

			// 폼의 너비에 따라 합판/목재 영역의 높이가 달라짐
			if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST) == TRUE)
				bEuroform1 = true;
			else
				bEuroform1 = false;
			if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST) == TRUE)
				bEuroform2 = true;
			else
				bEuroform2 = false;
			if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST) == TRUE)
				bEufoformStandard1 = true;
			else
				bEufoformStandard1 = false;
			if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST) == TRUE)
				bEufoformStandard2 = true;
			else
				bEufoformStandard2 = false;

			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				if (placingZone.topRestCells [xx].objType == PLYWOOD) {
					initPlywoodHeight = placingZone.topRestCells [xx].verLen;
					break;
				}
			}
			
			changedPlywoodHeight = initPlywoodHeight;
			euroformWidth1 = 0.0;
			euroformWidth2 = 0.0;

			if (bEuroform1) {
				if (bEufoformStandard1)
					euroformWidth1 = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST)).ToCStr ()) / 1000.0;
				else
					euroformWidth1 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
			}

			if (bEuroform2) {
				if (bEufoformStandard2)
					euroformWidth2 = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST)).ToCStr ()) / 1000.0;
				else
					euroformWidth2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
			}

			changedPlywoodHeight -= (euroformWidth1 + euroformWidth2);

			if (changedPlywoodHeight < EPS) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "없음");
			} else if (changedPlywoodHeight < 0.110) {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "목재");
			} else {
				DGSetItemText (dialogID, LABEL_PLYWOOD_TOPREST, "합판");
			}
			DGSetItemValDouble (dialogID, EDITCONTROL_PLYWOOD_TOPREST, changedPlywoodHeight);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 각 유로폼 및 합판 영역의 높이를 계산함
					if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_1_TOPREST) == TRUE)
						bEuroform1 = true;
					else
						bEuroform1 = false;
					if (DGGetItemValLong (dialogID, CHECKBOX_FORM_ONOFF_2_TOPREST) == TRUE)
						bEuroform2 = true;
					else
						bEuroform2 = false;
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_1_TOPREST) == TRUE)
						bEufoformStandard1 = true;
					else
						bEufoformStandard1 = false;
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_2_TOPREST) == TRUE)
						bEufoformStandard2 = true;
					else
						bEufoformStandard2 = false;

					for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
						if (placingZone.topRestCells [xx].objType == PLYWOOD) {
							initPlywoodHeight = placingZone.topRestCells [xx].verLen;
							break;
						}
					}
			
					changedPlywoodHeight = initPlywoodHeight;
					euroformWidth1 = 0.0;
					euroformWidth2 = 0.0;

					if (bEuroform1) {
						if (bEufoformStandard1)
							euroformWidth1 = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST)).ToCStr ()) / 1000.0;
						else
							euroformWidth1 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST);
					}

					if (bEuroform2) {
						if (bEufoformStandard2)
							euroformWidth2 = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST)).ToCStr ()) / 1000.0;
						else
							euroformWidth2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST);
					}

					changedPlywoodHeight -= (euroformWidth1 + euroformWidth2);

					if (bEuroform1 || bEuroform2) {
						xx = 0;
						while (xx < placingZone.nCells) {
							// 중간에 NONE 타입은 건너뛰고 비-NONE 타입만 인덱스 값으로 저장해야 함
							ind1 = -1;
							ind2 = -1;
							ind3 = -1;

							// ind1을 먼저 찾고
							if (placingZone.topRestCells [xx].objType != NONE) {
								ind1 = xx;
							}

							// ind2, ind3를 찾음
							for (tt = xx+1 ; tt < placingZone.nCells ; ++tt) {
								if (placingZone.topRestCells [tt].objType != NONE) {
									// ind1 찾은 후 ind2를 찾고
									if ((ind1 != -1) && (ind2 == -1)) {
										ind2 = tt;
										continue;
									}
									// ind2 찾은 후 ind3까지 찾음
									if ((ind2 != -1) && (ind3 == -1)) {
										ind3 = tt;
										continue;
									}
								}
							}

							// 너무 동떨어져 있는 셀은 이웃한 것이 아니므로 못 찾은 걸로 간주함
							if ((ind3 - ind2) > 2)
								ind3 = -1;
							if ((ind2 - ind1) > 2)
								ind2 = -1;
							if (ind2 == -1)
								ind3 = -1;

							// 합판 연속 3개/2개/1개의 너비를 구함
							bValid3Window = false;
							bValid2Window = false;
							bValid1Window = false;
							width1Window = 0.0;
							width2Window = 0.0;
							width3Window = 0.0;
							if ( (ind1 != -1) && (ind2 != -1) && (ind3 != -1) ) {
								if ( (placingZone.topRestCells [ind1].objType == PLYWOOD) && (placingZone.topRestCells [ind2].objType == PLYWOOD) && (placingZone.topRestCells [ind3].objType == PLYWOOD) ) {
									bValid3Window = true;
									width3Window = placingZone.topRestCells [ind1].horLen + placingZone.topRestCells [ind2].horLen + placingZone.topRestCells [ind3].horLen;
								}
							}

							if ( (ind1 != -1) && (ind2 != -1) ) {
								if ( (placingZone.topRestCells [ind1].objType == PLYWOOD) && (placingZone.topRestCells [ind2].objType == PLYWOOD) ) {
									bValid2Window = true;
									width2Window = placingZone.topRestCells [ind1].horLen + placingZone.topRestCells [ind2].horLen;
								}
							}

							if ( (ind1 != -1) ) {
								if ( (placingZone.topRestCells [ind1].objType == PLYWOOD) ) {
									bValid1Window = true;
									width1Window = placingZone.topRestCells [ind1].horLen;
								}
							}

							// 합판 너비의 합이 1200/900/600 중 하나인가?
							bFindWidth = false;
							processedIndex = 0;
							totalWidth = 0.0;

							if (bValid3Window == true) {
								if ( (abs (width3Window - 1.200) < EPS) || (abs (width3Window - 0.900) < EPS) || (abs (width3Window - 0.600) < EPS) ) {
									bFindWidth = true;
									totalWidth = width3Window;
									processedIndex = ind3 - ind1 + 1;
								}
							}

							if ((bFindWidth == false) && (bValid2Window == true)) {
								if ( (abs (width2Window - 1.200) < EPS) || (abs (width2Window - 0.900) < EPS) || (abs (width2Window - 0.600) < EPS) ) {
									bFindWidth = true;
									totalWidth = width2Window;
									processedIndex = ind2 - ind1 + 1;
								}
							}

							if ((bFindWidth == false) && (bValid1Window == true)) {
								if ( (abs (width1Window - 1.200) < EPS) || (abs (width1Window - 0.900) < EPS) || (abs (width1Window - 0.600) < EPS) ) {
									bFindWidth = true;
									totalWidth = width1Window;
									processedIndex = 1;
								}
							}

							// 기존 합판을 제거함
							if (bFindWidth) {
								if ( processedIndex == (ind3 - ind1 + 1) ) {
									elem [0].header.guid = placingZone.topRestCells [ind1].guid;
									elem [1].header.guid = placingZone.topRestCells [ind2].guid;
									elem [2].header.guid = placingZone.topRestCells [ind3].guid;
									elem [3].header.guid = placingZoneBackside.topRestCells [ind1].guid;
									elem [4].header.guid = placingZoneBackside.topRestCells [ind2].guid;
									elem [5].header.guid = placingZoneBackside.topRestCells [ind3].guid;

									headList = new API_Elem_Head [6];
									for (tt = 0 ; tt < 6 ; ++tt)
										headList [tt] = elem [tt].header;
									ACAPI_Element_Delete (&headList, 6);
									delete headList;

									backsideDistance = totalWidth / 3;
								}
								if ( processedIndex == (ind2 - ind1 + 1) ) {
									elem [0].header.guid = placingZone.topRestCells [ind1].guid;
									elem [1].header.guid = placingZone.topRestCells [ind2].guid;
									elem [2].header.guid = placingZoneBackside.topRestCells [ind1].guid;
									elem [3].header.guid = placingZoneBackside.topRestCells [ind2].guid;

									headList = new API_Elem_Head [4];
									for (tt = 0 ; tt < 4 ; ++tt)
										headList [tt] = elem [tt].header;
									ACAPI_Element_Delete (&headList, 4);
									delete headList;

									backsideDistance = totalWidth / 2;
								}
								if ( processedIndex == 1 ) {
									elem [0].header.guid = placingZone.topRestCells [ind1].guid;
									elem [1].header.guid = placingZoneBackside.topRestCells [ind1].guid;

									headList = new API_Elem_Head [2];
									for (tt = 0 ; tt < 2 ; ++tt)
										headList [tt] = elem [tt].header;
									ACAPI_Element_Delete (&headList, 2);
									delete headList;

									backsideDistance = 0.0;
								}
							}

							// 사용자가 입력한 대로 유로폼(벽눕히기) 및 합판 또는 목재를 배치함
							if (bFindWidth) {
								// 유로폼 (1단)
								if (bEuroform1) {
									insCell.objType = EUROFORM;
									insCell.leftBottomX = placingZone.topRestCells [xx].leftBottomX;
									insCell.leftBottomY = placingZone.topRestCells [xx].leftBottomY;
									insCell.leftBottomZ = placingZone.topRestCells [xx].leftBottomZ;
									insCell.ang = placingZone.topRestCells [xx].ang;
									insCell.libPart.form.u_ins_wall = false;

									if (bEufoformStandard1) {
										insCell.libPart.form.eu_stan_onoff = true;
										insCell.libPart.form.eu_wid = euroformWidth1;
										insCell.libPart.form.eu_hei = totalWidth;
									} else {
										insCell.libPart.form.eu_stan_onoff = false;
										insCell.libPart.form.eu_wid2 = euroformWidth1;
										insCell.libPart.form.eu_hei2 = totalWidth;
									}
									insCell.horLen = totalWidth;
									insCell.verLen = euroformWidth1;

									insCellB.objType = EUROFORM;
									insCellB.leftBottomX = placingZoneBackside.topRestCells [xx].leftBottomX - (backsideDistance * cos(placingZoneBackside.topRestCells [xx].ang));
									insCellB.leftBottomY = placingZoneBackside.topRestCells [xx].leftBottomY - (backsideDistance * sin(placingZoneBackside.topRestCells [xx].ang));
									insCellB.leftBottomZ = placingZoneBackside.topRestCells [xx].leftBottomZ;
									insCellB.ang = placingZoneBackside.topRestCells [xx].ang;
									insCellB.libPart.form.u_ins_wall = false;

									if (bEufoformStandard1) {
										insCellB.libPart.form.eu_stan_onoff = true;
										insCellB.libPart.form.eu_wid = euroformWidth1;
										insCellB.libPart.form.eu_hei = totalWidth;
									} else {
										insCellB.libPart.form.eu_stan_onoff = false;
										insCellB.libPart.form.eu_wid2 = euroformWidth1;
										insCellB.libPart.form.eu_hei2 = totalWidth;
									}
									insCellB.horLen = totalWidth;
									insCellB.verLen = euroformWidth1;

									elemList.Push (placeLibPartForWall (insCell));
									elemList.Push (placeLibPartForWall (insCellB));
								}
								
								// 유로폼 (2단)
								if (bEuroform2) {
									insCell.objType = EUROFORM;
									insCell.leftBottomX = placingZone.topRestCells [xx].leftBottomX;
									insCell.leftBottomY = placingZone.topRestCells [xx].leftBottomY;
									insCell.leftBottomZ = placingZone.topRestCells [xx].leftBottomZ + (bEuroform1 * euroformWidth1);
									insCell.ang = placingZone.topRestCells [xx].ang;
									insCell.libPart.form.u_ins_wall = false;

									if (bEufoformStandard1) {
										insCell.libPart.form.eu_stan_onoff = true;
										insCell.libPart.form.eu_wid = euroformWidth2;
										insCell.libPart.form.eu_hei = totalWidth;
									} else {
										insCell.libPart.form.eu_stan_onoff = false;
										insCell.libPart.form.eu_wid2 = euroformWidth2;
										insCell.libPart.form.eu_hei2 = totalWidth;
									}
									insCell.horLen = totalWidth;
									insCell.verLen = euroformWidth2;

									insCellB.objType = EUROFORM;
									insCellB.leftBottomX = placingZoneBackside.topRestCells [xx].leftBottomX - (backsideDistance * cos(placingZoneBackside.topRestCells [xx].ang));
									insCellB.leftBottomY = placingZoneBackside.topRestCells [xx].leftBottomY - (backsideDistance * sin(placingZoneBackside.topRestCells [xx].ang));
									insCellB.leftBottomZ = placingZoneBackside.topRestCells [xx].leftBottomZ + (bEuroform1 * euroformWidth1);
									insCellB.ang = placingZoneBackside.topRestCells [xx].ang;
									insCellB.libPart.form.u_ins_wall = false;

									if (bEufoformStandard2) {
										insCellB.libPart.form.eu_stan_onoff = true;
										insCellB.libPart.form.eu_wid = euroformWidth2;
										insCellB.libPart.form.eu_hei = totalWidth;
									} else {
										insCellB.libPart.form.eu_stan_onoff = false;
										insCellB.libPart.form.eu_wid2 = euroformWidth2;
										insCellB.libPart.form.eu_hei2 = totalWidth;
									}
									insCellB.horLen = totalWidth;
									insCellB.verLen = euroformWidth2;

									elemList.Push (placeLibPartForWall (insCell));
									elemList.Push (placeLibPartForWall (insCellB));
								}

								// 합판 또는 목재 (공간이 없으면 폼 위에 높이 50mm의 목재를 붙임)
								if (changedPlywoodHeight < EPS) {
									insCell.objType = WOOD;
									insCell.leftBottomX = placingZone.topRestCells [xx].leftBottomX + (0.064 * sin(placingZone.topRestCells [xx].ang));
									insCell.leftBottomY = placingZone.topRestCells [xx].leftBottomY - (0.064 * cos(placingZone.topRestCells [xx].ang));
									insCell.leftBottomZ = placingZone.topRestCells [xx].leftBottomZ + (bEuroform1 * euroformWidth1) + (bEuroform2 * euroformWidth2) - 0.050;
									insCell.ang = placingZone.topRestCells [xx].ang;
									insCell.libPart.wood.w_ang = 0.0;
									insCell.libPart.wood.w_w = 0.080;	// 두께: 80mm
									insCell.libPart.wood.w_h = 0.050;	// 높이: 50mm
									insCell.libPart.wood.w_leng = totalWidth;
									insCell.horLen = totalWidth;
									insCell.verLen = 0.050;

									insCellB.objType = WOOD;
									insCellB.leftBottomX = placingZoneBackside.topRestCells [xx].leftBottomX - (backsideDistance * cos(placingZoneBackside.topRestCells [xx].ang)) - (0.064 * sin(placingZone.topRestCells [xx].ang));
									insCellB.leftBottomY = placingZoneBackside.topRestCells [xx].leftBottomY - (backsideDistance * sin(placingZoneBackside.topRestCells [xx].ang)) + (0.064 * cos(placingZone.topRestCells [xx].ang));
									insCellB.leftBottomZ = placingZoneBackside.topRestCells [xx].leftBottomZ + (bEuroform1 * euroformWidth1) + (bEuroform2 * euroformWidth2) - 0.050;
									insCellB.ang = placingZoneBackside.topRestCells [xx].ang;
									insCellB.libPart.wood.w_ang = 0.0;
									insCellB.libPart.wood.w_w = 0.080;	// 두께: 80mm
									insCellB.libPart.wood.w_h = 0.050;	// 높이: 50mm
									insCellB.libPart.wood.w_leng = totalWidth;
									insCellB.horLen = totalWidth;
									insCellB.verLen = 0.050;

									elemList.Push (placeLibPartForWall (insCell));
									elemList.Push (placeLibPartForWall (insCellB));

								} else if (changedPlywoodHeight < 0.110) {
									insCell.objType = WOOD;
									insCell.leftBottomX = placingZone.topRestCells [xx].leftBottomX;
									insCell.leftBottomY = placingZone.topRestCells [xx].leftBottomY;
									insCell.leftBottomZ = placingZone.topRestCells [xx].leftBottomZ + (bEuroform1 * euroformWidth1) + (bEuroform2 * euroformWidth2);
									insCell.ang = placingZone.topRestCells [xx].ang;
									insCell.libPart.wood.w_ang = 0.0;
									insCell.libPart.wood.w_w = 0.080;	// 두께: 80mm
									insCell.libPart.wood.w_h = changedPlywoodHeight;
									insCell.libPart.wood.w_leng = totalWidth;
									insCell.horLen = totalWidth;
									insCell.verLen = changedPlywoodHeight;

									insCellB.objType = WOOD;
									insCellB.leftBottomX = placingZoneBackside.topRestCells [xx].leftBottomX - (backsideDistance * cos(placingZoneBackside.topRestCells [xx].ang));
									insCellB.leftBottomY = placingZoneBackside.topRestCells [xx].leftBottomY - (backsideDistance * sin(placingZoneBackside.topRestCells [xx].ang));
									insCellB.leftBottomZ = placingZoneBackside.topRestCells [xx].leftBottomZ + (bEuroform1 * euroformWidth1) + (bEuroform2 * euroformWidth2);
									insCellB.ang = placingZoneBackside.topRestCells [xx].ang;
									insCellB.libPart.wood.w_ang = 0.0;
									insCellB.libPart.wood.w_w = 0.080;	// 두께: 80mm
									insCellB.libPart.wood.w_h = changedPlywoodHeight;
									insCellB.libPart.wood.w_leng = totalWidth;
									insCellB.horLen = totalWidth;
									insCellB.verLen = changedPlywoodHeight;

									elemList.Push (placeLibPartForWall (insCell));
									elemList.Push (placeLibPartForWall (insCellB));
								} else {
									insCell.objType = PLYWOOD;
									insCell.leftBottomX = placingZone.topRestCells [xx].leftBottomX;
									insCell.leftBottomY = placingZone.topRestCells [xx].leftBottomY;
									insCell.leftBottomZ = placingZone.topRestCells [xx].leftBottomZ + (bEuroform1 * euroformWidth1) + (bEuroform2 * euroformWidth2);
									insCell.ang = placingZone.topRestCells [xx].ang;
									insCell.libPart.plywood.w_dir_wall = false;
									insCell.libPart.plywood.p_leng = totalWidth;
									insCell.libPart.plywood.p_wid = changedPlywoodHeight;
									insCell.horLen = totalWidth;
									insCell.verLen = changedPlywoodHeight;

									insCellB.objType = PLYWOOD;
									insCellB.leftBottomX = placingZoneBackside.topRestCells [xx].leftBottomX - (backsideDistance * cos(placingZoneBackside.topRestCells [xx].ang));
									insCellB.leftBottomY = placingZoneBackside.topRestCells [xx].leftBottomY - (backsideDistance * sin(placingZoneBackside.topRestCells [xx].ang));
									insCellB.leftBottomZ = placingZoneBackside.topRestCells [xx].leftBottomZ + (bEuroform1 * euroformWidth1) + (bEuroform2 * euroformWidth2);
									insCellB.ang = placingZoneBackside.topRestCells [xx].ang;
									insCellB.libPart.plywood.w_dir_wall = false;
									insCellB.libPart.plywood.p_leng = totalWidth;
									insCellB.libPart.plywood.p_wid = changedPlywoodHeight;
									insCellB.horLen = totalWidth;
									insCellB.verLen = changedPlywoodHeight;

									elemList.Push (placeLibPartForWall (insCell));
									elemList.Push (placeLibPartForWall (insCellB));
								}

								xx += processedIndex;
							} else
								++xx;	// 다음 셀로 넘어감
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