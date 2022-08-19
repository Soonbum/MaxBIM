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

				if (bSuccess == true) {
					ElemsChanged ++;
					ACAPI_Element_Change (&elem, NULL, &memo, APIMemoMask_AddPars, true);
				}
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

			// ���� ũ��
			DGSetItemFont (dialogID, LABEL_LETTER_SIZE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LETTER_SIZE, L"���� ũ��");

			// ���⼱ ����
			DGSetItemFont (dialogID, LABEL_WITHDRAWAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WITHDRAWAL_LENGTH, L"���⼱ ����");

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

			// ������ ������ ���� �ε���
			if (loadDialogStatus_bubble (cbInfo) == NoError) {
				// �ε�� ��
				DGSetItemValDouble (dialogID, EDITCONTROL_DIAMETER, cbInfo->szBubbleDia);				// ���� ����
				DGSetItemValDouble (dialogID, EDITCONTROL_LETTER_SIZE, cbInfo->szFont);					// ���� ũ��
				DGSetItemValDouble (dialogID, EDITCONTROL_WITHDRAWAL_LENGTH, cbInfo->lenWithdrawal);	// ���⼱ ����
				DGSetItemValLong (dialogID, USERCONTROL_LAYER_CIRCULAR_BUBBLE, cbInfo->layerInd);		// ���̾�
			} else {
				// �ʱⰪ
				DGSetItemValDouble (dialogID, EDITCONTROL_DIAMETER, 0.550);				// ���� ����
				DGSetItemValDouble (dialogID, EDITCONTROL_LETTER_SIZE, 0.200);			// ���� ũ��
				DGSetItemValDouble (dialogID, EDITCONTROL_WITHDRAWAL_LENGTH, 0.700);	// ���⼱ ����
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					cbInfo->layerInd = (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CIRCULAR_BUBBLE);
					cbInfo->lenWithdrawal = DGGetItemValDouble (dialogID, EDITCONTROL_WITHDRAWAL_LENGTH);
					cbInfo->pos = (short)DGPopUpGetSelected (dialogID, POPUPCONTROL_BUBBLE_POS);
					cbInfo->szBubbleDia = DGGetItemValDouble (dialogID, EDITCONTROL_DIAMETER);
					cbInfo->szFont = DGGetItemValDouble (dialogID, EDITCONTROL_LETTER_SIZE);

					// ���������� �Է��� ���� ������
					saveDialogStatus_bubble (cbInfo);

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

// ���� ���� ���� ���� ����
GSErrCode	saveDialogStatus_bubble (CircularBubble	*cbInfo)
{
	GSErrCode err = NoError;
	CircularBubble	cbInfoSaved;	// cbInfo�� ���� ���¸� ������ ����

	API_ModulData	info;
	BNZeroMemory (&info, sizeof (API_ModulData));
	info.dataVersion = 1;
	info.platformSign = GS::Act_Platform_Sign;
	info.dataHdl = BMAllocateHandle (sizeof (cbInfoSaved), 0, 0);
	if (info.dataHdl != NULL) {

		cbInfoSaved.layerInd		= cbInfo->layerInd;
		cbInfoSaved.lenWithdrawal	= cbInfo->lenWithdrawal;
		cbInfoSaved.pos				= cbInfo->pos;
		cbInfoSaved.szBubbleDia		= cbInfo->szBubbleDia;
		cbInfoSaved.szFont			= cbInfo->szFont;

		*(reinterpret_cast<CircularBubble*> (*info.dataHdl)) = cbInfoSaved;
		err = ACAPI_ModulData_Store (&info, "DialogStatus_bubble");
		BMKillHandle (&info.dataHdl);
	} else {
		err = APIERR_MEMFULL;
	}

	return	err;
}

// ���� ���� ���� ���� �ε�
GSErrCode	loadDialogStatus_bubble (CircularBubble	*cbInfo)
{
	GSErrCode err = NoError;
	CircularBubble	cbInfoSaved;	// cbInfo�� ���� ���°� ����� ����

	API_ModulData	info;
	BNZeroMemory (&info, sizeof (API_ModulData));
	err = ACAPI_ModulData_Get (&info, "DialogStatus_bubble");

	if (err == NoError && info.dataVersion == 1) {
		cbInfoSaved = *(reinterpret_cast<CircularBubble*> (*info.dataHdl));

		cbInfo->layerInd		= cbInfoSaved.layerInd;
		cbInfo->lenWithdrawal	= cbInfoSaved.lenWithdrawal;
		cbInfo->pos				= cbInfoSaved.pos;
		cbInfo->szBubbleDia		= cbInfoSaved.szBubbleDia;
		cbInfo->szFont			= cbInfoSaved.szFont;
	}

	BMKillHandle (&info.dataHdl);

	return	err;
}

// ī�޶� ��ġ �����ϱ�/�ҷ�����
GSErrCode	manageCameraInfo (void)
{
	GSErrCode	err = NoError;
	short	result;

	result = DGBlankModalDialog (500, 540, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, cameraPosManagerHandler, 0);

	return	err;
}

// [���̾�α� �ڽ�] ī�޶� ��ġ �����ϱ�
short DGCALLBACK cameraPosManagerHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;

	sqlite3 *db = NULL;
	sqlite3_stmt* res;
	char *err_msg = NULL;
	int rc = 0;
	char *sql = NULL;
	char comment [512];

	API_3DProjectionInfo	currentCamera;
	bool					regenerate = true;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"ī�޶� ��ġ �����ϱ�");

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 460, 480, 30);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"ī�޶� ��ġ �����ϱ�");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 390, 480, 30);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"ī�޶� ��ġ �����ϱ�");
			DGShowItem (dialogID, DG_CANCEL);

			// �ε� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 350, 480, 30);
			DGSetItemFont (dialogID, BUTTON_LOAD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_LOAD, L"ī�޶� ��ġ �ҷ�����");
			DGShowItem (dialogID, BUTTON_LOAD);

			// �ݱ� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 500, 480, 30);
			DGSetItemFont (dialogID, BUTTON_CLOSE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_CLOSE, L"â �ݱ�");
			DGShowItem (dialogID, BUTTON_CLOSE);

			// ����Ʈ ��
			DGAppendDialogItem (dialogID, DG_ITM_LISTVIEW, DG_LVT_SINGLESELECT, DG_LVVM_SINGLECOLUMN, 10, 10, 480, 330);
			DGSetItemFont (dialogID, LISTVIEW_CAMERA_POS_NAME, DG_IS_LARGE | DG_IS_PLAIN);
			DGListViewSetImageSize (dialogID, LISTVIEW_CAMERA_POS_NAME, 0, 0);
			DGShowItem (dialogID, LISTVIEW_CAMERA_POS_NAME);

			// Edit��Ʈ�� (ī�޶� ��ġ�� ���� �ڸ�Ʈ �Է�)
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_TEXT, 0, 10, 430, 480, 25);
			DGSetItemFont (dialogID, EDITCONTROL_CAMERA_POS_NAME, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_CAMERA_POS_NAME);

			// DB�κ��� ������ �о�ͼ� ����Ʈ �信 �߰���
			db = NULL;
			err_msg = NULL;
			rc = sqlite3_open ("cameraInfo.db", &db);

			if (rc != SQLITE_OK) {
				DGAlert (DG_ERROR, L"����", L"DB ���� ����.", "", L"Ȯ��", "", "");
				sqlite3_close (db);
				db = NULL;
			}

			sql = "SELECT * FROM Cameras;";
			rc = sqlite3_prepare_v2 (db, sql, -1, &res, 0);
			rc = sqlite3_step (res);
			while (rc == SQLITE_ROW) {
				// 1��° ���� �ؽ�Ʈ�� ����Ʈ �信 �߰�
				DGListViewInsertItem (dialogID, LISTVIEW_CAMERA_POS_NAME, DG_LIST_BOTTOM);
				sprintf (comment, "%s", sqlite3_column_text (res, 0));
				DGListViewSetItemText (dialogID, LISTVIEW_CAMERA_POS_NAME, DG_LIST_BOTTOM, comment);
				
				// ���� ���� �ҷ���
				rc = sqlite3_step (res);
			}

			sqlite3_close (db);
			db = NULL;

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:			// ���� ��ư
					db = NULL;
					err_msg = NULL;
					rc = sqlite3_open ("cameraInfo.db", &db);

					if (rc != SQLITE_OK) {
						DGAlert (DG_ERROR, L"����", L"DB ���� ����.", "", L"Ȯ��", "", "");
						sqlite3_close (db);
						db = NULL;
					}

					// ���̺��� ������ ���� ������
					sql = "CREATE TABLE IF NOT EXISTS Cameras(Comment TEXT PRIMARY KEY, isPersp INT, camGuid_f01 INT, camGuid_f02 INT, camGuid_f03 INT, camGuid_f04 INT, camGuid_f05 INT, camGuid_f06 INT, camGuid_f07 INT, camGuid_f08 INT, camGuid_f09 INT, camGuid_f10 INT, camGuid_f11 INT, actCamSet_f01 INT, actCamSet_f02 INT, actCamSet_f03 INT, actCamSet_f04 INT, actCamSet_f05 INT, actCamSet_f06 INT, actCamSet_f07 INT, actCamSet_f08 INT, actCamSet_f09 INT, actCamSet_f10 INT, actCamSet_f11 INT, persp_azimuth REAL, persp_SunAzimuth REAL, persp_sunAltitude REAL, persp_viewCone REAL, persp_rollAngle REAL, persp_distance REAL, persp_cameraZ REAL, persp_targetZ REAL, persp_posX REAL, persp_posY REAL, persp_targetX REAL, persp_targetY REAL, axono_azimuth REAL, axono_sunAzimuth REAL, axono_sunAltitude REAL, axono_projMod INT, axono_tranmat_01 REAL, axono_tranmat_02 REAL, axono_tranmat_03 REAL, axono_tranmat_04 REAL, axono_tranmat_05 REAL, axono_tranmat_06 REAL, axono_tranmat_07 REAL, axono_tranmat_08 REAL, axono_tranmat_09 REAL, axono_tranmat_10 REAL, axono_tranmat_11 REAL, axono_tranmat_12 REAL, axono_invtranmat_01 REAL, axono_invtranmat_02 REAL, axono_invtranmat_03 REAL, axono_invtranmat_04 REAL, axono_invtranmat_05 REAL, axono_invtranmat_06 REAL, axono_invtranmat_07 REAL, axono_invtranmat_08 REAL, axono_invtranmat_09 REAL, axono_invtranmat_10 REAL, axono_invtranmat_11 REAL, axono_invtranmat_12 REAL);";
					rc = sqlite3_exec (db, sql, 0, 0, &err_msg);

					// ���̺� �����ϰ��� �ϴ� �ڸ�Ʈ �ؽ�Ʈ�� �����ϴ��� Ȯ����
					sql = "SELECT * FROM Cameras WHERE Comment = ?;";
					rc = sqlite3_prepare_v2 (db, sql, -1, &res, 0);
					sqlite3_bind_text (res, 1, DGGetItemText (dialogID, EDITCONTROL_CAMERA_POS_NAME).ToCStr ().Get (), -1, SQLITE_STATIC);
					rc = sqlite3_step (res);
					rc = sqlite3_data_count (res);

					if (rc > 0) {
						// ���� �����ϸ�, �̹� �����ϰ� �ִٴ� �޽����� ������
						DGAlert (DG_WARNING, L"���", L"�ش� �ڸ�Ʈ�� �̹� �����մϴ�.", "", L"Ȯ��", "", "");
						sqlite3_finalize (res);
					} else {
						// ���� �������� ������, DB�� �ʵ带 �߰��ϰ� ����Ʈ �信�� �߰���
						ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &currentCamera);
						sql = "INSERT INTO Cameras VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
						rc = sqlite3_prepare_v2 (db, sql, -1, &res, 0);
						sqlite3_bind_text (res, 1, DGGetItemText (dialogID, EDITCONTROL_CAMERA_POS_NAME).ToCStr ().Get (), -1, SQLITE_STATIC);
						sqlite3_bind_int (res,  2, currentCamera.isPersp);
						sqlite3_bind_int (res,  3, currentCamera.camGuid.time_low);
						sqlite3_bind_int (res,  4, currentCamera.camGuid.time_mid);
						sqlite3_bind_int (res,  5, currentCamera.camGuid.time_hi_and_version);
						sqlite3_bind_int (res,  6, currentCamera.camGuid.clock_seq_hi_and_reserved);
						sqlite3_bind_int (res,  7, currentCamera.camGuid.clock_seq_low);
						sqlite3_bind_int (res,  8, currentCamera.camGuid.node [0]);
						sqlite3_bind_int (res,  9, currentCamera.camGuid.node [1]);
						sqlite3_bind_int (res, 10, currentCamera.camGuid.node [2]);
						sqlite3_bind_int (res, 11, currentCamera.camGuid.node [3]);
						sqlite3_bind_int (res, 12, currentCamera.camGuid.node [4]);
						sqlite3_bind_int (res, 13, currentCamera.camGuid.node [5]);
						sqlite3_bind_int (res, 14, currentCamera.actCamSet.time_low);
						sqlite3_bind_int (res, 15, currentCamera.actCamSet.time_mid);
						sqlite3_bind_int (res, 16, currentCamera.actCamSet.time_hi_and_version);
						sqlite3_bind_int (res, 17, currentCamera.actCamSet.clock_seq_hi_and_reserved);
						sqlite3_bind_int (res, 18, currentCamera.actCamSet.clock_seq_low);
						sqlite3_bind_int (res, 19, currentCamera.actCamSet.node [0]);
						sqlite3_bind_int (res, 20, currentCamera.actCamSet.node [1]);
						sqlite3_bind_int (res, 21, currentCamera.actCamSet.node [2]);
						sqlite3_bind_int (res, 22, currentCamera.actCamSet.node [3]);
						sqlite3_bind_int (res, 23, currentCamera.actCamSet.node [4]);
						sqlite3_bind_int (res, 24, currentCamera.actCamSet.node [5]);
						sqlite3_bind_double (res, 25, currentCamera.u.persp.azimuth);
						sqlite3_bind_double (res, 26, currentCamera.u.persp.sunAzimuth);
						sqlite3_bind_double (res, 27, currentCamera.u.persp.sunAltitude);
						sqlite3_bind_double (res, 28, currentCamera.u.persp.viewCone);
						sqlite3_bind_double (res, 29, currentCamera.u.persp.rollAngle);
						sqlite3_bind_double (res, 30, currentCamera.u.persp.distance);
						sqlite3_bind_double (res, 31, currentCamera.u.persp.cameraZ);
						sqlite3_bind_double (res, 32, currentCamera.u.persp.targetZ);
						sqlite3_bind_double (res, 33, currentCamera.u.persp.pos.x);
						sqlite3_bind_double (res, 34, currentCamera.u.persp.pos.y);
						sqlite3_bind_double (res, 35, currentCamera.u.persp.target.x);
						sqlite3_bind_double (res, 36, currentCamera.u.persp.target.y);
						sqlite3_bind_double (res, 37, currentCamera.u.axono.azimuth);
						sqlite3_bind_double (res, 38, currentCamera.u.axono.sunAzimuth);
						sqlite3_bind_double (res, 39, currentCamera.u.axono.sunAltitude);
						sqlite3_bind_int (res, 40, currentCamera.u.axono.projMod);
						sqlite3_bind_double (res, 41, currentCamera.u.axono.tranmat.tmx [0]);
						sqlite3_bind_double (res, 42, currentCamera.u.axono.tranmat.tmx [1]);
						sqlite3_bind_double (res, 43, currentCamera.u.axono.tranmat.tmx [2]);
						sqlite3_bind_double (res, 44, currentCamera.u.axono.tranmat.tmx [3]);
						sqlite3_bind_double (res, 45, currentCamera.u.axono.tranmat.tmx [4]);
						sqlite3_bind_double (res, 46, currentCamera.u.axono.tranmat.tmx [5]);
						sqlite3_bind_double (res, 47, currentCamera.u.axono.tranmat.tmx [6]);
						sqlite3_bind_double (res, 48, currentCamera.u.axono.tranmat.tmx [7]);
						sqlite3_bind_double (res, 49, currentCamera.u.axono.tranmat.tmx [8]);
						sqlite3_bind_double (res, 50, currentCamera.u.axono.tranmat.tmx [9]);
						sqlite3_bind_double (res, 51, currentCamera.u.axono.tranmat.tmx [10]);
						sqlite3_bind_double (res, 52, currentCamera.u.axono.tranmat.tmx [11]);
						sqlite3_bind_double (res, 53, currentCamera.u.axono.invtranmat.tmx [0]);
						sqlite3_bind_double (res, 54, currentCamera.u.axono.invtranmat.tmx [1]);
						sqlite3_bind_double (res, 55, currentCamera.u.axono.invtranmat.tmx [2]);
						sqlite3_bind_double (res, 56, currentCamera.u.axono.invtranmat.tmx [3]);
						sqlite3_bind_double (res, 57, currentCamera.u.axono.invtranmat.tmx [4]);
						sqlite3_bind_double (res, 58, currentCamera.u.axono.invtranmat.tmx [5]);
						sqlite3_bind_double (res, 59, currentCamera.u.axono.invtranmat.tmx [6]);
						sqlite3_bind_double (res, 60, currentCamera.u.axono.invtranmat.tmx [7]);
						sqlite3_bind_double (res, 61, currentCamera.u.axono.invtranmat.tmx [8]);
						sqlite3_bind_double (res, 62, currentCamera.u.axono.invtranmat.tmx [9]);
						sqlite3_bind_double (res, 63, currentCamera.u.axono.invtranmat.tmx [10]);
						sqlite3_bind_double (res, 64, currentCamera.u.axono.invtranmat.tmx [11]);
						rc = sqlite3_step (res);
						DGListViewInsertItem (dialogID, LISTVIEW_CAMERA_POS_NAME, DG_LIST_BOTTOM);
						DGListViewSetItemText (dialogID, LISTVIEW_CAMERA_POS_NAME, DG_LIST_BOTTOM, DGGetItemText (dialogID, EDITCONTROL_CAMERA_POS_NAME));
					}

					sqlite3_close (db);
					db = NULL;

					item = 0;
					break;

				case DG_CANCEL:		// ���� ��ư
					// ������ �׸��� ����Ʈ���� ������
					if (DGListViewGetSelected (dialogID, LISTVIEW_CAMERA_POS_NAME, 0) != 0) {
						db = NULL;
						err_msg = NULL;
						rc = sqlite3_open ("cameraInfo.db", &db);

						if (rc != SQLITE_OK) {
							DGAlert (DG_ERROR, L"����", L"DB ���� ����.", "", L"Ȯ��", "", "");
							sqlite3_close (db);
							db = NULL;
						}

						// ���̺��� ������ �ڸ�Ʈ�� ���� �ʵ带 ����
						sql = "DELETE FROM Cameras WHERE Comment = ?;";
						rc = sqlite3_prepare_v2 (db, sql, -1, &res, 0);
						sprintf (comment, "%s", DGListViewGetItemText (dialogID, LISTVIEW_CAMERA_POS_NAME, DGListViewGetSelected (dialogID, LISTVIEW_CAMERA_POS_NAME, 0)).ToCStr ().Get ());
						sqlite3_bind_text (res, 1, comment, -1, SQLITE_STATIC);
						rc = sqlite3_step (res);
						sqlite3_finalize (res);
						sqlite3_close (db);
						db = NULL;

						// ����Ʈ �信���� ������ �׸� ����
						DGListViewDeleteItem (dialogID, LISTVIEW_CAMERA_POS_NAME, DGListViewGetSelected (dialogID, LISTVIEW_CAMERA_POS_NAME, 0));
					}

					item = 0;
					break;

				case BUTTON_LOAD:	// �ε� ��ư
					// ������ �׸��� ī�޶� ������ ������
					if (DGListViewGetSelected (dialogID, LISTVIEW_CAMERA_POS_NAME, 0) != 0) {
						db = NULL;
						err_msg = NULL;
						rc = sqlite3_open ("cameraInfo.db", &db);

						if (rc != SQLITE_OK) {
							DGAlert (DG_ERROR, L"����", L"DB ���� ����.", "", L"Ȯ��", "", "");
							sqlite3_close (db);
							db = NULL;
						}

						// ���̺��� ������ �ڸ�Ʈ�� ���� �ʵ带 ������
						sql = "SELECT * FROM Cameras WHERE Comment = ?;";
						rc = sqlite3_prepare_v2 (db, sql, -1, &res, 0);
						sprintf (comment, "%s", DGListViewGetItemText (dialogID, LISTVIEW_CAMERA_POS_NAME, DGListViewGetSelected (dialogID, LISTVIEW_CAMERA_POS_NAME, 0)).ToCStr ().Get ());
						sqlite3_bind_text (res, 1, comment, -1, SQLITE_STATIC);
						rc = sqlite3_step (res);

						while (rc == SQLITE_ROW) {
							currentCamera.isPersp								= (GS::Bool8)sqlite3_column_int (res, 1);
							currentCamera.camGuid.time_low						= sqlite3_column_int (res, 2);
							currentCamera.camGuid.time_mid						= (unsigned short)sqlite3_column_int (res, 3);
							currentCamera.camGuid.time_hi_and_version			= (unsigned short)sqlite3_column_int (res, 4);
							currentCamera.camGuid.clock_seq_hi_and_reserved		= (unsigned char)sqlite3_column_int (res, 5);
							currentCamera.camGuid.clock_seq_low					= (unsigned char)sqlite3_column_int (res, 6);
							currentCamera.camGuid.node [0]						= (unsigned char)sqlite3_column_int (res, 7);
							currentCamera.camGuid.node [1]						= (unsigned char)sqlite3_column_int (res, 8);
							currentCamera.camGuid.node [2]						= (unsigned char)sqlite3_column_int (res, 9);
							currentCamera.camGuid.node [3]						= (unsigned char)sqlite3_column_int (res, 10);
							currentCamera.camGuid.node [4]						= (unsigned char)sqlite3_column_int (res, 11);
							currentCamera.camGuid.node [5]						= (unsigned char)sqlite3_column_int (res, 12);
							currentCamera.actCamSet.time_low					= sqlite3_column_int (res, 13);
							currentCamera.actCamSet.time_mid					= (unsigned short)sqlite3_column_int (res, 14);
							currentCamera.actCamSet.time_hi_and_version			= (unsigned short)sqlite3_column_int (res, 15);
							currentCamera.actCamSet.clock_seq_hi_and_reserved	= (unsigned char)sqlite3_column_int (res, 16);
							currentCamera.actCamSet.clock_seq_low				= (unsigned char)sqlite3_column_int (res, 17);
							currentCamera.actCamSet.node [0]					= (unsigned char)sqlite3_column_int (res, 18);
							currentCamera.actCamSet.node [1]					= (unsigned char)sqlite3_column_int (res, 19);
							currentCamera.actCamSet.node [2]					= (unsigned char)sqlite3_column_int (res, 20);
							currentCamera.actCamSet.node [3]					= (unsigned char)sqlite3_column_int (res, 21);
							currentCamera.actCamSet.node [4]					= (unsigned char)sqlite3_column_int (res, 22);
							currentCamera.actCamSet.node [5]					= (unsigned char)sqlite3_column_int (res, 23);
							currentCamera.u.persp.azimuth						= sqlite3_column_double (res, 24);
							currentCamera.u.persp.sunAzimuth					= sqlite3_column_double (res, 25);
							currentCamera.u.persp.sunAltitude					= sqlite3_column_double (res, 26);
							currentCamera.u.persp.viewCone						= sqlite3_column_double (res, 27);
							currentCamera.u.persp.rollAngle						= sqlite3_column_double (res, 28);
							currentCamera.u.persp.distance						= sqlite3_column_double (res, 29);
							currentCamera.u.persp.cameraZ						= sqlite3_column_double (res, 30);
							currentCamera.u.persp.targetZ						= sqlite3_column_double (res, 31);
							currentCamera.u.persp.pos.x							= sqlite3_column_double (res, 32);
							currentCamera.u.persp.pos.y							= sqlite3_column_double (res, 33);
							currentCamera.u.persp.target.x						= sqlite3_column_double (res, 34);
							currentCamera.u.persp.target.y						= sqlite3_column_double (res, 35);
							currentCamera.u.axono.azimuth						= sqlite3_column_double (res, 36);
							currentCamera.u.axono.sunAzimuth					= sqlite3_column_double (res, 37);
							currentCamera.u.axono.sunAltitude					= sqlite3_column_double (res, 38);
							currentCamera.u.axono.projMod						= (short)sqlite3_column_int (res, 39);
							currentCamera.u.axono.tranmat.tmx [0]				= sqlite3_column_double (res, 40);
							currentCamera.u.axono.tranmat.tmx [1]				= sqlite3_column_double (res, 41);
							currentCamera.u.axono.tranmat.tmx [2]				= sqlite3_column_double (res, 42);
							currentCamera.u.axono.tranmat.tmx [3]				= sqlite3_column_double (res, 43);
							currentCamera.u.axono.tranmat.tmx [4]				= sqlite3_column_double (res, 44);
							currentCamera.u.axono.tranmat.tmx [5]				= sqlite3_column_double (res, 45);
							currentCamera.u.axono.tranmat.tmx [6]				= sqlite3_column_double (res, 46);
							currentCamera.u.axono.tranmat.tmx [7]				= sqlite3_column_double (res, 47);
							currentCamera.u.axono.tranmat.tmx [8]				= sqlite3_column_double (res, 48);
							currentCamera.u.axono.tranmat.tmx [9]				= sqlite3_column_double (res, 49);
							currentCamera.u.axono.tranmat.tmx [10]				= sqlite3_column_double (res, 50);
							currentCamera.u.axono.tranmat.tmx [11]				= sqlite3_column_double (res, 51);
							currentCamera.u.axono.invtranmat.tmx [0]			= sqlite3_column_double (res, 52);
							currentCamera.u.axono.invtranmat.tmx [1]			= sqlite3_column_double (res, 53);
							currentCamera.u.axono.invtranmat.tmx [2]			= sqlite3_column_double (res, 54);
							currentCamera.u.axono.invtranmat.tmx [3]			= sqlite3_column_double (res, 55);
							currentCamera.u.axono.invtranmat.tmx [4]			= sqlite3_column_double (res, 56);
							currentCamera.u.axono.invtranmat.tmx [5]			= sqlite3_column_double (res, 57);
							currentCamera.u.axono.invtranmat.tmx [6]			= sqlite3_column_double (res, 58);
							currentCamera.u.axono.invtranmat.tmx [7]			= sqlite3_column_double (res, 59);
							currentCamera.u.axono.invtranmat.tmx [8]			= sqlite3_column_double (res, 60);
							currentCamera.u.axono.invtranmat.tmx [9]			= sqlite3_column_double (res, 61);
							currentCamera.u.axono.invtranmat.tmx [10]			= sqlite3_column_double (res, 62);
							currentCamera.u.axono.invtranmat.tmx [11]			= sqlite3_column_double (res, 63);
				
							// ���� ���� �ҷ���
							rc = sqlite3_step (res);
						}

						sqlite3_finalize (res);
						sqlite3_close (db);
						db = NULL;

						// ī�޶� ���� ����
						ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &currentCamera, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
					}
					break;

				case BUTTON_CLOSE:	// �ݱ� ��ư
					break;

				default:
					break;
			}
			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
			break;
	}

	result = item;

	return	result;
}
