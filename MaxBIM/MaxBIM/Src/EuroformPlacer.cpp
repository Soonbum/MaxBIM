#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"

static PlacingZone		placingZone;			// 기본 벽면 영역 정보
static PlacingZone		placingZoneBackside;	// 반대쪽 벽면에도 벽면 영역 정보 부여, 벽 기준으로 대칭됨 (placingZone과 달리 오른쪽부터 객체를 설치함)
static InfoWall			infoWall;				// 벽 객체 정보
static short			clickedBtnItemIdx;		// 그리드 버튼에서 클릭한 버튼의 인덱스 번호를 저장
static short			itemInitIdx = GRIDBUTTON_IDX_START;		// 그리드 버튼 항목 인덱스 시작번호
static short			layerInd;				// 객체를 배치할 레이어 인덱스


// 1번 메뉴: 유로폼/인코너 등을 배치하는 통합 루틴
GSErrCode	placeEuroformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx, yy;
	long		result_compare_x = 0, result_compare_y = 0;
	double		dx, dy;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	walls = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	long					nWalls = 0;
	long					nMorphs = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElemInfo3D			info3D;

	// 모프 객체 정보
	InfoMorph				infoMorph [31];

	// 영역 정보
	long					indWall = -1;			// 벽면 모프의 인덱스
	short					nInterfereBeams = 0;	// 간섭보의 개수
	short					nBacksideColumns = 0;	// 후면기둥의 개수
	short					iInterfereBeams;
	short					iBacksideColumns;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			minusLevel;

	//////////////////////////////////////////////////////////// Morph를 이용하여 영역 추출
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);	// 선택한 요소 가져오기
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.", true);
		return err;
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 모프 객체만 수집함
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
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nWalls = walls.GetSize ();
	nMorphs = morphs.GetSize ();

	// 벽이 1개인가?
	if (nWalls != 1) {
		ACAPI_WriteReport ("벽은 1개를 선택해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개 이상인가?
	if (nMorphs < 1) {
		ACAPI_WriteReport ("모프는 1개 이상 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 벽 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = walls.Pop ();
	err = ACAPI_Element_Get (&elem);

	if (elem.wall.thickness != elem.wall.thickness1) {
		ACAPI_WriteReport ("벽의 두께는 균일해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}
	infoWall.wallThk		= elem.wall.thickness;
	infoWall.bottomOffset	= elem.wall.bottomOffset;
	infoWall.floorInd		= elem.header.floorInd;
	infoWall.begX			= elem.wall.begC.x;
	infoWall.begY			= elem.wall.begC.y;
	infoWall.endX			= elem.wall.endC.x;
	infoWall.endY			= elem.wall.endC.y;

	// 모프 정보를 가져옴
	for (xx = 0 ; xx < nMorphs ; ++xx) {
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
		infoMorph [xx].guid = elem.header.guid;

		// 모프의 좌하단, 우상단 좌표 저장	!!! direction에 따라 순서를 바꾸어야 할 수도 있다. findDirection 함수 이용
		infoMorph [xx].leftBottomX = info3D.bounds.xMin;
		infoMorph [xx].leftBottomY = info3D.bounds.yMin;
		infoMorph [xx].leftBottomZ = info3D.bounds.zMin;
		infoMorph [xx].rightTopX = info3D.bounds.xMax;
		infoMorph [xx].rightTopY = info3D.bounds.yMax;
		infoMorph [xx].rightTopZ = info3D.bounds.zMax;

		// 모프의 가로 길이
		infoMorph [xx].horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);

		// 모프의 세로 길이
		infoMorph [xx].verLen = info3D.bounds.zMax - info3D.bounds.zMin;

		// 모프의 Z축 회전 각도
		dx = infoWall.endX - infoWall.begX;
		dy = infoWall.endY - infoWall.begY;
		infoMorph [xx].ang = RadToDegree (atan2 (dy, dx));

		// !!! 각도, 벽과의 관계가 매번 달라짐 (방향에 따라 맞춰야 함)
		/*
		char msg [200];
		sprintf (msg, "wall direction: %ld\nmorph direction: %ld, angle: %.0f\nlevel: %.4f\n[%.4f, %.4f, %.4f, %.4f]\n[%.4f, %.4f, %.4f, %.4f]\n[%.4f, %.4f, %.4f, %.4f]",
			findDirection (infoWall.begX, infoWall.begY, infoWall.endX, infoWall.endY),
			findDirection (infoMorph [xx].leftBottomX, infoMorph [xx].leftBottomY, infoMorph [xx].rightTopX, infoMorph [xx].rightTopY), infoMorph [xx].ang,
			elem.morph.level, elem.morph.tranmat.tmx [0], elem.morph.tranmat.tmx [1], elem.morph.tranmat.tmx [2], elem.morph.tranmat.tmx [3],
			elem.morph.tranmat.tmx [4], elem.morph.tranmat.tmx [5], elem.morph.tranmat.tmx [6], elem.morph.tranmat.tmx [7],
			elem.morph.tranmat.tmx [8], elem.morph.tranmat.tmx [9], elem.morph.tranmat.tmx [10], elem.morph.tranmat.tmx [11]);
		ACAPI_WriteReport (msg, true);
		sprintf (msg, "X(%.4f, %.4f)\nY(%.4f, %.4f)\nZ(%.4f, %.4f)", info3D.bounds.xMin, info3D.bounds.xMax, info3D.bounds.yMin, info3D.bounds.yMax, info3D.bounds.zMin, info3D.bounds.zMax);
		ACAPI_WriteReport (msg, true);
		*/
	}

	// 벽면/간섭보/후면기둥 모프 판정 (Assertion 시행)
	if (nMorphs == 1) {
		// 모프가 1개만 있으면 무조건 벽면
		indWall = 0;

		placingZone.leftBottomX		= infoMorph [0].leftBottomX;
		placingZone.leftBottomY		= infoMorph [0].leftBottomY;
		placingZone.leftBottomZ		= infoMorph [0].leftBottomZ;
		placingZone.horLen			= infoMorph [0].horLen;
		placingZone.verLen			= infoMorph [0].verLen;
		placingZone.ang				= degreeToRad (infoMorph [0].ang);
		placingZone.nInterfereBeams	= 0;
		placingZone.nBacksideColumn	= 0;

	} else {
		// 모프가 2개 이상이면,

		nInterfereBeams = 0;
		nBacksideColumns = 0;

		// 벽면, 간섭보, 후면기둥의 인덱스 번호 검색
		for (xx = 0 ; xx < nMorphs ; ++xx) {
			for (yy = 0 ; yy < nMorphs ; ++yy) {
				if (xx == yy) continue;

				// 가로축 범위 비교
				result_compare_x = compareRanges (	infoMorph [xx].leftBottomX, infoMorph [xx].leftBottomX + infoMorph [xx].horLen,
													infoMorph [yy].leftBottomX, infoMorph [yy].leftBottomX + infoMorph [yy].horLen );
				// 세로축 범위 비교
				result_compare_y = compareRanges (	infoMorph [xx].leftBottomZ, infoMorph [xx].leftBottomZ + infoMorph [xx].verLen,
													infoMorph [yy].leftBottomZ, infoMorph [yy].leftBottomZ + infoMorph [yy].verLen );

				// yy: xx의 간섭보
				if ( (result_compare_x == 5) && (result_compare_y == 5) ) {
					indWall = xx;
					++nInterfereBeams;		// 간섭보 카운트
				}

				// yy: xx의 후면기둥
				if ( (result_compare_x == 5) && (result_compare_y == 12) ) {
					indWall = xx;
					++nBacksideColumns;		// 후면기둥 카운트
				}
			}
		}

		placingZone.leftBottomX	= infoMorph [indWall].leftBottomX;
		placingZone.leftBottomY	= infoMorph [indWall].leftBottomY;
		placingZone.leftBottomZ	= infoMorph [indWall].leftBottomZ;
		placingZone.horLen		= infoMorph [indWall].horLen;
		placingZone.verLen		= infoMorph [indWall].verLen;
		placingZone.ang			= degreeToRad (infoMorph [indWall].ang);
		placingZone.nInterfereBeams	= nInterfereBeams;
		placingZone.nBacksideColumn	= nBacksideColumns;

		iInterfereBeams = 0;
		iBacksideColumns = 0;

		// 벽면보다 작은 영역 (간섭보), 세로는 동일하나 가로만 벽면보다 작은 영역 (후면기둥)
		for (xx = 0 ; xx < nMorphs ; ++xx) {
			if (xx == indWall) continue;

			// 가로축 범위 비교
			result_compare_x = compareRanges (	infoMorph [indWall].leftBottomX,	infoMorph [indWall].leftBottomX + infoMorph [indWall].horLen,
												infoMorph [xx].leftBottomX,			infoMorph [xx].leftBottomX + infoMorph [xx].horLen				);

			// 세로축 범위 비교
			result_compare_x = compareRanges (	infoMorph [indWall].leftBottomZ,	infoMorph [indWall].leftBottomZ + infoMorph [indWall].verLen,
												infoMorph [xx].leftBottomZ,			infoMorph [xx].leftBottomZ + infoMorph [xx].verLen				);

			// 간섭보
			if ( (result_compare_x == 5) && (result_compare_y == 5) ) {
				placingZone.beams [iInterfereBeams].leftBottomX = infoMorph [xx].leftBottomX;
				placingZone.beams [iInterfereBeams].leftBottomZ = infoMorph [xx].leftBottomZ;
				placingZone.beams [iInterfereBeams].horLen = infoMorph [xx].horLen;
				placingZone.beams [iInterfereBeams].verLen = infoMorph [xx].verLen;

				++iInterfereBeams;
			}

			// 후면기둥
			if ( (result_compare_x == 5) && (result_compare_y == 12) ) {
				placingZone.columns [iBacksideColumns].leftBottomX = infoMorph [xx].leftBottomX;
				placingZone.columns [iBacksideColumns].leftBottomZ = infoMorph [xx].leftBottomZ;
				placingZone.columns [iBacksideColumns].horLen = infoMorph [xx].horLen;
				placingZone.columns [iBacksideColumns].verLen = infoMorph [xx].verLen;

				++iBacksideColumns;
			}
		}
	}

	// 벽을 찾지 못하면 종료
	if (indWall == -1) {
		ACAPI_WriteReport ("모프 영역을 제대로 그리지 않았습니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 작업 층 높이만큼 낮춤
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
	for (xx = 0 ; xx < placingZone.nBacksideColumn ; ++xx)	placingZone.columns [xx].leftBottomZ -= minusLevel;

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

				tElem.header.guid = (*selNeigs)[xx].guid;
				if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
					continue;

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
	err = ACAPI_CallUndoableCommand ("유로폼/인코너 1차 배치", [&] () -> GSErrCode {

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

	// 세로 영역 채우기 ???

	return	err;
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
	short xx;

	for (xx = 0 ; xx < placingZone->nCells ; ++xx)
		placingZone->cells [0][xx].objType = NONE;
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

	dst_zone->nBacksideColumn		= 0;
	dst_zone->nInterfereBeams		= 0;

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
			dst_zone->cells [0][xx].leftBottomZ					= src_zone->cells [0][xx].leftBottomZ;
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
	short	idxCell;
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
			DGSetItemText (dialogID, DG_CANCEL, "종  료");
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
			groupboxSizeX = 40 + (btnSizeX * placingZone.nCells);
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
			dialogSizeX = 270 + (btnSizeX * placingZone.nCells);
			dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver));
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_CENTER, true);

			// 그리드 구조체에 따라서 버튼을 동적으로 배치함
			for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
				for (yy = 0 ; yy < placingZone.nCells ; ++yy) {
					idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, idxBtn, DG_IS_EXTRASMALL);

					idxCell = (idxBtn - itemInitIdx) - (xx * placingZone.nCells);		// 버튼 인덱스로 셀 인덱스를 구함

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

					// 그리드 버튼 텍스트 업데이트
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.nCells ; ++yy) {

							// 셀 인덱스로 버튼 인덱스를 구함
							idxBtn = yy + (xx * placingZone.nCells) + itemInitIdx;

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

					// 그리드 버튼 텍스트 업데이트
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.nCells ; ++yy) {

							// 셀 인덱스로 버튼 인덱스를 구함
							idxBtn = yy + (xx * placingZone.nCells) + itemInitIdx;

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
						}
					}

					// 남은 가로 길이 업데이트
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated);

					// 기존 배치된 객체 전부 삭제
					err = ACAPI_CallUndoableCommand ("예전에 배치된 객체 제거", [&] () -> GSErrCode {
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

						return err;
					});

					// 업데이트된 셀 정보대로 객체 재배치
					err = ACAPI_CallUndoableCommand ("유로폼/인코너 재배치", [&] () -> GSErrCode {

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
					result = DGBlankModalDialog (240, 260, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, placerHandlerThird, 0);

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
	long	idxCell;
	short	popupSelectedIdx = 0;
	double	temp;

	switch (message) {
		case DG_MSG_INIT:
			// placerHandlerSecondary 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
			idxCell = clickedBtnItemIdx - itemInitIdx;
			while (idxCell >= placingZone.nCells)
				idxCell -= placingZone.nCells;

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

			// 라벨: 객체 타입
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, "객체 타입");
			DGShowItem (dialogID, idxItem);

			// 팝업컨트롤: 객체 타입을 바꿀 수 있는 콤보박스가 맨 위에 나옴
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 20-7, 120, 25);
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

			//////////////////////////////////////////////////////////// 필드 생성
			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, "너비");

			// Edit 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, "높이");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION, "설치방향");
				
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 110-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_PLYWOOD, "벽눕히기");

			// 체크박스: 규격폼
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20, 50, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "규격폼");

			// 라벨: 너비
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, "너비");

			// 팝업 컨트롤: 너비
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 80-7, 100, 25);
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
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);

			// 라벨: 높이
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, "높이");

			// 팝업 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 110-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "600");

			// Edit 컨트롤: 높이
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 110-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			
			// 라벨: 설치방향
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, "설치방향");
			
			// 라디오 버튼: 설치방향 (벽세우기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_EUROFORM, "벽세우기");
			// 라디오 버튼: 설치방향 (벽눕히기)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 170-6, 70, 25);
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
				else
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

				if (placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff) {
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
				} else {
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

				if (placingZone.cells [0][idxCell].libPart.form.u_ins_wall) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, true);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, false);
				} else {
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

				if (placingZone.cells [0][idxCell].libPart.plywood.w_dir_wall) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD, true);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD, false);
				} else {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD, false);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD, true);
				}
			}

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case POPUP_OBJ_TYPE:	// 객체 타입 콤보박스 값을 변경할 때마다 입력 필드가 달라짐 (변경해야 하므로 Cell 값을 불러오지 않음)

					// placerHandlerSecondary 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
					idxCell = clickedBtnItemIdx - itemInitIdx;
					while (idxCell >= placingZone.nCells)
						idxCell -= placingZone.nCells;

					// 일단 항목을 숨기고, 객체 타입 관련 항목과 버튼만 표시함
					DGHideItem (dialogID, DG_ALL_ITEMS);
					DGShowItem (dialogID, DG_OK);
					DGShowItem (dialogID, DG_CANCEL);
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
						} else {
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

				case CHECKBOX_SET_STANDARD:	// 유로폼의 경우, 규격폼 체크박스 값을 바꿀 때마다 너비, 높이 입력 필드 타입이 바뀜
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
							// 팝업 컨트롤: 너비
							DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
							// 팝업 컨트롤: 높이
							DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
						} else {
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
			}

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// placerHandlerSecondary 에서 클릭한 그리드 버튼의 인덱스 값을 이용하여 셀 인덱스 값 로드
					idxCell = clickedBtnItemIdx - itemInitIdx;
					while (idxCell >= placingZone.nCells)
						idxCell -= placingZone.nCells;

					// 입력한 값을 다시 셀에 저장
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
						placingZone.cells [0][idxCell].objType = NONE;

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == INCORNER + 1) {
						placingZone.cells [0][idxCell].objType = INCORNER;

						// 너비
						if (idxCell == 0) {
							placingZone.cells [0][idxCell].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
							placingZone.cells [0][idxCell].libPart.incorner.wid_s = 0.100;
						} else {
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
						else
							placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff = false;

						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
							// 너비
							placingZone.cells [0][idxCell].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS)).ToCStr ()) / 1000.0;
							placingZone.cells [0][idxCell].horLen = placingZone.cells [0][idxCell].libPart.form.eu_wid;
							// 높이
							placingZone.cells [0][idxCell].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS)).ToCStr ()) / 1000.0;
							placingZone.cells [0][idxCell].verLen = placingZone.cells [0][idxCell].libPart.form.eu_hei;
						} else {
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
						else {
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
						else {
							placingZone.cells [0][idxCell].libPart.plywood.w_dir_wall = false;
							// 가로, 세로 길이 교환
							temp = placingZone.cells [0][idxCell].horLen;
							placingZone.cells [0][idxCell].horLen = placingZone.cells [0][idxCell].verLen;
							placingZone.cells [0][idxCell].verLen = temp;
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
