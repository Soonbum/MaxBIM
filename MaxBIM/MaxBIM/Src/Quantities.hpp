#ifndef	__QUANTITIES__
#define __QUANTITIES__

#include "MaxBIM.hpp"

using namespace std;

namespace quantitiesDG
{
	enum qElemPalette {
		LABEL_EXPLANATION = 1,
		USERCONTROL_PLYWOOD_TYPE = 2,
		POPUP_PLYWOOD_TYPE = 3,
		BUTTON_DRAW_RECT = 4,
		BUTTON_DRAW_WINDOW = 5
	};
}

struct qElem
{
	short	dialogID;		// 팔레트창 ID
	short	floorInd;		// 층 인덱스
	short	layerInd;		// 레이어 인덱스
	char	m_type [32];	// 분류
	short	panel_mat;		// 재질
};

GSErrCode	placeQuantityPlywood (void);		// 물량합판을 부착할 수 있는 팔레트를 띄움
static short DGCALLBACK qElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData msgData);		// 팔레트에 대한 콜백 함수 1
static GSErrCode __ACENV_CALL	PaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);	// 팔레트에 대한 콜백 함수 2

#endif