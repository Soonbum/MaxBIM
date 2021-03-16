#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "WallTableformPlacer.hpp"

using namespace wallTableformPlacerDG;

static WallTableformPlacingZone		placingZone;		// 기본 벽면 영역 정보
static InfoWallForWallTableform		infoWall;			// 벽 객체 정보

static short	layerInd_Euroform;		// 레이어 번호: 유로폼
static short	layerInd_RectPipe;		// 레이어 번호: 비계 파이프
static short	layerInd_PinBolt;		// 레이어 번호: 핀볼트 세트
static short	layerInd_WallTie;		// 레이어 번호: 빅체 타이
static short	layerInd_Clamp;			// 레이어 번호: 직교 클램프
static short	layerInd_HeadPiece;		// 레이어 번호: 헤드피스

// 다이얼로그 동적 요소 인덱스 번호 저장
static short	EDITCONTROL_REMAIN_WIDTH;
static short	POPUP_WIDTH [50];


// 벽에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx, yy;
	double		dx, dy, ang1, ang2;
	//double		xPosLB, yPosLB, zPosLB;
	//double		xPosRT, yPosRT, zPosRT;
	double		width;

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
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 객체 정보
	InfoMorphForWallTableform	infoMorph;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_wall;		// 벽의 작업 층 높이


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 벽 (1개), 벽을 덮는 모프 (1개)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 벽 1개, 모프 1개 선택해야 함
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

	// 테이블폼 개수 초기화
	placingZone.n1800w = 0;
	placingZone.n1850w = 0;
	placingZone.n1900w = 0;
	placingZone.n1950w = 0;
	placingZone.n2000w = 0;
	placingZone.n2050w = 0;
	placingZone.n2100w = 0;
	placingZone.n2150w = 0;
	placingZone.n2200w = 0;
	placingZone.n2250w = 0;
	placingZone.n2300w = 0;

	// 테이블폼 개수 계산
	width = placingZone.horLen;
	while (width > EPS) {
		if (width > 2.300) {
			width -= 2.300;		placingZone.n2300w ++;
		} else if (width > 2.250) {
			width -= 2.250;		placingZone.n2250w ++;
		} else if (width > 2.200) {
			width -= 2.200;		placingZone.n2200w ++;
		} else if (width > 2.150) {
			width -= 2.150;		placingZone.n2150w ++;
		} else if (width > 2.100) {
			width -= 2.100;		placingZone.n2100w ++;
		} else if (width > 2.050) {
			width -= 2.050;		placingZone.n2050w ++;
		} else if (width > 2.000) {
			width -= 2.000;		placingZone.n2000w ++;
		} else if (width > 1.950) {
			width -= 1.950;		placingZone.n1950w ++;
		} else if (width > 1.900) {
			width -= 1.900;		placingZone.n1900w ++;
		} else if (width > 1.850) {
			width -= 1.850;		placingZone.n1850w ++;
		} else if (width > 1.800) {
			width -= 1.800;		placingZone.n1800w ++;
		} else {
			break;
		}
	}

	// [DIALOG] 1번째 다이얼로그에서 벽 너비 방향의 테이블폼 수량을 설정함
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32517, ACAPI_GetOwnResModule (), wallTableformPlacerHandler, 0);

	// 벽과의 간격으로 인해 정보 업데이트
	infoWall.wallThk		+= (placingZone.gap * 2);

	if (result != DG_OK)
		return err;

	// ... 셀 정보, 셀 개수를 읽어온 후 테이블폼 배치하기

	return	err;
}

// 테이블폼 배치를 위한 질의를 요청하는 다이얼로그
short DGCALLBACK wallTableformPlacerHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	short	xx;
	short	tableColumn;	// 테이블폼 너비 방향 개수
	short	buttonPosX;		// 동적인 버튼 위치
	short	itmIdx;
	double	width;

	char	labelStr [32];

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "테이블폼 벽에 배치");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			//////////////////////////////////////////////////////////// 아이템 배치 (나머지)
			DGSetItemText (dialogID, LABEL_HEIGHT, "높이");
			DGSetItemText (dialogID, LABEL_WIDTH, "너비");
			DGSetItemText (dialogID, LABEL_ERR_MESSAGE, "높이는 다음 치수만 가능함\n1500, 1800, 2100, 2400, 2700, 3000, 3300, 3600, 3900\n4200, 4500, 4800, 5100, 5400, 5700, 6000");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "벽과의 간격");

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "비계 파이프");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트 세트");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "벽체 타이");
			DGSetItemText (dialogID, LABEL_LAYER_CLAMP, "직교 클램프");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "헤드피스");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WALLTIE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_CLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CLAMP, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);

			// 동적인 요소 그리기
			tableColumn = 0;
			if (placingZone.n2300w > 0)		tableColumn += placingZone.n2300w;
			if (placingZone.n2250w > 0)		tableColumn += placingZone.n2250w;
			if (placingZone.n2200w > 0)		tableColumn += placingZone.n2200w;
			if (placingZone.n2150w > 0)		tableColumn += placingZone.n2150w;
			if (placingZone.n2100w > 0)		tableColumn += placingZone.n2100w;
			if (placingZone.n2050w > 0)		tableColumn += placingZone.n2050w;
			if (placingZone.n2000w > 0)		tableColumn += placingZone.n2000w;
			if (placingZone.n1950w > 0)		tableColumn += placingZone.n1950w;
			if (placingZone.n1900w > 0)		tableColumn += placingZone.n1900w;
			if (placingZone.n1850w > 0)		tableColumn += placingZone.n1850w;
			if (placingZone.n1800w > 0)		tableColumn += placingZone.n1800w;

			// 테이블폼 개수 계산
			width = placingZone.horLen;
			while (width > EPS) {
				if (width > 2.300) {
					width -= 2.300;
				} else if (width > 2.250) {
					width -= 2.250;
				} else if (width > 2.200) {
					width -= 2.200;
				} else if (width > 2.150) {
					width -= 2.150;
				} else if (width > 2.100) {
					width -= 2.100;
				} else if (width > 2.050) {
					width -= 2.050;
				} else if (width > 2.000) {
					width -= 2.000;
				} else if (width > 1.950) {
					width -= 1.950;
				} else if (width > 1.900) {
					width -= 1.900;
				} else if (width > 1.850) {
					width -= 1.850;
				} else if (width > 1.800) {
					width -= 1.800;
				} else {
					break;
				}
			}

			// 남은 너비
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 280, 20, 70, 23);
			DGSetItemText (dialogID, itmIdx, "남은 너비");
			DGShowItem (dialogID, itmIdx);

			EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 350, 13, 50, 25);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, width);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_WIDTH);

			// 다이얼로그 너비 변경
			DGSetDialogSize (dialogID, DG_CLIENT, 350 + (tableColumn * 100), 500, DG_TOPLEFT, true);
			
			// 구분자
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 170, 45, tableColumn * 100 + 50, 110);
			DGShowItem (dialogID, itmIdx);

			width = placingZone.horLen;
			buttonPosX = 195;
			for (xx = 0 ; xx < tableColumn ; ++xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, buttonPosX, 50, 99, 100);
				DGShowItem (dialogID, itmIdx);

				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, buttonPosX + 20, 60, 60, 23);
				sprintf (labelStr, "너비 (%d)", xx + 1);
				DGSetItemText (dialogID, itmIdx, labelStr);
				DGShowItem (dialogID, itmIdx);

				// 너비에 해당하는 콤보박스
				POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, buttonPosX + 20, 100, 60, 25);
				DGSetItemFont (dialogID, POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2300");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2250");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2200");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2150");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2100");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2050");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2000");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1950");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1900");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1850");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1800");
				DGShowItem (dialogID, POPUP_WIDTH [xx]);

				// 콤보박스의 값 설정
				if (width > EPS) {
					if (width > 2.300) {
						width -= 2.300;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 1);
					} else if (width > 2.250) {
						width -= 2.250;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 2);
					} else if (width > 2.200) {
						width -= 2.200;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 3);
					} else if (width > 2.150) {
						width -= 2.150;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 4);
					} else if (width > 2.100) {
						width -= 2.100;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 5);
					} else if (width > 2.050) {
						width -= 2.050;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 6);
					} else if (width > 2.000) {
						width -= 2.000;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 7);
					} else if (width > 1.950) {
						width -= 1.950;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 8);
					} else if (width > 1.900) {
						width -= 1.900;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 9);
					} else if (width > 1.850) {
						width -= 1.850;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 10);
					} else if (width > 1.800) {
						width -= 1.800;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 11);
					}
				}
				buttonPosX += 100;
			}

			// 높이 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.verLen);
			DGDisableItem (dialogID, EDITCONTROL_HEIGHT);

			// 높이 값이 받아들일 수 있는 값인가?
			if ( (abs (placingZone.verLen - 1.500) < EPS) || (abs (placingZone.verLen - 1.800) < EPS) || (abs (placingZone.verLen - 2.100) < EPS) || (abs (placingZone.verLen - 2.400) < EPS) ||
				(abs (placingZone.verLen - 2.700) < EPS) || (abs (placingZone.verLen - 3.000) < EPS) || (abs (placingZone.verLen - 3.300) < EPS) || (abs (placingZone.verLen - 3.600) < EPS) ||
				(abs (placingZone.verLen - 3.900) < EPS) || (abs (placingZone.verLen - 4.200) < EPS) || (abs (placingZone.verLen - 4.500) < EPS) || (abs (placingZone.verLen - 4.800) < EPS) ||
				(abs (placingZone.verLen - 5.100) < EPS) || (abs (placingZone.verLen - 5.400) < EPS) || (abs (placingZone.verLen - 5.700) < EPS) || (abs (placingZone.verLen - 6.000) < EPS) ) {

				DGHideItem (dialogID, LABEL_ERR_MESSAGE);
			} else {
				DGShowItem (dialogID, LABEL_ERR_MESSAGE);
			}

			// 너비 계산
			DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.horLen);
			DGDisableItem (dialogID, EDITCONTROL_WIDTH);

			break;

		case DG_MSG_CHANGE:

			// ... POPUP_WIDTH [50] : 너비 콤보박스를 변경할 때마다 남은 너비 계산

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// 다이얼로그 창 정보를 입력 받음
					// ... POPUP_WIDTH [50] : 셀 정보와 셀 개수 입력

					// 벽와의 간격
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// 레이어 번호 저장
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					layerInd_Clamp			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CLAMP);
					layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
	
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