#ifndef	__OTHERS__
#define __OTHERS__

#include "MaxBIM.hpp"

namespace othersDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_others {
	};
}

GSErrCode	showLayersEasily (void);	// 레이어 쉽게 선택하기
//GSErrCode	showAbout (void);		// MaxBIM 애드온 정보
//short DGCALLBACK helpHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [도움말] 애드온 사용법 보기
//short DGCALLBACK aboutHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그 박스] MaxBIM 애드온 정보
//static GSErrCode __ACENV_CALL	APIHelpPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);		// 모달리스 창을 제어하기 위한 콜백함수

#endif