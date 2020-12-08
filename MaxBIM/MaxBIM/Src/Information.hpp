#ifndef	__INFORMATION__
#define __INFORMATION__

#include "MaxBIM.hpp"

namespace informationDG {
	// ���̾�α� �׸� �ε���
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

GSErrCode	showHelp (void);		// �ֵ�� ���� ����
GSErrCode	showAbout (void);		// MaxBIM �ֵ�� ����
short DGCALLBACK helpHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [����] �ֵ�� ���� ����
short DGCALLBACK aboutHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α� �ڽ�] MaxBIM �ֵ�� ����
static GSErrCode __ACENV_CALL	APIHelpPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);		// ��޸��� â�� �����ϱ� ���� �ݹ��Լ�

#endif