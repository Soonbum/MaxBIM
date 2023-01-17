#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Information.hpp"

using namespace informationDG;

short	modelessDialogID;

// �ֵ�� ���� ����
GSErrCode	showHelp (void)
{
	GSErrCode	err = NoError;

	modelessDialogID = 0;

	// �ȷ�Ʈ â ���� (��޸��� â�� ȣȯ��)
	if ((modelessDialogID == 0) || !DGIsDialogOpen (modelessDialogID)) {
		modelessDialogID = DGModelessInit (ACAPI_GetOwnResModule (), 32501, ACAPI_GetOwnResModule (), helpHandler, 0, 1);
	}
	
	return err;
}

// MaxBIM �ֵ�� ����
GSErrCode	showAbout (void)
{
	GSErrCode	err = NoError;
	short	result;

	result = DGBlankModalDialog (550, 500, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, aboutHandler, 0);

	return err;
}

// [����] �ֵ�� ���� ����
short DGCALLBACK helpHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;

	switch (message) {
		case DG_MSG_INIT:
			// ��޸��� â ���
			if (ACAPI_RegisterModelessWindow (dialogID, APIHelpPaletteAPIControlCallBack,
						API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
						API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_3D + API_PalEnabled_Layout) != NoError)
				DBPrintf ("Test:: ACAPI_RegisterModelessWindow failed\n");

			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"�ֵ�� ����");

			// �ݱ� ��ư
			DGSetItemText (dialogID, DG_OK, L"�ݱ�");

			// ��: ���̺��� ��ġ
			DGSetItemText (dialogID, LABEL_PLACING_EUROFORM, L"���̺��� ��ġ");

			// ��ư: ��
			DGSetItemText (dialogID, BUTTON_WALL, L"��");

			// ��ư: ������
			DGSetItemText (dialogID, BUTTON_SLAB, L"������");

			// ��ư: ��
			DGSetItemText (dialogID, BUTTON_BEAM, L"��");

			// ��ư: ���
			DGSetItemText (dialogID, BUTTON_COLUMN, L"���");

			DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
			DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
			DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
			DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);

			break;
		
		case DG_MSG_CHANGE:
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					ACAPI_UnregisterModelessWindow (modelessDialogID);
					modelessDialogID = 0;
					break;

				case BUTTON_WALL:
					item = 0;

					// �� ���� �Ŵ��� ǥ��
					DGShowItem (dialogID, ICON_MORPH_FOR_WALL);
					DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);

					// ���� ���ɼ��� �ִ� DG �׸� ��� ����
					DGRemoveDialogItems (dialogID, AFTER_ALL);

					// �׸��� ���� ����
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 260, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���̺����� ���� ���鿡 ���簢�� ������ �׸���,\n���ϴܺ��� �������� �׸��ʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���鿡 �ܿ��簡 ������ ������ �ܿ��� ���� �׸���\n�˴ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���� ���纸�� ���� ���̺����� ���� ����� ���Ī�̸�\n������ 2�� �׸��ʽÿ�");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���� 2���� �׸� ���, �غ��� ���� ���ƾ� �ϸ�\n������ �ʺ� ���ƾ� �մϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 380, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���� ��Ҹ� ������ �� �����Ͻʽÿ�:\n��, ����, (���̰� �ٸ� ����)");
					DGShowItem (dialogID, itmIdx);

					break;

				case BUTTON_SLAB:
					item = 0;

					// ������ ���� �Ŵ��� ǥ��
					DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
					DGShowItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);

					// ���� ���ɼ��� �ִ� DG �׸� ��� ����
					DGRemoveDialogItems (dialogID, AFTER_ALL);
					
					// �׸��� ���� ����
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 260, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���ǳ��� ���� ������ �Ϻο� ������ �׸��ʽÿ�.\n��� � ���� �ڳʰ� ���� �� �ֽ��ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"������ �Ϻο� �ܿ��簡 ������ ������ �ܿ��� ����\n�׸��� �˴ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���� �� ���� �ϴ��� ����, ���� ���� �������\nŬ���Ͻʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���� ��Ҹ� ������ �� �����Ͻʽÿ�: ������, ����");
					DGShowItem (dialogID, itmIdx);

					break;

				case BUTTON_BEAM:
					item = 0;

					// �� ���� �Ŵ��� ǥ��
					DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
					DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGShowItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGHideItem (dialogID, ICON_MORPH_FOR_COLUMN);

					// ���� ���ɼ��� �ִ� DG �׸� ��� ����
					DGRemoveDialogItems (dialogID, AFTER_ALL);

					// �׸��� ���� ����
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 260, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���̺����� ���� �� ���鿡 ������\n�׸��ʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"�� ����/�Ϻο� �ܿ��簡 ������ ������ �ܿ��� ����\n�׸��� �˴ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���� ��Ҹ� ������ �� �����Ͻʽÿ�: ��, ����, (���̰� �ٸ� ����)");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"��, ��� ���� ��� ���� ������ ������ ���ƾ� �մϴ�. (���� ������ ����)");
					DGShowItem (dialogID, itmIdx);

					break;

				case BUTTON_COLUMN:
					item = 0;

					// ��� ���� �Ŵ��� ǥ��
					DGHideItem (dialogID, ICON_MORPH_FOR_WALL);
					DGHideItem (dialogID, ICON_MORPH_FOR_SLAB);
					DGHideItem (dialogID, ICON_MORPH_FOR_BEAM);
					DGShowItem (dialogID, ICON_MORPH_FOR_COLUMN);

					// ���� ���ɼ��� �ִ� DG �׸� ��� ����
					DGRemoveDialogItems (dialogID, AFTER_ALL);

					// �׸��� ���� ����
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 260, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"��� ������ ���̺����� ���� ������ ������\n�׸��ʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"����� ���, �ܿ��縦 ������� �ʽ��ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"��տ� ���� �� ��ó�� ������ ��ġ�ϱ� ���� ���� ����\n�����Ͻʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, L"���� ��Ҹ� ������ �� �����Ͻʽÿ�:\n���, ����, (�� �ټ�)");
					DGShowItem (dialogID, itmIdx);

					break;
			}
		case DG_MSG_CLOSE:
			ACAPI_UnregisterModelessWindow (modelessDialogID);
			modelessDialogID = 0;
			break;
	}

	result = item;

	return	result;
}

// [���̾�α� �ڽ�] MaxBIM �ֵ�� ����
short DGCALLBACK aboutHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"MaxBIM �ֵ�� ����");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 215, 460, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"Ȯ��");
			DGShowItem (dialogID, DG_OK);

			itmPosX = 20;
			itmPosY = 20;

			// ��: ���� (�ֱ� ������)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 250, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"������: 2023.01.17 - 18:31");
			DGShowItem (dialogID, itmIdx);
			itmPosY += 30;

			// ��: �ۼ��� (�̸���)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 250, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"�ۼ���: ������ (canvas84@naver.com)");
			DGShowItem (dialogID, itmIdx);
			itmPosY += 30;

			// ��: ���α׷� ���
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 255, 320);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"1. ���̺��� ��ġ\n - ���� ���̺��� ��ġ\n - ������ �Ϻο� ���̺��� ��ġ\n - ���� ���̺��� ��ġ\n - ��տ� ���̺��� ��ġ\n - ���� ������ ���鿡 ���̺��� ��ġ\n\n\
											 2. ���̾� ��ƿ\n - ���̾� ���� �����ϱ�\n - ���̾� ���� �����\n - ���̾� ���� �����ϱ�\n - ���̾� �̸� �˻��ϱ�");
			DGShowItem (dialogID, itmIdx);

			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 260, itmPosY, 255, 320);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"3. ��������\n - ������ ���� ���� �������� (Single ���)\n - ������ ���� ���� �������� (Multi ���)\n - ���纰 ���� �� �����ֱ�\n - �� ���̺��� ����ǥ �ۼ�\n - ���̺��� ���� ���\n - ��ũ��Ʈ ���� ��� (Single ���)\n - ��ũ��Ʈ ���� ��� (Multi ���)\n - ������ ����/�Ϻθ��� ��� (Single ���)\n - ������ ����/�Ϻθ��� ��� (Multi ���)\n - �ܿ��� ����/���� ��� (Single ���)\n - �ܿ��� ����/���� ��� (Multi ���)\n - ��� �Ը鵵 PDF�� �������� (Single ���)\n - ��� �Ը鵵 PDF�� �������� (Multi ���)\n - ���� ���� ��� (Single ���)\n\n\
											 4. ���ڵ� ��ġ\n - �������� �����ϱ�\n - �ܿ��� �����ϱ�\n\n\
											 5. ���� ���\n - 3D ǰ��/�ӵ� �����ϱ�\n - ������ 3D �� ���̱�\n - ���� ��鵵�� ���̺����� ���� �ڵ� ��ġ\n - ī�޶� ��ġ �����ϱ�/�ҷ�����");
			DGShowItem (dialogID, itmIdx);

			break;
		
		case DG_MSG_CHANGE:
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					break;
			}
		case DG_MSG_CLOSE:
			break;
	}

	result = item;

	return	result;
}

// ��޸��� â�� �����ϱ� ���� �ݹ��Լ�
static GSErrCode __ACENV_CALL	APIHelpPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	if (referenceID == modelessDialogID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGModelessClose (modelessDialogID);
												break;
			case APIPalMsg_HidePalette_Begin:	DGHideModelessDialog (modelessDialogID);
												break;
			case APIPalMsg_HidePalette_End:		DGShowModelessDialog (modelessDialogID, DG_DF_FIRST);
												break;
			case APIPalMsg_DisableItems_Begin:	EnablePaletteControls (modelessDialogID, false);
												break;
			case APIPalMsg_DisableItems_End:	EnablePaletteControls (modelessDialogID, true);
												break;
			case APIPalMsg_IsPaletteVisible:	DGModelessClose (modelessDialogID);
												break;
			case APIPalMsg_OpenPalette:			break;
			default:							break;
		}
	}

	return NoError;
}