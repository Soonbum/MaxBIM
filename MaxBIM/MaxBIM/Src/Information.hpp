#ifndef	__INFORMATION__
#define __INFORMATION__

#include "MaxBIM.hpp"

GSErrCode	showHelp (void);		// �ֵ�� ���� ����
GSErrCode	showAbout (void);		// MaxBIM �ֵ�� ����
short DGCALLBACK helpHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [����] �ֵ�� ���� ����
short DGCALLBACK aboutHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α� �ڽ�] MaxBIM �ֵ�� ����
static GSErrCode __ACENV_CALL	APIHelpPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);		// ��޸��� â�� �����ϱ� ���� �ݹ��Լ�

#endif