#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Facility.hpp"

// 3D ǰ��/�ӵ� �����ϱ�
GSErrCode	select3DQuality (void)
{
	GSErrCode	err = NoError;

	bool	suspGrp;
	short	result;
	double	gs_resol;
	long	nElems;

	bool	bSuccess;
	long	ElemsChanged = 0;
	long	ElemsUnchanged = 0;

	GS::Array<API_Guid> elemList;
	API_Element			elem;
	API_ElementMemo		memo;

	// �׷�ȭ �Ͻ�����
	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
	if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	result = DGAlert (DG_INFORMATION, "3D ǰ��/�ӵ� �����ϱ�", "3D ǰ���� �����Ͻʽÿ�", "", "����-��ǰ��(32)", "�߰�(12)", "����-��ǰ��(4)");

	if (result == 1)		gs_resol = 32.0;
	else if (result == 2)	gs_resol = 12.0;
	else					gs_resol = 4.0;

	result = DGAlert (DG_WARNING, "3D ǰ��/�ӵ� �����ϱ�", "��� �����Ͻðڽ��ϱ�?", "", "��", "�ƴϿ�", "");

	if (result == DG_CANCEL)
		return err;

	// ��� ��ü�� �ҷ���
	ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);
	nElems = elemList.GetSize ();

	for (short xx = 0 ; xx < nElems ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList [xx];
		err = ACAPI_Element_Get (&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
				bSuccess = setParameterByName (&memo, "gs_resol", gs_resol);	// �ػ� ���� ������

				ACAPI_Element_Change (&elem, NULL, &memo, APIMemoMask_AddPars, true);
				ACAPI_DisposeElemMemoHdls (&memo);

				if (bSuccess == true)
					ElemsChanged ++;
				else
					ElemsUnchanged ++;
			}
		}
	}

	elemList.Clear ();

	WriteReport_Alert ("����� ��ü ����: %d\n������� ���� ��ü ����: %d", ElemsChanged, ElemsUnchanged);

	return	err;
}

// ������ 3D �� ���̱�
GSErrCode	attach3DLabelOnZone (void)
{
	GSErrCode	err = NoError;

	bool	suspGrp;
	short	result;
	long	nElems;
	short	layerInd;
	char	roomName [256];
	const char*	foundStr;

	long	ElemsAdded = 0;
	long	ElemsDeleted = 0;

	GS::Array<API_Guid>	elemList;
	API_Element			elem;
	API_ElementMemo		memo;

	// �׷�ȭ �Ͻ�����
	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
	if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	// ���� �� ��ü�� ��� ����
	result = DGAlert (DG_WARNING, "���� �� ��ü ��� ����", "���� �� ��ü�� ��� �����Ͻðڽ��ϱ�?", "", "��", "�ƴϿ�", "");

	ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_InMyWorkspace);
	nElems = elemList.GetSize ();

	for (long xx = 0 ; xx < nElems ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList [xx];
		err = ACAPI_Element_Get (&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
				foundStr = getParameterStringByName (&memo, "d_comp");
				if (my_strcmp (foundStr, "��ǥ") == 0) {
					API_Elem_Head* headList = new API_Elem_Head [1];
					headList [0] = elem.header;
					ACAPI_Element_Delete (&headList, 1);
					delete headList;

					ElemsDeleted ++;
				}
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}

	elemList.Clear ();
	
	// ���ο� �� ��ü�� ��ġ�� (���̾� �̸� ����)
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32510, ACAPI_GetOwnResModule (), selectLayerHandler, (DGUserData) &layerInd);

	if (result == DG_CANCEL)
		goto EXIT;

	// ��� ����(Zone) ��ü�� �ҷ���
	ACAPI_Element_GetElemList (API_ZoneID , &elemList, APIFilt_InMyWorkspace);
	nElems = elemList.GetSize ();

	EasyObjectPlacement	label;

	for (long xx = 0 ; xx < nElems ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = elemList [xx];
		err = ACAPI_Element_Get (&elem);

		if (err == NoError) {
			// ����(Zone) ��ü�� ������ ������� �� ��ġ
			label.init (L("��v1.0.gsm"), layerInd, elem.header.floorInd, elem.zone.pos.x, elem.zone.pos.y, elem.zone.roomBaseLev + elem.zone.roomHeight/2, DegreeToRad (0.0));

			sprintf (roomName, "%s", (const char *) GS::UniString (elem.zone.roomName).ToCStr ());

			label.placeObject (9,
				"scaleValues", APIParT_CString, "�����Ͽ� �°� (�� ũ��)",
				"iScaleValues", APIParT_Integer, "1",
				"bShowOn2D", APIParT_Boolean, "0",
				"bShowOn3D", APIParT_Boolean, "1",
				"bLocalOrigin", APIParT_Boolean, "0",
				"szFont", APIParT_Length, format_string ("%f", 1.000),
				"bCoords", APIParT_Boolean, "0",
				"bComment", APIParT_Boolean, "1",
				"txtComment", APIParT_CString, roomName);

			ElemsAdded ++;
		}
	}

	elemList.Clear ();

EXIT:
	WriteReport_Alert ("���ŵ� �� ����: %ld\n�߰��� �� ����: %ld", ElemsDeleted, ElemsAdded);

	return	err;
}

// [���̾�α� �ڽ�] ���̾� ����
short DGCALLBACK selectLayerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	short	result;
	short	*layerInd = (short*) userData;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "3D ���� ���̾� �����ϱ�");

			// Ȯ�� ��ư
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");

			// ��� ��ư
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");

			// ��
			DGSetItemFont (dialogID, 3, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 3, "��� ����(Zone)�� �� ��ü�� ��ġ�Ͻðڽ��ϱ�?");

			DGSetItemFont (dialogID, 5, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 5, "���纰 ���̾� ����");

			DGSetItemFont (dialogID, 6, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 6, "��");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = 7;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, 7, 1);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					*layerInd = (short)DGGetItemValLong (dialogID, 7);
					break;

				case DG_CANCEL:
					break;
			}
			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}