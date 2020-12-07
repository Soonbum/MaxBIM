#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Information.hpp"

using namespace informationDG;

static short	paletteID;

// 애드온 사용법 보기
GSErrCode	showHelp (void)
{
	GSErrCode	err = NoError;

	// 팔레트 창 열기 (모달리스 창과 호환됨)
	paletteID = DGModelessInit (ACAPI_GetOwnResModule (), 32515, ACAPI_GetOwnResModule (), helpHandler, 0, 1);
	
	return err;
}

// MaxBIM 애드온 정보
GSErrCode	showAbout (void)
{
	GSErrCode	err = NoError;
	short	result;

	result = DGBlankModalDialog (500, 300, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, aboutHandler, 0);

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
			ACAPI_RegisterModelessWindow (dialogID, APIHelpPaletteAPIControlCallBack,
							API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation + API_PalEnabled_InteriorElevation +
							API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_Layout + API_PalEnabled_3D);

			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "애드온 사용법");

			// 닫기 버튼
			DGSetItemText (dialogID, DG_OK, "닫기");

			// 라벨: 유로폼 배치
			DGSetItemText (dialogID, LABEL_PLACING_EUROFORM, "유로폼 배치");

			// 버튼: 벽
			DGSetItemText (dialogID, BUTTON_WALL, "벽");

			// 버튼: 슬래브
			DGSetItemText (dialogID, BUTTON_SLAB, "슬래브");

			// 버튼: 보
			DGSetItemText (dialogID, BUTTON_BEAM, "보");

			// 버튼: 기둥
			DGSetItemText (dialogID, BUTTON_COLUMN, "기둥");

			// 처음에는 벽 관련 매뉴얼을 보여줌
			DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
			DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
			DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);

			break;
		
		case DG_MSG_CHANGE:
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					DGModelessClose (paletteID);
					DGDestroyModelessDialog (paletteID);
					ACAPI_UnregisterModelessWindow (paletteID);
					break;

				case BUTTON_WALL:
					item = 0;

					// 벽 관련 매뉴얼 표시
					DGShowItem (dialogID, ICON_MORPH_FOR_WALL);
					DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);
					// ... 그 외 텍스트 추가

					break;

				case BUTTON_SLAB:
					item = 0;

					// 슬래브 관련 매뉴얼 표시
					DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
					DGShowItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);
					// ... 그 외 텍스트 추가

					break;

				case BUTTON_BEAM:
					item = 0;

					// 보 관련 매뉴얼 표시
					DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
					DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGShowItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);
					// ... 그 외 텍스트 추가

					break;

				case BUTTON_COLUMN:
					item = 0;

					// 기둥 관련 매뉴얼 표시
					DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
					DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGShowItem (dialogID, ICON_MORPH_FOR_COLUMN);
					// ... 그 외 텍스트 추가

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
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 200, 260, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			itmPosX = 20;
			itmPosY = 20;

			// 라벨: 버전 (최근 배포일)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 230, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "버전 5.1 (배포일: 2020.12.03)");
			DGShowItem (dialogID, itmIdx);
			itmPosY += 30;

			// 라벨: 작성자 (이메일)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 230, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "작성자: 정순범 (canvas84@naver.com)");
			DGShowItem (dialogID, itmIdx);
			itmPosY += 30;

			// 라벨: 프로그램 기능
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 230, 23*4);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "1. 유로폼 배치\n  - 벽에 유로폼 배치\n  - 슬래브 하부에 유로폼 배치\n  - 보에 유로폼 배치\n  - 기둥에 유로폼 배치");
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
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// 모달리스 창을 제어하기 위한 콜백함수
static GSErrCode __ACENV_CALL	APIHelpPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	if (referenceID == paletteID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGModelessClose (paletteID);
												DGDestroyModelessDialog (paletteID);
												ACAPI_UnregisterModelessWindow (paletteID);
												break;
			case APIPalMsg_HidePalette_Begin:	DGHideModelessDialog (paletteID);
												break;
			case APIPalMsg_HidePalette_End:		DGShowModelessDialog (paletteID, DG_DF_FIRST);
												break;
			case APIPalMsg_DisableItems_Begin:
			case APIPalMsg_DisableItems_End:	// actually do nothing, because the input finish functionality the buttons have to stay enabled
			case APIPalMsg_IsPaletteVisible:
			case APIPalMsg_OpenPalette:			break;
		}
	}

	return NoError;
}