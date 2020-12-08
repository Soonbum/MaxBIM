#ifndef	__INFORMATION__
#define __INFORMATION__

#include "MaxBIM.hpp"

namespace informationDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_information {
		LABEL_PLACING_EUROFORM = 2,
		BUTTON_WALL,
		BUTTON_SLAB,
		BUTTON_BEAM,
		BUTTON_COLUMN,
		LEFT_SEPARATOR,
		ICON_MORPH_FOR_WALL,
		ICON_MORPH_FOR_SLAB,
		ICON_MORPH_FOR_BEAM,
		ICON_MORPH_FOR_COLUMN,
		AFTER_ALL
	};
}

GSErrCode	showHelp (void);		// 애드온 사용법 보기
GSErrCode	showAbout (void);		// MaxBIM 애드온 정보
short DGCALLBACK helpHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [도움말] 애드온 사용법 보기
short DGCALLBACK aboutHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그 박스] MaxBIM 애드온 정보
static GSErrCode __ACENV_CALL	APIHelpPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);		// 모달리스 창을 제어하기 위한 콜백함수

#endif