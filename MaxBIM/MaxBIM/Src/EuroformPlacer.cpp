#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"

static PlacingZone		placingZone;			// 기본 벽면 영역 정보
static PlacingZone		placingZoneBackside;	// 반대쪽 벽면에도 벽면 영역 정보 부여, 벽 기준으로 대칭됨 (placingZone과 달리 오른쪽부터 객체를 설치함)
static InfoWall			infoWall;				// 벽 객체 정보
static short			clickedBtnItemIdx;		// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
static short			layerInd;				// 객체를 배치할 레이어 인덱스
static short			itemInitIdx = GRIDBUTTON_IDX_START;		// 그리드 버튼 항목 인덱스 시작번호


// 1번 메뉴: 유로폼/인코너 등을 배치하는 통합 루틴
GSErrCode	placeEuroformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx, yy;
	long		result_compare_x = 0, result_compare_y = 0, result_compare_z = 0;
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
	InfoMorph				infoMorph;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			minusLevel;


	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);	// 선택한 요소 가져오기
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 벽 (1개), 벽을 덮는 모프 (1개)\n옵션 선택: 모프 위쪽과 맞닿는 보 (다수)", true);
		return err;
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
	infoWall.wallThk	= elem.wall.thickness;
	infoWall.floorInd	= elem.header.floorInd;
	infoWall.begX		= elem.wall.begC.x;
	infoWall.begY		= elem.wall.begC.y;
	infoWall.endX		= elem.wall.endC.x;
	infoWall.endY		= elem.wall.endC.y;

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

	// 모프의 가로 길이
	infoMorph.horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);

	// 모프의 세로 길이
	infoMorph.verLen = info3D.bounds.zMax - info3D.bounds.zMin;

	// 모프의 좌하단, 우상단 점 지정
	if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
		// 좌상단 좌표 결정
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

	// 벽면 모프를 통해 영역 정보 업데이트
	placingZone.leftBottomX		= infoMorph.leftBottomX;
	placingZone.leftBottomY		= infoMorph.leftBottomY;
	placingZone.leftBottomZ		= infoMorph.leftBottomZ;
	placingZone.horLen			= infoMorph.horLen;
	placingZone.verLen			= infoMorph.verLen;
	placingZone.ang				= degreeToRad (infoMorph.ang);
	placingZone.nInterfereBeams	= (short)nBeams;
	
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

		if (abs (infoMorph.ang - ang1) < EPS) {
			// 보의 LeftBottom 좌표
			xPosLB = elem.beam.begC.x - elem.beam.width/2 * cos(degreeToRad (infoMorph.ang)),
			yPosLB = elem.beam.begC.y - elem.beam.width/2 * sin(degreeToRad (infoMorph.ang)),
			zPosLB = elem.beam.level - elem.beam.height;

			// 보의 RightTop 좌표
			xPosRT = elem.beam.begC.x + elem.beam.width/2 * cos(degreeToRad (infoMorph.ang)),
			yPosRT = elem.beam.begC.y + elem.beam.width/2 * sin(degreeToRad (infoMorph.ang)),
			zPosRT = elem.beam.level;
		} else {
			// 보의 LeftBottom 좌표
			xPosLB = elem.beam.endC.x - elem.beam.width/2 * cos(degreeToRad (infoMorph.ang)),
			yPosLB = elem.beam.endC.y - elem.beam.width/2 * sin(degreeToRad (infoMorph.ang)),
			zPosLB = elem.beam.level - elem.beam.height;

			// 보의 RightTop 좌표
			xPosRT = elem.beam.endC.x + elem.beam.width/2 * cos(degreeToRad (infoMorph.ang)),
			yPosRT = elem.beam.endC.y + elem.beam.width/2 * sin(degreeToRad (infoMorph.ang)),
			zPosRT = elem.beam.level;
		}

		for (yy = 0 ; yy < 2 ; ++yy) {
			// X축 범위 비교
			result_compare_x = compareRanges (infoMorph.leftBottomX, infoMorph.leftBottomX + infoMorph.horLen * cos(degreeToRad (infoMorph.ang)), xPosLB, xPosRT);

			// Y축 범위 비교
			result_compare_y = compareRanges (infoMorph.leftBottomY, infoMorph.leftBottomY + infoMorph.horLen * sin(degreeToRad (infoMorph.ang)), yPosLB, yPosRT);

			// Z축 범위 비교
			result_compare_z = compareRanges (infoMorph.leftBottomZ, infoMorph.leftBottomZ + infoMorph.verLen, zPosLB, zPosRT);

			// 벽 내부 상단에 붙어 있으면 처리해야 할 보라고 인식할 것
			if ( (result_compare_x == 5) && (result_compare_y == 5) && (result_compare_z == 6) ) {
				placingZone.beams [xx].leftBottomX	= xPosLB;
				placingZone.beams [xx].leftBottomY	= yPosLB;
				placingZone.beams [xx].leftBottomZ	= zPosLB;
				placingZone.beams [xx].horLen		= elem.beam.width;
				placingZone.beams [xx].verLen		= elem.beam.height;
			}
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	minusLevel = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoWall.floorInd) {
			minusLevel = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 모든 영역 정보의 Z 정보를 수정
	placingZone.leftBottomZ -= minusLevel;
	for (xx = 0 ; xx < placingZone.nInterfereBeams ; ++xx)	placingZone.beams [xx].leftBottomZ -= minusLevel;

	//////////////////////////////////////////////////////////// 1차 유로폼/인코너 배치
	// [DIALOG] 1번째 다이얼로그에서 인코너, 유로폼 정보 입력 받음
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32500, ACAPI_GetOwnResModule (), placerHandlerPrimary, 0);

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

	// 가로 나머지 길이 값 분리 (고정값, 변동값)
	placingZone.remain_hor_updated = placingZone.remain_hor;

	// 선택된 모프 전부 제거
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);	// 선택한 요소 가져오기
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	err = ACAPI_CallUndoableCommand ("영역 모프 제거", [&] () -> GSErrCode {
		if (selectionInfo.typeID != API_SelEmpty) {
			nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
			for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
				tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

				if (tElem.header.typeID == API_MorphID) {	// 모프 제거
					API_Elem_Head* headList = new API_Elem_Head [1];
					headList [0] = tElem.header;
					err = ACAPI_Element_Delete (&headList, 1);
					delete headList;
				}
			}
		}
		BMKillHandle ((GSHandle *) &selNeigs);

		return err;
	});

	// placingZone의 Cell 정보 초기화
	placingZone.nCells = (placingZone.eu_count_hor * 2) + 3;
	initCells (&placingZone);
	
	// 반대쪽 벽에 대한 벽면 영역 정보 초기화
	initCells (&placingZoneBackside);
	placingZoneBackside.nCells = placingZone.nCells;

	// 배치를 위한 정보 입력
	firstPlacingSettings (&placingZone);
	copyPlacingZoneSymmetric (&placingZone, &placingZoneBackside, &infoWall);

	// 1번째 다이얼로그에서 입력한 대로 유로폼/인코너 배치
	err = ACAPI_CallUndoableCommand ("유로폼/인코너 초기 배치", [&] () -> GSErrCode {

		//////////////////////////////////////////////////////////// 벽 앞쪽
		for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
			for (yy = 0 ; yy < placingZone.nCells ; ++yy)
				// cells 객체의 objType이 NONE이 아니면 연속으로 배치할 것
				placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

			// Cell 정보 업데이트: 다음 높이로 상승시킴
			if (placingZone.eu_ori.compare (std::string ("벽세우기")) == 0)
				setCellPositionLeftBottomZ (&placingZone, placingZone.leftBottomZ + (placingZone.eu_hei_numeric * (xx + 1)));
			else
				setCellPositionLeftBottomZ (&placingZone, placingZone.leftBottomZ + (placingZone.eu_wid_numeric * (xx + 1)));
		}
		setCellPositionLeftBottomZ (&placingZone, placingZone.leftBottomZ);		// Cell 정보 업데이트: 높이 초기화

		//////////////////////////////////////////////////////////// 벽 뒤쪽
		for (xx = 0 ; xx < placingZoneBackside.eu_count_ver ; ++xx) {
			for (yy = 0 ; yy < placingZoneBackside.nCells ; ++yy)
				// cells 객체의 objType이 NONE이 아니면 연속으로 배치할 것
				placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);

			// Cell 정보 업데이트: 다음 높이로 상승시킴
			if (placingZoneBackside.eu_ori.compare (std::string ("벽세우기")) == 0)
				setCellPositionLeftBottomZ (&placingZoneBackside, placingZoneBackside.leftBottomZ + (placingZoneBackside.eu_hei_numeric * (xx + 1)));
			else
				setCellPositionLeftBottomZ (&placingZoneBackside, placingZoneBackside.leftBottomZ + (placingZoneBackside.eu_wid_numeric * (xx + 1)));
		}
		setCellPositionLeftBottomZ (&placingZoneBackside, placingZoneBackside.leftBottomZ);		// Cell 정보 업데이트: 높이 초기화

		return err;
	});

	// [DIALOG] 2번째 다이얼로그에서 유로폼/인코너 배치를 수정하거나 휠러스페이서를 삽입합니다.
	result = DGBlankModalDialog (185, 250, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, placerHandlerSecondary, 0);

	// 자투리 공간 채우기
	fillRestAreas ();

	return	err;
}

// 가로 채우기까지 완료된 후 자투리 공간 채우기
GSErrCode	fillRestAreas (void)
{
	GSErrCode	err = NoError;
	short	xx, yy, zz;
	short	indInterfereBeam;		// 중첩되는 보의 인덱스 (-1은 중첩 없음)
	double	cellLeftX, cellRightX;	// 셀의 L/R측 X 좌표
	double	cellRightX2;			// 셀 우측 R측 X 좌표
	double	beamLeftX, beamRightX;	// 보의 L/R측 X 좌표
	double	currentHeight;			// 현재 라인 높이
	double	dist;

	short	kk, moveCountLimit;
	double	newXPosOffset, newXSizeOffset;		// 합판의 새로운 위치, 크기 차이

	err = ACAPI_CallUndoableCommand ("자투리 공간 채우기", [&] () -> GSErrCode {

		// 유로폼 설치 방향: 벽세우기
		if (placingZone.eu_ori.compare (std::string ("벽세우기")) == 0) {

			xx = placingZone.eu_count_ver;

			// Cell 정보 업데이트: 다음 높이로 상승시킴
			setCellPositionLeftBottomZ (&placingZone, placingZone.leftBottomZ + (placingZone.eu_hei_numeric * xx));
			setCellPositionLeftBottomZ (&placingZoneBackside, placingZoneBackside.leftBottomZ + (placingZoneBackside.eu_hei_numeric * xx));

			// 기본 채우기 라인 상단
			for (yy = 0 ; yy < placingZone.nCells ; ++yy) {

				// 보의 중첩 관계 확인 - 중첩되는 보의 인덱스를 먼저 추출
				indInterfereBeam = -1;
				for (zz = 0 ; zz < placingZone.nInterfereBeams ; ++zz) {
				
					// 간섭을 찾을 때까지만 확인
					if (indInterfereBeam == -1) {
						dist = GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.cells [0][yy].leftBottomX, placingZone.cells [0][yy].leftBottomY);
						cellLeftX	= dist;
						cellRightX	= dist + placingZone.cells [0][yy].horLen;
						dist = GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.beams [zz].leftBottomX, placingZone.beams [zz].leftBottomY);
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

				// 인코너는 무조건 벽 위까지 붙임
				if (placingZone.cells [0][yy].objType == INCORNER) {
					placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
					placingZone.cells [0][yy].libPart.incorner.hei_s = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;;
					placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

					placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
					placingZoneBackside.cells [0][yy].libPart.incorner.hei_s = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
					placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
				}

				// 인코너 옆에 붙은 휠러스페이서도 벽 위까지 붙임
				if ((placingZone.cells [0][yy].objType == FILLERSPACER) && ( (yy == 1) || (yy == (placingZone.nCells - 2)) )) {
					placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
					placingZone.cells [0][yy].libPart.fillersp.f_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;;
					placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

					placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
					placingZoneBackside.cells [0][yy].libPart.fillersp.f_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
					placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
				}

				// 보가 셀에 간섭하지 않는 영역까지 일단 채움
				if (indInterfereBeam == -1) {
					// 위 공간이 셀에 배치될 객체 높이 이상이면, 위 셀에도 현재 셀과 동일한 객체를 설치할 것
					if ( ((placingZone.cells [0][yy].leftBottomZ + placingZone.cells [0][yy].verLen) <= placingZone.verLen) ||
						 (abs (placingZone.verLen - (placingZone.cells [0][yy].leftBottomZ + placingZone.cells [0][yy].verLen)) < EPS) ) {
						placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
						placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);

						// 어중간하게 남는 영역 채우기 (110mm 이상은 합판으로)
						if ( (placingZone.verLen - placingZone.cells [0][yy].leftBottomZ - placingZone.cells [0][yy].verLen) >= 0.110) {
							if ( !((placingZone.cells [0][yy].objType == INCORNER) || (placingZone.cells [0][yy].objType == FILLERSPACER) || (placingZone.cells [0][yy].objType == WOOD) || (placingZone.cells [0][yy].objType == NONE)) ) {
								// 왼쪽으로 NONE이 아닌 영역을 확인해보고, WOOD와 FILLERSPACER를 만나면 NONE으로 바꾸고 그 영역들까지 합판으로 덮음
								moveCountLimit = 0;
								newXPosOffset = 0.0;
								newXSizeOffset = 0.0;
								for (kk = yy-1 ; 2 <= kk ; --kk) {

									++moveCountLimit;

									// 목재, 휠러스페이서 영역까지 덮음 (단, 인코너 옆의 휠러스페이서는 덮지 않음)
									if ((placingZone.cells [0][kk].objType == WOOD) || (placingZone.cells [0][kk].objType == FILLERSPACER)) {
										// 합판 크기 확장
										newXSizeOffset += placingZone.cells [0][kk].horLen;

										// 위치 앞으로 이동
										newXPosOffset -= placingZone.cells [0][kk].horLen;
									}

									if (moveCountLimit == 1) break;		// 이동 횟수는 최대 1회까지 제한
								}

								placingZone.cells [0][yy].objType = PLYWOOD;
								placingZone.cells [0][yy].leftBottomX += (newXPosOffset * cos(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomY += (newXPosOffset * sin(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomZ += placingZone.cells [0][yy].verLen;
								placingZone.cells [0][yy].horLen += newXSizeOffset;
								placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.p_wid = placingZone.cells [0][yy].horLen;
								placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

								placingZoneBackside.cells [0][yy].objType = PLYWOOD;
								placingZoneBackside.cells [0][yy].leftBottomZ += placingZoneBackside.cells [0][yy].verLen;
								placingZoneBackside.cells [0][yy].horLen += newXSizeOffset;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = placingZoneBackside.cells [0][yy].horLen;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							}
						
						// 어중간하게 남는 영역 채우기 (110mm 미만은 목재로)
						} else {
							if ( !((placingZone.cells [0][yy].objType == INCORNER) || (placingZone.cells [0][yy].objType == FILLERSPACER) || (placingZone.cells [0][yy].objType == NONE)) ) {
								// 왼쪽으로 NONE이 아닌 영역을 확인해보고, WOOD와 FILLERSPACER를 만나면 NONE으로 바꾸고 그 영역들까지 목재로 덮음
								moveCountLimit = 0;
								newXPosOffset = 0.0;
								newXSizeOffset = 0.0;
								for (kk = yy-1 ; 2 <= kk ; --kk) {

									++moveCountLimit;

									// 목재, 휠러스페이서 영역까지 덮음 (단, 인코너 옆의 휠러스페이서는 덮지 않음)
									if ((placingZone.cells [0][kk].objType == WOOD) || (placingZone.cells [0][kk].objType == FILLERSPACER)) {
										// 합판 크기 확장
										newXSizeOffset += placingZone.cells [0][kk].horLen;

										// 위치 앞으로 이동
										newXPosOffset -= placingZone.cells [0][kk].horLen;
									}

									if (moveCountLimit == 1) break;		// 이동 횟수는 최대 1회까지 제한
								}

								placingZone.cells [0][yy].objType = WOOD;
								placingZone.cells [0][yy].leftBottomX += (newXPosOffset * cos(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomY += (newXPosOffset * sin(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomZ += placingZone.cells [0][yy].verLen;
								placingZone.cells [0][yy].horLen += newXSizeOffset;
								placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.wood.w_w = 0.080;
								placingZone.cells [0][yy].libPart.wood.w_leng = placingZone.cells [0][yy].horLen;
								placingZone.cells [0][yy].libPart.wood.w_h = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

								placingZoneBackside.cells [0][yy].objType = WOOD;
								placingZoneBackside.cells [0][yy].leftBottomZ += placingZoneBackside.cells [0][yy].verLen;
								placingZoneBackside.cells [0][yy].horLen += newXSizeOffset;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.wood.w_w = 0.080;
								placingZoneBackside.cells [0][yy].libPart.wood.w_leng = placingZoneBackside.cells [0][yy].horLen;
								placingZoneBackside.cells [0][yy].libPart.wood.w_h = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							}
						}

					// 공간이 부족하면 합판 배치
					} else {

						if ( !((placingZone.cells [0][yy].objType == INCORNER) || (placingZone.cells [0][yy].objType == FILLERSPACER)) ) {
							// 왼쪽으로 NONE이 아닌 영역을 확인해보고, WOOD와 FILLERSPACER를 만나면 NONE으로 바꾸고 그 영역들까지 합판으로 덮음
							moveCountLimit = 0;
							newXPosOffset = 0.0;
							newXSizeOffset = 0.0;
							for (kk = yy-1 ; 2 <= kk ; --kk) {

								++moveCountLimit;

								// 목재, 휠러스페이서 영역까지 덮음 (단, 인코너 옆의 휠러스페이서는 덮지 않음)
								if ((placingZone.cells [0][kk].objType == WOOD) || (placingZone.cells [0][kk].objType == FILLERSPACER)) {
									// 합판 크기 확장
									newXSizeOffset += placingZone.cells [0][kk].horLen;

									// 위치 앞으로 이동
									newXPosOffset -= placingZone.cells [0][kk].horLen;
								}

								if (moveCountLimit == 1) break;		// 이동 횟수는 최대 1회까지 제한
							}

							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].leftBottomX += (newXPosOffset * cos(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].leftBottomY += (newXPosOffset * sin(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].horLen += newXSizeOffset;
							placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = placingZone.cells [0][yy].horLen;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].horLen += newXSizeOffset;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = placingZoneBackside.cells [0][yy].horLen;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
						}
					}
			
				// 보가 셀에 간섭하는 경우
				} else {
					dist = GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.cells [0][yy].leftBottomX, placingZone.cells [0][yy].leftBottomY);
					cellLeftX	= dist;
					cellRightX	= dist + placingZone.cells [0][yy].horLen;
					cellRightX2	= dist + placingZone.cells [0][yy].horLen * 2;
					dist = GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.beams [indInterfereBeam].leftBottomX, placingZone.beams [indInterfereBeam].leftBottomY);
					beamLeftX	= dist;
					beamRightX	= dist + placingZone.beams [indInterfereBeam].horLen;

					// 기본 유로폼 배치가 벽세우기 방식일 경우에만
					if ((placingZone.cells [0][yy].objType == EUROFORM) && (placingZone.cells [0][yy].libPart.form.u_ins_wall == true)) {
						// 보가 셀의 오른쪽으로 침범한 경우
						if ( (cellLeftX < beamLeftX) && (beamLeftX < cellRightX) && (cellRightX <= beamRightX) ) {
							// 현재 셀의 타입이 유로폼일 경우에 한해 회전시켜 배치했을 때 겹치는지 검토해 보고 안 겹치면 배치
							if ((placingZone.cells [0][yy].leftBottomZ + placingZone.cells [0][yy].horLen) <= placingZone.beams [indInterfereBeam].leftBottomZ) {
								// 유로폼 눕혀서 배치
								placingZone.cells [0][yy].libPart.form.u_ins_wall = false;
								placingZone.cells [0][yy].leftBottomX += (placingZone.cells [0][yy].verLen * cos(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomY += (placingZone.cells [0][yy].verLen * sin(placingZone.cells [0][yy].ang));
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
								placingZone.cells [0][yy].leftBottomX -= (placingZone.cells [0][yy].verLen * cos(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomY -= (placingZone.cells [0][yy].verLen * sin(placingZone.cells [0][yy].ang));

								placingZoneBackside.cells [0][yy].libPart.form.u_ins_wall = false;
								placingZoneBackside.cells [0][yy].leftBottomX += (placingZoneBackside.cells [0][yy].horLen * cos(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomY += (placingZoneBackside.cells [0][yy].horLen * sin(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
								placingZoneBackside.cells [0][yy].leftBottomX -= (placingZoneBackside.cells [0][yy].horLen * cos(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomY -= (placingZoneBackside.cells [0][yy].horLen * sin(placingZoneBackside.cells [0][yy].ang));

								// 나머지 공간 채울 것 (잠시 고도를 높였다가 나중에 복귀)
								currentHeight = placingZoneBackside.cells [0][yy].leftBottomZ;

								// Cell 정보 업데이트: 누운 유로폼 높이 위로 상승시킴
								setCellPositionLeftBottomZ (&placingZone, currentHeight + placingZone.eu_wid_numeric);
								setCellPositionLeftBottomZ (&placingZoneBackside, currentHeight + placingZone.eu_wid_numeric);

								// 보 좌측면
								placingZone.cells [0][yy].objType = PLYWOOD;
								placingZone.cells [0][yy].horLen = beamLeftX - cellLeftX;
								placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
								placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

								placingZoneBackside.cells [0][yy].objType = PLYWOOD;
								placingZoneBackside.cells [0][yy].horLen = beamLeftX - cellLeftX;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZoneBackside.cells [0][yy].leftBottomX += ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
								placingZoneBackside.cells [0][yy].leftBottomY += ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
								placingZoneBackside.cells [0][yy].leftBottomX -= ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
								placingZoneBackside.cells [0][yy].leftBottomY -= ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);

								// 보의 아래면
								placingZone.cells [0][yy].objType = PLYWOOD;
								placingZone.cells [0][yy].horLen = beamRightX - beamLeftX;
								placingZone.cells [0][yy].verLen = placingZone.beams [indInterfereBeam].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
								placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.beams [indInterfereBeam].leftBottomZ - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZone.cells [0][yy].leftBottomX += (beamLeftX - cellLeftX) * cos(placingZone.ang);
								placingZone.cells [0][yy].leftBottomY += (beamLeftX - cellLeftX) * sin(placingZone.ang);
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
								placingZone.cells [0][yy].leftBottomX -= (beamLeftX - cellLeftX) * cos(placingZone.ang);
								placingZone.cells [0][yy].leftBottomY -= (beamLeftX - cellLeftX) * sin(placingZone.ang);

								placingZoneBackside.cells [0][yy].objType = PLYWOOD;
								placingZoneBackside.cells [0][yy].horLen = beamRightX - beamLeftX;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.beams [indInterfereBeam].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.beams [indInterfereBeam].leftBottomZ - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZoneBackside.cells [0][yy].leftBottomX += (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang));
								placingZoneBackside.cells [0][yy].leftBottomY += (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang));
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
								placingZoneBackside.cells [0][yy].leftBottomX -= (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang));
								placingZoneBackside.cells [0][yy].leftBottomY -= (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang));

								// 보의 우측면
								placingZone.cells [0][yy].objType = PLYWOOD;
								placingZone.cells [0][yy].horLen = cellRightX2 - beamRightX;
								placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.p_wid = cellRightX2 - beamRightX;
								placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZone.cells [0][yy].leftBottomX += (beamRightX - cellLeftX) * cos(placingZone.ang);
								placingZone.cells [0][yy].leftBottomY += (beamRightX - cellLeftX) * sin(placingZone.ang);
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
								placingZone.cells [0][yy].leftBottomX -= (beamRightX - cellLeftX) * cos(placingZone.ang);
								placingZone.cells [0][yy].leftBottomY -= (beamRightX - cellLeftX) * sin(placingZone.ang);

								placingZoneBackside.cells [0][yy].objType = PLYWOOD;
								placingZoneBackside.cells [0][yy].horLen = cellRightX2 - beamRightX;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = cellRightX2 - beamRightX;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZoneBackside.cells [0][yy].leftBottomX += (placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
								placingZoneBackside.cells [0][yy].leftBottomY += (placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
								placingZoneBackside.cells [0][yy].leftBottomX -= (placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
								placingZoneBackside.cells [0][yy].leftBottomY -= (placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);

								// Cell 정보 업데이트: 원래 높이로 복귀
								setCellPositionLeftBottomZ (&placingZone, currentHeight);
								setCellPositionLeftBottomZ (&placingZoneBackside, currentHeight);

							// 눕힌 유로폼도 들어가지 않으면 합판이나 목재 배치
							} else {
								// 보 좌측면
								placingZone.cells [0][yy].objType = PLYWOOD;
								placingZone.cells [0][yy].horLen = beamLeftX - cellLeftX;
								placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
								placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

								placingZoneBackside.cells [0][yy].objType = PLYWOOD;
								placingZoneBackside.cells [0][yy].horLen = beamLeftX - cellLeftX;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZoneBackside.cells [0][yy].leftBottomX += ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
								placingZoneBackside.cells [0][yy].leftBottomY += ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
								placingZoneBackside.cells [0][yy].leftBottomX -= ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
								placingZoneBackside.cells [0][yy].leftBottomY -= ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);

								// 보의 아래면
								placingZone.cells [0][yy].objType = PLYWOOD;
								placingZone.cells [0][yy].horLen = beamRightX - beamLeftX;
								placingZone.cells [0][yy].verLen = placingZone.beams [indInterfereBeam].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
								placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.beams [indInterfereBeam].leftBottomZ - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZone.cells [0][yy].leftBottomX += (beamLeftX - cellLeftX) * cos(placingZone.ang);
								placingZone.cells [0][yy].leftBottomY += (beamLeftX - cellLeftX) * sin(placingZone.ang);
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
								placingZone.cells [0][yy].leftBottomX -= (beamLeftX - cellLeftX) * cos(placingZone.ang);
								placingZone.cells [0][yy].leftBottomY -= (beamLeftX - cellLeftX) * sin(placingZone.ang);

								placingZoneBackside.cells [0][yy].objType = PLYWOOD;
								placingZoneBackside.cells [0][yy].horLen = beamRightX - beamLeftX;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.beams [indInterfereBeam].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.beams [indInterfereBeam].leftBottomZ - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZoneBackside.cells [0][yy].leftBottomX += (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang));
								placingZoneBackside.cells [0][yy].leftBottomY += (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang));
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
								placingZoneBackside.cells [0][yy].leftBottomX -= (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang));
								placingZoneBackside.cells [0][yy].leftBottomY -= (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang));

								// 보의 우측면
								placingZone.cells [0][yy].objType = PLYWOOD;
								placingZone.cells [0][yy].horLen = cellRightX2 - beamRightX;
								placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.p_wid = cellRightX2 - beamRightX;
								placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZone.cells [0][yy].leftBottomX += (beamRightX - cellLeftX) * cos(placingZone.ang);
								placingZone.cells [0][yy].leftBottomY += (beamRightX - cellLeftX) * sin(placingZone.ang);
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
								placingZone.cells [0][yy].leftBottomX -= (beamRightX - cellLeftX) * cos(placingZone.ang);
								placingZone.cells [0][yy].leftBottomY -= (beamRightX - cellLeftX) * sin(placingZone.ang);

								placingZoneBackside.cells [0][yy].objType = PLYWOOD;
								placingZoneBackside.cells [0][yy].horLen = cellRightX2 - beamRightX;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = cellRightX2 - beamRightX;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZoneBackside.cells [0][yy].leftBottomX += (placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
								placingZoneBackside.cells [0][yy].leftBottomY += (placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
								placingZoneBackside.cells [0][yy].leftBottomX -= (placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
								placingZoneBackside.cells [0][yy].leftBottomY -= (placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);
							}
						}

						// 보가 셀의 왼쪽으로 침범한 경우
						if ( (cellLeftX < beamRightX) && (beamRightX < cellRightX) && (beamLeftX <= cellLeftX) ) {
							// 비어 있음
						}

						// 보가 셀 안에 들어오는 경우
						if ( (cellLeftX < beamLeftX) && (beamRightX < cellRightX) ) {
							// 보 좌측면
							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].horLen = beamLeftX - cellLeftX;
							placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].horLen = beamLeftX - cellLeftX;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [0][yy].leftBottomX += ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].leftBottomY += ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							placingZoneBackside.cells [0][yy].leftBottomX -= ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].leftBottomY -= ((beamLeftX - cellLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang);

							// 보의 아래면
							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].horLen = beamRightX - beamLeftX;
							placingZone.cells [0][yy].verLen = placingZone.beams [indInterfereBeam].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.beams [indInterfereBeam].leftBottomZ - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [0][yy].leftBottomX += (beamLeftX - cellLeftX) * cos(placingZone.ang);
							placingZone.cells [0][yy].leftBottomY += (beamLeftX - cellLeftX) * sin(placingZone.ang);
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
							placingZone.cells [0][yy].leftBottomX -= (beamLeftX - cellLeftX) * cos(placingZone.ang);
							placingZone.cells [0][yy].leftBottomY -= (beamLeftX - cellLeftX) * sin(placingZone.ang);

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].horLen = beamRightX - beamLeftX;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.beams [indInterfereBeam].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.beams [indInterfereBeam].leftBottomZ - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [0][yy].leftBottomX += (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang));
							placingZoneBackside.cells [0][yy].leftBottomY += (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang));
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							placingZoneBackside.cells [0][yy].leftBottomX -= (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * cos(placingZoneBackside.ang));
							placingZoneBackside.cells [0][yy].leftBottomY -= (((beamLeftX - cellLeftX + beamRightX - beamLeftX) - placingZoneBackside.eu_wid_numeric) * sin(placingZoneBackside.ang));

							// 보의 우측면
							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].horLen = cellRightX - beamRightX;
							placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = cellRightX - beamRightX;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [0][yy].leftBottomX += (beamRightX - cellLeftX) * cos(placingZone.ang);
							placingZone.cells [0][yy].leftBottomY += (beamRightX - cellLeftX) * sin(placingZone.ang);
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
							placingZone.cells [0][yy].leftBottomX -= (beamRightX - cellLeftX) * cos(placingZone.ang);
							placingZone.cells [0][yy].leftBottomY -= (beamRightX - cellLeftX) * sin(placingZone.ang);

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].horLen = cellRightX - beamRightX;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = cellRightX - beamRightX;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
						}
					}
				}
			}

		// 유로폼 설치 방향: 벽눕히기
		} else {

			xx = placingZone.eu_count_ver;

			// Cell 정보 업데이트: 다음 높이로 상승시킴
			setCellPositionLeftBottomZ (&placingZone, placingZone.leftBottomZ + (placingZone.eu_wid_numeric * xx));
			setCellPositionLeftBottomZ (&placingZoneBackside, placingZoneBackside.leftBottomZ + (placingZoneBackside.eu_wid_numeric * xx));

			// 기본 채우기 라인 상단
			for (yy = 0 ; yy < placingZone.nCells ; ++yy) {

			// 보의 중첩 관계 확인 - 중첩되는 보의 인덱스를 먼저 추출
				indInterfereBeam = -1;
				for (zz = 0 ; zz < placingZone.nInterfereBeams ; ++zz) {
				
					// 간섭을 찾을 때까지만 확인
					if (indInterfereBeam == -1) {
						dist = GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.cells [0][yy].leftBottomX, placingZone.cells [0][yy].leftBottomY);
						cellLeftX	= dist - placingZone.cells [0][yy].horLen;
						cellRightX	= dist;
						dist = GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.beams [zz].leftBottomX, placingZone.beams [zz].leftBottomY);
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

				// 인코너는 무조건 벽 위까지 붙임
				if (placingZone.cells [0][yy].objType == INCORNER) {
					placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
					placingZone.cells [0][yy].libPart.incorner.hei_s = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;;
					placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

					placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
					placingZoneBackside.cells [0][yy].libPart.incorner.hei_s = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
					placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
				}

				// 인코너 옆에 붙은 휠러스페이서도 벽 위까지 붙임
				if ((placingZone.cells [0][yy].objType == FILLERSPACER) && ( (yy == 1) || (yy == (placingZone.nCells - 2)) )) {
					placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
					placingZone.cells [0][yy].libPart.fillersp.f_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;;
					placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

					placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
					placingZoneBackside.cells [0][yy].libPart.fillersp.f_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
					placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
				}

				// 보가 셀에 간섭하지 않는 영역까지 일단 채움
				if (indInterfereBeam == -1) {
					// 위 공간이 셀에 배치될 객체 높이 이상이면, 위 셀에도 현재 셀과 동일한 객체를 설치할 것
					if ( ((placingZone.cells [0][yy].leftBottomZ + placingZone.cells [0][yy].verLen) <= placingZone.verLen) ||
						 (abs (placingZone.verLen - (placingZone.cells [0][yy].leftBottomZ + placingZone.cells [0][yy].verLen)) < EPS) ) {
						placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
						placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);

						// 어중간하게 남는 영역 채우기 (110mm 이상은 합판으로)
						if ( (placingZone.verLen - placingZone.cells [0][yy].leftBottomZ - placingZone.cells [0][yy].verLen) >= 0.110) {
							if ( !((placingZone.cells [0][yy].objType == INCORNER) || (placingZone.cells [0][yy].objType == FILLERSPACER) || (placingZone.cells [0][yy].objType == NONE)) ) {
								// 왼쪽으로 NONE이 아닌 영역을 확인해보고, WOOD와 FILLERSPACER를 만나면 NONE으로 바꾸고 그 영역들까지 합판으로 덮음
								moveCountLimit = 0;
								newXPosOffset = 0.0;
								newXSizeOffset = 0.0;
								for (kk = yy-1 ; 2 <= kk ; --kk) {

									++moveCountLimit;

									// 목재, 휠러스페이서 영역까지 덮음 (단, 인코너 옆의 휠러스페이서는 덮지 않음)
									if ((placingZone.cells [0][kk].objType == WOOD) || (placingZone.cells [0][kk].objType == FILLERSPACER)) {
										// 합판 크기 확장
										newXSizeOffset += placingZone.cells [0][kk].horLen;

										// 위치 앞으로 이동
										newXPosOffset -= placingZone.cells [0][kk].horLen;
									}

									if (moveCountLimit == 1) break;		// 이동 횟수는 최대 1회까지 제한
								}

								placingZone.cells [0][yy].objType = PLYWOOD;
								placingZone.cells [0][yy].leftBottomX += ((newXPosOffset - placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomY += ((newXPosOffset - placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomZ += placingZone.cells [0][yy].verLen;
								placingZone.cells [0][yy].horLen += newXSizeOffset;
								placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.p_wid = placingZone.cells [0][yy].horLen;
								placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
								placingZone.cells [0][yy].leftBottomX -= ((newXPosOffset - placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomY -= ((newXPosOffset - placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomZ -= placingZone.cells [0][yy].verLen;

								placingZoneBackside.cells [0][yy].objType = PLYWOOD;
								placingZoneBackside.cells [0][yy].leftBottomX += ((- placingZoneBackside.cells [0][yy].horLen) * cos(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomY += ((- placingZoneBackside.cells [0][yy].horLen) * sin(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomZ += placingZoneBackside.cells [0][yy].verLen;
								placingZoneBackside.cells [0][yy].horLen += newXSizeOffset;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = placingZoneBackside.cells [0][yy].horLen;
								placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
								placingZoneBackside.cells [0][yy].leftBottomX -= ((- placingZoneBackside.cells [0][yy].horLen) * cos(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomY -= ((- placingZoneBackside.cells [0][yy].horLen) * sin(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomZ -= placingZoneBackside.cells [0][yy].verLen;
							}
						
						// 어중간하게 남는 영역 채우기 (110mm 미만은 목재로)
						} else {
							if ( !((placingZone.cells [0][yy].objType == INCORNER) || (placingZone.cells [0][yy].objType == FILLERSPACER) || (placingZone.cells [0][yy].objType == NONE)) ) {
								// 왼쪽으로 NONE이 아닌 영역을 확인해보고, WOOD와 FILLERSPACER를 만나면 NONE으로 바꾸고 그 영역들까지 합판으로 덮음
								moveCountLimit = 0;
								newXPosOffset = 0.0;
								newXSizeOffset = 0.0;
								for (kk = yy-1 ; 2 <= kk ; --kk) {

									++moveCountLimit;

									// 목재, 휠러스페이서 영역까지 덮음 (단, 인코너 옆의 휠러스페이서는 덮지 않음)
									if ((placingZone.cells [0][kk].objType == WOOD) || (placingZone.cells [0][kk].objType == FILLERSPACER)) {
										// 합판 크기 확장
										newXSizeOffset += placingZone.cells [0][kk].horLen;

										// 위치 앞으로 이동
										newXPosOffset -= placingZone.cells [0][kk].horLen;
									}

									if (moveCountLimit == 1) break;		// 이동 횟수는 최대 1회까지 제한
								}

								placingZone.cells [0][yy].objType = WOOD;
								placingZone.cells [0][yy].leftBottomX += ((newXPosOffset - placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomY += ((newXPosOffset - placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomZ += placingZone.cells [0][yy].verLen;
								placingZone.cells [0][yy].horLen += newXSizeOffset;
								placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [0][yy].libPart.wood.w_w = 0.080;
								placingZone.cells [0][yy].libPart.wood.w_leng = placingZone.cells [0][yy].horLen;
								placingZone.cells [0][yy].libPart.wood.w_h = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
								placingZone.cells [0][yy].leftBottomX -= ((newXPosOffset - placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomY -= ((newXPosOffset - placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));
								placingZone.cells [0][yy].leftBottomZ -= placingZone.cells [0][yy].verLen;

								placingZoneBackside.cells [0][yy].objType = WOOD;
								placingZoneBackside.cells [0][yy].leftBottomX += ((- placingZoneBackside.cells [0][yy].horLen) * cos(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomY += ((- placingZoneBackside.cells [0][yy].horLen) * sin(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomZ += placingZoneBackside.cells [0][yy].verLen;
								placingZoneBackside.cells [0][yy].horLen += newXSizeOffset;
								placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [0][yy].libPart.wood.w_w = 0.080;
								placingZoneBackside.cells [0][yy].libPart.wood.w_leng = placingZoneBackside.cells [0][yy].horLen;
								placingZoneBackside.cells [0][yy].libPart.wood.w_h = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
								placingZoneBackside.cells [0][yy].leftBottomX -= ((- placingZoneBackside.cells [0][yy].horLen) * cos(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomY -= ((- placingZoneBackside.cells [0][yy].horLen) * sin(placingZoneBackside.cells [0][yy].ang));
								placingZoneBackside.cells [0][yy].leftBottomZ -= placingZoneBackside.cells [0][yy].verLen;
							}
						}

					// 공간이 부족하면 합판 배치
					} else {

						if ( !((placingZone.cells [0][yy].objType == INCORNER) || (placingZone.cells [0][yy].objType == FILLERSPACER)) ) {
							// 왼쪽으로 NONE이 아닌 영역을 확인해보고, WOOD와 FILLERSPACER를 만나면 NONE으로 바꾸고 그 영역들까지 합판으로 덮음
							moveCountLimit = 0;
							newXPosOffset = 0.0;
							newXSizeOffset = 0.0;
							for (kk = yy-1 ; 2 <= kk ; --kk) {

								++moveCountLimit;

								// 목재, 휠러스페이서 영역까지 덮음 (단, 인코너 옆의 휠러스페이서는 덮지 않음)
								if ((placingZone.cells [0][kk].objType == WOOD) || (placingZone.cells [0][kk].objType == FILLERSPACER)) {
									// 합판 크기 확장
									newXSizeOffset += placingZone.cells [0][kk].horLen;

									// 위치 앞으로 이동
									newXPosOffset -= placingZone.cells [0][kk].horLen;
								}

								if (moveCountLimit == 1) break;		// 이동 횟수는 최대 1회까지 제한
							}

							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].leftBottomX += ((newXPosOffset - placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].leftBottomY += ((newXPosOffset - placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].horLen += newXSizeOffset;
							placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = placingZone.cells [0][yy].horLen;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
							placingZone.cells [0][yy].leftBottomX -= ((newXPosOffset - placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].leftBottomY -= ((newXPosOffset - placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].leftBottomX += (( - placingZoneBackside.cells [0][yy].horLen) * cos(placingZoneBackside.cells [0][yy].ang));
							placingZoneBackside.cells [0][yy].leftBottomY += (( - placingZoneBackside.cells [0][yy].horLen) * sin(placingZoneBackside.cells [0][yy].ang));
							placingZoneBackside.cells [0][yy].horLen += newXSizeOffset;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = placingZoneBackside.cells [0][yy].horLen;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							placingZoneBackside.cells [0][yy].leftBottomX -= (( - placingZoneBackside.cells [0][yy].horLen) * cos(placingZoneBackside.cells [0][yy].ang));
							placingZoneBackside.cells [0][yy].leftBottomY -= (( - placingZoneBackside.cells [0][yy].horLen) * sin(placingZoneBackside.cells [0][yy].ang));
						}
					}
			
				// 보가 셀에 간섭하는 경우
				} else {
					dist = GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.cells [0][yy].leftBottomX, placingZone.cells [0][yy].leftBottomY);
					cellLeftX	= dist - placingZone.cells [0][yy].horLen;
					cellRightX	= dist;
					cellRightX2	= dist + placingZone.cells [0][yy].horLen;
					dist = GetDistance (placingZone.leftBottomX, placingZone.leftBottomY, placingZone.beams [indInterfereBeam].leftBottomX, placingZone.beams [indInterfereBeam].leftBottomY);
					beamLeftX	= dist;
					beamRightX	= dist + placingZone.beams [indInterfereBeam].horLen;

					// 기본 유로폼 배치가 벽눕히기 방식일 경우에만
					if ((placingZone.cells [0][yy].objType == EUROFORM) && (placingZone.cells [0][yy].libPart.form.u_ins_wall == false)) {
						// 보가 셀의 오른쪽으로 침범한 경우
						if ( (cellLeftX < beamLeftX) && (beamLeftX < cellRightX) && (cellRightX <= beamRightX) ) {
							// 보 좌측면
							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].leftBottomX += ((- placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].leftBottomY += ((- placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].horLen = beamLeftX - cellLeftX;
							placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
							placingZone.cells [0][yy].leftBottomX -= ((- placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].leftBottomY -= ((- placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].leftBottomX += ((beamLeftX - cellLeftX)) * cos(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].leftBottomY += ((beamLeftX - cellLeftX)) * sin(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].horLen = beamLeftX - cellLeftX;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							placingZoneBackside.cells [0][yy].leftBottomX -= ((beamLeftX - cellLeftX)) * cos(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].leftBottomY -= ((beamLeftX - cellLeftX)) * sin(placingZoneBackside.ang);

							// 보의 아래면
							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].horLen = beamRightX - beamLeftX;
							placingZone.cells [0][yy].verLen = placingZone.beams [indInterfereBeam].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.beams [indInterfereBeam].leftBottomZ - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].leftBottomX += ((- (beamLeftX - cellLeftX + beamRightX - beamLeftX)) * cos(placingZoneBackside.cells [0][yy].ang));
							placingZoneBackside.cells [0][yy].leftBottomY += ((- (beamLeftX - cellLeftX + beamRightX - beamLeftX)) * sin(placingZoneBackside.cells [0][yy].ang));
							placingZoneBackside.cells [0][yy].horLen = beamRightX - beamLeftX;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.beams [indInterfereBeam].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.beams [indInterfereBeam].leftBottomZ - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							placingZoneBackside.cells [0][yy].leftBottomX -= ((- (beamLeftX - cellLeftX + beamRightX - beamLeftX)) * cos(placingZoneBackside.cells [0][yy].ang));
							placingZoneBackside.cells [0][yy].leftBottomY -= ((- (beamLeftX - cellLeftX + beamRightX - beamLeftX)) * sin(placingZoneBackside.cells [0][yy].ang));

							// 보의 우측면
							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].leftBottomX += (beamRightX - beamLeftX) * cos(placingZone.ang);
							placingZone.cells [0][yy].leftBottomY += (beamRightX - beamLeftX) * sin(placingZone.ang);
							placingZone.cells [0][yy].horLen = cellRightX2 - beamRightX;
							placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = cellRightX2 - beamRightX;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
							placingZone.cells [0][yy].leftBottomX -= (beamRightX - beamLeftX) * cos(placingZone.ang);
							placingZone.cells [0][yy].leftBottomY -= (beamRightX - beamLeftX) * sin(placingZone.ang);

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].leftBottomX += (placingZoneBackside.eu_hei_numeric * 2) * cos(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].leftBottomY += (placingZoneBackside.eu_hei_numeric * 2) * sin(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].horLen = cellRightX2 - beamRightX;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = cellRightX2 - beamRightX;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							placingZoneBackside.cells [0][yy].leftBottomX -= (placingZoneBackside.eu_hei_numeric * 2) * cos(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].leftBottomY -= (placingZoneBackside.eu_hei_numeric * 2) * sin(placingZoneBackside.ang);
						}

						// 보가 셀의 왼쪽으로 침범한 경우
						if ( (cellLeftX < beamRightX) && (beamRightX < cellRightX) && (beamLeftX <= cellLeftX) ) {
							// 비어 있음
						}

						// 보가 셀 안에 들어오는 경우
						if ( (cellLeftX < beamLeftX) && (beamRightX < cellRightX) ) {
							// 보 좌측면
							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].leftBottomX += ((- placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].leftBottomY += ((- placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].horLen = beamLeftX - cellLeftX;
							placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
							placingZone.cells [0][yy].leftBottomX -= ((- placingZone.cells [0][yy].horLen) * cos(placingZone.cells [0][yy].ang));
							placingZone.cells [0][yy].leftBottomY -= ((- placingZone.cells [0][yy].horLen) * sin(placingZone.cells [0][yy].ang));

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].leftBottomX += ((beamLeftX - cellLeftX)) * cos(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].leftBottomY += ((beamLeftX - cellLeftX)) * sin(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].horLen = beamLeftX - cellLeftX;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamLeftX - cellLeftX;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							placingZoneBackside.cells [0][yy].leftBottomX -= ((beamLeftX - cellLeftX)) * cos(placingZoneBackside.ang);
							placingZoneBackside.cells [0][yy].leftBottomY -= ((beamLeftX - cellLeftX)) * sin(placingZoneBackside.ang);

							// 보의 아래면
							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].horLen = beamRightX - beamLeftX;
							placingZone.cells [0][yy].verLen = placingZone.beams [indInterfereBeam].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.beams [indInterfereBeam].leftBottomZ - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].leftBottomX += ((beamLeftX - cellLeftX + beamRightX - beamLeftX) * cos(placingZoneBackside.ang));
							placingZoneBackside.cells [0][yy].leftBottomY += ((beamLeftX - cellLeftX + beamRightX - beamLeftX) * sin(placingZoneBackside.ang));
							placingZoneBackside.cells [0][yy].horLen = beamRightX - beamLeftX;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.beams [indInterfereBeam].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = beamRightX - beamLeftX;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.beams [indInterfereBeam].leftBottomZ - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							placingZoneBackside.cells [0][yy].leftBottomX -= ((beamLeftX - cellLeftX + beamRightX - beamLeftX) * cos(placingZoneBackside.ang));
							placingZoneBackside.cells [0][yy].leftBottomY -= ((beamLeftX - cellLeftX + beamRightX - beamLeftX) * sin(placingZoneBackside.ang));

							// 보의 우측면
							placingZone.cells [0][yy].objType = PLYWOOD;
							placingZone.cells [0][yy].leftBottomX += (beamRightX - beamLeftX) * cos(placingZone.ang);
							placingZone.cells [0][yy].leftBottomY += (beamRightX - beamLeftX) * sin(placingZone.ang);
							placingZone.cells [0][yy].horLen = cellRightX - beamRightX;
							placingZone.cells [0][yy].verLen = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.p_wid = cellRightX - beamRightX;
							placingZone.cells [0][yy].libPart.plywood.p_leng = placingZone.verLen - placingZone.cells [0][yy].leftBottomZ;
							placingZone.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);
							placingZone.cells [0][yy].leftBottomX -= (beamRightX - beamLeftX) * cos(placingZone.ang);
							placingZone.cells [0][yy].leftBottomY -= (beamRightX - beamLeftX) * sin(placingZone.ang);

							placingZoneBackside.cells [0][yy].objType = PLYWOOD;
							placingZoneBackside.cells [0][yy].leftBottomX += ((- placingZoneBackside.eu_hei_numeric) * cos(placingZoneBackside.cells [0][yy].ang));
							placingZoneBackside.cells [0][yy].leftBottomY += ((- placingZoneBackside.eu_hei_numeric) * sin(placingZoneBackside.cells [0][yy].ang));
							placingZoneBackside.cells [0][yy].horLen = cellRightX - beamRightX;
							placingZoneBackside.cells [0][yy].verLen = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_wid = cellRightX - beamRightX;
							placingZoneBackside.cells [0][yy].libPart.plywood.p_leng = placingZoneBackside.verLen - placingZoneBackside.cells [0][yy].leftBottomZ;
							placingZoneBackside.cells [0][yy].libPart.plywood.w_dir_wall = true;
							placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);
							placingZoneBackside.cells [0][yy].leftBottomX -= ((- placingZoneBackside.eu_hei_numeric) * cos(placingZoneBackside.cells [0][yy].ang));
							placingZoneBackside.cells [0][yy].leftBottomY -= ((- placingZoneBackside.eu_hei_numeric) * sin(placingZoneBackside.cells [0][yy].ang));
						}
					}
				}
			}
		}

		return NoError;
	});

	return err;
}

// 해당 셀 정보를 기반으로 라이브러리 배치
API_Guid	placeLibPart (Cell objInfo)
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
	element.header.layer = layerInd;
	element.header.floorInd = infoWall.floorInd;

	if (objInfo.objType == INCORNER) {
		memo.params [0][27].value.real = objInfo.libPart.incorner.wid_s;	// 가로(빨강)
		memo.params [0][28].value.real = objInfo.libPart.incorner.leng_s;	// 세로(파랑)
		memo.params [0][29].value.real = objInfo.libPart.incorner.hei_s;	// 높이
		GS::ucscpy (memo.params [0][30].value.uStr, L("세우기"));			// 설치방향

	} else if (objInfo.objType == EUROFORM) {
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
		if (objInfo.libPart.form.u_ins_wall == true)
			tempString = "벽세우기";
		else
			tempString = "벽눕히기";
		GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

	} else if (objInfo.objType == FILLERSPACER) {
		memo.params [0][27].value.real = objInfo.libPart.fillersp.f_thk;	// 두께
		memo.params [0][28].value.real = objInfo.libPart.fillersp.f_leng;	// 길이

	} else if (objInfo.objType == PLYWOOD) {
		GS::ucscpy (memo.params [0][32].value.uStr, L("비규격"));
		memo.params [0][35].value.real = objInfo.libPart.fillersp.f_thk;	// 가로
		memo.params [0][36].value.real = objInfo.libPart.fillersp.f_leng;	// 세로
		
		// 설치방향
		if (objInfo.libPart.plywood.w_dir_wall == true)
			tempString = "벽세우기";
		else
			tempString = "벽눕히기";
		GS::ucscpy (memo.params [0][33].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());
	} else if (objInfo.objType == WOOD) {
		GS::ucscpy (memo.params [0][27].value.uStr, L("벽세우기"));		// 설치방향
		memo.params [0][28].value.real = objInfo.libPart.wood.w_w;		// 두께
		memo.params [0][29].value.real = objInfo.libPart.wood.w_h;		// 너비
		memo.params [0][30].value.real = objInfo.libPart.wood.w_leng;	// 길이
		memo.params [0][31].value.real = 0;								// 각도
	}

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// [0]행 - 해당 셀의 좌하단 좌표X 위치를 리턴
double	getCellPositionLeftBottomX (PlacingZone *src_zone, short idx)
{
	double		distance = 0.0;
	short		xx;

	for (xx = 0 ; xx < idx ; ++xx) {
		if (src_zone->cells [0][xx].objType != NONE)
			distance += src_zone->cells [0][xx].horLen;
	}

	return distance;
}

// [0]행 - 전체 셀의 최하단 좌표Z 위치를 설정
void	setCellPositionLeftBottomZ (PlacingZone *src_zone, double new_hei)
{
	short		xx;

	for (xx = 0 ; xx < src_zone->nCells ; ++xx) {
		if (src_zone->cells [0][xx].objType != NONE)
			src_zone->cells [0][xx].leftBottomZ = new_hei;
	}
}

// Cell 배열을 초기화함
void	initCells (PlacingZone* placingZone)
{
	short xx, yy;

	for (xx = 0 ; xx < 50 ; ++xx)
		for (yy = 0 ; yy < 100 ; ++yy)
			placingZone->cells [xx][yy].objType = NONE;
}

// 1차 배치: 인코너, 유로폼
void	firstPlacingSettings (PlacingZone* placingZone)
{
	short			xx, yy;
	std::string		tempString;

	// 왼쪽 인코너 설정
	if (placingZone->bLIncorner) {
		placingZone->cells [0][0].objType = INCORNER;
		placingZone->cells [0][0].horLen = placingZone->lenLIncorner;
		if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0)
			placingZone->cells [0][0].verLen = placingZone->eu_hei_numeric;
		else
			placingZone->cells [0][0].verLen = placingZone->eu_wid_numeric;
		placingZone->cells [0][0].ang = placingZone->ang + degreeToRad (-90);
		placingZone->cells [0][0].leftBottomX = placingZone->leftBottomX;
		placingZone->cells [0][0].leftBottomY = placingZone->leftBottomY;
		placingZone->cells [0][0].leftBottomZ = placingZone->leftBottomZ;
		placingZone->cells [0][0].libPart.incorner.wid_s = 0.100;								// 인코너패널 - 가로(빨강)
		placingZone->cells [0][0].libPart.incorner.leng_s = placingZone->lenLIncorner;			// 인코너패널 - 세로(파랑)
		if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0)
			placingZone->cells [0][0].libPart.incorner.hei_s = placingZone->eu_hei_numeric;		// 인코너패널 - 높이
		else
			placingZone->cells [0][0].libPart.incorner.hei_s = placingZone->eu_wid_numeric;		// 인코너패널 - 높이
	}

	// 유로폼 설정
	for (xx = 1 ; xx <= placingZone->eu_count_hor ; ++xx) {
		yy = xx * 2;	// Cell의 객체 정보를 채움 (인덱스: 2, 4, 6 식으로)

		placingZone->cells [0][yy].objType = EUROFORM;
		placingZone->cells [0][yy].ang = placingZone->ang;

		if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0) {
			placingZone->cells [0][yy].libPart.form.u_ins_wall = true;
			placingZone->cells [0][yy].leftBottomX = placingZone->leftBottomX + (getCellPositionLeftBottomX (placingZone, yy) * cos(placingZone->ang));
			placingZone->cells [0][yy].leftBottomY = placingZone->leftBottomY + (getCellPositionLeftBottomX (placingZone, yy) * sin(placingZone->ang));
			placingZone->cells [0][yy].horLen = placingZone->eu_wid_numeric;
			placingZone->cells [0][yy].verLen = placingZone->eu_hei_numeric;
		} else {
			placingZone->cells [0][yy].libPart.form.u_ins_wall = false;
			placingZone->cells [0][yy].leftBottomX = placingZone->leftBottomX + ((getCellPositionLeftBottomX (placingZone, yy) + placingZone->eu_hei_numeric) * cos(placingZone->ang));
			placingZone->cells [0][yy].leftBottomY = placingZone->leftBottomY + ((getCellPositionLeftBottomX (placingZone, yy) + placingZone->eu_hei_numeric) * sin(placingZone->ang));
			placingZone->cells [0][yy].horLen = placingZone->eu_hei_numeric;
			placingZone->cells [0][yy].verLen = placingZone->eu_wid_numeric;
		}
		placingZone->cells [0][yy].leftBottomZ = placingZone->leftBottomZ;
		placingZone->cells [0][yy].libPart.form.eu_stan_onoff = true;
		placingZone->cells [0][yy].libPart.form.eu_wid = placingZone->eu_wid_numeric;
		placingZone->cells [0][yy].libPart.form.eu_hei = placingZone->eu_hei_numeric;
	}

	// 오른쪽 인코너 설정
	if (placingZone->bRIncorner) {
		placingZone->cells [0][placingZone->nCells - 1].objType = INCORNER;
		placingZone->cells [0][placingZone->nCells - 1].horLen = placingZone->lenRIncorner;
		if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0)
			placingZone->cells [0][placingZone->nCells - 1].verLen = placingZone->eu_hei_numeric;
		else
			placingZone->cells [0][placingZone->nCells - 1].verLen = placingZone->eu_wid_numeric;
		placingZone->cells [0][placingZone->nCells - 1].ang = placingZone->ang + degreeToRad (-180);
		placingZone->cells [0][placingZone->nCells - 1].leftBottomX = placingZone->leftBottomX + ((getCellPositionLeftBottomX (placingZone, placingZone->nCells - 1) + placingZone->lenRIncorner) * cos(placingZone->ang));
		placingZone->cells [0][placingZone->nCells - 1].leftBottomY = placingZone->leftBottomY + ((getCellPositionLeftBottomX (placingZone, placingZone->nCells - 1) + placingZone->lenRIncorner) * sin(placingZone->ang));
		placingZone->cells [0][placingZone->nCells - 1].leftBottomZ = placingZone->leftBottomZ;
		placingZone->cells [0][placingZone->nCells - 1].libPart.incorner.wid_s = placingZone->lenRIncorner;				// 인코너패널 - 가로(빨강)
		placingZone->cells [0][placingZone->nCells - 1].libPart.incorner.leng_s = 0.100;								// 인코너패널 - 세로(파랑)
		if (placingZone->eu_ori.compare (std::string ("벽세우기")) == 0)
			placingZone->cells [0][placingZone->nCells - 1].libPart.incorner.hei_s = placingZone->eu_hei_numeric;		// 인코너패널 - 높이
		else
			placingZone->cells [0][placingZone->nCells - 1].libPart.incorner.hei_s = placingZone->eu_wid_numeric;		// 인코너패널 - 높이
	}
}

// 원본 벽면 영역 정보를 대칭하는 반대쪽에도 복사함
void	copyPlacingZoneSymmetric (PlacingZone* src_zone, PlacingZone* dst_zone, InfoWall* infoWall)
{
	short	xx;

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
	for (xx = 0 ; xx < dst_zone->nCells ; ++xx) {

		// 아무것도 없으면,
		if (src_zone->cells [0][xx].objType == NONE) {

			dst_zone->cells [0][xx].objType			= NONE;
			dst_zone->cells [0][xx].horLen			= 0;
			dst_zone->cells [0][xx].verLen			= 0;
			dst_zone->cells [0][xx].ang				= dst_zone->ang;
			dst_zone->cells [0][xx].leftBottomX		= dst_zone->leftBottomX + (getCellPositionLeftBottomX (src_zone, xx) * cos(src_zone->ang));
			dst_zone->cells [0][xx].leftBottomY		= dst_zone->leftBottomY + (getCellPositionLeftBottomX (src_zone, xx) * sin(src_zone->ang));
			dst_zone->cells [0][xx].leftBottomZ		= dst_zone->leftBottomZ;

		// 인코너인 경우,
		} else if (src_zone->cells [0][xx].objType == INCORNER) {
			// 왼쪽 인코너
			if ((placingZoneBackside.bLIncorner) && (xx == 0)) {
				dst_zone->cells [0][xx].objType						= INCORNER;
				dst_zone->cells [0][xx].horLen						= src_zone->cells [0][xx].horLen;
				dst_zone->cells [0][xx].verLen						= src_zone->cells [0][xx].verLen;
				dst_zone->cells [0][xx].ang							= dst_zone->ang;
				dst_zone->cells [0][xx].leftBottomX					= dst_zone->leftBottomX;
				dst_zone->cells [0][xx].leftBottomY					= dst_zone->leftBottomY;
				dst_zone->cells [0][xx].leftBottomZ					= dst_zone->leftBottomZ;
				dst_zone->cells [0][xx].libPart.incorner.wid_s		= src_zone->cells [0][xx].libPart.incorner.leng_s;		// 인코너패널 - 가로(빨강)
				dst_zone->cells [0][xx].libPart.incorner.leng_s		= src_zone->cells [0][xx].libPart.incorner.wid_s;		// 인코너패널 - 세로(파랑)
				dst_zone->cells [0][xx].libPart.incorner.hei_s		= src_zone->cells [0][xx].libPart.incorner.hei_s;		// 인코너패널 - 높이

			// 오른쪽 인코너
			} else if ((placingZoneBackside.bRIncorner) && (xx == src_zone->nCells - 1)) {
				dst_zone->cells [0][xx].objType						= INCORNER;
				dst_zone->cells [0][xx].horLen						= src_zone->cells [0][xx].horLen;
				dst_zone->cells [0][xx].verLen						= src_zone->cells [0][xx].verLen;
				dst_zone->cells [0][xx].ang							= dst_zone->ang + degreeToRad (90);
				dst_zone->cells [0][xx].leftBottomX					= dst_zone->leftBottomX + ((getCellPositionLeftBottomX (src_zone, xx) + src_zone->lenRIncorner) * cos(src_zone->ang));
				dst_zone->cells [0][xx].leftBottomY					= dst_zone->leftBottomY + ((getCellPositionLeftBottomX (src_zone, xx) + src_zone->lenRIncorner) * sin(src_zone->ang));
				dst_zone->cells [0][xx].leftBottomZ					= dst_zone->leftBottomZ;
				dst_zone->cells [0][xx].libPart.incorner.wid_s		= src_zone->cells [0][xx].libPart.incorner.leng_s;		// 인코너패널 - 가로(빨강)
				dst_zone->cells [0][xx].libPart.incorner.leng_s		= src_zone->cells [0][xx].libPart.incorner.wid_s;		// 인코너패널 - 세로(파랑)
				dst_zone->cells [0][xx].libPart.incorner.hei_s		= src_zone->cells [0][xx].libPart.incorner.hei_s;		// 인코너패널 - 높이

			// 기타 위치
			} else {
				dst_zone->cells [0][xx].objType						= INCORNER;
				dst_zone->cells [0][xx].horLen						= src_zone->cells [0][xx].horLen;
				dst_zone->cells [0][xx].verLen						= src_zone->cells [0][xx].verLen;
				dst_zone->cells [0][xx].ang							= dst_zone->ang + degreeToRad (90);
				dst_zone->cells [0][xx].leftBottomX					= dst_zone->leftBottomX + ((getCellPositionLeftBottomX (src_zone, xx) + src_zone->lenRIncorner) * cos(src_zone->ang));
				dst_zone->cells [0][xx].leftBottomY					= dst_zone->leftBottomY + ((getCellPositionLeftBottomX (src_zone, xx) + src_zone->lenRIncorner) * sin(src_zone->ang));
				dst_zone->cells [0][xx].leftBottomZ					= dst_zone->leftBottomZ;
				dst_zone->cells [0][xx].libPart.incorner.wid_s		= src_zone->cells [0][xx].libPart.incorner.leng_s;		// 인코너패널 - 가로(빨강)
				dst_zone->cells [0][xx].libPart.incorner.leng_s		= src_zone->cells [0][xx].libPart.incorner.wid_s;		// 인코너패널 - 세로(파랑)
				dst_zone->cells [0][xx].libPart.incorner.hei_s		= src_zone->cells [0][xx].libPart.incorner.hei_s;		// 인코너패널 - 높이
			}
		
		// 유로폼인 경우,
		} else if (src_zone->cells [0][xx].objType == EUROFORM) {
		
			dst_zone->cells [0][xx].objType					= EUROFORM;
			dst_zone->cells [0][xx].horLen					= src_zone->cells [0][xx].horLen;
			dst_zone->cells [0][xx].verLen					= src_zone->cells [0][xx].verLen;
			dst_zone->cells [0][xx].ang						= dst_zone->ang + degreeToRad (180);

			// 벽세우기
			if (src_zone->cells [0][xx].libPart.form.u_ins_wall == true) {
				dst_zone->cells [0][xx].leftBottomX = dst_zone->leftBottomX + ((getCellPositionLeftBottomX (src_zone, xx) + src_zone->cells [0][xx].horLen) * cos(src_zone->ang));
				dst_zone->cells [0][xx].leftBottomY = dst_zone->leftBottomY + ((getCellPositionLeftBottomX (src_zone, xx) + src_zone->cells [0][xx].horLen) * sin(src_zone->ang));
			// 벽눕히기
			} else {
				dst_zone->cells [0][xx].leftBottomX = dst_zone->leftBottomX + ((getCellPositionLeftBottomX (src_zone, xx)) * cos(src_zone->ang));
				dst_zone->cells [0][xx].leftBottomY = dst_zone->leftBottomY + ((getCellPositionLeftBottomX (src_zone, xx)) * sin(src_zone->ang));
			}
			dst_zone->cells [0][xx].leftBottomZ					= dst_zone->cells [0][xx].leftBottomZ;
			dst_zone->cells [0][xx].libPart.form.eu_stan_onoff	= src_zone->cells [0][xx].libPart.form.eu_stan_onoff;
			dst_zone->cells [0][xx].libPart.form.eu_wid			= src_zone->cells [0][xx].libPart.form.eu_wid;
			dst_zone->cells [0][xx].libPart.form.eu_hei			= src_zone->cells [0][xx].libPart.form.eu_hei;
			dst_zone->cells [0][xx].libPart.form.eu_wid2		= src_zone->cells [0][xx].libPart.form.eu_wid2;
			dst_zone->cells [0][xx].libPart.form.eu_hei2		= src_zone->cells [0][xx].libPart.form.eu_hei2;
			dst_zone->cells [0][xx].libPart.form.u_ins_wall		= src_zone->cells [0][xx].libPart.form.u_ins_wall;

		// 휠러스페이서인 경우,
		} else if (src_zone->cells [0][xx].objType == FILLERSPACER) {
		
			dst_zone->cells [0][xx].objType			= FILLERSPACER;
			dst_zone->cells [0][xx].horLen			= src_zone->cells [0][xx].horLen;
			dst_zone->cells [0][xx].verLen			= src_zone->cells [0][xx].verLen;
			dst_zone->cells [0][xx].ang				= dst_zone->ang + degreeToRad (180);
			dst_zone->cells [0][xx].leftBottomX		= dst_zone->leftBottomX + ((getCellPositionLeftBottomX (src_zone, xx)) * cos(src_zone->ang));
			dst_zone->cells [0][xx].leftBottomY		= dst_zone->leftBottomY + ((getCellPositionLeftBottomX (src_zone, xx)) * sin(src_zone->ang));
			dst_zone->cells [0][xx].leftBottomZ		= dst_zone->cells [0][xx].leftBottomZ;
			dst_zone->cells [0][xx].libPart.fillersp.f_leng		= src_zone->cells [0][xx].libPart.fillersp.f_leng;
			dst_zone->cells [0][xx].libPart.fillersp.f_thk		= src_zone->cells [0][xx].libPart.fillersp.f_thk;

		// 합판인 경우,
		} else if (src_zone->cells [0][xx].objType == PLYWOOD) {

			dst_zone->cells [0][xx].objType			= PLYWOOD;
			dst_zone->cells [0][xx].horLen			= src_zone->cells [0][xx].horLen;
			dst_zone->cells [0][xx].verLen			= src_zone->cells [0][xx].verLen;
			dst_zone->cells [0][xx].ang				= dst_zone->ang + degreeToRad (180);
			dst_zone->cells [0][xx].leftBottomX		= dst_zone->leftBottomX + ((getCellPositionLeftBottomX (src_zone, xx) + src_zone->cells [0][xx].horLen) * cos(src_zone->ang));
			dst_zone->cells [0][xx].leftBottomY		= dst_zone->leftBottomY + ((getCellPositionLeftBottomX (src_zone, xx) + src_zone->cells [0][xx].horLen) * sin(src_zone->ang));
			dst_zone->cells [0][xx].leftBottomZ		= dst_zone->cells [0][xx].leftBottomZ;
			dst_zone->cells [0][xx].libPart.plywood.p_leng		= src_zone->cells [0][xx].libPart.plywood.p_leng;
			dst_zone->cells [0][xx].libPart.plywood.p_wid		= src_zone->cells [0][xx].libPart.plywood.p_wid;
			dst_zone->cells [0][xx].libPart.plywood.w_dir_wall	= src_zone->cells [0][xx].libPart.plywood.w_dir_wall;
		}
	}
}

// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	alignPlacingZone (PlacingZone* target_zone)
{
	short			xx;

	// 각 Cell마다 위치 및 각도 정보가 업데이트됨
	for (xx = 0 ; xx < target_zone->nCells ; ++xx) {

		// 아무것도 없으면,
		if (target_zone->cells [0][xx].objType == NONE) {

			target_zone->cells [0][xx].leftBottomX	= target_zone->leftBottomX + (getCellPositionLeftBottomX (target_zone, xx) * cos(target_zone->ang));
			target_zone->cells [0][xx].leftBottomY	= target_zone->leftBottomX + (getCellPositionLeftBottomX (target_zone, xx) * sin(target_zone->ang));
			target_zone->cells [0][xx].leftBottomZ	= target_zone->leftBottomZ;

		// 인코너인 경우,
		} else if (target_zone->cells [0][xx].objType == INCORNER) {

			if (xx == 0) {
				target_zone->cells [0][xx].ang			= target_zone->ang + degreeToRad (-90);
				target_zone->cells [0][xx].leftBottomX	= target_zone->leftBottomX;
				target_zone->cells [0][xx].leftBottomY	= target_zone->leftBottomY;
				target_zone->cells [0][xx].leftBottomZ	= target_zone->leftBottomZ;
			} else {
				target_zone->cells [0][xx].ang			= target_zone->ang + degreeToRad (-180);
				target_zone->cells [0][xx].leftBottomX	= target_zone->leftBottomX + ((getCellPositionLeftBottomX (target_zone, xx) + target_zone->lenRIncorner) * cos(target_zone->ang));
				target_zone->cells [0][xx].leftBottomY	= target_zone->leftBottomY + ((getCellPositionLeftBottomX (target_zone, xx) + target_zone->lenRIncorner) * sin(target_zone->ang));
				target_zone->cells [0][xx].leftBottomZ	= target_zone->leftBottomZ;
			}

		// 유로폼인 경우,
		} else if (target_zone->cells [0][xx].objType == EUROFORM) {

			target_zone->cells [0][xx].ang			= target_zone->ang;

			// 벽세우기
			if (target_zone->cells [0][xx].libPart.form.u_ins_wall == true) {

				target_zone->cells [0][xx].leftBottomX	= target_zone->leftBottomX + ((getCellPositionLeftBottomX (target_zone, xx)) * cos(target_zone->ang));;
				target_zone->cells [0][xx].leftBottomY	= target_zone->leftBottomY + ((getCellPositionLeftBottomX (target_zone, xx)) * sin(target_zone->ang));;

			// 벽눕히기
			} else {

				target_zone->cells [0][xx].leftBottomX	= target_zone->leftBottomX + ((getCellPositionLeftBottomX (target_zone, xx) + target_zone->cells [0][xx].horLen) * cos(target_zone->ang));;
				target_zone->cells [0][xx].leftBottomY	= target_zone->leftBottomY + ((getCellPositionLeftBottomX (target_zone, xx) + target_zone->cells [0][xx].horLen) * sin(target_zone->ang));;

			}
			target_zone->cells [0][xx].leftBottomZ		=	target_zone->leftBottomZ;

		// 휠러스페이서인 경우,
		} else if (target_zone->cells [0][xx].objType == FILLERSPACER) {

			target_zone->cells [0][xx].ang			= target_zone->ang;
			target_zone->cells [0][xx].leftBottomX	= target_zone->leftBottomX + ((getCellPositionLeftBottomX (target_zone, xx) + target_zone->cells [0][xx].horLen) * cos(target_zone->ang));;
			target_zone->cells [0][xx].leftBottomY	= target_zone->leftBottomY + ((getCellPositionLeftBottomX (target_zone, xx) + target_zone->cells [0][xx].horLen) * sin(target_zone->ang));;
			target_zone->cells [0][xx].leftBottomZ	= target_zone->leftBottomZ;

		// 합판인 경우,
		} else if (target_zone->cells [0][xx].objType == PLYWOOD) {

			target_zone->cells [0][xx].ang			= target_zone->ang;
			target_zone->cells [0][xx].leftBottomX	= target_zone->leftBottomX + ((getCellPositionLeftBottomX (target_zone, xx)) * cos(target_zone->ang));;
			target_zone->cells [0][xx].leftBottomY	= target_zone->leftBottomY + ((getCellPositionLeftBottomX (target_zone, xx)) * sin(target_zone->ang));;
			target_zone->cells [0][xx].leftBottomZ	= target_zone->leftBottomZ;
		}
	}

	// 영역 정보에서 남은 거리 관련 항목들이 업데이트됨
	target_zone->remain_hor_updated = target_zone->remain_hor = target_zone->horLen;

	// 가로 방향 남은 길이: 각 셀의 너비만큼 차감
	for (xx = 0 ; xx < target_zone->nCells ; ++xx) {
		if (target_zone->cells [0][xx].objType != NONE) {
			target_zone->remain_hor_updated -= target_zone->cells [0][xx].horLen;
		}
	}
}

// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK placerHandlerPrimary (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "유로폼/인코너/기타 배치 - 기본 배치");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "배  치");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취  소");

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

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER, 1);

			break;

		case DG_MSG_CHANGE:
			// 체크박스 제어 (인코너 관련)
			if (DGGetItemValLong (dialogID, CHECKBOX_SET_LEFT_INCORNER) == 1)
				DGEnableItem (dialogID, EDITCONTROL_LEFT_INCORNER);
			else
				DGDisableItem (dialogID, EDITCONTROL_LEFT_INCORNER);

			if (DGGetItemValLong (dialogID, CHECKBOX_SET_RIGHT_INCORNER) == 1)
				DGEnableItem (dialogID, EDITCONTROL_RIGHT_INCORNER);
			else
				DGDisableItem (dialogID, EDITCONTROL_RIGHT_INCORNER);

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

					// 레이어 번호 저장
					layerInd = (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER);

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
short DGCALLBACK placerHandlerSecondary (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			DGSetDialogTitle (dialogID, "유로폼/인코너/기타 배치 - 가로 채우기");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 업데이트 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 100, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "배  치");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 140, 100, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "자투리 채우기");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// 아이템 배치 (인코너 관련)
			// 라벨: 남은 가로 길이
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 15, 90, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, "남은 가로 길이");
			DGShowItem (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH);

			// Edit 컨트롤: 남은 가로 길이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 110, 15-7, 50, 25);
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
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 60, 100, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, "남은 길이 확인");
			DGShowItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);

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

					idxCell = ((idxBtn - itemInitIdx) * 2) - (xx * (placingZone.eu_count_hor + 2)) * 2;		// 버튼 인덱스로 셀 인덱스를 구함

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
					alignPlacingZone (&placingZone);
					copyPlacingZoneSymmetric (&placingZone, &placingZoneBackside, &infoWall);

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

					break;

				case DG_OK:
					// 종료하지 않고 배치된 객체를 수정 및 재배치하고 그리드 버튼 속성을 변경함
					item = 0;

					// 셀 정보(타입 및 크기) 변경 발생, 모든 셀의 위치 값을 업데이트
					alignPlacingZone (&placingZone);
					copyPlacingZoneSymmetric (&placingZone, &placingZoneBackside, &infoWall);

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

					err = ACAPI_CallUndoableCommand ("유로폼/인코너 재배치", [&] () -> GSErrCode {
						// 기존 배치된 객체 전부 삭제
						for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
							for (yy = 0 ; yy < placingZone.nCells ; ++ yy) {
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
							for (yy = 0 ; yy < placingZoneBackside.nCells ; ++ yy) {
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
						for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
							for (yy = 0 ; yy < placingZone.nCells ; ++yy)
								// cells 객체의 objType이 NONE이 아니면 연속으로 배치할 것
								placingZone.cells [xx][yy].guid = placeLibPart (placingZone.cells [0][yy]);

							// Cell 정보 업데이트: 다음 높이로 상승시킴
							if (placingZone.eu_ori.compare (std::string ("벽세우기")) == 0)
								setCellPositionLeftBottomZ (&placingZone, placingZone.leftBottomZ + (placingZone.eu_hei_numeric * (xx + 1)));
							else
								setCellPositionLeftBottomZ (&placingZone, placingZone.leftBottomZ + (placingZone.eu_wid_numeric * (xx + 1)));
						}
						setCellPositionLeftBottomZ (&placingZone, placingZone.leftBottomZ);		// Cell 정보 업데이트: 높이 초기화

						//////////////////////////////////////////////////////////// 벽 뒤쪽
						for (xx = 0 ; xx < placingZoneBackside.eu_count_ver ; ++xx) {
							for (yy = 0 ; yy < placingZoneBackside.nCells ; ++yy)
								// cells 객체의 objType이 NONE이 아니면 연속으로 배치할 것
								placingZoneBackside.cells [xx][yy].guid = placeLibPart (placingZoneBackside.cells [0][yy]);

							// Cell 정보 업데이트: 다음 높이로 상승시킴
							if (placingZoneBackside.eu_ori.compare (std::string ("벽세우기")) == 0)
								setCellPositionLeftBottomZ (&placingZoneBackside, placingZoneBackside.leftBottomZ + (placingZoneBackside.eu_hei_numeric * (xx + 1)));
							else
								setCellPositionLeftBottomZ (&placingZoneBackside, placingZoneBackside.leftBottomZ + (placingZoneBackside.eu_wid_numeric * (xx + 1)));
						}
						setCellPositionLeftBottomZ (&placingZoneBackside, placingZoneBackside.leftBottomZ);		// Cell 정보 업데이트: 높이 초기화

						return err;
					});

					break;
				case DG_CANCEL:

					break;

				default:
					// [DIALOG] 그리드 버튼을 누르면 Cell을 설정하기 위한 작은 창이 나옴
					clickedBtnItemIdx = item;
					result = DGBlankModalDialog (240*3, 260, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, placerHandlerThird, 0);

					item = 0;	// 그리드 버튼을 눌렀을 때 창이 닫히지 않게 함

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
short DGCALLBACK placerHandlerThird (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	short	idxCell;
	short	idxCell_prev = -1, idxCell_next = -1;
	short	popupSelectedIdx = 0;
	double	temp;

	switch (message) {
		case DG_MSG_INIT:

			// placerHandlerSecondary 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
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
			DGShowItem (dialogID, POPUP_OBJ_TYPE);

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, "너비");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, "높이");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION, "설치방향");
				
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100+240, 110-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100+240, 140-6, 70, 25);
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
			if (idxCell_prev != -1)	DGShowItem (dialogID, POPUP_OBJ_TYPE_PREV);

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH_PREV, "너비");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+0, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH_PREV, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT_PREV, "높이");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+0, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+0, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION_PREV, "설치방향");
				
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 779, 100+0, 110-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 779, 100+0, 140-6, 70, 25);
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
			if (idxCell_next != -1)	DGShowItem (dialogID, POPUP_OBJ_TYPE_NEXT);

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH_NEXT, "너비");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+480, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH_NEXT, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT_NEXT, "높이");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+480, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+480, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION_NEXT, "설치방향");
				
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 781, 100+480, 110-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 781, 100+480, 140-6, 70, 25);
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
					}

					break;

				case POPUP_OBJ_TYPE_PREV:	// 객체 타입 콤보박스 값을 변경할 때마다 입력 필드가 달라짐 (변경해야 하므로 Cell 값을 불러오지 않음)
					//////////////////////////////////////////////////////////// 필드 생성 (예전 셀)
					// 일단 항목을 숨기고, 객체 타입 관련 항목만 표시함
					DGHideItem (dialogID, LABEL_WIDTH_PREV);
					DGHideItem (dialogID, EDITCONTROL_WIDTH_PREV);
					DGHideItem (dialogID, LABEL_HEIGHT_PREV);
					DGHideItem (dialogID, EDITCONTROL_HEIGHT_PREV);
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
					}

					break;

				case POPUP_OBJ_TYPE_NEXT:	// 객체 타입 콤보박스 값을 변경할 때마다 입력 필드가 달라짐 (변경해야 하므로 Cell 값을 불러오지 않음)
					//////////////////////////////////////////////////////////// 필드 생성 (다음 셀)
					// 일단 항목을 숨기고, 객체 타입 관련 항목만 표시함
					DGHideItem (dialogID, LABEL_WIDTH_NEXT);
					DGHideItem (dialogID, EDITCONTROL_WIDTH_NEXT);
					DGHideItem (dialogID, LABEL_HEIGHT_NEXT);
					DGHideItem (dialogID, EDITCONTROL_HEIGHT_NEXT);
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

					// placerHandlerSecondary 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
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

					//////////////////////////////////////////////////////////// 필드 생성 (클릭한 셀)
					// 입력한 값을 다시 셀에 저장
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
						placingZone.cells [0][idxCell].objType = NONE;

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == INCORNER + 1) {
						placingZone.cells [0][idxCell].objType = INCORNER;

						// 너비
						if (idxCell == 0) {
							placingZone.cells [0][idxCell].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
							placingZone.cells [0][idxCell].libPart.incorner.wid_s = 0.100;
						} else if (idxCell > 0) {
							placingZone.cells [0][idxCell].libPart.incorner.leng_s = 0.100;
							placingZone.cells [0][idxCell].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
						}
						placingZone.cells [0][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

						// 높이
						placingZone.cells [0][idxCell].libPart.incorner.hei_s = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
						placingZone.cells [0][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
						placingZone.cells [0][idxCell].objType = EUROFORM;

						// 규격폼
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
							placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff = true;
						else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE)
							placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff = false;

						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
							// 너비
							placingZone.cells [0][idxCell].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS)).ToCStr ()) / 1000.0;
							placingZone.cells [0][idxCell].horLen = placingZone.cells [0][idxCell].libPart.form.eu_wid;
							// 높이
							placingZone.cells [0][idxCell].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS)).ToCStr ()) / 1000.0;
							placingZone.cells [0][idxCell].verLen = placingZone.cells [0][idxCell].libPart.form.eu_hei;
						} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
							// 너비
							placingZone.cells [0][idxCell].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
							placingZone.cells [0][idxCell].horLen = placingZone.cells [0][idxCell].libPart.form.eu_wid2;
							// 높이
							placingZone.cells [0][idxCell].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
							placingZone.cells [0][idxCell].verLen = placingZone.cells [0][idxCell].libPart.form.eu_hei2;
						}

						// 설치방향
						if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == TRUE)
							placingZone.cells [0][idxCell].libPart.form.u_ins_wall = true;
						else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == FALSE) {
							placingZone.cells [0][idxCell].libPart.form.u_ins_wall = false;
							// 가로, 세로 길이 교환
							temp = placingZone.cells [0][idxCell].horLen;
							placingZone.cells [0][idxCell].horLen = placingZone.cells [0][idxCell].verLen;
							placingZone.cells [0][idxCell].verLen = temp;
						}
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == FILLERSPACER + 1) {
						placingZone.cells [0][idxCell].objType = FILLERSPACER;

						// 너비
						placingZone.cells [0][idxCell].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
						placingZone.cells [0][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

						// 높이
						placingZone.cells [0][idxCell].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
						placingZone.cells [0][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PLYWOOD + 1) {
						placingZone.cells [0][idxCell].objType = PLYWOOD;

						// 너비
						placingZone.cells [0][idxCell].libPart.plywood.p_wid = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
						placingZone.cells [0][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

						// 높이
						placingZone.cells [0][idxCell].libPart.plywood.p_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
						placingZone.cells [0][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);

						// 설치방향
						if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD) == TRUE)
							placingZone.cells [0][idxCell].libPart.plywood.w_dir_wall = true;
						else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD) == FALSE) {
							placingZone.cells [0][idxCell].libPart.plywood.w_dir_wall = false;
							// 가로, 세로 길이 교환
							temp = placingZone.cells [0][idxCell].horLen;
							placingZone.cells [0][idxCell].horLen = placingZone.cells [0][idxCell].verLen;
							placingZone.cells [0][idxCell].verLen = temp;
						}
					}

					//////////////////////////////////////////////////////////// 필드 생성 (예전 셀)
					if (idxCell_prev != -1) {
						// 입력한 값을 다시 셀에 저장
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == NONE + 1) {
							placingZone.cells [0][idxCell_prev].objType = NONE;

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == INCORNER + 1) {
							placingZone.cells [0][idxCell_prev].objType = INCORNER;

							// 너비
							if (idxCell_prev == 0) {
								placingZone.cells [0][idxCell_prev].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);
								placingZone.cells [0][idxCell_prev].libPart.incorner.wid_s = 0.100;
							} else if (idxCell_prev > 0) {
								placingZone.cells [0][idxCell_prev].libPart.incorner.leng_s = 0.100;
								placingZone.cells [0][idxCell_prev].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);
							}
							placingZone.cells [0][idxCell_prev].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);

							// 높이
							placingZone.cells [0][idxCell_prev].libPart.incorner.hei_s = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
							placingZone.cells [0][idxCell_prev].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == EUROFORM + 1) {
							placingZone.cells [0][idxCell_prev].objType = EUROFORM;

							// 규격폼
							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == TRUE)
								placingZone.cells [0][idxCell_prev].libPart.form.eu_stan_onoff = true;
							else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == FALSE)
								placingZone.cells [0][idxCell_prev].libPart.form.eu_stan_onoff = false;

							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == TRUE) {
								// 너비
								placingZone.cells [0][idxCell_prev].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_PREV)).ToCStr ()) / 1000.0;
								placingZone.cells [0][idxCell_prev].horLen = placingZone.cells [0][idxCell_prev].libPart.form.eu_wid;
								// 높이
								placingZone.cells [0][idxCell_prev].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_PREV)).ToCStr ()) / 1000.0;
								placingZone.cells [0][idxCell_prev].verLen = placingZone.cells [0][idxCell_prev].libPart.form.eu_hei;
							} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_PREV) == FALSE) {
								// 너비
								placingZone.cells [0][idxCell_prev].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV);
								placingZone.cells [0][idxCell_prev].horLen = placingZone.cells [0][idxCell_prev].libPart.form.eu_wid2;
								// 높이
								placingZone.cells [0][idxCell_prev].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV);
								placingZone.cells [0][idxCell_prev].verLen = placingZone.cells [0][idxCell_prev].libPart.form.eu_hei2;
							}

							// 설치방향
							if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV) == TRUE)
								placingZone.cells [0][idxCell_prev].libPart.form.u_ins_wall = true;
							else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_PREV) == FALSE) {
								placingZone.cells [0][idxCell_prev].libPart.form.u_ins_wall = false;
								// 가로, 세로 길이 교환
								temp = placingZone.cells [0][idxCell_prev].horLen;
								placingZone.cells [0][idxCell_prev].horLen = placingZone.cells [0][idxCell_prev].verLen;
								placingZone.cells [0][idxCell_prev].verLen = temp;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == FILLERSPACER + 1) {
							placingZone.cells [0][idxCell_prev].objType = FILLERSPACER;

							// 너비
							placingZone.cells [0][idxCell_prev].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);
							placingZone.cells [0][idxCell_prev].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);

							// 높이
							placingZone.cells [0][idxCell_prev].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
							placingZone.cells [0][idxCell_prev].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_PREV) == PLYWOOD + 1) {
							placingZone.cells [0][idxCell_prev].objType = PLYWOOD;

							// 너비
							placingZone.cells [0][idxCell_prev].libPart.plywood.p_wid = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);
							placingZone.cells [0][idxCell_prev].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_PREV);

							// 높이
							placingZone.cells [0][idxCell_prev].libPart.plywood.p_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);
							placingZone.cells [0][idxCell_prev].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_PREV);

							// 설치방향
							if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV) == TRUE)
								placingZone.cells [0][idxCell_prev].libPart.plywood.w_dir_wall = true;
							else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_PREV) == FALSE) {
								placingZone.cells [0][idxCell_prev].libPart.plywood.w_dir_wall = false;
								// 가로, 세로 길이 교환
								temp = placingZone.cells [0][idxCell_prev].horLen;
								placingZone.cells [0][idxCell_prev].horLen = placingZone.cells [0][idxCell_prev].verLen;
								placingZone.cells [0][idxCell_prev].verLen = temp;
							}
						}
					}

					//////////////////////////////////////////////////////////// 필드 생성 (다음 셀)
					if (idxCell_next != -1) {
						// 입력한 값을 다시 셀에 저장
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == NONE + 1) {
							placingZone.cells [0][idxCell_next].objType = NONE;

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == INCORNER + 1) {
							placingZone.cells [0][idxCell_next].objType = INCORNER;

							// 너비
							if (idxCell_next == 0) {
								placingZone.cells [0][idxCell_next].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);
								placingZone.cells [0][idxCell_next].libPart.incorner.wid_s = 0.100;
							} else if (idxCell_next > 0) {
								placingZone.cells [0][idxCell_next].libPart.incorner.leng_s = 0.100;
								placingZone.cells [0][idxCell_next].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);
							}
							placingZone.cells [0][idxCell_next].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);

							// 높이
							placingZone.cells [0][idxCell_next].libPart.incorner.hei_s = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
							placingZone.cells [0][idxCell_next].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == EUROFORM + 1) {
							placingZone.cells [0][idxCell_next].objType = EUROFORM;

							// 규격폼
							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT) == TRUE)
								placingZone.cells [0][idxCell_next].libPart.form.eu_stan_onoff = true;
							else
								placingZone.cells [0][idxCell_next].libPart.form.eu_stan_onoff = false;

							if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD_NEXT) == TRUE) {
								// 너비
								placingZone.cells [0][idxCell_next].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS_NEXT)).ToCStr ()) / 1000.0;
								placingZone.cells [0][idxCell_next].horLen = placingZone.cells [0][idxCell_next].libPart.form.eu_wid;
								// 높이
								placingZone.cells [0][idxCell_next].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT)).ToCStr ()) / 1000.0;
								placingZone.cells [0][idxCell_next].verLen = placingZone.cells [0][idxCell_next].libPart.form.eu_hei;
							} else {
								// 너비
								placingZone.cells [0][idxCell_next].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT);
								placingZone.cells [0][idxCell_next].horLen = placingZone.cells [0][idxCell_next].libPart.form.eu_wid2;
								// 높이
								placingZone.cells [0][idxCell_next].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT);
								placingZone.cells [0][idxCell_next].verLen = placingZone.cells [0][idxCell_next].libPart.form.eu_hei2;
							}

							// 설치방향
							if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT) == TRUE)
								placingZone.cells [0][idxCell_next].libPart.form.u_ins_wall = true;
							else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM_NEXT) == FALSE) {
								placingZone.cells [0][idxCell_next].libPart.form.u_ins_wall = false;
								// 가로, 세로 길이 교환
								temp = placingZone.cells [0][idxCell_next].horLen;
								placingZone.cells [0][idxCell_next].horLen = placingZone.cells [0][idxCell_next].verLen;
								placingZone.cells [0][idxCell_next].verLen = temp;
							}
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == FILLERSPACER + 1) {
							placingZone.cells [0][idxCell_next].objType = FILLERSPACER;

							// 너비
							placingZone.cells [0][idxCell_next].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);
							placingZone.cells [0][idxCell_next].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);

							// 높이
							placingZone.cells [0][idxCell_next].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
							placingZone.cells [0][idxCell_next].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE_NEXT) == PLYWOOD + 1) {
							placingZone.cells [0][idxCell_next].objType = PLYWOOD;

							// 너비
							placingZone.cells [0][idxCell_next].libPart.plywood.p_wid = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);
							placingZone.cells [0][idxCell_next].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH_NEXT);

							// 높이
							placingZone.cells [0][idxCell_next].libPart.plywood.p_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);
							placingZone.cells [0][idxCell_next].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT_NEXT);

							// 설치방향
							if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT) == TRUE)
								placingZone.cells [0][idxCell_next].libPart.plywood.w_dir_wall = true;
							else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD_NEXT) == FALSE){
								placingZone.cells [0][idxCell_next].libPart.plywood.w_dir_wall = false;
								// 가로, 세로 길이 교환
								temp = placingZone.cells [0][idxCell_next].horLen;
								placingZone.cells [0][idxCell_next].horLen = placingZone.cells [0][idxCell_next].verLen;
								placingZone.cells [0][idxCell_next].verLen = temp;
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