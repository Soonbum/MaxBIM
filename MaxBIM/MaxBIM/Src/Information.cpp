#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Information.hpp"

static short	modelessID;


// 애드온 사용법 보기
GSErrCode	showHelp (void)
{
	GSErrCode	err = NoError;

	// 팔레트 창 열기 (모달리스 창과 호환됨)
	modelessID = DGCreateBlankPalette (500, 500, DG_DLG_HGROW | DG_DLG_VGROW, DG_DLG_CLOSE, DG_DLG_TOPCAPTION, DG_DLG_THICKFRAME, helpHandler, 0);
	
	if (modelessID != 0) {
		DGBeginProcessEvents (modelessID);
		DGShowModelessDialog (modelessID, DG_DF_FIRST);
	}

	// ...
	// MaxBIMFix.grc 파일 안에 'GICN' 리소스로 매뉴얼 캡쳐 파일을 지정할 것? 아니면 그림으로 읽을 땐 굳이 리소스 선언이 필요한가?
	/*	Sample icon, 29x20 pixels */
	/*'GICN' 32511 "Wall icon" {
		"wall_icon"
		0    128    128
	}*/
	/* Panel_Test 예제 참조
	'GICN' IDS_ATTRIBUTEPAGE_ICON "Settings Extra page icon" {
			"Panel_Icon"
			0  128  128
	}*/

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
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 200, 460, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "닫기");
			DGShowItem (dialogID, DG_OK);

			break;
		
		case DG_MSG_CHANGE:
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					DGEndProcessEvents (modelessID);
					DGDestroyModelessDialog (modelessID);
					ACAPI_UnregisterModelessWindow (modelessID);
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
			DGSetItemText (dialogID, itmIdx, "버전 5.0 (배포일: 2020.12.01)");
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
	if (referenceID == modelessID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGEndProcessEvents (modelessID);
												DGDestroyModelessDialog (modelessID);
												//DGModelessClose (modelessID);
												break;
			case APIPalMsg_HidePalette_Begin:	DGHideModelessDialog (modelessID);
												break;
			case APIPalMsg_HidePalette_End:		DGShowModelessDialog (modelessID, DG_DF_FIRST);
												break;
			case APIPalMsg_DisableItems_Begin:
			case APIPalMsg_DisableItems_End:	// actually do nothing, because the input finish functionality the buttons have to stay enabled
			case APIPalMsg_IsPaletteVisible:
			case APIPalMsg_OpenPalette:			break;
		}
	}

	return NoError;
}