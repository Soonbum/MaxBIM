#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Facility.hpp"
#include "Export.hpp"

using namespace FacilityDG;

// 3D ǰ��/�ӵ� �����ϱ�
GSErrCode	select3DQuality (void)
{
	GSErrCode	err = NoError;

	short	result;
	double	gs_resol;
	long	nElems;

	bool	bSuccess;
	long	ElemsChanged = 0;
	long	ElemsUnchanged = 0;

	GS::Array<API_Guid> elemList;
	API_Element			elem;
	API_ElementMemo		memo;

	// �׷�ȭ �Ͻ����� ON
	suspendGroups (true);

	result = DGAlert (DG_INFORMATION, L"3D ǰ��/�ӵ� �����ϱ�", L"3D ǰ���� �����Ͻʽÿ�", "", L"����-��ǰ��(32)", L"�߰�(12)", L"����-��ǰ��(4)");

	if (result == 1)		gs_resol = 32.0;
	else if (result == 2)	gs_resol = 12.0;
	else					gs_resol = 4.0;

	result = DGAlert (DG_WARNING, L"3D ǰ��/�ӵ� �����ϱ�", L"��� �����Ͻðڽ��ϱ�?", "", L"��", L"�ƴϿ�", "");

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

				if (bSuccess == true)
					ElemsChanged ++;
				else
					ElemsUnchanged ++;
			}
			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}

	elemList.Clear ();

	WriteReport_Alert ("����� ��ü ����: %d\n������� ���� ��ü ����: %d", ElemsChanged, ElemsUnchanged);

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups (false);

	return	err;
}

// ������ 3D �� ���̱�
GSErrCode	attach3DLabelOnZone (void)
{
	GSErrCode	err = NoError;

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

	// �׷�ȭ �Ͻ����� ON
	suspendGroups (true);

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
					GS::Array<API_Element>	elems;
					elems.Push (elem);
					deleteElements (elems);

					ElemsDeleted ++;
				}
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}

	elemList.Clear ();
	
	// ���ο� �� ��ü�� ��ġ�� (���̾� �̸� ����)
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32510, ACAPI_GetOwnResModule (), selectLayerHandler, (DGUserData) &layerInd);

	if (result == DG_CANCEL) {
		WriteReport_Alert ("���ŵ� �� ����: %ld\n�߰��� �� ����: %ld", ElemsDeleted, ElemsAdded);
		return err;
	}

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
			label.init (L("��v1.0.gsm"), layerInd, elem.header.floorInd, elem.zone.pos.x, elem.zone.pos.y, elem.zone.roomBaseLev + 1.000, DegreeToRad (0.0));

			sprintf (roomName, "%s", (const char *) GS::UniString (elem.zone.roomName).ToCStr ());

			label.placeObject (13,
				"scaleValues", APIParT_CString, "�����Ͽ� �°� (�� ũ��)",
				"iScaleValues", APIParT_Integer, "1",
				"bShowOn2D", APIParT_Boolean, "0",
				"bShowOn3D", APIParT_Boolean, "1",
				"bLocalOrigin", APIParT_Boolean, "0",
				"szFont", APIParT_Length, format_string ("%f", 2.000),
				"bCoords", APIParT_Boolean, "0",
				"bComment", APIParT_Boolean, "1",
				"txtComment", APIParT_CString, roomName,
				"gs_cont_pen", APIParT_PenCol, "20",
				"textMat", APIParT_Mater, "19",
				"bBg", APIParT_Boolean, "0",
				"thkTxt", APIParT_Length, "0.050");

			ElemsAdded ++;
		}
	}

	elemList.Clear ();

	WriteReport_Alert ("���ŵ� �� ����: %ld\n�߰��� �� ����: %ld", ElemsDeleted, ElemsAdded);

	// �׷�ȭ �Ͻ����� OFF
	suspendGroups (false);

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
			DGSetDialogTitle (dialogID, L"3D ���� ���̾� �����ϱ�");

			// Ȯ�� ��ư
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"Ȯ��");

			// ��� ��ư
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"���");

			// ��
			DGSetItemFont (dialogID, 3, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 3, L"��� ����(Zone)�� �� ��ü�� ��ġ�Ͻðڽ��ϱ�?");

			DGSetItemFont (dialogID, 5, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 5, L"���纰 ���̾� ����");

			DGSetItemFont (dialogID, 6, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 6, L"��");

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

// ���� ��鵵�� ���̺����� ���� �ڵ� ��ġ
GSErrCode	attachBubbleOnCurrentFloorPlan (void)
{
	GSErrCode	err = NoError;
	short	result;
	short	xx, yy, mm;
	API_StoryInfo	storyInfo;
	CircularBubble	cbInfo;		// ���� ���� ���� ����
	double	xMin, xMax;
	double	yMin, yMax;
	double	zMin, zMax;

	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	long					nElems = 0;
	long					nObjects = 0;

	// ������ ��ҵ��� ���� ����ϱ�
	API_Element			elem;
	API_ElemInfo3D		info3D;

	// ���̾� ���� ����
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// ������ ��ü���� ���̾� �ε������� ������
	bool	bIndexFound;
	vector<short>	selectedLayerIndex;

	// ���̾� �̸����� ���̻縦 �������� ���� ����
	char	bubbleText [64];
	char	strEnd [32];
	char	strEndBefore [32];
	char	*token;			// �о�� ���ڿ��� ��ū

	// ����ٸ� ǥ���ϱ� ���� ����
	GS::UniString       title ("���� ��ġ ���� ��Ȳ");
	GS::UniString       subtitle ("������...");
	short	nPhase;
	Int32	cur, total;


	// [1�� ���̾�α�] ���� ���� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32511, ACAPI_GetOwnResModule (), setBubbleHandler, (DGUserData) &cbInfo);

	// ������ ��ü���� �ִٸ� ��ü���� ���̾� �ε������� ������
	getGuidsOfSelection (&elemList, API_ObjectID, &nElems);
	for (xx = 0 ; xx < nElems ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = elemList.Pop ();
		err = ACAPI_Element_Get (&elem);

		bIndexFound = false;
		for (yy = 0 ; yy < selectedLayerIndex.size () ; ++yy) {
			if (selectedLayerIndex [yy] == elem.header.layer)
				bIndexFound = true;
		}
		if (bIndexFound == false)
			selectedLayerIndex.push_back (elem.header.layer);
	}
	elemList.Clear ();
	nElems = 0;

	// ���� Ȱ�� ���� �ε����� ������
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	cbInfo.floorInd = storyInfo.actStory;
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	nLayers = getLayerCount ();

	// ���̴� ���̾���� ��� �����ϱ�
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList [nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// ���̾� �̸��� �ε��� ����
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// ���̾� �̸� �������� �����Ͽ� ���̾� �ε��� ���� ����
	sort (layerList.begin (), layerList.end (), compareLayerName);		// ���̾� �̸� ���� �������� ����

	// �Ͻ������� ��� ���̾� �����
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	// ���� ��Ȳ ǥ���ϴ� ��� - �ʱ�ȭ
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

	// ���̴� ���̾���� �ϳ��� ��ȸ�ϸ鼭 ��ü ��ҵ��� ������ �� ���̺������� ���� ������ ��ġ��
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			// ���̾� ���̱�
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// ���̾� �̸� ������
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// ����� ��ü ���� �ʱ�ȭ
			nElems = 0;
			nObjects = 0;
			elemList.Clear ();
			objects.Clear ();

			// ���� ���̾��� ��� ��ü���� ������
			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����, ��ü Ÿ�Ը�
			nElems = elemList.GetSize ();

			// ���� ��, ������ ��ü�� ���� ��ü�鸸 �ɷ���
			for (xx = 0 ; xx < nElems ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				elem.header.guid = elemList.Pop ();
				err = ACAPI_Element_Get (&elem);

				if ((elem.header.floorInd == cbInfo.floorInd) && (elem.header.layer == attrib.layer.head.index)) {
					bIndexFound = false;
					for (yy = 0 ; yy < selectedLayerIndex.size () ; ++yy) {
						if (elem.header.layer == selectedLayerIndex [yy])
							bIndexFound = true;
					}
					if (bIndexFound == true)
						objects.Push (elem.header.guid);
				}
			}
			nObjects = objects.GetSize ();

			// ���� ���� ���� ��ü�� ���� ��� ��ŵ
			if (nObjects == 0)
				continue;

			// ���� ��, ���� ���̾ �ִ� ��� ��ü���� 3D �ڽ� ������ ������
			for (xx = 0 ; xx < nObjects ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				elem.header.guid = objects.Pop ();
				err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

				if (xx == 0) {
					xMin = info3D.bounds.xMin;	xMax = info3D.bounds.xMax;
					yMin = info3D.bounds.yMin;	yMax = info3D.bounds.yMax;
					zMin = info3D.bounds.zMin;	zMax = info3D.bounds.zMax;
				} else {
					if (xMin > info3D.bounds.xMin)	xMin = info3D.bounds.xMin;
					if (xMax < info3D.bounds.xMax)	xMax = info3D.bounds.xMax;

					if (yMin > info3D.bounds.yMin)	yMin = info3D.bounds.yMin;
					if (yMax < info3D.bounds.yMax)	yMax = info3D.bounds.yMax;

					if (zMin > info3D.bounds.zMin)	zMin = info3D.bounds.zMin;
					if (zMax < info3D.bounds.zMax)	zMax = info3D.bounds.zMax;
				}
			}

			// ���̾� �̸����κ��� ������ �ؽ�Ʈ�� ����
			strcpy (strEnd, "\0");
			strcpy (strEndBefore, "\0");
			token = strtok (fullLayerName, "-");
			while (token != NULL) {
				strcpy (strEndBefore, strEnd);	// ������ ���̻縦 ������ ���� ���̻�� ����
				strcpy (strEnd, token);			// ������ ���̻縦 ����
				token = strtok (NULL, "-");
			}

			// ���� ���� �� �ؽ�Ʈ�� ������
			if ((isStringDouble (strEndBefore) == TRUE) && (isStringDouble (strEnd) == TRUE)) {
				// ������ ���� ���̻簡 ����, ������ ���̻簡 �����̸�?
				sprintf (bubbleText, "%d-%d", atoi (strEndBefore), atoi (strEnd));
			} else if ((isStringDouble (strEndBefore) == FALSE) && (isStringDouble (strEnd) == TRUE)) {
				// ������ ���� ���̻簡 ����, ������ ���̻簡 �����̸�?
				sprintf (bubbleText, "%d", atoi (strEnd));
			} else {
				// ������ ���̻縦 ����
				strcpy (bubbleText, strEnd);
			}
			
			// ���� ���� ��ġ
			EasyObjectPlacement	bubble;
			if (cbInfo.pos == UP) {
				bubble.init (L("��������v1.0.gsm"), cbInfo.layerInd, cbInfo.floorInd, xMin + (xMax - xMin)/2, yMax + cbInfo.lenWithdrawal + cbInfo.szBubbleDia/2, 0.0, DegreeToRad (0.0));
				bubble.placeObject (14,
					"bShowOn2D", APIParT_Boolean, "1.0",
					"bShowOn3D", APIParT_Boolean, "1.0",
					"szBubbleDia", APIParT_Length, format_string ("%f", cbInfo.szBubbleDia),
					"szFont", APIParT_Length, format_string ("%f", cbInfo.szFont),
					"bWithdrawal", APIParT_Boolean, "1.0",
					"lenWithdrawal", APIParT_Length, format_string ("%f", cbInfo.lenWithdrawal),
					"angWithdrawal", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)),
					"gs_cont_pen", APIParT_PenCol, "19",
					"gs_fill_pen", APIParT_PenCol, "19",
					"gs_back_pen", APIParT_PenCol, "19",
					"textMat", APIParT_Mater, "49",
					"bgMat", APIParT_Mater, "60",
					"txtComment", APIParT_CString, bubbleText,
					"thkTxt", APIParT_Length, "0.005");
			} else if (cbInfo.pos == DOWN) {
				bubble.init (L("��������v1.0.gsm"), cbInfo.layerInd, cbInfo.floorInd, xMin + (xMax - xMin)/2, yMin - cbInfo.lenWithdrawal - cbInfo.szBubbleDia/2, 0.0, DegreeToRad (0.0));
				bubble.placeObject (14,
					"bShowOn2D", APIParT_Boolean, "1.0",
					"bShowOn3D", APIParT_Boolean, "1.0",
					"szBubbleDia", APIParT_Length, format_string ("%f", cbInfo.szBubbleDia),
					"szFont", APIParT_Length, format_string ("%f", cbInfo.szFont),
					"bWithdrawal", APIParT_Boolean, "1.0",
					"lenWithdrawal", APIParT_Length, format_string ("%f", cbInfo.lenWithdrawal),
					"angWithdrawal", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
					"gs_cont_pen", APIParT_PenCol, "19",
					"gs_fill_pen", APIParT_PenCol, "19",
					"gs_back_pen", APIParT_PenCol, "19",
					"textMat", APIParT_Mater, "49",
					"bgMat", APIParT_Mater, "60",
					"txtComment", APIParT_CString, bubbleText,
					"thkTxt", APIParT_Length, "0.005");
			} else if (cbInfo.pos == LEFT) {
				bubble.init (L("��������v1.0.gsm"), cbInfo.layerInd, cbInfo.floorInd, xMin - cbInfo.lenWithdrawal - cbInfo.szBubbleDia/2, yMin + (yMax - yMin)/2, 0.0, DegreeToRad (0.0));
				bubble.placeObject (14,
					"bShowOn2D", APIParT_Boolean, "1.0",
					"bShowOn3D", APIParT_Boolean, "1.0",
					"szBubbleDia", APIParT_Length, format_string ("%f", cbInfo.szBubbleDia),
					"szFont", APIParT_Length, format_string ("%f", cbInfo.szFont),
					"bWithdrawal", APIParT_Boolean, "1.0",
					"lenWithdrawal", APIParT_Length, format_string ("%f", cbInfo.lenWithdrawal),
					"angWithdrawal", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"gs_cont_pen", APIParT_PenCol, "19",
					"gs_fill_pen", APIParT_PenCol, "19",
					"gs_back_pen", APIParT_PenCol, "19",
					"textMat", APIParT_Mater, "49",
					"bgMat", APIParT_Mater, "60",
					"txtComment", APIParT_CString, bubbleText,
					"thkTxt", APIParT_Length, "0.005");
			} else if (cbInfo.pos == RIGHT) {
				bubble.init (L("��������v1.0.gsm"), cbInfo.layerInd, cbInfo.floorInd, xMax + cbInfo.lenWithdrawal + cbInfo.szBubbleDia/2, yMin + (yMax - yMin)/2, 0.0, DegreeToRad (0.0));
				bubble.placeObject (14,
					"bShowOn2D", APIParT_Boolean, "1.0",
					"bShowOn3D", APIParT_Boolean, "1.0",
					"szBubbleDia", APIParT_Length, format_string ("%f", cbInfo.szBubbleDia),
					"szFont", APIParT_Length, format_string ("%f", cbInfo.szFont),
					"bWithdrawal", APIParT_Boolean, "1.0",
					"lenWithdrawal", APIParT_Length, format_string ("%f", cbInfo.lenWithdrawal),
					"angWithdrawal", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)),
					"gs_cont_pen", APIParT_PenCol, "19",
					"gs_fill_pen", APIParT_PenCol, "19",
					"gs_back_pen", APIParT_PenCol, "19",
					"textMat", APIParT_Mater, "49",
					"bgMat", APIParT_Mater, "60",
					"txtComment", APIParT_CString, bubbleText,
					"thkTxt", APIParT_Length, "0.005");
			}

			// ���̾� �����
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}

		// ���� ��Ȳ ǥ���ϴ� ��� - ����
		cur = mm;
		ACAPI_Interface (APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// ���� ��Ȳ ǥ���ϴ� ��� - ������
	ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

	// ��� ���μ����� ��ġ�� ó���� �����ߴ� ���̴� ���̾���� �ٽ� �ѳ��� ��
	for (xx = 1 ; xx <= nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx-1];
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}
		}
	}

	return	err;
}

// [���̾�α� �ڽ�] ���� ���� ����
short DGCALLBACK setBubbleHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	short	result;
	CircularBubble	*cbInfo = (CircularBubble*) userData;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"���� ���� �����ϱ�");

			// Ȯ�� ��ư
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"Ȯ��");

			// ��� ��ư
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"���");

			// ���� ����
			DGSetItemFont (dialogID, LABEL_DIAMETER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DIAMETER, L"���� ����");
			DGSetItemValDouble (dialogID, EDITCONTROL_DIAMETER, 0.550);

			// ���� ũ��
			DGSetItemFont (dialogID, LABEL_LETTER_SIZE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LETTER_SIZE, L"���� ũ��");
			DGSetItemValDouble (dialogID, EDITCONTROL_LETTER_SIZE, 0.200);

			// ���⼱ ����
			DGSetItemFont (dialogID, LABEL_WITHDRAWAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WITHDRAWAL_LENGTH, L"���⼱ ����");
			DGSetItemValDouble (dialogID, EDITCONTROL_WITHDRAWAL_LENGTH, 0.700);

			// ���� ��ġ
			DGSetItemFont (dialogID, LABEL_BUBBLE_POS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_BUBBLE_POS, L"���� ��ġ");
			// �˾� ��Ʈ���� �׸��� �ѱ۷� ��ȯ
			DGPopUpSetItemText (dialogID, POPUPCONTROL_BUBBLE_POS, 1, L"��");
			DGPopUpSetItemText (dialogID, POPUPCONTROL_BUBBLE_POS, 2, L"��");
			DGPopUpSetItemText (dialogID, POPUPCONTROL_BUBBLE_POS, 3, L"��");
			DGPopUpSetItemText (dialogID, POPUPCONTROL_BUBBLE_POS, 4, L"��");

			// ���̾� �� ���� ��Ʈ�� �ʱ�ȭ
			DGSetItemFont (dialogID, LABEL_LAYER_SETTINGS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, L"���̾� ����");
			DGSetItemFont (dialogID, LABEL_LAYER_CIRCULAR_BUBBLE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LAYER_CIRCULAR_BUBBLE, L"���� ����");
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_CIRCULAR_BUBBLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CIRCULAR_BUBBLE, 1);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					cbInfo->layerInd = (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CIRCULAR_BUBBLE);
					cbInfo->lenWithdrawal = DGGetItemValDouble (dialogID, EDITCONTROL_WITHDRAWAL_LENGTH);
					cbInfo->pos = (short)DGPopUpGetSelected (dialogID, POPUPCONTROL_BUBBLE_POS);
					cbInfo->szBubbleDia = DGGetItemValDouble (dialogID, EDITCONTROL_DIAMETER);
					cbInfo->szFont = DGGetItemValDouble (dialogID, EDITCONTROL_LETTER_SIZE);
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
