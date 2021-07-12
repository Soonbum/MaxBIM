#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Information.hpp"

using namespace informationDG;

static short	paletteID;

// �ֵ�� ���� ����
GSErrCode	showHelp (void)
{
	GSErrCode	err = NoError;

	// �ȷ�Ʈ â ���� (��޸��� â�� ȣȯ��)
	paletteID = DGModelessInit (ACAPI_GetOwnResModule (), 32516, ACAPI_GetOwnResModule (), helpHandler, 0, 1);
	
	return err;
}

// MaxBIM �ֵ�� ����
GSErrCode	showAbout (void)
{
	GSErrCode	err = NoError;
	short	result;

	result = DGBlankModalDialog (550, 400, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, aboutHandler, 0);

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
			ACAPI_RegisterModelessWindow (dialogID, APIHelpPaletteAPIControlCallBack,
							API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation + API_PalEnabled_InteriorElevation +
							API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_Layout + API_PalEnabled_3D);

			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "�ֵ�� ����");

			// �ݱ� ��ư
			DGSetItemText (dialogID, DG_OK, "�ݱ�");

			// ��: ������ ��ġ
			DGSetItemText (dialogID, LABEL_PLACING_EUROFORM, "������ ��ġ");

			// ��ư: ��
			DGSetItemText (dialogID, BUTTON_WALL, "��");

			// ��ư: ������
			DGSetItemText (dialogID, BUTTON_SLAB, "������");

			// ��ư: ��
			DGSetItemText (dialogID, BUTTON_BEAM, "��");

			// ��ư: ���
			DGSetItemText (dialogID, BUTTON_COLUMN, "���");

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
					DGModelessClose (paletteID);
					DGDestroyModelessDialog (paletteID);
					ACAPI_UnregisterModelessWindow (paletteID);
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
					DGSetItemText (dialogID, itmIdx, "�������� ���� ���鿡 ���簢�� ������ �׸���,\n���ϴܺ��� �������� �׸��ʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "������ �ֻ�� ������ ���� ��� ���ΰ� ��ġ�ؾ�\n�մϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "���鿡 �ܿ��簡 ������ ������ �ܿ��� ���� �׸���\n�˴ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "�� �ֺ��� ���� ó���� �Ϸ��� ���鵵 ����\n�����Ͻʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 380, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "���� ��Ҹ� ������ �� �����Ͻʽÿ�:\n����, ��, (�� �ټ�)");
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
					DGSetItemText (dialogID, itmIdx, "�������� ���� ������ �Ϻο� ������ �׸��ʽÿ�.\n��� � ���� �ڳʰ� ���� �� �ֽ��ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "������ �Ϻο� �ܿ��簡 ������ ������ �ܿ��� ����\n�׸��� �˴ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "���� �� ���� �ϴ��� ����, ���� ���� �������\nŬ���Ͻʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "���� ��Ҹ� ������ �� �����Ͻʽÿ�: ����, ������");
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
					DGSetItemText (dialogID, itmIdx, "�������� ���� �� ���� ��ü Ȥ�� �Ϻο� ������\n�׸��ʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "�� ��ü�� ���, ���� ���� �ν��Ϸ��� ���� ����\n�����ؾ� �մϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "�� ��ü�� ���, ���� ���� 1���� �νĵǹǷ� ���� ����\n������ �� �Ϻη� ������ �۾��Ͻʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "���� �� ������ ������, ������ �������\nŬ���Ͻʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 380, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "�� ����/�Ϻο� �ܿ��簡 ������ ������ �ܿ��� ����\n�׸��� �˴ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 410, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "���� ��Ҹ� ������ �� �����Ͻʽÿ�: ����, ���κ�");
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
					DGSetItemText (dialogID, itmIdx, "��� ������ ������, ������ ���� ������ ������\n�׸��ʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 290, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "����� ���, �ܿ��縦 ������� �ʽ��ϴ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 320, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "��տ� ���� �� ��ó�� ������ ��ġ�ϱ� ���� ���� ����\n�����Ͻʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 350, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "����� ���� �ٰų� ħ���ϴ� ���, ���� ����\n�����Ͻʽÿ�.");
					DGShowItem (dialogID, itmIdx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 110, 380, 300, 23);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, "���� ��Ҹ� ������ �� �����Ͻʽÿ�:\n����, ���, (�� �ټ�), (��)");
					DGShowItem (dialogID, itmIdx);

					break;
			}
		case DG_MSG_CLOSE:
			ACAPI_UnregisterModelessWindow (paletteID);
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
			DGSetDialogTitle (dialogID, "MaxBIM �ֵ�� ����");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 215, 360, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			itmPosX = 20;
			itmPosY = 20;

			// ��: ���� (�ֱ� ������)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 250, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "������: 2021.07.12");
			DGShowItem (dialogID, itmIdx);
			itmPosY += 30;

			// ��: �ۼ��� (�̸���)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 250, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�ۼ���: ������ (canvas84@naver.com)");
			DGShowItem (dialogID, itmIdx);
			itmPosY += 30;

			// ��: ���α׷� ���
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 250, 23*10);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "1. ������ ��ġ\n - ���� ������ ��ġ\n - ������ �Ϻο� ������ ��ġ\n - ���� ������ ��ġ\n - ��տ� ������ ��ġ\n\n2. ���̺��� ��ġ\n - ���� ���̺��� ��ġ - ���� ����\n - ���� ���̺��� ��ġ - ���� ����\n - ������ �Ϻο� ���̺��� ��ġ\n\n3. Library Converting\n - ���� ������ ��� ��ȯ\n\n4. ���̾� ��ƿ\n - ���̾� ���� �����ϱ�\n - ���̾� ���� �����\n - ���̾� ���� �����ϱ�");
			DGShowItem (dialogID, itmIdx);

			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 270, itmPosY, 250, 23*10);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "5. ��������\n - ����(���,��,������) ���� �������� (CSV) (���ߺ���)\n - ������ ���� ���� �������� (Single ���)\n - ������ ���� ���� �������� (Multi ���)\n - ���纰 ���� �� �����ֱ�\n\n6. ���� ����\n - �������� �����ϱ�");
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
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// ��޸��� â�� �����ϱ� ���� �ݹ��Լ�
static GSErrCode __ACENV_CALL	APIHelpPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	if (referenceID == paletteID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGEndProcessEvents (paletteID);
												DGModelessClose (paletteID);
												DGDestroyModelessDialog (paletteID);
												ACAPI_UnregisterModelessWindow (paletteID);
												break;
			case APIPalMsg_HidePalette_Begin:	DGHideModelessDialog (paletteID);
												break;
			case APIPalMsg_HidePalette_End:		DGShowModelessDialog (paletteID, DG_DF_FIRST);
												break;
			case APIPalMsg_DisableItems_Begin:
			case APIPalMsg_DisableItems_End:	// actually do nothing, because the input finish functionality the buttons have to stay enabled
			case APIPalMsg_IsPaletteVisible:
			case APIPalMsg_OpenPalette:			break;
		}
	}

	return NoError;
}