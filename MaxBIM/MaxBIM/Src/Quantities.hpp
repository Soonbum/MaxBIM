#ifndef	__QUANTITIES__
#define __QUANTITIES__

#include "MaxBIM.hpp"

using namespace std;

namespace quantitiesDG
{
	enum qElemPalette {
		BUTTON_GEAR,
		BUTTON_RECTANGLE,
		BUTTON_WINDOW
	};
}

struct qElem
{
	short	dialogID;		// �ȷ�Ʈâ ID
	short	layerInd;		// ���̾� �ε���
	char	m_type [32];	// �з�
};

GSErrCode	placeQuantityPlywood (void);		// ���������� ������ �� �ִ� �ȷ�Ʈ�� ���
static short DGCALLBACK qElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData msgData);		// �ȷ�Ʈ�� ���� �ݹ� �Լ� 1
static GSErrCode __ACENV_CALL	PaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);	// �ȷ�Ʈ�� ���� �ݹ� �Լ� 2

#endif