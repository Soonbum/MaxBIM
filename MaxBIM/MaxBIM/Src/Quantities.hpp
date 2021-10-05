#ifndef	__QUANTITIES__
#define __QUANTITIES__

#include "MaxBIM.hpp"

using namespace std;

namespace quantitiesDG
{
	enum qElemPalette {
		LABEL_EXPLANATION_QELEM = 1,
		USERCONTROL_QPLYWOOD_LAYER,
		POPUP_QPLYWOOD_TYPE,
		BUTTON_DRAW_RECT_QELEM,
		BUTTON_DRAW_WINDOW_QELEM
	};

	enum insulElemPalette {
		LABEL_EXPLANATION_INS = 1,
		USERCONTROL_INSULATION_LAYER,
		LABEL_INSULATION_THK,
		EDITCONTROL_INSULATION_THK,
		CHECKBOX_INS_LIMIT_SIZE,
		LABEL_INS_HORLEN,
		EDITCONTROL_INS_HORLEN,
		LABEL_INS_VERLEN,
		EDITCONTROL_INS_VERLEN,
		BUTTON_DRAW_INSULATION
	};
}

struct qElem
{
	short	dialogID;		// 팔레트창 ID
	short	floorInd;		// 층 인덱스
	short	layerInd;		// 레이어 인덱스
	char	m_type [32];	// 분류
	short	panel_mat;		// 재질
};	// 물량합판

GSErrCode	placeQuantityPlywood (void);		// 물량합판을 부착할 수 있는 팔레트를 띄움
static short DGCALLBACK qElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData msgData);				// 팔레트에 대한 콜백 함수 1
static GSErrCode __ACENV_CALL	qElemPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);		// 팔레트에 대한 콜백 함수 2


struct insulElem
{
	short	dialogID;		// 팔레트창 ID
	short	floorInd;		// 층 인덱스
	short	layerInd;		// 레이어 인덱스
	double	thk;			// 두께
	bool	bLimitSize;		// 가로/세로 크기 제한
	double	maxHorLen;			// 가로 최대 길이
	double	maxVerLen;			// 세로 최대 길이
};	// 단열재

GSErrCode	placeInsulation (void);				// 단열재를 부착할 수 있는 팔레트를 띄움
static short DGCALLBACK insulElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData msgData);			// 팔레트에 대한 콜백 함수 1
static GSErrCode __ACENV_CALL	insulElemPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);	// 팔레트에 대한 콜백 함수 2

#endif