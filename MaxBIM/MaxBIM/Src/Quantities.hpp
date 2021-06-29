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
	short	dialogID;		// �ȷ�Ʈâ ID
	short	floorInd;		// �� �ε���
	short	layerInd;		// ���̾� �ε���
	char	m_type [32];	// �з�
	short	panel_mat;		// ����
};

GSErrCode	placeQuantityPlywood (void);		// ���������� ������ �� �ִ� �ȷ�Ʈ�� ���
static short DGCALLBACK qElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData msgData);		// �ȷ�Ʈ�� ���� �ݹ� �Լ� 1
static GSErrCode __ACENV_CALL	PaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);	// �ȷ�Ʈ�� ���� �ݹ� �Լ� 2

#endif