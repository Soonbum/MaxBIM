#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Others.hpp"

using namespace othersDG;

// ���̾� ���� �����ϱ�
GSErrCode	showLayersEasily (void)
{
	GSErrCode	err = NoError;
	
	API_Attribute	attrib;
	short			xx;
	short			nLayers;

	
	// ������Ʈ �� ���̾� �̸��� ���� �о��
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			// ���̾� �̸� ����
			//sprintf (buffer, "%d/%d : %s - flag: %d\n", xx, nLayers, attrib.layer.head.name, attrib.layer.head.flags);
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// ���̾� �̸��� ü�踦 �м��Ͽ� ����ü �ʱ�ȭ
	// ...

	// ���̾�α� â ����
	// ...


	// ������ �ڵ�
	// ���̾� ���̱�/���� ���� (APILay_Hidden �� ������ ����, ������ ����)
	// 1. Ư�� �̸����� ���̾� �Ӽ��� ã�� ���� (ACAPI_Attribute_Search)
	// 2. ���̾�α��� �Է� ���� ���� ����/���� �ɼ��� �ο��Ͽ� �Ӽ��� ���� (ACAPI_Attribute_Modify)
			
	// ���� ǥ���ϱ�
	//if ((attrib.layer.head.flags & APILay_Hidden) == true) {
	//	attrib.layer.head.flags ^= APILay_Hidden;
	//	ACAPI_Attribute_Modify (&attrib, NULL);
	//}

	// ���� �����
	//if ((attrib.layer.head.flags & APILay_Hidden) == false) {
	//	attrib.layer.head.flags |= APILay_Hidden;
	//	ACAPI_Attribute_Modify (&attrib, NULL);
	//}

	return err;
}

//// [���̾�α� �ڽ�] MaxBIM �ֵ�� ����
//short DGCALLBACK aboutHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
//{
//	short	result;
//	short	itmIdx;
//	short	itmPosX, itmPosY;
//
//	switch (message) {
//		case DG_MSG_INIT:
//			// ���̾�α� Ÿ��Ʋ
//			DGSetDialogTitle (dialogID, "MaxBIM �ֵ�� ����");
//
//			// Ȯ�� ��ư
//			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 200, 260, 100, 25);
//			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
//			DGSetItemText (dialogID, DG_OK, "Ȯ��");
//			DGShowItem (dialogID, DG_OK);
//
//			itmPosX = 20;
//			itmPosY = 20;
//
//			// ��: ���� (�ֱ� ������)
//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 230, 23);
//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
//			DGSetItemText (dialogID, itmIdx, "���� 6.0 (������: 2020.12.14)");
//			DGShowItem (dialogID, itmIdx);
//			itmPosY += 30;
//
//			// ��: �ۼ��� (�̸���)
//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 230, 23);
//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
//			DGSetItemText (dialogID, itmIdx, "�ۼ���: ������ (canvas84@naver.com)");
//			DGShowItem (dialogID, itmIdx);
//			itmPosY += 30;
//
//			// ��: ���α׷� ���
//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 230, 23*7);
//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
//			DGSetItemText (dialogID, itmIdx, "1. ������ ��ġ\n  - ���� ������ ��ġ\n  - ������ �Ϻο� ������ ��ġ\n  - ���� ������ ��ġ\n  - ��տ� ������ ��ġ\n\n2. ��ƿ��Ƽ\n  - ���̾� ���� �����ϱ� (������)");
//			DGShowItem (dialogID, itmIdx);
//
//			break;
//		
//		case DG_MSG_CHANGE:
//			break;
//
//		case DG_MSG_CLICK:
//			switch (item) {
//				case DG_OK:
//					break;
//			}
//		case DG_MSG_CLOSE:
//			switch (item) {
//				case DG_CLOSEBOX:
//					break;
//			}
//	}
//
//	result = item;
//
//	return	result;
//}
