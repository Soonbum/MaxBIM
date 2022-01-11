#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Information.hpp"

using namespace informationDG;

short	modelessDialogID;

// 애드온 사용법 보기
GSErrCode	showHelp (void)
{
	GSErrCode	err = NoError;

	modelessDialogID = 0;

	// 팔레트 창 열기 (모달리스 창과 호환됨)
	if ((modelessDialogID == 0) || !DGIsDialogOpen (modelessDialogID)) {
		modelessDialogID = DGModelessInit (ACAPI_GetOwnResModule (), 32501, ACAPI_GetOwnResModule (), helpHandler, 0, 1);
	}
	
	return err;
}

// MaxBIM 애드온 정보
GSErrCode	showAbout (void)
{
	GSErrCode	err = NoError;
	short	result;

	result = DGBlankModalDialog (550, 500, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, aboutHandler, 0);

	return err;
}

// [도움말] 애드온 사용법 보기
short DGCALLBACK helpHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;

	switch (message) {
		case DG_MSG_INIT:
			// 모달리스 창 등록
			if (ACAPI_RegisterModelessWindow (dialogID, APIHelpPaletteAPIControlCallBack,
						API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
						API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_3D + API_PalEnabled_Layout) != NoError)
				DBPrintf ("Test:: ACAPI_RegisterModelessWindow failed\n");

			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "애드온 사용법");

			// 닫기 버튼
			DGSetItemText (dialogID, DG_OK, "닫기");

			// 라벨: 테이블폼 배치
			DGSetItemText (dialogID, LABEL_PLACING_EUROFORM, "테이블폼 배치");

			// 버튼: 벽
			DGSetItemText (dialogID, BUTTON_WALL, "벽");

			// 버튼: 슬래브
			DGSetItemText (dialogID, BUTTON_SLAB, "슬래브");

			// 버튼: 보
			DGSetItemText (dialogID, BUTTON_BEAM, "보");

			// 버튼: 기둥
			DGSetItemText (dialogID, BUTTON_COLUMN, "기둥");

			DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
			DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
			DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
			DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);

			break;
		
		case DG_MSG_CHANGE:
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					ACAPI_UnregisterModelessWindow (modelessDialogID);
					modelessDialogID = 0;
					break;

				case BUTTON_WALL:
					item = 0;

					// 벽 관련 매뉴얼 표시
					DGShowItem (dialogID, ICON_MORPH_FOR_WALL);
					DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);

					// 변경 가능성이 있는 DG 항목 모두 제거
					DGRemoveDialogItems (dialogID, AFTER_ALL);

					// 그림에 대한 설명
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 260, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "테이블폼을 붙일 벽면에 직사각형 모프를 그리되,\n좌하단부터 우상단으로 그리십시오.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "벽면에 단열재가 있으면 모프를 단열재 위에 그리면\n됩니다.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "만약 눈썹보로 인해 테이블폼을 덮는 양면이 비대칭이면\n모프를 2개 그리십시오");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "모프 2개를 그릴 경우, 밑변의 고도는 같아야 하며\n모프의 너비도 같아야 합니다.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 380, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "다음 요소를 선택한 후 실행하십시오:\n벽, 모프, (높이가 다른 모프)");
					DGShowItem (dialogID, itmIdx);

					break;

				case BUTTON_SLAB:
					item = 0;

					// 슬래브 관련 매뉴얼 표시
					DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
					DGShowItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);

					// 변경 가능성이 있는 DG 항목 모두 제거
					DGRemoveDialogItems (dialogID, AFTER_ALL);
					
					// 그림에 대한 설명
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 260, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "콘판넬을 붙일 슬래브 하부에 모프를 그리십시오.\n기둥 등에 의해 코너가 꺾일 수 있습니다.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "슬래브 하부에 단열재가 있으면 모프를 단열재 위에\n그리면 됩니다.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "실행 후 모프 하단의 좌측, 우측 점을 순서대로\n클릭하십시오.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "다음 요소를 선택한 후 실행하십시오: 슬래브, 모프");
					DGShowItem (dialogID, itmIdx);

					break;

				case BUTTON_BEAM:
					item = 0;

					// 보 관련 매뉴얼 표시
					DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
					DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGShowItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);

					// 변경 가능성이 있는 DG 항목 모두 제거
					DGRemoveDialogItems (dialogID, AFTER_ALL);

					// 그림에 대한 설명
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 260, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "테이블폼을 붙일 보 측면에 모프를\n그리십시오.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "보 측면/하부에 단열재가 있으면 모프를 단열재 위에\n그리면 됩니다.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "다음 요소를 선택한 후 실행하십시오: 보, 모프, (높이가 다른 모프)");
					DGShowItem (dialogID, itmIdx);

					break;

				case BUTTON_COLUMN:
					item = 0;

					// 기둥 관련 매뉴얼 표시
					DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
					DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGShowItem (dialogID, ICON_MORPH_FOR_COLUMN);

					// 변경 가능성이 있는 DG 항목 모두 제거
					DGRemoveDialogItems (dialogID, AFTER_ALL);

					// 그림에 대한 설명
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 260, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "기둥 측면의 테이블폼을 붙일 영역에 모프를\n그리십시오.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "기둥의 경우, 단열재를 고려하지 않습니다.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "기둥에 붙은 보 근처에 합판을 설치하기 위해 보를 같이\n선택하십시오.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "다음 요소를 선택한 후 실행하십시오:\n기둥, 모프, (보 다수)");
					DGShowItem (dialogID, itmIdx);

					break;
			}
		case DG_MSG_CLOSE:
			ACAPI_UnregisterModelessWindow (modelessDialogID);
			modelessDialogID = 0;
			break;
	}

	result = item;

	return	result;
}

// [다이얼로그 박스] MaxBIM 애드온 정보
short DGCALLBACK aboutHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "MaxBIM 애드온 정보");

			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 215, 460, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			itmPosX = 20;
			itmPosY = 20;

			// 라벨: 버전 (최근 배포일)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 250, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "배포일: 2022.01.11 - 18:15");
			DGShowItem (dialogID, itmIdx);
			itmPosY += 30;

			// 라벨: 작성자 (이메일)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 250, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "작성자: 정순범 (canvas84@naver.com)");
			DGShowItem (dialogID, itmIdx);
			itmPosY += 30;

			// 라벨: 프로그램 기능
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 250, 320);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "1. 테이블폼 배치\n - 벽에 테이블폼 배치\n - 슬래브 하부에 테이블폼 배치\n - 보에 테이블폼 배치\n - 기둥에 테이블폼 배치\n\n\
											 2. 레이어 유틸\n - 레이어 쉽게 선택하기\n - 레이어 쉽게 만들기\n - 레이어 쉽게 지정하기\n - 레이어 이름 검사하기");
			DGShowItem (dialogID, itmIdx);

			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 270, itmPosY, 250, 320);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "3. 내보내기\n - 부재(기둥,보,슬래브) 정보 내보내기 (CSV) (개발중단)\n - 선택한 부재 정보 내보내기 (Single 모드)\n - 선택한 부재 정보 내보내기 (Multi 모드)\n - 부재별 선택 후 보여주기\n\n\
											 4. 반자동 배치\n - 물량합판 부착하기\n - 단열재 부착하기");
			DGShowItem (dialogID, itmIdx);

			break;
		
		case DG_MSG_CHANGE:
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					break;
			}
		case DG_MSG_CLOSE:
			break;
	}

	result = item;

	return	result;
}

// 모달리스 창을 제어하기 위한 콜백함수
static GSErrCode __ACENV_CALL	APIHelpPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	if (referenceID == modelessDialogID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGModelessClose (modelessDialogID);
												break;

			case APIPalMsg_HidePalette_Begin:	break;
			case APIPalMsg_HidePalette_End:		break;

			case APIPalMsg_DisableItems_Begin:	break;
			case APIPalMsg_DisableItems_End:	break;
			case APIPalMsg_OpenPalette:			break;
			default:							break;
		}
	}

	return NoError;
}