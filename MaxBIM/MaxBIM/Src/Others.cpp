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

	char	substr [10];
	char	*token;

	
	// ������Ʈ �� ���̾� �̸��� ���� �о��
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			// ���̾� �̸� ����
			//sprintf (buffer, "%d/%d : %s\n", xx, nLayers, attrib.layer.head.name);

			// ���̾� �̸��� "-" ���� �������� �ɰ���
			//token = strtok (attrib.layer.head.name, "-");
			//while (token != NULL) {
			//	sprintf (substr, "%s", token);
			//	ACAPI_WriteReport (substr, true);
			//	token = strtok (NULL, "-");
			//}
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// ���̾� �̸��� ü�踦 �м��Ͽ� ����ü �ʱ�ȭ
	// ...

	// �˰��� (�ʾ�)
	// 1. ���� ����(�ʼ�) ��ū�� ��ȿ���� ���� -> ���� ������ �����ϴ� �͸� ����
		// 01-S ����, 02-A ���ึ��, 03-M ��輳��, 04-E ���⼳��, 05-T ������, 06-F ���ü�, 07-Q ��������, 08-L ����, 09-C ���, 10-K �Ǽ����
	// 2. �� ����(����) ��ū�� �ִ°�, ���°�? -> ���� �����ϸ� �� ���� ������
		// 4���� �ڵ��̾�� �ϸ� �� ���� ��ū�� �ƴ� ��� (���ڷθ� �Ǿ� ������ �� ��ȣ, �� �ܿ���?)
	// 3. �� ����(�ʼ�) ��ū�� ��ȿ���� ���� -> ���� �����ϴ� �͸� ����
		// 4���� �ڵ� (������, ������, ��ž��)
	// 4. CJ���� (Construction Joint) ��ū�� �ִ°�, ���°�? -> CJ ������ �����ϸ� CJ ���� ���� ������
		// ���� �ڵ�(01~99)
	// 5. CJ�� �ð����� ��ū�� �ִ°�, ���°�?
		// ���� �ڵ�(01~99)
	// 6. ���� �� ��ü ������ �ִ°�, ���°�?
		// ���� ���� ��ū�� ���õ� ���� ���� ��ū�� �ִ��� Ȯ��

	// ���ڿ� ���� �˰���
	/*
		1. ó�� 2���� ��ū�� ���� �޴´�.
			- ���� ���� �ڵ尡 �´��� Ȯ���Ѵ�. (01~10������ �ν�, 50(2D����)�̳� �������� ����)
			- ���� �´ٸ�, ����ü�� ���� ���� �ڵ� flag�� set��Ŵ*
		2. ���� ��ū�� �޴´�.
			- 4�����̸� �� ���� (0101~1504, SHOP, SECU ��) - ����
			- 3�����̸� �� ���� (1B9~9B1, F01~F99, PH1~PH3) - �ʼ�
		3. �� �����̶��?
			- ������ ����� �� ���� ���ڿ��� ���ٸ� list�� ������ ��
			- "�ڵ� ���ڿ�" - "���� ���ڿ�"
		4. �� �����̶��?
			- ������ ����� �� ���� ���ڿ��� ���ٸ� list�� ������ ��
			- "�ڵ� ���ڿ�" - "���� ���ڿ�"
		5. ���� ��ū�� �޴´�.
			- 2�����̸� CJ ���� (01~99) : list�� ������ ��
			- �� �ܿ��� ���� �� ��ü ����
		6. ���� ��ū�� �޴´�.
			- 2�����̸� CJ �� �ð����� (01~99) : list�� ������ ��
			- �� �ܿ��� ���� �� ��ü ����
		7. ���� �� ��ü ���� (�ʼ�)
			- list�� ������ ��
		
		���� �ڵ�: 01-S-(0101)-9B1-(01)-(01)-WALL  ��, ��ȣ ���� ���û���
	*/

	// ���̾�α� ǥ�� �˰���
	/*
		1. ���� ���� �ڵ� flag�� true�� �͸� ��ư ǥ�� + ��� ǥ�� ��ư
		2. �� ���� (���� ���): ��ư ǥ�� + ��� ǥ�� ��ư
		3. �� ����: ��ư ǥ�� + (������, ������, ��ž�� ���� ��� ǥ�� ��ư)
		4. CJ ���� (���� ���): ��ư ǥ�� + ��� ǥ�� ��ư
		5. CJ �� �ð����� (���� ���): ��ư ǥ�� + ��� ǥ�� ��ư
		6. ���� �� ��ü ����
	*/

	// ���ڿ� ���� �˰���
	/*
		1. ���� �ڵ� flag�� �о�� (�����̸� ��� �о��)
		2. �� �ڵ带 �о�� (���� ����, �ڵ� ���� ���� �� ����)
		3. �� �ڵ带 �о�� (���� ����)
		4. CJ ���� �ڵ带 �о�� (���� ����, �ڵ� ���� ���� �� ����)
		5. CJ �� �ð����� �ڵ带 �о�� (���� ����, �ڵ� ���� ���� �� ����)
		6. ���� �� ��ü ���� �ڵ带 �о�� (���� ����)
	*/

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
